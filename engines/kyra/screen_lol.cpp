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

#ifdef ENABLE_LOL

#include "kyra/screen_lol.h"
#include "kyra/lol.h"
#include "kyra/resource.h"

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
	_curDimIndex = 0;

	_mapDimX = _mapDimY = _mapDimW = _mapDimH = _mapDimDstX = _mapBlockWidth = _mapDimDstY = _mapBlockHeight = _mapDimU5 = _mapDimU6 = _mapBlockWidth2 = _mapDimU8 = 0;
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
	_curDimIndex = dim;
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

void Screen_LoL::fadeClearSceneWindow(int delay) {
	if (_fadeFlag == 1)
		return;
	
	uint8 *tpal = new uint8[768];

	memcpy(tpal, _currentPalette, 768);
	memset(tpal, 0, 384);
	loadSpecialColours(tpal);
	fadePalette(tpal, delay);
	fillRect(112, 0, 288, 120, 0);
	delete[] tpal;

	_fadeFlag = 1;
}

void Screen_LoL::backupSceneWindow(int srcPageNum, int dstPageNum) {
	uint8 *src = getPagePtr(srcPageNum) + 112;
	uint8 *dst = getPagePtr(dstPageNum) + 0xa500;

	for (int h = 0; h < 120; h++) {
		for (int w = 0; w < 176; w++)
			*dst++ = *src++;
		src += 144;
	}
}

void Screen_LoL::restoreSceneWindow(int srcPageNum, int dstPageNum) {
	uint8 *src = getPagePtr(srcPageNum) + 0xa500;
	uint8 *dst = getPagePtr(dstPageNum) + 112;

	for (int h = 0; h < 120; h++) {
		memcpy(dst, src, 176);
		src += 176;
		dst += 320;
	}

	if (!dstPageNum)
		addDirtyRect(112, 0, 176, 120);
}

void Screen_LoL::smoothScrollZoomStepTop(int srcPageNum, int dstPageNum, int x, int y) {
	uint8 *src = getPagePtr(srcPageNum) + 0xa500 + y * 176 + x;
	uint8 *dst = getPagePtr(dstPageNum) + 0xa500;

	x <<= 1;
	uint16 width = 176 - x;
	uint16 scaleX = (((x + 1) << 8) / width + 0x100);
	uint16 cntW = scaleX >> 8;
	scaleX <<= 8;
	width--;
	uint16 widthCnt = width;

	uint16 height = 46 - y;
	uint16 scaleY = (((y + 1) << 8) / height + 0x100);
	scaleY <<= 8;

	uint32 scaleYc = 0;
	while (height) {
		uint32 scaleXc = 0;
		do {
			scaleXc += scaleX;
			int numbytes = cntW + (scaleXc >> 16);
			scaleXc &= 0xffff;
			memset(dst, *src++, numbytes);
			dst += numbytes;
		} while (--widthCnt);

		*dst++ = *src++;
		widthCnt = width;

		src += x;
		scaleYc += scaleY;

		if (scaleYc >> 16) {
			scaleYc = 0;
			src -= 176;
			continue;
		}

		height--;
	}
}

void Screen_LoL::smoothScrollZoomStepBottom(int srcPageNum, int dstPageNum, int x, int y) {
	uint8 *src = getPagePtr(srcPageNum) + 0xc4a0 + x;
	uint8 *dst = getPagePtr(dstPageNum) + 0xc4a0;

	x <<= 1;
	uint16 width = 176 - x;
	uint16 scaleX = (((x + 1) << 8) / width + 0x100);
	uint16 cntW = scaleX >> 8;
	scaleX <<= 8;
	width--;
	uint16 widthCnt = width;

	uint16 height = 74 - y;
	uint16 scaleY = (((y + 1) << 8) / height + 0x100);
	scaleY <<= 8;

	uint32 scaleYc = 0;
	while (height) {
		uint32 scaleXc = 0;
		do {
			scaleXc += scaleX;
			int numbytes = cntW + (scaleXc >> 16);
			scaleXc &= 0xffff;
			memset(dst, *src++, numbytes);
			dst += numbytes;
		} while (--widthCnt);

		*dst++ = *src++;
		widthCnt = width;

		src += x;
		scaleYc += scaleY;

		if (scaleYc >> 16) {
			scaleYc = 0;
			src -= 176;
			continue;
		}

		height--;
	}
}

void Screen_LoL::smoothScrollHorizontalStep(int pageNum, int srcX, int dstX, int w) {
	uint8 *d = getPagePtr(pageNum);
	uint8 *s = d + 112 + srcX;

	int w2 = srcX + w - dstX;
	int pitchS = 320 + w2 - (w << 1);

	int pitchD = 320 - w;
	int h = 120;

	while (h--) {
		for (int i = 0; i < w; i++)
			*d++ = *s++;
		d -= w;
		s -= w2;

		for (int i = 0; i < w; i++)
			*s++ = *d++;
		
		s += pitchS;
		d += pitchD;
	}
}

void Screen_LoL::smoothScrollTurnStep1(int srcPage1Num, int srcPage2Num, int dstPageNum) {
	uint8 *s = getPagePtr(srcPage1Num) + 273;
	uint8 *d = getPagePtr(dstPageNum) + 0xa500;

	for (int i = 0; i < 120; i++) {
		uint8 a = *s++;
		*d++ = a;
		*d++ = a;

		for (int ii = 0; ii < 14; ii++) {
			a = *s++;
			*d++ = a;
			*d++ = a;
			*d++ = a;
		}

		s += 305;
		d += 132;
	}

	s = getPagePtr(srcPage2Num) + 112;
	d = getPagePtr(dstPageNum)  + 0xa52c;

	for (int i = 0; i < 120; i++) {
		for (int ii = 0; ii < 33; ii++) {
			*d++ = *s++;
			*d++ = *s++;
			uint8 a = *s++;
			*d++ = a;
			*d++ = a;
		}

		s += 221;
		d += 44;
	}
}

void Screen_LoL::smoothScrollTurnStep2(int srcPage1Num, int srcPage2Num, int dstPageNum) {
	uint8 *s = getPagePtr(srcPage1Num) + 244;
	uint8 *d = getPagePtr(dstPageNum) + 0xa500;

	for (int k = 0; k < 2; k++) {
		for (int i = 0; i < 120; i++) {
			for (int ii = 0; ii < 44; ii++) {
				uint8 a = *s++;
				*d++ = a;
				*d++ = a;
			}

			s += 276;
			d += 88;
		}

		s = getPagePtr(srcPage2Num) + 112;
		d = getPagePtr(dstPageNum) + 0xa558;
	}
}

void Screen_LoL::smoothScrollTurnStep3(int srcPage1Num, int srcPage2Num, int dstPageNum) {
	uint8 *s = getPagePtr(srcPage1Num) + 189;
	uint8 *d = getPagePtr(dstPageNum) + 0xa500;

	for (int i = 0; i < 120; i++) {
		for (int ii = 0; ii < 33; ii++) {
			*d++ = *s++;
			*d++ = *s++;
			uint8 a = *s++;
			*d++ = a;
			*d++ = a;
		}

		s += 221;
		d += 44;
	}

	s = getPagePtr(srcPage2Num) + 112;
	d = getPagePtr(dstPageNum)  + 0xa584;

	for (int i = 0; i < 120; i++) {
		for (int ii = 0; ii < 14; ii++) {
			uint8 a = *s++;
			*d++ = a;
			*d++ = a;
			*d++ = a;
		}

		uint8 a = *s++;
		*d++ = a;
		*d++ = a;

		s += 305;
		d += 132;
	}
}

void Screen_LoL::copyBlockSpecial(int page1, int x1, int y1, int page2, int x2, int y2, int w, int h, int dim, uint8 *ovl) {
	if (!w || !h || !ovl)
		return;

	const ScreenDim *cdim = getScreenDim(dim);
	_mapDimX = cdim->sx << 3;
	_mapDimY = cdim->sy;
	_mapDimW = cdim->w << 3;
	_mapDimH = cdim->h;

	calcMapBoundaries(x2, y2, w, h);
	if (_mapBlockWidth == -1)
		return;

	uint8 *src = getPagePtr(page1) + y1 * 320 + x1;
	uint8 *dst = getPagePtr(page2) + (_mapDimDstY + _mapDimY) * 320;

	for (int i = 0; i < _mapBlockHeight; i++) {
		uint8 *s = src + _mapDimU5;
		uint8 *d = dst + (_mapDimDstX + _mapDimX);

		for (int ii = 0; ii < _mapBlockWidth; ii++) {
			uint8 p = ovl[*s++];
			if (p)
				*d = p;
			d++;
		}

		dst += 320;
		src += 320;
	}

	addDirtyRect(_mapDimDstX + _mapDimX, _mapDimDstY + _mapDimY, _mapBlockWidth, _mapBlockHeight);
}

void Screen_LoL::calcMapBoundaries(int dstX, int dstY, int width, int height) {
	_mapBlockWidth = _mapBlockWidth2 = width;
	_mapBlockHeight = height;
	_mapDimDstX = dstX;
	_mapDimDstY = dstY;

	_mapDimU5 = _mapDimU6 = _mapDimU8 = 0;

	int t = _mapDimDstX + _mapBlockWidth;
	if (t <= 0) {
		_mapBlockWidth = _mapBlockHeight = -1;
		return;
	}

	if (t <= _mapDimDstX) {
		_mapDimU5 = _mapBlockWidth - t;
		_mapBlockWidth = t;
		_mapDimDstX = 0;
	}

	t = _mapDimW - _mapDimDstX;
	if (t <= 0) {
		_mapBlockWidth = _mapBlockHeight = -1;
		return;
	}

	if (t <= _mapBlockWidth)
		_mapBlockWidth = t;

	_mapBlockWidth2 -= _mapBlockWidth;

	t = _mapDimDstY + _mapBlockHeight;
	if (t <= 0) {
		_mapBlockWidth = _mapBlockHeight = -1;
		return;
	}

	if (t <= _mapDimDstY) {
		_mapDimU6 = _mapBlockHeight - t;
		_mapBlockHeight = t;
		_mapDimDstY = 0;
	}

	t = _mapDimH - _mapDimDstY;
	if (t <= 0) {
		_mapBlockWidth = _mapBlockHeight = -1;
		return;
	}

	if (t <= _mapBlockHeight)
		_mapBlockHeight = t;
}

void Screen_LoL::fadeToBlack(int delay, const UpdateFunctor *upFunc) {
	Screen::fadeToBlack(delay, upFunc);
	_fadeFlag = 2;
}

void Screen_LoL::loadSpecialColours(uint8 *destPalette) {
	memcpy(destPalette + 0x240, _screenPalette + 0x240, 12);	
}

void Screen_LoL::copyColour(int dstColourIndex, int srcColourIndex) {
	uint8 *s = _screenPalette + srcColourIndex * 3;
	uint8 *d = _screenPalette + dstColourIndex * 3;
	memcpy(d, s, 3);

	uint8 ci[4];
	ci[0] = (d[0] << 2) | (d[0] & 3);
	ci[1] = (d[1] << 2) | (d[1] & 3);
	ci[2] = (d[2] << 2) | (d[2] & 3);
	ci[3] = 0;

	_system->setPalette(ci, dstColourIndex, 1);
}

bool Screen_LoL::fadeColour(int dstColourIndex, int srcColourIndex, uint32 elapsedTime, uint32 targetTime) {
	uint8 *dst = _screenPalette + 3 * dstColourIndex;
	uint8 *src = _screenPalette + 3 * srcColourIndex;
	uint8 *p = getPalette(1) + 3 * dstColourIndex;

	bool res = false;

	int16 t1 = 0;
	int16 t2 = 0;
	int32 t3 = 0;

	uint8 tmpPalEntry[3];

	for (int i = 0; i < 3; i++) {
		if (elapsedTime < targetTime) {
			t1 = *src & 0x3f;
			t2 = *dst & 0x3f;

			t3 = t1 - t2;
			if (t3)
				res = true;

			t3 = (((t3 << 8) / (int)targetTime) * (int)elapsedTime) >> 8;
			t3 =  t2 + t3;
		} else {
			t1 = *dst & 0x3f;
			*p = t3 = t1;
			res = false;
		}

		tmpPalEntry[i] = t3 & 0xff;
		src++;
		dst++;
		p++;
	}

	uint8 tpal[768];
	memcpy(tpal, _screenPalette, 768);
	memcpy(tpal + dstColourIndex * 3, tmpPalEntry, 3);
	setScreenPalette(tpal);
	updateScreen();

	return res;
}

uint8 Screen_LoL::getShapePaletteSize(const uint8 *shp) {
	debugC(9, kDebugLevelScreen, "Screen_LoL::getShapePaletteSize(%p)", (const void *)shp);

	return shp[10];
}

} // end of namespace Kyra

#endif // ENABLE_LOL

