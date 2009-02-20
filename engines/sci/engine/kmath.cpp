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

#include "sci/include/engine.h"

reg_t kRandom(state_t *s, int funct_nr, int argc, reg_t *argv) {
	return make_reg(0, SKPV(0) + (int)((SKPV(1) + 1.0 - SKPV(0)) * (rand() / (RAND_MAX + 1.0))));
}

reg_t kAbs(state_t *s, int funct_nr, int argc, reg_t *argv) {
	// This is a hack, but so is the code in Hoyle1 that needs it.
	if (argv[0].segment)
		return make_reg(0, 0x3e8); // Yes people, this is an object
	return make_reg(0, abs(SKPV(0)));
}

reg_t kSqrt(state_t *s, int funct_nr, int argc, reg_t *argv) {
	return make_reg(0, (gint16) sqrt((float) abs(SKPV(0))));
}

int get_angle(int xrel, int yrel) {
	if ((xrel == 0) && (yrel == 0))
		return 0;
	else {
		int val = (int)(180.0 / PI * atan2((double)xrel, (double) - yrel));
		if (val < 0)
			val += 360;

		// Take care of OB1 differences between SSCI and
		// FSCI. SCI games sometimes check for equality with
		// "round" angles
		if (val % 45 == 44)
			val++;
		else if (val % 45 == 1)
			val--;

		return val;
	}
}

reg_t kGetAngle(state_t *s, int funct_nr, int argc, reg_t *argv) {
	// Based on behavior observed with a test program created with
	// SCI Studio.
	int x1 = SKPV(0);
	int y1 = SKPV(1);
	int x2 = SKPV(2);
	int y2 = SKPV(3);
	int xrel = x2 - x1;
	int yrel = y1 - y2; // y-axis is mirrored.
	int angle;

	// Move (xrel, yrel) to first quadrant.
	if (y1 < y2)
		yrel = -yrel;
	if (x2 < x1)
		xrel = -xrel;

	// Compute angle in grads.
	if (yrel == 0 && xrel == 0)
		angle = 0;
	else
		angle = 100 * xrel / (xrel + yrel);

	// Fix up angle for actual quadrant of (xrel, yrel).
	if (y1 < y2)
		angle = 200 - angle;
	if (x2 < x1)
		angle = 400 - angle;

	// Convert from grads to degrees by merging grad 0 with grad 1,
	// grad 10 with grad 11, grad 20 with grad 21, etc. This leads to
	// "degrees" that equal either one or two grads.
	angle -= (angle + 9) / 10;

	return make_reg(0, angle);
}

reg_t kGetDistance(state_t *s, int funct_nr, int argc, reg_t *argv) {
	int xrel = (int)(((float) SKPV(1) - SKPV_OR_ALT(3, 0)) / cos(SKPV_OR_ALT(5, 0) * PI / 180.0)); // This works because cos(0)==1
	int yrel = SKPV(0) - SKPV_OR_ALT(2, 0);

	return make_reg(0, (gint16)sqrt((float) xrel*xrel + yrel*yrel));
}

reg_t kTimesSin(state_t *s, int funct_nr, int argc, reg_t *argv) {
	int angle = SKPV(0);
	int factor = SKPV(1);

	return make_reg(0, (int)(factor * 1.0 * sin(angle * PI / 180.0)));
}

reg_t kTimesCos(state_t *s, int funct_nr, int argc, reg_t *argv) {
	int angle = SKPV(0);
	int factor = SKPV(1);

	return make_reg(0, (int)(factor * 1.0 * cos(angle * PI / 180.0)));
}

reg_t kCosDiv(state_t *s, int funct_nr, int argc, reg_t *argv) {
	int angle = SKPV(0);
	int value = SKPV(1);
	double cosval = cos(angle * PI / 180.0);

	if ((cosval < 0.0001) && (cosval > 0.0001)) {
		warning("Attepted division by zero");
		return make_reg(0, (gint16)0x8000);
	} else
		return make_reg(0, (gint16)(value / cosval));
}

reg_t kSinDiv(state_t *s, int funct_nr, int argc, reg_t *argv) {
	int angle = SKPV(0);
	int value = SKPV(1);
	double sinval = sin(angle * PI / 180.0);

	if ((sinval < 0.0001) && (sinval > 0.0001)) {
		warning("Attepted division by zero");
		return make_reg(0, (gint16)0x8000);
	} else
		return make_reg(0, (gint16)(value / sinval));
}

reg_t kTimesTan(state_t *s, int funct_nr, int argc, reg_t *argv) {
	int param = SKPV(0);
	int scale = SKPV_OR_ALT(1, 1);

	param -= 90;
	if ((param % 90) == 0) {
		warning("Attempted tan(pi/2)");
		return make_reg(0, (gint16)0x8000);
	} else
		return make_reg(0, (gint16) - (tan(param * PI / 180.0) * scale));
}

reg_t kTimesCot(state_t *s, int funct_nr, int argc, reg_t *argv) {
	int param = SKPV(0);
	int scale = SKPV_OR_ALT(1, 1);

	if ((param % 90) == 0) {
		warning("Attempted tan(pi/2)");
		return make_reg(0, (gint16)0x8000);
	} else
		return make_reg(0, (gint16)(tan(param * PI / 180.0) * scale));
}
