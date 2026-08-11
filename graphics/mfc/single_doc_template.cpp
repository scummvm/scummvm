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

IMPLEMENT_DYNAMIC(CSingleDocTemplate, CDocTemplate)
BEGIN_MESSAGE_MAP(CSingleDocTemplate, CDocTemplate)
END_MESSAGE_MAP()

CSingleDocTemplate::~CSingleDocTemplate() {
	assert(!m_pOnlyDoc);
}

CDocument *CSingleDocTemplate::OpenDocumentFile(const char *lpszPathName,
		bool bMakeVisible) {
	CDocument *pDocument = nullptr;
	CFrameWnd *pFrame = nullptr;
	bool bCreated = false;      // => doc and frame created
	bool bWasModified = false;

	if (m_pOnlyDoc != nullptr) {
		// already have a document - reinit it
		pDocument = m_pOnlyDoc;
		if (!pDocument->SaveModified())
			return nullptr;        // leave the original one

		pFrame = (CFrameWnd *)AfxGetMainWnd();
		assert(pFrame != nullptr);
		ASSERT_KINDOF(CFrameWnd, pFrame);
		ASSERT_VALID(pFrame);
	} else {
		// create a new document
		pDocument = CreateNewDocument();
		assert(pFrame == nullptr);     // will be created below
		bCreated = true;
	}

	assert(pDocument == m_pOnlyDoc);

	if (pFrame == nullptr) {
		assert(bCreated);

		// Create frame - set as main document frame
		bool bAutoDelete = pDocument->m_bAutoDelete;
		pDocument->m_bAutoDelete = false;
		// don't destroy if something goes wrong
		pFrame = CreateNewFrame(pDocument, nullptr);
		pDocument->m_bAutoDelete = bAutoDelete;
		assert(pFrame);
	}

	if (lpszPathName == nullptr) {
		// Create a new document
		SetDefaultTitle(pDocument);

		// Avoid creating temporary compound file when starting up invisible
		if (!bMakeVisible)
			pDocument->m_bEmbedded = true;

		if (!pDocument->OnNewDocument()) {
			warning("CDocument::OnNewDocument returned false.");
			if (bCreated)
				pFrame->DestroyWindow();	// Will destroy document
			return nullptr;
		}
	} else {
		// open an existing document
		bWasModified = pDocument->IsModified();
		pDocument->SetModifiedFlag(false);  // not dirty for open

		if (!pDocument->OnOpenDocument(lpszPathName)) {
			warning("CDocument::OnOpenDocument returned false.");
			if (bCreated) {
				pFrame->DestroyWindow();    // will destroy document
			} else if (!pDocument->IsModified()) {
				// Original document is untouched
				pDocument->SetModifiedFlag(bWasModified);
			} else {
				// We corrupted the original document
				SetDefaultTitle(pDocument);

				if (!pDocument->OnNewDocument()) {
					warning("Error: OnNewDocument failed after trying to open a document - trying to continue.");
					// Assume we can continue
				}
			}

			return nullptr;	// Open failed
		}

		pDocument->SetPathName(lpszPathName);
	}

	// Set as main frame
	CWinApp *app = AfxGetApp();
	if (bCreated && !app->m_pMainWnd) {
		// Set as main frame (InitialUpdateFrame will show the window)
		app->m_pMainWnd = pFrame;
	}

	InitialUpdateFrame(pFrame, pDocument, bMakeVisible);

	return pDocument;
}

void CSingleDocTemplate::AddDocument(CDocument *pDoc) {
	ASSERT(m_pOnlyDoc == nullptr);     // one at a time please
	ASSERT_VALID(pDoc);

	CDocTemplate::AddDocument(pDoc);
	m_pOnlyDoc = pDoc;
}

void CSingleDocTemplate::SetDefaultTitle(CDocument *pDocument) {
	pDocument->SetTitle("Untitled");
}

POSITION CSingleDocTemplate::GetFirstDocPosition() const {
	return (m_pOnlyDoc == nullptr) ? nullptr : (POSITION)true;
}

CDocument *CSingleDocTemplate::GetNextDoc(POSITION &rPos) const {
	CDocument *pDoc = nullptr;
	if (rPos) {
		// First time through, return the single document
		assert(m_pOnlyDoc != nullptr);
		pDoc = m_pOnlyDoc;
	}

	rPos = nullptr;        // No more
	return pDoc;
}

} // namespace MFC
} // namespace Graphics
