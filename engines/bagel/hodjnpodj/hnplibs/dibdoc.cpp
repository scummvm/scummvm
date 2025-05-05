// dibdoc.cpp : implementation of the CDibDoc class
//
// Version 2.0
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and Microsoft
// QuickHelp and/or WinHelp documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#include <limits.h>

#include "dibdoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDibDoc

IMPLEMENT_DYNCREATE(CDibDoc, CDocument)

BEGIN_MESSAGE_MAP(CDibDoc, CDocument)
	//{{AFX_MSG_MAP(CDibDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDibDoc construction/destruction

CDibDoc::CDibDoc()
{
	m_hDIB = NULL;
	m_palDIB = NULL;
	m_sizeDoc = CSize(1,1);     // dummy value to make CScrollView happy
}

CDibDoc::~CDibDoc()
{
	if (m_hDIB != NULL)
	{
		::GlobalFree((HGLOBAL) m_hDIB);
	}
	if (m_palDIB != NULL)
	{
		(*m_palDIB).DeleteObject();
		delete m_palDIB;
	}
}

void CDibDoc::InitDIBData()
{
	if (m_palDIB != NULL)
	{
		delete m_palDIB;
		m_palDIB = NULL;
	}
	if (m_hDIB == NULL)
	{
		return;
	}
	// Set up document size
	LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) m_hDIB);
	
	if (::DIBWidth(lpDIB) > INT_MAX ||::DIBHeight(lpDIB) > INT_MAX)
	{
		::GlobalUnlock((HGLOBAL) m_hDIB);
		::GlobalFree((HGLOBAL) m_hDIB);
		m_hDIB = NULL;
		return;
	}

	m_sizeDoc = CSize((int) ::DIBWidth(lpDIB), (int) ::DIBHeight(lpDIB));

	::GlobalUnlock((HGLOBAL) m_hDIB);
	// Create copy of palette
	m_palDIB = new CPalette;
	if (m_palDIB == NULL)
	{
		// we must be really low on memory
		::GlobalFree((HGLOBAL) m_hDIB);
		m_hDIB = NULL;
		ShowMemoryInfo("Unable to create artwork palette", "Internal Problem");
		return;
	}

	if (::CreateDIBPalette(m_hDIB, m_palDIB) == NULL)
	{
		// DIB may not have a palette
		delete m_palDIB;
		m_palDIB = NULL;
		ShowMemoryInfo("Unable to create artwork palette", "Internal Problem");
		return;
	}
}


BOOL CDibDoc::OpenResourceDocument(const int nResID)
{
char	chResID[8];

	DeleteContents();

	sprintf(chResID,"#%d",nResID);
	m_hDIB = ReadDIBResource(chResID);
	if (m_hDIB != NULL)
		InitDIBData();

	if (m_hDIB == NULL)
	{
    	char	buf[128];
        	
    	sprintf(buf,"Unable to load artwork resource: %s",chResID);
		ShowMemoryInfo(buf, "Internal Problem");
		return FALSE;
	}
	SetPathName(chResID);
	SetModifiedFlag(FALSE);     // start off with unmodified
	return TRUE;
}


BOOL CDibDoc::OpenResourceDocument(const char* pszPathName)
{
	DeleteContents();

	m_hDIB = ReadDIBResource(pszPathName);
	if (m_hDIB != NULL)
		InitDIBData();

	if (m_hDIB == NULL)
	{
    	char	buf[128];
        	
    	sprintf(buf,"Unable to load artwork file: %s",pszPathName);
		ShowMemoryInfo(buf, "Internal Problem");
		return FALSE;
	}
	SetPathName(" ");
	SetModifiedFlag(FALSE);     // start off with unmodified
	return TRUE;
}



CPalette * CDibDoc::DetachPalette()
{
CPalette	*pMyPalette;

	pMyPalette = m_palDIB;
	m_palDIB = NULL;
	return(pMyPalette);
}


BOOL CDibDoc::OpenDocument(const char* pszPathName)
{
	CFile file;
	CFileException fe;
	if (!file.Open(pszPathName, CFile::modeRead | CFile::shareDenyWrite, &fe))
	{
    	char	buf[128];
        	
    	sprintf(buf,"Unable to open artwork file: %s",pszPathName);
		ShowMemoryInfo(buf, "Internal Problem");

		ReportSaveLoadException(pszPathName, &fe,
			FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
		return FALSE;
	}

	DeleteContents();
//	BeginWaitCursor();

	// replace calls to Serialize with ReadDIBFile function
	TRY
	{
		m_hDIB = ::ReadDIBFile(file);
		
		if (m_hDIB == NULL) {
	    	char	buf[128];
	        	
	    	sprintf(buf,"Unable to load artwork file: %s",pszPathName);
			ShowMemoryInfo(buf, "Internal Problem");
		}
	}
	CATCH (CFileException, eLoad)
	{
		file.Abort(); // will not throw an exception
//		EndWaitCursor();
		ReportSaveLoadException(pszPathName, eLoad,
			FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
		m_hDIB = NULL;
		return FALSE;
	}
	END_CATCH

	InitDIBData();
//	EndWaitCursor();

	if (m_hDIB == NULL)
	{
    	char	buf[128];
        	
    	sprintf(buf,"Unable to load artwork file: %s",pszPathName);
		ShowMemoryInfo(buf, "Internal Problem");
		return FALSE;
	}

	SetPathName(pszPathName);
	SetModifiedFlag(FALSE);     // start off with unmodified
	return TRUE;
}


BOOL CDibDoc::SaveDocument(const char* pszPathName)
{
	CFile file;
	CFileException fe;

	if (!file.Open(pszPathName, CFile::modeCreate |
	  CFile::modeReadWrite | CFile::shareExclusive, &fe))
	{
		ReportSaveLoadException(pszPathName, &fe,
			TRUE, AFX_IDP_INVALID_FILENAME);
		return FALSE;
	}

	// replace calls to Serialize with SaveDIB function
	BOOL bSuccess = FALSE;
	TRY
	{
//		BeginWaitCursor();
		bSuccess = ::SaveDIB(m_hDIB, file);
		file.Close();
	}
	CATCH (CException, eSave)
	{
		file.Abort(); // will not throw an exception
//		EndWaitCursor();
		ReportSaveLoadException(pszPathName, eSave,
			TRUE, AFX_IDP_FAILED_TO_SAVE_DOC);
		return FALSE;
	}
	END_CATCH

//	EndWaitCursor();
	SetModifiedFlag(FALSE);     // back to unmodified

	return bSuccess;
}

void CDibDoc::ReplaceHDIB(HDIB hDIB)
{
	if (m_hDIB != NULL)
	{
		::GlobalFree((HGLOBAL) m_hDIB);
	}
	m_hDIB = hDIB;
}

/////////////////////////////////////////////////////////////////////////////
// CDibDoc diagnostics

#ifdef _DEBUG
void CDibDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CDibDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDibDoc commands

