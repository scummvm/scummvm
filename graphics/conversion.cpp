/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "graphics/conversion.h"
#include "graphics/pixelformat.h"

#include "common/endian.h"

namespace Graphics {

// TODO: YUV to RGB conversion function

// Function to blit a rect
void copyBlit(byte *dst, const byte *src,
               const uint dstPitch, const uint srcPitch,
               const uint w, const uint h,
               const uint bytesPerPixel) {
	if (dst == src)
		return;

	if (dstPitch == srcPitch && ((w * bytesPerPixel) == dstPitch)) {
		memcpy(dst, src, dstPitch * h);
	} else {
		for (uint i = 0; i < h; ++i) {
			memcpy(dst, src, w * bytesPerPixel);
			dst += dstPitch;
			src += srcPitch;
		}
	}
}

namespace {

template<typename SrcColor, typename DstColor, bool backward>
inline void crossBlitLogic(byte *dst, const byte *src, const uint w, const uint h,
                           const PixelFormat &srcFmt, const PixelFormat &dstFmt,
                           const uint srcDelta, const uint dstDelta) {
	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
			const uint32 color = *(const SrcColor *)src;
			byte a, r, g, b;
			srcFmt.colorToARGB(color, a, r, g, b);
			*(DstColor *)dst = dstFmt.ARGBToColor(a, r, g, b);

			if (backward) {
				src -= sizeof(SrcColor);
				dst -= sizeof(DstColor);
			} else {
				src += sizeof(SrcColor);
				dst += sizeof(DstColor);
			}
		}

		if (backward) {
			src -= srcDelta;
			dst -= dstDelta;
		} else {
			src += srcDelta;
			dst += dstDelta;
		}
	}
}

template<typename DstColor, bool backward>
inline void crossBlitLogic3BppSource(byte *dst, const byte *src, const uint w, const uint h,
                                     const PixelFormat &srcFmt, const PixelFormat &dstFmt,
                                     const uint srcDelta, const uint dstDelta) {
	uint32 color;
	byte r, g, b, a;
	uint8 *col = (uint8 *)&color;
#ifdef SCUMM_BIG_ENDIAN
	col++;
#endif
	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
			memcpy(col, src, 3);
			srcFmt.colorToARGB(color, a, r, g, b);
			*(DstColor *)dst = dstFmt.ARGBToColor(a, r, g, b);

			if (backward) {
				src -= 3;
				dst -= sizeof(DstColor);
			} else {
				src += 3;
				dst += sizeof(DstColor);
			}
		}

		if (backward) {
			src -= srcDelta;
			dst -= dstDelta;
		} else {
			src += srcDelta;
			dst += dstDelta;
		}
	}
}

} // End of anonymous namespace

// Function to blit a rect from one color format to another
bool crossBlit(byte *dst, const byte *src,
               const uint dstPitch, const uint srcPitch,
               const uint w, const uint h,
               const Graphics::PixelFormat &dstFmt, const Graphics::PixelFormat &srcFmt) {
	// Error out if conversion is impossible
	if ((srcFmt.bytesPerPixel == 1) || (dstFmt.bytesPerPixel == 1)
			 || (dstFmt.bytesPerPixel == 3)
			 || (!srcFmt.bytesPerPixel) || (!dstFmt.bytesPerPixel))
		return false;

	// Don't perform unnecessary conversion
	if (srcFmt == dstFmt) {
		copyBlit(dst, src, dstPitch, srcPitch, w, h, dstFmt.bytesPerPixel);
		return true;
	}

	// Faster, but larger, to provide optimized handling for each case.
	const uint srcDelta = (srcPitch - w * srcFmt.bytesPerPixel);
	const uint dstDelta = (dstPitch - w * dstFmt.bytesPerPixel);

	// TODO: optimized cases for dstDelta of 0
	if (dstFmt.bytesPerPixel == 2) {
		if (srcFmt.bytesPerPixel == 2) {
			crossBlitLogic<uint16, uint16, false>(dst, src, w, h, srcFmt, dstFmt, srcDelta, dstDelta);
		} else if (srcFmt.bytesPerPixel == 3) {
			crossBlitLogic3BppSource<uint16, false>(dst, src, w, h, srcFmt, dstFmt, srcDelta, dstDelta);
		} else {
			crossBlitLogic<uint32, uint16, false>(dst, src, w, h, srcFmt, dstFmt, srcDelta, dstDelta);
		}
	} else if (dstFmt.bytesPerPixel == 4) {
		if (srcFmt.bytesPerPixel == 2) {
			// We need to blit the surface from bottom right to top left here.
			// This is neeeded, because when we convert to the same memory
			// buffer copying the surface from top left to bottom right would
			// overwrite the source, since we have more bits per destination
			// color than per source color.
			dst += h * dstPitch - dstDelta - dstFmt.bytesPerPixel;
			src += h * srcPitch - srcDelta - srcFmt.bytesPerPixel;
			crossBlitLogic<uint16, uint32, true>(dst, src, w, h, srcFmt, dstFmt, srcDelta, dstDelta);
		} else if (srcFmt.bytesPerPixel == 3) {
			// We need to blit the surface from bottom right to top left here.
			// This is neeeded, because when we convert to the same memory
			// buffer copying the surface from top left to bottom right would
			// overwrite the source, since we have more bits per destination
			// color than per source color.
			dst += h * dstPitch - dstDelta - dstFmt.bytesPerPixel;
			src += h * srcPitch - srcDelta - srcFmt.bytesPerPixel;
			crossBlitLogic3BppSource<uint32, true>(dst, src, w, h, srcFmt, dstFmt, srcDelta, dstDelta);
		} else {
			crossBlitLogic<uint32, uint32, false>(dst, src, w, h, srcFmt, dstFmt, srcDelta, dstDelta);
		}
	} else {
		return false;
	}
	return true;
}

namespace {

template <typename Size>
void scaleNN(byte *dst, const byte *src,
               const uint dstPitch, const uint srcPitch,
               const uint dstW, const uint dstH,
               const uint srcW, const uint srcH,
               int *scaleCacheX) {

	const uint dstDelta = (dstPitch - dstW * sizeof(Size));

	for (uint y = 0; y < dstH; y++) {
		const Size *srcP = (const Size *)(src + ((y * srcH) / dstH) * srcPitch);
		for (uint x = 0; x < dstW; x++) {
			int val = srcP[scaleCacheX[x]];
			*(Size *)dst = val;
			dst += sizeof(Size);
		}
		dst += dstDelta;
	}
}

} // End of anonymous namespace

bool scaleBlit(byte *dst, const byte *src,
               const uint dstPitch, const uint srcPitch,
               const uint dstW, const uint dstH,
               const uint srcW, const uint srcH,
               const Graphics::PixelFormat &fmt) {

	int *scaleCacheX = new int[dstW];
	for (uint x = 0; x < dstW; x++) {
		scaleCacheX[x] = (x * srcW) / dstW;
	}

	switch (fmt.bytesPerPixel) {
	case 1:
		scaleNN<uint8>(dst, src, dstPitch, srcPitch, dstW,  dstH, srcW, srcH, scaleCacheX);
		break;
	case 2:
		scaleNN<uint16>(dst, src, dstPitch, srcPitch, dstW,  dstH, srcW, srcH, scaleCacheX);
		break;
	case 4:
		scaleNN<uint32>(dst, src, dstPitch, srcPitch, dstW,  dstH, srcW, srcH, scaleCacheX);
		break;
	default:
		delete[] scaleCacheX;
		return false;
	}

	delete[] scaleCacheX;

	return true;
}

/*

The functions below are adapted from SDL_rotozoom.c,
taken from SDL_gfx-2.0.18.

Its copyright notice:

=============================================================================
SDL_rotozoom.c: rotozoomer, zoomer and shrinker for 32bit or 8bit surfaces

Copyright (C) 2001-2012  Andreas Schiffler

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.

Andreas Schiffler -- aschiffler at ferzkopp dot net
=============================================================================


The functions have been adapted for different structures, coordinate
systems and pixel formats.

*/

namespace {

inline byte scaleBlitBilinearInterpolate(byte c01, byte c00, byte c11, byte c10, int ex, int ey) {
	int t1 = ((((c01 - c00) * ex) >> 16) + c00) & 0xff;
	int t2 = ((((c11 - c10) * ex) >> 16) + c10) & 0xff;
	return (((t2 - t1) * ey) >> 16) + t1;
}

template <typename Size>
Size scaleBlitBilinearInterpolate(Size c01, Size c00, Size c11, Size c10, int ex, int ey,
                                  const Graphics::PixelFormat &fmt) {
	byte c01_a, c01_r, c01_g, c01_b;
	fmt.colorToARGB(c01, c01_a, c01_r, c01_g, c01_b);

	byte c00_a, c00_r, c00_g, c00_b;
	fmt.colorToARGB(c00, c00_a, c00_r, c00_g, c00_b);

	byte c11_a, c11_r, c11_g, c11_b;
	fmt.colorToARGB(c11, c11_a, c11_r, c11_g, c11_b);

	byte c10_a, c10_r, c10_g, c10_b;
	fmt.colorToARGB(c10, c10_a, c10_r, c10_g, c10_b);

	byte dp_a = scaleBlitBilinearInterpolate(c01_a, c00_a, c11_a, c10_a, ex, ey);
	byte dp_r = scaleBlitBilinearInterpolate(c01_r, c00_r, c11_r, c10_r, ex, ey);
	byte dp_g = scaleBlitBilinearInterpolate(c01_g, c00_g, c11_g, c10_g, ex, ey);
	byte dp_b = scaleBlitBilinearInterpolate(c01_b, c00_b, c11_b, c10_b, ex, ey);
	return fmt.ARGBToColor(dp_a, dp_r, dp_g, dp_b);
}

template <typename Size, bool flipx, bool flipy> // TODO: See mirroring comment in RenderTicket ctor
void scaleBlitBilinearLogic(byte *dst, const byte *src,
                            const uint dstPitch, const uint srcPitch,
                            const uint dstW, const uint dstH,
							const uint srcW, const uint srcH,
                            const Graphics::PixelFormat &fmt,
                            int *sax, int *say) {

	int spixelw = (srcW - 1);
	int spixelh = (srcH - 1);

	const byte *sp = src;

	if (flipx) {
		sp += spixelw;
	}
	if (flipy) {
		sp += srcPitch * spixelh;
	}

	int *csay = say;
	for (uint y = 0; y < dstH; y++) {
		Size *dp = (Size *)(dst + (dstPitch * y));
		const byte *csp = sp;
		int *csax = sax;
		for (uint x = 0; x < dstW; x++) {
			/*
			* Setup color source pointers
			*/
			int ex = (*csax & 0xffff);
			int ey = (*csay & 0xffff);
			int cx = (*csax >> 16);
			int cy = (*csay >> 16);

			const byte *c00, *c01, *c10, *c11;
			c00 = c01 = c10 = sp;
			if (cy < spixelh) {
				if (flipy) {
					c10 -= srcPitch;
				} else {
					c10 += srcPitch;
				}
			}
			c11 = c10;
			if (cx < spixelw) {
				if (flipx) {
					c01 -= sizeof(Size);
					c11 -= sizeof(Size);
				} else {
					c01 += sizeof(Size);
					c11 += sizeof(Size);
				}
			}

			/*
			* Draw and interpolate colors
			*/
			*dp = scaleBlitBilinearInterpolate(*(const Size *)c01, *(const Size *)c00, *(const Size *)c11, *(const Size *)c10, ex, ey, fmt);
			/*
			* Advance source pointer x
			*/
			int *salastx = csax;
			csax++;
			int sstepx = (*csax >> 16) - (*salastx >> 16);
			if (flipx) {
				sp -= sstepx * sizeof(Size);
			} else {
				sp += sstepx * sizeof(Size);
			}

			/*
			* Advance destination pointer x
			*/
			dp++;
		}
		/*
		* Advance source pointer y
		*/
		int *salasty = csay;
		csay++;
		int sstepy = (*csay >> 16) - (*salasty >> 16);
		sstepy *= srcPitch;
		if (flipy) {
			sp = csp - sstepy;
		} else {
			sp = csp + sstepy;
		}
	}
}

} // End of anonymous namespace

bool scaleBlitBilinear(byte *dst, const byte *src,
                       const uint dstPitch, const uint srcPitch,
                       const uint dstW, const uint dstH,
                       const uint srcW, const uint srcH,
                       const Graphics::PixelFormat &fmt) {
	if (fmt.bytesPerPixel != 2 && fmt.bytesPerPixel != 4)
		return false;

	int *sax = new int[dstW + 1];
	int *say = new int[dstH + 1];
	assert(sax && say);

	/*
	* Precalculate row increments
	*/
	int spixelw = (srcW - 1);
	int spixelh = (srcH - 1);
	int sx = (int)(65536.0f * (float) spixelw / (float) (dstW - 1));
	int sy = (int)(65536.0f * (float) spixelh / (float) (dstH - 1));

	/* Maximum scaled source size */
	int ssx = (srcW << 16) - 1;
	int ssy = (srcH << 16) - 1;

	/* Precalculate horizontal row increments */
	int csx = 0;
	int *csax = sax;
	for (uint x = 0; x <= dstW; x++) {
		*csax = csx;
		csax++;
		csx += sx;

		/* Guard from overflows */
		if (csx > ssx) {
			csx = ssx;
		}
	}

	/* Precalculate vertical row increments */
	int csy = 0;
	int *csay = say;
	for (uint y = 0; y <= dstH; y++) {
		*csay = csy;
		csay++;
		csy += sy;

		/* Guard from overflows */
		if (csy > ssy) {
			csy = ssy;
		}
	}

	if (fmt.bytesPerPixel == 4) {
		scaleBlitBilinearLogic<uint32, false, false>(dst, src, dstPitch, srcPitch, dstW, dstH, srcW, srcH, fmt, sax, say);
	} else if (fmt.bytesPerPixel == 2) {
		scaleBlitBilinearLogic<uint16, false, false>(dst, src, dstPitch, srcPitch, dstW, dstH, srcW, srcH, fmt, sax, say);
	} else {
		delete[] sax;
		delete[] say;

		return false;
	}

	delete[] sax;
	delete[] say;

	return true;
}

} // End of namespace Graphics
