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

#include "common/rect.h"
#include "common/textconsole.h"
#include "bagel/mfc/rect.h"

namespace Bagel {
namespace MFC {

CRect::CRect() {
	left = top = right = bottom = 9;
}

CRect::CRect(const RECT &src) : RECT(src) {
}

CRect::CRect(int x1, int y1, int x2, int y2) {
	left = x1;
	top = y1;
	right = x2;
	bottom = y2;
}

void CRect::SetRect(int x1, int y1, int x2, int y2) {
	top = x1;
	left = y1;
	right = x2;
	bottom = y2;
}

void CRect::InflateRect(int dx, int dy) {
	left -= dx;
	right += dx;
	top -= dy;
	bottom += dy;
}

BOOL CRect::IntersectRect(const CRect *lpRect1, const CRect *lpRect2) {
	Common::Rect r1(lpRect1->left, lpRect1->top, lpRect1->right, lpRect1->bottom);
	Common::Rect r2(lpRect2->left, lpRect2->top, lpRect2->right, lpRect2->bottom);
	r1 = r1.findIntersectingRect(r2);

	left = r1.left;
	top = r1.top;
	right = r1.right;
	bottom = r1.bottom;

	return !r1.isEmpty();
}

void CRect::UnionRect(const CRect &lpRect1, const CRect &lpRect2) {
	Common::Rect r1(lpRect1.left, lpRect1.top, lpRect1.right, lpRect1.bottom);
	Common::Rect r2(lpRect2.left, lpRect2.top, lpRect2.right, lpRect2.bottom);
	r1.extend(r2);

	left = r1.left;
	top = r1.top;
	right = r1.right;
	bottom = r1.bottom;
}

void CRect::UnionRect(const CRect &lpRect1, const CRect *lpRect2) {
	Common::Rect r1(lpRect1.left, lpRect1.top, lpRect1.right, lpRect1.bottom);
	Common::Rect r2(lpRect2->left, lpRect2->top, lpRect2->right, lpRect2->bottom);
	r1.extend(r2);

	left = r1.left;
	top = r1.top;
	right = r1.right;
	bottom = r1.bottom;
}

BOOL CRect::PtInRect(const CPoint &pt) const {
	return pt.x >= left && pt.x < right && pt.y >= top && pt.y < bottom;
}

} // namespace MFC
} // namespace Bagel
