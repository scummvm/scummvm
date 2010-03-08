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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "graphics/scaler/intern.h"

#ifdef USE_NASM
// Assembly version of HQ2x

extern "C" {

#if !defined(_WIN32) && !defined(MACOSX) && !defined(__OS2__)
#define hq2x_16 _hq2x_16
#endif

void hq2x_16(const byte *, byte *, uint32, uint32, uint32, uint32);

}

void HQ2x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	hq2x_16(srcPtr, dstPtr, width, height, srcPitch, dstPitch);
}

#else

#define PIXEL00_0	*(q) = w5;
#define PIXEL00_10	*(q) = interpolate16_3_1<ColorMask >(w5, w1);
#define PIXEL00_11	*(q) = interpolate16_3_1<ColorMask >(w5, w4);
#define PIXEL00_12	*(q) = interpolate16_3_1<ColorMask >(w5, w2);
#define PIXEL00_20	*(q) = interpolate16_2_1_1<ColorMask >(w5, w4, w2);
#define PIXEL00_21	*(q) = interpolate16_2_1_1<ColorMask >(w5, w1, w2);
#define PIXEL00_22	*(q) = interpolate16_2_1_1<ColorMask >(w5, w1, w4);
#define PIXEL00_60	*(q) = interpolate16_5_2_1<ColorMask >(w5, w2, w4);
#define PIXEL00_61	*(q) = interpolate16_5_2_1<ColorMask >(w5, w4, w2);
#define PIXEL00_70	*(q) = interpolate16_6_1_1<ColorMask >(w5, w4, w2);
#define PIXEL00_90	*(q) = interpolate16_2_3_3<ColorMask >(w5, w4, w2);
#define PIXEL00_100	*(q) = interpolate16_14_1_1<ColorMask >(w5, w4, w2);

#define PIXEL01_0	*(q+1) = w5;
#define PIXEL01_10	*(q+1) = interpolate16_3_1<ColorMask >(w5, w3);
#define PIXEL01_11	*(q+1) = interpolate16_3_1<ColorMask >(w5, w2);
#define PIXEL01_12	*(q+1) = interpolate16_3_1<ColorMask >(w5, w6);
#define PIXEL01_20	*(q+1) = interpolate16_2_1_1<ColorMask >(w5, w2, w6);
#define PIXEL01_21	*(q+1) = interpolate16_2_1_1<ColorMask >(w5, w3, w6);
#define PIXEL01_22	*(q+1) = interpolate16_2_1_1<ColorMask >(w5, w3, w2);
#define PIXEL01_60	*(q+1) = interpolate16_5_2_1<ColorMask >(w5, w6, w2);
#define PIXEL01_61	*(q+1) = interpolate16_5_2_1<ColorMask >(w5, w2, w6);
#define PIXEL01_70	*(q+1) = interpolate16_6_1_1<ColorMask >(w5, w2, w6);
#define PIXEL01_90	*(q+1) = interpolate16_2_3_3<ColorMask >(w5, w2, w6);
#define PIXEL01_100	*(q+1) = interpolate16_14_1_1<ColorMask >(w5, w2, w6);

#define PIXEL10_0	*(q+nextlineDst) = w5;
#define PIXEL10_10	*(q+nextlineDst) = interpolate16_3_1<ColorMask >(w5, w7);
#define PIXEL10_11	*(q+nextlineDst) = interpolate16_3_1<ColorMask >(w5, w8);
#define PIXEL10_12	*(q+nextlineDst) = interpolate16_3_1<ColorMask >(w5, w4);
#define PIXEL10_20	*(q+nextlineDst) = interpolate16_2_1_1<ColorMask >(w5, w8, w4);
#define PIXEL10_21	*(q+nextlineDst) = interpolate16_2_1_1<ColorMask >(w5, w7, w4);
#define PIXEL10_22	*(q+nextlineDst) = interpolate16_2_1_1<ColorMask >(w5, w7, w8);
#define PIXEL10_60	*(q+nextlineDst) = interpolate16_5_2_1<ColorMask >(w5, w4, w8);
#define PIXEL10_61	*(q+nextlineDst) = interpolate16_5_2_1<ColorMask >(w5, w8, w4);
#define PIXEL10_70	*(q+nextlineDst) = interpolate16_6_1_1<ColorMask >(w5, w8, w4);
#define PIXEL10_90	*(q+nextlineDst) = interpolate16_2_3_3<ColorMask >(w5, w8, w4);
#define PIXEL10_100	*(q+nextlineDst) = interpolate16_14_1_1<ColorMask >(w5, w8, w4);

#define PIXEL11_0	*(q+1+nextlineDst) = w5;
#define PIXEL11_10	*(q+1+nextlineDst) = interpolate16_3_1<ColorMask >(w5, w9);
#define PIXEL11_11	*(q+1+nextlineDst) = interpolate16_3_1<ColorMask >(w5, w6);
#define PIXEL11_12	*(q+1+nextlineDst) = interpolate16_3_1<ColorMask >(w5, w8);
#define PIXEL11_20	*(q+1+nextlineDst) = interpolate16_2_1_1<ColorMask >(w5, w6, w8);
#define PIXEL11_21	*(q+1+nextlineDst) = interpolate16_2_1_1<ColorMask >(w5, w9, w8);
#define PIXEL11_22	*(q+1+nextlineDst) = interpolate16_2_1_1<ColorMask >(w5, w9, w6);
#define PIXEL11_60	*(q+1+nextlineDst) = interpolate16_5_2_1<ColorMask >(w5, w8, w6);
#define PIXEL11_61	*(q+1+nextlineDst) = interpolate16_5_2_1<ColorMask >(w5, w6, w8);
#define PIXEL11_70	*(q+1+nextlineDst) = interpolate16_6_1_1<ColorMask >(w5, w6, w8);
#define PIXEL11_90	*(q+1+nextlineDst) = interpolate16_2_3_3<ColorMask >(w5, w6, w8);
#define PIXEL11_100	*(q+1+nextlineDst) = interpolate16_14_1_1<ColorMask >(w5, w6, w8);

extern "C" uint32   *RGBtoYUV;
#define YUV(x)	RGBtoYUV[w ## x]


#define ColorMask Graphics::ColorMasks<565>
void HQ2x_565(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	#include "graphics/scaler/hq2x.h"
}
#undef ColorMask

#define ColorMask Graphics::ColorMasks<555>
void HQ2x_555(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	#include "graphics/scaler/hq2x.h"
}
#undef ColorMask


void HQ2x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	if (gBitFormat == 565)
		HQ2x_565(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
	else
		HQ2x_555(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
}

#endif // Assembly version
