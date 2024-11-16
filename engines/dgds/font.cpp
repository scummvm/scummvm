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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

DgdsFont *DgdsFont::load(const Common::String &filename, ResourceManager *resourceManager, Decompressor *decompressor) {
	Common::SeekableReadStream *fontFile = resourceManager->getResource(filename);
	if (!fontFile) {
		warning("Font file %s not found", filename.c_str());
		return nullptr;
	}

	DgdsChunkReader chunk(fontFile);

	DgdsFont *font = nullptr;

	while (chunk.readNextHeader(EX_FNT, filename)) {
		if (chunk.isContainer()) {
			continue;
		}

		chunk.readContent(decompressor);
		Common::SeekableReadStream *stream = chunk.getContent();

		if (chunk.isSection(ID_FNT)) {
			byte magic = stream->readByte();
			stream->seek(-1, SEEK_CUR);
			debug(1, "    magic: %u", magic);

			if (magic != 0xFF)
				font = FFont::load(*stream);
			else
				font = PFont::load(*stream, decompressor);
		}
	}

	delete fontFile;

	return font;
}

DgdsFont::DgdsFont(byte w, byte h, byte start, byte count, const byte *glyphs) : _w(w), _h(h), _start(start), _count(count), _glyphs(glyphs) { }

DgdsFont::~DgdsFont() {
}

bool DgdsFont::hasChar(byte chr) const {
	return (chr >= _start && chr <= (_start + _count));
}

static inline bool isSet(const byte *data, uint bit) {
	return data[bit / 8] & (1 << (7 - (bit % 8)));
}

void DgdsFont::drawDgdsChar(Graphics::Surface* dst, int pos, int x, int y, int w, uint32 color) const {
	const Common::Rect destRect(Common::Point(x, y), w, _h);
	Common::Rect clippedDestRect(dst->w, dst->h);
	clippedDestRect.clip(destRect);

	const Common::Point croppedBy(clippedDestRect.left - destRect.left, clippedDestRect.top - destRect.top);

	const int rows = clippedDestRect.height();
	const int columns = clippedDestRect.width();

	int bitOffset = croppedBy.x;
	int bytesPerRow = (w + 7) / 8;
	const byte *src = _glyphs + pos + (croppedBy.y * bytesPerRow);
	byte *ptr = (byte *)dst->getBasePtr(clippedDestRect.left, clippedDestRect.top);

	for (int i = 0; i < rows; ++i) {
		for (int j = 0; j < columns; ++j) {
			if (isSet(src, bitOffset + j))
				ptr[j] = color;
		}
		ptr += dst->pitch;
		src += bytesPerRow;
	}
}

FFont::FFont(byte w, byte h, byte start, byte count, byte *data) : DgdsFont(w, h, start, count, data), _rawData(data) {
}

FFont::~FFont() {
	delete [] _rawData;
}


int FFont::charOffset(byte chr) const {
	return (chr - _start) * _h;
}

void FFont::drawChar(Graphics::Surface* dst, uint32 chr, int x, int y, uint32 color) const {
	if (!hasChar(chr))
		return;

	int pos = charOffset(chr);
	drawDgdsChar(dst, pos, x, y, _w, color);
}

FFont *FFont::load(Common::SeekableReadStream &input) {
	byte w = input.readByte();
	byte h = input.readByte();
	byte start = input.readByte();
	byte count = input.readByte();
	int size = h * count;

	assert((4 + size) == input.size());

	debug(1, "FFont w: %u, h: %u, start: 0x%x, count: %u", w, h, start, count);

	byte *data = new byte[size];
	input.read(data, size);

	return new FFont(w, h, start, count, data);
}

PFont::PFont(byte w, byte h, byte start, byte count, byte *data)
: DgdsFont(w, h, start, count, data + 3 * count), _offsets(reinterpret_cast<const uint16 *>(data)), _widths(data + 2 * count), _rawData(data)
{
}

PFont::~PFont() {
	delete [] _rawData;
}

int PFont::charOffset(byte chr) const {
	return READ_LE_UINT16(&_offsets[chr - _start]);
}

void PFont::drawChar(Graphics::Surface* dst, uint32 chr, int x, int y, uint32 color) const {
	if (!hasChar(chr))
		return;

	int pos = charOffset(chr);
	int w = getCharWidth(chr);
	drawDgdsChar(dst, pos, x, y, w, color);
}

int PFont::getCharWidth(uint32 chr) const {
	if (!hasChar(chr))
		return 0;
	return _widths[(byte)chr - _start];
}

PFont *PFont::load(Common::SeekableReadStream &input, Decompressor *decompressor) {
	byte magic = input.readByte();
	byte w = input.readByte();
	byte h = input.readByte();
	byte unknown = input.readByte();
	byte start = input.readByte();
	byte count = input.readByte();
	int size = input.readUint16LE();

	debug(1, "PFont magic: 0x%x, w: %u, h: %u, unk: %u, start: 0x%x, count: %u, size: %u",
			magic, w, h, unknown, start, count, size);

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

const DgdsFont *FontManager::getFont(FontType type) const {
	return _fonts.getVal(type);
}

void FontManager::tryLoadFont(const char *fname, ResourceManager *resMgr, Decompressor *decomp) {
	FontType ftype = fontTypeByName(fname);
	DgdsFont *font = DgdsFont::load(fname, resMgr, decomp);
	if (font)
		_fonts.setVal(ftype, font);
	else
		warning("Failed to load font %s", fname);
}

FontManager::FontType FontManager::fontTypeByName(const Common::String &filename) const {
	if (filename == "8X8.FNT") return k8x8Font;
	if (filename == "6X6.FNT") return k6x6Font;
	if (filename == "4x5.FNT") return k4x5Font;
	if (filename == "DRAGON.FNT") return kGameFont;
	if (filename == "7X8.FNT") return k7x8Font;
	if (filename == "P6X6.FNT") return kGameDlgFont;
	if (filename == "HOC.FNT") return kGameFont;
	if (filename == "CHINA.FNT") return kGameDlgFont;
	if (filename == "CHINESE.FNT") return kChinaFont;
	if (filename == "WILLY.FNT") return kGameFont;
	if (filename == "COMIX_16.FNT") return kGameDlgFont;
	if (filename == "WVCR.FNT") return kVCRFont;
	if (filename == "EXIT.FNT") return kVCRFont;
	if (filename == "SSM1_12.FNT") return kGameFont;
	if (filename == "SSM1_15.FNT") return kGameDlgFont;
	if (filename == "SSM1_30.FNT") return k8x8Font;
	if (filename == "RMN7_19.FNT") return kGameDlgFont;
	if (filename == "RMN8_11.FNT") return kGameFont;
	return FontManager::kDefaultFont;
}


void FontManager::loadFonts(DgdsGameId gameId, ResourceManager *resMgr, Decompressor *decomp) {
	if (gameId == GID_CASTAWAY)
		return; // no fonts

	if (gameId == GID_SQ5DEMO) {
		tryLoadFont("SSM1_12.FNT", resMgr, decomp);
		tryLoadFont("SSM1_15.FNT", resMgr, decomp);
		tryLoadFont("SSM1_30.FNT", resMgr, decomp);
		tryLoadFont("EXIT.FNT", resMgr, decomp);
	} else if (gameId == GID_COMINGATTRACTIONS) {
		tryLoadFont("RMN8_11.FNT", resMgr, decomp);
		tryLoadFont("RMN7_19.FNT", resMgr, decomp);
		tryLoadFont("EXIT.FNT", resMgr, decomp);
		_fonts.setVal(kDefaultFont, _fonts.getVal(kGameDlgFont));
		return;
	} else {
		tryLoadFont("8X8.FNT", resMgr, decomp);
		tryLoadFont("6X6.FNT", resMgr, decomp);
		tryLoadFont("4x5.FNT", resMgr, decomp);
		if (gameId == GID_DRAGON) {
			tryLoadFont("DRAGON.FNT", resMgr, decomp);
			tryLoadFont("7X8.FNT", resMgr, decomp);
			tryLoadFont("P6X6.FNT", resMgr, decomp);
		} else if (gameId == GID_HOC) {
			tryLoadFont("HOC.FNT", resMgr, decomp);
			tryLoadFont("CHINA.FNT", resMgr, decomp);
			tryLoadFont("CHINESE.FNT", resMgr, decomp);
		} else if (gameId == GID_WILLY) {
			tryLoadFont("WILLY.FNT", resMgr, decomp);
			tryLoadFont("WVCR.FNT", resMgr, decomp);
			tryLoadFont("COMIX_16.FNT", resMgr, decomp);
		} else if (gameId == GID_QUARKY) {
			tryLoadFont("MRALLY.FNT", resMgr, decomp);
			tryLoadFont("MVCR.FNT", resMgr, decomp);
			tryLoadFont("RUDEFONT.FNT", resMgr, decomp);
			_fonts.setVal(kDefaultFont, _fonts.getVal(k8x8Font));
			return;
		}
	}

	_fonts.setVal(kDefaultFont, _fonts.getVal(kGameFont));
}

} // End of namespace Dgds
