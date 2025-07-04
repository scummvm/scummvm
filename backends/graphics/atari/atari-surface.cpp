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

#include "atari-surface.h"
#include "graphics/surface.h"

#include <mint/cookie.h>
#include <mint/falcon.h>
#include <mint/trap14.h>
#define ct60_vm(mode, value) (long)trap_14_wwl((short)0xc60e, (short)(mode), (long)(value))
#define ct60_vmalloc(value)  ct60_vm(0, value)
#define ct60_vmfree(value)   ct60_vm(1, value)

#include "backends/graphics/atari/atari-c2p-asm.h"
#include "backends/graphics/atari/atari-graphics-asm.h"
#include "backends/graphics/atari/atari-supervidel.h"
#include "backends/platform/atari/atari-debug.h"
#include "backends/platform/atari/dlmalloc.h"
#include "common/textconsole.h"	// error()

static struct MemoryPool {
	void create() {
		if (base)
			_mspace = create_mspace_with_base((void *)base, size, 0);

		if (_mspace)
			atari_debug("Allocated mspace at 0x%08lx (%ld bytes)", base, size);
		else
			error("mspace allocation failed at 0x%08lx (%ld bytes)", base, size);
	}

	void destroy() {
		if (_mspace) {
			destroy_mspace(_mspace);
			_mspace = nullptr;
		}
	}

	void *malloc(size_t bytes) {
		assert(_mspace);
		return mspace_malloc(_mspace, bytes);
	}

	void *calloc(size_t n_elements, size_t elem_size) {
		assert(_mspace);
		return mspace_calloc(_mspace, n_elements, elem_size);
	}

	void free(void *mem) {
		assert(_mspace);
		mspace_free(_mspace, mem);
	}

	long base;
	long size;

private:
	mspace _mspace;
} s_videoRamPool, s_blitterPool;

static MemoryPool *s_currentPool;

namespace Graphics {

void Surface::create(int16 width, int16 height, const PixelFormat &f) {
	assert(width >= 0 && height >= 0);
	free();

	w = width;
	h = height;
	format = f;
	pitch = w * format.bytesPerPixel;

	if (width && height) {
		if (s_currentPool) {
			pixels = s_currentPool->calloc(height * pitch, format.bytesPerPixel);
			if (!pixels)
				error("Not enough VRAM to allocate a surface");

			if (s_currentPool == &s_blitterPool) {
				assert(pixels >= (void *)0xA1000000);
			} else if (s_currentPool == &s_videoRamPool) {
#ifdef USE_SUPERVIDEL
				if (g_hasSuperVidel)
					assert(pixels >= (void *)0xA0000000 && pixels < (void *)0xA1000000);
				else
#endif
					assert(pixels < (void *)0x01000000);
			}
		} else {
			pixels = ::calloc(height * pitch, format.bytesPerPixel);
			if (!pixels)
				error("Not enough RAM to allocate a surface");
		}

		assert(((uintptr)pixels & (MALLOC_ALIGNMENT - 1)) == 0);
	}
}

void Surface::free() {
	if (pixels) {
		if (s_currentPool)
			s_currentPool->free(pixels);
		else
			::free(pixels);

		pixels = nullptr;
	}

	w = h = pitch = 0;
	format = PixelFormat();
}

} // End of namespace Graphics

///////////////////////////////////////////////////////////////////////////////

AtariSurface::AtariSurface(int16 width, int16 height, const Graphics::PixelFormat &pixelFormat) {
	create(width, height, pixelFormat);
}

AtariSurface::~AtariSurface() {
	free();
}

void AtariSurface::create(int16 width, int16 height, const Graphics::PixelFormat &pixelFormat) {
	MemoryPool *oldPool = s_currentPool;
	s_currentPool = &s_videoRamPool;

	Graphics::ManagedSurface::create(width * (format == PIXELFORMAT_RGB121 ? 4 : 8) / 8, height, pixelFormat);
	w = width;

	s_currentPool = oldPool;
}

void AtariSurface::free() {
	MemoryPool *oldPool = s_currentPool;
	s_currentPool = &s_videoRamPool;

	Graphics::ManagedSurface::free();

	s_currentPool = oldPool;
}

void AtariSurface::copyRectToSurface(const void *buffer, int srcPitch, int destX, int destY, int width, int height) {
	assert(width % 16 == 0);
	assert(destX % 16 == 0);
	assert(format.bytesPerPixel == 1);

	const byte *pChunky    = (const byte *)buffer;
	const byte *pChunkyEnd = pChunky + (height - 1) * srcPitch + width;

	byte *pScreen = (byte *)getBasePtr(0, destY) + destX * getBitsPerPixel()/8;

	if (getBitsPerPixel() == 8) {
		if (srcPitch == width) {
			if (srcPitch == pitch) {
				asm_c2p1x1_8(pChunky, pChunkyEnd, pScreen);
				return;
			} else if (srcPitch == pitch/2) {
				asm_c2p1x1_8_tt(pChunky, pChunkyEnd, pScreen, pitch);
				return;
			}
		}

		asm_c2p1x1_8_rect(
			pChunky, pChunkyEnd,
			width,
			srcPitch,
			pScreen,
			pitch);
	} else {
		if (srcPitch == width && srcPitch/2 == pitch) {
			asm_c2p1x1_4(pChunky, pChunkyEnd, pScreen);
			return;
		}

		asm_c2p1x1_4_rect(
			pChunky, pChunkyEnd,
			width,
			srcPitch,
			pScreen,
			pitch);
	}
}

void AtariSurface::drawMaskedSprite(
	const Graphics::Surface &srcSurface, const Graphics::Surface &srcMask,
	const Graphics::Surface &boundingSurface,
	int destX, int destY,
	const Common::Rect &subRect) {
	assert(srcSurface.w == srcMask.w);
	assert(srcSurface.h == srcMask.h);

	bool skipFirstPix16 = false;
	bool skipLastPix16  = false;

	int srcSurfaceLeft  = 0;
	int srcSurfaceWidth = srcSurface.w;
	int dstSurfaceLeft  = 0;

	if (subRect.left > 0) {
		skipFirstPix16   = true;

		const int offset = subRect.left & (-16);
		srcSurfaceLeft  += offset;
		srcSurfaceWidth -= offset;

		destX            = 16 - (subRect.left & (16-1));
		dstSurfaceLeft  -= 16;
	}

	if (destX + srcSurfaceWidth > boundingSurface.w) {
		skipLastPix16    = true;

		const int offset = (destX + srcSurfaceWidth - boundingSurface.w) & (-16);
		srcSurfaceWidth -= offset;
	}

	assert(srcSurfaceLeft % 16 == 0);
	assert(srcSurfaceWidth % 16 == 0);

	destX += (this->w - boundingSurface.w) / 2;

	if (getBitsPerPixel() == 8) {
		asm_draw_8bpl_sprite(
			(uint16 *)getBasePtr(dstSurfaceLeft, 0),
			(const uint16 *)srcSurface.getBasePtr(srcSurfaceLeft, subRect.top),
			(const uint16 *)srcMask.getBasePtr(srcSurfaceLeft / 8, subRect.top),
			destX, destY,
			pitch, srcSurface.w, srcSurfaceWidth, subRect.height(),
			skipFirstPix16, skipLastPix16);
	} else {
		asm_draw_4bpl_sprite(
			(uint16 *)getBasePtr(dstSurfaceLeft / 2, 0),
			(const uint16 *)srcSurface.getBasePtr(srcSurfaceLeft / 2, subRect.top),
			(const uint16 *)srcMask.getBasePtr(srcSurfaceLeft / 8, subRect.top),
			destX, destY,
			pitch, srcSurface.w / 2, srcSurfaceWidth, subRect.height(),
			skipFirstPix16, skipLastPix16);
	}
}

///////////////////////////////////////////////////////////////////////////////

#ifdef USE_SUPERVIDEL
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
#endif	// USE_SUPERVIDEL

void AtariSurfaceInit() {
#ifdef USE_SUPERVIDEL
	g_hasSuperVidel = Getcookie(C_SupV, NULL) == C_FOUND && VgetMonitor() == MON_VGA;

	if (g_hasSuperVidel) {
#ifdef USE_SV_BLITTER
		g_superVidelFwVersion = *SV_VERSION & 0x01ff;

		atari_debug("SuperVidel FW Revision: %d, using %s", g_superVidelFwVersion,
			g_superVidelFwVersion >= 9 ? "fast async FIFO" : "slower sync blitting");
#else
		atari_debug("SuperVidel FW Revision: %d, SuperBlitter not used", *SV_VERSION & 0x01ff);
#endif
		if (Supexec(hasSvRamBoosted))
			atari_debug("SV_XBIOS has the pmmu boost enabled");
		else
			atari_warning("SV_XBIOS has the pmmu boost disabled, set 'pmmu_boost = true' in C:\\SV.INF");

#ifdef USE_SV_BLITTER
		s_blitterPool.size = ct60_vmalloc(-1) - (16 * 1024 * 1024);	// SV XBIOS seems to forget the initial 16 MB ST RAM mirror
		s_blitterPool.base = s_blitterPool.size > 0 ? ct60_vmalloc(s_blitterPool.size) : 0;
		s_blitterPool.create();
		// default pool is either null or blitter
		s_currentPool = &s_blitterPool;
#endif
	}
#endif	// USE_SUPERVIDEL

	s_videoRamPool.size = 2 * 1024 * 1024;	// allocate 2 MiB, leave the rest for SDMA / Blitter usage
	s_videoRamPool.base = s_videoRamPool.size > 0 ? Mxalloc(s_videoRamPool.size, MX_STRAM) : 0;
#ifdef USE_SUPERVIDEL
	if (g_hasSuperVidel && s_videoRamPool.base)
		s_videoRamPool.base |= 0xA0000000;
#endif
	s_videoRamPool.create();
}

void AtariSurfaceDeinit() {
	s_videoRamPool.destroy();
	if (s_videoRamPool.base) {
#ifdef USE_SUPERVIDEL
		if (g_hasSuperVidel)
			s_videoRamPool.base &= 0x00FFFFFF;
#endif
		Mfree(s_videoRamPool.base);
		s_videoRamPool.base = 0;
		s_videoRamPool.size = 0;
	}

#ifdef USE_SV_BLITTER
	s_blitterPool.destroy();
	if (s_blitterPool.base) {
		ct60_vmfree(s_blitterPool.base);
		s_blitterPool.base = 0;
		s_blitterPool.size = 0;
	}
#endif
}
