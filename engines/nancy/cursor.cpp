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

#include "graphics/cursorman.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/util.h"

namespace Nancy {

CursorManager::CursorManager()  :
	_isInitialized(false),
	_curItemID(-1),
	_curCursorType(kNormal),
	_curCursorID(0),
	_hasItem(false),
	_numCursorTypes(0),
	_puzzleExitCursor((g_nancy->getGameType() >= kGameTypeNancy4) ? kMoveBackward : kExit) {}

void CursorManager::init(Common::SeekableReadStream *chunkStream) {
	assert(chunkStream);

	chunkStream->seek(0);

	if (g_nancy->getGameType() == kGameTypeVampire) {
		_numCursorTypes = g_nancy->getStaticData().numNonItemCursors / 2;
	} else {
		_numCursorTypes = g_nancy->getStaticData().numNonItemCursors / 3;
	}

	uint numCursors = g_nancy->getStaticData().numNonItemCursors + g_nancy->getStaticData().numItems * _numCursorTypes;
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

	const INV *inventoryData = (const INV *)g_nancy->getEngineData("INV");
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
		if (gameType <= kGameTypeNancy1) {
			_curCursorID = 4;
		} else if (gameType == kGameTypeNancy2) {
			_curCursorID = 5;
		} else if (gameType ==  kGameTypeNancy3) {
			_curCursorID = 8;
		} else if (gameType <= kGameTypeNancy5) {
			_curCursorID = 12;
		} else {
			_curCursorID = 16;
		}

		return;
	case kHotspotArrow:
		if (gameType <= kGameTypeNancy1) {
			_curCursorID = 5;
		} else if (gameType == kGameTypeNancy2) {
			_curCursorID = 6;
		} else if (gameType == kGameTypeNancy3) {
			_curCursorID = 9;
		} else if (gameType <= kGameTypeNancy5) {
			_curCursorID = 13;
		} else {
			_curCursorID = 17;
		}

		return;
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
	case kSwivelLeft:
		// Only valid for nancy6 and up, but we don't need a check for now
		_curCursorID = kSwivelLeft;
		return;
	case kSwivelRight:
		// Only valid for nancy6 and up, but we don't need a check for now
		_curCursorID = kSwivelRight;
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
		itemsOffset = g_nancy->getStaticData().numNonItemCursors;
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

void CursorManager::applyCursor() {
	Graphics::ManagedSurface *surf;
	Common::Rect bounds = _cursors[_curCursorID].bounds;
	Common::Point hotspot = _cursors[_curCursorID].hotspot;

	if (_hasItem) {
		surf = &_invCursorsSurface;

	} else {
		surf = &g_nancy->_graphicsManager->_object0;
	}

	Graphics::ManagedSurface temp(*surf, bounds);

	CursorMan.replaceCursor(temp, hotspot.x, hotspot.y, g_nancy->_graphicsManager->getTransColor(), false);
	if (g_nancy->getGameType() == kGameTypeVampire) {
		byte palette[3 * 256];
		surf->grabPalette(palette, 0, 256);
		CursorMan.replaceCursorPalette(palette, 0, 256);
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
