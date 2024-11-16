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

#ifdef USE_SUPERVIDEL

#include "backends/graphics/atari/atari-graphics.h"

#include <mint/osbind.h>

#ifdef USE_SV_BLITTER
#include <mint/trap14.h>
#define ct60_vm(mode, value) (long)trap_14_wwl((short)0xc60e, (short)(mode), (long)(value))
#define ct60_vmalloc(value) ct60_vm(0, value)
#define ct60_vmfree(value)  ct60_vm(1, value)

#include "backends/platform/atari/dlmalloc.h"
extern mspace g_mspace;
#endif

#include "backends/graphics/atari/atari-graphics-superblitter.h"
#include "backends/platform/atari/atari-debug.h"
#include "common/scummsys.h"

class AtariSuperVidelManager : public AtariGraphicsManager {
public:
	AtariSuperVidelManager() {
#ifdef USE_SV_BLITTER
		atari_debug("SuperVidel FW Revision: %d, using %s", superVidelFwVersion, superVidelFwVersion >= 9
			? "fast async FIFO" : "slower sync blitting");
#else
		atari_debug("SuperVidel FW Revision: %d, SuperBlitter not used", superVidelFwVersion);
#endif
		if (Supexec(hasSvRamBoosted))
			atari_debug("SV_XBIOS has the pmmu boost enabled");
		else
			atari_warning("SV_XBIOS has the pmmu boost disabled, set 'pmmu_boost = true' in C:\\SV.INF");

#ifdef USE_SV_BLITTER
		size_t vramSize = ct60_vmalloc(-1) - (16 * 1024 * 1024);	// SV XBIOS seems to forget the initial 16 MB ST RAM mirror
		_vramBase = vramSize > 0 ? (void *)ct60_vmalloc(vramSize) : nullptr;
		if (_vramBase) {
			g_mspace = create_mspace_with_base(_vramBase, vramSize, 0);
			atari_debug("Allocated VRAM at %p (%ld bytes)", _vramBase, vramSize);
		}

		if (!g_mspace)
			atari_warning("VRAM allocation failed");
#endif
		// using virtual methods so must be done here
		allocateSurfaces();
	}

	~AtariSuperVidelManager() {
		// using virtual methods so must be done here
		freeSurfaces();

#ifdef USE_SV_BLITTER
		if (_vramBase) {
			destroy_mspace(g_mspace);
			g_mspace = nullptr;

			ct60_vmfree(_vramBase);
			_vramBase = nullptr;
		}
#endif
	}

private:
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

	void drawMaskedSprite(Graphics::Surface &dstSurface, int dstBitsPerPixel,
						  const Graphics::Surface &srcSurface, const Graphics::Surface &srcMask,
						  int destX, int destY,
						  const Common::Rect &subRect) override {
		assert(dstBitsPerPixel == 8);
		assert(subRect.width() % 16 == 0);
		assert(subRect.width() == srcSurface.w);

		const byte *src = (const byte *)srcSurface.getBasePtr(subRect.left, subRect.top);
		const uint16 *mask = (const uint16 *)srcMask.getBasePtr(subRect.left, subRect.top);
		byte *dst = (byte *)dstSurface.getBasePtr(destX, destY);

		const int h = subRect.height();
		const int w = subRect.width();
		const int dstOffset = dstSurface.pitch - w;

		for (int j = 0; j < h; ++j) {
			for (int i = 0; i < w; i += 16, mask++) {
				const uint16 m = *mask;

				if (m == 0xFFFF) {
					// all 16 pixels transparentm6
					src += 16;
					dst += 16;
					continue;
				}

				for (int k = 0; k < 16; ++k) {
					const uint16 bit = 1 << (15 - k);

					if (m & bit) {
						// transparent
						src++;
						dst++;
					} else {
						*dst++ = *src++;
					}
				}
			}

			dst += dstOffset;
		}
	}

	Common::Rect alignRect(int x, int y, int w, int h) const override {
		return Common::Rect(x, y, x + w, y + h);
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

#ifdef USE_SV_BLITTER
	void *_vramBase = nullptr;
#endif
};

#endif	// USE_SUPERVIDEL

#endif
