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

namespace Graphics {
namespace MFC {


IMPLEMENT_DYNAMIC(CDocument, CCmdTarget)
BEGIN_MESSAGE_MAP(CDocument, CCmdTarget)
END_MESSAGE_MAP()

const CString &CDocument::GetTitle() const {
	return _title;
}

void CDocument::SetTitle(const char *lpszTitle) {
	_title = CString(lpszTitle);
}

const CString &CDocument::GetPathName() const {
	return _unusedPathName;
}

void CDocument::SetPathName(const char *lpszPathName, bool bAddToMRU) {
}

void CDocument::ClearPathName() {
}

bool CDocument::IsModified() {
	return m_bModified;
}

void CDocument::SetModifiedFlag(bool bModified) {
	m_bModified = bModified;
}

void CDocument::AddView(CView *pView) {
	assert(dynamic_cast<CView *>(pView) != nullptr);
	assert(pView->m_pDocument == nullptr); // must not be already attached
	assert(!m_viewList.contains(pView));   // must not be in list

	m_viewList.push_back(pView);
	assert(pView->m_pDocument == nullptr); // must be un-attached
	pView->m_pDocument = this;

	OnChangedViewList();    // must be the last thing done to the document
}

void CDocument::RemoveView(CView *pView) {
	ASSERT_VALID(pView);
	assert(pView->m_pDocument == this); // Must be attached to us

	m_viewList.remove(pView);
	pView->m_pDocument = nullptr;

	// Must be the last thing done to the document
	OnChangedViewList();
}

void CDocument::DeleteContents() {
}

void CDocument::ReportSaveLoadException(const char *lpszPathName,
        CException *e, bool bSaving, unsigned int nIDPDefault) {
	error("Save/load error");
}

void CDocument::UpdateAllViews(CView *pSender, LPARAM lHint,
        CObject *pHint) {
	assert(pSender == nullptr || !m_viewList.empty());

	for (ViewArray::iterator pos = m_viewList.begin();
			pos != m_viewList.end(); ++pos) {
		CView *pView = *pos;
		if (pView != pSender)
			pView->OnUpdate(pSender, lHint, pHint);
	}
}

POSITION CDocument::GetFirstViewPosition() {
	return m_viewList.empty() ? nullptr :
		(POSITION)& m_viewList[0];
}

CView *CDocument::GetNextView(POSITION &rPosition) const {
	if (rPosition == nullptr)
		return nullptr;

	CView **&rPos = reinterpret_cast<CView **&>(rPosition);
	const CView *const *rEnd = &m_viewList[0] + m_viewList.size();

	if (rPos >= rEnd)
		return nullptr;

	return *rPos++;
}

bool CDocument::SaveModified() {
	// TODO: CDocument::SaveModified
	return true;
}

void CDocument::OnCloseDocument() {
	// Destroy all frames viewing this document
	// the last destroy may destroy us
	bool bAutoDelete = m_bAutoDelete;
	m_bAutoDelete = false;  // don't destroy document while closing views

	while (!m_viewList.empty()) {
		size_t arrSize = m_viewList.size();

		// get frame attached to the view
		CView *pView = m_viewList.front();
		ASSERT_VALID(pView);
		CFrameWnd *pFrame = pView->GetParentFrame();
		ASSERT_VALID(pFrame);

		// and close it
		PreCloseFrame(pFrame);
		pFrame->DestroyWindow();

		// will destroy the view as well
		assert(m_viewList.size() < arrSize);
	}
	m_bAutoDelete = bAutoDelete;

	// clean up contents of document before destroying the document itself
	DeleteContents();

	// delete the document if necessary
	if (m_bAutoDelete)
		delete this;
}

void CDocument::OnChangedViewList() {
	// if no more views on the document, delete ourself
	// not called if directly closing the document or terminating the app
	if (m_viewList.empty() && m_bAutoDelete) {
		OnCloseDocument();
		return;
	}

	// update the frame counts as needed
	UpdateFrameCounts();
}

} // namespace MFC
} // namespace Graphics
