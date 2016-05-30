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
#include "sci/graphics/remap.h"
#include "sci/graphics/text32.h"
#include "sci/graphics/plane32.h"
#include "sci/graphics/screen_item32.h"
#include "sci/graphics/frameout.h"
#include "sci/video/robot_decoder.h"

namespace Sci {

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

	switch (g_sci->getGameId()) {
	case GID_GK2:
	case GID_LIGHTHOUSE:
	case GID_LSL7:
	case GID_PHANTASMAGORIA2:
	case GID_PQSWAT:
	case GID_TORIN:
	case GID_RAMA:
		_currentBuffer.scriptWidth = 640;
		_currentBuffer.scriptHeight = 480;
		break;
	default:
		// default script width for other games is 320x200
		break;
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

	_segMan->getObject(object)->setInfoSelectorFlag(kInfoFlagViewInserted);

	Plane *plane = _planes.findByObject(planeObject);
	if (plane == nullptr) {
		error("kAddScreenItem: Plane %04x:%04x not found for screen item %04x:%04x", PRINT_REG(planeObject), PRINT_REG(object));
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
			error("kUpdateScreenItem: Plane %04x:%04x not found for screen item %04x:%04x", PRINT_REG(planeObject), PRINT_REG(object));
		}

		ScreenItem *screenItem = plane->_screenItemList.findByObject(object);
		if (screenItem == nullptr) {
			error("kUpdateScreenItem: Screen item %04x:%04x not found in plane %04x:%04x", PRINT_REG(object), PRINT_REG(planeObject));
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
		return;
	}

	ScreenItem *screenItem = plane->_screenItemList.findByObject(object);
	if (screenItem == nullptr) {
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
		error("kUpdatePlane: Plane %04x:%04x not found", PRINT_REG(object));
	}

	plane->update(object);
	updatePlane(*plane);
}

void GfxFrameout::kernelDeletePlane(const reg_t object) {
	Plane *plane = _planes.findByObject(object);
	if (plane == nullptr) {
		error("kDeletePlane: Plane %04x:%04x not found", PRINT_REG(object));
	}

	if (plane->_created) {
		// NOTE: The original engine calls some `AbortPlane` function that
		// just ends up doing this anyway so we skip the extra indirection
		_planes.erase(plane);
	} else {
		plane->_created = 0;
		plane->_deleted = g_sci->_gfxFrameout->getScreenCount();
	}
}

void GfxFrameout::deletePlane(Plane &planeToFind) {
	Plane *plane = _planes.findByObject(planeToFind._object);
	if (plane == nullptr) {
		error("deletePlane: Plane %04x:%04x not found", PRINT_REG(planeToFind._object));
	}

	if (plane->_created) {
		_planes.erase(plane);
	} else {
		plane->_created = 0;
		plane->_moved = 0;
		plane->_deleted = getScreenCount();
	}
}

void GfxFrameout::kernelMovePlaneItems(const reg_t object, const int16 deltaX, const int16 deltaY, const bool scrollPics) {
	Plane *plane = _planes.findByObject(object);
	if (plane == nullptr) {
		error("kMovePlaneItems: Plane %04x:%04x not found", PRINT_REG(object));
	}

	plane->scrollScreenItems(deltaX, deltaY, scrollPics);

	for (ScreenItemList::iterator it = plane->_screenItemList.begin(); it != plane->_screenItemList.end(); ++it) {
		ScreenItem &screenItem = **it;

		// If object is a number, the screen item from the
		// engine, not a script, and should be ignored
		if (screenItem._object.isNumber()) {
			continue;
		}

		if (deltaX != 0) {
			writeSelectorValue(_segMan, screenItem._object, SELECTOR(x), readSelectorValue(_segMan, screenItem._object, SELECTOR(x)) + deltaX);
		}

		if (deltaY != 0) {
			writeSelectorValue(_segMan, screenItem._object, SELECTOR(y), readSelectorValue(_segMan, screenItem._object, SELECTOR(y)) + deltaY);
		}
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
		error("kAddPicAt: Plane %04x:%04x not found", PRINT_REG(planeObject));
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

	if (g_sci->_gfxRemap32->getRemapCount() > 0 && _remapOccurred) {
		remapMarkRedraw();
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
			if (visibleOuterPlane == nullptr) {
				warning("calcLists could not find visible plane for %04x:%04x", PRINT_REG(outerPlane->_object));
				continue;
			}

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

	int16 prevRoom = g_sci->getEngineState()->variables[VAR_GLOBAL][12].toSint16();

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

	if (g_sci->_gfxRemap32->getRemapCount() > 0 && _remapOccurred) {
		remapMarkRedraw();
	}

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

	if (prevRoom < 1000) {
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
		warning("Transition %d not implemented!", showStyle->type);
	} else {
		showBits();
	}

	_frameNowVisible = true;

	for (PlaneList::iterator plane = _planes.begin(); plane != _planes.end(); ++plane) {
		(*plane)->_redrawAllCount = getScreenCount();
	}

	if (g_sci->_gfxRemap32->getRemapCount() > 0 && _remapOccurred) {
		remapMarkRedraw();
	}

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
			int minDiffIndex = paletteIndex;

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
			int minDiffIndex = paletteIndex;

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
			// NOTE: In original engine this assignment happens outside of the
			// condition, but if the branch is not followed the value is just
			// going to be the same as it was before
			styleRangeValue = styleRanges[currentValue];
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

	delete[] showStyle->fadeColorRanges;
	delete showStyle;

	// TODO: Verify that this is the correct entry to return
	// for the loop in processShowStyles to work correctly
	return lastEntry;
}

// TODO: 10-argument version is only in SCI3; argc checks are currently wrong for this version
// and need to be fixed in future
// TODO: SQ6 does not use 'priority' (exists since SCI2) or 'blackScreen' (exists since SCI3);
// check to see if other versions use or if they are just always ignored
void GfxFrameout::kernelSetShowStyle(const uint16 argc, const reg_t planeObj, const ShowStyleType type, const int16 seconds, const int16 back, const int16 priority, const int16 animate, const int16 frameOutNow, reg_t pFadeArray, int16 divisions, const int16 blackScreen) {

	bool hasDivisions = false;
	bool hasFadeArray = false;

	// KQ7 2.0b uses a mismatched version of the Styler script (SCI2.1early script
	// for SCI2.1mid engine), so the calls it makes to kSetShowStyle are wrong and
	// put `divisions` where `pFadeArray` is supposed to be
	if (getSciVersion() == SCI_VERSION_2_1_MIDDLE && g_sci->getGameId() == GID_KQ7) {
		hasDivisions = argc > 7;
		hasFadeArray = false;
		divisions = argc > 7 ? pFadeArray.toSint16() : -1;
		pFadeArray = NULL_REG;
	} else if (getSciVersion() < SCI_VERSION_2_1_MIDDLE) {
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

	bool createNewEntry = true;
	ShowStyleEntry *entry = findShowStyleForPlane(planeObj);
	if (entry != nullptr) {
		// TODO: SCI2.1early has different criteria for show style reuse
		bool useExisting = true;

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
			// TODO: Implement SCI2.1early and SCI3
			entry->next = _showStyles;
			_showStyles = entry;
		}
	}
}

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
					case kShowStyleHShutterIn:
					case kShowStyleVShutterIn:
					case kShowStyleWipeRight:
					case kShowStyleWipeDown:
					case kShowStyleIrisIn:
					case kShowStyle11:
					case kShowStyle12:
					case kShowStyleUnknown: {
						retval = processShowStyleMorph(showStyle);
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

void GfxFrameout::kernelFrameOut(const bool shouldShowBits) {
	if (_showStyles != nullptr) {
		processShowStyles();
	} else if (_palMorphIsOn) {
		palMorphFrameOut(_styleRanges, nullptr);
		_palMorphIsOn = false;
	} else {
// TODO: Window scroll
//		if (g_PlaneScroll) {
//			processScrolls();
//		}

		frameOut(shouldShowBits);
	}
}

#pragma mark -
#pragma mark Mouse cursor

reg_t GfxFrameout::kernelIsOnMe(const reg_t object, const Common::Point &position, bool checkPixel) const {
	const reg_t planeObject = readSelector(_segMan, object, SELECTOR(plane));
	Plane *plane = _visiblePlanes.findByObject(planeObject);
	if (plane == nullptr) {
		return make_reg(0, 0);
	}

	ScreenItem *screenItem = plane->_screenItemList.findByObject(object);
	if (screenItem == nullptr) {
		return make_reg(0, 0);
	}

	// NOTE: The original engine passed a copy of the ScreenItem into isOnMe
	// as a hack around the fact that the screen items in `_visiblePlanes`
	// did not have their `_celObj` pointers cleared when their CelInfo was
	// updated by `Plane::decrementScreenItemArrayCounts`. We handle this
	// this more intelligently by clearing `_celObj` in the copy assignment
	// operator, which is only ever called by `decrementScreenItemArrayCounts`
	// anyway.
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

void GfxFrameout::kernelSetNowSeen(const reg_t screenItemObject) const {
	const reg_t planeObject = readSelector(_segMan, screenItemObject, SELECTOR(plane));

	Plane *plane = _planes.findByObject(planeObject);
	if (plane == nullptr) {
		error("kSetNowSeen: Plane %04x:%04x not found for screen item %04x:%04x", PRINT_REG(planeObject), PRINT_REG(screenItemObject));
	}

	ScreenItem *screenItem = plane->_screenItemList.findByObject(screenItemObject);
	if (screenItem == nullptr) {
		error("kSetNowSeen: Screen item %04x:%04x not found in plane %04x:%04x", PRINT_REG(screenItemObject), PRINT_REG(planeObject));
	}

	Common::Rect result = screenItem->getNowSeenRect(*plane);
	writeSelectorValue(_segMan, screenItemObject, SELECTOR(nsLeft), result.left);
	writeSelectorValue(_segMan, screenItemObject, SELECTOR(nsTop), result.top);
	writeSelectorValue(_segMan, screenItemObject, SELECTOR(nsRight), result.right - 1);
	writeSelectorValue(_segMan, screenItemObject, SELECTOR(nsBottom), result.bottom - 1);
}

void GfxFrameout::remapMarkRedraw() {
	for (PlaneList::const_iterator it = _planes.begin(); it != _planes.end(); ++it) {
		Plane *p = *it;
		p->remapMarkRedraw();
	}
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
