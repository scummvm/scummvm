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

#include "ags/lib/allegro/fmaths.h"

namespace AGS3 {

fixed fixsqrt(fixed x) {
	if (x > 0)
		return ftofix(sqrt(fixtof(x)));

	if (x < 0)
		*allegro_errno = EDOM;

	return 0;
}

fixed fixhypot(fixed x, fixed y) {
	return ftofix(hypot(fixtof(x), fixtof(y)));
}

fixed fixatan(fixed x) {
	int a, b, c;            /* for binary search */
	fixed d;                /* difference value for search */

	if (x >= 0) {           /* search the first part of tan table */
		a = 0;
		b = 127;
	} else {                  /* search the second half instead */
		a = 128;
		b = 255;
	}

	do {
		c = (a + b) >> 1;
		d = x - _tan_tbl[c];

		if (d > 0)
			a = c + 1;
		else if (d < 0)
			b = c - 1;

	} while ((a <= b) && (d));

	if (x >= 0)
		return ((long)c) << 15;

	return (-0x00800000L + (((long)c) << 15));
}

fixed fixatan2(fixed y, fixed x) {
	fixed r;

	if (x == 0) {
		if (y == 0) {
			*allegro_errno = EDOM;
			return 0L;
		} else
			return ((y < 0) ? -0x00400000L : 0x00400000L);
	}

	*allegro_errno = 0;
	r = fixdiv(y, x);

	if (*allegro_errno) {
		*allegro_errno = 0;
		return ((y < 0) ? -0x00400000L : 0x00400000L);
	}

	r = fixatan(r);

	if (x >= 0)
		return r;

	if (y >= 0)
		return 0x00800000L + r;

	return r - 0x00800000L;
}

} // namespace AGS3
