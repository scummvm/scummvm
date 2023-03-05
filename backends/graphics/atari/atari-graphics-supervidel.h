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

#ifndef BACKENDS_GRAPHICS_ATARI_SUPERVIDEL_H
#define BACKENDS_GRAPHICS_ATARI_SUPERVIDEL_H

#define USE_SV_BLITTER	// TODO: into configure?

#include "backends/graphics/atari/atari-graphics.h"

#include <cstring>
#include <mint/osbind.h>

#ifdef USE_SV_BLITTER
#include "backends/graphics/atari/atari-graphics-superblitter.h"
#endif

#include "backends/graphics/atari/videl-resolutions.h"
#include "common/scummsys.h"
#include "common/textconsole.h"	// for error()

class AtariSuperVidelManager : public AtariGraphicsManager {
public:
	AtariSuperVidelManager() {
#ifdef USE_SV_BLITTER
		_fwVersion = *SV_VERSION & 0x01ff;
		debug("SuperVidel FW Revision: %d, using %s", _fwVersion, _fwVersion >= 9
			  ? "fast async FIFO" : "slower sync blitting" );
#endif

		for (int i = 0; i < SCREENS; ++i) {
			if (!allocateAtariSurface(_screen[i], _screenSurface,
					SCREEN_WIDTH, SCREEN_HEIGHT, PIXELFORMAT8,
					MX_STRAM, 0xA0000000))
				error("Failed to allocate screen memory in ST RAM");
			_screenAligned[i] = (byte*)_screenSurface.getPixels();
		}
		_screenSurface.setPixels(_screenAligned[getDefaultGraphicsMode() <= 1 ? FRONT_BUFFER : BACK_BUFFER1]);

		if (!allocateAtariSurface(_chunkyBuffer, _chunkySurface,
				SCREEN_WIDTH, SCREEN_HEIGHT, PIXELFORMAT8,
				MX_PREFTTRAM))
			error("Failed to allocate chunky buffer memory in ST/TT RAM");

		if (!allocateAtariSurface(_overlayScreen, _screenOverlaySurface,
				getOverlayWidth(), getOverlayHeight(), getOverlayFormat(),
				MX_STRAM, 0xA0000000))
			error("Failed to allocate overlay memory in ST RAM");

		if (!allocateAtariSurface(_overlayBuffer, _overlaySurface,
				getOverlayWidth(), getOverlayHeight(), getOverlayFormat(),
				MX_PREFTTRAM))
			error("Failed to allocate overlay buffer memory in ST/TT RAM");

		// patch SPSHIFT for SuperVidel's BPS8C
		for (byte *p : {scp_320x200x8_vga, scp_320x240x8_vga, scp_640x400x8_vga, scp_640x480x8_vga}) {
			uint16 *p16 = (uint16*)(p + 122 + 30);
			*p16 |= 0x1000;
		}
	}

	~AtariSuperVidelManager() {
#ifdef USE_SV_BLITTER
		ct60_vmfree(_chunkyBuffer);
#else
		Mfree(_chunkyBuffer);
#endif
		_chunkyBuffer = nullptr;

#ifdef USE_SV_BLITTER
		ct60_vmfree(_overlayBuffer);
#else
		Mfree(_overlayBuffer);
#endif
		_overlayBuffer = nullptr;
	}

	virtual const OSystem::GraphicsMode *getSupportedGraphicsModes() const override {
		static const OSystem::GraphicsMode graphicsModes[] = {
			{"direct", "Direct rendering", 0},
			{"single", "Single buffering", 1},
			{"double", "Double buffering", 2},
			{"triple", "Triple buffering", 3},
			{nullptr, nullptr, 0 }
		};
		return graphicsModes;
	}

	int16 getOverlayHeight() const override { return 2 * OVERLAY_HEIGHT; }
	int16 getOverlayWidth() const override { return 2 * OVERLAY_WIDTH; }

private:
	virtual void* allocFast(size_t bytes) const override {
#ifdef USE_SV_BLITTER
		return (void*)ct60_vmalloc(bytes);
#else
		return (void*)Mxalloc(bytes, MX_PREFTTRAM);
#endif
	}

	void copySurfaceToSurface(const Graphics::Surface &srcSurface, Graphics::Surface &dstSurface) const override {
#ifdef USE_SV_BLITTER
		if (_fwVersion >= 9) {
			*SV_BLITTER_FIFO = (long)srcSurface.getPixels();	// SV_BLITTER_SRC1
			*SV_BLITTER_FIFO = 0x00000000;						// SV_BLITTER_SRC2
			*SV_BLITTER_FIFO = (long)dstSurface.getPixels();	// SV_BLITTER_DST
			*SV_BLITTER_FIFO = srcSurface.w - 1;				// SV_BLITTER_COUNT
			*SV_BLITTER_FIFO = srcSurface.pitch;				// SV_BLITTER_SRC1_OFFSET
			*SV_BLITTER_FIFO = 0x00000000;						// SV_BLITTER_SRC2_OFFSET
			*SV_BLITTER_FIFO = dstSurface.pitch;				// SV_BLITTER_DST_OFFSET
			*SV_BLITTER_FIFO = srcSurface.h;					// SV_BLITTER_MASK_AND_LINES
			*SV_BLITTER_FIFO = 0x01;							// SV_BLITTER_CONTROL
		} else {
			sync();

			*SV_BLITTER_SRC1           = (long)srcSurface.getPixels();
			*SV_BLITTER_SRC2           = 0x00000000;
			*SV_BLITTER_DST            = (long)dstSurface.getPixels();
			*SV_BLITTER_COUNT          = srcSurface.w - 1;
			*SV_BLITTER_SRC1_OFFSET    = srcSurface.pitch;
			*SV_BLITTER_SRC2_OFFSET    = 0x00000000;
			*SV_BLITTER_DST_OFFSET     = dstSurface.pitch;
			*SV_BLITTER_MASK_AND_LINES = srcSurface.h;
			*SV_BLITTER_CONTROL        = 0x01;
		}
#else
		memcpy(dstSurface.getPixels(), srcSurface.getPixels(), srcSurface.h * srcSurface.pitch);
#endif
	}

	void copyRectToSurface(const Graphics::Surface &srcSurface, int destX, int destY, Graphics::Surface &dstSurface,
						   const Common::Rect &subRect) const override {
#ifdef USE_SV_BLITTER
		if (_fwVersion >= 9) {
			*SV_BLITTER_FIFO = (long)srcSurface.getBasePtr(subRect.left, subRect.top);	// SV_BLITTER_SRC1
			*SV_BLITTER_FIFO = 0x00000000;												// SV_BLITTER_SRC2
			*SV_BLITTER_FIFO = (long)dstSurface.getBasePtr(destX, destY);				// SV_BLITTER_DST
			*SV_BLITTER_FIFO = subRect.width() - 1;										// SV_BLITTER_COUNT
			*SV_BLITTER_FIFO = srcSurface.pitch;										// SV_BLITTER_SRC1_OFFSET
			*SV_BLITTER_FIFO = 0x00000000;												// SV_BLITTER_SRC2_OFFSET
			*SV_BLITTER_FIFO = dstSurface.pitch;										// SV_BLITTER_DST_OFFSET
			*SV_BLITTER_FIFO = subRect.height();										// SV_BLITTER_MASK_AND_LINES
			*SV_BLITTER_FIFO = 0x01;													// SV_BLITTER_CONTROL
		} else {
			sync();

			*SV_BLITTER_SRC1           = (long)srcSurface.getBasePtr(subRect.left, subRect.top);
			*SV_BLITTER_SRC2           = 0x00000000;
			*SV_BLITTER_DST            = (long)dstSurface.getBasePtr(destX, destY);
			*SV_BLITTER_COUNT          = subRect.width() - 1;
			*SV_BLITTER_SRC1_OFFSET    = srcSurface.pitch;
			*SV_BLITTER_SRC2_OFFSET    = 0x00000000;
			*SV_BLITTER_DST_OFFSET     = dstSurface.pitch;
			*SV_BLITTER_MASK_AND_LINES = subRect.height();
			*SV_BLITTER_CONTROL        = 0x01;
		}
#else
		dstSurface.copyRectToSurface(srcSurface, destX, destY, subRect);
#endif
	}

	void copyRectToSurfaceWithKey(const Graphics::Surface &srcSurface, int destX, int destY, Graphics::Surface &dstSurface,
								  const Common::Rect &subRect, uint32 key) const override {
		sync();
		dstSurface.copyRectToSurfaceWithKey(srcSurface, destX, destY, subRect, key);
	}

	virtual void sync() const override {
#ifdef USE_SV_BLITTER
		// while FIFO not empty...
		if (_fwVersion >= 9)
			while (!(*SV_BLITTER_FIFO & 1));
		// while busy blitting...
		while (*SV_BLITTER_CONTROL & 1);
#endif
	}

#ifdef USE_SV_BLITTER
	int _fwVersion = 0;
#endif
};

#endif
