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

#include "common/textconsole.h"
#include "graphics/mfc/afxwin.h"

namespace Graphics {
namespace MFC {

IMPLEMENT_DYNAMIC(CScrollBar, CWnd)
BEGIN_MESSAGE_MAP(CScrollBar, CWnd)
ON_WM_PAINT()
ON_WM_LBUTTONDOWN()
ON_WM_LBUTTONUP()
ON_WM_MOUSEMOVE()
ON_WM_MOUSELEAVE()
END_MESSAGE_MAP()

bool CScrollBar::Create(uint32 dwStyle, const RECT &rect, CWnd *pParentWnd, unsigned int nID) {
	return CWnd::Create("SCROLLBAR", nullptr, dwStyle,
		rect, pParentWnd, nID);
}

int CScrollBar::GetScrollPos() const {
	return _value;
}

int CScrollBar::SetScrollPos(int nPos, bool bRedraw) {
	int oldPos = _value;
	_value = CLIP(nPos, _minValue, _maxValue);
	Invalidate();
	return oldPos;
}

void CScrollBar::GetScrollRange(int *lpMinPos, int *lpMaxPos) const {
	if (lpMinPos)
		*lpMinPos = _minValue;
	if (lpMaxPos)
		*lpMaxPos = _maxValue;
}

void CScrollBar::SetScrollRange(int nMinPos, int nMaxPos, bool bRedraw) {
	_minValue = nMinPos;
	_maxValue = nMaxPos;
	_pageSize = MAX<int>((_maxValue - _minValue + 1) / 10, 1);

	Invalidate();
}

bool CScrollBar::SetScrollInfo(LPSCROLLINFO lpScrollInfo, bool bRedraw) {
	assert(lpScrollInfo->cbSize == sizeof(SCROLLINFO));
	_minValue = lpScrollInfo->nMin;
	_maxValue = lpScrollInfo->nMax;
	_pageSize = lpScrollInfo->nPage;
	_value = lpScrollInfo->nPos;

	if (bRedraw)
		Invalidate();
	return true;
}

void CScrollBar::ShowScrollBar(bool bShow) {
	warning("TODO: CScrollBar::ShowScrollBar");
}

void CScrollBar::OnPaint() {
	CPaintDC dc(this);
	CRect clientRect;
	GetClientRect(&clientRect);
	const int thumbSize = clientRect.bottom;

	// Clear background and draw border edge
	dc.FillSolidRect(&clientRect, RGB(211, 211, 211));
	CBrush borderBrush(RGB(0, 0, 0));
	dc.FrameRect(&clientRect, &borderBrush);

	CRect left(0, 0, thumbSize, clientRect.bottom);
	drawSquare(dc, left);
	drawArrow(dc, left, true);
	CRect right(clientRect.right - thumbSize, 0,
		clientRect.right, clientRect.bottom);
	drawSquare(dc, right);
	drawArrow(dc, right, false);

	drawSquare(dc, getThumbRect());
}

void CScrollBar::drawSquare(CPaintDC &dc, const CRect &r) {
	CBrush black(RGB(0, 0, 0));
	dc.FrameRect(&r, &black);

	CBrush darkGrey(RGB(169, 169, 169));
	CBrush white(RGB(255, 255, 255));
	CBrush *oldBrush = dc.SelectObject(&darkGrey);

	dc.MoveTo(r.left + 2, r.bottom - 3);
	dc.LineTo(r.right - 2, r.bottom - 3);
	dc.MoveTo(r.left + 1, r.bottom - 2);
	dc.LineTo(r.right - 2, r.bottom - 2);
	dc.MoveTo(r.right - 3, r.top + 1);
	dc.LineTo(r.right - 3, r.bottom - 2);
	dc.MoveTo(r.right - 2, r.top + 1);
	dc.LineTo(r.right - 2, r.bottom - 2);

	dc.MoveTo(r.left + 1, r.top + 1);
	dc.LineTo(r.right - 3, r.top + 1);
	dc.MoveTo(r.left + 1, r.top + 1);
	dc.MoveTo(r.left + 1, r.bottom - 3);

	dc.SelectObject(oldBrush);
}

void CScrollBar::drawArrow(CPaintDC &dc, const CRect &r, bool leftArrow) {
	int xCenter = (r.left + r.right) / 2;
	int yCenter = r.bottom / 2;
	int xDelta = leftArrow ? 1 : -1;
	int x = leftArrow ? xCenter - 3 : xCenter + 3;

	CBrush black(RGB(0, 0, 0));
	CBrush *oldBrush = dc.SelectObject(&black);

	for (int xCtr = 0; xCtr < 7; ++xCtr, x += xDelta) {
		int yDiff = (xCtr >= 4) ? 1 : xCtr;
		dc.MoveTo(x, yCenter - yDiff);
		dc.LineTo(x, yCenter + yDiff);
	}

	dc.SelectObject(oldBrush);
}

CRect CScrollBar::getThumbRect() const {
	CRect clientRect;
	GetClientRect(&clientRect);

	// The thumb will start after the left arrow button,
	// and at most, it will be drawn before the right arrow
	int slideArea = clientRect.right -
		(clientRect.bottom * 3) + 1;

	int xStart = clientRect.bottom +
		slideArea * (_value - _minValue) /
		(_maxValue - _minValue);

	return CRect(xStart, 0, xStart + clientRect.bottom,
		clientRect.bottom);
}

int CScrollBar::getIndexFromX(int xp) const {
	CRect clientRect;
	GetClientRect(&clientRect);

	int slideStart = clientRect.bottom;
	int slideFinish = clientRect.right - (clientRect.bottom * 2) + 1;
	int slideArea = clientRect.right - (clientRect.bottom * 3) + 1;

	if (xp < slideStart)
		return _minValue;
	if (xp >= slideFinish)
		return _maxValue;

	return _minValue + (xp - slideStart)
		* (_maxValue - _minValue) / slideArea;
}

void CScrollBar::OnLButtonDown(unsigned int nFlags, CPoint point) {
	CRect clientRect;
	GetClientRect(&clientRect);

	if (point.x < clientRect.bottom) {
		// Left arrow button
		if (_value > _minValue) {
			SetScrollPos(_value - 1);
			scrollEvent(SB_LINELEFT);
		}

	} else if (point.x >= (clientRect.right - clientRect.bottom)) {
		// Right arrow button
		if (_value < _maxValue) {
			SetScrollPos(_value + 1);
			scrollEvent(SB_LINERIGHT);
		}

	} else {
		// We're in the slider area
		CRect thumbRect = getThumbRect();

		if (point.x < thumbRect.left) {
			// Left of thumb, page left
			SetScrollPos(_value - _pageSize);
			scrollEvent(SB_PAGELEFT);
		} else if (point.x >= thumbRect.right) {
			// Right of thumb, page right
			SetScrollPos(_value + _pageSize);
			scrollEvent(SB_PAGERIGHT);
		} else {
			// Directly on thumb, so start dragging it
			SetScrollPos(getIndexFromX(point.x));
			SetCapture();
			scrollEvent(SB_THUMBTRACK);
		}
	}
}

void CScrollBar::OnLButtonUp(unsigned int nFlags, CPoint point) {
	if (GetCapture() == m_hWnd) {
		ReleaseCapture();
		scrollEvent(SB_THUMBPOSITION);
	}

	scrollEvent(SB_ENDSCROLL);
}

void CScrollBar::OnMouseMove(unsigned int, CPoint point) {
	if (GetCapture() == m_hWnd) {
		SetScrollPos(getIndexFromX(point.x));
		scrollEvent(SB_THUMBTRACK);
	}
}

void CScrollBar::scrollEvent(int action) {
	m_pParentWnd->SendMessage(WM_HSCROLL,
		MAKEWPARAM(action, _value),
		(LPARAM)m_hWnd);
}

} // namespace MFC
} // namespace Graphics
