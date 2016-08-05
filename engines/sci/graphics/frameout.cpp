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
#include "sci/graphics/screen.h"
#include "sci/graphics/paint32.h"
#include "sci/graphics/palette32.h"
#include "sci/graphics/plane32.h"
#include "sci/graphics/remap32.h"
#include "sci/graphics/screen_item32.h"
#include "sci/graphics/text32.h"
#include "sci/graphics/frameout.h"
#include "sci/video/robot_decoder.h"
#include "sci/graphics/transitions32.h"

namespace Sci {

GfxFrameout::GfxFrameout(SegManager *segMan, ResourceManager *resMan, GfxCoordAdjuster *coordAdjuster, GfxScreen *screen, GfxPalette32 *palette, GfxTransitions32 *transitions) :
	_isHiRes(ConfMan.getBool("enable_high_resolution_graphics")),
	_palette(palette),
	_resMan(resMan),
	_screen(screen),
	_segMan(segMan),
	_transitions(transitions),
	_benchmarkingFinished(false),
	_throttleFrameOut(true),
	_throttleState(0),
	// TODO: Stop using _gfxScreen
	_currentBuffer(screen->getDisplayWidth(), screen->getDisplayHeight(), nullptr),
	_remapOccurred(false),
	_frameNowVisible(false),
	_screenRect(screen->getDisplayWidth(), screen->getDisplayHeight()),
	_overdrawThreshold(0),
	_palMorphIsOn(false) {

	_currentBuffer.setPixels(calloc(1, screen->getDisplayWidth() * screen->getDisplayHeight()));

	// QFG4 is the only SCI32 game that doesn't have a high-resolution toggle
	if (g_sci->getGameId() == GID_QFG4) {
		_isHiRes = false;
	}

	switch (g_sci->getGameId()) {
	case GID_HOYLE5:
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
#pragma mark Benchmarking

bool GfxFrameout::checkForFred(const reg_t object) {
	const int16 viewId = readSelectorValue(_segMan, object, SELECTOR(view));
	const SciGameId gameId = g_sci->getGameId();

	if (gameId == GID_QFG4 && viewId == 9999) {
		return true;
	}

	if (gameId != GID_QFG4 && viewId == -556) {
		return true;
	}

	if (Common::String(_segMan->getObjectName(object)) == "fred") {
		return true;
	}

	return false;
}

#pragma mark -
#pragma mark Screen items

void GfxFrameout::addScreenItem(ScreenItem &screenItem) const {
	Plane *plane = _planes.findByObject(screenItem._plane);
	if (plane == nullptr) {
		error("GfxFrameout::addScreenItem: Could not find plane %04x:%04x for screen item %04x:%04x", PRINT_REG(screenItem._plane), PRINT_REG(screenItem._object));
	}
	plane->_screenItemList.add(&screenItem);
}

void GfxFrameout::updateScreenItem(ScreenItem &screenItem) const {
	// TODO: In SCI3+ this will need to go through Plane
//	Plane *plane = _planes.findByObject(screenItem._plane);
//	if (plane == nullptr) {
//		error("GfxFrameout::updateScreenItem: Could not find plane %04x:%04x for screen item %04x:%04x", PRINT_REG(screenItem._plane), PRINT_REG(screenItem._object));
//	}

	screenItem.update();
}

void GfxFrameout::deleteScreenItem(ScreenItem &screenItem) {
	Plane *plane = _planes.findByObject(screenItem._plane);
	if (plane == nullptr) {
		error("GfxFrameout::deleteScreenItem: Could not find plane %04x:%04x for screen item %04x:%04x", PRINT_REG(screenItem._plane), PRINT_REG(screenItem._object));
	}
	if (plane->_screenItemList.findByObject(screenItem._object) == nullptr) {
		error("GfxFrameout::deleteScreenItem: Screen item %04x:%04x not found in plane %04x:%04x", PRINT_REG(screenItem._object), PRINT_REG(screenItem._plane));
	}
	deleteScreenItem(screenItem, *plane);
}

void GfxFrameout::deleteScreenItem(ScreenItem &screenItem, Plane &plane) {
	if (screenItem._created == 0) {
		screenItem._created = 0;
		screenItem._updated = 0;
		screenItem._deleted = getScreenCount();
	} else {
		plane._screenItemList.erase(&screenItem);
		plane._screenItemList.pack();
	}
}

void GfxFrameout::deleteScreenItem(ScreenItem &screenItem, const reg_t planeObject) {
	Plane *plane = _planes.findByObject(planeObject);
	if (plane == nullptr) {
		error("GfxFrameout::deleteScreenItem: Could not find plane %04x:%04x for screen item %04x:%04x", PRINT_REG(planeObject), PRINT_REG(screenItem._object));
	}
	deleteScreenItem(screenItem, *plane);
}

void GfxFrameout::kernelAddScreenItem(const reg_t object) {
	// The "fred" object is used to test graphics performance;
	// it is impacted by framerate throttling, so disable the
	// throttling when this item is on the screen for the
	// performance check to pass.
	if (!_benchmarkingFinished && _throttleFrameOut && checkForFred(object)) {
		_throttleFrameOut = false;
	}

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
	// The "fred" object is used to test graphics performance;
	// it is impacted by framerate throttling, so disable the
	// throttling when this item is on the screen for the
	// performance check to pass.
	if (!_benchmarkingFinished && checkForFred(object)) {
		_benchmarkingFinished = true;
		_throttleFrameOut = true;
	}

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

	deleteScreenItem(*screenItem, *plane);
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

void GfxFrameout::kernelAddPicAt(const reg_t planeObject, const GuiResourceId pictureId, const int16 x, const int16 y, const bool mirrorX, const bool deleteDuplicate) {
	Plane *plane = _planes.findByObject(planeObject);
	if (plane == nullptr) {
		error("kAddPicAt: Plane %04x:%04x not found", PRINT_REG(planeObject));
	}
	plane->addPic(pictureId, Common::Point(x, y), mirrorX, deleteDuplicate);
}

#pragma mark -
#pragma mark Rendering

void GfxFrameout::frameOut(const bool shouldShowBits, const Common::Rect &eraseRect) {
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

	calcLists(screenItemLists, eraseLists, eraseRect);

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

	_palette->updateHardware(!shouldShowBits);

	if (shouldShowBits) {
		showBits();
	}

	_frameNowVisible = true;

// TODO: Robot
//	if (_robot != nullptr) {
//		robot->frameNowVisible();
//	}
}

void GfxFrameout::palMorphFrameOut(const int8 *styleRanges, PlaneShowStyle *showStyle) {
	Palette sourcePalette(_palette->getNextPalette());
	alterVmap(sourcePalette, sourcePalette, -1, styleRanges);

	int16 prevRoom = g_sci->getEngineState()->variables[VAR_GLOBAL][12].toSint16();

	Common::Rect rect(_screen->getDisplayWidth(), _screen->getDisplayHeight());
	_showList.add(rect);
	showBits();

	// NOTE: The original engine allocated these as static arrays of 100
	// pointers to ScreenItemList / RectList
	ScreenItemListList screenItemLists;
	EraseListList eraseLists;

	screenItemLists.resize(_planes.size());
	eraseLists.resize(_planes.size());

	if (g_sci->_gfxRemap32->getRemapCount() > 0 && _remapOccurred) {
		remapMarkRedraw();
	}

	calcLists(screenItemLists, eraseLists);
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

	Palette nextPalette(_palette->getNextPalette());

	if (prevRoom < 1000) {
		for (int i = 0; i < ARRAYSIZE(sourcePalette.colors); ++i) {
			if (styleRanges[i] == -1 || styleRanges[i] == 0) {
				sourcePalette.colors[i] = nextPalette.colors[i];
				sourcePalette.colors[i].used = true;
			}
		}
	} else {
		for (int i = 0; i < ARRAYSIZE(sourcePalette.colors); ++i) {
			if (styleRanges[i] == -1 || validZeroStyle(styleRanges[i], i)) {
				sourcePalette.colors[i] = nextPalette.colors[i];
				sourcePalette.colors[i].used = true;
			}
		}
	}

	_palette->submit(sourcePalette);
	_palette->updateFFrame();
	_palette->updateHardware();
	alterVmap(nextPalette, sourcePalette, 1, _transitions->_styleRanges);

	if (showStyle && showStyle->type != kShowStyleMorph) {
		_transitions->processEffects(*showStyle);
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

	calcLists(screenItemLists, eraseLists);
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
	_palette->updateHardware(false);
	showBits();

	_frameNowVisible = true;
}

/**
 * Determines the parts of `r` that aren't overlapped by `other`.
 * Returns -1 if `r` and `other` have no intersection.
 * Returns number of returned parts (in `outRects`) otherwise.
 * (In particular, this returns 0 if `r` is contained in `other`.)
 */
int splitRects(Common::Rect r, const Common::Rect &other, Common::Rect(&outRects)[4]) {
	if (!r.intersects(other)) {
		return -1;
	}

	int splitCount = 0;
	if (r.top < other.top) {
		Common::Rect &t = outRects[splitCount++];
		t = r;
		t.bottom = other.top;
		r.top = other.top;
	}

	if (r.bottom > other.bottom) {
		Common::Rect &t = outRects[splitCount++];
		t = r;
		t.top = other.bottom;
		r.bottom = other.bottom;
	}

	if (r.left < other.left) {
		Common::Rect &t = outRects[splitCount++];
		t = r;
		t.right = other.left;
		r.left = other.left;
	}

	if (r.right > other.right) {
		Common::Rect &t = outRects[splitCount++];
		t = r;
		t.left = other.right;
	}

	return splitCount;
}

/**
 * Determines the parts of `middleRect` that aren't overlapped
 * by `showRect`, optimised for contiguous memory writes.
 * Returns -1 if `middleRect` and `showRect` have no intersection.
 * Returns number of returned parts (in `outRects`) otherwise.
 * (In particular, this returns 0 if `middleRect` is contained
 * in `other`.)
 *
 * `middleRect` is modified directly to extend into the upper
 * and lower rects.
 */
int splitRectsForRender(Common::Rect &middleRect, const Common::Rect &showRect, Common::Rect(&outRects)[2]) {
	if (!middleRect.intersects(showRect)) {
		return -1;
	}

	const int16 minLeft = MIN(middleRect.left, showRect.left);
	const int16 maxRight = MAX(middleRect.right, showRect.right);

	int16 upperLeft, upperTop, upperRight, upperMaxTop;
	if (middleRect.top < showRect.top) {
		upperLeft = middleRect.left;
		upperTop = middleRect.top;
		upperRight = middleRect.right;
		upperMaxTop = showRect.top;
	}
	else {
		upperLeft = showRect.left;
		upperTop = showRect.top;
		upperRight = showRect.right;
		upperMaxTop = middleRect.top;
	}

	int16 lowerLeft, lowerRight, lowerBottom, lowerMinBottom;
	if (middleRect.bottom > showRect.bottom) {
		lowerLeft = middleRect.left;
		lowerRight = middleRect.right;
		lowerBottom = middleRect.bottom;
		lowerMinBottom = showRect.bottom;
	} else {
		lowerLeft = showRect.left;
		lowerRight = showRect.right;
		lowerBottom = showRect.bottom;
		lowerMinBottom = middleRect.bottom;
	}

	int splitCount = 0;
	middleRect.left = minLeft;
	middleRect.top = upperMaxTop;
	middleRect.right = maxRight;
	middleRect.bottom = lowerMinBottom;

	if (upperTop != upperMaxTop) {
		Common::Rect &upperRect = outRects[0];
		upperRect.left = upperLeft;
		upperRect.top = upperTop;
		upperRect.right = upperRight;
		upperRect.bottom = upperMaxTop;

		// Merge upper rect into middle rect if possible
		if (upperRect.left == middleRect.left && upperRect.right == middleRect.right) {
			middleRect.top = upperRect.top;
		} else {
			++splitCount;
		}
	}

	if (lowerBottom != lowerMinBottom) {
		Common::Rect &lowerRect = outRects[splitCount];
		lowerRect.left = lowerLeft;
		lowerRect.top = lowerMinBottom;
		lowerRect.right = lowerRight;
		lowerRect.bottom = lowerBottom;

		// Merge lower rect into middle rect if possible
		if (lowerRect.left == middleRect.left && lowerRect.right == middleRect.right) {
			middleRect.bottom = lowerRect.bottom;
		} else {
			++splitCount;
		}
	}

	assert(splitCount <= 2);
	return splitCount;
}

// NOTE: The third rectangle parameter is only ever given a non-empty rect
// by VMD code, via `frameOut`
void GfxFrameout::calcLists(ScreenItemListList &drawLists, EraseListList &eraseLists, const Common::Rect &eraseRect) {
	RectList eraseList;
	Common::Rect outRects[4];
	int deletedPlaneCount = 0;
	bool addedToEraseList = false;
	bool foundTransparentPlane = false;

	if (!eraseRect.isEmpty()) {
		addedToEraseList = true;
		eraseList.add(eraseRect);
	}

	PlaneList::size_type planeCount = _planes.size();
	for (PlaneList::size_type outerPlaneIndex = 0; outerPlaneIndex < planeCount; ++outerPlaneIndex) {
		const Plane *outerPlane = _planes[outerPlaneIndex];
		const Plane *visiblePlane = _visiblePlanes.findByObject(outerPlane->_object);

		// NOTE: SSCI only ever checks for kPlaneTypeTransparent here, even
		// though kPlaneTypeTransparentPicture is also a transparent plane
		if (outerPlane->_type == kPlaneTypeTransparent) {
			foundTransparentPlane = true;
		}

		if (outerPlane->_deleted) {
			if (visiblePlane != nullptr && !visiblePlane->_screenRect.isEmpty()) {
				eraseList.add(visiblePlane->_screenRect);
				addedToEraseList = true;
			}
			++deletedPlaneCount;
		} else if (visiblePlane != nullptr && outerPlane->_moved) {
			// _moved will be decremented in the final loop through the planes,
			// at the end of this function

			{
				const int splitCount = splitRects(visiblePlane->_screenRect, outerPlane->_screenRect, outRects);
				if (splitCount) {
					if (splitCount == -1 && !visiblePlane->_screenRect.isEmpty()) {
						eraseList.add(visiblePlane->_screenRect);
					} else {
						for (int i = 0; i < splitCount; ++i) {
							eraseList.add(outRects[i]);
						}
					}
					addedToEraseList = true;
				}
			}

			if (!outerPlane->_redrawAllCount) {
				const int splitCount = splitRects(outerPlane->_screenRect, visiblePlane->_screenRect, outRects);
				if (splitCount) {
					for (int i = 0; i < splitCount; ++i) {
						eraseList.add(outRects[i]);
					}
					addedToEraseList = true;
				}
			}
		}

		if (addedToEraseList) {
			for (RectList::size_type rectIndex = 0; rectIndex < eraseList.size(); ++rectIndex) {
				const Common::Rect &rect = *eraseList[rectIndex];
				for (int innerPlaneIndex = planeCount - 1; innerPlaneIndex >= 0; --innerPlaneIndex) {
					const Plane &innerPlane = *_planes[innerPlaneIndex];

					if (
						!innerPlane._deleted &&
						innerPlane._type != kPlaneTypeTransparent &&
						innerPlane._screenRect.intersects(rect)
					) {
						if (!innerPlane._redrawAllCount) {
							eraseLists[innerPlaneIndex].add(innerPlane._screenRect.findIntersectingRect(rect));
						}

						const int splitCount = splitRects(rect, innerPlane._screenRect, outRects);
						for (int i = 0; i < splitCount; ++i) {
							eraseList.add(outRects[i]);
						}

						eraseList.erase_at(rectIndex);
						break;
					}
				}
			}

			eraseList.pack();
		}
	}

	// clean up deleted planes
	if (deletedPlaneCount) {
		for (int planeIndex = planeCount - 1; planeIndex >= 0; --planeIndex) {
			Plane *plane = _planes[planeIndex];

			if (plane->_deleted) {
				--plane->_deleted;
				if (plane->_deleted <= 0) {
					const int visiblePlaneIndex = _visiblePlanes.findIndexByObject(plane->_object);
					if (visiblePlaneIndex != -1) {
						_visiblePlanes.remove_at(visiblePlaneIndex);
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

	// Some planes may have been deleted, so re-retrieve count
	planeCount = _planes.size();

	for (PlaneList::size_type outerIndex = 0; outerIndex < planeCount; ++outerIndex) {
		// "outer" just refers to the outer loop
		Plane &outerPlane = *_planes[outerIndex];
		if (outerPlane._priorityChanged) {
			--outerPlane._priorityChanged;

			const Plane *visibleOuterPlane = _visiblePlanes.findByObject(outerPlane._object);
			if (visibleOuterPlane == nullptr) {
				warning("calcLists could not find visible plane for %04x:%04x", PRINT_REG(outerPlane._object));
				continue;
			}

			eraseList.add(outerPlane._screenRect.findIntersectingRect(visibleOuterPlane->_screenRect));

			for (int innerIndex = (int)planeCount - 1; innerIndex >= 0; --innerIndex) {
				// "inner" just refers to the inner loop
				const Plane &innerPlane = *_planes[innerIndex];
				const Plane *visibleInnerPlane = _visiblePlanes.findByObject(innerPlane._object);

				const RectList::size_type rectCount = eraseList.size();
				for (RectList::size_type rectIndex = 0; rectIndex < rectCount; ++rectIndex) {
					const int splitCount = splitRects(*eraseList[rectIndex], innerPlane._screenRect, outRects);
					if (splitCount == 0) {
						if (visibleInnerPlane != nullptr) {
							// same priority, or relative priority between inner/outer changed
							if ((visibleOuterPlane->_priority - visibleInnerPlane->_priority) * (outerPlane._priority - innerPlane._priority) <= 0) {
								if (outerPlane._priority <= innerPlane._priority) {
									eraseLists[innerIndex].add(*eraseList[rectIndex]);
								} else {
									eraseLists[outerIndex].add(*eraseList[rectIndex]);
								}
							}
						}

						eraseList.erase_at(rectIndex);
					} else if (splitCount != -1) {
						for (int i = 0; i < splitCount; ++i) {
							eraseList.add(outRects[i]);
						}

						if (visibleInnerPlane != nullptr) {
							// same priority, or relative priority between inner/outer changed
							if ((visibleOuterPlane->_priority - visibleInnerPlane->_priority) * (outerPlane._priority - innerPlane._priority) <= 0) {
								*eraseList[rectIndex] = outerPlane._screenRect.findIntersectingRect(innerPlane._screenRect);

								if (outerPlane._priority <= innerPlane._priority) {
									eraseLists[innerIndex].add(*eraseList[rectIndex]);
								} else {
									eraseLists[outerIndex].add(*eraseList[rectIndex]);
								}
							}
						}
						eraseList.erase_at(rectIndex);
					}
				}
				eraseList.pack();
			}
		}
	}

	for (PlaneList::size_type planeIndex = 0; planeIndex < planeCount; ++planeIndex) {
		Plane &plane = *_planes[planeIndex];
		Plane *visiblePlane = _visiblePlanes.findByObject(plane._object);

		if (!plane._screenRect.isEmpty()) {
			if (plane._redrawAllCount) {
				plane.redrawAll(visiblePlane, _planes, drawLists[planeIndex], eraseLists[planeIndex]);
			} else {
				if (visiblePlane == nullptr) {
					error("Missing visible plane for source plane %04x:%04x", PRINT_REG(plane._object));
				}

				plane.calcLists(*visiblePlane, _planes, drawLists[planeIndex], eraseLists[planeIndex]);
			}
		} else {
			plane.decrementScreenItemArrayCounts(visiblePlane, false);
		}

		if (plane._moved) {
			// the work for handling moved/resized planes was already done
			// earlier in the function, we are just cleaning up now
			--plane._moved;
		}

		if (plane._created) {
			_visiblePlanes.add(new Plane(plane));
			--plane._created;
		} else if (plane._updated) {
			*visiblePlane = plane;
			--plane._updated;
		}
	}

	// NOTE: SSCI only looks for kPlaneTypeTransparent, not
	// kPlaneTypeTransparentPicture
	if (foundTransparentPlane) {
		for (PlaneList::size_type planeIndex = 0; planeIndex < planeCount; ++planeIndex) {
			for (PlaneList::size_type i = planeIndex + 1; i < planeCount; ++i) {
				if (_planes[i]->_type == kPlaneTypeTransparent) {
					_planes[i]->filterUpEraseRects(drawLists[i], eraseLists[planeIndex]);
				}
			}

			if (_planes[planeIndex]->_type == kPlaneTypeTransparent) {
				for (int i = (int)planeIndex - 1; i >= 0; --i) {
					_planes[i]->filterDownEraseRects(drawLists[i], eraseLists[i], eraseLists[planeIndex]);
				}

				if (eraseLists[planeIndex].size() > 0) {
					error("Transparent plane's erase list not absorbed");
				}
			}

			for (PlaneList::size_type i = planeIndex + 1; i < planeCount; ++i) {
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

	const RectList::size_type eraseListSize = eraseList.size();
	for (RectList::size_type i = 0; i < eraseListSize; ++i) {
		mergeToShowList(*eraseList[i], _showList, _overdrawThreshold);
		_currentBuffer.fillRect(*eraseList[i], plane._back);
	}
}

void GfxFrameout::drawScreenItemList(const DrawList &screenItemList) {
	const DrawList::size_type drawListSize = screenItemList.size();
	for (DrawList::size_type i = 0; i < drawListSize; ++i) {
		const DrawItem &drawItem = *screenItemList[i];
		mergeToShowList(drawItem.rect, _showList, _overdrawThreshold);
		const ScreenItem &screenItem = *drawItem.screenItem;
		// TODO: Remove
//		debug("Drawing item %04x:%04x to %d %d %d %d", PRINT_REG(screenItem._object), PRINT_RECT(drawItem.rect));
		CelObj &celObj = *screenItem._celObj;
		celObj.draw(_currentBuffer, screenItem, drawItem.rect, screenItem._mirrorX ^ celObj._mirrorX);
	}
}

void GfxFrameout::mergeToShowList(const Common::Rect &drawRect, RectList &showList, const int overdrawThreshold) {
	RectList mergeList;
	Common::Rect merged;
	mergeList.add(drawRect);

	for (RectList::size_type i = 0; i < mergeList.size(); ++i) {
		bool didMerge = false;
		const Common::Rect &r1 = *mergeList[i];
		if (!r1.isEmpty()) {
			for (RectList::size_type j = 0; j < showList.size(); ++j) {
				const Common::Rect &r2 = *showList[j];
				if (!r2.isEmpty()) {
					merged = r1;
					merged.extend(r2);

					int difference = merged.width() * merged.height();
					difference -= r1.width() * r1.height();
					difference -= r2.width() * r2.height();
					if (r1.intersects(r2)) {
						const Common::Rect overlap = r1.findIntersectingRect(r2);
						difference += overlap.width() * overlap.height();
					}

					if (difference <= overdrawThreshold) {
						mergeList.erase_at(i);
						showList.erase_at(j);
						mergeList.add(merged);
						didMerge = true;
						break;
					} else {
						Common::Rect outRects[2];
						int splitCount = splitRectsForRender(*mergeList[i], *showList[j], outRects);
						if (splitCount != -1) {
							mergeList.add(*mergeList[i]);
							mergeList.erase_at(i);
							showList.erase_at(j);
							didMerge = true;
							while (splitCount--) {
								mergeList.add(outRects[splitCount]);
							}
							break;
						}
					}
				}
			}

			if (didMerge) {
				showList.pack();
			}
		}
	}

	mergeList.pack();
	for (RectList::size_type i = 0; i < mergeList.size(); ++i) {
		showList.add(*mergeList[i]);
	}
}

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

		// TODO: Sometimes transition screen items generate zero-dimension
		// show rectangles. Is this a bug?
		if (rounded.width() == 0 || rounded.height() == 0) {
			warning("Zero-dimension show rectangle ignored");
			continue;
		}

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

void GfxFrameout::kernelFrameOut(const bool shouldShowBits) {
	if (_transitions->hasShowStyles()) {
		_transitions->processShowStyles();
	} else if (_palMorphIsOn) {
		palMorphFrameOut(_transitions->_styleRanges, nullptr);
		_palMorphIsOn = false;
	} else {
		if (_transitions->hasScrolls()) {
			_transitions->processScrolls();
		}

		frameOut(shouldShowBits);
	}

	throttle();
}

void GfxFrameout::throttle() {
	if (_throttleFrameOut) {
		uint8 throttleTime;
		if (_throttleState == 2) {
			throttleTime = 16;
			_throttleState = 0;
		} else {
			throttleTime = 17;
			++_throttleState;
		}

		g_sci->getEngineState()->speedThrottler(throttleTime);
		g_sci->getEngineState()->_throttleTrigger = true;
	}
}

void GfxFrameout::showRect(const Common::Rect &rect) {
	if (!rect.isEmpty()) {
		_showList.clear();
		_showList.add(rect);
		showBits();
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
