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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifdef ENABLE_HE

#include "scumm/he/intern_he.h"
#include "scumm/scumm.h"
#include "scumm/util.h"

namespace Scumm {

static const int _scummMathSineTable[181] = {
	     0,  1745,  3489,  5233,  6975,  8715, 10452, 12186, 13917,
	 15643, 17364, 19080, 20791, 22495, 24192, 25881, 27563, 29237,
	 30901, 32556, 34202, 35836, 37460, 39073, 40673, 42261, 43837,
	 45399, 46947, 48480, 50000, 51503, 52991, 54463, 55919, 57357,
	 58778, 60181, 61656, 62932, 64278, 65605, 66913, 68199, 69465,
	 70710, 71933, 73135, 74314, 75470, 76604, 77714, 78801, 79863,
	 80901, 81915, 82903, 83867, 84804, 85716, 86602, 87461, 88294,
	 89100, 89879, 90630, 91354, 92050, 92718, 93358, 93969, 94551,
	 95105, 95630, 96126, 96592, 97029, 97437, 97814, 98162, 98480,
	 98768, 99026, 99254, 99452, 99619, 99756, 99862, 99939, 99984,
	100000, 99984, 99939, 99862, 99756, 99619, 99452, 99254, 99026,
	 98768, 98480, 98162, 97814, 97437, 97029, 96592, 96126, 95630,
	 95105, 94551, 93969, 93358, 92718, 92050, 91354, 90630, 89879,
	 89100, 88294, 87461, 86602, 85716, 84804, 83867, 82903, 81915,
	 80901, 79863, 78801, 77714, 76604, 75470, 74314, 73135, 71933,
	 70710,	69465, 68199, 66913, 65605, 64278, 62932, 61656, 60181,
	 58778, 57357, 55919, 54463, 52991, 51503, 50000, 48480, 46947,
	 45399, 43837, 42261, 40673, 39073, 37460, 35836, 34202, 32556,
	 30901, 29237, 27563, 25881, 24192, 22495, 20791, 19080, 17364,
	 15643, 13917, 12186, 10452,  8715,  6975,  5233,  3489,  1745,
	 0
};

int ScummEngine_v90he::scummMathSin(int angle) {
	bool positive;

	if (angle) {
		positive = (angle > 0) ? 1 : 0;
		angle %= 360;
		angle = abs(angle);

		if (angle > 180) {
			positive = !positive;
			angle -= 180;
		}

		if (positive) {
			return _scummMathSineTable[angle];
		} else {
			return -(_scummMathSineTable[angle]);
		}
	}

	return 0;
}

int ScummEngine_v90he::scummMathCos(int angle) {
	bool positive;

	if (angle += 90) {
		positive = (angle > 0) ? 1 : 0;
		angle %= 360;
		angle = abs(angle);

		if (angle > 180) {
			positive = !positive;
			angle -= 180;
		}

		if (positive) {
			return _scummMathSineTable[angle];

		} else {
			return -(_scummMathSineTable[angle]);
		}
	}

	return 0;
}

int ScummEngine_v90he::scummMathSqrt(int value) {
	int32 s1, s2, x;

	x = (int32)abs(value);
	if (x < 2)
		return x;

	s1 = x;
	s2 = 2;

	do {
		s1 >>= 1;
		s2 <<= 1;
	} while (s1 > s2);

	do {
		s1 = s2;
		s2 = (x / s1 + s1) / 2;
	} while (s1 > s2);

	return s2;
}

int ScummEngine_v90he::scummMathDist2D(int x1, int y1, int x2, int y2) {
	int dx, dy;

	dx = x1 - x2;
	dx *= dx;

	dy = y1 - y2;
	dy *= dy;

	return scummMathSqrt(dx + dy);
}


int ScummEngine_v90he::scummMathAngleFromDelta(int dx, int dy) {
	int angle = (int)((atan2((double)dy, (double)dx) * 180.0) / M_PI);

	if (angle < 0) {
		angle = 360 + angle;
	}

	if (angle == 360)
		angle = 0;

	return angle;
}

int ScummEngine_v90he::scummMathAngleOfLineSegment(int x1, int y1, int x2, int y2) {
	return scummMathAngleFromDelta((x2 - x1), (y2 - y1));
}

} // End of namespace Scumm

#endif // ENABLE_HE
