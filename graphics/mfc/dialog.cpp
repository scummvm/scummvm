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
#include "graphics/mfc/afxwin.h"
#include "graphics/mfc/keyboard.h"

namespace Graphics {
namespace MFC {

IMPLEMENT_DYNAMIC(CDialog, CWnd)
BEGIN_MESSAGE_MAP(CDialog, CWnd)
	ON_COMMAND(IDOK, CDialog::OnOK)
	ON_COMMAND(IDCANCEL, CDialog::OnCancel)
	ON_MESSAGE(WM_INITDIALOG, CDialog::HandleInitDialog)
	ON_MESSAGE(WM_SETFONT, CDialog::HandleSetFont)
	ON_WM_SYSCHAR()
	ON_WM_CLOSE()
	ON_WM_ACTIVATE()
END_MESSAGE_MAP()

CDialog::CDialog(const char *lpszTemplateName, CWnd *pParentWnd) {
	SetParent(pParentWnd);
	m_lpszTemplateName = lpszTemplateName;

	if (HIWORD(m_lpszTemplateName) == 0)
		m_nIDHelp = LOWORD(m_lpszTemplateName);
}

CDialog::CDialog(unsigned int nIDTemplate, CWnd *pParentWnd) {
	SetParent(pParentWnd);
	m_lpszTemplateName = MAKEINTRESOURCE(nIDTemplate);
	m_nIDHelp = nIDTemplate;
}

bool CDialog::Create(const char *lpszTemplateName,
	CWnd *pParentWnd) {
	m_lpszTemplateName = lpszTemplateName;  // used for help
	SetParent(pParentWnd);

	if (HIWORD(m_lpszTemplateName) == 0 && m_nIDHelp == 0)
		m_nIDHelp = LOWORD(m_lpszTemplateName);

	HINSTANCE hInst = AfxFindResourceHandle(lpszTemplateName, RT_DIALOG);
	HRSRC hResource = FindResource(hInst, lpszTemplateName, RT_DIALOG);
	HGLOBAL hTemplate = LoadResource(hInst, hResource);
	bool bResult = CreateIndirect(hTemplate, m_pParentWnd, hInst);
	FreeResource(hTemplate);

	return bResult;
}

bool CDialog::Create(unsigned int nIDTemplate, CWnd *pParentWnd) {
	return Create(MAKEINTRESOURCE(nIDTemplate), pParentWnd);
}

int CDialog::DoModal() {
	// can be constructed with a resource template or InitModalIndirect
	assert(m_lpszTemplateName != nullptr || m_hDialogTemplate != nullptr ||
		m_lpDialogTemplate != nullptr);

	// Save the prior focus, if any
	CWnd *oldWin = AfxGetApp()->GetActiveWindow();
	CWnd *oldFocus = GetFocus();
	if (oldFocus)
		AfxGetApp()->SetFocus(nullptr);

	// load resource as necessary
	LPCDLGTEMPLATE lpDialogTemplate = m_lpDialogTemplate;
	HGLOBAL hDialogTemplate = m_hDialogTemplate;
	HINSTANCE hInst = AfxGetResourceHandle();

	if (m_lpszTemplateName != nullptr) {
		hInst = AfxFindResourceHandle(m_lpszTemplateName, RT_DIALOG);
		HRSRC hResource = FindResource(hInst, m_lpszTemplateName, RT_DIALOG);
		assert(hResource);

		hDialogTemplate = LoadResource(hInst, hResource);
	}

	// Set up pointer to template data
	if (hDialogTemplate != nullptr)
		lpDialogTemplate = (LPCDLGTEMPLATE)LockResource(hDialogTemplate);

	// Ensure the template is presenet
	if (lpDialogTemplate == nullptr)
		return -1;

	// Set the dialog parent to be the app window
	if (!m_pParentWnd)
		m_pParentWnd = AfxGetApp()->m_pMainWnd;

	if (CreateDlgIndirect(lpDialogTemplate,
		this /*m_pParentWnd*/, hInst)) {
		AfxGetApp()->doModal(this);
	}

	// Finish the dialog
	PostModal();

	// unlock/free resources as necessary
	if (m_lpszTemplateName != NULL || m_hDialogTemplate != NULL)
		UnlockResource(hDialogTemplate);
	if (m_lpszTemplateName != NULL)
		FreeResource(hDialogTemplate);

	// Restore any old focus
	if (oldFocus && AfxGetApp()->GetActiveWindow() == oldWin)
		oldFocus->SetFocus();

	if (_modalResult == DEFAULT_MODAL_RESULT)
		_modalResult = -1;

	return _modalResult;
}

bool CDialog::CreateDlgIndirect(LPCDLGTEMPLATE lpDialogTemplate,
	CWnd *pParentWnd, HINSTANCE hInst) {
	assert(lpDialogTemplate != nullptr);

	if (hInst == nullptr)
		hInst = AfxGetInstanceHandle();

	HGLOBAL hTemplate = nullptr;

#if 0
	// If no font specified, set the system font.
	CString strFace;
	uint16 wSize = 0;
	bool bSetSysFont = !CDialogTemplate::GetFont(lpDialogTemplate, strFace,
		wSize);
	if (bSetSysFont) {
		CDialogTemplate dlgTemp(lpDialogTemplate);
		dlgTemp.SetSystemFont(wSize);
		hTemplate = dlgTemp.Detach();
	}
#endif
	if (hTemplate != nullptr)
		lpDialogTemplate = (LPDLGTEMPLATE)GlobalLock(hTemplate);

	// setup for modal loop and creation
	_modalResult = DEFAULT_MODAL_RESULT;
	m_nFlags |= WF_CONTINUEMODAL;

	// Create modeless dialog
	createDialogIndirect(lpDialogTemplate);

	if (hTemplate != nullptr) {
		GlobalUnlock(hTemplate);
		GlobalFree(hTemplate);
	}

	return true;
}

void CDialog::SetParent(CWnd *wnd) {
	m_pParentWnd = wnd;
}

uint32 CDialog::GetDefID() const {
	return _defaultId;
}

void CDialog::SetDefID(unsigned int nID) {
	CWnd *oldBtn;

	if (_defaultId && (oldBtn = GetDlgItem(_defaultId)) != nullptr) {
		uint32 style = oldBtn->GetStyle();

		if ((style & 0xf) != BS_OWNERDRAW) {
			style &= ~BS_DEFPUSHBUTTON;
			oldBtn->SetStyle(style);
		}
	}

	_defaultId = nID;

	// Set new default
	CWnd *newBtn = GetDlgItem(nID);
	if (newBtn) {
		uint32 style = newBtn->GetStyle();

		if ((style & 0xf) != BS_OWNERDRAW) {
			style = (style & ~0xF) | BS_DEFPUSHBUTTON;
			newBtn->SetStyle(style);
		}
	}
}

LRESULT CDialog::HandleInitDialog(WPARAM, LPARAM) {
	PreInitDialog();

	OnInitDialog();
	return 0;
}

LRESULT CDialog::HandleSetFont(WPARAM wParam, LPARAM) {
	// Allow the CWnd code to set the _hFont field
	CWnd::OnSetFont((HFONT)wParam, true);

	// Virtual method dialog descendants can override
	OnSetFont(CFont::FromHandle((HFONT)wParam));
	return 0;
}

CButton *CDialog::GetDefaultPushButton() const {
	for (auto &child : _children) {
		CButton *pChild = dynamic_cast<CButton *>(child._value);

		if (pChild) {
			uint32 style = pChild->GetStyle();
			if ((style & BS_DEFPUSHBUTTON) && pChild->GetDlgCtrlID() == IDOK) {
				return pChild;
			}
		}
	}

	return nullptr;
}

void CDialog::DDX_Control(CDataExchange *pDX, int nIDC, CWnd &rControl) {
	rControl.SubclassDlgItem(nIDC, this);
}

void CDialog::DDX_Radio(CDataExchange *pDX,
	int nIDCButton1, int &value) {
	error("TODO: CDialog::DDX_Radio");
}

void CDialog::DDX_Text(CDataExchange *pDX, int nIDC, int &value) {
	error("TODO: CDialog::DDX_Text");
}

void CDialog::DDX_Text(CDataExchange *pDX, int nIDC, CString &value) {
	CEdit *edit = dynamic_cast<CEdit *>(GetDlgItem(nIDC));
	assert(edit);

	if (pDX->m_bSaveAndValidate) {
		edit->GetWindowText(value);
	} else {
		edit->SetWindowText(value.c_str());
	}
}

void CDialog::DDX_Text(CDataExchange *pDX, int nIDC, unsigned int &value) {
	error("TODO: CDialog::DDX_Text");
}
void CDialog::DDX_Text(CDataExchange *pDX, int nIDC, long &value) {
	error("TODO: CDialog::DDX_Text");
}
void CDialog::DDX_Text(CDataExchange *pDX, int nIDC, double &value) {
	error("TODO: CDialog::DDX_Text");
}

void CDialog::DDX_Check(CDataExchange *pDX,
	int nIDC, bool value) {
	error("CDialog::DDX_Check");
}

void CDialog::DDV_MinMaxInt(CDataExchange *pDX,
	int value, int nMin, int nMax) {
	error("TODO: CDialog::DDV_MinMaxInt");
}

void CDialog::EndDialog(int nResult) {
	_modalResult = nResult;
}

bool CDialog::UpdateData(bool bSaveAndValidate) {
	if (bSaveAndValidate) {
		CDataExchange exchange = { true };
		DoDataExchange(&exchange);
	}

	return true;
}

void CDialog::OnOK() {
	if (!UpdateData(true))
		return;

	EndDialog(IDOK);
}

void CDialog::OnCancel() {
	EndDialog(IDCANCEL);
}

bool CDialog::CreateIndirect(LPCDLGTEMPLATE lpDialogTemplate,
	CWnd *pParentWnd, void *lpDialogInit, HINSTANCE hInst) {
	error("TODO: CDialog::CreateIndirect");
}

bool CDialog::CreateIndirect(HGLOBAL hDialogTemplate,
	CWnd *pParentWnd, HINSTANCE hInst) {
	error("TODO: CDialog::CreateIndirect");
}

void CDialog::OnSysChar(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	// Convert input character to uppercase for case-insensitive match
	char pressedChar = toupper(nChar);
	for (auto &child : _children) {
		CWnd *pChild = child._value;
		CString text;
		pChild->GetWindowText(text);

		// Check for ampersand for hotkey underlining
		uint idx = text.findFirstOf('&');

		if (idx != Common::String::npos &&
			pressedChar == toupper(text[idx + 1])) {
			// Found match - simulate button click
			pChild->SendMessage(BM_CLICK);
			return;
		}
	}

	// If no match found, pass to base class
	CWnd::OnSysChar(nChar, nRepCnt, nFlags);
}

void CDialog::OnActivate(unsigned int nState, CWnd *pWndOther, bool bMinimized) {
	if (nState != WA_INACTIVE) {
		// Invalidate the dialog and its children
		Invalidate(true);
		for (auto child : _children)
			child._value->Invalidate(true);
	}

	CWnd::OnActivate(nState, pWndOther, bMinimized);
}

bool CDialog::IsDialogMessage(LPMSG lpMsg) {
	if (lpMsg->message != WM_KEYDOWN)
		return false;

	switch (lpMsg->wParam) {
	case VK_RETURN:
		return handleEnterKey(lpMsg);

	case VK_ESCAPE:
		return handleEscapeKey(lpMsg);

	default:
		break;
	}

	return false;
}

bool CDialog::handleEnterKey(LPMSG lpMsg) {
	// Check for a focused control. We can close immediately if there isn't
	HWND hFocus = GetFocus();
	if (!hFocus) {
		OnOK();
		return true;
	}

	// Ask control what it wants
	LRESULT dlgCode = MFC::SendMessage(hFocus, WM_GETDLGCODE,
		lpMsg->wParam, (LPARAM)lpMsg);

	// If control wants Enter, do nothing
	if (dlgCode & DLGC_WANTMESSAGE)
		return false;

	// Multiline edits implicitly want Enter
	if (dlgCode & DLGC_HASSETSEL)
		return false;

	// Find default push button
	CButton *pDefault = GetDefaultPushButton();
	if (!pDefault) {
		// No default button, so go ahead and close dialog
		OnOK();
		return true;
	}

	if (!pDefault->IsWindowEnabled())
		// consume Enter, do nothing
		return true;

	// Simulate button click
	sendButtonClicked(pDefault);
	return true;
}

bool CDialog::handleEscapeKey(LPMSG lpMsg) {
	// Get the default Cancel button
	CButton *pCancel = dynamic_cast<CButton *>(GetDlgItem(IDCANCEL));
	if (!pCancel) {
		// No button, so close directly
		OnCancel();
		return true;
	}

	if (!pCancel->IsWindowEnabled())
		return true;

	sendButtonClicked(pCancel);
	return true;
}

void CDialog::sendButtonClicked(CButton *btn) {
	int id = btn->GetDlgCtrlID();
	SendMessage(WM_COMMAND, MAKEWPARAM(id, BN_CLICKED), (LPARAM)btn->m_hWnd);
}

} // namespace MFC
} // namespace Graphics
