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
#include "common/system.h"
#include "graphics/palette.h"

namespace Sherlock {

Surface::Surface(uint16 width, uint16 height): _freePixels(true) {
	create(width, height);
}

Surface::Surface() : _freePixels(false) {
}

Surface::~Surface() {
	if (_freePixels)
		free();
}

/**
 * Sets up an internal surface with the specified dimensions that will be automatically freed
 * when the surface object is destroyed
 */
void Surface::create(uint16 width, uint16 height) {
	if (_freePixels)
		free();

	Graphics::Surface::create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	_freePixels = true;
}

/**
 * Copy a surface into this one
 */
void Surface::blitFrom(const Graphics::Surface &src) {
	blitFrom(src, Common::Point(0, 0));
}

/**
 * Draws a surface at a given position within this surface
 */
void Surface::blitFrom(const Graphics::Surface &src, const Common::Point &pt) {
	blitFrom(src, pt, Common::Rect(0, 0, src.w, src.h));
}

/**
 * Draws a sub-section of a surface at a given position within this surface
 */
void Surface::blitFrom(const Graphics::Surface &src, const Common::Point &pt,
		const Common::Rect &srcBounds) {
	Common::Rect srcRect = srcBounds;
	Common::Rect destRect(pt.x, pt.y, pt.x + srcRect.width(), pt.y + srcRect.height());

	if (srcRect.isValidRect() && clip(srcRect, destRect)) {
		// Surface is at least partially or completely on-screen
		addDirtyRect(destRect);
		copyRectToSurface(src, destRect.left, destRect.top, srcRect);
	}
}

/**
* Draws an image frame at a given position within this surface with transparency
*/
void Surface::transBlitFrom(const ImageFrame &src, const Common::Point &pt,
		bool flipped, int overrideColor) {
	transBlitFrom(src._frame, pt + src._offset, flipped, overrideColor);
}

/**
* Draws a surface at a given position within this surface with transparency
*/
void Surface::transBlitFrom(const Graphics::Surface &src, const Common::Point &pt,
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

	// Draw loop
	const int TRANSPARENCY = 0xFF;
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
}

/**
 * Fill a given area of the surface with a given color
 */
void Surface::fillRect(int x1, int y1, int x2, int y2, byte color) {
	fillRect(Common::Rect(x1, y1, x2, y2), color);
}

/**
 * Fill a given area of the surface with a given color
 */
void Surface::fillRect(const Common::Rect &r, byte color) {
	Graphics::Surface::fillRect(r, color);
	addDirtyRect(r);
}

/**
 * Clips the given source bounds so the passed destBounds will be entirely on-screen
 */
bool Surface::clip(Common::Rect &srcBounds, Common::Rect &destBounds) {
	if (destBounds.left >= this->w || destBounds.top >= this->h ||
		destBounds.right <= 0 || destBounds.bottom <= 0)
		return false;

	// Clip the bounds if necessary to fit on-screen
	if (destBounds.right > this->w) {
		srcBounds.right -= destBounds.right - this->w;
		destBounds.right = this->w;
	}

	if (destBounds.bottom > this->h) {
		srcBounds.bottom -= destBounds.bottom - this->h;
		destBounds.bottom = this->h;
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

/**
 * Clear the screen
 */
void Surface::clear() {
	fillRect(Common::Rect(0, 0, this->w, this->h), 0);
}

} // End of namespace Sherlock
