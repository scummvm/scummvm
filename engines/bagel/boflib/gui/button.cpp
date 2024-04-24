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

CBofButton::CBofButton() {
	// Inits
	m_nState = BUTTON_UP;

	// Load a default color scheme until another is loaded
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


ErrorCode CBofButton::Paint(CBofRect *) {
	Assert(IsValidObject(this));

	// Only continue if this button is visible
	if (IsVisible() && (_parent != nullptr) && _parent->IsVisible()) {
		CBofPalette *pPalette;
		RGBCOLOR cTextColor;
		byte iHighlight, iShadow, iTemp;
		int nWidth, nHeight;
		int i, left, right, top, bottom;

		pPalette = CBofApp::GetApp()->GetPalette();

		nWidth = m_cRect.Width();
		nHeight = m_cRect.Height();

		// Create our off-screen buffer
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

		byte c1 = iShadow;
		byte c2 = iHighlight;

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
		CBofRect cTempRect(3, 3, m_cRect.right - 3, m_cRect.bottom - 3);

		if (m_nState == BUTTON_DOWN) {
			cTempRect += CBofPoint(1, 1);
		}

		CBofText cText(&cTempRect);

		// Print text into button
		cTextColor = m_cTextColor;
		if (m_nState == BUTTON_DISABLED)
			cTextColor = m_cTextDisabledColor;

		cText.Display(&cBmp, m_szTitle, BUTTON_TEXT_SIZE, TEXT_NORMAL, cTextColor);

		if (m_nState == BUTTON_FOCUS) {
			cBmp.DrawRect(&cTempRect, pPalette->GetNearestIndex(m_cOutlineColor));
		}

		// Now we can update the window
		cBmp.Paint(this, 0, 0);
	}

	return m_errCode;
}


void CBofButton::LoadColorScheme(ST_COLORSCHEME *pColorScheme) {
	Assert(IsValidObject(this));

	Assert(pColorScheme != nullptr);

	// Save all of the color info we need to build a button
	m_cFaceColor = pColorScheme->m_cFace;
	m_cHighlightColor = pColorScheme->m_cHighlight;
	m_cShadowColor = pColorScheme->m_cShadow;
	m_cTextColor = pColorScheme->m_cText;
	m_cTextDisabledColor = pColorScheme->m_cTextDisabled;
	m_cOutlineColor = pColorScheme->m_cOutline;
}


void CBofButton::Enable() {
	Assert(IsValidObject(this));

	CBofWindow::Enable();

	SetState(BUTTON_UP);
}


void CBofButton::Disable() {
	Assert(IsValidObject(this));

	SetState(BUTTON_DISABLED);

	CBofWindow::Disable();
}


ErrorCode CBofButton::SetState(int nNewState, bool bRepaintNow) {
	Assert(IsValidObject(this));
	Assert(nNewState >= BUTTON_UP && nNewState <= BUTTON_DISABLED);

	// Remember last button state
	int nOldState = m_nState;

	m_nState = nNewState;

	// Update the window if forced to or if button state has changed
	if (bRepaintNow || (nOldState != nNewState)) {
		Paint();
	}

	// I must have a valid parent
	Assert(_parent != nullptr);

	// Tell parent the new state of this button
	if (_parent != nullptr) {
		_parent->OnBofButton(this, m_nState);
	}

	return m_errCode;
}


void CBofButton::OnPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));

	Assert(pRect != nullptr);

	Paint(pRect);
}


void CBofButton::OnLButtonDown(uint32, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	if (!m_bCaptured && m_nState != BUTTON_DISABLED) {

		SetCapture();

		SetState(BUTTON_DOWN, true);
	}
}


void CBofButton::OnLButtonUp(uint32, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	if (m_bCaptured) {
		ReleaseCapture();

		SetState(BUTTON_UP, true);

		if (m_cRect.PtInRect(*pPoint) && (_parent != nullptr)) {
			_parent->OnBofButton(this, BUTTON_CLICKED);
		}
	}
}

/*****************************************************************************/
/*                           CBofRadioButton                                 */
/*****************************************************************************/


void CBofRadioButton::OnLButtonDown(uint32, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	if ((m_nState != BUTTON_DISABLED) && (m_nState == BUTTON_UP)) {

		SetState(BUTTON_DOWN, true);
	}
}


void CBofRadioButton::OnLButtonUp(uint32, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);
}


ErrorCode CBofRadioButton::Paint(CBofRect *) {
	Assert(IsValidObject(this));

	// Only continue if this button is visible
	if (IsVisible() && (_parent != nullptr) && _parent->IsVisible()) {

		CBofPalette *pPalette = CBofApp::GetApp()->GetPalette();

		int nWidth = m_cRect.Width();
		int nHeight = m_cRect.Height();

		// Create a temporary off-screen buffer
		CBofBitmap cBmp(nWidth, nHeight, pPalette);

		// Fill in the background color
		cBmp.FillRect(&m_cRect, pPalette->GetNearestIndex(m_cFaceColor));

		RGBCOLOR cTextColor = m_cTextColor;
		if (m_nState == BUTTON_DISABLED)
			cTextColor = m_cTextDisabledColor;

		byte iShadow = pPalette->GetNearestIndex(cTextColor);
		byte iHighlight = pPalette->GetNearestIndex(cTextColor);

		// Paint the radio button circle
		int nRadius = 7;
		int x = nRadius + RADIO_BOX_OFFSET_DX;
		int y = nHeight / 2;
		cBmp.Circle(x, y, (uint16)nRadius, iShadow);
		nRadius--;
		cBmp.Circle(x, y, (uint16)nRadius, iShadow);

		// Create a temporary text object
		CBofRect cTempRect(20, RADIO_BOX_OFFSET_DY, m_cRect.right, m_cRect.bottom - RADIO_BOX_OFFSET_DY);
		if (m_nState == BUTTON_DOWN) {
			nRadius = 1;
			cBmp.Circle(x, y, (uint16)nRadius, iHighlight);
			nRadius = 2;
			cBmp.Circle(x, y, (uint16)nRadius, iHighlight);
			nRadius = 3;
			cBmp.Circle(x, y, (uint16)nRadius, iHighlight);
			nRadius = 4;
			cBmp.Circle(x, y, (uint16)nRadius, iHighlight);
		}
		CBofText cText(&cTempRect, JUSTIFY_LEFT);

		// Put a box around the whole button
		cBmp.DrawRect(&m_cRect, pPalette->GetNearestIndex(m_cOutlineColor));

		// Show text disabled if button is disabled

		// Print text into button
		cText.Display(&cBmp, m_szTitle, RADIO_BOX_TEXT_SIZE, TEXT_NORMAL, cTextColor);

		// If button has focus, then put a box around the text
		if (m_nState == BUTTON_FOCUS) {
			cBmp.DrawRect(&cTempRect, pPalette->GetNearestIndex(m_cOutlineColor));
		}

		// Now we can update the window
		cBmp.Paint(this, 0, 0);
	}

	return m_errCode;
}


/*****************************************************************************/
/*                           CBofCheckButton                                 */
/*****************************************************************************/


void CBofCheckButton::OnLButtonDown(uint32, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	if (m_nState != BUTTON_DISABLED) {

		if (m_nState == BUTTON_UP) {

			SetState(BUTTON_DOWN, true);

		} else if (m_nState == BUTTON_DOWN) {

			SetState(BUTTON_UP, true);
		}
	}
}


void CBofCheckButton::OnLButtonUp(uint32, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	// Do nothing, and don't call the base class version of this function
}


ErrorCode CBofCheckButton::Paint(CBofRect *) {
	Assert(IsValidObject(this));

	// Only continue if this button is visible
	if (IsVisible() && (_parent != nullptr) && _parent->IsVisible()) {

		CBofPalette *pPalette = CBofApp::GetApp()->GetPalette();

		int nWidth = m_cRect.Width();
		int nHeight = m_cRect.Height();

		// Create a temporary off-screen buffer
		CBofBitmap cBmp(nWidth, nHeight, pPalette);

		// Fill in the background color
		cBmp.FillRect(&m_cRect, pPalette->GetNearestIndex(m_cFaceColor));

		// Show text disabled if button is disabled
		RGBCOLOR cTextColor = m_cTextColor;
		if (m_nState == BUTTON_DISABLED)
			cTextColor = m_cTextDisabledColor;

		byte iShadow = pPalette->GetNearestIndex(cTextColor);

		// Draw the check box (centered vertically)
		int y = ((nHeight - CHECK_BOX_SIZE) / 2);
		CBofRect cTempRect(CHECK_BOX_OFFSET_DX, y, CHECK_BOX_SIZE + CHECK_BOX_OFFSET_DX - 1, y + CHECK_BOX_SIZE - 1);
		cBmp.DrawRect(&cTempRect, iShadow);

		// if button is in DOWN/ON state, then put an X in the box
		//
		if (m_nState == BUTTON_DOWN) {
			cBmp.Line(cTempRect.left, cTempRect.top, cTempRect.right, cTempRect.bottom, iShadow);
			cBmp.Line(cTempRect.left, cTempRect.bottom, cTempRect.right, cTempRect.top, iShadow);
		}

		// Create a temporary text object
		cTempRect.SetRect(CHECK_BOX_SIZE + CHECK_BOX_OFFSET_DX, CHECK_BOX_OFFSET_DX, m_cRect.right, m_cRect.bottom - CHECK_BOX_OFFSET_DX);
		CBofText cText(&cTempRect, JUSTIFY_LEFT);

		// Put a box around the whole button
		cBmp.DrawRect(&m_cRect, pPalette->GetNearestIndex(m_cOutlineColor));

		// Print text into button
		//
		cText.Display(&cBmp, m_szTitle, CHECK_BOX_TEXT_SIZE, TEXT_NORMAL, cTextColor);

		// If button has focus, then put a box around the text
		if (m_nState == BUTTON_FOCUS) {
			cBmp.DrawRect(&cTempRect, pPalette->GetNearestIndex(m_cOutlineColor));
		}

		// Now we can update the window
		cBmp.Paint(this, 0, 0);
	}

	return m_errCode;
}


ErrorCode CBofCheckButton::SetCheck(bool bChecked) {
	Assert(IsValidObject(this));

	SetState(bChecked ? BUTTON_CHECKED : BUTTON_UNCHECKED, false);

	return m_errCode;
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


ErrorCode CBofBmpButton::Paint(CBofRect *) {
	Assert(IsValidObject(this));

	// LoadBitmaps must be called before the button can be painted
	Assert(m_pButtonUp != nullptr);
	Assert(m_pButtonDown != nullptr);
	Assert(m_pButtonFocus != nullptr);
	Assert(m_pButtonDisabled != nullptr);

	// Only continue if this button is visible
	if (IsVisible() && (_parent != nullptr) && _parent->IsVisible()) {
		CBofPalette *pPalette = m_pButtonUp->GetPalette();

		int nWidth = m_cRect.Width();
		int nHeight = m_cRect.Height();

		// Do all painting off-screen
		CBofBitmap cOffScreen(nWidth, nHeight, pPalette);

		if (m_pBackground == nullptr) {
			if ((m_pBackground = new CBofBitmap(nWidth, nHeight, pPalette)) != nullptr) {

			} else {
				ReportError(ERR_MEMORY, "Could not allocate a CBofBitmap(%d x %d)", nWidth, nHeight);
			}

		} else {
			m_pBackground->Paint(&cOffScreen, 0, 0);
		}

		// Assume UP state
		CBofBitmap *pBitmap = m_pButtonUp;

		// Display the correct bitmap based on state
		if (m_nState == BUTTON_DOWN) {
			pBitmap = m_pButtonDown;

		} else if (m_nState == BUTTON_FOCUS) {
			pBitmap = m_pButtonFocus;

		} else if (m_nState == BUTTON_DISABLED) {
			pBitmap = m_pButtonDisabled;
		}

		// Paint button offscreen
		pBitmap->Paint(&cOffScreen, 0, 0, nullptr, m_nMaskColor);

		// Now we can update the window
		cOffScreen.Paint(this, 0, 0);
	}

	return m_errCode;
}


ErrorCode CBofBmpButton::LoadBitmaps(CBofBitmap *pUp, CBofBitmap *pDown, CBofBitmap *pFocus, CBofBitmap *pDisabled, int nMaskColor) {
	Assert(IsValidObject(this));

	Assert(pUp != nullptr);
	Assert(pDown != nullptr);

	// Use the bitmaps passed in
	m_pButtonUp = pUp;
	m_pButtonDown = pDown;
	m_pButtonFocus = pFocus;
	m_pButtonDisabled = pDisabled;

	// Remember the transparent color for these bitmaps
	m_nMaskColor = nMaskColor;

	return m_errCode;
}


ErrorCode CBofBmpButton::LoadBitmaps(CBofPalette *pPalette, const char *pszUp, const char *pszDown, const char *pszFocus, const char *pszDisabled, int nMaskColor) {
	Assert(IsValidObject(this));

	Assert(pPalette != nullptr);
	Assert(pszUp != nullptr);
	Assert(pszDown != nullptr);
	Assert(pszFocus != nullptr);
	Assert(pszDisabled != nullptr);

	// Remember the button transparent color
	m_nMaskColor = nMaskColor;

	// Load each of the bitmaps that represent the button state
	if ((m_pButtonUp = new CBofBitmap(pszUp, pPalette)) != nullptr) {
		m_pButtonUp->SetReadOnly(true);
	}
	if ((m_pButtonDown = new CBofBitmap(pszDown, pPalette)) != nullptr) {
		m_pButtonDown->SetReadOnly(true);
	}
	if ((m_pButtonFocus = new CBofBitmap(pszFocus, pPalette)) != nullptr) {
		m_pButtonFocus->SetReadOnly(true);
	}
	if ((m_pButtonDisabled = new CBofBitmap(pszDisabled, pPalette)) != nullptr) {
		m_pButtonDisabled->SetReadOnly(true);
	}

	return m_errCode;
}


ErrorCode CBofBmpButton::SetState(int nNewState, bool bRepaintNow) {
	Assert(IsValidObject(this));
	Assert(nNewState >= BUTTON_UP && nNewState <= BUTTON_DISABLED);

	// Remember last button state
	int nOldState = m_nState;

	m_nState = nNewState;

	// Update the window if forced to or if button state has changed
	if (bRepaintNow || (nOldState != nNewState)) {
		Paint();
	}

	// I MUST have a valid parent
	Assert(_parent != nullptr);

	// Tell parent the new state of this button
	if (_parent != nullptr) {
		_parent->OnBofButton(this, m_nState);
	}

	return m_errCode;
}


void CBofBmpButton::OnPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));

	Assert(pRect != nullptr);

	Paint(pRect);
}


void CBofBmpButton::OnLButtonDown(uint32, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	if (!m_bCaptured && m_nState != BUTTON_DISABLED) {
		SetCapture();

		SetState(BUTTON_DOWN, true);
	}
}


void CBofBmpButton::OnLButtonUp(uint32, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	if (m_bCaptured) {
		ReleaseCapture();

		SetState(BUTTON_UP, true);

		if (m_cRect.PtInRect(*pPoint) && (_parent != nullptr)) {
			_parent->OnBofButton(this, BUTTON_CLICKED);
		}
	}
}

} // namespace Bagel
