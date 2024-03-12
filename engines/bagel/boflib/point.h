
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

#ifndef BAGEL_BOFLIB_POINT_H
#define BAGEL_BOFLIB_POINT_H

#include "common/rect.h"
#include "bagel/boflib/boffo.h"
#include "bagel/boflib/stdinc.h"
#include "bagel/boflib/object.h"

namespace Bagel {

typedef Common::Point POINT;

class CBofPoint : public bofPOINT, public CBofObject {
public:
	// Constructors
	CBofPoint();
	CBofPoint(const INT initX, const INT initY);
	CBofPoint(const ST_POINT stPoint);
	CBofPoint(const CBofPoint &cPoint);
	CBofPoint(const ST_SIZE &cSize);

	CBofPoint(const POINT &stPoint) {
		SetWinPoint(&stPoint);
	}

	VOID SetWinPoint(const POINT *pPoint);
	POINT GetWinPoint();

	VOID operator=(const POINT &stPoint) {
		SetWinPoint(&stPoint);
	}

	operator const POINT() {
		return GetWinPoint();
	}


	// Operations
	VOID Offset(INT xOffset, INT yOffset);
	VOID Offset(ST_POINT point);
	VOID Offset(ST_SIZE size);
	VOID operator=(const ST_POINT &point);
	VOID operator=(const CBofPoint &point);
	BOOL operator==(ST_POINT point);
	BOOL operator!=(ST_POINT point);
	VOID operator+=(ST_SIZE size);
	VOID operator-=(ST_SIZE size);
	VOID operator+=(ST_POINT point);
	VOID operator-=(ST_POINT point);

	// Operators returning CBofPoint values
	CBofPoint operator+(ST_SIZE size);
	CBofPoint operator+(ST_POINT point);
	CBofPoint operator-(ST_SIZE size);
	CBofPoint operator-(ST_POINT point);
	CBofPoint operator-();
};

// CBofPoint
INLINE CBofPoint::CBofPoint() {
	x = 0;
	y = 0;
}

INLINE CBofPoint::CBofPoint(const INT initX, const INT initY) {
	x = initX;
	y = initY;
}

INLINE CBofPoint::CBofPoint(const ST_POINT stPoint) {
	x = stPoint.x;
	y = stPoint.y;
}

INLINE CBofPoint::CBofPoint(const ST_SIZE &stSize) {
	x = stSize.cx;
	y = stSize.cy;
}

INLINE CBofPoint::CBofPoint(const CBofPoint &cPoint) {
	x = cPoint.x;
	y = cPoint.y;
}

INLINE VOID CBofPoint::SetWinPoint(const POINT *pPoint) {
	Assert(pPoint != nullptr);
	Assert(IsValidObject(this));

	x = pPoint->x;
	y = pPoint->y;
}

INLINE POINT CBofPoint::GetWinPoint() {
	Assert(IsValidObject(this));

	POINT stPoint;

	stPoint.x = x;
	stPoint.y = y;

	return stPoint;
}

INLINE VOID CBofPoint::Offset(INT xOffset, INT yOffset) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	x += xOffset;
	y += yOffset;
}

INLINE VOID CBofPoint::Offset(ST_POINT point) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	x += point.x;
	y += point.y;
}

INLINE VOID CBofPoint::Offset(ST_SIZE size) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	x += size.cx;
	y += size.cy;
}

INLINE VOID CBofPoint::operator=(const ST_POINT &point) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	x = point.x;
	y = point.y;
}

INLINE VOID CBofPoint::operator=(const CBofPoint &point) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	x = point.x;
	y = point.y;
}

INLINE BOOL CBofPoint::operator==(ST_POINT point) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return (x == point.x && y == point.y);
}

INLINE BOOL CBofPoint::operator!=(ST_POINT point) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return (x != point.x || y != point.y);
}

INLINE VOID CBofPoint::operator+=(ST_SIZE size) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	x += size.cx;
	y += size.cy;
}

INLINE VOID CBofPoint::operator+=(ST_POINT point) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	x += point.x;
	y += point.y;
}

INLINE VOID CBofPoint::operator-=(ST_POINT point) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	x -= point.x;
	y -= point.y;
}

INLINE VOID CBofPoint::operator-=(ST_SIZE size) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	x -= size.cx;
	y -= size.cy;
}

INLINE CBofPoint CBofPoint::operator+(ST_SIZE size) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return CBofPoint(x + size.cx, y + size.cy);
}

INLINE CBofPoint CBofPoint::operator+(ST_POINT point) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return CBofPoint(x + point.x, y + point.y);
}

INLINE CBofPoint CBofPoint::operator-(ST_SIZE size) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return CBofPoint(x - size.cx, y - size.cy);
}

INLINE CBofPoint CBofPoint::operator-(ST_POINT point) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return CBofPoint(x - point.x, y - point.y);
}

INLINE CBofPoint CBofPoint::operator-() {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return CBofPoint(-x, -y);
}

#define CPoint CBofPoint

} // namespace Bagel

#endif
