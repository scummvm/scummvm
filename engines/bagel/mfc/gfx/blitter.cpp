/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/textconsole.h"
#include "bagel/mfc/gfx/blitter.h"
#include "bagel/mfc/afxwin.h"

namespace Bagel {
namespace MFC {
namespace Gfx {

static inline void copyPixel(const byte *srcP, byte *destP, int mode, const byte &WHITE,
		bool isDestMonochrome, uint bgColor) {
	switch (mode) {
	case SRCCOPY:
		*destP = *srcP;
		break;
	case SRCAND:
		*destP &= *srcP;
		break;
	case SRCINVERT:
		*destP ^= *srcP;
		break;
	case SRCPAINT:
		*destP |= *srcP;
		break;
	case NOTSRCCOPY:
		if (isDestMonochrome) {
			*destP = *srcP == bgColor ? 0 : 0xff;
			return;
		}

		*destP = ~*srcP;
		break;
	case DSTINVERT:
		*destP = ~*destP;
		return;
	case BLACKNESS:
		*destP = 0;
		return;
	case WHITENESS:
		*destP = WHITE;
		return;
	default:
		error("Unsupported blit mode");
		break;
	}

	if (isDestMonochrome)
		*destP = *destP == bgColor ? 0xff : 0;
}

static void blitInner(Gfx::Surface *srcSurface,
		Gfx::Surface *destSurface,
		const Common::Rect &srcRect, const Common::Point &destPos,
		uint bgColor, int mode) {
	const bool isDestMonochrome = destSurface->format.bytesPerPixel == 1 &&
		destSurface->format.aLoss == 255;
	const byte WHITE = 255;
	Surface::YIterator ySrc(srcSurface);
	Surface::XIterator xSrc(&ySrc);
	Surface::YIterator yDest(destSurface);
	Surface::XIterator xDest(&yDest);
	byte dummy = 0;
	byte *srcP, *destP;

	for (ySrc = srcRect.top, yDest = destPos.y; ySrc < srcRect.bottom; ++ySrc, ++yDest) {
		for (xSrc = srcRect.left, xDest = destPos.x; xSrc < srcRect.right; ++xSrc, ++xDest) {
			srcP = xSrc;
			destP = xDest;
			if (!srcP)
				srcP = &dummy;
			if (!destP)
				destP = &dummy;

			copyPixel(srcP, destP, mode, WHITE, isDestMonochrome, bgColor);
		}
	}
}

static void stretchBlitInner(Gfx::Surface *srcSurface,
		Gfx::Surface *destSurface,
		const Common::Rect &srcRect, const Common::Rect &dstRect,
		uint bgColor, int mode) {
	const bool isDestMonochrome = destSurface->format.bytesPerPixel == 1 &&
		destSurface->format.aLoss == 255;
	const byte WHITE = 255;
	const int srcWidth = srcRect.right - srcRect.left;
	const int srcHeight = srcRect.bottom - srcRect.top;
	const int dstWidth = dstRect.right - dstRect.left;
	const int dstHeight = dstRect.bottom - dstRect.top;
	Surface::YIterator ySrc(srcSurface);
	Surface::XIterator xSrc(&ySrc);
	Surface::YIterator yDest(destSurface);
	Surface::XIterator xDest(&yDest);

	if (srcWidth <= 0 || srcHeight <= 0 || dstWidth <= 0 || dstHeight <= 0)
		return; // Invalid rectangles

	for (int y = 0; y < dstHeight; ++y) {
		// Map destination y to source y using fixed-point arithmetic
		int srcY = srcRect.top + (y * srcHeight) / dstHeight;
		if (srcY >= srcSurface->h)
			continue;

		int dstY = dstRect.top + y;
		if (dstY >= destSurface->h)
			continue;

		for (int x = 0; x < dstWidth; ++x) {
			int srcX = srcRect.left + (x * srcWidth) / dstWidth;
			if (srcX >= srcSurface->w)
				continue;
			int dstX = dstRect.left + x;
			if (dstX >= destSurface->w)
				continue;

			xSrc = srcX;
			ySrc = srcY;
			byte *srcP = xSrc;
			xDest = dstX;
			yDest = dstY;
			byte *destP = xDest;

			copyPixel(srcP, destP, mode, WHITE, isDestMonochrome, bgColor);
		}
	}
}

void blit(Gfx::Surface *src, Gfx::Surface *dest,
		const Common::Rect &srcRect, const Common::Point &destPos,
		uint bgColor, int mode) {
	// For normal copying modes, the formats must match.
	// Other modes like DSTINVERT don't need a source,
	// so in that case the source can remain uninitialized
	assert(src->format.bytesPerPixel == dest->format.bytesPerPixel ||
		src->format.bytesPerPixel == 0);
	assert(dest->format.bytesPerPixel == dest->format.bytesPerPixel ||
		dest->format.bytesPerPixel == 0);

	blitInner(src, dest, srcRect, destPos, bgColor, mode);

	Common::Rect dirtyRect(destPos.x, destPos.y,
		destPos.x + srcRect.width(), destPos.y + srcRect.height());
	dest->addDirtyRect(dirtyRect);
}

void stretchBlit(Gfx::Surface *src, Gfx::Surface *dest,
		const Common::Rect &srcRect, const Common::Rect &destRect,
		uint bgColor, int mode) {
	assert(src->format.bytesPerPixel == dest->format.bytesPerPixel ||
		src->format.bytesPerPixel == 0);
	assert(dest->format.bytesPerPixel == dest->format.bytesPerPixel ||
		dest->format.bytesPerPixel == 0);

	stretchBlitInner(src, dest, srcRect, destRect, bgColor, mode);
}

static inline void rasterPixel(byte *pixel, byte) {
	// Currently only R2_NOT
	*pixel = ~*pixel;
}

void frameRect(Gfx::Surface *dest,
		const Common::Rect &r, byte color, int drawMode) {
	assert(dest->format.bytesPerPixel == 1);

	if (drawMode == R2_COPYPEN) {
		dest->frameRect(r, color);
		return;
	} else if (drawMode == R2_NOP) {
		return;
	}

	assert(drawMode == R2_NOT);

	const int w = r.right - r.left;
	const int h = r.bottom - r.top - 2;
	byte *pixel;

	// Top line
	pixel = (byte *)dest->getBasePtr(r.left, r.top);
	for (int x = 0; x < w; ++x, ++pixel)
		rasterPixel(pixel, color);

	// Bottom line
	pixel = (byte *)dest->getBasePtr(r.left, r.bottom - 1);
	for (int x = 0; x < w; ++x, ++pixel)
		rasterPixel(pixel, color);

	// Left edge
	pixel = (byte *)dest->getBasePtr(r.left, r.top + 1);
	for (int y = 0; y < h; ++y, pixel += dest->pitch)
		rasterPixel(pixel, color);

	// Right edge
	pixel = (byte *)dest->getBasePtr(r.right - 1, r.top + 1);
	for (int y = 0; y < h; ++y, pixel += dest->pitch)
		rasterPixel(pixel, color);

	// Mark the rectangle area as dirty
	dest->addDirtyRect(r);
}

} // namespace Gfx
} // namespace MFC
} // namespace Bagel
