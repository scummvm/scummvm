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

#include "graphics/font.h"
#include "graphics/surface.h"
#include "common/stream.h"

#include "dgds/decompress.h"
#include "dgds/font.h"

namespace Dgds {

bool Font::hasChar(byte chr) const {
	return (chr >= _start && chr <= (_start+_count));
}

static inline uint isSet(byte *set, uint bit) {
	return (set[(bit >> 3)] & (1 << (bit & 7)));
}

void Font::drawChar(Graphics::Surface* dst, int pos, int bit, int x, int y, uint32 color) const {
	const Common::Rect destRect(x, y, x+_w, y+_h);
	Common::Rect clippedDestRect(0, 0, dst->w, dst->h);
	clippedDestRect.clip(destRect);

	const Common::Point croppedBy(clippedDestRect.left-destRect.left, clippedDestRect.top-destRect.top);

	const int rows = clippedDestRect.height();
	const int columns = clippedDestRect.width();

	int idx = bit + croppedBy.x;
	byte *src = _data + pos + croppedBy.y;
	byte *ptr = (byte *)dst->getBasePtr(clippedDestRect.left, clippedDestRect.top);
	for (int i=0; i<rows; ++i) {
		for (int j=0; j<columns; ++j) {
			if (isSet(src, idx+_w-1-j))
				ptr[j] = color;
		}
		ptr += dst->pitch;
		src++;
	}
}

void FFont::mapChar(byte chr, int& pos, int& bit) const {
	pos = (chr-_start)*_h;
	bit = 8-_w;
}

void FFont::drawChar(Graphics::Surface* dst, uint32 chr, int x, int y, uint32 color) const {
	if (!hasChar(chr)) return;

	int pos, bit;
	mapChar(chr, pos, bit);
	Font::drawChar(dst, pos, bit, x, y, color);
}

FFont *FFont::load(Common::SeekableReadStream &input) {
	byte w, h, start, count;
	w = input.readByte();
	h = input.readByte();
	start = input.readByte();
	count = input.readByte();

	int size = h*count;
	debug("    w: %u, h: %u, start: 0x%x, count: %u", w, h, start, count);
	assert((4+size) == input.size());

	FFont* fnt = new FFont;
	fnt->_w = w;
	fnt->_h = h;
	fnt->_start = start;
	fnt->_count = count;
	fnt->_data = new byte[size];
	input.read(fnt->_data, size);
	return fnt;
}

void PFont::mapChar(byte chr, int& pos, int& bit) const {
	pos = READ_LE_UINT16(&_offsets[chr-_start]);
	bit = 0;
}

void PFont::drawChar(Graphics::Surface* dst, uint32 chr, int x, int y, uint32 color) const {
	if (!hasChar(chr)) return;

	int pos, bit;
	mapChar(chr, pos, bit);
	Font::drawChar(dst, pos, bit, x, y, color);
}

PFont *PFont::load(Common::SeekableReadStream &input, Decompressor *decompressor) {
	byte magic;

	magic = input.readByte();
	assert(magic == 0xFF);

	byte w, h;
	byte unknown, start, count, compression;
	int size;
	int uncompressedSize;

	w = input.readByte();
	h = input.readByte();
	unknown = input.readByte();
	start = input.readByte();
	count = input.readByte();
	size = input.readUint16LE();
	compression = input.readByte();
	uncompressedSize = input.readUint32LE();
	debug("    magic: 0x%x, w: %u, h: %u, unknown: %u, start: 0x%x, count: %u\n"
	      "    size: %u, compression: 0x%x, uncompressedSize: %u",
			magic, w, h, unknown, start, count,
			size, compression, uncompressedSize);
	assert(uncompressedSize == size);

	size = input.size()-input.pos();

	byte *data = new byte[uncompressedSize];
	decompressor->decompress(compression, data, uncompressedSize, &input, size);

	PFont* fnt = new PFont;
	fnt->_w = w;
	fnt->_h = h;
	fnt->_start = start;
	fnt->_count = count;

	fnt->_offsets = (uint16*)data;
	fnt->_widths = data+2*count;
	fnt->_data = data+3*count;
	return fnt;
}

} // End of namespace Dgds
