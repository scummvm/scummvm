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

#include "bagel/boflib/gui/display_button.h"
#include "bagel/boflib/bof_app.h"
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


CBofDisplayButton::CBofDisplayButton(VOID) {
	// Inits
	m_nState = BUTTON_UP;

	// load a default color scheme until another is loaded
	LoadColorScheme(&g_stDefaultColors);
}


CBofDisplayButton::CBofDisplayButton(ST_COLORSCHEME *pColorScheme) {
	Assert(pColorScheme != NULL);

	// Inits
	m_nState = BUTTON_UP;

	LoadColorScheme(pColorScheme);
}


CBofDisplayButton::~CBofDisplayButton() {
	Assert(IsValidObject(this));
}


VOID CBofDisplayButton::LoadColorScheme(ST_COLORSCHEME *pColorScheme) {
	Assert(IsValidObject(this));

	Assert(pColorScheme != NULL);

	// save all of the color info we need to build a button
	//
	m_cFaceColor = pColorScheme->m_cFace;
	m_cHighlightColor = pColorScheme->m_cHighlight;
	m_cShadowColor = pColorScheme->m_cShadow;
	m_cTextColor = pColorScheme->m_cText;
	m_cTextDisabledColor = pColorScheme->m_cTextDisabled;
	m_cOutlineColor = pColorScheme->m_cOutline;
}


VOID CBofDisplayButton::Enable(VOID) {
	Assert(IsValidObject(this));

	CBofDisplayWindow::Enable();

	SetState(BUTTON_UP);
}


VOID CBofDisplayButton::Disable(VOID) {
	Assert(IsValidObject(this));

	SetState(BUTTON_DISABLED);

	CBofDisplayWindow::Disable();
}


ERROR_CODE CBofDisplayButton::SetState(INT nNewState, bool bRepaintNow) {
	Assert(IsValidObject(this));
	Assert(nNewState >= BUTTON_UP && nNewState <= BUTTON_DISABLED);

	INT nOldState;

	// remember last button state
	nOldState = m_nState;

	m_nState = nNewState;

	// update the window if forced to or if button state has changed
	//
	if (bRepaintNow || (nOldState != nNewState)) {
		Paint(m_cPosition);
	}

	// I MUST have a valid parent
	Assert(m_pParent != NULL);

	// tell parent the new state of this button
	//
	if (m_pParent != NULL) {
		m_pParent->OnBofButton(this, m_nState);
	}

	return (m_errCode);
}


ERROR_CODE CBofDisplayButton::OnPaint(CBofBitmap *pDestBmp, CBofRect *pDirtyRect) {
	Assert(IsValidObject(this));
	Assert(pDestBmp != NULL);
	Assert(pDirtyRect != NULL);

	// only continue if this button is visible
	//
	if (!ErrorOccurred()) {

		CBofPalette *pPalette;
		RGBCOLOR cTextColor;
		UBYTE iHighlight, iShadow, iTemp;
		INT nWidth, nHeight;
		INT i, left, right, top, bottom;

		pPalette = CBofApp::GetApp()->GetPalette();

		nWidth = Width();
		nHeight = Height();

		// create our off-screen buffer
		//
		CBofBitmap cBmp(nWidth, nHeight, pPalette);
		CBofRect rect(cBmp.GetRect());

		cBmp.FillRect(&rect, pPalette->GetNearestIndex(m_cFaceColor));

		left = 0;
		right = nWidth;
		top = 0;
		bottom = nHeight;

		iShadow = pPalette->GetNearestIndex(m_cShadowColor);
		iHighlight = pPalette->GetNearestIndex(m_cHighlightColor);
		if (m_nState == BUTTON_DOWN) {
			iTemp = iShadow;
			iShadow = iHighlight;
			iHighlight = iTemp;
		}

		for (i = 1; i <= 3; i++) {
			cBmp.Line(left + i, bottom - i, right - i, bottom - i, iShadow);

			cBmp.Line(right - i, bottom - i, right - i, top + i - 1, iShadow);
		}

		for (i = 1; i <= 3; i++) {
			cBmp.Line(left + i, bottom - i, left + i, top + i - 1, iHighlight);

			cBmp.Line(left + i, top + i - 1, right - i, top + i - 1, iHighlight);
		}

		cBmp.DrawRect(&rect, pPalette->GetNearestIndex(m_cOutlineColor));

		// Create a temporary text object
		//
		CBofRect cTempRect(3, 3, right - 3, bottom - 3);
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

		// Get the area that needs updating (in screen coordinates).
		//
		CBofRect cGlobalRect;
		cGlobalRect = LocalToGlobal(*pDirtyRect);

		// Now we can update the screen
		cBmp.Paint(pDestBmp, &cGlobalRect, pDirtyRect);
	}

	return (m_errCode);
}


VOID CBofDisplayButton::OnLButtonDown(UINT, CBofPoint *pPoint) {
	Assert(IsValidObject(this));
	Assert(pPoint != NULL);

	if (!HasCapture() && m_nState != BUTTON_DISABLED) {

		SetCapture();

		SetState(BUTTON_DOWN, TRUE);
	}
}


VOID CBofDisplayButton::OnLButtonUp(UINT, CBofPoint *pPoint) {
	Assert(IsValidObject(this));
	Assert(pPoint != NULL);

	if (HasCapture()) {

		ReleaseCapture();

		SetState(BUTTON_UP, TRUE);

		if (GetRect().PtInRect(*pPoint) && (m_pParent != NULL)) {
			m_pParent->OnBofButton(this, BUTTON_CLICKED);
		}
	}
}

} // namespace Bagel
