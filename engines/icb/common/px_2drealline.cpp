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

#include "engines/icb/common/px_2drealline.h"
#include "engines/icb/common/px_assert.h"

#include "common/math.h"

namespace ICB {

typedef float PXfloat;
typedef float PXreal;
#define REAL_ZERO 0.0f
#define REAL_TWO 2.0f
#define REAL_MAX FLT_MAX

px2DRealLine::IntersectionLogicVal px2DRealLine::Intersects(const px2DRealLine &oLineB, px2DRealPoint &oIntersection) const {
	PXfloat fAX, fBX, fCX, fAY, fBY, fCY;
	PXfloat fX1Low, fX1High, fY1Low, fY1High;
	PXfloat fD, fE, fF;

	// Initialize return value (costs little and tidies up code no end).
	oIntersection.SetX(REAL_MAX);
	oIntersection.SetY(REAL_MAX);

	// Work out some commonly used terms.
	fAX = m_fX2 - m_fX1;
	fBX = oLineB.m_fX1 - oLineB.m_fX2;

	// X bounding box test.
	if (fAX < REAL_ZERO) {
		fX1Low = m_fX2;
		fX1High = m_fX1;
	} else {
		fX1High = m_fX2;
		fX1Low = m_fX1;
	}

	if (fBX > REAL_ZERO) {
		if ((fX1High < oLineB.m_fX2) || oLineB.m_fX1 < fX1Low)
			return DONT_INTERSECT;
	} else {
		if ((fX1High < oLineB.m_fX1) || oLineB.m_fX2 < fX1Low)
			return DONT_INTERSECT;
	}

	// More common terms.
	fAY = m_fY2 - m_fY1;
	fBY = oLineB.m_fY1 - oLineB.m_fY2;

	// Y bounding box test.
	if (fAY < REAL_ZERO) {
		fY1Low = m_fY2;
		fY1High = m_fY1;
	} else {
		fY1High = m_fY2;
		fY1Low = m_fY1;
	}

	if (fBY > REAL_ZERO) {
		if ((fY1High < oLineB.m_fY2) || oLineB.m_fY1 < fY1Low)
			return DONT_INTERSECT;
	} else {
		if ((fY1High < oLineB.m_fY1) || oLineB.m_fY2 < fY1Low)
			return DONT_INTERSECT;
	}

	// Couldn't dismiss the lines on their bounding rectangles, so do a proper intersection.
	fCX = m_fX1 - oLineB.m_fX1;
	fCY = m_fY1 - oLineB.m_fY1;

	fD = (fBY * fCX) - (fBX * fCY);
	fF = (fAY * fBX) - (fAX * fBY);

	if (fF > REAL_ZERO) {
		if ((fD < REAL_ZERO) || (fD > fF))
			return DONT_INTERSECT;
	} else {
		if ((fD > REAL_ZERO) || (fD < fF))
			return DONT_INTERSECT;
	}

	fE = (fAX * fCY) - (fAY * fCX);

	if (fF > REAL_ZERO) {
		if ((fE < REAL_ZERO) || (fE > fF))
			return DONT_INTERSECT;
	} else {
		if ((fE > REAL_ZERO) || (fE < fF))
			return DONT_INTERSECT;
	}

	// At this point, we can say that the lines do intersect as int32 as they are not
	// colinear (colinear is indicated by fF == 0.0).
	if (fabs(fF - REAL_ZERO) < (FLT_MIN * 10.0f))
		return COLLINEAR;

	// Right, lines do intersect, so calculate where.
	PXfloat fNum, fOffset;
	fNum = fD * fAX;
	fOffset = SameSigns(fNum, fF) ? fF / REAL_TWO : -fF / REAL_TWO;
	oIntersection.SetX(m_fX1 + (fNum + fOffset) / fF);

	fNum = fD * fAY;
	fOffset = SameSigns(fNum, fF) ? fF / REAL_TWO : -fF / REAL_TWO;
	oIntersection.SetY(m_fY1 + (fNum + fOffset) / fF);

	return DO_INTERSECT;
}

} // End of namespace ICB
