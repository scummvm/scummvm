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

#include "bagel/boflib/gui/button.h"
#include "bagel/boflib/app.h"
#include "bagel/boflib/gfx/text.h"

namespace Bagel {

#define SELECTED_TEXT_OFFSET_DX 1
#define SELECTED_TEXT_OFFSET_DY 1

#define BUTTON_TEXT_SIZE        10

#define CHECK_BOX_SIZE          14
#define CHECK_BOX_OFFSET_DX      4
#define CHECK_BOX_OFFSET_DY      4
#define CHECK_BOX_TEXT_SIZE     10

#define RADIO_BOX_SIZE          13
#define RADIO_BOX_OFFSET_DX      3
#define RADIO_BOX_OFFSET_DY      3
#define RADIO_BOX_TEXT_SIZE     10


static ST_COLORSCHEME g_stDefaultColors = {

	RGB(171, 151, 127),
	RGB(207, 199, 183),
	RGB(131, 111, 91),
	RGB(0, 0, 0),
	RGB(0, 0, 0),
	RGB(0, 0, 0)
};

static ST_COLORSCHEME g_stGrayColors = {

	RGB(199, 167, 139),
	RGB(223, 199, 175),
	RGB(155, 123, 95),
	RGB(51, 31, 19),
	RGB(155, 139, 123),
	RGB(83, 59, 51)
};


CBofButton::CBofButton() {
	// Inits
	m_nState = BUTTON_UP;

	// load a default color scheme until another is loaded
	LoadColorScheme(&g_stDefaultColors);
}


CBofButton::CBofButton(ST_COLORSCHEME *pColorScheme) {
	Assert(pColorScheme != nullptr);

	// Inits
	m_nState = BUTTON_UP;

	LoadColorScheme(pColorScheme);
}


CBofButton::~CBofButton() {
	Assert(IsValidObject(this));
}


ERROR_CODE CBofButton::Paint(CBofRect *) {
	Assert(IsValidObject(this));

	// only continue if this button is visible
	//
	if (IsVisible() && (m_pParentWnd != nullptr) && m_pParentWnd->IsVisible()) {

		CBofPalette *pPalette;
		RGBCOLOR cTextColor;
		UBYTE iHighlight, iShadow, iTemp;
		INT nWidth, nHeight;
		INT i, left, right, top, bottom;

		pPalette = CBofApp::GetApp()->GetPalette();

		nWidth = m_cRect.Width();
		nHeight = m_cRect.Height();

		// create our off-screen buffer
		//
		CBofBitmap cBmp(nWidth, nHeight, pPalette);

		cBmp.FillRect(&m_cRect, pPalette->GetNearestIndex(m_cFaceColor));

		left = m_cRect.left;
		right = m_cRect.right;
		top = m_cRect.top;
		bottom = m_cRect.bottom;

		iShadow = pPalette->GetNearestIndex(m_cShadowColor);
		iHighlight = pPalette->GetNearestIndex(m_cHighlightColor);
		if (m_nState == BUTTON_DOWN) {
			iTemp = iShadow;
			iShadow = iHighlight;
			iHighlight = iTemp;
		}

		// jwl 07.10.96 If we're doing a copybits compatible version, then
		// we have to draw our buttons as they appear on the screen, i.e. they
		// will not be inverted when drawn (they've "top down").
#if COPYBITS
		UBYTE c1 = iHighlight;
		UBYTE c2 = iShadow;
#else
		UBYTE c1 = iShadow;
		UBYTE c2 = iHighlight;
#endif
		for (i = 1; i <= 3; i++) {
			cBmp.Line(left + i, bottom - i, right - i, bottom - i, c1);
			cBmp.Line(right - i, bottom - i, right - i, top + i - 1, c1);
		}

		for (i = 1; i <= 3; i++) {
			cBmp.Line(left + i, bottom - i, left + i, top + i - 1, c2);
			cBmp.Line(left + i, top + i - 1, right - i, top + i - 1, c2);
		}

		cBmp.DrawRect(&m_cRect, pPalette->GetNearestIndex(m_cOutlineColor));

		// Create a temporary text object
		//
		CBofRect cTempRect(3, 3, m_cRect.right - 3, m_cRect.bottom - 3);
		if (m_nState == BUTTON_DOWN) {
			cTempRect += CBofPoint(1, 1);
		}

		CBofText cText(&cTempRect);

		// Print text into button
		//
		cTextColor = m_cTextColor;
		if (m_nState == BUTTON_DISABLED)
			cTextColor = m_cTextDisabledColor;

		cText.Display(&cBmp, m_szTitle, BUTTON_TEXT_SIZE, TEXT_NORMAL, cTextColor);

		if (m_nState == BUTTON_FOCUS) {
			cBmp.DrawRect(&cTempRect, pPalette->GetNearestIndex(m_cOutlineColor));
		}

		// now we can update the window
		cBmp.Paint(this, 0, 0);
	}

	return (m_errCode);
}


VOID CBofButton::LoadColorScheme(ST_COLORSCHEME *pColorScheme) {
	Assert(IsValidObject(this));

	Assert(pColorScheme != nullptr);

	// save all of the color info we need to build a button
	//
	m_cFaceColor = pColorScheme->m_cFace;
	m_cHighlightColor = pColorScheme->m_cHighlight;
	m_cShadowColor = pColorScheme->m_cShadow;
	m_cTextColor = pColorScheme->m_cText;
	m_cTextDisabledColor = pColorScheme->m_cTextDisabled;
	m_cOutlineColor = pColorScheme->m_cOutline;
}


VOID CBofButton::Enable() {
	Assert(IsValidObject(this));

	CBofWindow::Enable();

	SetState(BUTTON_UP);
}


VOID CBofButton::Disable() {
	Assert(IsValidObject(this));

	SetState(BUTTON_DISABLED);

	CBofWindow::Disable();
}


ERROR_CODE CBofButton::SetState(INT nNewState, BOOL bRepaintNow) {
	Assert(IsValidObject(this));
	Assert(nNewState >= BUTTON_UP && nNewState <= BUTTON_DISABLED);

	INT nOldState;

	// remember last button state
	nOldState = m_nState;

	m_nState = nNewState;

	// update the window if forced to or if button state has changed
	//
	if (bRepaintNow || (nOldState != nNewState)) {
		Paint();
	}

	// I MUST have a valid parent
	Assert(m_pParentWnd != nullptr);

	// tell parent the new state of this button
	//
	if (m_pParentWnd != nullptr) {
		m_pParentWnd->OnBofButton(this, m_nState);
	}

	return (m_errCode);
}


VOID CBofButton::OnPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));

	Assert(pRect != nullptr);

	Paint(pRect);
}


VOID CBofButton::OnLButtonDown(UINT, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	if (!m_bCaptured && m_nState != BUTTON_DISABLED) {

		SetCapture();

		SetState(BUTTON_DOWN, TRUE);
	}
}


VOID CBofButton::OnLButtonUp(UINT, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	if (m_bCaptured) {

		ReleaseCapture();

		SetState(BUTTON_UP, TRUE);

		if (m_cRect.PtInRect(*pPoint) && (m_pParentWnd != nullptr)) {
			m_pParentWnd->OnBofButton(this, BUTTON_CLICKED);
		}
	}
}

/*****************************************************************************/
/*                           CBofRadioButton                                 */
/*****************************************************************************/


VOID CBofRadioButton::OnLButtonDown(UINT, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	if ((m_nState != BUTTON_DISABLED) && (m_nState == BUTTON_UP)) {

		SetState(BUTTON_DOWN, TRUE);
	}
}


VOID CBofRadioButton::OnLButtonUp(UINT, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);
}


ERROR_CODE CBofRadioButton::Paint(CBofRect *) {
	Assert(IsValidObject(this));

	// only continue if this button is visible
	//
	if (IsVisible() && (m_pParentWnd != nullptr) && m_pParentWnd->IsVisible()) {

		CBofPalette *pPalette;
		RGBCOLOR cTextColor;
		UBYTE iHighlight, iShadow;
		INT nWidth, nHeight, x, y, nRadius;

		pPalette = CBofApp::GetApp()->GetPalette();

		nWidth = m_cRect.Width();
		nHeight = m_cRect.Height();

		// create a temporary off-screen buffer
		//
		CBofBitmap cBmp(nWidth, nHeight, pPalette);

		// fill in the background color
		cBmp.FillRect(&m_cRect, pPalette->GetNearestIndex(m_cFaceColor));

		cTextColor = m_cTextColor;
		if (m_nState == BUTTON_DISABLED)
			cTextColor = m_cTextDisabledColor;

		iShadow = pPalette->GetNearestIndex(cTextColor);
		iHighlight = pPalette->GetNearestIndex(cTextColor);

		// paint the radio button circle
		nRadius = 7;
		x = nRadius + RADIO_BOX_OFFSET_DX;
		y = nHeight / 2;
		cBmp.Circle(x, y, (USHORT)nRadius, iShadow);
		nRadius--;
		cBmp.Circle(x, y, (USHORT)nRadius, iShadow);

		// Create a temporary text object
		//
		CBofRect cTempRect(20, RADIO_BOX_OFFSET_DY, m_cRect.right, m_cRect.bottom - RADIO_BOX_OFFSET_DY);
		if (m_nState == BUTTON_DOWN) {

			// Should do a cBmp.FillCircle(x, y, nRadius, iHighlight) here instead, but that function is not finished
			//
			nRadius = 1;
			cBmp.Circle(x, y, (USHORT)nRadius, iHighlight);
			nRadius = 2;
			cBmp.Circle(x, y, (USHORT)nRadius, iHighlight);
			nRadius = 3;
			cBmp.Circle(x, y, (USHORT)nRadius, iHighlight);
			nRadius = 4;
			cBmp.Circle(x, y, (USHORT)nRadius, iHighlight);
		}
		CBofText cText(&cTempRect, JUSTIFY_LEFT);

		// put a box around the whole button
		cBmp.DrawRect(&m_cRect, pPalette->GetNearestIndex(m_cOutlineColor));

		// show text disabled if button is disabled
		//

		// Print text into button
		//
		cText.Display(&cBmp, m_szTitle, RADIO_BOX_TEXT_SIZE, TEXT_NORMAL, cTextColor);

		// if button has focus, then put a box around the text
		//
		if (m_nState == BUTTON_FOCUS) {
			cBmp.DrawRect(&cTempRect, pPalette->GetNearestIndex(m_cOutlineColor));
		}

		// now we can update the window
		cBmp.Paint(this, 0, 0);
	}

	return (m_errCode);
}


/*****************************************************************************/
/*                           CBofCheckButton                                 */
/*****************************************************************************/


VOID CBofCheckButton::OnLButtonDown(UINT, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	if (m_nState != BUTTON_DISABLED) {

		if (m_nState == BUTTON_UP) {

			SetState(BUTTON_DOWN, TRUE);

		} else if (m_nState == BUTTON_DOWN) {

			SetState(BUTTON_UP, TRUE);
		}
	}
}


VOID CBofCheckButton::OnLButtonUp(UINT, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	// Do nothing, and don't call the base class version of this function
}


ERROR_CODE CBofCheckButton::Paint(CBofRect *) {
	Assert(IsValidObject(this));

	// only continue if this button is visible
	//
	if (IsVisible() && (m_pParentWnd != nullptr) && m_pParentWnd->IsVisible()) {

		CBofPalette *pPalette;
		RGBCOLOR cTextColor;
		UBYTE iHighlight, iShadow;
		INT nWidth, nHeight, y;

		pPalette = CBofApp::GetApp()->GetPalette();

		nWidth = m_cRect.Width();
		nHeight = m_cRect.Height();

		// create a temporary off-screen buffer
		//
		CBofBitmap cBmp(nWidth, nHeight, pPalette);

		// fill in the background color
		cBmp.FillRect(&m_cRect, pPalette->GetNearestIndex(m_cFaceColor));

		// show text disabled if button is disabled
		//
		cTextColor = m_cTextColor;
		if (m_nState == BUTTON_DISABLED)
			cTextColor = m_cTextDisabledColor;

		iShadow = pPalette->GetNearestIndex(cTextColor);
		iHighlight = pPalette->GetNearestIndex(cTextColor);

		// draw the check box (centered vertically)
		//
		y = ((nHeight - CHECK_BOX_SIZE) / 2);
		CBofRect cTempRect(CHECK_BOX_OFFSET_DX, y, CHECK_BOX_SIZE + CHECK_BOX_OFFSET_DX - 1, y + CHECK_BOX_SIZE - 1);
		cBmp.DrawRect(&cTempRect, iShadow);

		// if button is in DOWN/ON state, then put an X in the box
		//
		if (m_nState == BUTTON_DOWN) {
			cBmp.Line(cTempRect.left, cTempRect.top, cTempRect.right, cTempRect.bottom, iShadow);
			cBmp.Line(cTempRect.left, cTempRect.bottom, cTempRect.right, cTempRect.top, iShadow);
		}

		// Create a temporary text object
		//
		cTempRect.SetRect(CHECK_BOX_SIZE + CHECK_BOX_OFFSET_DX, CHECK_BOX_OFFSET_DX, m_cRect.right, m_cRect.bottom - CHECK_BOX_OFFSET_DX);
		CBofText cText(&cTempRect, JUSTIFY_LEFT);

		// put a box around the whole button
		cBmp.DrawRect(&m_cRect, pPalette->GetNearestIndex(m_cOutlineColor));

		// Print text into button
		//
		cText.Display(&cBmp, m_szTitle, CHECK_BOX_TEXT_SIZE, TEXT_NORMAL, cTextColor);

		// if button has focus, then put a box around the text
		//
		if (m_nState == BUTTON_FOCUS) {
			cBmp.DrawRect(&cTempRect, pPalette->GetNearestIndex(m_cOutlineColor));
		}

		// now we can update the window
		cBmp.Paint(this, 0, 0);
	}

	return (m_errCode);
}


ERROR_CODE CBofCheckButton::SetCheck(BOOL bChecked) {
	Assert(IsValidObject(this));

	SetState(bChecked ? BUTTON_CHECKED : BUTTON_UNCHECKED, FALSE);

	return (m_errCode);
}


/*****************************************************************************/
/*                           CBofBmpButton                                   */
/*****************************************************************************/


CBofBmpButton::CBofBmpButton() {
	m_pButtonUp = nullptr;
	m_pButtonDown = nullptr;
	m_pButtonFocus = nullptr;
	m_pButtonDisabled = nullptr;
	m_pBackground = nullptr;
	m_nState = BUTTON_UP;
	m_nMaskColor = NOT_TRANSPARENT;
}


CBofBmpButton::~CBofBmpButton() {
	if (m_pButtonUp != nullptr) {
		delete m_pButtonUp;
		m_pButtonUp = nullptr;
	}
	if (m_pButtonDown != nullptr) {
		delete m_pButtonDown;
		m_pButtonDown = nullptr;
	}
	if (m_pButtonDisabled != nullptr) {
		delete m_pButtonDisabled;
		m_pButtonDisabled = nullptr;
	}
	if (m_pButtonFocus != nullptr) {
		delete m_pButtonFocus;
		m_pButtonFocus = nullptr;
	}
	if (m_pBackground != nullptr) {
		delete m_pBackground;
		m_pBackground = nullptr;
	}
}


ERROR_CODE CBofBmpButton::Paint(CBofRect *) {
	Assert(IsValidObject(this));

	// You must call LoadBitmaps before you can paint this button
	//
	Assert(m_pButtonUp != nullptr);
	Assert(m_pButtonDown != nullptr);
	Assert(m_pButtonFocus != nullptr);
	Assert(m_pButtonDisabled != nullptr);

	// only continue if this button is visible
	//
	if (IsVisible() && (m_pParentWnd != nullptr) && m_pParentWnd->IsVisible()) {

		CBofBitmap *pBitmap;
		CBofPalette *pPalette;
		INT nWidth, nHeight;

		pPalette = m_pButtonUp->GetPalette();

		nWidth = m_cRect.Width();
		nHeight = m_cRect.Height();

		// do all painting off-screen
		//
		CBofBitmap cOffScreen(nWidth, nHeight, pPalette);

		if (m_pBackground == nullptr) {
			if ((m_pBackground = new CBofBitmap(nWidth, nHeight, pPalette)) != nullptr) {

			} else {
				ReportError(ERR_MEMORY, "Could not allocate a CBofBitmap(%d x %d)", nWidth, nHeight);
			}

		} else {
			m_pBackground->Paint(&cOffScreen, 0, 0);
		}

		// assume UP state
		pBitmap = m_pButtonUp;

		// display the correct bitmap based on state
		//
		if (m_nState == BUTTON_DOWN) {

			pBitmap = m_pButtonDown;

		} else if (m_nState == BUTTON_FOCUS) {

			pBitmap = m_pButtonFocus;

		} else if (m_nState == BUTTON_DISABLED) {

			pBitmap = m_pButtonDisabled;
		}

		// paint button offscreen
		pBitmap->Paint(&cOffScreen, 0, 0, nullptr, m_nMaskColor);

		// now we can update the window
		cOffScreen.Paint(this, 0, 0);
	}

	return (m_errCode);
}


ERROR_CODE CBofBmpButton::LoadBitmaps(CBofBitmap *pUp, CBofBitmap *pDown, CBofBitmap *pFocus, CBofBitmap *pDisabled, INT nMaskColor) {
	Assert(IsValidObject(this));

	Assert(pUp != nullptr);
	Assert(pDown != nullptr);

	// use the bitmaps passed in
	m_pButtonUp = pUp;
	m_pButtonDown = pDown;
	m_pButtonFocus = pFocus;
	m_pButtonDisabled = pDisabled;

	// remember the transparent color for these bitmaps
	m_nMaskColor = nMaskColor;

	return (m_errCode);
}


ERROR_CODE CBofBmpButton::LoadBitmaps(CBofPalette *pPalette, const CHAR *pszUp, const CHAR *pszDown, const CHAR *pszFocus, const CHAR *pszDisabled, INT nMaskColor) {
	Assert(IsValidObject(this));

	Assert(pPalette != nullptr);
	Assert(pszUp != nullptr);
	Assert(pszDown != nullptr);
	Assert(pszFocus != nullptr);
	Assert(pszDisabled != nullptr);

	// remember the button transparent color
	m_nMaskColor = nMaskColor;

	// load each of the bitmaps that represent the button state
	//
	if ((m_pButtonUp = new CBofBitmap(pszUp, pPalette)) != nullptr) {
		m_pButtonUp->SetReadOnly(TRUE);
	}
	if ((m_pButtonDown = new CBofBitmap(pszDown, pPalette)) != nullptr) {
		m_pButtonDown->SetReadOnly(TRUE);
	}
	if ((m_pButtonFocus = new CBofBitmap(pszFocus, pPalette)) != nullptr) {
		m_pButtonFocus->SetReadOnly(TRUE);
	}
	if ((m_pButtonDisabled = new CBofBitmap(pszDisabled, pPalette)) != nullptr) {
		m_pButtonDisabled->SetReadOnly(TRUE);
	}

	return (m_errCode);
}


ERROR_CODE CBofBmpButton::SetState(INT nNewState, BOOL bRepaintNow) {
	Assert(IsValidObject(this));
	Assert(nNewState >= BUTTON_UP && nNewState <= BUTTON_DISABLED);

	INT nOldState;

	// remember last button state
	nOldState = m_nState;

	m_nState = nNewState;

	// update the window if forced to or if button state has changed
	//
	if (bRepaintNow || (nOldState != nNewState)) {
		Paint();
	}

	// I MUST have a valid parent
	Assert(m_pParentWnd != nullptr);

	// tell parent the new state of this button
	//
	if (m_pParentWnd != nullptr) {
		m_pParentWnd->OnBofButton(this, m_nState);
	}

	return (m_errCode);
}


VOID CBofBmpButton::OnPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));

	Assert(pRect != nullptr);

	Paint(pRect);
}


VOID CBofBmpButton::OnLButtonDown(UINT, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	if (!m_bCaptured && m_nState != BUTTON_DISABLED) {

		SetCapture();

		SetState(BUTTON_DOWN, TRUE);
	}
}


VOID CBofBmpButton::OnLButtonUp(UINT, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	if (m_bCaptured) {

		ReleaseCapture();

		SetState(BUTTON_UP, TRUE);

		if (m_cRect.PtInRect(*pPoint) && (m_pParentWnd != nullptr)) {
			m_pParentWnd->OnBofButton(this, BUTTON_CLICKED);
		}
	}
}

} // namespace Bagel
