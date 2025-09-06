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

IMPLEMENT_DYNAMIC(CDocTemplate, CCmdTarget)
BEGIN_MESSAGE_MAP(CDocTemplate, CCmdTarget)
END_MESSAGE_MAP()

CDocTemplate::CDocTemplate(unsigned int nIDResource, const CRuntimeClass *pDocClass,
		const CRuntimeClass *pFrameClass, const CRuntimeClass *pViewClass) {
	assert(nIDResource != 0);
	ASSERT(pDocClass == nullptr ||
		pDocClass->IsDerivedFrom(RUNTIME_CLASS(CDocument)));
	ASSERT(pFrameClass == nullptr ||
		pFrameClass->IsDerivedFrom(RUNTIME_CLASS(CFrameWnd)));
	ASSERT(pViewClass == nullptr ||
		pViewClass->IsDerivedFrom(RUNTIME_CLASS(CView)));

	m_nIDResource = nIDResource;
	m_pDocClass = pDocClass;
	m_pFrameClass = pFrameClass;
	m_pViewClass = pViewClass;
}

CDocument *CDocTemplate::CreateNewDocument() {
	assert(m_pDocClass);
	CDocument *pDocument = (CDocument *)m_pDocClass->CreateObject();
	assert(pDocument);
	ASSERT_KINDOF(CDocument, pDocument);

	AddDocument(pDocument);
	return pDocument;
}

void CDocTemplate::AddDocument(CDocument *pDoc) {
	ASSERT_VALID(pDoc);
	assert(pDoc->m_pDocTemplate == nullptr);   // no template attached yet
	pDoc->m_pDocTemplate = this;
}

void CDocTemplate::RemoveDocument(CDocument *pDoc) {
	ASSERT_VALID(pDoc);
	assert(pDoc->m_pDocTemplate == this);   // must be attached to us
	pDoc->m_pDocTemplate = nullptr;
}

CFrameWnd *CDocTemplate::CreateNewFrame(CDocument *pDoc, CFrameWnd *pOther) {
	if (pDoc != nullptr)
		ASSERT_VALID(pDoc);

	// Create a frame wired to the specified document
	// Must have a resource ID to load from
	assert(m_nIDResource != 0);
	CCreateContext context;
	context.m_pCurrentFrame = pOther;
	context.m_pCurrentDoc = pDoc;
	context.m_pNewViewClass = m_pViewClass;
	context.m_pNewDocTemplate = this;

	assert(m_pFrameClass);
	CFrameWnd *pFrame = (CFrameWnd *)m_pFrameClass->CreateObject();
	assert(pFrame);
	ASSERT_KINDOF(CFrameWnd, pFrame);
	assert(context.m_pNewViewClass);

	// Create new from resource
	if (!pFrame->LoadFrame(m_nIDResource,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE,
		nullptr, &context))
	{
		warning("CDocTemplate couldn't create a frame.");
		// frame will be deleted in PostNcDestroy cleanup
		return nullptr;
	}

	return pFrame;
}

void CDocTemplate::InitialUpdateFrame(CFrameWnd *pFrame, CDocument *pDoc,
		bool bMakeVisible) {
	// Just delagate to implementation in CFrameWnd
	pFrame->InitialUpdateFrame(pDoc, bMakeVisible);
}


void CDocTemplate::LoadTemplate() {
	if (m_strDocStrings.IsEmpty() && !m_strDocStrings.LoadString(m_nIDResource)) {
		warning("No document names in string for template #%d.",
			m_nIDResource);
	}

	// TODO: If needed, menus and accelerators
}

void CDocTemplate::CloseAllDocuments(bool) {
	POSITION pos = GetFirstDocPosition();
	while (pos != NULL)
	{
		CDocument *pDoc = GetNextDoc(pos);
		pDoc->OnCloseDocument();
	}
}

} // namespace MFC
} // namespace Bagel
