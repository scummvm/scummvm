
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

#ifndef BAGEL_BOFLIB_FIXED_H
#define BAGEL_BOFLIB_FIXED_H

#include "bagel/boflib/object.h"
#include "bagel/boflib/misc.h"

namespace Bagel {
namespace SpaceBar {

class CBofFixed : public CBofObject {
private:
	Fixed _lVal;
	CBofFixed(const Fixed Arg) {
		_lVal = Arg;
	}

public:
	// Constructors
	CBofFixed() {
		_lVal = 0L;
	}
	CBofFixed(const CBofFixed &Arg) {
		_lVal = Arg._lVal;
	}
	CBofFixed(const int Arg) {
		_lVal = (Fixed)(((long)(Arg)) << 16);
	}
	CBofFixed(const double Arg) {
		_lVal = (Fixed)(Arg * (1 << 16));
	}

	// Operators
	// inline CBofFixed operator =(const CBofFixed& Arg) const;
	CBofFixed operator+(const CBofFixed &Arg) const {
		return _lVal + Arg._lVal;
	}
	CBofFixed operator-(const CBofFixed &Arg) const {
		return _lVal - Arg._lVal;
	}

	CBofFixed operator*(const CBofFixed &Arg) const {
		return fixedMultiply(_lVal, Arg._lVal);
	}

	CBofFixed operator/(const CBofFixed &Arg) const {
		return fixedDivide(_lVal, Arg._lVal);
	}

	CBofFixed operator-=(const CBofFixed &Arg) {
		_lVal -= Arg._lVal;
		return *this;
	}

	CBofFixed operator+=(const CBofFixed &Arg) {
		_lVal += Arg._lVal;
		return *this;
	}

	CBofFixed operator*=(const CBofFixed &Arg) {
		_lVal = _lVal * Arg._lVal;

		return *this;
	}

	CBofFixed operator/=(const CBofFixed &Arg) {
		_lVal = _lVal / Arg._lVal;

		return *this;
	}

	CBofFixed operator=(const CBofFixed &Arg) {
		_lVal = Arg._lVal;
		return *this;
	}

	// Conversion operators
	operator int() {
		return (int)(((int32)_lVal) >> 16);
	}
};

} // namespace SpaceBar
} // namespace Bagel

#endif
