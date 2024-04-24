
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
#include "bagel/boflib/stdinc.h"
#include "bagel/boflib/object.h"

namespace Bagel {

typedef Common::Point POINT;

class CBofPoint : public StPoint, public CBofObject {
public:
	// Constructors
	CBofPoint();
	CBofPoint(const int initX, const int initY);
	CBofPoint(const StPoint stPoint);
	CBofPoint(const CBofPoint &cPoint);
	CBofPoint(const StSize &cSize);

	CBofPoint(const POINT &stPoint) {
		SetWinPoint(&stPoint);
	}

	void SetWinPoint(const POINT *pPoint);
	POINT GetWinPoint();

	void operator=(const POINT &stPoint) {
		SetWinPoint(&stPoint);
	}

	operator const POINT() {
		return GetWinPoint();
	}


	// Operations
	void Offset(int xOffset, int yOffset);
	void Offset(StPoint point);
	void Offset(StSize size);
	void operator=(const StPoint &point);
	void operator=(const CBofPoint &point);
	bool operator==(StPoint point);
	bool operator!=(StPoint point);
	void operator+=(StSize size);
	void operator-=(StSize size);
	void operator+=(StPoint point);
	void operator-=(StPoint point);

	// Operators returning CBofPoint values
	CBofPoint operator+(StSize size);
	CBofPoint operator+(StPoint point);
	CBofPoint operator-(StSize size);
	CBofPoint operator-(StPoint point);
	CBofPoint operator-();
};

// CBofPoint
inline CBofPoint::CBofPoint() {
	x = 0;
	y = 0;
}

inline CBofPoint::CBofPoint(const int initX, const int initY) {
	x = initX;
	y = initY;
}

inline CBofPoint::CBofPoint(const StPoint stPoint) {
	x = stPoint.x;
	y = stPoint.y;
}

inline CBofPoint::CBofPoint(const StSize &stSize) {
	x = stSize.cx;
	y = stSize.cy;
}

inline CBofPoint::CBofPoint(const CBofPoint &cPoint) {
	x = cPoint.x;
	y = cPoint.y;
}

inline void CBofPoint::SetWinPoint(const POINT *pPoint) {
	Assert(pPoint != nullptr);
	Assert(IsValidObject(this));

	x = pPoint->x;
	y = pPoint->y;
}

inline POINT CBofPoint::GetWinPoint() {
	Assert(IsValidObject(this));

	POINT stPoint;

	stPoint.x = x;
	stPoint.y = y;

	return stPoint;
}

inline void CBofPoint::Offset(int xOffset, int yOffset) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	x += xOffset;
	y += yOffset;
}

inline void CBofPoint::Offset(StPoint point) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	x += point.x;
	y += point.y;
}

inline void CBofPoint::Offset(StSize size) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	x += size.cx;
	y += size.cy;
}

inline void CBofPoint::operator=(const StPoint &point) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	x = point.x;
	y = point.y;
}

inline void CBofPoint::operator=(const CBofPoint &point) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	x = point.x;
	y = point.y;
}

inline bool CBofPoint::operator==(StPoint point) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return (x == point.x && y == point.y);
}

inline bool CBofPoint::operator!=(StPoint point) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return (x != point.x || y != point.y);
}

inline void CBofPoint::operator+=(StSize size) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	x += size.cx;
	y += size.cy;
}

inline void CBofPoint::operator+=(StPoint point) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	x += point.x;
	y += point.y;
}

inline void CBofPoint::operator-=(StPoint point) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	x -= point.x;
	y -= point.y;
}

inline void CBofPoint::operator-=(StSize size) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	x -= size.cx;
	y -= size.cy;
}

inline CBofPoint CBofPoint::operator+(StSize size) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return CBofPoint(x + size.cx, y + size.cy);
}

inline CBofPoint CBofPoint::operator+(StPoint point) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return CBofPoint(x + point.x, y + point.y);
}

inline CBofPoint CBofPoint::operator-(StSize size) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return CBofPoint(x - size.cx, y - size.cy);
}

inline CBofPoint CBofPoint::operator-(StPoint point) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return CBofPoint(x - point.x, y - point.y);
}

inline CBofPoint CBofPoint::operator-() {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return CBofPoint(-x, -y);
}

#define CPoint CBofPoint

} // namespace Bagel

#endif
