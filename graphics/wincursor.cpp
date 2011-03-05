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
 * $URL$
 * $Id$
 *
 */

#include "common/debug.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/winexe_ne.h"
#include "common/winexe_pe.h"

#include "graphics/wincursor.h"

namespace Graphics {

WinCursor::WinCursor() {
	_width    = 0;
	_height   = 0;
	_hotspotX = 0;
	_hotspotY = 0;
	_surface  = 0;
	_keyColor = 0;
	memset(_palette, 0, 256 * 3);
}

WinCursor::~WinCursor() {
	clear();
}

uint16 WinCursor::getWidth() const {
	return _width;
}

uint16 WinCursor::getHeight() const {
	return _height;
}

uint16 WinCursor::getHotspotX() const {
	return _hotspotX;
}

uint16 WinCursor::getHotspotY() const {
	return _hotspotY;
}

byte WinCursor::getKeyColor() const {
	return _keyColor;
}

bool WinCursor::readFromStream(Common::SeekableReadStream &stream) {
	clear();

	_hotspotX = stream.readUint16LE();
	_hotspotY = stream.readUint16LE();

	// Check header size
	if (stream.readUint32LE() != 40)
		return false;

	// Check dimensions
	_width = stream.readUint32LE();
	_height = stream.readUint32LE() / 2;

	// Color planes
	if (stream.readUint16LE() != 1)
		return false;
	// Bits per pixel
	if (stream.readUint16LE() != 1)
		return false;
	// Compression
	if (stream.readUint32LE() != 0)
		return false;

	// Image size + X resolution + Y resolution
	stream.skip(12);

	uint32 numColors = stream.readUint32LE();

	if (numColors == 0)
		numColors = 2;
	else if (numColors > 2)
		return false;

	// Sanity check: ensure that enough data is there for the whole cursor
	if ((uint32)(stream.size() - stream.pos()) < 40 + numColors * 4 + _width * _height * 2 / 8)
		return false;

	// Standard palette: transparent, black, white
	_palette[6] = 0xff;
	_palette[7] = 0xff;
	_palette[8] = 0xff;

	// Reading the palette
	stream.seek(40 + 4);
	for (uint32 i = 0 ; i < numColors; i++) {
		_palette[(i + 1) * 3 + 2] = stream.readByte();
		_palette[(i + 1) * 3 + 1] = stream.readByte();
		_palette[(i + 1) * 3 + 0] = stream.readByte();
		stream.readByte();
	}

	// Reading the bitmap data
	uint32 dataSize = stream.size() - stream.pos();
	byte *initialSource = new byte[dataSize];
	stream.read(initialSource, dataSize);
	const byte *srcP = initialSource;
	const byte *srcM = srcP + ((_width * _height) / 8);

	_surface = new byte[_width * _height];
	byte *dest = _surface + _width * (_height - 1);

	for (uint32 i = 0; i < _height; i++) {
		byte *rowDest = dest;

		for (uint32 j = 0; j < (_width / 8); j++) {
			byte p = srcP[j];
			byte m = srcM[j];

			for (int k = 0; k < 8; k++, rowDest++, p <<= 1, m <<= 1) {
				if ((m & 0x80) != 0x80) {
					if ((p & 0x80) == 0x80)
						*rowDest = 2;
					else
						*rowDest = 1;
				} else
					*rowDest = _keyColor;
			}
		}

		dest -= _width;
		srcP += _width / 8;
		srcM += _width / 8;
	}

	delete[] initialSource;
	return true;
}

void WinCursor::clear() {
	delete[] _surface; _surface = 0;
}

WinCursorGroup::WinCursorGroup() {
}

WinCursorGroup::~WinCursorGroup() {
	for (uint32 i = 0; i < cursors.size(); i++)
		delete cursors[i].cursor;
}

WinCursorGroup *WinCursorGroup::createCursorGroup(Common::NEResources &exe, const Common::WinResourceID &id) {
	Common::SeekableReadStream *stream = exe.getResource(Common::kNEGroupCursor, id);

	if (!stream || stream->size() <= 6)
		return 0;

	stream->skip(4);
	uint32 cursorCount = stream->readUint16LE();
	if ((uint32)stream->size() < (6 + cursorCount * 16))
		return 0;

	WinCursorGroup *group = new WinCursorGroup();
	group->cursors.reserve(cursorCount);

	for (uint32 i = 0; i < cursorCount; i++) {
		stream->readUint16LE(); // width
		stream->readUint16LE(); // height

		// Plane count
		if (stream->readUint16LE() != 1) {
			delete stream;
			delete group;
			return 0;
		}

		// Bit count
		if (stream->readUint16LE() != 1) {
			delete stream;
			delete group;
			return 0;
		}

		stream->readUint32LE(); // data size
		uint32 cursorId = stream->readUint32LE();

		Common::SeekableReadStream *cursorStream = exe.getResource(Common::kNECursor, cursorId);
		if (!cursorStream) {
			delete stream;
			delete group;
			return 0;
		}

		WinCursor *cursor = new WinCursor();
		if (!cursor->readFromStream(*cursorStream)) {
			delete stream;
			delete cursorStream;
			delete cursor;
			delete group;
			return 0;
		}

		delete cursorStream;

		CursorItem item;
		item.id = cursorId;
		item.cursor = cursor;
		group->cursors.push_back(item);
	}

	delete stream;
	return group;
}

WinCursorGroup *WinCursorGroup::createCursorGroup(Common::PEResources &exe, const Common::WinResourceID &id) {
	Common::SeekableReadStream *stream = exe.getResource(Common::kPEGroupCursor, id);

	if (!stream || stream->size() <= 6)
		return 0;

	stream->skip(4);
	uint32 cursorCount = stream->readUint16LE();
	if ((uint32)stream->size() < (6 + cursorCount * 16))
		return 0;

	WinCursorGroup *group = new WinCursorGroup();
	group->cursors.reserve(cursorCount);

	for (uint32 i = 0; i < cursorCount; i++) {
		stream->readUint16LE(); // width
		stream->readUint16LE(); // height

		// Plane count
		if (stream->readUint16LE() != 1) {
			delete stream;
			delete group;
			return 0;
		}

		// Bit count
		if (stream->readUint16LE() != 1) {
			delete stream;
			delete group;
			return 0;
		}

		stream->readUint32LE(); // data size
		uint32 cursorId = stream->readUint32LE();

		Common::SeekableReadStream *cursorStream = exe.getResource(Common::kPECursor, cursorId);
		if (!cursorStream) {
			delete stream;
			delete group;
			return 0;
		}

		WinCursor *cursor = new WinCursor();
		if (!cursor->readFromStream(*cursorStream)) {
			delete stream;
			delete cursorStream;
			delete cursor;
			delete group;
			return 0;
		}

		delete cursorStream;

		CursorItem item;
		item.id = cursorId;
		item.cursor = cursor;
		group->cursors.push_back(item);
	}

	delete stream;
	return group;
}

} // End of namespace Graphics
