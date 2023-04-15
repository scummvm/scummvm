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

void CursorManager::init(Common::SeekableReadStream *chunkStream) {
	assert(chunkStream);

	chunkStream->seek(0);
	uint numCursorTypes = g_nancy->getGameType() <= kGameTypeNancy1 ? 4 : 5;
	uint numCursors = g_nancy->getStaticData().numNonItemCursors + g_nancy->getStaticData().numItems * numCursorTypes;
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

	g_nancy->_resource->loadImage(g_nancy->_inventoryData->inventoryCursorsImageName, _invCursorsSurface);

	setCursor(kNormalArrow, -1);
	showCursor(false);

	_isInitialized = true;
	
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

	switch (type) {
	case kNormalArrow:
		if (gameType <= kGameTypeNancy1) {
			_curCursorID = 4;
		} else {
			_curCursorID = 5;
		}
		
		break;
	case kHotspotArrow:
		if (gameType <= kGameTypeNancy1) {
			_curCursorID = 5;
		} else {
			_curCursorID = 6;
		}

		break;
	case kExit:
		if (gameType != kGameTypeVampire) {
			_curCursorID = 3;
			break;
		}
		// fall through
	default: {
		uint itemsOffset = 0;
		if (itemID == -1) {
			// No item held, set to eyeglass
			itemID = 0;
		} else {
			// Item held
			itemsOffset = g_nancy->getStaticData().numNonItemCursors;
			_hasItem = true;
		}

		_curCursorID = itemID * (gameType <= kGameTypeNancy1? 4 : 5) + itemsOffset + type;
	}
	}
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

	// Create a temporary surface to hold the cursor since giving replaceCursor() a pointer
	// to the original surface results in garbage. This also makes it so we don't have to deal
	// with TVD's palettes
	Graphics::ManagedSurface temp;
	temp.create(bounds.width(), bounds.height(), g_nancy->_graphicsManager->getScreenPixelFormat());
	temp.blitFrom(*surf, bounds, Common::Point());

	// Convert the trans color from the original format to the screen format
	uint transColor;
	if (g_nancy->getGameType() == kGameTypeVampire) {
		uint8 palette[1 * 3];
		surf->grabPalette(palette, 1, 1);
		transColor = temp.format.RGBToColor(palette[0], palette[1], palette[2]);
	} else {
		uint8 r, g, b;
		surf->format.colorToRGB(g_nancy->_graphicsManager->getTransColor(), r, g, b);
		transColor = temp.format.RGBToColor(r, g, b);
	}

	CursorMan.replaceCursor(temp.getPixels(), temp.w, temp.h, hotspot.x, hotspot.y, transColor, false, &temp.format);
}

void CursorManager::showCursor(bool shouldShow) {
	CursorMan.showMouse(shouldShow);
}

} // End of namespace Nancy
