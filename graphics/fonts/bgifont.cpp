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

#include "graphics/fonts/bgifont.h"

namespace Graphics {

BgiFont::BgiFont() {
}

BgiFont::~BgiFont() {
}

bool BgiFont::loadChr(const Common::Path &fileName) {

	Common::File fontFile;
	if (!fontFile.open(fileName)) {
		error("unable to load font file %s", fileName.toString().c_str());
	}
	return loadChr(fontFile);
}

bool BgiFont::loadChr(Common::SeekableReadStream &stream) {
	/* fileSignature = */ stream.readUint16LE();
	/*
		Description until finding value 0x1A
	*/
	Common::String description = stream.readString(0x1A);
	uint16 headerSize = stream.readUint16LE();
	Common::String name = stream.readString(0, 4);
	/*uint16 fontSize = */ stream.readUint16LE();
	/* byte majorVersion = */ stream.readByte();
	/* byte minorVersion = */ stream.readByte();
	/* byte majorRevision = */ stream.readByte();
	/* byte minorRevision = */ stream.readByte();

	int remainingBytes = headerSize - (description.size() + 1 + 14);
	stream.seek(remainingBytes, SEEK_CUR);

	/* char signature = */ stream.readByte();
	_charCount = stream.readUint16LE();
	// undefined byte
	stream.skip(1);

	_firstChar = stream.readByte();
	/* uint16 strokeOffset = */ stream.readUint16LE();
	/*byte scanFlag = */ stream.readByte();

	// Distance from the origin to the font's highest point
	_originToAscender = stream.readByte();
	// Distance from the origin to the font's baseline (typically 0). Ignored.
	/*signed char originToBaseline = */ stream.readByte();
	// Distance from the origin to the font's lowest point.
	_originToDescender = stream.readByte();
	/**
	 * ----------- originToAscender
	 *
	 *
	 *
	 * ----------- 0
	 *
	 * ----------- originToDescender
	 * totalHeight is the distance between originToAscender and originToDescender plus 1 (the baseline or the zero)
	 */
	_totalHeight = (_originToAscender - _originToDescender) + 1;
	// Unused bytes
	stream.skip(5);

	_glyphs = new GlyphEntry[_charCount];

	// Glyph offsets
	for (int i = 0; i < _charCount; i++) {
		_glyphs[i].offset = stream.readUint16LE();
	}
	_maxWidth = 0;
	// Glyph widths
	for (int i = 0; i < _charCount; i++) {
		_glyphs[i].charWidth = stream.readByte();
		if (_maxWidth < _glyphs[i].charWidth)
			_maxWidth = _glyphs[i].charWidth;
	}
	int64 pos = stream.pos();

	// Read drawing instructions until next glyph definition
	for (int i = 0; i < _charCount; i++) {
		_totalWidth += _glyphs[i].charWidth;
		stream.seek(pos + _glyphs[i].offset, SEEK_SET);
		int m;
		do {
			DrawingInstruction *inst = new DrawingInstruction();
			byte instructionX = stream.readByte();
			byte instructionY = stream.readByte();
			// Grabs the most significant bit which is the opcode
			m = instructionX >> 7 & 0x1;
			m += m + (instructionY >> 7 & 0x1);
			instructionX = fixSign(instructionX);
			instructionY = fixSign(instructionY);

			inst->opCode = m;
			inst->xCoord = instructionX;
			inst->yCoord = instructionY;
			_glyphs[i].insts.push_back(inst);

		} while (m);
	}
	_fontCache.push_back(drawCachedFont(1));
	return false;
}

byte Graphics::BgiFont::fixSign(byte original) {
	// If negative shifts the sign bit to the right position
	return (original & 0x7F) | ((original & 0x40) << 1);
}

BgiFont::CachedFont *BgiFont::drawCachedFont(int size) {
	CachedFont *cachedFont = new CachedFont();
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(_totalWidth, _totalHeight, Graphics::PixelFormat::createFormatCLUT8());
	uint32 offsetCount = 0;
	for (int i = 0; i < _charCount; i++) {

		int curPosX = offsetCount;
		int curPosY = 0;
		cachedFont->offsets[i] = offsetCount;
		cachedFont->widths[i] = _glyphs[i].charWidth;

		for (int j = 0; j < _glyphs[i].insts.size(); j++) {
			int opCode = _glyphs[i].insts[j]->opCode;
			// Need to normalize Y coord because the stroke instructions start at origin and extend upwards up to originAscender, downwards to originToDescender
			int adjustedY = _originToAscender - _glyphs[i].insts[j]->yCoord;

			switch (opCode) {
			case OPCODE_END:
				break;
			case OPCODE_MOVE:
				break;
			case OPCODE_DRAW:
				surface->drawLine(
					curPosX,
					curPosY,
					offsetCount + _glyphs[i].insts[j]->xCoord,
					adjustedY,
					255);
				break;
			default:
				/* nothing to do */
				break;
			};
			curPosX = offsetCount + _glyphs[i].insts[j]->xCoord;
			curPosY = adjustedY;
		}
		offsetCount += _glyphs[i].charWidth;
	}

	cachedFont->surface = surface;
	return cachedFont;
}

void Graphics::BgiFont::close() {
}

int BgiFont::getCharWidth(uint32 chr) const {
	return _glyphs[characterToIndex(chr)].charWidth;
}

void BgiFont::drawChar(Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	CachedFont *font = _fontCache[0];
	uint16 charIndex = characterToIndex(chr);
	int charWidth = font->widths[charIndex];

	for (uint16 i = 0; i < _totalHeight; i++) {
		for (uint16 j = 0; j < charWidth; j++) {
			if (font->surface->getPixel(font->offsets[charIndex] + j, i)) {
				if (dst->format.bytesPerPixel == 1)
					*((byte *)dst->getBasePtr(x + j, y + i)) = color;
				else if (dst->format.bytesPerPixel == 2)
					*((uint16 *)dst->getBasePtr(x + j, y + i)) = color;
				else if (dst->format.bytesPerPixel == 4)
					*((uint32 *)dst->getBasePtr(x + j, y + i)) = color;
			}
		}
	}
}

uint16 BgiFont::characterToIndex(uint32 character) const {
	if (character - _firstChar >= 0 && character - _firstChar < _charCount) {
		return character - _firstChar;
	} else
		return _firstChar;
}

} // End of namespace Graphics
