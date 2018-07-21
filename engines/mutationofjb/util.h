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

template <typename BlitOp>
void blit_if(const Graphics::Surface &src, const Common::Rect &srcRect, Graphics::ManagedSurface &dest, const Common::Point &destPos, BlitOp blitOp) {
	const Common::Rect srcBounds = srcRect;
	const Common::Rect destBounds(destPos.x, destPos.y, destPos.x + srcRect.width(), destPos.y + srcRect.height());

	assert(srcRect.isValidRect());
	assert(dest.format == src.format);

	for (int y = 0; y < srcBounds.height(); ++y) {
		const byte *srcP = reinterpret_cast<const byte *>(src.getBasePtr(srcBounds.left, srcBounds.top + y));
		const byte *srcEndP = srcP + srcBounds.width();
		byte *destP = reinterpret_cast<byte *>(dest.getBasePtr(destBounds.left, destBounds.top + y));

		while (srcP != srcEndP) {
			if (blitOp(*srcP, *destP)) {
				*destP = *srcP;
			}
			++srcP;
			++destP;
		}
	}

	dest.getSubArea(destBounds); // This is a hack to invalidate the destination rectangle.
}

template <typename BlitOp>
void blit_if(const Graphics::Surface &src, Graphics::ManagedSurface &dest, const Common::Point &destPos, BlitOp blitOp) {
	blit_if(src, Common::Rect(0, 0, src.w, src.h), dest, destPos, blitOp);
}

}

#endif
