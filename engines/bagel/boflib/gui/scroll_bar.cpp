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
	m_bMouseCaptured = false;
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


void CBofScrollBar::onPaint(CBofRect *pDirtyRect) {
	Assert(IsValidObject(this));

	paint(pDirtyRect);
}


ErrorCode CBofScrollBar::setText(const char *pszText, int nJustify) {
	Assert(IsValidObject(this));

	m_szScrollText[0] = '\0';

	if ((pszText != nullptr) && (_parent != nullptr)) {
		Common::strlcpy(m_szScrollText, pszText, MAX_TEXT);

		if (m_pScrollText == nullptr) {

			CBofPoint cPoint = _parent->GetWindowRect().TopLeft();
			CBofRect cTempRect = _cWindowRect - cPoint;

			cTempRect -= CPoint(0, 20);
			cTempRect.right += 20;

			m_pScrollText = new CBofText(&cTempRect, nJustify);
			if (m_pScrollText != nullptr) {

			} else {
				ReportError(ERR_MEMORY, "Could not allocate a new CBofText");
			}
		}

		if (m_pScrollText != nullptr) {
			m_pScrollText->Display(_parent, m_szScrollText, FONT_DEFAULT_SIZE, TEXT_DEFAULT_FACE);
		}
	}

	return _errCode;
}


ErrorCode CBofScrollBar::SetPos(const int nPos, bool bRepaint, bool isInitial) {
	Assert(IsValidObject(this));

	// Save old position
	int nOriginalPos = m_nPos;		

	m_nPos = nPos;
	if (nPos < m_nMin)
		m_nPos = m_nMin;
	if (nPos > m_nMax)
		m_nPos = m_nMax;

	Assert(m_nRange != 0);

	m_cThumbPos.x = (int)(((int32)(m_nScrollWidth - m_cThumbSize.cx) * m_nPos) / (m_nRange - 1)) + m_nOffset;
	m_cThumbPos.y = (int)(m_cBkSize.cy / 2) - (int)(m_cThumbSize.cy / 2);

	if (m_cThumbPos.x < 0)
		m_cThumbPos.x = 0;
	if (m_cThumbPos.x > (m_nScrollWidth - m_cThumbSize.cx + m_nOffset))
		m_cThumbPos.x = m_nScrollWidth - m_cThumbSize.cx + m_nOffset;

	// If forced to repaint
	if (bRepaint) {

		paint();

	} else if (m_nPos != nOriginalPos) {
		// Otherwise, only paint the thumb if it's position changed
		if (m_pThumb != nullptr) {
			if (m_pThumb->PaintSprite(this, m_cThumbPos) == false) {
				ReportError(ERR_UNKNOWN, "m_pThumb->PaintSprite() failed");
			}
		}
	}

	// If the thumb actually moved, then tell our parent about it
	if (m_nPos != nOriginalPos && !isInitial) {
		_parent->OnBofScrollBar(this, m_nPos);
	}

	return _errCode;
}


void CBofScrollBar::GetScrollRange(int &nMin, int &nMax) {
	Assert(IsValidObject(this));

	nMin = m_nMin;
	nMax = m_nMax;
}


void CBofScrollBar::SetScrollRange(int nMin, int nMax, bool bRepaint) {
	Assert(IsValidObject(this));

	m_nMin = nMin;
	m_nMax = nMax;

	m_nRange = m_nMax - m_nMin + 1;

	// Should we repaint the scroll bar now?
	if (bRepaint) {
		paint(nullptr);
	}
}


ErrorCode CBofScrollBar::loadBitmaps(const char *pszBack, const char *pszThumb, const char *pszLeftBtnUp, const char *pszRightBtnUp, const char *pszLeftBtnDn, const char *pszRightBtnDn) {
	Assert(IsValidObject(this));

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

		CBofPalette *pPalette = CBofApp::GetApp()->GetPalette();

		m_cBkSize = _pBackdrop->GetSize();
		m_nScrollWidth = m_cBkSize.cx;

		if ((m_pThumb = new CBofSprite) != nullptr) {
			if (m_pThumb->LoadSprite(pszThumb) != false) {
				m_pThumb->SetMaskColor(COLOR_WHITE);
				m_cThumbSize = m_pThumb->GetSize();
			}
		}

		if (m_pLeftBtnUp != nullptr) {
			delete m_pLeftBtnUp;
			m_pLeftBtnUp = nullptr;
		}

		CBofPoint cPoint;
		if (pszLeftBtnUp != nullptr) {
			if ((m_pLeftBtnUp = new CBofBitmap(pszLeftBtnUp, pPalette)) != nullptr) {
				cPoint.x = 0;
				cPoint.y = (_pBackdrop->Height() / 2) - (m_pLeftBtnUp->Height() / 2);

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
				cPoint.x = _pBackdrop->Width() - m_pRightBtnUp->Width();
				cPoint.y = (_pBackdrop->Height() / 2) - (m_pRightBtnUp->Height() / 2);
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

	return _errCode;
}


ErrorCode CBofScrollBar::paint(CBofRect *pDirtyRect) {
	Assert(IsValidObject(this));

	if (!ErrorOccurred()) {
		CBofRect cRect(0, 0, _cRect.Width() - 1, _cRect.Height() - 1);
		CBofPoint cPoint(0, 0);

		if (pDirtyRect == nullptr) {
			pDirtyRect = &cRect;
		}

		CBofPalette *pPalette = CBofApp::GetApp()->GetPalette();

		//
		// This function needs to be optimized to paint only the section that is
		// invalidated.  Right now it just repaints the entire scroll bar each time.
		//

		if ((_pBackdrop != nullptr) && (m_pThumb != nullptr)) {
			// Do all painting offscreen
			CBofBitmap *pBmp = new CBofBitmap(m_cBkSize.cx, m_cBkSize.cy, pPalette);
			if (pBmp != nullptr) {
				_pBackdrop->paint(pBmp, 0, 0, nullptr, COLOR_WHITE);

				if ((m_nScrollState == 1) && (m_pLeftBtnDn != nullptr)) {
					cPoint = m_cLeftBtnRect.TopLeft();
					m_pLeftBtnDn->paint(pBmp, cPoint.x, cPoint.y, nullptr, COLOR_WHITE);

				} else if (m_pLeftBtnUp != nullptr) {
					cPoint = m_cLeftBtnRect.TopLeft();
					m_pLeftBtnUp->paint(pBmp, cPoint.x, cPoint.y, nullptr, COLOR_WHITE);
				}

				if ((m_nScrollState == 4) && (m_pRightBtnDn != nullptr)) {
					cPoint = m_cRightBtnRect.TopLeft();
					m_pRightBtnDn->paint(pBmp, cPoint.x, cPoint.y, nullptr, COLOR_WHITE);

				} else if (m_pRightBtnUp != nullptr) {
					cPoint = m_cRightBtnRect.TopLeft();
					m_pRightBtnUp->paint(pBmp, cPoint.x, cPoint.y, nullptr, COLOR_WHITE);
				}

				m_cThumbPos.x = (int)(((int32)(m_nScrollWidth - m_cThumbSize.cx) * m_nPos) / (m_nRange - 1)) + m_nOffset;
				m_cThumbPos.y = (int)(m_cBkSize.cy / 2) - (int)(m_cThumbSize.cy / 2);

				if (m_cThumbPos.x < 0)
					m_cThumbPos.x = 0;
				if (m_cThumbPos.x > (m_nScrollWidth - m_cThumbSize.cx + m_nOffset))
					m_cThumbPos.x = m_nScrollWidth - m_cThumbSize.cx + m_nOffset;

				m_pThumb->PaintSprite(pBmp, m_cThumbPos);

				// now we can paint the offscreen buffer to the screen
				pBmp->paint(this, 0, 0);

				delete pBmp;
			} else {
				ReportError(ERR_MEMORY, "Could not allocate a new CBofBitmap(%d x %d))", m_cBkSize.cx, m_cBkSize.cy);
			}
		}

		if ((m_pScrollText != nullptr) && (_parent != nullptr)) {
			m_pScrollText->Display(_parent, m_szScrollText, FONT_DEFAULT_SIZE, TEXT_DEFAULT_FACE);
		}
	}

	return _errCode;
}


void CBofScrollBar::onLButtonDown(uint32 nFlags, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));

	CBofRect cLeftPageRect, cRightPageRect;

	bool bDoNothing = false;

	cLeftPageRect.SetRect(m_nOffset, 0, (m_nScrollWidth / m_nRange) * m_nPos + m_nOffset - 1, m_cBkSize.cy - 1);
	cRightPageRect.SetRect(((m_nScrollWidth / m_nRange) * m_nPos) + m_nOffset + m_cThumbSize.cx, 0, m_nOffset + m_nScrollWidth - 1, m_cBkSize.cy - 1);

	m_cCurPoint = *pPoint;

	if (m_pLeftBtnUp != nullptr && m_cLeftBtnRect.PtInRect(*pPoint)) {
		// Let timer know what happened
		m_nScrollState = 1;

		// Set new thumb position
		SetPos(m_nPos - m_nLineDelta, true);

	} else if (m_pThumb->GetRect().PtInRect(*pPoint)) {
		m_nScrollState = 5;

	} else if (cLeftPageRect.PtInRect(*pPoint)) {
		m_nScrollState = 2;

		// Set new thumb position
		SetPos(m_nPos - m_nPageDelta, true);

	} else if (cRightPageRect.PtInRect(*pPoint)) {
		m_nScrollState = 3;

		// Set new thumb position
		SetPos(m_nPos + m_nPageDelta, true);

	} else if (m_pRightBtnUp != nullptr && m_cRightBtnRect.PtInRect(*pPoint)) {
		// Let timer know what happened
		m_nScrollState = 4;

		// Set new thumb position
		SetPos(m_nPos + m_nLineDelta, true);

	} else {
		bDoNothing = true;
	}

	if (!bDoNothing) {
		m_bMouseCaptured = true;
		SetCapture();
		if (m_nScrollState != 5)
			SetTimer(BMP_SCROLL_TIMER, m_nTimerCount);
	}

	CBofWindow::onLButtonDown(nFlags, pPoint);
}


int CBofScrollBar::PointToPos(CBofPoint *pPoint) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	int nPos = m_nPos;

	if (_cRect.PtInRect(*pPoint)) {
		nPos = (pPoint->x - m_nOffset) / (int)(m_nScrollWidth / m_nRange);
	}

	return nPos;
}


void CBofScrollBar::onLButtonUp(uint32 nFlags, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));

	if (m_bMouseCaptured) {
		KillTimer(BMP_SCROLL_TIMER);
		m_bMouseCaptured = false;
		ReleaseCapture();

		int x, y;

		switch (m_nScrollState) {
		case 5:
			SetPos(PointToPos(pPoint));
			break;

		case 1:
			if (m_pLeftBtnUp != nullptr) {
				x = 0;
				y = (int)(m_cBkSize.cy / 2) - (int)(m_cLeftBtnRect.Height() / 2);
				m_pLeftBtnUp->paint(this, x, y, nullptr, COLOR_WHITE);
			}
			break;

		case 4:
			if (m_pRightBtnUp != nullptr) {
				x = m_cBkSize.cx - m_cRightBtnRect.Width();
				y = (int)(m_cBkSize.cy / 2) - (int)(m_cRightBtnRect.Height() / 2);
				m_pRightBtnUp->paint(this, x, y, nullptr, COLOR_WHITE);
			}
			break;

		default:
			break;
		}

		m_nScrollState = 0;
	}

	CBofWindow::onLButtonUp(nFlags, pPoint);
}


void CBofScrollBar::onMouseMove(uint32 nFlags, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));

	if (m_bMouseCaptured) {
		m_cCurPoint = *pPoint;

		if (m_nScrollState == 5) {
			SetPos(PointToPos(pPoint));
		}
	}

	CBofWindow::onMouseMove(nFlags, pPoint);
}


void CBofScrollBar::SetRepeatTimer(uint32 nTimerInt) {
	Assert(IsValidObject(this));

	m_nTimerCount = nTimerInt;
}


void CBofScrollBar::onTimer(uint32 nWhichTimer) {
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
