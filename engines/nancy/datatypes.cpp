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

SceneSummary::SceneSummary(Common::SeekableReadStream *stream) {
    char *buf = new char[0x32];

    stream->read(buf, 0x31);
    buf[32] = 0;
    description = Common::String(buf);

    stream->seek(1, SEEK_CUR);
    stream->read(buf, 9);
    buf[9] = 0;
    videoFile = Common::String(buf);

    // skip 1 extra byte & 2 unknown bytes
    stream->seek(3, SEEK_CUR);
    videoFormat = stream->readUint16LE();

    stream->read(buf, 9);
    buf[9] = 0;
    audioFile = Common::String(buf);

    stream->seek(0x72);
    verticalScrollDelta = stream->readUint16LE();
    sceneHasRotation = stream->readUint16LE();
    sceneHasMovement = stream->readUint16LE();
    slowMoveTimeDelta = stream->readUint16LE();
    fastMoveTimeDelta = stream->readUint16LE();
    unknown7C = stream->readByte();

    // put the entire chunk into a temp buffer until we figure out the rest of the structure
    stream->seek(0);
    chunkData = new byte[stream->size()];
    stream->read(chunkData, stream->size());

    delete[] buf;
}

// Takes a VIEW chunk as input
View::View(Common::SeekableReadStream *stream) {
    stream->seek(0);
    destLeft = stream->readUint32LE();
    destTop = stream->readUint32LE();
    destRight = stream->readUint32LE();
    destBottom = stream->readUint32LE();
    srcLeft = stream->readUint32LE();
    srcTop = stream->readUint32LE();
    srcRight = stream->readUint32LE();
    srcBottom = stream->readUint32LE();
    f1Left = stream->readUint32LE();
    f1Top = stream->readUint32LE();
    f1Right = stream->readUint32LE();
    f1Bottom = stream->readUint32LE();
    f2Left = stream->readUint32LE();
    f2Top = stream->readUint32LE();
    f2Right = stream->readUint32LE();
    f2Bottom = stream->readUint32LE();
}

// Takes a CURS chunk as input
Cursors::Cursors(Common::SeekableReadStream *stream) {
    stream->seek(0);
    for (uint i = 0; i < 85; ++i) {
        Common::Rect &rect = rects[i];
        rect.left = stream->readUint32LE();
        rect.top = stream->readUint32LE();
        rect.right = stream->readUint32LE();
        rect.bottom = stream->readUint32LE();
    }
    primaryVideoCursorX = stream->readUint16LE();
    primaryVideoCursorY = stream->readUint16LE();
}

} // End of namespace Nancy