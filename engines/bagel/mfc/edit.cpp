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

#include "common/textconsole.h"
#include "bagel/mfc/afxwin.h"

namespace Bagel {
namespace MFC {

IMPLEMENT_DYNAMIC(CEdit, CWnd)
BEGIN_MESSAGE_MAP(CEdit, CWnd)
ON_WM_PAINT()
ON_WM_KEYDOWN()
END_MESSAGE_MAP()

BOOL CEdit::Create(DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, UINT nID) {
	return CWnd::Create("EDIT", nullptr, dwStyle,
		rect, pParentWnd, nID);
}

void CEdit::LimitText(int nChars) {
	_maxLength = nChars;
}

void CEdit::SetSel(int nStartChar, int nEndChar, BOOL bNoScroll) {
	warning("TODO: CEdit::SetSel");
}

void CEdit::OnPaint() {
	CPaintDC dc(this); // Automatically calls BeginPaint and EndPaint

	// Get client area
	CRect clientRect;
	GetClientRect(&clientRect);

	// Fill background with system window color
	HBRUSH hBrush = MFC::GetSysColorBrush(COLOR_WINDOW);
	CBrush *winBrush = CBrush::FromHandle(hBrush);
	dc.FillRect(&clientRect, winBrush);

	// Set text and background properties
	dc.SetTextColor(MFC::GetSysColor(COLOR_WINDOWTEXT));
	dc.SetBkMode(TRANSPARENT);

	// Select the current font if one was set
	HFONT hFont = (HFONT)SendMessage(WM_GETFONT);
	HFONT hOldFont = nullptr;
	if (hFont)
		hOldFont = (HFONT)dc.SelectObject(hFont);

	// Draw the stored text
	dc.DrawText(_windowText.c_str(), -1, &clientRect, DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX);

	// Restore font if changed
	if (hFont)
		dc.SelectObject(hOldFont);
}

void CEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
	if (Common::isPrint(nChar)) {
		if (_maxLength != 0 && _windowText.size() == _maxLength)
			return;

		_windowText += (char)nChar;
	} else if (nChar == Common::KEYCODE_BACKSPACE ||
			nChar == Common::KEYCODE_DELETE) {
		if (!_windowText.empty()) {
			_windowText.deleteLastChar();
		}
	}

	Invalidate();
}

} // namespace MFC
} // namespace Bagel
