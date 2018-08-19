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

#ifndef MUTATIONOFJB_UTIL_H
#define MUTATIONOFJB_UTIL_H

#include "common/rect.h"

#include "graphics/managed_surface.h"
#include "graphics/surface.h"

namespace Common {
class String;
}

namespace MutationOfJB {

void reportFileMissingError(const char *fileName);
Common::String toUpperCP895(const Common::String &str);

// Taken from ManagedSurface::clip.
template<typename SurfaceType>
bool clipBounds(Common::Rect &srcBounds, Common::Rect &destBounds, SurfaceType &destSurf) {
	if (destBounds.left >= destSurf.w || destBounds.top >= destSurf.h ||
			destBounds.right <= 0 || destBounds.bottom <= 0)
		return false;

	// Clip the bounds if source is too big to fit into destination.
	if (destBounds.right > destSurf.w) {
		srcBounds.right -= destBounds.right - destSurf.w;
		destBounds.right = destSurf.w;
	}

	if (destBounds.bottom > destSurf.h) {
		srcBounds.bottom -= destBounds.bottom - destSurf.h;
		destBounds.bottom = destSurf.h;
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

template<typename BlitOp>
void blit_if(const Graphics::Surface &src, const Common::Rect &srcRect, Graphics::Surface &dest, const Common::Point &destPos, BlitOp blitOp) {
	Common::Rect srcBounds = srcRect;
	Common::Rect destBounds(destPos.x, destPos.y, destPos.x + srcRect.width(), destPos.y + srcRect.height());

	assert(srcRect.isValidRect());
	assert(dest.format == src.format);

	if (!clipBounds(srcBounds, destBounds, dest))
		return;

	for (int y = 0; y < srcBounds.height(); ++y) {
		const byte *srcP = reinterpret_cast<const byte *>(src.getBasePtr(srcBounds.left, srcBounds.top + y));
		const byte *srcEndP = srcP + srcBounds.width();
		byte *destP = reinterpret_cast<byte *>(dest.getBasePtr(destBounds.left, destBounds.top + y));

		while (srcP != srcEndP) {
			const byte newColor = blitOp(*srcP, *destP);
			if (*destP != newColor) {
				*destP = newColor;
			}
			++srcP;
			++destP;
		}
	}
}

template<typename BlitOp>
void blit_if(const Graphics::Surface &src, const Common::Rect &srcRect, Graphics::ManagedSurface &dest, const Common::Point &destPos, BlitOp blitOp) {
	Common::Rect srcBounds = srcRect;
	Common::Rect destBounds(destPos.x, destPos.y, destPos.x + srcRect.width(), destPos.y + srcRect.height());

	assert(srcRect.isValidRect());
	assert(dest.format == src.format);

	if (!clipBounds(srcBounds, destBounds, dest))
		return;

	Graphics::Surface destSurf = dest.getSubArea(destBounds); // This will invalidate the rectangle.
	blit_if(src, srcRect, destSurf, Common::Point(0, 0), blitOp);
}

template<typename BlitOp>
void blit_if(const Graphics::Surface &src, Graphics::Surface &dest, const Common::Point &destPos, BlitOp blitOp) {
	blit_if(src, Common::Rect(0, 0, src.w, src.h), dest, destPos, blitOp);
}

template<typename BlitOp>
void blit_if(const Graphics::Surface &src, Graphics::ManagedSurface &dest, const Common::Point &destPos, BlitOp blitOp) {
	blit_if(src, Common::Rect(0, 0, src.w, src.h), dest, destPos, blitOp);
}

}

#endif
