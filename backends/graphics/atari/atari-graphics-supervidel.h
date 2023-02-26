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

#include "backends/graphics/atari/atari-graphics.h"

#include <mint/osbind.h>

#include "backends/graphics/atari/atari-graphics-superblitter.h"
#include "backends/graphics/atari/videl-resolutions.h"
#include "common/debug.h"	// error() & warning()
#include "common/scummsys.h"

class AtariSuperVidelManager : public AtariGraphicsManager {
public:
	AtariSuperVidelManager() {
#ifdef USE_SV_BLITTER
		debug("SuperVidel FW Revision: %d, using %s", superVidelFwVersion, superVidelFwVersion >= 9
			? "fast async FIFO" : "slower sync blitting");
#else
		debug("SuperVidel FW Revision: %d, SuperBlitter not used", superVidelFwVersion);
#endif
		if (Supexec(hasSvRamBoosted))
			debug("SV_XBIOS has the pmmu boost enabled");
		else
			warning("SV_XBIOS has the pmmu boost disabled, set 'pmmu_boost = true' in C:\\SV.INF");

		// patch SPSHIFT for SuperVidel's BPS8C
		for (byte *p : {scp_320x200x8_vga, scp_320x240x8_vga, scp_640x400x8_vga, scp_640x480x8_vga}) {
			uint16 *p16 = (uint16*)(p + 122 + 30);
			*p16 |= 0x1000;
		}

		// using virtual methods so must be done here
		allocateSurfaces();
	}

	~AtariSuperVidelManager() {
		// using virtual methods so must be done here
		freeSurfaces();
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

protected:
	AtariMemAlloc getStRamAllocFunc() const override {
		return [](size_t bytes) {
			uintptr ptr = Mxalloc(bytes, MX_STRAM);

			if (ptr != 0)
				ptr |= 0xA0000000;

			return (void*)ptr;
		};
	}
	AtariMemFree getStRamFreeFunc() const override {
		return [](void *ptr) { Mfree((uintptr)ptr & 0x00FFFFFF); };
	}

private:
	void copyRectToSurface(Graphics::Surface &dstSurface,
						   const Graphics::Surface &srcSurface, int destX, int destY,
						   const Common::Rect &subRect) const override {
		dstSurface.copyRectToSurface(srcSurface, destX, destY, subRect);
	}

	void copyRectToSurfaceWithKey(Graphics::Surface &dstSurface, const Graphics::Surface &bgSurface,
								  const Graphics::Surface &srcSurface, int destX, int destY,
								  const Common::Rect &subRect, uint32 key, const byte srcPalette[256*3]) const override {
		dstSurface.copyRectToSurfaceWithKey(srcSurface, destX, destY, subRect, key);
	}

	static long hasSvRamBoosted() {
		register long ret __asm__ ("d0") = 0;

		__asm__ volatile(
		"\tmovec	%%itt0,%%d1\n"
		"\tcmp.l	#0xA007E060,%%d1\n"
		"\tbne.s	1f\n"

		"\tmovec	%%dtt0,%%d1\n"
		"\tcmp.l	#0xA007E060,%%d1\n"
		"\tbne.s	1f\n"

		"\tmoveq	#1,%%d0\n"

	"1:\n"
		: "=g"(ret)	/* outputs */
		:			/* inputs  */
		: __CLOBBER_RETURN("d0") "d1", "cc"
		);

		return ret;
	}
};

#endif
