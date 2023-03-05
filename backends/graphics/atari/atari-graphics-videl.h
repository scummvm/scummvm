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

#include <mint/osbind.h>

#include "backends/graphics/atari/atari_c2p-asm.h"
#include "common/system.h"
#include "common/textconsole.h"	// for error()

class AtariVidelManager : public AtariGraphicsManager {
public:
	AtariVidelManager() {
		for (int i = 0; i < SCREENS; ++i) {
			if (!allocateAtariSurface(_screen[i], _screenSurface, SCREEN_WIDTH, SCREEN_HEIGHT, PIXELFORMAT8, MX_STRAM))
				error("Failed to allocate screen memory in ST RAM");
			_screenAligned[i] = (byte*)_screenSurface.getPixels();
		}
		_screenSurface.setPixels(_screenAligned[getDefaultGraphicsMode() <= 1 ? FRONT_BUFFER : BACK_BUFFER1]);

		if (!allocateAtariSurface(_chunkyBuffer, _chunkySurface, SCREEN_WIDTH, SCREEN_HEIGHT, PIXELFORMAT8, MX_PREFTTRAM))
			error("Failed to allocate chunky buffer memory in ST/TT RAM");

		if (!allocateAtariSurface(_overlayScreen, _screenOverlaySurface, getOverlayWidth(), getOverlayHeight(),
								  getOverlayFormat(), MX_STRAM))
			error("Failed to allocate overlay memory in ST RAM");

		if (!allocateAtariSurface(_overlayBuffer, _overlaySurface, getOverlayWidth(), getOverlayHeight(),
								  getOverlayFormat(), MX_PREFTTRAM))
			error("Failed to allocate overlay buffer memory in ST/TT RAM");
	}

	~AtariVidelManager() {
		Mfree(_chunkyBuffer);
		_chunkyBuffer = nullptr;

		Mfree(_overlayBuffer);
		_overlayBuffer = nullptr;
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

	int16 getOverlayHeight() const override { return _vgaMonitor ? OVERLAY_HEIGHT : 2 * OVERLAY_HEIGHT; }
	int16 getOverlayWidth() const override { return _vgaMonitor ? OVERLAY_WIDTH : 2 * OVERLAY_WIDTH; }

private:
	virtual void* allocFast(size_t bytes) const override {
		return (void*)Mxalloc(bytes, MX_PREFTTRAM);
	}

	void copySurfaceToSurface(const Graphics::Surface &srcSurface, Graphics::Surface &dstSurface) const override {
		asm_c2p1x1_8(
			(const byte*)srcSurface.getPixels(),
			(const byte*)srcSurface.getBasePtr(srcSurface.w, srcSurface.h-1),
			(byte*)dstSurface.getPixels());
	}

	void copyRectToSurface(const Graphics::Surface &srcSurface, int destX, int destY,
						   Graphics::Surface &dstSurface, const Common::Rect &subRect) const override {
		// 'pChunkyEnd' is a delicate parameter: the c2p routine compares it to the address register
		// used for pixel reading; two common mistakes:
		// 1. (subRect.left, subRect.bottom) = beginning of the next line *including the offset*
		// 2. (subRect.right, subRect.bottom) = even worse, end of the *next* line, not current one
		asm_c2p1x1_8_rect(
			(const byte*)srcSurface.getBasePtr(subRect.left, subRect.top),
			(const byte*)srcSurface.getBasePtr(subRect.right, subRect.bottom-1),
			subRect.width(),
			srcSurface.pitch,
			(byte*)dstSurface.getBasePtr(destX, destY),
			dstSurface.pitch);
	}

	// TODO: allow specifying different background than _chunkySurface?
	void copyRectToSurfaceWithKey(const Graphics::Surface &srcSurface, int destX, int destY,
								  Graphics::Surface &dstSurface, const Common::Rect &subRect, uint32 key) const override {
		Common::Rect backgroundRect(destX, destY, destX + subRect.width(), destY + subRect.height());

		// ensure that background's left and right lie on a 16px boundary and double the width if needed
		backgroundRect.moveTo(backgroundRect.left & 0xfff0, backgroundRect.top);

		const int deltaX = destX - backgroundRect.left;

		backgroundRect.right = (backgroundRect.right + deltaX + 15) & 0xfff0;
		if (backgroundRect.right > _chunkySurface.w)
			backgroundRect.right = _chunkySurface.w;

		static Graphics::Surface cachedSurface;

		if (cachedSurface.w != backgroundRect.width() || cachedSurface.h != backgroundRect.height()) {
			cachedSurface.create(
				backgroundRect.width(),
				backgroundRect.height(),
				_chunkySurface.format);
		}

		// copy background
		cachedSurface.copyRectToSurface(_chunkySurface, 0, 0, backgroundRect);
		// copy cursor
		cachedSurface.copyRectToSurfaceWithKey(srcSurface, deltaX, 0, subRect, key);

		copyRectToSurface(
			cachedSurface,
			backgroundRect.left, backgroundRect.top,
			dstSurface,
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
