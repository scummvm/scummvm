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


int gBitFormat = 565;

// RGB-to-YUV lookup table
int   RGBtoYUV[65536];

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

static void InitLUT(uint32 BitFormat);

void InitScalers(uint32 BitFormat) {
	if (BitFormat == 565) {
		dotmatrix = dotmatrix_565;
	} else if (BitFormat == 555) {
		dotmatrix = dotmatrix_555;
	} else {
		error("Unknwon bit format %d\n", BitFormat);
	}

	gBitFormat = BitFormat;
	InitLUT(BitFormat);
}

void InitLUT(uint32 BitFormat) {
	int r, g, b;
	int Y, u, v;
	int gInc, gShift;
	
	if (BitFormat == 565) {
		gInc = 256 >> 6;
		gShift = 6 - 3;
	} else {
		gInc = 256 >> 5;
		gShift = 5 - 3;
	}

	for (r = 0; r < 256; r += 8) {
		for (g = 0; g < 256; g += gInc) {
			for (b = 0; b < 256; b += 8) {
				Y = (r + g + b) >> 2;
				u = 128 + ((r - b) >> 2);
				v = 128 + ((-r + 2 * g -b) >> 3);
				RGBtoYUV[ (r << (5+gShift)) + (g << gShift) + (b >> 3) ] = (Y << 16) + (u << 8) + v;
			}
		}
	}
}

/**
 * Trivial 'scaler' - in fact it doesn't do any scaling but just copies the
 * source to the destionation.
 */
void Normal1x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							int width, int height) {
	while (height--) {
		memcpy(dstPtr, srcPtr, 2 * width);
		srcPtr += srcPitch;
		dstPtr += dstPitch;
	}
}

/**
 * Trivial nearest-neighbour 2x scaler.
 */
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

/**
 * Trivial nearest-neighbour 3x scaler.
 */
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

/**
 * The Scale2x filter, also known as AdvMame2x.
 * See also http://scale2x.sourceforge.net
 */
void AdvMame2x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							 int width, int height) {
	const uint32 nextlineSrc = srcPitch / sizeof(uint16);
	const uint16 *p = (const uint16 *)srcPtr;

	const uint32 nextlineDst = dstPitch / sizeof(uint16);
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
		int tmpWidth = width;
		while (tmpWidth--) {
			p++;
			A = B; B = C; C = *(p - nextlineSrc);
			D = E; E = F; F = *(p);
			G = H; H = I; I = *(p + nextlineSrc);
			*(q + 0) = D == B && B != F && D != H ? D : E;
			*(q + 1) = B == F && B != D && F != H ? F : E;
			*(q + nextlineDst + 0) = D == H && D != B && H != F ? D : E;
			*(q + nextlineDst + 1) = H == F && D != H && B != F ? F : E;
			q += 2;
		}
		p += nextlineSrc - width;
		q += (nextlineDst - width) << 1;
	}
}

/**
 * The Scale3x filter, also known as AdvMame3x.
 * See also http://scale2x.sourceforge.net
 */
void AdvMame3x(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
							 int width, int height) {
	const uint32 nextlineSrc = srcPitch / sizeof(uint16);
	const uint16 *p = (const uint16 *)srcPtr;

	const uint32 nextlineDst = dstPitch / sizeof(uint16);
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
		int tmpWidth = width;
		while (tmpWidth--) {
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

template<int bitFormat>
void TV2xTemplate(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch, 
					int width, int height) {
	const uint32 nextlineSrc = srcPitch / sizeof(uint16);
	const uint16 *p = (const uint16 *)srcPtr;

	const uint32 nextlineDst = dstPitch / sizeof(uint16);
	uint16 *q = (uint16 *)dstPtr;

	while (height--) {
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
MAKE_WRAPPER(TV2x)

static inline uint16 DOT_16(uint16 c, int j, int i) {
  return c - ((c >> 2) & *(dotmatrix + ((j & 3) << 2) + (i & 3)));
}

void DotMatrix(const uint8 *srcPtr, uint32 srcPitch, uint8 *dstPtr, uint32 dstPitch,
					int width, int height)
{
	const uint32 nextlineSrc = srcPitch / sizeof(uint16);
	const uint16 *p = (const uint16 *)srcPtr;

	const uint32 nextlineDst = dstPitch / sizeof(uint16);
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

