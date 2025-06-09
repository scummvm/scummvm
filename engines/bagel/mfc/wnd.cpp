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
#include "common/textconsole.h"
#include "bagel/mfc/afxwin.h"
#include "bagel/mfc/gfx/dialog_template.h"

namespace Bagel {
namespace MFC {

IMPLEMENT_DYNAMIC(CWnd, CCmdTarget)
BEGIN_MESSAGE_MAP(CWnd, CCmdTarget)
END_MESSAGE_MAP()

CWnd::CWnd() : m_hWnd(this) {
	_surface.Attach(&_surfaceBitmap);
	_dc.Attach(&_surface);
}

CWnd::~CWnd() {
	clear();
}

BOOL CWnd::Create(LPCSTR lpszClassName, LPCSTR lpszWindowName,
        DWORD dwStyle, const RECT &rect, CWnd *pParentWnd,
		UINT nID, CCreateContext *pContext) {
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

	Graphics::PixelFormat format = g_system->getScreenFormat();
	_surfaceBitmap.create(*AfxGetApp()->getScreen(), _windowRect);

	_controlId = nID;

	assert(pParentWnd);
	m_pParentWnd = pParentWnd;
	m_pParentWnd->_children[nID] = this;

	return true;
}

void CWnd::clear() {
	// Free any owned controls
	for (CWnd *ctl : _ownedControls) {
		_children.erase(ctl->_controlId);
		delete ctl;
	}

	_children.clear();
}

const MSG *CWnd::GetCurrentMessage() {
	return &AfxGetApp()->_currentMessage;
}

void CWnd::DoDataExchange(CDataExchange *) {
	error("TODO: CWnd::DoDataExchange");
}

CWnd *CWnd::GetParent() const {
	error("TODO: CWnd::GetParent");
}

HWND CWnd::GetSafeHwnd() const {
	error("TODO: CWnd::GetSafeHwnd");
}

void CWnd::ShowWindow(int nCmdShow) {
	assert(nCmdShow == SW_SHOWNORMAL);
	Invalidate(false);
}

BOOL CWnd::EnableWindow(BOOL bEnable) {
	error("TODO: CWnd::EnableWindow");
}

void CWnd::UpdateWindow() {
	// If there's a pending paint, do it now
	if (_messages.popPaint())
		SendMessage(WM_PAINT);
}

BOOL CWnd::RedrawWindow(LPCRECT lpRectUpdate,
                        CRgn *prgnUpdate, UINT flags) {
	error("TODO: CWnd::RedrawWindow");
}

void CWnd::SetActiveWindow() {
	error("TODO: CWnd::SetActiveWindow");
}

void CWnd::SetFocus() {
	error("TODO: CWnd::SetFocus");
}

void CWnd::DestroyWindow() {
	clear();
}

void CWnd::Invalidate(BOOL bErase) {
	// Mark the entire window for redrawing
	_updateRect = _windowRect;
	_updateErase = bErase;

	PostMessage(WM_PAINT);
}

int CWnd::GetWindowText(CString &rString) const {
	error("TODO: CWnd::GetWindowText");
}

int CWnd::GetWindowText(LPSTR lpszStringBuf, int nMaxCount) const {
	Common::strcpy_s(lpszStringBuf, nMaxCount, _windowText.c_str());
	error("TODO: CWnd::GetWindowText");
}

BOOL CWnd::SetWindowText(LPCSTR lpszString) {
	_windowText = lpszString;
	return true;
}

UINT CWnd::GetState() const {
	error("TODO: CWnd::GetState");
}

CDC *CWnd::GetDC() {
	return &_dc;
}

int CWnd::ReleaseDC(CDC *pDC) {
	// No implementation in ScummVM
	return 1;
}

BOOL CWnd::PostMessage(UINT message, WPARAM wParam, LPARAM lParam) {
	MSG msg(message, wParam, lParam);
	msg.hwnd = m_hWnd;

	_messages.push(msg);
	return true;
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
	return lResult;
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
	}

	// Special cases we don't currently support
#define UNHANDLED(MSG) if (message == MSG) \
		error(#MSG " not currently supported")
	UNHANDLED(WM_NOTIFY);
	UNHANDLED(WM_ACTIVATE);
	UNHANDLED(WM_SETCURSOR);
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

	case AfxSig_vWp: {
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
	case AfxSig_vwwx: {
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

	case AfxSig_wp: {
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

	case AfxSig_vwp: {
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

	default:
		error("Unknown AFX signature");
		break;
	}

LReturnTrue:
	if (pResult != nullptr)
		*pResult = lResult;
	return TRUE;
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

BOOL CWnd::InvalidateRect(LPCRECT lpRect, BOOL bErase) {
	if (lpRect)
		_updateRect.extend(*lpRect);
	else
		_updateRect = _windowRect;

	return true;
}

void CWnd::GetWindowRect(LPRECT lpRect) const {
	*lpRect = _windowRect;
}

BOOL CWnd::GetUpdateRect(LPRECT lpRect, BOOL bErase) {
	*lpRect = _updateRect;
	return true;
}

BOOL CWnd::GetClientRect(LPRECT lpRect) const {
	GetWindowRect(lpRect);
	return true;
}

void CWnd::MoveWindow(LPCRECT lpRect, BOOL bRepaint) {
	_windowRect = *lpRect;
	Graphics::PixelFormat format = g_system->getScreenFormat();
	_surfaceBitmap.create(*AfxGetApp()->getScreen(), _windowRect);

	if (bRepaint)
		InvalidateRect(nullptr, true);
}

void CWnd::MoveWindow(int x, int y, int nWidth, int nHeight, BOOL bRepaint) {
	const RECT r(x, y, x + nWidth, y + nHeight);
	MoveWindow(&r, bRepaint);
}

HDC CWnd::BeginPaint(LPPAINTSTRUCT lpPaint) {
	lpPaint->hdc = &_surface;
	lpPaint->fErase = _updateErase;
	lpPaint->rcPaint = _updateRect;
	lpPaint->fRestore = false;
	lpPaint->fIncUpdate = false;
	Common::fill(lpPaint->rgbReserved,
	             lpPaint->rgbReserved + 32, 0);

	return lpPaint->hdc;
}

BOOL CWnd::EndPaint(const PAINTSTRUCT *lpPaint) {
	_updateRect = Common::Rect();
	_updateErase = false;

	return true;
}

CWnd *CWnd::GetDlgItem(int nID) const {
	error("TODO: CWnd::GetDlgItem");
}

CWnd *CWnd::GetNextDlgGroupItem(CWnd *pWndCtl, BOOL bPrevious) const {
	error("TODO: CWnd::GetNextDlgGroupItem");
}

BOOL CWnd::GotoDlgCtrl(CWnd *pWndCtrl) {
	error("TODO: CWnd::GotoDlgCtrl");
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

	Graphics::PixelFormat format = g_system->getScreenFormat();
	_surfaceBitmap.create(*AfxGetApp()->getScreen(), _windowRect);

	return true;
}

BOOL CWnd::SetDlgItemText(int nIDDlgItem, LPCSTR lpString) {
	error("TODO: CWnd::SetDlgItemText");
}

int CWnd::GetDlgCtrlID() const {
	error("TODO: CWnd::GetDlgCtrlID");
}

void CWnd::CheckDlgButton(int nIDButton, UINT nCheck) {
	error("TODO: CWnd::CheckDlgButton");
}

LRESULT CWnd::SendDlgItemMessage(int nID, UINT message,
                                 WPARAM wParam, LPARAM lParam) const {
	error("TODO: CWnd::SendDlgItemMessage");
}

UINT_PTR CWnd::SetTimer(UINT_PTR nIDEvent, UINT nElapse,
                        void (CALLBACK *lpfnTimer)(HWND, UINT, UINT_PTR, DWORD)) {
	error("TODO: CWnd::SetTimer");
}

BOOL CWnd::KillTimer(UINT_PTR nIDEvent) {
	error("TODO: CWnd::KillTimer");
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
	// Parse the template and use it to load controls
	Gfx::CDialogTemplate dt(dlgTemplate);
	dt.loadTemplate(this);

	SendMessage(WM_INITDIALOG);
	ShowWindow(SW_SHOWNORMAL);
}

void CWnd::GetMessage(MSG &msg) {
	Libs::Event ev;

	// Check for any existing messages
	if (!_messages.empty()) {
		msg = _messages.pop();
		return;
	}

	// Poll for event in ScummVM event manager
	if (AfxGetApp()->pollEvents(ev))
		// Convert other event types
		msg = ev;
	else
		msg.message = WM_NULL;

	msg.hwnd = this;
}

} // namespace MFC
} // namespace Bagel
