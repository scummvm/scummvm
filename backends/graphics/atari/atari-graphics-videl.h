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

#ifndef BACKENDS_GRAPHICS_ATARI_VIDEL_H
#define BACKENDS_GRAPHICS_ATARI_VIDEL_H

#include "backends/graphics/atari/atari-graphics.h"

#include "backends/graphics/atari/atari-graphics-asm.h"
#include "backends/graphics/atari/atari_c2p-asm.h"
#include "common/system.h"

class AtariVidelManager : public AtariGraphicsManager {
public:
	AtariVidelManager() {
		// using virtual methods so must be done here
		allocateSurfaces();
	}

	~AtariVidelManager() {
		// using virtual methods so must be done here
		freeSurfaces();
	}

private:
	void copyRectToSurface(Graphics::Surface &dstSurface, int dstBitsPerPixel, const Graphics::Surface &srcSurface,
						   int destX, int destY,
						   const Common::Rect &subRect) const override {
		// 'pChunkyEnd' is a delicate parameter: the c2p routine compares it to the address register
		// used for pixel reading; two common mistakes:
		// 1. (subRect.left, subRect.bottom) = beginning of the next line *including the offset*
		// 2. (subRect.right, subRect.bottom) = even worse, end of the *next* line, not current one
		const byte *pChunky    = (const byte *)srcSurface.getBasePtr(subRect.left, subRect.top);
		const byte *pChunkyEnd = (const byte *)srcSurface.getBasePtr(subRect.right, subRect.bottom-1);

        byte *pScreen = (byte *)dstSurface.getPixels() + destY * dstSurface.pitch + destX * dstBitsPerPixel/8;

		if (dstBitsPerPixel == 8) {
			if (srcSurface.pitch == subRect.width()) {
				if (srcSurface.pitch == dstSurface.pitch) {
					asm_c2p1x1_8(pChunky, pChunkyEnd, pScreen);
					return;
				} else if (srcSurface.pitch == dstSurface.pitch/2) {
                    asm_c2p1x1_8_tt(pChunky, pChunkyEnd, pScreen, dstSurface.pitch);
					return;
				}
			}

			asm_c2p1x1_8_rect(
				pChunky, pChunkyEnd,
				subRect.width(),
				srcSurface.pitch,
				pScreen,
                dstSurface.pitch);
		} else {
            if (srcSurface.pitch == subRect.width() && srcSurface.pitch/2 == dstSurface.pitch) {
				asm_c2p1x1_4(pChunky, pChunkyEnd, pScreen);
				return;
			}

			asm_c2p1x1_4_rect(
				pChunky, pChunkyEnd,
				subRect.width(),
				srcSurface.pitch,
				pScreen,
                dstSurface.pitch);
		}
	}

	void drawMaskedSprite(Graphics::Surface &dstSurface, int dstBitsPerPixel,
						  const Graphics::Surface &srcSurface, const Graphics::Surface &srcMask,
						  int destX, int destY,
						  const Common::Rect &subRect) override {
		if (dstBitsPerPixel == 4) {
			asm_draw_4bpl_sprite(
				(uint16 *)dstSurface.getPixels(), (const uint16 *)srcSurface.getBasePtr(subRect.left, subRect.top),
				(const uint16 *)srcMask.getBasePtr(subRect.left, subRect.top),
				destX, destY,
				dstSurface.pitch, subRect.width(), subRect.height());
		} else if (dstBitsPerPixel == 8) {
			asm_draw_8bpl_sprite(
				(uint16 *)dstSurface.getPixels(), (const uint16 *)srcSurface.getBasePtr(subRect.left, subRect.top),
				(const uint16 *)srcMask.getBasePtr(subRect.left, subRect.top),
				destX, destY,
				dstSurface.pitch, subRect.width(), subRect.height());
		}
	}

	Common::Rect alignRect(int x, int y, int w, int h) const override {
		return Common::Rect(x & (-16), y, (x + w + 15) & (-16), y + h);
	}
};

#endif
