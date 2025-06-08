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

template<typename T>
static inline void copyPixel(const T *srcP, T *destP, int mode, const T &WHITE) {
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
		*destP = ~*srcP;
		break;
	case DSTINVERT:
		*destP = ~*destP;
		break;
	case BLACKNESS:
		*destP = 0;
		break;
	case WHITENESS:
		*destP = WHITE;
		break;
	default:
		error("Unsupport blit mode");
		break;
	}
}


template<typename T>
static void normalBlit(const Graphics::ManagedSurface *srcSurface,
		Graphics::ManagedSurface *destSurface,
		const Common::Rect &srcRect, const Common::Point &destPos,
		int mode) {
	const T WHITE = destSurface->format.RGBToColor(255, 255, 255);

	for (int y = 0; y < srcRect.height(); ++y) {
		const T *srcP = (const T *)srcSurface->getBasePtr(
			srcRect.left, srcRect.top + y);
		T *destP = (T *)destSurface->getBasePtr(
			destPos.x, destPos.y + y);

		for (int x = 0; x < srcRect.width(); ++x, ++srcP, ++destP) {
			copyPixel<T>(srcP, destP, mode, WHITE);
		}
	}
}

template<typename T>
static void stretchBlit(const Graphics::ManagedSurface *srcSurface,
		Graphics::ManagedSurface *destSurface,
		const Common::Rect &srcRect, const Common::Rect &dstRect,
		int mode) {
	const T WHITE = destSurface->format.RGBToColor(255, 255, 255);

	const int srcWidth = srcRect.right - srcRect.left;
	const int srcHeight = srcRect.bottom - srcRect.top;
	const int dstWidth = dstRect.right - dstRect.left;
	const int dstHeight = dstRect.bottom - dstRect.top;

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

			const T *srcP = (const T *)srcSurface->getBasePtr(srcX, srcY);
			T *destP = (T *)destSurface->getBasePtr(dstX, dstY);

			copyPixel<T>(srcP, destP, mode, WHITE);
		}
	}
}

void blit(const Graphics::ManagedSurface *src,
		Graphics::ManagedSurface *dest,
		const Common::Rect &srcRect, const Common::Point &destPos,
		int mode) {
	assert(src->format.bytesPerPixel == dest->format.bytesPerPixel);

	switch (src->format.bytesPerPixel) {
	case 1:
		normalBlit<byte>(src, dest, srcRect, destPos, mode);
		break;
	case 2:
		normalBlit<uint16>(src, dest, srcRect, destPos, mode);
		break;
	case 4:
		normalBlit<uint32>(src, dest, srcRect, destPos, mode);
		break;
	default:
		error("Invalid surface bytesPerPixel");
		break;
	}
}

void stretchBlit(const Graphics::ManagedSurface *src,
		Graphics::ManagedSurface *dest,
		const Common::Rect &srcRect, const Common::Rect &destRect,
		int mode) {
	assert(src->format.bytesPerPixel == dest->format.bytesPerPixel);

	switch (src->format.bytesPerPixel) {
	case 1:
		stretchBlit<byte>(src, dest, srcRect, destRect, mode);
		break;
	case 2:
		stretchBlit<uint16>(src, dest, srcRect, destRect, mode);
		break;
	case 4:
		stretchBlit<uint32>(src, dest, srcRect, destRect, mode);
		break;
	default:
		error("Invalid surface bytesPerPixel");
		break;
	}
}

} // namespace Gfx
} // namespace MFC
} // namespace Bagel
