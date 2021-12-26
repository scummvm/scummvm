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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * Another edge-directed 2x/3x anti-aliasing scaler for ScummVM
 *
 * Author: Eric A. Welsh
 *
 *
 * Sharp, clean, anti-aliased image with very few artifacts.
 * Detects and appropriately handles mouse overlays with transparent pixels.
 * The Edge3x filter detects unchanged pixels and does not redraw them,
 * resulting in a considerable gain in speed when there are even a moderate
 * number of unchanged pixels.  Edge3x and Edge2x anti-alias using nearest-
 * neighbor methods.  Edge2xi interpolates.
 *
 * The really slow speed is mainly due to the edge detection algorithm.  In
 * order to accurately detect the edge direction (and thus avoid artifacts
 * caused by mis-detection), the edge detection and refinement process is
 * rather long and involved.  Speed must be sacrificed in order to avoid
 * artifacts :(  If anyone is tempted to optimize using lower precision
 * math, such as converting some of the double math to fixed-point integer,
 * or lowering the number of significant bits used in the existing integer
 * math to squeeze it into accelerated 16-bit vector instructions, please do
 * not do this.  Any loss in precision results in visibly degraded image
 * quality.  I've tried integer conversions of various double math, and tried
 * reducing the number of significant digits I use in the integer math, and
 * it always results in less accurate edge detection and lower image quality.
 * If you're going to optimize, make sure you don't sacrifice any precision.
 * There may be a few places I could change the variable types from
 * int32 to int16 without introducing overflows, though.  I should also
 * probably change some of the flags and arrays to bools or chars, rather
 * than ints, since they are only 0 or 1.
 *
 * It's a bit slow... but ScummVM runs most things fine on my 1.53 GHz Athlon.
 * Increasing the Win32 thread priority can help by forcing Windows not to
 * twiddle its thumbs and idle as much during heavy CPU load.  The Dig
 * cutscene when the asteroid is activated is a little jerky, pans/fades in
 * Amazon Queen and Beneath a Steel Sky are slow.  Faster machines probably
 * won't have a problem.  I remember a time when my home machine was too slow
 * to run SNES emulators with 2xSaI filters, so I don't think speed is such a
 * big issue here.  It won't be too long before the average home computer is
 * plenty fast enough to run this filter.
 *
 */

/*
 * Notes on handling overlays, mouse, transparencies, etc.:
 *
 * As I write this, the SDL backend does not call different filters based on
 * whether or not the bitmaps contain transparency.  Bitmaps with transparency
 * need to be treated differently.  1) Interpolation needs to be disabled,
 * since interpolating with transparent pixels produces ugly smears around the
 * transparent areas.  2) Transparent pixels need to be treated differently
 * during edge detection, so that they appear to be as if they were colors
 * that would give maximal contrast in the current 3x3 window.
 *
 * Currently, the SDL backend calls anti-aliasing resize filters to either
 * resize the game screen or resize the mouse.  The filter stores the src
 * array bounds whenever the width and height of the area to be resized are
 * equal to the width and height of the current game screen.  If the current
 * src array is outside these bounds, then it is assumed that the mouse or
 * menu overlay is being drawn.  This works perfectly for the current SDL
 * backend, but it is still a hack.  If, in the future, the filter were to
 * to be used to resize a transparent overlay with dimensions equal to those
 * of the current game screen, that overlay would be resized without any
 * special transparency consideration.  The same goes for a mouse pointer
 * that is equal to the size of the screen, but I don't forsee this ever
 * being a problem....  The correct solution would be to rewrite the backends
 * to call filters differently depending on whether or not the bitmap contains
 * transparent pixels, and whether or not the end result should be
 * interpolated or resized using nearest-neighbor.  Until then, the array
 * bounds checking hack will have to do.
 *
 */

#include <math.h>
#include "common/scummsys.h"
#include "common/system.h"
#include "graphics/scaler/intern.h"
#include "graphics/scaler/edge.h"

/* Randomly XORs one of 2x2 or 3x3 resized pixels in order to indicate
 * which pixels have been redrawn.  Useful for seeing which areas of
 * the screen are being redrawn.  Good for seeing dirty rects, full screen
 * refreshes, etc..  Also good for seeing if the unchanged pixel detection is
 * working correctly or not :)
 */
#define DEBUG_REFRESH_RANDOM_XOR        0   /* debug redraws */

/* Use with DEBUG_REFRESH_RANDOM_XOR.  Randomize the borders of the drawing
 * area, whether they are unchanged or not.  Useful for visualizing the
 * borders of the drawing area.  You might be surprised at which areas of the
 * screen get redraw requests, even areas with absolutely nothing moving,
 * or color cycling, or anything that would cause a dirty rect or require a
 * redraw....
 */
#define DEBUG_DRAW_REFRESH_BORDERS      0   /* more redraw debug */

#define PARANOID_KNIGHTS            1   /* avoid artifacts */
#define PARANOID_ARROWS             1   /* avoid artifacts */

#define SIN45 0.7071067811865       /* sin of 45 degrees */
#define GREY_SHIFT 12           /* bit shift for greyscale precision */
#define RGB_SHIFT 13            /* bit shift for RGB precision */

#define SQRT2 1.41421356237309504880
static const int16 one_sqrt2 = (int16)(((int16)1 << GREY_SHIFT) / SQRT2 + 0.5);
// static const int16 int32_sqrt3 = (int16)(((int16)1 << GREY_SHIFT) * sqrt(3.0) + 0.5);


#define interpolate_1_1(a,b)         (ColorMask::kBytesPerPixel == 2 ? interpolate16_1_1<ColorMask>(a,b) : interpolate32_1_1<ColorMask>(a,b))
#define interpolate_3_1(a,b)         (ColorMask::kBytesPerPixel == 2 ? interpolate16_3_1<ColorMask>(a,b) : interpolate32_3_1<ColorMask>(a,b))
#define interpolate_2_1(a,b)         (ColorMask::kBytesPerPixel == 2 ? interpolate16_2_1<ColorMask>(a,b) : interpolate32_2_1<ColorMask>(a,b))
#define interpolate_1_1_1(a,b,c)     (ColorMask::kBytesPerPixel == 2 ? interpolate16_1_1_1<ColorMask>(a,b,c) : interpolate32_1_1_1<ColorMask>(a,b,c))

#if DEBUG_REFRESH_RANDOM_XOR
/* Random number generators with very good randomness properties, far better
 * than the simple linear congruential generator in the ScummVM utils.
 * The RNG algorithms were developed by George Marsaglia, as published in his
 * "Xorshift RNGs" paper and various USENET postings.
 *
 * Marsaglia, George, 2003, "Xorshift RNGs", Journal of Statistical Software,
 * Volume 7, Issue 14
 *
 * Note the comments in the code below about how many of the triplet and shift
 * combinations published in his paper and various USENET postings don't quite
 * work as expected (yes, I exhaustively tested all 648 of them with his test
 * suite).  Only a few "magic" combinations actually produce good random
 * numbers.  I suspect this is due to the numbers being shifted too far off
 * the ends of the registers?
 *
 * While numbers this highly random are overkill for our purposes, I already
 * had this code written for various scientific analysis programs, and I've
 * tested the generators with Marsaglia's comprehensive randomness test
 * suite, so I know that they have very good randomness.  The simple single
 * seed algorithm does fail a few minor tests, but it is still LOADS better
 * than a linear congruential generator.  The 4 seed RNG passes all tests
 * with flying colors and has a pretty big period to boot.  I actually use
 * a 4096 seed RNG for my scientific work, but that is MAJOR overkill for
 * the simple purposes we require here, so I've not included it :)
 */

static uint32 seed0, seed1, seed2, seed3;

/* period 2^32 - 1 */
/* fails Gorilla test, binary rank matrix */
/* the ONLY good triplet and shift combinations out of the list of 648:
 *
 *    3  7 13     >> >> <<
 *    3  7 13     << << >>
 *    7  3 13     >> >> <<
 *    7  3 13     << << >>
 *
 *    5  6 13     >> >> <<
 *    5  6 13     << << >>
 *    6  5 13     >> >> <<
 *    6  5 13     << << >>  seems to be slightly "better" than the others?
 *
 * all others, including the "favorite" (13, 17, 5), fail some Monkey tests
 */
static uint32 xorshift32(void) {
	seed0 ^= seed0 << 6;
	seed0 ^= seed0 << 5;
	seed0 ^= seed0 >> 13;

	return(seed0);
}

/* period 2^128 - 1 */
/* None of the other published 2^128-1 xorshift RNGs passed OPERM5 */
static uint32 xorshift128(void) {
	uint32 temp;

	temp = (seed0 ^ (seed0 << 20)) ^ (seed1 ^ (seed1 >> 11)) ^
	       (seed2 ^ (seed2 << 27)) ^ (seed3 ^ (seed3 >> 6));
	seed0 = seed1;
	seed1 = seed2;
	seed2 = seed3;
	seed3 = temp;

	return (temp);
}

/* return a random fraction over the range [0, 1) */
static double dxorshift128(void) {
	uint32 temp;

	temp = (seed0 ^ (seed0 << 20)) ^ (seed1 ^ (seed1 >> 11)) ^
	       (seed2 ^ (seed2 << 27)) ^ (seed3 ^ (seed3 >> 6));
	seed0 = seed1;
	seed1 = seed2;
	seed2 = seed3;
	seed3 = temp;

	return (temp / 4294967296.0);
}

static void initializeXorshift128(uint32 seed) {
	/* seed0 needs to be initialized prior to calling xorshift32() */
	seed0 = seed;

	/* initialize with xorshift32() */
	seed0 = xorshift32();
	seed1 = xorshift32();
	seed2 = xorshift32();
	seed3 = xorshift32();
}
#endif



/*
 * Faster than standard double atan(), |error| < 7E-6
 *
 * Original equation from Ranko Bojanic in StuChat37:
 *
 * |x| <= 1:
 *
 *    x + A*x3      A = 0.43157974, B = 0.76443945, C = 0.05831938
 * ---------------
 * 1 + B*x2 + C*x4
 *
 *
 *
 * After some optimizations:
 *
 * |x| <= 1:
 *
 *  x * (E + F*x2)  E = 1/C, F = A/C
 * ---------------- G = (B/C + sqrt(B2/C2 - 4/C)) / 2
 * (G + x2)(H + x2) H = (B/C - sqrt(B2/C2 - 4/C)) / 2
 *
 *          E = 17.14695869537, F = 7.400279975541
 *          G = 11.63393762882, H = 1.473874045440
 *
 * |x| > 1: pi/2 -
 *
 *   x * (I + x2)   I = A
 * ---------------- J = (B + sqrt(B2 - 4C)) / 2
 * (J + x2)(K + x2) K = (B - sqrt(B2 - 4C)) / 2
 *
 *          I = 0.43157974
 *          J = 0.6784840295980, K = 0.0859554204018
 *
 */
static double fastAtan(double x0) {
	double x2;
	double x;

	x = fabs(x0);
	x2 = x * x;
	if (x > 1) {
		x2 = 1.570796326795 -
		     x * (0.43157974 + x2) /
		     ((0.6784840295980 + x2) * (0.0859554204018 + x2));
		if (x0 < 0) return -x2;
		return x2;
	}

	return x0 * (17.14695869537 + 7.400279975541 * x2) /
	       ((11.63393762882 + x2) * (1.473874045440 + x2));
}


/**
 * Convert a pixel to a 16bpp format. If called on
 * a 16bpp pixel, nothing happens and the call should
 * be removed at compile time by an optimizing compiler.
 * This allows 16 and 32 bit functions to easily share
 * the same code.
 */
template<typename ColorMask>
uint16 convertTo16Bit(const typename ColorMask::PixelType p) {
	if (sizeof(typename ColorMask::PixelType) == 2)
		return p;

	uint16 r, g, b;
	r = (ColorMask::kRedMask & (ColorMask::kRedMask << (8 - Graphics::ColorMasks<565>::kRedBits)) & p)
		>> (ColorMask::kRedShift + (8 - Graphics::ColorMasks<565>::kRedBits) - Graphics::ColorMasks<565>::kRedShift);
	g = (ColorMask::kGreenMask & (ColorMask::kGreenMask << (8 - Graphics::ColorMasks<565>::kGreenBits)) & p)
		>> (ColorMask::kGreenShift + (8 - Graphics::ColorMasks<565>::kGreenBits) - Graphics::ColorMasks<565>::kGreenShift);
	b = (ColorMask::kBlueMask & (ColorMask::kBlueMask << (8 - Graphics::ColorMasks<565>::kBlueBits)) & p)
		>> (ColorMask::kBlueShift + (8 - Graphics::ColorMasks<565>::kBlueBits) - Graphics::ColorMasks<565>::kBlueShift);

	return r | g | b;
}


template<typename ColorMask>
int16 *EdgeScaler::chooseGreyscale(typename ColorMask::PixelType *pixels) {
	int i, j;
	int32 scores[3];

	for (i = 0; i < 3; i++) {
		int16 *diff_ptr;
		int16 *bptr;
		typename ColorMask::PixelType *pptr;
		int16 *grey_ptr;
		int16 center;
		int32 sum_diffs;

		sum_diffs = 0;

		grey_ptr = _greyscaleTable[i];

		/* fill the 9 pixel window with greyscale values */
		bptr = _bplanes[i];
		pptr = pixels;
		for (j = 9; j; --j)
			*bptr++ = grey_ptr[convertTo16Bit<ColorMask>(*pptr++)];
		bptr = _bplanes[i];

		center = grey_ptr[convertTo16Bit<ColorMask>(pixels[4])];
		diff_ptr = _greyscaleDiffs[i];

		/* calculate the delta from center pixel */
		diff_ptr[0] = bptr[0] - center;
		diff_ptr[1] = bptr[1] - center;
		diff_ptr[2] = bptr[2] - center;
		diff_ptr[3] = bptr[3] - center;
		diff_ptr[4] = bptr[5] - center;
		diff_ptr[5] = bptr[6] - center;
		diff_ptr[6] = bptr[7] - center;
		diff_ptr[7] = bptr[8] - center;

		/* calculate sum of squares distance */
		for (j = 8; j; --j) {
			sum_diffs += *diff_ptr * *diff_ptr;
			++diff_ptr;
		}

		scores[i] = sum_diffs;
	}

	/* choose greyscale with highest score, ties decided in GRB order */

	if (scores[1] >= scores[0] && scores[1] >= scores[2]) {
		if (!scores[1]) return NULL;

		_chosenGreyscale = _greyscaleTable[1];
		_bptr = _bplanes[1];
		return _greyscaleDiffs[1];
	}

	if (scores[0] >= scores[1] && scores[0] >= scores[2]) {
		if (!scores[0]) return NULL;

		_chosenGreyscale = _greyscaleTable[0];
		_bptr = _bplanes[0];
		return _greyscaleDiffs[0];
	}

	if (!scores[2]) return NULL;

	_chosenGreyscale = _greyscaleTable[2];
	_bptr = _bplanes[2];
	return _greyscaleDiffs[2];
}


template<typename ColorMask>
int32 EdgeScaler::calcPixelDiffNosqrt(typename ColorMask::PixelType pixel1, typename ColorMask::PixelType pixel2) {
	pixel1 = convertTo16Bit<ColorMask>(pixel1);
	pixel2 = convertTo16Bit<ColorMask>(pixel2);

#if 1   /* distance between pixels, weighted by roughly luma proportions */
	int32 sum = 0;
	int16 *rgb_ptr1 = _rgbTable[pixel1];
	int16 *rgb_ptr2 = _rgbTable[pixel2];
	int16 diff;

	diff = (*rgb_ptr1++ - *rgb_ptr2++) << 1;
	sum += diff * diff;
	diff = (*rgb_ptr1++ - *rgb_ptr2++) << 2;
	sum += diff * diff;
	diff = (*rgb_ptr1 - *rgb_ptr2);
	sum += diff * diff;

	return sum;
#endif

#if 0   /* distance between pixels, weighted by chosen greyscale proportions */
	int32 sum = 0;
	int16 *rgb_ptr1 = _rgbTable[pixel1];
	int16 *rgb_ptr2 = _rgbTable[pixel2];
	int16 diff;
	int r_shift, g_shift, b_shift;

	if (_chosenGreyscale == _greyscaleTable[1]) {
		r_shift = 1;
		g_shift = 2;
		b_shift = 0;
	} else if (_chosenGreyscale == _greyscaleTable[0]) {
		r_shift = 2;
		g_shift = 1;
		b_shift = 0;
	} else {
		r_shift = 0;
		g_shift = 1;
		b_shift = 2;
	}

	diff = (*rgb_ptr1++ - *rgb_ptr2++) << r_shift;
	sum += diff * diff;
	diff = (*rgb_ptr1++ - *rgb_ptr2++) << g_shift;
	sum += diff * diff;
	diff = (*rgb_ptr1 - *rgb_ptr2) << b_shift;
	sum += diff * diff;

	return sum;
#endif

#if 0   /* distance between pixels, unweighted */
	int32 sum = 0;
	int16 *rgb_ptr1 = _rgbTable[pixel1];
	int16 *rgb_ptr2 = _rgbTable[pixel2];
	int16 diff;

	diff = *rgb_ptr1++ - *rgb_ptr2++;
	sum += diff * diff;
	diff = *rgb_ptr1++ - *rgb_ptr2++;
	sum += diff * diff;
	diff = *rgb_ptr1 - *rgb_ptr2;
	sum += diff * diff;

	return sum;
#endif

#if 0   /* use the greyscale directly */
	return labs(_chosenGreyscale[pixel1] - _chosenGreyscale[pixel2]);
#endif
}


int EdgeScaler::findPrincipleAxis(int16 *diffs, int16 *bplane,
								  int8 *sim,
								  int32 *return_angle) {
	struct xy_point {
		int16 x, y;
	};

	int i;
	int16 centx = 0, centy = 0;
	struct xy_point xy_points[9];
	int angle;
	int reverse_flag = 1;
	int16 cutoff;
	int16 max_diff;

	double x, y;
	int32 half_matrix[3] = {0};

	double eigenval1, eigenval2;
	double best_val;
	double a, b, c;
	double ratio;
	int32 colorScale;

	/* absolute value of differences */
	for (i = 0; i < 8; i++)
		diffs[i] = labs(diffs[i]);

	/* find the max difference */
	max_diff = *diffs;
	for (i = 1; i < 8; i++)
		if (diffs[i] > max_diff) max_diff = diffs[i];

	/* exit early on uniform window */
	/* already taken care of earlier elsewhere after greyscale assignment */
	/* if (max_diff == 0) return '0'; */

	/* normalize the differences */
	colorScale = (1L << (GREY_SHIFT + GREY_SHIFT)) / max_diff;
	for (i = 0; i < 8; i++)
		diffs[i] = (diffs[i] * colorScale + ((int16)1 << (GREY_SHIFT - 1))) >> GREY_SHIFT;

	/*
	 * Some pixel patterns need to NOT be reversed, since the pixels of
	 * interest that form the edge to be detected are off-center.
	 *
	 */

	/* calculate yes/no similarity matrix to center pixel */
	/* store the number of similar pixels */
	cutoff = ((int16)1 << (GREY_SHIFT - 3));
	for (i = 0, _simSum = 0; i < 8; i++)
		_simSum += (sim[i] = (diffs[i] < cutoff));

	/* don't reverse pattern for off-center knights and sharp corners */
	if (_simSum >= 3 && _simSum <= 5) {
		/* |. */ /* '- */
		if (sim[1] && sim[4] && sim[5] && !sim[3] && !sim[6] &&
		        (!sim[0] ^ !sim[7]))
			reverse_flag = 0;

		/* -. */ /* '| */
		else if (sim[2] && sim[3] && sim[6] && !sim[1] && !sim[4] &&
		         (!sim[0] ^ !sim[7]))
			reverse_flag = 0;

		/* .- */ /* |' */
		else if (sim[4] && sim[6] && sim[0] && !sim[1] && !sim[3] &&
		         (!sim[2] ^ !sim[5]))
			reverse_flag = 0;

		/* .| */ /* -' */
		else if (sim[1] && sim[3] && sim[7] && !sim[4] && !sim[6] &&
		         (!sim[2] ^ !sim[5]))
			reverse_flag = 0;

		/* 90 degree corners */
		else if (_simSum == 3) {
			if ((sim[0] && sim[1] && sim[3]) ||
			        (sim[1] && sim[2] && sim[4]) ||
			        (sim[3] && sim[5] && sim[6]) ||
			        (sim[4] && sim[6] && sim[7]))
				reverse_flag = 0;
		}
	}

	/* redo similarity array, less stringent for later checks */
	cutoff = ((int16)1 << (GREY_SHIFT - 1));
	for (i = 0, _simSum = 0; i < 8; i++)
		_simSum += (sim[i] = (diffs[i] < cutoff));

	/* center pixel is different from all the others, not an edge */
	if (_simSum == 0) return '0';

	/* reverse the difference array, so most similar is closest to 1 */
	if (reverse_flag) {
		diffs[0] = ((int16)1 << GREY_SHIFT) - diffs[0];
		diffs[1] = ((int16)1 << GREY_SHIFT) - diffs[1];
		diffs[2] = ((int16)1 << GREY_SHIFT) - diffs[2];
		diffs[3] = ((int16)1 << GREY_SHIFT) - diffs[3];
		diffs[4] = ((int16)1 << GREY_SHIFT) - diffs[4];
		diffs[5] = ((int16)1 << GREY_SHIFT) - diffs[5];
		diffs[6] = ((int16)1 << GREY_SHIFT) - diffs[6];
		diffs[7] = ((int16)1 << GREY_SHIFT) - diffs[7];
	}

	/* scale diagonals for projection onto axes */
	diffs[0] = (diffs[0] * one_sqrt2 + ((int16)1 << (GREY_SHIFT - 1))) >> GREY_SHIFT;
	diffs[2] = (diffs[2] * one_sqrt2 + ((int16)1 << (GREY_SHIFT - 1))) >> GREY_SHIFT;
	diffs[5] = (diffs[5] * one_sqrt2 + ((int16)1 << (GREY_SHIFT - 1))) >> GREY_SHIFT;
	diffs[7] = (diffs[7] * one_sqrt2 + ((int16)1 << (GREY_SHIFT - 1))) >> GREY_SHIFT;

	/* create the vectors, centered at 0,0 */
	xy_points[0].x = -diffs[0];
	xy_points[0].y = diffs[0];
	xy_points[1].x = 0;
	xy_points[1].y = diffs[1];
	xy_points[2].x = xy_points[2].y = diffs[2];
	xy_points[3].x = -diffs[3];
	xy_points[3].y = 0;
	xy_points[4].x = 0;
	xy_points[4].y = 0;
	xy_points[5].x = diffs[4];
	xy_points[5].y = 0;
	xy_points[6].x = xy_points[6].y = -diffs[5];
	xy_points[7].x = 0;
	xy_points[7].y = -diffs[6];
	xy_points[8].x = diffs[7];
	xy_points[8].y = -diffs[7];

	/* calculate the centroid of the points */
	for (i = 0; i < 9; i++) {
		centx += xy_points[i].x;
		centy += xy_points[i].y;
	}
	centx /= 9;
	centy /= 9;

	/* translate centroid to 0,0 */
	for (i = 0; i < 9; i++) {
		xy_points[i].x -= centx;
		xy_points[i].y -= centy;
	}

	/* fill inertia tensor 3x3 matrix */
	for (i = 0; i < 9; i++) {
		half_matrix[0] += xy_points[i].x * xy_points[i].x;
		half_matrix[1] += xy_points[i].y * xy_points[i].x;
		half_matrix[2] += xy_points[i].y * xy_points[i].y;
	}

	/* calculate eigenvalues */
	a = half_matrix[0] - half_matrix[2];
	b = half_matrix[1] << 1;
	b = sqrt(b * b + a * a);
	a = half_matrix[0] + half_matrix[2];
	eigenval1 = (a + b);
	eigenval2 = (a - b);

	/* find largest eigenvalue */
	if (eigenval1 == eigenval2) /* X and + shapes */
		return '*';
	else if (eigenval1 > eigenval2)
		best_val = eigenval1;
	else
		best_val = eigenval2;

	/* white center pixel, black background */
	if (!best_val)
		return '*';

	/* divide eigenvalue by 2, postponed from when it should have been
	   done during the eigenvalue calculation but was delayed for
	   another early exit opportunity */
	best_val *= 0.5;

	/* calculate eigenvectors */
	c = best_val + half_matrix[1];
	a = c - half_matrix[0];
	b = c - half_matrix[2];
	if (b) {
		x = 1.0;
		ratio = y = a / b;
	} else if (a) {
		y = 1.0;
		x = b / a;
		ratio = a / b;
	} else if (a == b) {
		*return_angle = 13500;
		return '\\';
	} else
		return '*';

	/* calculate angle in degrees * 100 */
	if (x) {
		angle = (int32) floor(5729.577951307 * fastAtan(ratio) + 0.5);
		if (x < 0.0) angle += 18000;
	} else {
		if (y > 0.0) angle = 9000;
		else if (y < 0.0) angle = -9000;
		else return '0';
	}

	/* force angle to lie between 0 to 360 */
	if (angle < 0) angle += 36000;
	if (angle > 18000) angle -= 18000;
	*return_angle = angle;

	if (angle <= 2250)
		return '-';

	if (angle < 6750)
		return '/';

	if (angle <= 11250)
		return '|';

	if (angle < 15750)
		return '\\';

	return '-';
}


template<typename Pixel>
int EdgeScaler::checkArrows(int best_dir, Pixel *pixels, int8 *sim, int half_flag) {
	Pixel center = pixels[4];

	if (center == pixels[0] && center == pixels[2] &&
	        center == pixels[6] && center == pixels[8]) {
		switch (best_dir) {
		case 5:
			if (center != pixels[5])    /* < */
				return 0;
			break;
		case 6:
			if (center != pixels[3])    /* > */
				return 0;
			break;
		case 7:
			if (center != pixels[7])    /* ^ */
				return 0;
			break;
		case 8:
			if (center != pixels[1])    /* v */
				return 0;
			break;
		}
	}

	switch (best_dir) {
	case 5:     /* < */
		if (center == pixels[2] && center == pixels[8] &&
		        pixels[1] == pixels[5] && pixels[5] == pixels[7] &&
		        (((center == pixels[0]) ^ (center == pixels[6])) ||
		         (center == pixels[0] && center == pixels[6] &&
		          pixels[1] != pixels[3])))
			return 0;
		break;

	case 6:     /* > */
		if (center == pixels[0] && center == pixels[6] &&
		        pixels[1] == pixels[3] && pixels[3] == pixels[7] &&
		        (((center == pixels[2]) ^ (center == pixels[8])) ||
		         (center == pixels[2] && center == pixels[8] &&
		          pixels[1] != pixels[5])))
			return 0;
		break;

	case 7:     /* ^ */
		if (center == pixels[6] && center == pixels[8] &&
		        pixels[3] == pixels[7] && pixels[7] == pixels[5] &&
		        (((center == pixels[0]) ^ (center == pixels[2])) ||
		         (center == pixels[0] && center == pixels[2] &&
		          pixels[3] != pixels[1])))
			return 0;
		break;

	case 8:     /* v */
		if (center == pixels[0] && center == pixels[2] &&
		        pixels[1] == pixels[3] && pixels[1] == pixels[5] &&
		        (((center == pixels[6]) ^ (center == pixels[8])) ||
		         (center == pixels[6] && center == pixels[8] &&
		          pixels[3] != pixels[7])))
			return 0;
		break;
	}

	switch (best_dir) {
	case 5:
		if (sim[0] == sim[5] &&
		        sim[1] == sim[3] &&
		        sim[3] == sim[6] &&
		        ((sim[2] && sim[7]) ||
		         (half_flag && _simSum == 2 && sim[4] &&
		          (sim[2] || sim[7])))) /* < */
			return 1;
		break;
	case 6:
		if (sim[2] == sim[7] &&
		        sim[1] == sim[4] &&
		        sim[4] == sim[6] &&
		        ((sim[0] && sim[5]) ||
		         (half_flag && _simSum == 2 && sim[3] &&
		          (sim[0] || sim[5])))) /* > */
			return 1;
		break;
	case 7:
		if (sim[0] == sim[2] &&
		        sim[1] == sim[3] &&
		        sim[3] == sim[4] &&
		        ((sim[5] && sim[7]) ||
		         (half_flag && _simSum == 2 && sim[6] &&
		          (sim[5] || sim[7])))) /* ^ */
			return 1;
		break;
	case 8:
		if (sim[5] == sim[7] &&
		        sim[3] == sim[6] &&
		        sim[4] == sim[6] &&
		        ((sim[0] && sim[2]) ||
		         (half_flag && _simSum == 2 && sim[1] &&
		          (sim[0] || sim[2])))) /* v */
			return 1;
		break;
	}

	return 0;
}


template<typename Pixel>
int EdgeScaler::refineDirection(char edge_type, Pixel *pixels, int16 *bptr,
								int8 *sim, double angle) {
	int32 sums_dir[9] = { 0 };
	int32 sum;
	int32 best_sum;
	int i, n, best_dir;
	int16 diff_array[26];
	int ok_arrow_flag = 1;

	/*
	 * -   '-  -.  \   '|  |.  |   |'  .|  /   -'  .-  <   >   ^   v
	 *
	 * 0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15
	 *
	 * \'  .\  '/  /.
	 *
	 * 16  17  18  19
	 *
	 */

	switch (edge_type) {
	case '|':
		diff_array[0]  = labs(bptr[4] - bptr[1]);
		diff_array[1]  = labs(bptr[4] - bptr[7]);
		diff_array[2]  = labs(bptr[3] - bptr[0]);
		diff_array[3]  = labs(bptr[3] - bptr[6]);
		diff_array[4]  = labs(bptr[5] - bptr[2]);
		diff_array[5]  = labs(bptr[5] - bptr[8]);
		diff_array[6]  = labs(bptr[4] - bptr[2]);
		diff_array[7]  = labs(bptr[4] - bptr[8]);
		diff_array[8]  = labs(bptr[3] - bptr[1]);
		diff_array[9]  = labs(bptr[3] - bptr[7]);
		diff_array[10] = labs(bptr[4] - bptr[0]);
		diff_array[11] = labs(bptr[4] - bptr[6]);
		diff_array[12] = labs(bptr[5] - bptr[1]);
		diff_array[13] = labs(bptr[5] - bptr[7]);
		diff_array[14] = labs(bptr[0] - bptr[6]);
		diff_array[15] = labs(bptr[2] - bptr[8]);
		diff_array[16] = labs(bptr[1] - bptr[7]);
		diff_array[17] = labs(bptr[0] - bptr[1]);
		diff_array[18] = labs(bptr[2] - bptr[1]);
		diff_array[19] = labs(bptr[0] - bptr[2]);
		diff_array[20] = labs(bptr[3] - bptr[5]);
		diff_array[21] = labs(bptr[6] - bptr[8]);
		diff_array[22] = labs(bptr[6] - bptr[7]);
		diff_array[23] = labs(bptr[8] - bptr[7]);

		/* | vertical */
		sums_dir[0] = diff_array[0] + diff_array[1] + diff_array[2] +
		              diff_array[3] + diff_array[4] + diff_array[5];

		/* << top */
		sum = diff_array[8] + diff_array[9] +
		      ((diff_array[6] + diff_array[7] +
		        diff_array[12] + diff_array[13]) << 1) +
		      diff_array[14] + diff_array[16] + diff_array[15];
		sum = (sum * 6) / 13;
		sums_dir[5] = sum;

		/* >> top */
		sum = diff_array[12] + diff_array[13] +
		      ((diff_array[10] + diff_array[11] +
		        diff_array[8] + diff_array[9]) << 1) +
		      diff_array[15] + diff_array[16] + diff_array[14];
		sum = (sum * 6) / 13;
		sums_dir[6] = sum;

		/* ^ bottom */
		sum = diff_array[8] + diff_array[12] +
		      ((diff_array[11] + diff_array[7]) << 1) +
		      (diff_array[1] << 2) +
		      diff_array[19] + diff_array[20] + diff_array[21];
		sum = (sum * 6) / 13;
		sums_dir[7] = sum;

		/* v bottom */
		sum = diff_array[9] + diff_array[13] +
		      ((diff_array[10] + diff_array[6]) << 1) +
		      (diff_array[0] << 2) +
		      diff_array[21] + diff_array[20] + diff_array[19];
		sum = (sum * 6) / 13;
		sums_dir[8] = sum;

		/* '| */
		sums_dir[1] = diff_array[1] + diff_array[5] + diff_array[10] +
		              diff_array[12] + (diff_array[3] << 1);
		/* '| alt */
		sum = diff_array[10] + diff_array[1] + diff_array[18] +
		      diff_array[4] + diff_array[5] + diff_array[14];
		if (sum < sums_dir[1])
			sums_dir[1] = sum;

		/* |. */
		sums_dir[2] = diff_array[0] + diff_array[2] + diff_array[7] +
		              diff_array[9] + (diff_array[4] << 1);
		/* |. alt */
		sum = diff_array[0] + diff_array[7] + diff_array[22] +
		      diff_array[3] + diff_array[2] + diff_array[15];
		if (sum < sums_dir[2])
			sums_dir[2] = sum;

		/* |' */
		sums_dir[3] = diff_array[1] + diff_array[3] + diff_array[6] +
		              diff_array[8] + (diff_array[5] << 1);
		/* |' alt */
		sum = diff_array[6] + diff_array[1] + diff_array[17] +
		      diff_array[2] + diff_array[3] + diff_array[15];
		if (sum < sums_dir[3])
			sums_dir[3] = sum;

		/* .| */
		sums_dir[4] = diff_array[0] + diff_array[4] + diff_array[11] +
		              diff_array[13] + (diff_array[2] << 1);
		/* .| alt */
		sum = diff_array[11] + diff_array[0] + diff_array[23] +
		      diff_array[5] + diff_array[4] + diff_array[14];
		if (sum < sums_dir[4])
			sums_dir[4] = sum;

		best_sum = sums_dir[0];
		for (i = 1; i < 9; i++)
			if (sums_dir[i] < best_sum) best_sum = sums_dir[i];
		if (best_sum == sums_dir[0]) return 6;  /* | */

		best_dir = 0;
		for (i = 0, n = 0; i < 9; i++) {
			if (sums_dir[i] == best_sum) {
				best_dir = i;
				n++;
			}
		}

		/* best direction uncertain, return original direction */
		if (n > 1) return 6;    /* | */

		if (best_dir >= 5)
			ok_arrow_flag = checkArrows<Pixel>(best_dir, pixels, sim, 1);

		switch (best_dir) {
		case 1:
			return 4;       /* '| */
			break;
		case 2:
			return 5;       /* |. */
			break;
		case 3:
			return 7;       /* |' */
			break;
		case 4:
			return 8;       /* .| */
			break;
		case 5:
			if (ok_arrow_flag)
				return 12;      /* < */
			break;
		case 6:
			if (ok_arrow_flag)
				return 13;      /* > */
			break;
		case 7:
			if (ok_arrow_flag)
				return 14;      /* ^ */
			break;
		case 8:
			if (ok_arrow_flag)
				return 15;      /* V */
			break;
		case 0:
		default:
			return 6;       /* | */
			break;
		}

		break;

	case '-':
		diff_array[0]  = labs(bptr[4] - bptr[3]);
		diff_array[1]  = labs(bptr[4] - bptr[5]);
		diff_array[2]  = labs(bptr[0] - bptr[1]);
		diff_array[3]  = labs(bptr[1] - bptr[2]);
		diff_array[4]  = labs(bptr[7] - bptr[6]);
		diff_array[5]  = labs(bptr[7] - bptr[8]);
		diff_array[6]  = labs(bptr[4] - bptr[6]);
		diff_array[7]  = labs(bptr[4] - bptr[8]);
		diff_array[8]  = labs(bptr[1] - bptr[3]);
		diff_array[9]  = labs(bptr[1] - bptr[5]);
		diff_array[10] = labs(bptr[4] - bptr[0]);
		diff_array[11] = labs(bptr[4] - bptr[2]);
		diff_array[12] = labs(bptr[7] - bptr[3]);
		diff_array[13] = labs(bptr[7] - bptr[5]);
		diff_array[14] = labs(bptr[0] - bptr[2]);
		diff_array[15] = labs(bptr[6] - bptr[8]);
		diff_array[16] = labs(bptr[3] - bptr[5]);
		diff_array[17] = labs(bptr[0] - bptr[3]);
		diff_array[18] = labs(bptr[6] - bptr[3]);
		diff_array[19] = labs(bptr[0] - bptr[6]);
		diff_array[20] = labs(bptr[1] - bptr[7]);
		diff_array[21] = labs(bptr[2] - bptr[8]);
		diff_array[22] = labs(bptr[2] - bptr[5]);
		diff_array[23] = labs(bptr[8] - bptr[5]);

		/* - horizontal */
		sums_dir[0] = diff_array[0] + diff_array[1] + diff_array[2] +
		              diff_array[3] + diff_array[4] + diff_array[5];

		/* << bottom */
		sum = diff_array[8] + diff_array[12] +
		      ((diff_array[11] + diff_array[7]) << 1) +
		      (diff_array[1] << 2) +
		      diff_array[19] + diff_array[20] + diff_array[21];
		sum = (sum * 6) / 13;
		sums_dir[5] = sum;

		/* >> bottom */
		sum = diff_array[9] + diff_array[13] +
		      ((diff_array[10] + diff_array[6]) << 1) +
		      (diff_array[0] << 2) +
		      diff_array[21] + diff_array[20] + diff_array[19];
		sum = (sum * 6) / 13;
		sums_dir[6] = sum;

		/* ^ top */
		sum = diff_array[8] + diff_array[9] +
		      ((diff_array[6] + diff_array[7] +
		        diff_array[12] + diff_array[13]) << 1) +
		      diff_array[14] + diff_array[16] + diff_array[15];
		sum = (sum * 6) / 13;
		sums_dir[7] = sum;

		/* v top */
		sum = diff_array[12] + diff_array[13] +
		      ((diff_array[10] + diff_array[11] +
		        diff_array[8] + diff_array[9]) << 1) +
		      diff_array[15] + diff_array[16] + diff_array[14];
		sum = (sum * 6) / 13;
		sums_dir[8] = sum;

		/* '- */
		sums_dir[1] = diff_array[1] + diff_array[5] + diff_array[10] +
		              diff_array[12] + (diff_array[3] << 1);
		/* '- alt */
		sum = diff_array[10] + diff_array[1] + diff_array[18] +
		      diff_array[4] + diff_array[5] + diff_array[14];
		if (sum < sums_dir[1])
			sums_dir[1] = sum;

		/* -. */
		sums_dir[2] = diff_array[0] + diff_array[2] + diff_array[7] +
		              diff_array[9] + (diff_array[4] << 1);
		/* -. alt */
		sum = diff_array[0] + diff_array[7] + diff_array[22] +
		      diff_array[3] + diff_array[2] + diff_array[15];
		if (sum < sums_dir[2])
			sums_dir[2] = sum;

		/* -' */
		sums_dir[3] = diff_array[0] + diff_array[4] + diff_array[11] +
		              diff_array[13] + (diff_array[2] << 1);
		/* -' alt */
		sum = diff_array[11] + diff_array[0] + diff_array[23] +
		      diff_array[5] + diff_array[4] + diff_array[14];
		if (sum < sums_dir[3])
			sums_dir[3] = sum;

		/* .- */
		sums_dir[4] = diff_array[1] + diff_array[3] + diff_array[6] +
		              diff_array[8] + (diff_array[5] << 1);
		/* .- alt */
		sum = diff_array[6] + diff_array[1] + diff_array[17] +
		      diff_array[2] + diff_array[3] + diff_array[15];
		if (sum < sums_dir[4])
			sums_dir[4] = sum;

		best_sum = sums_dir[0];
		for (i = 1; i < 9; i++)
			if (sums_dir[i] < best_sum) best_sum = sums_dir[i];
		if (best_sum == sums_dir[0]) return 0;  /* - */

		best_dir = 0;
		for (i = 0, n = 0; i < 9; i++) {
			if (sums_dir[i] == best_sum) {
				best_dir = i;
				n++;
			}
		}

		/* best direction uncertain, return original direction */
		if (n > 1) return 0;    /* - */

		if (best_dir >= 5)
			ok_arrow_flag = checkArrows<Pixel>(best_dir, pixels, sim, 1);

		switch (best_dir) {
		case 1:
			return 1;       /* '- */
			break;
		case 2:
			return 2;       /* -. */
			break;
		case 3:
			return 10;      /* -' */
			break;
		case 4:
			return 11;      /* .- */
			break;
		case 5:
			if (ok_arrow_flag)
				return 12;      /* < */
			break;
		case 6:
			if (ok_arrow_flag)
				return 13;      /* > */
			break;
		case 7:
			if (ok_arrow_flag)
				return 14;      /* ^ */
			break;
		case 8:
			if (ok_arrow_flag)
				return 15;      /* V */
			break;
		case 0:
		default:
			return 0;       /* - */
			break;
		}

		break;

	case '\\':

		/* CHECK -- handle noisy half-diags */
		if (_simSum == 1) {
			if (pixels[1] == pixels[3] && pixels[3] == pixels[5] &&
			        pixels[5] == pixels[7]) {
				if (pixels[2] != pixels[1] && pixels[6] != pixels[1]) {
					sum = labs(bptr[2] - bptr[4]) +
					      labs(bptr[6] - bptr[4]);

					if (sim[0] && sum < (labs(bptr[8] - bptr[4]) << 1)) {
						if (bptr[4] > bptr[8]) {
							if (bptr[2] > bptr[4] &&
							        bptr[6] > bptr[4])
								return 18;      /* '/ */
						} else {
							if (bptr[2] < bptr[4] &&
							        bptr[6] < bptr[4])
								return 18;      /* '/ */
						}
					}

					if (sim[7] && sum < (labs(bptr[0] - bptr[4]) << 1)) {
						if (bptr[4] > bptr[0]) {
							if (bptr[2] > bptr[4] &&
							        bptr[6] > bptr[4])
								return 19;      /* /. */
						} else {
							if (bptr[2] < bptr[4] &&
							        bptr[6] < bptr[4])
								return 19;      /* /. */
						}
					}
				}
			}

			if (sim[0] &&
			        labs(bptr[4] - bptr[0]) < ((int16)1 << (GREY_SHIFT - 3)))
				return 3;   /* \ */
			if (sim[7] &&
			        labs(bptr[4] - bptr[8]) < ((int16)1 << (GREY_SHIFT - 3)))
				return 3;   /* \ */
		}

		diff_array[0]  = labs(bptr[4] - bptr[0]);
		diff_array[1]  = labs(bptr[4] - bptr[5]);
		diff_array[2]  = labs(bptr[3] - bptr[7]);
		diff_array[3]  = labs(bptr[7] - bptr[8]);
		diff_array[4]  = labs(bptr[1] - bptr[2]);
		diff_array[5]  = labs(bptr[4] - bptr[3]);
		diff_array[6]  = labs(bptr[4] - bptr[8]);
		diff_array[7]  = labs(bptr[0] - bptr[1]);
		diff_array[8]  = labs(bptr[1] - bptr[5]);
		diff_array[9]  = labs(bptr[6] - bptr[7]);
		diff_array[10] = labs(bptr[4] - bptr[7]);
		diff_array[11] = labs(bptr[5] - bptr[8]);
		diff_array[12] = labs(bptr[3] - bptr[6]);
		diff_array[13] = labs(bptr[4] - bptr[1]);
		diff_array[14] = labs(bptr[0] - bptr[3]);
		diff_array[15] = labs(bptr[2] - bptr[5]);
		diff_array[20] = labs(bptr[0] - bptr[2]);
		diff_array[21] = labs(bptr[6] - bptr[8]);
		diff_array[22] = labs(bptr[0] - bptr[6]);
		diff_array[23] = labs(bptr[2] - bptr[8]);
		diff_array[16] = labs(bptr[4] - bptr[2]);
		diff_array[18] = labs(bptr[4] - bptr[6]);

		/* '- */
		sums_dir[1] = diff_array[0] + diff_array[1] + diff_array[2] +
		              diff_array[3] + (diff_array[4] << 1);
		/* '- alt */
		sum = diff_array[0] + diff_array[1] + diff_array[12] +
		      diff_array[9] + diff_array[3] + diff_array[20];
		if (sum < sums_dir[1])
			sums_dir[1] = sum;

		/* -. */
		sums_dir[2] = diff_array[5] + diff_array[6] + diff_array[7] +
		              diff_array[8] + (diff_array[9] << 1);
		/* -. alt */
		sum = diff_array[6] + diff_array[5] + diff_array[15] +
		      diff_array[4] + diff_array[7] + diff_array[21];
		if (sum < sums_dir[2])
			sums_dir[2] = sum;

		/* '| */
		sums_dir[3] = diff_array[0] + diff_array[8] + diff_array[10] +
		              diff_array[11] + (diff_array[12] << 1);
		/* '| alt */
		sum = diff_array[0] + diff_array[10] + diff_array[4] +
		      diff_array[15] + diff_array[11] + diff_array[22];
		if (sum < sums_dir[3])
			sums_dir[3] = sum;

		/* |. */
		sums_dir[4] = diff_array[2] + diff_array[6] + diff_array[13] +
		              diff_array[14] + (diff_array[15] << 1);
		/* |. alt */
		sum = diff_array[13] + diff_array[6] + diff_array[9] +
		      diff_array[12] + diff_array[14] + diff_array[23];
		if (sum < sums_dir[4])
			sums_dir[4] = sum;

		/* \ 45 */
		sums_dir[0] = diff_array[0] + diff_array[6] +
		              (diff_array[2] << 1) + (diff_array[8] << 1);

		/* << top */
		sum = labs(bptr[3] - bptr[1]) + labs(bptr[3] - bptr[7]) +
		      ((labs(bptr[4] - bptr[2]) + labs(bptr[4] - bptr[8]) +
		        labs(bptr[5] - bptr[1]) + labs(bptr[5] - bptr[7])) << 1) +
		      labs(bptr[0] - bptr[6]) + labs(bptr[1] - bptr[7]) +
		      labs(bptr[2] - bptr[8]);
		sum = (sum * 6) / 13;
		sums_dir[5] = sum;

		/* >> top */
		sum = labs(bptr[5] - bptr[1]) + labs(bptr[5] - bptr[7]) +
		      ((labs(bptr[4] - bptr[0]) + labs(bptr[4] - bptr[6]) +
		        labs(bptr[3] - bptr[1]) + labs(bptr[3] - bptr[7])) << 1) +
		      labs(bptr[2] - bptr[8]) + labs(bptr[1] - bptr[7]) +
		      labs(bptr[0] - bptr[6]);
		sum = (sum * 6) / 13;
		sums_dir[6] = sum;

		/* ^ top */
		sum = labs(bptr[1] - bptr[3]) + labs(bptr[1] - bptr[5]) +
		      ((labs(bptr[4] - bptr[6]) + labs(bptr[4] - bptr[8]) +
		        labs(bptr[7] - bptr[3]) + labs(bptr[7] - bptr[5])) << 1) +
		      labs(bptr[0] - bptr[2]) + labs(bptr[3] - bptr[5]) +
		      labs(bptr[6] - bptr[8]);
		sum = (sum * 6) / 13;
		sums_dir[7] = sum;

		/* v top */
		sum = labs(bptr[7] - bptr[3]) + labs(bptr[7] - bptr[5]) +
		      ((labs(bptr[4] - bptr[0]) + labs(bptr[4] - bptr[2]) +
		        labs(bptr[1] - bptr[3]) + labs(bptr[1] - bptr[5])) << 1) +
		      labs(bptr[6] - bptr[8]) + labs(bptr[3] - bptr[5]) +
		      labs(bptr[0] - bptr[2]);
		sum = (sum * 6) / 13;
		sums_dir[8] = sum;

		best_sum = sums_dir[0];
		for (i = 1; i < 9; i++)
			if (sums_dir[i] < best_sum) best_sum = sums_dir[i];

		best_dir = 0;
		for (i = 0, n = 0; i < 9; i++) {
			if (sums_dir[i] == best_sum) {
				best_dir = i;
				n++;
			}
		}

		/* CHECK -- handle zig-zags */
		if (_simSum == 3) {
			if ((best_dir == 0 || best_dir == 1) &&
			        sim[0] && sim[1] && sim[4])
				return 1;               /* '- */
			if ((best_dir == 0 || best_dir == 2) &&
			        sim[3] && sim[6] && sim[7])
				return 2;               /* -. */
			if ((best_dir == 0 || best_dir == 3) &&
			        sim[0] && sim[3] && sim[6])
				return 4;               /* '| */
			if ((best_dir == 0 || best_dir == 4) &&
			        sim[1] && sim[4] && sim[7])
				return 5;               /* |. */
		}

		if (n > 1 && best_sum == sums_dir[0]) return 3; /* \ */

		/* best direction uncertain, return non-edge to avoid artifacts */
		if (n > 1) return -1;

		/* CHECK -- diagonal intersections */
		if (best_dir == 0 &&
		        (sim[1] == sim[4] || sim[3] == sim[6]) &&
		        (sim[1] == sim[3] || sim[4] == sim[6])) {
			if ((pixels[1] == pixels[3] || pixels[5] == pixels[7]) ||
			        ((pixels[0] == pixels[4]) ^ (pixels[8] == pixels[4]))) {
				if (pixels[2] == pixels[4])
					return 16;      /* \' */
				if (pixels[6] == pixels[4])
					return 17;      /* .\ */
			}

			if (_simSum == 3 && sim[0] && sim[7] &&
			        pixels[1] == pixels[3] && pixels[3] == pixels[5] &&
			        pixels[5] == pixels[7]) {
				if (sim[2])
					return 16;      /* \' */
				if (sim[5])
					return 17;      /* .\ */
			}

			if (_simSum == 3 && sim[2] && sim[5]) {
				if (sim[0])
					return 18;      /* '/ */
				if (sim[7])
					return 19;      /* /. */
			}
		}

		if (best_dir >= 5)
			ok_arrow_flag = checkArrows<Pixel>(best_dir, pixels, sim, 0);

		switch (best_dir) {
		case 1:
			return 1;       /* '- */
			break;
		case 2:
			return 2;       /* -. */
			break;
		case 3:
			return 4;       /* '| */
			break;
		case 4:
			return 5;       /* |. */
			break;
		case 5:
			if (ok_arrow_flag)
				return 12;      /* < */
			break;
		case 6:
			if (ok_arrow_flag)
				return 13;      /* > */
			break;
		case 7:
			if (ok_arrow_flag)
				return 14;      /* ^ */
			break;
		case 8:
			if (ok_arrow_flag)
				return 15;      /* V */
			break;
		case 0:
		default:
			return 3;       /* \ */
			break;
		}

		break;

	case '/':

		/* CHECK -- handle noisy half-diags */
		if (_simSum == 1) {
			if (pixels[1] == pixels[3] && pixels[3] == pixels[5] &&
			        pixels[5] == pixels[7]) {
				if (pixels[0] != pixels[1] && pixels[8] != pixels[1]) {
					sum = labs(bptr[0] - bptr[4]) +
					      labs(bptr[8] - bptr[4]);

					if (sim[2] && sum < (labs(bptr[6] - bptr[4]) << 1)) {
						if (bptr[4] > bptr[6]) {
							if (bptr[0] > bptr[4] &&
							        bptr[8] > bptr[4])
								return 16;      /* \' */
						} else {
							if (bptr[0] < bptr[4] &&
							        bptr[8] < bptr[4])
								return 16;      /* \' */
						}
					}

					if (sim[5] && sum < (labs(bptr[2] - bptr[4]) << 1)) {
						if (bptr[4] > bptr[2]) {
							if (bptr[0] > bptr[4] &&
							        bptr[8] > bptr[4]) {
								if (pixels[6] == pixels[4])
									return 17;      /* .\ */
								return 3;           /* \ */
							}
						} else {
							if (bptr[0] < bptr[4] &&
							        bptr[8] < bptr[4]) {
								if (pixels[6] == pixels[4])
									return 17;      /* .\ */
								return 3;           /* \ */
							}
						}
					}
				}
			}

			if (sim[2] &&
			        labs(bptr[4] - bptr[2]) < ((int16)1 << (GREY_SHIFT - 3)))
				return 9;   /* / */
			if (sim[5] &&
			        labs(bptr[4] - bptr[6]) < ((int16)1 << (GREY_SHIFT - 3)))
				return 9;   /* / */
		}

		diff_array[0]  = labs(bptr[4] - bptr[0]);
		diff_array[1]  = labs(bptr[4] - bptr[5]);
		diff_array[3]  = labs(bptr[7] - bptr[8]);
		diff_array[4]  = labs(bptr[1] - bptr[2]);
		diff_array[5]  = labs(bptr[4] - bptr[3]);
		diff_array[6]  = labs(bptr[4] - bptr[8]);
		diff_array[7]  = labs(bptr[0] - bptr[1]);
		diff_array[9]  = labs(bptr[6] - bptr[7]);
		diff_array[10] = labs(bptr[4] - bptr[7]);
		diff_array[11] = labs(bptr[5] - bptr[8]);
		diff_array[12] = labs(bptr[3] - bptr[6]);
		diff_array[13] = labs(bptr[4] - bptr[1]);
		diff_array[14] = labs(bptr[0] - bptr[3]);
		diff_array[15] = labs(bptr[2] - bptr[5]);
		diff_array[16] = labs(bptr[4] - bptr[2]);
		diff_array[17] = labs(bptr[1] - bptr[3]);
		diff_array[18] = labs(bptr[4] - bptr[6]);
		diff_array[19] = labs(bptr[5] - bptr[7]);
		diff_array[20] = labs(bptr[0] - bptr[2]);
		diff_array[21] = labs(bptr[6] - bptr[8]);
		diff_array[22] = labs(bptr[0] - bptr[6]);
		diff_array[23] = labs(bptr[2] - bptr[8]);

		/* |' */
		sums_dir[1] = diff_array[10] + diff_array[12] + diff_array[16] +
		              diff_array[17] + (diff_array[11] << 1);
		/* |' alt */
		sum = diff_array[16] + diff_array[10] + diff_array[7] +
		      diff_array[14] + diff_array[12] + diff_array[23];
		if (sum < sums_dir[1])
			sums_dir[1] = sum;

		/* .| */
		sums_dir[2] = diff_array[13] + diff_array[15] + diff_array[18] +
		              diff_array[19] + (diff_array[14] << 1);
		/* .| alt */
		sum = diff_array[18] + diff_array[13] + diff_array[3] +
		      diff_array[11] + diff_array[15] + diff_array[22];
		if (sum < sums_dir[2])
			sums_dir[2] = sum;

		/* -' */
		sums_dir[3] = diff_array[5] + diff_array[9] + diff_array[16] +
		              diff_array[19] + (diff_array[7] << 1);
		/* -' alt */
		sum = diff_array[16] + diff_array[5] + diff_array[11] +
		      diff_array[3] + diff_array[9] + diff_array[20];
		if (sum < sums_dir[3])
			sums_dir[3] = sum;

		/* .- */
		sums_dir[4] = diff_array[1] + diff_array[4] + diff_array[17] +
		              diff_array[18] + (diff_array[3] << 1);
		/* .- alt */
		sum = diff_array[18] + diff_array[1] + diff_array[14] +
		      diff_array[7] + diff_array[4] + diff_array[21];
		if (sum < sums_dir[4])
			sums_dir[4] = sum;

		/* / 135 */
		sums_dir[0] = diff_array[16] + diff_array[18] +
		              (diff_array[17] << 1) + (diff_array[19] << 1);

		/* << top */
		sum = labs(bptr[3] - bptr[1]) + labs(bptr[3] - bptr[7]) +
		      ((labs(bptr[4] - bptr[2]) + labs(bptr[4] - bptr[8]) +
		        labs(bptr[5] - bptr[1]) + labs(bptr[5] - bptr[7])) << 1) +
		      labs(bptr[0] - bptr[6]) + labs(bptr[1] - bptr[7]) +
		      labs(bptr[2] - bptr[8]);
		sum = (sum * 6) / 13;
		sums_dir[5] = sum;

		/* >> top */
		sum = labs(bptr[5] - bptr[1]) + labs(bptr[5] - bptr[7]) +
		      ((labs(bptr[4] - bptr[0]) + labs(bptr[4] - bptr[6]) +
		        labs(bptr[3] - bptr[1]) + labs(bptr[3] - bptr[7])) << 1) +
		      labs(bptr[2] - bptr[8]) + labs(bptr[1] - bptr[7]) +
		      labs(bptr[0] - bptr[6]);
		sum = (sum * 6) / 13;
		sums_dir[6] = sum;

		/* ^ top */
		sum = labs(bptr[1] - bptr[3]) + labs(bptr[1] - bptr[5]) +
		      ((labs(bptr[4] - bptr[6]) + labs(bptr[4] - bptr[8]) +
		        labs(bptr[7] - bptr[3]) + labs(bptr[7] - bptr[5])) << 1) +
		      labs(bptr[0] - bptr[2]) + labs(bptr[3] - bptr[5]) +
		      labs(bptr[6] - bptr[8]);
		sum = (sum * 6) / 13;
		sums_dir[7] = sum;

		/* v top */
		sum = labs(bptr[7] - bptr[3]) + labs(bptr[7] - bptr[5]) +
		      ((labs(bptr[4] - bptr[0]) + labs(bptr[4] - bptr[2]) +
		        labs(bptr[1] - bptr[3]) + labs(bptr[1] - bptr[5])) << 1) +
		      labs(bptr[6] - bptr[8]) + labs(bptr[3] - bptr[5]) +
		      labs(bptr[0] - bptr[2]);
		sum = (sum * 6) / 13;
		sums_dir[8] = sum;

		best_sum = sums_dir[0];
		for (i = 1; i < 9; i++)
			if (sums_dir[i] < best_sum) best_sum = sums_dir[i];

		best_dir = 0;
		for (i = 0, n = 0; i < 9; i++) {
			if (sums_dir[i] == best_sum) {
				best_dir = i;
				n++;
			}
		}

		/* CHECK -- handle zig-zags */
		if (_simSum == 3) {
			if ((best_dir == 0 || best_dir == 1) &&
			        sim[2] && sim[4] && sim[6])
				return 7;               /* |' */
			if ((best_dir == 0 || best_dir == 2) &&
			        sim[1] && sim[3] && sim[5])
				return 8;               /* .| */
			if ((best_dir == 0 || best_dir == 3) &&
			        sim[1] && sim[2] && sim[3])
				return 10;              /* -' */
			if ((best_dir == 0 || best_dir == 4) &&
			        sim[4] && sim[5] && sim[6])
				return 11;              /* .- */
		}

		if (n > 1 && best_sum == sums_dir[0]) return 9; /* / */

		/* best direction uncertain, return non-edge to avoid artifacts */
		if (n > 1) return -1;

		/* CHECK -- diagonal intersections */
		if (best_dir == 0 &&
		        (sim[1] == sim[3] || sim[4] == sim[6]) &&
		        (sim[1] == sim[4] || sim[3] == sim[6])) {
			if ((pixels[1] == pixels[5] || pixels[3] == pixels[7]) ||
			        ((pixels[2] == pixels[4]) ^ (pixels[6] == pixels[4]))) {
				if (pixels[0] == pixels[4])
					return 18;      /* '/ */
				if (pixels[8] == pixels[4])
					return 19;      /* /. */
			}

			if (_simSum == 3 && sim[2] && sim[5] &&
			        pixels[1] == pixels[3] && pixels[3] == pixels[5] &&
			        pixels[5] == pixels[7]) {
				if (sim[0])
					return 18;      /* '/ */
				if (sim[7])
					return 19;      /* /. */
			}

			if (_simSum == 3 && sim[0] && sim[7]) {
				if (sim[2])
					return 16;      /* \' */
				if (sim[5])
					return 17;      /* .\ */
			}
		}

		if (best_dir >= 5)
			ok_arrow_flag = checkArrows<Pixel>(best_dir, pixels, sim, 0);

		switch (best_dir) {
		case 1:
			return 7;       /* |' */
			break;
		case 2:
			return 8;       /* .| */
			break;
		case 3:
			return 10;      /* -' */
			break;
		case 4:
			return 11;      /* .- */
			break;
		case 5:
			if (ok_arrow_flag)
				return 12;      /* < */
			break;
		case 6:
			if (ok_arrow_flag)
				return 13;      /* > */
			break;
		case 7:
			if (ok_arrow_flag)
				return 14;      /* ^ */
			break;
		case 8:
			if (ok_arrow_flag)
				return 15;      /* V */
			break;
			break;
		case 0:
		default:
			return 9;       /* / */
			break;
		}

		break;

	case '*':
		return 127;
		break;

	case '0':
	default:
		return -1;
		break;
	}

	return -1;
}


template<typename Pixel>
int EdgeScaler::fixKnights(int sub_type, Pixel *pixels, int8 *sim) {
	Pixel center = pixels[4];
	int dir = sub_type;
	int n = 0;
	int flags[12] = {0};
	int ok_orig_flag = 0;

	/*
	 * -   '-  -.  \   '|  |.  |   |'  .|  /   -'  .-
	 *
	 * 0   1   2   3   4   5   6   7   8   9   10  11
	 *
	 */

	/* check to see if original knight is ok */
	switch (sub_type) {
	case 1:     /* '- */
		if (sim[0] && sim[4] &&
		        !(_simSum == 3 && sim[5] &&
		          pixels[0] == pixels[4] && pixels[6] == pixels[4]))
			ok_orig_flag = 1;
		break;

	case 2:     /* -. */
		if (sim[3] && sim[7] &&
		        !(_simSum == 3 && sim[2] &&
		          pixels[2] == pixels[4] && pixels[8] == pixels[4]))
			ok_orig_flag = 1;
		break;

	case 4:     /* '| */
		if (sim[0] && sim[6] &&
		        !(_simSum == 3 && sim[2] &&
		          pixels[0] == pixels[4] && pixels[2] == pixels[4]))
			ok_orig_flag = 1;
		break;

	case 5:     /* |. */
		if (sim[1] && sim[7] &&
		        !(_simSum == 3 && sim[5] &&
		          pixels[6] == pixels[4] && pixels[8] == pixels[4]))
			ok_orig_flag = 1;
		break;

	case 7:     /* |' */
		if (sim[2] && sim[6] &&
		        !(_simSum == 3 && sim[0] &&
		          pixels[0] == pixels[4] && pixels[2] == pixels[4]))
			ok_orig_flag = 1;
		break;

	case 8:     /* .| */
		if (sim[1] && sim[5] &&
		        !(_simSum == 3 && sim[7] &&
		          pixels[6] == pixels[4] && pixels[8] == pixels[4]))
			ok_orig_flag = 1;
		break;

	case 10:    /* -' */
		if (sim[2] && sim[3] &&
		        !(_simSum == 3 && sim[7] &&
		          pixels[2] == pixels[4] && pixels[8] == pixels[4]))
			ok_orig_flag = 1;
		break;

	case 11:    /* .- */
		if (sim[4] && sim[5] &&
		        !(_simSum == 3 && sim[0] &&
		          pixels[0] == pixels[4] && pixels[6] == pixels[4]))
			ok_orig_flag = 1;
		break;

	default:    /* not a knight */
		return sub_type;
		break;
	}

	/* look for "better" knights */
	if (center == pixels[0] && center == pixels[5]) { /* '- */
		dir = 1;
		flags[dir] = 1;
		n++;
	}
	if (center == pixels[3] && center == pixels[8]) { /* -. */
		dir = 2;
		flags[dir] = 1;
		n++;
	}
	if (center == pixels[0] && center == pixels[7]) { /* '| */
		dir = 4;
		flags[dir] = 1;
		n++;
	}
	if (center == pixels[1] && center == pixels[8]) { /* |. */
		dir = 5;
		flags[dir] = 1;
		n++;
	}
	if (center == pixels[2] && center == pixels[7]) { /* |' */
		dir = 7;
		flags[dir] = 1;
		n++;
	}
	if (center == pixels[1] && center == pixels[6]) { /* .| */
		dir = 8;
		flags[dir] = 1;
		n++;
	}
	if (center == pixels[3] && center == pixels[2]) { /* -' */
		dir = 10;
		flags[dir] = 1;
		n++;
	}
	if (center == pixels[6] && center == pixels[5]) { /* .- */
		dir = 11;
		flags[dir] = 1;
		n++;
	}

	if (n == 0) {
		if (ok_orig_flag) return sub_type;
		return -1;
	}
	if (n == 1) return dir;
	if (n == 2) {
		/* slanted W patterns */
		if (flags[1] && flags[5]) return 3; /* \ */
		if (flags[2] && flags[4]) return 3; /* \ */
		if (flags[7] && flags[11]) return 9;    /* / */
		if (flags[8] && flags[10]) return 9;    /* / */
	}
	if (flags[sub_type] && ok_orig_flag) return sub_type;

	return -1;
}



/* From ScummVM HQ2x/HQ3x scalers (Maxim Stepin and Max Horn) */
#define highBits    0xF7DEF7DE
#define lowBits     0x08210821
#define qhighBits   0xE79CE79C
#define qlowBits    0x18631863
#define redblueMask 0xF81F
#define greenMask   0x07E0

template<typename ColorMask>
void EdgeScaler::antiAliasGridClean3x(uint8 *dptr, int dstPitch,
		typename ColorMask::PixelType *pixels, int sub_type, int16 *bptr) {
	typedef typename ColorMask::PixelType Pixel;

	Pixel *dptr2;
	int16 tmp_grey;
	Pixel center = pixels[4];
	int32 diff1, diff2, diff3;
	Pixel tmp[9];
	Pixel *ptmp;
	int i;

	switch (sub_type) {
	case 1:     /* '- */
		for (i = 0; i < 9; i++)
			tmp[i] = center;

		tmp[6] = interpolate_1_1_1(pixels[3], pixels[7], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[6])];
#if PARANOID_KNIGHTS
		diff1 = labs(bptr[4] - bptr[3]);
		diff2 = labs(bptr[4] - bptr[7]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[6] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[6], pixels[3]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[6], pixels[7]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[6], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[6] = pixels[3];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[6] = pixels[7];
			else
				tmp[6] = pixels[4];

			tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[6])];
			diff1 = labs(bptr[4] - tmp_grey);
			diff2 = labs(bptr[4] - bptr[8]);
			if (diff1 <= diff2)
				tmp[7] = tmp[6];
		}

		break;

	case 2:     /* -. */
		for (i = 0; i < 9; i++)
			tmp[i] = center;

		tmp[2] = interpolate_1_1_1(pixels[1], pixels[5], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[2])];
#if PARANOID_KNIGHTS
		diff1 = labs(bptr[4] - bptr[5]);
		diff2 = labs(bptr[4] - bptr[1]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[2] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[1]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[5]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[2] = pixels[1];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[2] = pixels[5];
			else
				tmp[2] = pixels[4];

			tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[2])];
			diff1 = labs(bptr[4] - tmp_grey);
			diff2 = labs(bptr[4] - bptr[0]);
			if (diff1 <= diff2)
				tmp[1] = tmp[2];
		}

		break;

	case 3:     /* \ */
	case 16:    /* \' */
	case 17:    /* .\ */
		for (i = 0; i < 9; i++)
			tmp[i] = center;

		if (sub_type != 16) {
			tmp[2] = interpolate_1_1_1(pixels[1], pixels[5], center);
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[1]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[5]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[2] = pixels[1];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[2] = pixels[5];
			else
				tmp[2] = pixels[4];
		}

		if (sub_type != 17) {
			tmp[6] = interpolate_1_1_1(pixels[3], pixels[7], center);
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[6], pixels[3]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[6], pixels[7]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[6], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[6] = pixels[3];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[6] = pixels[7];
			else
				tmp[6] = pixels[4];
		}

		break;

	case 4:     /* '| */
		for (i = 0; i < 9; i++)
			tmp[i] = center;

		tmp[2] = interpolate_1_1_1(pixels[1], pixels[5], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[2])];
#if PARANOID_KNIGHTS
		diff1 = labs(bptr[4] - bptr[1]);
		diff2 = labs(bptr[4] - bptr[5]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[2] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[1]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[5]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[2] = pixels[1];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[2] = pixels[5];
			else
				tmp[2] = pixels[4];

			tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[2])];
			diff1 = labs(bptr[4] - tmp_grey);
			diff2 = labs(bptr[4] - bptr[8]);
			if (diff1 <= diff2)
				tmp[5] = tmp[2];
		}

		break;

	case 5:     /* |. */
		for (i = 0; i < 9; i++)
			tmp[i] = center;

		tmp[6] = interpolate_1_1_1(pixels[3], pixels[7], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[6])];
#if PARANOID_KNIGHTS
		diff1 = labs(bptr[4] - bptr[7]);
		diff2 = labs(bptr[4] - bptr[3]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[6] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[6], pixels[3]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[6], pixels[7]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[6], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[6] = pixels[3];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[6] = pixels[7];
			else
				tmp[6] = pixels[4];

			tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[6])];
			diff1 = labs(bptr[4] - tmp_grey);
			diff2 = labs(bptr[4] - bptr[0]);
			if (diff1 <= diff2)
				tmp[3] = tmp[6];
		}

		break;

	case 7:     /* |' */
		for (i = 0; i < 9; i++)
			tmp[i] = center;

		tmp[0] = interpolate_1_1_1(pixels[1], pixels[3], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[0])];
#if PARANOID_KNIGHTS
		diff1 = labs(bptr[4] - bptr[1]);
		diff2 = labs(bptr[4] - bptr[3]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[0] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[1]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[3]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[0] = pixels[1];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[0] = pixels[3];
			else
				tmp[0] = pixels[4];

			tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[0])];
			diff1 = labs(bptr[4] - tmp_grey);
			diff2 = labs(bptr[4] - bptr[6]);
			if (diff1 <= diff2)
				tmp[3] = tmp[0];
		}

		break;

	case 8:     /* .| */
		for (i = 0; i < 9; i++)
			tmp[i] = center;

		tmp[8] = interpolate_1_1_1(pixels[5], pixels[7], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[8])];
#if PARANOID_KNIGHTS
		diff1 = labs(bptr[4] - bptr[7]);
		diff2 = labs(bptr[4] - bptr[5]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[8] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[8], pixels[5]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[8], pixels[7]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[8], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[8] = pixels[5];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[8] = pixels[7];
			else
				tmp[8] = pixels[4];

			tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[8])];
			diff1 = labs(bptr[4] - tmp_grey);
			diff2 = labs(bptr[4] - bptr[2]);
			if (diff1 <= diff2)
				tmp[5] = tmp[8];
		}

		break;

	case 9:     /* / */
	case 18:    /* '/ */
	case 19:    /* /. */
		for (i = 0; i < 9; i++)
			tmp[i] = center;

		if (sub_type != 18) {
			tmp[0] = interpolate_1_1_1(pixels[1], pixels[3], center);
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[1]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[3]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[0] = pixels[1];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[0] = pixels[3];
			else
				tmp[0] = pixels[4];
		}

		if (sub_type != 19) {
			tmp[8] = interpolate_1_1_1(pixels[5], pixels[7], center);
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[8], pixels[5]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[8], pixels[7]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[8], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[8] = pixels[5];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[8] = pixels[7];
			else
				tmp[8] = pixels[4];
		}

		break;

	case 10:    /* -' */
		for (i = 0; i < 9; i++)
			tmp[i] = center;

		tmp[8] = interpolate_1_1_1(pixels[5], pixels[7], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[8])];
#if PARANOID_KNIGHTS
		diff1 = labs(bptr[4] - bptr[5]);
		diff2 = labs(bptr[4] - bptr[7]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[8] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[8], pixels[5]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[8], pixels[7]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[8], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[8] = pixels[5];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[8] = pixels[7];
			else
				tmp[8] = pixels[4];

			tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[8])];
			diff1 = labs(bptr[4] - tmp_grey);
			diff2 = labs(bptr[4] - bptr[6]);
			if (diff1 <= diff2)
				tmp[7] = tmp[8];
		}

		break;

	case 11:    /* .- */
		for (i = 0; i < 9; i++)
			tmp[i] = center;

		tmp[0] = interpolate_1_1_1(pixels[1], pixels[3], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[0])];
#if PARANOID_KNIGHTS
		diff1 = labs(bptr[4] - bptr[3]);
		diff2 = labs(bptr[4] - bptr[1]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[0] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[1]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[3]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[0] = pixels[1];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[0] = pixels[3];
			else
				tmp[0] = pixels[4];

			tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[0])];
			diff1 = labs(bptr[4] - tmp_grey);
			diff2 = labs(bptr[4] - bptr[2]);
			if (diff1 <= diff2)
				tmp[1] = tmp[0];
		}

		break;

	case 12:    /* < */
		for (i = 0; i < 9; i++)
			tmp[i] = center;

		tmp[0] = interpolate_1_1_1(pixels[1], pixels[3], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[0])];
#if PARANOID_ARROWS
		diff1 = labs(bptr[4] - bptr[1]);
		diff2 = labs(bptr[4] - bptr[3]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[0] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[1]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[3]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[0] = pixels[1];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[0] = pixels[3];
			else
				tmp[0] = pixels[4];
		}

		tmp[6] = interpolate_1_1_1(pixels[3], pixels[7], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[6])];
#if PARANOID_ARROWS
		diff1 = labs(bptr[4] - bptr[3]);
		diff2 = labs(bptr[4] - bptr[7]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[6] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[6], pixels[3]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[6], pixels[7]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[6], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[6] = pixels[3];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[6] = pixels[7];
			else
				tmp[6] = pixels[4];
		}

		break;

	case 13:    /* > */
		for (i = 0; i < 9; i++)
			tmp[i] = center;

		tmp[2] = interpolate_1_1_1(pixels[1], pixels[5], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[2])];
#if PARANOID_ARROWS
		diff1 = labs(bptr[4] - bptr[5]);
		diff2 = labs(bptr[4] - bptr[1]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[2] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[1]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[5]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[2] = pixels[1];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[2] = pixels[5];
			else
				tmp[2] = pixels[4];
		}

		tmp[8] = interpolate_1_1_1(pixels[5], pixels[7], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[8])];
#if PARANOID_ARROWS
		diff1 = labs(bptr[4] - bptr[7]);
		diff2 = labs(bptr[4] - bptr[5]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[8] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[8], pixels[5]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[8], pixels[7]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[8], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[8] = pixels[5];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[8] = pixels[7];
			else
				tmp[8] = pixels[4];
		}

		break;

	case 14:    /* ^ */
		for (i = 0; i < 9; i++)
			tmp[i] = center;

		tmp[0] = interpolate_1_1_1(pixels[1], pixels[3], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[0])];
#if PARANOID_ARROWS
		diff1 = labs(bptr[4] - bptr[1]);
		diff2 = labs(bptr[4] - bptr[3]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[0] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[1]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[3]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[0] = pixels[1];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[0] = pixels[3];
			else
				tmp[0] = pixels[4];
		}

		tmp[2] = interpolate_1_1_1(pixels[1], pixels[5], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[2])];
#if PARANOID_ARROWS
		diff1 = labs(bptr[4] - bptr[5]);
		diff2 = labs(bptr[4] - bptr[1]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[2] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[1]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[5]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[2] = pixels[1];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[2] = pixels[5];
			else
				tmp[2] = pixels[4];
		}

		break;

	case 15:    /* v */
		for (i = 0; i < 9; i++)
			tmp[i] = center;

		tmp[6] = interpolate_1_1_1(pixels[3], pixels[7], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[6])];
#if PARANOID_ARROWS
		diff1 = labs(bptr[4] - bptr[3]);
		diff2 = labs(bptr[4] - bptr[7]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[6] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[6], pixels[3]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[6], pixels[7]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[6], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[6] = pixels[3];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[6] = pixels[7];
			else
				tmp[6] = pixels[4];
		}

		tmp[8] = interpolate_1_1_1(pixels[5], pixels[7], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[8])];
#if PARANOID_ARROWS
		diff1 = labs(bptr[4] - bptr[7]);
		diff2 = labs(bptr[4] - bptr[5]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[8] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[8], pixels[5]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[8], pixels[7]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[8], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[8] = pixels[5];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[8] = pixels[7];
			else
				tmp[8] = pixels[4];
		}

		break;

	case 127:   /* * */
	case -1:    /* no edge */
	case 0:     /* - */
	case 6:     /* | */
	default:
		dptr2 = ((Pixel *)(dptr - dstPitch)) - 1;
		*dptr2++ = center;
		*dptr2++ = center;
		*dptr2 = center;
		dptr2 = ((Pixel *) dptr) - 1;
		*dptr2++ = center;
		*dptr2++ = center;
#if DEBUG_REFRESH_RANDOM_XOR
		*dptr2 = center ^ (Pixel)(dxorshift128() * (1L << 16));
#else
		*dptr2 = center;
#endif
		dptr2 = ((Pixel *)(dptr + dstPitch)) - 1;
		*dptr2++ = center;
		*dptr2++ = center;
		*dptr2 = center;

		return;

		break;
	}

	ptmp = tmp;
	dptr2 = ((Pixel *)(dptr - dstPitch)) - 1;
	*dptr2++ = *ptmp++;
	*dptr2++ = *ptmp++;
	*dptr2 = *ptmp++;
	dptr2 = ((Pixel *) dptr) - 1;
	*dptr2++ = *ptmp++;
	*dptr2++ = *ptmp++;
#if DEBUG_REFRESH_RANDOM_XOR
	*dptr2 = *ptmp++ ^ (Pixel)(dxorshift128() * (1L << 16));
#else
	*dptr2 = *ptmp++;
#endif
	dptr2 = ((Pixel *)(dptr + dstPitch)) - 1;
	*dptr2++ = *ptmp++;
	*dptr2++ = *ptmp++;
	*dptr2 = *ptmp;
}


template<typename ColorMask>
void EdgeScaler::antiAliasGrid2x(uint8 *dptr, int dstPitch,
									typename ColorMask::PixelType *pixels, int sub_type, int16 *bptr,
									int8 *sim,
									int interpolate_2x) {
	typedef typename ColorMask::PixelType Pixel;

	Pixel *dptr2;
	Pixel center = pixels[4];
	int32 diff1, diff2, diff3;
	int16 tmp_grey;
	Pixel tmp[4];
	Pixel *ptmp;

	switch (sub_type) {
	case 1:     /* '- */
		tmp[0] = tmp[1] = tmp[3] = center;

		tmp[2] = interpolate_1_1_1(pixels[3], pixels[7], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[2])];
#if PARANOID_KNIGHTS
		diff1 = labs(bptr[4] - bptr[3]);
		diff2 = labs(bptr[4] - bptr[7]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[2] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[3]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[7]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[2] = pixels[3];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[2] = pixels[7];
			else
				tmp[2] = pixels[4];

			tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[2])];
			diff1 = labs(bptr[4] - tmp_grey);
			diff2 = labs(bptr[4] - bptr[8]);
			if (diff1 <= diff2) {
				if (interpolate_2x) {
					Pixel tmp_pixel = tmp[2];
					tmp[2] = interpolate_3_1(tmp_pixel, center);
					tmp[3] = interpolate_3_1(center, tmp_pixel);
				}
			} else {
				if (interpolate_2x) {
					tmp[2] = interpolate_1_1(tmp[2], center);
				} else {
					if (bptr[4] > _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[2])])
						tmp[2] = center;
				}
			}
		}

		break;

	case 2:     /* -. */
		tmp[0] = tmp[2] = tmp[3] = center;

		tmp[1] = interpolate_1_1_1(pixels[1], pixels[5], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[1])];
#if PARANOID_KNIGHTS
		diff1 = labs(bptr[4] - bptr[5]);
		diff2 = labs(bptr[4] - bptr[1]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[1] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[1], pixels[1]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[1], pixels[5]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[1], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[1] = pixels[1];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[1] = pixels[5];
			else
				tmp[1] = pixels[4];

			tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[1])];
			diff1 = labs(bptr[4] - tmp_grey);
			diff2 = labs(bptr[4] - bptr[0]);
			if (diff1 <= diff2) {
				if (interpolate_2x) {
					Pixel tmp_pixel = tmp[1];
					tmp[1] = interpolate_3_1(tmp_pixel, center);
					tmp[0] = interpolate_3_1(center, tmp_pixel);
				}
			} else {
				if (interpolate_2x) {
					tmp[1] = interpolate_1_1(tmp[1], center);
				} else {
					if (bptr[4] > _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[1])])
						tmp[1] = center;
				}
			}
		}

		break;

	case 3:     /* \ */
	case 16:    /* \' */
	case 17:    /* .\ */
		tmp[0] = tmp[1] = tmp[2] = tmp[3] = center;

		if (sub_type != 16) {
			tmp[1] = interpolate_1_1_1(pixels[1], pixels[5], center);
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[1], pixels[1]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[1], pixels[5]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[1], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[1] = pixels[1];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[1] = pixels[5];
			else
				tmp[1] = pixels[4];

			if (interpolate_2x) {
				tmp[1] = interpolate_1_1(tmp[1], center);
			}
			/* sim test is for hyper-cephalic kitten eyes and squeeze toy
			 * mouse pointer in Sam&Max.  Half-diags can be too thin in 2x
			 * nearest-neighbor, so detect them and don't anti-alias them.
			 */
			else if (bptr[4] > _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[1])] ||
			         (_simSum == 1 && (sim[0] || sim[7]) &&
			          pixels[1] == pixels[3] && pixels[5] == pixels[7]))
				tmp[1] = center;
		}

		if (sub_type != 17) {
			tmp[2] = interpolate_1_1_1(pixels[3], pixels[7], center);
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[3]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[7]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[2] = pixels[3];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[2] = pixels[7];
			else
				tmp[2] = pixels[4];

			if (interpolate_2x) {
				tmp[2] = interpolate_1_1(tmp[2], center);
			}
			/* sim test is for hyper-cephalic kitten eyes and squeeze toy
			 * mouse pointer in Sam&Max.  Half-diags can be too thin in 2x
			 * nearest-neighbor, so detect them and don't anti-alias them.
			 */
			else if (bptr[4] > _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[2])] ||
			         (_simSum == 1 && (sim[0] || sim[7]) &&
			          pixels[1] == pixels[3] && pixels[5] == pixels[7]))
				tmp[2] = center;
		}

		break;

	case 4:     /* '| */
		tmp[0] = tmp[2] = tmp[3] = center;

		tmp[1] = interpolate_1_1_1(pixels[1], pixels[5], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[1])];
#if PARANOID_KNIGHTS
		diff1 = labs(bptr[4] - bptr[1]);
		diff2 = labs(bptr[4] - bptr[5]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[1] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[1], pixels[1]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[1], pixels[5]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[1], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[1] = pixels[1];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[1] = pixels[5];
			else
				tmp[1] = pixels[4];

			tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[1])];
			diff1 = labs(bptr[4] - tmp_grey);
			diff2 = labs(bptr[4] - bptr[8]);
			if (diff1 <= diff2) {
				if (interpolate_2x) {
					Pixel tmp_pixel = tmp[1];
					tmp[1] = interpolate_3_1(tmp_pixel, center);
					tmp[3] = interpolate_3_1(center, tmp_pixel);
				}
			} else {
				if (interpolate_2x) {
					tmp[1] = interpolate_1_1(tmp[1], center);
				} else {
					if (bptr[4] > _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[1])])
						tmp[1] = center;
				}
			}
		}

		break;

	case 5:     /* |. */
		tmp[0] = tmp[1] = tmp[3] = center;

		tmp[2] = interpolate_1_1_1(pixels[3], pixels[7], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[2])];
#if PARANOID_KNIGHTS
		diff1 = labs(bptr[4] - bptr[7]);
		diff2 = labs(bptr[4] - bptr[3]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[2] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[3]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[7]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[2] = pixels[3];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[2] = pixels[7];
			else
				tmp[2] = pixels[4];

			tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[2])];
			diff1 = labs(bptr[4] - tmp_grey);
			diff2 = labs(bptr[4] - bptr[0]);
			if (diff1 <= diff2) {
				if (interpolate_2x) {
					Pixel tmp_pixel = tmp[2];
					tmp[2] = interpolate_3_1(tmp_pixel, center);
					tmp[0] = interpolate_3_1(center, tmp_pixel);
				}
			} else {
				if (interpolate_2x) {
					tmp[2] = interpolate_1_1(tmp[2], center);
				} else {
					if (bptr[4] > _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[2])])
						tmp[2] = center;
				}
			}
		}

		break;

	case 7:     /* |' */
		tmp[1] = tmp[2] = tmp[3] = center;

		tmp[0] = interpolate_1_1_1(pixels[1], pixels[3], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[0])];
#if PARANOID_KNIGHTS
		diff1 = labs(bptr[4] - bptr[1]);
		diff2 = labs(bptr[4] - bptr[3]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[0] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[1]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[3]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[0] = pixels[1];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[0] = pixels[3];
			else
				tmp[0] = pixels[4];

			tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[0])];
			diff1 = labs(bptr[4] - tmp_grey);
			diff2 = labs(bptr[4] - bptr[6]);
			if (diff1 <= diff2) {
				if (interpolate_2x) {
					Pixel tmp_pixel = tmp[0];
					tmp[0] = interpolate_3_1(tmp_pixel, center);
					tmp[2] = interpolate_3_1(center, tmp_pixel);
				}
			} else {
				if (interpolate_2x) {
					tmp[0] = interpolate_1_1(tmp[0], center);
				} else {
					if (bptr[4] > _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[0])])
						tmp[0] = center;
				}
			}
		}

		break;

	case 8:     /* .| */
		tmp[0] = tmp[1] = tmp[2] = center;

		tmp[3] = interpolate_1_1_1(pixels[5], pixels[7], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[3])];
#if PARANOID_KNIGHTS
		diff1 = labs(bptr[4] - bptr[7]);
		diff2 = labs(bptr[4] - bptr[5]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[3] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[3], pixels[5]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[3], pixels[7]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[3], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[3] = pixels[5];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[3] = pixels[7];
			else
				tmp[3] = pixels[4];

			tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[3])];
			diff1 = labs(bptr[4] - tmp_grey);
			diff2 = labs(bptr[4] - bptr[2]);
			if (diff1 <= diff2) {
				if (interpolate_2x) {
					Pixel tmp_pixel = tmp[3];
					tmp[3] = interpolate_3_1(tmp_pixel, center);
					tmp[1] = interpolate_3_1(center, tmp_pixel);
				}
			} else {
				if (interpolate_2x) {
					tmp[3] = interpolate_1_1(tmp[3], center);
				} else {
					if (bptr[4] > _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[3])])
						tmp[3] = center;
				}
			}
		}

		break;

	case 9:     /* / */
	case 18:    /* '/ */
	case 19:    /* /. */
		tmp[0] = tmp[1] = tmp[2] = tmp[3] = center;

		if (sub_type != 18) {
			tmp[0] = interpolate_1_1_1(pixels[1], pixels[3], center);
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[1]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[3]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[0] = pixels[1];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[0] = pixels[3];
			else
				tmp[0] = pixels[4];

			if (interpolate_2x) {
				tmp[0] = interpolate_1_1(tmp[0], center);
			}
			/* sim test is for hyper-cephalic kitten eyes and squeeze toy
			 * mouse pointer in Sam&Max.  Half-diags can be too thin in 2x
			 * nearest-neighbor, so detect them and don't anti-alias them.
			 */
			else if (bptr[4] > _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[0])] ||
			         (_simSum == 1 && (sim[2] || sim[5]) &&
			          pixels[1] == pixels[5] && pixels[3] == pixels[7]))
				tmp[0] = center;
		}

		if (sub_type != 19) {
			tmp[3] = interpolate_1_1_1(pixels[5], pixels[7], center);
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[3], pixels[5]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[3], pixels[7]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[3], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[3] = pixels[5];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[3] = pixels[7];
			else
				tmp[3] = pixels[4];

			if (interpolate_2x) {
				tmp[3] = interpolate_1_1(tmp[3], center);
			}
			/* sim test is for hyper-cephalic kitten eyes and squeeze toy
			 * mouse pointer in Sam&Max.  Half-diags can be too thin in 2x
			 * nearest-neighbor, so detect them and don't anti-alias them.
			 */
			else if (bptr[4] > _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[3])] ||
			         (_simSum == 1 && (sim[2] || sim[5]) &&
			          pixels[1] == pixels[5] && pixels[3] == pixels[7]))
				tmp[3] = center;
		}

		break;

	case 10:    /* -' */
		tmp[0] = tmp[1] = tmp[2] = center;

		tmp[3] = interpolate_1_1_1(pixels[5], pixels[7], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[3])];
#if PARANOID_KNIGHTS
		diff1 = labs(bptr[4] - bptr[5]);
		diff2 = labs(bptr[4] - bptr[7]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[3] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[3], pixels[5]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[3], pixels[7]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[3], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[3] = pixels[5];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[3] = pixels[7];
			else
				tmp[3] = pixels[4];

			tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[3])];
			diff1 = labs(bptr[4] - tmp_grey);
			diff2 = labs(bptr[4] - bptr[6]);
			if (diff1 <= diff2) {
				if (interpolate_2x) {
					Pixel tmp_pixel = tmp[3];
					tmp[3] = interpolate_3_1(tmp_pixel, center);
					tmp[2] = interpolate_3_1(center, tmp_pixel);
				}
			} else {
				if (interpolate_2x) {
					tmp[3] = interpolate_1_1(tmp[3], center);
				} else {
					if (bptr[4] > _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[3])])
						tmp[3] = center;
				}
			}
		}

		break;

	case 11:    /* .- */
		tmp[1] = tmp[2] = tmp[3] = center;

		tmp[0] = interpolate_1_1_1(pixels[1], pixels[3], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[0])];
#if PARANOID_KNIGHTS
		diff1 = labs(bptr[4] - bptr[3]);
		diff2 = labs(bptr[4] - bptr[1]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[0] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[1]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[3]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[0] = pixels[1];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[0] = pixels[3];
			else
				tmp[0] = pixels[4];

			tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[0])];
			diff1 = labs(bptr[4] - tmp_grey);
			diff2 = labs(bptr[4] - bptr[2]);
			if (diff1 <= diff2) {
				if (interpolate_2x) {
					Pixel tmp_pixel = tmp[0];
					tmp[0] = interpolate_3_1(tmp_pixel, center);
					tmp[1] = interpolate_3_1(center, tmp_pixel);
				}
			} else {
				if (interpolate_2x) {
					tmp[0] = interpolate_1_1(tmp[0], center);
				} else {
					if (bptr[4] > _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[0])])
						tmp[0] = center;
				}
			}
		}

		break;

	case 12:    /* < */
		tmp[0] = tmp[1] = tmp[2] = tmp[3] = center;

		tmp[0] = interpolate_1_1_1(pixels[1], pixels[3], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[0])];
#if PARANOID_ARROWS
		diff1 = labs(bptr[4] - bptr[1]);
		diff2 = labs(bptr[4] - bptr[3]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[0] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[1]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[3]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[0] = pixels[1];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[0] = pixels[3];
			else
				tmp[0] = pixels[4];

			/* check for half-arrow */
			if (_simSum == 2 && sim[4] && sim[2]) {
				if (interpolate_2x) {
					tmp[0] = interpolate_1_1(center, tmp[0]);
					tmp[2] = interpolate_2_1(center, tmp[0]);
				} else {
					if (bptr[4] > _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[0])])
						tmp[0] = center;
				}

				break;
			}

			if (interpolate_2x)
				tmp[0] = interpolate_2_1(center, tmp[0]);
			else
				tmp[0] = center;
		}

		tmp[2] = interpolate_1_1_1(pixels[3], pixels[7], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[2])];
#if PARANOID_ARROWS
		diff1 = labs(bptr[4] - bptr[3]);
		diff2 = labs(bptr[4] - bptr[7]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[2] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[3]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[7]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[2] = pixels[3];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[2] = pixels[7];
			else
				tmp[2] = pixels[4];

			/* check for half-arrow */
			if (_simSum == 2 && sim[4] && sim[7]) {
				if (interpolate_2x) {
					tmp[2] = interpolate_1_1(center, tmp[2]);
					tmp[0] = interpolate_2_1(center, tmp[2]);
				} else {
					if (bptr[4] > _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[2])])
						tmp[2] = center;
				}

				break;
			}

			if (interpolate_2x)
				tmp[2] = interpolate_2_1(center, tmp[2]);
			else
				tmp[2] = center;
		}

		break;

	case 13:    /* > */
		tmp[0] = tmp[1] = tmp[2] = tmp[3] = center;

		tmp[1] = interpolate_1_1_1(pixels[1], pixels[5], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[1])];
#if PARANOID_ARROWS
		diff1 = labs(bptr[4] - bptr[5]);
		diff2 = labs(bptr[4] - bptr[1]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[1] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[1], pixels[1]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[1], pixels[5]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[1], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[1] = pixels[1];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[1] = pixels[5];
			else
				tmp[1] = pixels[4];

			/* check for half-arrow */
			if (_simSum == 2 && sim[3] && sim[0]) {
				if (interpolate_2x) {
					tmp[1] = interpolate_1_1(center, tmp[1]);
					tmp[3] = interpolate_2_1(center, tmp[1]);
				} else {
					if (bptr[4] > _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[1])])
						tmp[1] = center;
				}

				break;
			}

			if (interpolate_2x)
				tmp[1] = interpolate_2_1(center, tmp[1]);
			else
				tmp[1] = center;
		}

		tmp[3] = interpolate_1_1_1(pixels[5], pixels[7], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[3])];
#if PARANOID_ARROWS
		diff1 = labs(bptr[4] - bptr[7]);
		diff2 = labs(bptr[4] - bptr[5]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[3] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[3], pixels[5]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[3], pixels[7]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[3], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[3] = pixels[5];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[3] = pixels[7];
			else
				tmp[3] = pixels[4];

			/* check for half-arrow */
			if (_simSum == 2 && sim[3] && sim[5]) {
				if (interpolate_2x) {
					tmp[3] = interpolate_1_1(center, tmp[3]);
					tmp[1] = interpolate_2_1(center, tmp[3]);
				} else {
					if (bptr[4] > _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[3])])
						tmp[3] = center;
				}

				break;
			}

			if (interpolate_2x)
				tmp[3] = interpolate_2_1(center, tmp[3]);
			else
				tmp[3] = center;
		}

		break;

	case 14:    /* ^ */
		tmp[0] = tmp[1] = tmp[2] = tmp[3] = center;

		tmp[0] = interpolate_1_1_1(pixels[1], pixels[3], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[0])];
#if PARANOID_ARROWS
		diff1 = labs(bptr[4] - bptr[1]);
		diff2 = labs(bptr[4] - bptr[3]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[0] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[1]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[3]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[0], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[0] = pixels[1];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[0] = pixels[3];
			else
				tmp[0] = pixels[4];

			/* check for half-arrow */
			if (_simSum == 2 && sim[6] && sim[5]) {
				if (interpolate_2x) {
					tmp[0] = interpolate_1_1(center, tmp[0]);
					tmp[1] = interpolate_2_1(center, tmp[0]);
				} else {
					if (bptr[4] > _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[0])])
						tmp[0] = center;
				}

				break;
			}

			if (interpolate_2x)
				tmp[0] = interpolate_2_1(center, tmp[0]);
			else
				tmp[0] = center;
		}

		tmp[1] = interpolate_1_1_1(pixels[1], pixels[5], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[1])];
#if PARANOID_ARROWS
		diff1 = labs(bptr[4] - bptr[5]);
		diff2 = labs(bptr[4] - bptr[1]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[1] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[1], pixels[1]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[1], pixels[5]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[1], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[1] = pixels[1];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[1] = pixels[5];
			else
				tmp[1] = pixels[4];

			/* check for half-arrow */
			if (_simSum == 2 && sim[6] && sim[7]) {
				if (interpolate_2x) {
					tmp[1] = interpolate_1_1(center, tmp[1]);
					tmp[0] = interpolate_2_1(center, tmp[1]);
				} else {
					if (bptr[4] > _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[1])])
						tmp[1] = center;
				}

				break;
			}

			if (interpolate_2x)
				tmp[1] = interpolate_2_1(center, tmp[1]);
			else
				tmp[1] = center;
		}

		break;

	case 15:    /* v */
		tmp[0] = tmp[1] = tmp[2] = tmp[3] = center;

		tmp[2] = interpolate_1_1_1(pixels[3], pixels[7], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[2])];
#if PARANOID_ARROWS
		diff1 = labs(bptr[4] - bptr[3]);
		diff2 = labs(bptr[4] - bptr[7]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[2] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[3]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[7]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[2], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[2] = pixels[3];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[2] = pixels[7];
			else
				tmp[2] = pixels[4];

			/* check for half-arrow */
			if (_simSum == 2 && sim[1] && sim[0]) {
				if (interpolate_2x) {
					tmp[2] = interpolate_1_1(center, tmp[2]);
					tmp[3] = interpolate_2_1(center, tmp[2]);
				} else {
					if (bptr[4] > _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[2])])
						tmp[2] = center;
				}

				break;
			}

			if (interpolate_2x)
				tmp[2] = interpolate_2_1(center, tmp[2]);
			else
				tmp[2] = center;
		}

		tmp[3] = interpolate_1_1_1(pixels[5], pixels[7], center);
		tmp_grey = _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[3])];
#if PARANOID_ARROWS
		diff1 = labs(bptr[4] - bptr[7]);
		diff2 = labs(bptr[4] - bptr[5]);
		diff3 = labs(bptr[4] - tmp_grey);
		if (diff1 < diff3 || diff2 < diff3)
			tmp[3] = center;
		else    /* choose nearest pixel */
#endif
		{
			diff1 = calcPixelDiffNosqrt<ColorMask>(tmp[3], pixels[5]);
			diff2 = calcPixelDiffNosqrt<ColorMask>(tmp[3], pixels[7]);
			diff3 = calcPixelDiffNosqrt<ColorMask>(tmp[3], pixels[4]);
			if (diff1 <= diff2 && diff1 <= diff3)
				tmp[3] = pixels[5];
			else if (diff2 <= diff1 && diff2 <= diff3)
				tmp[3] = pixels[7];
			else
				tmp[3] = pixels[4];

			/* check for half-arrow */
			if (_simSum == 2 && sim[1] && sim[2]) {
				if (interpolate_2x) {
					tmp[3] = interpolate_1_1(center, tmp[3]);
					tmp[2] = interpolate_2_1(center, tmp[3]);
				} else {
					if (bptr[4] > _chosenGreyscale[convertTo16Bit<ColorMask>(tmp[3])])
						tmp[3] = center;
				}

				break;
			}

			if (interpolate_2x)
				tmp[3] = interpolate_2_1(center, tmp[3]);
			else
				tmp[3] = center;
		}

		break;

	case -1:    /* no edge */
	case 0:     /* - */
	case 6:     /* | */
	case 127:   /* * */
	default:    /* no edge */
		dptr2 = (Pixel *) dptr;
		*dptr2++ = center;
#if DEBUG_REFRESH_RANDOM_XOR
		*dptr2 = center ^ (Pixel)(dxorshift128() * (1L << 16));
#else
		*dptr2 = center;
#endif
		dptr2 = (Pixel *)(dptr + dstPitch);
		*dptr2++ = center;
		*dptr2 = center;

		return;

		break;
	}

	ptmp = tmp;
	dptr2 = (Pixel *) dptr;
	*dptr2++ = *ptmp++;
#if DEBUG_REFRESH_RANDOM_XOR
	*dptr2 = *ptmp++ ^ (Pixel)(dxorshift128() * (1L << 16));
#else
	*dptr2 = *ptmp++;
#endif
	dptr2 = (Pixel *)(dptr + dstPitch);
	*dptr2++ = *ptmp++;
	*dptr2 = *ptmp;
}


/* Check for changed pixel grid, return 1 if unchanged. */
template<typename Pixel>
int checkUnchangedPixels(const Pixel *old_src_ptr, const Pixel *pixels, int w) {
	const Pixel *dptr;

	dptr = old_src_ptr - w - 1;
	if (*dptr++ != pixels[0]) return 0;
	if (*dptr++ != pixels[1]) return 0;
	if (*dptr != pixels[2]) return 0;

	dptr += w - 2;
	if (*dptr != pixels[3]) return 0;
	dptr += 2;
	if (*dptr != pixels[5]) return 0;

	dptr += w - 2;
	if (*dptr++ != pixels[6]) return 0;
	if (*dptr++ != pixels[7]) return 0;
	if (*dptr != pixels[8]) return 0;

	return 1;
}


/* Draw unchanged pixel grid, 3x */
/* old_dptr starts in top left of grid, dptr in center */
template<typename Pixel>
void drawUnchangedGrid3x(byte *dptr, int dstPitch,
						 const byte *old_dptr, int old_dst_inc) {
	const Pixel *sp;
	Pixel *dp;

	sp = (const Pixel *)old_dptr;
	dp = (Pixel *)(dptr - dstPitch) - 1;
	*dp++ = *sp++;
	*dp++ = *sp++;
	*dp = *sp;

	sp = (const Pixel *)(old_dptr + old_dst_inc);
	dp = (Pixel *)dptr - 1;
	*dp++ = *sp++;
	*dp++ = *sp++;
	*dp = *sp;

	sp = (const Pixel *)(old_dptr + old_dst_inc + old_dst_inc);
	dp = (Pixel *)(dptr + dstPitch) - 1;
	*dp++ = *sp++;
	*dp++ = *sp++;
	*dp = *sp;
}



/* Draw unchanged pixel grid, 2x */
template<typename Pixel>
void drawUnchangedGrid2x(byte *dptr, int dstPitch,
						 const byte *old_dptr, int old_dst_inc) {
	const Pixel *sp;
	Pixel *dp;

	sp = (const Pixel *)old_dptr;
	dp = (Pixel *)dptr;
	*dp++ = *sp++;
	*dp = *sp;

	sp = (const Pixel *)(old_dptr + old_dst_inc);
	dp = (Pixel *)(dptr + dstPitch);
	*dp++ = *sp++;
	*dp = *sp;
}


template<typename ColorMask>
void EdgeScaler::antiAliasPass3x(const uint8 *src, uint8 *dst,
								 int w, int h,
								 int srcPitch, int dstPitch,
								 bool haveOldSrc,
								 const uint8* oldSrc, int oldPitch,
								 const uint8 *buffer, int bufferPitch) {
	typedef typename ColorMask::PixelType Pixel;

	int x, y;
	const uint8 *sptr8 = src;
	uint8 *dptr8 = dst + dstPitch + sizeof(Pixel);
	const Pixel *sptr16;
	const Pixel *oldSptr;
	const Pixel *oldDptr;
	Pixel *dptr16;
	int16 *bplane;
	int8 sim[8];
	int sub_type;
	int32 angle;
	int16 *diffs;
	int dstPitch3 = dstPitch * 3;
	int bufferPitch3 = bufferPitch * 3;

	for (y = 0; y < h; y++, sptr8 += srcPitch, dptr8 += dstPitch3, oldSrc += oldPitch, buffer += bufferPitch3) {
		for (x = 0,
		        sptr16 = (const Pixel *) sptr8,
		        oldSptr = (const Pixel *) oldSrc,
		        oldDptr = (const Pixel *) buffer,
		        dptr16 = (Pixel *) dptr8;
		        x < w; x++, sptr16++, dptr16 += 3, oldDptr += 3, oldSptr++) {
			const Pixel *sptr2, *addr3;
			Pixel pixels[9];
			char edge_type;

			sptr2 = ((const Pixel *)((const uint8 *) sptr16 - srcPitch)) - 1;
			addr3 = ((const Pixel *)((const uint8 *) sptr16 + srcPitch)) + 1;

			/* fill the 3x3 grid */
			memcpy(pixels, sptr2, 3 * sizeof(Pixel));
			memcpy(pixels + 3, sptr16 - 1, 3 * sizeof(Pixel));
			memcpy(pixels + 6, addr3 - 2, 3 * sizeof(Pixel));

			if (haveOldSrc) {
				/* skip interior unchanged 3x3 blocks */
				if (*sptr16 == *oldSptr &&
#if DEBUG_DRAW_REFRESH_BORDERS
						x > 0 && x < w - 1 && y > 0 && y < h - 1 &&
#endif
						checkUnchangedPixels(oldSptr, pixels, oldPitch / sizeof(Pixel))) {
					drawUnchangedGrid3x<Pixel>((byte *)dptr16, dstPitch, (const byte *)oldDptr, bufferPitch);

#if DEBUG_REFRESH_RANDOM_XOR
					*(dptr16 + 1) = 0;
#endif
					continue;
				}
			}

			diffs = chooseGreyscale<ColorMask>(pixels);

			/* block of solid color */
			if (!diffs) {
				antiAliasGridClean3x<ColorMask>((uint8 *) dptr16, dstPitch, pixels,
				                                    0, NULL);
				continue;
			}

			bplane = _bptr;

			edge_type = findPrincipleAxis(diffs, bplane,
			                              sim, &angle);
			sub_type = refineDirection<Pixel>(edge_type, pixels, bplane,
			                           sim, angle);
			if (sub_type >= 0)
				sub_type = fixKnights<Pixel>(sub_type, pixels, sim);

			antiAliasGridClean3x<ColorMask>((uint8 *) dptr16, dstPitch, pixels,
			                                    sub_type, bplane);
		}
	}
}


template<typename ColorMask>
void EdgeScaler::antiAliasPass2x(const uint8 *src, uint8 *dst,
								 int w, int h,
								 int srcPitch, int dstPitch,
								 int interpolate_2x,
								 bool haveOldSrc,
								 const uint8 *oldSrc, int oldSrcPitch,
								 const uint8 *buffer, int bufferPitch) {
	typedef typename ColorMask::PixelType Pixel;

	int x, y;
	const uint8 *sptr8 = src;
	uint8 *dptr8 = dst;
	const Pixel *sptr16;
	const Pixel *oldSptr;
	const Pixel *oldDptr;
	Pixel *dptr16;
	int16 *bplane;
	int8 sim[8];
	int sub_type;
	int32 angle;
	int16 *diffs;
	int dstPitch2 = dstPitch << 1;
	int bufferPitch2 = bufferPitch * 2;

	for (y = 0; y < h; y++, sptr8 += srcPitch, dptr8 += dstPitch2, oldSrc += oldSrcPitch, buffer += bufferPitch2) {
		for (x = 0,
		        sptr16 = (const Pixel *) sptr8,
		        dptr16 = (Pixel *) dptr8,
				oldSptr = (const Pixel *) oldSrc,
				oldDptr = (const Pixel *) buffer;
		        x < w; x++, sptr16++, dptr16 += 2, oldDptr += 2, oldSptr++) {
			const Pixel *sptr2, *addr3;
			Pixel pixels[9];
			char edge_type;

			sptr2 = ((const Pixel *)((const uint8 *) sptr16 - srcPitch)) - 1;
			addr3 = ((const Pixel *)((const uint8 *) sptr16 + srcPitch)) + 1;

			/* fill the 3x3 grid */
			memcpy(pixels, sptr2, 3 * sizeof(Pixel));
			memcpy(pixels + 3, sptr16 - 1, 3 * sizeof(Pixel));
			memcpy(pixels + 6, addr3 - 2, 3 * sizeof(Pixel));

			if (haveOldSrc) {
				/* skip interior unchanged 3x3 blocks */
				if (*sptr16 == *oldSptr &&
#if DEBUG_DRAW_REFRESH_BORDERS
						x > 0 && x < w - 1 && y > 0 && y < h - 1 &&
#endif
						checkUnchangedPixels<Pixel>(oldSptr, pixels, oldSrcPitch / sizeof(Pixel))) {
					drawUnchangedGrid2x<Pixel>((byte *)dptr16, dstPitch, (const byte *)oldDptr, bufferPitch);

#if DEBUG_REFRESH_RANDOM_XOR
					*(dptr16 + 1) = 0;
#endif
					continue;
				}
			}

			diffs = chooseGreyscale<ColorMask>(pixels);

			/* block of solid color */
			if (!diffs) {
				antiAliasGrid2x<ColorMask>((uint8 *) dptr16, dstPitch, pixels,
				                              0, NULL, NULL, 0);
				continue;
			}

			bplane = _bptr;

			edge_type = findPrincipleAxis(diffs, bplane,
			                              sim, &angle);
			sub_type = refineDirection<Pixel>(edge_type, pixels, bplane,
			                           sim, angle);
			if (sub_type >= 0)
				sub_type = fixKnights<Pixel>(sub_type, pixels, sim);

			antiAliasGrid2x<ColorMask>((uint8 *) dptr16, dstPitch, pixels,
			                              sub_type, bplane, sim,
			                              interpolate_2x);
		}
	}
}


void EdgeScaler::initTables(const uint8 *srcPtr, uint32 srcPitch,
							int width, int height) {
	double r_float, g_float, b_float;
	int r, g, b;
	uint16 i;
	double val[3];
	double intensity;
	int16 *rgb_ptr;

#if DEBUG_REFRESH_RANDOM_XOR
	/* seed the random number generator, we don't care if the seed is random */
	initializeXorshift128(42);
#endif

	/* initialize greyscale table */
	for (r = 0; r < 32; r++) {
		r_float = r / 31.0;

		for (g = 0; g < 64; g++) {
			g_float = g / 63.0;

			for (b = 0; b < 32; b++) {
				b_float = b / 31.0;

				intensity = (r_float + g_float + b_float) / 3;

				i = (r << 11) | (g << 5) | b;

				/* use luma-like weights for each color, 2x increments */
				val[0] = 0.571 * r_float + 0.286 * g_float + 0.143 * b_float;
				val[1] = 0.286 * r_float + 0.571 * g_float + 0.143 * b_float;
				val[2] = 0.143 * r_float + 0.286 * g_float + 0.571 * b_float;

				/* factor in a little intensity too, it helps */
				val[0] = (intensity + 9 * val[0]) / 10;
				val[1] = (intensity + 9 * val[1]) / 10;
				val[2] = (intensity + 9 * val[2]) / 10;

				/* store the greyscale tables */
				_greyscaleTable[0][i] = (int16)(val[0] * ((int16)1 << GREY_SHIFT) + 0.5);
				_greyscaleTable[1][i] = (int16)(val[1] * ((int16)1 << GREY_SHIFT) + 0.5);
				_greyscaleTable[2][i] = (int16)(val[2] * ((int16)1 << GREY_SHIFT) + 0.5);

				/* normalized RGB channel lookups */
				rgb_ptr = _rgbTable[(r << 11) | (g << 5) | b];
				rgb_ptr[0] = (int16)(r_float * ((int16)1 << RGB_SHIFT) + 0.5);
				rgb_ptr[1] = (int16)(g_float * ((int16)1 << RGB_SHIFT) + 0.5);
				rgb_ptr[2] = (int16)(b_float * ((int16)1 << RGB_SHIFT) + 0.5);
			}
		}
	}
}

EdgeScaler::EdgeScaler(const Graphics::PixelFormat &format) : SourceScaler(format) {
	_factor = 2;

	initTables(0, 0, 0, 0);
}

#if 0
void EdgeScaler::scale(const uint8 *srcPtr, uint32 srcPitch,
					   uint8 *dstPtr, uint32 dstPitch, int width, int height, int x, int y) {
	if (_format.bytesPerPixel == 2) {
		if (_factor == 2) {
			if (_format.gLoss == 2)
				antiAliasPass2x<Graphics::ColorMasks<565>, uint16>(srcPtr, dstPtr, width, height, 2 * width, 2 * height, srcPitch, dstPitch, 1, false, NULL, 0);
			else
				antiAliasPass2x<Graphics::ColorMasks<555>, uint16>(srcPtr, dstPtr, width, height, 2 * width, 2 * height, srcPitch, dstPitch, 1, false, NULL, 0);
		} else {
			if (_format.gLoss == 2)
				antiAliasPass3x<Graphics::ColorMasks<565>, uint16>(srcPtr, dstPtr, width, height, 3 * width, 3 * height, srcPitch, dstPitch, false, NULL, 0);
			else
				antiAliasPass3x<Graphics::ColorMasks<555>, uint16>(srcPtr, dstPtr, width, height, 3 * width, 3 * height, srcPitch, dstPitch, false, NULL, 0);
		}
	} else {
		if (_factor == 2) {
			if (_format.aLoss == 0)
				antiAliasPass2x<Graphics::ColorMasks<8888>, uint32>(srcPtr, dstPtr, width, height, 2 * width, 2 * height, srcPitch, dstPitch, 1, false, NULL, 0);
			else
				antiAliasPass2x<Graphics::ColorMasks<888>, uint32>(srcPtr, dstPtr, width, height, 2 * width, 2 * height, srcPitch, dstPitch, 1, false, NULL, 0);
		} else {
			if (_format.aLoss == 0)
				antiAliasPass3x<Graphics::ColorMasks<8888>, uint32>(srcPtr, dstPtr, width, height, 3 * width, 3 * height, srcPitch, dstPitch, false, NULL, 0);
			else
				antiAliasPass3x<Graphics::ColorMasks<888>, uint32>(srcPtr, dstPtr, width, height, 3 * width, 3 * height, srcPitch, dstPitch, false, NULL, 0);
		}
	}
}
#endif

void EdgeScaler::internScale(const uint8 *srcPtr, uint32 srcPitch,
					   uint8 *dstPtr, uint32 dstPitch, const uint8 *oldSrcPtr, uint32 oldSrcPitch, int width, int height, const uint8 *buffer, uint32 bufferPitch) {
	bool enable = oldSrcPtr != NULL;
	if (_format.bytesPerPixel == 2) {
		if (_factor == 2) {
			if (_format.gLoss == 2)
				antiAliasPass2x<Graphics::ColorMasks<565> >(srcPtr, dstPtr, width, height, srcPitch, dstPitch, 1, enable, oldSrcPtr, oldSrcPitch, buffer, bufferPitch);
			else
				antiAliasPass2x<Graphics::ColorMasks<555> >(srcPtr, dstPtr, width, height, srcPitch, dstPitch, 1, enable, oldSrcPtr, oldSrcPitch, buffer, bufferPitch);
		} else {
			if (_format.gLoss == 2)
				antiAliasPass3x<Graphics::ColorMasks<565> >(srcPtr, dstPtr, width, height, srcPitch, dstPitch, enable, oldSrcPtr, oldSrcPitch, buffer, bufferPitch);
			else
				antiAliasPass3x<Graphics::ColorMasks<555> >(srcPtr, dstPtr, width, height, srcPitch, dstPitch, enable, oldSrcPtr, oldSrcPitch, buffer, bufferPitch);
		}
	} else {
		if (_factor == 2) {
			if (_format.aLoss == 0)
				antiAliasPass2x<Graphics::ColorMasks<8888> >(srcPtr, dstPtr, width, height, srcPitch, dstPitch, 1, enable, oldSrcPtr, oldSrcPitch, buffer, bufferPitch);
			else
				antiAliasPass2x<Graphics::ColorMasks<888> >(srcPtr, dstPtr, width, height, srcPitch, dstPitch, 1, enable, oldSrcPtr, oldSrcPitch, buffer, bufferPitch);
		} else {
			if (_format.aLoss == 0)
				antiAliasPass3x<Graphics::ColorMasks<8888> >(srcPtr, dstPtr, width, height, srcPitch, dstPitch, enable, oldSrcPtr, oldSrcPitch, buffer, bufferPitch);
			else
				antiAliasPass3x<Graphics::ColorMasks<888> >(srcPtr, dstPtr, width, height, srcPitch, dstPitch, enable, oldSrcPtr, oldSrcPitch, buffer, bufferPitch);
		}
	}
}

uint EdgeScaler::increaseFactor() {
	if (_factor == 2)
		setFactor(_factor + 1);
	return _factor;
}

uint EdgeScaler::decreaseFactor() {
	if (_factor == 3)
		setFactor(_factor - 1);
	return _factor;
}


class EdgePlugin final : public ScalerPluginObject {
public:
	EdgePlugin();

	Scaler *createInstance(const Graphics::PixelFormat &format) const override;

	bool canDrawCursor() const override { return false; }
	bool useOldSource() const override { return true; }
	uint extraPixels() const override { return 1; }
	const char *getName() const override;
	const char *getPrettyName() const override;
};

EdgePlugin::EdgePlugin() {
	_factors.push_back(2);
	_factors.push_back(3);
}

Scaler *EdgePlugin::createInstance(const Graphics::PixelFormat &format) const {
	return new EdgeScaler(format);
}

const char *EdgePlugin::getName() const {
	return "edge";
}

const char *EdgePlugin::getPrettyName() const {
	return "Edge";
}

REGISTER_PLUGIN_STATIC(EDGE, PLUGIN_TYPE_SCALER, EdgePlugin);
