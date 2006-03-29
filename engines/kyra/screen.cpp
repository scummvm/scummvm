/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"
#include "common/system.h"
#include "kyra/screen.h"
#include "kyra/kyra.h"

namespace Kyra {

#define BITBLIT_RECTS 10

Screen::Screen(KyraEngine *vm, OSystem *system)
	: _system(system), _vm(vm) {
	_curPage = 0;
	for (int pageNum = 0; pageNum < SCREEN_PAGE_NUM; pageNum += 2) {
		uint8 *pagePtr = (uint8 *)malloc(SCREEN_PAGE_SIZE);
		if (pagePtr) {
			memset(pagePtr, 0, SCREEN_PAGE_SIZE);
			_pagePtrs[pageNum] = _pagePtrs[pageNum + 1] = pagePtr;
		}
	}
	memset(_shapePages, 0, sizeof(_shapePages));
	_currentPalette = (uint8 *)malloc(768);
	if (_currentPalette) {
		memset(_currentPalette, 0, 768);
	}
	_screenPalette = (uint8 *)malloc(768);
	if (_screenPalette) {
		memset(_screenPalette, 0, 768);
	}
	for (int i = 0; i < 3; ++i) {
		_palettes[i] = (uint8 *)malloc(768);
		if (_palettes[i]) {
			memset(_palettes[i], 0, 768);
		}
	}
	_curDim = &_screenDimTable[0];
	_charWidth = 0;
	_charOffset = 0;
	memset(_fonts, 0, sizeof(_fonts));
	for (int i = 0; i < ARRAYSIZE(_textColorsMap); ++i) {
		_textColorsMap[i] = i;
	}
	_decodeShapeBuffer = NULL;
	_decodeShapeBufferSize = 0;
	_animBlockPtr = NULL;
	_animBlockSize = 0;
	_mouseLockCount = 0;
	
	_bitBlitRects = new Rect[BITBLIT_RECTS];
	assert(_bitBlitRects);
	memset(_bitBlitRects, 0, sizeof(Rect)*BITBLIT_RECTS);
	_bitBlitNum = 0;
	memset(_saveLoadPage, 0, sizeof(_saveLoadPage));

	_unkPtr1 = (uint8*)malloc(getRectSize(1, 144));
	memset(_unkPtr1, 0, getRectSize(1, 144));
	_unkPtr2 = (uint8*)malloc(getRectSize(1, 144));
	memset(_unkPtr2, 0, getRectSize(1, 144));
}

Screen::~Screen() {
	for (int pageNum = 0; pageNum < SCREEN_PAGE_NUM; pageNum += 2) {
		free(_pagePtrs[pageNum]);
		_pagePtrs[pageNum] = _pagePtrs[pageNum + 1] = 0;
	}
	for (int f = 0; f < ARRAYSIZE(_fonts); ++f) {
		delete[] _fonts[f].fontData;
		_fonts[f].fontData = NULL;
	}
	free(_currentPalette);
	free(_screenPalette);
	free(_decodeShapeBuffer);
	free(_animBlockPtr);
	for (int i = 0; i < 3; ++i) {
		free(_palettes[i]);
	}
	delete [] _bitBlitRects;
	for (int i = 0; i < ARRAYSIZE(_saveLoadPage); ++i) {
		delete [] _saveLoadPage[i];
		_saveLoadPage[i] = 0;
	}

	free(_unkPtr1);
	free(_unkPtr2);
}

void Screen::updateScreen() {
	debugC(9, kDebugLevelScreen, "Screen::updateScreen()");
	_system->copyRectToScreen(getPagePtr(0), SCREEN_W, 0, 0, SCREEN_W, SCREEN_H);
	//for debug reasons (needs 640x200 screen)
	//_system->copyRectToScreen(getPagePtr(2), SCREEN_W, 320, 0, SCREEN_W, SCREEN_H);
	_system->updateScreen();
}

uint8 *Screen::getPagePtr(int pageNum) {
	debugC(9, kDebugLevelScreen, "Screen::getPagePtr(%d)", pageNum);
	assert(pageNum < SCREEN_PAGE_NUM);
	return _pagePtrs[pageNum];
}

void Screen::clearPage(int pageNum) {
	debugC(9, kDebugLevelScreen, "Screen::clearPage(%d)", pageNum);
	assert(pageNum < SCREEN_PAGE_NUM);
	memset(getPagePtr(pageNum), 0, SCREEN_PAGE_SIZE);
}

int Screen::setCurPage(int pageNum) {
	debugC(9, kDebugLevelScreen, "Screen::setCurPage(%d)", pageNum);
	assert(pageNum < SCREEN_PAGE_NUM);
	int previousPage = _curPage;
	_curPage = pageNum;
	return previousPage;
}

void Screen::clearCurPage() {
	debugC(9, kDebugLevelScreen, "Screen::clearCurPage()");
	memset(getPagePtr(_curPage), 0, SCREEN_PAGE_SIZE);
}

uint8 Screen::getPagePixel(int pageNum, int x, int y) {
	debugC(9, kDebugLevelScreen, "Screen::getPagePixel(%d, %d, %d)", pageNum, x, y);
	assert(pageNum < SCREEN_PAGE_NUM);
	assert(x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H);
	return _pagePtrs[pageNum][y * SCREEN_W + x];
}

void Screen::setPagePixel(int pageNum, int x, int y, uint8 color) {
	debugC(9, kDebugLevelScreen, "Screen::setPagePixel(%d, %d, %d, %d)", pageNum, x, y, color);
	assert(pageNum < SCREEN_PAGE_NUM);
	assert(x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H);
	_pagePtrs[pageNum][y * SCREEN_W + x] = color;
}

void Screen::fadeFromBlack() {
	debugC(9, kDebugLevelScreen, "Screen::fadeFromBlack()");
	fadePalette(_currentPalette, 0x54);
}

void Screen::fadeToBlack() {
	debugC(9, kDebugLevelScreen, "Screen::fadeToBlack()");
	uint8 blackPal[768];
	memset(blackPal, 0, 768);
	fadePalette(blackPal, 0x54);
}

void Screen::fadeSpecialPalette(int palIndex, int startIndex, int size, int fadeTime) {
	debugC(9, kDebugLevelScreen, "fadeSpecialPalette(%d, %d, %d, %d)", palIndex, startIndex, size, fadeTime);
	assert(_vm->palTable1()[palIndex]);
	assert(_currentPalette);
	uint8 tempPal[768];
	memcpy(tempPal, _currentPalette, 768);
	memcpy(&tempPal[startIndex*3], _vm->palTable1()[palIndex], size*3);
	fadePalette(tempPal, fadeTime*18);
	memcpy(&_currentPalette[startIndex*3], &tempPal[startIndex*3], size*3);
	setScreenPalette(_currentPalette);
	_system->updateScreen();
}

void Screen::fadePalette(const uint8 *palData, int delay) {
	debugC(9, kDebugLevelScreen, "Screen::fadePalette(%p, %d)", (const void *)palData, delay);
	uint8 fadePal[768];
	memcpy(fadePal, _screenPalette, 768);
	uint8 diff, maxDiff = 0;
	for (int i = 0; i < 768; ++i) {
		diff = ABS(palData[i] - fadePal[i]);
		if (diff > maxDiff) {
			maxDiff = diff;
		}
	}
	int16 delayInc = delay << 8;
	if (maxDiff != 0) {
		delayInc /= maxDiff;
	}
	delay = delayInc;
	for (diff = 1; diff <= maxDiff; ++diff) {
		if (delayInc >= 512) {
			break;
		}
		delayInc += delay;
	}
	int delayAcc = 0;
	while (1) {
		delayAcc += delayInc;
		bool needRefresh = false;
		for (int i = 0; i < 768; ++i) {
			int c1 = palData[i];
			int c2 = fadePal[i];
			if (c1 != c2) {
				needRefresh = true;
				if (c1 > c2) {
					c2 += diff;
					if (c1 < c2) {
						c2 = c1;
					}
				}
				if (c1 < c2) {
					c2 -= diff;
					if (c1 > c2) {
						c2 = c1;
					}
				}
				fadePal[i] = (uint8)c2;
			}
		}
		if (!needRefresh) {
			break;
		}
		setScreenPalette(fadePal);
		_system->updateScreen();
		//_system->delayMillis((delayAcc >> 8) * 1000 / 60);
		_vm->delay((delayAcc >> 8) * 1000 / 60);
		delayAcc &= 0xFF;
	}
}

void Screen::setPaletteIndex(uint8 index, uint8 red, uint8 green, uint8 blue) {
	debugC(9, kDebugLevelScreen, "Screen::setPaletteIndex(%u, %u, %u, %u)", index, red, green, blue);
	_currentPalette[index * 3 + 0] = red;
	_currentPalette[index * 3 + 1] = green;
	_currentPalette[index * 3 + 2] = blue;
	setScreenPalette(_currentPalette);
}

void Screen::setScreenPalette(const uint8 *palData) {
	debugC(9, kDebugLevelScreen, "Screen::setScreenPalette(%p)", (const void *)palData);
	memcpy(_screenPalette, palData, 768);
	uint8 screenPal[256 * 4];
	for (int i = 0; i < 256; ++i) {
		screenPal[4 * i + 0] = (palData[0] << 2) | (palData[0] & 3);
		screenPal[4 * i + 1] = (palData[1] << 2) | (palData[1] & 3);
		screenPal[4 * i + 2] = (palData[2] << 2) | (palData[2] & 3);
		screenPal[4 * i + 3] = 0;
		palData += 3;
	}
	_system->setPalette(screenPal, 0, 256);
}

void Screen::copyToPage0(int y, int h, uint8 page, uint8 *seqBuf) {
	debugC(9, kDebugLevelScreen, "Screen::copyToPage0(%d, %d, %d, %p)", y, h, page, (const void *)seqBuf);
	assert(y + h <= SCREEN_H);
	const uint8 *src = getPagePtr(page) + y * SCREEN_W;
	uint8 *dstPage = getPagePtr(0) + y * SCREEN_W;
	for (int i = 0; i < h; ++i) {
		for (int x = 0; x < SCREEN_W; ++x) {
			if (seqBuf[x] != src[x]) {
				seqBuf[x] = src[x];
				dstPage[x] = src[x];
			}
		}
		src += SCREEN_W;
		seqBuf += SCREEN_W;
		dstPage += SCREEN_W;
	}
}

void Screen::copyRegion(int x1, int y1, int x2, int y2, int w, int h, int srcPage, int dstPage, int flags) {
	debugC(9, kDebugLevelScreen, "Screen::copyRegion(%d, %d, %d, %d, %d, %d, %d, %d, %d)", x1, y1, x2, y2, w, h, srcPage, dstPage, flags);
	
	if (flags & CR_CLIPPED) {
		if (x2 < 0) {
			if (x2  <= -w)
				return;
			w += x2;
			x1 -= x2;
			x2 = 0;
		} else if (x2 + w >= SCREEN_W) {
			if (x2 > SCREEN_W)
				return;
			w = SCREEN_W - x2;
		}

		if (y2 < 0) {
			if (y2 <= -h )
				return;
			h += y2;
			y1 -= y2;
			y2 = 0;
		} else if (y2 + h >= SCREEN_H) {
			if (y2 > SCREEN_H)
				return;
			h = SCREEN_H - y2;
		}
	}

	assert(x1 + w <= SCREEN_W && y1 + h <= SCREEN_H);
	const uint8 *src = getPagePtr(srcPage) + y1 * SCREEN_W + x1;
	assert(x2 + w <= SCREEN_W && y2 + h <= SCREEN_H);
	uint8 *dst = getPagePtr(dstPage) + y2 * SCREEN_W + x2;

	if (flags & CR_X_FLIPPED) {
		while (h--) {
			for (int i = 0; i < w; ++i) {
				if (src[i]) {
					dst[w-i] = src[i];
				}
			}
			src += SCREEN_W;
			dst += SCREEN_W;
		}
	} else {
		while (h--) {
			for (int i = 0; i < w; ++i) {
				if (src[i]) {
					dst[i] = src[i];
				}
			}
			src += SCREEN_W;
			dst += SCREEN_W;
		}
	}
}

void Screen::copyRegionToBuffer(int pageNum, int x, int y, int w, int h, uint8 *dest) {
	debugC(9, kDebugLevelScreen, "Screen::copyRegionToBuffer(%d, %d, %d, %d, %d)", pageNum, x, y, w, h);
	assert(x >= 0 && x < Screen::SCREEN_W && y >= 0 && y < Screen::SCREEN_H && dest);
	uint8 *pagePtr = getPagePtr(pageNum);
	for (int i = y; i < y + h; i++) {
		memcpy(dest + (i - y) * w, pagePtr + i * SCREEN_W + x, w);
	}
}

void Screen::copyBlockToPage(int pageNum, int x, int y, int w, int h, const uint8 *src) {
	debugC(9, kDebugLevelScreen, "Screen::copyBlockToPage(%d, %d, %d, %d, %d, %p)", pageNum, x, y, w, h, (const void *)src);
	assert(x >= 0 && x < Screen::SCREEN_W && y >= 0 && y < Screen::SCREEN_H);
	uint8 *dst = getPagePtr(pageNum) + y * SCREEN_W + x;
	while (h--) {
		memcpy(dst, src, w);
		dst += SCREEN_W;
		src += w;
	}
}

void Screen::copyFromCurPageBlock(int x, int y, int w, int h, const uint8 *src) {
	debugC(9, kDebugLevelScreen, "Screen::copyFromCurPageBlock(%d, %d, %d, %d, %p)", x, y, w, h, (const void *)src);
	if (x < 0) {
		x = 0;	
	} else if (x >= 40) {
		return;
	}
	if (x + w > 40) {
		w = 40 - x;
	}
	if (y < 0) {
		y = 0;
	} else if (y >= 200) {
		return;
	}
	if (y + h > 200) {
		h = 200 - y;
	}
	uint8 *dst = getPagePtr(_curPage) + y * SCREEN_W + x * 8;
	while (h--) {
		memcpy(dst, src, w*8);
		dst += SCREEN_W;
		src += w*8;
	}
}

void Screen::copyCurPageBlock(int x, int y, int w, int h, uint8 *dst) {
	debugC(9, kDebugLevelScreen, "Screen::copyCurPageBlock(%d, %d, %d, %d, %p)", x, y, w, h, (const void *)dst);
	assert(dst);
	if (x < 0) {
		x = 0;	
	} else if (x >= 40) {
		return;
	}
	if (x + w > 40) {
		w = 40 - x;
	}
	if (y < 0) {
		y = 0;
	} else if (y >= 200) {
		return;
	}
	if (y + h > 200) {
		h = 200 - y;
	}
	const uint8 *src = getPagePtr(_curPage) + y * SCREEN_W + x * 8;
	while (h--) {
		memcpy(dst, src, w*8);
		dst += w*8;
		src += SCREEN_W;
	}
}

void Screen::shuffleScreen(int sx, int sy, int w, int h, int srcPage, int dstPage, int ticks, bool transparent) {
	debugC(9, kDebugLevelScreen, "Screen::shuffleScreen(%d, %d, %d, %d, %d, %d, %d, %d)", sx, sy, w, h, srcPage, dstPage, ticks, transparent);
	assert(sx >= 0 && w <= SCREEN_W);
	int x;
	uint16 x_offs[SCREEN_W];
	for (x = 0; x < SCREEN_W; ++x) {
		x_offs[x] = x;
	}
	for (x = 0; x < w; ++x) {
		int i = _vm->_rnd.getRandomNumber(w - 1);
		SWAP(x_offs[x], x_offs[i]);
	}
	
	assert(sy >= 0 && h <= SCREEN_H);
	int y;
	uint8 y_offs[SCREEN_H];
	for (y = 0; y < SCREEN_H; ++y) {
		y_offs[y] = y;
	}
	for (y = 0; y < h; ++y) {
		int i = _vm->_rnd.getRandomNumber(h - 1);
		SWAP(y_offs[y], y_offs[i]);
	}

	int32 start, now;
	int wait;
	for (y = 0; y < h; ++y) {
		start = (int32)_system->getMillis();
		int y_cur = y;
		for (x = 0; x < w; ++x) {
			int i = sx + x_offs[x];
			int j = sy + y_offs[y_cur];
			++y_cur;
			if (y_cur >= h) {
				y_cur = 0;
			}
			uint8 color = getPagePixel(srcPage, i, j);
			if (!transparent || color != 0) {
				setPagePixel(dstPage, i, j, color);
			}
		}
		updateScreen();
		now = (int32)_system->getMillis();
		wait = ticks * _vm->tickLength() - (now - start);
		if (wait > 0) {
			_vm->delay(wait);
		}
	}
}

void Screen::fillRect(int x1, int y1, int x2, int y2, uint8 color, int pageNum) {
	debugC(9, kDebugLevelScreen, "Screen::fillRect(%d, %d, %d, %d, %d, %d)", x1, y1, x2, y2, color, pageNum);
	assert(x2 < SCREEN_W && y2 < SCREEN_H);
	if (pageNum == -1) {
		pageNum = _curPage;
	}
	uint8 *dst = getPagePtr(pageNum) + y1 * SCREEN_W + x1;
	for (; y1 <= y2; ++y1) {
		memset(dst, color, x2 - x1 + 1);
		dst += SCREEN_W;
	}
}

void Screen::drawBox(int x1, int y1, int x2, int y2, int color) {
	debugC(9, kDebugLevelScreen, "Screen::drawBox(%i, %i, %i, %i, %i)", x1, y1, x2, y2, color);

	drawClippedLine(x1, y1, x2, y1, color);
	drawClippedLine(x1, y1, x1, y2, color);
	drawClippedLine(x2, y1, x2, y2, color);
	drawClippedLine(x1, y2, x2, y2, color);
}

void Screen::drawShadedBox(int x1, int y1, int x2, int y2, int color1, int color2) {
	debugC(9, kDebugLevelScreen, "Screen::drawShadedBox(%i, %i, %i, %i, %i, %i)", x1, y1, x2, y2, color1, color2);
	assert(x1 > 0 && y1 > 0);
	hideMouse();

	fillRect(x1, y1, x2, y1 + 1, color1);
	fillRect(x2 - 1, y1, x2, y2, color1);

	drawClippedLine(x1, y1, x1, y2, color2);
	drawClippedLine(x1 + 1, y1 + 1, x1 + 1, y2 - 1, color2);
	drawClippedLine(x1, y2, x2, y2, color2);
	drawClippedLine(x1, y2 - 1, x2 - 1, y2 - 1, color2);

	showMouse();
}

void Screen::drawClippedLine(int x1, int y1, int x2, int y2, int color) {
	debugC(9, kDebugLevelScreen, "Screen::drawClippedLine(%i, %i, %i, %i, %i)", x1, y1, x2, y2, color);

	if (x1 < 0)
		x1 = 0;
	else if (x1 > 319)
		x1 = 319;

	if (x2 < 0)
		x2 = 0;
	else if (x2 > 319)
		x2 = 319;

	if (y1 < 0)
		y1 = 0;
	else if (y1 > 199)
		y1 = 199;

	if (y2 < 0)
		y2 = 0;
	else if (y2 > 199)
		y2 = 199;

	if (x1 == x2)
		if (y1 > y2)
			drawLine(true, x1, y2, y1 - y2 + 1, color);
		else
			drawLine(true, x1, y1, y2 - y1 + 1, color);
	else
		if (x1 > x2)
			drawLine(false, x2, y1, x1 - x2 + 1, color);
		else
			drawLine(false, x1, y1, x2 - x1 + 1, color);
}

void Screen::drawLine(bool horizontal, int x, int y, int length, int color) {
	debugC(9, kDebugLevelScreen, "Screen::drawLine(%i, %i, %i, %i, %i)", horizontal, x, y, length, color);

	uint8 *ptr = getPagePtr(_curPage) + y * SCREEN_W + x;

	if (horizontal) {
		assert((y + length) <= SCREEN_H);
		int currLine = 0;
		while (currLine < length) {
			*ptr = color;
			ptr += SCREEN_W;
			currLine++;
		}
	} else {
		assert((x + length) <= SCREEN_W);
		memset(ptr, color, length);
	}
}

void Screen::setAnimBlockPtr(int size) {
	debugC(9, kDebugLevelScreen, "Screen::setAnimBlockPtr(%d)", size);
	free(_animBlockPtr);
	_animBlockPtr = (uint8 *)malloc(size);
	assert(_animBlockPtr);
	memset(_animBlockPtr, 0, size);
	_animBlockSize = size;
}

void Screen::setTextColorMap(const uint8 *cmap) {
	debugC(9, kDebugLevelScreen, "Screen::setTextColorMap(%p)", (const void *)cmap);
	setTextColor(cmap, 0, 11);
}

void Screen::setTextColor(const uint8 *cmap, int a, int b) {
	debugC(9, kDebugLevelScreen, "Screen::setTextColor(%p, %d, %d)", (const void *)cmap, a, b);
	for (int i = a; i <= b; ++i) {
		_textColorsMap[i] = *cmap++;
	}
}

void Screen::loadFont(FontId fontId, uint8 *fontData) {
	debugC(9, kDebugLevelScreen, "Screen::loadFont(%d, %p)", fontId, (const void *)fontData);
	Font *fnt = &_fonts[fontId];
	assert(fontData && !fnt->fontData);
	fnt->fontData = fontData;
	uint16 fontSig = READ_LE_UINT16(fontData + 2);
	if (fontSig != 0x500) {
		error("Invalid font data");
	}
	fnt->charWidthTable = fontData + READ_LE_UINT16(fontData + 8);
	fnt->charBoxHeight = READ_LE_UINT16(fontData + 4);
	fnt->charBitmapOffset = READ_LE_UINT16(fontData + 6);
	fnt->charWidthTableOffset = READ_LE_UINT16(fontData + 8);
	fnt->charHeightTableOffset = READ_LE_UINT16(fontData + 0xC);
}

Screen::FontId Screen::setFont(FontId fontId) {
	debugC(9, kDebugLevelScreen, "Screen::setFont(%d)", fontId);
	FontId prev = _currentFont;
	_currentFont = fontId;
	return prev;
}

int Screen::getCharWidth(uint8 c) const {
	debugC(9, kDebugLevelScreen, "Screen::getCharWidth('%c')", c);
	return (int)_fonts[_currentFont].charWidthTable[c] + _charWidth;
}

int Screen::getTextWidth(const char *str) const {
	debugC(9, kDebugLevelScreen, "Screen::getTextWidth('%s')", str);
	int curLineLen = 0;
	int maxLineLen = 0;
	while (1) {
		char c = *str++;
		if (c == 0) {
			break;
		} else if (c == '\r') {
			if (curLineLen > maxLineLen) {
				maxLineLen = curLineLen;
			} else {
				curLineLen = 0;
			}
		} else {
			curLineLen += getCharWidth(c);
		}
	}
	return MAX(curLineLen, maxLineLen);
}

void Screen::printText(const char *str, int x, int y, uint8 color1, uint8 color2) {
	debugC(9, kDebugLevelScreen, "Screen::printText('%s', %d, %d, 0x%X, 0x%X)", str, x, y, color1, color2);
	uint8 cmap[2];
	cmap[0] = color2;
	cmap[1] = color1;
	setTextColor(cmap, 0, 1);
	
	Font *fnt = &_fonts[_currentFont];
	uint8 charHeight = *(fnt->fontData + fnt->charBoxHeight + 4);
	
	if (x < 0) {
		x = 0;
	} else if (x >= SCREEN_W) {
		return;
	}
	int x_start = x;
	if (y < 0) {
		y = 0;
	} else if (y >= SCREEN_H) {
		return;
	}

	while (1) {
		char c = *str++;
		if (c == 0) {
			break;
		} else if (c == '\r') {
			x = x_start;
			y += charHeight + _charOffset;
		} else {
			int charWidth = getCharWidth(c);
			if (x + charWidth > SCREEN_W) {
				x = x_start;
				y += charHeight + _charOffset;
				if (y >= SCREEN_H) {
					break;
				}
			}
			drawChar(c, x, y);
			x += charWidth;
		}
	}
}

void Screen::drawChar(uint8 c, int x, int y) {
	debugC(9, kDebugLevelScreen, "Screen::drawChar('%c', %d, %d)", c, x, y);
	Font *fnt = &_fonts[_currentFont];
	uint8 *dst = getPagePtr(_curPage) + y * SCREEN_W + x;
	uint16 bitmapOffset = READ_LE_UINT16(fnt->fontData + fnt->charBitmapOffset + c * 2);
	if (bitmapOffset == 0) {
		return;
	}
	uint8 charWidth = *(fnt->fontData + fnt->charWidthTableOffset + c);
	if (charWidth + x > SCREEN_W) {
		return;
	}
	uint8 charH0 = *(fnt->fontData + fnt->charBoxHeight + 4);
	if (charH0 + y > SCREEN_H) {
		return;
	}
	uint8 charH1 = *(fnt->fontData + fnt->charHeightTableOffset + c * 2);
	uint8 charH2 = *(fnt->fontData + fnt->charHeightTableOffset + c * 2 + 1);
	charH0 -= charH1 + charH2;
	
	const uint8 *src = fnt->fontData + bitmapOffset;
	const int pitch = SCREEN_W - charWidth;
	
	while (charH1--) {
		uint8 col = _textColorsMap[0];
		for (int i = 0; i < charWidth; ++i) {
			if (col != 0) {
				*dst = col;
			}
			++dst;
		}
		dst += pitch;
	}
	
	while (charH2--) {
		uint8 b = 0;
		for (int i = 0; i < charWidth; ++i) {
			uint8 col;
			if (i & 1) {
				col = _textColorsMap[b >> 4];
			} else {
				b = *src++;
				col = _textColorsMap[b & 0xF];
			}
			if (col != 0) {
				*dst = col;
			}
			++dst;
		}
		dst += pitch;
	}
	
	while (charH0--) {
		uint8 col = _textColorsMap[0];
		for (int i = 0; i < charWidth; ++i) {
			if (col != 0) {
				*dst = col;
			}
			++dst;
		}
		dst += pitch;
	}
}

void Screen::setScreenDim(int dim) {
	debugC(9, kDebugLevelScreen, "setScreenDim(%d)", dim);
	assert(dim < _screenDimTableCount);
	_curDim = &_screenDimTable[dim];
	// XXX
}

void Screen::drawShape(uint8 pageNum, const uint8 *shapeData, int x, int y, int sd, int flags, ...) {
	debugC(9, kDebugLevelScreen, "Screen::drawShape(%d, %p, %d, %d, %d, 0x%.04X, ...)", pageNum, (const void *)shapeData, x, y, sd, flags);
	if (!shapeData)
		return;
	va_list args;
	va_start(args, flags);
	
	static int drawShapeVar1 = 0;
	static int drawShapeVar2[] = {
		1, 3, 2, 5, 4, 3, 2, 1
	};
	static int drawShapeVar3 = 1;
	static int drawShapeVar4 = 0;
	static int drawShapeVar5 = 0;
	
	uint8 *table = 0;
	int tableLoopCount = 0;
	int drawLayer = 0;
	uint8 *table2 = 0;
	uint8 *table3 = 0;
	uint8 *table4 = 0;
	
	if (flags & 0x8000) {
		table2 = va_arg(args, uint8*);
	}
	if (flags & 0x100) {
		table = va_arg(args, uint8*);
		tableLoopCount = va_arg(args, int);
		if (!tableLoopCount)
			flags &= 0xFFFFFEFF;
	}
	if (flags & 0x1000) {
		table3 = va_arg(args, uint8*);
		table4 = va_arg(args, uint8*);
	}
	if (flags & 0x200) {
		drawShapeVar1 += 1;
		drawShapeVar1 &= 7;
		drawShapeVar3 = drawShapeVar2[drawShapeVar1];
		drawShapeVar4 = 0;
		drawShapeVar5 = 256;
	}
	if (flags & 0x4000) {
		drawShapeVar5 = va_arg(args, int);
	}
	if (flags & 0x800) {
		drawLayer = va_arg(args, int);
	}
	int scale_w, scale_h;
	if (flags & DSF_SCALE) {
		scale_w = va_arg(args, int);
		scale_h = va_arg(args, int);
	} else {
		scale_w = 0x100;
		scale_h = 0x100;
	}
	
	int ppc = (flags >> 8) & 0x3F;
	
	const uint8 *src = shapeData;
	if (_vm->features() & GF_TALKIE) {
		src += 2;
	}
	uint16 shapeFlags = READ_LE_UINT16(src); src += 2;
	
	int shapeHeight = *src++;
	int scaledShapeHeight = (shapeHeight * scale_h) >> 8;
	if (scaledShapeHeight == 0) {
		va_end(args);
		return;
	}

	int shapeWidth = READ_LE_UINT16(src); src += 2;
	int scaledShapeWidth = (shapeWidth * scale_w) >> 8;
	if (scaledShapeWidth == 0) {
		va_end(args);
		return;
	}

	if (flags & DSF_CENTER) {
		x -= scaledShapeWidth >> 1;
		y -= scaledShapeHeight >> 1;
	}
	
	src += 3;
	
	uint16 frameSize = READ_LE_UINT16(src); src += 2;
	if ((shapeFlags & 1) || (flags & 0x400)) {
		src += 0x10;
	}
	if (!(shapeFlags & 2)) {
		decodeFrame4(src, _animBlockPtr, frameSize);
		src = _animBlockPtr;
	}
	
	int shapeSize = shapeWidth * shapeHeight;
	if (_decodeShapeBufferSize < shapeSize) {
		free(_decodeShapeBuffer);
		_decodeShapeBuffer = (uint8 *)malloc(shapeSize);
		_decodeShapeBufferSize = shapeSize;
	}
	if (!_decodeShapeBuffer) {
		_decodeShapeBufferSize = 0;
		va_end(args);
		return;
	}
	memset(_decodeShapeBuffer, 0, _decodeShapeBufferSize);
	uint8 *decodedShapeFrame = _decodeShapeBuffer;
	
	// only used if shapeFlag & 1 is NOT zero
	const uint8 *colorTable = shapeData + 10;
	if (_vm->features() & GF_TALKIE) {
		colorTable += 2;
	}
	
	for (int j = 0; j < shapeHeight; ++j) {
		uint8 *dsbNextLine = decodedShapeFrame + shapeWidth;
		int count = shapeWidth;
		while (count > 0) {
			uint8 code = *src++;
			if (code != 0) {
				// this is guessed
				if (shapeFlags & 1) {
					if (code < 16) {
						*decodedShapeFrame++ = colorTable[code];
					}
				} else {
					*decodedShapeFrame++ = code;
				}
				--count;
			} else {
				code = *src++;
				decodedShapeFrame += code;
				count -= code;
			}
		}
		decodedShapeFrame = dsbNextLine;
	}
	
	uint16 sx1 = _screenDimTable[sd].sx * 8;
	uint16 sy1 = _screenDimTable[sd].sy;
	uint16 sx2 = sx1 + _screenDimTable[sd].w * 8;
	uint16 sy2 = sy1 + _screenDimTable[sd].h;
	if (flags & DSF_WND_COORDS) {
		x += sx1;
		y += sy1;
	}
	
	int x1, x2;
	if (x >= 0) {
		x1 = 0;
		if (x + scaledShapeWidth < sx2) {
			x2 = scaledShapeWidth;
		} else {
			x2 = sx2 - x;
		}
	} else {
		x2 = scaledShapeWidth;
		x1 = -x;
		x = 0;
		if (x2 > sx2) {
			x2 = sx2;
		}
	}
	
	int y1, y2;
	if (y >= 0) {
		y1 = 0;
		if (y + scaledShapeHeight < sy2) {
			y2 = scaledShapeHeight;
		} else {
			y2 = sy2 - y;
		}
	} else {
		y2 = scaledShapeHeight;
		y1 = -y;
		y = 0;
		if (y2 > sy2) {
			y2 = sy2;
		}
	}

	uint8 *dst = getPagePtr(pageNum) + y * SCREEN_W + x;
	uint8 *dstStart = getPagePtr(pageNum);
	
	int scaleYTable[SCREEN_H];
	assert(y1 >= 0 && y2 < SCREEN_H);
	for (y = y1; y < y2; ++y) {
		scaleYTable[y] = (y << 8) / scale_h;
	}
	int scaleXTable[SCREEN_W];
	assert(x1 >= 0 && x2 < SCREEN_W);
	for (x = x1; x < x2; ++x) {
		scaleXTable[x] = (x << 8) / scale_w;
	}
	
	const uint8 *shapeBuffer = _decodeShapeBuffer;
	if (flags & DSF_Y_FLIPPED) {
		shapeBuffer += shapeWidth * (shapeHeight - 1);
	}
	if (flags & DSF_X_FLIPPED) {
		shapeBuffer += shapeWidth - 1;
	}
	
	for (y = y1; y < y2; ++y) {
		uint8 *dstNextLine = dst + SCREEN_W;
		int j = scaleYTable[y];
		if (flags & DSF_Y_FLIPPED) {
			j = -j;
		}
		for (x = x1; x < x2; ++x) {
			int xpos = scaleXTable[x];
			if (flags & DSF_X_FLIPPED) {
				xpos = -xpos;
			}
			uint8 color = shapeBuffer[j * shapeWidth + xpos];
			if (color != 0) {
				switch (ppc) {
					case 0:
						*dst = color;
						break;

					case 1:
						for (int i = 0; i < tableLoopCount; ++i) {
							color = table[color];
						}
						break;
						
					case 2: {
						int temp = drawShapeVar4 + drawShapeVar5;
						if (temp & 0xFF00) {
							drawShapeVar4 = temp & 0xFF;
							dst += drawShapeVar3;
							color = *dst;
							dst -= drawShapeVar3;
						} else {
							drawShapeVar4 = temp;
						}
					}	break;
					
					case 7:
					case 3:
						color = *dst;
						for (int i = 0; i < tableLoopCount; ++i) {
							color = table[color];
						}
						break;
						
					case 4:
						color = table2[color];
						break;
						
					case 5:
						color = table2[color];
						for (int i = 0; i < tableLoopCount; ++i) {
							color = table[color];
						}
						break;
						
					case 6: {
						int temp = drawShapeVar4 + drawShapeVar5;
						if (temp & 0xFF00) {
							drawShapeVar4 = temp & 0xFF;
							dst += drawShapeVar3;
							color = *dst;
							dst -= drawShapeVar3;
						} else {
							drawShapeVar4 = temp;
							color = table2[color];
						}
					}	break;
						
					case 8: {
						int offset = dst - dstStart;
						uint8 pixel = *(_shapePages[0] + offset);
						pixel &= 0x7F;
						pixel &= 0x87;
						if (drawLayer < pixel) {
							color = *(_shapePages[1] + offset);
						}
					}	break;
					
					case 9: {
						int offset = dst - dstStart;
						uint8 pixel = *(_shapePages[0] + offset);
						pixel &= 0x7F;
						pixel &= 0x87;
						if (drawLayer < pixel) {
							color = *(_shapePages[1] + offset);
						} else {
							for (int i = 0; i < tableLoopCount; ++i) {
								color = table[color];
							}
						}
					}	break;
					
					case 10: {
						int offset = dst - dstStart;
						uint8 pixel = *(_shapePages[0] + offset);
						pixel &= 0x7F;
						pixel &= 0x87;
						if (drawLayer < pixel) {
							color = *(_shapePages[1] + offset);
							drawShapeVar4 = pixel;
						} else {
							int temp = drawShapeVar4 + drawShapeVar5;
							if (temp & 0xFF00) {
								dst += drawShapeVar3;
								color = *dst;
								dst -= drawShapeVar3;
							}
							drawShapeVar4 = temp & 0xFF;
						}
					}	break;
					
					case 15:
					case 11: {
						int offset = dst - dstStart;
						uint8 pixel = *(_shapePages[0] + offset);
						pixel &= 0x7F;
						pixel &= 0x87;
						if (drawLayer < pixel) {
							color = *(_shapePages[1] + offset);
						} else {
							color = *dst;
							for (int i = 0; i < tableLoopCount; ++i) {
								color = table[color];
							}
						}
					}	break;
					
					case 12: {
						int offset = dst - dstStart;
						uint8 pixel = *(_shapePages[0] + offset);
						pixel &= 0x7F;
						pixel &= 0x87;
						if (drawLayer < pixel) {
							color = *(_shapePages[1] + offset);
						} else {
							color = table2[color];
						}
					}	break;
					
					case 13: {
						int offset = dst - dstStart;
						uint8 pixel = *(_shapePages[0] + offset);
						pixel &= 0x7F;
						pixel &= 0x87;
						if (drawLayer < pixel) {
							color = *(_shapePages[1] + offset);
						} else {
							color = table2[color];
							for (int i = 0; i < tableLoopCount; ++i) {
								color = table[color];
							}
						}
					}	break;
					
					case 14: {
						int offset = dst - dstStart;
						uint8 pixel = *(_shapePages[0] + offset);
						pixel &= 0x7F;
						pixel &= 0x87;
						if (drawLayer < pixel) {
							color = *(_shapePages[1] + offset);
							drawShapeVar4 = pixel;
						} else {
							int temp = drawShapeVar4 + drawShapeVar5;
							if (temp & 0xFF00) {
								dst += drawShapeVar3;
								color = *dst;
								dst -= drawShapeVar3;
								drawShapeVar4 = temp % 0xFF;
							} else {
								drawShapeVar4 = temp;
								color = table2[color];
							}
						}
					}	break;
					
					case 16: {
						uint8 newColor = table3[color];
						if (!(newColor & 0x80)) {
							color = *dst;
							color = table4[color + (newColor << 8)];
						}
					}	break;
					
					case 17: {
						for (int i = 0; i < tableLoopCount; ++i) {
							color = table[color];
						}
						uint8 newColor = table3[color];
						if (!(newColor & 0x80)) {
							color = *dst;
							color = table4[color + (newColor << 8)];
						}
					}	break;
					
					case 18: {
						int temp = drawShapeVar4 + drawShapeVar5;
						if (temp & 0xFF00) {
							drawShapeVar4 = temp & 0xFF;
							dst += drawShapeVar3;
							color = *dst;
							dst -= drawShapeVar3;
							uint8 newColor = table3[color];
							if (!(newColor & 0x80)) {
								color = *dst;
								color = table4[color + (newColor << 8)];
							}
						} else {
							drawShapeVar4 = temp;
						}
					}	break;
					
					case 23:
					case 19: {
						color = *dst;
						for (int i = 0; i < tableLoopCount; ++i) {
							color = table[color];
						}
						uint8 newColor = table3[color];
						if (!(newColor & 0x80)) {
							color = *dst;
							color = table4[color + (newColor << 8)];
						}
					}	break;
					
					case 20: {
						color = table2[color];
						uint8 newColor = table3[color];
						if (!(newColor & 0x80)) {
							color = *dst;
							color = table4[color + (newColor << 8)];
						}
					}	break;
					
					case 21: {
						color = table2[color];
						for (int i = 0; i < tableLoopCount; ++i) {
							color = table[color];
						}
						uint8 newColor = table3[color];
						if (!(newColor & 0x80)) {
							color = *dst;
							color = table4[color + (newColor << 8)];
						}
					}	break;
					
					case 22: {
						int temp = drawShapeVar4 + drawShapeVar5;
						if (temp & 0xFF00) {
							drawShapeVar4 = temp & 0xFF;
							dst += drawShapeVar3;
							color = *dst;
							dst -= drawShapeVar3;
							uint8 newColor = table3[color];
							if (!(newColor & 0x80)) {
								color = *dst;
								color = table4[color + (newColor << 8)];
							}
						} else {
							drawShapeVar4 = temp;
							color = table2[color];
							uint8 newColor = table3[color];
							if (!(newColor & 0x80)) {
								color = *dst;
								color = table4[color + (newColor << 8)];
							}
						}
					}	break;
					
					case 24: {
						int offset = dst - dstStart;
						uint8 pixel = *(_shapePages[0] + offset);
						pixel &= 0x7F;
						pixel &= 0x87;
						if (drawLayer < pixel) {
							color = *(_shapePages[1] + offset);
						}
						uint8 newColor = table3[color];
						if (!(newColor & 0x80)) {
							color = *dst;
							color = table4[color + (newColor << 8)];
						}
					}	break;
					
					default:
						warning("unhandled ppc: %d", ppc);
						break;
				}
				*dst = color;
			}
			++dst;
		}
		dst = dstNextLine;
	}
	va_end(args);
}

void Screen::decodeFrame3(const uint8 *src, uint8 *dst, uint32 size) {
	debugC(9, kDebugLevelScreen, "Screen::decodeFrame3(%p, %p, %d)", (const void *)src, (const void *)dst, size);
	const uint8 *dstEnd = dst + size;
	while (dst < dstEnd) {
		int8 code = *src++;
		if (code == 0) {
			uint16 sz = READ_BE_UINT16(src);
			src += 2;
			memset(dst, *src++, sz);
			dst += sz;
		} else if (code < 0) {
			memset(dst, *src++, -code);
			dst -= code;
		} else {
			memcpy(dst, src, code);
			dst += code;
			src += code;
		}
	}
}

void Screen::decodeFrame4(const uint8 *src, uint8 *dst, uint32 dstSize) {
	debugC(9, kDebugLevelScreen, "Screen::decodeFrame4(%p, %p, %d)", (const void *)src, (const void *)dst, dstSize);
	uint8 *dstOrig = dst;
	uint8 *dstEnd = dst + dstSize;
	while (1) {
		int count = dstEnd - dst;
		if (count == 0) {
			break;
		}
		uint8 code = *src++;
		if (!(code & 0x80)) {
			int len = MIN(count, (code >> 4) + 3);
			int offs = ((code & 0xF) << 8) | *src++;
			const uint8 *dstOffs = dst - offs;
			while (len--) {
				*dst++ = *dstOffs++;
			}
		} else if (code & 0x40) {
			int len = (code & 0x3F) + 3;
			if (code == 0xFE) {
				len = READ_LE_UINT16(src); src += 2;
				if (len > count) {
					len = count;
				}
				memset(dst, *src++, len); dst += len;
			} else {
				if (code == 0xFF) {
					len = READ_LE_UINT16(src); src += 2;
				}
				int offs = READ_LE_UINT16(src); src += 2;
				if (len > count) {
					len = count;
				}
				const uint8 *dstOffs = dstOrig + offs;
				while (len--) {
					*dst++ = *dstOffs++;
				}
			}
		} else if (code != 0x80) {
			int len = MIN(count, code & 0x3F);
			while (len--) {
				*dst++ = *src++;
			}
		} else {
			break;
		}
	}
}

void Screen::decodeFrameDelta(uint8 *dst, const uint8 *src) {
	debugC(9, kDebugLevelScreen, "Screen::decodeFrameDelta(%p, %p)", (const void *)dst, (const void *)src);
	while (1) {
		uint8 code = *src++;
		if (code == 0) {
			uint8 len = *src++;
			code = *src++;
			while (len--) {
				*dst++ ^= code;
			}
		} else if (code & 0x80) {
			code -= 0x80;
			if (code != 0) {
				dst += code;
			} else {
				uint16 subcode = READ_LE_UINT16(src); src += 2;
				if (subcode == 0) {
					break;
				} else if (subcode & 0x8000) {
					subcode -= 0x8000;
					if (subcode & 0x4000) {
						uint16 len = subcode - 0x4000;
						code = *src++;
						while (len--) {
							*dst++ ^= code;
						}
					} else {
						while (subcode--) {
							*dst++ ^= *src++;
						}
					}
				} else {
					dst += subcode;
				}
			}
		} else {
			while (code--) {
				*dst++ ^= *src++;
			}
		}
	}
}

void Screen::decodeFrameDeltaPage(uint8 *dst, const uint8 *src, int pitch, int noXor) {
	debugC(9, kDebugLevelScreen, "Screen::decodeFrameDeltaPage(%p, %p, %d, %d)", (const void *)dst, (const void *)src, pitch, noXor);
	int count = 0;
	uint8 *dstNext = dst;
	while (1) {
		uint8 code = *src++;
		if (code == 0) {
			uint8 len = *src++;
			code = *src++;
			while (len--) {
				if (noXor) {
					*dst++ = code;
				} else {
					*dst++ ^= code;
				}
				if (++count == pitch) {
					count = 0;
					dstNext += SCREEN_W;
					dst = dstNext;
				}
			}
		} else if (code & 0x80) {
			code -= 0x80;
			if (code != 0) {
				dst += code;
				
				count += code;
				while (count >= pitch) {
					count -= pitch;
					dstNext += SCREEN_W;
					dst = dstNext + count;
				}
			} else {
				uint16 subcode = READ_LE_UINT16(src); src += 2;
				if (subcode == 0) {
					break;
				} else if (subcode & 0x8000) {
					subcode -= 0x8000;
					if (subcode & 0x4000) {
						uint16 len = subcode - 0x4000;
						code = *src++;
						while (len--) {
							if (noXor) {
								*dst++ = code;
							} else {
								*dst++ ^= code;
							}
							if (++count == pitch) {
								count = 0;
								dstNext += SCREEN_W;
								dst = dstNext;
							}
						}
					} else {
						while (subcode--) {
							if (noXor) {
								*dst++ = *src++;
							} else {
								*dst++ ^= *src++;
							}
							if (++count == pitch) {
								count = 0;
								dstNext += SCREEN_W;
								dst = dstNext;
							}
						}
					}
				} else {
					dst += subcode;
					
					count += subcode;
					while (count >= pitch) {
						count -= pitch;
						dstNext += SCREEN_W;
						dst = dstNext + count;
					}
					
				}
			}
		} else {
			while (code--) {
				if (noXor) {
					*dst++ = *src++;
				} else {
					*dst++ ^= *src++;
				}
				if (++count == pitch) {
					count = 0;
					dstNext += SCREEN_W;
					dst = dstNext;
				}
			}
		}
	}
}

uint8 *Screen::encodeShape(int x, int y, int w, int h, int flags) {
	debugC(9, kDebugLevelScreen, "Screen::encodeShape(%d, %d, %d, %d, %d)", x, y, w, h, flags);
	uint8 *srcPtr = &_pagePtrs[_curPage][y * SCREEN_W + x];
	int16 shapeSize = 0;
	uint8 *tmp = srcPtr;
	int xpos = w;

	for (int i = h; i > 0; --i) {
		uint8 *start = tmp;
		shapeSize += w;
		xpos = w;
		while (xpos) {
			uint8 value = *tmp++;
			--xpos;
			
			if (!value) {
				shapeSize += 2;
				int16 curX = xpos;
				bool skip = false;
				
				while (xpos) {
					value = *tmp++;
					--xpos;
					
					if (value) {
						skip = true;
						break;
					}
				}
				
				if (!skip)
					++curX;
					
				curX -= xpos;
				shapeSize -= curX;
				
				while (curX > 0xFF) {
					curX -= 0xFF;
					shapeSize += 2;
				}
			}
		}
	
		tmp = start + SCREEN_W;
	}
	
	int16 shapeSize2 = shapeSize;
	if (_vm->features() & GF_TALKIE) {
		shapeSize += 12;
	} else {
		shapeSize += 10;
	}
	if (flags & 1)
		shapeSize += 16;
	
	static uint8 table[274];	
	int tableIndex = 0;
	
	uint8 *newShape = NULL;
	newShape = (uint8*)malloc(shapeSize+16);
	assert(newShape);
	
	byte *dst = newShape;
	if (_vm->features() & GF_TALKIE)
		dst += 2;
	WRITE_LE_UINT16(dst, (flags & 3)); dst += 2;
	*dst = h; dst += 1;
	WRITE_LE_UINT16(dst, w); dst += 2;
	*dst = h; dst += 1;
	WRITE_LE_UINT16(dst, shapeSize); dst += 2;
	WRITE_LE_UINT16(dst, shapeSize2); dst += 2;

	byte *src = srcPtr;
	if (flags & 1) {
		dst += 16;
		memset(table, 0, sizeof(uint8)*274);
		tableIndex = 1;
	}
	
	for (int ypos = h; ypos > 0; --ypos) {
		uint8 *srcBackUp = src;
		xpos = w;
		while (xpos) {
			uint8 value = *src++;
			if (value) {
				if (flags & 1) {
					if (!table[value]) {
						if (tableIndex == 16) {
							value = 1;
						} else {
							table[0x100+tableIndex] = value;
							table[value] = tableIndex;
							++tableIndex;
							value = table[value];
						}
					} else {
						value = table[value];
					}
				}
				--xpos;
				*dst++ = value;
			} else {
				int16 temp = 1;
				--xpos;
				
				while (xpos) {
					if (*src)
						break;
					++src;
					++temp;
					--xpos;
				}
				
				while (temp > 0xFF) {
					*dst++ = 0;
					*dst++ = 0xFF;
					temp -= 0xFF;
				}
				
				if (temp & 0xFF) {
					*dst++ = 0;
					*dst++ = temp & 0xFF;
				}
			}
		}
		src = srcBackUp + SCREEN_W;
	}
	
	if (!(flags & 2)) {
		if (shapeSize > _animBlockSize) {
			dst = newShape;
			if (_vm->features() & GF_TALKIE) {
				dst += 2;
			}
			flags = READ_LE_UINT16(dst);
			flags |= 2;
			WRITE_LE_UINT16(dst, flags);
		} else {
			src = newShape;
			if (_vm->features() & GF_TALKIE) {
				src += 2;
			}
			if (flags & 1) {
				src += 16;
			}
			src += 10;
			uint8 *shapePtrBackUp = src;
			dst = _animBlockPtr;
			memcpy(dst, src, shapeSize2);
			
			int16 size = encodeShapeAndCalculateSize(_animBlockPtr, shapePtrBackUp, shapeSize2);
			if (size > shapeSize2) {
				shapeSize -= shapeSize2 - size;
				newShape = (uint8*)realloc(newShape, shapeSize);
				assert(newShape);
			} else {
				dst = shapePtrBackUp;
				src = _animBlockPtr;
				memcpy(dst, src, shapeSize2);
				dst = newShape;
				flags = READ_LE_UINT16(dst);
				flags |= 2;
				WRITE_LE_UINT16(dst, flags);
			}
		}
	}
	
	dst = newShape;
	if (_vm->features() & GF_TALKIE) {
		dst += 2;
	}
	WRITE_LE_UINT16((dst + 6), shapeSize);
	
	if (flags & 1) {
		dst = newShape + 10;
		if (_vm->features() & GF_TALKIE) {
			dst += 2;
		}
		src = &table[0x100];
		memcpy(dst, src, sizeof(uint8)*16);
	}
	
	return newShape;
}

int16 Screen::encodeShapeAndCalculateSize(uint8 *from, uint8 *to, int size_to) {
	debugC(9, kDebugLevelScreen, "Screen::encodeShapeAndCalculateSize(%p, %p, %d)", (const void *)from, (const void *)to, size_to);
	byte *fromPtrEnd = from + size_to;
	bool skipPixel = true;
	byte *tempPtr = 0;
	byte *toPtr = to;
	byte *fromPtr = from;
	byte *toPtr2 = to;
	
	*to++ = 0x81;
	*to++ = *from++;
	
	while (from < fromPtrEnd) {
		byte *curToPtr = to;
		to = fromPtr;
		int size = 1;
		
		while (true) {
			byte curPixel = *from;
			if (curPixel == *(from+0x40)) {
				byte *toBackUp = to;
				to = from;
				
				for (int i = 0; i < (fromPtrEnd - from); ++i) {
					if (*to++ != curPixel)
						break;
				}
				--to;
				uint16 diffSize = (to - from);
				if (diffSize >= 0x41) {
					skipPixel = false;
					from = to;
					to = curToPtr;
					*to++ = 0xFE;
					WRITE_LE_UINT16(to, diffSize); to += 2;
					*to++ = (size >> 8) & 0xFF;
					curToPtr = to;
					to = toBackUp;
					continue;
				} else {
					to = toBackUp;
				}
			}
			
			bool breakLoop = false;
			while (true) {
				if ((from - to) == 0) {
					breakLoop = true;
					break;
				}
				for (int i = 0; i < (from - to); ++i) {
					if (*to++ == curPixel)
						break;
				}
				if (*to == curPixel) {
					if (*(from+size-1) == *(to+size-2))
						break;
					
					byte *fromBackUp = from;
					byte *toBackUp = to;
					--to;
					for (int i = 0; i < (fromPtrEnd - from); ++i) {
						if (*from++ != *to++)
							break;
					}
					if (*(from - 1) == *(to - 1))
						++to;
					from = fromBackUp;
					int temp = to - toBackUp;
					to = toBackUp;
					if (temp >= size) {
						size = temp;
						tempPtr = toBackUp - 1;
					}
					break;
				} else {
					breakLoop = true;
					break;
				}
			}
			
			if (breakLoop)
				break;
		}
		
		to = curToPtr;
		if (size > 2) {
			uint16 word = 0;
			if (size <= 0x0A) {
				uint16 diffSize = from - tempPtr;
				if (size <= 0x0FFF) {
					byte highByte = ((diffSize & 0xFF00) >> 8) + (((size & 0xFF) - 3) << 4);
					word = ((diffSize & 0xFF) << 8) | highByte;
					WRITE_LE_UINT16(to, word); to += 2;
					from += size;
					skipPixel = false;
					continue;
				}
			}
			
			if (size > 0x40) {
				*to++ = 0xFF;
				WRITE_LE_UINT16(to, size); to += 2;
			} else {
				*to++ = ((size & 0xFF) - 3) | 0xC0;
			}
			
			word = tempPtr - fromPtr;
			WRITE_LE_UINT16(to, word); to += 2;
			from += size;
			skipPixel = false;
		} else {
			if (!skipPixel) {
				toPtr2 = to;
				*to++ = 0x80;
			}
			
			if (*toPtr2 == 0xBF) {
				toPtr2 = to;
				*to++ = 0x80;
			}
			
			++(*toPtr2);
			*to++ = *from++;
			skipPixel = true;			
		}
	}
	*to++ = 0x80;
	
	return (to - toPtr);
}

int Screen::getRectSize(int x, int y) {
	if (x < 1) {
		x = 1;
	} else if (x > 40) {
		x = 40;
	}
	
	if (y < 1) {
		y = 1;
	} else if (y > 200) {
		y = 200;
	}
	
	return ((x*y) << 3);
}

void Screen::hideMouse() {
	debugC(9, kDebugLevelScreen, "Screen::hideMouse()");
	++_mouseLockCount;
	_system->showMouse(false);
}

void Screen::showMouse() {
	debugC(9, kDebugLevelScreen, "Screen::showMouse()");

	if (_mouseLockCount == 1) {
		// HACK: Flush the event queue before showing the mouse, so
		//       that the mouse cursor won't do a little dance catching
		//       up with the mouse movements when entering a new room.
		_vm->delay(0);
		_system->showMouse(true);
	}

	if (_mouseLockCount > 0)
		_mouseLockCount--;

}

void Screen::setShapePages(int page1, int page2) {
	debugC(9, kDebugLevelScreen, "Screen::setShapePages(%d, %d)", page1, page2);
	_shapePages[0] = _pagePtrs[page1];
	_shapePages[1] = _pagePtrs[page2];
}

void Screen::setMouseCursor(int x, int y, byte *shape) {
	debugC(9, kDebugLevelScreen, "Screen::setMouseCursor(%d, %d, %p)", x, y, (const void *)shape);
	if (!shape)
		return;
	// if mouseDisabled
	//	return _mouseShape

	if (_vm->features() & GF_TALKIE)
		shape += 2;

	int mouseHeight = *(shape+2);
	int mouseWidth = (READ_LE_UINT16(shape + 3)) + 2;

	if (_vm->features() & GF_TALKIE)
		shape -= 2;

	uint8 *cursor = (uint8 *)malloc(mouseHeight * mouseWidth);
	fillRect(0, 0, mouseWidth, mouseHeight, 0, 8);
	drawShape(8, shape, 0, 0, 0, 0);

	_system->showMouse(false);
	copyRegionToBuffer(8, 0, 0, mouseWidth, mouseHeight, cursor);
	_system->setMouseCursor(cursor, mouseWidth, mouseHeight, x, y, 0);
	_system->showMouse(true);
	free(cursor);

	return;

}

void Screen::copyScreenFromRect(int x, int y, int w, int h, uint8 *ptr) {
	debugC(9, kDebugLevelScreen, "Screen::copyScreenFromRect(%d, %d, %d, %d, %p)", x, y, w, h, (const void *)ptr);
	x <<= 3; w <<= 3;
	uint8 *src = ptr;
	uint8 *dst = &_pagePtrs[0][y * SCREEN_W + x];
	for (int i = 0; i < h; ++i) {
		memcpy(dst, src, w);
		src += w;
		dst += SCREEN_W;
	}
}

void Screen::copyScreenToRect(int x, int y, int w, int h, uint8 *ptr) {
	debugC(9, kDebugLevelScreen, "Screen::copyScreenToRect(%d, %d, %d, %d, %p)", x, y, w, h, (const void *)ptr);
	x <<= 3; w <<= 3;
	uint8 *src = &_pagePtrs[0][y * SCREEN_W + x];
	uint8 *dst = ptr;
	for (int i = 0; i < h; ++i) {
		memcpy(dst, src, w);
		dst += w;
		src += SCREEN_W;
	}
}

uint8 *Screen::getPalette(int num) {
	debugC(9, kDebugLevelScreen, "Screen::getPalette(%d)", num);
	assert(num >= 0 && num < 4);
	if (num == 0) {
		return _screenPalette;
	}
	
	return _palettes[num-1];
}

byte Screen::getShapeFlag1(int x, int y) {
	debugC(9, kDebugLevelScreen, "Screen::getShapeFlag1(%d, %d)", x, y);
	uint8 color = _shapePages[0][y * SCREEN_W + x];
	color &= 0x80;
	color ^= 0x80;

	if (color & 0x80) {
		return 1;
	}
	return 0;
}

byte Screen::getShapeFlag2(int x, int y) {
	debugC(9, kDebugLevelScreen, "Screen::getShapeFlag2(%d, %d)", x, y);
	uint8 color = _shapePages[0][y * SCREEN_W + x];
	color &= 0x7F;
	color &= 0x87;
	return color;
}

int Screen::setNewShapeHeight(uint8 *shape, int height) {
	debugC(9, kDebugLevelScreen, "Screen::setNewShapeHeight(%p, %d)", (const void *)shape, height);
	if (_vm->features() & GF_TALKIE)
		shape += 2;
	int oldHeight = shape[2];
	shape[2] = height;
	return oldHeight;
}

int Screen::resetShapeHeight(uint8 *shape) {
	debugC(9, kDebugLevelScreen, "Screen::setNewShapeHeight(%p)", (const void *)shape);
	if (_vm->features() & GF_TALKIE)
		shape += 2;
	int oldHeight = shape[2];
	shape[2] = shape[5];
	return oldHeight;
}

void Screen::addBitBlitRect(int x, int y, int w, int h) {
	debugC(9, kDebugLevelScreen, "Screen::addBitBlitRects(%d, %d, %d, %d)", x, y, w, h);
	if (_bitBlitNum >= BITBLIT_RECTS) {
		error("too many bit blit rects");
	}
	_bitBlitRects[_bitBlitNum].x = x;
	_bitBlitRects[_bitBlitNum].y = y;
	_bitBlitRects[_bitBlitNum].x2 = w;
	_bitBlitRects[_bitBlitNum].y2 = h;
	++_bitBlitNum;
}

void Screen::bitBlitRects() {
	debugC(9, kDebugLevelScreen, "Screen::bitBlitRects()");
	Rect *cur = _bitBlitRects;
	while (_bitBlitNum) {
		_bitBlitNum--;
		copyRegion(cur->x, cur->y, cur->x, cur->y, cur->x2, cur->y2, 2, 0);
		++cur;
	}
}

void Screen::savePageToDisk(const char *file, int page) {
	debugC(9, kDebugLevelScreen, "Screen::savePageToDisk('%s', %d)", file, page);
	if (!_saveLoadPage[page/2]) {
		_saveLoadPage[page/2] = new uint8[SCREEN_W * SCREEN_H];
		assert(_saveLoadPage[page/2]);
	}
	memcpy(_saveLoadPage[page/2], getPagePtr(page), SCREEN_W * SCREEN_H);
}

void Screen::loadPageFromDisk(const char *file, int page) {
	debugC(9, kDebugLevelScreen, "Screen::loadPageFromDisk('%s', %d)", file, page);
	copyBlockToPage(page, 0, 0, SCREEN_W, SCREEN_H, _saveLoadPage[page/2]);
	delete [] _saveLoadPage[page/2];
	_saveLoadPage[page/2] = 0;
}

void Screen::deletePageFromDisk(int page) {
	debugC(9, kDebugLevelScreen, "Screen::deletePageFromDisk(%d)", page);
	delete [] _saveLoadPage[page/2];
	_saveLoadPage[page/2] = 0;
}

void Screen::blockInRegion(int x, int y, int width, int height) {
	debugC(9, kDebugLevelScreen, "Screen::blockInRegion(%d, %d, %d, %d)", x, y, width, height);
	assert(_shapePages[0]);
	byte *toPtr = _shapePages[0] + (y * 320 + x);
	for (int i = 0; i < height; ++i) {
		byte *backUpTo = toPtr;
		for (int i2 = 0; i2 < width; ++i2) {
			*toPtr++ &= 0x7F;
		}
		toPtr = (backUpTo + 320);
	}
}

void Screen::blockOutRegion(int x, int y, int width, int height) {
	debugC(9, kDebugLevelScreen, "Screen::blockOutRegion(%d, %d, %d, %d)", x, y, width, height);
	assert(_shapePages[0]);
	byte *toPtr = _shapePages[0] + (y * 320 + x);
	for (int i = 0; i < height; ++i) {
		byte *backUpTo = toPtr;
		for (int i2 = 0; i2 < width; ++i2) {
			*toPtr++ |= 0x80;
		}
		toPtr = (backUpTo + 320);
	}
}

void Screen::rectClip(int &x, int &y, int w, int h) {
	if (x < 0) {
		x = 0;
	} else if (x + w >= 320) {
		x = 320 - w;
	}
	if (y < 0) {
		y = 0;
	} else if (y + h >= 200) {
		y = 200 - h;
	}
}

void Screen::backUpRect0(int xpos, int ypos) {
	debugC(9, kDebugLevelScreen, "Screen::backUpRect0(%d, %d)", xpos, ypos);
	rectClip(xpos, ypos, 3<<3, 24);
	copyRegionToBuffer(_curPage, xpos, ypos, 3<<3, 24, _vm->shapes()[0]);
}

void Screen::restoreRect0(int xpos, int ypos) {
	debugC(9, kDebugLevelScreen, "Screen::restoreRect0(%d, %d)", xpos, ypos);
	rectClip(xpos, ypos, 3<<3, 24);
	copyBlockToPage(_curPage, xpos, ypos, 3<<3, 24, _vm->shapes()[0]);
}

void Screen::backUpRect1(int xpos, int ypos) {
	debugC(9, kDebugLevelScreen, "Screen::backUpRect1(%d, %d)", xpos, ypos);
	rectClip(xpos, ypos, 4<<3, 32);
	copyRegionToBuffer(_curPage, xpos, ypos, 4<<3, 32, _vm->shapes()[1]);
}

void Screen::restoreRect1(int xpos, int ypos) {
	debugC(9, kDebugLevelScreen, "Screen::restoreRect1(%d, %d)", xpos, ypos);
	rectClip(xpos, ypos, 4<<3, 32);
	copyBlockToPage(_curPage, xpos, ypos, 4<<3, 32, _vm->shapes()[1]);
}

int Screen::getDrawLayer(int x, int y) {
	debugC(9, kDebugLevelScreen, "Screen::getDrawLayer(%d, %d)", x, y);
	int xpos = x - 8;
	int ypos = y - 1;
	int layer = 1;
	for (int curX = xpos; curX < xpos + 16; ++curX) {
		int tempLayer = getShapeFlag2(curX, ypos);
		if (layer < tempLayer) {
			layer = tempLayer;
		}
		if (layer >= 7) {
			return 7;
		}
	}
	return layer;
}

int Screen::getDrawLayer2(int x, int y, int height) {
	debugC(9, kDebugLevelScreen, "Screen::getDrawLayer2(%d, %d, %d)", x, y, height);
	int xpos = x - 8;
	int ypos = y - 1;
	int layer = 1;
	
	for (int useX = xpos; useX < xpos + 16; ++useX) {
		for (int useY = ypos - height; useY < ypos; ++useY) {
			int tempLayer = getShapeFlag2(useX, useY);
			if (tempLayer > layer) {
				layer = tempLayer;
			}
			
			if (tempLayer >= 7) {
				return 7;
			}
		}
	}	
	return layer;
}

void Screen::copyBackgroundBlock(int x, int page, int flag) {
	debugC(9, kDebugLevelScreen, "Screen::copyBackgroundBlock(%d, %d, %d)", x, page, flag);
	
	if (x < 1)
		return;
	
	int height = 128;
	if (flag)
		height += 8;	
	if (!(x & 1))
		++x;
	if (x == 19)
		x = 17;
	uint8 *ptr1 = _unkPtr1;
	uint8 *ptr2 = _unkPtr2;
	int oldVideoPage = _curPage;
	_curPage = page;
	
	int curX = x;
	hideMouse();
	copyRegionToBuffer(_curPage, 8, 8, 8, height, ptr2);
	for (int i = 0; i < 19; ++i) {
		int tempX = curX + 1;
		copyRegionToBuffer(_curPage, tempX<<3, 8, 8, height, ptr1);
		copyBlockToPage(_curPage, tempX<<3, 8, 8, height, ptr2);
		int newXPos = curX + x;
		if (newXPos > 37) {
			newXPos = newXPos % 38;
		}
		tempX = newXPos + 1;
		copyRegionToBuffer(_curPage, tempX<<3, 8, 8, height, ptr2);
		copyBlockToPage(_curPage, tempX<<3, 8, 8, height, ptr1);
		curX += x*2;
		if (curX > 37) {
			curX = curX % 38;
		}
	}
	showMouse();
	_curPage = oldVideoPage;
}

void Screen::copyBackgroundBlock2(int x) {
	copyBackgroundBlock(x, 4, 1);
}

void Screen::shakeScreen(int times) {
	debugC(9, kDebugLevelScreen, "Screen::shakeScreen(%d)", times);
	
	while (times--) {
		// seems to be 1 line (320 pixels) offset in the original
		// 4 looks more like dosbox though, maybe check this again
		_system->setShakePos(4);
		_system->updateScreen();
		_system->setShakePos(0);
		_system->updateScreen();
	}
}

} // End of namespace Kyra
