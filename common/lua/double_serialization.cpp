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

#include "common/lua/double_serialization.h"

#include "common/scummsys.h"


namespace Util {

SerializedDouble encodeDouble(double value) {
	// Split the value into its significand and exponent
	int exponent;
	double significand = frexp(value, &exponent);

	// Shift the the first part of the significand into the integer range
	double shiftedsignificandPart = ldexp(fabs(significand), 32);
	uint32 significandOne = uint32(floor(shiftedsignificandPart));

	// Shift the remainder of the significand into the integer range
	shiftedsignificandPart -= significandOne;
	uint32 significandTwo = (uint32)(ldexp(shiftedsignificandPart, 31));

	SerializedDouble returnValue;
	returnValue.significandOne = significandOne;                                // SignificandOne
	returnValue.signAndSignificandTwo = ((uint32)(value < 0 ? 1 : 0) << 31) |   // Sign
	                                    significandTwo;                         // SignificandTwo
	returnValue.exponent = (int16)exponent;
	return returnValue;
}

double decodeDouble(SerializedDouble value) {
	// Expand the exponent and the parts of the significand
	int exponent = (int)value.exponent;
	double expandedsignificandOne = (double)value.significandOne;
	double expandedsignificandTwo = (double)(value.signAndSignificandTwo & 0x7FFFFFFF);

	// Deflate the significand
	double shiftedsignificand = ldexp(expandedsignificandTwo, -21);
	double significand = ldexp(expandedsignificandOne + shiftedsignificand, -32);

	// Re-calculate the actual double
	double returnValue = ldexp(significand, exponent);

	// Check the sign bit and return
	return ((value.signAndSignificandTwo & 0x80000000) == 0x80000000) ? -returnValue : returnValue;
}

} // End of namespace Util
