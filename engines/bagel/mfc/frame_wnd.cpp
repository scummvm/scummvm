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
#ifdef TODO
	// if the frame does not have an active view, set to first pane
	CView *pView = NULL;
	if (GetActiveView() == NULL)
	{
		CWnd *pWnd = GetDescendantWindow(AFX_IDW_PANE_FIRST, TRUE);
		if (pWnd != NULL && pWnd->IsKindOf(RUNTIME_CLASS(CView)))
		{
			pView = (CView *)pWnd;
			SetActiveView(pView, FALSE);
		}
	}

	if (bMakeVisible)
	{
		// send initial update to all views (and other controls) in the frame
		SendMessageToDescendants(WM_INITIALUPDATE, 0, 0, TRUE, TRUE);

		// give view a chance to save the focus (CFormView needs this)
		if (pView != NULL)
			pView->OnActivateFrame(WA_INACTIVE, this);

		// finally, activate the frame
		// (send the default show command unless the main desktop window)
		int nCmdShow = -1;      // default
		CWinApp *pApp = AfxGetApp();
		if (pApp != NULL && pApp->m_pMainWnd == this)
		{
			nCmdShow = pApp->m_nCmdShow; // use the parameter from WinMain
			pApp->m_nCmdShow = -1; // set to default after first time
		}
		ActivateFrame(nCmdShow);
		if (pView != NULL)
			pView->OnActivateView(TRUE, pView, pView);
	}

	// update frame counts and frame title (may already have been visible)
	if (pDoc != NULL)
		pDoc->UpdateFrameCounts();
	OnUpdateFrameTitle(TRUE);
#endif
}

} // namespace MFC
} // namespace Bagel
