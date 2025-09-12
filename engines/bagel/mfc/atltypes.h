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

#ifndef BAGEL_MFC_ATLTYPES_H
#define BAGEL_MFC_ATLTYPES_H

#include "bagel/mfc/minwindef.h"

namespace Bagel {
namespace MFC {

/////////////////////////////////////////////////////////////////////////////
// Classes declared in this file

class CSize;
class CPoint;
class CRect;

/////////////////////////////////////////////////////////////////////////////
// CSize - An extent, similar to Windows SIZE structure.

class CSize : public tagSIZE {
public:

	// Constructors
	// construct an uninitialized size
	CSize();
	// create from two integers
	CSize(int initCX, int initCY);
	// create from another size
	CSize(SIZE initSize);
	// create from a point
	CSize(POINT initPt);
	// create from a uint32: cx = LOWORD(dw) cy = HIWORD(dw)
	CSize(uint32 dwSize);

	// Operations
	bool operator==(SIZE size) const;
	bool operator!=(SIZE size) const;
	void operator+=(SIZE size);
	void operator-=(SIZE size);
	void SetSize(int CX, int CY);

	// Operators returning CSize values
	CSize operator+(SIZE size) const;
	CSize operator-(SIZE size) const;
	CSize operator-() const;

	// Operators returning CPoint values
	CPoint operator+(POINT point) const;
	CPoint operator-(POINT point) const;

	// Operators returning CRect values
	CRect operator+(const RECT *lpRect) const;
	CRect operator-(const RECT *lpRect) const;
};

/////////////////////////////////////////////////////////////////////////////
// CPoint - A 2-D point, similar to Windows POINT structure.

class CPoint : public tagPOINT {
public:
	// Constructors

	// create an uninitialized point
	CPoint();
	// create from two integers
	CPoint(
	    int initX,
	    int initY);
	// create from another point
	CPoint(POINT initPt);
	// create from a size
	CPoint(SIZE initSize);
	// create from an LPARAM: x = LOWORD(dw) y = HIWORD(dw)
	CPoint(LPARAM dwPoint);


	// Operations

	// translate the point
	void Offset(
	    int xOffset,
	    int yOffset);
	void Offset(POINT point);
	void Offset(SIZE size);
	void SetPoint(int X, int Y);

	bool operator==(POINT point) const;
	bool operator!=(POINT point) const;
	void operator+=(SIZE size);
	void operator-=(SIZE size);
	void operator+=(POINT point);
	void operator-=(POINT point);

	// Operators returning CPoint values
	CPoint operator+(SIZE size) const;
	CPoint operator-(SIZE size) const;
	CPoint operator-() const;
	CPoint operator+(POINT point) const;

	// Operators returning CSize values
	CSize operator-(POINT point) const;

	// Operators returning CRect values
	CRect operator+(const RECT *lpRect) const;
	CRect operator-(const RECT *lpRect) const;
};

/////////////////////////////////////////////////////////////////////////////
// CRect - A 2-D rectangle, similar to Windows RECT structure.

class CRect : public tagRECT {
public:
	// uninitialized rectangle
	CRect();
	// from left, top, right, and bottom
	CRect(
	    int l,
	    int t,
	    int r,
	    int b);
	// copy constructor
	CRect(const RECT &srcRect);

	// from a pointer to another rect
	CRect(const LPCRECT lpSrcRect);
	// from a point and size
	CRect(const POINT &point, const SIZE &size);
	// from two points
	CRect(const POINT &topLeft, const POINT &bottomRight);

	// Attributes (in addition to RECT members)

	// retrieves the width
	int Width() const;
	// returns the height
	int Height() const;
	// returns the size
	CSize Size() const;
	// the geometric center point of the rectangle
	CPoint CenterPoint() const;
	// swap the left and right
	void SwapLeftRight();
	static void WINAPI SwapLeftRight(LPRECT lpRect);

	// convert between CRect and LPRECT/LPCRECT (no need for &)
	operator LPRECT();
	operator LPCRECT() const;

	// returns true if rectangle has no area
	bool IsRectEmpty() const;
	// returns true if rectangle is at (0,0) and has no area
	bool IsRectNull() const;
	// returns true if point is within rectangle
	bool PtInRect(POINT point) const;

	// Operations

	// set rectangle from left, top, right, and bottom
	void SetRect(
	    int x1,
	    int y1,
	    int x2,
	    int y2);
	void SetRect(
	    POINT topLeft,
	    POINT bottomRight);
	// empty the rectangle
	void SetRectEmpty();
	// copy from another rectangle
	void CopyRect(LPCRECT lpSrcRect);
	// true if exactly the same as another rectangle
	bool EqualRect(LPCRECT lpRect) const;

	// Inflate rectangle's width and height by
	// x units to the left and right ends of the rectangle
	// and y units to the top and bottom.
	void InflateRect(
	    int x,
	    int y);
	// Inflate rectangle's width and height by
	// size.cx units to the left and right ends of the rectangle
	// and size.cy units to the top and bottom.
	void InflateRect(SIZE size);
	// Inflate rectangle's width and height by moving individual sides.
	// Left side is moved to the left, right side is moved to the right,
	// top is moved up and bottom is moved down.
	void InflateRect(LPCRECT lpRect);
	void InflateRect(
	    int l,
	    int t,
	    int r,
	    int b);

	// deflate the rectangle's width and height without
	// moving its top or left
	void DeflateRect(
	    int x,
	    int y);
	void DeflateRect(SIZE size);
	void DeflateRect(LPCRECT lpRect);
	void DeflateRect(
	    int l,
	    int t,
	    int r,
	    int b);

	// translate the rectangle by moving its top and left
	void OffsetRect(
	    int x,
	    int y);
	void OffsetRect(SIZE size);
	void OffsetRect(POINT point);
	void NormalizeRect();

	// absolute position of rectangle
	void MoveToY(int y);
	void MoveToX(int x);
	void MoveToXY(
	    int x,
	    int y);
	void MoveToXY(POINT point);

	// set this rectangle to intersection of two others
	bool IntersectRect(
	    LPCRECT lpRect1,
	    LPCRECT lpRect2);

	// set this rectangle to bounding union of two others
	bool UnionRect(
	    LPCRECT lpRect1,
	    LPCRECT lpRect2);

	// set this rectangle to minimum of two others
	bool SubtractRect(
	    LPCRECT lpRectSrc1,
	    LPCRECT lpRectSrc2);

	// Additional Operations
	void operator=(const RECT &srcRect);
	bool operator==(const RECT &rect) const;
	bool operator!=(const RECT &rect) const;
	void operator+=(POINT point);
	void operator+=(SIZE size);
	void operator+=(LPCRECT lpRect);
	void operator-=(POINT point);
	void operator-=(SIZE size);
	void operator-=(LPCRECT lpRect);
	void operator&=(const RECT &rect);
	void operator|=(const RECT &rect);

	// Operators returning CRect values
	CRect operator+(POINT point) const;
	CRect operator-(POINT point) const;
	CRect operator+(LPCRECT lpRect) const;
	CRect operator+(SIZE size) const;
	CRect operator-(SIZE size) const;
	CRect operator-(LPCRECT lpRect) const;
	CRect operator&(const RECT &rect2) const;
	CRect operator|(const RECT &rect2) const;
	CRect MulDiv(
	    int nMultiplier,
	    int nDivisor) const;

	CPoint &TopLeft() {
		return *((CPoint *)this);
	}
	CPoint &BottomRight() {
		return *((CPoint *)this + 1);
	}
};

// CSize
inline CSize::CSize() {
	cx = 0;
	cy = 0;
}

inline CSize::CSize(
    int initCX,
    int initCY) {
	cx = initCX;
	cy = initCY;
}

inline CSize::CSize(SIZE initSize) {
	*(SIZE *)this = initSize;
}

inline CSize::CSize(POINT initPt) {
	*(POINT *)this = initPt;
}

inline CSize::CSize(uint32 dwSize) {
	cx = (short)LOWORD(dwSize);
	cy = (short)HIWORD(dwSize);
}

inline bool CSize::operator==(SIZE size) const {
	return (cx == size.cx && cy == size.cy);
}

inline bool CSize::operator!=(SIZE size) const {
	return (cx != size.cx || cy != size.cy);
}

inline void CSize::operator+=(SIZE size) {
	cx += size.cx;
	cy += size.cy;
}

inline void CSize::operator-=(SIZE size) {
	cx -= size.cx;
	cy -= size.cy;
}

inline void CSize::SetSize(
    int CX,
    int CY) {
	cx = CX;
	cy = CY;
}

inline CSize CSize::operator+(SIZE size) const {
	return CSize(cx + size.cx, cy + size.cy);
}

inline CSize CSize::operator-(SIZE size) const {
	return CSize(cx - size.cx, cy - size.cy);
}

inline CSize CSize::operator-() const {
	return CSize(-cx, -cy);
}

inline CPoint CSize::operator+(POINT point) const {
	return CPoint(cx + point.x, cy + point.y);
}

inline CPoint CSize::operator-(POINT point) const {
	return CPoint(cx - point.x, cy - point.y);
}

inline CRect CSize::operator+(const RECT *lpRect) const {
	return CRect(lpRect) + *this;
}

inline CRect CSize::operator-(const RECT *lpRect) const {
	return CRect(lpRect) - *this;
}

// CPoint
inline CPoint::CPoint() {
	x = 0;
	y = 0;
}

inline CPoint::CPoint(
    int initX,
    int initY) {
	x = initX;
	y = initY;
}

inline CPoint::CPoint(POINT initPt) {
	*(POINT *)this = initPt;
}

inline CPoint::CPoint(SIZE initSize) {
	*(SIZE *)this = initSize;
}

inline CPoint::CPoint(LPARAM dwPoint) {
	x = (short)LOWORD(dwPoint);
	y = (short)HIWORD(dwPoint);
}

inline void CPoint::Offset(
    int xOffset,
    int yOffset) {
	x += xOffset;
	y += yOffset;
}

inline void CPoint::Offset(POINT point) {
	x += point.x;
	y += point.y;
}

inline void CPoint::Offset(SIZE size) {
	x += size.cx;
	y += size.cy;
}

inline void CPoint::SetPoint(
    int X,
    int Y) {
	x = X;
	y = Y;
}

inline bool CPoint::operator==(POINT point) const {
	return (x == point.x && y == point.y);
}

inline bool CPoint::operator!=(POINT point) const {
	return (x != point.x || y != point.y);
}

inline void CPoint::operator+=(SIZE size) {
	x += size.cx;
	y += size.cy;
}

inline void CPoint::operator-=(SIZE size) {
	x -= size.cx;
	y -= size.cy;
}

inline void CPoint::operator+=(POINT point) {
	x += point.x;
	y += point.y;
}

inline void CPoint::operator-=(POINT point) {
	x -= point.x;
	y -= point.y;
}

inline CPoint CPoint::operator+(SIZE size) const {
	return CPoint(x + size.cx, y + size.cy);
}

inline CPoint CPoint::operator-(SIZE size) const {
	return CPoint(x - size.cx, y - size.cy);
}

inline CPoint CPoint::operator-() const {
	return CPoint(-x, -y);
}

inline CPoint CPoint::operator+(POINT point) const {
	return CPoint(x + point.x, y + point.y);
}

inline CSize CPoint::operator-(POINT point) const {
	return CSize(x - point.x, y - point.y);
}

inline CRect CPoint::operator+(const RECT *lpRect) const {
	return CRect(lpRect) + *this;
}

inline CRect CPoint::operator-(const RECT *lpRect) const {
	return CRect(lpRect) - *this;
}

// CRect
inline CRect::CRect() {
	left = 0;
	top = 0;
	right = 0;
	bottom = 0;
}

inline CRect::CRect(
    int l,
    int t,
    int r,
    int b) {
	left = l;
	top = t;
	right = r;
	bottom = b;
}

inline CRect::CRect(const RECT &srcRect) {
	CopyRect(&srcRect);
}

inline CRect::CRect(LPCRECT lpSrcRect) {
	CopyRect(lpSrcRect);
}

inline CRect::CRect(const POINT &point, const SIZE &size) {
	right = (left = point.x) + size.cx;
	bottom = (top = point.y) + size.cy;
}

inline CRect::CRect(const POINT &topLeft, const POINT &bottomRight) {
	left = topLeft.x;
	top = topLeft.y;
	right = bottomRight.x;
	bottom = bottomRight.y;
}

inline int CRect::Width() const {
	return right - left;
}

inline int CRect::Height() const {
	return bottom - top;
}

inline CSize CRect::Size() const {
	return CSize(right - left, bottom - top);
}

inline CPoint CRect::CenterPoint() const {
	return CPoint((left + right) / 2, (top + bottom) / 2);
}

inline void CRect::SwapLeftRight() {
	SwapLeftRight(LPRECT(this));
}

inline void WINAPI CRect::SwapLeftRight(LPRECT lpRect) {
	long temp = lpRect->left;
	lpRect->left = lpRect->right;
	lpRect->right = temp;
}

inline CRect::operator LPRECT() {
	return this;
}

inline CRect::operator LPCRECT() const {
	return this;
}

inline bool CRect::IsRectEmpty() const {
	return (left >= right) && (top >= bottom);
}

inline bool CRect::IsRectNull() const {
	return (left == 0 && right == 0 && top == 0 && bottom == 0);
}

inline bool CRect::PtInRect(POINT point) const {
	return (point.x >= left) && (point.x < right) &&
	       (point.y >= top) && (point.y < bottom);
}

inline void CRect::SetRect(int x1, int y1,
                           int x2, int y2) {
	left = x1;
	top = y1;
	right = x2;
	bottom = y2;
}

inline void CRect::SetRect(
    POINT topLeft,
    POINT bottomRight) {
	left = topLeft.x;
	top = topLeft.y;
	right = bottomRight.x;
	bottom = bottomRight.y;
}

inline void CRect::SetRectEmpty() {
	left = top = right = bottom = 0;
}

inline void CRect::CopyRect(LPCRECT lpSrcRect) {
	left = lpSrcRect->left;
	top = lpSrcRect->top;
	right = lpSrcRect->right;
	bottom = lpSrcRect->bottom;
}

inline bool CRect::EqualRect(LPCRECT lpRect) const {
	return left == lpRect->left &&
	       top == lpRect->top &&
	       right == lpRect->right &&
	       bottom == lpRect->bottom;
}

inline void CRect::InflateRect(
    int x, int y) {
	left -= x;
	top -= y;
	right += x;
	bottom += y;
}

inline void CRect::InflateRect(SIZE size) {
	left -= size.cx;
	top -= size.cy;
	right += size.cx;
	bottom += size.cy;
}

inline void CRect::DeflateRect(
    int x, int y) {
	InflateRect(-x, -y);
}

inline void CRect::DeflateRect(SIZE size) {
	InflateRect(-size.cx, -size.cy);
}

inline void CRect::OffsetRect(
    int x, int y) {
	left += x;
	top += y;
	right += x;
	bottom += y;
}

inline void CRect::OffsetRect(POINT point) {
	left += point.x;
	top += point.y;
	right += point.x;
	bottom += point.y;
}

inline void CRect::OffsetRect(SIZE size) {
	OffsetRect(size.cx, size.cy);
}

inline void CRect::MoveToY(int y) {
	bottom = Height() + y;
	top = y;
}

inline void CRect::MoveToX(int x) {
	right = Width() + x;
	left = x;
}

inline void CRect::MoveToXY(
    int x,
    int y) {
	MoveToX(x);
	MoveToY(y);
}

inline void CRect::MoveToXY(POINT pt) {
	MoveToX(pt.x);
	MoveToY(pt.y);
}

inline bool CRect::IntersectRect(
    LPCRECT lpRect1, LPCRECT lpRect2) {
	return (lpRect1->left < lpRect2->right) &&
	       (lpRect2->left < lpRect1->right) &&
	       (lpRect1->top < lpRect2->bottom) &&
	       (lpRect2->top < lpRect1->bottom);
}

inline bool CRect::UnionRect(LPCRECT lpRect1, LPCRECT lpRect2) {
	if (!lpRect1 || !lpRect2) {
		// Defensive: null input treated as empty
		SetRect(0, 0, 0, 0);
		return false;
	}

	// Check for empty rects
	bool empty1 = lpRect1->left >= lpRect1->right || lpRect1->top >= lpRect1->bottom;
	bool empty2 = lpRect2->left >= lpRect2->right || lpRect2->top >= lpRect2->bottom;

	if (empty1 && empty2) {
		SetRect(0, 0, 0, 0);
		return false;
	} else if (empty1) {
		*this = *lpRect2;
		return true;
	} else if (empty2) {
		*this = *lpRect1;
		return true;
	}

	// Compute union of two valid rects
	left = MIN(lpRect1->left, lpRect2->left);
	top = MIN(lpRect1->top, lpRect2->top);
	right = MAX(lpRect1->right, lpRect2->right);
	bottom = MAX(lpRect1->bottom, lpRect2->bottom);

	return true;
}

inline void CRect::operator=(const RECT &srcRect) {
	CopyRect(&srcRect);
}

inline bool CRect::operator==(const RECT &rect) const {
	return EqualRect(&rect);
}

inline bool CRect::operator!=(const RECT &rect) const {
	return !EqualRect(&rect);
}

inline void CRect::operator+=(POINT point) {
	OffsetRect(point.x, point.y);
}

inline void CRect::operator+=(SIZE size) {
	OffsetRect(size.cx, size.cy);
}

inline void CRect::operator+=(LPCRECT lpRect) {
	InflateRect(lpRect);
}

inline void CRect::operator-=(POINT point) {
	OffsetRect(-point.x, -point.y);
}

inline void CRect::operator-=(SIZE size) {
	OffsetRect(-size.cx, -size.cy);
}

inline void CRect::operator-=(LPCRECT lpRect) {
	DeflateRect(lpRect);
}

inline void CRect::operator&=(const RECT &rect) {
	IntersectRect(this, &rect);
}

inline void CRect::operator|=(const RECT &rect) {
	UnionRect(this, &rect);
}

inline CRect CRect::operator+(POINT pt) const {
	CRect rect(*this);
	rect.OffsetRect(pt.x, pt.y);
	return rect;
}

inline CRect CRect::operator-(POINT pt) const {
	CRect rect(*this);
	rect.OffsetRect(-pt.x, -pt.y);
	return rect;
}

inline CRect CRect::operator+(SIZE size) const {
	CRect rect(*this);
	rect.OffsetRect(size.cx, size.cy);
	return rect;
}

inline CRect CRect::operator-(SIZE size) const {
	CRect rect(*this);
	rect.OffsetRect(-size.cx, -size.cy);
	return rect;
}

inline CRect CRect::operator+(LPCRECT lpRect) const {
	CRect rect(this);
	rect.InflateRect(lpRect);
	return rect;
}

inline CRect CRect::operator-(LPCRECT lpRect) const {
	CRect rect(this);
	rect.DeflateRect(lpRect);
	return rect;
}

inline CRect CRect::operator&(const RECT &rect2) const {
	CRect rect;
	rect.IntersectRect(this, &rect2);
	return rect;
}

inline CRect CRect::operator|(const RECT &rect2) const {
	CRect rect;
	rect.UnionRect(this, &rect2);
	return rect;
}

inline bool CRect::SubtractRect(
    LPCRECT lpRectSrc1, LPCRECT lpRectSrc2) {
	// Calculate the intersection of the two rectangles
	CRect intersect;
	if (!intersect.IntersectRect(lpRectSrc1, lpRectSrc2)) {
		// No overlap - return full original
		*this = *lpRectSrc1;
		return true;
	}

	// If lpRectSrc2 fully covers lpRectSrc1, result is empty
	if (intersect == *lpRectSrc1) {
		SetRectEmpty();
		return false;
	}

	// Try to return a remaining portion that is a single rectangle

	// Top strip
	if (intersect.top > lpRectSrc1->top &&
	        intersect.left <= lpRectSrc1->left &&
	        intersect.right >= lpRectSrc1->right) {
		SetRect(lpRectSrc1->left, lpRectSrc1->top,
		        lpRectSrc1->right, intersect.top);
		return true;
	}

	// Bottom strip
	if (intersect.bottom < lpRectSrc1->bottom &&
	        intersect.left <= lpRectSrc1->left &&
	        intersect.right >= lpRectSrc1->right) {
		SetRect(lpRectSrc1->left, intersect.bottom,
		        lpRectSrc1->right, lpRectSrc1->bottom);
		return true;
	}

	// Left strip
	if (intersect.left > lpRectSrc1->left &&
	        intersect.top <= lpRectSrc1->top &&
	        intersect.bottom >= lpRectSrc1->bottom) {
		SetRect(lpRectSrc1->left, lpRectSrc1->top,
		        intersect.left, lpRectSrc1->bottom);
		return true;
	}

	// Right strip
	if (intersect.right < lpRectSrc1->right &&
	        intersect.top <= lpRectSrc1->top &&
	        intersect.bottom >= lpRectSrc1->bottom) {
		SetRect(intersect.right, lpRectSrc1->top,
		        lpRectSrc1->right, lpRectSrc1->bottom);
		return true;
	}

	// If none of the simple cases apply, we can't represent the subtraction
	// as a single rectangle - result is undefined
	SetRectEmpty();
	return false;
}

inline void CRect::NormalizeRect() {
	int nTemp;
	if (left > right) {
		nTemp = left;
		left = right;
		right = nTemp;
	}
	if (top > bottom) {
		nTemp = top;
		top = bottom;
		bottom = nTemp;
	}
}

inline void CRect::InflateRect(LPCRECT lpRect) {
	left -= lpRect->left;
	top -= lpRect->top;
	right += lpRect->right;
	bottom += lpRect->bottom;
}

inline void CRect::InflateRect(
    int l,
    int t,
    int r,
    int b) {
	left -= l;
	top -= t;
	right += r;
	bottom += b;
}

inline void CRect::DeflateRect(LPCRECT lpRect) {
	left += lpRect->left;
	top += lpRect->top;
	right -= lpRect->right;
	bottom -= lpRect->bottom;
}

inline void CRect::DeflateRect(
    int l,
    int t,
    int r,
    int b) {
	left += l;
	top += t;
	right -= r;
	bottom -= b;
}

inline int SafeMulDiv(int a, int b, int c) {
	if (c == 0)
		return 0; // Or handle divide-by-zero error

	// Use 64-bit intermediate result to avoid overflow
	int64 temp = (int64)a * (int64)b;

	// Round to nearest
	if ((temp >= 0))
		return (int)((temp + (c / 2)) / c);
	else
		return (int)((temp - (c / 2)) / c);
}

inline CRect CRect::MulDiv(
    int nMultiplier,
    int nDivisor) const {
	return CRect(
	           SafeMulDiv(left, nMultiplier, nDivisor),
	           SafeMulDiv(top, nMultiplier, nDivisor),
	           SafeMulDiv(right, nMultiplier, nDivisor),
	           SafeMulDiv(bottom, nMultiplier, nDivisor));
}

} // namespace MFC
} // namespace Bagel

#endif
