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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/system.h"
#include "graphics/cursorman.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/util.h"

namespace Nancy {

CursorManager::CursorManager() :
	_isInitialized(false),
	_curItemID(-1),
	_curCursorType(kNormal),
	_curCursorID(0),
	_lastCursorID(10000), // nonsense default value to ensure cursor is drawn the first time
	_hasItem(false),
	_numCursorTypes(0),
	_puzzleExitCursor((g_nancy->getGameType() >= kGameTypeNancy4) ? kMoveBackward : kExit),
	_warpedMousePos(-500, -500) {}

void CursorManager::init(Common::SeekableReadStream *chunkStream) {
	assert(chunkStream);
	chunkStream->seek(0);

	// First, we need to figure out the number of possible CursorTypes in the current game
	_numCursorTypes = g_nancy->getStaticData().numCursorTypes;

	// The structure of CURS is weird:

	// The data is divided in half: first half is source rectangles, second half is hotspots (all of which are identical...)
	// However, each of those halves are divided into a number of arrays, each one of size _numCursorTypes.

	// The first few arrays are the following:
	// - an array of cursors used when the mouse is in the VIEWPORT (hourglass, directional arrows, etc.)
	// - an array of cursors used in the FRAME
	// - an array of cursors used in MENUS (not present in TVD)
	// The only frame cursors used are the first two: the classic arrow cursor, and its hotspot variant, which is slightly shorter
	// The same applies to the menu cursors; however, we completely ignore those (technically the arrow cursor has sliiiiightly
	// different shading from the one in the frame array, but I don't care enough to implement it).

	// Following those are the ITEM arrays; these cursors are used to indicate that the player is holding an item.
	// Their number is the same as the number of items described in INV, and their size is also _numCursorTypes.
	// Out of those arrays, the only cursors that get used are the kNormal and kHotspot ones. The first few games also
	// had kMove item cursors, but the Move cursors quickly fell out of use.

	// Due to the logic in setCursor(), directional arrow cursors found in the VIEWPORT array take precedence over
	// the ones in the item arrays. As a result, most of the CURS data is effectively junk that never gets used.

	// Perhaps in the future the class could be modified so we no longer have to store or care about all of the junk cursors;
	// however, this cannot happen until the engine is more mature and I'm more aware of what changes they made to the
	// cursor code in later games.

	uint numCursors = _numCursorTypes * (g_nancy->getGameType() == kGameTypeVampire ? 2 : 3) + g_nancy->getStaticData().numItems * _numCursorTypes;
	_cursors.resize(numCursors);

	for (uint i = 0; i < numCursors; ++i) {
		readRect(*chunkStream, _cursors[i].bounds);
	}

	for (uint i = 0; i < numCursors; ++i) {
		_cursors[i].hotspot.x = chunkStream->readUint32LE();
		_cursors[i].hotspot.y = chunkStream->readUint32LE();
	}

	readRect(*chunkStream, _primaryVideoInactiveZone);
	_primaryVideoInitialPos.x = chunkStream->readUint16LE();
	_primaryVideoInitialPos.y = chunkStream->readUint16LE();

	auto *inventoryData = GetEngineData(INV);
	assert(inventoryData);

	g_nancy->_resource->loadImage(inventoryData->inventoryCursorsImageName, _invCursorsSurface);

	setCursor(kNormalArrow, -1);
	showCursor(false);

	_isInitialized = true;

	adjustCursorHotspot();

	delete chunkStream;
}

void CursorManager::setCursor(CursorType type, int16 itemID) {
	if (!_isInitialized) {
		return;
	}

	Nancy::GameType gameType = g_nancy->getGameType();

	if (type == _curCursorType && itemID == _curItemID) {
		return;
	} else {
		_curCursorType = type;
		_curItemID = itemID;
	}

	_hasItem = false;

	// For all cases below, the selected cursor is _always_ shown, regardless
	// of whether or not an item is held. All other types of cursor
	// are overridable when holding an item. Every item cursor has
	// _numItemCursor variants, one corresponding to every numbered
	// value of the CursorType enum.
	switch (type) {
	case kNormalArrow:
		_curCursorID = _numCursorTypes;
		return;
	case kHotspotArrow:
		_curCursorID = _numCursorTypes + 1;
		return;
	case kInvertedRotateLeft:
		// Only valid for nancy6 and up
		if (gameType >= kGameTypeNancy6) {
			_curCursorID = kInvertedRotateLeft;
			return;
		}

		// fall through
	case kRotateLeft:
		// Only valid for nancy6 and up
		if (gameType >= kGameTypeNancy6) {
			_curCursorID = kRotateLeft;
			return;
		}

		// fall through
	case kMoveLeft:
		// Only valid for nancy3 and up
		if (gameType >= kGameTypeNancy3) {
			_curCursorID = kMoveLeft;
			return;
		} else {
			type = kMove;
		}

		break;
	case kInvertedRotateRight:
		// Only valid for nancy6 and up
		if (gameType >= kGameTypeNancy6) {
			_curCursorID = kInvertedRotateRight;
			return;
		}

		// fall through
	case kRotateRight:
		// Only valid for nancy6 and up
		if (gameType >= kGameTypeNancy6) {
			_curCursorID = kRotateRight;
			return;
		}

		// fall through
	case kMoveRight:
		// Only valid for nancy3 and up
		if (gameType >= kGameTypeNancy3) {
			_curCursorID = kMoveRight;
			return;
		} else {
			type = kMove;
		}

		break;
	case kMoveUp:
		// Only valid for nancy4 and up
		if (gameType >= kGameTypeNancy4) {
			_curCursorID = kMoveUp;
			return;
		} else {
			type = kMove;
		}

		break;
	case kMoveDown:
		// Only valid for nancy4 and up
		if (gameType >= kGameTypeNancy4) {
			_curCursorID = kMoveDown;
			return;
		} else {
			type = kMove;
		}

		break;
	case kMoveForward:
		// Only valid for nancy4 and up
		if (gameType >= kGameTypeNancy4) {
			_curCursorID = kMoveForward;
			return;
		} else {
			type = kHotspot;
		}

		break;
	case kMoveBackward:
		// Only valid for nancy4 and up
		if (gameType >= kGameTypeNancy4) {
			_curCursorID = kMoveBackward;
			return;
		} else {
			type = kHotspot;
		}

		break;
	case kExit:
		// Not valid in TVD
		if (gameType != kGameTypeVampire) {
			_curCursorID = 3;
			return;
		}

		break;
	case kRotateCW:
		_curCursorID = kRotateCW;
		return;
	case kRotateCCW:
		_curCursorID = kRotateCCW;
		return;
	default:
		break;
	}

	// Special cases have been handled, now choose correct
	// item cursor if holding something
	uint itemsOffset = 0;
	if (itemID == -1) {
		// No item held, set to eyeglass
		itemID = 0;
	} else {
		// Item held
		itemsOffset = _numCursorTypes * (g_nancy->getGameType() == kGameTypeVampire ? 2 : 3);
		_hasItem = true;
	}

	_curCursorID = (itemID * _numCursorTypes) + itemsOffset + type;
}

void CursorManager::setCursorType(CursorType type) {
	setCursor(type, _curItemID);
}

void CursorManager::setCursorItemID(int16 itemID) {
	setCursor(_curCursorType, itemID);
}

void CursorManager::warpCursor(const Common::Point &pos) {
	_warpedMousePos = pos;
}

void CursorManager::applyCursor() {
	if (_curCursorID != _lastCursorID) {
		Graphics::ManagedSurface *surf;
		Common::Rect bounds = _cursors[_curCursorID].bounds;
		Common::Point hotspot = _cursors[_curCursorID].hotspot;

		if (_hasItem) {
			surf = &_invCursorsSurface;

		} else {
			surf = &g_nancy->_graphics->_object0;
		}

		Graphics::ManagedSurface temp(*surf, bounds);

		CursorMan.replaceCursor(temp, hotspot.x, hotspot.y, g_nancy->_graphics->getTransColor(), false);
		if (g_nancy->getGameType() == kGameTypeVampire) {
			byte palette[3 * 256];
			surf->grabPalette(palette, 0, 256);
			CursorMan.replaceCursorPalette(palette, 0, 256);
		}

		_lastCursorID = _curCursorID;
	}

	if (_warpedMousePos.x != -500 && _warpedMousePos.y != -500) {
		g_system->warpMouse(_warpedMousePos.x, _warpedMousePos.y);
		_warpedMousePos.x = -500;
		_warpedMousePos.y = -500;
	}
}

void CursorManager::showCursor(bool shouldShow) {
	CursorMan.showMouse(shouldShow);
}

void CursorManager::adjustCursorHotspot() {
	if (g_nancy->getGameType() == kGameTypeVampire) {
		return;
	}

	// Improvement: the arrow cursor in the Nancy games has an atrocious hotspot that's
	// right in the middle of the graphic, instead of in the top left where
	// it would make sense to be. This function fixes that.
	// The hotspot is still a few pixels lower than it should be to account
	// for the different graphic when hovering UI elements

	// TODO: Make this optional?

	uint startID = _curCursorID;

	setCursorType(kNormalArrow);
	_cursors[_curCursorID].hotspot = {3, 4};
	setCursorType(kHotspotArrow);
	_cursors[_curCursorID].hotspot = {3, 4};

	_curCursorID = startID;
}

} // End of namespace Nancy
