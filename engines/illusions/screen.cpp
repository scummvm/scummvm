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

namespace Illusions {

// SpriteDecompressQueue

SpriteDecompressQueue::SpriteDecompressQueue() {
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
	byte *src = item->_compressedPixels;
	Graphics::Surface *dstSurface = item->_surface;
	int dstSize = item->_dimensions._width * item->_dimensions._height;
	int processedSize = 0;
	int xincr, x, xstart;
	int yincr, y;
	
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

	*item->_drawFlags &= ~1;
 
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
	
	/* CHECKME This seems to be unused basically and only called from debug code
		Left here just in case...
	if (gfx_seemsAlways0) {
		dstRect.left += screenOffsetPt.x;
		dstRect.right = screenOffsetPt.x + dstRect.right;
		dstRect.top = screenOffsetPt.y + dstRect.top;
		dstRect.bottom = screenOffsetPt.y + dstRect.bottom;
	}
	*/

	// Check if the sprite is on-screen
	if (dstRect.left >= 640 || dstRect.right <= 0 || dstRect.top >= 480 || dstRect.bottom <= 0)
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

	if (dstRect.right > 640) {
		srcRect.right += 100 * (640 - dstRect.right) / item->_scale;
		dstRect.right = 640;
	}

	if (dstRect.bottom > 480) {
		srcRect.bottom += 100 * (480 - dstRect.bottom) / item->_scale;
		dstRect.bottom = 480;
	}

	return true;
}

// Screen

Screen::Screen(IllusionsEngine *vm)
	: _vm(vm), _colorKey2(0) {
	_displayOn = true;
	_backSurface = allocSurface(640, 480);
	_decompressQueue = new SpriteDecompressQueue();
	_drawQueue = new SpriteDrawQueue(this);
	_colorKey1 = 0xF800 | 0x1F;
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

uint16 Screen::getColorKey2() {
	return _colorKey2;
}

void Screen::updateSprites() {
	_decompressQueue->decompressAll();
	// NOTE Skipped doShiftBrightness and related as it seems to be unused
	_drawQueue->drawAll();
	if (!_displayOn) // TODO Check if a video is playing then don't do it
		_backSurface->fillRect(Common::Rect(_backSurface->w, _backSurface->h), 0);
	g_system->copyRectToScreen((byte*)_backSurface->getBasePtr(0, 0), _backSurface->pitch, 0, 0, _backSurface->w, _backSurface->h);
}

void Screen::drawSurface(Common::Rect &dstRect, Graphics::Surface *surface, Common::Rect &srcRect, int16 scale, uint32 flags) {
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

static uint16 average(const uint16 a, const uint16 b) {
	byte r1, g1, b1, r2, g2, b2;
	g_system->getScreenFormat().colorToRGB(a, r1, g1, b1);
	g_system->getScreenFormat().colorToRGB(b, r2, g2, b2);
	return g_system->getScreenFormat().RGBToColor((r1 + r1 + r2) / 3, (g1 + g1 + g2) / 3, (b1 + b1 + b2) / 3);
}

void Screen::drawSurface21(Common::Rect &dstRect, Graphics::Surface *surface, Common::Rect &srcRect) {
	// Scaled
	const int dstWidth = dstRect.width(), dstHeight = dstRect.height();
	const int srcWidth = srcRect.width(), srcHeight = srcRect.height();
	const int errYStart = srcHeight / dstHeight;
	const int errYIncr = srcHeight % dstHeight;
	const int midY = dstHeight / 2;
	const int errXStart = srcWidth / dstWidth;
	const int errXIncr = srcWidth % dstWidth;
	const int midX = dstWidth / 2;
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
				if (errX >= midX) {
					uint16 npixel = READ_LE_UINT16(src + 2);
					if (npixel == _colorKey1)
						npixel = READ_LE_UINT16(dstRow);
					pixel = average(pixel, npixel);
				}
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

} // End of namespace Illusions
