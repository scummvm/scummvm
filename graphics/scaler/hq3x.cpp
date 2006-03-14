/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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
// Assembly version of HQ3x

extern "C" {

#ifndef _WIN32
#ifndef MACOSX
#define hq3x_16 _hq3x_16
#endif
#endif

void hq3x_16(const byte *, byte *, uint32, uint32, uint32, uint32);

}

void HQ3x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	hq3x_16(srcPtr, dstPtr, width, height, srcPitch, dstPitch);
}

#else

#ifdef HAS_ALTIVEC

#ifdef __amigaos4__
#include <proto/exec.h>
static bool isAltiVecAvailable() {
	uint32 vecUnit;
	IExec->GetCPUInfo(GCIT_VectorUnit, &vecUnit, TAG_DONE);
	if (vecUnit == VECTORTYPE_NONE)
		return false;
	else
		return true;
}
#else

#include <sys/sysctl.h>

static bool isAltiVecAvailable()  {
	int selectors[2] = { CTL_HW, HW_VECTORUNIT };
	int hasVectorUnit = 0;
	size_t length = sizeof(hasVectorUnit);
	int error = sysctl(selectors, 2, &hasVectorUnit, &length, NULL, 0);
	if ( 0 == error )
		return hasVectorUnit != 0;
	return false;
}
#endif
#endif

#define PIXEL00_1M  *(q) = interpolate16_2<bitFormat,3,1>(w5, w1);
#define PIXEL00_1U  *(q) = interpolate16_2<bitFormat,3,1>(w5, w2);
#define PIXEL00_1L  *(q) = interpolate16_2<bitFormat,3,1>(w5, w4);
#define PIXEL00_2   *(q) = interpolate16_3<bitFormat,2,1,1>(w5, w4, w2);
#define PIXEL00_4   *(q) = interpolate16_3<bitFormat,2,7,7>(w5, w4, w2);
#define PIXEL00_5   *(q) = interpolate16_2<bitFormat,1,1>(w4, w2);
#define PIXEL00_C   *(q) = w5;

#define PIXEL01_1   *(q+1) = interpolate16_2<bitFormat,3,1>(w5, w2);
#define PIXEL01_3   *(q+1) = interpolate16_2<bitFormat,7,1>(w5, w2);
#define PIXEL01_6   *(q+1) = interpolate16_2<bitFormat,3,1>(w2, w5);
#define PIXEL01_C   *(q+1) = w5;

#define PIXEL02_1M  *(q+2) = interpolate16_2<bitFormat,3,1>(w5, w3);
#define PIXEL02_1U  *(q+2) = interpolate16_2<bitFormat,3,1>(w5, w2);
#define PIXEL02_1R  *(q+2) = interpolate16_2<bitFormat,3,1>(w5, w6);
#define PIXEL02_2   *(q+2) = interpolate16_3<bitFormat,2,1,1>(w5, w2, w6);
#define PIXEL02_4   *(q+2) = interpolate16_3<bitFormat,2,7,7>(w5, w2, w6);
#define PIXEL02_5   *(q+2) = interpolate16_2<bitFormat,1,1>(w2, w6);
#define PIXEL02_C   *(q+2) = w5;

#define PIXEL10_1   *(q+nextlineDst) = interpolate16_2<bitFormat,3,1>(w5, w4);
#define PIXEL10_3   *(q+nextlineDst) = interpolate16_2<bitFormat,7,1>(w5, w4);
#define PIXEL10_6   *(q+nextlineDst) = interpolate16_2<bitFormat,3,1>(w4, w5);
#define PIXEL10_C   *(q+nextlineDst) = w5;

#define PIXEL11     *(q+1+nextlineDst) = w5;

#define PIXEL12_1   *(q+2+nextlineDst) = interpolate16_2<bitFormat,3,1>(w5, w6);
#define PIXEL12_3   *(q+2+nextlineDst) = interpolate16_2<bitFormat,7,1>(w5, w6);
#define PIXEL12_6   *(q+2+nextlineDst) = interpolate16_2<bitFormat,3,1>(w6, w5);
#define PIXEL12_C   *(q+2+nextlineDst) = w5;

#define PIXEL20_1M  *(q+nextlineDst2) = interpolate16_2<bitFormat,3,1>(w5, w7);
#define PIXEL20_1D  *(q+nextlineDst2) = interpolate16_2<bitFormat,3,1>(w5, w8);
#define PIXEL20_1L  *(q+nextlineDst2) = interpolate16_2<bitFormat,3,1>(w5, w4);
#define PIXEL20_2   *(q+nextlineDst2) = interpolate16_3<bitFormat,2,1,1>(w5, w8, w4);
#define PIXEL20_4   *(q+nextlineDst2) = interpolate16_3<bitFormat,2,7,7>(w5, w8, w4);
#define PIXEL20_5   *(q+nextlineDst2) = interpolate16_2<bitFormat,1,1>(w8, w4);
#define PIXEL20_C   *(q+nextlineDst2) = w5;

#define PIXEL21_1   *(q+1+nextlineDst2) = interpolate16_2<bitFormat,3,1>(w5, w8);
#define PIXEL21_3   *(q+1+nextlineDst2) = interpolate16_2<bitFormat,7,1>(w5, w8);
#define PIXEL21_6   *(q+1+nextlineDst2) = interpolate16_2<bitFormat,3,1>(w8, w5);
#define PIXEL21_C   *(q+1+nextlineDst2) = w5;

#define PIXEL22_1M  *(q+2+nextlineDst2) = interpolate16_2<bitFormat,3,1>(w5, w9);
#define PIXEL22_1D  *(q+2+nextlineDst2) = interpolate16_2<bitFormat,3,1>(w5, w8);
#define PIXEL22_1R  *(q+2+nextlineDst2) = interpolate16_2<bitFormat,3,1>(w5, w6);
#define PIXEL22_2   *(q+2+nextlineDst2) = interpolate16_3<bitFormat,2,1,1>(w5, w6, w8);
#define PIXEL22_4   *(q+2+nextlineDst2) = interpolate16_3<bitFormat,2,7,7>(w5, w6, w8);
#define PIXEL22_5   *(q+2+nextlineDst2) = interpolate16_2<bitFormat,1,1>(w6, w8);
#define PIXEL22_C   *(q+2+nextlineDst2) = w5;

#define YUV(x)	RGBtoYUV[w ## x]


#define bitFormat 565
void HQ3x_565(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	#include "graphics/scaler/hq3x.h"
}
#undef bitFormat

#define bitFormat 555
void HQ3x_555(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	#include "graphics/scaler/hq3x.h"
}
#undef bitFormat


#ifdef HAS_ALTIVEC
	#define USE_ALTIVEC	1

	#define bitFormat 565
	void HQ3x_565_Altivec(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
		#include "graphics/scaler/hq3x.h"
	}
	#undef bitFormat

	#define bitFormat 555
	void HQ3x_555_Altivec(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
		#include "graphics/scaler/hq3x.h"
	}
	#undef bitFormat
#endif

void HQ3x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
#ifdef HAS_ALTIVEC
	if (isAltiVecAvailable()) {
		if (gBitFormat == 565)
			HQ3x_565_Altivec(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
		else
			HQ3x_555_Altivec(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
		return;
	}
#endif

	if (gBitFormat == 565)
		HQ3x_565(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
	else
		HQ3x_555(srcPtr, srcPitch, dstPtr, dstPitch, width, height);
}

#endif
