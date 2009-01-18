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

#include "kyra/screen_lol.h"
#include "kyra/lol.h"

namespace Kyra {

Screen_LoL::Screen_LoL(LoLEngine *vm, OSystem *system) : Screen_v2(vm, system), _vm(vm) {
	_customDimTable = new ScreenDim*[_screenDimTableCount];
	memset(_customDimTable, 0, sizeof(ScreenDim*) * _screenDimTableCount);

	_paletteOverlay1 = new uint8[0x100];
	_paletteOverlay2 = new uint8[0x100];
	_grayOverlay = new uint8[0x100];
	memset(_paletteOverlay1, 0, 0x100);
	memset(_paletteOverlay2, 0, 0x100);
	memset(_grayOverlay, 0, 0x100);

	for (int i = 0; i < 8; i++)
		_levelOverlays[i] = new uint8[256];
	
	_fadeFlag = 2;
	_drawGuiFlag = 0;
}

Screen_LoL::~Screen_LoL() {
	for (int i = 0; i < _screenDimTableCount; i++)
		delete _customDimTable[i];
	delete[] _customDimTable;

	for (int i = 0; i < 8; i++)
		delete[] _levelOverlays[i];

	delete[] _paletteOverlay1;
	delete[] _paletteOverlay2;
	delete[] _grayOverlay;
}

void Screen_LoL::setScreenDim(int dim) {
	debugC(9, kDebugLevelScreen, "Screen_LoL::setScreenDim(%d)", dim);
	assert(dim < _screenDimTableCount);
	_curDim = _customDimTable[dim] ? (const ScreenDim *)_customDimTable[dim] : &_screenDimTable[dim];
}

const ScreenDim *Screen_LoL::getScreenDim(int dim) {
	debugC(9, kDebugLevelScreen, "Screen_LoL::getScreenDim(%d)", dim);
	assert(dim < _screenDimTableCount);
	return _customDimTable[dim] ? (const ScreenDim *)_customDimTable[dim] : &_screenDimTable[dim];
}

void Screen_LoL::modifyScreenDim(int dim, int x, int y, int w, int h) {
	delete _customDimTable[dim];
	_customDimTable[dim] = new ScreenDim;
	memcpy(_customDimTable[dim], &_screenDimTable[dim], sizeof(ScreenDim));
	_customDimTable[dim]->sx = x;
	_customDimTable[dim]->sy = y;
	_customDimTable[dim]->w = w;
	_customDimTable[dim]->h = h;
	setScreenDim(dim);
}

void Screen_LoL::clearDim(int dim) {
	const ScreenDim *tmp = getScreenDim(dim);
	fillRect(tmp->sx << 3, tmp->sy, ((tmp->sx + tmp->w) << 3) - 1, (tmp->sy + tmp->h) - 1, tmp->unkA);
}

void Screen_LoL::clearCurDim() {
	fillRect(_curDim->sx << 3, _curDim->sy, ((_curDim->sx + _curDim->w) << 3) - 1, (_curDim->sy + _curDim->h) - 1, _curDim->unkA);
}

void Screen_LoL::fprintString(const char *format, int x, int y, uint8 col1, uint8 col2, uint16 flags, ...) {
	debugC(9, kDebugLevelScreen, "Screen_LoL::fprintString('%s', %d, %d, %d, %d, %d, ...)", format, x, y, col1, col2, flags);
	if (!format)
		return;

	char string[240];
	va_list vaList;
	va_start(vaList, flags);
	vsnprintf(string, sizeof(string), format, vaList);
	va_end(vaList);

	if (flags & 1)
		x -= getTextWidth(string) >> 1;

	if (flags & 2)
		x -= getTextWidth(string);

	if (flags & 4) {
		printText(string, x - 1, y, 1, col2);
		printText(string, x, y + 1, 1, col2);
	}

	if (flags & 8) {
		printText(string, x - 1, y, 227, col2);
		printText(string, x, y + 1, 227, col2);
	}

	printText(string, x, y, col1, col2);
}

void Screen_LoL::fprintStringIntro(const char *format, int x, int y, uint8 c1, uint8 c2, uint8 c3, uint16 flags, ...) {
	debugC(9, kDebugLevelScreen, "Screen_LoL::fprintStringIntro('%s', %d, %d, %d, %d, %d, %d, ...)", format, x, y, c1, c2, c3, flags);
	char buffer[400];

	va_list args;
	va_start(args, flags);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);

	if ((flags & 0x0F00) == 0x100)
		x -= getTextWidth(buffer) >> 1;
	if ((flags & 0x0F00) == 0x200)
		x -= getTextWidth(buffer);

	if ((flags & 0x00F0) == 0x20) {
		printText(buffer, x-1, y, c3, c2);
		printText(buffer, x, y+1, c3, c2);
	}

	printText(buffer, x, y, c1, c2);
}

void Screen_LoL::generateGrayOverlay(const uint8 *srcPal, uint8 *grayOverlay, int factor, int addR, int addG, int addB, int lastColor, bool skipSpecialColours) {
	uint8 tmpPal[768];

	for (int i = 0; i != lastColor; i++) {
		int v = (((srcPal[3 * i] & 0x3f) * factor) / 0x40) + addR;
		tmpPal[3 * i] = (v > 0x3f) ? 0x3f : v & 0xff;
		v = (((srcPal[3 * i + 1] & 0x3f) * factor) / 0x40) + addG;
		tmpPal[3 * i + 1] = (v > 0x3f) ? 0x3f : v & 0xff;
		v = (((srcPal[3 * i + 2] & 0x3f) * factor) / 0x40) + addB;
		tmpPal[3 * i + 2] = (v > 0x3f) ? 0x3f : v & 0xff;
	}

	for (int i = 0; i < lastColor; i++)
		grayOverlay[i] = findLeastDifferentColor(tmpPal + 3 * i, srcPal, lastColor, skipSpecialColours);
}

uint8 *Screen_LoL::generateLevelOverlay(const uint8 *srcPal, uint8 *ovl, int opColor, int weight) {
	if (!srcPal || !ovl)
		return ovl;

	if (weight > 255)
		weight = 255;

	uint16 r = srcPal[opColor * 3];
	uint16 g = srcPal[opColor * 3 + 1];
	uint16 b = srcPal[opColor * 3 + 2];

	uint8 *d = ovl;
	*d++ = 0;

	for (int i = 1; i != 255; i++) {
		uint16 a = srcPal[i * 3];
		uint8 dr = a - ((((a - r) * (weight >> 1)) << 1) >> 8);
		a = srcPal[i * 3 + 1];
		uint8 dg = a - ((((a - g) * (weight >> 1)) << 1) >> 8);
		a = srcPal[i * 3 + 2];
		uint8 db = a - ((((a - b) * (weight >> 1)) << 1) >> 8);

		int l = opColor;
		int m = 0x7fff;
		int ii = 127;
		int x = 1;
		const uint8 *s = srcPal + 3;

		do {
			if (i == x) {
				s += 3;
			} else {
				int t = *s++ - dr;
				int c = t * t;
				t = *s++ - dg;
				c += (t * t);
				t = *s++ - db;
				c += (t * t);

				if (!c) {
					l = x;
					break;
				}

				if (c <= m) {
					m = c;
					l = x;
				}				
			}
			x++;
		} while (--ii);

		*d++ = l & 0xff;
	}

	return ovl;	
}

void Screen_LoL::drawGridBox(int x, int y, int w, int h, int col) {
	if (w <= 0 || x >= 320 || h <= 0 || y >= 200)
		return;

	if (x < 0) {
		x += w;
		if (x <= 0)
			return;
		w = x;
		x = 0;		
	}

	int tmp = x + w;
	if (tmp >= 320) {
		w = 320 - x;
	}

	int pitch = 320 - w;

	if (y < 0) {
		y += h;
		if (y <= 0)
			return;
		h = y;
		y = 0;		
	}

	tmp = y + h;
	if (tmp >= 200) {
		h = 200 - y;
	}

	tmp = (y + x) & 1;
	uint8 *p = getPagePtr(_curPage) + y * 320 + x;
	uint8 s = (tmp >> 8) & 1;

	w >>= 1;
	int w2 = w;
	
	while (h--) {
		if (w) {
			while (w--) {
				*(p + tmp) = col;
				p += 2;
			}
		} else {
			w = 1;
		}

		if (s == 1) {
			if (tmp == 0)
				*p = col;
			p++;
		}
		tmp ^= 1;
		p += pitch;
		w = w2;
	}
}

void Screen_LoL::fadeToBlack(int delay, const UpdateFunctor *upFunc) {
	Screen::fadeToBlack(delay, upFunc);
	_fadeFlag = 2;
}

void Screen_LoL::setPaletteBrightness(uint8 *palette, int brightness, int modifier) {
	generateBrightnessPalette(palette, getPalette(1), brightness, modifier);
	fadePalette(getPalette(1), 5, 0);
	_fadeFlag = 0;
}

void Screen_LoL::generateBrightnessPalette(uint8 *src, uint8 *dst, int brightness, int modifier) {
	memcpy(dst, src, 0x300);
	setPaletteColoursSpecial(dst);
	brightness = (8 - brightness) << 5;
	if (modifier >= 0 && modifier < 8 && _drawGuiFlag & 0x800) {
		brightness = 256 - ((((modifier & 0xfffe) << 5) * (256 - brightness)) >> 8);
		if (brightness < 0)
			brightness = 0;
	}
	
	for (int i = 0; i < 384; i++) {
		uint16 c = (dst[i] * brightness) >> 8;
		dst[i] = c & 0xff;
	}
}

void Screen_LoL::setPaletteColoursSpecial(uint8 *palette) {
	const uint8 src[] = { 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00 };
	palette += 0x240;
	memcpy(palette, src, 12);	
}

uint8 Screen_LoL::getShapePaletteSize(const uint8 *shp) {
	debugC(9, kDebugLevelScreen, "Screen_LoL::getShapePaletteSize(%p)", (const void *)shp);

	return shp[10];
}

} // end of namespace Kyra

