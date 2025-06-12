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
	error("TODO: CButton::GetCheck");
}

void CButton::SetCheck(int nCheck) {
	error("TODO: CButton::SetChunk");
}

void CButton::SetButtonStyle(UINT nStyle, BOOL bRedraw) {
	error("TODO: CButton::SetButtonStyle");
}

void CButton::OnPaint() {
	CPaintDC dc(this);

	// Get the client rectangle of the window
	CRect rect;
	GetClientRect(&rect);

	// Fill the background with light blue
	dc.FillSolidRect(&rect, RGB(173, 216, 230));
}

} // namespace MFC
} // namespace Bagel
