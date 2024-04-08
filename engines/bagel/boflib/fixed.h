
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

class CBofFixed : public CBofObject {
private:
	Fixed m_lVal;
	CBofFixed(const Fixed Arg) {
		m_lVal = Arg;
	}

public:
	// Constructors
	CBofFixed() {
		m_lVal = 0L;
	}
	CBofFixed(const CBofFixed &Arg) {
		m_lVal = Arg.m_lVal;
	}
	CBofFixed(const INT Arg) {
		m_lVal = (Fixed)(((long)(Arg)) << 16);
	}
	CBofFixed(const double Arg) {
		m_lVal = (Fixed)(Arg * (1 << 16));
	}

	// Operators
	// inline CBofFixed operator =(const CBofFixed& Arg) const;
	CBofFixed operator+(const CBofFixed &Arg) const {
		return m_lVal + Arg.m_lVal;
	}
	CBofFixed operator-(const CBofFixed &Arg) const {
		return m_lVal - Arg.m_lVal;
	}

	CBofFixed operator*(const CBofFixed &Arg) const {
		return FixedMultiply(m_lVal, Arg.m_lVal);
	}

	CBofFixed operator/(const CBofFixed &Arg) const {
		return FixedDivide(m_lVal, Arg.m_lVal);
	}

	CBofFixed operator-=(const CBofFixed &Arg) {
		m_lVal -= Arg.m_lVal;
		return *this;
	}

	CBofFixed operator+=(const CBofFixed &Arg) {
		m_lVal += Arg.m_lVal;
		return *this;
	}

	CBofFixed operator*=(const CBofFixed &Arg) {
		m_lVal = m_lVal * Arg.m_lVal;

		return *this;
	}

	CBofFixed operator/=(const CBofFixed &Arg) {
		m_lVal = m_lVal / Arg.m_lVal;

		return *this;
	}

	CBofFixed operator=(const CBofFixed &Arg) {
		m_lVal = Arg.m_lVal;
		return *this;
	}

	// Conversion operators
	operator INT() {
		return (INT)(((LONG)m_lVal) >> 16);
	}
};

} // namespace Bagel

#endif
