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

#include "bagel/boflib/app.h"
#include "bagel/boflib/gui/scroll_bar.h"

namespace Bagel {

#define BMP_SCROLL_TIMER    9999
#define DEF_TIMER_INTERVAL  100


CBofScrollBar::CBofScrollBar() {
	m_pLeftBtnUp = nullptr;
	m_pRightBtnUp = nullptr;
	m_pLeftBtnDn = nullptr;
	m_pRightBtnDn = nullptr;
	m_pThumb = nullptr;
	m_nMin = 0;
	m_nMax = 10;
	m_nPos = 0;
	m_nLineDelta = 1;
	m_nPageDelta = 1;

	m_pScrollText = nullptr;
	m_szScrollText[0] = '\0';

	m_cThumbSize.cx = 0;
	m_cThumbSize.cy = 0;
	m_cBkSize.cx = 0;
	m_cBkSize.cy = 0;
	m_cCurPoint.x = 0;
	m_cCurPoint.y = 0;
	m_cThumbPos.x = 0;
	m_cThumbPos.y = 0;

	m_nOffset = 0;
	m_nScrollWidth = 0;
	m_nRange = 0;
	m_bMouseCaptured = FALSE;
	m_nScrollState = 0;

	m_nTimerCount = DEF_TIMER_INTERVAL;
}


CBofScrollBar::~CBofScrollBar() {
	Assert(IsValidObject(this));

	m_szScrollText[0] = '\0';

	if (m_pScrollText != nullptr) {
		delete m_pScrollText;
		m_pScrollText = nullptr;
	}
	if (m_pThumb != nullptr) {
		delete m_pThumb;
		m_pThumb = nullptr;
	}
	if (m_pLeftBtnUp != nullptr) {
		delete m_pLeftBtnUp;
		m_pLeftBtnUp = nullptr;
	}
	if (m_pRightBtnUp != nullptr) {
		delete m_pRightBtnUp;
		m_pRightBtnUp = nullptr;
	}
	if (m_pLeftBtnDn != nullptr) {
		delete m_pLeftBtnDn;
		m_pLeftBtnDn = nullptr;
	}
	if (m_pRightBtnDn != nullptr) {
		delete m_pRightBtnDn;
		m_pRightBtnDn = nullptr;
	}
}


void CBofScrollBar::OnPaint(CBofRect *pDirtyRect) {
	Assert(IsValidObject(this));

	Paint(pDirtyRect);
}


ERROR_CODE CBofScrollBar::SetText(const CHAR *pszText, INT nJustify) {
	Assert(IsValidObject(this));

	m_szScrollText[0] = '\0';

	if ((pszText != nullptr) && (_parent != nullptr)) {

		Common::strlcpy(m_szScrollText, pszText, MAX_TEXT);

		if (m_pScrollText == nullptr) {

			CBofRect cTempRect;
			CBofPoint cPoint;

			cPoint = _parent->GetWindowRect().TopLeft();

			cTempRect = m_cWindowRect - cPoint;

			cTempRect -= CPoint(0, 20);

			cTempRect.right += 20;

			if ((m_pScrollText = new CBofText(&cTempRect, nJustify)) != nullptr) {

			} else {
				ReportError(ERR_MEMORY, "Could not allocate a new CBofText");
			}
		}
		if (m_pScrollText != nullptr) {
			m_pScrollText->Display(_parent, m_szScrollText, FONT_DEFAULT_SIZE, TEXT_DEFAULT_FACE);
		}
	}

	return m_errCode;
}


ERROR_CODE CBofScrollBar::SetPos(const INT nPos, BOOL bRepaint) {
	Assert(IsValidObject(this));

	INT nOriginalPos;

	// save old position
	nOriginalPos = m_nPos;

	m_nPos = nPos;
	if (nPos < m_nMin)
		m_nPos = m_nMin;
	if (nPos > m_nMax)
		m_nPos = m_nMax;

	Assert(m_nRange != 0);

	m_cThumbPos.x = (INT)(((int32)(m_nScrollWidth - m_cThumbSize.cx) * m_nPos) / (m_nRange - 1)) + m_nOffset;
	m_cThumbPos.y = (INT)(m_cBkSize.cy / 2) - (INT)(m_cThumbSize.cy / 2);

	if (m_cThumbPos.x < 0)
		m_cThumbPos.x = 0;
	if (m_cThumbPos.x > (m_nScrollWidth - m_cThumbSize.cx + m_nOffset))
		m_cThumbPos.x = m_nScrollWidth - m_cThumbSize.cx + m_nOffset;

	// if forced to repaint
	//
	if (bRepaint) {

		Paint();

		// otherwise, only paint the thumb if it's position changed
		//
	} else if (m_nPos != nOriginalPos) {

		if (m_pThumb != nullptr) {
			if (m_pThumb->PaintSprite(this, m_cThumbPos) == FALSE) {
				ReportError(ERR_UNKNOWN, "m_pThumb->PaintSprite() failed");
			}
		}
	}

	// if the thumb actually moved, then tell our parent about it
	//
	if (m_nPos != nOriginalPos) {
		_parent->OnBofScrollBar(this, m_nPos);
	}

	return m_errCode;
}


void CBofScrollBar::GetScrollRange(INT &nMin, INT &nMax) {
	Assert(IsValidObject(this));

	nMin = m_nMin;
	nMax = m_nMax;
}


void CBofScrollBar::SetScrollRange(INT nMin, INT nMax, BOOL bRepaint) {
	Assert(IsValidObject(this));

	m_nMin = nMin;
	m_nMax = nMax;

	m_nRange = m_nMax - m_nMin + 1;

	// should we repaint the scroll bar now?
	//
	if (bRepaint) {
		Paint(nullptr);
	}
}


ERROR_CODE CBofScrollBar::LoadBitmaps(const CHAR *pszBack, const CHAR *pszThumb, const CHAR *pszLeftBtnUp, const CHAR *pszRightBtnUp, const CHAR *pszLeftBtnDn, const CHAR *pszRightBtnDn) {
	Assert(IsValidObject(this));
	CBofPoint cPoint;
	CBofPalette *pPalette;

	if ((pszBack != nullptr) && (pszThumb != nullptr)) {

		m_cLeftBtnRect.SetRect(0, 0, 0, 0);
		m_cRightBtnRect.SetRect(0, 0, 0, 0);

		if (m_pThumb != nullptr) {
			m_pThumb->EraseSprite(this);
			delete m_pThumb;
			m_pThumb = nullptr;
		}

		KillBackdrop();
		SetBackdrop(pszBack);

		pPalette = CBofApp::GetApp()->GetPalette();

		m_cBkSize = m_pBackdrop->GetSize();
		m_nScrollWidth = m_cBkSize.cx;

		if ((m_pThumb = new CBofSprite) != nullptr) {

			if (m_pThumb->LoadSprite(pszThumb) != FALSE) {

				m_pThumb->SetMaskColor(COLOR_WHITE);
				m_cThumbSize = m_pThumb->GetSize();
			}
		}

		if (m_pLeftBtnUp != nullptr) {
			delete m_pLeftBtnUp;
			m_pLeftBtnUp = nullptr;
		}

		if (pszLeftBtnUp != nullptr) {

			if ((m_pLeftBtnUp = new CBofBitmap(pszLeftBtnUp, pPalette)) != nullptr) {
				cPoint.x = 0;
				cPoint.y = (m_pBackdrop->Height() / 2) - (m_pLeftBtnUp->Height() / 2);

				m_cLeftBtnRect = m_pLeftBtnUp->GetRect() + cPoint;

				m_nOffset = m_pLeftBtnUp->Width();
				m_nScrollWidth -= m_nOffset;

			} else {
				ReportError(ERR_MEMORY, "Could not allocate a new CBofBitmap(%s)", pszLeftBtnUp);
			}
		}

		if (m_pRightBtnUp != nullptr) {
			delete m_pRightBtnUp;
			m_pRightBtnUp = nullptr;
		}
		if (pszRightBtnUp != nullptr) {
			if ((m_pRightBtnUp = new CBofBitmap(pszRightBtnUp, pPalette)) != nullptr) {

				cPoint.x = m_pBackdrop->Width() - m_pRightBtnUp->Width();
				cPoint.y = (m_pBackdrop->Height() / 2) - (m_pRightBtnUp->Height() / 2);
				m_cRightBtnRect = m_pLeftBtnUp->GetRect() + cPoint;

				m_nScrollWidth -= m_cRightBtnRect.Width();

			} else {
				ReportError(ERR_MEMORY, "Could not allocate a new CBofBitmap(%s)", pszRightBtnUp);
			}
		}

		if (m_pLeftBtnDn != nullptr) {
			delete m_pLeftBtnDn;
			m_pLeftBtnDn = nullptr;
		}
		if (pszLeftBtnDn != nullptr) {
			if ((m_pLeftBtnDn = new CBofBitmap(pszLeftBtnDn, pPalette)) != nullptr) {

			} else {
				ReportError(ERR_MEMORY, "Could not allocate a new CBofBitmap(%s)", pszLeftBtnDn);
			}
		}

		if (m_pRightBtnDn != nullptr) {
			delete m_pRightBtnDn;
			m_pRightBtnDn = nullptr;
		}
		if (pszRightBtnDn != nullptr) {
			if ((m_pRightBtnDn = new CBofBitmap(pszRightBtnDn, pPalette)) != nullptr) {

			} else {
				ReportError(ERR_MEMORY, "Could not allocate a new CBofBitmap(%s)", pszRightBtnDn);
			}
		}
	}

	return m_errCode;
}


ERROR_CODE CBofScrollBar::Paint(CBofRect *pDirtyRect) {
	Assert(IsValidObject(this));

	if (!ErrorOccurred()) {

		CBofRect cRect(0, 0, m_cRect.Width() - 1, m_cRect.Height() - 1);
		CBofPoint cPoint(0, 0);
		CBofBitmap *pBmp;
		CBofPalette *pPalette;

		if (pDirtyRect == nullptr) {
			pDirtyRect = &cRect;
		}

		pPalette = CBofApp::GetApp()->GetPalette();

		//
		// This function needs to be optimized to paint only the section that is
		// invalidated.  Right now it just repaints the entire scroll bar each time.
		//

		if ((m_pBackdrop != nullptr) && (m_pThumb != nullptr)) {

			// do all painting offscreen
			//
			if ((pBmp = new CBofBitmap(m_cBkSize.cx, m_cBkSize.cy, pPalette)) != nullptr) {

				m_pBackdrop->Paint(pBmp, 0, 0, nullptr, COLOR_WHITE);

				if ((m_nScrollState == 1) && (m_pLeftBtnDn != nullptr)) {

					cPoint = m_cLeftBtnRect.TopLeft();
					m_pLeftBtnDn->Paint(pBmp, cPoint.x, cPoint.y, nullptr, COLOR_WHITE);

				} else {
					if (m_pLeftBtnUp != nullptr) {

						cPoint = m_cLeftBtnRect.TopLeft();
						m_pLeftBtnUp->Paint(pBmp, cPoint.x, cPoint.y, nullptr, COLOR_WHITE);
					}
				}

				if ((m_nScrollState == 4) && (m_pRightBtnDn != nullptr)) {

					cPoint = m_cRightBtnRect.TopLeft();
					m_pRightBtnDn->Paint(pBmp, cPoint.x, cPoint.y, nullptr, COLOR_WHITE);

				} else {
					if (m_pRightBtnUp != nullptr) {

						cPoint = m_cRightBtnRect.TopLeft();
						m_pRightBtnUp->Paint(pBmp, cPoint.x, cPoint.y, nullptr, COLOR_WHITE);
					}
				}

				m_cThumbPos.x = (INT)(((int32)(m_nScrollWidth - m_cThumbSize.cx) * m_nPos) / (m_nRange - 1)) + m_nOffset;
				m_cThumbPos.y = (INT)(m_cBkSize.cy / 2) - (INT)(m_cThumbSize.cy / 2);

				if (m_cThumbPos.x < 0)
					m_cThumbPos.x = 0;
				if (m_cThumbPos.x > (m_nScrollWidth - m_cThumbSize.cx + m_nOffset))
					m_cThumbPos.x = m_nScrollWidth - m_cThumbSize.cx + m_nOffset;

				m_pThumb->PaintSprite(pBmp, m_cThumbPos);

				// now we can paint the offscreen buffer to the screen
				pBmp->Paint(this, 0, 0);

				delete pBmp;
			} else {
				ReportError(ERR_MEMORY, "Could not allocate a new CBofBitmap(%d x %d))", m_cBkSize.cx, m_cBkSize.cy);
			}
		}

		if ((m_pScrollText != nullptr) && (_parent != nullptr)) {

			m_pScrollText->Display(_parent, m_szScrollText, FONT_DEFAULT_SIZE, TEXT_DEFAULT_FACE);
		}
	}

	return m_errCode;
}


void CBofScrollBar::OnLButtonDown(uint32 nFlags, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));

	CBofRect cLeftPageRect, cRightPageRect;
	BOOL bDoNothing;

	bDoNothing = FALSE;

	cLeftPageRect.SetRect(m_nOffset, 0, (m_nScrollWidth / m_nRange) * m_nPos + m_nOffset - 1, m_cBkSize.cy - 1);
	cRightPageRect.SetRect(((m_nScrollWidth / m_nRange) * m_nPos) + m_nOffset + m_cThumbSize.cx, 0, m_nOffset + m_nScrollWidth - 1, m_cBkSize.cy - 1);

	m_cCurPoint = *pPoint;

	if (m_pLeftBtnUp != nullptr && m_cLeftBtnRect.PtInRect(*pPoint)) {

		// let timer know what happened
		m_nScrollState = 1;

		// set new thumb position
		SetPos(m_nPos - m_nLineDelta, TRUE);

	} else if (m_pThumb->GetRect().PtInRect(*pPoint)) {

		m_nScrollState = 5;

	} else if (cLeftPageRect.PtInRect(*pPoint)) {
		m_nScrollState = 2;

		// set new thumb position
		SetPos(m_nPos - m_nPageDelta, TRUE);

	} else if (cRightPageRect.PtInRect(*pPoint)) {
		m_nScrollState = 3;

		// set new thumb position
		SetPos(m_nPos + m_nPageDelta, TRUE);

	} else if (m_pRightBtnUp != nullptr && m_cRightBtnRect.PtInRect(*pPoint)) {

		// let timer know what happened
		m_nScrollState = 4;

		// set new thumb position
		SetPos(m_nPos + m_nLineDelta, TRUE);

	} else {
		bDoNothing = TRUE;
	}

	if (!bDoNothing) {

		m_bMouseCaptured = TRUE;
		SetCapture();
		if (m_nScrollState != 5)
			SetTimer(BMP_SCROLL_TIMER, m_nTimerCount);
	}

	CBofWindow::OnLButtonDown(nFlags, pPoint);
}


INT CBofScrollBar::PointToPos(CBofPoint *pPoint) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	INT nPos = m_nPos;

	if (m_cRect.PtInRect(*pPoint)) {

		nPos = (pPoint->x - m_nOffset) / (INT)(m_nScrollWidth / m_nRange);
	}

	return nPos;
}


void CBofScrollBar::OnLButtonUp(uint32 nFlags, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));

	INT x, y;

	if (m_bMouseCaptured) {

		KillTimer(BMP_SCROLL_TIMER);
		m_bMouseCaptured = FALSE;
		ReleaseCapture();

		switch (m_nScrollState) {

		case 5:
			SetPos(PointToPos(pPoint));
			break;

		case 1:
			if (m_pLeftBtnUp != nullptr) {
				x = 0;
				y = (INT)(m_cBkSize.cy / 2) - (INT)(m_cLeftBtnRect.Height() / 2);
				m_pLeftBtnUp->Paint(this, x, y, nullptr, COLOR_WHITE);
			}
			break;

		case 4:
			if (m_pRightBtnUp != nullptr) {

				x = m_cBkSize.cx - m_cRightBtnRect.Width();
				y = (INT)(m_cBkSize.cy / 2) - (INT)(m_cRightBtnRect.Height() / 2);
				m_pRightBtnUp->Paint(this, x, y, nullptr, COLOR_WHITE);
			}
			break;

		default:
			break;
		}
		m_nScrollState = 0;
	}

	CBofWindow::OnLButtonUp(nFlags, pPoint);
}


void CBofScrollBar::OnMouseMove(uint32 nFlags, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));

	if (m_bMouseCaptured) {

		m_cCurPoint = *pPoint;
		if (m_nScrollState == 5) {
			SetPos(PointToPos(pPoint));
		}
	}

	CBofWindow::OnMouseMove(nFlags, pPoint);
}


void CBofScrollBar::SetRepeatTimer(uint32 nTimerInt) {
	Assert(IsValidObject(this));

	m_nTimerCount = nTimerInt;
}


void CBofScrollBar::OnTimer(uint32 nWhichTimer) {
	Assert(IsValidObject(this));

	CBofRect cLeftPageRect, cRightPageRect;

	cLeftPageRect.SetRect(m_nOffset, 0, (m_nScrollWidth / m_nRange) * m_nPos + m_nOffset - 1, m_cBkSize.cy - 1);
	cRightPageRect.SetRect(((m_nScrollWidth / m_nRange) * m_nPos) + m_nOffset + m_cThumbSize.cx, 0, m_nOffset + m_nScrollWidth - 1, m_cBkSize.cy - 1);

	if (nWhichTimer == BMP_SCROLL_TIMER) {

		if ((m_nScrollState == 1) && m_cLeftBtnRect.PtInRect(m_cCurPoint)) {
			LineLeft();

		} else if ((m_nScrollState == 2) && cLeftPageRect.PtInRect(m_cCurPoint)) {
			PageLeft();

		} else if ((m_nScrollState == 3) && cRightPageRect.PtInRect(m_cCurPoint)) {
			PageRight();

		} else if ((m_nScrollState == 4) && m_cRightBtnRect.PtInRect(m_cCurPoint)) {
			LineRight();
		}
	}
}

} // namespace Bagel
