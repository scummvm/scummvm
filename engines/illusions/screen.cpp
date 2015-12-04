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
 */

#include "illusions/illusions.h"
#include "illusions/screen.h"
#include "illusions/resources/fontresource.h"
#include "engines/util.h"
#include "graphics/palette.h"

namespace Illusions {

// SpriteDecompressQueue

SpriteDecompressQueue::SpriteDecompressQueue(Screen *screen)
	: _screen(screen) {
}

SpriteDecompressQueue::~SpriteDecompressQueue() {
}

void SpriteDecompressQueue::insert(byte *drawFlags, uint32 flags, uint32 field8, WidthHeight &dimensions,
	byte *compressedPixels, Graphics::Surface *surface) {
	SpriteDecompressQueueItem *item = new SpriteDecompressQueueItem();
	item->_drawFlags = drawFlags;
	*item->_drawFlags &= 1;
	item->_flags = flags;
	item->_dimensions = dimensions;
	item->_compressedPixels = compressedPixels;
	item->_field8 = field8;
	item->_surface = surface;
	_queue.push_back(item);
}

void SpriteDecompressQueue::decompressAll() {
	SpriteDecompressQueueListIterator it = _queue.begin();
	while (it != _queue.end()) {
		decompress(*it);
		delete *it;
		it = _queue.erase(it);
	}
}

void SpriteDecompressQueue::decompress(SpriteDecompressQueueItem *item) {
	_screen->decompressSprite(item);
}

// SpriteDrawQueue

SpriteDrawQueue::SpriteDrawQueue(Screen *screen)
	: _screen(screen) {
}

SpriteDrawQueue::~SpriteDrawQueue() {
}

bool SpriteDrawQueue::draw(SpriteDrawQueueItem *item) {

	// Check if the sprite has finished decompressing
	if (item->_kind != 0 && (*item->_drawFlags & 1)) {		
		insert(item, item->_priority);
		return false;
	}

	if (!_screen->isDisplayOn()) {
		if (item->_drawFlags)
			*item->_drawFlags &= ~4;
		return true;			
	}	

	Common::Rect srcRect, dstRect;
	
	// Check if the sprite is on-screen
	if (!calcItemRect(item, srcRect, dstRect))
		return true;

	_screen->drawSurface(dstRect, item->_surface, srcRect, item->_scale, item->_flags);

	if (item->_drawFlags)
		*item->_drawFlags &= ~4;

	return true;
}

void SpriteDrawQueue::drawAll() {
	SpriteDrawQueueListIterator it = _queue.begin();
	while (it != _queue.end()) {
		if (draw(*it)) {
			delete *it;
			it = _queue.erase(it);
		} else
			++it;
	}
}

void SpriteDrawQueue::insertSprite(byte *drawFlags, Graphics::Surface *surface, WidthHeight &dimensions,
	Common::Point &drawPosition, Common::Point &controlPosition, uint32 priority, int16 scale, uint16 flags) {
	SpriteDrawQueueItem *item = new SpriteDrawQueueItem();
	item->_drawFlags = drawFlags;
	*item->_drawFlags &= 4;
	item->_surface = surface;
	item->_dimensions = dimensions;
	item->_controlPosition = controlPosition;
	item->_scale = scale;
	item->_priority = priority;
	item->_drawPosition = drawPosition;
	item->_kind = 1;
	item->_flags = flags;
	insert(item, priority);
}

void SpriteDrawQueue::insertSurface(Graphics::Surface *surface, WidthHeight &dimensions,
	Common::Point &drawPosition, uint32 priority) {
	SpriteDrawQueueItem *item = new SpriteDrawQueueItem();
	item->_surface = surface;
	item->_dimensions = dimensions;
	item->_drawFlags = 0;
	item->_kind = 0;
	item->_drawPosition.x = -drawPosition.x;
	item->_drawPosition.y = -drawPosition.y;
	item->_controlPosition.x = 0;
	item->_controlPosition.y = 0;
	item->_flags = 0;
	item->_scale = 100;
	item->_priority = priority;// << 16;
	insert(item, priority);
}

void SpriteDrawQueue::insertTextSurface(Graphics::Surface *surface, WidthHeight &dimensions,
	Common::Point &drawPosition, uint32 priority) {
	SpriteDrawQueueItem *item = new SpriteDrawQueueItem();
	item->_surface = surface;
	item->_drawPosition = drawPosition;
	item->_dimensions = dimensions;
	item->_drawFlags = 0;
	item->_kind = 0;
	item->_controlPosition.x = 0;
	item->_controlPosition.y = 0;
	item->_flags = 0;
	item->_priority = priority;
	item->_scale = 100;
	insert(item, priority);
}

void SpriteDrawQueue::insert(SpriteDrawQueueItem *item, uint32 priority) {
	SpriteDrawQueueListIterator insertionPos = Common::find_if(_queue.begin(), _queue.end(),
		FindInsertionPosition(priority));
	_queue.insert(insertionPos, item);
}

bool SpriteDrawQueue::calcItemRect(SpriteDrawQueueItem *item, Common::Rect &srcRect, Common::Rect &dstRect) {

	srcRect.left = 0;
	srcRect.top = 0;
	srcRect.right = item->_dimensions._width;
	srcRect.bottom = item->_dimensions._height;
	
	dstRect.left = item->_drawPosition.x - item->_scale * item->_controlPosition.x / 100;
	dstRect.top = item->_drawPosition.y - item->_scale * item->_controlPosition.y / 100;
	dstRect.right = item->_drawPosition.x + item->_scale * (item->_dimensions._width - item->_controlPosition.x) / 100;
	dstRect.bottom = item->_drawPosition.y + item->_scale * (item->_dimensions._height - item->_controlPosition.y) / 100;
	
	if (_screen->_isScreenOffsetActive) {
		dstRect.left += _screen->_screenOffsetPt.x;
		dstRect.right += _screen->_screenOffsetPt.x;
		dstRect.top += _screen->_screenOffsetPt.y;
		dstRect.bottom += _screen->_screenOffsetPt.y;
	}

	// Check if the sprite is on-screen
	if (dstRect.left >= _screen->getScreenWidth() || dstRect.right <= 0 || dstRect.top >= _screen->getScreenHeight() || dstRect.bottom <= 0)
		return false;

	// Clip the sprite rect if neccessary

	if (dstRect.left < 0) {
		srcRect.left += -100 * dstRect.left / item->_scale;
		dstRect.left = 0;
	}

	if (dstRect.top < 0) {
		srcRect.top += -100 * dstRect.top / item->_scale;
		dstRect.top = 0;
	}

	if (dstRect.right > _screen->getScreenWidth()) {
		srcRect.right += 100 * (_screen->getScreenWidth() - dstRect.right) / item->_scale;
		dstRect.right = _screen->getScreenWidth();
	}

	if (dstRect.bottom > _screen->getScreenHeight()) {
		srcRect.bottom += 100 * (_screen->getScreenHeight() - dstRect.bottom) / item->_scale;
		dstRect.bottom = _screen->getScreenHeight();
	}

	return true;
}

// Screen

Screen::Screen(IllusionsEngine *vm, int16 width, int16 height, int bpp)
	: _vm(vm), _colorKey1(0), _colorKey2(0) {
	_displayOn = true;
	_decompressQueue = new SpriteDecompressQueue(this);
	_drawQueue = new SpriteDrawQueue(this);
	if (bpp == 8) {
		initGraphics(width, height, false);
	} else {
		Graphics::PixelFormat pixelFormat16(2, 5, 6, 5, 0, 11, 5, 0, 0);
		initGraphics(width, height, true, &pixelFormat16);
	}

	_backSurface = allocSurface(width, height);

	_needRefreshPalette = false;
	memset(_mainPalette, 0, sizeof(_mainPalette));

	_isFaderActive = false;
	_isScreenOffsetActive = false;

}

Screen::~Screen() {
	delete _drawQueue;
	delete _decompressQueue;
	_backSurface->free();
	delete _backSurface;
}

Graphics::Surface *Screen::allocSurface(int16 width, int16 height) {
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(width, height, _vm->_system->getScreenFormat());
	return surface; 
}

Graphics::Surface *Screen::allocSurface(SurfInfo &surfInfo) {
	return allocSurface(surfInfo._dimensions._width, surfInfo._dimensions._height);
}

bool Screen::isDisplayOn() {
	return _displayOn;
}

void Screen::setDisplayOn(bool isOn) {
	_displayOn = isOn;
	// TODO Clear screen when off
}

void Screen::setScreenOffset(Common::Point offsPt) {
	if (offsPt.x != 0 || offsPt.y != 0) {
		_isScreenOffsetActive = true;
		_screenOffsetPt = offsPt;
	} else {
		_isScreenOffsetActive = false;
	}
}

void Screen::updateSprites() {
	_decompressQueue->decompressAll();
	// NOTE Skipped doShiftBrightness and related as it seems to be unused
	_drawQueue->drawAll();
	if (_isScreenOffsetActive)
		clearScreenOffsetAreas();
	if (!_displayOn) // TODO Check if a video is playing then don't do it
		_backSurface->fillRect(Common::Rect(_backSurface->w, _backSurface->h), 0);
	g_system->copyRectToScreen((byte*)_backSurface->getBasePtr(0, 0), _backSurface->pitch, 0, 0, _backSurface->w, _backSurface->h);
}

void Screen::clearScreenOffsetAreas() {
	int16 x1 = 0, y1 = 0, x2 = 0, y2 = 0;
	if (_screenOffsetPt.x < 0) {
		x1 = _backSurface->w + _screenOffsetPt.x;
		x2 = _backSurface->w;
	} else if (_screenOffsetPt.x > 0) {
		x1 = 0;
		x2 = _screenOffsetPt.x;
	}
	if (_screenOffsetPt.y < 0) {
		y1 = _backSurface->h + _screenOffsetPt.y;
		y2 = _backSurface->h;
	} else if (_screenOffsetPt.y > 0) {
		y1 = 0;
		y2 = _screenOffsetPt.y;
	}
	_backSurface->fillRect(Common::Rect(0, y1, _backSurface->w, y2), 0);
	_backSurface->fillRect(Common::Rect(x1, 0, x2, _backSurface->h), 0);
}

void Screen::decompressSprite(SpriteDecompressQueueItem *item) {
	switch (_backSurface->format.bytesPerPixel) {
	case 1:
		decompressSprite8(item);
		break;
	case 2:
		decompressSprite16(item);
		break;
	default:
		break;
	}
}

void Screen::drawSurface(Common::Rect &dstRect, Graphics::Surface *surface, Common::Rect &srcRect, int16 scale, uint32 flags) {
	switch (_backSurface->format.bytesPerPixel) {
	case 1:
		drawSurface8(dstRect, surface, srcRect, scale, flags);
		break;
	case 2:
		drawSurface16(dstRect, surface, srcRect, scale, flags);
		break;
	default:
		break;
	}
}

void Screen::setPalette(byte *colors, uint start, uint count) {
	if (_backSurface->format.bytesPerPixel == 1) {
		byte *dstPal = &_mainPalette[3 * (start - 1)];
		for (uint i = 0; i < count; ++i) {
			*dstPal++ = *colors++;
			*dstPal++ = *colors++;
			*dstPal++ = *colors++;
			++colors;
		}
		buildColorTransTbl();
		_needRefreshPalette = true;
	}
}

void Screen::setPaletteEntry(int16 index, byte r, byte g, byte b) {
	byte colors[4];
	colors[0] = r;
	colors[1] = g;
	colors[2] = b;
	setPalette(colors, index, 1);
}

void Screen::getPalette(byte *colors) {
	byte *srcPal = _mainPalette;
	for (uint i = 0; i < 256; ++i) {
		*colors++ = *srcPal++;
		*colors++ = *srcPal++;
		*colors++ = *srcPal++;
		++colors;
	}
}

void Screen::shiftPalette(int16 fromIndex, int16 toIndex) {
	byte r, g, b;
	if (toIndex > fromIndex) {
		r = _mainPalette[3 * toIndex + 0];
		g = _mainPalette[3 * toIndex + 1];
		b = _mainPalette[3 * toIndex + 2];
		for (int16 i = toIndex; i > fromIndex; --i) {
			byte *dst = &_mainPalette[3 * i];
			byte *src = &_mainPalette[3 * (i - 1)];
			dst[0] = src[0];
			dst[1] = src[1];
			dst[2] = src[2];
		}
		_mainPalette[3 * fromIndex + 0] = r;
		_mainPalette[3 * fromIndex + 1] = g;
		_mainPalette[3 * fromIndex + 2] = b;
	} else {
		r = _mainPalette[3 * toIndex + 0];
		g = _mainPalette[3 * toIndex + 1];
		b = _mainPalette[3 * toIndex + 2];
		for (int16 i = toIndex + 1; i < fromIndex; +i) {
			byte *dst = &_mainPalette[3 * i];
			byte *src = &_mainPalette[3 * (i + 1)];
			dst[0] = src[0];
			dst[1] = src[1];
			dst[2] = src[2];
		}
		_mainPalette[3 * fromIndex + 0] = r;
		_mainPalette[3 * fromIndex + 1] = g;
		_mainPalette[3 * fromIndex + 2] = b;
	}
	// TODO Refresh colorTransTbl
	_needRefreshPalette = true;
}

void Screen::updatePalette() {
	if (_needRefreshPalette) {
		if (_isFaderActive) {
			updateFaderPalette();
			setSystemPalette(_faderPalette);
		} else {
			setSystemPalette(_mainPalette);
		}
		_needRefreshPalette = false;
	}
}

void Screen::updateFaderPalette() {
	if (_newFaderValue >= 255) {
		_newFaderValue -= 256;
		for (int i = _firstFaderIndex; i <= _lastFaderIndex; ++i) {
			byte r = _mainPalette[i * 3 + 0];
			byte g = _mainPalette[i * 3 + 1];
			byte b = _mainPalette[i * 3 + 2];
			_faderPalette[i * 3 + 0] = r - (((_newFaderValue * (255 - r)) >> 8) & 0xFF);
			_faderPalette[i * 3 + 1] = g - (((_newFaderValue * (255 - g)) >> 8) & 0xFF);
			_faderPalette[i * 3 + 2] = b - (((_newFaderValue * (255 - b)) >> 8) & 0xFF);
		}
	} else {
		for (int i = _firstFaderIndex; i <= _lastFaderIndex; ++i) {
			byte r = _mainPalette[i * 3 + 0];
			byte g = _mainPalette[i * 3 + 1];
			byte b = _mainPalette[i * 3 + 2];
			_faderPalette[i * 3 + 0] = _newFaderValue * r / 255;
			_faderPalette[i * 3 + 1] = _newFaderValue * g / 255;
			_faderPalette[i * 3 + 2] = _newFaderValue * b / 255;
		}
	}
}

void Screen::setFader(int newValue, int firstIndex, int lastIndex) {
	if (newValue == 255) {
		_isFaderActive = false;
		_needRefreshPalette = true;
	} else {
		_isFaderActive = true;
		_needRefreshPalette = true;
		_newFaderValue = newValue;
		_firstFaderIndex = firstIndex - 1;
		_lastFaderIndex = lastIndex;
	}
}

void Screen::buildColorTransTbl() {
	const int cr = _mainPalette[3 * 1 + 0];
	const int cg = _mainPalette[3 * 1 + 1];
	const int cb = _mainPalette[3 * 1 + 2];
	for (int index1 = 0; index1 < 256; ++index1) {
		const int dr = (cr + _mainPalette[3 * index1 + 0]) / 2;
		const int dg = (cg + _mainPalette[3 * index1 + 1]) / 2;
		const int db = (cb + _mainPalette[3 * index1 + 2]) / 2;
		int minDistance = 766;
		int minIndex2 = 2;
		for (int index2 = 2; index2 < 256; ++index2) {
			int distance =
				ABS(dr - _mainPalette[3 * index2 + 0]) +
				ABS(dg - _mainPalette[3 * index2 + 1]) +
				ABS(db - _mainPalette[3 * index2 + 2]);
			if (distance < minDistance) {
				minDistance = distance;
				minIndex2 = index2;
			}
		}
		_colorTransTbl[index1] = minIndex2;
	}
}

void Screen::drawText(FontResource *font, Graphics::Surface *surface, int16 x, int16 y, uint16 *text, uint count) {
	switch (_backSurface->format.bytesPerPixel) {
	case 1:
		drawText8(font, surface, x, y, text, count);
		break;
	case 2:
		drawText16(font, surface, x, y, text, count);
		break;
	default:
		break;
	}
}

void Screen::drawText8(FontResource *font, Graphics::Surface *surface, int16 x, int16 y, uint16 *text, uint count) {
	for (uint i = 0; i < count; ++i)
		x += font->_widthC + drawChar8(font, surface, x, y, *text++);
}

int16 Screen::drawChar8(FontResource *font, Graphics::Surface *surface, int16 x, int16 y, uint16 c) {
	const CharInfo *charInfo = font->getCharInfo(c);
	const int16 charWidth = charInfo->_width;
	byte *dst = (byte*)surface->getBasePtr(x, y);
	byte *pixels = charInfo->_pixels;
	for (int16 yc = 0; yc < font->_charHeight; ++yc) {
		for (int16 xc = 0; xc < charWidth; ++xc)
			if (pixels[xc])
				dst[xc] = pixels[xc];
		dst += surface->pitch;
		pixels += charWidth;
	}
	return charWidth;
}

void Screen::drawText16(FontResource *font, Graphics::Surface *surface, int16 x, int16 y, uint16 *text, uint count) {
	for (uint i = 0; i < count; ++i)
		x += font->_widthC + drawChar16(font, surface, x, y, *text++);
}

int16 Screen::drawChar16(FontResource *font, Graphics::Surface *surface, int16 x, int16 y, uint16 c) {
	const CharInfo *charInfo = font->getCharInfo(c);
	const int16 charWidth = charInfo->_width;
	byte *pixels = charInfo->_pixels;
	for (int16 yc = 0; yc < font->_charHeight; ++yc) {
		byte *dst = (byte*)surface->getBasePtr(x, y + yc);
		for (int16 xc = 0; xc < charWidth; ++xc) {
			if (pixels[xc])
				WRITE_LE_UINT16(dst, convertFontColor(pixels[xc]));
			dst += 2;
		}
		pixels += charWidth;
	}
	return charWidth;
}

void Screen::setSystemPalette(byte *palette) {
	g_system->getPaletteManager()->setPalette(palette, 0, 256);
}

void Screen::decompressSprite8(SpriteDecompressQueueItem *item) {
	byte *src = item->_compressedPixels;
	Graphics::Surface *dstSurface = item->_surface;
	int dstSize = item->_dimensions._width * item->_dimensions._height;
	int processedSize = 0;
	int xincr, x, xstart;
	int yincr, y;
	
	*item->_drawFlags &= ~1;

	// Safeguard
	if (item->_dimensions._width > item->_surface->w ||
		item->_dimensions._height > item->_surface->h) {
		debug("Incorrect frame dimensions (%d, %d <> %d, %d)",
			item->_dimensions._width, item->_dimensions._height,
			item->_surface->w, item->_surface->h);
		return;
	}
	
	if (item->_flags & 1) {
		x = xstart = item->_dimensions._width - 1;
		xincr = -1;
	} else {
		x = xstart = 0;
		xincr = 1;
	}

	if (item->_flags & 2) {
		y = item->_dimensions._height - 1;
		yincr = -1;
	} else {
		y = 0;
		yincr = 1;
	}
	
	byte *dst = (byte*)dstSurface->getBasePtr(x, y);
	
	while (processedSize < dstSize) {
		byte op = *src++;
		if (op & 0x80) {
			int runCount = (op & 0x7F) + 1;
			processedSize += runCount;
			byte runColor = *src++;
			while (runCount--) {
				*dst = runColor;
				x += xincr;
				if (x >= item->_dimensions._width || x < 0) {
					x = xstart;
					y += yincr;
					dst = (byte*)dstSurface->getBasePtr(x, y);
				} else {
					dst += xincr;
				}
			}
		} else {
			int copyCount = op + 1;
			processedSize += copyCount;
			while (copyCount--) {
				byte color = *src++;
				*dst = color;
				x += xincr;
				if (x >= item->_dimensions._width || x < 0) {
					x = xstart;
					y += yincr;
					dst = (byte*)dstSurface->getBasePtr(x, y);
				} else {
					dst += xincr;
				}
			}
		}
	}

}

void Screen::drawSurface8(Common::Rect &dstRect, Graphics::Surface *surface, Common::Rect &srcRect, int16 scale, uint32 flags) {
	if (scale == 100) {
		drawSurface81(dstRect.left, dstRect.top, surface, srcRect);
	} else {
		drawSurface82(dstRect, surface, srcRect);
	}
}

void Screen::drawSurface81(int16 destX, int16 destY, Graphics::Surface *surface, Common::Rect &srcRect) {
	// Unscaled
	const int16 w = srcRect.width();
	const int16 h = srcRect.height();
	for (int16 yc = 0; yc < h; ++yc) {
		byte *src = (byte*)surface->getBasePtr(srcRect.left, srcRect.top + yc);
		byte *dst = (byte*)_backSurface->getBasePtr(destX, destY + yc);
		for (int16 xc = 0; xc < w; ++xc) {
			const byte pixel = *src++;
			if (pixel != 0) {
				if (pixel == 1)
					*dst = _colorTransTbl[*dst];
				else
					*dst = pixel;
			}
			++dst;				
		}
	}
}

void Screen::drawSurface82(Common::Rect &dstRect, Graphics::Surface *surface, Common::Rect &srcRect) {
	// Scaled
	const int dstWidth = dstRect.width(), dstHeight = dstRect.height();
	const int srcWidth = srcRect.width(), srcHeight = srcRect.height();
	const int errYStart = srcHeight / dstHeight;
	const int errYIncr = srcHeight % dstHeight;
//	const int midY = dstHeight / 2;
	const int errXStart = srcWidth / dstWidth;
	const int errXIncr = srcWidth % dstWidth;
//	const int midX = dstWidth / 2;
	int h = dstHeight, errY = 0, skipY, srcY = srcRect.top;
	byte *dst = (byte*)_backSurface->getBasePtr(dstRect.left, dstRect.top);
	skipY = (dstHeight < srcHeight) ? 0 : dstHeight / (2*srcHeight) + 1;
	h -= skipY;
	while (h-- > 0) {
		int w = dstWidth, errX = 0, skipX;
		skipX = (dstWidth < srcWidth) ? 0 : dstWidth / (2*srcWidth) + 1;
		w -= skipX;
		byte *src = (byte*)surface->getBasePtr(srcRect.left, srcY);
		byte *dstRow = dst; 
		while (w-- > 0) {
			const byte pixel = *src;
			if (pixel != 0) {
				if (pixel == 1)
					*dstRow = _colorTransTbl[*dstRow];
				else
					*dstRow = pixel;
			}
			++dstRow;
			src += errXStart;
			errX += errXIncr;
			if (errX >= dstWidth) {
				errX -= dstWidth;
				++src;
			}
		}
		while (skipX-- > 0) {
			const byte pixel = *src;
			if (pixel != 0) {
				if (pixel == 1)
					*dstRow = _colorTransTbl[*dstRow];
				else
					*dstRow = pixel;
			}
			++src;
			++dstRow;
		}
		dst += _backSurface->pitch;
		srcY += errYStart;
		errY += errYIncr;
		if (errY >= dstHeight) {
			errY -= dstHeight;
			++srcY;
		}
	}
}

void Screen::decompressSprite16(SpriteDecompressQueueItem *item) {
	byte *src = item->_compressedPixels;
	Graphics::Surface *dstSurface = item->_surface;
	int dstSize = item->_dimensions._width * item->_dimensions._height;
	int processedSize = 0;
	int xincr, x, xstart;
	int yincr, y;
	
	*item->_drawFlags &= ~1;

	// Safeguard
	if (item->_dimensions._width > item->_surface->w ||
		item->_dimensions._height > item->_surface->h) {
		debug("Incorrect frame dimensions (%d, %d <> %d, %d)",
			item->_dimensions._width, item->_dimensions._height,
			item->_surface->w, item->_surface->h);
		return;
	}
	
	if (item->_flags & 1) {
		x = xstart = item->_dimensions._width - 1;
		xincr = -1;
	} else {
		x = xstart = 0;
		xincr = 1;
	}

	if (item->_flags & 2) {
		y = item->_dimensions._height - 1;
		yincr = -1;
	} else {
		y = 0;
		yincr = 1;
	}
	
	byte *dst = (byte*)dstSurface->getBasePtr(x, y);
	
	while (processedSize < dstSize) {
		int16 op = READ_LE_UINT16(src);
		src += 2;
		if (op & 0x8000) {
			int runCount = (op & 0x7FFF) + 1;
			processedSize += runCount;
			uint16 runColor = READ_LE_UINT16(src);
			src += 2;
			while (runCount--) {
				WRITE_LE_UINT16(dst, runColor);
				x += xincr;
				if (x >= item->_dimensions._width || x < 0) {
					x = xstart;
					y += yincr;
					dst = (byte*)dstSurface->getBasePtr(x, y);
				} else {
					dst += 2 * xincr;
				}
			}
		} else {
			int copyCount = op + 1;
			processedSize += copyCount;
			while (copyCount--) {
				uint16 color = READ_LE_UINT16(src);
				src += 2;
				WRITE_LE_UINT16(dst, color);
				x += xincr;
				if (x >= item->_dimensions._width || x < 0) {
					x = xstart;
					y += yincr;
					dst = (byte*)dstSurface->getBasePtr(x, y);
				} else {
					dst += 2 * xincr;
				}
			}
		}
	}

}

void Screen::drawSurface16(Common::Rect &dstRect, Graphics::Surface *surface, Common::Rect &srcRect, int16 scale, uint32 flags) {
	if (scale == 100) {
		if (flags & 1)
			drawSurface10(dstRect.left, dstRect.top, surface, srcRect, _colorKey2);
		else
			drawSurface11(dstRect.left, dstRect.top, surface, srcRect);
	} else {
		if (flags & 1)
			drawSurface20(dstRect, surface, srcRect, _colorKey2);
		else
			drawSurface21(dstRect, surface, srcRect);
	}
}

void Screen::drawSurface10(int16 destX, int16 destY, Graphics::Surface *surface, Common::Rect &srcRect, uint16 colorKey) {
	// Unscaled
	// TODO
	//debug("Screen::drawSurface10");
}

void Screen::drawSurface11(int16 destX, int16 destY, Graphics::Surface *surface, Common::Rect &srcRect) {
	// Unscaled
	//debug("Screen::drawSurface11() destX: %d; destY: %d; srcRect: (%d, %d, %d, %d)", destX, destY, srcRect.left, srcRect.top, srcRect.right, srcRect.bottom);
	const int16 w = srcRect.width();
	const int16 h = srcRect.height();
	for (int16 yc = 0; yc < h; ++yc) {
		byte *src = (byte*)surface->getBasePtr(srcRect.left, srcRect.top + yc);
		byte *dst = (byte*)_backSurface->getBasePtr(destX, destY + yc);
		for (int16 xc = 0; xc < w; ++xc) {
			uint16 pixel = READ_LE_UINT16(src);
			if (pixel != _colorKey1)
				WRITE_LE_UINT16(dst, pixel);
			src += 2;
			dst += 2;
		}
	}
}

void Screen::drawSurface20(Common::Rect &dstRect, Graphics::Surface *surface, Common::Rect &srcRect, uint16 colorKey) {
	// Scaled
	// TODO
	//debug("Screen::drawSurface20");
}

//#define TEST_SMOOTHING
#ifdef TEST_SMOOTHING
static uint16 average(const uint16 a, const uint16 b) {
	byte r1, g1, b1, r2, g2, b2;
	g_system->getScreenFormat().colorToRGB(a, r1, g1, b1);
	g_system->getScreenFormat().colorToRGB(b, r2, g2, b2);
//	return g_system->getScreenFormat().RGBToColor((r1 + r1 + r2) / 3, (g1 + g1 + g2) / 3, (b1 + b1 + b2) / 3);
	return g_system->getScreenFormat().RGBToColor((r1 + r2) / 2, (g1 + g2) / 2, (b1 + b2) / 2);
}
#endif

void Screen::drawSurface21(Common::Rect &dstRect, Graphics::Surface *surface, Common::Rect &srcRect) {
	// Scaled
	const int dstWidth = dstRect.width(), dstHeight = dstRect.height();
	const int srcWidth = srcRect.width(), srcHeight = srcRect.height();
	const int errYStart = srcHeight / dstHeight;
	const int errYIncr = srcHeight % dstHeight;
//	const int midY = dstHeight / 2;
	const int errXStart = srcWidth / dstWidth;
	const int errXIncr = srcWidth % dstWidth;
//	const int midX = dstWidth / 2;
	int h = dstHeight, errY = 0, skipY, srcY = srcRect.top;
	byte *dst = (byte*)_backSurface->getBasePtr(dstRect.left, dstRect.top);
	skipY = (dstHeight < srcHeight) ? 0 : dstHeight / (2*srcHeight) + 1;
	h -= skipY;
	while (h-- > 0) {
		int w = dstWidth, errX = 0, skipX;
		skipX = (dstWidth < srcWidth) ? 0 : dstWidth / (2*srcWidth) + 1;
		w -= skipX;
		byte *src = (byte*)surface->getBasePtr(srcRect.left, srcY);
		byte *dstRow = dst; 
		while (w-- > 0) {
			uint16 pixel = READ_LE_UINT16(src);
			if (pixel != _colorKey1) {
#ifdef TEST_SMOOTHING
				if (errX >= midX) {
					uint16 npixel = READ_LE_UINT16(src + 2);
					if (npixel == _colorKey1)
						npixel = READ_LE_UINT16(dstRow);
					pixel = average(pixel, npixel);
				}
#endif
				WRITE_LE_UINT16(dstRow, pixel);
			}
			dstRow += 2;
			src += 2 * errXStart;
			errX += errXIncr;
			if (errX >= dstWidth) {
				errX -= dstWidth;
				src += 2;
			}
		}
		while (skipX-- > 0) {
			uint16 pixel = READ_LE_UINT16(src);
			if (pixel != _colorKey1)
				WRITE_LE_UINT16(dstRow, pixel);
			src += 2;
			dstRow += 2;
		}
		dst += _backSurface->pitch;
		srcY += errYStart;
		errY += errYIncr;
		if (errY >= dstHeight) {
			errY -= dstHeight;
			++srcY;
		}
	}

}

uint16 Screen::convertFontColor(byte color) {
	if (color) {
		byte r, g, b;
		if (color == 204) {
			r = 50;
			g = 50;
			b = 180;
		} else {
			r = 256 - color;
			g = 256 - color;
			b = 256 - color;
		}
		return g_system->getScreenFormat().RGBToColor(r, g, b);
	}
	return _colorKey1;
}

} // End of namespace Illusions
