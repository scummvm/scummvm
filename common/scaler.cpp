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
#include "scummsys.h"
#include "scaler.h"

/********** 2XSAI Filter *****************/
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

int Init_2xSaI(uint32 BitFormat) {
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
		return 0;
	}

	return 1;
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
		return (((A & colorMask) + (B & colorMask)) >> 1) + (A & B & lowPixelMask);
	} else
		return A;
}

static inline uint32 Q_INTERPOLATE(uint32 A, uint32 B, uint32 C, uint32 D) {
	register uint32 x = ((A & qcolorMask) + (B & qcolorMask) + (C & qcolorMask) + (D & qcolorMask))>>2;
	register uint32 y = (A & qlowpixelMask) + (B & qlowpixelMask) + (C & qlowpixelMask) + (D & qlowpixelMask);

	y = (y >> 2) & qlowpixelMask;
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
			if (color2 == color6 && color5 != color3) {
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
			} else if (color5 == color3 && color2 != color6) {
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

			} else if (color5 == color3 && color2 == color6) {
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
			} else {
				product2b = product1a = INTERPOLATE(color2, color6);
				product2b = Q_INTERPOLATE(color3, color3, color3, product2b);
				product1a = Q_INTERPOLATE(color5, color5, color5, product1a);

				product2a = product1b = INTERPOLATE(color5, color3);
				product2a = Q_INTERPOLATE(color2, color2, color2, product2a);
				product1b = Q_INTERPOLATE(color6, color6, color6, product1b);
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
	unsigned int nextlineSrc = srcPitch / sizeof(uint16);
	const uint16 *p = (const uint16 *)srcPtr;

	unsigned int nextlineDst = dstPitch / sizeof(uint16);
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
	unsigned int nextlineSrc = srcPitch / sizeof(uint16);
	const uint16 *p = (const uint16 *)srcPtr;

	unsigned int nextlineDst = dstPitch / sizeof(uint16);
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
	unsigned int nextlineSrc = srcPitch / sizeof(uint16);
	const uint16 *p = (const uint16 *)srcPtr;

	unsigned int nextlineDst = dstPitch / sizeof(uint16);
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
	unsigned int nextlineSrc = srcPitch / sizeof(uint16);
	const uint16 *p = (const uint16 *)srcPtr;

	unsigned int nextlineDst = dstPitch / sizeof(uint16);
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


static inline uint16 interpolate5(uint16 A, uint16 B, int scale) {
	uint16 r = (uint16)(((A & redMask) * scale + (B & redMask) * (5 - scale)) / 5);
	uint16 g = (uint16)(((A & greenMask) * scale + (B & greenMask) * (5 - scale)) / 5);
	uint16 b = (uint16)(((A & blueMask) * scale + (B & blueMask) * (5 - scale)) / 5);

	return (r & redMask) | (g & greenMask) | (b & blueMask);
}

static inline void interpolate5Line(uint16 *dst, const uint16 *srcA, const uint16 *srcB, int scale, int width) {
#if 1
	// Accurate but slightly slower code
	while (width--) {
		*dst++ = interpolate5(*srcA++, *srcB++, scale);
	}
#else
	// Not fully accurate, but a bit faster
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
	int off = srcY - origSrcY;
	int y;

	uint8 *dstPtr = buf + srcX * 2 + maxDstY * pitch;

	for (y = maxDstY; y >= srcY; y--) {
		uint8 *srcPtr = buf + srcX * 2 + (aspect2Real(y) + off) * pitch;

#if 0
		// Don't use bilinear filtering, rather just duplicate pixel lines:
		// a little bit faster, but looks ugly
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
		case 4:
			interpolate5Line((uint16 *)dstPtr, (uint16 *)(srcPtr - pitch), (uint16 *)srcPtr, 1, width);
			break;
		case 2:
		case 3:
			interpolate5Line((uint16 *)dstPtr, (uint16 *)(srcPtr - pitch), (uint16 *)srcPtr, 2, width);
			break;
		}
#endif
		dstPtr -= pitch;
	}

	return 1 + maxDstY - srcY;
}
