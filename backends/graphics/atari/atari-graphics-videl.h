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

	virtual const OSystem::GraphicsMode *getSupportedGraphicsModes() const override {
		static const OSystem::GraphicsMode graphicsModes[] = {
			{"single", "Single buffering", 1},
			{"double", "Double buffering", 2},
			{"triple", "Triple buffering", 3},
			{nullptr, nullptr, 0 }
		};
		return graphicsModes;
	}

	OSystem::TransactionError endGFXTransaction() override {
		int error = OSystem::TransactionError::kTransactionSuccess;

		if (_pendingState.mode == GraphicsMode::DirectRendering)
			error |= OSystem::TransactionError::kTransactionModeSwitchFailed;

		if (error != OSystem::TransactionError::kTransactionSuccess) {
			// all our errors are fatal but engine.cpp takes only this one seriously
			error |= OSystem::TransactionError::kTransactionSizeChangeFailed;
			return static_cast<OSystem::TransactionError>(error);
		}

		return AtariGraphicsManager::endGFXTransaction();
	}

private:
	void copyRectToSurface(Graphics::Surface &dstSurface,
						   const Graphics::Surface &srcSurface, int destX, int destY,
						   const Common::Rect &subRect) const override {
		// 'pChunkyEnd' is a delicate parameter: the c2p routine compares it to the address register
		// used for pixel reading; two common mistakes:
		// 1. (subRect.left, subRect.bottom) = beginning of the next line *including the offset*
		// 2. (subRect.right, subRect.bottom) = even worse, end of the *next* line, not current one
		if (subRect.width() == dstSurface.w) {
			asm_c2p1x1_8(
				(const byte*)srcSurface.getBasePtr(subRect.left, subRect.top),
				(const byte*)srcSurface.getBasePtr(subRect.right, subRect.bottom-1),
				(byte*)dstSurface.getBasePtr(destX, destY));
		} else {
			asm_c2p1x1_8_rect(
				(const byte*)srcSurface.getBasePtr(subRect.left, subRect.top),
				(const byte*)srcSurface.getBasePtr(subRect.right, subRect.bottom-1),
				subRect.width(),
				srcSurface.pitch,
				(byte*)dstSurface.getBasePtr(destX, destY),
				dstSurface.pitch);
		}
	}

	void copyRectToSurfaceWithKey(Graphics::Surface &dstSurface, const Graphics::Surface &bgSurface,
								  const Graphics::Surface &srcSurface, int destX, int destY,
								  const Common::Rect &subRect, uint32 key, const byte srcPalette[256*3]) const override {
		Common::Rect backgroundRect(destX, destY, destX + subRect.width(), destY + subRect.height());

		// ensure that background's left and right lie on a 16px boundary and double the width if needed
		backgroundRect.moveTo(backgroundRect.left & 0xfff0, backgroundRect.top);

		const int deltaX = destX - backgroundRect.left;

		backgroundRect.right = (backgroundRect.right + deltaX + 15) & 0xfff0;
		if (backgroundRect.right > bgSurface.w)
			backgroundRect.right = bgSurface.w;

		static Graphics::Surface cachedSurface;

		if (cachedSurface.w != backgroundRect.width()
				|| cachedSurface.h != backgroundRect.height()
				|| cachedSurface.format != bgSurface.format) {
			cachedSurface.create(
				backgroundRect.width(),
				backgroundRect.height(),
				bgSurface.format);
		}

		// copy background
		cachedSurface.copyRectToSurface(bgSurface, 0, 0, backgroundRect);

		// copy cursor
		if (cachedSurface.format == PIXELFORMAT_RGB332) {
			assert(srcSurface.format == PIXELFORMAT_CLUT8);

			// Convert CLUT8 to RGB332 palette and do copyRectToSurfaceWithKey() at the same time
			const byte *src = (const byte*)srcSurface.getBasePtr(subRect.left, subRect.top);
			byte *dst = (byte*)cachedSurface.getBasePtr(deltaX, 0);

			const int16 w = subRect.width();
			const int16 h = subRect.height();

			for (int16 y = 0; y < h; ++y) {
				for (int16 x = 0; x < w; ++x) {
					const uint32 color = *src++;
					if (color != key) {
						*dst++ = (srcPalette[color*3 + 0] & 0xe0)
							  | ((srcPalette[color*3 + 1] >> 3) & 0x1c)
							  | ((srcPalette[color*3 + 2] >> 6) & 0x03);
					} else {
						dst++;
					}
				}

				src += (srcSurface.pitch - w);
				dst += (cachedSurface.pitch - w);
			}
		} else {
			cachedSurface.copyRectToSurfaceWithKey(srcSurface, deltaX, 0, subRect, key);
		}

		copyRectToSurface(
			dstSurface,
			cachedSurface,
			backgroundRect.left, backgroundRect.top,
			Common::Rect(cachedSurface.w, cachedSurface.h));
	}

	void alignRect(const Graphics::Surface &srcSurface, Common::Rect &rect) const override {
		// align on 16px
		rect.left &= 0xfff0;
		rect.right = (rect.right + 15) & 0xfff0;
		if (rect.right > srcSurface.w)
			rect.right = srcSurface.w;
	}
};

#endif
