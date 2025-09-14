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

#include "bagel/hodjnpodj/metagame/gtl/gtl.h"
#include "bagel/hodjnpodj/metagame/bgen/stdafx.h"
#include "bagel/hodjnpodj/metagame/gtl/init.h"
#include "bagel/hodjnpodj/metagame/gtl/gtldoc.h"
#include "bagel/hodjnpodj/metagame/gtl/gtlview.h"
#include "bagel/hodjnpodj/metagame/gtl/gtlfrm.h"
#include "bagel/hodjnpodj/metagame/bgen/backpack.h"
#include "bagel/hodjnpodj/metagame/gtl/optdlg.h"
#include "bagel/hodjnpodj/metagame/gtl/pawn.h"
#include "bagel/hodjnpodj/metagame/gtl/store.h"
#include "bagel/hodjnpodj/metagame/bgen/notebook.h"
#include "bagel/hodjnpodj/metagame/bgen/note.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

extern bool bExitMetaDLL;
extern bool st_bExitDll;

extern CBgbMgr *gpBgbMgr;
extern HWND ghwndParent;
extern CWinApp *priorApp;
extern CBfcMgr *lpMetaGameStruct;
CGtlFrame *pMainWindow = nullptr;

/////////////////////////////////////////////////////////////////////////////
// CGtlFrame

#if GTLMDI
	IMPLEMENT_DYNAMIC(CGtlFrame, MFC_FRAME)
#else
	IMPLEMENT_DYNCREATE(CGtlFrame, MFC_FRAME)
#endif

BEGIN_MESSAGE_MAP(CGtlFrame, MFC_FRAME)
	//{{AFX_MSG_MAP(CGtlFrame)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()

	ON_COMMAND(ID_CALL_NEW, CGtlFrame::OnCallNew)
	ON_COMMAND(ID_CALL_CLOSE, CGtlFrame::OnCallClose)
	ON_COMMAND(ID_CALL_EXIT, CGtlFrame::OnCallExit)
	ON_COMMAND(ID_CALL_OPEN, CGtlFrame::OnCallOpen)
	ON_WM_ACTIVATEAPP()
	ON_WM_SYSCOMMAND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGtlFrame construction/destruction

CGtlFrame::CGtlFrame() {
	TRACECONSTRUCTOR(CGtlFrame) ;

	m_xpcLastFocusView = m_xpcLastMouseView = nullptr ;
	// no last view

	#if GTLDLL
	if ((m_lpBfcMgr = lpMetaGameStruct) == nullptr)
		m_lpBfcMgr = new CBfcMgr ;
	#else
	m_lpBfcMgr = new CBfcMgr ;
	#endif

	m_nReturnCode = -1;

}

CGtlFrame::~CGtlFrame() {
	TRACEDESTRUCTOR(CGtlFrame) ;
}

//* CGtlFrame::NewFrame -- set pointer to interface manager in frame
bool CGtlFrame::NewFrame(CBfcMgr *lpBfcMgr)
// returns: true if error, false otherwise
{
	JXENTER(CGtlFrame::NewFrame) ;
	int iError = 0 ;        // error code

	if ((m_lpBfcMgr = lpBfcMgr) == nullptr)
		m_lpBfcMgr = new CBfcMgr ;
	m_nReturnCode = -1;

	OnCallNew();

//cleanup:

	JXELEAVE(CGtlFrame::NewFrame) ;
	RETURN(iError != 0) ;
}

void CGtlFrame::OnDestroy() {
	int iReturnValue = -1;

	if (m_lpBfcMgr && m_lpBfcMgr->m_iFunctionCode > 0
	        && m_lpBfcMgr->m_iFunctionCode != MG_DLLX_QUIT)
		iReturnValue = m_lpBfcMgr->m_iFunctionCode ;

	lpMetaGameStruct->m_bRestart = true;

	priorApp->PostMessage(ghwndParent, WM_PARENTNOTIFY, WM_DESTROY, iReturnValue);
	pMainWindow = nullptr;

	CFrameWnd::OnDestroy();
}

int CGtlFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (MFC_FRAME::OnCreate(lpCreateStruct) == -1)
		return -1;

	pMainWindow = this;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CGtlFrame message handlers

bool CGtlFrame::ShowClue(CPalette *pPalette, CNote *pNote) {
	CNotebook dlgNoteBook((CWnd *)this, pPalette, nullptr, pNote);
	dlgNoteBook.DoModal();
	SetupCursor();
	return false;
}

//* CGtlFrame::GetCurrentDocAndView -- get last focused doc/view
bool CGtlFrame::GetCurrentDocAndView(CGtlDoc * &xpcGtlDoc,
                                     CGtlView *&xpcGtlFocusView, CGtlView *&xpcGtlMouseView)
// xpcGtlDoc (output) -- the document which most recently had focus
//      in some view, or nullptr if none
// xpcGtlFocusView (output) -- view which had most recent focus, or nullptr
// xpcGtlMouseView (output) -- the view for this same document that was
//      most recently touched by the mouse, or nullptr
// returns: true if error, false otherwise
{
	JXENTER(CGtlFrame::GetCurrentDocAndView) ;
	int iError = 0 ;        // error code

	xpcGtlDoc = m_xpDocument ;

	if ((xpcGtlFocusView = m_xpcLastFocusView) != nullptr) {

		if (!xpcGtlDoc)
			xpcGtlDoc = m_xpcLastFocusView->GetDocument() ;

		if (xpcGtlDoc)
			xpcGtlMouseView = xpcGtlDoc->m_xpcLastMouseView ;

		// no pointer to last focus view
		//
	} else {
		xpcGtlMouseView = nullptr;
	}

// cleanup:

	JXELEAVE(CGtlFrame::GetCurrentDocAndView) ;
	RETURN(iError != 0) ;
}


//* CGtlFrame::PreCreateWindow -- change frame window style
bool CGtlFrame::PreCreateWindow(CREATESTRUCT& cCs)
// returns: value returned by base class function
{
	JXENTER(CGtlFrame::PreCreateWindow) ;
	bool bRetval ;      // return value
	static CString stWndClass("") ;
	CGtlApp * xpGtlApp = (CGtlApp *)AfxGetApp() ; // get application
//  CClientDC cDc(this) ;

//  if (stWndClass == "")
//  stWndClass = AfxRegisterWndClass(CS_DBLCLKS |
//              CS_BYTEALIGNWINDOW | CS_OWNDC,
//              nullptr, nullptr, nullptr) ;

	if (!xpGtlApp->m_bTitle) {   // if we don't want a title bar
//      cCs.lpszClass = stWndClass ;

		// determine where to place the game window
		// ... so it is centered on the screen
//      cCs.x = (cDc.GetDeviceCaps(HORZRES) - xpGtlApp->m_iWidth) >> 1 ;
//      cCs.y = (cDc.GetDeviceCaps(VERTRES) - xpGtlApp->m_iHeight) >> 1 ;
//      cCs.x = (GetSystemMetrics(SM_CXSCREEN) - xpGtlApp->m_iWidth) >> 1 ;
//      cCs.y = (GetSystemMetrics(SM_CYSCREEN)- xpGtlApp->m_iHeight) >> 1 ;
		cCs.x = xpGtlApp->m_iX ;
		cCs.y = xpGtlApp->m_iY ;
		cCs.cx = xpGtlApp->m_iWidth ;
		cCs.cy = xpGtlApp->m_iHeight ;

		if (!(cCs.style & WS_CHILD)) {
			cCs.style = WS_POPUP | WS_VISIBLE ;
			if (cCs.hMenu != nullptr)
				MFC::DestroyMenu(cCs.hMenu);
			cCs.hMenu = 0 ;
		}
	}

	bRetval = MFC_FRAME::PreCreateWindow(cCs) ;

// cleanup:

	JXELEAVE(CGtlFrame::PreCreateWindow) ;
	RETURN(bRetval) ;
}


//* CGtlFrame::RecalcLayout -- override CFrameWnd::RecalcLayout
void CGtlFrame::RecalcLayout(bool bNotify)
// returns: void
{
	JXENTER(CGtlFrame::RecalcLayout) ;
	CRect cParamRect, cClientRect(0, 0, 200, 200) ;
	CGtlApp * xpGtlApp = (CGtlApp *)AfxGetApp() ; // get application

//      // call the layout hook -- OLE 2.0 support uses this hook
//      if (bNotify && m_pNotifyHook != nullptr)
//              m_pNotifyHook->OnRecalcLayout();

//      // reposition all the child windows (regardless of ID)
//      RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST,
//      reposExtra, &m_rectBorder)

	if (xpGtlApp->m_bTitle) // if there's a title bar
		MFC_FRAME::RecalcLayout(bNotify) ;
	else
		RepositionBars(0, 0, AFX_IDW_PANE_FIRST,
		               reposQuery, &cParamRect, &cClientRect) ;
	// reposition all the child windows (regardless of ID)

//cleanup:

	JXELEAVE(CGtlFrame::RecalcLayout) ;
	RETURN_VOID ;
}

void CGtlFrame::ProcessMiniGameReturn() {
}

void CGtlFrame::ShowOptions(CPalette *pPalette) {
	CMetaOptDlg cOptionsDlg((CWnd*)this, pPalette);
	int         nOptionsReturn = 0;

	cOptionsDlg.SetInitialOptions(lpMetaGameStruct);    // Sets the private members
	nOptionsReturn = cOptionsDlg.DoModal();
	if (nOptionsReturn == 1) {
		bExitMetaDLL = true;
		st_bExitDll = true;
	}

	return;
}

void CGtlFrame::ShowInventory(CPalette *pPalette, int nWhichDlg) {
	CHodjPodj *pPlayer;

	// which player
	pPlayer = &lpMetaGameStruct->m_cPodj;
	if (lpMetaGameStruct->m_cHodj.m_bMoving)
		pPlayer = &lpMetaGameStruct->m_cHodj;

	switch (nWhichDlg) {

	case 4: {   // black market
		CGeneralStore cBMarketDlg((CWnd *)this, pPalette, pPlayer->m_pBlackMarket, pPlayer->m_pInventory);
		cBMarketDlg.DoModal();
		break;
	}

	case 3: {   // pawn shop
		CPawnShop cPawnnShopDlg((CWnd *)this, pPalette, ((brand() & 1)  == 1 ? pPlayer->m_pGenStore : pPlayer->m_pBlackMarket), pPlayer->m_pInventory);
		cPawnnShopDlg.DoModal();
		break;
	}

	case 2: {   // general store
		CGeneralStore cGenStoreDlg((CWnd *)this, pPalette, pPlayer->m_pGenStore, pPlayer->m_pInventory);
		cGenStoreDlg.DoModal();
		break;
	}

	case 1: {
		default:
			CBackpack dlgBackPack((CWnd *)this, pPalette, pPlayer->m_pInventory);
			dlgBackPack.DoModal();
			break;
		}
	}

	SetupCursor();
}

void CGtlFrame::OnCallNew() {
	// TODO: Add your command handler code here
	JXENTER(CGtlFrame::OnCallNew) ;
	CGtlApp * xpGtlApp = (CGtlApp *)AfxGetApp() ; // get application

	xpGtlApp->CallOnFileNew() ;

//cleanup:

	JXELEAVE(CGtlFrame::OnCallNew) ;
	RETURN_VOID ;

}

void CGtlFrame::OnCallOpen() {
	// TODO: Add your command handler code here
	JXENTER(CGtlFrame::OnCallOpen) ;
	CGtlApp * xpGtlApp = (CGtlApp *)AfxGetApp() ; // get application

	xpGtlApp->CallOnFileOpen() ;

//cleanup:

	JXELEAVE(CGtlFrame::OnCallOpen) ;
	RETURN_VOID ;

}

bool CGtlFrame::OnEraseBkgnd(CDC *) {
	return true;
}

void CGtlFrame::OnCallClose() {
}

void CGtlFrame::OnCallExit() {
	int         iReturnValue = -1 ;
	CGtlDoc     *xpGtlDoc = nullptr ;
	CGtlView    *xpGtlFocusView, *xpGtlMouseView ;

	GetCurrentDocAndView(xpGtlDoc, xpGtlFocusView, xpGtlMouseView) ;

	xpGtlDoc->m_xpGtlData->m_xpGtlView->KillTimer(ANIMATION_TIMER_ID);

	#if RETAIN_META_DLL
	ShowWindow(SW_HIDE);

	if (gpBgbMgr != nullptr)
		gpBgbMgr->CacheOptimize(2000000);
	#else
	if (gpBgbMgr != nullptr)
		gpBgbMgr->CacheFlush();
	#endif

	if (m_lpBfcMgr && m_lpBfcMgr->m_iFunctionCode > 0
	        && m_lpBfcMgr->m_iFunctionCode != MG_DLLX_QUIT)
		iReturnValue = m_lpBfcMgr->m_iFunctionCode ;

	lpMetaGameStruct->m_iFunctionCode = iReturnValue;

	#if RETAIN_META_DLL
	::PostMessage(ghwndParent, WM_PARENTNOTIFY, WM_DESTROY, (LPARAM) iReturnValue);
	#else
//  CWnd::DestroyWindow();
	AfxGetApp()->CloseAllDocuments(false);
	#endif
}


void CGtlFrame::OnSysCommand(unsigned int nID, LPARAM lParam) {
	if ((nID & 0xfff0) != SC_SCREENSAVE)
		CWnd::OnSysCommand(nID, lParam);
}


void CGtlFrame::OnActivateApp(bool bActive, HTASK /*hTask*/) {
	if (!bActive)
		gpBgbMgr->CacheFlush();
}

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
