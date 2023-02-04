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
 *
 * The bottom part of this is file is adapted from SDL_rotozoom.c. The
 * relevant copyright notice for those specific functions can be found at the
 * top of that section.
 *
 */

#include "graphics/blit.h"
#include "graphics/pixelformat.h"
#include "graphics/transform_struct.h"

#include "common/math.h"
#include "common/rect.h"

namespace Graphics {

namespace {

template <typename Size>
void scaleNN(byte *dst, const byte *src,
			   const uint dstPitch, const uint srcPitch,
			   const uint dstW, const uint dstH,
			   const uint srcW, const uint srcH,
			   int *scaleCacheX, const byte flip) {
	const bool flipx = flip & FLIP_H;
	const bool flipy = flip & FLIP_V;

	const int dstIncX = (flipx ? -1 : 1);
	const int dstIncY = (flipy ? -dstPitch : dstPitch);

	if (flipx) {
		dst += (dstW - 1) * sizeof(Size);
	}

	if (flipy) {
		dst += (dstH - 1) * dstPitch;
	}

	for (uint y = 0; y < dstH; y++) {
		const Size *srcP = (const Size *)(src + ((y * srcH) / dstH) * srcPitch);
		Size *dst1 = (Size *)dst;
		for (uint x = 0; x < dstW; x++) {
			int val = srcP[scaleCacheX[x]];
			*dst1 = val;
			dst1 += dstIncX;
		}
		dst += dstIncY;
	}
}

} // End of anonymous namespace

bool scaleBlit(byte *dst, const byte *src,
			   const uint dstPitch, const uint srcPitch,
			   const uint dstW, const uint dstH,
			   const uint srcW, const uint srcH,
			   const Graphics::PixelFormat &fmt,
                           const byte flip) {

	int *scaleCacheX = new int[dstW];
	for (uint x = 0; x < dstW; x++) {
		scaleCacheX[x] = (x * srcW) / dstW;
	}

	switch (fmt.bytesPerPixel) {
	case 1:
		scaleNN<uint8>(dst, src, dstPitch, srcPitch, dstW,  dstH, srcW, srcH, scaleCacheX, flip);
		break;
	case 2:
		scaleNN<uint16>(dst, src, dstPitch, srcPitch, dstW,  dstH, srcW, srcH, scaleCacheX, flip);
		break;
	case 4:
		scaleNN<uint32>(dst, src, dstPitch, srcPitch, dstW,  dstH, srcW, srcH, scaleCacheX, flip);
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

template <typename ColorMask, typename Size>
Size scaleBlitBilinearInterpolate(Size c01, Size c00, Size c11, Size c10, int ex, int ey,
								  const Graphics::PixelFormat &fmt) {
	byte c01_a, c01_r, c01_g, c01_b;
	fmt.colorToARGBT<ColorMask>(c01, c01_a, c01_r, c01_g, c01_b);

	byte c00_a, c00_r, c00_g, c00_b;
	fmt.colorToARGBT<ColorMask>(c00, c00_a, c00_r, c00_g, c00_b);

	byte c11_a, c11_r, c11_g, c11_b;
	fmt.colorToARGBT<ColorMask>(c11, c11_a, c11_r, c11_g, c11_b);

	byte c10_a, c10_r, c10_g, c10_b;
	fmt.colorToARGBT<ColorMask>(c10, c10_a, c10_r, c10_g, c10_b);

	byte dp_a = scaleBlitBilinearInterpolate(c01_a, c00_a, c11_a, c10_a, ex, ey);
	byte dp_r = scaleBlitBilinearInterpolate(c01_r, c00_r, c11_r, c10_r, ex, ey);
	byte dp_g = scaleBlitBilinearInterpolate(c01_g, c00_g, c11_g, c10_g, ex, ey);
	byte dp_b = scaleBlitBilinearInterpolate(c01_b, c00_b, c11_b, c10_b, ex, ey);
	return fmt.ARGBToColorT<ColorMask>(dp_a, dp_r, dp_g, dp_b);
}

template <typename ColorMask, typename Size>
void scaleBlitBilinearLogic(byte *dst, const byte *src,
							const uint dstPitch, const uint srcPitch,
							const uint dstW, const uint dstH,
							const uint srcW, const uint srcH,
							const Graphics::PixelFormat &fmt,
							int *sax, int *say, byte flip) {
	const bool flipx = flip & FLIP_H;
	const bool flipy = flip & FLIP_V;

	int spixelw = (srcW - 1);
	int spixelh = (srcH - 1);

	const byte *sp = src;

	if (flipx) {
		sp += spixelw * sizeof(Size);
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
			*dp = scaleBlitBilinearInterpolate<ColorMask, Size>(*(const Size *)c01, *(const Size *)c00, *(const Size *)c11, *(const Size *)c10, ex, ey, fmt);
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

template<typename ColorMask, typename Size, bool filtering>
void rotoscaleBlitLogic(byte *dst, const byte *src,
						const uint dstPitch, const uint srcPitch,
						const uint dstW, const uint dstH,
						const uint srcW, const uint srcH,
						const Graphics::PixelFormat &fmt,
						const TransformStruct &transform,
						const Common::Point &newHotspot) {
	const bool flipx = transform._flip & FLIP_H;
	const bool flipy = transform._flip & FLIP_V;

	assert(transform._angle != kDefaultAngle); // This would not be ideal; rotoscale() should never be called in conditional branches where angle = 0 anyway.

	if (transform._zoom.x == 0 || transform._zoom.y == 0) {
		return;
	}

	uint32 invAngle = 360 - (transform._angle % 360);
	float invAngleRad = Common::deg2rad<uint32,float>(invAngle);
	float invCos = cos(invAngleRad);
	float invSin = sin(invAngleRad);

	int icosx = (int)(invCos * (65536.0f * kDefaultZoomX / transform._zoom.x));
	int isinx = (int)(invSin * (65536.0f * kDefaultZoomX / transform._zoom.x));
	int icosy = (int)(invCos * (65536.0f * kDefaultZoomY / transform._zoom.y));
	int isiny = (int)(invSin * (65536.0f * kDefaultZoomY / transform._zoom.y));

	int xd = transform._hotspot.x << 16;
	int yd = transform._hotspot.y << 16;
	int cx = newHotspot.x;
	int cy = newHotspot.y;

	int ax = -icosx * cx;
	int ay = -isiny * cx;
	int sw = srcW - 1;
	int sh = srcH - 1;

	Size *pc = (Size *)dst;

	for (uint y = 0; y < dstH; y++) {
		int t = cy - y;
		int sdx = ax + (isinx * t) + xd;
		int sdy = ay - (icosy * t) + yd;
		for (uint x = 0; x < dstW; x++) {
			int dx = (sdx >> 16);
			int dy = (sdy >> 16);
			if (flipx) {
				dx = sw - dx;
			}
			if (flipy) {
				dy = sh - dy;
			}

			if (filtering) {
				if ((dx > -1) && (dy > -1) && (dx < sw) && (dy < sh)) {
					const byte *sp = src + dy * srcPitch + dx * sizeof(Size);
					Size c00, c01, c10, c11;
					c00 = *(const Size *)sp;
					sp += sizeof(Size);
					c01 = *(const Size *)sp;
					sp += srcPitch;
					c11 = *(const Size *)sp;
					sp -= sizeof(Size);
					c10 = *(const Size *)sp;
					if (flipx) {
						SWAP(c00, c01);
						SWAP(c10, c11);
					}
					if (flipy) {
						SWAP(c00, c10);
						SWAP(c01, c11);
					}
					/*
					* Interpolate colors
					*/
					int ex = (sdx & 0xffff);
					int ey = (sdy & 0xffff);
					*pc = scaleBlitBilinearInterpolate<ColorMask, Size>(c01, c00, c11, c10, ex, ey, fmt);
				}
			} else {
				if ((dx >= 0) && (dy >= 0) && (dx < (int)srcW) && (dy < (int)srcH)) {
					const byte *sp = src + dy * srcPitch + dx * sizeof(Size);
					*pc = *(const Size *)sp;
				}
			}
			sdx += icosx;
			sdy += isiny;
			pc++;
		}
	}
}

} // End of anonymous namespace

bool scaleBlitBilinear(byte *dst, const byte *src,
					   const uint dstPitch, const uint srcPitch,
					   const uint dstW, const uint dstH,
					   const uint srcW, const uint srcH,
					   const Graphics::PixelFormat &fmt,
					   const byte flip) {
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

	if (fmt == createPixelFormat<8888>()) {
		scaleBlitBilinearLogic<ColorMasks<8888>, uint32>(dst, src, dstPitch, srcPitch, dstW, dstH, srcW, srcH, fmt, sax, say, flip);
	} else if (fmt == createPixelFormat<888>()) {
		scaleBlitBilinearLogic<ColorMasks<888>,  uint32>(dst, src, dstPitch, srcPitch, dstW, dstH, srcW, srcH, fmt, sax, say, flip);
	} else if (fmt == createPixelFormat<565>()) {
		scaleBlitBilinearLogic<ColorMasks<565>,  uint16>(dst, src, dstPitch, srcPitch, dstW, dstH, srcW, srcH, fmt, sax, say, flip);
	} else if (fmt == createPixelFormat<555>()) {
		scaleBlitBilinearLogic<ColorMasks<555>,  uint16>(dst, src, dstPitch, srcPitch, dstW, dstH, srcW, srcH, fmt, sax, say, flip);

	} else if (fmt.bytesPerPixel == 4) {
		scaleBlitBilinearLogic<ColorMasks<0>,    uint32>(dst, src, dstPitch, srcPitch, dstW, dstH, srcW, srcH, fmt, sax, say, flip);
	} else if (fmt.bytesPerPixel == 2) {
		scaleBlitBilinearLogic<ColorMasks<0>,    uint16>(dst, src, dstPitch, srcPitch, dstW, dstH, srcW, srcH, fmt, sax, say, flip);
	} else {
		delete[] sax;
		delete[] say;

		return false;
	}

	delete[] sax;
	delete[] say;

	return true;
}

bool rotoscaleBlit(byte *dst, const byte *src,
				   const uint dstPitch, const uint srcPitch,
				   const uint dstW, const uint dstH,
				   const uint srcW, const uint srcH,
				   const Graphics::PixelFormat &fmt,
				   const TransformStruct &transform,
				   const Common::Point &newHotspot) {
	if (fmt.bytesPerPixel == 4) {
		rotoscaleBlitLogic<ColorMasks<0>, uint32, false>(dst, src, dstPitch, srcPitch, dstW, dstH, srcW, srcH, fmt, transform, newHotspot);
	} else if (fmt.bytesPerPixel == 2) {
		rotoscaleBlitLogic<ColorMasks<0>, uint16, false>(dst, src, dstPitch, srcPitch, dstW, dstH, srcW, srcH, fmt, transform, newHotspot);
	} else if (fmt.bytesPerPixel == 1) {
		rotoscaleBlitLogic<ColorMasks<0>, uint8, false>(dst, src, dstPitch, srcPitch, dstW, dstH, srcW, srcH, fmt, transform, newHotspot);
	} else {
		return false;
	}

	return true;
}

bool rotoscaleBlitBilinear(byte *dst, const byte *src,
						   const uint dstPitch, const uint srcPitch,
						   const uint dstW, const uint dstH,
						   const uint srcW, const uint srcH,
						   const Graphics::PixelFormat &fmt,
						   const TransformStruct &transform,
						   const Common::Point &newHotspot) {
	if (fmt == createPixelFormat<8888>()) {
		rotoscaleBlitLogic<ColorMasks<8888>, uint32, true>(dst, src, dstPitch, srcPitch, dstW, dstH, srcW, srcH, fmt, transform, newHotspot);
	} else if (fmt == createPixelFormat<888>()) {
		rotoscaleBlitLogic<ColorMasks<888>,  uint32, true>(dst, src, dstPitch, srcPitch, dstW, dstH, srcW, srcH, fmt, transform, newHotspot);
	} else if (fmt == createPixelFormat<565>()) {
		rotoscaleBlitLogic<ColorMasks<565>,  uint16, true>(dst, src, dstPitch, srcPitch, dstW, dstH, srcW, srcH, fmt, transform, newHotspot);
	} else if (fmt == createPixelFormat<555>()) {
		rotoscaleBlitLogic<ColorMasks<555>,  uint16, true>(dst, src, dstPitch, srcPitch, dstW, dstH, srcW, srcH, fmt, transform, newHotspot);

	} else if (fmt.bytesPerPixel == 4) {
		rotoscaleBlitLogic<ColorMasks<0>,    uint32, true>(dst, src, dstPitch, srcPitch, dstW, dstH, srcW, srcH, fmt, transform, newHotspot);
	} else if (fmt.bytesPerPixel == 2) {
		rotoscaleBlitLogic<ColorMasks<0>,    uint16, true>(dst, src, dstPitch, srcPitch, dstW, dstH, srcW, srcH, fmt, transform, newHotspot);
	} else {
		return false;
	}

	return true;
}

} // End of namespace Graphics
