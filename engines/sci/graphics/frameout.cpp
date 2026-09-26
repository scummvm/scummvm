/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
#include "engines/util.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"

#include "sci/sci.h"
#include "sci/console.h"
#include "sci/event.h"
#include "sci/engine/features.h"
#include "sci/engine/kernel.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/engine/vm.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/compare.h"
#include "sci/graphics/cursor32.h"
#include "sci/graphics/scifont.h"
#include "sci/graphics/frameout.h"
#include "sci/graphics/helpers.h"
#include "sci/graphics/paint32.h"
#include "sci/graphics/palette32.h"
#include "sci/graphics/plane32.h"
#include "sci/graphics/remap32.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/screen_item32.h"
#include "sci/graphics/text32.h"
#include "sci/graphics/frameout.h"
#include "sci/graphics/transitions32.h"
#include "sci/graphics/video32
.h"

namespace Sci {

GfxFrameout::GfxFrameout(SegManager *segMan, GfxPalette32 *palette, GfxTransitions32 *transitions, GfxCursor32 *cursor) :
	_isHiRes(detectHiRes()),
	_palette(palette),
	_cursor(cursor),
	_segMan(segMan),
	_transitions(transitions),
	_throttleState(0),
	_remapOccurred(false),
	_overdrawThreshold(0),
	_throttleKernelFrameOut(true),
	_palMorphIsOn(false),
	_lastScreenUpdateTick(0) {

	if (g_sci->getGameId() == GID_PHANTASMAGORIA) {
		_currentBuffer.create(630, 450, Graphics::PixelFormat::createFormatCLUT8());
	} else if (_isHiRes) {
		_currentBuffer.create(640, 480, Graphics::PixelFormat::createFormatCLUT8());
	} else {
		_currentBuffer.create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	}
	initGraphics(_currentBuffer.w, _currentBuffer.h);

	switch (g_sci->getGameId()) {
	case GID_HOYLE5:
		if (g_sci->getResMan()->testResource(ResourceId(kResourceTypeView, 21))) {
			// Hoyle school house
			_scriptWidth = 320;
			_scriptHeight = 200;
			break;
		}
		// fall-through
	case GID_LIGHTHOUSE:
	case GID_LSL7:
	case GID_PHANTASMAGORIA2:
	case GID_TORIN:
	case GID_RAMA:
		_scriptWidth = 640;
		_scriptHeight = 480;
		break;
	case GID_GK2:
	case GID_PQSWAT:
		if (!g_sci->isDemo()) {
			_scriptWidth = 640;
			_scriptHeight = 480;
			break;
		}
		// fall through
	default:
		_scriptWidth = 320;
		_scriptHeight = 200;
		break;
	}
}

GfxFrameout::~GfxFrameout() {
	clear();
	CelObj::deinit();
	_currentBuffer.free();
}

void GfxFrameout::run() {
	CelObj::init();
	Plane::init();
	ScreenItem::init();
	GfxText32::init();

	// This plane is created in SCI::InitPlane in SSCI, and is a background fill
	// plane to ensure "hidden" planes (planes with negative priority) are never
	// drawn
	Plane *initPlane = new Plane(Common::Rect(_scriptWidth, _scriptHeight));
	initPlane->_priority = 0;
	_planes.add(initPlane);
}

void GfxFrameout::clear() {
	_planes.clear();
	_visiblePlanes.clear();
	_showList.clear();
	_screenItemLists.clear();
}

bool GfxFrameout::detectHi
Res() const {
	// QFG4 is always low resolution
	if (g_sci->getGameId() == GID_QFG4) {
		return false;
	}

	// PQ4 DOS floppy is low resolution only
	if (g_sci->getGameId() == GID_PQ4 &&
		g_sci->getPlatform() == Common::kPlatformDOS &&
		!g_sci->isCD()) {
		return false;
	}

	// GK1 DOS floppy is low resolution only, but GK1 Mac floppy is high
	// resolution only
	if (g_sci->getGameId() == GID_GK1 &&
		!g_sci->isCD() &&
		g_sci->getPlatform() != Common::kPlatformMacintosh) {

		return false;
	}

	// All other games are either high resolution by default, or have a
	// user-defined toggle
	return ConfMan.getBool("enable_high_resolution_graphics");
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
	delete
ScreenItem(screenItem, *plane);
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
			// Script bug in PQ:SWAT, when skipping the Tactics Training
			warning("kUpdateScreenItem: Plane %04x:%04x not found for screen item %04x:%04x", PRINT_REG(planeObject), PRINT_REG(object));
			return;
		}

		ScreenItem *screenItem = plane->_screenItemList.findByObject(object);
		if (screenItem == nullptr) {
			error("kUpdateScre
enItem: Screen item %04x:%04x not found in plane %04x:%04x", PRINT_REG(object), PRINT_REG(planeObject));
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
		addPlane(plane);
	}

	// Detect the QFG4 import character dialog, disable the Change Directory
	//  button, and display a message box explaining how to import saved
	//  character files in ScummVM. SCI16 games are handled by kDrawControl.
	if (g_sci->inQfGImportRoom()) {
		// kAddPlane is called several times, this detects the second call
		//  which is for the import character dialog. If changeButton:value
		//  is non-zero then the dialog is initializing. If the button isn't
		//  disabled then we haven't displayed the message box yet. There
		//  are multiple changeButtons because the script clones the object.
		SegManager *segMan = g_sci->getEngineState()->_segMan;
		Common::Array<reg_t> changeDirButtons = _segMan->findObjectsByName("changeButton");
		for (uint i = 0; i < changeDirButtons.size(); ++i) {
			if (readSelectorValue(segMan, changeDirButtons[i], SELECTOR(value))) {
				// disable Change Directory button by s
etting state to zero
				if (readSelectorValue(segMan, changeDirButtons[i], SELECTOR(state))) {
					writeSelectorValue(segMan, changeDirButtons[i], SELECTOR(state), 0);
					g_sci->showQfgImportMessageBox();
					break;
				}
			}
		}
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
		// SSCI calls some `AbortPlane` function that just ends up doing this
		// anyway, so we skip the extra indirection
		_planes.erase(plane);
	} else {
		plane->_created = 0;
		plane->_deleted = getScreenCount();
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

void GfxFrameout::deletePlanesForMacRestore() {
	// SCI32 PC games delete planes and screen items from
	//  their Game:restore script before calling kRestore.
	//  In Mac this work was moved into the interpreter
	//  for some games, while others added it back to
	//  Game:restore or used their own scripts that took
	//  care of this in both PC and Mac versions.
	if (!(g_sci->getGameId() == GID_GK1 ||
		  g_sci->getGameId() == GID_PQ4 ||
		  g_sci->getGameId() == GID_LSL6HIRES ||
		  g_sci->getGameId() == GID_KQ7)) {
		return;
	}

	for (PlaneList::size_type i = 0; i < _planes.size(); ) {
		Plane *plane = _planes[i];

		// don't delete the default plane
		if (plane->isDefaultPlane()
) {
			i++;
			continue;
		}

		// delete all inserted screen items from the plane
		for (ScreenItemList::size_type j = 0; j < plane->_screenItemList.size(); ++j) {
			ScreenItem *screenItem = plane->_screenItemList[j];
			if (screenItem != nullptr &&
				!screenItem->_object.isNumber() &&
				_segMan->getObject(screenItem->_object)->isInserted()) {

				// delete the screen item
				if (screenItem->_created) {
					plane->_screenItemList.erase_at(j);
				} else {
					screenItem->_updated = 0;
					screenItem->_deleted = getScreenCount();
				}
			}
		}
		plane->_screenItemList.pack();

		// delete the plane
		if (plane->_created) {
			_planes.erase(plane);
		} else {
			plane->_moved = 0;
			plane->_deleted = getScreenCount();
			i++;
		}
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

		// If object is a number, the screen item from the engine, not a script,
		// and should be ignored
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

void GfxFrameout::addPlane(Plane *plane) {
	// In SSCI, if a plane with the same object ID already existed, this call
	// would cancel deletion and update an already-existing plane, but caller
s
	// expect the passed plane object to become memory-managed by GfxFrameout,
	// so doing what SSCI did would end up leaking the Plane objects
	if (_planes.findByObject(plane->_object) != nullptr) {
		error("Plane %04x:%04x already exists", PRINT_REG(plane->_object));
	}

	plane->clipScreenRect(Common::Rect(_currentBuffer.w, _currentBuffer.h));
	_planes.add(plane);
}

void GfxFrameout::updatePlane(Plane &plane) {
	// This assertion comes from SSCI
	assert(_planes.findByObject(plane._object) == &plane);

	Plane *visiblePlane = _visiblePlanes.findByObject(plane._object);
	plane.sync(visiblePlane, Common::Rect(_currentBuffer.w, _currentBuffer.h));
	// updateScreenRect was called a second time here in SSCI, but it is already
	// called at the end of the sync call (also in SSCI) so there is no reason
	// to do it again

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
	updateMousePositionForRendering();

	RobotDecoder &robotPlayer = g_sci->_video32->getRobotPlayer();
	const bool robotIsActive = robotPlayer.getStatus() != RobotDecoder::kRobotStatusUninitialized;

	if (robotIsActive) {
		robotPlayer.doRobot();
	}

	// SSCI allocated these as static arrays of 100 pointers to
	// ScreenItemList / RectList
	_screenItemLists.resize(_planes.size());
	for (DrawList::size_type i = 0; i < _screenItemLists.size(); ++i) {
		_screenItemLists[i].clear();
	}
	EraseListList eraseLists(_planes.size());

	if (g_sci->_gfxRemap32->getRemapCount() > 0 && _remapOccurred) {

		remapMarkRedraw();
	}

	calcLists(_screenItemLists, eraseLists, eraseRect);

	for (ScreenItemListList::iterator list = _screenItemLists.begin(); list != _screenItemLists.end(); ++list) {
		list->sort();
	}

	for (ScreenItemListList::iterator list = _screenItemLists.begin(); list != _screenItemLists.end(); ++list) {
		for (DrawList::iterator drawItem = list->begin(); drawItem != list->end(); ++drawItem) {
			(*drawItem)->screenItem->getCelObj().submitPalette();
		}
	}

	_remapOccurred = _palette->updateForFrame();

	for (PlaneList::size_type i = 0; i < _planes.size(); ++i) {
		drawEraseList(eraseLists[i], *_planes[i]);
		drawScreenItemList(_screenItemLists[i]);
	}

	if (robotIsActive) {
		robotPlayer.frameAlmostVisible();
	}

	_palette->updateHardware();

	if (shouldShowBits) {
		showBits();
	}

	if (robotIsActive) {
		robotPlayer.frameNowVisible();
	}
}

void GfxFrameout::palMorphFrameOut(const int8 *styleRanges, PlaneShowStyle *showStyle) {
	updateMousePositionForRendering();

	Palette sourcePalette(_palette->getNextPalette());
	alterVmap(sourcePalette, sourcePalette, -1, styleRanges);

	int16 prevRoom = g_sci->getEngineState()->variables[VAR_GLOBAL][kGlobalVarPreviousRoomNo].toSint16();

	Common::Rect rect(_currentBuffer.w, _currentBuffer.h);
	_showList.add(rect);
	showBits();

	// SSCI allocated these as static arrays of 100 pointers to
	// ScreenItemList / RectList
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

	for (PlaneList::size_type i = 0; i < _planes.size(); ++i) {
		drawEraseList(eraseLists[i], *_planes[i]);
		drawScreenItemList(screenItemLists[i]);
	}

	_palette->submit(nextPalette);
	_palette->upda
teFFrame();
	_palette->updateHardware();
	showBits();
}

void GfxFrameout::directFrameOut(const Common::Rect &showRect) {
	updateMousePositionForRendering();
	_showList.add(showRect);
	showBits();
}

void GfxFrameout::redrawGameScreen(const Common::Rect &skipRect) const {
	Common::ScopedPtr<Graphics::Surface> game(_currentBuffer.convertTo(g_system->getScreenFormat(), _palette->getHardwarePalette()));
	assert(game);

	Common::Rect rects[4];
	int splitCount = splitRects(Common::Rect(game->w, game->h), skipRect, rects);
	if (splitCount != -1) {
		while (splitCount--) {
			const Common::Rect &drawRect = rects[splitCount];
			g_system->copyRectToScreen(game->getBasePtr(drawRect.left, drawRect.top), game->pitch, drawRect.left, drawRect.top, drawRect.width(), drawRect.height());
		}
	}

	game->free();
}

void GfxFrameout::resetHardware() {
	updateMousePositionForRendering();
	_showList.add(Common::Rect(_currentBuffer.w, _currentBuffer.h));
	g_system->getPaletteManager()->setPalette(_palette->getHardwarePalette(), 0, 256);
	showBits();
}

/**
 * Determines the parts of `middleRect` that aren't overlapped by `showRect`,
 * optimised for contiguous memory writes.
 *
 * `middleRect` is modified directly to extend into the upper and lower rects.
 *
 * @returns -1 if `middleRect` and `showRect` have no intersection, or the
 * number of returned parts (in `outRects`) otherwise. (In particular, this
 * returns 0 if `middleRect` is contained in `showRect`.)
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
		upperLeft =
 showRect.left;
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

// The third rectangle parameter is only ever passed by VMD code
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
	for (Plan
eList::size_type outerPlaneIndex = 0; outerPlaneIndex < planeCount; ++outerPlaneIndex) {
		const Plane *outerPlane = _planes[outerPlaneIndex];
		const Plane *visiblePlane = _visiblePlanes.findByObject(outerPlane->_object);

		// SSCI only ever checks for kPlaneTypeTransparent here, even though
		// kPlaneTypeTransparentPicture is also a transparent plane
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
						if (!innerPlane._re
drawAllCount) {
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
				for (RectList::size_type rec
tIndex = 0; rectIndex < rectCount; ++rectIndex) {
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

				plane.calcLists(*visiblePlane, _plan
es, drawLists[planeIndex], eraseLists[planeIndex]);
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
			if (visiblePlane == nullptr) {
				error("[GfxFrameout::calcLists]: Attempt to update nonexistent visible plane");
			}

			*visiblePlane = plane;
			--plane._updated;
		}
	}

	// SSCI really only looks for kPlaneTypeTransparent, not
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

void GfxFrameout::drawScreenItemList(const Draw
List &screenItemList) {
	const DrawList::size_type drawListSize = screenItemList.size();
	for (DrawList::size_type i = 0; i < drawListSize; ++i) {
		const DrawItem &drawItem = *screenItemList[i];
		mergeToShowList(drawItem.rect, _showList, _overdrawThreshold);
		const ScreenItem &screenItem = *drawItem.screenItem;
		CelObj &celObj = *screenItem._celObj;
		celObj.draw(_currentBuffer, screenItem, drawItem.rect, screenItem._mirrorX ^ celObj._mirrorX);
	}
}

void GfxFrameout::mergeToShowList(const Common::Rect &drawRect, RectList &showList, const int overdrawThreshold) {
	RectList mergeList;
	Common::Rect merged;
	mergeList.add(drawRect);

	for (RectList::size_type i = 0; i < mergeList.size(); ++i) {
		const Common::Rect &r1 = *mergeList[i];
		if

... [Content truncated]