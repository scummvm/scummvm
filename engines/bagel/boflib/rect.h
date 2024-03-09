
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

#ifndef BAGEL_BOFLIB_RECT_H
#define BAGEL_BOFLIB_RECT_H

#include "common/rect.h"
#include "bagel/boflib/boffo.h"
#include "bagel/boflib/stdinc.h"
#include "bagel/boflib/object.h"
#include "bagel/boflib/point.h"
#include "bagel/boflib/size.h"

namespace Bagel {

typedef Common::Rect RECT;

class CBofRect : public CBofObject {

public:
	// Constructors
	CBofRect();
	CBofRect(INT l, INT t, INT r, INT b);
	CBofRect(const CBofRect &srcRect);
	CBofRect(const CBofPoint &point, const CBofSize &size);
	CBofRect(const CBofPoint &ptTopLeft, const CBofPoint &ptBottomRight);

	// Attributes
	INT Width() const;
	INT Height() const;
	CBofSize Size() const;
	CBofPoint TopLeft() const;
	CBofPoint BottomRight() const;

	CBofPoint TopRight() const;
	CBofPoint BottomLeft() const;

	BOOL IsRectEmpty() const;
	BOOL IsRectNull() const;
	BOOL PtInRect(const CBofPoint &point) const;

	// Operations

	INLINE VOID SetRect(INT x1, INT y1, INT x2, INT y2);
	VOID SetRectEmpty();
	VOID CopyRect(const CBofRect &cRect);
	BOOL EqualRect(const CBofRect &cRect);

	CBofRect(const RECT &cRect) {
		SetWinRect(&cRect);
	}
	CBofRect(const RECT *pRect) {
		SetWinRect(pRect);
	}

	VOID SetWinRect(const RECT *pRect);
	RECT GetWinRect();

	VOID operator=(const RECT &srcRect) {
		SetWinRect(&srcRect);
	}

	operator const RECT() {
		return (GetWinRect());
	}

	VOID OffsetRect(INT x, INT y);
	VOID OffsetRect(const CBofSize &size);
	VOID OffsetRect(const CBofPoint &point);

	// operations that fill '*this' with result
	BOOL IntersectRect(const CBofRect *pRect1, const CBofRect *pRect2);
	BOOL IntersectRect(const CBofRect &cRect1, const CBofRect &cRect2);
	BOOL UnionRect(const CBofRect *pRect1, const CBofRect *pRect2);

	// Additional Operations
	VOID operator=(const CBofRect &srcRect);
	BOOL operator==(const CBofRect &rect);
	BOOL operator!=(const CBofRect &rect);
	VOID operator+=(const CBofPoint &point);
	VOID operator-=(const CBofPoint &point);
	VOID operator&=(const CBofRect &rect);
	VOID operator|=(const CBofRect &rect);

	// Operators returning CBofRect values
	CBofRect operator+(const CBofPoint &point);
	CBofRect operator-(const CBofPoint &point);
	CBofRect operator&(const CBofRect &rect2);
	CBofRect operator|(const CBofRect &rect2);

	INT left;
	INT top;
	INT right;
	INT bottom;
};

INLINE CBofRect::CBofRect() {
	top = left = 0;
	right = bottom = -1;
}

INLINE CBofRect::CBofRect(INT l, INT t, INT r, INT b) {
	left = l;
	top = t;
	right = r;
	bottom = b;
}

INLINE CBofRect::CBofRect(const CBofRect &srcRect) {
	left = srcRect.left;
	top = srcRect.top;
	right = srcRect.right;
	bottom = srcRect.bottom;
}

INLINE CBofRect::CBofRect(const CBofPoint &point, const CBofSize &size) {
	right = (left = point.x) + size.cx - 1;
	bottom = (top = point.y) + size.cy - 1;
}

INLINE CBofRect::CBofRect(const CBofPoint &ptTopLeft, const CBofPoint &ptBottomRight) {
	SetRect(ptTopLeft.x, ptTopLeft.y, ptBottomRight.x, ptBottomRight.y);
}

INLINE INT CBofRect::Width() const {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return (right - left + 1);
}

INLINE INT CBofRect::Height() const {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return (bottom - top + 1);
}

INLINE CBofSize CBofRect::Size() const {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return (CBofSize(Width(), Height()));
}

INLINE CBofPoint CBofRect::TopLeft() const {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return (CBofPoint(left, top));
}

INLINE CBofPoint CBofRect::BottomRight() const {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return (CBofPoint(right, bottom));
}

INLINE CBofPoint CBofRect::TopRight() const {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return (CBofPoint(right, top));
}

INLINE CBofPoint CBofRect::BottomLeft() const {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return (CBofPoint(left, bottom));
}

INLINE BOOL CBofRect::IsRectEmpty() const {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return (Width() <= 0 || Height() <= 0);
}

INLINE BOOL CBofRect::IsRectNull() const {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return (left == 0 && right == 0 && top == 0 && bottom == 0);
}

INLINE BOOL CBofRect::PtInRect(const CBofPoint &point) const {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return (point.x >= left && point.x <= right && point.y >= top && point.y <= bottom);
}

INLINE VOID CBofRect::SetRect(INT x1, INT y1, INT x2, INT y2) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	left = x1;
	top = y1;
	right = x2;
	bottom = y2;
}

INLINE VOID CBofRect::SetRectEmpty() {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	left = top = 0;
	right = bottom = -1;

	// verify that the rectangle is now empty
	Assert(IsRectEmpty());
}

INLINE VOID CBofRect::CopyRect(const CBofRect &cSrcRect) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	Assert(IsValidObject(&cSrcRect));

	left = cSrcRect.left;
	right = cSrcRect.right;
	top = cSrcRect.top;
	bottom = cSrcRect.bottom;
}

INLINE BOOL CBofRect::EqualRect(const CBofRect &cRect) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	Assert(IsValidObject(&cRect));

	return (left == cRect.left && right == cRect.right && top == cRect.top && bottom == cRect.bottom);
}

INLINE RECT CBofRect::GetWinRect() {
	Assert(IsValidObject(this));

	RECT stRect;

	stRect.left = left;
	stRect.top = top;
	stRect.right = right + 1;
	stRect.bottom = bottom + 1;

	return (stRect);
}

INLINE VOID CBofRect::SetWinRect(const RECT *pRect) {
	Assert(IsValidObject(this));

	Assert(pRect != nullptr);

	left = pRect->left;
	top = pRect->top;
	right = pRect->right - 1;
	bottom = pRect->bottom - 1;
}

INLINE VOID CBofRect::OffsetRect(INT x, INT y) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	left += x;
	right += x;
	top += y;
	bottom += y;
}

INLINE VOID CBofRect::OffsetRect(const CBofPoint &point) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	left += point.x;
	right += point.x;
	top += point.y;
	bottom += point.y;
}

INLINE VOID CBofRect::OffsetRect(const CBofSize &size) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	left += size.cx;
	right += size.cx;
	top += size.cy;
	bottom += size.cy;
}

INLINE BOOL CBofRect::IntersectRect(const CBofRect *pRect1, const CBofRect *pRect2) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	// can't access null pointers
	Assert(pRect1 != nullptr);
	Assert(pRect2 != nullptr);

	left = max(pRect1->left, pRect2->left);
	top = max(pRect1->top, pRect2->top);
	right = min(pRect1->right, pRect2->right);
	bottom = min(pRect1->bottom, pRect2->bottom);

	return ((Width() > 0) && (Height() > 0));
}

INLINE BOOL CBofRect::IntersectRect(const CBofRect &cRect1, const CBofRect &cRect2) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return (IntersectRect(&cRect1, &cRect2));
}

INLINE BOOL CBofRect::UnionRect(const CBofRect *pRect1, const CBofRect *pRect2) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	// can't access null pointers
	Assert(pRect1 != nullptr);
	Assert(pRect2 != nullptr);

	left = min(pRect1->left, pRect2->left);
	top = min(pRect1->top, pRect2->top);
	right = max(pRect1->right, pRect2->right);
	bottom = max(pRect1->bottom, pRect2->bottom);

	return (IsRectEmpty());
}

INLINE VOID CBofRect::operator=(const CBofRect &srcRect) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	left = srcRect.left;
	right = srcRect.right;
	top = srcRect.top;
	bottom = srcRect.bottom;
}

INLINE BOOL CBofRect::operator==(const CBofRect &rect) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return (left == rect.left && right == rect.right &&
	        top == rect.top && bottom == rect.bottom);
}

INLINE BOOL CBofRect::operator!=(const CBofRect &rect) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return (left != rect.left || right != rect.right ||
	        top != rect.top || bottom != rect.bottom);
}

INLINE VOID CBofRect::operator+=(const CBofPoint &point) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	OffsetRect(point);
}

INLINE VOID CBofRect::operator-=(const CBofPoint &point) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	OffsetRect(-point.x, -point.y);
}

INLINE VOID CBofRect::operator&=(const CBofRect &rect) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	IntersectRect(this, &rect);
}

INLINE VOID CBofRect::operator|=(const CBofRect &rect) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	UnionRect(this, &rect);
}

INLINE CBofRect CBofRect::operator+(const CBofPoint &pt) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	CBofRect rect(*this);

	rect.OffsetRect(pt.x, pt.y);

	return (rect);
}

INLINE CBofRect CBofRect::operator-(const CBofPoint &pt) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	CBofRect rect(*this);

	rect.OffsetRect(-pt.x, -pt.y);

	return (rect);
}

INLINE CBofRect CBofRect::operator&(const CBofRect &rect2) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	CBofRect rect;

	rect.IntersectRect(this, &rect2);

	return (rect);
}

INLINE CBofRect CBofRect::operator|(const CBofRect &rect2) {
	// make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	CBofRect rect;

	rect.UnionRect(this, &rect2);

	return (rect);
}

#define CRect CBofRect

} // namespace Bagel

#endif
