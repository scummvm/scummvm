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

#include "common/algorithm.h"
#include "common/util.h"
#include "xeen/xsurface.h"
#include "xeen/resources.h"
#include "xeen/screen.h"

namespace Xeen {

XSurface::XSurface() : Graphics::Surface(), _freeFlag(false) {
}

XSurface::XSurface(int w, int h) : Graphics::Surface(), _freeFlag(false) {
	create(w, h);
}

XSurface::~XSurface() {
	if (_freeFlag)
		free();
}

void XSurface::create(uint16 w, uint16 h) {
	Graphics::Surface::create(w, h, Graphics::PixelFormat::createFormatCLUT8());
	_freeFlag = true;
}

void XSurface::create(XSurface *s, const Common::Rect &bounds) {
	pixels = (byte *)s->getBasePtr(bounds.left, bounds.top);
	format = Graphics::PixelFormat::createFormatCLUT8();
	pitch = s->pitch;
	w = bounds.width();
	h = bounds.height();

	_freeFlag = false;
}


void XSurface::transBlitTo(XSurface &dest) const {
	transBlitTo(dest, Common::Point());
}

void XSurface::blitTo(XSurface &dest) const {
	blitTo(dest, Common::Point());
}

void XSurface::transBlitTo(XSurface &dest, const Common::Point &destPos) const {
	if (dest.getPixels() == nullptr)
		dest.create(w, h);

	for (int yp = 0; yp < h; ++yp) {
		const byte *srcP = (const byte *)getBasePtr(0, yp);
		byte *destP = (byte *)dest.getBasePtr(destPos.x, destPos.y + yp);

		for (int xp = 0; xp < w; ++xp, ++srcP, ++destP) {
			if (*srcP != 0)
				*destP = *srcP;
		}
	}

	dest.addDirtyRect(Common::Rect(destPos.x, destPos.y, destPos.x + w, destPos.y));
}

void XSurface::transBlitTo(XSurface &dest, const Common::Point &destPos, 
		int scale, int transparentColor) {
	int destX = destPos.x, destY = destPos.y;
	int frameWidth = this->w;
	int frameHeight = this->h;
	int direction = 1;

	int highestDim = MAX(frameWidth, frameHeight);
	bool lineDist[SCREEN_WIDTH];
	int distXCount = 0, distYCount = 0;

	if (scale != 0) {
		int distCtr = 0;
		int distIndex = 0;
		do {
			distCtr += scale;
			if (distCtr < 100) {
				lineDist[distIndex] = false;
			}
			else {
				lineDist[distIndex] = true;
				distCtr -= 100;

				if (distIndex < frameWidth)
					++distXCount;

				if (distIndex < frameHeight)
					++distYCount;
			}
		} while (++distIndex < highestDim);

		destX -= distXCount / 2;
		destY -= distYCount - 1;
	}

	// Start of draw logic for scaled sprites
	const byte *srcPixelsP = (const byte *)getPixels();

	int destRight = dest.w - 1;
	int destBottom = dest.h - 1;

	// Check x bounding area
	int spriteLeft = 0;
	int spriteWidth = distXCount;
	int widthAmount = destX + distXCount - 1;

	if (destX < 0) {
		spriteWidth += destX;
		spriteLeft -= destX;
	}
	widthAmount -= destRight;
	if (widthAmount > 0)
		spriteWidth -= widthAmount;

	if (spriteWidth <= 0)
		return;

	// Check y bounding area
	int spriteTop = 0;
	int spriteHeight = distYCount;
	int heightAmount = destY + distYCount - 1;

	if (destY < 0) {
		spriteHeight += destY;
		spriteTop -= destY;
	}
	heightAmount -= destBottom;
	if (heightAmount > 0)
		spriteHeight -= heightAmount;
	int spriteBottom = spriteTop + spriteHeight;

	if (spriteHeight <= 0)
		return;

	byte *destPixelsP = (byte *)dest.getBasePtr(destX + spriteLeft, destY + spriteTop);
	int destWidth = 0, destHeight = 0;

	spriteLeft = spriteLeft * direction;

	// Loop through the lines of the sprite
	for (int yp = 0, sprY = -1; yp < frameHeight; ++yp, srcPixelsP += this->pitch) {
		if (!lineDist[yp])
			// Not a display line, so skip it
			continue;
		// Check whether the sprite line is in the display range
		++sprY;
		if ((sprY >= spriteBottom) || (sprY < spriteTop))
			continue;

		// Found a line to display. Loop through the pixels
		const byte *srcP = srcPixelsP;
		byte *destP = destPixelsP;
		++destHeight;

		for (int xp = 0, sprX = 0; xp < frameWidth; ++xp, ++srcP) {
			if (xp < spriteLeft)
				// Not yet reached start of display area
				continue;
			if (!lineDist[sprX++])
				// Not a display pixel
				continue;

			if (*srcP != transparentColor)
				*destP = *srcP;

			destP += direction;
		}

		// Keep track of widest line drawn
		destWidth = MAX(destP - destPixelsP, destWidth);

		// Move to the next destination line
		destPixelsP += dest.pitch;
	}

	// Add a dirty rect for the affected area
	dest.addDirtyRect(Common::Rect(destX + spriteLeft, destY + spriteTop,
		destX + spriteLeft + destWidth, destY + spriteTop + destHeight));
}

void XSurface::blitTo(XSurface &dest, const Common::Point &destPos) const {
	if (dest.getPixels() == nullptr)
		dest.create(w, h);

	for (int yp = 0; yp < h; ++yp) {
		const byte *srcP = (const byte *)getBasePtr(0, yp);
		byte *destP = (byte *)dest.getBasePtr(destPos.x, destPos.y + yp);

		Common::copy(srcP, srcP + w, destP);
	}

	dest.addDirtyRect(Common::Rect(destPos.x, destPos.y, destPos.x + w, destPos.y + h));
}

} // End of namespace Xeen
