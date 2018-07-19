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
 */

#ifndef STARTREK_FIXEDINT_H
#define STARTREK_FIXEDINT_H

#include "common/serializer.h"

#include "startrek/common.h"

namespace StarTrek {

/**
 * Signed fixed-point number.
 */
template<typename T, uint totalBits, uint decimalBits>
class TFixedInt : Common::Serializable {
	const static int max = (1 << (totalBits - decimalBits - 1)) - 1;
	const static int min = -max - 1;

	T val;

public:
	static TFixedInt fromRaw(T raw) {
		TFixedInt ret;
		ret.val = raw;
		return ret;
	}

	TFixedInt() : val(0) {}
	TFixedInt(double d) {
		assert(d >= min && d <= max); // FIXME: downgrade this to a warning?
		val = (T)(d * (1 << decimalBits));
	}

	/**
	 * Constructor from other fixed-point formats.
	 */
	template<typename T2, uint otherTB, uint otherDB>
	TFixedInt<T, totalBits, decimalBits>(const TFixedInt<T2, otherTB, otherDB> &fi) {
		int diff = otherDB - decimalBits;
		if (otherDB >= decimalBits)
			val = fi.raw() >> diff;
		else
			val = fi.raw() << (-diff);
	}

	T raw() const {
		return val;
	}

	int16 toInt() const {
		return val >> decimalBits;
	}

	double toDouble() const {
		return ((double)val) / (1 << decimalBits);
	}

	/**
	 * Multiplication with an int, with the result being an int.
	 */
	int32 multToInt(int32 i) {
		return ((val * i) << (totalBits - decimalBits)) >> totalBits;
	}

	/**
	 * Multiplication with an int, with the result being the same type.
	 */
	TFixedInt operator*(int32 i) const {
		return fromRaw(val * i);
	}
	/**
	 * Division with an int, with the result being the same type.
	 */
	TFixedInt operator/(int32 i) const {
		return fromRaw(val / i);
	}
	TFixedInt operator+(const TFixedInt &f) const {
		return fromRaw(val + f.val);
	}
	TFixedInt operator-(const TFixedInt &f) const {
		return fromRaw(val - f.val);
	}

	void operator+=(const TFixedInt &f) {
		val += f.val;
	}
	void operator-=(const TFixedInt &f) {
		val -= f.val;
	}

	bool operator==(double d) const {
		return toDouble() == d;
	}
	bool operator!=(double d) const {
		return toDouble() != d;
	}
	bool operator<(double d) const {
		return toDouble() < d;
	}
	bool operator<=(double d) const {
		return toDouble() <= d;
	}

	void saveLoadWithSerializer(Common::Serializer &ser) {
		if (totalBits == 16)
			ser.syncAsSint16LE(val);
		else if (totalBits == 32)
			ser.syncAsSint32LE(val);
		else
			error("Unsupported bit size for TFixedInt");
	}
};

template<typename T, uint totalBits, uint decimalBits>
int32 operator*(const int16 lhs, const TFixedInt<T, totalBits, decimalBits> &rhs) {
	return rhs * lhs;
}


// Fixed-point (2.14) number (between -1 and 1)
typedef TFixedInt<int16, 16, 14> Fixed14;

// Fixed-point (8.8) number
typedef TFixedInt<int16, 16, 8> Fixed16;

// Fixed-point (16.16) number
typedef TFixedInt<int32, 32, 16> Fixed32;

}

#endif
