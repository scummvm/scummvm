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

#include "stdafx.h"
#include "common/scummsys.h"
#include "common/scaler.h"
#include "common/util.h"


// TODO: get rid of the colorMask etc. variables and instead use templates.
// This should give a respectable boost, since variable access (i.e. memory reads)
// in the innermost loops of our operations would work with constant data instead.
// That should help the inliner; reduce memory access; thus improve cache efficeny
// etc. The drawback will be that each scaler will exist twice, once for 555 and
// once for 555, resulting in the object file being twice as big (but thanks to
// templates, no source code would be duplicated.

static uint32 colorMask = 0xF7DEF7DE;
static uint32 lowPixelMask = 0x08210821;
static uint32 qcolorMask = 0xE79CE79C;
static uint32 qlowpixelMask = 0x18631863;
static uint32 redblueMask = 0xF81F;
static uint32 redMask = 0xF800;
static uint32 greenMask = 0x07E0;
static uint32 blueMask = 0x001F;

static const uint16 dotmatrix_565[16] = {
	0x01E0, 0x0007, 0x3800, 0x0000,
	0x39E7, 0x0000, 0x39E7, 0x0000,
	0x3800, 0x0000, 0x01E0, 0x0007,
	0x39E7, 0x0000, 0x39E7, 0x0000
};
static const uint16 dotmatrix_555[16] = {
	0x00E0, 0x0007, 0x1C00, 0x0000,
	0x1CE7, 0x0000, 0x1CE7, 0x0000,
	0x1C00, 0x0000, 0x00E0, 0x0007,
	0x1CE7, 0x0000, 0x1CE7, 0x0000
};
static const uint16 *dotmatrix;

static void InitLUTs(uint32 BitFormat);

void InitScalers(uint32 BitFormat) {
	if (BitFormat == 565) {
		colorMask = 0xF7DEF7DE;
		lowPixelMask = 0x08210821;
		qcolorMask = 0xE79CE79C;
		qlowpixelMask = 0x18631863;
		redblueMask = 0xF81F;
		redMask = 0xF800;
		greenMask = 0x07E0;
		blueMask = 0x001F;
		dotmatrix = dotmatrix_565;
	} else if (BitFormat == 555) {
		colorMask = 0x7BDE7BDE;
		lowPixelMask = 0x04210421;
		qcolorMask = 0x739C739C;
		qlowpixelMask = 0x0C630C63;
		redblueMask = 0x7C1F;
		redMask = 0x7C00;
		greenMask = 0x03E0;
		blueMask = 0x001F;
		dotmatrix = dotmatrix_555;
	} else {
		error("Unknwon bit format %d\n", BitFormat);
	}

	InitLUTs(BitFormat);
}

static inline int GetResult(uint32 A, uint32 B, uint32 C, uint32 D) { 
	const bool ac = (A==C);
	const bool bc = (B==C);
	const int x1 = ac;
	const int y1 = (bc & !ac);
	const bool ad = (A==D);
	const bool bd = (B==D);
	const int x2 = ad;
	const int y2 = (bd & !ad);
	const int x = x1+x2;
	const int y = y1+y2;
	static const int rmap[3][3] = {
			{0, 0, -1},
			{0, 0, -1},
			{1, 1,  0}
		};
	return rmap[y][x];
}

static inline uint32 INTERPOLATE(uint32 A, uint32 B) {
	if (A != B) {
		return (((A & colorMask) >> 1) + ((B & colorMask) >> 1) + (A & B & lowPixelMask));
	} else
		return A;
}

static inline uint32 Q_INTERPOLATE(uint32 A, uint32 B, uint32 C, uint32 D) {
	register uint32 x = ((A & qcolorMask) >> 2) + ((B & qcolorMask) >> 2) + ((C & qcolorMask) >> 2) + ((D & qcolorMask) >> 2);
	register uint32 y = ((A & qlowpixelMask) + (B & qlowpixelMask) + (C & qlowpixelMask) + (D & qlowpixelMask)) >> 2;

	y &= qlowpixelMask;
	return x + y;
}

void Super2xSaI(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	const uint16 *bP;
	uint16 *dP;
	const uint32 nextlineSrc = srcPitch >> 1;

	while (height--) {
		bP = (const uint16 *)srcPtr;
		dP = (uint16 *)dstPtr;

		for (int i = 0; i < width; ++i) {
			uint32 color4, color5, color6;
			uint32 color1, color2, color3;
			uint32 colorA0, colorA1, colorA2, colorA3;
			uint32 colorB0, colorB1, colorB2, colorB3;
			uint32 colorS1, colorS2;
			uint32 product1a, product1b, product2a, product2b;

//---------------------------------------    B1 B2
//                                         4  5  6 S2
//                                         1  2  3 S1
//                                           A1 A2

			colorB0 = *(bP - nextlineSrc - 1);
			colorB1 = *(bP - nextlineSrc);
			colorB2 = *(bP - nextlineSrc + 1);
			colorB3 = *(bP - nextlineSrc + 2);

			color4 = *(bP - 1);
			color5 = *(bP);
			color6 = *(bP + 1);
			colorS2 = *(bP + 2);

			color1 = *(bP + nextlineSrc - 1);
			color2 = *(bP + nextlineSrc);
			color3 = *(bP + nextlineSrc + 1);
			colorS1 = *(bP + nextlineSrc + 2);

			colorA0 = *(bP + 2 * nextlineSrc - 1);
			colorA1 = *(bP + 2 * nextlineSrc);
			colorA2 = *(bP + 2 * nextlineSrc + 1);
			colorA3 = *(bP + 2 * nextlineSrc + 2);

//--------------------------------------
			if (color2 == color6 && color5 != color3) {
				product2b = product1b = color2;
			} else if (color5 == color3 && color2 != color6) {
				product2b = product1b = color5;
			} else if (color5 == color3 && color2 == color6) {
				register int r = 0;

				r += GetResult(color6, color5, color1, colorA1);
				r += GetResult(color6, color5, color4, colorB1);
				r += GetResult(color6, color5, colorA2, colorS1);
				r += GetResult(color6, color5, colorB2, colorS2);

				if (r > 0)
					product2b = product1b = color6;
				else if (r < 0)
					product2b = product1b = color5;
				else {
					product2b = product1b = INTERPOLATE(color5, color6);
				}
			} else {
				if (color6 == color3 && color3 == colorA1 && color2 != colorA2 && color3 != colorA0)
					product2b = Q_INTERPOLATE(color3, color3, color3, color2);
				else if (color5 == color2 && color2 == colorA2 && colorA1 != color3 && color2 != colorA3)
					product2b = Q_INTERPOLATE(color2, color2, color2, color3);
				else
					product2b = INTERPOLATE(color2, color3);

				if (color6 == color3 && color6 == colorB1 && color5 != colorB2 && color6 != colorB0)
					product1b = Q_INTERPOLATE(color6, color6, color6, color5);
				else if (color5 == color2 && color5 == colorB2 && colorB1 != color6 && color5 != colorB3)
					product1b = Q_INTERPOLATE(color6, color5, color5, color5);
				else
					product1b = INTERPOLATE(color5, color6);
			}

			if (color5 == color3 && color2 != color6 && color4 == color5 && color5 != colorA2)
				product2a = INTERPOLATE(color2, color5);
			else if (color5 == color1 && color6 == color5 && color4 != color2 && color5 != colorA0)
				product2a = INTERPOLATE(color2, color5);
			else
				product2a = color2;

			if (color2 == color6 && color5 != color3 && color1 == color2 && color2 != colorB2)
				product1a = INTERPOLATE(color2, color5);
			else if (color4 == color2 && color3 == color2 && color1 != color5 && color2 != colorB0)
				product1a = INTERPOLATE(color2, color5);
			else
				product1a = color5;

			*(dP + 0) = (uint16) product1a;
			*(dP + 1) = (uint16) product1b;
			*(dP + dstPitch/2 + 0) = (uint16) product2a;
			*(dP + dstPitch/2 + 1) = (uint16) product2b;

			bP += 1;
			dP += 2;
		}

		srcPtr += srcPitch;
		dstPtr += dstPitch * 2;
	}
}

void SuperEagle(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	const uint16 *bP;
	uint16 *dP;
	const uint32 nextlineSrc = srcPitch >> 1;

	while (height--) {
		bP = (const uint16 *)srcPtr;
		dP = (uint16 *)dstPtr;
		for (int i = 0; i < width; ++i) {
			uint32 color4, color5, color6;
			uint32 color1, color2, color3;
			uint32 colorA1, colorA2, colorB1, colorB2, colorS1, colorS2;
			uint32 product1a, product1b, product2a, product2b;

			colorB1 = *(bP - nextlineSrc);
			colorB2 = *(bP - nextlineSrc + 1);

			color4 = *(bP - 1);
			color5 = *(bP);
			color6 = *(bP + 1);
			colorS2 = *(bP + 2);

			color1 = *(bP + nextlineSrc - 1);
			color2 = *(bP + nextlineSrc);
			color3 = *(bP + nextlineSrc + 1);
			colorS1 = *(bP + nextlineSrc + 2);

			colorA1 = *(bP + 2 * nextlineSrc);
			colorA2 = *(bP + 2 * nextlineSrc + 1);

			// --------------------------------------
			if (color5 != color3)
			{
				if (color2 == color6)
				{
					product1b = product2a = color2;
					if ((color1 == color2) || (color6 == colorB2)) {
						product1a = INTERPOLATE(color2, color5);
						product1a = INTERPOLATE(color2, product1a);
					} else {
						product1a = INTERPOLATE(color5, color6);
					}

					if ((color6 == colorS2) || (color2 == colorA1)) {
						product2b = INTERPOLATE(color2, color3);
						product2b = INTERPOLATE(color2, product2b);
					} else {
						product2b = INTERPOLATE(color2, color3);
					}
				}
				else
				{
					product2b = product1a = INTERPOLATE(color2, color6);
					product2b = Q_INTERPOLATE(color3, color3, color3, product2b);
					product1a = Q_INTERPOLATE(color5, color5, color5, product1a);

					product2a = product1b = INTERPOLATE(color5, color3);
					product2a = Q_INTERPOLATE(color2, color2, color2, product2a);
					product1b = Q_INTERPOLATE(color6, color6, color6, product1b);
				}
			}
			else //if (color5 == color3)
			{
				if (color2 != color6)
				{
					product2b = product1a = color5;

					if ((colorB1 == color5) || (color3 == colorS1)) {
						product1b = INTERPOLATE(color5, color6);
						product1b = INTERPOLATE(color5, product1b);
					} else {
						product1b = INTERPOLATE(color5, color6);
					}

					if ((color3 == colorA2) || (color4 == color5)) {
						product2a = INTERPOLATE(color5, color2);
						product2a = INTERPOLATE(color5, product2a);
					} else {
						product2a = INTERPOLATE(color2, color3);
					}
				}
				else	//if (color2 != color6)
				{
					register int r = 0;

					r += GetResult(color6, color5, color1, colorA1);
					r += GetResult(color6, color5, color4, colorB1);
					r += GetResult(color6, color5, colorA2, colorS1);
					r += GetResult(color6, color5, colorB2, colorS2);

					if (r > 0) {
						product1b = product2a = color2;
						product1a = product2b = INTERPOLATE(color5, color6);
					} else if (r < 0) {
						product2b = product1a = color5;
						product1b = product2a = INTERPOLATE(color5, color6);
					} else {
						product2b = product1a = color5;
						product1b = product2a = color2;
					}
				}
			}

			*(dP + 0) = (uint16) product1a;
			*(dP + 1) = (uint16) product1b;
			*(dP + dstPitch/2 + 0) = (uint16) product2a;
			*(dP + dstPitch/2 + 1) = (uint16) product2b;

			bP += 1;
			dP += 2;
		}

		srcPtr += srcPitch;
		dstPtr += dstPitch * 2;
	}
}

void _2xSaI(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	const uint16 *bP;
	uint16 *dP;
	const uint32 nextlineSrc = srcPitch >> 1;

	while (height--) {
		bP = (const uint16 *)srcPtr;
		dP = (uint16 *)dstPtr;

		for (int i = 0; i < width; ++i) {

			register uint32 colorA, colorB;
			uint32 colorC, colorD,
				colorE, colorF, colorG, colorH, colorI, colorJ, colorK, colorL, colorM, colorN, colorO, colorP;
			uint32 product, product1, product2;

//---------------------------------------
// Map of the pixels:                    I|E F|J
//                                       G|A B|K
//                                       H|C D|L
//                                       M|N O|P
			colorI = *(bP - nextlineSrc - 1);
			colorE = *(bP - nextlineSrc);
			colorF = *(bP - nextlineSrc + 1);
			colorJ = *(bP - nextlineSrc + 2);

			colorG = *(bP - 1);
			colorA = *(bP);
			colorB = *(bP + 1);
			colorK = *(bP + 2);

			colorH = *(bP + nextlineSrc - 1);
			colorC = *(bP + nextlineSrc);
			colorD = *(bP + nextlineSrc + 1);
			colorL = *(bP + nextlineSrc + 2);

			colorM = *(bP + 2 * nextlineSrc - 1);
			colorN = *(bP + 2 * nextlineSrc);
			colorO = *(bP + 2 * nextlineSrc + 1);
			colorP = *(bP + 2 * nextlineSrc + 2);

			if ((colorA == colorD) && (colorB != colorC)) {
				if (((colorA == colorE) && (colorB == colorL)) ||
					((colorA == colorC) && (colorA == colorF) && (colorB != colorE) && (colorB == colorJ))) {
					product = colorA;
				} else {
					product = INTERPOLATE(colorA, colorB);
				}

				if (((colorA == colorG) && (colorC == colorO)) ||
					((colorA == colorB) && (colorA == colorH) && (colorG != colorC)  && (colorC == colorM))) {
					product1 = colorA;
				} else {
					product1 = INTERPOLATE(colorA, colorC);
				}
				product2 = colorA;
			} else if ((colorB == colorC) && (colorA != colorD)) {
				if (((colorB == colorF) && (colorA == colorH)) ||
					((colorB == colorE) && (colorB == colorD) && (colorA != colorF) && (colorA == colorI))) {
					product = colorB;
				} else {
					product = INTERPOLATE(colorA, colorB);
				}

				if (((colorC == colorH) && (colorA == colorF)) ||
					((colorC == colorG) && (colorC == colorD) && (colorA != colorH) && (colorA == colorI))) {
					product1 = colorC;
				} else {
					product1 = INTERPOLATE(colorA, colorC);
				}
				product2 = colorB;
			} else if ((colorA == colorD) && (colorB == colorC)) {
				if (colorA == colorB) {
					product = colorA;
					product1 = colorA;
					product2 = colorA;
				} else {
					register int r = 0;

					product1 = INTERPOLATE(colorA, colorC);
					product = INTERPOLATE(colorA, colorB);

					r += GetResult(colorA, colorB, colorG, colorE);
					r -= GetResult(colorB, colorA, colorK, colorF);
					r -= GetResult(colorB, colorA, colorH, colorN);
					r += GetResult(colorA, colorB, colorL, colorO);

					if (r > 0)
						product2 = colorA;
					else if (r < 0)
						product2 = colorB;
					else {
						product2 = Q_INTERPOLATE(colorA, colorB, colorC, colorD);
					}
				}
			} else {
				product2 = Q_INTERPOLATE(colorA, colorB, colorC, colorD);

				if ((colorA == colorC) && (colorA == colorF)
						&& (colorB != colorE) && (colorB == colorJ)) {
					product = colorA;
				} else if ((colorB == colorE) && (colorB == colorD)
									 && (colorA != colorF) && (colorA == colorI)) {
					product = colorB;
				} else {
					product = INTERPOLATE(colorA, colorB);
				}

				if ((colorA == colorB) && (colorA == colorH)
						&& (colorG != colorC) && (colorC == colorM)) {
					product1 = colorA;
				} else if ((colorC == colorG) && (colorC == colorD)
									 && (colorA != colorH) && (colorA == colorI)) {
					product1 = colorC;
				} else {
					product1 = INTERPOLATE(colorA, colorC);
				}
			}

			*(dP + 0) = (uint16) colorA;
			*(dP + 1) = (uint16) product;
			*(dP + dstPitch/2 + 0) = (uint16) product1;
			*(dP + dstPitch/2 + 1) = (uint16) product2;

			bP += 1;
			dP += 2;
		}

		srcPtr += srcPitch;
		dstPtr += dstPitch * 2;
	}
}

void AdvMame2x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							 int width, int height) {
	const unsigned int nextlineSrc = srcPitch / sizeof(uint16);
	const uint16 *p = (const uint16 *)srcPtr;

	const unsigned int nextlineDst = dstPitch / sizeof(uint16);
	uint16 *q = (uint16 *)dstPtr;
	
	uint16 A, B, C;
	uint16 D, E, F;
	uint16 G, H, I;

	while (height--) {
		B = *(p - 1 - nextlineSrc);
		E = *(p - 1);
		H = *(p - 1 + nextlineSrc);
		C = *(p - nextlineSrc);
		F = *(p);
		I = *(p + nextlineSrc);
		for (int i = 0; i < width; ++i) {
			p++;
			A = B; B = C; C = *(p - nextlineSrc);
			D = E; E = F; F = *(p);
			G = H; H = I; I = *(p + nextlineSrc);

			*(q) = D == B && B != F && D != H ? D : E;
			*(q + 1) = B == F && B != D && F != H ? F : E;
			*(q + nextlineDst) = D == H && D != B && H != F ? D : E;
			*(q + nextlineDst + 1) = H == F && D != H && B != F ? F : E;
			q += 2;
		}
		p += nextlineSrc - width;
		q += (nextlineDst - width) << 1;
	}
}

void AdvMame3x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							 int width, int height) {
	const unsigned int nextlineSrc = srcPitch / sizeof(uint16);
	const uint16 *p = (const uint16 *)srcPtr;

	const unsigned int nextlineDst = dstPitch / sizeof(uint16);
	uint16 *q = (uint16 *)dstPtr;
	
	uint16 A, B, C;
	uint16 D, E, F;
	uint16 G, H, I;

	while (height--) {
		B = *(p - 1 - nextlineSrc);
		E = *(p - 1);
		H = *(p - 1 + nextlineSrc);
		C = *(p - nextlineSrc);
		F = *(p);
		I = *(p + nextlineSrc);
		for (int i = 0; i < width; ++i) {
			p++;
			A = B; B = C; C = *(p - nextlineSrc);
			D = E; E = F; F = *(p);
			G = H; H = I; I = *(p + nextlineSrc);

			*(q) = D == B && B != F && D != H ? D : E;
			*(q + 1) = E;
			*(q + 2) = B == F && B != D && F != H ? F : E;
			*(q + nextlineDst) = E;
			*(q + nextlineDst + 1) = E;
			*(q + nextlineDst + 2) = E;
			*(q + 2 * nextlineDst) = D == H && D != B && H != F ? D : E;
			*(q + 2 * nextlineDst + 1) = E;
			*(q + 2 * nextlineDst + 2) = H == F && D != H && B != F ? F : E;
			q += 3;
		}
		p += nextlineSrc - width;
		q += (nextlineDst - width) * 3;
	}
}

void Normal1x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							int width, int height) {
	while (height--) {
		memcpy(dstPtr, srcPtr, 2 * width);
		srcPtr += srcPitch;
		dstPtr += dstPitch;
	}
}

void Normal2x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							int width, int height) {
	uint8 *r;

	while (height--) {
		r = dstPtr;
		for (int i = 0; i < width; ++i, r += 4) {
			uint16 color = *(((const uint16 *)srcPtr) + i);

			*(uint16 *)(r + 0) = color;
			*(uint16 *)(r + 2) = color;
			*(uint16 *)(r + 0 + dstPitch) = color;
			*(uint16 *)(r + 2 + dstPitch) = color;
		}
		srcPtr += srcPitch;
		dstPtr += dstPitch << 1;
	}
}

void Normal3x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							int width, int height) {
	uint8 *r;
	uint32 dstPitch2 = dstPitch * 2;
	uint32 dstPitch3 = dstPitch * 3;

	while (height--) {
		r = dstPtr;
		for (int i = 0; i < width; ++i, r += 6) {
			uint16 color = *(((const uint16 *)srcPtr) + i);

			*(uint16 *)(r + 0) = color;
			*(uint16 *)(r + 2) = color;
			*(uint16 *)(r + 4) = color;
			*(uint16 *)(r + 0 + dstPitch) = color;
			*(uint16 *)(r + 2 + dstPitch) = color;
			*(uint16 *)(r + 4 + dstPitch) = color;
			*(uint16 *)(r + 0 + dstPitch2) = color;
			*(uint16 *)(r + 2 + dstPitch2) = color;
			*(uint16 *)(r + 4 + dstPitch2) = color;
		}
		srcPtr += srcPitch;
		dstPtr += dstPitch3;
	}
}

void TV2x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, 
					int width, int height) {
	const unsigned int nextlineSrc = srcPitch / sizeof(uint16);
	const uint16 *p = (const uint16 *)srcPtr;

	const unsigned int nextlineDst = dstPitch / sizeof(uint16);
	uint16 *q = (uint16 *)dstPtr;

	while(height--) {
		for (int i = 0, j = 0; i < width; ++i, j += 2) {
			uint16 p1 = *(p + i);
			uint32 pi;

			pi = (((p1 & redblueMask) * 7) >> 3) & redblueMask;
			pi |= (((p1 & greenMask) * 7) >> 3) & greenMask;

			*(q + j) = p1;
			*(q + j + 1) = p1;
			*(q + j + nextlineDst) = (uint16)pi;
			*(q + j + nextlineDst + 1) = (uint16)pi;
		}
		p += nextlineSrc;
		q += nextlineDst << 1;
	}
}

static inline uint16 DOT_16(uint16 c, int j, int i) {
  return c - ((c >> 2) & *(dotmatrix + ((j & 3) << 2) + (i & 3)));
}

void DotMatrix(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
					int width, int height)
{
	const unsigned int nextlineSrc = srcPitch / sizeof(uint16);
	const uint16 *p = (const uint16 *)srcPtr;

	const unsigned int nextlineDst = dstPitch / sizeof(uint16);
	uint16 *q = (uint16 *)dstPtr;

	for (int j = 0, jj = 0; j < height; ++j, jj += 2) {
		for (int i = 0, ii = 0; i < width; ++i, ii += 2) {
			uint16 c = *(p + i);
			*(q + ii) = DOT_16(c, jj, ii);
			*(q + ii + 1) = DOT_16(c, jj, ii + 1);
			*(q + ii + nextlineDst) = DOT_16(c, jj + 1, ii);
			*(q + ii + nextlineDst + 1) = DOT_16(c, jj + 1, ii + 1);
		}
		p += nextlineSrc;
		q += nextlineDst << 1;
	}
}

//
// The HQ3x filter (C++ version) by Maxim Stepin (see http://www.hiend3d.com/hq3x.html).
// Adapted to 16 bit output and optimized by Max Horn
//
static int   RGBtoYUV[65536];

#define INTERPOLATE_3_1(x, y)	Q_INTERPOLATE(x, x, x, y)

#define INTERPOLATE_2_1_1(x, y, z)	Q_INTERPOLATE(x, x, y, z)

static inline uint16 INTERPOLATE_7_1(uint16 w1, uint16 w2) {
	return ((((w1 & redblueMask) * 7 + (w2 & redblueMask)) >> 3)  & redblueMask) |
	       ((((w1 & greenMask) * 7 + (w2 & greenMask)) >> 3)  & greenMask);
}

static inline uint16 INTERPOLATE_2_7_7(uint16 w1, uint16 w2, uint16 w3) {
	return ((((w1 & redblueMask) * 2 + ((w2 & redblueMask) + (w3 & redblueMask)) * 7) >> 4)  & redblueMask) |
	       ((((w1 & greenMask) * 2 + ((w2 & greenMask) + (w3 & greenMask)) * 7) >> 4)  & greenMask);
}

#define INTERPOLATE_1_1(x, y) INTERPOLATE(x, y)

#define PIXEL00_1M  *(q) = INTERPOLATE_3_1(w[5], w[1]);
#define PIXEL00_1U  *(q) = INTERPOLATE_3_1(w[5], w[2]);
#define PIXEL00_1L  *(q) = INTERPOLATE_3_1(w[5], w[4]);
#define PIXEL00_2   *(q) = INTERPOLATE_2_1_1(w[5], w[4], w[2]);
#define PIXEL00_4   *(q) = INTERPOLATE_2_7_7(w[5], w[4], w[2]);
#define PIXEL00_5   *(q) = INTERPOLATE_1_1(w[4], w[2]);
#define PIXEL00_C   *(q) = w[5];

#define PIXEL01_1   *(q+1) = INTERPOLATE_3_1(w[5], w[2]);
#define PIXEL01_3   *(q+1) = INTERPOLATE_7_1(w[5], w[2]);
#define PIXEL01_6   *(q+1) = INTERPOLATE_3_1(w[2], w[5]);
#define PIXEL01_C   *(q+1) = w[5];

#define PIXEL02_1M  *(q+2) = INTERPOLATE_3_1(w[5], w[3]);
#define PIXEL02_1U  *(q+2) = INTERPOLATE_3_1(w[5], w[2]);
#define PIXEL02_1R  *(q+2) = INTERPOLATE_3_1(w[5], w[6]);
#define PIXEL02_2   *(q+2) = INTERPOLATE_2_1_1(w[5], w[2], w[6]);
#define PIXEL02_4   *(q+2) = INTERPOLATE_2_7_7(w[5], w[2], w[6]);
#define PIXEL02_5   *(q+2) = INTERPOLATE_1_1(w[2], w[6]);
#define PIXEL02_C   *(q+2) = w[5];

#define PIXEL10_1   *(q+nextlineDst) = INTERPOLATE_3_1(w[5], w[4]);
#define PIXEL10_3   *(q+nextlineDst) = INTERPOLATE_7_1(w[5], w[4]);
#define PIXEL10_6   *(q+nextlineDst) = INTERPOLATE_3_1(w[4], w[5]);
#define PIXEL10_C   *(q+nextlineDst) = w[5];

#define PIXEL11     *(q+1+nextlineDst) = w[5];

#define PIXEL12_1   *(q+2+nextlineDst) = INTERPOLATE_3_1(w[5], w[6]);
#define PIXEL12_3   *(q+2+nextlineDst) = INTERPOLATE_7_1(w[5], w[6]);
#define PIXEL12_6   *(q+2+nextlineDst) = INTERPOLATE_3_1(w[6], w[5]);
#define PIXEL12_C   *(q+2+nextlineDst) = w[5];

#define PIXEL20_1M  *(q+nextlineDst2) = INTERPOLATE_3_1(w[5], w[7]);
#define PIXEL20_1D  *(q+nextlineDst2) = INTERPOLATE_3_1(w[5], w[8]);
#define PIXEL20_1L  *(q+nextlineDst2) = INTERPOLATE_3_1(w[5], w[4]);
#define PIXEL20_2   *(q+nextlineDst2) = INTERPOLATE_2_1_1(w[5], w[8], w[4]);
#define PIXEL20_4   *(q+nextlineDst2) = INTERPOLATE_2_7_7(w[5], w[8], w[4]);
#define PIXEL20_5   *(q+nextlineDst2) = INTERPOLATE_1_1(w[8], w[4]);
#define PIXEL20_C   *(q+nextlineDst2) = w[5];

#define PIXEL21_1   *(q+1+nextlineDst2) = INTERPOLATE_3_1(w[5], w[8]);
#define PIXEL21_3   *(q+1+nextlineDst2) = INTERPOLATE_7_1(w[5], w[8]);
#define PIXEL21_6   *(q+1+nextlineDst2) = INTERPOLATE_3_1(w[8], w[5]);
#define PIXEL21_C   *(q+1+nextlineDst2) = w[5];

#define PIXEL22_1M  *(q+2+nextlineDst2) = INTERPOLATE_3_1(w[5], w[9]);
#define PIXEL22_1D  *(q+2+nextlineDst2) = INTERPOLATE_3_1(w[5], w[8]);
#define PIXEL22_1R  *(q+2+nextlineDst2) = INTERPOLATE_3_1(w[5], w[6]);
#define PIXEL22_2   *(q+2+nextlineDst2) = INTERPOLATE_2_1_1(w[5], w[6], w[8]);
#define PIXEL22_4   *(q+2+nextlineDst2) = INTERPOLATE_2_7_7(w[5], w[6], w[8]);
#define PIXEL22_5   *(q+2+nextlineDst2) = INTERPOLATE_1_1(w[6], w[8]);
#define PIXEL22_C   *(q+2+nextlineDst2) = w[5];

static inline bool diffYUV(unsigned int yuv1, unsigned int yuv2) {
	static const  int   Ymask = 0x00FF0000;
	static const  int   Umask = 0x0000FF00;
	static const  int   Vmask = 0x000000FF;
	static const  int   trY   = 0x00300000;
	static const  int   trU   = 0x00000700;
	static const  int   trV   = 0x00000006;

	return
	  ( ( abs((yuv1 & Ymask) - (yuv2 & Ymask)) > trY ) ||
	    ( abs((yuv1 & Umask) - (yuv2 & Umask)) > trU ) ||
	    ( abs((yuv1 & Vmask) - (yuv2 & Vmask)) > trV ) );
}

void HQ3x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, int width, int height) {
	int  i, k;
	int  w[10];
	int  yuv[10];
  
	const unsigned int nextlineSrc = srcPitch / sizeof(uint16);
	const uint16 *p = (const uint16 *)srcPtr;

	const unsigned int nextlineDst = dstPitch / sizeof(uint16);
	const unsigned int nextlineDst2 = 2 * nextlineDst;
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

	while (height--) {
		w[2] = *(p - 1 - nextlineSrc);
		w[5] = *(p - 1);
		w[8] = *(p - 1 + nextlineSrc);

		w[3] = *(p - nextlineSrc);
		w[6] = *(p);
		w[9] = *(p + nextlineSrc);

		for (i = 0; i < width; i++) {
			p++;

			w[1] = w[2];
			w[4] = w[5];
			w[7] = w[8];

			w[2] = w[3];
			w[5] = w[6];
			w[8] = w[9];

			w[3] = *(p - nextlineSrc);
			w[6] = *(p);
			w[9] = *(p + nextlineSrc);

			int pattern = 0;
			int flag = 1;

			for (k = 1; k <= 9; k++) {
				yuv[k] = RGBtoYUV[w[k]];
			}

			for (k = 1; k <= 9; k++) {
				if (k == 5) continue;

				if (w[k] != w[5]) {
					if (diffYUV(yuv[5], yuv[k]))
						pattern |= flag;
				}
				flag <<= 1;
			}

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
				PIXEL00_2
				PIXEL01_1
				PIXEL02_2
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_2
				PIXEL21_1
				PIXEL22_2
				break;
			case 2:
			case 34:
			case 130:
			case 162:
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_2
				PIXEL21_1
				PIXEL22_2
				break;
			case 16:
			case 17:
			case 48:
			case 49:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_2
				PIXEL21_1
				PIXEL22_1M
				break;
			case 64:
			case 65:
			case 68:
			case 69:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_2
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1M
				break;
			case 8:
			case 12:
			case 136:
			case 140:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_2
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_2
				break;
			case 3:
			case 35:
			case 131:
			case 163:
				PIXEL00_1L
				PIXEL01_C
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_2
				PIXEL21_1
				PIXEL22_2
				break;
			case 6:
			case 38:
			case 134:
			case 166:
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_2
				PIXEL21_1
				PIXEL22_2
				break;
			case 20:
			case 21:
			case 52:
			case 53:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_2
				PIXEL21_1
				PIXEL22_1M
				break;
			case 144:
			case 145:
			case 176:
			case 177:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_2
				PIXEL21_1
				PIXEL22_1D
				break;
			case 192:
			case 193:
			case 196:
			case 197:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_2
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1R
				break;
			case 96:
			case 97:
			case 100:
			case 101:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_2
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1L
				PIXEL21_C
				PIXEL22_1M
				break;
			case 40:
			case 44:
			case 168:
			case 172:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_2
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_2
				break;
			case 9:
			case 13:
			case 137:
			case 141:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_2
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_2
				break;
			case 18:
			case 50:
				PIXEL00_1M
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_C
					PIXEL02_1M
					PIXEL12_C
				} else {
					PIXEL01_3
					PIXEL02_4
					PIXEL12_3
				}
				PIXEL10_1
				PIXEL11
				PIXEL20_2
				PIXEL21_1
				PIXEL22_1M
				break;
			case 80:
			case 81:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL20_1M
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL12_C
					PIXEL21_C
					PIXEL22_1M
				} else {
					PIXEL12_3
					PIXEL21_3
					PIXEL22_4
				}
				break;
			case 72:
			case 76:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_2
				PIXEL11
				PIXEL12_1
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_C
					PIXEL20_1M
					PIXEL21_C
				} else {
					PIXEL10_3
					PIXEL20_4
					PIXEL21_3
				}
				PIXEL22_1M
				break;
			case 10:
			case 138:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_1M
					PIXEL01_C
					PIXEL10_C
				} else {
					PIXEL00_4
					PIXEL01_3
					PIXEL10_3
				}
				PIXEL02_1M
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_2
				break;
			case 66:
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1M
				break;
			case 24:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1M
				break;
			case 7:
			case 39:
			case 135:
				PIXEL00_1L
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_2
				PIXEL21_1
				PIXEL22_2
				break;
			case 148:
			case 149:
			case 180:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_2
				PIXEL21_1
				PIXEL22_1D
				break;
			case 224:
			case 228:
			case 225:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_2
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1L
				PIXEL21_C
				PIXEL22_1R
				break;
			case 41:
			case 169:
			case 45:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_2
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_2
				break;
			case 22:
			case 54:
				PIXEL00_1M
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_C
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL01_3
					PIXEL02_4
					PIXEL12_3
				}
				PIXEL10_1
				PIXEL11
				PIXEL20_2
				PIXEL21_1
				PIXEL22_1M
				break;
			case 208:
			case 209:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL20_1M
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL12_C
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL12_3
					PIXEL21_3
					PIXEL22_4
				}
				break;
			case 104:
			case 108:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_2
				PIXEL11
				PIXEL12_1
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_C
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL10_3
					PIXEL20_4
					PIXEL21_3
				}
				PIXEL22_1M
				break;
			case 11:
			case 139:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_C
					PIXEL01_C
					PIXEL10_C
				} else {
					PIXEL00_4
					PIXEL01_3
					PIXEL10_3
				}
				PIXEL02_1M
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_2
				break;
			case 19:
			case 51:
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL00_1L
					PIXEL01_C
					PIXEL02_1M
					PIXEL12_C
				} else {
					PIXEL00_2
					PIXEL01_6
					PIXEL02_5
					PIXEL12_1
				}
				PIXEL10_1
				PIXEL11
				PIXEL20_2
				PIXEL21_1
				PIXEL22_1M
				break;
			case 146:
			case 178:
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_C
					PIXEL02_1M
					PIXEL12_C
					PIXEL22_1D
				} else {
					PIXEL01_1
					PIXEL02_5
					PIXEL12_6
					PIXEL22_2
				}
				PIXEL00_1M
				PIXEL10_1
				PIXEL11
				PIXEL20_2
				PIXEL21_1
				break;
			case 84:
			case 85:
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL02_1U
					PIXEL12_C
					PIXEL21_C
					PIXEL22_1M
				} else {
					PIXEL02_2
					PIXEL12_6
					PIXEL21_1
					PIXEL22_5
				}
				PIXEL00_2
				PIXEL01_1
				PIXEL10_1
				PIXEL11
				PIXEL20_1M
				break;
			case 112:
			case 113:
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL12_C
					PIXEL20_1L
					PIXEL21_C
					PIXEL22_1M
				} else {
					PIXEL12_1
					PIXEL20_2
					PIXEL21_6
					PIXEL22_5
				}
				PIXEL00_2
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				break;
			case 200:
			case 204:
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_C
					PIXEL20_1M
					PIXEL21_C
					PIXEL22_1R
				} else {
					PIXEL10_1
					PIXEL20_5
					PIXEL21_6
					PIXEL22_2
				}
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_2
				PIXEL11
				PIXEL12_1
				break;
			case 73:
			case 77:
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL00_1U
					PIXEL10_C
					PIXEL20_1M
					PIXEL21_C
				} else {
					PIXEL00_2
					PIXEL10_6
					PIXEL20_5
					PIXEL21_1
				}
				PIXEL01_1
				PIXEL02_2
				PIXEL11
				PIXEL12_1
				PIXEL22_1M
				break;
			case 42:
			case 170:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_1M
					PIXEL01_C
					PIXEL10_C
					PIXEL20_1D
				} else {
					PIXEL00_5
					PIXEL01_1
					PIXEL10_6
					PIXEL20_2
				}
				PIXEL02_1M
				PIXEL11
				PIXEL12_1
				PIXEL21_1
				PIXEL22_2
				break;
			case 14:
			case 142:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_1M
					PIXEL01_C
					PIXEL02_1R
					PIXEL10_C
				} else {
					PIXEL00_5
					PIXEL01_6
					PIXEL02_2
					PIXEL10_1
				}
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_2
				break;
			case 67:
				PIXEL00_1L
				PIXEL01_C
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1M
				break;
			case 70:
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1M
				break;
			case 28:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1M
				break;
			case 152:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1D
				break;
			case 194:
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1R
				break;
			case 98:
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1L
				PIXEL21_C
				PIXEL22_1M
				break;
			case 56:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_1M
				break;
			case 25:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1M
				break;
			case 26:
			case 31:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_C
					PIXEL10_C
				} else {
					PIXEL00_4
					PIXEL10_3
				}
				PIXEL01_C
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL02_4
					PIXEL12_3
				}
				PIXEL11
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1M
				break;
			case 82:
			case 214:
				PIXEL00_1M
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_C
					PIXEL02_C
				} else {
					PIXEL01_3
					PIXEL02_4
				}
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL21_3
					PIXEL22_4
				}
				break;
			case 88:
			case 248:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1M
				PIXEL11
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_C
					PIXEL20_C
				} else {
					PIXEL10_3
					PIXEL20_4
				}
				PIXEL21_C
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL12_C
					PIXEL22_C
				} else {
					PIXEL12_3
					PIXEL22_4
				}
				break;
			case 74:
			case 107:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_C
					PIXEL01_C
				} else {
					PIXEL00_4
					PIXEL01_3
				}
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL20_4
					PIXEL21_3
				}
				PIXEL22_1M
				break;
			case 27:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_C
					PIXEL01_C
					PIXEL10_C
				} else {
					PIXEL00_4
					PIXEL01_3
					PIXEL10_3
				}
				PIXEL02_1M
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1M
				break;
			case 86:
				PIXEL00_1M
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_C
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL01_3
					PIXEL02_4
					PIXEL12_3
				}
				PIXEL10_1
				PIXEL11
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1M
				break;
			case 216:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL20_1M
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL12_C
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL12_3
					PIXEL21_3
					PIXEL22_4
				}
				break;
			case 106:
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1M
				PIXEL11
				PIXEL12_1
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_C
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL10_3
					PIXEL20_4
					PIXEL21_3
				}
				PIXEL22_1M
				break;
			case 30:
				PIXEL00_1M
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_C
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL01_3
					PIXEL02_4
					PIXEL12_3
				}
				PIXEL10_C
				PIXEL11
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1M
				break;
			case 210:
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL20_1M
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL12_C
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL12_3
					PIXEL21_3
					PIXEL22_4
				}
				break;
			case 120:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1M
				PIXEL11
				PIXEL12_C
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_C
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL10_3
					PIXEL20_4
					PIXEL21_3
				}
				PIXEL22_1M
				break;
			case 75:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_C
					PIXEL01_C
					PIXEL10_C
				} else {
					PIXEL00_4
					PIXEL01_3
					PIXEL10_3
				}
				PIXEL02_1M
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1M
				break;
			case 29:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1M
				break;
			case 198:
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1R
				break;
			case 184:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_1D
				break;
			case 99:
				PIXEL00_1L
				PIXEL01_C
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1L
				PIXEL21_C
				PIXEL22_1M
				break;
			case 57:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_1M
				break;
			case 71:
				PIXEL00_1L
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1M
				break;
			case 156:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1D
				break;
			case 226:
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1L
				PIXEL21_C
				PIXEL22_1R
				break;
			case 60:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_1M
				break;
			case 195:
				PIXEL00_1L
				PIXEL01_C
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1R
				break;
			case 102:
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1L
				PIXEL21_C
				PIXEL22_1M
				break;
			case 153:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1D
				break;
			case 58:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_1M
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL02_1M
				} else {
					PIXEL02_2
				}
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_1M
				break;
			case 83:
				PIXEL00_1L
				PIXEL01_C
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL02_1M
				} else {
					PIXEL02_2
				}
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				PIXEL21_C
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL22_1M
				} else {
					PIXEL22_2
				}
				break;
			case 92:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL20_1M
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL22_1M
				} else {
					PIXEL22_2
				}
				break;
			case 202:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_1M
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL20_1M
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				PIXEL22_1R
				break;
			case 78:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_1M
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL20_1M
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				PIXEL22_1M
				break;
			case 154:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_1M
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL02_1M
				} else {
					PIXEL02_2
				}
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1D
				break;
			case 114:
				PIXEL00_1M
				PIXEL01_C
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL02_1M
				} else {
					PIXEL02_2
				}
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_1L
				PIXEL21_C
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL22_1M
				} else {
					PIXEL22_2
				}
				break;
			case 89:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL20_1M
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL22_1M
				} else {
					PIXEL22_2
				}
				break;
			case 90:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_1M
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL02_1M
				} else {
					PIXEL02_2
				}
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL20_1M
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL22_1M
				} else {
					PIXEL22_2
				}
				break;
			case 55:
			case 23:
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL00_1L
					PIXEL01_C
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL00_2
					PIXEL01_6
					PIXEL02_5
					PIXEL12_1
				}
				PIXEL10_1
				PIXEL11
				PIXEL20_2
				PIXEL21_1
				PIXEL22_1M
				break;
			case 182:
			case 150:
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_C
					PIXEL02_C
					PIXEL12_C
					PIXEL22_1D
				} else {
					PIXEL01_1
					PIXEL02_5
					PIXEL12_6
					PIXEL22_2
				}
				PIXEL00_1M
				PIXEL10_1
				PIXEL11
				PIXEL20_2
				PIXEL21_1
				break;
			case 213:
			case 212:
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL02_1U
					PIXEL12_C
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL02_2
					PIXEL12_6
					PIXEL21_1
					PIXEL22_5
				}
				PIXEL00_2
				PIXEL01_1
				PIXEL10_1
				PIXEL11
				PIXEL20_1M
				break;
			case 241:
			case 240:
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL12_C
					PIXEL20_1L
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL12_1
					PIXEL20_2
					PIXEL21_6
					PIXEL22_5
				}
				PIXEL00_2
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				break;
			case 236:
			case 232:
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_C
					PIXEL20_C
					PIXEL21_C
					PIXEL22_1R
				} else {
					PIXEL10_1
					PIXEL20_5
					PIXEL21_6
					PIXEL22_2
				}
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_2
				PIXEL11
				PIXEL12_1
				break;
			case 109:
			case 105:
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL00_1U
					PIXEL10_C
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL00_2
					PIXEL10_6
					PIXEL20_5
					PIXEL21_1
				}
				PIXEL01_1
				PIXEL02_2
				PIXEL11
				PIXEL12_1
				PIXEL22_1M
				break;
			case 171:
			case 43:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_C
					PIXEL01_C
					PIXEL10_C
					PIXEL20_1D
				} else {
					PIXEL00_5
					PIXEL01_1
					PIXEL10_6
					PIXEL20_2
				}
				PIXEL02_1M
				PIXEL11
				PIXEL12_1
				PIXEL21_1
				PIXEL22_2
				break;
			case 143:
			case 15:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_C
					PIXEL01_C
					PIXEL02_1R
					PIXEL10_C
				} else {
					PIXEL00_5
					PIXEL01_6
					PIXEL02_2
					PIXEL10_1
				}
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_2
				break;
			case 124:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1U
				PIXEL11
				PIXEL12_C
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_C
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL10_3
					PIXEL20_4
					PIXEL21_3
				}
				PIXEL22_1M
				break;
			case 203:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_C
					PIXEL01_C
					PIXEL10_C
				} else {
					PIXEL00_4
					PIXEL01_3
					PIXEL10_3
				}
				PIXEL02_1M
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1R
				break;
			case 62:
				PIXEL00_1M
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_C
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL01_3
					PIXEL02_4
					PIXEL12_3
				}
				PIXEL10_C
				PIXEL11
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_1M
				break;
			case 211:
				PIXEL00_1L
				PIXEL01_C
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL20_1M
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL12_C
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL12_3
					PIXEL21_3
					PIXEL22_4
				}
				break;
			case 118:
				PIXEL00_1M
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_C
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL01_3
					PIXEL02_4
					PIXEL12_3
				}
				PIXEL10_1
				PIXEL11
				PIXEL20_1L
				PIXEL21_C
				PIXEL22_1M
				break;
			case 217:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL20_1M
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL12_C
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL12_3
					PIXEL21_3
					PIXEL22_4
				}
				break;
			case 110:
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1R
				PIXEL11
				PIXEL12_1
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_C
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL10_3
					PIXEL20_4
					PIXEL21_3
				}
				PIXEL22_1M
				break;
			case 155:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_C
					PIXEL01_C
					PIXEL10_C
				} else {
					PIXEL00_4
					PIXEL01_3
					PIXEL10_3
				}
				PIXEL02_1M
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1D
				break;
			case 188:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_1D
				break;
			case 185:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_1D
				break;
			case 61:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_1M
				break;
			case 157:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1D
				break;
			case 103:
				PIXEL00_1L
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1L
				PIXEL21_C
				PIXEL22_1M
				break;
			case 227:
				PIXEL00_1L
				PIXEL01_C
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1L
				PIXEL21_C
				PIXEL22_1R
				break;
			case 230:
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1L
				PIXEL21_C
				PIXEL22_1R
				break;
			case 199:
				PIXEL00_1L
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1R
				break;
			case 220:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_C
				PIXEL11
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL20_1M
				} else {
					PIXEL20_2
				}
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL12_C
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL12_3
					PIXEL21_3
					PIXEL22_4
				}
				break;
			case 158:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_1M
				} else {
					PIXEL00_2
				}
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_C
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL01_3
					PIXEL02_4
					PIXEL12_3
				}
				PIXEL10_C
				PIXEL11
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1D
				break;
			case 234:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_1M
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				PIXEL02_1M
				PIXEL11
				PIXEL12_1
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_C
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL10_3
					PIXEL20_4
					PIXEL21_3
				}
				PIXEL22_1R
				break;
			case 242:
				PIXEL00_1M
				PIXEL01_C
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL02_1M
				} else {
					PIXEL02_2
				}
				PIXEL10_1
				PIXEL11
				PIXEL20_1L
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL12_C
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL12_3
					PIXEL21_3
					PIXEL22_4
				}
				break;
			case 59:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_C
					PIXEL01_C
					PIXEL10_C
				} else {
					PIXEL00_4
					PIXEL01_3
					PIXEL10_3
				}
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL02_1M
				} else {
					PIXEL02_2
				}
				PIXEL11
				PIXEL12_C
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_1M
				break;
			case 121:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_1M
				PIXEL11
				PIXEL12_C
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_C
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL10_3
					PIXEL20_4
					PIXEL21_3
				}
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL22_1M
				} else {
					PIXEL22_2
				}
				break;
			case 87:
				PIXEL00_1L
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_C
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL01_3
					PIXEL02_4
					PIXEL12_3
				}
				PIXEL10_1
				PIXEL11
				PIXEL20_1M
				PIXEL21_C
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL22_1M
				} else {
					PIXEL22_2
				}
				break;
			case 79:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_C
					PIXEL01_C
					PIXEL10_C
				} else {
					PIXEL00_4
					PIXEL01_3
					PIXEL10_3
				}
				PIXEL02_1R
				PIXEL11
				PIXEL12_1
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL20_1M
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				PIXEL22_1M
				break;
			case 122:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_1M
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL02_1M
				} else {
					PIXEL02_2
				}
				PIXEL11
				PIXEL12_C
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_C
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL10_3
					PIXEL20_4
					PIXEL21_3
				}
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL22_1M
				} else {
					PIXEL22_2
				}
				break;
			case 94:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_1M
				} else {
					PIXEL00_2
				}
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_C
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL01_3
					PIXEL02_4
					PIXEL12_3
				}
				PIXEL10_C
				PIXEL11
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL20_1M
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL22_1M
				} else {
					PIXEL22_2
				}
				break;
			case 218:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_1M
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL02_1M
				} else {
					PIXEL02_2
				}
				PIXEL10_C
				PIXEL11
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL20_1M
				} else {
					PIXEL20_2
				}
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL12_C
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL12_3
					PIXEL21_3
					PIXEL22_4
				}
				break;
			case 91:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_C
					PIXEL01_C
					PIXEL10_C
				} else {
					PIXEL00_4
					PIXEL01_3
					PIXEL10_3
				}
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL02_1M
				} else {
					PIXEL02_2
				}
				PIXEL11
				PIXEL12_C
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL20_1M
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL22_1M
				} else {
					PIXEL22_2
				}
				break;
			case 229:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_2
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1L
				PIXEL21_C
				PIXEL22_1R
				break;
			case 167:
				PIXEL00_1L
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_2
				PIXEL21_1
				PIXEL22_2
				break;
			case 173:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_2
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_2
				break;
			case 181:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_2
				PIXEL21_1
				PIXEL22_1D
				break;
			case 186:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_1M
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL02_1M
				} else {
					PIXEL02_2
				}
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_1D
				break;
			case 115:
				PIXEL00_1L
				PIXEL01_C
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL02_1M
				} else {
					PIXEL02_2
				}
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_1L
				PIXEL21_C
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL22_1M
				} else {
					PIXEL22_2
				}
				break;
			case 93:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL20_1M
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL22_1M
				} else {
					PIXEL22_2
				}
				break;
			case 206:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_1M
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL20_1M
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				PIXEL22_1R
				break;
			case 205:
			case 201:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_2
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL20_1M
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				PIXEL22_1R
				break;
			case 174:
			case 46:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_1M
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_2
				break;
			case 179:
			case 147:
				PIXEL00_1L
				PIXEL01_C
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL02_1M
				} else {
					PIXEL02_2
				}
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_2
				PIXEL21_1
				PIXEL22_1D
				break;
			case 117:
			case 116:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_1L
				PIXEL21_C
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL22_1M
				} else {
					PIXEL22_2
				}
				break;
			case 189:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_1D
				break;
			case 231:
				PIXEL00_1L
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_1
				PIXEL11
				PIXEL12_1
				PIXEL20_1L
				PIXEL21_C
				PIXEL22_1R
				break;
			case 126:
				PIXEL00_1M
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_C
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL01_3
					PIXEL02_4
					PIXEL12_3
				}
				PIXEL11
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_C
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL10_3
					PIXEL20_4
					PIXEL21_3
				}
				PIXEL22_1M
				break;
			case 219:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_C
					PIXEL01_C
					PIXEL10_C
				} else {
					PIXEL00_4
					PIXEL01_3
					PIXEL10_3
				}
				PIXEL02_1M
				PIXEL11
				PIXEL20_1M
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL12_C
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL12_3
					PIXEL21_3
					PIXEL22_4
				}
				break;
			case 125:
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL00_1U
					PIXEL10_C
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL00_2
					PIXEL10_6
					PIXEL20_5
					PIXEL21_1
				}
				PIXEL01_1
				PIXEL02_1U
				PIXEL11
				PIXEL12_C
				PIXEL22_1M
				break;
			case 221:
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL02_1U
					PIXEL12_C
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL02_2
					PIXEL12_6
					PIXEL21_1
					PIXEL22_5
				}
				PIXEL00_1U
				PIXEL01_1
				PIXEL10_C
				PIXEL11
				PIXEL20_1M
				break;
			case 207:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_C
					PIXEL01_C
					PIXEL02_1R
					PIXEL10_C
				} else {
					PIXEL00_5
					PIXEL01_6
					PIXEL02_2
					PIXEL10_1
				}
				PIXEL11
				PIXEL12_1
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1R
				break;
			case 238:
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_C
					PIXEL20_C
					PIXEL21_C
					PIXEL22_1R
				} else {
					PIXEL10_1
					PIXEL20_5
					PIXEL21_6
					PIXEL22_2
				}
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1R
				PIXEL11
				PIXEL12_1
				break;
			case 190:
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_C
					PIXEL02_C
					PIXEL12_C
					PIXEL22_1D
				} else {
					PIXEL01_1
					PIXEL02_5
					PIXEL12_6
					PIXEL22_2
				}
				PIXEL00_1M
				PIXEL10_C
				PIXEL11
				PIXEL20_1D
				PIXEL21_1
				break;
			case 187:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_C
					PIXEL01_C
					PIXEL10_C
					PIXEL20_1D
				} else {
					PIXEL00_5
					PIXEL01_1
					PIXEL10_6
					PIXEL20_2
				}
				PIXEL02_1M
				PIXEL11
				PIXEL12_C
				PIXEL21_1
				PIXEL22_1D
				break;
			case 243:
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL12_C
					PIXEL20_1L
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL12_1
					PIXEL20_2
					PIXEL21_6
					PIXEL22_5
				}
				PIXEL00_1L
				PIXEL01_C
				PIXEL02_1M
				PIXEL10_1
				PIXEL11
				break;
			case 119:
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL00_1L
					PIXEL01_C
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL00_2
					PIXEL01_6
					PIXEL02_5
					PIXEL12_1
				}
				PIXEL10_1
				PIXEL11
				PIXEL20_1L
				PIXEL21_C
				PIXEL22_1M
				break;
			case 237:
			case 233:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_2
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL20_C
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				PIXEL22_1R
				break;
			case 175:
			case 47:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_C
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_2
				break;
			case 183:
			case 151:
				PIXEL00_1L
				PIXEL01_C
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL02_C
				} else {
					PIXEL02_2
				}
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_2
				PIXEL21_1
				PIXEL22_1D
				break;
			case 245:
			case 244:
				PIXEL00_2
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_1L
				PIXEL21_C
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL22_C
				} else {
					PIXEL22_2
				}
				break;
			case 250:
				PIXEL00_1M
				PIXEL01_C
				PIXEL02_1M
				PIXEL11
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_C
					PIXEL20_C
				} else {
					PIXEL10_3
					PIXEL20_4
				}
				PIXEL21_C
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL12_C
					PIXEL22_C
				} else {
					PIXEL12_3
					PIXEL22_4
				}
				break;
			case 123:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_C
					PIXEL01_C
				} else {
					PIXEL00_4
					PIXEL01_3
				}
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL20_4
					PIXEL21_3
				}
				PIXEL22_1M
				break;
			case 95:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_C
					PIXEL10_C
				} else {
					PIXEL00_4
					PIXEL10_3
				}
				PIXEL01_C
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL02_4
					PIXEL12_3
				}
				PIXEL11
				PIXEL20_1M
				PIXEL21_C
				PIXEL22_1M
				break;
			case 222:
				PIXEL00_1M
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_C
					PIXEL02_C
				} else {
					PIXEL01_3
					PIXEL02_4
				}
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL21_3
					PIXEL22_4
				}
				break;
			case 252:
				PIXEL00_1M
				PIXEL01_1
				PIXEL02_1U
				PIXEL11
				PIXEL12_C
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_C
					PIXEL20_C
				} else {
					PIXEL10_3
					PIXEL20_4
				}
				PIXEL21_C
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL22_C
				} else {
					PIXEL22_2
				}
				break;
			case 249:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL20_C
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL12_C
					PIXEL22_C
				} else {
					PIXEL12_3
					PIXEL22_4
				}
				break;
			case 235:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_C
					PIXEL01_C
				} else {
					PIXEL00_4
					PIXEL01_3
				}
				PIXEL02_1M
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL20_C
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				PIXEL22_1R
				break;
			case 111:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_C
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL20_4
					PIXEL21_3
				}
				PIXEL22_1M
				break;
			case 63:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_C
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL02_4
					PIXEL12_3
				}
				PIXEL10_C
				PIXEL11
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_1M
				break;
			case 159:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_C
					PIXEL10_C
				} else {
					PIXEL00_4
					PIXEL10_3
				}
				PIXEL01_C
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL02_C
				} else {
					PIXEL02_2
				}
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				PIXEL21_1
				PIXEL22_1D
				break;
			case 215:
				PIXEL00_1L
				PIXEL01_C
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL02_C
				} else {
					PIXEL02_2
				}
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_1M
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL21_3
					PIXEL22_4
				}
				break;
			case 246:
				PIXEL00_1M
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_C
					PIXEL02_C
				} else {
					PIXEL01_3
					PIXEL02_4
				}
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_1L
				PIXEL21_C
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL22_C
				} else {
					PIXEL22_2
				}
				break;
			case 254:
				PIXEL00_1M
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_C
					PIXEL02_C
				} else {
					PIXEL01_3
					PIXEL02_4
				}
				PIXEL11
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_C
					PIXEL20_C
				} else {
					PIXEL10_3
					PIXEL20_4
				}
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL12_C
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL12_3
					PIXEL21_3
					PIXEL22_2
				}
				break;
			case 253:
				PIXEL00_1U
				PIXEL01_1
				PIXEL02_1U
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL20_C
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL22_C
				} else {
					PIXEL22_2
				}
				break;
			case 251:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_C
					PIXEL01_C
				} else {
					PIXEL00_4
					PIXEL01_3
				}
				PIXEL02_1M
				PIXEL11
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL10_C
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL10_3
					PIXEL20_2
					PIXEL21_3
				}
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL12_C
					PIXEL22_C
				} else {
					PIXEL12_3
					PIXEL22_4
				}
				break;
			case 239:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_C
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				PIXEL02_1R
				PIXEL10_C
				PIXEL11
				PIXEL12_1
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL20_C
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				PIXEL22_1R
				break;
			case 127:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_C
					PIXEL01_C
					PIXEL10_C
				} else {
					PIXEL00_2
					PIXEL01_3
					PIXEL10_3
				}
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL02_4
					PIXEL12_3
				}
				PIXEL11
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL20_C
					PIXEL21_C
				} else {
					PIXEL20_4
					PIXEL21_3
				}
				PIXEL22_1M
				break;
			case 191:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_C
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL02_C
				} else {
					PIXEL02_2
				}
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				PIXEL20_1D
				PIXEL21_1
				PIXEL22_1D
				break;
			case 223:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_C
					PIXEL10_C
				} else {
					PIXEL00_4
					PIXEL10_3
				}
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL01_C
					PIXEL02_C
					PIXEL12_C
				} else {
					PIXEL01_3
					PIXEL02_2
					PIXEL12_3
				}
				PIXEL11
				PIXEL20_1M
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL21_C
					PIXEL22_C
				} else {
					PIXEL21_3
					PIXEL22_4
				}
				break;
			case 247:
				PIXEL00_1L
				PIXEL01_C
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL02_C
				} else {
					PIXEL02_2
				}
				PIXEL10_1
				PIXEL11
				PIXEL12_C
				PIXEL20_1L
				PIXEL21_C
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL22_C
				} else {
					PIXEL22_2
				}
				break;
			case 255:
				if (diffYUV(yuv[4], yuv[2])) {
					PIXEL00_C
				} else {
					PIXEL00_2
				}
				PIXEL01_C
				if (diffYUV(yuv[2], yuv[6])) {
					PIXEL02_C
				} else {
					PIXEL02_2
				}
				PIXEL10_C
				PIXEL11
				PIXEL12_C
				if (diffYUV(yuv[8], yuv[4])) {
					PIXEL20_C
				} else {
					PIXEL20_2
				}
				PIXEL21_C
				if (diffYUV(yuv[6], yuv[8])) {
					PIXEL22_C
				} else {
					PIXEL22_2
				}
				break;
			}
			q += 3;
		}
		p += nextlineSrc - width;
		q += (nextlineDst - width) * 3;
	}
}

void InitLUTs(uint32 BitFormat) {
	int i, j, k, r, g, b, Y, u, v;

	for (i = 0; i < 32; i++) {
		r = i << 3;
		for (j = 0; j < 64; j++) {
			g = j << 2;
			for (k = 0; k < 32; k++) {
				b = k << 3;
				Y = (r + g + b) >> 2;
				u = 128 + ((r - b) >> 2);
				v = 128 + ((-r + 2 * g -b) >> 3);
				RGBtoYUV[ (i << 11) + (j << 5) + k ] = (Y << 16) + (u << 8) + v;
			}
		}
	}
}


#pragma mark -


#define	kVeryFastAndUglyAspectMode	0	// No interpolation at all, but super-fast
#define	kFastAndNiceAspectMode		1	// Quite good quality with good speed
#define	kSlowAndPerfectAspectMode	2	// Accurate but slow code

#define ASPECT_MODE	kFastAndNiceAspectMode


#if ASPECT_MODE == kSlowAndPerfectAspectMode
template<int scale>
static inline uint16 interpolate5(uint16 A, uint16 B) {
	uint16 r = (uint16)(((A & redMask) * scale + (B & redMask) * (5 - scale)) / 5);
	uint16 g = (uint16)(((A & greenMask) * scale + (B & greenMask) * (5 - scale)) / 5);
	uint16 b = (uint16)(((A & blueMask) * scale + (B & blueMask) * (5 - scale)) / 5);

	return (uint16)((r & redMask) | (g & greenMask) | (b & blueMask));
}

template<int scale>
static inline void interpolate5Line(uint16 *dst, const uint16 *srcA, const uint16 *srcB, int width) {
	// Accurate but slightly slower code
	while (width--) {
		*dst++ = interpolate5<scale>(*srcA++, *srcB++);
	}
}
#endif

#if ASPECT_MODE == kFastAndNiceAspectMode
template<int scale>
static inline void interpolate5Line(uint16 *dst, const uint16 *srcA, const uint16 *srcB, int width) {
	// For efficiency reasons we blit two pixels at a time, so it is
	// important that makeRectStretchable() guarantees that the width is
	// even and that the rect starts on a well-aligned address. (Even
	// where unaligned memory access is allowed there may be a speed
	// penalty for it.)

	// These asserts are disabled for maximal speed; but I leave them in here in case
	// other people want to test if the memory alignment (to an address divisibl by 4)
	// are really effective.
	//assert(((int)dst & 3) == 0);
	//assert(((int)srcA & 3) == 0);
	//assert(((int)srcB & 3) == 0);
	//assert((width & 1) == 0);

	width /= 2;
	const uint32 *sA = (const uint32 *)srcA;
	const uint32 *sB = (const uint32 *)srcB;
	uint32 *d = (uint32 *)dst;
	if (scale == 1) {
		while (width--) {
			uint32 B = *sB++;
			*d++ = Q_INTERPOLATE(*sA++, B, B, B);
		}
	} else {
		while (width--) {
			*d++ = INTERPOLATE(*sA++, *sB++);
		}
	}
}
#endif

void makeRectStretchable(int &x, int &y, int &w, int &h) {
#if ASPECT_MODE != kVeryFastAndUglyAspectMode
	int m = real2Aspect(y) % 6;

	// Ensure that the rect will start on a line that won't have its
	// colours changed by the stretching function.
	if (m != 0 && m != 5) {
		y -= m;
		h += m;
	}

  #if ASPECT_MODE == kFastAndNiceAspectMode
	// Force x to be even, to ensure aligned memory access (this assumes
	// that each line starts at an even memory location, but that should
	// be the case on every target anyway).
	if (x & 1) {
		x--;
		w++;
	}

	// Finally force the width to be even, since we blit 2 pixels at a time.
	// While this means we may sometimes blit one column more than necessary,
	// this should actually be faster than having the check for the 
	if (w & 1)
		w++;
  #endif
#endif
}

/**
 * Stretch a 16bpp image vertically by factor 1.2. Used to correct the
 * aspect-ratio in games using 320x200 pixel graphics with non-qudratic
 * pixels. Applying this method effectively turns that into 320x240, which
 * provides the correct aspect-ratio on modern displays.
 *
 * The image would normally have occupied y coordinates origSrcY through
 * origSrcY + height - 1.
 *
 * However, we have already placed it at srcY - the aspect-corrected y
 * coordinate - to allow in-place stretching.
 *
 * Therefore, the source image now occupies Y coordinates srcY through
 * srcY + height - 1, and it should be stretched to Y coordinates srcY
 * through real2Aspect(srcY + height - 1).
 */
int stretch200To240(uint8 *buf, uint32 pitch, int width, int height, int srcX, int srcY, int origSrcY) {
	int maxDstY = real2Aspect(origSrcY + height - 1);
	int y;
	const uint8 *startSrcPtr = buf + srcX * 2 + (srcY - origSrcY) * pitch;
	uint8 *dstPtr = buf + srcX * 2 + maxDstY * pitch;

	for (y = maxDstY; y >= srcY; y--) {
		const uint8 *srcPtr = startSrcPtr + aspect2Real(y) * pitch;

#if ASPECT_MODE == kVeryFastAndUglyAspectMode
		if (srcPtr == dstPtr)
			break;
		memcpy(dstPtr, srcPtr, width * 2);
#else
		// Bilinear filter
		switch (y % 6) {
		case 0:
		case 5:
			if (srcPtr != dstPtr)
				memcpy(dstPtr, srcPtr, width * 2);
			break;
		case 1:
			interpolate5Line<1>((uint16 *)dstPtr, (const uint16 *)(srcPtr - pitch), (const uint16 *)srcPtr, width);
			break;
		case 2:
			interpolate5Line<2>((uint16 *)dstPtr, (const uint16 *)(srcPtr - pitch), (const uint16 *)srcPtr, width);
			break;
		case 3:
			interpolate5Line<2>((uint16 *)dstPtr, (const uint16 *)srcPtr, (const uint16 *)(srcPtr - pitch), width);
			break;
		case 4:
			interpolate5Line<1>((uint16 *)dstPtr, (const uint16 *)srcPtr, (const uint16 *)(srcPtr - pitch), width);
			break;
		}
#endif
		dstPtr -= pitch;
	}

	return 1 + maxDstY - srcY;
}
