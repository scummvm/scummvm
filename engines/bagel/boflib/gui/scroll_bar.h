
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

	ERROR_CODE LoadBitmaps(const CHAR *pszBack, const CHAR *pszThumb, const CHAR *pszLeftUp = nullptr, const CHAR *pszRightUp = nullptr, const CHAR *pszLeftDown = nullptr, const CHAR *pszRightDown = nullptr);

	ERROR_CODE SetPos(const INT nPos, BOOL bRepaint = TRUE);
	INT GetPos() { return (m_nPos); }

	ERROR_CODE LineLeft() { return (SetPos(m_nPos - m_nLineDelta)); }
	ERROR_CODE LineRight() { return (SetPos(m_nPos + m_nLineDelta)); }
	ERROR_CODE PageLeft() { return (SetPos(m_nPos - m_nPageDelta)); }
	ERROR_CODE PageRight() { return (SetPos(m_nPos + m_nPageDelta)); }

	ERROR_CODE Home() { return (SetPos(m_nMin)); }
	ERROR_CODE End() { return (SetPos(m_nMax)); }

	INT GetScrollMin() { return (m_nMin); }
	INT GetScrollMax() { return (m_nMax); }

	VOID SetLineDelta(const INT nDelta) { m_nLineDelta = nDelta; }
	INT GetLineDelta() { return (m_nLineDelta); }

	VOID SetPageDelta(const INT nDelta) { m_nPageDelta = nDelta; }
	INT GetPageDelta() { return (m_nPageDelta); }

	VOID GetScrollRange(INT &nMin, INT &nMax);
	VOID SetScrollRange(INT nMin, INT nMax, BOOL bRepaint = TRUE);

	ERROR_CODE SetText(const CHAR *pszText, INT nFlags = JUSTIFY_CENTER);

	VOID SetRepeatTimer(UINT nMilliSeconds);
	ERROR_CODE Paint(CBofRect *pRect = nullptr);

protected:
	INT PointToPos(CBofPoint *pPoint);

	virtual VOID OnPaint(CBofRect *pDirtyRect);
	virtual VOID OnLButtonDown(UINT nFlags, CBofPoint *pPoint);
	virtual VOID OnLButtonUp(UINT nFlags, CBofPoint *pPoint);
	virtual VOID OnMouseMove(UINT nFlags, CBofPoint *pPoint);
	virtual VOID OnTimer(UINT);

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

	INT m_nMin;
	INT m_nMax;
	INT m_nPos;
	INT m_nLineDelta;
	INT m_nPageDelta;

	CBofText *m_pScrollText;
	CHAR m_szScrollText[MAX_TEXT];

	CBofSize m_cThumbSize;
	CBofSize m_cBkSize;

	INT m_nOffset;
	INT m_nScrollWidth;
	INT m_nRange;
	BOOL m_bMouseCaptured;
	CBofPoint m_cCurPoint;
	CBofPoint m_cThumbPos;
	INT m_nScrollState;
	UINT m_nTimerCount;
	BOOL m_bHavePainted;
};

} // namespace Bagel

#endif
