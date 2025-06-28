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

IMPLEMENT_DYNAMIC(CDialog, CWnd)
BEGIN_MESSAGE_MAP(CDialog, CWnd)
ON_COMMAND(IDOK, CDialog::OnOK)
ON_COMMAND(IDCANCEL, CDialog::OnCancel)
ON_MESSAGE(WM_INITDIALOG, CDialog::HandleInitDialog)
ON_MESSAGE(WM_SETFONT, CDialog::HandleSetFont)
END_MESSAGE_MAP()

CDialog::CDialog(LPCSTR lpszTemplateName, CWnd *pParentWnd) {
	SetParent(pParentWnd);
	m_lpszTemplateName = lpszTemplateName;

	if (HIWORD(m_lpszTemplateName) == 0)
		m_nIDHelp = LOWORD(m_lpszTemplateName);
}

CDialog::CDialog(UINT nIDTemplate, CWnd *pParentWnd) {
	SetParent(pParentWnd);
	m_lpszTemplateName = MAKEINTRESOURCE(nIDTemplate);
	m_nIDHelp = nIDTemplate;
}

BOOL CDialog::Create(LPCSTR lpszTemplateName,
        CWnd *pParentWnd) {
	m_lpszTemplateName = lpszTemplateName;  // used for help
	SetParent(pParentWnd);

	if (HIWORD(m_lpszTemplateName) == 0 && m_nIDHelp == 0)
		m_nIDHelp = LOWORD(m_lpszTemplateName);

	HINSTANCE hInst = AfxFindResourceHandle(lpszTemplateName, RT_DIALOG);
	HRSRC hResource = FindResource(hInst, lpszTemplateName, RT_DIALOG);
	HGLOBAL hTemplate = LoadResource(hInst, hResource);
	BOOL bResult = CreateIndirect(hTemplate, m_pParentWnd, hInst);
	FreeResource(hTemplate);

	return bResult;
}

BOOL CDialog::Create(UINT nIDTemplate, CWnd *pParentWnd) {
	return Create(MAKEINTRESOURCE(nIDTemplate), pParentWnd);
}

int CDialog::DoModal() {
	// can be constructed with a resource template or InitModalIndirect
	assert(m_lpszTemplateName != nullptr || m_hDialogTemplate != nullptr ||
		m_lpDialogTemplate != nullptr);

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
		auto *app = AfxGetApp();
		app->SetActiveWindow(this);

		SendMessage(WM_ACTIVATE, MAKEWPARAM(WA_ACTIVE, false), 0);
		app->runEventLoop();
		SendMessage(WM_ACTIVATE, MAKEWPARAM(WA_INACTIVE, false), 0);
	}

	// Finish the dialog
	DestroyWindow();
	PostModal();

	// unlock/free resources as necessary
	if (m_lpszTemplateName != NULL || m_hDialogTemplate != NULL)
		UnlockResource(hDialogTemplate);
	if (m_lpszTemplateName != NULL)
		FreeResource(hDialogTemplate);

	return m_nModalResult;
}

BOOL CDialog::CreateDlgIndirect(LPCDLGTEMPLATE lpDialogTemplate,
	CWnd *pParentWnd, HINSTANCE hInst) {
	assert(lpDialogTemplate != nullptr);

	if (hInst == nullptr)
		hInst = AfxGetInstanceHandle();

	HGLOBAL hTemplate = nullptr;

#if 0
	// If no font specified, set the system font.
	CString strFace;
	WORD wSize = 0;
	BOOL bSetSysFont = !CDialogTemplate::GetFont(lpDialogTemplate, strFace,
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
	m_nModalResult = -1;
	m_nFlags |= WF_CONTINUEMODAL;

	// Create modeless dialog
	createDialogIndirect(lpDialogTemplate);

	if (hTemplate != nullptr) {
		GlobalUnlock(hTemplate);
		GlobalFree(hTemplate);
	}

	return TRUE;
}

void CDialog::SetParent(CWnd *wnd) {
	m_pParentWnd = wnd;
}

DWORD CDialog::GetDefID() const {
	return _defaultId;
}

void CDialog::SetDefID(UINT nID) {
	CWnd *oldBtn;

	if (_defaultId && (oldBtn = GetDlgItem(_defaultId)) != nullptr) {
		DWORD style = oldBtn->GetStyle();
		style &= ~BS_DEFPUSHBUTTON;
		oldBtn->SetStyle(style);
	}

	_defaultId = nID;

	// Set new default
	CWnd *newBtn = GetDlgItem(nID);
	if (newBtn) {
		DWORD style = newBtn->GetStyle();
		style = (style & ~0xF) | BS_DEFPUSHBUTTON;
		newBtn->SetStyle(style);
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

void CDialog::DDX_Control(CDataExchange *pDX, int nIDC, CWnd &rControl) {
	error("TODO: CDialog::DDX_Control");
}

void CDialog::DDX_Radio(CDataExchange *pDX,
                        int nIDCButton1, int &value) {
	error("TODO: CDialog::DDX_Radio");
}

void CDialog::DDX_Text(CDataExchange *pDX, int nIDC, int &value) {
	error("TODO: CDialog::DDX_Text");
}
void CDialog::DDX_Text(CDataExchange *pDX, int nIDC, CString &value) {
	error("TODO: CDialog::DDX_Text");
}
void CDialog::DDX_Text(CDataExchange *pDX, int nIDC, UINT &value) {
	error("TODO: CDialog::DDX_Text");
}
void CDialog::DDX_Text(CDataExchange *pDX, int nIDC, long &value) {
	error("TODO: CDialog::DDX_Text");
}
void CDialog::DDX_Text(CDataExchange *pDX, int nIDC, double &value) {
	error("TODO: CDialog::DDX_Text");
}

void CDialog::DDX_Check(CDataExchange *pDX,
                        int nIDC, int &value) {
	error("CDialog::DDX_Check");
}

void CDialog::DDV_MinMaxInt(CDataExchange *pDX,
                            int value, int nMin, int nMax) {
	error("TODO: CDialog::DDV_MinMaxInt");
}

void CDialog::EndDialog(int nResult) {
	m_nModalResult = nResult;
}

BOOL CDialog::UpdateData(BOOL bSaveAndValidate) {
	error("TODO: CDialog::UpdateData");
}

void CDialog::OnOK() {
	error("TODO: CDialog::OnOK");
}

void CDialog::OnCancel() {
	error("TODO: CDialog::OnCancel");
}

BOOL CDialog::CreateIndirect(LPCDLGTEMPLATE lpDialogTemplate,
		CWnd *pParentWnd, void *lpDialogInit, HINSTANCE hInst) {
	error("TODO: CDialog::CreateIndirect");
}

BOOL CDialog::CreateIndirect(HGLOBAL hDialogTemplate,
		CWnd *pParentWnd, HINSTANCE hInst) {
	error("TODO: CDialog::CreateIndirect");
}

} // namespace MFC
} // namespace Bagel
