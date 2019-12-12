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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/graphics/scalers/2x_sal_scalers.h"
#include "ultima/ultima8/graphics/manips.h"
#include "ultima/ultima8/graphics/texture.h"

namespace Ultima8 {
namespace Pentagram {

template<class uintX, class Manip, class uintS = uintX> class _2xSaIScalerInternal {

public:


	/**
	 ** 2xSaI scaling filter source code adapted for Exult
	 ** August 29 2000, originally written in May 1999
	 ** by Derek Liauw Kie Fa (DerekL666@yahoo.com/D.A.K.L.LiauwKieFa@student.tudelft.nl)
	 ** This source is made available under the terms of the GNU GPL
	 ** I'd appreciate it I am given credit in the program or documentation
	 **/

	static inline uintX Interpolate_2xSaI(uintS colorA, uintS colorB) {
		uint8 r0, r1, g0, g1, b0, b1, a0, a1;
		Manip::split(colorA, r0, g0, b0, a0);
		Manip::split(colorB, r1, g1, b1, a1);
		int r = (r0 + r1) >> 1;
		int g = (g0 + g1) >> 1;
		int b = (b0 + b1) >> 1;
		int a = (a0 + a1) >> 1;
		return Manip::merge(r, g, b, a);
	}

	static inline uintX OInterpolate_2xSaI(uintS colorA, uintS colorB, uintS colorC) {
		uint8 r0, r1, g0, g1, b0, b1, a0, a1;
		uint8 r2, g2, b2, a2;
		Manip::split(colorA, r0, g0, b0, a0);
		Manip::split(colorB, r1, g1, b1, a1);
		Manip::split(colorC, r2, g2, b2, a2);
		unsigned int r = ((r0 << 2) + (r0 << 1) + r1 + r2) >> 3;
		unsigned int g = ((g0 << 2) + (g0 << 1) + g1 + g2) >> 3;
		unsigned int b = ((b0 << 2) + (b0 << 1) + b1 + b2) >> 3;
		unsigned int a = ((a0 << 2) + (a0 << 1) + a1 + a2) >> 3;
		return Manip::merge(r, g, b, a);
	}

	static inline uintX QInterpolate_2xSaI(uintS colorA, uintS colorB, uintS colorC, uintS colorD) {
		uint8 r0, r1, g0, g1, b0, b1, a0, a1;
		uint8 r2, r3, g2, g3, b2, b3, a2, a3;
		Manip::split(colorA, r0, g0, b0, a0);
		Manip::split(colorB, r1, g1, b1, a1);
		Manip::split(colorC, r2, g2, b2, a2);
		Manip::split(colorD, r3, g3, b3, a3);
		unsigned int r = (r0 + r1 + r2 + r3) >> 2;
		unsigned int g = (g0 + g1 + g2 + g3) >> 2;
		unsigned int b = (b0 + b1 + b2 + b3) >> 2;
		unsigned int a = (a0 + a1 + a2 + a3) >> 2;
		return Manip::merge(r, g, b, a);
	}

	static inline int GetResult1(uintS A, uintS B, uintS C, uintS D) {
		int x = 0;
		int y = 0;
		int r = 0;
		if (A == C) x += 1;
		else if (B == C) y += 1;
		if (A == D) x += 1;
		else if (B == D) y += 1;
		if (x <= 1) r += 1;
		if (y <= 1) r -= 1;
		return r;
	}

	static inline int GetResult2(uintS A, uintS B, uintS C, uintS D) {
		int x = 0;
		int y = 0;
		int r = 0;
		if (A == C) x += 1;
		else if (B == C) y += 1;
		if (A == D) x += 1;
		else if (B == D) y += 1;
		if (x <= 1) r -= 1;
		if (y <= 1) r += 1;
		return r;
	}

//
// 2xSaI Scaler
//
	static void Scale_2xSaI
	(
	    uintS *source,          // ->source pixels.
	    int srcx, int srcy,         // Start of rectangle within src.
	    int srcw, int srch,         // Dims. of rectangle.
	    const int sline_pixels,     // Pixels (words)/line for source.
	    const int sheight,          // Source height.
	    uintX *dest,            // ->dest pixels.
	    const int dline_pixels      // Pixels (words)/line for dest.
	) {
		uintS *srcPtr = source + (srcx + srcy * sline_pixels);
		uintX *dstPtr = dest;

		if (srcx + srcw >= sline_pixels) {
			srcw = sline_pixels - srcx;
		}
		// Init offset to prev. line, next 2.
		int prev1_yoff = srcy ? sline_pixels : 0;
		int next1_yoff = sline_pixels, next2_yoff = 2 * sline_pixels;
		// Figure threshholds for counters.
		int ybeforelast = sheight - 2 - srcy;
		int xbeforelast = sline_pixels - 2 - srcx;
		for (int y = 0; y < srch; y++, prev1_yoff = sline_pixels) {
			if (y >= ybeforelast)   // Last/next-to-last row?
				if (y == ybeforelast)
					next2_yoff = sline_pixels;
				else        // Very last line?
					next2_yoff = next1_yoff = 0;

			uintS *bP = srcPtr;
			uintX *dP = dstPtr;
			int prev1_xoff = srcx ? 1 : 0;
			int next1_xoff = 1, next2_xoff = 2;

			for (int x = 0; x < srcw; x++) {
				uintS colorA, colorB;
				uintS colorC, colorD,
				      colorE, colorF, colorG, colorH,
				      colorI, colorJ, colorK, colorL,
				      colorM, colorN, colorO, colorP;
				uintX product, product1, product2, orig;

				// Last/next-to-last row?
				if (x >= xbeforelast)
					if (x == xbeforelast)
						next2_xoff = 1;
					else
						next2_xoff = next1_xoff = 0;

				//---------------------------------------
				// Map of the pixels:                    I|E F|J
				//                                       G|A B|K
				//                                       H|C D|L
				//                                       M|N O|P
				colorI = *(bP - prev1_yoff - prev1_xoff);
				colorE = *(bP - prev1_yoff);
				colorF = *(bP - prev1_yoff + next1_xoff);
				colorJ = *(bP - prev1_yoff + next2_xoff);

				colorG = *(bP - prev1_xoff);
				colorA = *(bP);
				colorB = *(bP + next1_xoff);
				colorK = *(bP + next2_xoff);

				colorH = *(bP + next1_yoff - prev1_xoff);
				colorC = *(bP + next1_yoff);
				colorD = *(bP + next1_yoff + next1_xoff);
				colorL = *(bP + next1_yoff + next2_xoff);

				colorM = *(bP + next2_yoff - prev1_xoff);
				colorN = *(bP + next2_yoff);
				colorO = *(bP + next2_yoff + next1_xoff);
				colorP = *(bP + next2_yoff + next2_xoff);

				if ((colorA == colorD) && (colorB != colorC)) {
					if (((colorA == colorE) && (colorB == colorL)) ||
					        ((colorA == colorC) && (colorA == colorF) && (colorB != colorE) && (colorB == colorJ))) {
						//product = colorA;
						product = Manip::copy(colorA);
					} else {
						//product = INTERPOLATE(colorA, colorB);
						product = Interpolate_2xSaI(colorA, colorB);
					}

					if (((colorA == colorG) && (colorC == colorO)) ||
					        ((colorA == colorB) && (colorA == colorH) && (colorG != colorC) && (colorC == colorM))) {
						//product1 = colorA;
						product1 = Manip::copy(colorA);
					} else {
						//product1 = INTERPOLATE(colorA, colorC);
						product1 = Interpolate_2xSaI(colorA, colorC);
					}
					//product2 = colorA;
					product2 = Manip::copy(colorA);
				} else if ((colorB == colorC) && (colorA != colorD)) {
					if (((colorB == colorF) && (colorA == colorH)) ||
					        ((colorB == colorE) && (colorB == colorD) && (colorA != colorF) && (colorA == colorI))) {
						//product = colorB;
						product = Manip::copy(colorB);
					} else {
						//product = INTERPOLATE(colorA, colorB);
						product = Interpolate_2xSaI(colorA, colorB);
					}

					if (((colorC == colorH) && (colorA == colorF)) ||
					        ((colorC == colorG) && (colorC == colorD) && (colorA != colorH) && (colorA == colorI))) {
						//product1 = colorC;
						product1 = Manip::copy(colorC);
					} else {
						//product1 = INTERPOLATE(colorA, colorC);
						product1 = Interpolate_2xSaI(colorA, colorC);
					}
					//product2 = colorB;
					product2 = Manip::copy(colorB);
				} else if ((colorA == colorD) && (colorB == colorC)) {
					if (colorA == colorB) {
						//product = colorA;
						product = Manip::copy(colorA);
						//product1 = colorA;
						product1 = Manip::copy(colorA);
						//product2 = colorA;
						product2 = Manip::copy(colorA);
					} else {
						register int r = 0;
						//product1 = INTERPOLATE(colorA, colorC);
						product1 = Interpolate_2xSaI(colorA, colorC);
						//product = INTERPOLATE(colorA, colorB);
						product = Interpolate_2xSaI(colorA, colorB);

						r += GetResult1(colorA, colorB, colorG, colorE);
						r += GetResult2(colorB, colorA, colorK, colorF);
						r += GetResult2(colorB, colorA, colorH, colorN);
						r += GetResult1(colorA, colorB, colorL, colorO);

						if (r > 0)
							//product2 = colorA;
							product2 = Manip::copy(colorA);
						else if (r < 0)
							//product2 = colorB;
							product2 = Manip::copy(colorB);
						else {
							//product2 = Q_INTERPOLATE(colorA, colorB, colorC, colorD);
							product2 = QInterpolate_2xSaI(colorA, colorB, colorC, colorD);
						}
					}
				} else {
					//product2 = Q_INTERPOLATE(colorA, colorB, colorC, colorD);
					product2 = QInterpolate_2xSaI(colorA, colorB, colorC, colorD);

					if ((colorA == colorC) && (colorA == colorF) && (colorB != colorE) && (colorB == colorJ)) {
						//product = colorA;
						product = Manip::copy(colorA);
					} else if ((colorB == colorE) && (colorB == colorD) && (colorA != colorF) && (colorA == colorI)) {
						//product = colorB;
						product = Manip::copy(colorB);
					} else {
						//product = INTERPOLATE(colorA, colorB);
						product = Interpolate_2xSaI(colorA, colorB);
					}

					if ((colorA == colorB) && (colorA == colorH) && (colorG != colorC) && (colorC == colorM)) {
						//product1 = colorA;
						product1 = Manip::copy(colorA);
					} else if ((colorC == colorG) && (colorC == colorD) && (colorA != colorH) && (colorA == colorI)) {
						//product1 = colorC;
						product1 = Manip::copy(colorC);
					} else {
						//product1 = INTERPOLATE(colorA, colorC);
						product1 = Interpolate_2xSaI(colorA, colorC);
					}
				}


				//product = colorA | (product << 16);
				//product1 = product1 | (product2 << 16);
				orig = Manip::copy(colorA);
				*dP = orig;
				*(dP + 1) = product;
				*(dP + dline_pixels) = product1;
				*(dP + dline_pixels + 1) = product2;

				bP += 1;
				dP += 2;
				prev1_xoff = 1;
			}//end of for ( finish= width etc..)

			srcPtr += sline_pixels;
			dstPtr += 2 * dline_pixels;
			prev1_yoff = 1;
		};
	}

//
// Super2xSaI Scaler
//
	static void Scale_Super2xSaI
	(
	    uintS *source,          // ->source pixels.
	    int srcx, int srcy,         // Start of rectangle within src.
	    int srcw, int srch,         // Dims. of rectangle.
	    const int sline_pixels,     // Pixels (words)/line for source.
	    const int sheight,          // Source height.
	    uintX *dest,            // ->dest pixels.
	    const int dline_pixels      // Pixels (words)/line for dest.
	) {

		uintS *srcPtr = source + (srcx + srcy * sline_pixels);
		uintX *dstPtr = dest;

		if (srcx + srcw >= sline_pixels) {
			srcw = sline_pixels - srcx;
		}

		int ybeforelast1 = sheight - 1 - srcy;
		int ybeforelast2 = sheight - 2 - srcy;
		int xbeforelast1 = sline_pixels - 1 - srcx;
		int xbeforelast2 = sline_pixels - 2 - srcx;

		for (int y = 0; y < srch; y++) {
			uintS *bP = srcPtr;
			uintX *dP = dstPtr;

			for (int x = 0; x < srcw; x++) {
				uintS color4, color5, color6;
				uintS color1, color2, color3;
				uintS colorA0, colorA1, colorA2, colorA3,
				      colorB0, colorB1, colorB2, colorB3,
				      colorS1, colorS2;
				uintX product1a, product1b,
				      product2a, product2b;

				//---------------------------------------  B0 B1 B2 B3
				//                                         4  5  6  S2
				//                                         1  2  3  S1
				//                                         A0 A1 A2 A3
				//--------------------------------------
				int add1, add2;
				int sub1;
				int nextl1, nextl2;
				int prevl1;

				if ((x + srcx) == 0)
					sub1 = 0;
				else
					sub1 = 1;

				if (x >= xbeforelast2)
					add2 = 0;
				else add2 = 1;

				if (x >= xbeforelast1)
					add1 = 0;
				else add1 = 1;

				if ((y + srcy) == 0)
					prevl1 = 0;
				else
					prevl1 = sline_pixels;

				if (y >= ybeforelast2)
					nextl2 = 0;
				else nextl2 = sline_pixels;

				if (y >= ybeforelast1)
					nextl1 = 0;
				else nextl1 = sline_pixels;


				colorB0 = *(bP - prevl1 - sub1);
				colorB1 = *(bP - prevl1);
				colorB2 = *(bP - prevl1 + add1);
				colorB3 = *(bP - prevl1 + add1 + add2);

				color4 = *(bP - sub1);
				color5 = *(bP);
				color6 = *(bP + add1);
				colorS2 = *(bP + add1 + add2);

				color1 = *(bP + nextl1 - sub1);
				color2 = *(bP + nextl1);
				color3 = *(bP + nextl1 + add1);
				colorS1 = *(bP + nextl1 + add1 + add2);

				colorA0 = *(bP + nextl1 + nextl2 - sub1);
				colorA1 = *(bP + nextl1 + nextl2);
				colorA2 = *(bP + nextl1 + nextl2 + add1);
				colorA3 = *(bP + nextl1 + nextl2 + add1 + add2);

				if (color2 == color6 && color5 != color3) {
					//product2b = product1b = color2;
					product2b = product1b = Manip::copy(color2);
				} else if (color5 == color3 && color2 != color6) {
					//product2b = product1b = color5;
					product2b = product1b = Manip::copy(color5);
				} else if (color5 == color3 && color2 == color6) {
					register int r = 0;

					//r += GetResult (color6, color5, color1, colorA1);
					//r += GetResult (color6, color5, color4, colorB1);
					//r += GetResult (color6, color5, colorA2, colorS1);
					//r += GetResult (color6, color5, colorB2, colorS2);
					r += GetResult1(color5, color6, color4, colorB1);
					r += GetResult2(color6, color5, colorA2, colorS1);
					r += GetResult2(color6, color5, color1, colorA1);
					r += GetResult1(color5, color6, colorB2, colorS2);

					if (r > 0) {
						//product2b = product1b = color6;
						product2b = product1b = Manip::copy(color6);
					} else if (r < 0) {
						//product2b = product1b = color5;
						product2b = product1b = Manip::copy(color5);
					} else {
						//product2b = product1b = INTERPOLATE (color5, color6);
						product1b = product2b = Interpolate_2xSaI(color5, color6);
					}

				} else {

					if (color6 == color3 && color3 == colorA1 && color2 != colorA2 && color3 != colorA0)
						//product2b = Q_INTERPOLATE (color3, color3, color3, color2);
						product2b = QInterpolate_2xSaI(color3, color3, color3, color2);
					else if (color5 == color2 && color2 == colorA2 && colorA1 != color3 && color2 != colorA3)
						//product2b = Q_INTERPOLATE (color2, color2, color2, color3);
						product2b = QInterpolate_2xSaI(color3, color2, color2, color2);
					else
						//product2b = INTERPOLATE (color2, color3);
						product2b = Interpolate_2xSaI(color2, color3);


					if (color6 == color3 && color6 == colorB1 && color5 != colorB2 && color6 != colorB0)
						//product1b = Q_INTERPOLATE (color6, color6, color6, color5);
						product1b = QInterpolate_2xSaI(color5, color6, color6, color6);
					else if (color5 == color2 && color5 == colorB2 && colorB1 != color6 && color5 != colorB3)
						//product1b = Q_INTERPOLATE (color6, color5, color5, color5);
						product1b = QInterpolate_2xSaI(color6, color5, color5, color5);
					else
						//product1b = INTERPOLATE (color5, color6);
						product1b = Interpolate_2xSaI(color5, color6);

				}

				if (color5 == color3 && color2 != color6 && color4 == color5 && color5 != colorA2)
					//product2a = INTERPOLATE (color2, color5);
					product2a = Interpolate_2xSaI(color5, color2);
				else if (color5 == color1 && color6 == color5 && color4 != color2 && color5 != colorA0)
					//product2a = INTERPOLATE(color2, color5);
					product2a = Interpolate_2xSaI(color5, color2);
				else
					//product2a = color2;
					product2a = Manip::copy(color2);


				if (color2 == color6 && color5 != color3 && color1 == color2 && color2 != colorB2)
					//product1a = INTERPOLATE (color2, color5);
					product1a = Interpolate_2xSaI(color5, color2);
				else if (color4 == color2 && color3 == color2 && color1 != color5 && color2 != colorB0)
					//product1a = INTERPOLATE(color2, color5);
					product1a = Interpolate_2xSaI(color5, color2);
				else
					//product1a = color5;
					product1a = Manip::copy(color5);


				*dP = product1a;
				*(dP + 1) = product1b;
				*(dP + dline_pixels) = product2a;
				*(dP + dline_pixels + 1) = product2b;

				bP += 1;
				dP += 2;

			}
			srcPtr += sline_pixels;
			dstPtr += 2 * dline_pixels;
		};
	}

//
// SuperEagle Scaler
//
	static void Scale_SuperEagle
	(
	    uintS *source,          // ->source pixels.
	    int srcx, int srcy,         // Start of rectangle within src.
	    int srcw, int srch,         // Dims. of rectangle.
	    const int sline_pixels,     // Pixels (words)/line for source.
	    const int sheight,          // Source height.
	    uintX *dest,            // ->dest pixels.
	    const int dline_pixels      // Pixels (words)/line for dest.
	) {

		uintS *srcPtr = source + (srcx + srcy * sline_pixels);
		uintX *dstPtr = dest;

		if (srcx + srcw >= sline_pixels) {
			srcw = sline_pixels - srcx;
		}

		int ybeforelast1 = sheight - 1 - srcy;
		int ybeforelast2 = sheight - 2 - srcy;
		int xbeforelast1 = sline_pixels - 1 - srcx;
		int xbeforelast2 = sline_pixels - 2 - srcx;

		for (int y = 0; y < srch; y++) {
			uintS *bP = srcPtr;
			uintX *dP = dstPtr;

			for (int x = 0; x < srcw; x++) {
				uintS color4, color5, color6;
				uintS color1, color2, color3;
				uintS colorA0, colorA1, colorA2, colorA3,
				      colorB0, colorB1, colorB2, colorB3,
				      colorS1, colorS2;
				uintX product1a, product1b,
				      product2a, product2b;

				//---------------------------------------  B0 B1 B2 B3
				//                                         4  5  6  S2
				//                                         1  2  3  S1
				//                                         A0 A1 A2 A3
				//--------------------------------------
				int add1, add2;
				int sub1;
				int nextl1, nextl2;
				int prevl1;

				if ((x + srcx) == 0)
					sub1 = 0;
				else
					sub1 = 1;

				if (x >= xbeforelast2)
					add2 = 0;
				else add2 = 1;

				if (x >= xbeforelast1)
					add1 = 0;
				else add1 = 1;

				if ((y + srcy) == 0)
					prevl1 = 0;
				else
					prevl1 = sline_pixels;

				if (y >= ybeforelast2)
					nextl2 = 0;
				else nextl2 = sline_pixels;

				if (y >= ybeforelast1)
					nextl1 = 0;
				else nextl1 = sline_pixels;


				colorB0 = *(bP - prevl1 - sub1);
				colorB1 = *(bP - prevl1);
				colorB2 = *(bP - prevl1 + add1);
				colorB3 = *(bP - prevl1 + add1 + add2);

				color4 = *(bP - sub1);
				color5 = *(bP);
				color6 = *(bP + add1);
				colorS2 = *(bP + add1 + add2);

				color1 = *(bP + nextl1 - sub1);
				color2 = *(bP + nextl1);
				color3 = *(bP + nextl1 + add1);
				colorS1 = *(bP + nextl1 + add1 + add2);

				colorA0 = *(bP + nextl1 + nextl2 - sub1);
				colorA1 = *(bP + nextl1 + nextl2);
				colorA2 = *(bP + nextl1 + nextl2 + add1);
				colorA3 = *(bP + nextl1 + nextl2 + add1 + add2);


				if (color2 == color6 && color5 != color3) {
					//product1b = product2a = color2;
					product1b = product2a = Manip::copy(color2);


					if ((color1 == color2) || (color6 == colorB2)) {
						//product1a = INTERPOLATE (color2, color5);
						//product1a = INTERPOLATE (color2, product1a);
						product1a = QInterpolate_2xSaI(color2, color2, color2, color5);

					} else {
						//product1a = INTERPOLATE (color5, color6);
						product1a = Interpolate_2xSaI(color6, color5);
					}

					if ((color6 == colorS2) || (color2 == colorA1)) {
						//product2b = INTERPOLATE (color2, color3);
						//product2b = INTERPOLATE (color2, product2b);
						product2b = QInterpolate_2xSaI(color2, color2, color2, color3);

					} else {
						//product2b = INTERPOLATE (color2, color3);
						product2b = Interpolate_2xSaI(color2, color3);
					}
				} else if (color5 == color3 && color2 != color6) {
					//product2b = product1a = color5;
					product2b = product1a = Manip::copy(color5);


					if ((colorB1 == color5) || (color3 == colorS1)) {
						//product1b = INTERPOLATE (color5, color6);
						//product1b = INTERPOLATE (color5, product1b);
						product1b = QInterpolate_2xSaI(color5, color5, color5, color6);
					} else {
						//product1b = INTERPOLATE (color5, color6);
						product1b = Interpolate_2xSaI(color5, color6);
					}

					if ((color3 == colorA2) || (color4 == color5)) {
						//product2a = INTERPOLATE (color5, color2);
						//product2a = INTERPOLATE (color5, product2a);
						product2a = QInterpolate_2xSaI(color2, color5, color5, color5);
					} else {
						//product2a = INTERPOLATE (color2, color3);
						product2a = Interpolate_2xSaI(color3, color2);
					}

				} else if (color5 == color3 && color2 == color6) {
					register int r = 0;

					//r += GetResult (color6, color5, color1, colorA1);
					//r += GetResult (color6, color5, color4, colorB1);
					//r += GetResult (color6, color5, colorA2, colorS1);
					//r += GetResult (color6, color5, colorB2, colorS2);
					r += GetResult1(color5, color6, color4, colorB1);
					r += GetResult2(color6, color5, colorA2, colorS1);
					r += GetResult2(color6, color5, color1, colorA1);
					r += GetResult1(color5, color6, colorB2, colorS2);

					if (r > 0) {
						//product1b = product2a = color2;
						product1b = product2a = Manip::copy(color2);
						//product1a = product2b = INTERPOLATE (color5, color6);
						product1a = product2b = Interpolate_2xSaI(color5, color6);
					} else if (r < 0) {
						//product2b = product1a = color5;
						product2b = product1a = Manip::copy(color5);
						//product1b = product2a = INTERPOLATE (color5, color6);
						product1b = product2a = Interpolate_2xSaI(color5, color6);
					} else {
						//product2b = product1a = color5;
						product2b = product1a = Manip::copy(color5);
						//product1b = product2a = color2;
						product1b = product2a = Manip::copy(color2);
					}
				} else {
					//product2b = product1a = INTERPOLATE (color2, color6);
					//product2b = Q_INTERPOLATE (color3, color3, color3, product2b);
					//product1a = Q_INTERPOLATE (color5, color5, color5, product1a);
					product2b = OInterpolate_2xSaI(color3, color2, color6);
					product1a = OInterpolate_2xSaI(color5, color6, color2);

					//product2a = product1b = INTERPOLATE (color5, color3);
					//product2a = Q_INTERPOLATE (color2, color2, color2, product2a);
					//product1b = Q_INTERPOLATE (color6, color6, color6, product1b);
					product2a = OInterpolate_2xSaI(color2, color5, color3);
					product1b = OInterpolate_2xSaI(color6, color5, color3);
				}

				*dP = product1a;
				*(dP + 1) = product1b;
				*(dP + dline_pixels) = product2a;
				*(dP + dline_pixels + 1) = product2b;

				bP += 1;
				dP += 2;

			}
			srcPtr += sline_pixels;
			dstPtr += 2 * dline_pixels;
		}

	}

	static bool Scale2xSaI(Texture *tex, int32 sx, int32 sy, int32 sw, int32 sh,
	                       uint8 *pixel, int32 dw, int32 dh, int32 pitch, bool clamp_src) {
		if (sw * 2 != dw || sh * 2 != dh) return false;

		if (clamp_src) {
			Scale_2xSaI(reinterpret_cast<uintS *>(tex->buffer) + sx + sy * tex->width,
			            0, 0, sw, sh, tex->width, sh,
			            reinterpret_cast<uintX *>(pixel), pitch / sizeof(uintX));
		} else {
			Scale_2xSaI(reinterpret_cast<uintS *>(tex->buffer),
			            sx, sy, sw, sh, tex->width, tex->height,
			            reinterpret_cast<uintX *>(pixel), pitch / sizeof(uintX));
		}
		return true;
	}

	static bool ScaleSuper2xSaI(Texture *tex, int32 sx, int32 sy, int32 sw, int32 sh,
	                            uint8 *pixel, int32 dw, int32 dh, int32 pitch, bool clamp_src) {
		if (sw * 2 != dw || sh * 2 != dh) return false;

		if (clamp_src) {
			Scale_Super2xSaI(reinterpret_cast<uintS *>(tex->buffer) + sx + sy * tex->width,
			                 0, 0, sw, sh, tex->width, sh,
			                 reinterpret_cast<uintX *>(pixel), pitch / sizeof(uintX));
		} else {
			Scale_Super2xSaI(reinterpret_cast<uintS *>(tex->buffer),
			                 sx, sy, sw, sh, tex->width, tex->height,
			                 reinterpret_cast<uintX *>(pixel), pitch / sizeof(uintX));
		}
		return true;
	}

	static bool ScaleSuperEagle(Texture *tex, int32 sx, int32 sy, int32 sw, int32 sh,
	                            uint8 *pixel, int32 dw, int32 dh, int32 pitch, bool clamp_src) {
		if (sw * 2 != dw || sh * 2 != dh) return false;

		if (clamp_src) {
			Scale_SuperEagle(reinterpret_cast<uintS *>(tex->buffer) + sx + sy * tex->width,
			                 0, 0, sw, sh, tex->width, sh,
			                 reinterpret_cast<uintX *>(pixel), pitch / sizeof(uintX));
		} else {
			Scale_SuperEagle(reinterpret_cast<uintS *>(tex->buffer),
			                 sx, sy, sw, sh, tex->width, tex->height,
			                 reinterpret_cast<uintX *>(pixel), pitch / sizeof(uintX));
		}
		return true;
	}

};  // Class

//
// 2xSaI
//
_2xSaIScaler::_2xSaIScaler() : Scaler() {
	Scale16Nat = _2xSaIScalerInternal<uint16, Manip_Nat2Nat_16, uint16>::Scale2xSaI;
	Scale16Sta = _2xSaIScalerInternal<uint16, Manip_Sta2Nat_16, uint32>::Scale2xSaI;

	Scale32Nat = _2xSaIScalerInternal<uint32, Manip_Nat2Nat_32, uint32>::Scale2xSaI;
	Scale32Sta = _2xSaIScalerInternal<uint32, Manip_Sta2Nat_32, uint32>::Scale2xSaI;
	Scale32_A888 = _2xSaIScalerInternal<uint32, Manip_32_A888, uint32>::Scale2xSaI;
	Scale32_888A = _2xSaIScalerInternal<uint32, Manip_32_888A, uint32>::Scale2xSaI;
}

uint32 _2xSaIScaler::ScaleBits() const {
	return 1 << 2;
}
bool _2xSaIScaler::ScaleArbitrary() const {
	return false;
}

const char *_2xSaIScaler::ScalerName() const {
	return "2xSaI";
}
const char *_2xSaIScaler::ScalerDesc() const {
	return "2xSaI Scaling Filter";
}
const char *_2xSaIScaler::ScalerCopyright() const {
	return "Copyright (c) 1999-2001 Derek Liauw Kie Fa";
}

const _2xSaIScaler _2xSaI_scaler;

//
// Super2xSaI
//
Super2xSaIScaler::Super2xSaIScaler() : Scaler() {
	Scale16Nat = _2xSaIScalerInternal<uint16, Manip_Nat2Nat_16, uint16>::ScaleSuper2xSaI;
	Scale16Sta = _2xSaIScalerInternal<uint16, Manip_Sta2Nat_16, uint32>::ScaleSuper2xSaI;

	Scale32Nat = _2xSaIScalerInternal<uint32, Manip_Nat2Nat_32, uint32>::ScaleSuper2xSaI;
	Scale32Sta = _2xSaIScalerInternal<uint32, Manip_Sta2Nat_32, uint32>::ScaleSuper2xSaI;
	Scale32_A888 = _2xSaIScalerInternal<uint32, Manip_32_A888, uint32>::ScaleSuper2xSaI;
	Scale32_888A = _2xSaIScalerInternal<uint32, Manip_32_888A, uint32>::ScaleSuper2xSaI;
}

uint32 Super2xSaIScaler::ScaleBits() const {
	return 1 << 2;
}
bool Super2xSaIScaler::ScaleArbitrary() const {
	return false;
}

const char *Super2xSaIScaler::ScalerName() const {
	return "Super2xSaI";
}
const char *Super2xSaIScaler::ScalerDesc() const {
	return "Super2xSaI Scaling Filter";
}
const char *Super2xSaIScaler::ScalerCopyright() const {
	return "Copyright (c) 1999-2001 Derek Liauw Kie Fa";
}

const Super2xSaIScaler Super2xSaI_scaler;

//
// SuperEagle
//
SuperEagleScaler::SuperEagleScaler() : Scaler() {
	Scale16Nat = _2xSaIScalerInternal<uint16, Manip_Nat2Nat_16, uint16>::ScaleSuperEagle;
	Scale16Sta = _2xSaIScalerInternal<uint16, Manip_Sta2Nat_16, uint32>::ScaleSuperEagle;

	Scale32Nat = _2xSaIScalerInternal<uint32, Manip_Nat2Nat_32, uint32>::ScaleSuperEagle;
	Scale32Sta = _2xSaIScalerInternal<uint32, Manip_Sta2Nat_32, uint32>::ScaleSuperEagle;
	Scale32_A888 = _2xSaIScalerInternal<uint32, Manip_32_A888, uint32>::ScaleSuperEagle;
	Scale32_888A = _2xSaIScalerInternal<uint32, Manip_32_888A, uint32>::ScaleSuperEagle;
}

uint32 SuperEagleScaler::ScaleBits() const {
	return 1 << 2;
}
bool SuperEagleScaler::ScaleArbitrary() const {
	return false;
}

const char *SuperEagleScaler::ScalerName() const {
	return "SuperEagle";
}
const char *SuperEagleScaler::ScalerDesc() const {
	return "SuperEagle Scaling Filter";
}
const char *SuperEagleScaler::ScalerCopyright() const {
	return "Copyright (c) 1999-2001 Derek Liauw Kie Fa";
}

const SuperEagleScaler SuperEagle_scaler;


#ifdef COMPILE_GAMMA_CORRECT_SCALERS

//
// Gamma Corrected 2xSaI
//

GC_2xSaIScaler::GC_2xSaIScaler() : Scaler() {
	Scale16Nat = _2xSaIScalerInternal<uint16, Manip_Nat2Nat_16_GC, uint16>::Scale2xSaI;
	Scale16Sta = _2xSaIScalerInternal<uint16, Manip_Sta2Nat_16_GC, uint32>::Scale2xSaI;

	Scale32Nat = _2xSaIScalerInternal<uint32, Manip_Nat2Nat_32_GC, uint32>::Scale2xSaI;
	Scale32Sta = _2xSaIScalerInternal<uint32, Manip_Sta2Nat_32_GC, uint32>::Scale2xSaI;
	Scale32_A888 = _2xSaIScalerInternal<uint32, Manip_32_A888_GC, uint32>::Scale2xSaI;
	Scale32_888A = _2xSaIScalerInternal<uint32, Manip_32_888A_GC, uint32>::Scale2xSaI;

}

uint32 GC_2xSaIScaler::ScaleBits() const {
	return 1 << 2;
}
bool GC_2xSaIScaler::ScaleArbitrary() const {
	return false;
}

const char *GC_2xSaIScaler::ScalerName() const {
	return "GC-2xSaI";
}
const char *GC_2xSaIScaler::ScalerDesc() const {
	return "Gamma 2.2 Correct 2xSaI Scaling Filter";
}
const char *GC_2xSaIScaler::ScalerCopyright() const {
	return "Copyright (c) 1999-2001 Derek Liauw Kie Fa";
}

const GC_2xSaIScaler GC_2xSaI_scaler;


//
// Gamma Corrected Super2xSaI
//

GC_Super2xSaIScaler::GC_Super2xSaIScaler() : Scaler() {
	Scale16Nat = _2xSaIScalerInternal<uint16, Manip_Nat2Nat_16_GC, uint16>::ScaleSuper2xSaI;
	Scale16Sta = _2xSaIScalerInternal<uint16, Manip_Sta2Nat_16_GC, uint32>::ScaleSuper2xSaI;

	Scale32Nat = _2xSaIScalerInternal<uint32, Manip_Nat2Nat_32_GC, uint32>::ScaleSuper2xSaI;
	Scale32Sta = _2xSaIScalerInternal<uint32, Manip_Sta2Nat_32_GC, uint32>::ScaleSuper2xSaI;
	Scale32_A888 = _2xSaIScalerInternal<uint32, Manip_32_A888_GC, uint32>::ScaleSuper2xSaI;
	Scale32_888A = _2xSaIScalerInternal<uint32, Manip_32_888A_GC, uint32>::ScaleSuper2xSaI;

}

uint32 GC_Super2xSaIScaler::ScaleBits() const {
	return 1 << 2;
}
bool GC_Super2xSaIScaler::ScaleArbitrary() const {
	return false;
}

const char *GC_Super2xSaIScaler::ScalerName() const {
	return "GC-Super2xSaI";
}
const char *GC_Super2xSaIScaler::ScalerDesc() const {
	return "Gamma 2.2 Correct Super2xSaI Scaling Filter";
}
const char *GC_Super2xSaIScaler::ScalerCopyright() const {
	return "Copyright (c) 1999-2001 Derek Liauw Kie Fa";
}

const GC_Super2xSaIScaler GC_Super2xSaI_scaler;


//
// Gamma Corrected SuperEagle
//
GC_SuperEagleScaler::GC_SuperEagleScaler() : Scaler() {
	Scale16Nat = _2xSaIScalerInternal<uint16, Manip_Nat2Nat_16_GC, uint16>::ScaleSuperEagle;
	Scale16Sta = _2xSaIScalerInternal<uint16, Manip_Sta2Nat_16_GC, uint32>::ScaleSuperEagle;

	Scale32Nat = _2xSaIScalerInternal<uint32, Manip_Nat2Nat_32_GC, uint32>::ScaleSuperEagle;
	Scale32Sta = _2xSaIScalerInternal<uint32, Manip_Sta2Nat_32_GC, uint32>::ScaleSuperEagle;
	Scale32_A888 = _2xSaIScalerInternal<uint32, Manip_32_A888_GC, uint32>::ScaleSuperEagle;
	Scale32_888A = _2xSaIScalerInternal<uint32, Manip_32_888A_GC, uint32>::ScaleSuperEagle;
}

uint32 GC_SuperEagleScaler::ScaleBits() const {
	return 1 << 2;
}
bool GC_SuperEagleScaler::ScaleArbitrary() const {
	return false;
}

const char *GC_SuperEagleScaler::ScalerName() const {
	return "GC-SuperEagle";
}
const char *GC_SuperEagleScaler::ScalerDesc() const {
	return "Gamma 2.2 Correct SuperEagle Scaling Filter";
}
const char *GC_SuperEagleScaler::ScalerCopyright() const {
	return "Copyright (c) 1999-2001 Derek Liauw Kie Fa";
}

const GC_SuperEagleScaler GC_SuperEagle_scaler;

#endif

} // End of namespace Pentagram
} // End of namespace Ultima8
