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
 */

#include "common/scummsys.h"
#include "graphics/sjis.h"

#ifdef GRAPHICS_SJIS_H

#include "common/debug.h"
#include "common/archive.h"
#include "common/endian.h"
#include "common/stream.h"
#include "common/textconsole.h"

#include "graphics/surface.h"

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

void FontSJIS::drawChar(Graphics::Surface &dst, uint16 ch, int x, int y, uint32 c1, uint32 c2) const {
	drawChar(dst.getBasePtr(x, y), ch, dst.pitch, dst.format.bytesPerPixel, c1, c2, dst.w - x, dst.h - y);
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

#ifndef DISABLE_FLIPPED_MODE
const uint8 *FontSJISBase::flipCharacter(const uint8 *glyph, const int w) const {
	static const uint8 flipData[] = {
		0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
		0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
		0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
		0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
		0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
		0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
		0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
		0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
		0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
		0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
		0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
		0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
		0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
		0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
		0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
		0x0F, 0x8F, 0x4F, 0xC7, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x97, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
	};

	for (int i = 0; i < w; i++) {
		_tempGlyph[i] = flipData[glyph[(w * 2 - 1) - i]];
		_tempGlyph[(w * 2 - 1) - i] = flipData[glyph[i]];
	}

	return _tempGlyph;
}
#endif

void FontSJISBase::drawChar(void *dst, uint16 ch, int pitch, int bpp, uint32 c1, uint32 c2, int maxW, int maxH) const {
	const uint8 *glyphSource = 0;
	int width = 0, height = 0;
	int outlineExtraWidth = 2, outlineExtraHeight = 2;
	int outlineXOffset = 0, outlineYOffset = 0;

	if (is8x16(ch)) {
		glyphSource = getCharData8x16(ch);
		width = 8;
		height = 16;
	} else {
		glyphSource = getCharData(ch);
		width = 16;
		height = 16;
	}

	if (maxW != -1 && maxW < width) {
		width = maxW;
		outlineExtraWidth = 0;
		outlineXOffset = 1;
	}

	if (maxH != -1 && maxH < height) {
		height = maxH;
		outlineExtraHeight = 0;
		outlineYOffset = 1;
	}

	if (width <= 0 || height <= 0)
		return;

	if (!glyphSource) {
		warning("FontSJISBase::drawChar: Font does not offer data for %02X %02X", ch & 0xFF, ch >> 8);
		return;
	}

#ifndef DISABLE_FLIPPED_MODE
	if (_flippedMode)
		glyphSource = flipCharacter(glyphSource, width);
#endif

	uint8 outline[18 * 18];
	if (_drawMode == kOutlineMode) {
		memset(outline, 0, sizeof(outline));
		createOutline(outline, glyphSource, width, height);
	}

	if (bpp == 1) {
		if (_drawMode == kOutlineMode) {
			blitCharacter<uint8>(outline, width + outlineExtraWidth, height + outlineExtraHeight, (uint8 *)dst, pitch, c2);
			blitCharacter<uint8>(glyphSource, width - outlineXOffset, height - outlineYOffset, (uint8 *)dst + pitch + 1, pitch, c1);
		} else {
			if (_drawMode != kDefaultMode) {
				blitCharacter<uint8>(glyphSource, width - outlineXOffset, height, ((uint8*)dst) + 1, pitch, c2);
				blitCharacter<uint8>(glyphSource, width, height - outlineYOffset, ((uint8*)dst) + pitch, pitch, c2);
				if (_drawMode == kShadowMode)
					blitCharacter<uint8>(glyphSource, width - outlineXOffset, height - outlineYOffset, ((uint8*)dst) + pitch + 1, pitch, c2);
			}

			blitCharacter<uint8>(glyphSource, width, height, (uint8 *)dst, pitch, c1);
		}
	} else if (bpp == 2) {
		if (_drawMode == kOutlineMode) {
			blitCharacter<uint16>(outline, width + outlineExtraWidth, height + outlineExtraHeight, (uint8 *)dst, pitch, c2);
			blitCharacter<uint16>(glyphSource, width - outlineXOffset, height - outlineYOffset, (uint8 *)dst + pitch + 2, pitch, c1);
		} else {
			if (_drawMode != kDefaultMode) {
				blitCharacter<uint16>(glyphSource, width - outlineXOffset, height, ((uint8*)dst) + 2, pitch, c2);
				blitCharacter<uint16>(glyphSource, width, height - outlineYOffset, ((uint8*)dst) + pitch, pitch, c2);
				if (_drawMode == kShadowMode)
					blitCharacter<uint16>(glyphSource, width - outlineXOffset, height - outlineYOffset, ((uint8*)dst) + pitch + 2, pitch, c2);
			}

			blitCharacter<uint16>(glyphSource, width, height, (uint8 *)dst, pitch, c1);
		}
	} else {
		error("FontSJISBase::drawChar: unsupported bpp: %d", bpp);
	}
}

uint FontSJISBase::getCharWidth(uint16 ch) const {
	if (is8x16(ch))
		return (_drawMode == kOutlineMode) ? 10 : (_drawMode == kDefaultMode ? 8 : 9);
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

	if (magic1 != MKTAG('S','C','V','M') || magic2 != MKTAG('S','J','I','S')) {
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

