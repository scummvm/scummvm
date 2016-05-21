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

#include "graphics/palette.h"
#include "video/smk_decoder.h"
#include "neverhood/screen.h"

namespace Neverhood {

Screen::Screen(NeverhoodEngine *vm)
	: _vm(vm), _paletteData(NULL), _paletteChanged(false), _smackerDecoder(NULL),
	_yOffset(0), _fullRefresh(false), _frameDelay(0), _savedSmackerDecoder(NULL),
	_savedFrameDelay(0), _savedYOffset(0) {

	_ticks = _vm->_system->getMillis();

	_backScreen = new Graphics::Surface();
	_backScreen->create(640, 480, Graphics::PixelFormat::createFormatCLUT8());

	_renderQueue = new RenderQueue();
	_prevRenderQueue = new RenderQueue();
	_microTiles = new MicroTileArray(640, 480);

}

Screen::~Screen() {
	delete _microTiles;
	delete _renderQueue;
	delete _prevRenderQueue;
	_backScreen->free();
	delete _backScreen;
}

void Screen::update() {
	_ticks = _vm->_system->getMillis();
	updatePalette();

	if (_fullRefresh) {
		// NOTE When playing a fullscreen/doubled Smacker video usually a full screen refresh is needed
		_vm->_system->copyRectToScreen((const byte*)_backScreen->getPixels(), _backScreen->pitch, 0, 0, 640, 480);
		_fullRefresh = false;
		return;
	}

	_microTiles->clear();

	for (RenderQueue::iterator it = _renderQueue->begin(); it != _renderQueue->end(); ++it) {
		RenderItem &renderItem = (*it);
		renderItem._refresh = true;
		for (RenderQueue::iterator jt = _prevRenderQueue->begin(); jt != _prevRenderQueue->end(); ++jt) {
			RenderItem &prevRenderItem = (*jt);
			if (prevRenderItem == renderItem) {
				prevRenderItem._refresh = false;
				renderItem._refresh = false;
			}
		}
	}

	for (RenderQueue::iterator jt = _prevRenderQueue->begin(); jt != _prevRenderQueue->end(); ++jt) {
		RenderItem &prevRenderItem = (*jt);
		if (prevRenderItem._refresh)
			_microTiles->addRect(Common::Rect(prevRenderItem._destX, prevRenderItem._destY, prevRenderItem._destX + prevRenderItem._width, prevRenderItem._destY + prevRenderItem._height));
	}

	for (RenderQueue::iterator it = _renderQueue->begin(); it != _renderQueue->end(); ++it) {
		RenderItem &renderItem = (*it);
		if (renderItem._refresh)
			_microTiles->addRect(Common::Rect(renderItem._destX, renderItem._destY, renderItem._destX + renderItem._width, renderItem._destY + renderItem._height));
		renderItem._refresh = true;
	}

	RectangleList *updateRects = _microTiles->getRectangles();

	for (RenderQueue::iterator it = _renderQueue->begin(); it != _renderQueue->end(); ++it) {
		RenderItem &renderItem = (*it);
		for (RectangleList::iterator ri = updateRects->begin(); ri != updateRects->end(); ++ri)
			blitRenderItem(renderItem, *ri);
	}

	SWAP(_renderQueue, _prevRenderQueue);
	_renderQueue->clear();

	for (RectangleList::iterator ri = updateRects->begin(); ri != updateRects->end(); ++ri) {
		Common::Rect &r = *ri;
		_vm->_system->copyRectToScreen((const byte*)_backScreen->getBasePtr(r.left, r.top), _backScreen->pitch, r.left, r.top, r.width(), r.height());
	}

	delete updateRects;

}

uint32 Screen::getNextFrameTime() {
	int32 frameDelay = _frameDelay;
	if (_smackerDecoder && _smackerDecoder->isVideoLoaded() && !_smackerDecoder->endOfVideo())
		frameDelay = _smackerDecoder->getTimeToNextFrame();
	int32 waitTicks = frameDelay - (_vm->_system->getMillis() - _ticks);
	return _vm->_system->getMillis() + waitTicks;
}

void Screen::saveParams() {
	_savedSmackerDecoder = _smackerDecoder;
	_savedFrameDelay = _frameDelay;
	_savedYOffset = _yOffset;
}

void Screen::restoreParams() {
	_smackerDecoder = _savedSmackerDecoder;
	_frameDelay = _savedFrameDelay;
	_yOffset = _savedYOffset;
}

void Screen::setFps(int fps) {
	_frameDelay = 1000 / fps;
}

int Screen::getFps() {
	return 1000 / _frameDelay;
}

void Screen::setYOffset(int16 yOffset) {
	_yOffset = yOffset;
}

int16 Screen::getYOffset() {
	return _yOffset;
}

void Screen::setPaletteData(byte *paletteData) {
	_paletteChanged = true;
	_paletteData = paletteData;
}

void Screen::unsetPaletteData(byte *paletteData) {
	if (_paletteData == paletteData) {
		_paletteChanged = false;
		_paletteData = NULL;
	}
}

void Screen::testPalette(byte *paletteData) {
	if (_paletteData == paletteData)
		_paletteChanged = true;
}

void Screen::updatePalette() {
	if (_paletteChanged && _paletteData) {
		byte *tempPalette = new byte[768];
		for (int i = 0; i < 256; i++) {
			tempPalette[i * 3 + 0] = _paletteData[i * 4 + 0];
			tempPalette[i * 3 + 1] = _paletteData[i * 4 + 1];
			tempPalette[i * 3 + 2] = _paletteData[i * 4 + 2];
		}
		_vm->_system->getPaletteManager()->setPalette(tempPalette, 0, 256);
		delete[] tempPalette;
		_paletteChanged = false;
	}
}

void Screen::clear() {
	memset(_backScreen->getPixels(), 0, _backScreen->pitch * _backScreen->h);
	_fullRefresh = true;
	clearRenderQueue();
}

void Screen::clearRenderQueue() {
	_renderQueue->clear();
	_prevRenderQueue->clear();
}

void Screen::drawSurface2(const Graphics::Surface *surface, NDrawRect &drawRect, NRect &clipRect, bool transparent, byte version,
	const Graphics::Surface *shadowSurface) {

	int16 destX, destY;
	NRect ddRect;

	if (drawRect.x + drawRect.width >= clipRect.x2)
		ddRect.x2 = clipRect.x2 - drawRect.x;
	else
		ddRect.x2 = drawRect.width;

	if (drawRect.x < clipRect.x1) {
		destX = clipRect.x1;
		ddRect.x1 = clipRect.x1 - drawRect.x;
	} else {
		destX = drawRect.x;
		ddRect.x1 = 0;
	}

	if (drawRect.y + drawRect.height >= clipRect.y2)
		ddRect.y2 = clipRect.y2 - drawRect.y;
	else
		ddRect.y2 = drawRect.height;

	if (drawRect.y < clipRect.y1) {
		destY = clipRect.y1;
		ddRect.y1 = clipRect.y1 - drawRect.y;
	} else {
		destY = drawRect.y;
		ddRect.y1 = 0;
	}

	queueBlit(surface, destX, destY, ddRect, transparent, version, shadowSurface);

}

void Screen::drawSurface3(const Graphics::Surface *surface, int16 x, int16 y, NDrawRect &drawRect, NRect &clipRect, bool transparent, byte version) {

	int16 destX, destY;
	NRect ddRect;

	if (x + drawRect.width >= clipRect.x2)
		ddRect.x2 = clipRect.x2 - drawRect.x - x;
	else
		ddRect.x2 = drawRect.x + drawRect.width;

	if (x < clipRect.x1) {
		destX = clipRect.x1;
		ddRect.x1 = clipRect.x1 + drawRect.x - x;
	} else {
		destX = x;
		ddRect.x1 = drawRect.x;
	}

	if (y + drawRect.height >= clipRect.y2)
		ddRect.y2 = clipRect.y2 + drawRect.y - y;
	else
		ddRect.y2 = drawRect.y + drawRect.height;

	if (y < clipRect.y1) {
		destY = clipRect.y1;
		ddRect.y1 = clipRect.y1 + drawRect.y - y;
	} else {
		destY = y;
		ddRect.y1 = drawRect.y;
	}

	queueBlit(surface, destX, destY, ddRect, transparent, version);

}

void Screen::drawDoubleSurface2(const Graphics::Surface *surface, NDrawRect &drawRect) {

	const byte *source = (const byte*)surface->getPixels();
	byte *dest = (byte*)_backScreen->getBasePtr(drawRect.x, drawRect.y);

	for (int16 yc = 0; yc < surface->h; yc++) {
		byte *row = dest;
		for (int16 xc = 0; xc < surface->w; xc++) {
			*row++ = *source;
			*row++ = *source++;
		}
		memcpy(dest + _backScreen->pitch, dest, surface->w * 2);
		dest += _backScreen->pitch;
		dest += _backScreen->pitch;
	}

	_fullRefresh = true; // See Screen::update

}

void Screen::drawUnk(const Graphics::Surface *surface, NDrawRect &drawRect, NDrawRect &sysRect, NRect &clipRect, bool transparent, byte version) {

	int16 x, y;
	bool xflag, yflag;
	NDrawRect newDrawRect;

	x = sysRect.x;
	if (sysRect.width <= x || -sysRect.width >= x)
		x = x % sysRect.width;
	if (x < 0)
		x += sysRect.width;

	y = sysRect.y;
	if (y >= sysRect.height || -sysRect.height >= y)
		y = y % sysRect.height;
	if (y < 0)
		y += sysRect.height;

	xflag = x <= 0;
	yflag = y <= 0;

	newDrawRect.x = x;
	newDrawRect.width = sysRect.width - x;
	if (drawRect.width < newDrawRect.width) {
		xflag = true;
		newDrawRect.width = drawRect.width;
	}

	newDrawRect.y = y;
	newDrawRect.height = sysRect.height - y;
	if (drawRect.height < newDrawRect.height) {
		yflag = true;
		newDrawRect.height = drawRect.height;
	}

	drawSurface3(surface, drawRect.x, drawRect.y, newDrawRect, clipRect, transparent, version);

	if (!xflag) {
		newDrawRect.x = 0;
		newDrawRect.y = y;
		newDrawRect.width = x + drawRect.width - sysRect.width;
		newDrawRect.height = sysRect.height - y;
		if (drawRect.height < newDrawRect.height)
			newDrawRect.height = drawRect.height;
		drawSurface3(surface, sysRect.width + drawRect.x - x, drawRect.y, newDrawRect, clipRect, transparent, version);
	}

	if (!yflag) {
		newDrawRect.x = x;
		newDrawRect.y = 0;
		newDrawRect.width = sysRect.width - x;
		newDrawRect.height = y + drawRect.height - sysRect.height;
		if (drawRect.width < newDrawRect.width)
			newDrawRect.width = drawRect.width;
		drawSurface3(surface, drawRect.x, sysRect.height + drawRect.y - y, newDrawRect, clipRect, transparent, version);
	}

	if (!xflag && !yflag) {
		newDrawRect.x = 0;
		newDrawRect.y = 0;
		newDrawRect.width = x + drawRect.width - sysRect.width;
		newDrawRect.height = y + drawRect.height - sysRect.height;
		drawSurface3(surface, sysRect.width + drawRect.x - x, sysRect.height + drawRect.y - y, newDrawRect, clipRect, transparent, version);
	}

}

void Screen::drawSurfaceClipRects(const Graphics::Surface *surface, NDrawRect &drawRect, NRect *clipRects, uint clipRectsCount, bool transparent, byte version) {
	NDrawRect clipDrawRect(0, 0, drawRect.width, drawRect.height);
	for (uint i = 0; i < clipRectsCount; i++)
		drawSurface3(surface, drawRect.x, drawRect.y, clipDrawRect, clipRects[i], transparent, version);
}

void Screen::queueBlit(const Graphics::Surface *surface, int16 destX, int16 destY, NRect &ddRect, bool transparent, byte version,
	const Graphics::Surface *shadowSurface) {

	const int width = ddRect.x2 - ddRect.x1;
	const int height = ddRect.y2 - ddRect.y1;

	if (width <= 0 || height <= 0)
		return;

	RenderItem renderItem;
	renderItem._surface = surface;
	renderItem._shadowSurface = shadowSurface;
	renderItem._destX = destX;
	renderItem._destY = destY;
	renderItem._srcX = ddRect.x1;
	renderItem._srcY = ddRect.y1;
	renderItem._width = width;
	renderItem._height = height;
	renderItem._transparent = transparent;
	renderItem._version = version;
	_renderQueue->push_back(renderItem);

}

void Screen::blitRenderItem(const RenderItem &renderItem, const Common::Rect &clipRect) {

	const Graphics::Surface *surface = renderItem._surface;
	const Graphics::Surface *shadowSurface = renderItem._shadowSurface;
	const int16 x0 = MAX<int16>(clipRect.left, renderItem._destX);
	const int16 y0 = MAX<int16>(clipRect.top, renderItem._destY);
	const int16 x1 = MIN<int16>(clipRect.right, renderItem._destX + renderItem._width);
	const int16 y1 = MIN<int16>(clipRect.bottom, renderItem._destY + renderItem._height);
	const int16 width = x1 - x0;
	int16 height = y1 - y0;

	if (width < 0 || height < 0)
		return;

	const byte *source = (const byte*)surface->getBasePtr(renderItem._srcX + x0 - renderItem._destX, renderItem._srcY + y0 - renderItem._destY);
	byte *dest = (byte*)_backScreen->getBasePtr(x0, y0);

	if (shadowSurface) {
		const byte *shadowSource = (const byte*)shadowSurface->getBasePtr(x0, y0);
		while (height--) {
			for (int xc = 0; xc < width; xc++)
				if (source[xc] != 0)
					dest[xc] = shadowSource[xc];
			source += surface->pitch;
			shadowSource += shadowSurface->pitch;
			dest += _backScreen->pitch;
		}
	} else if (!renderItem._transparent) {
		while (height--) {
			memcpy(dest, source, width);
			source += surface->pitch;
			dest += _backScreen->pitch;
		}
	} else {
		while (height--) {
			for (int xc = 0; xc < width; xc++)
				if (source[xc] != 0)
					dest[xc] = source[xc];
			source += surface->pitch;
			dest += _backScreen->pitch;
		}
	}

}

} // End of namespace Neverhood
