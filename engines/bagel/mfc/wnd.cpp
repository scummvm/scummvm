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

#include "common/system.h"
#include "common/queue.h"
#include "common/textconsole.h"
#include "bagel/mfc/afxwin.h"
#include "bagel/mfc/wingdi.h"
#include "bagel/mfc/win_hand.h"
#include "bagel/mfc/gfx/dialog_template.h"

namespace Bagel {
namespace MFC {

IMPLEMENT_DYNAMIC(CWnd, CCmdTarget)
BEGIN_MESSAGE_MAP(CWnd, CCmdTarget)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_DRAWITEM()
	ON_WM_SETFONT()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

CWnd *CWnd::FromHandlePermanent(HWND hWnd) {
	auto *pMap = AfxGetApp()->afxMapWnd();
	assert(pMap);
	return pMap->LookupPermanent(hWnd);
}

CWnd *CWnd::FromHandle(HWND hWnd) {
	return FromHandlePermanent(hWnd);
}

CWnd::CWnd() : m_hWnd(this) {
	CWinApp *app = AfxGetApp();
	auto *pMap = app->afxMapWnd(true);
	assert(pMap != nullptr);

	pMap->SetPermanent(m_hWnd, this);

	// Defaults
	_hFont = app->getDefaultFont();
	_hPen = app->getDefaultPen();
	_hBrush = app->getDefaultBrush();
	_hPalette = app->getSystemPalette();
}

CWnd::~CWnd() {
	// Although we check the flag here, currently it's
	// hardcoded to be on for ScummVM
	if ((m_nClassStyle & CS_OWNDC) && _pDC) {
		_pDC->DeleteDC();
		delete _pDC;
		_pDC = nullptr;
	}

	DestroyWindow();

	// Remove wnd from the map
	auto *pMap = AfxGetApp()->afxMapWnd();
	assert(pMap != nullptr);
	pMap->RemoveHandle(m_hWnd);
}

BOOL CWnd::Create(LPCSTR lpszClassName, LPCSTR lpszWindowName,
	DWORD dwStyle, const RECT &rect, CWnd *pParentWnd,
	UINT nID, CCreateContext *pContext) {
	m_pParentWnd = pParentWnd;
	m_nStyle = dwStyle;
	_controlId = nID;

	// Set up create structure
	CREATESTRUCT cs;
	cs.x = rect.left;
	cs.y = rect.top;
	cs.cx = rect.right - rect.left;
	cs.cy = rect.bottom - rect.top;

	// Trigger pre-create event
	if (!PreCreateWindow(cs))
		return false;

	// Create the actual window content
	_windowRect.left = cs.x;
	_windowRect.top = cs.y;
	_windowRect.right = cs.x + cs.cx;
	_windowRect.bottom = cs.y + cs.cy;
	_windowText = lpszWindowName;

	// Get the screen area
	RECT screenRect(0, 0, cs.cx, cs.cy);
	ClientToScreen(&screenRect);

	// Get the class details
	WNDCLASS wc;
	GetClassInfo(nullptr, lpszClassName, &wc);
	m_nClassStyle = wc.style;

	if (m_pParentWnd)
		m_pParentWnd->_children[nID] = this;

	SendMessage(WM_CREATE, 0, (LPARAM)&cs);

	return true;
}

const MSG *CWnd::GetCurrentMessage() {
	return &AfxGetApp()->_currentMessage;
}

CWnd *CWnd::GetParent() const {
	return m_pParentWnd;
}

Common::Array<const CWnd *> CWnd::GetSafeParents(bool includeSelf) const {
	Common::Array<const CWnd *> results;
	bool hasParentDialog = false;

	for (const CWnd *wnd = includeSelf ? this : m_pParentWnd;
			wnd; wnd = wnd->m_pParentWnd) {
		if (dynamic_cast<const CDialog *>(wnd)) {
			if (!hasParentDialog) {
				hasParentDialog = true;
				results.push_back(wnd);
			}
		} else {
			results.push_back(wnd);
		}
	}

	return results;
}

Common::Array<CWnd *> CWnd::GetSafeParents(bool includeSelf) {
	Common::Array<CWnd *> results;
	bool hasParentDialog = !includeSelf &&
		dynamic_cast<CDialog *>(this) != nullptr;

	for (CWnd *wnd = includeSelf ? this : m_pParentWnd;
		wnd; wnd = wnd->m_pParentWnd) {
		if (dynamic_cast<const CDialog *>(wnd)) {
			if (!hasParentDialog) {
				hasParentDialog = true;
				results.push_back(wnd);
			}
		} else {
			results.push_back(wnd);
		}
	}

	return results;
}

HWND CWnd::GetSafeHwnd() const {
	error("TODO: CWnd::GetSafeHwnd");
}

void CWnd::ShowWindow(int nCmdShow) {
	assert(nCmdShow == SW_SHOWNORMAL || nCmdShow == SW_HIDE);

	if (nCmdShow == SW_SHOWNORMAL)
		m_nStyle |= WS_VISIBLE;
	else
		m_nStyle &= ~WS_VISIBLE;

	Invalidate(false);
	SendMessage(WM_SHOWWINDOW);
}

BOOL CWnd::EnableWindow(BOOL bEnable) {
	bool oldEnabled = (_itemState & ODS_DISABLED) == 0;
	if (bEnable)
		_itemState &= ~ODS_DISABLED;
	else
		_itemState |= ODS_DISABLED;

	return oldEnabled;
}

void CWnd::UpdateWindow() {
	// If there's a pending paint, do it now
	MSG msg;
	if (PeekMessage(&msg, nullptr, WM_PAINT, WM_PAINT, PM_REMOVE) ||
		IsWindowDirty()) {
		msg.hwnd = m_hWnd;
		msg.message = WM_PAINT;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

BOOL CWnd::RedrawWindow(LPCRECT lpRectUpdate,
	CRgn *prgnUpdate, UINT flags) {
	if (flags & RDW_INVALIDATE) {
		// Invalidate the region or rectangle
		if (prgnUpdate != nullptr) {
			error("TODO: InvalidateRgn");
		} else {
			InvalidateRect(lpRectUpdate, !(flags & RDW_NOERASE));
		}
	}

	if (flags & RDW_VALIDATE) {
		// Optionally validate instead
		if (prgnUpdate != nullptr) {
			error("TODO: ValidateRgn");
		} else {
			ValidateRect(lpRectUpdate);
		}
	}

	if (flags & RDW_UPDATENOW) {
		// Send WM_PAINT immediately
		UpdateWindow();
	}

	return true;
}

void CWnd::SetActiveWindow() {
	AfxGetApp()->SetActiveWindow(this);
}

void CWnd::SetFocus() {
	AfxGetApp()->SetFocus(this);
}

CWnd *CWnd::GetFocus() const {
	return AfxGetApp()->GetFocus();
}

void CWnd::OnClose() {
	DestroyWindow();
}

void CWnd::DestroyWindow() {
	// Lose focus if it currently has it
	auto *app = AfxGetApp();
	if (app->GetFocus() == this)
		app->SetFocus(nullptr);

	if (IsActiveWindow()) {
		SendMessage(WM_ACTIVATE, MAKEWPARAM(WA_INACTIVE, false), 0);
		app->PopActiveWindow();
	}

	// Flush any other pending events
	MSG msg;
	while (app->PeekMessage(&msg, m_hWnd, 0, 0, PM_REMOVE)) {
	}

	// Mark as not needed any repainting
	Validate();

	// Detach any child controls
	for (auto &node : _children)
		node._value->m_pParentWnd = nullptr;
	_children.clear();

	// Free any owned controls
	for (CWnd *ctl : _ownedControls)
		delete ctl;
	_ownedControls.clear();

	// If it's the active window, pop it
	if (this == AfxGetApp()->GetActiveWindow()) {
		AfxGetApp()->PopActiveWindow();
	}

	// If still attached to parent, remove from it
	if (m_pParentWnd) {
		m_pParentWnd->_ownedControls.remove(this);
		m_pParentWnd->_children.erase(_controlId);
	}

	SendMessage(WM_DESTROY);
}

int CWnd::GetWindowText(CString &rString) const {
	rString = _windowText;
	return rString.size();
}

int CWnd::GetWindowText(LPSTR lpszStringBuf, int nMaxCount) const {
	Common::strcpy_s(lpszStringBuf, nMaxCount, _windowText.c_str());
	return strlen(lpszStringBuf);
}

BOOL CWnd::SetWindowText(LPCSTR lpszString) {
	_windowText = lpszString;
	Invalidate();
	return true;
}

UINT CWnd::GetState() const {
	return _itemState;
}

void CWnd::SetStyle(DWORD nStyle) {
	m_nStyle = nStyle;
	Invalidate();
}

CDC *CWnd::GetDC() {
	if (_pDC != nullptr) {
		// Return persistent DC
		return _pDC;
	} else if (_windowRect.isEmpty()) {
		// Window hasn't yet been created, so return
		// a full screen DC
		HDC hdc = MFC::GetDC(nullptr);
		CDC *dc = new CDC();
		dc->Attach(hdc);
		return dc;

	} else {
		// Return a new DC for the window
		CDC::Impl *hDC = new CDC::Impl();
		hDC->Attach(_hFont);
		hDC->Attach(_hPen);
		hDC->Attach(_hBrush);
		hDC->selectPalette(_hPalette);

		RECT screenRect(0, 0, _windowRect.width(), _windowRect.height());
		ClientToScreen(&screenRect);
		hDC->setScreenRect(screenRect);

		CDC *pDC = new CDC();
		pDC->Attach(hDC);

		if (m_nClassStyle & CS_OWNDC)
			_pDC = pDC;

		return pDC;
	}
}

int CWnd::ReleaseDC(CDC *pDC) {
	if (pDC && pDC == _pDC) {
		// Wnd has persistent dc
		assert(m_nClassStyle & CS_OWNDC);

	} else {
		delete pDC;
	}

	return 1;
}

BOOL CWnd::PostMessage(UINT message, WPARAM wParam, LPARAM lParam) {
	return AfxGetApp()->PostMessage(m_hWnd, message, wParam, lParam);
}

LRESULT CWnd::SendMessage(UINT message, WPARAM wParam, LPARAM lParam) {
	auto &msg = AfxGetApp()->_currentMessage;
	msg.hwnd = m_hWnd;
	msg.message = message;
	msg.wParam = wParam;
	msg.lParam = lParam;

	LRESULT lResult = 0;
	if (!OnWndMsg(message, wParam, lParam, &lResult))
		lResult = DefWindowProc(message, wParam, lParam);

	// Handle messages that get sent to child controls
	if (isRecursiveMessage(message)) {
		for (auto &ctl : _children) {
			if (message == WM_PAINT) {
				if (!_updatingRect.intersects(ctl._value->_windowRect))
					continue;
			}

			ctl._value->SendMessage(message, wParam, lParam);
		}
	}

	return lResult;
}

bool CWnd::isRecursiveMessage(UINT message) {
	// TODO: Need to refactor these to use the
	// SendMessageToDescendants instead of this
	return message == WM_PAINT ||
		message == WM_SHOWWINDOW ||
		message == WM_ENABLE;
}

BOOL CWnd::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT *pResult) {
	LRESULT lResult = 0;
	const AFX_MSGMAP_ENTRY *lpEntry;

	// Special case for commands
	if (message == WM_COMMAND) {
		if (OnCommand(wParam, lParam)) {
			lResult = 1;
			goto LReturnTrue;
		}

		return FALSE;
	} else if (message == WM_ACTIVATE) {
		OnActivate(LOWORD(wParam), nullptr, false);
		lResult = 1;
		goto LReturnTrue;
	}

	// Ignoring messages to hidden controls
	if (!IsWindowVisible()) {
		static const uint16 MESSAGES[] = {
			WM_PAINT, WM_ERASEBKGND, WM_LBUTTONDOWN,
			WM_LBUTTONUP, WM_MOUSEMOVE, WM_SETFOCUS,
			WM_KILLFOCUS, WM_KEYDOWN, WM_KEYUP, WM_CHAR,
			WM_COMMAND, WM_TIMER, 0
		};
		for (const uint16 *msgP = MESSAGES; *msgP; ++msgP) {
			if (message == *msgP)
				return true;
		}
	}

	// Special cases we don't currently support
#define UNHANDLED(MSG) if (message == MSG) \
		error(#MSG " not currently supported")
	UNHANDLED(WM_NOTIFY);
	UNHANDLED(WM_SETTINGCHANGE);
#undef UNHANDLED

	// Look up the message in the message map
	lpEntry = LookupMessage(message);
	if (!lpEntry)
		return false;

	assert(message < 0xC000);
	union MessageMapFunctions mmf;
	mmf.pfn = lpEntry->pfn;

	int nSig;
	nSig = lpEntry->nSig;

	switch (nSig) {
	case AfxSig_bD:
		lResult = (this->*mmf.pfn_bD)(CDC::FromHandle((HDC)wParam));
		break;

	case AfxSig_bb:     // AfxSig_bb, AfxSig_bw, AfxSig_bh
		lResult = (this->*mmf.pfn_bb)((BOOL)wParam);
		break;

	case AfxSig_bWww:   // really AfxSig_bWiw
		lResult = (this->*mmf.pfn_bWww)(CWnd::FromHandle((HWND)wParam),
			(short)LOWORD(lParam), HIWORD(lParam));
		break;

	case AfxSig_bWCDS:
		lResult = (this->*mmf.pfn_bWCDS)(CWnd::FromHandle((HWND)wParam),
			(COPYDATASTRUCT *)lParam);
		break;

	case AfxSig_bHELPINFO:
		lResult = (this->*mmf.pfn_bHELPINFO)((HELPINFO *)lParam);
		break;

	case AfxSig_hDWw:
		// Special case for OnCtlColor to avoid too many temporary objects
		error("Unsupported OnCtlColor");
		break;

	case AfxSig_hDw:
		// Special case for CtlColor to avoid too many temporary objects
		error("Unsupported WM_REFLECT_BASE");
		break;

	case AfxSig_iwWw:
		lResult = (this->*mmf.pfn_iwWw)(LOWORD(wParam),
			CWnd::FromHandle((HWND)lParam), HIWORD(wParam));
		break;

	case AfxSig_iww:
		lResult = (this->*mmf.pfn_iww)(LOWORD(wParam), HIWORD(wParam));
		break;

	case AfxSig_iWww:
		// Really AfxSig_iWiw
		lResult = (this->*mmf.pfn_iWww)(CWnd::FromHandle((HWND)wParam),
			(short)LOWORD(lParam), HIWORD(lParam));
		break;

	case AfxSig_is:
		lResult = (this->*mmf.pfn_is)((LPTSTR)lParam);
		break;

	case AfxSig_lwl:
		lResult = (this->*mmf.pfn_lwl)(wParam, lParam);
		break;

	case AfxSig_lwwM:
		lResult = (this->*mmf.pfn_lwwM)((UINT)LOWORD(wParam),
			(UINT)HIWORD(wParam), (CMenu *)CMenu::FromHandle((HMENU)lParam));
		break;

	case AfxSig_vv:
		(this->*mmf.pfn_vv)();
		break;

	case AfxSig_vw: // AfxSig_vb, AfxSig_vh
		(this->*mmf.pfn_vw)(wParam);
		break;

	case AfxSig_vww:
		(this->*mmf.pfn_vww)((UINT)wParam, (UINT)lParam);
		break;

	case AfxSig_vvii:
		(this->*mmf.pfn_vvii)((short)LOWORD(lParam), (short)HIWORD(lParam));
		break;

	case AfxSig_vwww:
		(this->*mmf.pfn_vwww)(wParam, LOWORD(lParam), HIWORD(lParam));
		break;

	case AfxSig_vwii:
		(this->*mmf.pfn_vwii)(wParam, LOWORD(lParam), HIWORD(lParam));
		break;

	case AfxSig_vwl:
		(this->*mmf.pfn_vwl)(wParam, lParam);
		break;

	case AfxSig_vbWW:
		(this->*mmf.pfn_vbWW)(m_hWnd == (HWND)lParam,
			CWnd::FromHandle((HWND)lParam),
			CWnd::FromHandle((HWND)wParam));
		break;

	case AfxSig_vD:
		(this->*mmf.pfn_vD)(CDC::FromHandle((HDC)wParam));
		break;

	case AfxSig_vM:
		(this->*mmf.pfn_vM)(CMenu::FromHandle((HMENU)wParam));
		break;

	case AfxSig_vMwb:
		(this->*mmf.pfn_vMwb)(CMenu::FromHandle((HMENU)wParam),
			LOWORD(lParam), (BOOL)HIWORD(lParam));
		break;

	case AfxSig_vW:
		(this->*mmf.pfn_vW)(CWnd::FromHandle((HWND)wParam));
		break;

	case AfxSig_vW2:
		(this->*mmf.pfn_vW)(CWnd::FromHandle((HWND)lParam));
		break;

	case AfxSig_vWww:
		(this->*mmf.pfn_vWww)(CWnd::FromHandle((HWND)wParam), LOWORD(lParam),
			HIWORD(lParam));
		break;

	case AfxSig_vWp:
	{
		CPoint point((DWORD)lParam);
		(this->*mmf.pfn_vWp)(CWnd::FromHandle((HWND)wParam), point);
	}
	break;

	case AfxSig_vWh:
		(this->*mmf.pfn_vWh)(CWnd::FromHandle((HWND)wParam),
			(HANDLE)lParam);
		break;

	case AfxSig_vwW:
		(this->*mmf.pfn_vwW)(wParam, CWnd::FromHandle((HWND)lParam));
		break;

	case AfxSig_vwWb:
		(this->*mmf.pfn_vwWb)((UINT)(LOWORD(wParam)),
			CWnd::FromHandle((HWND)lParam), (BOOL)HIWORD(wParam));
		break;

	case AfxSig_vwwW:
	case AfxSig_vwwx:
	{
		// special case for WM_VSCROLL and WM_HSCROLL
		ASSERT(message == WM_VSCROLL || message == WM_HSCROLL ||
			message == WM_VSCROLL + WM_REFLECT_BASE || message == WM_HSCROLL + WM_REFLECT_BASE);
		int nScrollCode = (short)LOWORD(wParam);
		int nPos = (short)HIWORD(wParam);
		if (lpEntry->nSig == AfxSig_vwwW)
			(this->*mmf.pfn_vwwW)(nScrollCode, nPos,
				CWnd::FromHandle((HWND)lParam));
		else
			(this->*mmf.pfn_vwwx)(nScrollCode, nPos);
	}
	break;

	case AfxSig_vs:
		(this->*mmf.pfn_vs)((LPTSTR)lParam);
		break;

	case AfxSig_vws:
		(this->*mmf.pfn_vws)((UINT)wParam, (LPCTSTR)lParam);
		break;

	case AfxSig_vOWNER:
		(this->*mmf.pfn_vOWNER)((int)wParam, (LPTSTR)lParam);
		lResult = TRUE;
		break;

	case AfxSig_iis:
		lResult = (this->*mmf.pfn_iis)((int)wParam, (LPTSTR)lParam);
		break;

	case AfxSig_wp:
	{
		CPoint point((DWORD)lParam);
		lResult = (this->*mmf.pfn_wp)(point);
	}
	break;

	case AfxSig_wv: // AfxSig_bv, AfxSig_wv
		lResult = (this->*mmf.pfn_wv)();
		break;

	case AfxSig_vCALC:
		(this->*mmf.pfn_vCALC)((BOOL)wParam, (NCCALCSIZE_PARAMS *)lParam);
		break;

	case AfxSig_vPOS:
		(this->*mmf.pfn_vPOS)((WINDOWPOS *)lParam);
		break;

	case AfxSig_vwwh:
		(this->*mmf.pfn_vwwh)(LOWORD(wParam), HIWORD(wParam), (HANDLE)lParam);
		break;

	case AfxSig_vwp:
	{
		CPoint point((DWORD)lParam);
		(this->*mmf.pfn_vwp)(wParam, point);
		break;
	}
	case AfxSig_vwSIZING:
		(this->*mmf.pfn_vwl)(wParam, lParam);
		lResult = TRUE;
		break;

	case AfxSig_bwsp:
		lResult = (this->*mmf.pfn_bwsp)(LOWORD(wParam), (short)HIWORD(wParam),
			CPoint(LOWORD(lParam), HIWORD(lParam)));
		if (!lResult)
			return FALSE;

	case AfxSig_vwpb:
		(this->*mmf.pfn_vFb)((HFONT)wParam, (BOOL)lParam);
		break;

	default:
		error("Unknown AFX signature");
		break;
	}

LReturnTrue:
	if (pResult != nullptr)
		*pResult = lResult;
	return TRUE;
}

BOOL CWnd::Validate() {
	return ValidateRect(nullptr);
}

BOOL CWnd::ValidateRect(LPCRECT lpRect) {
	if (!lpRect) {
		// Remove entire area
		_updateRect = Common::Rect();

	} else {
		if (_updateRect.isEmpty() || _updateRect.contains(*lpRect))
			_updateRect = Common::Rect();
	}

	return true;
}

void CWnd::Invalidate(BOOL bErase) {
	CRect clientRect;
	GetClientRect(&clientRect);
	InvalidateRect(&clientRect, bErase);
}

BOOL CWnd::InvalidateRect(LPCRECT lpRect, BOOL bErase) {
	if (lpRect)
		_updateRect.extend(*lpRect);
	else
		_updateRect = Common::Rect(0, 0,
			_windowRect.width(), _windowRect.height());
	assert(_updateRect.left >= 0 && _updateRect.top >= 0 &&
		_updateRect.right <= _windowRect.width() &&
		_updateRect.bottom <= _windowRect.bottom);

	// Handle bubbling up to parent
	const CWnd *child = this;
	Common::Rect r = _updateRect;
	for (CWnd *wnd : GetSafeParents(false)) {
		r.translate(child->_windowRect.left,
			child->_windowRect.top);
		if (wnd->_updateRect.isEmpty())
			wnd->_updateRect = r;
		else
			wnd->_updateRect.extend(r);

		assert(wnd->_updateRect.left >= 0 && wnd->_updateRect.top >= 0 &&
			wnd->_updateRect.right <= wnd->_windowRect.width() &&
			wnd->_updateRect.bottom <= wnd->_windowRect.bottom);

		child = wnd;
	}

	return true;
}

void CWnd::GetWindowRect(LPRECT lpRect) const {
	*lpRect = _windowRect;
}

BOOL CWnd::GetUpdateRect(LPRECT lpRect, BOOL bErase) {
	if (lpRect)
		*lpRect = _updateRect;
	_updateErase = bErase;

	return IsWindowDirty();
}

BOOL CWnd::GetClientRect(LPRECT lpRect) const {
	lpRect->left = 0;
	lpRect->top = 0;
	lpRect->right = _windowRect.width();
	lpRect->bottom = _windowRect.height();

	return true;
}

bool CWnd::PointInClientRect(const POINT &pt) const {
	RECT clientRect;
	GetClientRect(&clientRect);

	return clientRect.contains(pt);
}

void CWnd::ClientToScreen(LPPOINT lpPoint) const {
	for (const CWnd *wnd : GetSafeParents()) {
		lpPoint->x += wnd->_windowRect.left;
		lpPoint->y += wnd->_windowRect.top;
	}
}

void CWnd::ClientToScreen(LPRECT lpRect) const {
	for (const CWnd *wnd : GetSafeParents()) {
		lpRect->left += wnd->_windowRect.left;
		lpRect->top += wnd->_windowRect.top;
		lpRect->right += wnd->_windowRect.left;
		lpRect->bottom += wnd->_windowRect.top;
	}
}

void CWnd::ScreenToClient(LPPOINT lpPoint) const {
	for (const CWnd *wnd : GetSafeParents()) {
		lpPoint->x -= wnd->_windowRect.left;
		lpPoint->y -= wnd->_windowRect.top;
	}
}

void CWnd::ScreenToClient(LPRECT lpRect) const {
	for (const CWnd *wnd : GetSafeParents()) {
		lpRect->left -= wnd->_windowRect.left;
		lpRect->top -= wnd->_windowRect.top;
		lpRect->right -= wnd->_windowRect.left;
		lpRect->bottom -= wnd->_windowRect.top;
	}
}

void CWnd::MoveWindow(LPCRECT lpRect, BOOL bRepaint) {
	_windowRect = *lpRect;
	ValidateRect(nullptr);

	if (_pDC) {
		// Get the screen area
		RECT screenRect(0, 0, _windowRect.width(), _windowRect.height());
		ClientToScreen(&screenRect);
		_pDC->impl()->setScreenRect(screenRect);
	}

	// Iterate through all child controls. We won't
	// change their relative position, but doing so will
	// cause their screen surface area to be updated
	for (auto &ctl : _children) {
		RECT ctlRect = ctl._value->_windowRect;
		ctl._value->MoveWindow(&ctlRect, false);
	}

	if (bRepaint)
		InvalidateRect(nullptr, true);
}

void CWnd::MoveWindow(int x, int y, int nWidth, int nHeight, BOOL bRepaint) {
	const RECT r(x, y, x + nWidth, y + nHeight);
	MoveWindow(&r, bRepaint);
}

HDC CWnd::BeginPaint(LPPAINTSTRUCT lpPaint) {
	CDC *dc = GetDC();

	lpPaint->hdc = dc->m_hDC;
	lpPaint->fErase = _updateErase;
	lpPaint->rcPaint = _updateRect;
	lpPaint->fRestore = false;
	lpPaint->fIncUpdate = false;
	Common::fill(lpPaint->rgbReserved,
		lpPaint->rgbReserved + 32, 0);

	_updatingRect = _updateRect;

	// Restrict drawing to the update area
	dc->setClipRect(_updateRect);

	if (_hFont)
		SelectObject(lpPaint->hdc, _hFont);

	return dc->m_hDC;
}

BOOL CWnd::EndPaint(const PAINTSTRUCT *lpPaint) {
	_updateRect = Common::Rect();
	_updateErase = false;

	// If it's a persistent dc for the window, reset clipping
	if (_pDC)
		_pDC->resetClipRect();

	return true;
}

CWnd *CWnd::GetDlgItem(int nID) const {
	return _children.contains(nID) ? _children[nID] : nullptr;
}

CWnd *CWnd::GetNextDlgGroupItem(CWnd *pWndCtl, BOOL bPrevious) const {
	// First set up the children hash map as a straight array
	// for easier iterating over
	Common::Array<CWnd *> children;
	for (auto &child : _children)
		children.push_back(child._value);

	// Get the starting index
	int idStart = pWndCtl->GetDlgCtrlID();
	int startIdx;
	for (startIdx = 0; startIdx < (int)children.size() &&
		children[startIdx]->GetDlgCtrlID() != idStart; ++startIdx) {
	}
	assert(startIdx != (int)children.size());

	// Remove any items from the array from the next WS_GROUP onwards
	const int style = WS_GROUP | WS_VISIBLE;
	int idx = startIdx + 1;
	for (; idx < (int)children.size(); ++idx) {
		if ((children[idx]->GetStyle() & style) == style) {
			// Found next group, remove remainder
			while (idx < (int)children.size())
				children.remove_at(idx);
		}
	}

	// Remove any items before the start of the current group
	idx = startIdx;
	while (idx >= 0 && !(children[idx]->GetStyle() & WS_GROUP))
		--idx;
	for (; idx > 0; --idx, --startIdx)
		children.remove_at(0);
	assert(children[startIdx] == pWndCtl);

	// Get the next item
	if (bPrevious) {
		if (--startIdx < 0)
			startIdx = (int)children.size() - 1;
	} else {
		if (++startIdx == (int)children.size())
			startIdx = 0;
	}

	return children[startIdx];
}

BOOL CWnd::GotoDlgCtrl(CWnd *pWndCtrl) {
	if (pWndCtrl != nullptr) {
		pWndCtrl->SetFocus();            // Give it focus
		SendMessage(WM_NEXTDLGCTL, (WPARAM)pWndCtrl->m_hWnd, TRUE);	// Update focus info
		return TRUE;
	}

	return FALSE;
}

BOOL CWnd::SubclassDlgItem(UINT nID, CWnd *pParent) {
	// Validate we're replacing the same kind of control
	assert(pParent->_children.contains(nID));
	CWnd *oldControl = pParent->_children[nID];
	assert(IsKindOf(oldControl->GetRuntimeClass()));

	// Remove the old control from the window
	pParent->_ownedControls.remove(oldControl);
	pParent->_children.erase(oldControl->_controlId);

	// Add the new control to the parent
	pParent->_children[nID] = this;
	m_pParentWnd = pParent;

	// Copy over the properties to the new control
	_windowText = oldControl->_windowText;
	_windowRect = oldControl->_windowRect;
	_controlId = oldControl->_controlId;
	m_nClassStyle = oldControl->m_nClassStyle;
	m_nStyle = oldControl->m_nStyle;
	_hFont = oldControl->_hFont;

	RECT screenRect(0, 0, _windowRect.width(), _windowRect.height());
	ClientToScreen(&screenRect);

	// Copy over the settings	
	_hFont = oldControl->_hFont;
	_hPen = oldControl->_hPen;
	_hBrush = oldControl->_hBrush;
	_hPalette = oldControl->_hPalette;

	return true;
}

BOOL CWnd::SetDlgItemText(int nIDDlgItem, LPCSTR lpString) {
	error("TODO: CWnd::SetDlgItemText");
}

int CWnd::GetDlgCtrlID() const {
	return _controlId;
}

void CWnd::CheckDlgButton(int nIDButton, UINT nCheck) {
	CWnd *btn = GetDlgItem(nIDButton);
	assert(btn);
	btn->SendMessage(BM_SETCHECK, nCheck);
}

LRESULT CWnd::SendDlgItemMessage(int nID, UINT message,
		WPARAM wParam, LPARAM lParam) const {
	CWnd *ctl = GetDlgItem(nID);
	assert(ctl);
	return ctl->SendMessage(message, wParam, lParam);
}

UINT_PTR CWnd::SetTimer(UINT_PTR nIDEvent, UINT nElapse,
	void (CALLBACK *lpfnTimer)(HWND, UINT, UINT_PTR, DWORD)) {
	return MFC::SetTimer(m_hWnd, nIDEvent, nElapse, lpfnTimer);
}

BOOL CWnd::KillTimer(UINT_PTR nIDEvent) {
	return MFC::KillTimer(m_hWnd, nIDEvent);
}

BOOL CWnd::GetScrollRange(int nBar,
	LPINT lpMinPos, LPINT lpMaxPos) const {
	error("TODO: CWnd::GetScrollRange");
}

INT CWnd::GetScrollPosition() const {
	error("TODO: CWnd::GetScrollPosition");
}

int CWnd::SetScrollPos(int nBar, int nPos, BOOL bRedraw) {
	error("TODO: CWnd::SetScrollPos");
}

void CWnd::createDialogIndirect(LPCDLGTEMPLATE dlgTemplate) {
	CDialog *dialog = dynamic_cast<CDialog *>(this);
	assert(dialog);

	// Parse the template and use it to load controls
	Gfx::CDialogTemplate dt(dlgTemplate);
	dt.loadTemplate(dialog);

	SendMessage(WM_INITDIALOG);
	ShowWindow(SW_SHOWNORMAL);
}

void CWnd::SetCapture() {
	AfxGetApp()->SetCapture(m_hWnd);
}

void CWnd::ReleaseCapture() {
	AfxGetApp()->ReleaseCapture();
}

void CWnd::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) {
	CWnd *pControl = GetDlgItem(nIDCtl);
	if (pControl)
		pControl->DrawItem(lpDrawItemStruct);
}

BOOL CWnd::OnCommand(WPARAM wParam, LPARAM lParam) {
	UINT nID = LOWORD(wParam);
	int nCode = HIWORD(wParam);

	// TODO: Original does some stuff with reflecting
	// the previous message to child control by default

	return OnCmdMsg(nID, nCode, nullptr, nullptr);
}

void CWnd::OnSetFont(HFONT hFont, BOOL bRedraw) {
	_hFont = hFont;

	if (bRedraw)
		Invalidate();
}

BOOL CWnd::OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message) {
	MFC::SetCursor(MFC::LoadCursor(nullptr, IDC_ARROW));
	return true;
}

void CWnd::SendMessageToDescendants(UINT message,
		WPARAM wParam, LPARAM lParam,
		bool bDeep, bool) {
	// Note: since in ScummVM the m_hWnd points
	// to the CWnd itself, all controls are currently
	// "permanent", hence the original bPermanent
	// param is meaningless
	Common::Queue<CWnd *> queue;
	queue.push(this);

	while (!queue.empty()) {
		CWnd *wnd = queue.pop();

		// If the control has any children,
		// add them if deep mode is flagged
		if (bDeep) {
			for (auto &node : wnd->_children)
				queue.push(node._value);
		}

		// Send the message to the control
		wnd->SendMessage(message, wParam, lParam);
	}
}

void CWnd::SendMessageToDescendants(HWND hWnd, UINT message,
		WPARAM wParam, LPARAM lParam, BOOL bDeep, BOOL bOnlyPerm) {
	CWnd *wnd = CWnd::FromHandle(hWnd);
	wnd->SendMessageToDescendants(message,
		wParam, lParam, bDeep, bOnlyPerm);
}

bool CWnd::IsActiveWindow() const {
	return AfxGetApp()->GetActiveWindow() == this;
}

void CWnd::SetFont(CFont *pFont, BOOL bRedraw) {
	SendMessage(WM_SETFONT, (WPARAM)pFont->m_hObject, bRedraw);
}

void CWnd::pause() {
	AfxGetApp()->pause();
}

} // namespace MFC
} // namespace Bagel
