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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/glulx/glulx.h"

namespace Glk {
namespace Glulx {

uint Glulx::encode_float(gfloat32 val) {
	gfloat32 absval;
	uint sign;
	int expo;
	gfloat32 mant;
	uint fbits;

	if (signbit(val)) {
		sign = 0x80000000;
		absval = -val;
	} else {
		sign = 0x0;
		absval = val;
	}

	if (isinf(val)) {
		return sign | 0x7f800000; /* infinity */
	}

	if (isnan(val)) {
		return sign | 0x7fc00000;
	}

	mant = frexpf(absval, &expo);

	/* Normalize mantissa to be in the range [1.0, 2.0) */
	if (0.5 <= mant && mant < 1.0) {
		mant *= 2.0;
		expo--;
	} else if (mant == 0.0) {
		expo = 0;
	} else {
		return sign | 0x7f800000; /* infinity */
	}

	if (expo >= 128) {
		return sign | 0x7f800000; /* infinity */
	} else if (expo < -126) {
		/* Denormalized (very small) number */
		mant = ldexpf(mant, 126 + expo);
		expo = 0;
	} else if (!(expo == 0 && mant == 0.0)) {
		expo += 127;
		mant -= 1.0; /* Get rid of leading 1 */
	}

	mant *= 8388608.0; /* 2^23 */
	fbits = (uint)(mant + 0.5); /* round mant to nearest int */
	if (fbits >> 23) {
		/* The carry propagated out of a string of 23 1 bits. */
		fbits = 0;
		expo++;
		if (expo >= 255) {
			return sign | 0x7f800000; /* infinity */
		}
	}

	return (sign) | ((uint)(expo << 23)) | (fbits);
}

gfloat32 Glulx::decode_float(uint val) {
	int sign;
	int expo;
	uint mant;
	gfloat32 res;

	/* First byte */
	sign = ((val & 0x80000000) != 0);
	expo = (val >> 23) & 0xFF;
	mant = val & 0x7FFFFF;

	if (expo == 255) {
		if (mant == 0) {
			/* Infinity */
			return (sign ? (-INFINITY) : (INFINITY));
		} else {
			/* Not a number */
			return (sign ? (-NAN) : (NAN));
		}
	}

	res = (gfloat32)mant / 8388608.0;

	if (expo == 0) {
		expo = -126;
	} else {
		res += 1.0;
		expo -= 127;
	}
	res = ldexpf(res, expo);

	return (sign ? (-res) : (res));
}

} // End of namespace Glulx
} // End of namespace Glk
