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

#include "common/xpfloat.h"
#include "common/textconsole.h"

/*
Format:
s eeeeeeeeeeeeeee i fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
^ ^               ^ ^
| |               | |
| Exponent (15)   | Fraction (63)
Sign (1)          Integer i (1)

MC68881 semantics:
e                i    f     meaning
0 <= e <= 32766  1    any   (-1)^s x 2^(e-16383) x (1.f)    Normalized
0 <= e <= 32766  0    non-0 (-1)^s x 2^(e-16383) x (0.f)    Denormalized
0 <= e <= 32766  0    0     (-1)^s x 0                      Zero
32767            any  0     (-1)^s x Infinity               Infinity
32767            any  non-0 NaN                             NaN
*/

namespace Common {

XPFloat XPFloat::fromDouble(double value, Semantics semantics) {
	uint64 bits;
	memcpy(&bits, &value, 8);
	return fromDoubleBits(bits, semantics);
}

XPFloat XPFloat::fromDoubleBits(uint64 inBits, Semantics semantics) {
	uint64 inMantissa = inBits & 0xfffffffffffffu;
	int16 inExponent = (inBits >> 52) & 0x7ff;
	uint8 inSign = (inBits >> 63) & 1;

	// Convert to 1.63 fraction and absolute exponent
	uint64 workMantissa = 0;
	int16 workExponent = 0;
	if (inExponent == 0) {
		if (inMantissa == 0) {
			// +/- 0
			return XPFloat(inSign << 15, 0);
		} else {
			// Subnormal
			workMantissa = inMantissa << 11;
			workExponent = -1022;

			// Move implicit 1 to the high bit
			while ((workMantissa & 0x8000000000000000u) == 0) {
				workMantissa <<= 1;
				workExponent--;
			}
		}
	} else if (inExponent == 0x7ff) {
		if (inMantissa == 0) {
			// Infinity
			return XPFloat((inSign << 15) | 0x7fffu, static_cast<uint64>(1) << 63);
		} else {
			// NaN
			return XPFloat(0xFFFFu, 0xffffffffffffffffu);
		}
	} else {
		// Normal number
		workExponent = inExponent - 1023;
		workMantissa = (inMantissa | 0x10000000000000) << 11;
	}

	return XPFloat((inSign << 15) | (workExponent + 16383), workMantissa);
}

void XPFloat::toDoubleSafe(double &result, bool &outOverflowed, Semantics semantics) const {
	uint64 temp;
	toDoubleBitsSafe(temp, outOverflowed, semantics);
	memcpy(&result, &temp, 8);
}

void XPFloat::toDoubleBitsSafe(uint64 &result, bool &outOverflowed, Semantics semantics) const {
	bool overflowed = false;
	uint64 doubleBits = 0;
	if ((signAndExponent & 0x7fff) == 0x7fff) {
		if ((mantissa & 0x7fffffffffffffffu) == 0) {
			// Infinity
			doubleBits = (static_cast<uint64>(signAndExponent & 0x8000) << 63) | 0x7ff0000000000000u;
		} else {
			// NaN
			doubleBits = 0xffffffffffffffff;
		}
	} else {
		// For MC68881 semantics, denormal and normal numbers are handled the same way because the
		// i bit is effectively an explicit 1.
		uint8 signBit = ((signAndExponent >> 15) & 1);
		if (mantissa == 0) {
			// +/- 0
			doubleBits = static_cast<uint64>(signBit) << 63;
		} else {
			// Convert to 1.63
			int32 workExponent = static_cast<int32>(signAndExponent & 0x7fff) - 16383;
			uint64 workMantissa = mantissa;

			while ((workMantissa & 0x8000000000000000u) == 0) {
				workMantissa <<= 1;
				workExponent--;
			}

			int32 adjustedExponent = workExponent + 1023;
			if (adjustedExponent < 0) {
				// Subnormal
				int subnormalBits = -adjustedExponent;
				if (subnormalBits > 52)
					workMantissa = 0;
				else
					workMantissa >>= subnormalBits;
				adjustedExponent = 0;
			} else {
				// Normal
				if (adjustedExponent >= 0x7ff) {
					// Overflow to +/- infinity
					overflowed = true;
					adjustedExponent = 0x7ff;
					workMantissa = 0;
				}
			}

			doubleBits = (static_cast<uint64>(signBit) << 63) | (static_cast<uint64>(adjustedExponent) << 52) | ((workMantissa >> 11) & 0xfffffffffffffu);
		}
	}

	memcpy(&result, &doubleBits, 8);
	outOverflowed = overflowed;
}

double XPFloat::toDouble(Semantics semantics) const {
	double result;
	bool overflowed;
	toDoubleSafe(result, overflowed, semantics);
	if (overflowed)
		warning("Extended-precision floating point value was too large to represent as a double");

	return result;
}

}
