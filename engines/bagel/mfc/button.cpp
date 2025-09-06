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
ON_WM_LBUTTONDOWN()
ON_WM_LBUTTONUP()
ON_WM_MOUSEMOVE()
ON_MESSAGE(BM_CLICK, CButton::OnBnClicked)
ON_MESSAGE(BM_SETCHECK, CButton::OnBnSetCheck)
END_MESSAGE_MAP()

bool CButton::Create(const char *lpszCaption, uint32 dwStyle,
		const RECT &rect, CWnd *pParentWnd, unsigned int nID) {
	return CWnd::Create("BUTTON", lpszCaption, dwStyle, rect,
		pParentWnd, nID);
}

int CButton::GetCheck() const {
	return (_itemState & ODS_CHECKED) != 0 ?
		BST_CHECKED : BST_UNCHECKED;
}

void CButton::SetCheck(int nCheck) {
	SendMessage(BM_SETCHECK, nCheck);
}

void CButton::SetButtonStyle(unsigned int nStyle, bool bRedraw) {
	m_nStyle = (m_nStyle & ~0xf) | nStyle;
	Invalidate();
}

unsigned int CButton::GetButtonStyle() const {
	return GetStyle() & 0xf;
}

void CButton::OnPaint() {
	switch (GetButtonStyle()) {
	case BS_PUSHBUTTON:
	case BS_DEFPUSHBUTTON:
		OnPushButtonPaint();
		break;

	case BS_PUSHBOX:
		OnPushBoxPaint();
		break;

	case BS_OWNERDRAW:
		OnOwnerDrawPaint();
		break;

	default: {
		// Fallback for unhandled button types
		RECT clientRect;
		GetClientRect(&clientRect);
		CPaintDC dc(this);
		dc.FillSolidRect(&clientRect, RGB(255, 255, 255));
		break;
	}
	}
}

void CButton::OnPushButtonPaint() {
	CPaintDC dc(this);

	// Get the client rectangle of the window
	CRect rect;
	GetClientRect(&rect);

	// Determine state
	bool isEnabled = IsWindowEnabled();

	// Choose background color based on state
	COLORREF bgColor = isEnabled ? RGB(240, 240, 240) : RGB(200, 200, 200);
	dc.FillSolidRect(rect, bgColor);

	if (_pressed) {
		dc.DrawEdge(&rect, EDGE_SUNKEN, BF_RECT);
	} else {
		dc.DrawEdge(&rect, EDGE_RAISED, BF_RECT);
	}

	// Draw text
	CString text;
	GetWindowText(text);
	dc.SetBkMode(TRANSPARENT);
	dc.DrawText(text, rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void CButton::OnPushBoxPaint() {
	CPaintDC dc(this);

	CRect rect;
	GetClientRect(&rect);

	// Choose colors
	COLORREF clrFace = GetSysColor(COLOR_BTNFACE);
	COLORREF clrHighlight = GetSysColor(COLOR_BTNHIGHLIGHT); // Top/left
	COLORREF clrShadow = GetSysColor(COLOR_BTNSHADOW);       // Bottom/right
	COLORREF clrText = GetSysColor(COLOR_BTNTEXT);

	// Fill background
	dc.FillSolidRect(rect, clrFace);

	// Draw 3D border
	if (_pressed) {
		// Inset border (pressed look)
		dc.Draw3dRect(rect, clrShadow, clrHighlight);
		rect.DeflateRect(1, 1);
		dc.Draw3dRect(rect, clrShadow, clrHighlight);
	} else {
		// Raised border (normal look)
		dc.Draw3dRect(rect, clrHighlight, clrShadow);
		rect.DeflateRect(1, 1);
		dc.Draw3dRect(rect, clrHighlight, clrShadow);
	}

	// Adjust text position if pressed
	CPoint offset = _pressed ? CPoint(1, 1) : CPoint(0, 0);

	// Draw button text
	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(clrText);
	dc.SelectObject(GetStockObject(SYSTEM_FONT));

	CRect textRect = rect;
	textRect.OffsetRect(offset);

	dc.DrawText(_windowText, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	// Draw focus rectangle
	if (_hasFocus) {
		CRect focusRect = rect;
		focusRect.DeflateRect(3, 3);
		dc.DrawFocusRect(&focusRect);
	}
}

void CButton::OnOwnerDrawPaint() {
	// Prepare the draw item struct
	DRAWITEMSTRUCT dis = { 0, 0, 0, 0, 0, 0, 0, { 0, 0, 0, 0 }, 0 };
	dis.CtlType = ODT_BUTTON;
	dis.CtlID = GetDlgCtrlID();
	dis.itemID = 0;
	dis.itemAction = ODA_DRAWENTIRE;
	dis.itemState = 0;

	if (IsWindowEnabled())
		dis.itemState |= ODS_ENABLED;
	else
		dis.itemState |= ODS_DISABLED;
	if ((GetState() & ODS_CHECKED) != 0 || _pressed)
		dis.itemState |= ODS_SELECTED;
	if (GetFocus() == this)
		dis.itemState |= ODS_FOCUS;

	dis.hwndItem = m_hWnd;

	// Get the client rect for the button
	GetClientRect(&dis.rcItem);

	// Get a DC for drawing
	PAINTSTRUCT paintStruct;
	HDC hdc = BeginPaint(&paintStruct);
	dis.hDC = hdc;

	// Send WM_DRAWITEM to the parent window
	GetParent()->SendMessage(WM_DRAWITEM,
		dis.CtlID, (LPARAM)&dis);

	EndPaint(&paintStruct);
}

void CButton::OnLButtonDown(unsigned int nFlags, CPoint point) {
	if (!(_itemState & ODS_DISABLED)) {
		SetCapture();
		_pressed = true;
		Invalidate();
	}
}

void CButton::OnLButtonUp(unsigned int nFlags, CPoint point) {
	if (GetCapture() == this)
		ReleaseCapture();
	if (!_pressed)
		return;
	_pressed = false;

	// Redraw the button immediately, before we trigger any actions
	Invalidate();
	UpdateWindow();

	switch (GetButtonStyle()) {
	case BS_PUSHBUTTON:
	case BS_DEFPUSHBUTTON:
	case BS_PUSHBOX:
	case BS_OWNERDRAW:
		SendMessage(BM_CLICK);
		break;

	case BS_CHECKBOX:
		SetCheck(_itemState & BST_CHECKED ?
			BST_CHECKED : BST_UNCHECKED);
		SendMessage(BM_SETCHECK, _itemState & BST_CHECKED);
		break;

	case BS_AUTORADIOBUTTON:
		SetCheck(BST_CHECKED);
		SendMessage(BM_SETCHECK, BST_CHECKED);
		break;

	default:
		error("Unhandled button type");
		break;
	}
}

void CButton::OnMouseMove(unsigned int nFlags, CPoint point) {
	if (GetCapture() == this) {
		if (_pressed != PointInClientRect(point)) {
			_pressed = !_pressed;
			Invalidate();
		}
	}
}

LRESULT CButton::OnBnClicked(WPARAM wParam, LPARAM lParam) {
	GetParent()->SendMessage(WM_COMMAND, GetDlgCtrlID(),
		MAKELPARAM(0, BN_CLICKED));

	return 0;
}

LRESULT CButton::OnBnSetCheck(WPARAM wParam, LPARAM lParam) {
	if (wParam == BST_UNCHECKED) {
		// Uncheck the button
		_itemState &= ~BST_CHECKED;
	} else {
		// Check the button
		_itemState |= BST_CHECKED;

		// Need to clear all of the others
		CWnd *pParent = GetParent();
		CWnd *pWnd = this;

		for (;;) {
			pWnd = pParent->GetNextDlgGroupItem(pWnd, true);
			if (pWnd == this)
				// No more to do
				break;

			pWnd->SendMessage(BM_SETCHECK, false);
		}
	}

	Invalidate();
	return 0;
}

} // namespace MFC
} // namespace Bagel
