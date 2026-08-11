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

#include "gamos/blit.h"

namespace Gamos {

void Blitter::blitNormal(Graphics::Surface *src, const Common::Rect &srcRect, Graphics::Surface *dst, const Common::Rect &dstRect) {
	if (dst->format != src->format)
		return;

	Common::Rect drect = dstRect;
	if (drect.right <= drect.left)
		drect.right = dst->w;
	if (drect.bottom <= drect.top)
		drect.bottom = dst->h;

	drect.clip(dst->w, dst->h);

	Common::Rect proj(dstRect.origin(), srcRect.width(), srcRect.height());
	proj.clip(drect);

	if (proj.isEmpty())
		return;

	Common::Rect srect(srcRect.origin() + proj.origin() - dstRect.origin(), proj.width(), proj.height());

	for (int y = 0; y < srect.height(); y++) {
		byte *pdst = (byte *)dst->getBasePtr(proj.left, proj.top + y);
		byte *psrc = (byte *)src->getBasePtr(srect.left, srect.top + y);
		for (int x = srect.left; x < srect.right; x++) {
			if (*psrc != 0)
				*pdst = *psrc;

			psrc++;
			pdst++;
		}
	}

}

void Blitter::blitFlipH(Graphics::Surface *src, const Common::Rect &srcRect, Graphics::Surface *dst, const Common::Rect &dstRect) {
	if (dst->format != src->format)
		return;

	Common::Rect drect = dstRect;
	if (drect.right <= drect.left)
		drect.right = dst->w;
	if (drect.bottom <= drect.top)
		drect.bottom = dst->h;

	drect.clip(dst->w, dst->h);

	Common::Rect proj(dstRect.origin(), srcRect.width(), srcRect.height());
	proj.clip(drect);

	if (proj.isEmpty())
		return;

	Common::Rect srect(srcRect.origin() + proj.origin() - dstRect.origin(), proj.width(), proj.height());

	int32 sw = src->w;

	for (int y = 0; y < srect.height(); y++) {
		byte *pdst = (byte *)dst->getBasePtr(proj.left, proj.top + y);
		byte *psrc = (byte *)src->getBasePtr(sw - 1 - srect.left, srect.top + y);
		for (int x = srect.left; x < srect.right; x++) {
			if (*psrc != 0)
				*pdst = *psrc;

			psrc--;
			pdst++;
		}
	}
}

void Blitter::blitFlipV(Graphics::Surface *src, const Common::Rect &srcRect, Graphics::Surface *dst, const Common::Rect &dstRect) {
	if (dst->format != src->format)
		return;

	Common::Rect drect = dstRect;
	if (drect.right <= drect.left)
		drect.right = dst->w;
	if (drect.bottom <= drect.top)
		drect.bottom = dst->h;

	drect.clip(dst->w, dst->h);

	Common::Rect proj(dstRect.origin(), srcRect.width(), srcRect.height());
	proj.clip(drect);

	if (proj.isEmpty())
		return;

	Common::Rect srect(srcRect.origin() + proj.origin() - dstRect.origin(), proj.width(), proj.height());

	int32 sh = src->h;

	for (int y = 0; y < srect.height(); y++) {
		byte *pdst = (byte *)dst->getBasePtr(proj.left, proj.top + y);
		byte *psrc = (byte *)src->getBasePtr(srect.left, sh - 1 - srect.top - y);
		for (int x = srect.left; x < srect.right; x++) {
			if (*psrc != 0)
				*pdst = *psrc;

			psrc++;
			pdst++;
		}
	}
}

void Blitter::blitFlipVH(Graphics::Surface *src, const Common::Rect &srcRect, Graphics::Surface *dst, const Common::Rect &dstRect) {
	if (dst->format != src->format)
		return;

	Common::Rect drect = dstRect;
	if (drect.right <= drect.left)
		drect.right = dst->w;
	if (drect.bottom <= drect.top)
		drect.bottom = dst->h;

	drect.clip(dst->w, dst->h);

	Common::Rect proj(dstRect.origin(), srcRect.width(), srcRect.height());
	proj.clip(drect);

	if (proj.isEmpty())
		return;

	Common::Rect srect(srcRect.origin() + proj.origin() - dstRect.origin(), proj.width(), proj.height());

	int32 sw = src->w;
	int32 sh = src->h;

	for (int y = 0; y < srect.height(); y++) {
		byte *pdst = (byte *)dst->getBasePtr(proj.left, proj.top + y);
		byte *psrc = (byte *)src->getBasePtr(sw - 1 - srect.left, sh - 1 - srect.top - y);
		for (int x = srect.left; x < srect.right; x++) {
			if (*psrc != 0)
				*pdst = *psrc;

			psrc--;
			pdst++;
		}
	}
}

}
