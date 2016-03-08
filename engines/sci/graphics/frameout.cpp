/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/algorithm.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/list.h"
#include "common/str.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "engines/engine.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

#include "sci/sci.h"
#include "sci/console.h"
#include "sci/engine/kernel.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/engine/vm.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/coordadjuster.h"
#include "sci/graphics/compare.h"
#include "sci/graphics/font.h"
#include "sci/graphics/view.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/paint32.h"
#include "sci/graphics/palette32.h"
#include "sci/graphics/picture.h"
#include "sci/graphics/text32.h"
#include "sci/graphics/plane32.h"
#include "sci/graphics/screen_item32.h"
#include "sci/graphics/frameout.h"
#include "sci/video/robot_decoder.h"

namespace Sci {

// TODO/FIXME: This is partially guesswork

static int dissolveSequences[2][20] = {
	/* SCI2.1early- */ { 3, 6, 12, 20, 48, 96, 184, 272, 576, 1280, 3232, 6912, 13568, 24576, 46080 },
	/* SCI2.1mid+ */ { 0, 0, 3, 6, 12, 20, 48, 96, 184, 272, 576, 1280, 3232, 6912, 13568, 24576, 46080, 73728, 132096, 466944 }
};
static int16 divisionsDefaults[2][16] = {
	/* SCI2.1early- */ { 1, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 40, 40, 101, 101 },
	/* SCI2.1mid+ */   { 1, 20, 20, 20, 20, 10, 10, 10, 10, 20, 20,  6, 10, 101, 101, 2 }
};
static int16 unknownCDefaults[2][16] = {
	/* SCI2.1early- */ { 0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  0,  0,   0,   0 },
	/* SCI2.1mid+ */   { 0,  2,  2,  3,  3,  4,  4,  5,  5,  6,  6,  0,  0,   7,   7, 0 }
};

GfxFrameout::GfxFrameout(SegManager *segMan, ResourceManager *resMan, GfxCoordAdjuster *coordAdjuster, GfxCache *cache, GfxScreen *screen, GfxPalette32 *palette, GfxPaint32 *paint32) :
	_isHiRes(false),
	_cache(cache),
	_palette(palette),
	_resMan(resMan),
	_screen(screen),
	_segMan(segMan),
	_paint32(paint32),
	_showStyles(nullptr),
	// TODO: Stop using _gfxScreen
	_currentBuffer(screen->getDisplayWidth(), screen->getDisplayHeight(), nullptr),
	_priorityMap(screen->getDisplayWidth(), screen->getDisplayHeight(), nullptr),
	_remapOccurred(false),
	_frameNowVisible(false),
	_screenRect(screen->getDisplayWidth(), screen->getDisplayHeight()),
	_overdrawThreshold(0),
	_palMorphIsOn(false) {

	_currentBuffer.setPixels(calloc(1, screen->getDisplayWidth() * screen->getDisplayHeight()));

	for (int i = 0; i < 236; i += 2) {
		_styleRanges[i] = 0;
		_styleRanges[i + 1] = -1;
	}
	for (int i = 236; i < ARRAYSIZE(_styleRanges); ++i) {
		_styleRanges[i] = 0;
	}

	// TODO: Make hires detection work uniformly across all SCI engine
	// versions (this flag is normally passed by SCI::MakeGraphicsMgr
	// to the GraphicsMgr constructor depending upon video configuration,
	// so should be handled upstream based on game configuration instead
	// of here)
	if (getSciVersion() >= SCI_VERSION_2_1_EARLY && _resMan->detectHires()) {
		_isHiRes = true;
	}

	if (getSciVersion() < SCI_VERSION_2_1_MIDDLE) {
		_dissolveSequenceSeeds = dissolveSequences[0];
		_defaultDivisions = divisionsDefaults[0];
		_defaultUnknownC = unknownCDefaults[0];
	} else {
		_dissolveSequenceSeeds = dissolveSequences[1];
		_defaultDivisions = divisionsDefaults[1];
		_defaultUnknownC = unknownCDefaults[1];
	}

	// TODO: Nothing in the renderer really uses this. Currently,
	// the cursor renderer does, and kLocalToGlobal/kGlobalToLocal
	// do, but in the real engine (1) the cursor is handled in
	// frameOut, and (2) functions do a very simple lookup of the
	// plane and arithmetic with the plane's gameRect. In
	// principle, CoordAdjuster could be reused for
	// convertGameRectToPlaneRect, but it is not super clear yet
	// what the benefit would be to do that.
	_coordAdjuster = (GfxCoordAdjuster32 *)coordAdjuster;

	// TODO: Script resolution is hard-coded per game;
	// also this must be set or else the engine will crash
	_coordAdjuster->setScriptsResolution(_currentBuffer.scriptWidth, _currentBuffer.scriptHeight);
}

GfxFrameout::~GfxFrameout() {
	clear();
	CelObj::deinit();
	free(_currentBuffer.getPixels());
}

void GfxFrameout::run() {
	CelObj::init();
	Plane::init();
	ScreenItem::init();

	// NOTE: This happens in SCI::InitPlane in the actual engine,
	// and is a background fill plane to ensure hidden planes
	// (planes with a priority of -1) are never drawn
	Plane *initPlane = new Plane(Common::Rect(_currentBuffer.scriptWidth, _currentBuffer.scriptHeight));
	initPlane->_priority = 0;
	_planes.add(initPlane);
}

// SCI32 actually did not clear anything at all it seems on restore. The scripts actually cleared up
// planes + screen items right before restoring. And after restoring they sync'd its internal planes list
// as well.
void GfxFrameout::clear() {
	_planes.clear();
	_visiblePlanes.clear();
	_showList.clear();
}

// This is what Game::restore does, only needed when our ScummVM dialogs are patched in
// It actually does one pass before actual restore deleting screen items + planes
// And after restore it does another pass adding screen items + planes.
// Attention: at least Space Quest 6's option plane seems to stay in memory right from the start and is not re-created.
void GfxFrameout::syncWithScripts(bool addElements) {
	EngineState *engineState = g_sci->getEngineState();
	SegManager *segMan = engineState->_segMan;

	// In case original save/restore dialogs are active, don't do anything
	if (ConfMan.getBool("originalsaveload"))
		return;

	// Get planes list object
	reg_t planesListObject = engineState->variables[VAR_GLOBAL][10];
	reg_t planesListElements = readSelector(segMan, planesListObject, SELECTOR(elements));

	List *planesList = segMan->lookupList(planesListElements);
	reg_t planesNodeObject = planesList->first;

	// Go through all elements of planes::elements
	while (!planesNodeObject.isNull()) {
		Node *planesNode = segMan->lookupNode(planesNodeObject);
		reg_t planeObject = planesNode->value;

		if (addElements) {
			// Add this plane object
			kernelAddPlane(planeObject);
		}

		reg_t planeCastsObject = readSelector(segMan, planeObject, SELECTOR(casts));
		reg_t setListElements = readSelector(segMan, planeCastsObject, SELECTOR(elements));

		// Now go through all elements of plane::casts::elements
		List *planeCastsList = segMan->lookupList(setListElements);
		reg_t planeCastsNodeObject = planeCastsList->first;

		while (!planeCastsNodeObject.isNull()) {
			Node *castsNode = segMan->lookupNode(planeCastsNodeObject);
			reg_t castsObject = castsNode->value;

			reg_t castsListElements = readSelector(segMan, castsObject, SELECTOR(elements));

			List *castsList = segMan->lookupList(castsListElements);
			reg_t castNodeObject = castsList->first;

			while (!castNodeObject.isNull()) {
				Node *castNode = segMan->lookupNode(castNodeObject);
				reg_t castObject = castNode->value;

				// read selector "-info-" of this object
				// TODO: Seems to have been changed for SCI3
				// Do NOT use getInfoSelector in here. SCI3 games did not use infoToa, but an actual selector.
				// Maybe that selector is just a straight copy, but it needs to get verified/checked.
				uint16 castInfoSelector = readSelectorValue(segMan, castObject, SELECTOR(_info_));

				if (castInfoSelector & kInfoFlagViewInserted) {
					if (addElements) {
						// Flag set, so add this screen item
						kernelAddScreenItem(castObject);
					} else {
						// Flag set, so delete this screen item
						kernelDeleteScreenItem(castObject);
					}
				}

				castNodeObject = castNode->succ;
			}

			planeCastsNodeObject = castsNode->succ;
		}

		if (!addElements) {
			// Delete this plane object
			kernelDeletePlane(planeObject);
		}

		planesNodeObject = planesNode->succ;
	}
}

#pragma mark -
#pragma mark Screen items

void GfxFrameout::kernelAddScreenItem(const reg_t object) {
	const reg_t planeObject = readSelector(_segMan, object, SELECTOR(plane));

// TODO: Remove
//	debug("Adding screen item %04x:%04x to plane %04x:%04x", PRINT_REG(object), PRINT_REG(planeObject));

	_segMan->getObject(object)->setInfoSelectorFlag(kInfoFlagViewInserted);

	Plane *plane = _planes.findByObject(planeObject);
	if (plane == nullptr) {
		warning("screen item %x:%x (%s)", object.getSegment(), object.getOffset(), g_sci->getEngineState()->_segMan->getObjectName(object));
		warning("plane %x:%x (%s)", planeObject.getSegment(), planeObject.getOffset(), g_sci->getEngineState()->_segMan->getObjectName(planeObject));
		error("Invalid plane selector passed to kAddScreenItem");
	}

	ScreenItem *screenItem = plane->_screenItemList.findByObject(object);
	if (screenItem != nullptr) {
		screenItem->update(object);
	} else {
		screenItem = new ScreenItem(object);
		plane->_screenItemList.add(screenItem);
	}
}

void GfxFrameout::kernelUpdateScreenItem(const reg_t object) {
	const reg_t magnifierObject = readSelector(_segMan, object, SELECTOR(magnifier));
	if (magnifierObject.isNull()) {
		const reg_t planeObject = readSelector(_segMan, object, SELECTOR(plane));
		Plane *plane = _planes.findByObject(planeObject);
		if (plane == nullptr) {
			warning("screen item %x:%x (%s)", object.getSegment(), object.getOffset(), g_sci->getEngineState()->_segMan->getObjectName(object));
			warning("plane %x:%x (%s)", planeObject.getSegment(), planeObject.getOffset(), g_sci->getEngineState()->_segMan->getObjectName(planeObject));
			error("Invalid plane selector passed to kUpdateScreenItem");
		}

		ScreenItem *screenItem = plane->_screenItemList.findByObject(object);
		if (screenItem == nullptr) {
			warning("screen item %x:%x (%s)", object.getSegment(), object.getOffset(), g_sci->getEngineState()->_segMan->getObjectName(object));
			warning("plane %x:%x (%s)", planeObject.getSegment(), planeObject.getOffset(), g_sci->getEngineState()->_segMan->getObjectName(planeObject));
			error("Invalid screen item passed to kUpdateScreenItem");
		}

		screenItem->update(object);
	} else {
		error("Magnifier view is not known to be used by any game. Please submit a bug report with details about the game you were playing and what you were doing that triggered this error. Thanks!");
	}
}

void GfxFrameout::kernelDeleteScreenItem(const reg_t object) {
	_segMan->getObject(object)->clearInfoSelectorFlag(kInfoFlagViewInserted);

	const reg_t planeObject = readSelector(_segMan, object, SELECTOR(plane));
	Plane *plane = _planes.findByObject(planeObject);
	if (plane == nullptr) {
	// TODO: Remove
//		warning("Invalid plane selector %04x:%04x passed to kDeleteScreenItem (real engine ignores this)", PRINT_REG(object));
		return;
	}

// TODO: Remove
//	debug("Deleting screen item %04x:%04x from plane %04x:%04x", PRINT_REG(object), PRINT_REG(planeObject));

	ScreenItem *screenItem = plane->_screenItemList.findByObject(object);
	if (screenItem == nullptr) {
// TODO: Remove
//		warning("Invalid screen item %04x:%04x passed to kDeleteScreenItem (real engine ignores this)", PRINT_REG(object));
		return;
	}

	if (screenItem->_created == 0) {
		screenItem->_created = 0;
		screenItem->_updated = 0;
		screenItem->_deleted = getScreenCount();
	} else {
		plane->_screenItemList.erase(screenItem);
		plane->_screenItemList.pack();
	}
}

#pragma mark -
#pragma mark Planes

void GfxFrameout::kernelAddPlane(const reg_t object) {
	Plane *plane = _planes.findByObject(object);
	if (plane != nullptr) {
		plane->update(object);
		updatePlane(*plane);
	} else {
		plane = new Plane(object);
		addPlane(*plane);
	}
}

void GfxFrameout::kernelUpdatePlane(const reg_t object) {
	Plane *plane = _planes.findByObject(object);
	if (plane == nullptr) {
		warning("plane %x:%x (%s)", object.getSegment(), object.getOffset(), g_sci->getEngineState()->_segMan->getObjectName(object));
		error("Invalid plane selector passed to kUpdatePlane");
	}

	plane->update(object);
	updatePlane(*plane);
}

void GfxFrameout::kernelDeletePlane(const reg_t object) {
	Plane *plane = _planes.findByObject(object);
	if (plane == nullptr) {
		warning("plane %x:%x (%s)", object.getSegment(), object.getOffset(), g_sci->getEngineState()->_segMan->getObjectName(object));
		error("Invalid plane selector passed to kDeletePlane");
	}

	if (plane->_created) {
		// NOTE: The original engine calls some `AbortPlane` function that
		// just ends up doing this anyway so we skip the extra indirection
		_planes.erase(plane);
	} else {
		// TODO: Remove
//		debug("Deleting plane %04x:%04x", PRINT_REG(object));
		plane->_created = 0;
		plane->_deleted = g_sci->_gfxFrameout->getScreenCount();
	}
}

void GfxFrameout::deletePlane(Plane &planeToFind) {
	Plane *plane = _planes.findByObject(planeToFind._object);
	if (plane == nullptr) {
		error("Invalid plane passed to deletePlane");
	}

	if (plane->_created) {
		_planes.erase(plane);
	} else {
		plane->_created = 0;
		plane->_moved = 0;
		plane->_deleted = getScreenCount();
	}
}

int16 GfxFrameout::kernelGetHighPlanePri() {
	return _planes.getTopSciPlanePriority();
}

void GfxFrameout::addPlane(Plane &plane) {
	if (_planes.findByObject(plane._object) == nullptr) {
		plane.clipScreenRect(_screenRect);
		_planes.add(&plane);
	} else {
		plane._deleted = 0;
		if (plane._created == 0) {
			plane._moved = g_sci->_gfxFrameout->getScreenCount();
		}
		_planes.sort();
	}
}

void GfxFrameout::updatePlane(Plane &plane) {
	// NOTE: This assertion comes from SCI engine code.
	assert(_planes.findByObject(plane._object) == &plane);

	Plane *visiblePlane = _visiblePlanes.findByObject(plane._object);
	plane.sync(visiblePlane, _screenRect);
	// NOTE: updateScreenRect was originally called a second time here,
	// but it is already called at the end of the Plane::Update call
	// in the original engine anyway.

	_planes.sort();
}

#pragma mark -
#pragma mark Pics

void GfxFrameout::kernelAddPicAt(const reg_t planeObject, const GuiResourceId pictureId, const int16 x, const int16 y, const bool mirrorX) {
	Plane *plane = _planes.findByObject(planeObject);
	if (plane == nullptr) {
		warning("plane %x:%x (%s)", planeObject.getSegment(), planeObject.getOffset(), g_sci->getEngineState()->_segMan->getObjectName(planeObject));
		error("Invalid plane selector passed to kAddPicAt");
	}
	plane->addPic(pictureId, Common::Point(x, y), mirrorX);
}

#pragma mark -
#pragma mark Rendering

void GfxFrameout::frameOut(const bool shouldShowBits, const Common::Rect &rect) {
// TODO: Robot
//	if (_robot != nullptr) {
//		_robot.doRobot();
//	}

	// NOTE: The original engine allocated these as static arrays of 100
	// pointers to ScreenItemList / RectList
	ScreenItemListList screenItemLists;
	EraseListList eraseLists;

	screenItemLists.resize(_planes.size());
	eraseLists.resize(_planes.size());

	// _numActiveRemaps was a global in SCI engine
	if (/* TODO Remap::_numActiveRemaps > 0 */ false && _remapOccurred) {
		// remapMarkRedraw();
	}

	calcLists(screenItemLists, eraseLists, rect);

	for (ScreenItemListList::iterator list = screenItemLists.begin(); list != screenItemLists.end(); ++list) {
		list->sort();
	}

	for (ScreenItemListList::iterator list = screenItemLists.begin(); list != screenItemLists.end(); ++list) {
		for (DrawList::iterator drawItem = list->begin(); drawItem != list->end(); ++drawItem) {
			(*drawItem)->screenItem->getCelObj().submitPalette();
		}
	}

	_remapOccurred = _palette->updateForFrame();

	// NOTE: SCI engine set this to false on each loop through the
	// planelist iterator below. Since that is a waste, we only set
	// it once.
	_frameNowVisible = false;

	for (PlaneList::size_type i = 0; i < _planes.size(); ++i) {
		drawEraseList(eraseLists[i], *_planes[i]);
		drawScreenItemList(screenItemLists[i]);
	}

// TODO: Robot
//	if (_robot != nullptr) {
//		_robot->frameAlmostVisible();
//	}

	_palette->updateHardware();

	if (shouldShowBits) {
		showBits();
	}

	_frameNowVisible = true;

// TODO: Robot
//	if (_robot != nullptr) {
//		robot->frameNowVisible();
//	}
}

// Determine the parts of 'r' that aren't overlapped by 'other'.
// Returns -1 if r and other have no intersection.
// Returns number of returned parts (in outRects) otherwise.
// (In particular, this returns 0 if r is contained in other.)
int splitRects(Common::Rect r, const Common::Rect &other, Common::Rect(&outRects)[4]) {
	if (!r.intersects(other)) {
		return -1;
	}

	int count = 0;
	if (r.top < other.top) {
		Common::Rect &t = outRects[count++];
		t = r;
		t.bottom = other.top;
		r.top = other.top;
	}

	if (r.bottom > other.bottom) {
		Common::Rect &t = outRects[count++];
		t = r;
		t.top = other.bottom;
		r.bottom = other.bottom;
	}

	if (r.left < other.left) {
		Common::Rect &t = outRects[count++];
		t = r;
		t.right = other.left;
		r.left = other.left;
	}

	if (r.right > other.right) {
		Common::Rect &t = outRects[count++];
		t = r;
		t.left = other.right;
	}

	return count;
}

void GfxFrameout::calcLists(ScreenItemListList &drawLists, EraseListList &eraseLists, const Common::Rect &calcRect) {
	RectList rectlist;
	Common::Rect outRects[4];

	int deletedPlaneCount = 0;
	bool addedToRectList = false;
	int planeCount = _planes.size();
	bool foundTransparentPlane = false;

	if (!calcRect.isEmpty()) {
		addedToRectList = true;
		rectlist.add(calcRect);
	}

	for (int outerPlaneIndex = 0; outerPlaneIndex < planeCount; ++outerPlaneIndex) {
		Plane *outerPlane = _planes[outerPlaneIndex];

		if (outerPlane->_type == kPlaneTypeTransparent) {
			foundTransparentPlane = true;
		}

		Plane *visiblePlane = _visiblePlanes.findByObject(outerPlane->_object);

		if (outerPlane->_deleted) {
			if (visiblePlane != nullptr) {
				if (!visiblePlane->_screenRect.isEmpty()) {
					addedToRectList = true;
					rectlist.add(visiblePlane->_screenRect);
				}
			}
			++deletedPlaneCount;
		} else if (visiblePlane != nullptr) {
			if (outerPlane->_updated) {
				--outerPlane->_updated;

				int splitcount = splitRects(visiblePlane->_screenRect, outerPlane->_screenRect, outRects);
				if (splitcount) {
					if (splitcount == -1) {
						if (!visiblePlane->_screenRect.isEmpty()) {
							rectlist.add(visiblePlane->_screenRect);
						}
					} else {
						for (int i = 0; i < splitcount; ++i) {
							rectlist.add(outRects[i]);
						}
					}

					addedToRectList = true;
				}

				if (!outerPlane->_redrawAllCount) {
					int splitCount = splitRects(outerPlane->_screenRect, visiblePlane->_screenRect, outRects);
					if (splitCount) {
						for (int i = 0; i < splitCount; ++i) {
							rectlist.add(outRects[i]);
						}
						addedToRectList = true;
					}
				}
			}
		}

		if (addedToRectList) {
			for (RectList::iterator rect = rectlist.begin(); rect != rectlist.end(); ++rect) {
				for (int innerPlaneIndex = _planes.size() - 1; innerPlaneIndex >= 0; --innerPlaneIndex) {
					Plane *innerPlane = _planes[innerPlaneIndex];

					if (!innerPlane->_deleted && innerPlane->_type != kPlaneTypeTransparent && innerPlane->_screenRect.intersects(**rect)) {
						if (innerPlane->_redrawAllCount == 0) {
							eraseLists[innerPlaneIndex].add(innerPlane->_screenRect.findIntersectingRect(**rect));
						}

						int splitCount = splitRects(**rect, innerPlane->_screenRect, outRects);
						for (int i = 0; i < splitCount; ++i) {
							rectlist.add(outRects[i]);
						}

						rectlist.erase(rect);
						break;
					}
				}
			}

			rectlist.pack();
		}
	}

	// clean up deleted planes
	if (deletedPlaneCount) {
		for (int planeIndex = planeCount - 1; planeIndex >= 0; --planeIndex) {
			Plane *plane = _planes[planeIndex];

			if (plane->_deleted) {
				--plane->_deleted;
				if (plane->_deleted <= 0) {
					PlaneList::iterator visiblePlaneIt = Common::find_if(_visiblePlanes.begin(), _visiblePlanes.end(), FindByObject<Plane *>(plane->_object));
					if (visiblePlaneIt != _visiblePlanes.end()) {
						_visiblePlanes.erase(visiblePlaneIt);
					}

					_planes.remove_at(planeIndex);
					eraseLists.remove_at(planeIndex);
					drawLists.remove_at(planeIndex);
				}

				if (--deletedPlaneCount <= 0) {
					break;
				}
			}
		}
	}

	planeCount = _planes.size();
	for (int outerIndex = 0; outerIndex < planeCount; ++outerIndex) {
		// "outer" just refers to the outer loop
		Plane *outerPlane = _planes[outerIndex];
		if (outerPlane->_priorityChanged) {
			--outerPlane->_priorityChanged;

			Plane *visibleOuterPlane = _visiblePlanes.findByObject(outerPlane->_object);

			rectlist.add(outerPlane->_screenRect.findIntersectingRect(visibleOuterPlane->_screenRect));

			for (int innerIndex = planeCount - 1; innerIndex >= 0; --innerIndex) {
				// "inner" just refers to the inner loop
				Plane *innerPlane = _planes[innerIndex];
				Plane *visibleInnerPlane = _visiblePlanes.findByObject(innerPlane->_object);

				int rectCount = rectlist.size();
				for (int rectIndex = 0; rectIndex < rectCount; ++rectIndex) {
					int splitCount = splitRects(*rectlist[rectIndex], _planes[innerIndex]->_screenRect, outRects);

					if (splitCount == 0) {
						if (visibleInnerPlane != nullptr) {
							// same priority, or relative priority between inner/outer changed
							if ((visibleOuterPlane->_priority - visibleInnerPlane->_priority) * (outerPlane->_priority - innerPlane->_priority) <= 0) {
								if (outerPlane->_priority <= innerPlane->_priority) {
									eraseLists[innerIndex].add(*rectlist[rectIndex]);
								} else {
									eraseLists[outerIndex].add(*rectlist[rectIndex]);
								}
							}
						}

						rectlist.erase_at(rectIndex);
					} else if (splitCount != -1) {
						for (int i = 0; i < splitCount; ++i) {
							rectlist.add(outRects[i]);
						}

						if (visibleInnerPlane != nullptr) {
							// same priority, or relative priority between inner/outer changed
							if ((visibleOuterPlane->_priority - visibleInnerPlane->_priority) * (outerPlane->_priority - innerPlane->_priority) <= 0) {
								*rectlist[rectIndex] = outerPlane->_screenRect.findIntersectingRect(innerPlane->_screenRect);
								if (outerPlane->_priority <= innerPlane->_priority) {
									eraseLists[innerIndex].add(*rectlist[rectIndex]);
								}
								else {
									eraseLists[outerIndex].add(*rectlist[rectIndex]);
								}
							}
						}
						rectlist.erase_at(rectIndex);
					}
				}
				rectlist.pack();
			}
		}
	}

	for (int planeIndex = 0; planeIndex < planeCount; ++planeIndex) {
		Plane *plane = _planes[planeIndex];
		Plane *visiblePlane = nullptr;

		PlaneList::iterator visiblePlaneIt = Common::find_if(_visiblePlanes.begin(), _visiblePlanes.end(), FindByObject<Plane *>(plane->_object));
		if (visiblePlaneIt != _visiblePlanes.end()) {
			visiblePlane = *visiblePlaneIt;
		}

		if (plane->_redrawAllCount) {
			plane->redrawAll(visiblePlane, _planes, drawLists[planeIndex], eraseLists[planeIndex]);
		} else {
			if (visiblePlane == nullptr) {
				error("Missing visible plane for source plane %04x:%04x", PRINT_REG(plane->_object));
			}

			plane->calcLists(*visiblePlane, _planes, drawLists[planeIndex], eraseLists[planeIndex]);
		}

		if (plane->_created) {
			_visiblePlanes.add(new Plane(*plane));
			--plane->_created;
		} else if (plane->_moved) {
			assert(visiblePlaneIt != _visiblePlanes.end());
			**visiblePlaneIt = *plane;
			--plane->_moved;
		}
	}

	if (foundTransparentPlane) {
		for (int planeIndex = 0; planeIndex < planeCount; ++planeIndex) {
			for (int i = planeIndex + 1; i < planeCount; ++i) {
				if (_planes[i]->_type == kPlaneTypeTransparent) {
					_planes[i]->filterUpEraseRects(drawLists[i], eraseLists[planeIndex]);
				}
			}

			if (_planes[planeIndex]->_type == kPlaneTypeTransparent) {
				for (int i = planeIndex - 1; i >= 0; --i) {
					_planes[i]->filterDownEraseRects(drawLists[i], eraseLists[i], eraseLists[planeIndex]);
				}

				if (eraseLists[planeIndex].size() > 0) {
					error("Transparent plane's erase list not absorbed");
				}
			}

			for (int i = planeIndex + 1; i < planeCount; ++i) {
				if (_planes[i]->_type == kPlaneTypeTransparent) {
					_planes[i]->filterUpDrawRects(drawLists[i], drawLists[planeIndex]);
				}
			}
		}
	}
}

void GfxFrameout::drawEraseList(const RectList &eraseList, const Plane &plane) {
	if (plane._type != kPlaneTypeColored) {
		return;
	}

	for (RectList::const_iterator it = eraseList.begin(); it != eraseList.end(); ++it) {
		mergeToShowList(**it, _showList, _overdrawThreshold);
		_currentBuffer.fillRect(**it, plane._back);
	}
}

void GfxFrameout::drawScreenItemList(const DrawList &screenItemList) {
	_hasRemappedScreenItem = false;
	if (/* TODO: g_Remap_UnknownCounter2 */ false && !_priorityMap.isNull()) {
		for (DrawList::const_iterator it = screenItemList.begin(); it != screenItemList.end(); ++it) {
			if ((*it)->screenItem->getCelObj()._remap) {
				_hasRemappedScreenItem = true;
				break;
			}
		}
	}

	for (DrawList::const_iterator it = screenItemList.begin(); it != screenItemList.end(); ++it) {
		DrawItem &drawItem = **it;
		mergeToShowList(drawItem.rect, _showList, _overdrawThreshold);
		ScreenItem &screenItem = *drawItem.screenItem;
		// TODO: Remove
//		debug("Drawing item %04x:%04x to %d %d %d %d", PRINT_REG(screenItem._object), PRINT_RECT(drawItem.rect));
		CelObj &celObj = *screenItem._celObj;
		celObj.draw(_currentBuffer, screenItem, drawItem.rect, screenItem._mirrorX ^ celObj._mirrorX);
	}
}

void GfxFrameout::mergeToShowList(const Common::Rect &drawRect, RectList &showList, const int overdrawThreshold) {
	Common::Rect merged(drawRect);

	bool didDelete = true;
	RectList::size_type count = showList.size();
	while (didDelete && count) {
		didDelete = false;

		for (RectList::size_type i = 0; i < count; ++i) {
			Common::Rect existing = *showList[i];
			Common::Rect candidate;
			candidate.left = MIN(merged.left, existing.left);
			candidate.top = MIN(merged.top, existing.top);
			candidate.right = MAX(merged.right, existing.right);
			candidate.bottom = MAX(merged.bottom, existing.bottom);

			if (candidate.height() * candidate.width() - merged.width() * merged.height() - existing.width() * existing.height() <= overdrawThreshold) {
				merged = candidate;
				showList.erase_at(i);
				didDelete = true;
			}
		}

		count = showList.pack();
	}

	showList.add(merged);
}

void GfxFrameout::palMorphFrameOut(const int8 *styleRanges, const ShowStyleEntry *showStyle) {
	Palette sourcePalette(*_palette->getNextPalette());
	alterVmap(sourcePalette, sourcePalette, -1, styleRanges);

	// TODO: unsure if this is what this variable actually
	// represents, but it is the correct variable number
	int16 lastRoom = g_sci->getEngineState()->variables[VAR_GLOBAL][12].toSint16();

	Common::Rect rect(_screen->getDisplayWidth(), _screen->getDisplayHeight());
	_showList.add(rect);
	showBits();

	Common::Rect calcRect(0, 0);

	// NOTE: The original engine allocated these as static arrays of 100
	// pointers to ScreenItemList / RectList
	ScreenItemListList screenItemLists;
	EraseListList eraseLists;

	screenItemLists.resize(_planes.size());
	eraseLists.resize(_planes.size());

	// TODO: Remap
	// _numActiveRemaps was a global in SCI engine
	// if (Remap::_numActiveRemaps > 0 && _remapOccurred) {
		// _screen->remapMarkRedraw();
	// }

	calcLists(screenItemLists, eraseLists, calcRect);
	for (ScreenItemListList::iterator list = screenItemLists.begin(); list != screenItemLists.end(); ++list) {
		list->sort();
	}

	for (ScreenItemListList::iterator list = screenItemLists.begin(); list != screenItemLists.end(); ++list) {
		for (DrawList::iterator drawItem = list->begin(); drawItem != list->end(); ++drawItem) {
			(*drawItem)->screenItem->getCelObj().submitPalette();
		}
	}

	_remapOccurred = _palette->updateForFrame();
	_frameNowVisible = false;

	for (PlaneList::size_type i = 0; i < _planes.size(); ++i) {
		drawEraseList(eraseLists[i], *_planes[i]);
		drawScreenItemList(screenItemLists[i]);
	}

	Palette nextPalette(*_palette->getNextPalette());

	if (lastRoom < 1000) {
		for (int i = 0; i < ARRAYSIZE(sourcePalette.colors); ++i) {
			if (styleRanges[i] == -1 || styleRanges[i] == 0) {
				sourcePalette.colors[i] = nextPalette.colors[i];
				sourcePalette.colors[i].used = true;
			}
		}
	} else {
		for (int i = 0; i < ARRAYSIZE(sourcePalette.colors); ++i) {
			if (styleRanges[i] == -1 || (styleRanges[i] == 0 && i > 71 && i < 104)) {
				sourcePalette.colors[i] = nextPalette.colors[i];
				sourcePalette.colors[i].used = true;
			}
		}
	}

	_palette->submit(sourcePalette);
	_palette->updateFFrame();
	_palette->updateHardware();
	alterVmap(nextPalette, sourcePalette, 1, _styleRanges);

	if (showStyle && showStyle->type != kShowStyleUnknown) {
// TODO: SCI2.1mid transition effects
//		processEffects();
		warning("Transition not implemented!");
	} else {
		showBits();
	}

	_frameNowVisible = true;

	for (PlaneList::iterator plane = _planes.begin(); plane != _planes.end(); ++plane) {
// TODO:
//		plane->updateRedrawAllCount();
	}

	// TODO: Remap
	// _numActiveRemaps was a global in SCI engine
	// if (Remap::_numActiveRemaps > 0 && _remapOccurred) {
	// _screen->remapMarkRedraw();
	// }

	calcLists(screenItemLists, eraseLists, calcRect);
	for (ScreenItemListList::iterator list = screenItemLists.begin(); list != screenItemLists.end(); ++list) {
		list->sort();
	}

	for (ScreenItemListList::iterator list = screenItemLists.begin(); list != screenItemLists.end(); ++list) {
		for (DrawList::iterator drawItem = list->begin(); drawItem != list->end(); ++drawItem) {
			(*drawItem)->screenItem->getCelObj().submitPalette();
		}
	}

	_remapOccurred = _palette->updateForFrame();
	// NOTE: During this second loop, `_frameNowVisible = false` is
	// inside the next loop in SCI2.1mid
	_frameNowVisible = false;

	for (PlaneList::size_type i = 0; i < _planes.size(); ++i) {
		drawEraseList(eraseLists[i], *_planes[i]);
		drawScreenItemList(screenItemLists[i]);
	}

	_palette->submit(nextPalette);
	_palette->updateFFrame();
	_palette->updateHardware();
	showBits();

	_frameNowVisible = true;
}

// TODO: What does the bit masking for the show rects do,
// and does it cause an off-by-one error in rect calculations
// since SOL_Rect is BR inclusive and Common::Rect is BR
// exclusive?
void GfxFrameout::showBits() {
	for (RectList::const_iterator rect = _showList.begin(); rect != _showList.end(); ++rect) {
		Common::Rect rounded(**rect);
		// NOTE: SCI engine used BR-inclusive rects so used slightly
		// different masking here to ensure that the width of rects
		// was always even.
		rounded.left &= ~1;
		rounded.right = (rounded.right + 1) & ~1;

		// TODO:
		// _cursor->GonnaPaint(rounded);
	}

	// TODO:
	// _cursor->PaintStarting();

	for (RectList::const_iterator rect = _showList.begin(); rect != _showList.end(); ++rect) {
		Common::Rect rounded(**rect);
		// NOTE: SCI engine used BR-inclusive rects so used slightly
		// different masking here to ensure that the width of rects
		// was always even.
		rounded.left &= ~1;
		rounded.right = (rounded.right + 1) & ~1;

		byte *sourceBuffer = (byte *)_currentBuffer.getPixels() + rounded.top * _currentBuffer.screenWidth + rounded.left;

		g_system->copyRectToScreen(sourceBuffer, _currentBuffer.screenWidth, rounded.left, rounded.top, rounded.width(), rounded.height());
	}

	// TODO:
	// _cursor->DonePainting();

	_showList.clear();
}

void GfxFrameout::alterVmap(const Palette &palette1, const Palette &palette2, const int8 style, const int8 *const styleRanges) {
	uint8 clut[256];

	for (int paletteIndex = 0; paletteIndex < ARRAYSIZE(palette1.colors); ++paletteIndex) {
		int outerR = palette1.colors[paletteIndex].r;
		int outerG = palette1.colors[paletteIndex].g;
		int outerB = palette1.colors[paletteIndex].b;

		if (styleRanges[paletteIndex] == style) {
			int minDiff = 262140;
			int minDiffIndex;

			for (int i = 0; i < 236; ++i) {
				if (styleRanges[i] != style) {
					int r = palette1.colors[i].r;
					int g = palette1.colors[i].g;
					int b = palette1.colors[i].b;
					int diffSquared = (outerR - r) * (outerR - r) + (outerG - g) * (outerG - g) + (outerB - b) * (outerB - b);
					if (diffSquared < minDiff) {
						minDiff = diffSquared;
						minDiffIndex = i;
					}
				}
			}

			clut[paletteIndex] = minDiffIndex;
		}

		if (style == 1 && styleRanges[paletteIndex] == 0) {
			int minDiff = 262140;
			int minDiffIndex;

			for (int i = 0; i < 236; ++i) {
				int r = palette2.colors[i].r;
				int g = palette2.colors[i].g;
				int b = palette2.colors[i].b;

				int diffSquared = (outerR - r) * (outerR - r) + (outerG - g) * (outerG - g) + (outerB - b) * (outerB - b);
				if (diffSquared < minDiff) {
					minDiff = diffSquared;
					minDiffIndex = i;
				}
			}

			clut[paletteIndex] = minDiffIndex;
		}
	}

	// NOTE: This is currBuffer->ptr in SCI engine
	byte *pixels = (byte *)_currentBuffer.getPixels();

	for (int pixelIndex = 0, numPixels = _currentBuffer.screenWidth * _currentBuffer.screenHeight; pixelIndex < numPixels; ++pixelIndex) {
		byte currentValue = pixels[pixelIndex];
		int8 styleRangeValue = styleRanges[currentValue];
		if (styleRangeValue == -1 && styleRangeValue == style) {
			currentValue = pixels[pixelIndex] = clut[currentValue];
			styleRangeValue = styleRanges[clut[currentValue]];
		}

		if (
			(styleRangeValue == 1 && styleRangeValue == style) ||
			(styleRangeValue == 0 && style == 1)
		) {
			pixels[pixelIndex] = clut[currentValue];
		}
	}
}

void GfxFrameout::kernelSetPalStyleRange(const uint8 fromColor, const uint8 toColor) {
	if (toColor > fromColor) {
		return;
	}

	for (int i = fromColor; i < toColor; ++i) {
		_styleRanges[i] = 0;
	}
}

inline ShowStyleEntry * GfxFrameout::findShowStyleForPlane(const reg_t planeObj) const {
	ShowStyleEntry *entry = _showStyles;
	while (entry != nullptr) {
		if (entry->plane == planeObj) {
			break;
		}
		entry = entry->next;
	}

	return entry;
}

inline ShowStyleEntry *GfxFrameout::deleteShowStyleInternal(ShowStyleEntry *const showStyle) {
	ShowStyleEntry *lastEntry = nullptr;

	for (ShowStyleEntry *testEntry = _showStyles; testEntry != nullptr; testEntry = testEntry->next) {
		if (testEntry == showStyle) {
			break;
		}
		lastEntry = testEntry;
	}

	if (lastEntry == nullptr) {
		_showStyles = showStyle->next;
		lastEntry = _showStyles;
	} else {
		lastEntry->next = showStyle->next;
	}

	// NOTE: Differences from SCI2/2.1early engine:
	// 1. Memory of ShowStyle-owned objects was freed before ShowStyle was
	//    removed from the linked list, but since this operation is position
	//    independent, it has been moved after removal from the list for
	//    consistency with SCI2.1mid+
	// 2. In SCI2, `screenItems` was a pointer to an array of pointers, so
	//    extra deletes were performed here; we use an owned container object
	//    instead, which is automatically freed when ShowStyle is freed
#if 0
	if (getSciVersion() < SCI_VERSION_2_1_MIDDLE) {
		uint8 type = showStyle->type;

		if (type >= 1 && type <= 10) {
			ScreenItemList &styleItems = showStyle->screenItems;
			for (ScreenItemList::iterator it = styleItems.begin(); it != styleItems.end(); ++it) {
				if (active) {
					// TODO: _screen->deleteScreenItem(showStyle->plane, *it->id);
					_screenItems.remove(*it);
				}
				delete *it;
			}
		} else if (type == 11 || type == 12) {
			if (!showStyle->bitmapMemId.isNull()) {
				_segMan->freeHunkEntry(showStyle->bitmapMemId);
			}
			if (showStyle->bitmapScreenItem != nullptr) {
				// TODO: _screen->deleteScreenItem(showStyle->plane, showStyle->bitmapScreenItem->id);
				_screenItems.remove(showStyle->bitmapScreenItem);
				delete showStyle->bitmapScreenItem;
			}
		}
	} else {
#endif
		delete[] showStyle->fadeColorRanges;
#if 0
	}
#endif

	delete showStyle;

	// TODO: Verify that this is the correct entry to return
	// for the loop in processShowStyles to work correctly
	return lastEntry;
}

// TODO: 10-argument version is only in SCI3; argc checks are currently wrong for this version
// and need to be fixed in future
// TODO: SQ6 does not use 'priority' (exists since SCI2) or 'blackScreen' (exists since SCI3);
// check to see if other versions use or if they are just always ignored
void GfxFrameout::kernelSetShowStyle(const uint16 argc, const reg_t &planeObj, const ShowStyleType type, const int16 seconds, const int16 back, const int16 priority, const int16 animate, const int16 frameOutNow, const reg_t &pFadeArray, const int16 divisions, const int16 blackScreen) {

	bool hasDivisions = false;
	bool hasFadeArray = false;
	if (getSciVersion() < SCI_VERSION_2_1_MIDDLE) {
		hasDivisions = argc > 7;
		hasFadeArray = false;
	} else if (getSciVersion() < SCI_VERSION_3) {
		hasDivisions = argc > 8;
		hasFadeArray = argc > 7;
	} else {
		hasDivisions = argc > 9;
		hasFadeArray = argc > 8;
	}

	bool isFadeUp;
	int16 color;
	if (back != -1) {
		isFadeUp = false;
		color = back;
	} else {
		isFadeUp = true;
		color = 0;
	}

	if ((getSciVersion() < SCI_VERSION_2_1_MIDDLE && type == 15) || type > 15) {
		error("Illegal show style %d for plane %04x:%04x", type, PRINT_REG(planeObj));
	}

	Plane *plane = _planes.findByObject(planeObj);
	if (plane == nullptr) {
		error("Plane %04x:%04x is not present in active planes list", PRINT_REG(planeObj));
	}

	// TODO: This is Plane.gameRect in SCI engine, not planeRect. Engine uses
	// Plane::ConvGameRectToPlaneRect to convert from gameRect to planeRect.
	// Also this never gets used by SQ6 so it is not clear what it does yet
	//	Common::Rect gameRect = plane.planeRect;

	bool createNewEntry = true;
	ShowStyleEntry *entry = findShowStyleForPlane(planeObj);
	if (entry != nullptr) {
		bool useExisting = true;

		if (getSciVersion() < SCI_VERSION_2_1_MIDDLE) {
			useExisting = plane->_planeRect.width() == entry->width && plane->_planeRect.height() == entry->height;
		}

		if (useExisting) {
			useExisting = entry->divisions == (hasDivisions ? divisions : _defaultDivisions[type]) && entry->unknownC == _defaultUnknownC[type];
		}

		if (useExisting) {
			createNewEntry = false;
			isFadeUp = true;
			entry->currentStep = 0;
		} else {
			isFadeUp = true;
			color = entry->color;
			deleteShowStyleInternal(entry/*, true*/);
			entry = nullptr;
		}
	}

	if (type > 0) {
		if (createNewEntry) {
			entry = new ShowStyleEntry;
			// NOTE: SCI2.1 engine tests if allocation returned a null pointer
			// but then only avoids setting currentStep if this is so. Since
			// this is a nonsensical approach, we do not do that here
			entry->currentStep = 0;
			entry->unknownC = _defaultUnknownC[type];
			entry->processed = false;
			entry->divisions = hasDivisions ? divisions : _defaultDivisions[type];
			entry->plane = planeObj;

#if 0
			if (getSciVersion() < SCI_VERSION_2_1_MIDDLE) {
				entry->bitmapMemId = NULL_REG;
				entry->screenItems.empty();
				entry->width = plane->_planeRect.width();
				entry->height = plane->_planeRect.height();
			} else {
#endif
				entry->fadeColorRanges = nullptr;
				if (hasFadeArray) {
					// NOTE: SCI2.1mid engine does no check to verify that an array is
					// successfully retrieved, and SegMan will cause a fatal error
					// if we try to use a memory segment that is not an array
					SciArray<reg_t> *table = _segMan->lookupArray(pFadeArray);

					uint32 rangeCount = table->getSize();
					entry->fadeColorRangesCount = rangeCount;

					// NOTE: SCI engine code always allocates memory even if the range
					// table has no entries, but this does not really make sense, so
					// we avoid the allocation call in this case
					if (rangeCount > 0) {
						entry->fadeColorRanges = new uint16[rangeCount];
						for (size_t i = 0; i < rangeCount; ++i) {
							entry->fadeColorRanges[i] = table->getValue(i).toUint16();
						}
					}
				} else {
					entry->fadeColorRangesCount = 0;
				}
#if 0
			}
#endif
		}

		// NOTE: The original engine had no nullptr check and would just crash
		// if it got to here
		if (entry == nullptr) {
			error("Cannot edit non-existing ShowStyle entry");
		}

		entry->fadeUp = isFadeUp;
		entry->color = color;
		entry->nextTick = g_sci->getTickCount();
		entry->type = type;
		entry->animate = animate;
		entry->delay = (seconds * 60 + entry->divisions - 1) / entry->divisions;

		if (entry->delay == 0) {
#if 0
			if (getSciVersion() >= SCI_VERSION_2_1_MIDDLE && entry->fadeColorRanges != nullptr) {
#endif
			if (entry->fadeColorRanges != nullptr) {
				delete[] entry->fadeColorRanges;
			}
			delete entry;
			error("ShowStyle has no duration");
		}

		if (frameOutNow) {
			Common::Rect frameOutRect(0, 0);
			frameOut(false, frameOutRect);
		}

		if (createNewEntry) {
			// TODO: Implement SCI3, which may or may not actually have
			// the same transitions as SCI2/SCI2.1early, but implemented
			// differently
#if 0
			if (getSciVersion() < SCI_VERSION_2_1_MIDDLE) {
				switch (entry->type) {
					case kShowStyleHShutterIn:
					case kShowStyleHShutterOut:
						prepareShowStyleWipe(entry, priority, 2, true);
						break;

					case kShowStyleVShutterIn:
					case kShowStyleVShutterOut:
						prepareShowStyleWipe(entry, priority, 2, false);
						break;

					case kShowStyleHWipe1:
					case kShowStyleHWipe2:
						prepareShowStyleWipe(entry, priority, 1, true);
						break;

					case kShowStyleVWipe1:
					case kShowStyleVWipe2:
						prepareShowStyleWipe(entry, priority, 1, false);
						break;

					case kShowStyleIrisIn:
					case kShowStyleIrisOut:
						prepareShowStyleIris(entry, priority);
						break;

					case kShowStyle11:
					case kShowStyle12:
						prepareShowStylePixels(entry, priority, plane->planeRect);
						break;

					default:
						break;
				}
			}
#endif

			entry->next = _showStyles;
			_showStyles = entry;
		}
	}
}

#if 0
void addFrameoutEntryInternal(ShowStyleEntry *const showStyle, const int16 priority, const CelInfo32 &celInfo, const Common::Rect &rect) {
	ScreenItem *screenItem = new ScreenItem;
	screenItem->plane = showStyle->plane;
	screenItem->celInfo = celInfo;
	screenItem->celRect = rect;
	screenItem->isInList = true;
	screenItem->priority = priority;
	screenItem->visible = true;
	showStyle->screenItems.push_back(screenItem);
}

void GfxFrameout::prepareShowStyleWipe(ShowStyleEntry *const showStyle, const int16 priority, const int16 edgeCount, const bool horizontal) {
	assert(edgeCount == 1 || edgeCount == 2);

	const int numScreenItems = showStyle->divisions * edgeCount;
	const int extra = edgeCount > 1 ? 1 : 0;

	showStyle->edgeCount = edgeCount;
	showStyle->screenItems.reserve(numScreenItems);

	CelInfo32 celInfo;
	celInfo.bitmap = NULL_REG;
	celInfo.type = kCelObjTypeView;
	celInfo.color = showStyle->color;

	for (int i = 0; i < numScreenItems; ++i) {
		Common::Rect rect;

		if (horizontal) {
			rect.top = 0;
			rect.bottom = showStyle->height - 1;
			rect.left = (showStyle->width * i) / (showStyle->divisions * edgeCount);
			rect.right = ((i + 1) * (showStyle->width + extra)) / (showStyle->divisions * edgeCount) - 1;
		} else {
			rect.left = 0;
			rect.right = showStyle->width - 1;
			rect.top = (showStyle->height * i) / (showStyle->divisions * edgeCount);
			rect.bottom = ((i + 1) * (showStyle->height + extra)) / (showStyle->divisions * edgeCount) - 1;
		}

		addFrameoutEntryInternal(showStyle, priority, celInfo, rect);

		if (edgeCount == 2) {
			if (horizontal) {
				int temp = rect.left;
				rect.left = showStyle->width - rect.right - 1;
				rect.right = showStyle->width - temp - 1;
			} else {
				int temp = rect.top;
				rect.top = showStyle->height - rect.bottom - 1;
				rect.bottom = showStyle->height - temp - 1;
			}

			addFrameoutEntryInternal(showStyle, priority, celInfo, rect);
		}
	}
}

void GfxFrameout::prepareShowStyleIris(ShowStyleEntry *const showStyle, const int16 priority) {
	const int edgeCount = 4;
	const int numScreenItems = showStyle->divisions * edgeCount;

	showStyle->edgeCount = edgeCount;
	showStyle->screenItems.reserve(numScreenItems);

	CelInfo32 celInfo;
	celInfo.bitmap = NULL_REG;
	celInfo.type = kCelObjTypeView;
	celInfo.color = showStyle->color;

	for (int i = 0; i < numScreenItems; ++i) {
		Common::Rect rect;

		rect.right = showStyle->width - ((showStyle->width * i) / (showStyle->divisions * 2)) - 1;
		rect.left = (showStyle->width * i) / (showStyle->divisions * 2);
		rect.top = (showStyle->height * i) / (showStyle->divisions * 2);
		rect.bottom = ((i + 1) * (showStyle->height + 1)) / (showStyle->divisions * 2) - 1;

		addFrameoutEntryInternal(showStyle, priority, celInfo, rect);

		{
			int temp = rect.top;
			rect.top = showStyle->height - rect.bottom - 1;
			rect.bottom = showStyle->height - temp - 1;
		}

		addFrameoutEntryInternal(showStyle, priority, celInfo, rect);

		rect.top = ((i + 1) * (showStyle->height + 1)) / (showStyle->divisions * 2);
		rect.right = ((i + 1) * (showStyle->width + 1)) / (showStyle->divisions * 2) - 1;
		rect.bottom = ((i + 1) * (showStyle->height + 1)) / (showStyle->divisions * 2) - 1;

		addFrameoutEntryInternal(showStyle, priority, celInfo, rect);

		{
			int temp = rect.left;
			rect.left = showStyle->width - rect.right - 1;
			rect.right = showStyle->width - temp - 1;
		}

		addFrameoutEntryInternal(showStyle, priority, celInfo, rect);
	}
}

void GfxFrameout::prepareShowStylePixels(ShowStyleEntry *const showStyle, const int16 priority, const Common::Rect planeGameRect) {
	const int bitmapSize = showStyle->width * showStyle->height;

	// TODO: Verify that memory type 0x200 (what GK1 engine uses)
	// is Hunk type
	reg_t bitmapMemId = _segMan->allocateHunkEntry("ShowStylePixels()", bitmapSize + sizeof(GfxBitmapHeader));
	showStyle->bitmapMemId = bitmapMemId;

	// TODO: SCI2 GK1 uses a Bitmap constructor function to
	// do this work
	byte *bitmap = _segMan->getHunkPointer(bitmapMemId);
	GfxBitmapHeader *header = (GfxBitmapHeader *)bitmap;
	byte *bitmapData = bitmap + sizeof(GfxBitmapHeader);

	// TODO: These are defaults from the Bitmap constructor in
	// GK1, not specific values set by this function.
	// TODO: This probably should not even be using a struct at
	// all since this information is machine endian dependent
	// and will be reversed for Mac versions or when running
	// ScummVM on big-endian systems. GK1 used packed structs
	// everywhere so this probably worked better there too.
	header->field_18 = 36;
	header->field_1c = 36;
	memset(header, 0, sizeof(GfxBitmapHeader));

	header->width = showStyle->width;
	header->height = showStyle->height;
	header->field_8 = 250;
	header->size = bitmapSize;

	// TODO: Scaled dimensions in bitmap headers was not added
	// until SCI2.1mid. It is not clear what the right thing to
	// do here is.
	if (getSciVersion() >= SCI_VERSION_2_1_MIDDLE) {
		header->scaledWidth = _currentBuffer.scriptWidth;
		header->scaledHeight = _currentBuffer.scriptHeight;
	}

	Common::Rect copyRect;
	// TODO: planeGameRect is supposedly in script coordinates,
	// which are usually 320x200. If bitsSaveDisplayScreen is
	// in native resolution then seemingly this function will
	// not work properly since we will be not copy enough bits,
	// or from the correct location.
	copyRect.left = planeGameRect.left;
	copyRect.top = planeGameRect.top;
	copyRect.right = planeGameRect.left + showStyle->width;
	copyRect.bottom = planeGameRect.top + showStyle->height;
	_screen->bitsSaveDisplayScreen(copyRect, bitmapData);

	CelInfo32 celInfo;
	celInfo.bitmap = bitmapMemId;
	celInfo.type = kCelObjTypeMem;

	ScreenItem *screenItem = new ScreenItem;

	screenItem->position.x = 0;
	screenItem->position.y = 0;

	showStyle->bitmapScreenItem = screenItem;
	screenItem->priority = priority;

	// TODO: Have not seen/identified this particular flag yet in
	// SCI2.1mid (SQ6) engine; maybe (1) a duplicate of `created`,
	// or (2) does not exist any more, or (3) one of the other
	// still-unidentified fields. Probably need to look at the
	// GK1 source for its use in drawing algorithms to determine
	// if/how this correlates to ScreenItem members in the
	// SCI2.1mid engine.
//	screenItem->isInList = true;

	Plane *plane = _planes.findByObject(showStyle.plane);
	plane->_screenItemList.add(screenItem);
}
#endif

// NOTE: Different version of SCI engine support different show styles
// SCI2 implements 0, 1/3/5/7/9, 2/4/6/8/10, 11, 12, 13, 14
// SCI2.1 implements 0, 1/2/3/4/5/6/7/8/9/10/11/12/15, 13, 14
// SCI3 implements 0, 1/3/5/7/9, 2/4/6/8/10, 11, 12/15, 13, 14
// TODO: Sierra code needs to be replaced with code that uses the
// computed entry->delay property instead of just counting divisors,
// as the latter is machine-speed-dependent and leads to wrong
// transition speeds
void GfxFrameout::processShowStyles() {
	uint32 now = g_sci->getTickCount();

	bool continueProcessing;

	// TODO: Change to bool? Engine uses inc to set the value to true,
	// but there does not seem to be any reason to actually count how
	// many times it was set
	int doFrameOut;
	do {
		continueProcessing = false;
		doFrameOut = 0;
		ShowStyleEntry *showStyle = _showStyles;
		while (showStyle != nullptr) {
			bool retval = false;

			if (!showStyle->animate) {
				++doFrameOut;
			}

			if (showStyle->nextTick < now || !showStyle->animate) {
				// TODO: Different versions of SCI use different processors!
				// This is the SQ6/KQ7/SCI2.1mid table.
				switch (showStyle->type) {
					case kShowStyleNone: {
						retval = processShowStyleNone(showStyle);
						break;
					}
					case kShowStyleHShutterOut:
					case kShowStyleVShutterOut:
					case kShowStyleWipeLeft:
					case kShowStyleWipeUp:
					case kShowStyleIrisOut:
#if 0
						if (getSciVersion() >= SCI_VERSION_2_1_MIDDLE) {
#endif
							retval = processShowStyleMorph(showStyle);
#if 0
						} else {
							retval = processShowStyleWipe(-1, showStyle);
						}
#endif
						break;
					case kShowStyleHShutterIn:
					case kShowStyleVShutterIn:
					case kShowStyleWipeRight:
					case kShowStyleWipeDown:
					case kShowStyleIrisIn:
#if 0
						if (getSciVersion() >= SCI_VERSION_2_1_MIDDLE) {
#endif
							retval = processShowStyleMorph(showStyle);
#if 0
						} else {
							retval = processShowStyleWipe(1, showStyle);
						}
#endif
						break;
					case kShowStyle11:
#if 0
						if (getSciVersion() >= SCI_VERSION_2_1_MIDDLE) {
#endif
							retval = processShowStyleMorph(showStyle);
#if 0
						} else {
							retval = processShowStyle11(showStyle);
						}
#endif
						break;
					case kShowStyle12:
					case kShowStyleUnknown: {
#if 0
						if (getSciVersion() >= SCI_VERSION_2_1_MIDDLE) {
#endif
							retval = processShowStyleMorph(showStyle);
#if 0
						} else {
							retval = processShowStyle12(showStyle);
						}
#endif
						break;
					}
					case kShowStyleFadeOut: {
						retval = processShowStyleFade(-1, showStyle);
						break;
					}
					case kShowStyleFadeIn: {
						retval = processShowStyleFade(1, showStyle);
						break;
					}
				}
			}

			if (!retval) {
				continueProcessing = true;
			}

			if (retval && showStyle->processed) {
				showStyle = deleteShowStyleInternal(showStyle);
			} else {
				showStyle = showStyle->next;
			}
		}

		if (doFrameOut) {
			frameOut(true);

			// TODO: Transitions without the “animate” flag are too
			// fast, but the throttle value is arbitrary. Someone on
			// real hardware probably needs to test what the actual
			// speed of these transitions should be
			EngineState *state = g_sci->getEngineState();
			state->speedThrottler(33);
			state->_throttleTrigger = true;
		}
	} while(continueProcessing && doFrameOut);
}

bool GfxFrameout::processShowStyleNone(ShowStyleEntry *const showStyle) {
	if (showStyle->fadeUp) {
		_palette->setFade(100, 0, 255);
	} else {
		_palette->setFade(0, 0, 255);
	}

	showStyle->processed = true;
	return true;
}

bool GfxFrameout::processShowStyleMorph(ShowStyleEntry *const showStyle) {
	palMorphFrameOut(_styleRanges, showStyle);
	showStyle->processed = true;
	return true;
}

// TODO: Normalise use of 'entry' vs 'showStyle'
bool GfxFrameout::processShowStyleFade(const int direction, ShowStyleEntry *const showStyle) {
	bool unchanged = true;
	if (showStyle->currentStep < showStyle->divisions) {
		int percent;
		if (direction <= 0) {
			percent = showStyle->divisions - showStyle->currentStep - 1;
		} else {
			percent = showStyle->currentStep;
		}

		percent *= 100;
		percent /= showStyle->divisions - 1;

		if (showStyle->fadeColorRangesCount > 0) {
			for (int i = 0, len = showStyle->fadeColorRangesCount; i < len; i += 2) {
				_palette->setFade(percent, showStyle->fadeColorRanges[i], showStyle->fadeColorRanges[i + 1]);
			}
		} else {
			_palette->setFade(percent, 0, 255);
		}

		++showStyle->currentStep;
		showStyle->nextTick += showStyle->delay;
		unchanged = false;
	}

	if (showStyle->currentStep >= showStyle->divisions && unchanged) {
		if (direction > 0) {
			showStyle->processed = true;
		}

		return true;
	}

	return false;
}

// TODO: Rect sizes are wrong, rects in SCI are inclusive of bottom/right but
// in ScummVM are exclusive so extra ±1 operations here are wrong
#if 0
bool GfxFrameout::processShowStyleWipe(const ShowStyleEntry *const style) {
	const int16 divisions = style->divisions;
	Common::Rect rect(divisions, divisions);

	const Plane *const plane = _visibleScreen->planeList->findByObject(style->plane);

	const int16 planeLeft = plane->field_4C.left;
	const int16 planeTop = plane->field_4C.top;
	const int16 planeRight = plane->field_4C.right;
	const int16 planeBottom = plane->field_4C.bottom;
	const int16 planeWidth = planeRight - planeLeft + 1;
	const int16 planeHeight = planeBottom - planeTop + 1;

	const int16 divisionWidth = planeWidth / divisions - 1;
	int16 shutterDivisionWidth = planeWidth / (2 * divisions);
	if (shutterDivisionWidth >= 0) {
		const int16 heightPerDivision = planeHeight / divisions;
		int16 shutterMiddleX = divisions * shutterDivisionWidth;
		for (int16 x = divisionWidth - shutterDivisionWidth; x <= divisionWidth; ++x) {
			int16 divisionTop = 0;
			for (int16 y = 0; y < heightPerDivision; ++y, divisionTop += divisions) {
				rect.top = planeTop + divisionTop;
				rect.bottom = rect.top + divisions - 1;
				rect.left = planeLeft + shutterMiddleX;
				rect.right = rect.left + divisions - 1;
				//				_screen->rectList.clear();
				//				_screen->rectList.add(rect);
				//				showBits();
			}
			// number of divisions does not divide evenly into plane height,
			// draw the remainder
			if (planeHeight % divisions) {
				rect.top = planeTop + divisionTop;
				rect.bottom = rect.top + planeHeight % divisions - 1;
				rect.left = planeLeft + shutterMiddleX;
				rect.right = rect.left + divisions - 1;
				//				_screen->rectList.clear();
				//				_screen->rectList.add(rect);
				//				showBits();
			}

			divisionTop = 0;
			for (int16 y = 0; y < heightPerDivision; ++y, divisionTop += divisions) {
				rect.top = planeTop + divisionTop;
				rect.bottom = rect.top + divisions - 1;
				rect.left = planeLeft + divisions * x;
				rect.right = rect.left + divisions - 1;
				//				_screen->rectList.clear();
				//				_screen->rectList.add(rect);
				//				showBits();
			}
			if (planeHeight % divisions) {
				rect.top = planeTop + divisionTop;
				rect.bottom = rect.top + planeHeight % divisions - 1;
				rect.left = planeLeft + divisions * x;
				rect.right = rect.left + divisions - 1;
				//				_screen->rectList.clear();
				//				_screen->rectList.add(rect);
				//				showBits();
			}

			shutterMiddleX -= divisions;
			--shutterDivisionWidth;
		}
	}

	if (planeWidth % divisions) {
		const int16 roundedPlaneWidth = divisions * divisionWidth;
		int16 divisionTop = 0;
		for (int16 y = 0; y < planeHeight / divisions; ++y, divisionTop += divisions) {
			rect.top = planeTop + divisionTop;
			rect.bottom = rect.top + divisions - 1;
			rect.left = planeLeft + roundedPlaneWidth;
			rect.right = rect.left + planeWidth % divisions + divisions - 1;
			//			_screen->rectList.clear();
			//			_screen->rectList.add(rect);
			//			showBits();
		}
		if (planeHeight % divisions) {
			rect.top = planeTop + divisionTop;
			rect.bottom = rect.top + planeHeight % divisions - 1;
			rect.left = planeLeft + roundedPlaneWidth;
			rect.right = rect.left + planeWidth % divisions + divisions - 1;
			//			_screen->rectList.clear();
			//			_screen->rectList.add(rect);
			//			showBits();
		}
	}

	rect.right = planeRight;
	rect.left = planeLeft;
	rect.top = planeTop;
	rect.bottom = planeBottom;
	//	_screen->rectList.clear();
	//	_screen->rectList.add(rect);
	//	showBits();
}
#endif
#if 0
bool GfxFrameout::processShowStyleWipe(const int direction, ShowStyleEntry *const showStyle) {
	if (showStyle->currentStep < showStyle->divisions) {
		int index;
		if (direction <= 0) {
			index = showStyle->divisions - showStyle->currentStep - 1;
		} else {
			index = showStyle->currentStep;
		}

		index *= showStyle->edgeCount;

		if (showStyle->edgeCount > 0) {
			for (int i = 0; i < showStyle->edgeCount; ++i) {
				if (showStyle->fadeUp) {
					ScreenItem *screenItem = showStyle->screenItems[index + i];
					if (screenItem != nullptr) {
						// TODO: _screen->deleteScreenItem(screenItem);
						_screenItems.remove(screenItem);

						delete screenItem;
						showStyle->screenItems[index + i] = nullptr;
					}
				} else {
					ScreenItem *screenItem = showStyle->screenItems[index + i];
					// TODO: _screen->addScreenItem(screenItem);
					_screenItems.push_back(screenItem);
				}
			}

			++showStyle->currentStep;
			showStyle->nextTick += showStyle->delay;
		}
	}

	if (showStyle->currentStep >= showStyle->divisions) {
		if (showStyle->fadeUp) {
			showStyle->processed = true;
		}

		return true;
	}

	return false;
}

void fillRect(byte *data, const Common::Rect &rect, const int16 color, const int16 stride) {

}

bool GfxFrameout::processShowStyle11(ShowStyleEntry *const showStyle) {
	int divisions = showStyle->divisions * showStyle->divisions;

	byte *bitmapData = _segMan->getHunkPointer(showStyle->bitmapMemId) + sizeof(GfxBitmapHeader);

	int ebx;

	if (showStyle->currentStep == 0) {
		int ctr = 0;
		int bloot = divisions;
		do {
			bloot >>= 1;
			++ctr;
		} while (bloot != 1);

		showStyle->dissolveSeed = _dissolveSequenceSeeds[ctr];
		ebx = 800;
		showStyle->unknown3A = 800;
		showStyle->dissolveInitial = 800;
	} else {
		int ebx = showStyle->unknown3A;
		do {
			int eax = ebx >> 1;
			if (ebx & 1) {
				ebx = showStyle->dissolveSeed ^ eax;
			} else {
				ebx = eax;
			}
		} while (ebx >= divisions);

		if (ebx == showStyle->dissolveInitial) {
			ebx = 0;
		}
	}

	Common::Rect rect;

	rect.left = (showStyle->width + showStyle->divisions - 1) / showStyle->divisions;
	rect.top = (showStyle->height + showStyle->divisions - 1) / showStyle->divisions;

	if (showStyle->currentStep <= showStyle->divisions) {
		int ebp = 0;
		do {
			int ecx = ebx % showStyle->divisions;

			Common::Rect drawRect;
			drawRect.left = rect.left * ecx;
			drawRect.right = drawRect.left + rect.left - 1;
			drawRect.top = rect.top * ebx;
			drawRect.bottom = rect.top * ebx + rect.top - 1;

			bool doit;
			if (drawRect.right >= 0 && drawRect.bottom >= 0 && drawRect.left <= rect.right && drawRect.top <= rect.bottom) {
				doit = true;
			} else {
				doit = false;
			}

			if (doit) {
				if (drawRect.left < 0) {
					drawRect.left = 0;
				}

				if (drawRect.top < 0) {
					drawRect.top = 0;
				}

				if (drawRect.right > rect.right) {
					drawRect.right = rect.right;
				}

				if (drawRect.bottom > rect.bottom) {
					drawRect.bottom = rect.bottom;
				}
			} else {
				drawRect.right = 0;
				drawRect.bottom = 0;
				drawRect.left = 0;
				drawRect.top = 0;
			}

			fillRect(bitmapData, drawRect, showStyle->color, showStyle->width);

			int eax = ebx;
			do {
				eax >>= 1;
				if (ebx & 1) {
					ebx = showStyle->dissolveSeed;
					ebx ^= eax;
				} else {
					ebx = eax;
				}
			} while (ebx >= divisions);

			if (showStyle->currentStep != showStyle->divisions) {
				ebp++;
			} else {
				drawRect.left = 0;
				drawRect.top = 0;
				drawRect.right = showStyle->width - 1;
				drawRect.bottom = showStyle->height - 1;
				fillRect(bitmapData, drawRect, showStyle->color, showStyle->width);
			}

		} while (ebp <= showStyle->divisions);

		showStyle->unknown3A = ebx;
		++showStyle->currentStep;
		showStyle->nextTick += showStyle->delay;
		// _screen->updateScreenItem(showStyle->bitmapScreenItem);
	}

	if (showStyle->currentStep >= showStyle->divisions) {
		if (showStyle->fadeUp) {
			showStyle->processed = true;
		}

		return true;
	}

	return false;
}

bool GfxFrameout::processShowStyle12(ShowStyleEntry *const showStyle) {
	return true;
}
#endif

void GfxFrameout::kernelFrameOut(const bool shouldShowBits) {
	if (_showStyles != nullptr) {
		processShowStyles();
	} else if (_palMorphIsOn) {
		palMorphFrameOut(_styleRanges, nullptr);
		_palMorphIsOn = false;
	} else {
// TODO: Window scroll
//		if (g_ScrollWindow) {
//			doScroll();
//		}

		frameOut(shouldShowBits);
	}
}

#pragma mark -
#pragma mark Mouse cursor

reg_t GfxFrameout::kernelIsOnMe(const reg_t object, const Common::Point &position, bool checkPixel) const {
	reg_t planeObject = readSelector(_segMan, object, SELECTOR(plane));
	Plane *plane = _visiblePlanes.findByObject(planeObject);
	if (plane == nullptr) {
		return make_reg(0, 0);
	}

	ScreenItem *screenItem = plane->_screenItemList.findByObject(object);
	if (screenItem == nullptr) {
		return make_reg(0, 0);
	}

	return make_reg(0, isOnMe(*screenItem, *plane, position, checkPixel));
}

bool GfxFrameout::isOnMe(const ScreenItem &screenItem, const Plane &plane, const Common::Point &position, const bool checkPixel) const {

	Common::Point scaledPosition(position);
	mulru(scaledPosition, Ratio(_currentBuffer.screenWidth, _currentBuffer.scriptWidth), Ratio(_currentBuffer.screenHeight, _currentBuffer.scriptHeight));
	scaledPosition.x += plane._planeRect.left;
	scaledPosition.y += plane._planeRect.top;

	if (!screenItem._screenRect.contains(scaledPosition)) {
		return false;
	}

	if (checkPixel) {
		CelObj &celObj = screenItem.getCelObj();

		bool mirrorX = screenItem._mirrorX ^ celObj._mirrorX;

		scaledPosition.x -= screenItem._scaledPosition.x;
		scaledPosition.y -= screenItem._scaledPosition.y;

		mulru(scaledPosition, Ratio(celObj._scaledWidth, _currentBuffer.screenWidth), Ratio(celObj._scaledHeight, _currentBuffer.screenHeight));

		if (screenItem._scale.signal != kScaleSignalNone && screenItem._scale.x && screenItem._scale.y) {
			scaledPosition.x = scaledPosition.x * 128 / screenItem._scale.x;
			scaledPosition.y = scaledPosition.y * 128 / screenItem._scale.y;
		}

		uint8 pixel = celObj.readPixel(scaledPosition.x, scaledPosition.y, mirrorX);
		return pixel != celObj._transparentColor;
	}

	return true;
}

#pragma mark -
#pragma mark Debugging

void GfxFrameout::printPlaneListInternal(Console *con, const PlaneList &planeList) const {
	for (PlaneList::const_iterator it = planeList.begin(); it != planeList.end(); ++it) {
		Plane *p = *it;
		p->printDebugInfo(con);
	}
}

void GfxFrameout::printPlaneList(Console *con) const {
	printPlaneListInternal(con, _planes);
}

void GfxFrameout::printVisiblePlaneList(Console *con) const {
	printPlaneListInternal(con, _visiblePlanes);
}

void GfxFrameout::printPlaneItemListInternal(Console *con, const ScreenItemList &screenItemList) const {
	ScreenItemList::size_type i = 0;
	for (ScreenItemList::const_iterator sit = screenItemList.begin(); sit != screenItemList.end(); sit++) {
		ScreenItem *screenItem = *sit;
		con->debugPrintf("%2d: ", i++);
		screenItem->printDebugInfo(con);
	}
}

void GfxFrameout::printPlaneItemList(Console *con, const reg_t planeObject) const {
	Plane *p = _planes.findByObject(planeObject);

	if (p == nullptr) {
		con->debugPrintf("Plane does not exist");
		return;
	}

	printPlaneItemListInternal(con, p->_screenItemList);
}

void GfxFrameout::printVisiblePlaneItemList(Console *con, const reg_t planeObject) const {
	Plane *p = _visiblePlanes.findByObject(planeObject);

	if (p == nullptr) {
		con->debugPrintf("Plane does not exist");
		return;
	}

	printPlaneItemListInternal(con, p->_screenItemList);
}

} // End of namespace Sci
