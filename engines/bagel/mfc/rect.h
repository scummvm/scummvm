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

#ifndef BAGEL_MFC_RECT_H
#define BAGEL_MFC_RECT_H

#include "bagel/mfc/mfc_types.h"

namespace Bagel {
namespace MFC {

typedef Common::Point CPoint;

struct CRect : public Common::Rect {
public:
	CRect() : Common::Rect() {
	}
	CRect(const Common::Rect &src) : Common::Rect(src) {
	}
	CRect(int x1, int y1, int x2, int y2) : Common::Rect(x1, y1, x2, y2) {
	}

	void SetRect(int x1, int y1, int x2, int y2) {
		top = x1;
		left = y1;
		right = x2;
		bottom = y2;
	}
	void InflateRect(int dx, int dy) {
		left -= dx;
		right += dx;
		top -= dy;
		bottom += dy;
	}

	BOOL IntersectRect(const CRect *lpRect1, const CRect *lpRect2) {
		*this = lpRect1->findIntersectingRect(*lpRect2);
		return !isEmpty();
	}
	void UnionRect(const CRect &lpRect1, const CRect &lpRect2) {
		*this = lpRect1;
		this->extend(lpRect2);
	}
	void UnionRect(const CRect &lpRect1, const CRect *lpRect2) {
		*this = lpRect1;
		this->extend(*lpRect2);
	}
	BOOL PtInRect(const CPoint &pt) const {
		return contains(pt.x, pt.y);
	}
};

struct CSize : public Common::Point {
public:
	int16 &cx = Common::Point::x;
	int16 &cy = Common::Point::y;

	CSize() : Common::Point() {}
	CSize(int16 x1, int16 y1) : Common::Point(x1, y1) {}
	CSize(const CSize &src) : Common::Point(src.cx, src.cy) {}

	CSize &operator=(const CSize &rhs) {
		cx = rhs.cx;
		cy = rhs.cy;
		return *this;
	}
};

typedef CRect RECT;
typedef CRect *LPRECT;
typedef const CRect *LPCRECT;

} // namespace MFC
} // namespace Bagel

#endif
