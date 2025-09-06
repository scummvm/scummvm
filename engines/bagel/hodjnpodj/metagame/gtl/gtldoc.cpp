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

#include "bagel/hodjnpodj/metagame/bgen/stdafx.h"
#include "bagel/hodjnpodj/metagame/gtl/gtl.h"
#include "bagel/hodjnpodj/metagame/gtl/gtldoc.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

/////////////////////////////////////////////////////////////////////////////
// CGtlDoc

IMPLEMENT_DYNCREATE(CGtlDoc, CDocument)

BEGIN_MESSAGE_MAP(CGtlDoc, CDocument)
	//{{AFX_MSG_MAP(CGtlDoc)
	// NOTE - the ClassWizard will add and remove mapping macros here.
	//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGtlDoc construction/destruction

CGtlDoc::CGtlDoc() {
	TRACECONSTRUCTOR(CGtlDoc) ;

	// clear all data
	memset(&m_cStartData, 0, &m_cEndData - &m_cStartData);
}

CGtlDoc::~CGtlDoc() {
	TRACEDESTRUCTOR(CGtlDoc) ;
	DeleteContents() ;
}

bool CGtlDoc::OnNewDocument() {
	CGtlApp * xpGtlApp = (CGtlApp *)AfxGetApp() ; // get application
	if (!CDocument::OnNewDocument())
		return false;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	if (++xpGtlApp->m_iNumOpens == 1 && xpGtlApp->m_szFilename[0])
		InitDocument(xpGtlApp->m_szFilename) ;
	else
		InitDocument("default.gtl");

	return true;
}

/////////////////////////////////////////////////////////////////////////////


//* CGtlDoc::DeleteContents() --
void CGtlDoc::DeleteContents() {
	if (m_xpGtlData)
		delete m_xpGtlData ;

	// clear all data
	memset(&m_cStartData, 0, &m_cEndData - &m_cStartData) ;
}

//* CGtlDoc::InitDocument -- initialize document to specified file
void CGtlDoc::InitDocument(const char *xpszPathName)
// xpszPathName -- filename to be opened
// returns: void
{
	CGtlApp * xpGtlApp = (CGtlApp *)AfxGetApp() ; // get application
	memset(&m_cStartData, 0, &m_cEndData - &m_cStartData) ;

//  dbgtrc = true ;
	m_xpGtlData = new CGtlData ;
	m_xpGtlData->m_xpcGtlDoc = this ;
	m_xpGtlData->m_bShowNodes = xpGtlApp->m_bShowNodes ;
	m_xpGtlData->m_bPaintBackground  = xpGtlApp->m_bPaintBackground ;

	OnChangedViewList() ;
	FixChecks() ;
	if (xpszPathName && *xpszPathName)
		m_xpGtlData->Compile(xpszPathName) ;

	m_xpGtlData->m_bStartMetaGame = xpGtlApp->m_bStartMetaGame ;
	UpdateAllViews(nullptr, 0L, nullptr) ;
}

//* CGtlDoc::OnOpenDocument --
bool CGtlDoc::OnOpenDocument(const char *xpszPathName) {
	if (!CDocument::OnOpenDocument(xpszPathName))
		return false;

	InitDocument(xpszPathName) ;
	return true;
}

//* CGtlDoc::OnSaveDocument --
bool CGtlDoc::OnSaveDocument(const char *xpszPathName) {
	return true;
}


//* CGtlDoc::DoOnFileSaveAs --
bool CGtlDoc::DoOnFileSaveAs()
// returns: true if error, false otherwise
{
	CDocument::OnFileSaveAs() ;

	return false;
}


/////////////////////////////////////////////////////////////////////////////
// CGtlDoc serialization

void CGtlDoc::Serialize(CArchive& ar) {
	if (ar.IsStoring()) {
		// TODO: add storing code here
	} else {
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CGtlDoc commands



//* CGtlDoc::OnChangedViewList -- called by MFC when a view is
//		added or deleted
void CGtlDoc::OnChangedViewList() {
	JXENTER(CGtlDoc::OnChangedViewList) ;
	POSITION nViewPosition = GetFirstViewPosition() ;
	CGtlView *xpGtlView ;

	// If we haven't yet found frame window
	if (!m_xpGtlFrame) {
		if ((xpGtlView = (CGtlView *)GetNextView(nViewPosition)) != nullptr) {
			if (!m_xpcLastFocusView)
				m_xpcLastFocusView = xpGtlView ;

			if (!m_xpcLastMouseView)
				m_xpcLastMouseView = xpGtlView ;

			CWnd *pWnd = xpGtlView ;
			while (pWnd->GetParent())
				pWnd = pWnd->GetParent();

			m_xpGtlFrame = (CGtlFrame *)pWnd ;
		}
	}

	if (m_xpGtlFrame) {
		m_xpGtlFrame->m_xpDocument = this ;
		m_xpGtlFrame->m_xpcLastMouseView = m_xpcLastMouseView ;
		m_xpGtlFrame->m_xpcLastFocusView = m_xpcLastFocusView ;
	}

// cleanup:

	JXELEAVE(CGtlDoc::OnChangedViewList) ;
	RETURN_VOID ;
}

//* CGtlDoc::FixChecks -- fix dialog box check marks
bool CGtlDoc::FixChecks() {
	return false;
}

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
