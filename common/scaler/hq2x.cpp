/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "common/scaler/intern.h"

#define PIXEL00_0	*(q) = w5;
#define PIXEL00_10	*(q) = interpolate16_2<bitFormat,3,1>(w5, w1);
#define PIXEL00_11	*(q) = interpolate16_2<bitFormat,3,1>(w5, w4);
#define PIXEL00_12	*(q) = interpolate16_2<bitFormat,3,1>(w5, w2);
#define PIXEL00_20	*(q) = interpolate16_3<bitFormat,2,1,1>(w5, w4, w2);
#define PIXEL00_21	*(q) = interpolate16_3<bitFormat,2,1,1>(w5, w1, w2);
#define PIXEL00_22	*(q) = interpolate16_3<bitFormat,2,1,1>(w5, w1, w4);
#define PIXEL00_60	*(q) = interpolate16_3<bitFormat,5,2,1>(w5, w2, w4);
#define PIXEL00_61	*(q) = interpolate16_3<bitFormat,5,2,1>(w5, w4, w2);
#define PIXEL00_70	*(q) = interpolate16_3<bitFormat,6,1,1>(w5, w4, w2);
#define PIXEL00_90	*(q) = interpolate16_3<bitFormat,2,3,3>(w5, w4, w2);
#define PIXEL00_100	*(q) = interpolate16_3<bitFormat,14,1,1>(w5, w4, w2);

#define PIXEL01_0	*(q+1) = w5;
#define PIXEL01_10	*(q+1) = interpolate16_2<bitFormat,3,1>(w5, w3);
#define PIXEL01_11	*(q+1) = interpolate16_2<bitFormat,3,1>(w5, w2);
#define PIXEL01_12	*(q+1) = interpolate16_2<bitFormat,3,1>(w5, w6);
#define PIXEL01_20	*(q+1) = interpolate16_3<bitFormat,2,1,1>(w5, w2, w6);
#define PIXEL01_21	*(q+1) = interpolate16_3<bitFormat,2,1,1>(w5, w3, w6);
#define PIXEL01_22	*(q+1) = interpolate16_3<bitFormat,2,1,1>(w5, w3, w2);
#define PIXEL01_60	*(q+1) = interpolate16_3<bitFormat,5,2,1>(w5, w6, w2);
#define PIXEL01_61	*(q+1) = interpolate16_3<bitFormat,5,2,1>(w5, w2, w6);
#define PIXEL01_70	*(q+1) = interpolate16_3<bitFormat,6,1,1>(w5, w2, w6);
#define PIXEL01_90	*(q+1) = interpolate16_3<bitFormat,2,3,3>(w5, w2, w6);
#define PIXEL01_100	*(q+1) = interpolate16_3<bitFormat,14,1,1>(w5, w2, w6);

#define PIXEL10_0	*(q+nextlineDst) = w5;
#define PIXEL10_10	*(q+nextlineDst) = interpolate16_2<bitFormat,3,1>(w5, w7);
#define PIXEL10_11	*(q+nextlineDst) = interpolate16_2<bitFormat,3,1>(w5, w8);
#define PIXEL10_12	*(q+nextlineDst) = interpolate16_2<bitFormat,3,1>(w5, w4);
#define PIXEL10_20	*(q+nextlineDst) = interpolate16_3<bitFormat,2,1,1>(w5, w8, w4);
#define PIXEL10_21	*(q+nextlineDst) = interpolate16_3<bitFormat,2,1,1>(w5, w7, w4);
#define PIXEL10_22	*(q+nextlineDst) = interpolate16_3<bitFormat,2,1,1>(w5, w7, w8);
#define PIXEL10_60	*(q+nextlineDst) = interpolate16_3<bitFormat,5,2,1>(w5, w4, w8);
#define PIXEL10_61	*(q+nextlineDst) = interpolate16_3<bitFormat,5,2,1>(w5, w8, w4);
#define PIXEL10_70	*(q+nextlineDst) = interpolate16_3<bitFormat,6,1,1>(w5, w8, w4);
#define PIXEL10_90	*(q+nextlineDst) = interpolate16_3<bitFormat,2,3,3>(w5, w8, w4);
#define PIXEL10_100	*(q+nextlineDst) = interpolate16_3<bitFormat,14,1,1>(w5, w8, w4);

#define PIXEL11_0	*(q+1+nextlineDst) = w5;
#define PIXEL11_10	*(q+1+nextlineDst) = interpolate16_2<bitFormat,3,1>(w5, w9);
#define PIXEL11_11	*(q+1+nextlineDst) = interpolate16_2<bitFormat,3,1>(w5, w6);
#define PIXEL11_12	*(q+1+nextlineDst) = interpolate16_2<bitFormat,3,1>(w5, w8);
#define PIXEL11_20	*(q+1+nextlineDst) = interpolate16_3<bitFormat,2,1,1>(w5, w6, w8);
#define PIXEL11_21	*(q+1+nextlineDst) = interpolate16_3<bitFormat,2,1,1>(w5, w9, w8);
#define PIXEL11_22	*(q+1+nextlineDst) = interpolate16_3<bitFormat,2,1,1>(w5, w9, w6);
#define PIXEL11_60	*(q+1+nextlineDst) = interpolate16_3<bitFormat,5,2,1>(w5, w8, w6);
#define PIXEL11_61	*(q+1+nextlineDst) = interpolate16_3<bitFormat,5,2,1>(w5, w6, w8);
#define PIXEL11_70	*(q+1+nextlineDst) = interpolate16_3<bitFormat,6,1,1>(w5, w6, w8);
#define PIXEL11_90	*(q+1+nextlineDst) = interpolate16_3<bitFormat,2,3,3>(w5, w6, w8);
#define PIXEL11_100	*(q+1+nextlineDst) = interpolate16_3<bitFormat,14,1,1>(w5, w6, w8);

#define YUV(x)	RGBtoYUV[w ## x]

/**
 * The HQ2x high quality 2x graphics filter.
 * Original author Maxim Stepin (see http://www.hiend3d.com/hq2x.html).
 * Adapted for ScummVM to 16 bit output and optimized by Max Horn.
 */
template<int bitFormat>
void HQ2xTemplate(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	register int w1, w2, w3, w4, w5, w6, w7, w8, w9;

	const uint32 nextlineSrc = srcPitch / sizeof(uint16);
	const uint16 *p = (const uint16 *)srcPtr;

	const uint32 nextlineDst = dstPitch / sizeof(uint16);
	uint16 *q = (uint16 *)dstPtr;
	
	//	 +----+----+----+
	//	 |    |    |    |
	//	 | w1 | w2 | w3 |
	//	 +----+----+----+
	//	 |    |    |    |
	//	 | w4 | w5 | w6 |
	//	 +----+----+----+
	//	 |    |    |    |
	//	 | w7 | w8 | w9 |
	//	 +----+----+----+

#if USE_ALTIVEC
	// TODO:
	// * come up with a plan that allows the AltiVec/MMX/SSE/other asm to be
	//   compiled *in addition to* the C++ code. This is necessary since e.g.
	//   not all PowerPC processors support AltiVec, and when run on those,
	//   ScummVM should fallback to the "plain" scalers.
	//   This "switch" could be done in the wrapper HQ2x method (just like it
	//   also switches between 555 and 565 there).
	// * add code to configure which detects whether AltiVec/MMX asm may be
	//   compiled in at all (and also add explicit --disable-asm option)
	// * ...

	// The YUV threshold.
	static const vector unsigned char vThreshold = (vector unsigned char)((vector unsigned int)0x00300706);

	// Bit pattern mask.
	static const vector signed int vPatternMask1 = (vector signed int)(0x01, 0x02, 0x04, 0x08);
	static const vector signed int vPatternMask2 = (vector signed int)(0x10, 0x20, 0x40, 0x80);

	// Permutation masks for the incremental vector loading (see below for more information).
	static const vector unsigned char vPermuteToV1234 = (vector unsigned char)( 4, 5, 6, 7,  8,9,10,11,  20,21,22,23,  16,17,18,19);
	static const vector unsigned char vPermuteToV6789 = (vector unsigned char)(24,25,26,27,  8,9,10,11,  12,13,14,15,  28,29,30,31);

	// The YUV vectors.
	vector signed char vecYUV5555;
	vector signed char vecYUV1234;
	vector signed char vecYUV6789;
#endif

	while (height--) {
		w1 = *(p - 1 - nextlineSrc);
		w4 = *(p - 1);
		w7 = *(p - 1 + nextlineSrc);

		w2 = *(p - nextlineSrc);
		w5 = *(p);
		w8 = *(p + nextlineSrc);

#if USE_ALTIVEC
		// Load inital values of vecYUV1234 / vecYUV6789
		const int arr1234[4] = {0, YUV(1), YUV(2), 0};
		const int arr6789[4] = {YUV(5), 0, YUV(7), YUV(8)};

		vecYUV1234 = *(const vector signed char *)arr1234;
		vecYUV6789 = *(const vector signed char *)arr6789;
#endif

		int tmpWidth = width;
		while (tmpWidth--) {
			p++;

			w3 = *(p - nextlineSrc);
			w6 = *(p);
			w9 = *(p + nextlineSrc);

			int pattern = 0;

#if USE_ALTIVEC
			/*
			Consider this peephole into the image buffer:
			+----+----+----+----+
			|    |    |    |    |
			| w00| w01| w02| w03|
			+----+----+----+----+
			|    |    |    |    |
			| w10| w11| w12| w13|
			+----+----+----+----+
			|    |    |    |    |
			| w20| w21| w22| w23|
			+----+----+----+----+
			
			In the previous loop iteration, w11 was the center point, and our
			vectors contain the following data from the previous iteration:
			vecYUV5555 = { w11, w11, w11, w11 }
			vecYUV1234 = { w00, w01, w02, w10 }
			vecYUV6789 = { w12, w20, w21, w22 }

			Now we have the new center point w12, and we would like to have
			the following values in our vectors:
			vecYUV5555 = { w12, w12, w12, w12 }
			vecYUV1234 = { w01, w02, w03, w11 }
			vecYUV6789 = { w13, w21, w22, w23 }

			To this end we load a single new vector:
			vTmp = { w11, w03, w13, w23 }

			We then can compute all the new vector values using permutations only:
			vecYUV5555 = { vecYUV6789[0], vecYUV6789[0], vecYUV6789[0], vecYUV6789[0] }
			vecYUV1234 = { vecYUV1234[1], vecYUV1234[2],  vTmp[1],  vTmp[0] }
			vecYUV6789 = {  vTmp[2], vecYUV6789[2], vecYUV6789[3],  vTmp[3] }
			
			Beautiful, isn't it? :-)
			*/

			// Load the new values into a temporary vector (see above for an explanation)
			const int tmpArr[4] = {YUV(4), YUV(3), YUV(6), YUV(9)};
			vector signed char vTmp = *(const vector signed char *)tmpArr;
			
			// Next update the data vectors
			vecYUV5555 = (vector signed char)vec_splat((vector unsigned int)vecYUV6789, 0);
			vecYUV1234 = vec_perm(vecYUV1234, vTmp, vPermuteToV1234);
			vecYUV6789 = vec_perm(vecYUV6789, vTmp, vPermuteToV6789);

			// Compute the absolute difference between the center point's YUV and the outer points
			const vector signed char vDiff1 = vec_abs(vec_sub(vecYUV5555, vecYUV1234));
			const vector signed char vDiff2 = vec_abs(vec_sub(vecYUV5555, vecYUV6789));
			
			// Compare the difference to the threshold (byte-wise)
			const vector bool char vCmp1 = vec_cmpgt((vector unsigned char)vDiff1, vThreshold);
			const vector bool char vCmp2 = vec_cmpgt((vector unsigned char)vDiff2, vThreshold);
			
			// Convert all non-zero (long) vector elements to 0xF...F, keep 0 at 0.
			// Then and in the patter masks. The idea is that for 0 components, we get 0,
			// while for the other components we get exactly the mask value.
			const vector signed int vPattern1 = vec_and(vec_cmpgt((vector unsigned int)vCmp1, (vector unsigned int)0), vPatternMask1);
			const vector signed int vPattern2 = vec_and(vec_cmpgt((vector unsigned int)vCmp2, (vector unsigned int)0), vPatternMask2);
			
			// Now sum up the components of all vectors. Since our pattern mask values
			// are all "orthogonal", this is effectively the same as ORing them all
			// together. In the end, the rightmost word of vSum contains the 'pattern'
			vector signed int vSum = vec_sums(vPattern1, (vector signed int)0);
			vSum = vec_sums(vPattern2, vSum);
			pattern = ((int *)&vSum)[3];
#else
			const int yuv5 = YUV(5);
			if (w5 != w1 && diffYUV(yuv5, YUV(1))) pattern |= 0x0001;
			if (w5 != w2 && diffYUV(yuv5, YUV(2))) pattern |= 0x0002;
			if (w5 != w3 && diffYUV(yuv5, YUV(3))) pattern |= 0x0004;
			if (w5 != w4 && diffYUV(yuv5, YUV(4))) pattern |= 0x0008;
			if (w5 != w6 && diffYUV(yuv5, YUV(6))) pattern |= 0x0010;
			if (w5 != w7 && diffYUV(yuv5, YUV(7))) pattern |= 0x0020;
			if (w5 != w8 && diffYUV(yuv5, YUV(8))) pattern |= 0x0040;
			if (w5 != w9 && diffYUV(yuv5, YUV(9))) pattern |= 0x0080;
#endif

			switch (pattern) {
			case 0:
			case 1:
			case 4:
			case 32:
			case 128:
			case 5:
			case 132:
			case 160:
			case 33:
			case 129:
			case 36:
			case 133:
			case 164:
			case 161:
			case 37:
			case 165:
				PIXEL00_20
				PIXEL01_20
				PIXEL10_20
				PIXEL11_20
				break;
			case 2:
			case 34:
			case 130:
			case 162:
				PIXEL00_22
				PIXEL01_21
				PIXEL10_20
				PIXEL11_20
				break;
			case 16:
			case 17:
			case 48:
			case 49:
				PIXEL00_20
				PIXEL01_22
				PIXEL10_20
				PIXEL11_21
				break;
			case 64:
			case 65:
			case 68:
			case 69:
				PIXEL00_20
				PIXEL01_20
				PIXEL10_21
				PIXEL11_22
				break;
			case 8:
			case 12:
			case 136:
			case 140:
				PIXEL00_21
				PIXEL01_20
				PIXEL10_22
				PIXEL11_20
				break;
			case 3:
			case 35:
			case 131:
			case 163:
				PIXEL00_11
				PIXEL01_21
				PIXEL10_20
				PIXEL11_20
				break;
			case 6:
			case 38:
			case 134:
			case 166:
				PIXEL00_22
				PIXEL01_12
				PIXEL10_20
				PIXEL11_20
				break;
			case 20:
			case 21:
			case 52:
			case 53:
				PIXEL00_20
				PIXEL01_11
				PIXEL10_20
				PIXEL11_21
				break;
			case 144:
			case 145:
			case 176:
			case 177:
				PIXEL00_20
				PIXEL01_22
				PIXEL10_20
				PIXEL11_12
				break;
			case 192:
			case 193:
			case 196:
			case 197:
				PIXEL00_20
				PIXEL01_20
				PIXEL10_21
				PIXEL11_11
				break;
			case 96:
			case 97:
			case 100:
			case 101:
				PIXEL00_20
				PIXEL01_20
				PIXEL10_12
				PIXEL11_22
				break;
			case 40:
			case 44:
			case 168:
			case 172:
				PIXEL00_21
				PIXEL01_20
				PIXEL10_11
				PIXEL11_20
				break;
			case 9:
			case 13:
			case 137:
			case 141:
				PIXEL00_12
				PIXEL01_20
				PIXEL10_22
				PIXEL11_20
				break;
			case 18:
			case 50:
				PIXEL00_22
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
				} else {
					PIXEL01_20
				}
				PIXEL10_20
				PIXEL11_21
				break;
			case 80:
			case 81:
				PIXEL00_20
				PIXEL01_22
				PIXEL10_21
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_10
				} else {
					PIXEL11_20
				}
				break;
			case 72:
			case 76:
				PIXEL00_21
				PIXEL01_20
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
				} else {
					PIXEL10_20
				}
				PIXEL11_22
				break;
			case 10:
			case 138:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
				} else {
					PIXEL00_20
				}
				PIXEL01_21
				PIXEL10_22
				PIXEL11_20
				break;
			case 66:
				PIXEL00_22
				PIXEL01_21
				PIXEL10_21
				PIXEL11_22
				break;
			case 24:
				PIXEL00_21
				PIXEL01_22
				PIXEL10_22
				PIXEL11_21
				break;
			case 7:
			case 39:
			case 135:
				PIXEL00_11
				PIXEL01_12
				PIXEL10_20
				PIXEL11_20
				break;
			case 148:
			case 149:
			case 180:
				PIXEL00_20
				PIXEL01_11
				PIXEL10_20
				PIXEL11_12
				break;
			case 224:
			case 228:
			case 225:
				PIXEL00_20
				PIXEL01_20
				PIXEL10_12
				PIXEL11_11
				break;
			case 41:
			case 169:
			case 45:
				PIXEL00_12
				PIXEL01_20
				PIXEL10_11
				PIXEL11_20
				break;
			case 22:
			case 54:
				PIXEL00_22
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_20
				PIXEL11_21
				break;
			case 208:
			case 209:
				PIXEL00_20
				PIXEL01_22
				PIXEL10_21
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 104:
			case 108:
				PIXEL00_21
				PIXEL01_20
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_22
				break;
			case 11:
			case 139:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_21
				PIXEL10_22
				PIXEL11_20
				break;
			case 19:
			case 51:
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL00_11
					PIXEL01_10
				} else {
					PIXEL00_60
					PIXEL01_90
				}
				PIXEL10_20
				PIXEL11_21
				break;
			case 146:
			case 178:
				PIXEL00_22
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
					PIXEL11_12
				} else {
					PIXEL01_90
					PIXEL11_61
				}
				PIXEL10_20
				break;
			case 84:
			case 85:
				PIXEL00_20
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL01_11
					PIXEL11_10
				} else {
					PIXEL01_60
					PIXEL11_90
				}
				PIXEL10_21
				break;
			case 112:
			case 113:
				PIXEL00_20
				PIXEL01_22
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL10_12
					PIXEL11_10
				} else {
					PIXEL10_61
					PIXEL11_90
				}
				break;
			case 200:
			case 204:
				PIXEL00_21
				PIXEL01_20
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
					PIXEL11_11
				} else {
					PIXEL10_90
					PIXEL11_60
				}
				break;
			case 73:
			case 77:
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL00_12
					PIXEL10_10
				} else {
					PIXEL00_61
					PIXEL10_90
				}
				PIXEL01_20
				PIXEL11_22
				break;
			case 42:
			case 170:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
					PIXEL10_11
				} else {
					PIXEL00_90
					PIXEL10_60
				}
				PIXEL01_21
				PIXEL11_20
				break;
			case 14:
			case 142:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
					PIXEL01_12
				} else {
					PIXEL00_90
					PIXEL01_61
				}
				PIXEL10_22
				PIXEL11_20
				break;
			case 67:
				PIXEL00_11
				PIXEL01_21
				PIXEL10_21
				PIXEL11_22
				break;
			case 70:
				PIXEL00_22
				PIXEL01_12
				PIXEL10_21
				PIXEL11_22
				break;
			case 28:
				PIXEL00_21
				PIXEL01_11
				PIXEL10_22
				PIXEL11_21
				break;
			case 152:
				PIXEL00_21
				PIXEL01_22
				PIXEL10_22
				PIXEL11_12
				break;
			case 194:
				PIXEL00_22
				PIXEL01_21
				PIXEL10_21
				PIXEL11_11
				break;
			case 98:
				PIXEL00_22
				PIXEL01_21
				PIXEL10_12
				PIXEL11_22
				break;
			case 56:
				PIXEL00_21
				PIXEL01_22
				PIXEL10_11
				PIXEL11_21
				break;
			case 25:
				PIXEL00_12
				PIXEL01_22
				PIXEL10_22
				PIXEL11_21
				break;
			case 26:
			case 31:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_22
				PIXEL11_21
				break;
			case 82:
			case 214:
				PIXEL00_22
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_21
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 88:
			case 248:
				PIXEL00_21
				PIXEL01_22
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 74:
			case 107:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_21
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_22
				break;
			case 27:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_10
				PIXEL10_22
				PIXEL11_21
				break;
			case 86:
				PIXEL00_22
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_21
				PIXEL11_10
				break;
			case 216:
				PIXEL00_21
				PIXEL01_22
				PIXEL10_10
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 106:
				PIXEL00_10
				PIXEL01_21
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_22
				break;
			case 30:
				PIXEL00_10
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_22
				PIXEL11_21
				break;
			case 210:
				PIXEL00_22
				PIXEL01_10
				PIXEL10_21
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 120:
				PIXEL00_21
				PIXEL01_22
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_10
				break;
			case 75:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_21
				PIXEL10_10
				PIXEL11_22
				break;
			case 29:
				PIXEL00_12
				PIXEL01_11
				PIXEL10_22
				PIXEL11_21
				break;
			case 198:
				PIXEL00_22
				PIXEL01_12
				PIXEL10_21
				PIXEL11_11
				break;
			case 184:
				PIXEL00_21
				PIXEL01_22
				PIXEL10_11
				PIXEL11_12
				break;
			case 99:
				PIXEL00_11
				PIXEL01_21
				PIXEL10_12
				PIXEL11_22
				break;
			case 57:
				PIXEL00_12
				PIXEL01_22
				PIXEL10_11
				PIXEL11_21
				break;
			case 71:
				PIXEL00_11
				PIXEL01_12
				PIXEL10_21
				PIXEL11_22
				break;
			case 156:
				PIXEL00_21
				PIXEL01_11
				PIXEL10_22
				PIXEL11_12
				break;
			case 226:
				PIXEL00_22
				PIXEL01_21
				PIXEL10_12
				PIXEL11_11
				break;
			case 60:
				PIXEL00_21
				PIXEL01_11
				PIXEL10_11
				PIXEL11_21
				break;
			case 195:
				PIXEL00_11
				PIXEL01_21
				PIXEL10_21
				PIXEL11_11
				break;
			case 102:
				PIXEL00_22
				PIXEL01_12
				PIXEL10_12
				PIXEL11_22
				break;
			case 153:
				PIXEL00_12
				PIXEL01_22
				PIXEL10_22
				PIXEL11_12
				break;
			case 58:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
				} else {
					PIXEL00_70
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
				} else {
					PIXEL01_70
				}
				PIXEL10_11
				PIXEL11_21
				break;
			case 83:
				PIXEL00_11
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
				} else {
					PIXEL01_70
				}
				PIXEL10_21
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_10
				} else {
					PIXEL11_70
				}
				break;
			case 92:
				PIXEL00_21
				PIXEL01_11
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
				} else {
					PIXEL10_70
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_10
				} else {
					PIXEL11_70
				}
				break;
			case 202:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
				} else {
					PIXEL00_70
				}
				PIXEL01_21
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
				} else {
					PIXEL10_70
				}
				PIXEL11_11
				break;
			case 78:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
				} else {
					PIXEL00_70
				}
				PIXEL01_12
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
				} else {
					PIXEL10_70
				}
				PIXEL11_22
				break;
			case 154:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
				} else {
					PIXEL00_70
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
				} else {
					PIXEL01_70
				}
				PIXEL10_22
				PIXEL11_12
				break;
			case 114:
				PIXEL00_22
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
				} else {
					PIXEL01_70
				}
				PIXEL10_12
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_10
				} else {
					PIXEL11_70
				}
				break;
			case 89:
				PIXEL00_12
				PIXEL01_22
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
				} else {
					PIXEL10_70
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_10
				} else {
					PIXEL11_70
				}
				break;
			case 90:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
				} else {
					PIXEL00_70
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
				} else {
					PIXEL01_70
				}
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
				} else {
					PIXEL10_70
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_10
				} else {
					PIXEL11_70
				}
				break;
			case 55:
			case 23:
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL00_11
					PIXEL01_0
				} else {
					PIXEL00_60
					PIXEL01_90
				}
				PIXEL10_20
				PIXEL11_21
				break;
			case 182:
			case 150:
				PIXEL00_22
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
					PIXEL11_12
				} else {
					PIXEL01_90
					PIXEL11_61
				}
				PIXEL10_20
				break;
			case 213:
			case 212:
				PIXEL00_20
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL01_11
					PIXEL11_0
				} else {
					PIXEL01_60
					PIXEL11_90
				}
				PIXEL10_21
				break;
			case 241:
			case 240:
				PIXEL00_20
				PIXEL01_22
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL10_12
					PIXEL11_0
				} else {
					PIXEL10_61
					PIXEL11_90
				}
				break;
			case 236:
			case 232:
				PIXEL00_21
				PIXEL01_20
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
					PIXEL11_11
				} else {
					PIXEL10_90
					PIXEL11_60
				}
				break;
			case 109:
			case 105:
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL00_12
					PIXEL10_0
				} else {
					PIXEL00_61
					PIXEL10_90
				}
				PIXEL01_20
				PIXEL11_22
				break;
			case 171:
			case 43:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
					PIXEL10_11
				} else {
					PIXEL00_90
					PIXEL10_60
				}
				PIXEL01_21
				PIXEL11_20
				break;
			case 143:
			case 15:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
					PIXEL01_12
				} else {
					PIXEL00_90
					PIXEL01_61
				}
				PIXEL10_22
				PIXEL11_20
				break;
			case 124:
				PIXEL00_21
				PIXEL01_11
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_10
				break;
			case 203:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_21
				PIXEL10_10
				PIXEL11_11
				break;
			case 62:
				PIXEL00_10
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_11
				PIXEL11_21
				break;
			case 211:
				PIXEL00_11
				PIXEL01_10
				PIXEL10_21
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 118:
				PIXEL00_22
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_12
				PIXEL11_10
				break;
			case 217:
				PIXEL00_12
				PIXEL01_22
				PIXEL10_10
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 110:
				PIXEL00_10
				PIXEL01_12
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_22
				break;
			case 155:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_10
				PIXEL10_22
				PIXEL11_12
				break;
			case 188:
				PIXEL00_21
				PIXEL01_11
				PIXEL10_11
				PIXEL11_12
				break;
			case 185:
				PIXEL00_12
				PIXEL01_22
				PIXEL10_11
				PIXEL11_12
				break;
			case 61:
				PIXEL00_12
				PIXEL01_11
				PIXEL10_11
				PIXEL11_21
				break;
			case 157:
				PIXEL00_12
				PIXEL01_11
				PIXEL10_22
				PIXEL11_12
				break;
			case 103:
				PIXEL00_11
				PIXEL01_12
				PIXEL10_12
				PIXEL11_22
				break;
			case 227:
				PIXEL00_11
				PIXEL01_21
				PIXEL10_12
				PIXEL11_11
				break;
			case 230:
				PIXEL00_22
				PIXEL01_12
				PIXEL10_12
				PIXEL11_11
				break;
			case 199:
				PIXEL00_11
				PIXEL01_12
				PIXEL10_21
				PIXEL11_11
				break;
			case 220:
				PIXEL00_21
				PIXEL01_11
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
				} else {
					PIXEL10_70
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 158:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
				} else {
					PIXEL00_70
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_22
				PIXEL11_12
				break;
			case 234:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
				} else {
					PIXEL00_70
				}
				PIXEL01_21
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_11
				break;
			case 242:
				PIXEL00_22
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
				} else {
					PIXEL01_70
				}
				PIXEL10_12
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 59:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
				} else {
					PIXEL01_70
				}
				PIXEL10_11
				PIXEL11_21
				break;
			case 121:
				PIXEL00_12
				PIXEL01_22
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_10
				} else {
					PIXEL11_70
				}
				break;
			case 87:
				PIXEL00_11
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_21
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_10
				} else {
					PIXEL11_70
				}
				break;
			case 79:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_12
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
				} else {
					PIXEL10_70
				}
				PIXEL11_22
				break;
			case 122:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
				} else {
					PIXEL00_70
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
				} else {
					PIXEL01_70
				}
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_10
				} else {
					PIXEL11_70
				}
				break;
			case 94:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
				} else {
					PIXEL00_70
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
				} else {
					PIXEL10_70
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_10
				} else {
					PIXEL11_70
				}
				break;
			case 218:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
				} else {
					PIXEL00_70
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
				} else {
					PIXEL01_70
				}
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
				} else {
					PIXEL10_70
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 91:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
				} else {
					PIXEL01_70
				}
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
				} else {
					PIXEL10_70
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_10
				} else {
					PIXEL11_70
				}
				break;
			case 229:
				PIXEL00_20
				PIXEL01_20
				PIXEL10_12
				PIXEL11_11
				break;
			case 167:
				PIXEL00_11
				PIXEL01_12
				PIXEL10_20
				PIXEL11_20
				break;
			case 173:
				PIXEL00_12
				PIXEL01_20
				PIXEL10_11
				PIXEL11_20
				break;
			case 181:
				PIXEL00_20
				PIXEL01_11
				PIXEL10_20
				PIXEL11_12
				break;
			case 186:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
				} else {
					PIXEL00_70
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
				} else {
					PIXEL01_70
				}
				PIXEL10_11
				PIXEL11_12
				break;
			case 115:
				PIXEL00_11
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
				} else {
					PIXEL01_70
				}
				PIXEL10_12
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_10
				} else {
					PIXEL11_70
				}
				break;
			case 93:
				PIXEL00_12
				PIXEL01_11
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
				} else {
					PIXEL10_70
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_10
				} else {
					PIXEL11_70
				}
				break;
			case 206:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
				} else {
					PIXEL00_70
				}
				PIXEL01_12
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
				} else {
					PIXEL10_70
				}
				PIXEL11_11
				break;
			case 205:
			case 201:
				PIXEL00_12
				PIXEL01_20
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_10
				} else {
					PIXEL10_70
				}
				PIXEL11_11
				break;
			case 174:
			case 46:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_10
				} else {
					PIXEL00_70
				}
				PIXEL01_12
				PIXEL10_11
				PIXEL11_20
				break;
			case 179:
			case 147:
				PIXEL00_11
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_10
				} else {
					PIXEL01_70
				}
				PIXEL10_20
				PIXEL11_12
				break;
			case 117:
			case 116:
				PIXEL00_20
				PIXEL01_11
				PIXEL10_12
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_10
				} else {
					PIXEL11_70
				}
				break;
			case 189:
				PIXEL00_12
				PIXEL01_11
				PIXEL10_11
				PIXEL11_12
				break;
			case 231:
				PIXEL00_11
				PIXEL01_12
				PIXEL10_12
				PIXEL11_11
				break;
			case 126:
				PIXEL00_10
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_10
				break;
			case 219:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_10
				PIXEL10_10
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 125:
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL00_12
					PIXEL10_0
				} else {
					PIXEL00_61
					PIXEL10_90
				}
				PIXEL01_11
				PIXEL11_10
				break;
			case 221:
				PIXEL00_12
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL01_11
					PIXEL11_0
				} else {
					PIXEL01_60
					PIXEL11_90
				}
				PIXEL10_10
				break;
			case 207:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
					PIXEL01_12
				} else {
					PIXEL00_90
					PIXEL01_61
				}
				PIXEL10_10
				PIXEL11_11
				break;
			case 238:
				PIXEL00_10
				PIXEL01_12
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
					PIXEL11_11
				} else {
					PIXEL10_90
					PIXEL11_60
				}
				break;
			case 190:
				PIXEL00_10
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
					PIXEL11_12
				} else {
					PIXEL01_90
					PIXEL11_61
				}
				PIXEL10_11
				break;
			case 187:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
					PIXEL10_11
				} else {
					PIXEL00_90
					PIXEL10_60
				}
				PIXEL01_10
				PIXEL11_12
				break;
			case 243:
				PIXEL00_11
				PIXEL01_10
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL10_12
					PIXEL11_0
				} else {
					PIXEL10_61
					PIXEL11_90
				}
				break;
			case 119:
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL00_11
					PIXEL01_0
				} else {
					PIXEL00_60
					PIXEL01_90
				}
				PIXEL10_12
				PIXEL11_10
				break;
			case 237:
			case 233:
				PIXEL00_12
				PIXEL01_20
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_100
				}
				PIXEL11_11
				break;
			case 175:
			case 47:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_100
				}
				PIXEL01_12
				PIXEL10_11
				PIXEL11_20
				break;
			case 183:
			case 151:
				PIXEL00_11
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_100
				}
				PIXEL10_20
				PIXEL11_12
				break;
			case 245:
			case 244:
				PIXEL00_20
				PIXEL01_11
				PIXEL10_12
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_100
				}
				break;
			case 250:
				PIXEL00_10
				PIXEL01_10
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 123:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_10
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_10
				break;
			case 95:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_10
				PIXEL11_10
				break;
			case 222:
				PIXEL00_10
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_10
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 252:
				PIXEL00_21
				PIXEL01_11
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_100
				}
				break;
			case 249:
				PIXEL00_12
				PIXEL01_22
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_100
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 235:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_21
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_100
				}
				PIXEL11_11
				break;
			case 111:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_100
				}
				PIXEL01_12
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_22
				break;
			case 63:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_100
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_11
				PIXEL11_21
				break;
			case 159:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_100
				}
				PIXEL10_22
				PIXEL11_12
				break;
			case 215:
				PIXEL00_11
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_100
				}
				PIXEL10_21
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 246:
				PIXEL00_22
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				PIXEL10_12
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_100
				}
				break;
			case 254:
				PIXEL00_10
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_100
				}
				break;
			case 253:
				PIXEL00_12
				PIXEL01_11
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_100
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_100
				}
				break;
			case 251:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				PIXEL01_10
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_100
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 239:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_100
				}
				PIXEL01_12
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_100
				}
				PIXEL11_11
				break;
			case 127:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_100
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_20
				}
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_20
				}
				PIXEL11_10
				break;
			case 191:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_100
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_100
				}
				PIXEL10_11
				PIXEL11_12
				break;
			case 223:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_20
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_100
				}
				PIXEL10_10
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_20
				}
				break;
			case 247:
				PIXEL00_11
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_100
				}
				PIXEL10_12
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_100
				}
				break;
			case 255:
				if (diffYUV(YUV(4), YUV(2))) {
					PIXEL00_0
				} else {
					PIXEL00_100
				}
				if (diffYUV(YUV(2), YUV(6))) {
					PIXEL01_0
				} else {
					PIXEL01_100
				}
				if (diffYUV(YUV(8), YUV(4))) {
					PIXEL10_0
				} else {
					PIXEL10_100
				}
				if (diffYUV(YUV(6), YUV(8))) {
					PIXEL11_0
				} else {
					PIXEL11_100
				}
				break;
			}

			w1 = w2;
			w4 = w5;
			w7 = w8;

			w2 = w3;
			w5 = w6;
			w8 = w9;

			q += 2;
		}
		p += nextlineSrc - width;
		q += (nextlineDst - width) * 2;
	}
}

MAKE_WRAPPER(HQ2x)
