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


#include "common/endian.h"
#include "common/system.h"
#include "graphics/cursorman.h"
#include "kyra/screen.h"
#include "kyra/kyra_v1.h"
#include "kyra/resource.h"

namespace Kyra {

Screen::Screen(KyraEngine_v1 *vm, OSystem *system)
	: _system(system), _vm(vm), _sjisInvisibleColor(0) {
	_debugEnabled = false;
	_maskMinY = _maskMaxY = -1;
}

Screen::~Screen() {
	for (int i = 0; i < SCREEN_OVLS_NUM; ++i)
		delete[] _sjisOverlayPtrs[i];

	delete[] _pagePtrs[0];

	for (int f = 0; f < ARRAYSIZE(_fonts); ++f) {
		delete[] _fonts[f].fontData;
		_fonts[f].fontData = NULL;
	}

	delete[] _sjisFontData;
	delete[] _sjisTempPage;
	delete _screenPalette;
	delete _internFadePalette;
	delete[] _decodeShapeBuffer;
	delete[] _animBlockPtr;

	if (_vm->gameFlags().platform != Common::kPlatformAmiga) {
		for (int i = 0; i < ARRAYSIZE(_palettes); ++i)
			delete _palettes[i];
	}

	CursorMan.popAllCursors();
}

bool Screen::init() {
	_disableScreen = false;
	_debugEnabled = false;

	memset(_sjisOverlayPtrs, 0, sizeof(_sjisOverlayPtrs));
	_useOverlays = false;
	_useSJIS = false;
	_use16ColorMode = _vm->gameFlags().use16ColorMode;

	_sjisTempPage = _sjisFontData = 0;

	if (_vm->gameFlags().useHiResOverlay) {
		_useOverlays = true;
		_useSJIS = (_vm->gameFlags().lang == Common::JA_JPN);
		_sjisInvisibleColor = (_vm->gameFlags().gameID == GI_KYRA1) ? 0x80 : 0xF6;

		for (int i = 0; i < SCREEN_OVLS_NUM; ++i) {
			if (!_sjisOverlayPtrs[i]) {
				_sjisOverlayPtrs[i] = new uint8[SCREEN_OVL_SJIS_SIZE];
				assert(_sjisOverlayPtrs[i]);
				memset(_sjisOverlayPtrs[i], _sjisInvisibleColor, SCREEN_OVL_SJIS_SIZE);
			}
		}

		if (_useSJIS) {
			if (!_sjisFontData) {
				// we use the FM-Towns font rom for PC-98, too, until we feel
				// like adding support for the PC-98 font
				//if (_vm->gameFlags().platform == Common::kPlatformFMTowns) {
					// FM-Towns
					_sjisFontData = _vm->resource()->fileData("FMT_FNT.ROM", 0);
					if (!_sjisFontData)
						error("missing font rom ('FMT_FNT.ROM') required for this version");
				/*} else {
					// PC-98
					_sjisFontData = _vm->resource()->fileData("FONT.ROM", 0);
					if (!_sjisFontData)
						error("missing font rom ('FONT.ROM') required for this version");
				}*/
			}

			if (!_sjisTempPage) {
				_sjisTempPage = new uint8[420];
				assert(_sjisTempPage);
				_sjisTempPage2 = _sjisTempPage + 60;
				_sjisSourceChar = _sjisTempPage + 384;
			}
		}
	}

	_curPage = 0;
	uint8 *pagePtr = new uint8[SCREEN_PAGE_SIZE * 8];
	for (int pageNum = 0; pageNum < SCREEN_PAGE_NUM; pageNum += 2)
		_pagePtrs[pageNum] = _pagePtrs[pageNum + 1] = pagePtr + (pageNum >> 1) * SCREEN_PAGE_SIZE;
	memset(pagePtr, 0, SCREEN_PAGE_SIZE * 8);

	memset(_shapePages, 0, sizeof(_shapePages));

	memset(_palettes, 0, sizeof(_palettes));

	_screenPalette = new Palette(256);
	assert(_screenPalette);

	if (_vm->gameFlags().platform == Common::kPlatformAmiga) {
		for (int i = 0; i < 12; ++i) {
			_palettes[i] = new Palette(32);
			assert(_palettes[i]);
		}
	} else {
		for (int i = 0; i < 4; ++i) {
			_palettes[i] = new Palette(256);
			assert(_palettes[i]);
		}
	}

	_internFadePalette = new Palette(_palettes[0]->getNumColors());
	assert(_internFadePalette);

	setScreenPalette(getPalette(0));

	_curDim = 0;
	_charWidth = 0;
	_charOffset = 0;
	memset(_fonts, 0, sizeof(_fonts));
	for (int i = 0; i < ARRAYSIZE(_textColorsMap); ++i)
		_textColorsMap[i] = i;
	_decodeShapeBuffer = NULL;
	_decodeShapeBufferSize = 0;
	_animBlockPtr = NULL;
	_animBlockSize = 0;
	_mouseLockCount = 1;
	CursorMan.showMouse(false);

	_forceFullUpdate = false;

	return true;
}

bool Screen::enableScreenDebug(bool enable) {
	bool temp = _debugEnabled;

	if (_debugEnabled != enable) {
		_debugEnabled = enable;
		setResolution();
		_forceFullUpdate = true;
		updateScreen();
	}

	return temp;
}

void Screen::setResolution() {
	byte palette[4*256];
	_system->grabPalette(palette, 0, 256);

	int width = 320, height = 200;
	bool defaultTo1xScaler = false;

	if (_vm->gameFlags().useHiResOverlay) {
		defaultTo1xScaler = true;
		height = 400;

		if (_debugEnabled)
			width = 960;
		else
			width = 640;
	} else {
		if (_debugEnabled)
			width = 640;
		else
			width = 320;
	}

	initGraphics(width, height, defaultTo1xScaler);

	_system->setPalette(palette, 0, 256);
}

void Screen::updateScreen() {
	if (_disableScreen)
		return;

	if (_useOverlays)
		updateDirtyRectsOvl();
	else
		updateDirtyRects();

	if (_debugEnabled) {
		if (!_useOverlays)
			_system->copyRectToScreen(getPagePtr(2), SCREEN_W, 320, 0, SCREEN_W, SCREEN_H);
		else
			_system->copyRectToScreen(getPagePtr(2), SCREEN_W, 640, 0, SCREEN_W, SCREEN_H);
	}

	_system->updateScreen();
}

void Screen::updateDirtyRects() {
	if (_forceFullUpdate) {
		_system->copyRectToScreen(getCPagePtr(0), SCREEN_W, 0, 0, SCREEN_W, SCREEN_H);
	} else {
		const byte *page0 = getCPagePtr(0);
		Common::List<Common::Rect>::iterator it;
		for (it = _dirtyRects.begin(); it != _dirtyRects.end(); ++it) {
			_system->copyRectToScreen(page0 + it->top * SCREEN_W + it->left, SCREEN_W, it->left, it->top, it->width(), it->height());
		}
	}
	_forceFullUpdate = false;
	_dirtyRects.clear();
}

void Screen::updateDirtyRectsOvl() {
	if (_forceFullUpdate) {
		const byte *src = getCPagePtr(0);
		byte *dst = _sjisOverlayPtrs[0];

		scale2x(dst, 640, src, SCREEN_W, SCREEN_W, SCREEN_H);
		mergeOverlay(0, 0, 640, 400);
		_system->copyRectToScreen(dst, 640, 0, 0, 640, 400);
	} else {
		const byte *page0 = getCPagePtr(0);
		byte *ovl0 = _sjisOverlayPtrs[0];

		Common::List<Common::Rect>::iterator it;
		for (it = _dirtyRects.begin(); it != _dirtyRects.end(); ++it) {
			byte *dst = ovl0 + it->top * 1280 + (it->left<<1);
			const byte *src = page0 + it->top * SCREEN_W + it->left;

			scale2x(dst, 640, src, SCREEN_W, it->width(), it->height());
			mergeOverlay(it->left<<1, it->top<<1, it->width()<<1, it->height()<<1);
			_system->copyRectToScreen(dst, 640, it->left<<1, it->top<<1, it->width()<<1, it->height()<<1);
		}
	}
	_forceFullUpdate = false;
	_dirtyRects.clear();
}

void Screen::scale2x(byte *dst, int dstPitch, const byte *src, int srcPitch, int w, int h) {
	byte *dstL1 = dst;
	byte *dstL2 = dst + dstPitch;

	int dstAdd = dstPitch * 2 - w * 2;
	int srcAdd = srcPitch - w;

	while (h--) {
		for (int x = 0; x < w; ++x, dstL1 += 2, dstL2 += 2) {
			uint16 col = *src++;
			col |= col << 8;
			*(uint16*)(dstL1) = col;
			*(uint16*)(dstL2) = col;
		}
		dstL1 += dstAdd; dstL2 += dstAdd;
		src += srcAdd;
	}
}

void Screen::mergeOverlay(int x, int y, int w, int h) {
	byte *dst = _sjisOverlayPtrs[0] + y * 640 + x;
	const byte *src = _sjisOverlayPtrs[1] + y * 640 + x;

	int add = 640 - w;

	while (h--) {
		for (x = 0; x < w; ++x, ++dst) {
			byte col = *src++;
			if (col != _sjisInvisibleColor)
				*dst = col;
		}
		dst += add;
		src += add;
	}
}

uint8 *Screen::getPagePtr(int pageNum) {
	assert(pageNum < SCREEN_PAGE_NUM);
	return _pagePtrs[pageNum];
}

const uint8 *Screen::getCPagePtr(int pageNum) const {
	assert(pageNum < SCREEN_PAGE_NUM);
	return _pagePtrs[pageNum];
}

uint8 *Screen::getPageRect(int pageNum, int x, int y, int w, int h) {
	assert(pageNum < SCREEN_PAGE_NUM);
	if (pageNum == 0 || pageNum == 1)
		addDirtyRect(x, y, w, h);
	return _pagePtrs[pageNum] + y * SCREEN_W + x;
}

void Screen::clearPage(int pageNum) {
	assert(pageNum < SCREEN_PAGE_NUM);
	if (pageNum == 0 || pageNum == 1)
		_forceFullUpdate = true;
	memset(getPagePtr(pageNum), 0, SCREEN_PAGE_SIZE);
	clearOverlayPage(pageNum);
}

int Screen::setCurPage(int pageNum) {
	assert(pageNum < SCREEN_PAGE_NUM);
	int previousPage = _curPage;
	_curPage = pageNum;
	return previousPage;
}

void Screen::clearCurPage() {
	if (_curPage == 0 || _curPage == 1)
		_forceFullUpdate = true;
	memset(getPagePtr(_curPage), 0, SCREEN_PAGE_SIZE);
	clearOverlayPage(_curPage);
}

void Screen::copyWsaRect(int x, int y, int w, int h, int dimState, int plotFunc, const uint8 *src,
						int unk1, const uint8 *unkPtr1, const uint8 *unkPtr2) {
	uint8 *dstPtr = getPagePtr(_curPage);
	uint8 *origDst = dstPtr;

	const ScreenDim *dim = getScreenDim(dimState);
	int dimX1 = dim->sx << 3;
	int dimX2 = dim->w << 3;
	dimX2 += dimX1;

	int dimY1 = dim->sy;
	int dimY2 = dim->h;
	dimY2 += dimY1;

	int temp = y - dimY1;
	if (temp < 0) {
		if ((temp += h) <= 0)
			return;
		else {
			SWAP(temp, h);
			y += temp - h;
			src += (temp - h) * w;
		}
	}

	temp = dimY2 - y;
	if (temp <= 0)
		return;

	if (temp < h)
		h = temp;

	int srcOffset = 0;
	temp = x - dimX1;
	if (temp < 0) {
		temp = -temp;
		srcOffset = temp;
		x += temp;
		w -= temp;
	}

	int srcAdd = 0;

	temp = dimX2 - x;
	if (temp <= 0)
		return;

	if (temp < w) {
		SWAP(w, temp);
		temp -= w;
		srcAdd = temp;
	}

	dstPtr += y * SCREEN_W + x;
	uint8 *dst = dstPtr;

	if (_curPage == 0 || _curPage == 1)
		addDirtyRect(x, y, w, h);

	clearOverlayRect(_curPage, x, y, w, h);

	temp = h;
	int curY = y;
	while (h--) {
		src += srcOffset;
		++curY;
		int cW = w;

		switch (plotFunc) {
		case 0:
			memcpy(dst, src, cW);
			dst += cW; src += cW;
			break;

		case 1:
			while (cW--) {
				uint8 d = *src++;
				uint8 t = unkPtr1[d];
				if (t != 0xFF)
					d = unkPtr2[*dst + (t << 8)];
				*dst++ = d;
			}
			break;

		case 4:
			while (cW--) {
				uint8 d = *src++;
				if (d)
					*dst = d;
				++dst;
			}
			break;

		case 5:
			while (cW--) {
				uint8 d = *src++;
				if (d) {
					uint8 t = unkPtr1[d];
					if (t != 0xFF)
						d = unkPtr2[*dst + (t << 8)];
					*dst = d;
				}
				++dst;
			}
			break;

		case 8:
		case 9:
			while (cW--) {
				uint8 d = *src++;
				uint8 t = _shapePages[0][dst - origDst] & 7;
				if (unk1 < t && (curY > _maskMinY && curY < _maskMaxY))
					d = _shapePages[1][dst - origDst];
				*dst++ = d;
			}
			break;

		case 12:
		case 13:
			while (cW--) {
				uint8 d = *src++;
				if (d) {
					uint8 t = _shapePages[0][dst - origDst] & 7;
					if (unk1 < t && (curY > _maskMinY && curY < _maskMaxY))
						d = _shapePages[1][dst - origDst];
					*dst++ = d;
				} else {
					d = _shapePages[1][dst - origDst];
					*dst++ = d;
				}
			}
			break;

		default:
			break;
		}

		dst = (dstPtr += SCREEN_W);
		src += srcAdd;
	}
}

uint8 Screen::getPagePixel(int pageNum, int x, int y) {
	assert(pageNum < SCREEN_PAGE_NUM);
	assert(x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H);
	return _pagePtrs[pageNum][y * SCREEN_W + x];
}

void Screen::setPagePixel(int pageNum, int x, int y, uint8 color) {
	assert(pageNum < SCREEN_PAGE_NUM);
	assert(x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H);
	if (pageNum == 0 || pageNum == 1)
		addDirtyRect(x, y, 1, 1);
	_pagePtrs[pageNum][y * SCREEN_W + x] = color;
}

void Screen::fadeFromBlack(int delay, const UpdateFunctor *upFunc) {
	fadePalette(getPalette(0), delay, upFunc);
}

void Screen::fadeToBlack(int delay, const UpdateFunctor *upFunc) {
	Palette pal(getPalette(0).getNumColors());
	fadePalette(pal, delay, upFunc);
}

void Screen::fadePalette(const Palette &pal, int delay, const UpdateFunctor *upFunc) {
	updateScreen();

	int diff = 0, delayInc = 0;
	getFadeParams(pal, delay, delayInc, diff);

	int delayAcc = 0;
	while (!_vm->shouldQuit()) {
		delayAcc += delayInc;

		int refreshed = fadePalStep(pal, diff);

		if (upFunc && upFunc->isValid())
			(*upFunc)();
		else
			_system->updateScreen();

		if (!refreshed)
			break;

		_vm->delay((delayAcc >> 8) * 1000 / 60);
		delayAcc &= 0xFF;
	}

	if (_vm->shouldQuit()) {
		setScreenPalette(pal);
		if (upFunc && upFunc->isValid())
			(*upFunc)();
		else
			_system->updateScreen();
	}
}

void Screen::getFadeParams(const Palette &pal, int delay, int &delayInc, int &diff) {
	uint8 maxDiff = 0;

	for (int i = 0; i < pal.getNumColors() * 3; ++i) {
		diff = ABS(pal[i] - (*_screenPalette)[i]);
		maxDiff = MAX<uint8>(maxDiff, diff);
	}

	delayInc = (delay << 8) & 0x7FFF;
	if (maxDiff != 0)
		delayInc /= maxDiff;

	delay = delayInc;
	for (diff = 1; diff <= maxDiff; ++diff) {
		if (delayInc >= 512)
			break;
		delayInc += delay;
	}
}

int Screen::fadePalStep(const Palette &pal, int diff) {
	_internFadePalette->copy(*_screenPalette);

	bool needRefresh = false;

	for (int i = 0; i < pal.getNumColors() * 3; ++i) {
		int c1 = pal[i];
		int c2 = (*_internFadePalette)[i];
		if (c1 != c2) {
			needRefresh = true;
			if (c1 > c2) {
				c2 += diff;
				if (c1 < c2)
					c2 = c1;
			}

			if (c1 < c2) {
				c2 -= diff;
				if (c1 > c2)
					c2 = c1;
			}

			(*_internFadePalette)[i] = (uint8)c2;
		}
	}

	if (needRefresh)
		setScreenPalette(*_internFadePalette);

	return needRefresh ? 1 : 0;
}

void Screen::setPaletteIndex(uint8 index, uint8 red, uint8 green, uint8 blue) {
	getPalette(0)[index * 3 + 0] = red;
	getPalette(0)[index * 3 + 1] = green;
	getPalette(0)[index * 3 + 2] = blue;
	setScreenPalette(getPalette(0));
}

void Screen::getRealPalette(int num, uint8 *dst) {
	const int colors = (_vm->gameFlags().platform == Common::kPlatformAmiga ? 32 : 256);
	const uint8 *palData = getPalette(num).getData();

	if (!palData) {
		memset(dst, 0, colors * 3);
		return;
	}

	for (int i = 0; i < colors; ++i) {
		dst[0] = (palData[0] << 2) | (palData[0] & 3);
		dst[1] = (palData[1] << 2) | (palData[1] & 3);
		dst[2] = (palData[2] << 2) | (palData[2] & 3);
		dst += 3;
		palData += 3;
	}
}

void Screen::setScreenPalette(const Palette &pal) {
	uint8 screenPal[256 * 4];
	_screenPalette->copy(pal);

	for (int i = 0; i < pal.getNumColors(); ++i) {
		screenPal[4 * i + 0] = (pal[i * 3 + 0] << 2) | (pal[i * 3 + 0] & 3);
		screenPal[4 * i + 1] = (pal[i * 3 + 1] << 2) | (pal[i * 3 + 1] & 3);
		screenPal[4 * i + 2] = (pal[i * 3 + 2] << 2) | (pal[i * 3 + 2] & 3);
		screenPal[4 * i + 3] = 0;
	}

	_system->setPalette(screenPal, 0, pal.getNumColors());
}

void Screen::copyToPage0(int y, int h, uint8 page, uint8 *seqBuf) {
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
	addDirtyRect(0, y, SCREEN_W, h);
	// This would remove the text in the end sequence of
	// the (Kyrandia 1) FM-Towns version.
	// Since this method is just used for the Seqplayer
	// this shouldn't be a problem anywhere else, so it's
	// safe to disable the call here.
	//clearOverlayRect(0, 0, y, SCREEN_W, h);
}

void Screen::copyRegion(int x1, int y1, int x2, int y2, int w, int h, int srcPage, int dstPage, int flags) {
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

	const uint8 *src = getPagePtr(srcPage) + y1 * SCREEN_W + x1;
	uint8 *dst = getPagePtr(dstPage) + y2 * SCREEN_W + x2;

	if (dstPage == 0 || dstPage == 1)
		addDirtyRect(x2, y2, w, h);

	copyOverlayRegion(x1, y1, x2, y2, w, h, srcPage, dstPage);

	if (flags & CR_NO_P_CHECK) {
		while (h--) {
			memcpy(dst, src, w);
			src += SCREEN_W;
			dst += SCREEN_W;
		}
	} else {
		while (h--) {
			for (int i = 0; i < w; ++i) {
				if (src[i])
					dst[i] = src[i];
			}
			src += SCREEN_W;
			dst += SCREEN_W;
		}
	}
}

void Screen::copyRegionToBuffer(int pageNum, int x, int y, int w, int h, uint8 *dest) {
	if (y < 0) {
		dest += (-y) * w;
		h += y;
		y = 0;
	} else if (y + h > SCREEN_H) {
		h = SCREEN_H - y;
	}

	if (x < 0) {
		dest += -x;
		w += x;
		x = 0;
	} else if (x + w > SCREEN_W) {
		w = SCREEN_W - x;
	}

	if (w < 0 || h < 0)
		return;

	uint8 *pagePtr = getPagePtr(pageNum);

	for (int i = y; i < y + h; ++i)
		memcpy(dest + (i - y) * w, pagePtr + i * SCREEN_W + x, w);
}

void Screen::copyPage(uint8 srcPage, uint8 dstPage) {
	uint8 *src = getPagePtr(srcPage);
	uint8 *dst = getPagePtr(dstPage);
	memcpy(dst, src, SCREEN_W * SCREEN_H);
	copyOverlayRegion(0, 0, 0, 0, SCREEN_W, SCREEN_H, srcPage, dstPage);

	if (dstPage == 0 || dstPage == 1)
		_forceFullUpdate = true;
}

void Screen::copyBlockToPage(int pageNum, int x, int y, int w, int h, const uint8 *src) {
	if (y < 0) {
		src += (-y) * w;
		h += y;
		y = 0;
	} else if (y + h > SCREEN_H) {
		h = SCREEN_H - y;
	}

	if (x < 0) {
		src += -x;
		w += x;
		x = 0;
	} else if (x + w > SCREEN_W) {
		w = SCREEN_W - x;
	}

	if (w < 0 || h < 0)
		return;

	uint8 *dst = getPagePtr(pageNum) + y * SCREEN_W + x;

	if (pageNum == 0 || pageNum == 1)
		addDirtyRect(x, y, w, h);

	clearOverlayRect(pageNum, x, y, w, h);

	while (h--) {
		memcpy(dst, src, w);
		dst += SCREEN_W;
		src += w;
	}
}

void Screen::shuffleScreen(int sx, int sy, int w, int h, int srcPage, int dstPage, int ticks, bool transparent) {
	assert(sx >= 0 && w <= SCREEN_W);
	int x;
	uint16 x_offs[SCREEN_W];
	for (x = 0; x < SCREEN_W; ++x)
		x_offs[x] = x;

	for (x = 0; x < w; ++x) {
		int i = _vm->_rnd.getRandomNumber(w - 1);
		SWAP(x_offs[x], x_offs[i]);
	}

	assert(sy >= 0 && h <= SCREEN_H);
	int y;
	uint8 y_offs[SCREEN_H];
	for (y = 0; y < SCREEN_H; ++y)
		y_offs[y] = y;

	for (y = 0; y < h; ++y) {
		int i = _vm->_rnd.getRandomNumber(h - 1);
		SWAP(y_offs[y], y_offs[i]);
	}

	int32 start, now;
	int wait;
	for (y = 0; y < h && !_vm->shouldQuit(); ++y) {
		start = (int32)_system->getMillis();
		int y_cur = y;
		for (x = 0; x < w; ++x) {
			int i = sx + x_offs[x];
			int j = sy + y_offs[y_cur];
			++y_cur;
			if (y_cur >= h)
				y_cur = 0;

			uint8 color = getPagePixel(srcPage, i, j);
			if (!transparent || color != 0)
				setPagePixel(dstPage, i, j, color);
		}
		// forcing full update for now
		_forceFullUpdate = true;
		updateScreen();
		now = (int32)_system->getMillis();
		wait = ticks * _vm->tickLength() - (now - start);
		if (wait > 0)
			_vm->delay(wait);
	}

	copyOverlayRegion(sx, sy, sx, sy, w, h, srcPage, dstPage);

	if (_vm->shouldQuit()) {
		copyRegion(sx, sy, sx, sy, w, h, srcPage, dstPage);
		_system->updateScreen();
	}
}

void Screen::fillRect(int x1, int y1, int x2, int y2, uint8 color, int pageNum, bool xored) {
	assert(x2 < SCREEN_W && y2 < SCREEN_H);
	if (pageNum == -1)
		pageNum = _curPage;

	uint8 *dst = getPagePtr(pageNum) + y1 * SCREEN_W + x1;

	if (pageNum == 0 || pageNum == 1)
		addDirtyRect(x1, y1, x2-x1+1, y2-y1+1);

	clearOverlayRect(pageNum, x1, y1, x2-x1+1, y2-y1+1);

	if (xored) {
		for (; y1 <= y2; ++y1) {
			for (int x = x1; x <= x2; ++x)
				dst[x] ^= color;
			dst += SCREEN_W;
		}
	} else {
		for (; y1 <= y2; ++y1) {
			memset(dst, color, x2 - x1 + 1);
			dst += SCREEN_W;
		}
	}
}

void Screen::drawBox(int x1, int y1, int x2, int y2, int color) {
	drawClippedLine(x1, y1, x2, y1, color);
	drawClippedLine(x1, y1, x1, y2, color);
	drawClippedLine(x2, y1, x2, y2, color);
	drawClippedLine(x1, y2, x2, y2, color);
}

void Screen::drawShadedBox(int x1, int y1, int x2, int y2, int color1, int color2, ShadeType shadeType) {
	assert(x1 >= 0 && y1 >= 0);
	hideMouse();

	fillRect(x1, y1, x2, y1 + 1, color1);
	if (shadeType == kShadeTypeLol)
		fillRect(x1, y1, x1 + 1, y2, color1);
	else
		fillRect(x2 - 1, y1, x2, y2, color1);

	if (shadeType == kShadeTypeLol) {
		drawClippedLine(x2, y1, x2, y2, color2);
		drawClippedLine(x2 - 1, y1 + 1, x2 - 1, y2 - 1, color2);
		drawClippedLine(x1 + 1, y2 - 1, x2, y2 - 1, color2);
	} else {
		drawClippedLine(x1, y1, x1, y2, color2);
		drawClippedLine(x1 + 1, y1 + 1, x1 + 1, y2 - 1, color2);
		drawClippedLine(x1, y2 - 1, x2 - 1, y2 - 1, color2);
	}
	drawClippedLine(x1, y2, x2, y2, color2);	

	showMouse();
}

void Screen::drawClippedLine(int x1, int y1, int x2, int y2, int color) {
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

void Screen::drawLine(bool vertical, int x, int y, int length, int color) {
	uint8 *ptr = getPagePtr(_curPage) + y * SCREEN_W + x;

	if (vertical) {
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

	if (_curPage == 0 || _curPage == 1)
		addDirtyRect(x, y, (vertical) ? 1 : length, (vertical) ? length : 1);

	clearOverlayRect(_curPage, x, y, (vertical) ? 1 : length, (vertical) ? length : 1);
}

void Screen::setAnimBlockPtr(int size) {
	delete[] _animBlockPtr;
	_animBlockPtr = new uint8[size];
	assert(_animBlockPtr);
	memset(_animBlockPtr, 0, size);
	_animBlockSize = size;
}

void Screen::setTextColor(const uint8 *cmap, int a, int b) {
	memcpy(&_textColorsMap[a], cmap, b-a+1);
}

bool Screen::loadFont(FontId fontId, const char *filename) {
	Font *fnt = &_fonts[fontId];

	// FIXME: add font support for amiga version
	if (_vm->gameFlags().platform == Common::kPlatformAmiga)
		return true;

	if (!fnt)
		error("fontId %d is invalid", fontId);

	if (fnt->fontData)
		delete[] fnt->fontData;

	uint32 sz = 0;
	uint8 *fontData = fnt->fontData = _vm->resource()->fileData(filename, &sz);

	if (!fontData || !sz)
		error("Couldn't load font file '%s'", filename);

	uint16 fontSig = READ_LE_UINT16(fontData + 2);

	if (fontSig != 0x500)
		error("Invalid font data (file '%s', fontSig: %.04X)", filename, fontSig);

	fnt->charWidthTable = fontData + READ_LE_UINT16(fontData + 8);
	fnt->fontDescOffset = READ_LE_UINT16(fontData + 4);
	fnt->charBitmapOffset = READ_LE_UINT16(fontData + 6);
	fnt->charWidthTableOffset = READ_LE_UINT16(fontData + 8);
	fnt->charHeightTableOffset = READ_LE_UINT16(fontData + 0xC);

	fnt->lastGlyph = *(fnt->fontData + fnt->fontDescOffset + 3);

	return true;
}

Screen::FontId Screen::setFont(FontId fontId) {
	FontId prev = _currentFont;
	_currentFont = fontId;
	return prev;
}

int Screen::getFontHeight() const {
	// FIXME: add font support for amiga version
	if (_vm->gameFlags().platform == Common::kPlatformAmiga)
		return 0;

	return *(_fonts[_currentFont].fontData + _fonts[_currentFont].fontDescOffset + 4);
}

int Screen::getFontWidth() const {
	// FIXME: add font support for amiga version
	if (_vm->gameFlags().platform == Common::kPlatformAmiga)
		return 0;

	return *(_fonts[_currentFont].fontData + _fonts[_currentFont].fontDescOffset + 5);
}

int Screen::getCharWidth(uint16 c) const {
	// FIXME: add font support for amiga version
	if (_vm->gameFlags().platform == Common::kPlatformAmiga)
		return 0;

	if (c & 0xFF00)
		return SJIS_CHARSIZE >> 1;

	if (_fonts[_currentFont].lastGlyph < c)
		return 0;
	else
		return (int)_fonts[_currentFont].charWidthTable[c] + _charWidth;
}

int Screen::getTextWidth(const char *str) const {
	// FIXME: add font support for amiga version
	if (_vm->gameFlags().platform == Common::kPlatformAmiga)
		return 0;

	int curLineLen = 0;
	int maxLineLen = 0;
	while (1) {
		uint c = *str++;
		c &= 0xFF;
		if (c == 0) {
			break;
		} else if (c == '\r') {
			if (curLineLen > maxLineLen)
				maxLineLen = curLineLen;
			else
				curLineLen = 0;
		} else {
			if (c <= 0x7F || !_useSJIS) {
				curLineLen += getCharWidth(c);
			} else {
				c = READ_LE_UINT16(str - 1);
				++str;
				curLineLen += getCharWidth(c);
			}
		}
	}

	return MAX(curLineLen, maxLineLen);
}

void Screen::printText(const char *str, int x, int y, uint8 color1, uint8 color2) {
	// FIXME: add font support for amiga version
	if (_vm->gameFlags().platform == Common::kPlatformAmiga)
		return;
	uint8 cmap[2];
	cmap[0] = color2;
	cmap[1] = color1;
	setTextColor(cmap, 0, 1);

	const uint8 charHeightFnt = getFontHeight();
	uint8 charHeight = 0;

	if (x < 0)
		x = 0;
	else if (x >= SCREEN_W)
		return;

	int x_start = x;
	if (y < 0)
		y = 0;
	else if (y >= SCREEN_H)
		return;

	while (1) {
		uint c = *str++;
		c &= 0xFF;

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
				if (y >= SCREEN_H)
					break;
			}

			if (c <= 0x7F || !_useSJIS) {
				drawCharANSI(c, x, y);
				charHeight = charHeightFnt;
			} else {
				c = READ_LE_UINT16(str - 1);
				++str;
				charWidth = getCharWidth(c);
				charHeight = SJIS_CHARSIZE >> 1;
				drawCharSJIS(c, x, y);
			}

			x += charWidth;
		}
	}
}

void Screen::drawCharANSI(uint8 c, int x, int y) {
	Font *fnt = &_fonts[_currentFont];

	if (c > fnt->lastGlyph)
		return;

	uint8 *dst = getPagePtr(_curPage) + y * SCREEN_W + x;

	uint16 bitmapOffset = READ_LE_UINT16(fnt->fontData + fnt->charBitmapOffset + c * 2);
	if (bitmapOffset == 0)
		return;

	uint8 charWidth = *(fnt->fontData + fnt->charWidthTableOffset + c);
	if (!charWidth || charWidth + x > SCREEN_W)
		return;

	uint8 charH0 = getFontHeight();
	if (!charH0 || charH0 + y > SCREEN_H)
		return;

	uint8 charH1 = *(fnt->fontData + fnt->charHeightTableOffset + c * 2);
	uint8 charH2 = *(fnt->fontData + fnt->charHeightTableOffset + c * 2 + 1);
	
	charH0 -= charH1 + charH2;

	const uint8 *src = fnt->fontData + bitmapOffset;
	const int pitch = SCREEN_W - charWidth;

	while (charH1--) {
		uint8 col = _textColorsMap[0];
		for (int i = 0; i < charWidth; ++i) {
			if (col != 0)
				*dst = col;
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
			if (col != 0)
				*dst = col;
			++dst;
		}
		dst += pitch;
	}

	if (_curPage == 0 || _curPage == 1)
		addDirtyRect(x, y, charWidth, getFontHeight());
}

void Screen::drawShape(uint8 pageNum, const uint8 *shapeData, int x, int y, int sd, int flags, ...) {
	if (!shapeData)
		return;

	int f = _vm->gameFlags().useAltShapeHeader ? 2 : 0;
	if (shapeData[f] & 1)
		flags |= 0x400;

	va_list args;
	va_start(args, flags);

	static const int drawShapeVar2[] = {
		1, 3, 2, 5, 4, 3, 2, 1
	};

	_drawShapeVar1 = 0;
	_drawShapeVar3 = 1;
	_drawShapeVar4 = 0;
	_drawShapeVar5 = 0;

	_dsTable = 0;
	_dsTableLoopCount = 0;
	_dsTable2 = 0;
	_dsTable3 = 0;
	_dsTable4 = 0;
	_dsTable5 = 0;
	_dsDrawLayer = 0;

	if (flags & 0x8000) {
		_dsTable2 = va_arg(args, uint8*);
	}

	if (flags & 0x100) {
		_dsTable = va_arg(args, uint8*);
		_dsTableLoopCount = va_arg(args, int);
		if (!_dsTableLoopCount)
			flags &= ~0x100;
	}

	if (flags & 0x1000) {
		_dsTable3 = va_arg(args, uint8*);
		_dsTable4 = va_arg(args, uint8*);
	}

	if (flags & 0x200) {
		_drawShapeVar1 += 1;
		_drawShapeVar1 &= 7;
		_drawShapeVar3 = drawShapeVar2[_drawShapeVar1];
		_drawShapeVar4 = 0;
		_drawShapeVar5 = 256;
	}

	if (flags & 0x4000)
		_drawShapeVar5 = va_arg(args, int);

	if (flags & 0x800)
		_dsDrawLayer = va_arg(args, int);

	if (flags & DSF_SCALE) {
		_dsScaleW = va_arg(args, int);
		_dsScaleH = va_arg(args, int);
	} else {
		_dsScaleW = 0x100;
		_dsScaleH = 0x100;
	}

	if ((flags & 0x2000) && _vm->gameFlags().gameID != GI_KYRA1)
		_dsTable5 = va_arg(args,  uint8*);

	static const DsMarginSkipFunc dsMarginFunc[] = {
		&Screen::drawShapeMarginNoScaleUpwind,
		&Screen::drawShapeMarginNoScaleDownwind,
		&Screen::drawShapeMarginNoScaleUpwind,
		&Screen::drawShapeMarginNoScaleDownwind,
		&Screen::drawShapeMarginScaleUpwind,
		&Screen::drawShapeMarginScaleDownwind,
		&Screen::drawShapeMarginScaleUpwind,
		&Screen::drawShapeMarginScaleDownwind
	};

	static const DsMarginSkipFunc dsSkipFunc[] = {
		&Screen::drawShapeMarginNoScaleUpwind,
		&Screen::drawShapeMarginNoScaleDownwind,
		&Screen::drawShapeMarginNoScaleUpwind,
		&Screen::drawShapeMarginNoScaleDownwind,
		&Screen::drawShapeSkipScaleUpwind,
		&Screen::drawShapeSkipScaleDownwind,
		&Screen::drawShapeSkipScaleUpwind,
		&Screen::drawShapeSkipScaleDownwind
	};

	static const DsLineFunc dsLineFunc[] = {
		&Screen::drawShapeProcessLineNoScaleUpwind,
		&Screen::drawShapeProcessLineNoScaleDownwind,
		&Screen::drawShapeProcessLineNoScaleUpwind,
		&Screen::drawShapeProcessLineNoScaleDownwind,
		&Screen::drawShapeProcessLineScaleUpwind,
		&Screen::drawShapeProcessLineScaleDownwind,
		&Screen::drawShapeProcessLineScaleUpwind,
		&Screen::drawShapeProcessLineScaleDownwind
	};

	static const DsPlotFunc dsPlotFunc[] = {
		&Screen::drawShapePlotType0,		// used by Kyra 1 + 2
		&Screen::drawShapePlotType1,		// used by Kyra 3
		0,
		&Screen::drawShapePlotType3_7,		// used by Kyra 3 (shadow)
		&Screen::drawShapePlotType4,		// used by Kyra 1, 2 + 3
		&Screen::drawShapePlotType5,		// used by Kyra 1
		&Screen::drawShapePlotType6,		// used by Kyra 1 (invisibility)
		&Screen::drawShapePlotType3_7,		// used by Kyra 1 (invisibility)
		&Screen::drawShapePlotType8,		// used by Kyra 2
		&Screen::drawShapePlotType9,		// used by Kyra 1 + 3
		0,
		&Screen::drawShapePlotType11_15,	// used by Kyra 1 (invisibility) + Kyra 3 (shadow)
		&Screen::drawShapePlotType12,		// used by Kyra 2
		&Screen::drawShapePlotType13,		// used by Kyra 1
		&Screen::drawShapePlotType14,		// used by Kyra 1 (invisibility)
		&Screen::drawShapePlotType11_15,	// used by Kyra 1 (invisibility)
		0, 0, 0, 0,
		&Screen::drawShapePlotType20,		// used by LoL (heal spell effect)
		&Screen::drawShapePlotType21,		// used by LoL (white tower spirits)
		0, 0, 0, 0,	0, 0, 0, 0, 0, 0,
		0,
		&Screen::drawShapePlotType33,		// used by LoL (blood spots on the floor)
		0, 0, 0,
		&Screen::drawShapePlotType37,		// used by LoL (monsters)
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		&Screen::drawShapePlotType48,		// used by LoL (slime spots on the floor)
		0, 0, 0,
		&Screen::drawShapePlotType52,		// used by LoL (projectiles)
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0
	};

	int scaleCounterV = 0;

	f = flags & 0x0f;
	_dsProcessMargin = dsMarginFunc[f];
	_dsScaleSkip = dsSkipFunc[f];
	_dsProcessLine = dsLineFunc[f];

	int ppc = (flags >> 8) & 0x3F;
	_dsPlot = dsPlotFunc[ppc];
	DsPlotFunc dsPlot2 = dsPlotFunc[ppc], dsPlot3 = dsPlotFunc[ppc];
	if (flags & 0x800)
		dsPlot3 = dsPlotFunc[((flags >> 8) & 0xF7) & 0x3F];

	if (!_dsPlot || !dsPlot2 || !dsPlot3) {
		if (!dsPlot2)
			warning("Missing drawShape plotting method type %d", ppc);
		if (dsPlot3 != dsPlot2 && !dsPlot3)
			warning("Missing drawShape plotting method type %d", (((flags >> 8) & 0xF7) & 0x3F));
		va_end(args);
		return;
	}

	int curY = y;
	const uint8 *src = shapeData;
	uint8 *dst = _dsDstPage = getPagePtr(pageNum);

	const ScreenDim *dsDim = getScreenDim(sd);
	dst += (dsDim->sx << 3);

	if (!(flags & 0x10))
		x -= (dsDim->sx << 3);

	int x2 = (dsDim->w << 3);
	int y1 = dsDim->sy;
	if (flags & 0x10)
		y += y1;

	int y2 = y1 + dsDim->h;

	if (_vm->gameFlags().useAltShapeHeader)
		src += 2;
	uint16 shapeFlags = READ_LE_UINT16(src); src += 2;

	int shapeHeight = *src++;
	uint16 shapeWidth = READ_LE_UINT16(src); src += 2;

	int shpWidthScaled1 = shapeWidth;
	int shpWidthScaled2 = shapeWidth;

	if (flags & DSF_SCALE) {
		shapeHeight = (shapeHeight * _dsScaleH) >> 8;
		shpWidthScaled1 = shpWidthScaled2 = (shapeWidth * _dsScaleW) >> 8;

		if (!shapeHeight || !shpWidthScaled1) {
			va_end(args);
			return;
		}
	}

	if (flags & DSF_CENTER) {
		x -= (shpWidthScaled1 >> 1);
		y -= (shapeHeight >> 1);
	}

	src += 3;

	uint16 frameSize = READ_LE_UINT16(src); src += 2;

	int colorTableColors = ((_vm->gameFlags().gameID != GI_KYRA1) && (shapeFlags & 4)) ? *src++ : 16;

	if (!(flags & 0x8000) && (shapeFlags & 1))
		_dsTable2 = src;

	if (flags & 0x400)
		src += colorTableColors;

	if (!(shapeFlags & 2)) {
		decodeFrame4(src, _animBlockPtr, frameSize);
		src = _animBlockPtr;
	}

	int t = (flags & 2) ? y2 - y - shapeHeight : y - y1;

	const uint8 *s = src;

	if (t < 0) {
		shapeHeight += t;
		if (shapeHeight <= 0) {
			va_end(args);
			return;
		}

		t *= -1;
		uint8 *tmp = dst;

		do {
			_dsOffscreenScaleVal1 = 0;
			_dsTmpWidth = shapeWidth;
			int cnt = shapeWidth;
			(this->*_dsScaleSkip)(tmp, s, cnt);
			scaleCounterV += _dsScaleH;
			if (!(scaleCounterV & 0xff00))
				continue;
			uint8 r = scaleCounterV >> 8;
			scaleCounterV &= 0xff;
			t -= r;
		} while (t > 0);

		if (t < 0)
			scaleCounterV += (-t << 8);

		if (!(flags & 2))
			y = y1;
	}

	t = (flags & 2) ? y + shapeHeight - y1 : y2 - y;
	if (t <= 0) {
		va_end(args);
		return;
	}

	if (t < shapeHeight) {
		shapeHeight = t;
		if (flags & 2)
			y = y1;
	}

	_dsOffscreenLeft = 0;
	if (x < 0) {
		shpWidthScaled1 += x;
		_dsOffscreenLeft = -x;
		if (_dsOffscreenLeft >= shpWidthScaled2) {
			va_end(args);
			return;
		}
		x = 0;
	}

	_dsOffscreenRight = 0;
	t = x2 - x;

	if (t <= 0) {
		va_end(args);
		return;
	}

	if (t < shpWidthScaled1) {
		shpWidthScaled1 = t;
		_dsOffscreenRight = shpWidthScaled2 - _dsOffscreenLeft - shpWidthScaled1;
	}

	int dsPitch = 320;
	int ty = y;

	if (flags & 2) {
		dsPitch *= -1;
		ty = ty - 1 + shapeHeight;
	}

	if (flags & DSF_X_FLIPPED) {
		SWAP(_dsOffscreenLeft, _dsOffscreenRight);
		dst += (shpWidthScaled1 - 1);
	}

	dst += (320 * ty + x);

	if (flags & DSF_SCALE) {
		_dsOffscreenRight = 0;
		_dsOffscreenScaleVal2 = _dsOffscreenLeft;
		_dsOffscreenLeft <<= 8;
		_dsOffscreenScaleVal1 = (_dsOffscreenLeft % _dsScaleW) * -1;
		_dsOffscreenLeft /= _dsScaleW;
	}

	if (shapeHeight <= 0 || shpWidthScaled1 <= 0)
		return;

	if (pageNum == 0 || pageNum == 1)
		addDirtyRect(x, y, shpWidthScaled1, shapeHeight);
	clearOverlayRect(pageNum, x, y, shpWidthScaled1, shapeHeight);

	uint8 *d = dst;

	bool normalPlot = true;
	while (true) {
		while (!(scaleCounterV & 0xFF00)) {
			scaleCounterV += _dsScaleH;
			if (!(scaleCounterV & 0xFF00)) {
				_dsTmpWidth = shapeWidth;
				int cnt = shapeWidth;
				(this->*_dsScaleSkip)(d, s, cnt);
			}
		}

		const uint8 *b_src = s;

		do {
			s = b_src;
			_dsTmpWidth = shapeWidth;
			int cnt = _dsOffscreenLeft;
			int scaleState = (this->*_dsProcessMargin)(d, s, cnt);
			if (_dsTmpWidth) {
				cnt += shpWidthScaled1;
				if (cnt > 0) {
					if (flags & 0x800)
						normalPlot = (curY > _maskMinY && curY < _maskMaxY);
					_dsPlot = normalPlot ? dsPlot2 : dsPlot3;
					(this->*_dsProcessLine)(d, s, cnt, scaleState);
				}
				cnt += _dsOffscreenRight;
				if (cnt)
					(this->*_dsScaleSkip)(d, s, cnt);
			}
			dst += dsPitch;
			d = dst;
			++curY;

			if (!--shapeHeight)
				return;

			scaleCounterV -= 0x100;
		} while (scaleCounterV & 0xFF00);
	}

	va_end(args);
}

int Screen::drawShapeMarginNoScaleUpwind(uint8 *&dst, const uint8 *&src, int &cnt) {
	while (cnt-- > 0) {
		if (*src++)
			continue;
		cnt = cnt + 1 - (*src++);
	}

	cnt++;
	dst -= cnt;
	return 0;
}

int Screen::drawShapeMarginNoScaleDownwind(uint8 *&dst, const uint8 *&src, int &cnt) {
	while (cnt-- > 0) {
		if (*src++)
			continue;
		cnt = cnt + 1 - (*src++);
	}

	cnt++;
	dst += cnt;
	return 0;
}

int Screen::drawShapeMarginScaleUpwind(uint8 *&dst, const uint8 *&src, int &cnt) {
	_dsTmpWidth -= cnt;

	while (cnt > 0) {
		--cnt;

		if (*src++)
			continue;

		cnt = cnt + 1 - (*src++);
	}

	if (!cnt)
		return _dsOffscreenScaleVal1;

	_dsTmpWidth += cnt;

	int i = (_dsOffscreenLeft - cnt) * _dsScaleW;
	int res = i & 0xff;
	i >>= 8;
	i -= _dsOffscreenScaleVal2;
	dst += i;
	cnt = -i;

	return res;
}

int Screen::drawShapeMarginScaleDownwind(uint8 *&dst, const uint8 *&src, int &cnt) {
	_dsTmpWidth -= cnt;

	while (cnt > 0) {
		--cnt;

		if (*src++)
			continue;

		cnt = cnt + 1 - (*src++);
	}

	if (!cnt)
		return _dsOffscreenScaleVal1;

	_dsTmpWidth += cnt;

	int i = (_dsOffscreenLeft - cnt) * _dsScaleW;
	int res = i & 0xff;
	i >>= 8;
	i -= _dsOffscreenScaleVal2;
	dst -= i;
	cnt = -i;

	return res;
}

int Screen::drawShapeSkipScaleUpwind(uint8 *&dst, const uint8 *&src, int &cnt) {
	cnt = _dsTmpWidth;

	if (cnt <= 0)
		return 0;

	do {
		if (*src++)
			continue;
		cnt = cnt + 1 - (*src++);
	} while (--cnt > 0);

	return 0;
}

int Screen::drawShapeSkipScaleDownwind(uint8 *&dst, const uint8 *&src, int &cnt) {
	cnt = _dsTmpWidth;
	bool found = false;

	if (cnt == 0)
		return 0;

	do {
		if (*src++)
			continue;
		found = true;
		cnt = cnt + 1 - (*src++);
	} while (--cnt > 0);

	return found ? 0 : _dsOffscreenScaleVal1;
}

void Screen::drawShapeProcessLineNoScaleUpwind(uint8 *&dst, const uint8 *&src, int &cnt, uint16) {
	do {
		uint8 c = *src++;
		if (c) {
			uint8 *d = dst++;
			(this->*_dsPlot)(d, c);
			cnt--;
		} else {
			c = *src++;
			dst += c;
			cnt -= c;
		}
	} while (cnt > 0);
}

void Screen::drawShapeProcessLineNoScaleDownwind(uint8 *&dst, const uint8 *&src, int &cnt, uint16) {
	do {
		uint8 c = *src++;
		if (c) {
			uint8 *d = dst--;
			(this->*_dsPlot)(d, c);
			cnt--;
		} else {
			c = *src++;
			dst -= c;
			cnt -= c;
		}
	} while (cnt > 0);
}

void Screen::drawShapeProcessLineScaleUpwind(uint8 *&dst, const uint8 *&src, int &cnt, uint16 scaleState) {
	int c = 0;

	do {
		if ((scaleState & 0x8000) || !(scaleState & 0xFF00)) {
			c = *src++;
			_dsTmpWidth--;
			if (c) {
				scaleState += _dsScaleW;
			} else {
				_dsTmpWidth++;
				c = *src++;
				_dsTmpWidth -= c;
				int r = c * _dsScaleW + scaleState;
				dst += (r >> 8);
				cnt -= (r >> 8);
				scaleState = r & 0xff;
			}
		} else if (scaleState) {
			(this->*_dsPlot)(dst++, c);
			scaleState -= 0x100;
			cnt--;
		}
	} while (cnt > 0);

	cnt = -1;
}

void Screen::drawShapeProcessLineScaleDownwind(uint8 *&dst, const uint8 *&src, int &cnt, uint16 scaleState) {
	int c = 0;

	do {
		if ((scaleState & 0x8000) || !(scaleState & 0xFF00)) {
			c = *src++;
			_dsTmpWidth--;
			if (c) {
				scaleState += _dsScaleW;
			} else {
				_dsTmpWidth++;
				c = *src++;
				_dsTmpWidth -= c;
				int r = c * _dsScaleW + scaleState;
				dst -= (r >> 8);
				cnt -= (r >> 8);
				scaleState = r & 0xff;
			}
		} else {
			(this->*_dsPlot)(dst--, c);
			scaleState -= 0x100;
			cnt--;
		}
	} while (cnt > 0);

	cnt = -1;
}

void Screen::drawShapePlotType0(uint8 *dst, uint8 cmd) {
	*dst = cmd;
}

void Screen::drawShapePlotType1(uint8 *dst, uint8 cmd) {
	for (int i = 0; i < _dsTableLoopCount; ++i)
		cmd = _dsTable[cmd];

	if (cmd)
		*dst = cmd;
}

void Screen::drawShapePlotType3_7(uint8 *dst, uint8 cmd) {
	cmd = *dst;
	for (int i = 0; i < _dsTableLoopCount; ++i)
		cmd = _dsTable[cmd];

	if (cmd)
		*dst = cmd;
}

void Screen::drawShapePlotType4(uint8 *dst, uint8 cmd) {
	*dst = _dsTable2[cmd];
}

void Screen::drawShapePlotType5(uint8 *dst, uint8 cmd) {
	cmd = _dsTable2[cmd];
	for (int i = 0; i < _dsTableLoopCount; ++i)
		cmd = _dsTable[cmd];

	if (cmd)
		*dst = cmd;
}

void Screen::drawShapePlotType6(uint8 *dst, uint8 cmd) {
	int t = _drawShapeVar4 + _drawShapeVar5;
	if (t & 0xff00) {
		cmd = dst[_drawShapeVar3];
		t &= 0xff;
	} else {
		cmd = _dsTable2[cmd];
	}

	_drawShapeVar4 = t;
	*dst = cmd;
}

void Screen::drawShapePlotType8(uint8 *dst, uint8 cmd) {
	uint32 relOffs = dst - _dsDstPage;
	int t = (_shapePages[0][relOffs] & 0x7f) & 0x87;
	if (_dsDrawLayer < t)
		cmd = _shapePages[1][relOffs];

	*dst = cmd;
}

void Screen::drawShapePlotType9(uint8 *dst, uint8 cmd) {
	uint32 relOffs = dst - _dsDstPage;
	int t = (_shapePages[0][relOffs] & 0x7f) & 0x87;
	if (_dsDrawLayer < t) {
		cmd = _shapePages[1][relOffs];
	} else {
		for (int i = 0; i < _dsTableLoopCount; ++i)
			cmd = _dsTable[cmd];
	}

	if (cmd)
		*dst = cmd;
}

void Screen::drawShapePlotType11_15(uint8 *dst, uint8 cmd) {
	uint32 relOffs = dst - _dsDstPage;
	int t = (_shapePages[0][relOffs] & 0x7f) & 0x87;

	if (_dsDrawLayer < t) {
		cmd = _shapePages[1][relOffs];
	} else {
		cmd = *dst;
		for (int i = 0; i < _dsTableLoopCount; ++i)
			cmd = _dsTable[cmd];
	}

	if (cmd)
		*dst = cmd;
}

void Screen::drawShapePlotType12(uint8 *dst, uint8 cmd) {
	uint32 relOffs = dst - _dsDstPage;
	int t = (_shapePages[0][relOffs] & 0x7f) & 0x87;
	if (_dsDrawLayer < t) {
		cmd = _shapePages[1][relOffs];
	} else {
		cmd = _dsTable2[cmd];
	}

	*dst = cmd;
}

void Screen::drawShapePlotType13(uint8 *dst, uint8 cmd) {
	uint32 relOffs = dst - _dsDstPage;
	int t = (_shapePages[0][relOffs] & 0x7f) & 0x87;
	if (_dsDrawLayer < t) {
		cmd = _shapePages[1][relOffs];
	} else {
		cmd = _dsTable2[cmd];
		for (int i = 0; i < _dsTableLoopCount; ++i)
			cmd = _dsTable[cmd];
	}

	if (cmd)
		*dst = cmd;
}

void Screen::drawShapePlotType14(uint8 *dst, uint8 cmd) {
	uint32 relOffs = dst - _dsDstPage;
	int t = (_shapePages[0][relOffs] & 0x7f) & 0x87;
	if (_dsDrawLayer < t) {
		cmd = _shapePages[1][relOffs];
	} else {
		t = _drawShapeVar4 + _drawShapeVar5;
		if (t & 0xff00) {
			cmd = dst[_drawShapeVar3];
			t &= 0xff;
		} else {
			cmd = _dsTable2[cmd];
		}
	}

	_drawShapeVar4 = t;
	*dst = cmd;
}

void Screen::drawShapePlotType20(uint8 *dst, uint8 cmd) {
	cmd = _dsTable2[cmd];
	uint8 tOffs = _dsTable3[cmd];
	if (!(tOffs & 0x80))
		cmd = _dsTable4[tOffs << 8 | *dst];

	*dst = cmd;
}

void Screen::drawShapePlotType21(uint8 *dst, uint8 cmd) {
	cmd = _dsTable2[cmd];
	uint8 tOffs = _dsTable3[cmd];
	if (!(tOffs & 0x80))
		cmd = _dsTable4[tOffs << 8 | *dst];

	for (int i = 0; i < _dsTableLoopCount; ++i)
		cmd = _dsTable[cmd];

	if (cmd)
		*dst = cmd;
}

void Screen::drawShapePlotType33(uint8 *dst, uint8 cmd) {
	if (cmd == 255) {
		*dst = _dsTable5[*dst];
	} else {
		for (int i = 0; i < _dsTableLoopCount; ++i)
			cmd = _dsTable[cmd];
		if (cmd)
			*dst = cmd;
	}
}

void Screen::drawShapePlotType37(uint8 *dst, uint8 cmd) {
	cmd = _dsTable2[cmd];

	if (cmd == 255) {
		cmd = _dsTable5[*dst];
	} else {
		for (int i = 0; i < _dsTableLoopCount; ++i)
			cmd = _dsTable[cmd];
	}

	if (cmd)
		*dst = cmd;
}

void Screen::drawShapePlotType48(uint8 *dst, uint8 cmd) {
	uint8 offs = _dsTable3[cmd];
	if (!(offs & 0x80))
		cmd = _dsTable4[(offs << 8) | *dst];
	*dst = cmd;
}

void Screen::drawShapePlotType52(uint8 *dst, uint8 cmd) {
	cmd = _dsTable2[cmd];
	uint8 offs = _dsTable3[cmd];

	if (!(offs & 0x80))
		cmd = _dsTable4[(offs << 8) | *dst];

	*dst = cmd;
}

void Screen::decodeFrame3(const uint8 *src, uint8 *dst, uint32 size) {
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

uint Screen::decodeFrame4(const uint8 *src, uint8 *dst, uint32 dstSize) {
	uint8 *dstOrig = dst;
	uint8 *dstEnd = dst + dstSize;
	while (1) {
		int count = dstEnd - dst;
		if (count == 0)
			break;

		uint8 code = *src++;
		if (!(code & 0x80)) { // 8th bit isn't set
			int len = MIN(count, (code >> 4) + 3); //upper half of code is the length
			int offs = ((code & 0xF) << 8) | *src++; //lower half of code as byte 2 of offset.
			const uint8 *dstOffs = dst - offs;
			while (len--)
				*dst++ = *dstOffs++;
		} else if (code & 0x40) { // 7th bit is set
			int len = (code & 0x3F) + 3;
			if (code == 0xFE) {
				len = READ_LE_UINT16(src); src += 2;
				if (len > count)
					len = count;

				memset(dst, *src++, len); dst += len;
			} else {
				if (code == 0xFF) {
					len = READ_LE_UINT16(src);
					src += 2;
				}

				int offs = READ_LE_UINT16(src); src += 2;
				if (len > count)
					len = count;

				const uint8 *dstOffs = dstOrig + offs;
				while (len--)
					*dst++ = *dstOffs++;
			}
		} else if (code != 0x80) { // not just the 8th bit set.
			//Copy some bytes from source to dest.
			int len = MIN(count, code & 0x3F);
			while (len--)
				*dst++ = *src++;
		} else {
			break;
		}
	}
	return dst - dstOrig;
}

void Screen::decodeFrameDelta(uint8 *dst, const uint8 *src, bool noXor) {
	if (noXor)
		wrapped_decodeFrameDelta<true>(dst, src);
	else
		wrapped_decodeFrameDelta<false>(dst, src);
}

template <bool noXor>
void Screen::wrapped_decodeFrameDelta(uint8 *dst, const uint8 *src) {
	while (1) {
		uint8 code = *src++;
		if (code == 0) {
			uint8 len = *src++;
			code = *src++;
			while (len--) {
				if (noXor)
					*dst++ = code;
				else
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
							if (noXor)
								*dst++ = code;
							else
								*dst++ ^= code;
						}
					} else {
						while (subcode--) {
							if (noXor)
								*dst++ = *src++;
							else
								*dst++ ^= *src++;
						}
					}
				} else {
					dst += subcode;
				}
			}
		} else {
			while (code--) {
				if (noXor)
					*dst++ = *src++;
				else
					*dst++ ^= *src++;
			}
		}
	}
}

void Screen::decodeFrameDeltaPage(uint8 *dst, const uint8 *src, int pitch, bool noXor) {
	if (noXor)
		wrapped_decodeFrameDeltaPage<true>(dst, src, pitch);
	else
		wrapped_decodeFrameDeltaPage<false>(dst, src, pitch);
}

void Screen::convertAmigaGfx(uint8 *data, int w, int h, bool offscreen) {
	static uint8 tmp[320*200];

	if (offscreen) {
		uint8 *curLine = tmp;
		const uint8 *src = data;
		int hC = h;
		while (hC--) {
			uint8 *dst1 = curLine;
			uint8 *dst2 = dst1 + 8000;
			uint8 *dst3 = dst2 + 8000;
			uint8 *dst4 = dst3 + 8000;
			uint8 *dst5 = dst4 + 8000;

			int width = w >> 3;
			while (width--) {
				*dst1++ = *src++;
				*dst2++ = *src++;
				*dst3++ = *src++;
				*dst4++ = *src++;
				*dst5++ = *src++;
			}

			curLine += 40;
		}
	} else {
		memcpy(tmp, data, w*h);
	}

	for (int y = 0; y < h; ++y) {
		for (int x = 0; x < w; ++x) {
			int bytePos = x/8+y*40;
			int bitPos = 7-(x&7);

			byte colorIndex = 0;
			colorIndex |= (((tmp[bytePos + 8000 * 0] & (1 << bitPos)) >> bitPos) & 0x1) << 0;
			colorIndex |= (((tmp[bytePos + 8000 * 1] & (1 << bitPos)) >> bitPos) & 0x1) << 1;
			colorIndex |= (((tmp[bytePos + 8000 * 2] & (1 << bitPos)) >> bitPos) & 0x1) << 2;
			colorIndex |= (((tmp[bytePos + 8000 * 3] & (1 << bitPos)) >> bitPos) & 0x1) << 3;
			colorIndex |= (((tmp[bytePos + 8000 * 4] & (1 << bitPos)) >> bitPos) & 0x1) << 4;
			*data++ = colorIndex;
		}
	}
}

void Screen::convertAmigaMsc(uint8 *data) {
	byte *plane1 = data + 5760 * 1;
	byte *plane2 = data + 5760 * 2;
	byte *plane3 = data + 5760 * 3;
	byte *plane4 = data + 5760 * 4;
	byte *plane5 = data + 5760 * 5;
	byte *plane6 = data + 5760 * 6;
	for (int i = 0; i < 5760; ++i) {
		byte d = plane6[i];
		d = (plane5[i] |= d);
		d = (plane4[i] |= d);
		d = (plane3[i] |= d);
		d = (plane2[i] |= d);
		d = (plane1[i] |= d);
	}
	byte dst[320*144];
	memset(dst, 0, sizeof(dst));
	static const byte flagTable[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
	for (int y = 0; y < 144; ++y) {
		for (int x = 0; x < 320; ++x) {
			if (!(flagTable[x&7] & data[y*40+(x>>3)]))
				dst[y*320+x] |= 0x80;

			int layer = 0;
			for (int i = 0; i < 7; ++i) {
				if (flagTable[x&7] & data[y*40+(x>>3)+i*5760])
					layer = i;
			}

			if (layer)
				dst[y*320+x] |= (layer+1);
		}
	}
	memcpy(data, dst, 320*144);
}

template<bool noXor>
void Screen::wrapped_decodeFrameDeltaPage(uint8 *dst, const uint8 *src, int pitch) {
	int count = 0;
	uint8 *dstNext = dst;
	while (1) {
		uint8 code = *src++;
		if (code == 0) {
			uint8 len = *src++;
			code = *src++;
			while (len--) {
				if (noXor)
					*dst++ = code;
				else
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
							if (noXor)
								*dst++ = code;
							else
								*dst++ ^= code;

							if (++count == pitch) {
								count = 0;
								dstNext += SCREEN_W;
								dst = dstNext;
							}
						}
					} else {
						while (subcode--) {
							if (noXor)
								*dst++ = *src++;
							else
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
				if (noXor)
					*dst++ = *src++;
				else
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
	if (_vm->gameFlags().useAltShapeHeader)
		shapeSize += 12;
	else
		shapeSize += 10;

	if (flags & 1)
		shapeSize += 16;

	static uint8 table[274];
	int tableIndex = 0;

	uint8 *newShape = 0;
	newShape = new uint8[shapeSize+16];
	assert(newShape);

	byte *dst = newShape;

	if (_vm->gameFlags().useAltShapeHeader)
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
			if (_vm->gameFlags().useAltShapeHeader)
				dst += 2;

			flags = READ_LE_UINT16(dst);
			flags |= 2;
			WRITE_LE_UINT16(dst, flags);
		} else {
			src = newShape;
			if (_vm->gameFlags().useAltShapeHeader)
				src += 2;
			if (flags & 1)
				src += 16;

			src += 10;
			uint8 *shapePtrBackUp = src;
			dst = _animBlockPtr;
			memcpy(dst, src, shapeSize2);

			int16 size = encodeShapeAndCalculateSize(_animBlockPtr, shapePtrBackUp, shapeSize2);
			if (size > shapeSize2) {
				shapeSize -= shapeSize2 - size;
				uint8 *newShape2 = new uint8[shapeSize];
				assert(newShape2);
				memcpy(newShape2, newShape, shapeSize);
				delete[] newShape;
				newShape = newShape2;
			} else {
				dst = shapePtrBackUp;
				src = _animBlockPtr;
				memcpy(dst, src, shapeSize2);
				dst = newShape;
				if (_vm->gameFlags().useAltShapeHeader)
					dst += 2;
				flags = READ_LE_UINT16(dst);
				flags |= 2;
				WRITE_LE_UINT16(dst, flags);
			}
		}
	}

	dst = newShape;
	if (_vm->gameFlags().useAltShapeHeader)
		dst += 2;
	WRITE_LE_UINT16((dst + 6), shapeSize);

	if (flags & 1) {
		dst = newShape + 10;
		if (_vm->gameFlags().useAltShapeHeader)
			dst += 2;
		src = &table[0x100];
		memcpy(dst, src, sizeof(uint8)*16);
	}

	return newShape;
}

int16 Screen::encodeShapeAndCalculateSize(uint8 *from, uint8 *to, int size_to) {
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
					*to++ = curPixel;
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
				if (*(to-1) == curPixel) {
					if (*(from+size-1) != *(to+size-2))
						continue;

					byte *fromBackUp = from;
					byte *toBackUp = to;
					--to;
					const int checkSize = fromPtrEnd - from;
					for (int i = 0; i < checkSize; ++i) {
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
				if (diffSize <= 0x0FFF) {
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

void Screen::hideMouse() {
	++_mouseLockCount;
	CursorMan.showMouse(false);
}

void Screen::showMouse() {
	if (_mouseLockCount == 1)
		CursorMan.showMouse(true);

	if (_mouseLockCount > 0)
		_mouseLockCount--;
}


bool Screen::isMouseVisible() const {
	return _mouseLockCount == 0;
}

void Screen::setShapePages(int page1, int page2, int minY, int maxY) {
	_shapePages[0] = _pagePtrs[page1];
	_shapePages[1] = _pagePtrs[page2];
	_maskMinY = minY;
	_maskMaxY = maxY;
}

void Screen::setMouseCursor(int x, int y, const byte *shape) {
	if (!shape)
		return;
	// if mouseDisabled
	//	return _mouseShape

	if (_vm->gameFlags().useAltShapeHeader)
		shape += 2;

	int mouseHeight = *(shape + 2);
	int mouseWidth = (READ_LE_UINT16(shape + 3)) + 2;

	if (_vm->gameFlags().useAltShapeHeader)
		shape -= 2;

	if (_vm->gameFlags().useHiResOverlay) {
		x <<= 1;
		y <<= 1;
		mouseWidth <<= 1;
		mouseHeight <<= 1;
		fillRect(mouseWidth, 0, mouseWidth, mouseHeight, 0, 8);
	}


	uint8 *cursor = new uint8[mouseHeight * mouseWidth];
	fillRect(0, 0, mouseWidth, mouseHeight, 0, 8);
	drawShape(8, shape, 0, 0, 0, 0);

	int xOffset = 0;

	if (_vm->gameFlags().useHiResOverlay) {
		xOffset = mouseWidth;
		scale2x(getPagePtr(8) + mouseWidth, SCREEN_W, getPagePtr(8), SCREEN_W, mouseWidth, mouseHeight);
	}

	CursorMan.showMouse(false);
	copyRegionToBuffer(8, xOffset, 0, mouseWidth, mouseHeight, cursor);
	CursorMan.replaceCursor(cursor, mouseWidth, mouseHeight, x, y, 0);
	if (isMouseVisible())
		CursorMan.showMouse(true);
	delete[] cursor;

	// makes sure that the cursor is drawn
	// we do not use Screen::updateScreen here
	// so we can be sure that changes to page 0
	// are NOT updated on the real screen here
	_system->updateScreen();
}

Palette &Screen::getPalette(int num) {
	assert(num >= 0 && num < (_vm->gameFlags().platform == Common::kPlatformAmiga ? 12 : 4));
	return *_palettes[num];
}

void Screen::copyPalette(const int dst, const int src) {
	getPalette(dst).copy(getPalette(src));
}

byte Screen::getShapeFlag1(int x, int y) {
	uint8 color = _shapePages[0][y * SCREEN_W + x];
	color &= 0x80;
	color ^= 0x80;

	if (color & 0x80)
		return 1;
	return 0;
}

byte Screen::getShapeFlag2(int x, int y) {
	uint8 color = _shapePages[0][y * SCREEN_W + x];
	color &= 0x7F;
	color &= 0x87;
	return color;
}

int Screen::getDrawLayer(int x, int y) {
	int xpos = x - 8;
	int ypos = y - 1;
	int layer = 1;

	for (int curX = xpos; curX < xpos + 16; ++curX) {
		int tempLayer = getShapeFlag2(curX, ypos);

		if (layer < tempLayer)
			layer = tempLayer;

		if (layer >= 7)
			return 7;
	}
	return layer;
}

int Screen::getDrawLayer2(int x, int y, int height) {
	int xpos = x - 8;
	int ypos = y - 1;
	int layer = 1;

	for (int useX = xpos; useX < xpos + 16; ++useX) {
		for (int useY = ypos - height; useY < ypos; ++useY) {
			int tempLayer = getShapeFlag2(useX, useY);

			if (tempLayer > layer)
				layer = tempLayer;

			if (tempLayer >= 7)
				return 7;
		}
	}
	return layer;
}


int Screen::setNewShapeHeight(uint8 *shape, int height) {
	if (_vm->gameFlags().useAltShapeHeader)
		shape += 2;

	int oldHeight = shape[2];
	shape[2] = height;
	return oldHeight;
}

int Screen::resetShapeHeight(uint8 *shape) {
	if (_vm->gameFlags().useAltShapeHeader)
		shape += 2;

	int oldHeight = shape[2];
	shape[2] = shape[5];
	return oldHeight;
}

void Screen::blockInRegion(int x, int y, int width, int height) {
	assert(_shapePages[0]);
	byte *toPtr = _shapePages[0] + (y * 320 + x);
	for (int i = 0; i < height; ++i) {
		byte *backUpTo = toPtr;
		for (int i2 = 0; i2 < width; ++i2)
			*toPtr++ &= 0x7F;
		toPtr = (backUpTo + 320);
	}
}

void Screen::blockOutRegion(int x, int y, int width, int height) {
	assert(_shapePages[0]);
	byte *toPtr = _shapePages[0] + (y * 320 + x);
	for (int i = 0; i < height; ++i) {
		byte *backUpTo = toPtr;
		for (int i2 = 0; i2 < width; ++i2)
			*toPtr++ |= 0x80;
		toPtr = (backUpTo + 320);
	}
}

void Screen::rectClip(int &x, int &y, int w, int h) {
	if (x < 0)
		x = 0;
	else if (x + w >= 320)
		x = 320 - w;

	if (y < 0)
		y = 0;
	else if (y + h >= 200)
		y = 200 - h;
}

void Screen::shakeScreen(int times) {
	while (times--) {
		// seems to be 1 line (320 pixels) offset in the original
		// 4 looks more like dosbox though, maybe check this again
		_system->setShakePos(4);
		_system->updateScreen();
		_system->setShakePos(0);
		_system->updateScreen();
	}
}

void Screen::loadBitmap(const char *filename, int tempPage, int dstPage, Palette *pal, bool skip) {
	uint32 fileSize;
	uint8 *srcData = _vm->resource()->fileData(filename, &fileSize);

	if (!srcData) {
		warning("couldn't load bitmap: '%s'", filename);
		return;
	}

	if (skip)
		srcData += 4;

	const char *ext = filename + strlen(filename) - 3;
	uint8 compType = srcData[2];
	uint32 imgSize = scumm_stricmp(ext, "CMP") ? READ_LE_UINT32(srcData + 4) : READ_LE_UINT16(srcData);
	uint16 palSize = READ_LE_UINT16(srcData + 8);

	if (pal && palSize)
		loadPalette(srcData + 10, *pal, palSize);

	uint8 *srcPtr = srcData + 10 + palSize;
	uint8 *dstData = getPagePtr(dstPage);
	if (dstPage == 0 || tempPage == 0)
		_forceFullUpdate = true;

	switch (compType) {
	case 0:
		memcpy(dstData, srcPtr, imgSize);
		break;
	case 3:
		Screen::decodeFrame3(srcPtr, dstData, imgSize);
		break;
	case 4:
		Screen::decodeFrame4(srcPtr, dstData, imgSize);
		break;
	default:
		error("Unhandled bitmap compression %d", compType);
	}

	if (_vm->gameFlags().platform == Common::kPlatformAmiga) {
		if (!scumm_stricmp(ext, "MSC"))
			Screen::convertAmigaMsc(dstData);
		else
			Screen::convertAmigaGfx(dstData, 320, 200, false);
	}

	if (skip)
		srcData -= 4;

	delete[] srcData;
}

bool Screen::loadPalette(const char *filename, Palette &pal) {
	Common::SeekableReadStream *stream = _vm->resource()->createReadStream(filename);

	if (!stream)
		return false;

	debugC(3, kDebugLevelScreen, "Screen::loadPalette('%s', %p)", filename, (const void *)&pal);

	if (_vm->gameFlags().platform == Common::kPlatformAmiga)
		pal.loadAmigaPalette(*stream, stream->size() / Palette::kAmigaBytesPerColor);
	else
		pal.loadVGAPalette(*stream, stream->size() / Palette::kVGABytesPerColor);

	delete stream;
	return true;
}

bool Screen::loadPaletteTable(const char *filename, int firstPalette) {
	Common::SeekableReadStream *stream = _vm->resource()->createReadStream(filename);

	if (!stream)
		return false;

	debugC(3, kDebugLevelScreen, "Screen::loadPaletteTable('%s', %d)", filename, firstPalette);

	if (_vm->gameFlags().platform == Common::kPlatformAmiga) {
		const int numColors = getPalette(firstPalette).getNumColors();
		const int palSize = getPalette(firstPalette).getNumColors() * Palette::kAmigaBytesPerColor;
		const int numPals = stream->size() / palSize;

		for (int i = 0; i < numPals; ++i)
			getPalette(i + firstPalette).loadAmigaPalette(*stream, numColors);
	} else {
		const int numColors = getPalette(firstPalette).getNumColors();
		const int palSize = getPalette(firstPalette).getNumColors() * Palette::kVGABytesPerColor;
		const int numPals = stream->size() / palSize;

		for (int i = 0; i < numPals; ++i)
			getPalette(i + firstPalette).loadVGAPalette(*stream, numColors);
	}

	delete stream;
	return true;
}

void Screen::loadPalette(const byte *data, Palette &pal, int bytes) {
	Common::MemoryReadStream stream(data, bytes, false);

	if (_vm->gameFlags().platform == Common::kPlatformAmiga)
		pal.loadAmigaPalette(stream, stream.size() / 2);
	else
		pal.loadVGAPalette(stream, stream.size() / 3);
}

// dirty rect handling

void Screen::addDirtyRect(int x, int y, int w, int h) {
	if (_dirtyRects.size() >= kMaxDirtyRects || _forceFullUpdate) {
		_forceFullUpdate = true;
		return;
	}

	Common::Rect r(x, y, x + w, y + h);

	// Clip rectangle
	r.clip(SCREEN_W, SCREEN_H);

	// If it is empty after clipping, we are done
	if (r.isEmpty())
		return;

	// Check if the new rectangle is contained within another in the list
	Common::List<Common::Rect>::iterator it;
	for (it = _dirtyRects.begin(); it != _dirtyRects.end(); ) {
		// If we find a rectangle which fully contains the new one,
		// we can abort the search.
		if (it->contains(r))
			return;

		// Conversely, if we find rectangles which are contained in
		// the new one, we can remove them
		if (r.contains(*it))
			it = _dirtyRects.erase(it);
		else
			++it;
	}

	// If we got here, we can safely add r to the list of dirty rects.
	_dirtyRects.push_back(r);
}

// overlay functions

byte *Screen::getOverlayPtr(int page) {
	if (page == 0 || page == 1)
		return _sjisOverlayPtrs[1];
	else if (page == 2 || page == 3)
		return _sjisOverlayPtrs[2];

	if (_vm->gameFlags().gameID == GI_KYRA2) {
		if (page == 12 || page == 13)
			return _sjisOverlayPtrs[3];
	}

	return 0;
}

void Screen::clearOverlayPage(int page) {
	byte *dst = getOverlayPtr(page);
	if (!dst)
		return;
	memset(dst, _sjisInvisibleColor, SCREEN_OVL_SJIS_SIZE);
}

void Screen::clearOverlayRect(int page, int x, int y, int w, int h) {
	byte *dst = getOverlayPtr(page);

	if (!dst || w < 0 || h < 0)
		return;

	x <<= 1; y <<= 1;
	w <<= 1; h <<= 1;

	dst += y * 640 + x;

	if (w == 640 && h == 400) {
		memset(dst, _sjisInvisibleColor, SCREEN_OVL_SJIS_SIZE);
	} else {
		while (h--) {
			memset(dst, _sjisInvisibleColor, w);
			dst += 640;
		}
	}
}

void Screen::copyOverlayRegion(int x, int y, int x2, int y2, int w, int h, int srcPage, int dstPage) {
	byte *dst = getOverlayPtr(dstPage);
	const byte *src = getOverlayPtr(srcPage);

	if (!dst || !src)
		return;

	x <<= 1; x2 <<= 1;
	y <<= 1; y2 <<= 1;
	w <<= 1; h <<= 1;

	if (w == 640 && h == 400) {
		memcpy(dst, src, SCREEN_OVL_SJIS_SIZE);
	} else {
		dst += y2 * 640 + x2;
		src += y * 640 + x;

		while (h--) {
			for (x = 0; x < w; ++x)
				memcpy(dst, src, w);
			dst += 640;
			src += 640;
		}
	}
}

// SJIS rendering

namespace {
int SJIStoFMTChunk(int f, int s) { // copied from scumm\charset.cpp
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
} // end of anonymous namespace

void Screen::drawCharSJIS(uint16 c, int x, int y) {
	int color1 = _textColorsMap[1];
	int color2 = _textColorsMap[0];

	memset(_sjisTempPage2, _sjisInvisibleColor, 324);
	memset(_sjisSourceChar, 0, 36);
	memcpy(_sjisSourceChar, _sjisFontData + 0x20 * SJIStoFMTChunk(c & 0xff, c >> 8), 0x20);

	if (_curPage == 0 || _curPage == 1)
		addDirtyRect(x, y, SJIS_CHARSIZE >> 1, SJIS_CHARSIZE >> 1);

	x <<= 1;
	y <<= 1;

	uint8 *destPage = getOverlayPtr(_curPage);
	if (!destPage) {
		warning("trying to draw SJIS char on unsupported page %d", _curPage);
		return;
	}

	destPage += y * 640 + x;
	uint8 *src = 0, *dst = 0;

	if (color2 != _sjisInvisibleColor) {
		// draw color2 shadow
		src = _sjisSourceChar;
		dst = _sjisTempPage2;

		for (int i = 0; i < SJIS_CHARSIZE; i++) {
			*((uint16*)dst) = READ_LE_UINT16(src);
			dst += 2; src += 2;
			*dst++ = 0;
		}

		src = _sjisTempPage2;
		dst = _sjisTempPage;
		memset(dst, 0, 60);
		for (int i = 0; i < 48; i++)
			*dst++ |= *src++;

		src = _sjisTempPage2;
		dst = _sjisTempPage + 3;
		for (int i = 0; i < 48; i++)
			*dst++ |= *src++;

		src = _sjisTempPage2;
		dst = _sjisTempPage + 6;
		for (int i = 0; i < 48; i++)
			*dst++ |= *src++;

		for (int i = 0; i < 2; i++) {
			src = _sjisTempPage;
			for (int ii = 0; ii < SJIS_CHARSIZE; ii++) {
				uint8 cy2 = 0;
				uint8 cy1 = 0;
				for (int iii = 0; iii < 3; iii++) {
					cy1 = *src & 1;
					*src |= ((*src >> 1) | (cy2 << 7));
					cy2 = cy1;
					src++;
				}
			}
		}

		src = _sjisTempPage2;
		for (int i = 0; i < SJIS_CHARSIZE; i++) {
			uint8 cy2 = 0;
			uint8 cy1 = 0;
			for (int ii = 0; ii < 3; ii++) {
				cy1 = *src & 1;
				*src = ((*src >> 1) | (cy2 << 7));
				cy2 = cy1;
				src++;
			}
		}

		src = _sjisTempPage2;
		dst = _sjisTempPage + 3;
		for (int i = 0; i < 48; i++)
			*dst++ ^= *src++;

		memset(_sjisTempPage2, _sjisInvisibleColor, 324);
		src = _sjisTempPage;
		dst = _sjisTempPage2;

		uint8 height = SJIS_CHARSIZE * 3;
		uint8 width = SJIS_CHARSIZE;
		if (color2 & 0xff00) {
			height -= 3;
			width--;
			dst += 0x13;
		}

		for (int i = 0; i < height; i++) {
			uint8 rs = *src++;
			for (int ii = 0; ii < 8; ii++) {
				if (rs & 0x80)
					*dst = (color2 & 0xff);
				rs <<= 1;
				dst++;

				if (!--width) {
					width = SJIS_CHARSIZE;
					if (color2 & 0xff00) {
						width--;
						dst++;
					}
					break;
				}
			}
		}
	}

	//	draw color1 char
	src = _sjisSourceChar;
	dst = _sjisTempPage;

	for (int i = 0; i < SJIS_CHARSIZE; i++) {
		*(uint16*)dst = READ_LE_UINT16(src);
		dst += 2; src += 2;
		*dst++ = 0;
	}

	src = _sjisTempPage;
	dst = _sjisTempPage2;
	if (color2 != _sjisInvisibleColor)
		color1 = (color1 & 0xff) | 0x100;

	uint8 height = SJIS_CHARSIZE * 3;
	uint8 width = SJIS_CHARSIZE;
	if (color1 & 0xff00) {
		height -= 3;
		width--;
		dst += 0x13;
	}

	for (int i = 0; i < height; i++) {
		uint8 rs = *src++;
		for (int ii = 0; ii < 8; ii++) {
			if (rs & 0x80)
				*dst = (color1 & 0xff);
			rs <<= 1;
			dst++;

			if (!--width) {
				width = SJIS_CHARSIZE;
				if (color1 & 0xff00) {
					width--;
					dst++;
				}
				break;
			}
		}
	}

	//	copy char to surface
	src = _sjisTempPage2;
	dst = destPage;
	int pitch = 640 - SJIS_CHARSIZE;

	for (int i = 0; i < SJIS_CHARSIZE; i++) {
		for (int ii = 0; ii < SJIS_CHARSIZE; ii++) {
			if (*src != _sjisInvisibleColor)
				*dst = *src;
			src++;
			dst++;
		}
		dst += pitch;
	}
}

#pragma mark -

Palette::Palette(const int numColors) : _palData(0), _numColors(numColors) {
	_palData = new uint8[numColors * 3];
	assert(_palData);

	memset(_palData, 0, numColors * 3);
}

Palette::~Palette() {
	delete[] _palData;
	_palData = 0;
}

void Palette::loadVGAPalette(Common::ReadStream &stream, int colors) {
	if (colors == -1)
		colors = _numColors;

	assert(colors <= _numColors);

	stream.read(_palData, colors * 3);
	memset(_palData + colors * 3, 0, (_numColors - colors) * 3);
}

void Palette::loadAmigaPalette(Common::ReadStream &stream, int colors) {
	if (colors == -1)
		colors = _numColors;

	assert(colors <= _numColors);

	for (int i = 0; i < colors; ++i) {
		uint16 col = stream.readUint16BE();
		_palData[i * 3 + 2] = (col & 0xF) << 2; col >>= 4;
		_palData[i * 3 + 1] = (col & 0xF) << 2; col >>= 4;
		_palData[i * 3 + 0] = (col & 0xF) << 2; col >>= 4;
	}

	memset(_palData + colors * 3, 0, (_numColors - colors) * 3);
}

void Palette::clear() {
	memset(_palData, 0, _numColors * 3);
}

void Palette::copy(const Palette &source, int firstCol, int numCols, int dstStart) {
	if (numCols == -1)
		numCols = MIN(source.getNumColors(), _numColors) - firstCol;
	if (dstStart == -1)
		dstStart = firstCol;

	assert(numCols >= 0 && numCols <= _numColors);
	assert(firstCol >= 0 && firstCol <= source.getNumColors());
	assert(dstStart >= 0 && dstStart + numCols <= _numColors);

	memcpy(_palData + dstStart * 3, source._palData + firstCol * 3, numCols * 3);
}

void Palette::copy(const uint8 *source, int firstCol, int numCols, int dstStart) {
	if (source == _palData)
		return;

	if (dstStart == -1)
		dstStart = firstCol;

	assert(numCols >= 0 && numCols <= _numColors);
	assert(firstCol >= 0);
	assert(dstStart >= 0 && dstStart + numCols <= _numColors);

	memcpy(_palData + dstStart * 3, source + firstCol * 3, numCols * 3);
}

uint8 *Palette::fetchRealPalette() const {
	uint8 *buffer = new uint8[_numColors * 3];
	assert(buffer);

	uint8 *dst = buffer;
	const uint8 *palData = _palData;

	for (int i = 0; i < _numColors; ++i) {
		dst[0] = (palData[0] << 2) | (palData[0] & 3);
		dst[1] = (palData[1] << 2) | (palData[1] & 3);
		dst[2] = (palData[2] << 2) | (palData[2] & 3);

		dst += 3;
		palData += 3;
	}

	return buffer;
}

} // End of namespace Kyra

