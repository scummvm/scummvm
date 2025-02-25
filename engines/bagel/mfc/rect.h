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

#include "common/rect.h"
#include "bagel/mfc/mfc_types.h"

namespace Bagel {
namespace MFC {

struct CRect : public Common::Rect {
public:
	CRect() : Common::Rect() {
	}
	CRect(const Common::Rect &src) : Common::Rect(src) {
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
typedef Common::Point CPoint;

} // namespace MFC
} // namespace Bagel

#endif
