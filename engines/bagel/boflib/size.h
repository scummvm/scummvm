
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

#ifndef BAGEL_BOFLIB_SIZE_H
#define BAGEL_BOFLIB_SIZE_H

#include "bagel/boflib/stdinc.h"
#include "bagel/boflib/object.h"

namespace Bagel {

class CBofSize : public ST_SIZE, public CBofObject {
public:
	// Constructors
	CBofSize();
	CBofSize(INT initCX, INT initCY);
	CBofSize(const ST_SIZE &stSize);
	CBofSize(const CBofSize &cSize);
	CBofSize(ST_POINT stPoint);

	// Operations
	VOID operator=(const CBofSize &cSize);
	BOOL operator==(ST_SIZE size);
	BOOL operator!=(ST_SIZE size);
	VOID operator+=(ST_SIZE size);
	VOID operator-=(ST_SIZE size);

	// Operators returning CBofSize values
	CBofSize operator+(ST_SIZE size);
	CBofSize operator-(ST_SIZE size);
	CBofSize operator-();
};

// CBofSize
inline CBofSize::CBofSize() {
	cx = cy = 0;
}

inline CBofSize::CBofSize(INT initCX, INT initCY) {
	cx = initCX;
	cy = initCY;
}

inline CBofSize::CBofSize(const ST_SIZE &stSize) {
	cx = stSize.cx;
	cy = stSize.cy;
}

inline CBofSize::CBofSize(const CBofSize &cSize) {
	cx = cSize.cx;
	cy = cSize.cy;
}

inline CBofSize::CBofSize(ST_POINT stPoint) {
	cx = stPoint.x;
	cy = stPoint.y;
}

inline VOID CBofSize::operator=(const CBofSize &cSize) {
	cx = cSize.cx;
	cy = cSize.cy;
}

inline BOOL CBofSize::operator==(ST_SIZE size) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return (cx == size.cx && cy == size.cy);
}

inline BOOL CBofSize::operator!=(ST_SIZE size) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return (cx != size.cx || cy != size.cy);
}

inline VOID CBofSize::operator+=(ST_SIZE size) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	cx += size.cx;
	cy += size.cy;
}

inline VOID CBofSize::operator-=(ST_SIZE size) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	cx -= size.cx;
	cy -= size.cy;
}

inline CBofSize CBofSize::operator+(ST_SIZE size) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return CBofSize(cx + size.cx, cy + size.cy);
}

inline CBofSize CBofSize::operator-(ST_SIZE size) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return CBofSize(cx - size.cx, cy - size.cy);
}

inline CBofSize CBofSize::operator-() {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return CBofSize(-cx, -cy);
}

#define CSize CBofSize

} // namespace Bagel

#endif
