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


#ifdef ENABLE_EOB

#include "kyra/resource/resource.h"
#include "graphics/sjis.h"

namespace Kyra {

void Screen_EoB::selectPC98Palette(int palID, Palette &dest, int brightness, bool set) {
	if (palID < 0 || palID > 9)
		return;
	if (!_use16ColorMode)
		return;

	int temp = 0;
	const uint8 *pal16c = _vm->staticres()->loadRawData(kEoB1Palettes16c, temp);
	if (!pal16c)
		return;

	uint8 pal[48];
	for (int i = 0; i < 48; ++i)
		pal[i] = CLIP<int>(pal16c[palID * 48 + i] + brightness, 0, 15);
	loadPalette(pal, dest, 48);

	if (set)
		setScreenPalette(dest);
}

void Screen_EoB::decodeBIN(const uint8 *src, uint8 *dst, uint16 inSize) {
	const uint8 *end = src + inSize;
	memset(_dsTempPage, 0, 2048);
	int tmpDstOffs = 0;

	while (src < end) {
		uint8 code = *src++;
		if (!(code & 0x80)) {
			int offs = code << 4;
			code = *src++;
			offs |= (code >> 4);
			int len = (code & 0x0F) + 2;
			int tmpSrcOffs = (tmpDstOffs - offs) & 0x7FF;
			const uint8 *tmpSrc2 = dst;

			for (int len2 = len; len2; len2--) {
				*dst++ = _dsTempPage[tmpSrcOffs++];
				tmpSrcOffs &= 0x7FF;
			}

			while (len--) {
				_dsTempPage[tmpDstOffs++] = *tmpSrc2++;
				tmpDstOffs &= 0x7FF;
			}

		} else if (code & 0x40) {
			int len = code & 7;
			if (code & 0x20)
				len = (len << 8) | *src++;
			len += 2;

			int planes = ((code >> 3) & 3) + 1;
			while (len--) {
				for (int i = 0; i < planes; ++i) {
					*dst++ = _dsTempPage[tmpDstOffs++] = src[i];
					tmpDstOffs &= 0x7FF;
				}
			}
			src += planes;
		} else {
			for (int len = (code & 0x3F) + 1; len; len--) {
				*dst++ = _dsTempPage[tmpDstOffs++] = *src++;
				tmpDstOffs &= 0x7FF;
			}
		}
	}
}

void Screen_EoB::decodePC98PlanarBitmap(uint8 *srcDstBuffer, uint8 *tmpBuffer, uint16 size) {
	assert(tmpBuffer != srcDstBuffer);
	memcpy(tmpBuffer, srcDstBuffer, size);
	const uint8 *src = tmpBuffer;
	uint8 *dst1 = srcDstBuffer;
	uint8 *dst2 = srcDstBuffer + 4;
	size >>= 3;
	while (size--) {
		for (int i = 0; i < 4; ++i) {
			uint8 col1 = 0;
			uint8 col2 = 0;
			for (int ii = 0; ii < 4; ++ii) {
				col1 |= ((src[ii] >> (7 - i)) & 1) << ii;
				col2 |= ((src[ii] >> (3 - i)) & 1) << ii;
			}
			*dst1++ = col1;
			*dst2++ = col2;
		}
		src += 4;
		dst1 += 4;
		dst2 += 4;
	}
}

void Screen_EoB::initPC98PaletteCycle(int palID, PalCycleData *data) {
	if (!_use16ColorMode || !_cyclePalette)
		return;
	if (palID < 0 || palID > 9)
		return;

	_activePalCycle = data;
	int temp = 0;
	const uint8 *pal16c = _vm->staticres()->loadRawData(kEoB1Palettes16c, temp);
	if (!pal16c)
		return;

	if (data)
		memcpy(_cyclePalette, &pal16c[palID * 48], 48);
	else
		memset(_cyclePalette, 0, 48);
}

void Screen_EoB::updatePC98PaletteCycle(int brightness) {
	if (_activePalCycle) {
		for (int i = 0; i < 48; ++i) {
			if (--_activePalCycle[i].delay)
				continue;
			for (int8 in = 32; in == 32; ) {
				in = *_activePalCycle[i].data++;
				if (in < 16 && in > -16) {
					_cyclePalette[i] += in;
					_activePalCycle[i].delay = *_activePalCycle[i].data++;
				} else if (in < 32) {
					_cyclePalette[i] = in - 16;
					_activePalCycle[i].delay = *_activePalCycle[i].data++;
				} else if (in == 32)
					_activePalCycle[i].data += READ_BE_INT16(_activePalCycle[i].data);
			}
		}
	}

	uint8 pal[48];
	for (int i = 0; i < 48; ++i)
		pal[i] = CLIP<int>(_cyclePalette[i] + brightness, 0, 15);
	loadPalette(pal, *_palettes[0], 48);
	setScreenPalette(*_palettes[0]);
}

SJISFontEoB1PC98::SJISFontEoB1PC98(Common::SharedPtr<Graphics::FontSJIS> &font, /*uint8 shadowColor,*/ const uint16 *convTable1, const uint16 *convTable2) : SJISFont(font, 0, false, false, 0),
/*_shadowColor(shadowColor),*/ _convTable1(convTable1), _convTable2(convTable2), _defaultConv(true) {
	assert(_convTable1);
	assert(_convTable2);
}

int SJISFontEoB1PC98::getCharWidth(uint16 c) const {
	return SJISFont::getCharWidth(convert(c));
}

void SJISFontEoB1PC98::drawChar(uint16 c, byte *dst, int pitch, int) const {
	c = convert(c);
	_font->setDrawingMode(_style == kStyleLeftShadow ? Graphics::FontSJIS::kShadowLeftMode : Graphics::FontSJIS::kDefaultMode);
	_font->toggleFatPrint(false);
	_font->drawChar(dst, c, 640, 1, _colorMap[1], _colorMap[0], 640, 400);
}

uint16 SJISFontEoB1PC98::convert(uint16 c) const {
	uint8 l = c & 0xFF;
	uint8 h = c >> 8;

	if (c < 128) {
		assert(l > 31);
		c = _convTable2[l - 32];
	} else if (l > 160 && l < 225) {
		bool done = false;
		if (_defaultConv) {
			if (h == 0xDE) {
				if ((l >= 182 && l <= 196) || (l >= 202 && l <= 206)) {
					c = _convTable1[l - 182];
					done = true;
				}
			} else if (h == 0xDF) {
				if (l >= 202 && l <= 206) {
					c = _convTable1[l - 177];
					done = true;
				}
			}
		}
		if (!done)
			c = _convTable2[l - 64];
	}

	return c;
}

SJISFontEoB2PC98::SJISFontEoB2PC98(Common::SharedPtr<Graphics::FontSJIS> &font, /*uint8 shadowColor,*/ const char *convTable1, const char *convTable2) : SJISFont(font, 0, false, false, 0),
/*_shadowColor(shadowColor),*/ _convTable1(convTable1), _convTable2(convTable2), _defaultConv(true) {
	assert(_convTable1);
	assert(_convTable2);
}

int SJISFontEoB2PC98::getCharWidth(uint16 c) const {
	return SJISFont::getCharWidth(convert(c));
}

void SJISFontEoB2PC98::drawChar(uint16 c, byte *dst, int pitch, int) const {
	SJISFont::drawChar(convert(c), dst, pitch, 0);
}

uint16 SJISFontEoB2PC98::convert(uint16 c) const {
	uint8 l = c & 0xFF;
	uint8 h = c >> 8;

	if (h || l < 32 || l == 127) {
		return c;
	} else if (l < 127) {
		c = (l - 32) * 2;
		assert(c < 190);
		l = _convTable1[c];
		h = _convTable1[c + 1];
	} else if (l < 212) {
		h = l - 64;
		l = 0x83;
	} else {
		c = (l - 212) * 2;
		assert(c < 8);
		l = _convTable2[c];
		h = _convTable2[c + 1];
	}

	return (h << 8) | l;
}

Font12x12PC98::Font12x12PC98(uint8 shadowColor, const uint16 *convTable1, const uint16 *convTable2, const uint8 *lookupTable) : OldDOSFont(Common::kRenderDefault, 12), _convTable1(convTable1), _convTable2(convTable2) {
	assert(convTable1);
	assert(convTable2);
	assert(lookupTable);

	_width = _height = 12;
	_numGlyphs = 275;
	_bmpOffs = new uint16[_numGlyphs];
	for (int i = 0; i < _numGlyphs; ++i)
		_bmpOffs[i] = lookupTable[i] * 24;
}

Font12x12PC98::~Font12x12PC98() {
	delete[] _bmpOffs;
}

bool Font12x12PC98::load(Common::SeekableReadStream &file) {
	unload();

	_width = _height = 12;
	_numGlyphs = 275;
	_bitmapOffsets = _bmpOffs;

	_data = new uint8[file.size()];
	assert(_data);

	file.read(_data, file.size());
	if (file.err())
		return false;

	return true;
}

uint16 Font12x12PC98::convert(uint16 c) const {
	uint8 l = c & 0xFF;
	uint8 h = c >> 8;

	if (c < 128) {
		c = _convTable2[l - 32];
	} else if (l > 160 && l < 225) {
		bool done = false;
		if (h == 0xDE) {
			if ((l >= 182 && l <= 196) || (l >= 202 && l <= 206)) {
				c = _convTable1[l - 182];
				done = true;
			}
		} else if (h == 0xDF) {
			if (l >= 202 && l <= 206) {
				c = _convTable1[l - 177];
				done = true;
			}
		}
		if (!done)
			c = _convTable2[l - 64];
	}

	c = SWAP_BYTES_16(c);
	if (c < 0x813F)
		c = 1;
	else if (c < 0x824F)
		c -= 0x813F;
	else if (c < 0x833F)
		c -= 0x81EE;
	else if (c > 0x839F)
		c = 1;
	else
		c -= 0x828D;

	return c;
}

PC98Font::PC98Font(uint8 shadowColor, bool useOverlay, int scaleV, const uint8 *convTable1, const char *convTable2, const char *convTable3) : OldDOSFont(Common::kRenderVGA, shadowColor),
	_convTable1(convTable1), _convTable2(convTable2), _convTable3(convTable3), _outputWidth(0), _outputHeight(0), _type(convTable1 && convTable2 && convTable3 ? kSJIS : kASCII) {
	_numGlyphsMax = 256;
	_useOverlay = useOverlay;
	_scaleV = scaleV;

}

bool PC98Font::load(Common::SeekableReadStream &file) {
	bool res = OldDOSFont::load(file);

	_outputWidth = _width;
	_outputHeight = _height * _scaleV;

	if (_useOverlay) {
		_outputWidth >>= 1;
		_outputHeight >>= 1;
	}

	return res;
}

uint16 PC98Font::convert(uint16 c) const {
	if (_type == kSJIS)
		c = makeTwoByte(c);

	if (!_convTable1 || c < 128)
		return c;

	uint8 lo = c & 0xff;
	uint8 hi = c >> 8;

	if (lo == 0x81) {
		if (hi >= 0x40 && hi <= 0xac)
			return _convTable1[hi - 0x40];
	} else if (lo == 0x82) {
		if (hi >= 0x4f && hi <= 0x58)
			return hi - 31;
		if (hi >= 0x60 && hi <= 0x79)
			return hi - 31;
		if (hi >= 0x81 && hi <= 0x9a)
			return hi - 32;
	} else if (lo == 0x83 && hi >= 0x40 && hi <= 0x93) {
		return hi + 64;
	}

	return 0;
}

uint16 PC98Font::makeTwoByte(uint16 c) const {
	if (!_convTable2 || !_convTable3)
		return c;

	uint8 l = c & 0xFF;
	uint8 h = c >> 8;

	if (h || l < 32 || l == 127) {
		return c;
	} else if (l < 127) {
		c = (l - 32) * 2;
		assert(c < 190);
		l = _convTable2[c];
		h = _convTable2[c + 1];
	} else if (l < 212) {
		h = l - 64;
		l = 0x83;
	} else {
		c = (l - 212) * 2;
		assert(c < 8);
		l = _convTable3[c];
		h = _convTable3[c + 1];
	}

	return (h << 8) | l;
}

} // End of namespace Kyra

#endif // ENABLE_EOB
