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

IMPLEMENT_DYNAMIC(CFrameWnd, CWnd)
BEGIN_MESSAGE_MAP(CFrameWnd, CWnd)
ON_WM_CREATE()
ON_WM_ACTIVATE()
END_MESSAGE_MAP()

HMENU CFrameWnd::GetMenu() const {
	return nullptr;
}

void CFrameWnd::RecalcLayout(bool bNotify) {
	// No implementation in ScummVM
}

bool CFrameWnd::RepositionBars(unsigned int nIDFirst, unsigned int nIDLast,
                               unsigned int nIDLeftOver, unsigned int nFlag, LPRECT lpRectParam,
                               LPCRECT lpRectClient, bool bStretch) {
	error("TODO: CFrameWnd::RepositionBars");
}

void CFrameWnd::InitialUpdateFrame(CDocument *pDoc, bool bMakeVisible) {
	// if the frame does not have an active view, set to first pane
	CView *pView = nullptr;
	if (GetActiveView() == nullptr) {
		CWnd *pWnd = _children.empty() ? nullptr :
			_children.begin()->_value;

		if (pWnd != nullptr && pWnd->IsKindOf(RUNTIME_CLASS(CView))) {
			pView = (CView *)pWnd;
			SetActiveView(pView, false);
		}
	}

	if (bMakeVisible) {
		// send initial update to all views (and other controls) in the frame
		SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, true, true);

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
			pView->OnActivateView(true, pView, pView);
	}

	// update frame counts and frame title (may already have been visible)
	if (pDoc != nullptr)
		pDoc->UpdateFrameCounts();

	OnUpdateFrameTitle(true);
}

bool CFrameWnd::LoadFrame(unsigned int nIDResource, uint32 dwDefaultStyle,
	CWnd *pParentWnd, CCreateContext *pContext) {
	assert(m_nIDHelp == 0 || m_nIDHelp == nIDResource);
	m_nIDHelp = nIDResource;    // ID for help context (+HID_BASE_RESOURCE)

	CString strFullString;
	if (strFullString.LoadString(nIDResource))
		AfxExtractSubString(m_strTitle, strFullString, 0);    // first sub-string

	//VERIFY(AfxDeferRegisterClass(AFX_WNDFRAMEORVIEW_REG));

	// Create the window
	const char *lpszClass = nullptr; //GetIconWndClass(dwDefaultStyle, nIDResource);
	const char *lpszTitle = m_strTitle.c_str();
	CRect rectDefault;
	if (!Create(lpszClass, lpszTitle, dwDefaultStyle, rectDefault,
		pParentWnd, MAKEINTRESOURCE(nIDResource), 0L, pContext)) {
		return false;   // will self destruct on failure normally
	}

	ASSERT(m_hWnd != nullptr);

	// TODO: Menu and accelerators if needed

	if (pContext == nullptr)   // send initial update
		SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, true, true);

	return true;
}


bool CFrameWnd::Create(const char *lpszClassName,
		const char *lpszWindowName, uint32 dwStyle,
		const RECT &rect, CWnd *pParentWnd,
		const char *lpszMenuName, uint32 dwExStyle,
		CCreateContext *pContext) {
	HMENU hMenu = nullptr;
	if (lpszMenuName != nullptr) {
		// TODO: Menu loading if needed
	}

	m_strTitle = lpszWindowName;    // Save title for later

	if (!CreateEx(dwExStyle, lpszClassName, lpszWindowName, dwStyle,
			rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
			pParentWnd ? pParentWnd->GetSafeHwnd() : 0,
			(LPARAM)hMenu, pContext)) {
		warning("failed to create CFrameWnd.");

		if (hMenu != nullptr)
			DestroyMenu(hMenu);
		return false;
	}

	return true;
}

int CFrameWnd::OnCreate(LPCREATESTRUCT lpcs) {
	CCreateContext *pContext = (CCreateContext *)lpcs->lpCreateParams;
	return OnCreateHelper(lpcs, pContext);
}

int CFrameWnd::OnCreateHelper(LPCREATESTRUCT lpcs, CCreateContext *pContext) {
	if (CWnd::OnCreate(lpcs) == -1)
		return -1;

	// Create the client view, if any
	if (!OnCreateClient(lpcs, pContext)) {
		warning("Failed to create client pane/view for frame.\n");
		return -1;
	}

	RecalcLayout();
	return 0;
}

bool CFrameWnd::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext *pContext) {
	if (pContext != nullptr && pContext->m_pNewViewClass != nullptr) {
		if (CreateView(pContext, AFX_IDW_PANE_FIRST) == nullptr)
			return false;
	}
	return true;
}

CWnd *CFrameWnd::CreateView(CCreateContext *pContext, unsigned int nID) {
	assert(m_hWnd != nullptr);
	assert(pContext != nullptr);
	assert(pContext->m_pNewViewClass != nullptr);

	CWnd *pView = (CWnd *)pContext->m_pNewViewClass->CreateObject();
	assert(pView);
	ASSERT_KINDOF(CWnd, pView);

	if (!pView->Create(nullptr, nullptr, AFX_WS_DEFAULT_VIEW,
		CRect(0, 0, 0, 0), this, nID, pContext)) {
		return nullptr;        // can't continue without a view
	}

	return pView;
}

CView *CFrameWnd::GetActiveView() const {
	ASSERT(m_pViewActive == nullptr ||
		m_pViewActive->IsKindOf(RUNTIME_CLASS(CView)));
	return m_pViewActive;
}

void CFrameWnd::SetActiveView(CView *pViewNew, bool bNotify) {
	CView *pViewOld = m_pViewActive;
	if (pViewNew == pViewOld)
		return;     // do not re-activate if SetActiveView called more than once

	m_pViewActive = nullptr;   // no active for the following processing

	// deactivate the old one
	if (pViewOld != nullptr)
		pViewOld->OnActivateView(false, pViewNew, pViewOld);

	// if the OnActivateView moves the active window,
	//    that will veto this change
	if (m_pViewActive != nullptr)
		return;     // already set
	m_pViewActive = pViewNew;

	// activate
	if (pViewNew != nullptr && bNotify)
		pViewNew->OnActivateView(true, pViewNew, pViewOld);
}

void CFrameWnd::OnSetFocus(CWnd *pOldWnd) {
	if (m_pViewActive != nullptr)
		m_pViewActive->SetFocus();
	else
		CWnd::OnSetFocus(pOldWnd);
}

CDocument *CFrameWnd::GetActiveDocument() {
	ASSERT_VALID(this);
	CView *pView = GetActiveView();
	if (pView != nullptr)
		return pView->GetDocument();
	return nullptr;
}

void CFrameWnd::ActivateFrame(int nCmdShow) {
	// TODO: CFrameWnd::ActivateFrame
}

void CFrameWnd::OnUpdateFrameTitle(bool bAddToTitle) {
	// TODO: CFrameWnd::OnUpdateFrameTitle
}

void CFrameWnd::OnActivate(unsigned int nState, CWnd *pWndOther, bool bMinimized) {
	if (nState != WA_INACTIVE) {
		// Invalidate the dialog and its children
		Invalidate(true);
		for (auto child : _children)
			child._value->Invalidate(true);
	}

	CWnd::OnActivate(nState, pWndOther, bMinimized);
}

} // namespace MFC
} // namespace Bagel
