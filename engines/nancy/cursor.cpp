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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/nancy/cursor.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/util.h"

#include "common/stream.h"
#include "common/str.h"

namespace Nancy {

void CursorManager::init() {
    Common::SeekableReadStream *chunk = NanEngine.getBootChunkStream("INV");
    chunk->seek(0x1D2); // TODO
    Common::String inventoryCursorsImageName = chunk->readString();

    chunk = NanEngine.getBootChunkStream("CURS");
    for (uint i = 0; i < 56; ++i) {
        _cursors.push_back(Cursor());
        chunk->seek(i * 16, SEEK_SET);
        readRect(*chunk, _cursors[i].bounds);
        chunk->seek(0x380 + i * 8, SEEK_SET);
        _cursors[i].hotspot.x = chunk->readUint32LE();
        _cursors[i].hotspot.y = chunk->readUint32LE();
    }

    readRect(*chunk, _primaryVideoInactiveZone);
    _primaryVideoInitialPos.x = chunk->readUint16LE();
    _primaryVideoInitialPos.y = chunk->readUint16LE();

    NanEngine.resource->loadImage(inventoryCursorsImageName, _invCursorsSurface);

    setCursor(kNormalArrow, -1);
    showCursor(false);

    _isInitialized = true;
}

void CursorManager::setCursor(CursorType type, int16 itemID) {
    if (!_isInitialized) {
        return;
    }

    if (type == _curCursorType && itemID == _curItemID) {
        return;
    } else {
        _curCursorType = type;
        _curItemID = itemID;
    }

    uint16 newID = 0;
    bool hasItem = false;

    switch (type) {
    case kNormalArrow:
        newID = 4;
        break;
    case kHotspotArrow:
        newID = 6;
        break;
    case kExitArrow:
        newID = 3;
        break;
    default: {
        if (itemID == -1) {
            // No item held, set to eyeglass
            itemID = 0;
        } else {
            // Item held
            itemID += 3;
            hasItem = true;
        }

        newID = itemID * 4 + type;
    }
    }

    Graphics::ManagedSurface *surf;
    Common::Rect bounds = _cursors[newID].bounds;
    Common::Point hotspot = _cursors[newID].hotspot;

    if (hasItem) {
        surf = &_invCursorsSurface;
        
    } else {
        surf = &NanEngine.graphicsManager->object0;
    }

    // TODO this is ridiculous, figure out why just calling
    // GetBasePtr() results in garbage
    Graphics::Surface s;
    s.create(bounds.width(), bounds.height(), surf->format);
    s.copyRectToSurface(*surf, 0, 0, bounds);

    // TODO hotspots are terrible for arrow cursors, fix that??
    CursorMan.replaceCursor(s.getPixels(), s.w, s.h, hotspot.x, hotspot.y, GraphicsManager::getTransColor(), false, &GraphicsManager::getInputPixelFormat());

    s.free();

}

void CursorManager::setCursorType(CursorType type) {
    setCursor(type, _curItemID);
}

void CursorManager::setCursorItemID(int16 itemID) {
    setCursor(_curCursorType, itemID);
}

void CursorManager::showCursor(bool shouldShow) {
    CursorMan.showMouse(shouldShow);
}

} // End of namespace Nancy
