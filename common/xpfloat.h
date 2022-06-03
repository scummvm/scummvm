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

#ifndef COMMON_XPFLOAT_H
#define COMMON_XPFLOAT_H

// 80-bit extended precision floating point
// Mostly encountered stored as data on Apple systems

#include "common/scummsys.h"

namespace Common {

struct XPFloat {
	uint16 signAndExponent;
	uint64 mantissa;

	enum Semantics {
		kSemanticsMC68881,
		kSemanticsSANE = kSemanticsMC68881,

		// Could add Intel 8087 and derivatives here since they're mostly compatible,
		// but have different NaN/INF flag cases.
	};

	XPFloat();
	XPFloat(uint16 signAndExponent, uint64 mantissa);
	XPFloat(const XPFloat &other);

	static XPFloat fromDouble(double value, Semantics semantics = kSemanticsMC68881);
	static XPFloat fromDoubleBits(uint64 value, Semantics semantics = kSemanticsMC68881);

	void toDoubleSafe(double &result, bool &outOverflowed, Semantics semantics = kSemanticsMC68881) const;
	void toDoubleBitsSafe(uint64 &result, bool &outOverflowed, Semantics semantics = kSemanticsMC68881) const;

	// Simple version that clamps to infinity and warns on overflow
	double toDouble(Semantics semantics = kSemanticsMC68881) const;
};

inline XPFloat::XPFloat() : signAndExponent(0), mantissa(0) {}
inline XPFloat::XPFloat(uint16 fSignAndExponent, uint64 fMantissa) : signAndExponent(fSignAndExponent), mantissa(fMantissa) {}
inline XPFloat::XPFloat(const XPFloat &other) : signAndExponent(other.signAndExponent), mantissa(other.mantissa) {}

}

#endif
