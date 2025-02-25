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

#include "bagel/spacebar/boflib/gui/button.h"
#include "bagel/spacebar/boflib/app.h"
#include "bagel/spacebar/boflib/gfx/text.h"

namespace Bagel {
namespace SpaceBar {

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
	assert(pColorScheme != nullptr);

	// Inits
	_nState = BUTTON_UP;

	loadColorScheme(pColorScheme);
}


CBofButton::~CBofButton() {
	assert(isValidObject(this));
}


ErrorCode CBofButton::paint(CBofRect *) {
	assert(isValidObject(this));

	// Only continue if this button is visible
	if (isVisible() && (_parent != nullptr) && _parent->isVisible()) {
		CBofPalette *pPalette = CBofApp::getApp()->getPalette();

		int nWidth = _cRect.width();
		int nHeight = _cRect.height();

		// Create our off-screen buffer
		CBofBitmap cBmp(nWidth, nHeight, pPalette);

		cBmp.fillRect(&_cRect, pPalette->getNearestIndex(_cFaceColor));

		int left = _cRect.left;
		int right = _cRect.right;
		int top = _cRect.top;
		int bottom = _cRect.bottom;

		byte iShadow = pPalette->getNearestIndex(_cShadowColor);
		byte iHighlight = pPalette->getNearestIndex(_cHighlightColor);

		if (_nState == BUTTON_DOWN) {
			byte iTemp = iShadow;
			iShadow = iHighlight;
			iHighlight = iTemp;
		}

		byte c1 = iShadow;
		byte c2 = iHighlight;

		int i;
		for (i = 1; i <= 3; i++) {
			cBmp.line(left + i, bottom - i, right - i, bottom - i, c1);
			cBmp.line(right - i, bottom - i, right - i, top + i - 1, c1);
		}

		for (i = 1; i <= 3; i++) {
			cBmp.line(left + i, bottom - i, left + i, top + i - 1, c2);
			cBmp.line(left + i, top + i - 1, right - i, top + i - 1, c2);
		}

		cBmp.drawRect(&_cRect, pPalette->getNearestIndex(_cOutlineColor));

		// Create a temporary text object
		CBofRect cTempRect(3, 3, _cRect.right - 3, _cRect.bottom - 3);

		if (_nState == BUTTON_DOWN) {
			cTempRect += CBofPoint(1, 1);
		}

		CBofText cText(&cTempRect);

		// Print text into button
		COLORREF cTextColor = _cTextColor;
		if (_nState == BUTTON_DISABLED)
			cTextColor = _cTextDisabledColor;

		cText.display(&cBmp, _szTitle, BUTTON_TEXT_SIZE, TEXT_NORMAL, cTextColor);

		if (_nState == BUTTON_FOCUS) {
			cBmp.drawRect(&cTempRect, pPalette->getNearestIndex(_cOutlineColor));
		}

		// Now we can update the window
		cBmp.paint(this, 0, 0);
	}

	return _errCode;
}


void CBofButton::loadColorScheme(ST_COLORSCHEME *pColorScheme) {
	assert(isValidObject(this));

	assert(pColorScheme != nullptr);

	// Save all of the color info we need to build a button
	_cFaceColor = pColorScheme->_cFace;
	_cHighlightColor = pColorScheme->_cHighlight;
	_cShadowColor = pColorScheme->_cShadow;
	_cTextColor = pColorScheme->_cText;
	_cTextDisabledColor = pColorScheme->_cTextDisabled;
	_cOutlineColor = pColorScheme->_cOutline;
}


void CBofButton::enable() {
	assert(isValidObject(this));

	CBofWindow::enable();

	setState(BUTTON_UP);
}


void CBofButton::disable() {
	assert(isValidObject(this));

	setState(BUTTON_DISABLED);

	CBofWindow::disable();
}


ErrorCode CBofButton::setState(int nNewState, bool bRepaintNow) {
	assert(isValidObject(this));
	assert(nNewState >= BUTTON_UP && nNewState <= BUTTON_DISABLED);

	// Remember last button state
	int nOldState = _nState;

	_nState = nNewState;

	// Update the window if forced to or if button state has changed
	if (bRepaintNow || (nOldState != nNewState)) {
		paint();
	}

	// I must have a valid parent
	assert(_parent != nullptr);

	// Tell parent the new state of this button
	if (_parent != nullptr) {
		_parent->onBofButton(this, _nState);
	}

	return _errCode;
}


void CBofButton::onPaint(CBofRect *pRect) {
	assert(isValidObject(this));
	assert(pRect != nullptr);

	paint(pRect);
}


void CBofButton::onLButtonDown(uint32, CBofPoint *pPoint, void *) {
	assert(isValidObject(this));
	assert(pPoint != nullptr);

	if (!_bCaptured && _nState != BUTTON_DISABLED) {
		setCapture();
		setState(BUTTON_DOWN, true);
	}
}


void CBofButton::onLButtonUp(uint32, CBofPoint *pPoint, void *) {
	assert(isValidObject(this));
	assert(pPoint != nullptr);

	if (_bCaptured) {
		releaseCapture();
		setState(BUTTON_UP, true);

		if (_cRect.ptInRect(*pPoint) && (_parent != nullptr)) {
			_parent->onBofButton(this, BUTTON_CLICKED);
		}
	}
}

/*****************************************************************************/
/*                           CBofRadioButton                                 */
/*****************************************************************************/


void CBofRadioButton::onLButtonDown(uint32, CBofPoint *pPoint, void *) {
	assert(isValidObject(this));
	assert(pPoint != nullptr);

	if (_nState == BUTTON_UP) {
		setState(BUTTON_DOWN, true);
	}
}


void CBofRadioButton::onLButtonUp(uint32, CBofPoint *pPoint, void *) {
	assert(isValidObject(this));
	assert(pPoint != nullptr);
}


ErrorCode CBofRadioButton::paint(CBofRect *) {
	assert(isValidObject(this));

	// Only continue if this button is visible
	if (isVisible() && (_parent != nullptr) && _parent->isVisible()) {

		CBofPalette *pPalette = CBofApp::getApp()->getPalette();

		int nWidth = _cRect.width();
		int nHeight = _cRect.height();

		// Create a temporary off-screen buffer
		CBofBitmap cBmp(nWidth, nHeight, pPalette);

		// Fill in the background color
		cBmp.fillRect(&_cRect, pPalette->getNearestIndex(_cFaceColor));

		COLORREF cTextColor = _cTextColor;
		if (_nState == BUTTON_DISABLED)
			cTextColor = _cTextDisabledColor;

		byte iShadow = pPalette->getNearestIndex(cTextColor);
		byte iHighlight = pPalette->getNearestIndex(cTextColor);

		// Paint the radio button circle
		int nRadius = 7;
		int x = nRadius + RADIO_BOX_OFFSET_DX;
		int y = nHeight / 2;
		cBmp.circle(x, y, (uint16)nRadius, iShadow);
		nRadius--;
		cBmp.circle(x, y, (uint16)nRadius, iShadow);

		// Create a temporary text object
		CBofRect cTempRect(20, RADIO_BOX_OFFSET_DY, _cRect.right, _cRect.bottom - RADIO_BOX_OFFSET_DY);
		if (_nState == BUTTON_DOWN) {
			nRadius = 1;
			cBmp.circle(x, y, (uint16)nRadius, iHighlight);
			nRadius = 2;
			cBmp.circle(x, y, (uint16)nRadius, iHighlight);
			nRadius = 3;
			cBmp.circle(x, y, (uint16)nRadius, iHighlight);
			nRadius = 4;
			cBmp.circle(x, y, (uint16)nRadius, iHighlight);
		}
		CBofText cText(&cTempRect, JUSTIFY_LEFT);

		// Put a box around the whole button
		cBmp.drawRect(&_cRect, pPalette->getNearestIndex(_cOutlineColor));

		// Show text disabled if button is disabled

		// Print text into button
		cText.display(&cBmp, _szTitle, RADIO_BOX_TEXT_SIZE, TEXT_NORMAL, cTextColor);

		// If button has focus, then put a box around the text
		if (_nState == BUTTON_FOCUS) {
			cBmp.drawRect(&cTempRect, pPalette->getNearestIndex(_cOutlineColor));
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
	assert(isValidObject(this));
	assert(pPoint != nullptr);

	if (_nState == BUTTON_DISABLED)
		return;

	if (_nState == BUTTON_UP) {
		setState(BUTTON_DOWN, true);
	} else if (_nState == BUTTON_DOWN) {
		setState(BUTTON_UP, true);
	}
}


void CBofCheckButton::onLButtonUp(uint32, CBofPoint *pPoint, void *) {
	assert(isValidObject(this));
	assert(pPoint != nullptr);

	// Do nothing, and don't call the base class version of this function
}


ErrorCode CBofCheckButton::paint(CBofRect *) {
	assert(isValidObject(this));

	// Only continue if this button is visible
	if (isVisible() && (_parent != nullptr) && _parent->isVisible()) {

		CBofPalette *pPalette = CBofApp::getApp()->getPalette();

		int nWidth = _cRect.width();
		int nHeight = _cRect.height();

		// Create a temporary off-screen buffer
		CBofBitmap cBmp(nWidth, nHeight, pPalette);

		// Fill in the background color
		cBmp.fillRect(&_cRect, pPalette->getNearestIndex(_cFaceColor));

		// Show text disabled if button is disabled
		COLORREF cTextColor = _cTextColor;
		if (_nState == BUTTON_DISABLED)
			cTextColor = _cTextDisabledColor;

		byte iShadow = pPalette->getNearestIndex(cTextColor);

		// Draw the check box (centered vertically)
		int y = ((nHeight - CHECK_BOX_SIZE) / 2);
		CBofRect cTempRect(CHECK_BOX_OFFSET_DX, y, CHECK_BOX_SIZE + CHECK_BOX_OFFSET_DX - 1, y + CHECK_BOX_SIZE - 1);
		cBmp.drawRect(&cTempRect, iShadow);

		// if button is in DOWN/ON state, then put an X in the box
		//
		if (_nState == BUTTON_DOWN) {
			cBmp.line(cTempRect.left, cTempRect.top, cTempRect.right, cTempRect.bottom, iShadow);
			cBmp.line(cTempRect.left, cTempRect.bottom, cTempRect.right, cTempRect.top, iShadow);
		}

		// Create a temporary text object
		cTempRect.setRect(CHECK_BOX_SIZE + CHECK_BOX_OFFSET_DX, CHECK_BOX_OFFSET_DX, _cRect.right, _cRect.bottom - CHECK_BOX_OFFSET_DX);
		CBofText cText(&cTempRect, JUSTIFY_LEFT);

		// Put a box around the whole button
		cBmp.drawRect(&_cRect, pPalette->getNearestIndex(_cOutlineColor));

		// Print text into button
		//
		cText.display(&cBmp, _szTitle, CHECK_BOX_TEXT_SIZE, TEXT_NORMAL, cTextColor);

		// If button has focus, then put a box around the text
		if (_nState == BUTTON_FOCUS) {
			cBmp.drawRect(&cTempRect, pPalette->getNearestIndex(_cOutlineColor));
		}

		// Now we can update the window
		cBmp.paint(this, 0, 0);
	}

	return _errCode;
}


ErrorCode CBofCheckButton::SetCheck(bool bChecked) {
	assert(isValidObject(this));

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
	delete _pButtonUp;
	_pButtonUp = nullptr;

	delete _pButtonDown;
	_pButtonDown = nullptr;

	delete _pButtonDisabled;
	_pButtonDisabled = nullptr;

	delete _pButtonFocus;
	_pButtonFocus = nullptr;

	delete _pBackground;
	_pBackground = nullptr;
}


ErrorCode CBofBmpButton::paint(CBofRect *) {
	assert(isValidObject(this));

	// loadBitmaps must be called before the button can be painted
	assert(_pButtonUp != nullptr);
	assert(_pButtonDown != nullptr);
	assert(_pButtonFocus != nullptr);
	assert(_pButtonDisabled != nullptr);

	// Only continue if this button is visible
	if (isVisible() && (_parent != nullptr) && _parent->isVisible()) {
		CBofPalette *pPalette = _pButtonUp->getPalette();

		int nWidth = _cRect.width();
		int nHeight = _cRect.height();

		// Do all painting off-screen
		CBofBitmap cOffScreen(nWidth, nHeight, pPalette);

		if (_pBackground == nullptr) {
			_pBackground = new CBofBitmap(nWidth, nHeight, pPalette);
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
	assert(isValidObject(this));
	assert(pUp != nullptr);
	assert(pDown != nullptr);

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
	assert(isValidObject(this));
	assert(pPalette != nullptr);
	assert(pszUp != nullptr);
	assert(pszDown != nullptr);
	assert(pszFocus != nullptr);
	assert(pszDisabled != nullptr);

	// Remember the button transparent color
	_nMaskColor = nMaskColor;

	// Load each of the bitmaps that represent the button state
	_pButtonUp = new CBofBitmap(pszUp, pPalette);
	_pButtonUp->setReadOnly(true);

	_pButtonDown = new CBofBitmap(pszDown, pPalette);
	_pButtonDown->setReadOnly(true);

	_pButtonFocus = new CBofBitmap(pszFocus, pPalette);
	_pButtonFocus->setReadOnly(true);

	_pButtonDisabled = new CBofBitmap(pszDisabled, pPalette);
	_pButtonDisabled->setReadOnly(true);

	return _errCode;
}


ErrorCode CBofBmpButton::setState(int nNewState, bool bRepaintNow) {
	assert(isValidObject(this));
	assert(nNewState >= BUTTON_UP && nNewState <= BUTTON_DISABLED);

	// Remember last button state
	int nOldState = _nState;

	_nState = nNewState;

	// Update the window if forced to or if button state has changed
	if (bRepaintNow || (nOldState != nNewState)) {
		paint();
	}

	// I MUST have a valid parent
	assert(_parent != nullptr);

	// Tell parent the new state of this button
	if (_parent != nullptr) {
		_parent->onBofButton(this, _nState);
	}

	return _errCode;
}


void CBofBmpButton::onPaint(CBofRect *pRect) {
	assert(isValidObject(this));
	assert(pRect != nullptr);

	paint(pRect);
}


void CBofBmpButton::onLButtonDown(uint32, CBofPoint *pPoint, void *) {
	assert(isValidObject(this));
	assert(pPoint != nullptr);

	if (!_bCaptured && _nState != BUTTON_DISABLED) {
		setCapture();

		setState(BUTTON_DOWN, true);
	}
}


void CBofBmpButton::onLButtonUp(uint32, CBofPoint *pPoint, void *) {
	assert(isValidObject(this));
	assert(pPoint != nullptr);

	if (_bCaptured) {
		releaseCapture();

		setState(BUTTON_UP, true);

		if (_cRect.ptInRect(*pPoint) && (_parent != nullptr)) {
			_parent->onBofButton(this, BUTTON_CLICKED);
		}
	}
}

} // namespace SpaceBar
} // namespace Bagel
