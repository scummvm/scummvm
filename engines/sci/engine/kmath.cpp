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

#include "sci/engine/state.h"
#include "sci/engine/kernel.h"

namespace Sci {

reg_t kRandom(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	return make_reg(0, argv[0].toSint16() + (int)((argv[1].toSint16() + 1.0 - argv[0].toSint16()) * (rand() / (RAND_MAX + 1.0))));
}

reg_t kAbs(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	// This is a hack, but so is the code in Hoyle1 that needs it.
	if (argv[0].segment)
		return make_reg(0, 0x3e8); // Yes people, this is an object
	return make_reg(0, abs(argv[0].toSint16()));
}

reg_t kSqrt(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	return make_reg(0, (int16) sqrt((float) abs(argv[0].toSint16())));
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

reg_t kGetAngle(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	// Based on behavior observed with a test program created with
	// SCI Studio.
	int x1 = argv[0].toSint16();
	int y1 = argv[1].toSint16();
	int x2 = argv[2].toSint16();
	int y2 = argv[3].toSint16();
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

reg_t kGetDistance(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int xdiff = (argc > 3) ? argv[3].toSint16() : 0;
	int ydiff = (argc > 2) ? argv[2].toSint16() : 0;
	int angle = (argc > 5) ? argv[5].toSint16() : 0;
	int xrel = (int)(((float) argv[1].toSint16() - xdiff) / cos(angle * PI / 180.0)); // This works because cos(0)==1
	int yrel = argv[0].toSint16() - ydiff;
	return make_reg(0, (int16)sqrt((float) xrel*xrel + yrel*yrel));
}

reg_t kTimesSin(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int angle = argv[0].toSint16();
	int factor = argv[1].toSint16();

	return make_reg(0, (int)(factor * 1.0 * sin(angle * PI / 180.0)));
}

reg_t kTimesCos(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int angle = argv[0].toSint16();
	int factor = argv[1].toSint16();

	return make_reg(0, (int)(factor * 1.0 * cos(angle * PI / 180.0)));
}

reg_t kCosDiv(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int angle = argv[0].toSint16();
	int value = argv[1].toSint16();
	double cosval = cos(angle * PI / 180.0);

	if ((cosval < 0.0001) && (cosval > 0.0001)) {
		warning("Attepted division by zero");
		return make_reg(0, (int16)0x8000);
	} else
		return make_reg(0, (int16)(value / cosval));
}

reg_t kSinDiv(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int angle = argv[0].toSint16();
	int value = argv[1].toSint16();
	double sinval = sin(angle * PI / 180.0);

	if ((sinval < 0.0001) && (sinval > 0.0001)) {
		warning("Attepted division by zero");
		return make_reg(0, (int16)0x8000);
	} else
		return make_reg(0, (int16)(value / sinval));
}

reg_t kTimesTan(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int param = argv[0].toSint16();
	int scale = (argc > 1) ? argv[1].toSint16() : 1;

	param -= 90;
	if ((param % 90) == 0) {
		warning("Attempted tan(pi/2)");
		return make_reg(0, (int16)0x8000);
	} else
		return make_reg(0, (int16) - (tan(param * PI / 180.0) * scale));
}

reg_t kTimesCot(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	int param = argv[0].toSint16();
	int scale = (argc > 1) ? argv[1].toSint16() : 1;

	if ((param % 90) == 0) {
		warning("Attempted tan(pi/2)");
		return make_reg(0, (int16)0x8000);
	} else
		return make_reg(0, (int16)(tan(param * PI / 180.0) * scale));
}

} // End of namespace Sci
