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

#ifdef GRAPHICS_SJIS_H

#include "common/debug.h"
#include "common/archive.h"
#include "common/endian.h"

namespace Graphics {

FontSJIS *FontSJIS::createFont(const Common::Platform platform) {
	FontSJIS *ret = 0;

	// Try the font ROM of the specified platform
	if (platform == Common::kPlatformFMTowns) {
		ret = new FontTowns();
		if (ret && ret->loadData())
			return ret;
		delete ret;
	}

	// Try ScummVM's font.
	ret = new FontSjisSVM();
	if (ret && ret->loadData())
		return ret;
	delete ret;

	return 0;
}

template<typename Color>
void FontSJISBase::blitCharacter(const uint8 *glyph, const int w, const int h, uint8 *dst, int pitch, Color c) const {
	for (int y = 0; y < h; ++y) {
		Color *d = (Color *)dst;
		dst += pitch;

		uint8 mask = 0;
		for (int x = 0; x < w; ++x) {
			if (!(x % 8))
				mask = *glyph++;

			if (mask & 0x80)
				*d = c;
			++d;
			mask <<= 1;
		}
	}
}

void FontSJISBase::createOutline(uint8 *outline, const uint8 *glyph, const int w, const int h) const {
	const int glyphPitch = (w + 7) / 8;
	const int outlinePitch = (w + 9) / 8;

	uint8 *line1 = outline + 0 * outlinePitch;
	uint8 *line2 = outline + 1 * outlinePitch;
	uint8 *line3 = outline + 2 * outlinePitch;

	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < glyphPitch; ++x) {
			const uint8 mask = *glyph++;

			const uint8 b1 = mask | (mask >> 1) | (mask >> 2);
			const uint8 b2 = (mask << 7) | ((mask << 6) & 0xC0);

			line1[x] |= b1;
			line2[x] |= b1;
			line3[x] |= b1;

			if (x + 1 < outlinePitch) {
				line1[x + 1] |= b2;
				line2[x + 1] |= b2;
				line3[x + 1] |= b2;
			}
		}

		line1 += outlinePitch;
		line2 += outlinePitch;
		line3 += outlinePitch;
	}
}

void FontSJISBase::drawChar(void *dst, uint16 ch, int pitch, int bpp, uint32 c1, uint32 c2) const {
	const uint8 *glyphSource = 0;
	int width = 0, height = 0;
	if (is8x16(ch)) {
		glyphSource = getCharData8x16(ch);
		width = 8;
		height = 16;
	} else {
		glyphSource = getCharData(ch);
		width = 16;
		height = 16;
	}

	if (!glyphSource) {
		warning("FontSJISBase::drawChar: Font does not offer data for %02X %02X", ch & 0xFF, ch >> 8);
		return;
	}

	uint8 outline[18 * 18];
	if (_outlineEnabled) {
		memset(outline, 0, sizeof(outline));
		createOutline(outline, glyphSource, width, height);
	}

	if (bpp == 1) {
		if (_outlineEnabled) {
			blitCharacter<uint8>(outline, width + 2, height + 2, (uint8 *)dst, pitch, c2);
			blitCharacter<uint8>(glyphSource, width, height, (uint8 *)dst + pitch + 1, pitch, c1);
		} else {
			blitCharacter<uint8>(glyphSource, width, height, (uint8 *)dst, pitch, c1);
		}
	} else if (bpp == 2) {
		if (_outlineEnabled) {
			blitCharacter<uint16>(outline, width + 2, height + 2, (uint8 *)dst, pitch, c2);
			blitCharacter<uint16>(glyphSource, width, height, (uint8 *)dst + pitch + 2, pitch, c1);
		} else {
			blitCharacter<uint16>(glyphSource, width, height, (uint8 *)dst, pitch, c1);
		}
	} else {
		error("FontSJISBase::drawChar: unsupported bpp: %d", bpp);
	}
}

uint FontSJISBase::getCharWidth(uint16 ch) const {
	if (is8x16(ch))
		return _outlineEnabled ? 10 : 8;
	else
		return getMaxFontWidth();
}

bool FontSJISBase::is8x16(uint16 ch) const {
	if (ch >= 0xFF)
		return false;
	else if (ch <= 0x7F || (ch >= 0xA1 && ch <= 0xDF))
		return true;
	else
		return false;
}

// FM-TOWNS ROM font

bool FontTowns::loadData() {
	Common::SeekableReadStream *data = SearchMan.createReadStreamForMember("FMT_FNT.ROM");
	if (!data)
		return false;

	data->read(_fontData16x16, kFont16x16Chars * 32);
	data->seek(251904, SEEK_SET);
	data->read(_fontData8x16, kFont8x16Chars * 16);

	bool retValue = !data->err();
	delete data;
	return retValue;
}

const uint8 *FontTowns::getCharData(uint16 ch) const {
	uint8 f = ch & 0xFF;
	uint8 s = ch >> 8;

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
	const int chunkNum = (((chunk_f + chunk) * 32 + (s - base)) + cr);
	if (chunkNum < 0 || chunkNum >= kFont16x16Chars)
		return 0;
	else
		return _fontData16x16 + chunkNum * 32;
}

const uint8 *FontTowns::getCharData8x16(uint16 c) const {
	if (c >= kFont8x16Chars)
		return 0;
	return _fontData8x16 + c * 16;
}

// ScummVM SJIS font

bool FontSjisSVM::loadData() {
	Common::SeekableReadStream *data = SearchMan.createReadStreamForMember("SJIS.FNT");
	if (!data)
		return false;
	
	uint32 magic1 = data->readUint32BE();
	uint32 magic2 = data->readUint32BE();

	if (magic1 != MKID_BE('SCVM') || magic2 != MKID_BE('SJIS')) {
		delete data;
		return false;
	}

	uint32 version = data->readUint32BE();
	if (version != 2) {
		delete data;
		return false;
	}
	uint numChars16x16 = data->readUint16BE();
	uint numChars8x16 = data->readUint16BE();

	_fontData16x16Size = numChars16x16 * 32;
	_fontData16x16 = new uint8[_fontData16x16Size];
	assert(_fontData16x16);
	data->read(_fontData16x16, _fontData16x16Size);

	_fontData8x16Size = numChars8x16 * 16;
	_fontData8x16 = new uint8[numChars8x16 * 16];
	assert(_fontData8x16);

	data->read(_fontData8x16, _fontData8x16Size);

	bool retValue = !data->err();
	delete data;
	return retValue;
}
 
const uint8 *FontSjisSVM::getCharData(uint16 c) const {
	const uint8 fB = c & 0xFF;
	const uint8 sB = c >> 8;

	// We only allow 2 byte SJIS characters.
	if (fB <= 0x80 || fB >= 0xF0 || (fB >= 0xA0 && fB <= 0xDF) || sB == 0x7F)
		return 0;

	int base = fB;
	base -= 0x81;
	if (base >= 0x5F)
		base -= 0x40;

	int index = sB;
	index -= 0x40;
	if (index >= 0x3F)
		--index;

	// Another check if the passed character was an
	// correctly encoded SJIS character.
	if (index < 0 || index >= 0xBC || base < 0)
		return 0;

	const uint offset = (base * 0xBC + index) * 32;
	assert(offset + 16 <= _fontData16x16Size);
	return _fontData16x16 + offset;
}

const uint8 *FontSjisSVM::getCharData8x16(uint16 c) const {
	const uint8 fB = c & 0xFF;
	const uint8 sB = c >> 8;

	if (!is8x16(c) || sB)
		return 0;

	int index = fB;

	// half-width katakana
	if (fB >= 0xA1 && fB <= 0xDF)
		index -= 0x21;

	const uint offset = index * 16;
	assert(offset <= _fontData8x16Size);
	return _fontData8x16 + offset;
}

} // End of namespace Graphics

#endif // defined(GRAPHICS_SJIS_H)

