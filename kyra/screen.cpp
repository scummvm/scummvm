/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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
 * $Header$
 *
 */

#include "common/stdafx.h"
#include "common/system.h"
#include "kyra/screen.h"
#include "kyra/kyra.h"

namespace Kyra {

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
	_mouseShape = NULL;
	_mouseShapeSize = 0;
	_mouseRect = NULL;
	_mouseRectSize = 0;
	_mouseDrawWidth = 0;
}

Screen::~Screen() {
	for (int pageNum = 0; pageNum < SCREEN_PAGE_NUM; pageNum += 2) {
		free(_pagePtrs[pageNum]);
		_pagePtrs[pageNum] = _pagePtrs[pageNum + 1] = 0;
	}
	for (int f = 0; f < ARRAYSIZE(_fonts); ++f) {
		free(_fonts[f].fontData);
		_fonts[f].fontData = NULL;
	}
	free(_currentPalette);
	free(_screenPalette);
	free(_decodeShapeBuffer);
	free(_animBlockPtr);
	free(_mouseShape);
	free(_mouseRect);
}

void Screen::updateScreen() {
	debug(9, "Screen::updateScreen()");
	_system->copyRectToScreen(getPagePtr(0), SCREEN_W, 0, 0, SCREEN_W, SCREEN_H);
	_system->updateScreen();
}

uint8 *Screen::getPagePtr(int pageNum) {
	debug(9, "Screen::getPagePtr(%d)", pageNum);
	assert(pageNum < SCREEN_PAGE_NUM);
	return _pagePtrs[pageNum];
}

void Screen::clearPage(int pageNum) {
	debug(9, "Screen::clearPage(%d)", pageNum);
	assert(pageNum < SCREEN_PAGE_NUM);
	memset(getPagePtr(pageNum), 0, SCREEN_PAGE_SIZE);
}

int Screen::setCurPage(int pageNum) {
	debug(9, "Screen::setCurPage(%d)", pageNum);
	assert(pageNum < SCREEN_PAGE_NUM);
	int previousPage = _curPage;
	_curPage = pageNum;
	return previousPage;
}

void Screen::clearCurPage() {
	debug(9, "Screen::clearCurPage()");
	memset(getPagePtr(_curPage), 0, SCREEN_PAGE_SIZE);
}

uint8 Screen::getPagePixel(int pageNum, int x, int y) {
	debug(9, "Screen::getPagePixel(%d, %d, %d)", pageNum, x, y);
	assert(pageNum < SCREEN_PAGE_NUM);
	assert(x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H);
	return _pagePtrs[pageNum][y * SCREEN_W + x];
}

void Screen::setPagePixel(int pageNum, int x, int y, uint8 color) {
	debug(9, "Screen::setPagePixel(%d, %d, %d, %d)", pageNum, x, y, color);
	assert(pageNum < SCREEN_PAGE_NUM);
	assert(x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H);
	_pagePtrs[pageNum][y * SCREEN_W + x] = color;
}

void Screen::fadeFromBlack() {
	debug(9, "Screen::fadeFromBlack()");
	fadePalette(_currentPalette, 0x54);
}

void Screen::fadeToBlack() {
	debug(9, "Screen::fadeToBlack()");
	uint8 blackPal[768];
	memset(blackPal, 0, 768);
	fadePalette(blackPal, 0x54);
}

void Screen::fadePalette(const uint8 *palData, int delay) {
	debug(9, "Screen::fadePalette(0x%X, %d)", palData, delay);
	uint8 fadePal[768];
	memcpy(fadePal, _screenPalette, 768);
	uint8 diff, maxDiff = 0;
	for (int i = 0; i < 768; ++i) {
		diff = ABS(palData[i] - fadePal[i]);
		if (diff > maxDiff) {
			maxDiff = diff;
		}
	}
	delay <<= 8;
	if (maxDiff != 0) {
		delay /= maxDiff;
	}
	int delayInc = delay;
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
		_system->delayMillis((delayAcc >> 8) * 1000 / 60);
		delayAcc &= 0xFF;
	}
}

void Screen::setScreenPalette(const uint8 *palData) {
	debug(9, "Screen::setScreenPalette(0x%X)", palData);
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
	debug(9, "Screen::copyToPage0(%d, %d, %d, 0x%X)", y, h, page, seqBuf);
	assert(y + h <= SCREEN_H);
	const uint8 *src = getPagePtr(page) + y * SCREEN_W;
	memcpy(seqBuf, src, h * SCREEN_W);
	memcpy(getPagePtr(0) + y * SCREEN_W, src, h * SCREEN_W);
}

void Screen::copyRegion(int x1, int y1, int x2, int y2, int w, int h, int srcPage, int dstPage, int flags) {
	debug(9, "Screen::copyRegion(%d, %d, %d, %d, %d, %d, %d, %d, %d)", x1, y1, x2, y2, w, h, srcPage, dstPage, flags);
	
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
	debug(9, "Screen::copyRegionToBuffer(%d, %d, %d, %d, %d)", pageNum, x, y, w, h);
	assert(x >= 0 && x < Screen::SCREEN_W && y >= 0 && y < Screen::SCREEN_H && dest);
	uint8 *pagePtr = getPagePtr(pageNum);
	for (int i = y; i < y + h; i++) {
		memcpy(dest + (i - y) * w, pagePtr + i * SCREEN_W + x, w);
	}
}

void Screen::copyBlockToPage(int pageNum, int x, int y, int w, int h, const uint8 *src) {
	debug(9, "Screen::copyBlockToPage(%d, %d, %d, %d, %d, 0x%X)", pageNum, x, y, w, h, src);
	assert(x >= 0 && x < Screen::SCREEN_W && y >= 0 && y < Screen::SCREEN_H);
	uint8 *dst = getPagePtr(pageNum) + y * Screen::SCREEN_W + x;
	while (h--) {
		for (int i = 0; i < w; ++i) {
			dst[i] = src[i];
		}
		dst += Screen::SCREEN_W;
		src += w;
	}
}

void Screen::copyCurPageBlock(int x, int y, int h, int w, uint8 *dst) {
	debug(9, "Screen::copyCurPageBlock(%d, %d, %d, %d, 0x%X)", x, y, w, h, dst);
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
		memcpy(dst, src, w * 8);
		dst += SCREEN_W;
		src += SCREEN_H;
	}
}

void Screen::shuffleScreen(int sx, int sy, int w, int h, int srcPage, int dstPage, int ticks, bool transparent) {
	debug(9, "Screen::shuffleScreen(%d, %d, %d, %d, %d, %d, %d, %d)", sx, sy, w, h, srcPage, dstPage, ticks, transparent);
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
	
	for (y = 0; y < h; ++y) {
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
		_system->delayMillis(ticks * 1000 / 60);
	}
}

void Screen::fillRect(int x1, int y1, int x2, int y2, uint8 color, int pageNum) {
	debug(9, "Screen::fillRect(%d, %d, %d, %d, %d, %d)", x1, y1, x2, y2, color, pageNum);
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

void Screen::setAnimBlockPtr(int size) {
	debug(9, "Screen::setAnimBlockPtr(%d)", size);
	free(_animBlockPtr);
	_animBlockPtr = (uint8 *)malloc(size);
	_animBlockSize = size;
}

void Screen::setTextColorMap(const uint8 *cmap) {
	debug(9, "Screen::setTextColorMap(0x%X)", cmap);
	setTextColor(cmap, 0, 11);
}

void Screen::setTextColor(const uint8 *cmap, int a, int b) {
	debug(9, "Screen::setTextColor(0x%X, %d, %d)", cmap, a, b);
	for (int i = a; i <= b; ++i) {
		_textColorsMap[i] = *cmap++;
	}
}

void Screen::loadFont(FontId fontId, uint8 *fontData) {
	debug(9, "Screen::loadFont(%d, 0x%X)", fontId, fontData);
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
	debug(9, "Screen::setFont(%d)", fontId);
	FontId prev = _currentFont;
	_currentFont = fontId;
	return prev;
}

int Screen::getCharWidth(uint8 c) const {
	debug(9, "Screen::getCharWidth('%c')", c);
	return (int)_fonts[_currentFont].charWidthTable[c] + _charWidth;
}

int Screen::getTextWidth(const char *str) const {
	debug(9, "Screen::getTextWidth('%s')", str);
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
	debug(9, "Screen::printText('%s', %d, %d, 0x%X, 0x%X)", str, x, y, color1, color2);
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
	debug(9, "Screen::drawChar('%c', %d, %d)", c, x, y);
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
	debug(9, "setScreenDim(%d)", dim);
	assert(dim < _screenDimTableCount);
	_curDim = &_screenDimTable[dim];
	// XXX
}

void Screen::drawShapePlotPixelCallback1(uint8 *dst, uint8 color) {
	debug(9, "Screen::drawShapePlotPixelCallback1(0x%X, %d)", dst, color);
	*dst = color;
}

void Screen::drawShape(uint8 pageNum, const uint8 *shapeData, int x, int y, int sd, int flags, int *flagsTable, bool itemShape) {
	debug(9, "Screen::drawShape(%d, %d, %d, %d, %d, %d)", pageNum, x, y, sd, flags, itemShape);
	assert(shapeData);
	if (flags & 0x8000) {
		warning("unhandled (flags & 0x8000) in Screen::drawShape()");
	}
	if (flags & 0x100) {
		warning("unhandled (flags & 0x100) in Screen::drawShape()");
	}
	if (flags & 0x1000) {
		warning("unhandled (flags & 0x1000) in Screen::drawShape()");
	}
	if (flags & 0x200) {
		warning("unhandled (flags & 0x200) in Screen::drawShape()");
	}
	if (flags & 0x4000) {
		warning("unhandled (flags & 0x4000) in Screen::drawShape()");
	}
	if (flags & 0x800) {
		warning("unhandled (flags & 0x800) in Screen::drawShape()");
	}
	int scale_w, scale_h;
	if (flags & DSF_SCALE) {
		scale_w = *flagsTable++;
		scale_h = *flagsTable++;
	} else {
		scale_w = 0x100;
		scale_h = 0x100;
	}
	
	int ppc = (flags >> 8) & 0x3F;
	assert(ppc < _drawShapePlotPixelCount);
	DrawShapePlotPixelCallback plotPixel = _drawShapePlotPixelTable[ppc];
	
	const uint8 *src = shapeData;
	if (_vm->features() & GF_TALKIE && !itemShape) {
		src += 2;
	}
	uint16 shapeFlags = READ_LE_UINT16(src); src += 2;
	
	int shapeHeight = *src++;
	int scaledShapeHeight = (shapeHeight * scale_h) >> 8;
	if (scaledShapeHeight == 0) {
		return;
	}

	int shapeWidth = READ_LE_UINT16(src); src += 2;
	int scaledShapeWidth = (shapeWidth * scale_w) >> 8;
	if (scaledShapeWidth == 0) {
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
		return;
	}
	memset(_decodeShapeBuffer, 0, _decodeShapeBufferSize);
	uint8 *decodedShapeFrame = _decodeShapeBuffer;
	for (int j = 0; j < shapeHeight; ++j) {
		uint8 *dsbNextLine = decodedShapeFrame + shapeWidth;
		int count = shapeWidth;
		while (count > 0) {
			uint8 code = *src++;
			if (code != 0) {
				*decodedShapeFrame++ = code;
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
			int i = scaleXTable[x];
			if (flags & DSF_X_FLIPPED) {
				i = -i;
			}
			uint8 color = shapeBuffer[j * shapeWidth + i];
			if (color != 0) {
				(this->*plotPixel)(dst, color);
			}
			++dst;
		}
		dst = dstNextLine;
	}
}

void Screen::decodeFrame3(const uint8 *src, uint8 *dst, uint32 size) {
	debug(9, "Screen::decodeFrame3(0x%X, 0x%X, %d)", src, dst, size);
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
	debug(9, "Screen::decodeFrame4(0x%X, 0x%X, %d)", src, dst, dstSize);
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
	debug(9, "Screen::decodeFrameDelta(0x%X, 0x%X)", dst, src);
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

void Screen::decodeFrameDeltaPage(uint8 *dst, const uint8 *src, int pitch) {
	debug(9, "Screen::decodeFrameDeltaPage(0x%X, 0x%X, %d)", dst, src, pitch);
	int count = 0;
	uint8 *dstNext = dst;
	while (1) {
		uint8 code = *src++;
		if (code == 0) {
			uint8 len = *src++;
			code = *src++;
			while (len--) {
				*dst++ ^= code;
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
							*dst++ ^= code;
							if (++count == pitch) {
								count = 0;
								dstNext += SCREEN_W;
								dst = dstNext;
							}
						}
					} else {
						while (subcode--) {
							*dst++ ^= *src++;
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
				*dst++ ^= *src++;
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
	debug(9, "encodeShape(%d, %d, %d, %d, %d)", x, y, w, h, flags);
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
	shapeSize += 10;
	if (flags & 1)
		shapeSize += 16;
	
	static uint8 table[274];	
	int tableIndex = 0;
	
	uint8 *newShape = (uint8*)malloc(shapeSize+16);
	assert(newShape);
	
	byte *dst = newShape;
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
			flags = READ_LE_UINT16(dst);
			flags |= 2;
			WRITE_LE_UINT16(dst, flags);
		} else {
			src = newShape;
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
	
	WRITE_LE_UINT16((newShape + 6), shapeSize);
	
	if (flags & 1) {
		dst = newShape + 10;
		src = &table[0x100];
		memcpy(dst, src, sizeof(uint8)*16);
	}
	
	return newShape;
}

int16 Screen::encodeShapeAndCalculateSize(uint8 *from, uint8 *to, int size_to) {
	debug(9, "encodeShapeAndCalculateSize(0x%X, 0x%X, %d)", from, to, size_to);
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
	debug(9, "hideMouse()");
	// if mouseDisabled
	//	return
	restoreMouseRect();
}

void Screen::showMouse() {
	debug(9, "showMouse()");
	// if mouseDisabled
	//	return
	copyMouseToScreen();
}

void Screen::setShapePages(int page1, int page2) {
	debug(9, "setShapePages(%d, %d)", page1, page2);
	_shapePages[0] = _pagePtrs[page1];
	_shapePages[1] = _pagePtrs[page2];
}

byte *Screen::setMouseCursor(int x, int y, byte *shape) {
	debug(9, "setMouseCursor(%d, %d, 0x%X)", x, y, shape);
	if (!shape)
		return _mouseShape;
	// if mouseDisabled
	//	return _mouseShape
	
	restoreMouseRect();
	
	int mouseRectSize = getRectSize((READ_LE_UINT16(shape + 3) >> 3) + 2, shape[5]);
	if (_mouseRectSize < mouseRectSize) {
		free(_mouseRect);
		_mouseRect = (uint8*)malloc(mouseRectSize << 3);
		assert(_mouseRect);
		_mouseRectSize = mouseRectSize;
	}
	
	int shapeSize = READ_LE_UINT16(shape + 8) + 10;
	if (READ_LE_UINT16(shape) & 1)
		shapeSize += 16;
	
	if (_mouseShapeSize < shapeSize) {
		free(_mouseShape);
		_mouseShape = (uint8*)malloc(shapeSize);
		assert(_mouseShape);
		_mouseShapeSize = shapeSize;
	}
	
	byte *dst = _mouseShape;
	byte *src = shape;
	
	if (!(READ_LE_UINT16(shape) & 2)) {
		uint16 newFlags = 0;
		newFlags = READ_LE_UINT16(src) | 2; src += 2;
		WRITE_LE_UINT16(dst, newFlags); dst += 2;
		memcpy(dst, src, 6);
		dst += 6;
		src += 6;
		int size = READ_LE_UINT16(src); src += 2;
		WRITE_LE_UINT16(dst, size); dst += 2;
		if (newFlags & 1) {
			memcpy(dst, src, 8);
			dst += 16;
			src += 16;
		}
		decodeFrame4(src, _animBlockPtr, size);
		memcpy(dst, _animBlockPtr, size);
	} else {
		int size = READ_LE_UINT16(shape + 6);
		memcpy(dst, src, size);
	}
	
	_mouseXOffset = x; _mouseYOffset = y;
	_mouseHeight = _mouseShape[5];
	_mouseWidth = (READ_LE_UINT16(_mouseShape + 3) >> 3) + 2;
	
	copyMouseToScreen();
	
	return _mouseShape;
}

void Screen::restoreMouseRect() {
	debug(9, "restoreMouseRect()");
	// if disableMouse
	//	return
	
	// if mouseUnk == 0 {
		if (_mouseDrawWidth && _mouseRect) {
			copyScreenFromRect(_mouseDrawX, _mouseDrawY, _mouseDrawWidth, _mouseDrawHeight, _mouseRect);
		}
		_mouseDrawWidth = 0;
	// }
	// ++mouseUnk
}

void Screen::copyMouseToScreen() {
	debug(9, "copyMouseToScreen()");
	// if disableMouse
	// 	return
	
	// if mouseUnk == 0
	//	return
	// --mouseUnk
	// if mouseUnk != 0
	//	return
	int width = _mouseWidth;
	int height = _mouseHeight;
	int xpos = _vm->mouseX() - _mouseXOffset;
	int ypos = _vm->mouseY() - _mouseYOffset;
	
	int xposTemp = xpos;
	int yposTemp = ypos;
	
	if (xposTemp < 0) {
		xposTemp = 0;
	}
	if (yposTemp < 0) {
		height += ypos;
		yposTemp = 0;
	}
	
	xposTemp >>= 3;
	_mouseDrawX = xposTemp;
	_mouseDrawY = yposTemp;
	
	xposTemp += width;
	xposTemp -= 40;
	if (xposTemp >= 0) {
		width -= xposTemp;
	}
	
	yposTemp += height;
	yposTemp -= 200;
	if (yposTemp >= 0) {
		height -= yposTemp;
	}
	
	_mouseDrawWidth = width;
	_mouseDrawHeight = height;
	
	if (_mouseRect) {
		copyScreenToRect(_mouseDrawX, _mouseDrawY, width, height, _mouseRect);
	}
	
	drawShape(0, _mouseShape, xpos, ypos, 0, 0, 0, true);
}

void Screen::copyScreenFromRect(int x, int y, int w, int h, uint8 *ptr) {
	debug(9, "copyScreenFromRect(%d, %d, %d, %d, 0x%X)", x, y, w, h, ptr);
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
	debug(9, "copyScreenToRect(%d, %d, %d, %d, 0x%X)", x, y, w, h, ptr);
	x <<= 3; w <<= 3;
	uint8 *src = &_pagePtrs[0][y * SCREEN_W + x];
	uint8 *dst = ptr;	
	for (int i = 0; i < h; ++i) {
		memcpy(dst, src, w);
		dst += w;
		src += SCREEN_W;
	}
}

} // End of namespace Kyra
