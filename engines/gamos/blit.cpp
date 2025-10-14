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

    Common::Rect srect = srcRect;
    srect.translate(dstRect.left, dstRect.top);
    srect.clip(drect);

    drect = srect;
    srect.translate(-dstRect.left, -dstRect.top);

    if (srect.isEmpty())
        return;

    for(int y = 0; y < srect.height(); y++) {
        byte *pdst = (byte *)dst->getBasePtr(drect.left, drect.top + y);
        byte *psrc = (byte *)src->getBasePtr(srect.left, srect.top + y);
        for(int x = srect.left; x < srect.right; x++) {
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

    Common::Rect srect = srcRect;
    srect.translate(dstRect.left, dstRect.top);
    srect.clip(drect);

    drect = srect;
    srect.translate(-dstRect.left, -dstRect.top);

    if (srect.isEmpty())
        return;

    for(int y = srect.top; y < srect.bottom; y++) {
        byte *pdst = (byte *)dst->getBasePtr(drect.left, drect.top + y);
        byte *psrc = (byte *)src->getBasePtr(srect.right - 1, y);
        for(int x = srect.left; x < srect.right; x++) {
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

    Common::Rect srect = srcRect;
    srect.translate(dstRect.left, dstRect.top);
    srect.clip(drect);

    drect = srect;
    srect.translate(-dstRect.left, -dstRect.top);

    if (srect.isEmpty())
        return;

    for(int y = srect.top; y < srect.bottom; y++) {
        byte *pdst = (byte *)dst->getBasePtr(drect.left, drect.top + y);
        byte *psrc = (byte *)src->getBasePtr(srect.left, srect.bottom - 1 - y);
        for(int x = srect.left; x < srect.right; x++) {
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

    Common::Rect srect = srcRect;
    srect.translate(dstRect.left, dstRect.top);
    srect.clip(drect);

    drect = srect;
    srect.translate(-dstRect.left, -dstRect.top);

    if (srect.isEmpty())
        return;

    for(int y = srect.top; y < srect.bottom; y++) {
        byte *pdst = (byte *)dst->getBasePtr(drect.left, drect.top + y);
        byte *psrc = (byte *)src->getBasePtr(srect.right - 1, srect.bottom - 1 - y);
        for(int x = srect.left; x < srect.right; x++) {
            if (*psrc != 0)
                *pdst = *psrc;

            psrc--;
            pdst++;
        }
    }
}

}
