
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
	CBofRect(int l, int t, int r, int b);
	CBofRect(const CBofRect &srcRect);
	CBofRect(const CBofPoint &point, const CBofSize &size);
	CBofRect(const CBofPoint &ptTopLeft, const CBofPoint &ptBottomRight);

	// Attributes
	int width() const;
	int height() const;
	CBofSize Size() const;
	CBofPoint TopLeft() const;
	CBofPoint BottomRight() const;

	CBofPoint TopRight() const;
	CBofPoint BottomLeft() const;

	bool IsRectEmpty() const;
	bool IsRectNull() const;
	bool PtInRect(const CBofPoint &point) const;

	// Operations

	inline void SetRect(int x1, int y1, int x2, int y2);
	void SetRectEmpty();
	void CopyRect(const CBofRect &cRect);
	bool EqualRect(const CBofRect &cRect);

	CBofRect(const RECT &cRect) {
		SetWinRect(&cRect);
	}
	CBofRect(const RECT *pRect) {
		SetWinRect(pRect);
	}

	void SetWinRect(const RECT *pRect);
	RECT GetWinRect();

	void operator=(const RECT &srcRect) {
		SetWinRect(&srcRect);
	}

	operator const RECT() {
		return GetWinRect();
	}

	void OffsetRect(int x, int y);
	void OffsetRect(const CBofSize &size);
	void OffsetRect(const CBofPoint &point);

	// Operations that fill '*this' with result
	bool IntersectRect(const CBofRect *pRect1, const CBofRect *pRect2);
	bool IntersectRect(const CBofRect &cRect1, const CBofRect &cRect2);
	bool UnionRect(const CBofRect *pRect1, const CBofRect *pRect2);

	// Additional Operations
	void operator=(const CBofRect &srcRect);
	bool operator==(const CBofRect &rect);
	bool operator!=(const CBofRect &rect);
	void operator+=(const CBofPoint &point);
	void operator-=(const CBofPoint &point);
	void operator&=(const CBofRect &rect);
	void operator|=(const CBofRect &rect);

	// Operators returning CBofRect values
	CBofRect operator+(const CBofPoint &point);
	CBofRect operator-(const CBofPoint &point);
	CBofRect operator&(const CBofRect &rect2);
	CBofRect operator|(const CBofRect &rect2);

	int left;
	int top;
	int right;
	int bottom;
};

inline CBofRect::CBofRect() {
	top = left = 0;
	right = bottom = -1;
}

inline CBofRect::CBofRect(int l, int t, int r, int b) {
	left = l;
	top = t;
	right = r;
	bottom = b;
}

inline CBofRect::CBofRect(const CBofRect &srcRect) {
	left = srcRect.left;
	top = srcRect.top;
	right = srcRect.right;
	bottom = srcRect.bottom;
}

inline CBofRect::CBofRect(const CBofPoint &point, const CBofSize &size) {
	right = (left = point.x) + size.cx - 1;
	bottom = (top = point.y) + size.cy - 1;
}

inline CBofRect::CBofRect(const CBofPoint &ptTopLeft, const CBofPoint &ptBottomRight) {
	SetRect(ptTopLeft.x, ptTopLeft.y, ptBottomRight.x, ptBottomRight.y);
}

inline int CBofRect::width() const {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return right - left + 1;
}

inline int CBofRect::height() const {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return bottom - top + 1;
}

inline CBofSize CBofRect::Size() const {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return CBofSize(width(), height());
}

inline CBofPoint CBofRect::TopLeft() const {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return CBofPoint(left, top);
}

inline CBofPoint CBofRect::BottomRight() const {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return CBofPoint(right, bottom);
}

inline CBofPoint CBofRect::TopRight() const {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return CBofPoint(right, top);
}

inline CBofPoint CBofRect::BottomLeft() const {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return CBofPoint(left, bottom);
}

inline bool CBofRect::IsRectEmpty() const {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return (width() <= 0 || height() <= 0);
}

inline bool CBofRect::IsRectNull() const {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return (left == 0 && right == 0 && top == 0 && bottom == 0);
}

inline bool CBofRect::PtInRect(const CBofPoint &point) const {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return (point.x >= left && point.x <= right && point.y >= top && point.y <= bottom);
}

inline void CBofRect::SetRect(int x1, int y1, int x2, int y2) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	left = x1;
	top = y1;
	right = x2;
	bottom = y2;
}

inline void CBofRect::SetRectEmpty() {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	left = top = 0;
	right = bottom = -1;

	// Verify that the rectangle is now empty
	Assert(IsRectEmpty());
}

inline void CBofRect::CopyRect(const CBofRect &cSrcRect) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	Assert(IsValidObject(&cSrcRect));

	left = cSrcRect.left;
	right = cSrcRect.right;
	top = cSrcRect.top;
	bottom = cSrcRect.bottom;
}

inline bool CBofRect::EqualRect(const CBofRect &cRect) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	Assert(IsValidObject(&cRect));

	return (left == cRect.left && right == cRect.right && top == cRect.top && bottom == cRect.bottom);
}

inline RECT CBofRect::GetWinRect() {
	Assert(IsValidObject(this));

	RECT stRect;

	stRect.left = left;
	stRect.top = top;
	stRect.right = right + 1;
	stRect.bottom = bottom + 1;

	return stRect;
}

inline void CBofRect::SetWinRect(const RECT *pRect) {
	Assert(IsValidObject(this));

	Assert(pRect != nullptr);

	left = pRect->left;
	top = pRect->top;
	right = pRect->right - 1;
	bottom = pRect->bottom - 1;
}

inline void CBofRect::OffsetRect(int x, int y) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	left += x;
	right += x;
	top += y;
	bottom += y;
}

inline void CBofRect::OffsetRect(const CBofPoint &point) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	left += point.x;
	right += point.x;
	top += point.y;
	bottom += point.y;
}

inline void CBofRect::OffsetRect(const CBofSize &size) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	left += size.cx;
	right += size.cx;
	top += size.cy;
	bottom += size.cy;
}

inline bool CBofRect::IntersectRect(const CBofRect *pRect1, const CBofRect *pRect2) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	// Can't access null pointers
	Assert(pRect1 != nullptr);
	Assert(pRect2 != nullptr);

	left = MAX(pRect1->left, pRect2->left);
	top = MAX(pRect1->top, pRect2->top);
	right = MIN(pRect1->right, pRect2->right);
	bottom = MIN(pRect1->bottom, pRect2->bottom);

	return ((width() > 0) && (height() > 0));
}

inline bool CBofRect::IntersectRect(const CBofRect &cRect1, const CBofRect &cRect2) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return IntersectRect(&cRect1, &cRect2);
}

inline bool CBofRect::UnionRect(const CBofRect *pRect1, const CBofRect *pRect2) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	// Can't access null pointers
	Assert(pRect1 != nullptr);
	Assert(pRect2 != nullptr);

	left = MIN(pRect1->left, pRect2->left);
	top = MIN(pRect1->top, pRect2->top);
	right = MAX(pRect1->right, pRect2->right);
	bottom = MAX(pRect1->bottom, pRect2->bottom);

	return IsRectEmpty();
}

inline void CBofRect::operator=(const CBofRect &srcRect) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	left = srcRect.left;
	right = srcRect.right;
	top = srcRect.top;
	bottom = srcRect.bottom;
}

inline bool CBofRect::operator==(const CBofRect &rect) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return (left == rect.left && right == rect.right &&
	        top == rect.top && bottom == rect.bottom);
}

inline bool CBofRect::operator!=(const CBofRect &rect) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	return (left != rect.left || right != rect.right ||
	        top != rect.top || bottom != rect.bottom);
}

inline void CBofRect::operator+=(const CBofPoint &point) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	OffsetRect(point);
}

inline void CBofRect::operator-=(const CBofPoint &point) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	OffsetRect(-point.x, -point.y);
}

inline void CBofRect::operator&=(const CBofRect &rect) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	IntersectRect(this, &rect);
}

inline void CBofRect::operator|=(const CBofRect &rect) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	UnionRect(this, &rect);
}

inline CBofRect CBofRect::operator+(const CBofPoint &pt) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	CBofRect rect(*this);

	rect.OffsetRect(pt.x, pt.y);

	return rect;
}

inline CBofRect CBofRect::operator-(const CBofPoint &pt) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	CBofRect rect(*this);

	rect.OffsetRect(-pt.x, -pt.y);

	return rect;
}

inline CBofRect CBofRect::operator&(const CBofRect &rect2) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	CBofRect rect;

	rect.IntersectRect(this, &rect2);

	return rect;
}

inline CBofRect CBofRect::operator|(const CBofRect &rect2) {
	// Make sure object is not used after it is destructed
	Assert(IsValidObject(this));

	CBofRect rect;

	rect.UnionRect(this, &rect2);

	return rect;
}

#define CRect CBofRect

} // namespace Bagel

#endif
