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
 * $URL$
 * $Id$
 */

#include "graphics/sjis.h"

#include "common/debug.h"

namespace Graphics {

bool FontTowns::loadFromStream(Common::ReadStream &stream) {
	for (uint i = 0; i < (kFontRomSize / 2); ++i)
		_fontData[i] = stream.readUint16BE();
	return !stream.err();
}

template<typename Color>
void FontTowns::drawCharInternShadow(const uint16 *glyph, uint8 *dst, int pitch, Color c1, Color c2) const {
	uint32 outlineGlyph[18];
	memset(outlineGlyph, 0, sizeof(outlineGlyph));

	// Create an outline map including the original character
	const uint16 *src = glyph;
	for (int i = 0; i < 16; ++i) {
		uint32 line = *src++;
		line = (line << 2) | (line << 1) | (line << 0);

		outlineGlyph[i + 0] |= line;
		outlineGlyph[i + 1] |= line;
		outlineGlyph[i + 2] |= line;
	}

	uint8 *dstLine = dst;
	for (int y = 0; y < 18; ++y) {
		Color *lineBuf = (Color *)dstLine;
		uint32 line = outlineGlyph[y];

		for (int x = 0; x < 18; ++x) {
			if (line & 0x20000)
				*lineBuf = c2;
			line <<= 1;
			++lineBuf;
		}

		dstLine += pitch;
	}

	// draw the original char
	drawCharIntern<Color>(glyph, dst + pitch + 1, pitch, c1);
}

template<typename Color>
void FontTowns::drawCharIntern(const uint16 *glyph, uint8 *dst, int pitch, Color c1) const {
	for (int y = 0; y < 16; ++y) {
		Color *lineBuf = (Color *)dst;
		uint16 line = *glyph++;

		for (int x = 0; x < 16; ++x) {
			if (line & 0x8000)
				*lineBuf = c1;
			line <<= 1;
			++lineBuf;
		}

		dst += pitch;
	}
}

void FontTowns::drawChar(void *dst, uint16 ch, int pitch, int bpp, uint32 c1, uint32 c2) const {
	const uint16 *glyphSource = _fontData + sjisToChunk(ch & 0xFF, ch >> 8) * 16;

	if (bpp == 1) {
		if (!_shadowEnabled)
			drawCharIntern<uint8>(glyphSource, (uint8 *)dst, pitch, c1);
		else
			drawCharInternShadow<uint8>(glyphSource, (uint8 *)dst, pitch, c1, c2);
	} else if (bpp == 2) {
		if (!_shadowEnabled)
			drawCharIntern<uint16>(glyphSource, (uint8 *)dst, pitch, c1);
		else
			drawCharInternShadow<uint16>(glyphSource, (uint8 *)dst, pitch, c1, c2);
	} else {
		error("FontTowns::drawChar: unsupported bpp: %d", bpp);
	}
}

uint FontTowns::sjisToChunk(uint8 f, uint8 s) {
	// copied from scumm\charset.cpp
	enum {
		KANA = 0,
		KANJI = 1,
		EKANJI = 2
	};

	int base = s - ((s + 1) % 32);
	int c = 0, p = 0, chunk_f = 0, chunk = 0, cr = 0, kanjiType = KANA;

	if (f >= 0x81 && f <= 0x84) kanjiType = KANA;
	if (f >= 0x88 && f <= 0x9f) kanjiType = KANJI;
	if (f >= 0xe0 && f <= 0xea) kanjiType = EKANJI;

	if ((f > 0xe8 || (f == 0xe8 && base >= 0x9f)) || (f > 0x90 || (f == 0x90 && base >= 0x9f))) {
		c = 48; //correction
		p = -8; //correction
	}

	if (kanjiType == KANA) {//Kana
		chunk_f = (f - 0x81) * 2;
	} else if (kanjiType == KANJI) {//Standard Kanji
		p += f - 0x88;
		chunk_f = c + 2 * p;
	} else if (kanjiType == EKANJI) {//Enhanced Kanji
		p += f - 0xe0;
		chunk_f = c + 2 * p;
	}

	// Base corrections
	if (base == 0x7f && s == 0x7f)
		base -= 0x20;
	if (base == 0x9f && s == 0xbe)
		base += 0x20;
	if (base == 0xbf && s == 0xde)
		base += 0x20;
	//if (base == 0x7f && s == 0x9e)
	//	base += 0x20;

	switch (base) {
	case 0x3f:
		cr = 0; //3f
		if (kanjiType == KANA) chunk = 1;
		else if (kanjiType == KANJI) chunk = 31;
		else if (kanjiType == EKANJI) chunk = 111;
		break;
	case 0x5f:
		cr = 0; //5f
		if (kanjiType == KANA) chunk = 17;
		else if (kanjiType == KANJI) chunk = 47;
		else if (kanjiType == EKANJI) chunk = 127;
		break;
	case 0x7f:
		cr = -1; //80
		if (kanjiType == KANA) chunk = 9;
		else if (kanjiType == KANJI) chunk = 63;
		else if (kanjiType == EKANJI) chunk = 143;
		break;
	case 0x9f:
		cr = 1; //9e
		if (kanjiType == KANA) chunk = 2;
		else if (kanjiType == KANJI) chunk = 32;
		else if (kanjiType == EKANJI) chunk = 112;
		break;
	case 0xbf:
		cr = 1; //be
		if (kanjiType == KANA) chunk = 18;
		else if (kanjiType == KANJI) chunk = 48;
		else if (kanjiType == EKANJI) chunk = 128;
		break;
	case 0xdf:
		cr = 1; //de
		if (kanjiType == KANA) chunk = 10;
		else if (kanjiType == KANJI) chunk = 64;
		else if (kanjiType == EKANJI) chunk = 144;
		break;
	default:
		debug(4, "Invalid Char! f %x s %x base %x c %d p %d", f, s, base, c, p);
	}

	debug(6, "Kanji: %c%c f 0x%x s 0x%x base 0x%x c %d p %d chunk %d cr %d index %d", f, s, f, s, base, c, p, chunk, cr, ((chunk_f + chunk) * 32 + (s - base)) + cr);
	return ((chunk_f + chunk) * 32 + (s - base)) + cr;
}

} // end of namespace Graphics

