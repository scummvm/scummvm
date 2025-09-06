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
#include "bagel/hodjnpodj/metagame/gtl/gtlfrm.h"
#include "bagel/hodjnpodj/metagame/gtl/gtldoc.h"
#include "bagel/hodjnpodj/metagame/gtl/gtlview.h"
#include "bagel/metaengine.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

///DEFS gtl.h

extern CGtlFrame *pMainWindow;
extern bool bExitMetaDLL;
extern bool st_bExitDll;

static CSingleDocTemplate *pSdiDocTemplate = nullptr;


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
	bExitMetaDLL = st_bExitDll = false;
}

CGtlApp::~CGtlApp() {
}

bool CGtlApp::InitApplication() {
	bool result = CWinApp::InitApplication();
	setFocusChangeProc(focusChange);
	return result;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CGtlApp object

#if !GTLDLL
	CGtlApp NEAR theApp;
#endif /* GTLDLL */

/////////////////////////////////////////////////////////////////////////////
// CGtlApp initialization

bool CGtlApp::InitInstance() {

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

	m_iNumOpens = 0;

	return true;
}


void CGtlApp::CreateInstance() {
	pSdiDocTemplate = new CSingleDocTemplate(
	    IDR_GTLTYPE,
	    RUNTIME_CLASS(CGtlDoc),
	    RUNTIME_CLASS(CGtlFrame),     // main SDI frame window
	    RUNTIME_CLASS(CGtlView));
	AddDocTemplate(pSdiDocTemplate);

	m_nCmdShow = SW_SHOWNORMAL;

	OnFileNew();
}

//* CGtlApp::CallOnFileNew --
bool CGtlApp::CallOnFileNew()
// returns: true if error, false otherwise
{
	JXENTER(CGtlApp::CallOnFileNew) ;
	int iError = 0 ;            // error code

	OnFileNew() ;       // makes protected function public

//cleanup:

	JXELEAVE(CGtlApp::CallOnFileNew) ;
	RETURN(iError != 0) ;
}

//* CGtlApp::CallOnFileOpen --
bool CGtlApp::CallOnFileOpen()
// returns: true if error, false otherwise
{
	JXENTER(CGtlApp::CallOnFileOpen) ;
	int iError = 0 ;            // error code

	OnFileOpen() ;      // makes protected function public

//cleanup:

	JXELEAVE(CGtlApp::CallOnFileOpen) ;
	RETURN(iError != 0) ;
}

//* CGtlApp::CallOnFileSave --
bool CGtlApp::CallOnFileSave()
// returns: true if error, false otherwise
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
int CGtlApp::DoMessageBox(const char *lpszPrompt, unsigned int nType, unsigned int nIDPrompt)
// returns: true if error, false otherwise
{
	JXENTER(CGtlApp::DoMessageBox) ;
	int iRetval = IDOK ;                // return value

	JXELEAVE(CGtlApp::DoMessageBox) ;
	RETURN(iRetval) ;
}

bool CGtlApp::OnIdle(long lCount) {
	if (bExitMetaDLL) {
		PostMessage(nullptr, WM_QUIT, 0, 0);
		return false;
	}

	return true;
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
	virtual void DoDataExchange(CDataExchange *pDX) override;    // DDX/DDV support
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

void CGtlApp::focusChange(CWnd *oldFocus, CWnd *newFocus) {
	CEdit *oldCtl = dynamic_cast<CEdit *>(oldFocus);
	CEdit *newCtl = dynamic_cast<CEdit *>(newFocus);
	if (oldCtl != newCtl)
		BagelMetaEngine::setKeybindingMode(newFocus ? KBMODE_MINIMAL : KBMODE_NORMAL);
}

/////////////////////////////////////////////////////////////////////////////
// VB-Event registration
// (calls to AfxRegisterVBEvent will be placed here by ClassWizard)

//{{AFX_VBX_REGISTER_MAP()
//      unsigned int NEAR VBN_SPINDOWN = AfxRegisterVBEvent("SPINDOWN");
//      unsigned int NEAR VBN_SPINUP = AfxRegisterVBEvent("SPINUP");
//}}AFX_VBX_REGISTER_MAP

/////////////////////////////////////////////////////////////////////////////
// CGtlApp commands


#if GTLMDI

IMPLEMENT_DYNAMIC(CGtlMDIChildWnd, CMDIChildWnd)

bool CGtlMDIChildWnd::PreCreateWindow(CREATESTRUCT& cs) {
	// By turning off the default MFC-defined FWS_ADDTOTITLE style,
	// the framework will use first string in the document template
	// STRINGTABLE resource instead of the document name.

//      cs.style &= ~(long)FWS_ADDTOTITLE;
//      cs.style = WS_POPUP ;
	return CMDIChildWnd::PreCreateWindow(cs);
}
#endif

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
