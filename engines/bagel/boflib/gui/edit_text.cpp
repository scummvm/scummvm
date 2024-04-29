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
#include "bagel/boflib/gui/edit_text.h"
#include "bagel/boflib/gfx/text.h"
#include "bagel/boflib/std_keys.h"

namespace Bagel {


CBofEditText::CBofEditText(const char *pszName, int x, int y, int nWidth,
		int nHeight, CBofWindow *pParent)
		: CBofWindow(pszName, x, y, nWidth, nHeight, pParent) {
	create(pszName, x, y, nWidth, nHeight, pParent);
}


ErrorCode CBofEditText::create(const char *pszName, CBofRect *pRect,
		CBofWindow *pParent, uint32 nControlID) {
	Assert(IsValidObject(this));
	Assert(pszName != nullptr);

	// Remember who our parent is
	_parent = pParent;

	int x = 0;
	int y = 0;
	int nWidth = USE_DEFAULT;
	int nHeight = USE_DEFAULT;

	if (pRect != nullptr) {
		x = pRect->left;
		y = pRect->top;
		nWidth = pRect->width();
		nHeight = pRect->height();
	}

	return create(pszName, x, y, nWidth, nHeight, pParent, nControlID);
}


ErrorCode CBofEditText::create(const char *pszName, int x, int y,
		int nWidth, int nHeight, CBofWindow *pParent, uint32 nControlID) {
	Assert(IsValidObject(this));
	Assert(pszName != nullptr);

	// Remember who our parent is
	_parent = pParent;
	_nID = nControlID;

	// Remember the name of this window
	Common::strcpy_s(_szTitle, pszName);

	// Retain screen coordinates for this window
	_cWindowRect.SetRect(x, y, x + nWidth - 1, y + nHeight - 1);

	CBofPalette *pPalette = CBofApp::GetApp()->getPalette();
	if (pPalette != nullptr) {
		selectPalette(pPalette);
	}

	// Retain local coordinates (based on own window)
	_cRect.SetRect(0, 0, _cWindowRect.width() - 1, _cWindowRect.height() - 1);

	return _errCode;
}

void CBofEditText::setText(const char *pszString) {
	Assert(IsValidObject(this));
	Assert(isCreated());
	Assert(pszString != nullptr);

	_text = pszString;

	updateWindow();
}


void CBofEditText::onPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));
	Assert(pRect != nullptr);

	if (hasFocus())
		fillRect(nullptr, 255);

	// Draw the text, if any
	if (!_text.IsEmpty()) {
		CBofString tmp = _text + "|";

		paintText(this, &_cRect, tmp.GetBuffer(),
			12, 0, CTEXT_COLOR,
			JUSTIFY_LEFT,
			FORMAT_TOP_LEFT | FORMAT_SINGLE_LINE);
	}

}

void CBofEditText::onLButtonDown(uint32 nFlags, CBofPoint *pPoint, void *) {
	// First click focuses text input
	setFocus();
	_cursorPos = _text.GetBufferSize();

	updateWindow();
}

void CBofEditText::onKeyHit(uint32 lKey, uint32 lRepCount) {
	if (lKey >= 32 && lKey <= 127) {
		CBofString tmp = _text + lKey;
		CBofRect rect = calculateTextRect(this, &tmp, 12, 0);

		if ((_cRect.width() - rect.width()) > 10) {
			setText(tmp);
		}
	
	} else if (lKey == BKEY_BACK && !_text.IsEmpty()) {
		_text.DeleteLastChar();
		updateWindow();
	}
}

} // namespace Bagel
