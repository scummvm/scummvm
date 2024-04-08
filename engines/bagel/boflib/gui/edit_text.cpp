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

#include "bagel/boflib/boffo.h"
#include "bagel/boflib/app.h"
#include "bagel/boflib/gui/edit_text.h"
#include "bagel/boflib/gfx/text.h"

namespace Bagel {


CBofEditText::CBofEditText(const CHAR *pszName, INT x, INT y, INT nWidth,
		INT nHeight, CBofWindow *pParent)
		: CBofWindow(pszName, x, y, nWidth, nHeight, pParent) {
	Create(pszName, x, y, nWidth, nHeight, pParent);
}


ERROR_CODE CBofEditText::Create(const CHAR *pszName, CBofRect *pRect,
		CBofWindow *pParent, uint32 nControlID) {
	Assert(IsValidObject(this));
	Assert(pszName != nullptr);

	// Remember who our parent is
	_parent = pParent;

	INT x, y, nWidth, nHeight;

	x = y = 0;
	nWidth = nHeight = USE_DEFAULT;
	if (pRect != nullptr) {
		x = pRect->left;
		y = pRect->top;
		nWidth = pRect->Width();
		nHeight = pRect->Height();
	}

	return Create(pszName, x, y, nWidth, nHeight, pParent, nControlID);
}


ERROR_CODE CBofEditText::Create(const CHAR *pszName, INT x, INT y,
		INT nWidth, INT nHeight, CBofWindow *pParent, uint32 nControlID) {
	Assert(IsValidObject(this));
	Assert(pszName != nullptr);

	// Remember who our parent is
	_parent = pParent;
	m_nID = nControlID;

	// Remember the name of this window
	Common::strcpy_s(m_szTitle, pszName);

	// Retain screen coordinates for this window
	m_cWindowRect.SetRect(x, y, x + nWidth - 1, y + nHeight - 1);

	CBofPalette *pPalette;
	if ((pPalette = CBofApp::GetApp()->GetPalette()) != nullptr) {
		SelectPalette(pPalette);
	}

	// Retain local coordinates (based on own window)
	m_cRect.SetRect(0, 0, m_cWindowRect.Width() - 1, m_cWindowRect.Height() - 1);

	return m_errCode;
}

void CBofEditText::SetText(const CHAR *pszString) {
	Assert(IsValidObject(this));
	Assert(IsCreated());
	Assert(pszString != nullptr);

	_text = pszString;

	UpdateWindow();
}


void CBofEditText::OnPaint(CBofRect *pRect) {
	Assert(IsValidObject(this));
	Assert(pRect != nullptr);

	if (HasFocus())
		FillRect(nullptr, 255);

	// Draw the text, if any
	if (!_text.IsEmpty()) {
		CBofString tmp = _text + "|";

		PaintText(this, &m_cRect, tmp.GetBuffer(),
			12, 0, CTEXT_COLOR,
			JUSTIFY_LEFT,
			FORMAT_TOP_LEFT | FORMAT_SINGLE_LINE);
	}

	// Handle drawing the cursor

}

void CBofEditText::OnLButtonDown(uint32 nFlags, CBofPoint *pPoint, void *) {
	// First click focuses text input
	SetFocus();
	_cursorPos = _text.GetBufferSize();

	UpdateWindow();
}

void CBofEditText::OnKeyHit(uint32 lKey, uint32 lRepCount) {
	if (lKey >= 32 && lKey <= 127) {
		CBofString tmp = _text + lKey;
		CBofRect rect = CalculateTextRect(this, &tmp, 12, 0);

		if ((m_cRect.Width() - rect.Width()) > 10) {
			SetText(tmp);
		}
	
	} else if (lKey == BKEY_BACK && !_text.IsEmpty()) {
		_text.DeleteLastChar();
		UpdateWindow();
	}
}

} // namespace Bagel
