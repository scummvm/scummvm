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

#include "sword25/util/double_serialization.h"

#include "common/scummsys.h"


namespace Util {

SerializedDouble encodeDouble(double value) {
	// Split the value into its significand and exponent
	int exponent;
	double significand = frexp(value, &exponent);

	// Shift the the first part of the significand into the integer range
	double shiftedsignificandPart = ldexp(abs(significand), 32);
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

#if 0

// Why these are needed?

uint64 encodeDouble_64(double value) {
	// Split the value into its significand and exponent
	int exponent;
	double significand = frexp(value, &exponent);

	// Shift the significand into the integer range
	double shiftedsignificand = ldexp(abs(significand), 53);

	// Combine everything using the IEEE standard
	uint64 uintsignificand = (uint64)shiftedsignificand;
	return ((uint64)(value < 0 ? 1 : 0) << 63) |        // Sign
	       ((uint64)(exponent + 1023) << 52) |          // Exponent stored as an offset to 1023
	       (uintsignificand & 0x000FFFFFFFFFFFFFLL);      // significand with MSB inferred
}

double decodeDouble_64(uint64 value) {
	// Expand the exponent and significand
	int exponent = (int)((value >> 52) & 0x7FF) - 1023;
	double expandedsignificand = (double)(0x10000000000000LL /* Inferred MSB */ | (value & 0x000FFFFFFFFFFFFFLL));

	// Deflate the significand
	int temp;
	double significand = frexp(expandedsignificand, &temp);

	// Re-calculate the actual double
	double returnValue = ldexp(significand, exponent);

	// Check the sign bit and return
	return ((value & 0x8000000000000000LL) == 0x8000000000000000LL) ? -returnValue : returnValue;
}

CompactSerializedDouble encodeDouble_Compact(double value) {
	// Split the value into its significand and exponent
	int exponent;
	double significand = frexp(value, &exponent);

	// Shift the the first part of the significand into the integer range
	double shiftedsignificandPart = ldexp(abs(significand), 32);
	uint32 significandOne = uint32(floor(shiftedsignificandPart));

	// Shift the remainder of the significand into the integer range
	shiftedsignificandPart -= significandOne;
	uint32 significandTwo = (uint32)(ldexp(shiftedsignificandPart, 21));

	CompactSerializedDouble returnValue;
	returnValue.signAndSignificandOne = ((uint32)(value < 0 ? 1 : 0) << 31) |   // Sign
	                                    (significandOne & 0x7FFFFFFF);          // significandOne with MSB inferred
	// Exponent stored as an offset to 1023
	returnValue.exponentAndSignificandTwo = ((uint32)(exponent + 1023) << 21) | significandTwo;

	return returnValue;
}

double decodeDouble_Compact(CompactSerializedDouble value) {
	// Expand the exponent and the parts of the significand
	int exponent = (int)(value.exponentAndSignificandTwo >> 21) - 1023;
	double expandedsignificandOne = (double)(0x80000000 /* Inferred MSB */ | (value.signAndSignificandOne & 0x7FFFFFFF));
	double expandedsignificandTwo = (double)(value.exponentAndSignificandTwo & 0x1FFFFF);

	// Deflate the significand
	double shiftedsignificand = ldexp(expandedsignificandTwo, -21);
	double significand = ldexp(expandedsignificandOne + shiftedsignificand, -32);

	// Re-calculate the actual double
	double returnValue = ldexp(significand, exponent);

	// Check the sign bit and return
	return ((value.signAndSignificandOne & 0x80000000) == 0x80000000) ? -returnValue : returnValue;
}

#endif

} // End of namespace Sword25
