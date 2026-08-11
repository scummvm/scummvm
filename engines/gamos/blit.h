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

#ifndef GAMOS_BLIT_H
#define GAMOS_BLIT_H

#include "graphics/surface.h"
#include "common/rect.h"

namespace Gamos {

class Blitter {
protected:
	static void blitNormal(Graphics::Surface *src, const Common::Rect &srcRect, Graphics::Surface *dst, const Common::Rect &dstRect);
	static void blitFlipH(Graphics::Surface *src, const Common::Rect &srcRect, Graphics::Surface *dst, const Common::Rect &dstRect);
	static void blitFlipV(Graphics::Surface *src, const Common::Rect &srcRect, Graphics::Surface *dst, const Common::Rect &dstRect);
	static void blitFlipVH(Graphics::Surface *src, const Common::Rect &srcRect, Graphics::Surface *dst, const Common::Rect &dstRect);

public:
	static void blit(Graphics::Surface *src, const Common::Rect &srcRect, Graphics::Surface *dst, const Common::Rect &dstRect, uint flip = 0) {
		switch (flip) {
		default:
			blitNormal(src, srcRect, dst, dstRect);
			break;

		case Graphics::FLIP_H:
			blitFlipH(src, srcRect, dst, dstRect);
			break;

		case Graphics::FLIP_V:
			blitFlipV(src, srcRect, dst, dstRect);
			break;

		case Graphics::FLIP_VH:
			blitFlipVH(src, srcRect, dst, dstRect);
			break;
		}
	};
};

}

#endif
