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

IMPLEMENT_DYNAMIC(CButton, CWnd)
BEGIN_MESSAGE_MAP(CButton, CWnd)
ON_WM_PAINT()
END_MESSAGE_MAP()

BOOL CButton::Create(LPCTSTR lpszCaption, DWORD dwStyle,
		const RECT &rect, CWnd *pParentWnd, UINT nID) {
	return CWnd::Create("BUTTON", lpszCaption, dwStyle, rect,
		pParentWnd, nID);
}

int CButton::GetCheck() const {
	return (_itemState & ODS_CHECKED) != 0 ?
		BST_CHECKED : BST_UNCHECKED;
}

void CButton::SetCheck(int nCheck) {
	if (nCheck == BST_UNCHECKED)
		_itemState &= ~ODS_CHECKED;
	else
		_itemState |= ODS_CHECKED;
}

void CButton::SetButtonStyle(UINT nStyle, BOOL bRedraw) {
	error("TODO: CButton::SetButtonStyle");
}

void CButton::OnPaint() {
	CPaintDC dc(this);

	// Get the client rectangle of the window
	CRect rect;
	GetClientRect(&rect);

	// Determine state
	bool isEnabled = IsWindowEnabled();
	int checkState = GetCheck();

	// Choose background color based on state
	COLORREF bgColor = isEnabled ? RGB(240, 240, 240) : RGB(200, 200, 200);
	dc.FillSolidRect(rect, bgColor);

	// Draw checked state (e.g., highlight border or symbol)
	if (checkState == BST_CHECKED) {
		dc.DrawEdge(&rect, EDGE_SUNKEN, BF_RECT);
	} else {
		dc.DrawEdge(&rect, EDGE_RAISED, BF_RECT);
	}
#ifdef TODO
	// Draw text
	CString text;
	GetWindowText(text);
	dc.SetBkMode(TRANSPARENT);
	dc.DrawText(text, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
#endif
}

} // namespace MFC
} // namespace Bagel
