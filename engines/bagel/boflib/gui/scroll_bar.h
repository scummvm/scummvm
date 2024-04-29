
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

#ifndef BAGEL_BOFLIB_GUI_SCROLL_BAR_H
#define BAGEL_BOFLIB_GUI_SCROLL_BAR_H

#include "bagel/boflib/gui/window.h"
#include "bagel/boflib/gfx/sprite.h"
#include "bagel/boflib/gfx/text.h"

namespace Bagel {

#define BSB_LEFT 800
#define BSB_RIGHT 801
#define BSB_LINE_LEFT 802
#define BSB_LINE_RIGHT 803
#define BSB_PAGE_LEFT 804
#define BSB_PAGE_RIGHT 805
#define BSB_THUMB_POS 806
#define BSB_THUMB_TRACK 807

#define MAX_TEXT 128

class CBofScrollBar : public CBofWindow {
public:
	CBofScrollBar();

	virtual ~CBofScrollBar();

	// Implementation
	//

	ErrorCode loadBitmaps(const char *pszBack, const char *pszThumb, const char *pszLeftUp = nullptr, const char *pszRightUp = nullptr, const char *pszLeftDown = nullptr, const char *pszRightDown = nullptr);

	ErrorCode setPos(const int nPos, bool bRepaint = true, bool isInitial = false);
	int getPos() {
		return m_nPos;
	}

	ErrorCode lineLeft() {
		return setPos(m_nPos - m_nLineDelta);
	}
	ErrorCode lineRight() {
		return setPos(m_nPos + m_nLineDelta);
	}
	ErrorCode pageLeft() {
		return setPos(m_nPos - m_nPageDelta);
	}
	ErrorCode pageRight() {
		return setPos(m_nPos + m_nPageDelta);
	}

	ErrorCode home() {
		return setPos(m_nMin);
	}
	ErrorCode end() {
		return setPos(m_nMax);
	}

	int getScrollMin() {
		return m_nMin;
	}
	int getScrollMax() {
		return m_nMax;
	}

	void setLineDelta(const int nDelta) {
		m_nLineDelta = nDelta;
	}
	int getLineDelta() {
		return m_nLineDelta;
	}

	void setPageDelta(const int nDelta) {
		m_nPageDelta = nDelta;
	}
	int getPageDelta() {
		return m_nPageDelta;
	}

	void getScrollRange(int &nMin, int &nMax);
	void setScrollRange(int nMin, int nMax, bool bRepaint = true);

	ErrorCode setText(const char *pszText, int nFlags = JUSTIFY_CENTER);

	void setRepeatTimer(uint32 nMilliSeconds);
	ErrorCode paint(CBofRect *pRect = nullptr);

protected:
	int pointToPos(CBofPoint *pPoint);

	virtual void onPaint(CBofRect *pDirtyRect);
	virtual void onLButtonDown(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual void onLButtonUp(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual void onMouseMove(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual void onTimer(uint32);

	//
	// Data members
	//
	CBofBitmap *m_pLeftBtnUp;
	CBofBitmap *m_pRightBtnUp;
	CBofBitmap *m_pLeftBtnDn;
	CBofBitmap *m_pRightBtnDn;
	CBofSprite *m_pThumb;

	CBofRect m_cLeftBtnRect;
	CBofRect m_cRightBtnRect;

	int m_nMin;
	int m_nMax;
	int m_nPos;
	int m_nLineDelta;
	int m_nPageDelta;

	CBofText *m_pScrollText;
	char m_szScrollText[MAX_TEXT];

	CBofSize m_cThumbSize;
	CBofSize m_cBkSize;

	int m_nOffset;
	int m_nScrollWidth;
	int m_nRange;
	bool m_bMouseCaptured;
	CBofPoint m_cCurPoint;
	CBofPoint m_cThumbPos;
	int m_nScrollState;
	uint32 m_nTimerCount;
	bool m_bHavePainted;
};

} // namespace Bagel

#endif
