
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
#include "bagel/boflib/debug.h"
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

	ERROR_CODE LoadBitmaps(const char *pszBack, const char *pszThumb, const char *pszLeftUp = nullptr, const char *pszRightUp = nullptr, const char *pszLeftDown = nullptr, const char *pszRightDown = nullptr);

	ERROR_CODE SetPos(const int nPos, bool bRepaint = true);
	int GetPos() {
		return (m_nPos);
	}

	ERROR_CODE LineLeft() {
		return (SetPos(m_nPos - m_nLineDelta));
	}
	ERROR_CODE LineRight() {
		return (SetPos(m_nPos + m_nLineDelta));
	}
	ERROR_CODE PageLeft() {
		return (SetPos(m_nPos - m_nPageDelta));
	}
	ERROR_CODE PageRight() {
		return (SetPos(m_nPos + m_nPageDelta));
	}

	ERROR_CODE Home() {
		return (SetPos(m_nMin));
	}
	ERROR_CODE End() {
		return (SetPos(m_nMax));
	}

	int GetScrollMin() {
		return (m_nMin);
	}
	int GetScrollMax() {
		return (m_nMax);
	}

	void SetLineDelta(const int nDelta) {
		m_nLineDelta = nDelta;
	}
	int GetLineDelta() {
		return (m_nLineDelta);
	}

	void SetPageDelta(const int nDelta) {
		m_nPageDelta = nDelta;
	}
	int GetPageDelta() {
		return (m_nPageDelta);
	}

	void GetScrollRange(int &nMin, int &nMax);
	void SetScrollRange(int nMin, int nMax, bool bRepaint = true);

	ERROR_CODE SetText(const char *pszText, int nFlags = JUSTIFY_CENTER);

	void SetRepeatTimer(uint32 nMilliSeconds);
	ERROR_CODE Paint(CBofRect *pRect = nullptr);

protected:
	int PointToPos(CBofPoint *pPoint);

	virtual void OnPaint(CBofRect *pDirtyRect);
	virtual void OnLButtonDown(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual void OnLButtonUp(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual void OnMouseMove(uint32 nFlags, CBofPoint *pPoint, void * = nullptr);
	virtual void OnTimer(uint32);

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
