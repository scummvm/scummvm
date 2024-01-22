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
				font = FFont::load(*stream);
			else
				font = PFont::load(*stream, decompressor);
		}
	}

	delete fontFile;

	return font;
}

Font::Font(byte w, byte h, byte start, byte count, const byte *glyphs) : _w(w), _h(h), _start(start), _count(count), _glyphs(glyphs) { }

Font::~Font() {
}

bool Font::hasChar(byte chr) const {
	return (chr >= _start && chr <= (_start + _count));
}

static inline uint isSet(const byte *set, uint bit) {
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
	const byte *src = _glyphs + pos + croppedBy.y;
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

FFont::FFont(byte w, byte h, byte start, byte count, byte *data) : Font(w, h, start, count, data), _rawData(data) {
}

FFont::~FFont() {
	delete [] _rawData;
}
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

	byte *data = new byte[size];
	input.read(data, size);

	return new FFont(w, h, start, count, data);
}

PFont::PFont(byte w, byte h, byte start, byte count, byte *data)
: Font(w, h, start, count, data + 3 * count), _offsets(reinterpret_cast<const uint16 *>(data)), _widths(data + 2 * count), _rawData(data)
{
}

PFont::~PFont() {
	delete [] _rawData;
}

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

int PFont::getCharWidth(uint32 chr) const {
	if (!hasChar(chr))
		return 0;
	return _widths[chr - _start];
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

	return new PFont(w, h, start, count, data);
}

FontManager::~FontManager() {
	for (auto &entry : _fonts)
		if (entry._key != kDefaultFont)
			delete entry._value;
}

const Font *FontManager::getFont(FontType type) const {
	return _fonts.getVal(type);
}

void FontManager::tryLoadFont(FontType ftype, const char *fname, ResourceManager *resMgr, Decompressor *decomp) {
	Font *font = Font::load(fname, resMgr, decomp);
	if (font)
		_fonts.setVal(ftype, font);
	else
		error("Failed to load font %s", fname);
}


void FontManager::loadFonts(DgdsGameId gameId, ResourceManager *resMgr, Decompressor *decomp) {
	tryLoadFont(k8x8Font, "8X8.FNT", resMgr, decomp);
	tryLoadFont(k6x6Font, "6X6.FNT", resMgr, decomp);
	tryLoadFont(k4x5Font, "4x5.FNT", resMgr, decomp);
	if (gameId == GID_DRAGON) {
		tryLoadFont(kGameFont, "DRAGON.FNT", resMgr, decomp);
		tryLoadFont(k7x8Font, "7X8.FNT", resMgr, decomp);
		tryLoadFont(kGameDlgFont, "P6X6.FNT", resMgr, decomp);
	} else if (gameId == GID_CHINA) {
		tryLoadFont(kGameFont, "HOC.FNT", resMgr, decomp);
		tryLoadFont(kChinaFont, "CHINA.FNT", resMgr, decomp);
		tryLoadFont(kGameDlgFont, "CHINESE.FNT", resMgr, decomp);
	} else if (gameId == GID_BEAMISH) {
		tryLoadFont(kGameFont, "WILLY.FNT", resMgr, decomp);
		tryLoadFont(kWVCRFont, "WVCR.FNT", resMgr, decomp);
		tryLoadFont(kGameDlgFont, "COMIX_16.FNT", resMgr, decomp);
	}

	_fonts.setVal(kDefaultFont, _fonts.getVal(kGameFont));
}

} // End of namespace Dgds
