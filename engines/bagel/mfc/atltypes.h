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
	CSize(
		_In_ int initCX,
		_In_ int initCY);
	// create from another size
	CSize(_In_ SIZE initSize);
	// create from a point
	CSize(_In_ POINT initPt);
	// create from a DWORD: cx = LOWORD(dw) cy = HIWORD(dw)
	CSize(_In_ DWORD dwSize);

	// Operations
	BOOL operator==(_In_ SIZE size) const;
	BOOL operator!=(_In_ SIZE size) const;
	void operator+=(_In_ SIZE size);
	void operator-=(_In_ SIZE size);
	void SetSize(_In_ int CX, _In_ int CY);

	// Operators returning CSize values
	CSize operator+(_In_ SIZE size) const;
	CSize operator-(_In_ SIZE size) const;
	CSize operator-() const;

	// Operators returning CPoint values
	CPoint operator+(_In_ POINT point) const;
	CPoint operator-(_In_ POINT point) const;

	// Operators returning CRect values
	CRect operator+(_In_ const RECT *lpRect) const;
	CRect operator-(_In_ const RECT *lpRect) const;
};

/////////////////////////////////////////////////////////////////////////////
// CPoint - A 2-D point, similar to Windows POINT structure.

class CPoint :
	public tagPOINT
{
public:
	// Constructors

		// create an uninitialized point
	CPoint();
	// create from two integers
	CPoint(
		_In_ int initX,
		_In_ int initY);
	// create from another point
	CPoint(_In_ POINT initPt);
	// create from a size
	CPoint(_In_ SIZE initSize);
	// create from an LPARAM: x = LOWORD(dw) y = HIWORD(dw)
	CPoint(_In_ LPARAM dwPoint);


	// Operations

	// translate the point
	void Offset(
		_In_ int xOffset,
		_In_ int yOffset);
	void Offset(_In_ POINT point);
	void Offset(_In_ SIZE size);
	void SetPoint(
		_In_ int X,
		_In_ int Y);

	BOOL operator==(_In_ POINT point) const;
	BOOL operator!=(_In_ POINT point) const;
	void operator+=(_In_ SIZE size);
	void operator-=(_In_ SIZE size);
	void operator+=(_In_ POINT point);
	void operator-=(_In_ POINT point);

	// Operators returning CPoint values
	CPoint operator+(_In_ SIZE size) const;
	CPoint operator-(_In_ SIZE size) const;
	CPoint operator-() const;
	CPoint operator+(_In_ POINT point) const;

	// Operators returning CSize values
	CSize operator-(_In_ POINT point) const;

	// Operators returning CRect values
	CRect operator+(_In_ const RECT *lpRect) const;
	CRect operator-(_In_ const RECT *lpRect) const;
};

/////////////////////////////////////////////////////////////////////////////
// CRect - A 2-D rectangle, similar to Windows RECT structure.

class CRect :
	public tagRECT
{
	// Constructors
public:
	// uninitialized rectangle
	CRect();
	// from left, top, right, and bottom
	CRect(
		_In_ int l,
		_In_ int t,
		_In_ int r,
		_In_ int b);
	// copy constructor
	CRect(_In_ const RECT &srcRect);

	// from a pointer to another rect
	CRect(_In_ LPCRECT lpSrcRect);
	// from a point and size
	CRect(
		_In_ POINT point,
		_In_ SIZE size);
	// from two points
	CRect(
		_In_ POINT topLeft,
		_In_ POINT bottomRight);

	// Attributes (in addition to RECT members)

		// retrieves the width
	int Width() const;
	// returns the height
	int Height() const;
	// returns the size
	CSize Size() const;
	// reference to the top-left point
	CPoint &TopLeft();
	// reference to the bottom-right point
	CPoint &BottomRight();
	// const reference to the top-left point
	const CPoint &TopLeft() const;
	// const reference to the bottom-right point
	const CPoint &BottomRight() const;
	// the geometric center point of the rectangle
	CPoint CenterPoint() const;
	// swap the left and right
	void SwapLeftRight();
	static void WINAPI SwapLeftRight(_Inout_ LPRECT lpRect);

	// convert between CRect and LPRECT/LPCRECT (no need for &)
	operator LPRECT();
	operator LPCRECT() const;

	// returns TRUE if rectangle has no area
	BOOL IsRectEmpty() const;
	// returns TRUE if rectangle is at (0,0) and has no area
	BOOL IsRectNull() const;
	// returns TRUE if point is within rectangle
	BOOL PtInRect(_In_ POINT point) const;

	// Operations

		// set rectangle from left, top, right, and bottom
	void SetRect(
		_In_ int x1,
		_In_ int y1,
		_In_ int x2,
		_In_ int y2);
	void SetRect(
		_In_ POINT topLeft,
		_In_ POINT bottomRight);
	// empty the rectangle
	void SetRectEmpty();
	// copy from another rectangle
	void CopyRect(_In_ LPCRECT lpSrcRect);
	// TRUE if exactly the same as another rectangle
	BOOL EqualRect(_In_ LPCRECT lpRect) const;

	// Inflate rectangle's width and height by
	// x units to the left and right ends of the rectangle
	// and y units to the top and bottom.
	void InflateRect(
		_In_ int x,
		_In_ int y);
	// Inflate rectangle's width and height by
	// size.cx units to the left and right ends of the rectangle
	// and size.cy units to the top and bottom.
	void InflateRect(_In_ SIZE size);
	// Inflate rectangle's width and height by moving individual sides.
	// Left side is moved to the left, right side is moved to the right,
	// top is moved up and bottom is moved down.
	void InflateRect(_In_ LPCRECT lpRect);
	void InflateRect(
		_In_ int l,
		_In_ int t,
		_In_ int r,
		_In_ int b);

	// deflate the rectangle's width and height without
	// moving its top or left
	void DeflateRect(
		_In_ int x,
		_In_ int y);
	void DeflateRect(_In_ SIZE size);
	void DeflateRect(_In_ LPCRECT lpRect);
	void DeflateRect(
		_In_ int l,
		_In_ int t,
		_In_ int r,
		_In_ int b);

	// translate the rectangle by moving its top and left
	void OffsetRect(
		_In_ int x,
		_In_ int y);
	void OffsetRect(_In_ SIZE size);
	void OffsetRect(_In_ POINT point);
	void NormalizeRect();

	// absolute position of rectangle
	void MoveToY(_In_ int y);
	void MoveToX(_In_ int x);
	void MoveToXY(
		_In_ int x,
		_In_ int y);
	void MoveToXY(_In_ POINT point);

	// set this rectangle to intersection of two others
	BOOL IntersectRect(
		_In_ LPCRECT lpRect1,
		_In_ LPCRECT lpRect2);

	// set this rectangle to bounding union of two others
	BOOL UnionRect(
		_In_ LPCRECT lpRect1,
		_In_ LPCRECT lpRect2);

	// set this rectangle to minimum of two others
	BOOL SubtractRect(
		_In_ LPCRECT lpRectSrc1,
		_In_ LPCRECT lpRectSrc2);

	// Additional Operations
	void operator=(_In_ const RECT &srcRect);
	BOOL operator==(_In_ const RECT &rect) const;
	BOOL operator!=(_In_ const RECT &rect) const;
	void operator+=(_In_ POINT point);
	void operator+=(_In_ SIZE size);
	void operator+=(_In_ LPCRECT lpRect);
	void operator-=(_In_ POINT point);
	void operator-=(_In_ SIZE size);
	void operator-=(_In_ LPCRECT lpRect);
	void operator&=(_In_ const RECT &rect);
	void operator|=(_In_ const RECT &rect);

	// Operators returning CRect values
	CRect operator+(_In_ POINT point) const;
	CRect operator-(_In_ POINT point) const;
	CRect operator+(_In_ LPCRECT lpRect) const;
	CRect operator+(_In_ SIZE size) const;
	CRect operator-(_In_ SIZE size) const;
	CRect operator-(_In_ LPCRECT lpRect) const;
	CRect operator&(_In_ const RECT &rect2) const;
	CRect operator|(_In_ const RECT &rect2) const;
	CRect MulDiv(
		_In_ int nMultiplier,
		_In_ int nDivisor) const;
};

#ifndef _ATL_STATIC_LIB_IMPL

// CSize
inline CSize::CSize() {
	cx = 0;
	cy = 0;
}

inline CSize::CSize(
	_In_ int initCX,
	_In_ int initCY) {
	cx = initCX;
	cy = initCY;
}

inline CSize::CSize(_In_ SIZE initSize) {
	*(SIZE *)this = initSize;
}

inline CSize::CSize(_In_ POINT initPt) {
	*(POINT *)this = initPt;
}

inline CSize::CSize(_In_ DWORD dwSize) {
	cx = (short)LOWORD(dwSize);
	cy = (short)HIWORD(dwSize);
}

inline BOOL CSize::operator==(_In_ SIZE size) const {
	return (cx == size.cx && cy == size.cy);
}

inline BOOL CSize::operator!=(_In_ SIZE size) const {
	return (cx != size.cx || cy != size.cy);
}

inline void CSize::operator+=(_In_ SIZE size) {
	cx += size.cx;
	cy += size.cy;
}

inline void CSize::operator-=(_In_ SIZE size) {
	cx -= size.cx;
	cy -= size.cy;
}

inline void CSize::SetSize(
	_In_ int CX,
	_In_ int CY) {
	cx = CX;
	cy = CY;
}

inline CSize CSize::operator+(_In_ SIZE size) const {
	return CSize(cx + size.cx, cy + size.cy);
}

inline CSize CSize::operator-(_In_ SIZE size) const {
	return CSize(cx - size.cx, cy - size.cy);
}

inline CSize CSize::operator-() const {
	return CSize(-cx, -cy);
}

inline CPoint CSize::operator+(_In_ POINT point) const {
	return CPoint(cx + point.x, cy + point.y);
}

inline CPoint CSize::operator-(_In_ POINT point) const {
	return CPoint(cx - point.x, cy - point.y);
}

inline CRect CSize::operator+(_In_ const RECT *lpRect) const {
	return CRect(lpRect) + *this;
}

inline CRect CSize::operator-(_In_ const RECT *lpRect) const {
	return CRect(lpRect) - *this;
}

// CPoint
inline CPoint::CPoint() {
	x = 0;
	y = 0;
}

inline CPoint::CPoint(
	_In_ int initX,
	_In_ int initY) {
	x = initX;
	y = initY;
}

inline CPoint::CPoint(_In_ POINT initPt) {
	*(POINT *)this = initPt;
}

inline CPoint::CPoint(_In_ SIZE initSize) {
	*(SIZE *)this = initSize;
}

inline CPoint::CPoint(_In_ LPARAM dwPoint) {
	x = (short)LOWORD(dwPoint);
	y = (short)HIWORD(dwPoint);
}

inline void CPoint::Offset(
	_In_ int xOffset,
	_In_ int yOffset) {
	x += xOffset;
	y += yOffset;
}

inline void CPoint::Offset(_In_ POINT point) {
	x += point.x;
	y += point.y;
}

inline void CPoint::Offset(_In_ SIZE size) {
	x += size.cx;
	y += size.cy;
}

inline void CPoint::SetPoint(
	_In_ int X,
	_In_ int Y) {
	x = X;
	y = Y;
}

inline BOOL CPoint::operator==(_In_ POINT point) const {
	return (x == point.x && y == point.y);
}

inline BOOL CPoint::operator!=(_In_ POINT point) const {
	return (x != point.x || y != point.y);
}

inline void CPoint::operator+=(_In_ SIZE size) {
	x += size.cx;
	y += size.cy;
}

inline void CPoint::operator-=(_In_ SIZE size) {
	x -= size.cx;
	y -= size.cy;
}

inline void CPoint::operator+=(_In_ POINT point) {
	x += point.x;
	y += point.y;
}

inline void CPoint::operator-=(_In_ POINT point) {
	x -= point.x;
	y -= point.y;
}

inline CPoint CPoint::operator+(_In_ SIZE size) const {
	return CPoint(x + size.cx, y + size.cy);
}

inline CPoint CPoint::operator-(_In_ SIZE size) const {
	return CPoint(x - size.cx, y - size.cy);
}

inline CPoint CPoint::operator-() const {
	return CPoint(-x, -y);
}

inline CPoint CPoint::operator+(_In_ POINT point) const {
	return CPoint(x + point.x, y + point.y);
}

inline CSize CPoint::operator-(_In_ POINT point) const {
	return CSize(x - point.x, y - point.y);
}

inline CRect CPoint::operator+(_In_ const RECT *lpRect) const {
	return CRect(lpRect) + *this;
}

inline CRect CPoint::operator-(_In_ const RECT *lpRect) const {
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
	_In_ int l,
	_In_ int t,
	_In_ int r,
	_In_ int b) {
	left = l;
	top = t;
	right = r;
	bottom = b;
}

inline CRect::CRect(_In_ const RECT &srcRect) {
	*this = srcRect;
}

inline CRect::CRect(_In_ LPCRECT lpSrcRect) {
	*this = *lpSrcRect;
}

inline CRect::CRect(
	_In_ POINT point,
	_In_ SIZE size) {
	right = (left = point.x) + size.cx;
	bottom = (top = point.y) + size.cy;
}

inline CRect::CRect(
	_In_ POINT topLeft,
	_In_ POINT bottomRight) {
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

inline CPoint &CRect::TopLeft() {
	return *((CPoint *)this);
}

inline CPoint &CRect::BottomRight() {
	return *((CPoint *)this + 1);
}

inline const CPoint &CRect::TopLeft() const {
	return *((CPoint *)this);
}

inline const CPoint &CRect::BottomRight() const {
	return *((CPoint *)this + 1);
}

inline CPoint CRect::CenterPoint() const {
	return CPoint((left + right) / 2, (top + bottom) / 2);
}

inline void CRect::SwapLeftRight() {
	SwapLeftRight(LPRECT(this));
}

inline void WINAPI CRect::SwapLeftRight(_Inout_ LPRECT lpRect) {
	LONG temp = lpRect->left;
	lpRect->left = lpRect->right;
	lpRect->right = temp;
}

inline CRect::operator LPRECT() {
	return this;
}

inline CRect::operator LPCRECT() const {
	return this;
}

inline BOOL CRect::IsRectEmpty() const {
	return (left >= right) && (top >= bottom);
}

inline BOOL CRect::IsRectNull() const {
	return (left == 0 && right == 0 && top == 0 && bottom == 0);
}

inline BOOL CRect::PtInRect(_In_ POINT point) const {
	return (point.x >= left) && (point.x < right) &&
		(point.y >= top) && (point.y < bottom);
}

inline void CRect::SetRect(_In_ int x1, _In_ int y1,
		_In_ int x2, _In_ int y2) {
	left = x1; top = y1;
	right = x2; bottom = y2;
}

inline void CRect::SetRect(
		_In_ POINT topLeft,
		_In_ POINT bottomRight) {
	left = topLeft.x; top = topLeft.y;
	right = bottomRight.x; bottom = bottomRight.y;
}

inline void CRect::SetRectEmpty() {
	left = top = right = bottom = 0;
}

inline void CRect::CopyRect(_In_ LPCRECT lpSrcRect) {
	*this = *lpSrcRect;
}

inline BOOL CRect::EqualRect(_In_ LPCRECT lpRect) const {
	return left == lpRect->left &&
		top == lpRect->top &&
		right == lpRect->right &&
		bottom == lpRect->bottom;
}

inline void CRect::InflateRect(
		_In_ int x, _In_ int y) {
	left -= x;
	top -= y;
	right += x;
	bottom += y;
}

inline void CRect::InflateRect(_In_ SIZE size) {
	left -= size.cx;
	top -= size.cy;
	right += size.cx;
	bottom += size.cy;
}

inline void CRect::DeflateRect(
		_In_ int x, _In_ int y) {
	InflateRect(-x, -y);
}

inline void CRect::DeflateRect(_In_ SIZE size) {
	InflateRect(-size.cx, -size.cy);
}

inline void CRect::OffsetRect(
		_In_ int x, _In_ int y) {
	left += x;
	top += y;
	right += x;
	bottom += y;
}

inline void CRect::OffsetRect(_In_ POINT point) {
	left += point.x;
	top += point.y;
	right += point.x;
	bottom += point.y;
}

inline void CRect::OffsetRect(_In_ SIZE size) {
	OffsetRect(size.cx, size.cy);
}

inline void CRect::MoveToY(_In_ int y) {
	bottom = Height() + y;
	top = y;
}

inline void CRect::MoveToX(_In_ int x) {
	right = Width() + x;
	left = x;
}

inline void CRect::MoveToXY(
	_In_ int x,
	_In_ int y) {
	MoveToX(x);
	MoveToY(y);
}

inline void CRect::MoveToXY(_In_ POINT pt) {
	MoveToX(pt.x);
	MoveToY(pt.y);
}

inline BOOL CRect::IntersectRect(
		_In_ LPCRECT lpRect1, _In_ LPCRECT lpRect2) {
	return (lpRect1->left < lpRect2->right) &&
		(lpRect2->left < lpRect1->right) &&
		(lpRect1->top < lpRect2->bottom) &&
		(lpRect2->top < lpRect1->bottom);
}

inline BOOL CRect::UnionRect(
		_In_ LPCRECT lpRect1, _In_ LPCRECT lpRect2) {
	if (lpRect1->right < lpRect2->left ||
		lpRect1->left > lpRect2->right ||
		lpRect1->bottom < lpRect2->top ||
		lpRect2->top > lpRect2->bottom) {
		left = top = right = bottom = 0;
		return false;
	} else {
		left = MAX(lpRect1->left, lpRect2->left);
		top = MAX(lpRect1->top, lpRect2->top);
		right = MIN(lpRect1->right, lpRect2->right);
		bottom = MIN(lpRect1->bottom, lpRect2->bottom);
		return true;
	}
}

inline void CRect::operator=(_In_ const RECT &srcRect) {
	CopyRect(&srcRect);
}

inline BOOL CRect::operator==(_In_ const RECT &rect) const {
	return EqualRect(&rect);
}

inline BOOL CRect::operator!=(_In_ const RECT &rect) const {
	return !EqualRect(&rect);
}

inline void CRect::operator+=(_In_ POINT point) {
	OffsetRect(point.x, point.y);
}

inline void CRect::operator+=(_In_ SIZE size) {
	OffsetRect(size.cx, size.cy);
}

inline void CRect::operator+=(_In_ LPCRECT lpRect) {
	InflateRect(lpRect);
}

inline void CRect::operator-=(_In_ POINT point) {
	OffsetRect(-point.x, -point.y);
}

inline void CRect::operator-=(_In_ SIZE size) {
	OffsetRect(-size.cx, -size.cy);
}

inline void CRect::operator-=(_In_ LPCRECT lpRect) {
	DeflateRect(lpRect);
}

inline void CRect::operator&=(_In_ const RECT &rect) {
	IntersectRect(this, &rect);
}

inline void CRect::operator|=(_In_ const RECT &rect) {
	UnionRect(this, &rect);
}

inline CRect CRect::operator+(_In_ POINT pt) const {
	CRect rect(*this);
	rect.OffsetRect(pt.x, pt.y);
	return rect;
}

inline CRect CRect::operator-(_In_ POINT pt) const {
	CRect rect(*this);
	rect.OffsetRect(-pt.x, -pt.y);
	return rect;
}

inline CRect CRect::operator+(_In_ SIZE size) const {
	CRect rect(*this);
	rect.OffsetRect(size.cx, size.cy);
	return rect;
}

inline CRect CRect::operator-(_In_ SIZE size) const {
	CRect rect(*this);
	rect.OffsetRect(-size.cx, -size.cy);
	return rect;
}

inline CRect CRect::operator+(_In_ LPCRECT lpRect) const {
	CRect rect(this);
	rect.InflateRect(lpRect);
	return rect;
}

inline CRect CRect::operator-(_In_ LPCRECT lpRect) const {
	CRect rect(this);
	rect.DeflateRect(lpRect);
	return rect;
}

inline CRect CRect::operator&(_In_ const RECT &rect2) const {
	CRect rect;
	rect.IntersectRect(this, &rect2);
	return rect;
}

inline CRect CRect::operator|(_In_ const RECT &rect2) const {
	CRect rect;
	rect.UnionRect(this, &rect2);
	return rect;
}

inline BOOL CRect::SubtractRect(
		_In_ LPCRECT lpRectSrc1, _In_ LPCRECT lpRectSrc2) {
	// Calculate the intersection of the two rectangles
	CRect intersect;
	if (!intersect.IntersectRect(lpRectSrc1, lpRectSrc2))
	{
		// No overlap - return full original
		*this = *lpRectSrc1;
		return TRUE;
	}

	// If lpRectSrc2 fully covers lpRectSrc1, result is empty
	if (intersect == *lpRectSrc1)
	{
		SetRectEmpty();
		return FALSE;
	}

	// Try to return a remaining portion that is a single rectangle

	// Top strip
	if (intersect.top > lpRectSrc1->top &&
		intersect.left <= lpRectSrc1->left &&
		intersect.right >= lpRectSrc1->right)
	{
		SetRect(lpRectSrc1->left, lpRectSrc1->top,
			lpRectSrc1->right, intersect.top);
		return TRUE;
	}

	// Bottom strip
	if (intersect.bottom < lpRectSrc1->bottom &&
		intersect.left <= lpRectSrc1->left &&
		intersect.right >= lpRectSrc1->right)
	{
		SetRect(lpRectSrc1->left, intersect.bottom,
			lpRectSrc1->right, lpRectSrc1->bottom);
		return TRUE;
	}

	// Left strip
	if (intersect.left > lpRectSrc1->left &&
		intersect.top <= lpRectSrc1->top &&
		intersect.bottom >= lpRectSrc1->bottom)
	{
		SetRect(lpRectSrc1->left, lpRectSrc1->top,
			intersect.left, lpRectSrc1->bottom);
		return TRUE;
	}

	// Right strip
	if (intersect.right < lpRectSrc1->right &&
		intersect.top <= lpRectSrc1->top &&
		intersect.bottom >= lpRectSrc1->bottom)
	{
		SetRect(intersect.right, lpRectSrc1->top,
			lpRectSrc1->right, lpRectSrc1->bottom);
		return TRUE;
	}

	// If none of the simple cases apply, we can't represent the subtraction
	// as a single rectangle - result is undefined
	SetRectEmpty();
	return FALSE;
}

inline void CRect::NormalizeRect() {
	int nTemp;
	if (left > right)
	{
		nTemp = left;
		left = right;
		right = nTemp;
	}
	if (top > bottom)
	{
		nTemp = top;
		top = bottom;
		bottom = nTemp;
	}
}

inline void CRect::InflateRect(_In_ LPCRECT lpRect) {
	left -= lpRect->left;
	top -= lpRect->top;
	right += lpRect->right;
	bottom += lpRect->bottom;
}

inline void CRect::InflateRect(
	_In_ int l,
	_In_ int t,
	_In_ int r,
	_In_ int b) {
	left -= l;
	top -= t;
	right += r;
	bottom += b;
}

inline void CRect::DeflateRect(_In_ LPCRECT lpRect) {
	left += lpRect->left;
	top += lpRect->top;
	right -= lpRect->right;
	bottom -= lpRect->bottom;
}

inline void CRect::DeflateRect(
	_In_ int l,
	_In_ int t,
	_In_ int r,
	_In_ int b) {
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
	_In_ int nMultiplier,
	_In_ int nDivisor) const {
	return CRect(
		SafeMulDiv(left, nMultiplier, nDivisor),
		SafeMulDiv(top, nMultiplier, nDivisor),
		SafeMulDiv(right, nMultiplier, nDivisor),
		SafeMulDiv(bottom, nMultiplier, nDivisor));
}

#endif // _ATL_STATIC_LIB_IMPL

} // namespace MFC
} // namespace Bagel

#endif
