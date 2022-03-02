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

#include "ags/lib/allegro/fixed.h"
#include "ags/lib/allegro/error.h"
#include "ags/globals.h"

namespace AGS3 {

fixed ftofix(double x) {
	if (x > 32767.0) {
		*_G(allegro_errno) = AL_ERANGE;
		return 0x7FFFFFFF;
	}

	if (x < -32767.0) {
		*_G(allegro_errno) = AL_ERANGE;
		return (fixed) - 0x7FFFFFFF;
	}

	return (fixed)(x * 65536.0 + (x < 0 ? -0.5 : 0.5));
}

double fixtof(fixed x) {
	return (double)x / 65536.0;
}

fixed fixadd(fixed x, fixed y) {
	fixed result = x + y;

	if (result >= 0) {
		if ((x < 0) && (y < 0)) {
			*_G(allegro_errno) = AL_ERANGE;
			return (fixed) - 0x7FFFFFFF;
		} else
			return result;
	} else {
		if ((x > 0) && (y > 0)) {
			*_G(allegro_errno) = AL_ERANGE;
			return 0x7FFFFFFF;
		} else
			return result;
	}
}

fixed fixsub(fixed x, fixed y) {
	fixed result = x - y;

	if (result >= 0) {
		if ((x < 0) && (y > 0)) {
			*_G(allegro_errno) = AL_ERANGE;
			return (fixed) - 0x7FFFFFFF;
		} else
			return result;
	} else {
		if ((x > 0) && (y < 0)) {
			*_G(allegro_errno) = AL_ERANGE;
			return 0x7FFFFFFF;
		} else
			return result;
	}
}

fixed fixmul(fixed x, fixed y) {
	int64 lx = x;
	int64 ly = y;
	int64 lres = (lx * ly);

	if (lres > 0x7FFFFFFF0000LL) {
		*_G(allegro_errno) = AL_ERANGE;
		return 0x7FFFFFFF;
	} else if (lres < -0x7FFFFFFF0000LL) {
		*_G(allegro_errno) = AL_ERANGE;
		return 0x80000000;
	} else {
		int res = lres >> 16;
		return res;
	}
}

fixed fixdiv(fixed x, fixed y) {
	if (y == 0) {
		*_G(allegro_errno) = AL_ERANGE;
		return (fixed)(x < 0) ? -0x7FFFFFFF : 0x7FFFFFFF;
	} else
		return ftofix(fixtof(x) / fixtof(y));
}

int fixfloor(fixed x) {
	// FIXME: GCC warning "this condition has identical branches [-Wduplicated-branches]" on this code i.e. both branches are functionally identical. Remove?
#if 0
	/* (x >> 16) is not portable */
	if (x >= 0)
		return (x >> 16);
	else
		return ~((~x) >> 16);
#else
	return (x >> 16);
#endif
}


int fixceil(fixed x) {
	if (x > 0x7FFF0000) {
		*_G(allegro_errno) = AL_ERANGE;
		return 0x7FFF;
	}

	return fixfloor(x + 0xFFFF);
}

fixed itofix(int x) {
	return x << 16;
}


int fixtoi(fixed x) {
	return fixfloor(x) + ((x & 0x8000) >> 15);
}


fixed fixcos(fixed x) {
	return _cos_tbl[((x + 0x4000) >> 15) & 0x1FF];
}


fixed fixsin(fixed x) {
	return _cos_tbl[((x - 0x400000 + 0x4000) >> 15) & 0x1FF];
}


fixed fixtan(fixed x) {
	return _tan_tbl[((x + 0x4000) >> 15) & 0xFF];
}


fixed fixacos(fixed x) {
	if ((x < -65536) || (x > 65536)) {
		*_G(allegro_errno) = AL_EDOM;
		return 0;
	}

	return _acos_tbl[(x + 65536 + 127) >> 8];
}


fixed fixasin(fixed x) {
	if ((x < -65536) || (x > 65536)) {
		*_G(allegro_errno) = AL_EDOM;
		return 0;
	}

	return 0x00400000 - _acos_tbl[(x + 65536 + 127) >> 8];
}

} // namespace AGS3
