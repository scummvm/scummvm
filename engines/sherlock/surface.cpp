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

#include "sherlock/surface.h"
#include "sherlock/sherlock.h"
#include "sherlock/resources.h"
#include "common/system.h"
#include "graphics/palette.h"

namespace Sherlock {

Surface::Surface(uint16 width, uint16 height) : Fonts(), _freePixels(true) {
	create(width, height);
}

Surface::Surface() : Fonts(), _freePixels(false) {
}

Surface::~Surface() {
	if (_freePixels)
		_surface.free();
}

void Surface::create(uint16 width, uint16 height) {
	if (_freePixels)
		_surface.free();

	if (_vm->getPlatform() == Common::kPlatform3DO) {
		_surface.create(width, height, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
	} else {
		_surface.create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	}
	_freePixels = true;
}

Graphics::PixelFormat Surface::getPixelFormat() {
	return _surface.format;
}

void Surface::blitFrom(const Surface &src) {
	blitFrom(src, Common::Point(0, 0));
}

void Surface::blitFrom(const ImageFrame &src) {
	blitFrom(src._frame, Common::Point(0, 0));
}

void Surface::blitFrom(const Graphics::Surface &src) {
	blitFrom(src, Common::Point(0, 0));
}

void Surface::blitFrom(const Surface &src, const Common::Point &pt) {
	blitFrom(src, pt, Common::Rect(0, 0, src._surface.w, src._surface.h));
}

void Surface::blitFrom(const ImageFrame &src, const Common::Point &pt) {
	blitFrom(src._frame, pt, Common::Rect(0, 0, src._frame.w, src._frame.h));
}

void Surface::blitFrom(const Graphics::Surface &src, const Common::Point &pt) {
	blitFrom(src, pt, Common::Rect(0, 0, src.w, src.h));
}

void Surface::blitFrom(const Graphics::Surface &src, const Common::Point &pt, const Common::Rect &srcBounds) {
	Common::Rect srcRect = srcBounds;
	Common::Rect destRect(pt.x, pt.y, pt.x + srcRect.width(), pt.y + srcRect.height());

	if (srcRect.isValidRect() && clip(srcRect, destRect)) {
		// Surface is at least partially or completely on-screen
		addDirtyRect(destRect);
		_surface.copyRectToSurface(src, destRect.left, destRect.top, srcRect);
	}
}

void Surface::blitFrom(const ImageFrame &src, const Common::Point &pt, const Common::Rect &srcBounds) {
	blitFrom(src._frame, pt, srcBounds);
}

void Surface::blitFrom(const Surface &src, const Common::Point &pt, const Common::Rect &srcBounds) {
	blitFrom(src._surface, pt, srcBounds);
}

void Surface::transBlitFrom(const ImageFrame &src, const Common::Point &pt,
		bool flipped, int overrideColor, int scaleVal) {
	transBlitFrom(src._frame, pt + src._offset, flipped, overrideColor, scaleVal);
}

void Surface::transBlitFrom(const Surface &src, const Common::Point &pt,
		bool flipped, int overrideColor, int scaleVal) {
	const Graphics::Surface &s = src._surface;
	transBlitFrom(s, pt, flipped, overrideColor, scaleVal);
}

void Surface::transBlitFrom(const Graphics::Surface &src, const Common::Point &pt,
		bool flipped, int overrideColor, int scaleVal) {
	if (scaleVal == 256) {
		transBlitFromUnscaled(src, pt, flipped, overrideColor);
		return;
	}

	int scaleX = SCALE_THRESHOLD * SCALE_THRESHOLD / scaleVal;
	int scaleY = scaleX;
	int scaleXCtr = 0, scaleYCtr = 0;

	for (int yCtr = 0, destY = pt.y; yCtr < src.h && destY < this->h(); ++yCtr) {
		// Handle skipping lines if Y scaling
		scaleYCtr += scaleY;
		
		while (scaleYCtr >= SCALE_THRESHOLD && destY < this->h()) {
			scaleYCtr -= SCALE_THRESHOLD;

			if (destY >= 0) {
				// Handle drawing the line
				const byte *pSrc = (const byte *)src.getBasePtr(flipped ? src.w - 1 : 0, yCtr);
				byte *pDest = (byte *)getBasePtr(pt.x, destY);
				scaleXCtr = 0;

				for (int xCtr = 0, destX = pt.x; xCtr < src.w && destX < this->w(); ++xCtr) {
					// Handle horizontal scaling
					scaleXCtr += scaleX;

					while (scaleXCtr >= SCALE_THRESHOLD && destX < this->w()) {
						scaleXCtr -= SCALE_THRESHOLD;

						// Only handle on-screen pixels
						if (destX >= 0 && *pSrc != TRANSPARENCY)
							*pDest = *pSrc;

						++pDest;
						++destX;
					}

					pSrc = pSrc + (flipped ? -1 : 1);
				}
			}

			++destY;
		}
	}
}

void Surface::transBlitFromUnscaled(const Graphics::Surface &src, const Common::Point &pt,
		bool flipped, int overrideColor) {
	Common::Rect drawRect(0, 0, src.w, src.h);
	Common::Rect destRect(pt.x, pt.y, pt.x + src.w, pt.y + src.h);

	// Clip the display area to on-screen
	if (!clip(drawRect, destRect))
		// It's completely off-screen
		return;

	if (flipped)
		drawRect = Common::Rect(src.w - drawRect.right, src.h - drawRect.bottom,
			src.w - drawRect.left, src.h - drawRect.top);

	Common::Point destPt(destRect.left, destRect.top);
	addDirtyRect(Common::Rect(destPt.x, destPt.y, destPt.x + drawRect.width(),
		destPt.y + drawRect.height()));

	switch (src.format.bytesPerPixel) {
	case 1:
		// 8-bit palettized: Draw loop
		assert(_surface.format.bytesPerPixel == 1); // Security check
		for (int yp = 0; yp < drawRect.height(); ++yp) {
			const byte *srcP = (const byte *)src.getBasePtr(
				flipped ? drawRect.right - 1 : drawRect.left, drawRect.top + yp);
			byte *destP = (byte *)getBasePtr(destPt.x, destPt.y + yp);

			for (int xp = 0; xp < drawRect.width(); ++xp, ++destP) {
				if (*srcP != TRANSPARENCY)
					*destP = overrideColor ? overrideColor : *srcP;

				srcP = flipped ? srcP - 1 : srcP + 1;
			}
		}
		break;
	case 2:
		// 3DO 15-bit RGB565: Draw loop
		assert(_surface.format.bytesPerPixel == 2); // Security check
		for (int yp = 0; yp < drawRect.height(); ++yp) {
			const uint16 *srcP = (const uint16 *)src.getBasePtr(
				flipped ? drawRect.right - 1 : drawRect.left, drawRect.top + yp);
			uint16 *destP = (uint16 *)getBasePtr(destPt.x, destPt.y + yp);

			for (int xp = 0; xp < drawRect.width(); ++xp, ++destP) {
				if (*srcP) // RGB 0, 0, 0 -> transparent on 3DO
					*destP = *srcP; // overrideColor ? overrideColor : *srcP;

				srcP = flipped ? srcP - 1 : srcP + 1;
			}
		}
		break;
	default:
		error("Surface: unsupported bytesperpixel");
		break;
	}
}

void Surface::fillRect(int x1, int y1, int x2, int y2, byte color) {
	fillRect(Common::Rect(x1, y1, x2, y2), color);
}

void Surface::fillRect(const Common::Rect &r, byte color) {
	_surface.fillRect(r, color);
	addDirtyRect(r);
}

void Surface::fill(uint16 color) {
	_surface.fillRect(Common::Rect(_surface.w, _surface.h), color);
}

bool Surface::clip(Common::Rect &srcBounds, Common::Rect &destBounds) {
	if (destBounds.left >= _surface.w || destBounds.top >= _surface.h ||
			destBounds.right <= 0 || destBounds.bottom <= 0)
		return false;

	// Clip the bounds if necessary to fit on-screen
	if (destBounds.right > _surface.w) {
		srcBounds.right -= destBounds.right - _surface.w;
		destBounds.right = _surface.w;
	}

	if (destBounds.bottom > _surface.h) {
		srcBounds.bottom -= destBounds.bottom - _surface.h;
		destBounds.bottom = _surface.h;
	}

	if (destBounds.top < 0) {
		srcBounds.top += -destBounds.top;
		destBounds.top = 0;
	}

	if (destBounds.left < 0) {
		srcBounds.left += -destBounds.left;
		destBounds.left = 0;
	}

	return true;
}

void Surface::clear() {
	fillRect(Common::Rect(0, 0, _surface.w, _surface.h), 0);
}

void Surface::free() {
	if (_freePixels) {
		_surface.free();
		_freePixels = false;
	}
}

void Surface::setPixels(byte *pixels, int width, int height, Graphics::PixelFormat pixelFormat) {
	_surface.format = pixelFormat;
	_surface.w = width;
	_surface.h = height;
	_surface.pitch = width * pixelFormat.bytesPerPixel;
	_surface.setPixels(pixels);
}

void Surface::writeString(const Common::String &str, const Common::Point &pt, byte overrideColor) {
	Fonts::writeString(this, str, pt, overrideColor);
}

void Surface::writeFancyString(const Common::String &str, const Common::Point &pt, byte overrideColor1, byte overrideColor2) {
	writeString(str, Common::Point(pt.x, pt.y), overrideColor1);
	writeString(str, Common::Point(pt.x + 1, pt.y), overrideColor1);
	writeString(str, Common::Point(pt.x + 2, pt.y), overrideColor1);
	writeString(str, Common::Point(pt.x, pt.y + 1), overrideColor1);
	writeString(str, Common::Point(pt.x + 2, pt.y + 1), overrideColor1);
	writeString(str, Common::Point(pt.x, pt.y + 2), overrideColor1);
	writeString(str, Common::Point(pt.x + 1, pt.y + 2), overrideColor1);
	writeString(str, Common::Point(pt.x + 2, pt.y + 2), overrideColor1);
	writeString(str, Common::Point(pt.x + 1, pt.y + 1), overrideColor2);
}

void Surface::maskArea(const ImageFrame &src, const Common::Point &pt, int scrollX) {
	// TODO
	error("TODO: maskArea");
}

} // End of namespace Sherlock
