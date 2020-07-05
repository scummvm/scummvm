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
		if (dst != src) {
			if (dstPitch == srcPitch && ((w * dstFmt.bytesPerPixel) == dstPitch)) {
				memcpy(dst, src, dstPitch * h);
			} else {
				for (uint i = 0; i < h; ++i) {
					memcpy(dst, src, w * dstFmt.bytesPerPixel);
					dst += dstPitch;
					src += srcPitch;
				}
			}
		}

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

The function below is adapted from SDL_rotozoom.c,
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


The functions have been adapted for different structures and coordinate
systems.

*/

struct tColorRGBA { byte r; byte g; byte b; byte a; };

bool scaleBlitBilinear(byte *dst, const byte *src,
                       const uint dstPitch, const uint srcPitch,
                       const uint dstW, const uint dstH,
                       const uint srcW, const uint srcH,
                       const Graphics::PixelFormat &fmt) {
	assert(fmt.bytesPerPixel == 4);

	bool flipx = false, flipy = false; // TODO: See mirroring comment in RenderTicket ctor


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

	const tColorRGBA *sp = (const tColorRGBA *) src;
	tColorRGBA *dp = (tColorRGBA *) dst;
	int spixelgap = srcW;

	if (flipx) {
		sp += spixelw;
	}
	if (flipy) {
		sp += spixelgap * spixelh;
	}

	csay = say;
	for (uint y = 0; y < dstH; y++) {
		const tColorRGBA *csp = sp;
		csax = sax;
		for (uint x = 0; x < dstW; x++) {
			/*
			* Setup color source pointers
			*/
			int ex = (*csax & 0xffff);
			int ey = (*csay & 0xffff);
			int cx = (*csax >> 16);
			int cy = (*csay >> 16);

			const tColorRGBA *c00, *c01, *c10, *c11;
			c00 = sp;
			c01 = sp;
			c10 = sp;
			if (cy < spixelh) {
				if (flipy) {
					c10 -= spixelgap;
				} else {
					c10 += spixelgap;
				}
			}
			c11 = c10;
			if (cx < spixelw) {
				if (flipx) {
					c01--;
					c11--;
				} else {
					c01++;
					c11++;
				}
			}

			/*
			* Draw and interpolate colors
			*/
			int t1, t2;
			t1 = ((((c01->r - c00->r) * ex) >> 16) + c00->r) & 0xff;
			t2 = ((((c11->r - c10->r) * ex) >> 16) + c10->r) & 0xff;
			dp->r = (((t2 - t1) * ey) >> 16) + t1;
			t1 = ((((c01->g - c00->g) * ex) >> 16) + c00->g) & 0xff;
			t2 = ((((c11->g - c10->g) * ex) >> 16) + c10->g) & 0xff;
			dp->g = (((t2 - t1) * ey) >> 16) + t1;
			t1 = ((((c01->b - c00->b) * ex) >> 16) + c00->b) & 0xff;
			t2 = ((((c11->b - c10->b) * ex) >> 16) + c10->b) & 0xff;
			dp->b = (((t2 - t1) * ey) >> 16) + t1;
			t1 = ((((c01->a - c00->a) * ex) >> 16) + c00->a) & 0xff;
			t2 = ((((c11->a - c10->a) * ex) >> 16) + c10->a) & 0xff;
			dp->a = (((t2 - t1) * ey) >> 16) + t1;

			/*
			* Advance source pointer x
			*/
			int *salastx = csax;
			csax++;
			int sstepx = (*csax >> 16) - (*salastx >> 16);
			if (flipx) {
				sp -= sstepx;
			} else {
				sp += sstepx;
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
		sstepy *= spixelgap;
		if (flipy) {
			sp = csp - sstepy;
		} else {
			sp = csp + sstepy;
		}
	}

	delete[] sax;
	delete[] say;

	return true;
}

} // End of namespace Graphics
