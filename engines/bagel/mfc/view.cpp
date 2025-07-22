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

IMPLEMENT_DYNAMIC(CView, CWnd)
BEGIN_MESSAGE_MAP(CView, CWnd)
END_MESSAGE_MAP()

BOOL CView::PreCreateWindow(CREATESTRUCT &cs) {
	assert(cs.style & WS_CHILD);
	return TRUE;
}

int CView::OnCreate(LPCREATESTRUCT lpcs) {
	if (CWnd::OnCreate(lpcs) == -1)
		return -1;

	// If ok, wire in the current document
	assert(m_pDocument == nullptr);
	CCreateContext *pContext = (CCreateContext *)lpcs->lpCreateParams;

	// A view should be created in a given context!
	if (pContext != nullptr && pContext->m_pCurrentDoc != nullptr) {
		pContext->m_pCurrentDoc->AddView(this);
		assert(m_pDocument != nullptr);
	} else {
		warning("Creating a pane with no CDocument.\n");
	}

	return 0;
}

void CView::OnDestroy() {
	CFrameWnd *pFrame = GetParentFrame();
	if (pFrame != nullptr && pFrame->GetActiveView() == this)
		pFrame->SetActiveView(nullptr);    // deactivate during death
	CWnd::OnDestroy();
}

// self destruction
void CView::PostNcDestroy() {
	// default for views is to allocate them on the heap
	//  the default post-cleanup is to 'delete this'.
	//  never explicitly call 'delete' on a view
	delete this;
}

void CView::OnInitialUpdate() {
	OnUpdate(nullptr, 0, nullptr);
}

void CView::OnUpdate(CView *pSender, LPARAM /*lHint*/, CObject * /*pHint*/) {
	assert(pSender != this);
	Invalidate(TRUE);
}

} // namespace MFC
} // namespace Bagel
