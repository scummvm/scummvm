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

#include "engines/nancy/datatypes.h"

#include "common/stream.h"

namespace Nancy {

// Simple helper function to read rectangles
// TODO identical to the one in recordtypes.h, move somewhere else
static void readRect(Common::SeekableReadStream &stream, Common::Rect &inRect) {
    inRect.left = stream.readUint32LE();
    inRect.top = stream.readUint32LE();
    inRect.right = stream.readUint32LE();
    inRect.bottom = stream.readUint32LE();
}

void SceneSummary::read(Common::SeekableReadStream &stream) {
    char *buf = new char[0x32];

    stream.read(buf, 0x31);
    buf[32] = 0;
    description = Common::String(buf);

    stream.seek(1, SEEK_CUR);
    stream.read(buf, 9);
    buf[9] = 0;
    videoFile = Common::String(buf);

    // skip 1 extra byte & 2 unknown bytes
    stream.seek(3, SEEK_CUR);
    videoFormat = stream.readUint16LE();

    stream.read(buf, 10);
    buf[9] = 0;
    audioFile = Common::String(buf);
    audioID = stream.readSint16LE();
    stream.skip(0xE);
    audioVolume = stream.readUint16LE();

    stream.seek(0x72);
    verticalScrollDelta = stream.readUint16LE();
    horizontalEdgeSize = stream.readUint16LE();
    verticalEdgeSize = stream.readUint16LE();
    slowMoveTimeDelta = stream.readUint16LE();
    fastMoveTimeDelta = stream.readUint16LE();
    unknown7C = stream.readByte();

    delete[] buf;
}

// Takes a VIEW chunk as input
void View::read(Common::SeekableReadStream &stream) {
    stream.seek(0);
    readRect(stream, destination);
    readRect(stream, source);
    readRect(stream, f1Dest);
    readRect(stream, f2Dest);
}

// Takes a CURS chunk as input
void Cursors::read(Common::SeekableReadStream &stream) {
    stream.seek(0);
    for (uint i = 0; i < 85; ++i) {
        Common::Rect &rect = rects[i];
        rect.left = stream.readUint32LE();
        rect.top = stream.readUint32LE();
        rect.right = stream.readUint32LE();
        rect.bottom = stream.readUint32LE();
    }
    primaryVideoCursorX = stream.readUint16LE();
    primaryVideoCursorY = stream.readUint16LE();
}

void Inventory::read(Common::SeekableReadStream &stream) {
    stream.seek(0, SEEK_SET);

    readRect(stream, sliderSource);
    // Stored with uint16s for some reason
    sliderDefaultDest.x = stream.readUint16LE();
    sliderDefaultDest.y = stream.readUint16LE();

    stream.seek(0xD6, SEEK_SET);
    for (uint i = 0; i < 14; ++i) {
        readRect(stream, shadesSrc[i]);
    }
    readRect(stream, shadesDst);
    shadesFrameTime = stream.readUint16LE();

    char name[10];
    stream.read(name, 10);
    inventoryBoxIconsImageName = Common::String(name);
    stream.read(name, 10);
    inventoryCursorsImageName = Common::String(name);

    stream.skip(8);
    readRect(stream, emptySpaceSource);

    char itemName[14];
    for (uint i = 0; i < 11; ++i) {
        stream.read(itemName, 14);
        items[i].name = Common::String(itemName);
        items[i].unknown = stream.readUint16LE();
        readRect(stream, items[i].sourceRect);
    }
}

} // End of namespace Nancy