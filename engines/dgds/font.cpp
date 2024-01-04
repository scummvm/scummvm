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
#include "dgds/includes.h"
#include "dgds/resource.h"

namespace Dgds {

Font *Font::load(const Common::String &filename, ResourceManager *resourceManager, Decompressor *decompressor) {
	Common::SeekableReadStream *fontFile = resourceManager->getResource(filename);
	if (!fontFile)
		error("Font file %s not found", filename.c_str());

	DgdsChunkReader chunk(fontFile);

	Font *font = nullptr;

	while (chunk.readNextHeader(EX_FNT, filename)) {
		if (chunk.isContainer()) {
			continue;
		}

		chunk.readContent(decompressor);
		Common::SeekableReadStream *stream = chunk.getContent();

		if (chunk.isSection(ID_FNT)) {
			byte magic = stream->readByte();
			stream->seek(-1, SEEK_CUR);
			debug("    magic: %u", magic);

			if (magic != 0xFF)
				// font = FFont::load(*stream);	// TODO: Where is this used?
				error("Font::load(): Attempted to load a font of type FFont");
			else
				font = PFont::load(*stream, decompressor);
		}
	}

	delete fontFile;

	return font;
}

bool Font::hasChar(byte chr) const {
	return (chr >= _start && chr <= (_start + _count));
}

static inline uint isSet(byte *set, uint bit) {
	return (set[bit >> 3] & (1 << (bit & 7)));
}

void Font::drawChar(Graphics::Surface* dst, int pos, int bit, int x, int y, uint32 color) const {
	const Common::Rect destRect(x, y, x + _w, y + _h);
	Common::Rect clippedDestRect(0, 0, dst->w, dst->h);
	clippedDestRect.clip(destRect);

	const Common::Point croppedBy(clippedDestRect.left-destRect.left, clippedDestRect.top-destRect.top);

	const int rows = clippedDestRect.height();
	const int columns = clippedDestRect.width();

	int idx = bit + croppedBy.x;
	byte *src = _data + pos + croppedBy.y;
	byte *ptr = (byte *)dst->getBasePtr(clippedDestRect.left, clippedDestRect.top);

	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < columns; ++j) {
			if (isSet(src, idx + _w - 1 - j))
				ptr[j] = color;
		}
		ptr += dst->pitch;
		src++;
	}
}

#if 0
void FFont::mapChar(byte chr, int &pos, int &bit) const {
	pos = (chr - _start) * _h;
	bit = 8 - _w;
}

void FFont::drawChar(Graphics::Surface* dst, uint32 chr, int x, int y, uint32 color) const {
	if (!hasChar(chr))
		return;

	int pos, bit;
	mapChar(chr, pos, bit);
	Font::drawChar(dst, pos, bit, x, y, color);
}

FFont *FFont::load(Common::SeekableReadStream &input) {
	byte w = input.readByte();
	byte h = input.readByte();
	byte start = input.readByte();
	byte count = input.readByte();
	int size = h * count;

	assert((4 + size) == input.size());

	debug("    w: %u, h: %u, start: 0x%x, count: %u", w, h, start, count);

	FFont *fnt = new FFont;
	fnt->_w = w;
	fnt->_h = h;
	fnt->_start = start;
	fnt->_count = count;
	fnt->_data = new byte[size];
	input.read(fnt->_data, size);

	return fnt;
}
#endif

void PFont::mapChar(byte chr, int& pos, int& bit) const {
	pos = READ_LE_UINT16(&_offsets[chr - _start]);
	bit = 0;
}

void PFont::drawChar(Graphics::Surface* dst, uint32 chr, int x, int y, uint32 color) const {
	if (!hasChar(chr))
		return;

	int pos, bit;
	mapChar(chr, pos, bit);
	Font::drawChar(dst, pos, bit, x, y, color);
}

PFont *PFont::load(Common::SeekableReadStream &input, Decompressor *decompressor) {
	byte magic = input.readByte();
	byte w = input.readByte();
	byte h = input.readByte();
	byte unknown = input.readByte();
	byte start = input.readByte();
	byte count = input.readByte();
	int size = input.readUint16LE();

	debug("    magic: 0x%x, w: %u, h: %u, unknown: %u, start: 0x%x, count: %u\n"
	      "    size: %u",
			magic, w, h, unknown, start, count,
			size);

	assert(magic == 0xFF);

	size = input.size() - input.pos();

	uint32 uncompressedSize;
	byte *data = decompressor->decompress(&input, size, uncompressedSize);

	PFont *fnt = new PFont;
	fnt->_w = w;
	fnt->_h = h;
	fnt->_start = start;
	fnt->_count = count;

	fnt->_offsets = (uint16*)data;
	fnt->_widths = data + 2 * count;
	fnt->_data = data + 3 * count;

	return fnt;
}

} // End of namespace Dgds
