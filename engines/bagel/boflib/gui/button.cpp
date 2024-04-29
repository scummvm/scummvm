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
	_nState = BUTTON_UP;

	// Load a default color scheme until another is loaded
	loadColorScheme(&g_stDefaultColors);
}


CBofButton::CBofButton(ST_COLORSCHEME *pColorScheme) {
	Assert(pColorScheme != nullptr);

	// Inits
	_nState = BUTTON_UP;

	loadColorScheme(pColorScheme);
}


CBofButton::~CBofButton() {
	Assert(IsValidObject(this));
}


ErrorCode CBofButton::paint(CBofRect *) {
	Assert(IsValidObject(this));

	// Only continue if this button is visible
	if (IsVisible() && (_parent != nullptr) && _parent->IsVisible()) {
		CBofPalette *pPalette;
		RGBCOLOR cTextColor;
		byte iHighlight, iShadow, iTemp;
		int nWidth, nHeight;
		int i, left, right, top, bottom;

		pPalette = CBofApp::GetApp()->GetPalette();

		nWidth = _cRect.width();
		nHeight = _cRect.Height();

		// Create our off-screen buffer
		CBofBitmap cBmp(nWidth, nHeight, pPalette);

		cBmp.FillRect(&_cRect, pPalette->GetNearestIndex(_cFaceColor));

		left = _cRect.left;
		right = _cRect.right;
		top = _cRect.top;
		bottom = _cRect.bottom;

		iShadow = pPalette->GetNearestIndex(_cShadowColor);
		iHighlight = pPalette->GetNearestIndex(_cHighlightColor);

		if (_nState == BUTTON_DOWN) {
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

		cBmp.DrawRect(&_cRect, pPalette->GetNearestIndex(_cOutlineColor));

		// Create a temporary text object
		CBofRect cTempRect(3, 3, _cRect.right - 3, _cRect.bottom - 3);

		if (_nState == BUTTON_DOWN) {
			cTempRect += CBofPoint(1, 1);
		}

		CBofText cText(&cTempRect);

		// Print text into button
		cTextColor = _cTextColor;
		if (_nState == BUTTON_DISABLED)
			cTextColor = _cTextDisabledColor;

		cText.display(&cBmp, _szTitle, BUTTON_TEXT_SIZE, TEXT_NORMAL, cTextColor);

		if (_nState == BUTTON_FOCUS) {
			cBmp.DrawRect(&cTempRect, pPalette->GetNearestIndex(_cOutlineColor));
		}

		// Now we can update the window
		cBmp.paint(this, 0, 0);
	}

	return _errCode;
}


void CBofButton::loadColorScheme(ST_COLORSCHEME *pColorScheme) {
	Assert(IsValidObject(this));

	Assert(pColorScheme != nullptr);

	// Save all of the color info we need to build a button
	_cFaceColor = pColorScheme->_cFace;
	_cHighlightColor = pColorScheme->_cHighlight;
	_cShadowColor = pColorScheme->_cShadow;
	_cTextColor = pColorScheme->_cText;
	_cTextDisabledColor = pColorScheme->_cTextDisabled;
	_cOutlineColor = pColorScheme->_cOutline;
}


void CBofButton::enable() {
	Assert(IsValidObject(this));

	CBofWindow::enable();

	setState(BUTTON_UP);
}


void CBofButton::disable() {
	Assert(IsValidObject(this));

	setState(BUTTON_DISABLED);

	CBofWindow::disable();
}


ErrorCode CBofButton::setState(int nNewState, bool bRepaintNow) {
	Assert(IsValidObject(this));
	Assert(nNewState >= BUTTON_UP && nNewState <= BUTTON_DISABLED);

	// Remember last button state
	int nOldState = _nState;

	_nState = nNewState;

	// Update the window if forced to or if button state has changed
	if (bRepaintNow || (nOldState != nNewState)) {
		paint();
	}

	// I must have a valid parent
	Assert(_parent != nullptr);

	// Tell parent the new state of this button
	if (_parent != nullptr) {
		_parent->OnBofButton(this, _nState);
	}

	return _errCode;
}


void CBofButton::onPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));

	Assert(pRect != nullptr);

	paint(pRect);
}


void CBofButton::onLButtonDown(uint32, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	if (!_bCaptured && _nState != BUTTON_DISABLED) {

		SetCapture();

		setState(BUTTON_DOWN, true);
	}
}


void CBofButton::onLButtonUp(uint32, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	if (_bCaptured) {
		ReleaseCapture();

		setState(BUTTON_UP, true);

		if (_cRect.PtInRect(*pPoint) && (_parent != nullptr)) {
			_parent->OnBofButton(this, BUTTON_CLICKED);
		}
	}
}

/*****************************************************************************/
/*                           CBofRadioButton                                 */
/*****************************************************************************/


void CBofRadioButton::onLButtonDown(uint32, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	if ((_nState != BUTTON_DISABLED) && (_nState == BUTTON_UP)) {

		setState(BUTTON_DOWN, true);
	}
}


void CBofRadioButton::onLButtonUp(uint32, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);
}


ErrorCode CBofRadioButton::paint(CBofRect *) {
	Assert(IsValidObject(this));

	// Only continue if this button is visible
	if (IsVisible() && (_parent != nullptr) && _parent->IsVisible()) {

		CBofPalette *pPalette = CBofApp::GetApp()->GetPalette();

		int nWidth = _cRect.width();
		int nHeight = _cRect.Height();

		// Create a temporary off-screen buffer
		CBofBitmap cBmp(nWidth, nHeight, pPalette);

		// Fill in the background color
		cBmp.FillRect(&_cRect, pPalette->GetNearestIndex(_cFaceColor));

		RGBCOLOR cTextColor = _cTextColor;
		if (_nState == BUTTON_DISABLED)
			cTextColor = _cTextDisabledColor;

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
		CBofRect cTempRect(20, RADIO_BOX_OFFSET_DY, _cRect.right, _cRect.bottom - RADIO_BOX_OFFSET_DY);
		if (_nState == BUTTON_DOWN) {
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
		cBmp.DrawRect(&_cRect, pPalette->GetNearestIndex(_cOutlineColor));

		// Show text disabled if button is disabled

		// Print text into button
		cText.display(&cBmp, _szTitle, RADIO_BOX_TEXT_SIZE, TEXT_NORMAL, cTextColor);

		// If button has focus, then put a box around the text
		if (_nState == BUTTON_FOCUS) {
			cBmp.DrawRect(&cTempRect, pPalette->GetNearestIndex(_cOutlineColor));
		}

		// Now we can update the window
		cBmp.paint(this, 0, 0);
	}

	return _errCode;
}


/*****************************************************************************/
/*                           CBofCheckButton                                 */
/*****************************************************************************/


void CBofCheckButton::onLButtonDown(uint32, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	if (_nState != BUTTON_DISABLED) {

		if (_nState == BUTTON_UP) {

			setState(BUTTON_DOWN, true);

		} else if (_nState == BUTTON_DOWN) {

			setState(BUTTON_UP, true);
		}
	}
}


void CBofCheckButton::onLButtonUp(uint32, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	// Do nothing, and don't call the base class version of this function
}


ErrorCode CBofCheckButton::paint(CBofRect *) {
	Assert(IsValidObject(this));

	// Only continue if this button is visible
	if (IsVisible() && (_parent != nullptr) && _parent->IsVisible()) {

		CBofPalette *pPalette = CBofApp::GetApp()->GetPalette();

		int nWidth = _cRect.width();
		int nHeight = _cRect.Height();

		// Create a temporary off-screen buffer
		CBofBitmap cBmp(nWidth, nHeight, pPalette);

		// Fill in the background color
		cBmp.FillRect(&_cRect, pPalette->GetNearestIndex(_cFaceColor));

		// Show text disabled if button is disabled
		RGBCOLOR cTextColor = _cTextColor;
		if (_nState == BUTTON_DISABLED)
			cTextColor = _cTextDisabledColor;

		byte iShadow = pPalette->GetNearestIndex(cTextColor);

		// Draw the check box (centered vertically)
		int y = ((nHeight - CHECK_BOX_SIZE) / 2);
		CBofRect cTempRect(CHECK_BOX_OFFSET_DX, y, CHECK_BOX_SIZE + CHECK_BOX_OFFSET_DX - 1, y + CHECK_BOX_SIZE - 1);
		cBmp.DrawRect(&cTempRect, iShadow);

		// if button is in DOWN/ON state, then put an X in the box
		//
		if (_nState == BUTTON_DOWN) {
			cBmp.Line(cTempRect.left, cTempRect.top, cTempRect.right, cTempRect.bottom, iShadow);
			cBmp.Line(cTempRect.left, cTempRect.bottom, cTempRect.right, cTempRect.top, iShadow);
		}

		// Create a temporary text object
		cTempRect.SetRect(CHECK_BOX_SIZE + CHECK_BOX_OFFSET_DX, CHECK_BOX_OFFSET_DX, _cRect.right, _cRect.bottom - CHECK_BOX_OFFSET_DX);
		CBofText cText(&cTempRect, JUSTIFY_LEFT);

		// Put a box around the whole button
		cBmp.DrawRect(&_cRect, pPalette->GetNearestIndex(_cOutlineColor));

		// Print text into button
		//
		cText.display(&cBmp, _szTitle, CHECK_BOX_TEXT_SIZE, TEXT_NORMAL, cTextColor);

		// If button has focus, then put a box around the text
		if (_nState == BUTTON_FOCUS) {
			cBmp.DrawRect(&cTempRect, pPalette->GetNearestIndex(_cOutlineColor));
		}

		// Now we can update the window
		cBmp.paint(this, 0, 0);
	}

	return _errCode;
}


ErrorCode CBofCheckButton::SetCheck(bool bChecked) {
	Assert(IsValidObject(this));

	setState(bChecked ? BUTTON_CHECKED : BUTTON_UNCHECKED, false);

	return _errCode;
}


/*****************************************************************************/
/*                           CBofBmpButton                                   */
/*****************************************************************************/


CBofBmpButton::CBofBmpButton() {
	_pButtonUp = nullptr;
	_pButtonDown = nullptr;
	_pButtonFocus = nullptr;
	_pButtonDisabled = nullptr;
	_pBackground = nullptr;
	_nState = BUTTON_UP;
	_nMaskColor = NOT_TRANSPARENT;
}


CBofBmpButton::~CBofBmpButton() {
	if (_pButtonUp != nullptr) {
		delete _pButtonUp;
		_pButtonUp = nullptr;
	}
	if (_pButtonDown != nullptr) {
		delete _pButtonDown;
		_pButtonDown = nullptr;
	}
	if (_pButtonDisabled != nullptr) {
		delete _pButtonDisabled;
		_pButtonDisabled = nullptr;
	}
	if (_pButtonFocus != nullptr) {
		delete _pButtonFocus;
		_pButtonFocus = nullptr;
	}
	if (_pBackground != nullptr) {
		delete _pBackground;
		_pBackground = nullptr;
	}
}


ErrorCode CBofBmpButton::paint(CBofRect *) {
	Assert(IsValidObject(this));

	// loadBitmaps must be called before the button can be painted
	Assert(_pButtonUp != nullptr);
	Assert(_pButtonDown != nullptr);
	Assert(_pButtonFocus != nullptr);
	Assert(_pButtonDisabled != nullptr);

	// Only continue if this button is visible
	if (IsVisible() && (_parent != nullptr) && _parent->IsVisible()) {
		CBofPalette *pPalette = _pButtonUp->GetPalette();

		int nWidth = _cRect.width();
		int nHeight = _cRect.Height();

		// Do all painting off-screen
		CBofBitmap cOffScreen(nWidth, nHeight, pPalette);

		if (_pBackground == nullptr) {
			if ((_pBackground = new CBofBitmap(nWidth, nHeight, pPalette)) != nullptr) {

			} else {
				ReportError(ERR_MEMORY, "Could not allocate a CBofBitmap(%d x %d)", nWidth, nHeight);
			}

		} else {
			_pBackground->paint(&cOffScreen, 0, 0);
		}

		// Assume UP state
		CBofBitmap *pBitmap = _pButtonUp;

		// Display the correct bitmap based on state
		if (_nState == BUTTON_DOWN) {
			pBitmap = _pButtonDown;

		} else if (_nState == BUTTON_FOCUS) {
			pBitmap = _pButtonFocus;

		} else if (_nState == BUTTON_DISABLED) {
			pBitmap = _pButtonDisabled;
		}

		// Paint button offscreen
		pBitmap->paint(&cOffScreen, 0, 0, nullptr, _nMaskColor);

		// Now we can update the window
		cOffScreen.paint(this, 0, 0);
	}

	return _errCode;
}


ErrorCode CBofBmpButton::loadBitmaps(CBofBitmap *pUp, CBofBitmap *pDown, CBofBitmap *pFocus, CBofBitmap *pDisabled, int nMaskColor) {
	Assert(IsValidObject(this));

	Assert(pUp != nullptr);
	Assert(pDown != nullptr);

	// Use the bitmaps passed in
	_pButtonUp = pUp;
	_pButtonDown = pDown;
	_pButtonFocus = pFocus;
	_pButtonDisabled = pDisabled;

	// Remember the transparent color for these bitmaps
	_nMaskColor = nMaskColor;

	return _errCode;
}


ErrorCode CBofBmpButton::loadBitmaps(CBofPalette *pPalette, const char *pszUp, const char *pszDown, const char *pszFocus, const char *pszDisabled, int nMaskColor) {
	Assert(IsValidObject(this));

	Assert(pPalette != nullptr);
	Assert(pszUp != nullptr);
	Assert(pszDown != nullptr);
	Assert(pszFocus != nullptr);
	Assert(pszDisabled != nullptr);

	// Remember the button transparent color
	_nMaskColor = nMaskColor;

	// Load each of the bitmaps that represent the button state
	if ((_pButtonUp = new CBofBitmap(pszUp, pPalette)) != nullptr) {
		_pButtonUp->SetReadOnly(true);
	}
	if ((_pButtonDown = new CBofBitmap(pszDown, pPalette)) != nullptr) {
		_pButtonDown->SetReadOnly(true);
	}
	if ((_pButtonFocus = new CBofBitmap(pszFocus, pPalette)) != nullptr) {
		_pButtonFocus->SetReadOnly(true);
	}
	if ((_pButtonDisabled = new CBofBitmap(pszDisabled, pPalette)) != nullptr) {
		_pButtonDisabled->SetReadOnly(true);
	}

	return _errCode;
}


ErrorCode CBofBmpButton::setState(int nNewState, bool bRepaintNow) {
	Assert(IsValidObject(this));
	Assert(nNewState >= BUTTON_UP && nNewState <= BUTTON_DISABLED);

	// Remember last button state
	int nOldState = _nState;

	_nState = nNewState;

	// Update the window if forced to or if button state has changed
	if (bRepaintNow || (nOldState != nNewState)) {
		paint();
	}

	// I MUST have a valid parent
	Assert(_parent != nullptr);

	// Tell parent the new state of this button
	if (_parent != nullptr) {
		_parent->OnBofButton(this, _nState);
	}

	return _errCode;
}


void CBofBmpButton::onPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));

	Assert(pRect != nullptr);

	paint(pRect);
}


void CBofBmpButton::onLButtonDown(uint32, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	if (!_bCaptured && _nState != BUTTON_DISABLED) {
		SetCapture();

		setState(BUTTON_DOWN, true);
	}
}


void CBofBmpButton::onLButtonUp(uint32, CBofPoint *pPoint, void *) {
	Assert(IsValidObject(this));
	Assert(pPoint != nullptr);

	if (_bCaptured) {
		ReleaseCapture();

		setState(BUTTON_UP, true);

		if (_cRect.PtInRect(*pPoint) && (_parent != nullptr)) {
			_parent->OnBofButton(this, BUTTON_CLICKED);
		}
	}
}

} // namespace Bagel
