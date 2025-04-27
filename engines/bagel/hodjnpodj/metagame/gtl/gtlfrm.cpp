// gtlfrm.cpp : implementation of the CGtlFrame class
//

#include "stdafx.h"
#include <assert.h>
#include "gtl.h"
#include "dllinit.h"
#include "gtldoc.h"
#include "gtlview.h"
#include "gtlfrm.h"

#include "backpack.h"
#include "optdlg.h"
#include "pawn.h"
#include "store.h"
#include "notebook.h"
#include "note.h"

///DEFS gtlfrm.h

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern BOOL     bExitMetaDLL;
extern BOOL     st_bExitDll;

extern CBgbMgr  *gpBgbMgr;

#if GTLDLL
    extern HWND     ghwndParent;
    CGtlFrame       *pMainWindow = NULL;
    extern CBfcMgr  *lpMetaGameStruct;

#else
#define ITEMS_IN_PAWN_SHOP 12
    CGtlFrame       *pMainWindow = NULL;
    CBfcMgr             *lpMetaGameStruct = NULL;
#endif



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
#if GTLDLL
    ON_WM_DESTROY()
#endif

#ifdef NODEEDIT
    ON_UPDATE_COMMAND_UI(ID_VIEW_STATUS_BAR, OnUpdateViewStatusBar)
    ON_COMMAND(ID_VIEW_INFO_DLG, OnViewInfoDlg)
    ON_COMMAND(ID_VIEW_CTL_DLG, OnViewCtlDlg)
    ON_COMMAND(ID_VIEW_NODE_DLG, OnViewNodeDlg)
    ON_COMMAND(ID_VIEW_MENU_DLG, OnViewMenuDlg)
    ON_COMMAND(ID_CALL_SAVE, OnCallSave)
#endif
    ON_COMMAND(ID_CALL_NEW, OnCallNew)
    ON_COMMAND(ID_CALL_CLOSE, OnCallClose)
    ON_COMMAND(ID_CALL_EXIT, OnCallExit)
    ON_COMMAND(ID_CALL_OPEN, OnCallOpen)
	ON_WM_ACTIVATEAPP()
    ON_WM_SYSCOMMAND()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGtlFrame construction/destruction

CGtlFrame::CGtlFrame()
{
    TRACECONSTRUCTOR(CGtlFrame) ;

    m_xpcLastFocusView = m_xpcLastMouseView = NULL ;
                // no last view

#if GTLDLL
    if ((m_lpBfcMgr = lpMetaGameStruct) == NULL)
        m_lpBfcMgr = new CBfcMgr ;
#else
    m_lpBfcMgr = new CBfcMgr ;
#endif

    m_nReturnCode = -1;
        
}

CGtlFrame::~CGtlFrame()
{
    TRACEDESTRUCTOR(CGtlFrame) ;

#if !GTLDLL
    if (m_lpBfcMgr) {
        delete m_lpBfcMgr ;
        m_lpBfcMgr = NULL ;
    }
#endif
}

//* CGtlFrame::NewFrame -- set pointer to interface manager in frame
BOOL CGtlFrame::NewFrame(CBfcMgr *lpBfcMgr)
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlFrame::NewFrame) ;
    int iError = 0 ;        // error code

    if ((m_lpBfcMgr = lpBfcMgr) == NULL)
        m_lpBfcMgr = new CBfcMgr ;
    m_nReturnCode = -1;

    OnCallNew();

//cleanup:

    JXELEAVE(CGtlFrame::NewFrame) ;
    RETURN(iError != 0) ;
}

#if GTLDLL
void CGtlFrame::OnDestroy()
{
    int iReturnValue = -1 ;
    LPARAM lJunk = (LPARAM)m_nReturnCode; 
    HWND    hWnd = m_hWnd;
    

    if (m_lpBfcMgr && m_lpBfcMgr->m_iFunctionCode > 0
            && m_lpBfcMgr->m_iFunctionCode != MG_DLLX_QUIT)
        iReturnValue = m_lpBfcMgr->m_iFunctionCode ;

    lpMetaGameStruct->m_bRestart = TRUE;

    pMainWindow = NULL;
    
    CFrameWnd::OnDestroy();
}
#endif

int CGtlFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (MFC_FRAME::OnCreate(lpCreateStruct) == -1)
        return -1;

    pMainWindow = this;
    
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CGtlFrame diagnostics

#ifdef _DEBUG
void CGtlFrame::AssertValid() const
{
    MFC_FRAME::AssertValid();
}

void CGtlFrame::Dump(CDumpContext& dc) const
{
    MFC_FRAME::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGtlFrame message handlers

#ifdef NODEEDIT

void CGtlFrame::OnUpdateViewStatusBar(CCmdUI *)
{
    // TODO: Add your command update UI handler code here
}

void CGtlFrame::OnViewInfoDlg()
{
    CGtlDoc * xpGtlDoc = NULL ;
    CGtlView * xpGtlFocusView, * xpGtlMouseView ;
    GetCurrentDocAndView(xpGtlDoc, xpGtlFocusView, xpGtlMouseView) ;

    CMenu * xpMenu = GetMenu() ;
    UINT uMenuState = 0 ;

    if (xpMenu)
        uMenuState = xpMenu->GetMenuState(ID_VIEW_INFO_DLG, 0) ;

    if (xpGtlDoc) {
        // clearing dialog box
        if ((uMenuState & MF_CHECKED) && xpGtlDoc->m_xpcInfDlg)
            xpGtlDoc->m_xpcInfDlg->DestroyWindow() ;

        else if ( !(uMenuState & MF_CHECKED) && !xpGtlDoc->m_xpcInfDlg)
            ShowInfoDialog() ;
    }  else {
        ::MessageBox(NULL, "No current document.", NULL, MB_OK) ;
    }
}


void CGtlFrame::OnViewCtlDlg()
{
    CGtlDoc * xpGtlDoc = NULL ;
    CGtlView * xpGtlFocusView, * xpGtlMouseView ;
    GetCurrentDocAndView(xpGtlDoc, xpGtlFocusView, xpGtlMouseView) ;

    CMenu * xpMenu = GetMenu() ;
    UINT uMenuState = 0 ;

    if (xpMenu)
        uMenuState = xpMenu->GetMenuState(ID_VIEW_CTL_DLG, 0) ;

    if (xpGtlDoc) {
        // clearing dialog box
        if ((uMenuState & MF_CHECKED) && xpGtlDoc->m_xpcCtlDlg)
            xpGtlDoc->m_xpcCtlDlg->DestroyWindow() ;

        else if ( !(uMenuState & MF_CHECKED) && !xpGtlDoc->m_xpcCtlDlg)
            ShowControlDialog() ;

    } else {
        ::MessageBox(NULL, "No current document.", NULL, MB_OK) ;
    }
}

//* CGtlFrame::ShowControlDialog -- 
BOOL CGtlFrame::ShowControlDialog(void)
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlFrame::ShowControlDialog) ;
    int iError = 0 ;        // error code
    CGtlDoc * xpGtlDoc = NULL ;
    CGtlView * xpGtlFocusView, * xpGtlMouseView ;
    GetCurrentDocAndView(xpGtlDoc, xpGtlFocusView, xpGtlMouseView) ;

    CMenu * xpMenu = GetMenu() ;

    xpGtlDoc->m_xpcCtlDlg = new CCtlDlg(this) ;
                    // create dialog object
    xpGtlDoc->m_xpcCtlDlg->m_xpGtlData = xpGtlDoc->m_xpGtlData ;
    xpGtlDoc->m_xpcCtlDlg->m_xpDocument = xpGtlDoc ;

    xpGtlDoc->m_xpGtlData->UpdateDialogs(FALSE) ;

    xpGtlDoc->m_xpcCtlDlg->Create() ;
            // create modeless dialog box
    if (xpMenu)
    xpMenu->CheckMenuItem(ID_VIEW_CTL_DLG, MF_CHECKED) ;
            // check menu item in frame window
// cleanup:
    JXELEAVE(CGtlFrame::ShowControlDialog) ;
    RETURN(iError != 0) ;
}

//* CGtlFrame::ShowNodeDialog -- 
BOOL CGtlFrame::ShowNodeDialog(void)
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlFrame::ShowNodeDialog) ;
    int iError = 0 ;        // error code

    CGtlDoc * xpGtlDoc = NULL ;
    CGtlView * xpGtlFocusView, * xpGtlMouseView ;
    GetCurrentDocAndView(xpGtlDoc, xpGtlFocusView, xpGtlMouseView) ;

    CMenu * xpMenu = GetMenu() ;

    xpGtlDoc->m_xpcNodeDlg = new CNodeDlg(this) ;
                    // create dialog object
    xpGtlDoc->m_xpcNodeDlg->m_xpGtlData = xpGtlDoc->m_xpGtlData ;
    xpGtlDoc->m_xpcNodeDlg->m_xpDocument = xpGtlDoc ;

    xpGtlDoc->m_xpGtlData->UpdateDialogs(FALSE) ;

    xpGtlDoc->m_xpcNodeDlg->Create() ;
            // create modeless dialog box
    if (xpMenu)
    xpMenu->CheckMenuItem(ID_VIEW_NODE_DLG, MF_CHECKED) ;
            // check menu item in frame window
// cleanup:
    JXELEAVE(CGtlFrame::ShowNodeDialog) ;
    RETURN(iError != 0) ;
}

//* CGtlFrame::ShowInfoDialog -- 
BOOL CGtlFrame::ShowInfoDialog(void)
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlFrame::ShowInfoDialog) ;
    int iError = 0 ;        // error code
    CGtlDoc * xpGtlDoc = NULL ;
    CGtlView * xpGtlFocusView, * xpGtlMouseView ;
    GetCurrentDocAndView(xpGtlDoc, xpGtlFocusView, xpGtlMouseView) ;

    CMenu * xpMenu = GetMenu() ;

    xpGtlDoc->m_xpcInfDlg = new CInfDlg(this) ;
                    // create dialog object
    xpGtlDoc->m_xpcInfDlg->m_xpGtlData = xpGtlDoc->m_xpGtlData ;
    xpGtlDoc->m_xpcInfDlg->m_xpDocument = xpGtlDoc ;

    xpGtlDoc->m_xpGtlData->UpdateDialogs(FALSE) ;

    xpGtlDoc->m_xpcInfDlg->Create() ;
            // create modeless dialog box
    if (xpMenu)
    xpMenu->CheckMenuItem(ID_VIEW_INFO_DLG, MF_CHECKED) ;
            // check menu item in frame window
// cleanup:
    JXELEAVE(CGtlFrame::ShowInfoDialog) ;
    RETURN(iError != 0) ;
}

//* CGtlFrame::ShowMenuDialog -- 
BOOL CGtlFrame::ShowMenuDialog(void)
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlFrame::ShowMenuDialog) ;
    int iError = 0 ;        // error code
    CGtlDoc * xpGtlDoc = NULL ;
    CGtlView * xpGtlFocusView, * xpGtlMouseView ;
    GetCurrentDocAndView(xpGtlDoc, xpGtlFocusView, xpGtlMouseView) ;

    CMenu * xpMenu = GetMenu() ;

    xpGtlDoc->m_xpcMenuDlg = new CMenuDlg(this) ;
                    // create dialog object
    xpGtlDoc->m_xpcMenuDlg->m_xpGtlData = xpGtlDoc->m_xpGtlData ;
    xpGtlDoc->m_xpcMenuDlg->m_xpDocument = xpGtlDoc ;

    xpGtlDoc->m_xpGtlData->UpdateDialogs(FALSE) ;

    xpGtlDoc->m_xpcMenuDlg->Create() ;
            // create modeless dialog box
    if (xpMenu)
    xpMenu->CheckMenuItem(ID_VIEW_MENU_DLG, MF_CHECKED) ;
            // check menu item in frame window
// cleanup:
    JXELEAVE(CGtlFrame::ShowMenuDialog) ;
    RETURN(iError != 0) ;
}

void CGtlFrame::OnViewMenuDlg()
{
    CGtlDoc * xpGtlDoc = NULL ;
    CGtlView * xpGtlFocusView, * xpGtlMouseView ;
    GetCurrentDocAndView(xpGtlDoc, xpGtlFocusView, xpGtlMouseView) ;

    CMenu * xpMenu = GetMenu() ;
    UINT uMenuState = 0 ;

    if (xpMenu)
        uMenuState = xpMenu->GetMenuState(ID_VIEW_MENU_DLG, 0) ;

    if (xpGtlDoc) {

        // clearing dialog box
        if ((uMenuState & MF_CHECKED) && xpGtlDoc->m_xpcMenuDlg)
            xpGtlDoc->m_xpcMenuDlg->DestroyWindow() ;

        else if ( !(uMenuState & MF_CHECKED) && !xpGtlDoc->m_xpcMenuDlg)
            ShowMenuDialog() ;

    } else {
        ::MessageBox(NULL, "No current document.", NULL, MB_OK) ;
    }
}


void CGtlFrame::OnViewNodeDlg()
{
    CGtlDoc * xpGtlDoc = NULL ;
    CGtlView * xpGtlFocusView, * xpGtlMouseView ;
    GetCurrentDocAndView(xpGtlDoc, xpGtlFocusView, xpGtlMouseView) ;

    CMenu * xpMenu = GetMenu() ;
    UINT uMenuState = 0 ;

    if (xpMenu)
        uMenuState = xpMenu->GetMenuState(ID_VIEW_NODE_DLG, 0);

    if (xpGtlDoc) {

        // clearing dialog box
        if ((uMenuState & MF_CHECKED) && xpGtlDoc->m_xpcNodeDlg)
            xpGtlDoc->m_xpcNodeDlg->DestroyWindow() ;

        else if ( !(uMenuState & MF_CHECKED) && !xpGtlDoc->m_xpcNodeDlg)
            ShowNodeDialog() ;

    } else {
        ::MessageBox(NULL, "No current document.", NULL, MB_OK) ;
    }
}
#endif


BOOL CGtlFrame::ShowClue(CPalette *pPalette, CNote *pNote)
{
    CNotebook dlgNoteBook((CWnd *)this, pPalette, NULL, pNote);
    dlgNoteBook.DoModal();
    SetupCursor();
    return(FALSE);
}

//* CGtlFrame::GetCurrentDocAndView -- get last focused doc/view
BOOL CGtlFrame::GetCurrentDocAndView(CGtlDoc *& xpcGtlDoc,
    CGtlView *& xpcGtlFocusView, CGtlView *& xpcGtlMouseView)
// xpcGtlDoc (output) -- the document which most recently had focus
//      in some view, or NULL if none
// xpcGtlFocusView (output) -- view which had most recent focus, or NULL
// xpcGtlMouseView (output) -- the view for this same document that was
//      most recently touched by the mouse, or NULL
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlFrame::GetCurrentDocAndView) ;
    int iError = 0 ;        // error code

    xpcGtlDoc = m_xpDocument ;

    if ((xpcGtlFocusView = m_xpcLastFocusView) != NULL) {

        if (!xpcGtlDoc)
            xpcGtlDoc = m_xpcLastFocusView->GetDocument() ;

        if (xpcGtlDoc)
            xpcGtlMouseView = xpcGtlDoc->m_xpcLastMouseView ;

    // no pointer to last focus view
    //
    } else {
        xpcGtlMouseView = NULL;
    }

// cleanup:

    JXELEAVE(CGtlFrame::GetCurrentDocAndView) ;
    RETURN(iError != 0) ;
}


//* CGtlFrame::PreCreateWindow -- change frame window style
BOOL CGtlFrame::PreCreateWindow(CREATESTRUCT& cCs)
// returns: value returned by base class function
{
    JXENTER(CGtlFrame::PreCreateWindow) ;
    int iError = 0 ;        // error code
    BOOL bRetval ;      // return value
    static CString stWndClass("") ;
    CGtlApp * xpGtlApp = (CGtlApp *)AfxGetApp() ; // get application
//  CClientDC cDc(this) ;

//  if (stWndClass == "")
//  stWndClass = AfxRegisterWndClass(CS_DBLCLKS |
//              CS_BYTEALIGNWINDOW | CS_OWNDC,
//              NULL, NULL, NULL) ;

    if (!xpGtlApp->m_bTitle) {   // if we don't want a title bar
//  	cCs.lpszClass = stWndClass ;

    // determine where to place the game window
    // ... so it is centered on the screen
//  	cCs.x = (cDc.GetDeviceCaps(HORZRES) - xpGtlApp->m_iWidth) >> 1 ;
//  	cCs.y = (cDc.GetDeviceCaps(VERTRES) - xpGtlApp->m_iHeight) >> 1 ;
//  	cCs.x = (GetSystemMetrics(SM_CXSCREEN) - xpGtlApp->m_iWidth) >> 1 ;
//  	cCs.y = (GetSystemMetrics(SM_CYSCREEN)- xpGtlApp->m_iHeight) >> 1 ;
	    cCs.x = xpGtlApp->m_iX ;
	    cCs.y = xpGtlApp->m_iY ;
	    cCs.cx = xpGtlApp->m_iWidth ;
	    cCs.cy = xpGtlApp->m_iHeight ;
	
	    if ( !(cCs.style & WS_CHILD) ) {
	        cCs.style = WS_POPUP | WS_VISIBLE ;
	        if (cCs.hMenu != NULL) 
				::DestroyMenu(cCs.hMenu);
	        cCs.hMenu = 0 ;
	    }
    }

    bRetval = MFC_FRAME::PreCreateWindow(cCs) ;

// cleanup:

    JXELEAVE(CGtlFrame::PreCreateWindow) ;
    RETURN(bRetval) ;
}


//* CGtlFrame::RecalcLayout -- override CFrameWnd::RecalcLayout
void CGtlFrame::RecalcLayout(BOOL bNotify)
// returns: VOID
{
    JXENTER(CGtlFrame::RecalcLayout) ;
    int iError = 0 ;        // error code
    CRect cParamRect, cClientRect(0, 0, 200, 200) ;
    CGtlApp * xpGtlApp = (CGtlApp *)AfxGetApp() ; // get application

//      // call the layout hook -- OLE 2.0 support uses this hook
//      if (bNotify && m_pNotifyHook != NULL)
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

#if OLD_CODE

////* CGtlFrame::CallOnFileNew -- 
void CGtlFrame::CallOnFileNew(void)
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlFrame::CallOnFileNew) ;
    int iError = 0 ;        // error code
    CGtlApp * xpGtlApp = (CGtlApp *)AfxGetApp() ; // get application

    xpGtlApp->CallOnFileNew() ;

//cleanup:

    JXELEAVE(CGtlFrame::CallOnFileNew) ;
    RETURN_VOID ;
}

////* CGtlFrame::CallOnFileOpen -- 
void CGtlFrame::CallOnFileOpen(void)
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlFrame::CallOnFileOpen) ;
    int iError = 0 ;        // error code
    CGtlApp * xpGtlApp = (CGtlApp *)AfxGetApp() ; // get application

    xpGtlApp->CallOnFileOpen() ;

//cleanup:

    JXELEAVE(CGtlFrame::CallOnFileOpen) ;
    RETURN_VOID ;
}

#endif /* OLD_CODE */

void CGtlFrame::ProcessMiniGameReturn()
{
    //
    // Obsolete function
    //
    assert(0);
}

void CGtlFrame::ShowOptions( CPalette *pPalette )
{
    CMetaOptDlg cOptionsDlg( (CWnd*)this,pPalette );
    int         nOptionsReturn = 0;

    cOptionsDlg.SetInitialOptions( lpMetaGameStruct );  // Sets the private members
    nOptionsReturn = cOptionsDlg.DoModal();
    if ( nOptionsReturn == 1 ) {
        bExitMetaDLL = TRUE;
        st_bExitDll = TRUE;
    }
    
    return;
}

void CGtlFrame::ShowInventory( CPalette *pPalette, int nWhichDlg )
{
    CHodjPodj *pPlayer;

    // which player
    pPlayer = &lpMetaGameStruct->m_cPodj;
    if (lpMetaGameStruct->m_cHodj.m_bMoving)
        pPlayer = &lpMetaGameStruct->m_cHodj;

    switch ( nWhichDlg ) {

        case 4: {   // black market
            CGeneralStore cBMarketDlg((CWnd *)this, pPalette, pPlayer->m_pBlackMarket, pPlayer->m_pInventory);
            cBMarketDlg.DoModal();
            break;
        }

        case 3: {   // pawn shop
            CPawnShop cPawnnShopDlg((CWnd *)this, pPalette, ((rand() & 1)  == 1 ? pPlayer->m_pGenStore : pPlayer->m_pBlackMarket), pPlayer->m_pInventory);
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
            CBackpack dlgBackPack( (CWnd *)this, pPalette, pPlayer->m_pInventory);
            dlgBackPack.DoModal();
            break;
        }
    }

    SetupCursor();
}

void CGtlFrame::OnCallNew()
{
    // TODO: Add your command handler code here
    JXENTER(CGtlFrame::OnCallNew) ;
    int iError = 0 ;        // error code
    CGtlApp * xpGtlApp = (CGtlApp *)AfxGetApp() ; // get application 
    
    xpGtlApp->CallOnFileNew() ;

//cleanup:

    JXELEAVE(CGtlFrame::OnCallNew) ;
    RETURN_VOID ;

}

#if !GTLDLL
void CGtlFrame::CreateInventories(void)
{
    CItem                   *pItem = NULL;
    int                             i,j;

    if ( lpMetaGameStruct != NULL ) {
        delete lpMetaGameStruct;
        lpMetaGameStruct = NULL;     
    }
    lpMetaGameStruct = new CBfcMgr(); 

        lpMetaGameStruct->m_cPodj.m_iSkillLevel = SKILLLEVEL_LOW;
        lpMetaGameStruct->m_cHodj.m_iSkillLevel = SKILLLEVEL_LOW;

        lpMetaGameStruct->m_cPodj.m_bComputer = FALSE;
        lpMetaGameStruct->m_cHodj.m_bComputer = FALSE;

        lpMetaGameStruct->m_cPodj.m_pBlackMarket = NULL;
        lpMetaGameStruct->m_cHodj.m_pBlackMarket = NULL;
        lpMetaGameStruct->m_cPodj.m_pGenStore = NULL;
        lpMetaGameStruct->m_cHodj.m_pGenStore = NULL;
    lpMetaGameStruct->m_cPodj.m_pInventory = NULL;
    lpMetaGameStruct->m_cHodj.m_pInventory = NULL;

        if ( lpMetaGameStruct->m_cHodj.m_pInventory == NULL ) {
                lpMetaGameStruct->m_cHodj.m_pInventory = new CInventory("Hodj's Stuff");
                lpMetaGameStruct->m_cHodj.m_pInventory->AddItem( MG_OBJ_HODJ_NOTEBOOK, 1);
                lpMetaGameStruct->m_cHodj.m_pInventory->AddItem( MG_OBJ_CROWN, 1000);
                                        
                lpMetaGameStruct->m_cHodj.m_pGenStore = new CInventory("General Store");
                for ( i = MG_OBJ_BASE; i <= MG_OBJ_MAX; i++ ) {
                        switch ( i ) {
                                case MG_OBJ_HERRING:
                                case MG_OBJ_MISH:
                                case MG_OBJ_MOSH:
                                case MG_OBJ_HODJ_NOTEBOOK:
                                case MG_OBJ_PODJ_NOTEBOOK:
                                case MG_OBJ_CROWN:
                                        break;
                                default:  
                                        lpMetaGameStruct->m_cHodj.m_pGenStore->AddItem( i,1 );
                                        break;
                        }
                } 
                                        
                lpMetaGameStruct->m_cHodj.m_pBlackMarket = new CInventory("Black Market");
        for  ( i = 0; i < ITEMS_IN_PAWN_SHOP; i ++ ) {
                j = rand() % lpMetaGameStruct->m_cHodj.m_pGenStore->ItemCount();
                        pItem = lpMetaGameStruct->m_cHodj.m_pGenStore->FetchItem( j );
                        lpMetaGameStruct->m_cHodj.m_pGenStore->RemoveItem( pItem );
                                        
                        lpMetaGameStruct->m_cHodj.m_pBlackMarket->AddItem( pItem );
        }
                                                        
        }
                                        
        if ( lpMetaGameStruct->m_cPodj.m_pInventory == NULL ) {
                lpMetaGameStruct->m_cPodj.m_pInventory = new CInventory("Podj's Stuff");
                lpMetaGameStruct->m_cPodj.m_pInventory->AddItem( MG_OBJ_PODJ_NOTEBOOK, 1);
                lpMetaGameStruct->m_cPodj.m_pInventory->AddItem( MG_OBJ_CROWN, 1000);
                                        
                lpMetaGameStruct->m_cPodj.m_pGenStore = new CInventory("General Store");
                for ( i = MG_OBJ_BASE; i <= MG_OBJ_MAX; i++ ) {
                        switch ( i ) {
                                case MG_OBJ_HERRING:
                                case MG_OBJ_MISH:
                                case MG_OBJ_MOSH:
                                case MG_OBJ_HODJ_NOTEBOOK:
                                case MG_OBJ_PODJ_NOTEBOOK:
                                case MG_OBJ_CROWN:
                                        break;
                                default:  
                                        lpMetaGameStruct->m_cPodj.m_pGenStore->AddItem( i,1 );
                                        break;
                        }
                } 
                                        
                lpMetaGameStruct->m_cPodj.m_pBlackMarket = new CInventory("Black Market");
        for  ( i = 0; i < ITEMS_IN_PAWN_SHOP; i ++ ) {
                j = rand() % lpMetaGameStruct->m_cPodj.m_pGenStore->ItemCount();
                        pItem = lpMetaGameStruct->m_cPodj.m_pGenStore->FetchItem( j );
                        lpMetaGameStruct->m_cPodj.m_pGenStore->RemoveItem( pItem );
                                        
                        lpMetaGameStruct->m_cPodj.m_pBlackMarket->AddItem( pItem );
        }
                                                        
        }
        lpMetaGameStruct->m_bInventories = TRUE;
    lpMetaGameStruct->m_stGameStruct.lCrowns = 1000;
    lpMetaGameStruct->m_stGameStruct.lScore = 0;
    lpMetaGameStruct->m_stGameStruct.nSkillLevel = SKILLLEVEL_MEDIUM;
    lpMetaGameStruct->m_stGameStruct.bSoundEffectsEnabled = TRUE;
    lpMetaGameStruct->m_stGameStruct.bMusicEnabled = TRUE;
    lpMetaGameStruct->m_stGameStruct.bPlayingHodj = TRUE;
    lpMetaGameStruct->m_bRestart = FALSE;
    lpMetaGameStruct->m_stGameStruct.bPlayingMetagame = TRUE;
    lpMetaGameStruct->m_iGameTime = SHORT_GAME;
}
#endif

void CGtlFrame::OnCallOpen()
{
    // TODO: Add your command handler code here
    JXENTER(CGtlFrame::OnCallOpen) ;
    int iError = 0 ;        // error code
    CGtlApp * xpGtlApp = (CGtlApp *)AfxGetApp() ; // get application

    xpGtlApp->CallOnFileOpen() ;

//cleanup:

    JXELEAVE(CGtlFrame::OnCallOpen) ;
    RETURN_VOID ;
    
}

BOOL CGtlFrame::OnEraseBkgnd(CDC *)
{
    return(TRUE);
}

#ifdef NODEEDIT
void CGtlFrame::OnCallSave()
{
    // TODO: Add your command handler code here
    JXENTER(CGtlFrame::OnCallSave) ;
    int iError = 0 ;        // error code
    CGtlApp * xpGtlApp = (CGtlApp *)AfxGetApp() ; // get application

    xpGtlApp->CallOnFileSave() ;

//cleanup:

    JXELEAVE(CGtlFrame::OnCallSave) ;
    RETURN_VOID ;
}
#endif

void CGtlFrame::OnCallClose()
{
}

void CGtlFrame::OnCallExit()
{
    int         iReturnValue = -1 ;
    CGtlDoc     *xpGtlDoc = NULL ;
    CGtlView    *xpGtlFocusView, *xpGtlMouseView ;

    GetCurrentDocAndView(xpGtlDoc, xpGtlFocusView, xpGtlMouseView) ;

    xpGtlDoc->m_xpGtlData->m_xpGtlView->KillTimer(ANIMATION_TIMER_ID);
    
#if RETAIN_META_DLL
    ShowWindow(SW_HIDE);
    
    if (gpBgbMgr != NULL)    
        gpBgbMgr->CacheOptimize(2000000);
#else
    if (gpBgbMgr != NULL)    
        gpBgbMgr->CacheFlush();
#endif

    if (m_lpBfcMgr && m_lpBfcMgr->m_iFunctionCode > 0
            && m_lpBfcMgr->m_iFunctionCode != MG_DLLX_QUIT)
        iReturnValue = m_lpBfcMgr->m_iFunctionCode ;

    lpMetaGameStruct->m_iFunctionCode = iReturnValue;

#if RETAIN_META_DLL
    ::PostMessage( ghwndParent, WM_PARENTNOTIFY, WM_DESTROY, (LPARAM) iReturnValue );
#else
//  CWnd::DestroyWindow();
	AfxGetApp()->CloseAllDocuments(FALSE);
#endif
}

    
void CGtlFrame::OnSysCommand( UINT nID, LPARAM lParam )
{
    if ((nID & 0xfff0) != SC_SCREENSAVE)
        CWnd::OnSysCommand(  nID, lParam );
}


void CGtlFrame::OnActivateApp(BOOL bActive, HTASK /*hTask*/)
{
	if (!bActive)
		gpBgbMgr->CacheFlush();
}
