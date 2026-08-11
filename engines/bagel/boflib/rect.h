
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
#include "bagel/afxwin.h"
#include "bagel/boflib/stdinc.h"
#include "bagel/boflib/object.h"
#include "bagel/boflib/point.h"
#include "bagel/boflib/size.h"

namespace Bagel {

class CBofRect : public CBofObject {
public:
	// Constructors
	CBofRect();
	CBofRect(int l, int t, int r, int b);
	CBofRect(const CBofRect &srcRect);
	CBofRect(const CBofPoint &point, const CBofSize &size);
	CBofRect(const CBofPoint &pttopLeft, const CBofPoint &ptBottomRight);
	virtual ~CBofRect() {
	}

	// Attributes
	int width() const;
	int height() const;
	CBofSize size() const;
	CBofPoint topLeft() const;
	CBofPoint bottomRight() const;

	CBofPoint topRight() const;
	CBofPoint bottomLeft() const;

	bool isRectEmpty() const;
	bool isRectNull() const;
	bool ptInRect(const CBofPoint &point) const;

	// Operations

	inline void setRect(int x1, int y1, int x2, int y2);
	void setRectEmpty();
	void copyRect(const CBofRect &cRect);
	bool equalRect(const CBofRect &cRect);

	CBofRect(const RECT &cRect) {
		setWinRect(&cRect);
	}
	CBofRect(const RECT *pRect) {
		setWinRect(pRect);
	}

	void setWinRect(const RECT *pRect);
	RECT getWinRect();

	void operator=(const RECT &srcRect) {
		setWinRect(&srcRect);
	}
	void operator=(const Common::Rect &srcRect) {
		const RECT r = { srcRect.left, srcRect.top, srcRect.right, srcRect.bottom };
		setWinRect(&r);
	}

	operator const RECT() {
		return getWinRect();
	}
	operator const Common::Rect() {
		return Common::Rect(left, top, right, bottom);
	}

	void offsetRect(int x, int y);
	void offsetRect(const CBofSize &size);
	void offsetRect(const CBofPoint &point);

	// Operations that fill '*this' with result
	bool intersectRect(const CBofRect *pRect1, const CBofRect *pRect2);
	bool intersectRect(const CBofRect &cRect1, const CBofRect &cRect2);
	bool unionRect(const CBofRect *pRect1, const CBofRect *pRect2);

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

inline CBofRect::CBofRect(const CBofPoint &pttopLeft, const CBofPoint &ptBottomRight) {
	setRect(pttopLeft.x, pttopLeft.y, ptBottomRight.x, ptBottomRight.y);
}

inline int CBofRect::width() const {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	return right - left + 1;
}

inline int CBofRect::height() const {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	return bottom - top + 1;
}

inline CBofSize CBofRect::size() const {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	return CBofSize(width(), height());
}

inline CBofPoint CBofRect::topLeft() const {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	return CBofPoint(left, top);
}

inline CBofPoint CBofRect::bottomRight() const {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	return CBofPoint(right, bottom);
}

inline CBofPoint CBofRect::topRight() const {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	return CBofPoint(right, top);
}

inline CBofPoint CBofRect::bottomLeft() const {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	return CBofPoint(left, bottom);
}

inline bool CBofRect::isRectEmpty() const {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	return (width() <= 0 || height() <= 0);
}

inline bool CBofRect::isRectNull() const {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	return (left == 0 && right == 0 && top == 0 && bottom == 0);
}

inline bool CBofRect::ptInRect(const CBofPoint &point) const {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	return (point.x >= left && point.x <= right && point.y >= top && point.y <= bottom);
}

inline void CBofRect::setRect(int x1, int y1, int x2, int y2) {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	left = x1;
	top = y1;
	right = x2;
	bottom = y2;
}

inline void CBofRect::setRectEmpty() {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	left = top = 0;
	right = bottom = -1;

	// Verify that the rectangle is now empty
	assert(isRectEmpty());
}

inline void CBofRect::copyRect(const CBofRect &cSrcRect) {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	assert(isValidObject(&cSrcRect));

	left = cSrcRect.left;
	right = cSrcRect.right;
	top = cSrcRect.top;
	bottom = cSrcRect.bottom;
}

inline bool CBofRect::equalRect(const CBofRect &cRect) {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	assert(isValidObject(&cRect));

	return (left == cRect.left && right == cRect.right && top == cRect.top && bottom == cRect.bottom);
}

inline RECT CBofRect::getWinRect() {
	assert(isValidObject(this));

	RECT stRect;

	stRect.left = left;
	stRect.top = top;
	stRect.right = right + 1;
	stRect.bottom = bottom + 1;

	return stRect;
}

inline void CBofRect::setWinRect(const RECT *pRect) {
	assert(isValidObject(this));

	assert(pRect != nullptr);

	left = pRect->left;
	top = pRect->top;
	right = pRect->right - 1;
	bottom = pRect->bottom - 1;
}

inline void CBofRect::offsetRect(int x, int y) {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	left += x;
	right += x;
	top += y;
	bottom += y;
}

inline void CBofRect::offsetRect(const CBofPoint &point) {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	left += point.x;
	right += point.x;
	top += point.y;
	bottom += point.y;
}

inline void CBofRect::offsetRect(const CBofSize &size) {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	left += size.cx;
	right += size.cx;
	top += size.cy;
	bottom += size.cy;
}

inline bool CBofRect::intersectRect(const CBofRect *pRect1, const CBofRect *pRect2) {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	// Can't access null pointers
	assert(pRect1 != nullptr);
	assert(pRect2 != nullptr);

	left = MAX(pRect1->left, pRect2->left);
	top = MAX(pRect1->top, pRect2->top);
	right = MIN(pRect1->right, pRect2->right);
	bottom = MIN(pRect1->bottom, pRect2->bottom);

	return ((width() > 0) && (height() > 0));
}

inline bool CBofRect::intersectRect(const CBofRect &cRect1, const CBofRect &cRect2) {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	return intersectRect(&cRect1, &cRect2);
}

inline bool CBofRect::unionRect(const CBofRect *pRect1, const CBofRect *pRect2) {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	// Can't access null pointers
	assert(pRect1 != nullptr);
	assert(pRect2 != nullptr);

	left = MIN(pRect1->left, pRect2->left);
	top = MIN(pRect1->top, pRect2->top);
	right = MAX(pRect1->right, pRect2->right);
	bottom = MAX(pRect1->bottom, pRect2->bottom);

	return isRectEmpty();
}

inline void CBofRect::operator=(const CBofRect &srcRect) {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	left = srcRect.left;
	right = srcRect.right;
	top = srcRect.top;
	bottom = srcRect.bottom;
}

inline bool CBofRect::operator==(const CBofRect &rect) {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	return (left == rect.left && right == rect.right &&
	        top == rect.top && bottom == rect.bottom);
}

inline bool CBofRect::operator!=(const CBofRect &rect) {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	return (left != rect.left || right != rect.right ||
	        top != rect.top || bottom != rect.bottom);
}

inline void CBofRect::operator+=(const CBofPoint &point) {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	offsetRect(point);
}

inline void CBofRect::operator-=(const CBofPoint &point) {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	offsetRect(-point.x, -point.y);
}

inline void CBofRect::operator&=(const CBofRect &rect) {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	intersectRect(this, &rect);
}

inline void CBofRect::operator|=(const CBofRect &rect) {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	unionRect(this, &rect);
}

inline CBofRect CBofRect::operator+(const CBofPoint &pt) {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	CBofRect rect(*this);

	rect.offsetRect(pt.x, pt.y);

	return rect;
}

inline CBofRect CBofRect::operator-(const CBofPoint &pt) {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	CBofRect rect(*this);

	rect.offsetRect(-pt.x, -pt.y);

	return rect;
}

inline CBofRect CBofRect::operator&(const CBofRect &rect2) {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	CBofRect rect;

	rect.intersectRect(this, &rect2);

	return rect;
}

inline CBofRect CBofRect::operator|(const CBofRect &rect2) {
	// Make sure object is not used after it is destructed
	assert(isValidObject(this));

	CBofRect rect;

	rect.unionRect(this, &rect2);

	return rect;
}

} // namespace Bagel

#endif
