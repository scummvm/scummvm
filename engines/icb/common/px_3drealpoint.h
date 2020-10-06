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

#ifndef ICB_PX_3DREALPOINT_H_INCLUDED
#define ICB_PX_3DREALPOINT_H_INCLUDED

// Include the header files needed by this class.
#include "engines/icb/common/px_rccommon.h"
#include "engines/icb/common/px_rcutypes.h"

namespace ICB {

// Note, this needs pre-definitions / typedef's of :
// PXreal, REAL_ZERO
// For PC PXreal = float, REAL_ZERO = 0.0f
// For PSX PXreal = int, REAL_ZERO = 0

#ifndef REAL_ZERO
#error "REAL_ZERO not defined"
#endif // #ifndef REAL_ZERO

// A 2D point on a plane with endpoints stored as floating point.
class px3DRealPoint {
public:
	// Default constructor and destructor.
	px3DRealPoint() {
		m_fX = REAL_ZERO;
		m_fY = REAL_ZERO;
		m_fZ = REAL_ZERO;
	}
	~px3DRealPoint() { ; }

	// Alternative constructor that allows the point to be initialized.
	px3DRealPoint(PXreal fX, PXreal fY, PXreal fZ) {
		m_fX = fX;
		m_fY = fY;
		m_fZ = fZ;
	}

	// Gets and sets.
	void SetX(PXreal fX) { m_fX = fX; }
	void SetY(PXreal fY) { m_fY = fY; }
	void SetZ(PXreal fZ) { m_fZ = fZ; }
	PXreal GetX() const { return m_fX; }
	PXreal GetY() const { return m_fY; }
	PXreal GetZ() const { return m_fZ; }

	// This allows the values of a point to be set after it has been created.
	void Set(PXreal fX, PXreal fY, PXreal fZ) {
		m_fX = fX;
		m_fY = fY;
		m_fZ = fZ;
	}

private:
	PXreal m_fX, m_fY, m_fZ; // The point.
};

#if 0
inline bool8 px3DRealPoint::operator == (const px3DRealPoint &obOpB) const {
	if ((PXfabs(m_fX - obOpB.m_fX) < (FLT_MIN * 5.0f)) &&
	        (PXfabs(m_fY - obOpB.m_fY) < (FLT_MIN * 5.0f)) &&
	        (PXfabs(m_fZ - obOpB.m_fZ) < (FLT_MIN * 5.0f)))
		return TRUE8;
	else
		return FALSE8;
}

#endif

} // End of namespace ICB

#endif // #ifndef PX_3DREALPOINT_H_INCLUDED
