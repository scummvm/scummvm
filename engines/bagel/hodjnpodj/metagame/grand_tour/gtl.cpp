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
#include "bagel/hodjnpodj/metagame/grand_tour/gtl.h"
#include "bagel/hodjnpodj/metagame/grand_tour/gtlfrm.h"
#include "bagel/hodjnpodj/metagame/grand_tour/gtldoc.h"
#include "bagel/hodjnpodj/metagame/grand_tour/gtlview.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace GrandTour {

///DEFS gtl.h

extern CGtlFrame        *pMainWindow;

static CSingleDocTemplate *pSdiDocTemplate = NULL;


/////////////////////////////////////////////////////////////////////////////
// CGtlApp

BEGIN_MESSAGE_MAP(CGtlApp, CWinApp)
	//{{AFX_MSG_MAP(CGtlApp)
	ON_COMMAND(ID_APP_ABOUT, CGtlApp::OnAppAbout)
	// NOTE - the ClassWizard will add and remove mapping macros here.
	//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGtlApp construction

CGtlApp::CGtlApp() {
	// Place all significant initialization in InitInstance
	memset(&m_cStartData, 0, &m_cEndData - &m_cStartData) ;
}

CGtlApp::~CGtlApp() {
	#ifdef BAGEL_DEBUG
	m_cBdbgMgr.ReportTraceObjects() ;
	#endif
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CGtlApp object

#if !GTLDLL
	CGtlApp NEAR theApp;
#endif /* GTLDLL */

/////////////////////////////////////////////////////////////////////////////
// CGtlApp initialization

BOOL CGtlApp::InitInstance() {

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	SetDialogBkColor();        // Set dialog background color to gray

	// Initialize
	//
	m_iWidth = GetSystemMetrics(SM_CXSCREEN);
	m_iHeight = GetSystemMetrics(SM_CYSCREEN);

	m_iX = 0;
	m_iY = 0;

	// If we ever change Hodj 'n' Podj to live in entire screen
	// then delete these next 4 lines of code
	//
	m_iWidth = GAME_WIDTH;
	m_iHeight = GAME_HEIGHT;
	m_iX = (GetSystemMetrics(SM_CXSCREEN) - m_iWidth) / 2 ;
	m_iY = (GetSystemMetrics(SM_CYSCREEN) - m_iHeight) / 2 ;

	Common::strcpy_s(m_szFilename, "META.GTL");

	#ifdef BAGEL_DEBUG

	XPSTR xpszFile = "jxdebug.ini", xpszSect = "meta";

	m_cBdbgMgr.DebugInit(xpszFile, xpszSect) ;

	m_bTitle = m_cBdbgMgr.GetDebugInt("title") ;
	m_iHeight = m_cBdbgMgr.GetDebugInt("height", GAME_HEIGHT) ;
	m_iWidth = m_cBdbgMgr.GetDebugInt("width", GAME_WIDTH) ;

	m_iX = (GetSystemMetrics(SM_CXSCREEN) - m_iWidth) / 2 ;
	m_iY = (GetSystemMetrics(SM_CYSCREEN) - m_iHeight) / 2 ;

	m_iX = m_cBdbgMgr.GetDebugInt("x", m_iX) ;
	m_iY = m_cBdbgMgr.GetDebugInt("y", m_iY) ;

	m_bShowNodes = m_cBdbgMgr.GetDebugInt("shownodes") ;
	m_bDumpGamePlay = m_cBdbgMgr.GetDebugInt("dumpgameplay") ;
	m_bPaintBackground = m_cBdbgMgr.GetDebugInt("paintbackground") ;
	m_bStartMetaGame = m_cBdbgMgr.GetDebugInt("startmetagame", 1) ; // dft on

	m_cBdbgMgr.GetDebugString("filename", m_szFilename, sizeof(m_szFilename), "meta.gtl") ;
	#endif

	#ifdef NODEEDIT
	m_bControlDialog = m_cBdbgMgr.GetDebugInt("controldialog") ;
	m_bMenuDialog = m_cBdbgMgr.GetDebugInt("menudialog") ;
	m_bNodeDialog = m_cBdbgMgr.GetDebugInt("nodedialog") ;
	m_bInfoDialog = m_cBdbgMgr.GetDebugInt("infodialog") ;
	//m_bStartMetaGame = FALSE;
	#endif

	m_iNumOpens = 0;

	return TRUE;
}


void CGtlApp::CreateInstance(void) {
	pSdiDocTemplate = new CSingleDocTemplate(
	    IDR_GTLTYPE,
	    RUNTIME_CLASS(CGtlDoc),
	    RUNTIME_CLASS(CGtlFrame),     // main SDI frame window
	    RUNTIME_CLASS(CGtlView));
	AddDocTemplate(pSdiDocTemplate);

	m_nCmdShow = SW_SHOWNORMAL;

	OnFileNew();
}

#if GTLDLL

int CGtlApp::ExitInstance() {
	/*
	    if (pSdiDocTemplate != NULL) {
	        pSdiDocTemplate->CloseAllDocuments(TRUE);
	        pSdiDocTemplate = NULL;
	    }
	    if ( pMainWindow != NULL )
	        pMainWindow->DestroyWindow();
	*/
	// Don't call CWinApp::ExitInstance(); Because we would get the recent
	// file list in HNPMETA.INI
	return (0);
}
#endif /* GTLDLL */

//* CGtlApp::CallOnFileNew --
BOOL CGtlApp::CallOnFileNew(void)
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CGtlApp::CallOnFileNew) ;
	int iError = 0 ;            // error code

	OnFileNew() ;       // makes protected function public

//cleanup:

	JXELEAVE(CGtlApp::CallOnFileNew) ;
	RETURN(iError != 0) ;
}

//* CGtlApp::CallOnFileOpen --
BOOL CGtlApp::CallOnFileOpen(void)
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CGtlApp::CallOnFileOpen) ;
	int iError = 0 ;            // error code

	OnFileOpen() ;      // makes protected function public

//cleanup:

	JXELEAVE(CGtlApp::CallOnFileOpen) ;
	RETURN(iError != 0) ;
}

//* CGtlApp::CallOnFileSave --
BOOL CGtlApp::CallOnFileSave(void)
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CGtlApp::CallOnFileSave) ;
	int iError = 0 ;            // error code

//    OnFileSave() ;    // makes protected function public
	SaveAllModified() ;

//cleanup:

	JXELEAVE(CGtlApp::CallOnFileSave) ;
	RETURN(iError != 0) ;
}

//* CGtlApp::DoMessageBox -- override of CWinApp function to
//              display message box messages
int CGtlApp::DoMessageBox(LPCSTR lpszPrompt, UINT nType, UINT nIDPrompt)
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CGtlApp::DoMessageBox) ;
	int iRetval = IDOK ;                // return value

	#ifdef BAGEL_DEBUG
	if (lpszPrompt && lpszPrompt[0] == '~')
		m_cBdbgMgr.DebugMessageBox(lpszPrompt, nType, nIDPrompt) ;

	else
		iRetval = CWinApp::DoMessageBox(lpszPrompt, nType, nIDPrompt) ;

// cleanup:
	#endif

	JXELEAVE(CGtlApp::DoMessageBox) ;
	RETURN(iRetval) ;
}



/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog {
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//{{AFX_MSG(CAboutDlg)
	// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD) {
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CGtlApp::OnAppAbout() {
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// VB-Event registration
// (calls to AfxRegisterVBEvent will be placed here by ClassWizard)

//{{AFX_VBX_REGISTER_MAP()
//      UINT NEAR VBN_SPINDOWN = AfxRegisterVBEvent("SPINDOWN");
//      UINT NEAR VBN_SPINUP = AfxRegisterVBEvent("SPINUP");
//}}AFX_VBX_REGISTER_MAP

/////////////////////////////////////////////////////////////////////////////
// CGtlApp commands


#if GTLMDI

IMPLEMENT_DYNAMIC(CGtlMDIChildWnd, CMDIChildWnd)

BOOL CGtlMDIChildWnd::PreCreateWindow(CREATESTRUCT& cs) {
	// By turning off the default MFC-defined FWS_ADDTOTITLE style,
	// the framework will use first string in the document template
	// STRINGTABLE resource instead of the document name.

//      cs.style &= ~(LONG)FWS_ADDTOTITLE;
//      cs.style = WS_POPUP ;
	return CMDIChildWnd::PreCreateWindow(cs);
}
#endif

} // namespace GrandTour
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
