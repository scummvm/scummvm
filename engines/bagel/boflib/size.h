
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

#include "bagel/afxwin.h"
#include "bagel/boflib/stdinc.h"
#include "bagel/boflib/object.h"

namespace Bagel {

class CBofSize : public SIZE, public CBofObject {
public:
	// Constructors
	CBofSize();
	virtual ~CBofSize() {
	}
	CBofSize(int initCX, int initCY);
	CBofSize(const SIZE &stSize);
	CBofSize(const CBofSize &cSize);
	CBofSize(StPoint stPoint);

	// Operations
	void operator=(const CBofSize &cSize);
	bool operator==(SIZE size);
	bool operator!=(SIZE size);
	void operator+=(SIZE size);
	void operator-=(SIZE size);

	// Operators returning CBofSize values
	CBofSize operator+(SIZE size);
	CBofSize operator-(SIZE size);
	CBofSize operator-();
};

// CBofSize
inline CBofSize::CBofSize() {
	cx = cy = 0;
}

inline CBofSize::CBofSize(int initCX, int initCY) {
	cx = initCX;
	cy = initCY;
}

inline CBofSize::CBofSize(const SIZE &stSize) {
	cx = stSize.cx;
	cy = stSize.cy;
}

inline CBofSize::CBofSize(const CBofSize &cSize) {
	cx = cSize.cx;
	cy = cSize.cy;
}

inline CBofSize::CBofSize(StPoint stPoint) {
	cx = stPoint.x;
	cy = stPoint.y;
}

inline void CBofSize::operator=(const CBofSize &cSize) {
	cx = cSize.cx;
	cy = cSize.cy;
}

inline bool CBofSize::operator==(SIZE size) {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	return (cx == size.cx && cy == size.cy);
}

inline bool CBofSize::operator!=(SIZE size) {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	return (cx != size.cx || cy != size.cy);
}

inline void CBofSize::operator+=(SIZE size) {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	cx += size.cx;
	cy += size.cy;
}

inline void CBofSize::operator-=(SIZE size) {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	cx -= size.cx;
	cy -= size.cy;
}

inline CBofSize CBofSize::operator+(SIZE size) {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	return CBofSize(cx + size.cx, cy + size.cy);
}

inline CBofSize CBofSize::operator-(SIZE size) {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	return CBofSize(cx - size.cx, cy - size.cy);
}

inline CBofSize CBofSize::operator-() {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	return CBofSize(-cx, -cy);
}

} // namespace Bagel

#endif
