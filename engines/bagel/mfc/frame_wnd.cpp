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

const CRect CFrameWnd::rectDefault;

IMPLEMENT_DYNAMIC(CFrameWnd, CWnd)
BEGIN_MESSAGE_MAP(CFrameWnd, CWnd)
END_MESSAGE_MAP()

HMENU CFrameWnd::GetMenu() const {
	error("TODO: CFrameWnd::GetMenu");
}

void CFrameWnd::RecalcLayout(BOOL bNotify) {
	error("TODO: CFrameWnd::RecalcLayout");
}

BOOL CFrameWnd::RepositionBars(UINT nIDFirst, UINT nIDLast,
                               UINT nIDLeftOver, UINT nFlag, LPRECT lpRectParam,
                               LPCRECT lpRectClient, BOOL bStretch) {
	error("TODO: CFrameWnd::RepositionBars");
}

void CFrameWnd::InitialUpdateFrame(CDocument *pDoc, BOOL bMakeVisible) {
	// if the frame does not have an active view, set to first pane
	CView *pView = nullptr;
	if (GetActiveView() == nullptr) {
		CWnd *pWnd = _children.empty() ? nullptr :
			_children.begin()->_value;

		if (pWnd != nullptr && pWnd->IsKindOf(RUNTIME_CLASS(CView))) {
			pView = (CView *)pWnd;
			SetActiveView(pView, FALSE);
		}
	}

	if (bMakeVisible) {
		// send initial update to all views (and other controls) in the frame
		SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, TRUE, TRUE);

		// give view a chance to save the focus (CFormView needs this)
		if (pView != nullptr)
			pView->OnActivateFrame(WA_INACTIVE, this);

		// finally, activate the frame
		// (send the default show command unless the main desktop window)
		int nCmdShow = -1;      // default
		CWinApp *pApp = AfxGetApp();
		if (pApp != nullptr && pApp->m_pMainWnd == this) {
			nCmdShow = pApp->m_nCmdShow; // use the parameter from WinMain
			pApp->m_nCmdShow = -1; // set to default after first time
		}

		ActivateFrame(nCmdShow);

		if (pView != nullptr)
			pView->OnActivateView(TRUE, pView, pView);
	}

	// update frame counts and frame title (may already have been visible)
	if (pDoc != nullptr)
		pDoc->UpdateFrameCounts();

	OnUpdateFrameTitle(TRUE);
}

BOOL CFrameWnd::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle,
	CWnd *pParentWnd, CCreateContext *pContext) {
	assert(m_nIDHelp == 0 || m_nIDHelp == nIDResource);
	m_nIDHelp = nIDResource;    // ID for help context (+HID_BASE_RESOURCE)

	CString strFullString;
	if (strFullString.LoadString(nIDResource))
		AfxExtractSubString(m_strTitle, strFullString, 0);    // first sub-string

	//VERIFY(AfxDeferRegisterClass(AFX_WNDFRAMEORVIEW_REG));

	// Create the window
	LPCSTR lpszClass = nullptr; //GetIconWndClass(dwDefaultStyle, nIDResource);
	LPCSTR lpszTitle = m_strTitle.c_str();
	if (!Create(lpszClass, lpszTitle, dwDefaultStyle, rectDefault,
		pParentWnd, MAKEINTRESOURCE(nIDResource), 0L, pContext)) {
		return FALSE;   // will self destruct on failure normally
	}

	ASSERT(m_hWnd != nullptr);

	// TODO: Menu and accelerators if needed

	if (pContext == nullptr)   // send initial update
		SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, TRUE, TRUE);

	return TRUE;
}


BOOL CFrameWnd::Create(LPCSTR lpszClassName,
		LPCSTR lpszWindowName, DWORD dwStyle,
		const RECT &rect, CWnd *pParentWnd,
		LPCSTR lpszMenuName, DWORD dwExStyle,
		CCreateContext *pContext) {
	HMENU hMenu = nullptr;
	if (lpszMenuName != nullptr) {
		// TODO: Menu loading if needed
	}

	m_strTitle = lpszWindowName;    // Save title for later

	if (!CreateEx(dwExStyle, lpszClassName, lpszWindowName, dwStyle,
			rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
			pParentWnd ? pParentWnd->GetSafeHwnd() : nullptr,
			hMenu, (LPVOID)pContext)) {
		warning("failed to create CFrameWnd.");

		if (hMenu != nullptr)
			DestroyMenu(hMenu);
		return FALSE;
	}

	return TRUE;
}

CView *CFrameWnd::GetActiveView() const {
	ASSERT(m_pViewActive == NULL ||
		m_pViewActive->IsKindOf(RUNTIME_CLASS(CView)));
	return m_pViewActive;
}

void CFrameWnd::SetActiveView(CView *pViewNew, BOOL bNotify) {
	CView *pViewOld = m_pViewActive;
	if (pViewNew == pViewOld)
		return;     // do not re-activate if SetActiveView called more than once

	m_pViewActive = NULL;   // no active for the following processing

	// deactivate the old one
	if (pViewOld != NULL)
		pViewOld->OnActivateView(FALSE, pViewNew, pViewOld);

	// if the OnActivateView moves the active window,
	//    that will veto this change
	if (m_pViewActive != NULL)
		return;     // already set
	m_pViewActive = pViewNew;

	// activate
	if (pViewNew != NULL && bNotify)
		pViewNew->OnActivateView(TRUE, pViewNew, pViewOld);
}

void CFrameWnd::OnSetFocus(CWnd *pOldWnd) {
	if (m_pViewActive != NULL)
		m_pViewActive->SetFocus();
	else
		CWnd::OnSetFocus(pOldWnd);
}

CDocument *CFrameWnd::GetActiveDocument() {
	ASSERT_VALID(this);
	CView *pView = GetActiveView();
	if (pView != NULL)
		return pView->GetDocument();
	return NULL;
}

void CFrameWnd::ActivateFrame(int nCmdShow) {
	// TODO: CFrameWnd::ActivateFrame
}

void CFrameWnd::OnUpdateFrameTitle(BOOL bAddToTitle) {
	// TODO: CFrameWnd::OnUpdateFrameTitle
}


} // namespace MFC
} // namespace Bagel
