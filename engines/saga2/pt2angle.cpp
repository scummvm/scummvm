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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"

namespace Saga2 {

static int16 arcTanTable[257] = {
	0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5,
	5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7,
	8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 10, 10, 10,
	10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12,
	12, 12, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14,
	15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 17, 17,
	17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 19, 19, 19,
	19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 20, 20, 21, 21, 21,
	21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 22, 23, 23,
	23, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24,
	25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26,
	26, 26, 26, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 28, 28, 28,
	28, 28, 28, 28, 28, 28, 28, 28, 29, 29, 29, 29, 29, 29, 29, 29,
	29, 29, 29, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 31, 31,
	31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32,
	32
};

static int16 invCosTable[257] = {
	128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
	128, 128, 128, 128, 128, 128, 128, 129, 129, 129, 129, 129, 129, 129, 129, 129,
	129, 129, 129, 129, 129, 129, 129, 129, 130, 130, 130, 130, 130, 130, 130, 130,
	130, 130, 130, 131, 131, 131, 131, 131, 131, 131, 131, 131, 131, 132, 132, 132,
	132, 132, 132, 132, 132, 133, 133, 133, 133, 133, 133, 133, 134, 134, 134, 134,
	134, 134, 134, 135, 135, 135, 135, 135, 135, 136, 136, 136, 136, 136, 136, 137,
	137, 137, 137, 137, 137, 138, 138, 138, 138, 138, 139, 139, 139, 139, 139, 140,
	140, 140, 140, 140, 141, 141, 141, 141, 141, 142, 142, 142, 142, 142, 143, 143,
	143, 143, 144, 144, 144, 144, 144, 145, 145, 145, 145, 146, 146, 146, 146, 147,
	147, 147, 147, 148, 148, 148, 148, 149, 149, 149, 149, 150, 150, 150, 150, 151,
	151, 151, 151, 152, 152, 152, 153, 153, 153, 153, 154, 154, 154, 154, 155, 155,
	155, 156, 156, 156, 156, 157, 157, 157, 158, 158, 158, 159, 159, 159, 159, 160,
	160, 160, 161, 161, 161, 162, 162, 162, 162, 163, 163, 163, 164, 164, 164, 165,
	165, 165, 166, 166, 166, 167, 167, 167, 167, 168, 168, 168, 169, 169, 169, 170,
	170, 170, 171, 171, 171, 172, 172, 172, 173, 173, 173, 174, 174, 174, 175, 175,
	175, 176, 176, 176, 177, 177, 178, 178, 178, 179, 179, 179, 180, 180, 180, 181,
	181
};

/****** cmisc/ptToAngle *******************************
*
*   NAME
*       ptToAngle -- converts an (x,y) coordinate to angle and distance
*
*   SYNOPSIS
*       angle = ptToAngle( x, y, &dist );
*       angle = ptToAngle( x, y, NULL );
*
*       int16 ptToAngle( int16, int16, int16 * );
*
*   FUNCTION
*       This function quickly converts a pair of cartesian coordinates
*       (x,y) into polar coordinates (angle,distance) using table
*       lookups. Note that the distance calculation is optional.
*
*       All angular units are in 256ths of a full circle.
*
*   /H2/ACCURACY
*       Accuracy is better that 1%. Note, however, that in practical
*       applications, most of the inaccuracy will derive from the
*       quantization of the input values themselves. For example,
*       assume that a point is at a distance of 100 and and angle
*       of 6. The nearest integer point is 98,14, which is about
*       2% nearer the origin than the "real" point. Running these
*       numbers back through ptToAngle resuls in angle=6, dist=98.
*
*       (Accuracy has been verified empirically)
*
*   /H2/PERFORMANCE
*       For the angle calculation, a single divide is used, plus a
*       number of comparisons, assignments, and a table lookup.
*
*       If the dist parameter is non-NULL, then 1 multiplication,
*       1 shift, and 1 table lookup are also required.
*
*   INPUTS
*       x,y         The cartesion coordinates to be converted.
*
*       dist        [Optional Argument]. A pointer to a int16 value
*                   which will be filled in with the distance to
*                   the input point.
*
*   RESULTS
*       angle       The angle in 256ths of a circle.
*
*******************************************************************/

int16 ptToAngle(int16 dx, int16 dy, int16 *dist) {
	int16           angle = 0;

	if (dy < 0) {
		angle += 128;
		dy = -dy;
		dx = -dx;
	}

	if (dx < 0) {
		int16       t = dy;

		angle += 64;
		dy = -dx;
		dx = t;
	}

	if (dx == dy) {
		if (dist) *dist = (invCosTable[256] * dx) >> 7;
		return angle + 32;  // avoids division by zero...
	} else if (dx >= dy) {
		int16 i = (dy << 8) / dx;

		if (dist) *dist = (invCosTable[i] * dx) >> 7;
		return angle + arcTanTable[i];
	} else {
		int16 i = (dx << 8) / dy;

		if (dist) *dist = (invCosTable[i] * dy) >> 7;
		return angle + 64 - arcTanTable[i];
	}
}

} // end of namespace Saga2
