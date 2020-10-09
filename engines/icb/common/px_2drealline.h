/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#ifndef ICB_PX2DLINE_H_INCLUDED
#define ICB_PX2DLINE_H_INCLUDED

#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/common/px_array.h"
#include "engines/icb/common/px_2drealpoint.h"

namespace ICB {

// Note, this header file needs somethings :
// e.g. PXreal, REAL_ZERO
#ifndef REAL_ZERO
#error "REAL_ZERO not defined in px2drealline.h"
#endif // #ifndef REAL_ZERO

// Here I define an array type for the class.  This neatens syntax and also allows pxArrays of pxArrays to be declared.
class px2DRealLine;
typedef rcActArray<px2DRealLine> px2DRealLineArray;

// Holds a line on a plane as a pair of integer endpoints, and provides functions for working with them.
class px2DRealLine {
public:
	// Definitions for this class.
	enum IntersectionLogicVal { DONT_INTERSECT, DO_INTERSECT, COLLINEAR };

	// Default constructor and destructor.
	inline px2DRealLine() {
		m_fX1 = REAL_ZERO;
		m_fY1 = REAL_ZERO;
		m_fX2 = REAL_ZERO;
		m_fY2 = REAL_ZERO;
	}
	inline ~px2DRealLine() { ; }

	// Gets and sets.
	PXreal GetX1() const { return m_fX1; }
	PXreal GetY1() const { return m_fY1; }
	PXreal GetX2() const { return m_fX2; }
	PXreal GetY2() const { return m_fY2; }

	void SetX1(PXreal fX1) { m_fX1 = fX1; }
	void SetY1(PXreal fY1) { m_fY1 = fY1; }
	void SetX2(PXreal fX2) { m_fX2 = fX2; }
	void SetY2(PXreal fY2) { m_fY2 = fY2; }

	// This determines whether this-> line intersects another.
	IntersectionLogicVal Intersects(const px2DRealLine &oLineB, px2DRealPoint &oIntersection) const;

private:
	PXreal m_fX1, m_fY1, m_fX2, m_fY2; // The line's endpoints.

	// Functions used only by this class.
	inline bool8 SameSigns(PXreal dA, PXreal dB) const;
};

inline bool8 px2DRealLine::SameSigns(PXreal fA, PXreal fB) const {
	if (fA < REAL_ZERO) {
		if (fB < REAL_ZERO)
			return TRUE8;
		else
			return FALSE8;
	} else {
		if (fB < REAL_ZERO)
			return FALSE8;
		else
			return TRUE8;
	}
}

} // End of namespace ICB

#endif // #ifndef PX2DLINE_H_INCLUDED
