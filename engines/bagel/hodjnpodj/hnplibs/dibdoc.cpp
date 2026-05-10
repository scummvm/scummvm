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

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/hnplibs/dibdoc.h"

namespace Bagel {
namespace HodjNPodj {

/////////////////////////////////////////////////////////////////////////////
// CDibDoc

IMPLEMENT_DYNCREATE(CDibDoc, CDocument)

BEGIN_MESSAGE_MAP(CDibDoc, CDocument)
	//{{AFX_MSG_MAP(CDibDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDibDoc construction/destruction

CDibDoc::CDibDoc() {
	m_hDIB = nullptr;
	m_palDIB = nullptr;
	m_sizeDoc = CSize(1, 1);     // dummy value to make CScrollView happy
}

CDibDoc::~CDibDoc() {
	delete m_hDIB;

	if (m_palDIB != nullptr) {
		(*m_palDIB).DeleteObject();
		delete m_palDIB;
	}
}

void CDibDoc::InitDIBData() {
	if (m_palDIB != nullptr) {
		delete m_palDIB;
		m_palDIB = nullptr;
	}
	if (m_hDIB == nullptr) {
		return;
	}
	// Set up document size
	if (DIBWidth(m_hDIB) > INT_MAX || DIBHeight(m_hDIB) > INT_MAX) {
		delete m_hDIB;
		m_hDIB = nullptr;
		return;
	}

	m_sizeDoc = CSize((int)DIBWidth(m_hDIB), (int)DIBHeight(m_hDIB));

	// Create copy of palette
	m_palDIB = new CPalette;
	if (m_palDIB == nullptr) {
		// we must be really low on memory
		GlobalFree((HGLOBAL)m_hDIB);
		m_hDIB = nullptr;
		ShowMemoryInfo("Unable to create artwork palette", "Internal Problem");
		return;
	}

	if (!CreateDIBPalette(m_hDIB, m_palDIB)) {
		// DIB may not have a palette
		delete m_palDIB;
		m_palDIB = nullptr;
		ShowMemoryInfo("Unable to create artwork palette", "Internal Problem");
	}
}


bool CDibDoc::OpenResourceDocument(const int nResID) {
	char    chResID[8];

	DeleteContents();

	Common::sprintf_s(chResID, "#%d", nResID);
	m_hDIB = ReadDIBResource(chResID);
	if (m_hDIB != nullptr)
		InitDIBData();

	if (m_hDIB == nullptr) {
		char    buf[128];

		Common::sprintf_s(buf, "Unable to load artwork resource: %s", chResID);
		ShowMemoryInfo(buf, "Internal Problem");
		return false;
	}
	SetPathName(chResID);
	SetModifiedFlag(false);     // start off with unmodified
	return true;
}


bool CDibDoc::OpenResourceDocument(const char *pszPathName) {
	DeleteContents();

	m_hDIB = ReadDIBResource(pszPathName);
	if (m_hDIB != nullptr)
		InitDIBData();

	if (m_hDIB == nullptr) {
		char    buf[128];

		Common::sprintf_s(buf, "Unable to load artwork file: %s", pszPathName);
		ShowMemoryInfo(buf, "Internal Problem");
		return false;
	}
	SetPathName(" ");
	SetModifiedFlag(false);     // start off with unmodified
	return true;
}



CPalette *CDibDoc::DetachPalette() {
	CPalette *pMyPalette;

	pMyPalette = m_palDIB;
	m_palDIB = nullptr;
	return pMyPalette;
}


bool CDibDoc::OpenDocument(const char *pszPathName) {
	CFile file;
	CFileException fe;

	if (!file.Open(pszPathName, CFile::modeRead | CFile::shareDenyWrite, &fe)) {
		error("Unable to open artwork file: %s", pszPathName);
		return false;
	}

	DeleteContents();
	//  BeginWaitCursor();

	// replace calls to Serialize with ReadDIBFile function
	TRY {
		m_hDIB = ReadDIBFile(file);

		if (m_hDIB == nullptr)
			error("Unable to load artwork file: %s", pszPathName);
	}
	CATCH(CFileException, eLoad) {
		file.Abort(); // will not throw an exception
		//      EndWaitCursor();
		ReportSaveLoadException(pszPathName, eLoad,
		                        false, AFX_IDP_FAILED_TO_OPEN_DOC);
		m_hDIB = nullptr;
		return false;
	}
	END_CATCH

	InitDIBData();
	//  EndWaitCursor();

	if (m_hDIB == nullptr) {
		char    buf[128];

		Common::sprintf_s(buf, "Unable to load artwork file: %s", pszPathName);
		ShowMemoryInfo(buf, "Internal Problem");
		return false;
	}

	SetPathName(pszPathName);
	SetModifiedFlag(false);     // start off with unmodified
	return true;
}


bool CDibDoc::SaveDocument(const char *pszPathName) {
	// No implementation
	return true;
}

void CDibDoc::ReplaceHDIB(HDIB hDIB) {
	if (m_hDIB != nullptr) {
		GlobalFree((HGLOBAL)m_hDIB);
	}
	m_hDIB = hDIB;
}

} // namespace HodjNPodj
} // namespace Bagel
