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
 */

#include "common/endian.h"

#include "scumm/scumm.h"
#include "scumm/charset.h"
#include "scumm/util.h"
#include "scumm/resource.h"

#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE

namespace Scumm {

void ScummEngine::towns_drawStripToScreen(VirtScreen *vs, int dstX, int dstY, int srcX, int srcY, int width, int height) {
	if (width <= 0 || height <= 0 || !_townsScreen)
		return;

	assert(_textSurface.getPixels());
	int m = _textSurfaceMultiplier;

	const uint8 *src1 = vs->getPixels(srcX, srcY);
	const uint8 *src2 = (uint8 *)_textSurface.getBasePtr(srcX * m, (srcY + vs->topline - _screenTop) * m);
	int dstXScr = dstX + _scrollDestOffset;
	uint8 *dst1 = _townsScreen->getLayerPixels(0, dstXScr, dstY);
	uint16 *dst1a = (uint16*)dst1;
	uint8 *dst2 = _townsScreen->getLayerPixels(1, dstX * m, dstY * m);

	int lw1 = _townsScreen->getLayerWidth(0);
	int dp2 = _townsScreen->getLayerPitch(1) - width * m * _townsScreen->getLayerBpp(1);
	int sp1 = vs->pitch - (width * vs->format.bytesPerPixel);
	int sp2 = _textSurface.pitch - width * m;

	if (vs->number == kMainVirtScreen || _game.id == GID_INDY3 || _game.id == GID_ZAK) {
		if (_outputPixelFormat.bytesPerPixel == 2) {
			for (int h = 0; h < height; ++h) {
				uint16 *dst1tmp = dst1a;
				int xpos = dstXScr;
				for (int w = 0; w < width; ++w) {
					*dst1a++ = _16BitPalette[*src1++];
					if (++xpos == lw1) {
						dst1a -= lw1;
						xpos = 0;
					}
				}
				src1 += sp1;
				dst1a = dst1tmp + lw1;
			}
		} else {
			for (int h = 0; h < height; ++h) {
				uint8 *dst1tmp = dst1;
				int xpos = dstXScr;
				for (int w = 0; w < width; ++w) {
					*dst1++ = *src1++;
					if (++xpos == lw1) {
						dst1 -= lw1;
						xpos = 0;
					}
				}
				src1 += sp1;
				dst1 = dst1tmp + lw1;
			}
		}

		for (int h = 0; h < height * m; ++h) {
			memcpy(dst2, src2, width * m);
			src2 += _textSurface.pitch;
			dst2 += _townsScreen->getLayerPitch(1);
		}

	} else {
		dst1 = dst2;
		for (int h = 0; h < height; ++h) {
			for (int w = 0; w < width; ++w) {
				uint8 t = (*src1++) & 0x0f;
				memset(dst1, (t << 4) | t, m);
				dst1 += m;
			}

			dst1 = dst2;
			const uint8 *src3 = src2;

			if (m == 2) {
				dst2 += _townsScreen->getLayerPitch(1);
				src3 += _townsScreen->getLayerPitch(1);
			}

			for (int w = 0; w < width * m; ++w) {
				*dst2++ = (*src3 | (*dst1 & _townsLayer2Mask[*src3]));
				*dst1 = (*src2 | (*dst1 & _townsLayer2Mask[*src2]));
				src2++;
				src3++;
				dst1++;
			}

			src1 += sp1;
			src2 = src3 + sp2;
			dst1 = dst2 + dp2;
			dst2 += dp2;
		}
	}

	_townsScreen->addDirtyRect(dstX * m, dstY * m, width * m, height * m);
}

void ScummEngine::towns_clearStrip(int strip) {
	if (!_townsScreen)
		return;

	int h = _screenHeight;
	int pitch = _townsScreen->getLayerPitch(0) >> 2;
	uint32 *dst1 = (uint32*)_townsScreen->getLayerPixels(0, (strip * 8 + _scrollDestOffset) % _townsScreen->getLayerWidth(0), 0);
	if (_game.version == 5) {
		dst1 += (90 * pitch);
		h = 56;
	}
	uint32 *dst2 = dst1 + 1;
	uint32 *dst3 = dst2 + 1;
	uint32 *dst4 = dst3 + 1;

	if (_townsScreen->getLayerBpp(0) == 2) {
		while (h--) {
			*dst1 = *dst2 = *dst3 = *dst4 = 0;
			dst1 += pitch;
			dst2 += pitch;
			dst3 += pitch;
			dst4 += pitch;
		}
	} else {
		while (h--) {
			*dst1 = *dst2 = 0;
			dst1 += pitch;
			dst2 += pitch;
		}
	}
}

void ScummEngine::requestScroll(int dir) {
	if (_game.platform == Common::kPlatformFMTowns) {
		int lw = _townsScreen->getLayerWidth(0);
		// Wait for opposite direction scroll to finish.
		towns_waitForScroll(-dir);

		// WORKAROUND: In the LOOM intro the messenger nymph moves up the mountain so fast
		// (and thus accumulates so many scroll requests) that this will make the graphics
		// layer (width 512) wrap around. This causes a visible graphics glitch. The glitch
		// is exactly the same when starting the game in the UNZ emulator, so this is not
		// a mistake on my part. I work around this by adding an extra wait if the queued
		// up scroll requests exceed the critical mark. It is actually the exact same fix
		// that the SCUMM 5 games use for scrollEffect().
		towns_waitForScroll(dir, (lw - _screenWidth - 8) * dir);

		_scrollDestOffset = (_scrollDestOffset - 8 * dir) % lw;
		_scrollRequest += (8 * dir);
		towns_clearStrip(_scrollFeedStrips[dir + 1]);
	}
	redrawBGStrip(_scrollFeedStrips[dir + 1], 1);
}

void ScummEngine::towns_waitForScroll(int waitForDirection, int threshold) {
	while (!shouldQuit() && _townsScreen && (_scrollRequest || _townsScreen->isScrolling(waitForDirection, threshold)))
		waitForTimer(0);
}

void ScummEngine::towns_updateGfx() {
	if (!_townsScreen)
		return;

	uint32 cur = _system->getMillis();
	while (_scrollTimer <= cur) {
		if (!_scrollTimer)
			_scrollTimer = cur;
		_scrollTimer += 1000 / 60;
		_townsScreen->scrollLayers(1, _scrollRequest);
		if (_townsScreen->isScrolling(0))
			_scrollDeltaAdjust = 1;
		_scrollRequest = 0;
	}

	_townsScreen->update();
}

bool ScummEngine::towns_isRectInStringBox(int x1, int y1, int x2, int y2) {
	if (_game.platform == Common::kPlatformFMTowns && _charset->_hasMask && y1 <= _curStringRect.bottom && x1 <= _curStringRect.right && y2 >= _curStringRect.top && x2 >= _curStringRect.left)
		return true;
	return false;
}

void ScummEngine::towns_restoreCharsetBg() {
	if (_curStringRect.left != -1) {
		restoreBackground(_curStringRect, 0);
		_curStringRect.left = -1;
		_charset->_hasMask = false;
		_nextLeft = _string[0].xpos;
	}

	_nextLeft = _string[0].xpos;
	_nextTop = _string[0].ypos;
}

void ScummEngine::towns_scriptScrollEffect(int dir) {
	VirtScreen *vs = &_virtscr[kMainVirtScreen];
	const int stripWidth = 8;
	int destX = _gdi->_numStrips - 1;
	int srcX = 0;
	int layerW = _townsScreen->getLayerWidth(0);
	int threshold = (layerW - vs->w - stripWidth) * dir;
	if (dir == 1)
		SWAP(destX, srcX);

	// Wait for opposite direction scroll to finish.
	towns_waitForScroll(-dir);

	for (int x = 0; !shouldQuit() && x < _gdi->_numStrips; ++x) {
		_scrollDestOffset = (_scrollDestOffset - (dir << 3)) % layerW;
		uint32 nextFrame = _system->getMillis() + 1000 / 60;
		// Same as in requestScroll(): This prevents glitches from graphics layer wrapping.
		towns_waitForScroll(dir, threshold);
		_townsScreen->scrollLayers(0, dir << 3);
		towns_drawStripToScreen(vs, destX << 3, vs->topline, (srcX + (-dir * x)) << 3, 0, stripWidth, vs->h);
		waitForTimer(nextFrame - _system->getMillis());
	}

	// Wait for all scrolls to finish.
	towns_waitForScroll(0);
}

#ifdef USE_RGB_COLOR
void ScummEngine::towns_setPaletteFromPtr(const byte *ptr, int numcolor) {
	setPaletteFromPtr(ptr, numcolor);

	if (_game.version == 5)
		towns_setTextPaletteFromPtr(_currentPalette);

	_townsOverrideShadowColor = 1;
	int m = 48;
	for (int i = 1; i < 16; ++i) {
		int val = _currentPalette[i * 3] + _currentPalette[i * 3 + 1] + _currentPalette[i * 3 + 2];
		if (m > val) {
			_townsOverrideShadowColor = i;
			m = val;
		}
	}
}

void ScummEngine::towns_setTextPaletteFromPtr(const byte *ptr) {
	memcpy(_textPalette, ptr, 48);
}
#endif

void ScummEngine::towns_setupPalCycleField(int x1, int y1, int x2, int y2) {
	if (_numCyclRects >= 10)
		return;
	_cyclRects[_numCyclRects].left = x1;
	_cyclRects[_numCyclRects].top = y1;
	_cyclRects[_numCyclRects].right = x2;
	_cyclRects[_numCyclRects].bottom = y2;
	_numCyclRects++;
	_townsPaletteFlags |= 1;
}

void ScummEngine::towns_processPalCycleField() {
	for (int i = 0; i < _numCyclRects; i++) {
		int x1 = _cyclRects[i].left - _virtscr[kMainVirtScreen].xstart;
		int x2 = _cyclRects[i].right - _virtscr[kMainVirtScreen].xstart;
		if (x1 < 0)
			x1 = 0;
		if (x2 > 320)
			x2 = 320;
		if (x2 > 0)
			markRectAsDirty(kMainVirtScreen, x1, x2, _cyclRects[i].top, _cyclRects[i].bottom);
	}
}

void ScummEngine::towns_resetPalCycleFields() {
	_numCyclRects = 0;
	_townsPaletteFlags &= ~1;
}

const uint8 ScummEngine::_townsLayer2Mask[] = {
	0xFF, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define DIRTY_RECTS_MAX 20
#define FULL_REDRAW (DIRTY_RECTS_MAX + 1)

TownsScreen::TownsScreen(OSystem *system, int width, int height, Graphics::PixelFormat &format) :
	_system(system), _width(width), _height(height), _pixelFormat(format), _pitch(width * format.bytesPerPixel), _scrollOffset(0), _scrollRemainder(0) {
	memset(&_layers[0], 0, sizeof(TownsScreenLayer));
	memset(&_layers[1], 0, sizeof(TownsScreenLayer));
	_outBuffer = new byte[_pitch * _height];
	memset(_outBuffer, 0, _pitch * _height);

	setupLayer(0, width, height, 1, 1, 256);

	_numDirtyRects = 0;
}

TownsScreen::~TownsScreen() {
	delete[] _layers[0].pixels;
	delete[] _layers[1].pixels;
	delete[] _layers[0].bltInternX;
	delete[] _layers[1].bltInternX;
	delete[] _layers[0].bltInternY;
	delete[] _layers[1].bltInternY;
	delete[] _layers[0].bltTmpPal;
	delete[] _layers[1].bltTmpPal;
	delete[] _outBuffer;
	_dirtyRects.clear();
}

void TownsScreen::setupLayer(int layer, int width, int height, int scaleW, int scaleH, int numCol, void *pal) {
	if (layer < 0 || layer > 1)
		return;

	TownsScreenLayer *l = &_layers[layer];

	if (numCol >> 15)
		error("TownsScreen::setupLayer(): No more than 32767 colors supported.");

	if (width <= 0 || height <= 0 || numCol < 16)
		error("TownsScreen::setupLayer(): Invalid width/height/number of colors setting.");

	l->width = width;
	l->height = height;
	l->scaleW = scaleW;
	l->scaleH = scaleH;
	l->modW = width * scaleW;
	l->numCol = numCol;
	l->bpp = ((numCol - 1) & 0xff00) ? 2 : 1;
	l->pitch = width * l->bpp;
	l->palette = (uint8 *)pal;
	l->hScroll = 0;

	if (l->palette && _pixelFormat.bytesPerPixel == 1)
		warning("TownsScreen::setupLayer(): Layer palette usage requires 16 bit graphics setting.\nLayer palette will be ignored.");

	delete[] l->pixels;
	l->pixels = new uint8[l->pitch * l->height];
	assert(l->pixels);
	memset(l->pixels, 0, l->pitch * l->height);

	// build offset tables to speed up merging/scaling layers
	delete[] l->bltInternX;
	width = MAX<int>(_width, l->modW);
	l->bltInternX = new uint16[width];
	for (int i = 0; i < width; ++i)
		l->bltInternX[i] = (i / l->scaleW) * l->bpp;

	delete[] l->bltInternY;
	height = MAX<int>(_height, l->modW);
	l->bltInternY = new uint8*[height];
	for (int i = 0; i < height; ++i)
		l->bltInternY[i] = l->pixels + (i / l->scaleH) * l->pitch;

	delete[] l->bltTmpPal;
	l->bltTmpPal = (l->bpp == 1 && _pixelFormat.bytesPerPixel == 2) ? new uint16[l->numCol] : 0;

	l->enabled = true;
	_layers[0].onBottom = true;
	_layers[1].onBottom = !_layers[0].enabled;
	l->ready = true;
}

void TownsScreen::clearLayer(int layer) {
	if (layer < 0 || layer > 1)
		return;

	TownsScreenLayer *l = &_layers[layer];
	if (!l->ready)
		return;

	memset(l->pixels, 0, l->pitch * l->height);
	_dirtyRects.push_back(Common::Rect(_width - 1, _height - 1));
	_numDirtyRects = FULL_REDRAW;
}


void TownsScreen::fillLayerRect(int layer, int x, int y, int w, int h, int col) {
	if (layer < 0 || layer > 1 || w <= 0 || h <= 0)
		return;

	TownsScreenLayer *l = &_layers[layer];
	if (!l->ready)
		return;

	assert(x >= 0 && y >= 0 && ((x + w) * l->bpp) <= (l->pitch) && (y + h) <= (l->height));

	uint8 *pos = l->pixels + y * l->pitch + x * l->bpp;

	for (int i = 0; i < h; ++i) {
		if (l->bpp == 2) {
			for (int ii = 0; ii < w; ++ii) {
				*(uint16 *)pos = col;
				pos += 2;
			}
			pos += (l->pitch - w * 2);
		} else {
			memset(pos, col, w);
			pos += l->pitch;
		}
	}
	addDirtyRect(x * l->scaleW, y * l->scaleH, w * l->scaleW, h * l->scaleH);
}

uint8 *TownsScreen::getLayerPixels(int layer, int x, int y) const {
	if (layer < 0 || layer > 1)
		return 0;

	const TownsScreenLayer *l = &_layers[layer];
	if (!l->ready)
		return 0;

	return l->pixels + y * l->pitch + (x % l->width) * l->bpp;
}

void TownsScreen::addDirtyRect(int x, int y, int w, int h) {
	if (w <= 0 || h <= 0 || _numDirtyRects > DIRTY_RECTS_MAX)
		return;

	if (_numDirtyRects == DIRTY_RECTS_MAX) {
		// full redraw
		_dirtyRects.clear();
		_dirtyRects.push_back(Common::Rect(_width - 1, _height - 1));
		_numDirtyRects++;
		return;
	}

	int x2 = x + w - 1;
	int y2 = y + h - 1;

	assert(x >= 0 && y >= 0 && x2 <= _width && y2 <= _height);

	bool skip = false;
	for (Common::List<Common::Rect>::iterator r = _dirtyRects.begin(); r != _dirtyRects.end(); ++r) {
		// Try to merge new rect with an existing rect (only once, since trying to merge
		// more than one overlapping rect would be causing more overhead than doing any good).
		if (x > r->left && x < r->right && y > r->top && y < r->bottom) {
			x = r->left;
			y = r->top;
			skip = true;
		}

		if (x2 > r->left && x2 < r->right && y > r->top && y < r->bottom) {
			x2 = r->right;
			y = r->top;
			skip = true;
		}

		if (x2 > r->left && x2 < r->right && y2 > r->top && y2 < r->bottom) {
			x2 = r->right;
			y2 = r->bottom;
			skip = true;
		}

		if (x > r->left && x < r->right && y2 > r->top && y2 < r->bottom) {
			x = r->left;
			y2 = r->bottom;
			skip = true;
		}

		if (skip) {
			r->left = x;
			r->top = y;
			r->right = x2;
			r->bottom = y2;
			break;
		}
	}

	if (!skip) {
		_dirtyRects.push_back(Common::Rect(x, y, x2, y2));
		_numDirtyRects++;
	}
}

void TownsScreen::toggleLayers(int flags) {
	if (flags < 0 || flags > 3)
		return;

	_layers[0].enabled = (flags & 1) ? true : false;
	_layers[0].onBottom = true;
	_layers[1].enabled = (flags & 2) ? true : false;
	_layers[1].onBottom = !_layers[0].enabled;

	_dirtyRects.clear();
	_dirtyRects.push_back(Common::Rect(_width - 1, _height - 1));
	_numDirtyRects = FULL_REDRAW;

	memset(_outBuffer, 0, _pitch * _height);
	update();

	_system->updateScreen();
}

void TownsScreen::scrollLayers(int flags, int offset) {
	// This actually supports layer 0 only, since this is all we need.
	_scrollRemainder += offset;
	if (!_scrollRemainder)
		return;

	int step = (_scrollRemainder > 0) ? -1 : 1;
	_scrollRemainder += step;
	_scrollOffset = (_scrollOffset + step) % _layers[0].width;

	_dirtyRects.clear();
	_dirtyRects.push_back(Common::Rect(_width - 1, _height - 1));
	_numDirtyRects = FULL_REDRAW;

	for (int i = 0; i < 2; ++i) {
		if (!(flags & (1 << i)))
			continue;
		TownsScreenLayer *l = &_layers[i];
		if (l->ready)
			l->hScroll = (_scrollOffset * l->scaleH) % l->modW;
	}
}

void TownsScreen::update() {
	updateOutputBuffer();
	outputToScreen();
}

void TownsScreen::updateOutputBuffer() {
	for (Common::List<Common::Rect>::iterator r = _dirtyRects.begin(); r != _dirtyRects.end(); ++r) {
		for (int i = 0; i < 2; i++) {
			TownsScreenLayer *l = &_layers[i];
			if (!l->enabled || !l->ready)
				continue;

			uint8 *dst = _outBuffer + r->top * _pitch + r->left * _pixelFormat.bytesPerPixel;
			int pitch2 = _pitch - (r->right - r->left + 1) * _pixelFormat.bytesPerPixel;

			if (_pixelFormat.bytesPerPixel == 2 && l->bpp == 1) {
				if (!l->palette)
					error("void TownsScreen::updateOutputBuffer(): No palette assigned to 8 bit layer %d", i);
				for (int ic = 0; ic < l->numCol; ic++)
					l->bltTmpPal[ic] = calc16BitColor(&l->palette[ic * 3]);
			}

			for (int y = r->top; y <= r->bottom; ++y) {
				if (l->bpp == _pixelFormat.bytesPerPixel && l->scaleW == 1 && l->onBottom && l->numCol & 0xff00 && + !l->hScroll) {
					memcpy(dst, &l->bltInternY[y][l->bltInternX[r->left]], (r->right + 1 - r->left) * _pixelFormat.bytesPerPixel);
					dst += _pitch;

				} else if (_pixelFormat.bytesPerPixel == 2) {
					uint16 *dst2 = (uint16*)dst;
					int x = (r->left + l->hScroll) % l->modW;
					if (l->bpp == 1) {
						for (int w = r->right - r->left; w >= 0; --w) {
							uint8 col = l->bltInternY[y][l->bltInternX[x]];
							if (col || l->onBottom) {
								if (l->numCol == 16)
									col = (col >> 4) & (col & 0x0f);
								*dst2 = l->bltTmpPal[col];
							}
							dst2++;
							x = (x + 1) % l->modW;
						}
					} else {
						for (int w = r->right - r->left; w >= 0; --w) {
							*dst2++ = *(uint16*)&l->bltInternY[y][l->bltInternX[x]];
							x = (x + 1) % l->modW;
						}
					}
					dst += _pitch;

				} else {
					int x = (r->left + l->hScroll) % l->modW;
					for (int w = r->right - r->left; w >= 0; --w) {
						uint8 col = l->bltInternY[y][l->bltInternX[x]];
						if (col || l->onBottom) {
							if (l->numCol == 16)
								col = (col >> 4) & (col & 0x0f);
							*dst = col;
						}
						dst++;
						x = (x + 1) % l->modW;
					}
					dst += pitch2;
				}
			}
		}
	}
}

void TownsScreen::outputToScreen() {
	for (Common::List<Common::Rect>::iterator i = _dirtyRects.begin(); i != _dirtyRects.end(); ++i)
		_system->copyRectToScreen(_outBuffer + i->top * _pitch + i->left * _pixelFormat.bytesPerPixel, _pitch, i->left, i->top, i->right - i->left + 1, i->bottom - i->top + 1);
	_dirtyRects.clear();
	_numDirtyRects = 0;
}

uint16 TownsScreen::calc16BitColor(const uint8 *palEntry) {
	return _pixelFormat.RGBToColor(palEntry[0], palEntry[1], palEntry[2]);
}

#undef DIRTY_RECTS_MAX
#undef FULL_REDRAW

} // End of namespace Scumm

#endif // DISABLE_TOWNS_DUAL_LAYER_MODE
