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
#include "bagel/hodjnpodj/hnplibs/dibapi.h"
#include "bagel/hodjnpodj/metagame/gtl/gtl.h"
#include "bagel/hodjnpodj/metagame/gtl/gtldoc.h"
#include "bagel/hodjnpodj/metagame/gtl/gtlview.h"
#include "bagel/hodjnpodj/metagame/gtl/gtlfrm.h"
#include "bagel/hodjnpodj/metagame/gtl/infdlg.h"
#include "bagel/hodjnpodj/hnplibs/rules.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

///DEFS gtlview.h

extern HWND     ghwndParent;
extern CBfcMgr  *lpMetaGameStruct;
extern BOOL     bExitMetaDLL;
extern CBgbMgr  *gpBgbMgr;


CGtlView *gpMyView;

/////////////////////////////////////////////////////////////////////////////
// CGtlView

IMPLEMENT_DYNCREATE(CGtlView, MFC_VIEW)

BEGIN_MESSAGE_MAP(CGtlView, MFC_VIEW)
	//{{AFX_MSG_MAP(CGtlView)

// those functions not used in game shoule be put into here and
// then ifdef'd out
#ifdef NODEEDIT
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_SETFOCUS()
	ON_WM_SIZE()
	ON_WM_KEYUP()
	ON_COMMAND(ID_FILE_PRINT, MFC_VIEW::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, MFC_VIEW::OnFilePrintPreview)
#endif
	ON_WM_KEYDOWN()
	ON_WM_SYSKEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_WM_SETCURSOR()
	ON_MESSAGE(MM_MCINOTIFY, CGtlView::OnMCINotify)     // uncomment these when
	ON_MESSAGE(MM_WOM_DONE, CGtlView::OnMMIONotify)     // the spinner sound works
	//}}AFX_MSG_MAP
	// Standard printing commands
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGtlView construction/destruction

CGtlView::CGtlView() {
	TRACECONSTRUCTOR(CGtlView) ;
	memset(&m_cStartData, 0, &m_cEndData - &m_cStartData) ;

	if (GetDocument())
		GetDocument()->FixChecks() ;

	gpMyView = this;
}

CGtlView::~CGtlView() {
	TRACEDESTRUCTOR(CGtlView) ;
}

/////////////////////////////////////////////////////////////////////////////

int CGtlView::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	JXENTER(CGtlView::OnCreate) ;
	int iRetval = 0 ;       // return value

	CWnd * xpWnd = this ;
	while (xpWnd->GetParent())
		xpWnd = xpWnd->GetParent() ;
	m_xpFrame = (CGtlFrame *)xpWnd ;


	if (MFC_VIEW::OnCreate(lpCreateStruct) == -1) {
		iRetval = -1 ;
		goto cleanup ;
	}

cleanup:

	JXELEAVE(CGtlView::OnCreate) ;
	return (iRetval) ;
}


#ifndef NODEEDIT

//* CGtlView::OnUpdate -- called when document changes to update view
void CGtlView::OnUpdate(CView *xpSender, LPARAM lHint, CObject *xpHint)
// returns: void
{
//    dbgtrc = TRUE ;   // debugging code

	JXENTER(CGtlView::OnUpdate) ;
	CGtlHint * xpcGtlHint = (CGtlHint *)xpHint ;
	BOOL bDone = FALSE ;    // TRUE when hint is fully identified

	TRACE("Enter OnUpdate\n");

	// The document has informed this view that some data has changed.
	CGtlDoc* xpDoc = GetDocument() ;
	CGtlData * xpGtlData = xpDoc->m_xpGtlData ;

	// sending to all views but current one
	if (!xpGtlData || this == xpSender)
		goto cleanup ;      // just exit

	switch (lHint) {

	case HINT_UPDATE_RECT:
		if (xpHint) {
			xpGtlData->Draw(this, &xpcGtlHint->cHintRect);
			bDone = TRUE ;
		}
		break ;

	case HINT_UPDATE_FULL:
		xpGtlData->Draw(this, nullptr);
		bDone = TRUE ;
		break ;

	case HINT_INIT_METAGAME:
		assert(0);
		break ;

	// change in size of document
	//
	case HINT_SIZE:

		// make sure this flag is processed
		// for all views
		//
		if (xpDoc->m_xpGtlData)
			xpDoc->m_xpGtlData->m_bChangeSize = TRUE;
		break;

	default:

		//xpGtlData->Draw(this, nullptr);
		xpGtlData->m_bStartMetaGame = FALSE;
		xpGtlData->m_bMetaGame = xpGtlData->m_bInitMetaGame = TRUE;
		xpDoc->m_xpGtlData->InitMetaGame(this);

		// BRIAN
		SetTimer(ANIMATION_TIMER_ID, ANIMATION_TIMER_INTERVAL, nullptr);
		break;
	}

	if (xpDoc->m_xpGtlData)
		xpDoc->m_xpGtlData->SpecifyUpdate(this);

	TRACE("Exit OnUpdate\n");

cleanup:

	JXELEAVE(CGtlView::OnUpdate) ;
	RETURN_VOID ;
}

#else

//* CGtlView::OnUpdate -- called when document changes to update view
void CGtlView::OnUpdate(CView *xpSender, LPARAM lHint, CObject *xpHint)
// returns: void
{
//    dbgtrc = TRUE ;   // debugging code

	JXENTER(CGtlView::OnUpdate) ;
	int iError = 0 ;        // error code
	CGtlHint * xpcGtlHint = (CGtlHint *)xpHint ;
	BOOL bDone = FALSE ;    // TRUE when hint is fully identified

	// The document has informed this view that some data has changed.
	CGtlDoc* xpDoc = GetDocument() ;
	CGtlData * xpGtlData = xpDoc->m_xpGtlData ;

	if (!xpGtlData || this == xpSender)
		// sending to all views but current one
		goto cleanup ;      // just exit

	switch (lHint) {
	case HINT_UPDATE_RECT:
		if (xpHint) {
//      CClientDC cDc(this) ;
			#if NEWSCROLL
//      m_cViewBsuSet.PrepareDc(&cDc) ;
			#else
			OnPrepareDC(&cDc) ;
			#endif

//*****     cDc.LPtoDP(&xpcGtlHint->cHintRect) ;
			if (xpcGtlHint->m_bWmPaint) {
				InvalidateRect(&xpcGtlHint->cHintRect) ;
				UpdateWindow() ;
			} else      // direct paint
				if (!xpGtlData->m_bInhibitDraw)
					xpGtlData->Draw(this, &xpcGtlHint->cHintRect) ;

			bDone = TRUE ;
		}
		break ;

	case HINT_UPDATE_FULL:
		Invalidate(TRUE) ;
		bDone = TRUE ;
		break ;

	case HINT_INIT_METAGAME:
		if (xpDoc->m_xpGtlData) {
//      xpDoc->m_xpGtlData->Draw(this, nullptr) ;
			InvalidateRect(nullptr) ;
			UpdateWindow() ;
			xpDoc->m_xpGtlData->InitMetaGame(this), bDone = TRUE ;

			// start the animations
			SetTimer(TIMER_ID, 100, nullptr);
		}
		break ;

	case HINT_SIZE:     // change in size of document
		if (xpDoc->m_xpGtlData)
			xpDoc->m_xpGtlData->m_bChangeSize = TRUE ;
		// make sure this flag is processed
		// for all views
		break ;

	default:
		break ;
	}

	if (!bDone)
		Invalidate(TRUE) ;

	if (xpDoc->m_xpGtlData)
		xpDoc->m_xpGtlData->SpecifyUpdate(this) ;

//  if (lHint || xpHint)
//  {
//  if (xpHint->IsKindOf(RUNTIME_CLASS(CStroke)))
//  {
//      // The hint is that a stroke as been added (or changed).
//      // So, invalidate its rectangle.
//      CStroke* pStroke = (CStroke*)xpHint;
//      CClientDC dc(this);
//      OnPrepareDC(&dc);
//      CRect rectInvalid = pStroke->GetBoundingRect();
//      dc.LPtoDP(&rectInvalid);
//      InvalidateRect(&rectInvalid);
//      return;
//  }
//  }

	if (xpGtlData->m_bStartMetaGame) {
		UpdateWindow() ;
		xpGtlData->SetMetaGame(TRUE) ;
	}

cleanup:

	JXELEAVE(CGtlView::OnUpdate) ;
	RETURN_VOID ;
}
#endif

//  A view's OnInitialUpdate() overrideable function is called immediately
//  after the frame window is created, and the view within the frame
//  window is attached to its document.  This provides the scroll view the
//  opportunity to set its size (m_totalSize) based on the document size.


//* CGtlView::OnInitialUpdate --
void CGtlView::OnInitialUpdate(void) {
	JXENTER(CGtlView::OnInitialUpdate) ;
	CGtlDoc *xpDoc = GetDocument() ;

	#if NEWSCROLL
//  m_cViewBsuSet.InitWndBsuSet(this, TRUE) ;
	m_cViewBsuSet.InitWndBsuSet(this, SCROLLVIEW, FALSE) ;
	m_bBsuInit = TRUE ;
	if (m_bBsuInit && xpDoc->m_xpGtlData) {
		m_cViewBsuSet.PrepareWndBsuSet(CSize(xpDoc->m_xpGtlData->m_iSizeX,
		                                     xpDoc->m_xpGtlData->m_iSizeY),
		                               CRect(0, xpDoc->m_xpGtlData->m_iMargin, 0, 0)) ;
	}
	#else

	if (xpDoc->m_xpGtlData)
		SetScrollSizes(MM_TEXT, CSize(xpDoc->m_xpGtlData->m_iSizeX,
		                              xpDoc->m_xpGtlData->m_iSizeY)) ;
	#endif

//
//    OnUpdate(nullptr, 0, nullptr) ;

// cleanup:

	JXELEAVE(CGtlView::OnInitialUpdate) ;
	RETURN_VOID ;
}

void CGtlView::OnDestroy() {
	int iReturnValue = -1 ;

	JXENTER(CGtlView::OnDestroy) ;

	MFC_VIEW::OnDestroy();

	#if !RETAIN_META_DLL
	if ((lpMetaGameStruct && lpMetaGameStruct->m_iFunctionCode > 0) && (lpMetaGameStruct->m_iFunctionCode != MG_DLLX_QUIT)) {
		iReturnValue = lpMetaGameStruct->m_iFunctionCode ;
	}

	m_xpFrame = nullptr ;
	MFC::PostMessage(ghwndParent, WM_PARENTNOTIFY, WM_DESTROY, (LPARAM) iReturnValue);
	#endif

	JXELEAVE(CGtlView::OnDestroy) ;
	RETURN_VOID ;
}

//* CGtlView::PreCreateWindow -- change view window style
BOOL CGtlView::PreCreateWindow(CREATESTRUCT& cCs)
// returns: value returned by base class function
{
	JXENTER(CGtlView::PreCreateWindow) ;
	BOOL bRetval ;      // return value
	static CString stWndClass("") ;
	CGtlApp * xpGtlApp = (CGtlApp *)AfxGetApp() ; // get application

//  if (stWndClass == "")
//  stWndClass = AfxRegisterWndClass(CS_DBLCLKS |
//              CS_BYTEALIGNWINDOW | CS_OWNDC,
//              nullptr, nullptr, nullptr) ;

	// if we don't want a title bar
	if (!xpGtlApp->m_bTitle) {
		//cCs.lpszClass = stWndClass ;
		cCs.cx = xpGtlApp->m_iWidth ;
		cCs.cy = xpGtlApp->m_iHeight ;
		cCs.x = cCs.y = 0 ;

		if (!(cCs.style & WS_CHILD)) {
			cCs.style = WS_POPUP | WS_VISIBLE ;
			cCs.hMenu = 0 ;
		}
	}

	bRetval = MFC_VIEW::PreCreateWindow(cCs) ;

// cleanup:

	JXELEAVE(CGtlView::PreCreateWindow) ;
	RETURN(bRetval) ;
}



/////////////////////////////////////////////////////////////////////////////
// CGtlView drawing

//* CGtlView::OnDraw -- draw current view
void CGtlView::OnDraw(CDC *xpDc) {
	JXENTER(CGtlView::OnDraw) ;
	CGtlDoc* xpDoc = GetDocument();

	#ifdef BAGEL_DEBUG
	if (xpDoc == nullptr)
		MessageBox("No Document!!!");
	#endif

	#if NEWSCROLL
//    m_cViewBsuSet.PrepareDc(xpDc) ;
	xpDc->SetViewportOrg(0, 0) ;

	#else
	OnPrepareDC(xpDc) ;
	#endif
	if (xpDoc->m_xpGtlData && !xpDoc->m_xpGtlData->m_bInhibitDraw)
		xpDoc->m_xpGtlData->Draw(this, nullptr, xpDc) ;

// cleanup:

	JXELEAVE(CGtlView::OnDraw) ;
	RETURN_VOID ;
}


/////////////////////////////////////////////////////////////////////////////
// CGtlView diagnostics

#ifdef BAGEL_DEBUG
void CGtlView::AssertValid() const {
	JXENTER(CGtlView::AssertValid) ;
	int iError = 0 ;

	CView::AssertValid();
	JXELEAVE(CGtlView::AssertValid) ;
}

void CGtlView::Dump(CDumpContext& dc) const {
	JXENTER(CGtlView::Dump) ;
	int iError = 0 ;
	CView::Dump(dc);
	JXELEAVE(CGtlView::Dump) ;
}

CGtlDoc *CGtlView::GetDocument() { // non-debug version is inline
	JXENTER(CGtlView::GetDocument) ;
	int iError = 0 ;
	if (m_pDocument)
		ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGtlDoc)));
	;
	JXELEAVE(CGtlView::GetDocument) ;
	return (CGtlDoc*)m_pDocument;
}
#endif //BAGEL_DEBUG


#ifdef NODEEDIT
//* CGtlView::CheckSize -- check window size, adjust if necessary
BOOL CGtlView::CheckSize(void)
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CGtlView::CheckSize) ;
	int iError = 0 ;        // error code
	static BOOL bSkipFix = TRUE ;   // for debugging

	WINDOWPLACEMENT stWp ;

	stWp.length = sizeof(stWp) ;

	if (GetDocument()) {
		GetWindowPlacement(&stWp) ;
		if (stWp.showCmd == SW_SHOWNORMAL &&
		        (stWp.rcNormalPosition.right
		         < stWp.rcNormalPosition.left + 20
		         || stWp.rcNormalPosition.bottom
		         < stWp.rcNormalPosition.top + 20)) {
			stWp.rcNormalPosition.right
			    = stWp.rcNormalPosition.left + 200 ;
			stWp.rcNormalPosition.bottom
			    = stWp.rcNormalPosition.top + 200 ;
			if (!bSkipFix)
				((CWnd *)this)->SetWindowPlacement(&stWp) ;
		}
	}

// cleanup:

	JXELEAVE(CGtlView::CheckSize) ;
	RETURN(iError != 0) ;
}
#endif


/////////////////////////////////////////////////////////////////////////////
// CGtlView message handlers

BOOL CGtlView::OnEraseBkgnd(CDC *) {
	return (TRUE) ; // this tells MFC that the background has been erased
}

void CGtlView::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
	switch (nChar) {

	// User has hit ALT_F4 so close down this App
	//
	case VK_F4:
		lpMetaGameStruct->m_iFunctionCode = MG_DLLX_QUIT;
		GetParent()->DestroyWindow();
		break;

	default:
		CView::OnSysKeyDown(nChar, nRepCnt, nFlags);
		break;
	}
}

void CGtlView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
	JXENTER(CGtlView::OnKeyDown) ;
	CGtlDoc *xpDoc = GetDocument() ;
	BOOL    bJunk, bAnimations;
	CPoint  ptEdge(0, 0);
	CPoint  ptAdjustedEdge(0, 0);

	switch (nChar) {

	case VK_F1: {
		CRules RulesDlg(this, "metarule.txt", gpBgbMgr->m_xpGamePalette, nullptr);
		if ((bAnimations = gpBgbMgr->AnimationsActive()) != FALSE)
			gpBgbMgr->PauseAnimations();
		gpBgbMgr->CacheOptimize(2000000);
		(void) RulesDlg.DoModal();
		if (bAnimations)
			gpBgbMgr->ResumeAnimations();
		break;
	}

	case VK_F2:
		xpDoc->m_xpGtlData->InitInterface(MG_DLLX_SCROLL, bJunk);
		break;

	case VK_M:
		xpDoc->m_xpGtlData->InitInterface(MG_DLLX_ZOOM, bJunk);
		break;

	case VK_I:
		xpDoc->m_xpGtlData->InitInterface(MG_DLLX_INVENTORY, bJunk);
		break;

	case VK_H:
		if (xpDoc->m_xpGtlData->m_xpXodjChain->m_bHodj)
			xpDoc->m_xpcLastFocusView->m_cViewBsuSet.EdgeToCenter(xpDoc->m_xpGtlData->m_xpXodjChain->m_lpcCharSprite->m_crPosition, TRUE);
		else
			xpDoc->m_xpcLastFocusView->m_cViewBsuSet.EdgeToCenter(xpDoc->m_xpGtlData->m_xpXodjChain->m_xpXodjNext->m_lpcCharSprite->m_crPosition, TRUE);
		break;

	case VK_P:
		if (xpDoc->m_xpGtlData->m_xpXodjChain->m_bHodj)
			xpDoc->m_xpcLastFocusView->m_cViewBsuSet.EdgeToCenter(xpDoc->m_xpGtlData->m_xpXodjChain->m_xpXodjNext->m_lpcCharSprite->m_crPosition, TRUE);
		else
			xpDoc->m_xpcLastFocusView->m_cViewBsuSet.EdgeToCenter(xpDoc->m_xpGtlData->m_xpXodjChain->m_lpcCharSprite->m_crPosition, TRUE);
		break;

	/*
	        case VK_SPACE:  // space down is identical to left button down
	            xpDoc->m_xpGtlData->AcceptClick(this,m_cCurrentPosition, CLICK_LDOWN) ;
	            break;
	*/

	case VK_SPACE:
	case VK_RETURN:
		xpDoc->m_xpcLastFocusView->m_cViewBsuSet.EdgeToCenter(xpDoc->m_xpGtlData->m_xpCurXodj->m_lpcCharSprite->m_crPosition, TRUE);
		break;

	case VK_LEFT:
		ptEdge.x = 10;
		ptEdge.y = 264;
		ptAdjustedEdge = m_cViewBsuSet.PointLogical(ptEdge);
		m_cViewBsuSet.EdgeToCenter(ptAdjustedEdge);
		break;

	case VK_UP:
		ptEdge.x = 320;
		ptEdge.y = 34;
		ptAdjustedEdge = m_cViewBsuSet.PointLogical(ptEdge);
		m_cViewBsuSet.EdgeToCenter(ptAdjustedEdge);
		break;

	case VK_RIGHT:
		ptEdge.x = 630;
		ptEdge.y = 264;
		ptAdjustedEdge = m_cViewBsuSet.PointLogical(ptEdge);
		m_cViewBsuSet.EdgeToCenter(ptAdjustedEdge);
		break;

	case VK_DOWN:
		ptEdge.x = 320;
		ptEdge.y = 470;
		ptAdjustedEdge = m_cViewBsuSet.PointLogical(ptEdge);
		m_cViewBsuSet.EdgeToCenter(ptAdjustedEdge);
		break;

		#ifdef BAGEL_DEBUG
	case VK_F9:
		xpDoc->m_xpGtlData->GainRandomItem(xpDoc->m_xpGtlData->m_xpCurXodj);
		break;

	case VK_F10: {
		int iNode;
		iNode = xpDoc->m_xpGtlData->m_xpCurXodj->m_iCharNode;
		xpDoc->m_xpGtlData->m_xpCurXodj->m_iCharNode = xpDoc->m_xpGtlData->FindNodeId("Amphi");
		xpDoc->m_xpGtlData->DivulgeInformation(xpDoc->m_xpGtlData->m_xpCurXodj, FALSE);
		xpDoc->m_xpGtlData->m_xpCurXodj->m_iCharNode = iNode;
		break;
	}

	case VK_F11:
		xpDoc->m_xpGtlData->GainMoney(xpDoc->m_xpGtlData->m_xpCurXodj, 10);
		break;

	case VK_F12:
		::ShowMemoryInfo("Memory status follows...", "Debug Information");
		break;
		#endif

	default:
		break;
	}

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
	JXELEAVE(CGtlView::OnKeyDown) ;
}


#ifdef NODEEDIT
void CGtlView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {
	JXENTER(CGtlView::OnKeyUp) ;
	int iError = 0 ;
	CGtlDoc* xpDoc = GetDocument() ;
	BOOL bShiftKey ;        // shift key status
	int iMoveAmount = 1 ;   // amount to move cursor
	POINT stPoint ;     // current cursor position

	CClientDC cDc(this) ;   // client device context
	CDC * xpDc = &cDc ;

	#if NEWSCROLL
	m_cViewBsuSet.PrepareDc(xpDc) ;
	#else
	OnPrepareDC(xpDc) ;
	#endif


//    CPoint cCurrentPosition = xpDc->GetCurrentPosition() ;
//    CPoint cCurrentPosition = m_cCurrentPosition ;
//                  // get current mouse position

	::GetCursorPos(&stPoint) ;  // get current cursor position on screen

	bShiftKey = ::GetKeyState(VK_SHIFT) < 0 ;
	if (bShiftKey)
		iMoveAmount = 10 ;

	switch (nChar) {

	case VK_LEFT:
		::SetCursorPos(stPoint.x - iMoveAmount, stPoint.y) ;
		break ;

	case VK_RIGHT:
		::SetCursorPos(stPoint.x + iMoveAmount, stPoint.y) ;
		break ;

	case VK_UP:
		::SetCursorPos(stPoint.x, stPoint.y - iMoveAmount) ;
		break ;

	case VK_DOWN:
		::SetCursorPos(stPoint.x, stPoint.y + iMoveAmount) ;
		break ;

	case VK_SPACE:  // space down is identical to left button down
		xpDoc->m_xpGtlData->AcceptClick(this, m_cCurrentPosition, CLICK_LUP) ;
		break ;

	case VK_ADD:    // keypad plus same as double click
		xpDoc->m_xpGtlData->AcceptClick(this, m_cCurrentPosition, CLICK_LDOUBLE) ;
		break ;

	case VK_DELETE:     // delete key
		xpDoc->m_xpGtlData->AcceptDeleteKey(this) ;
		break ;

	default:
		break ;
	}
	// TODO: Add your message handler code here and/or call default

	CView::OnKeyUp(nChar, nRepCnt, nFlags);
	JXELEAVE(CGtlView::OnKeyUp) ;
}

void CGtlView::OnMouseMove(UINT nFlags, CPoint cMousePoint) {
	int iError = 0 ;

	m_cCurrentPosition = cMousePoint ;
	// save current mouse position


	CGtlDoc* xpDoc = GetDocument() ;

	if (xpDoc)
		xpDoc->m_xpcLastMouseView = this ;

	if (m_xpFrame)
		m_xpFrame->m_xpcLastMouseView = this ;


//  UpdateDialogs() ;

	// TODO: Add your message handler code here and/or call default

	if (nFlags & MK_LBUTTON)
		xpDoc->m_xpGtlData->AcceptClick(this, cMousePoint, CLICK_LMOVE);

	else if (nFlags & MK_RBUTTON)
		xpDoc->m_xpGtlData->AcceptClick(this, cMousePoint, CLICK_RMOVE);

	else
		xpDoc->m_xpGtlData->AcceptClick(this, cMousePoint, CLICK_UMOVE);

	CView::OnMouseMove(nFlags, cMousePoint);
}

void CGtlView::OnLButtonDblClk(UINT nFlags, CPoint cMousePoint) {
	JXENTER(CGtlView::OnLButtonDblClk) ;
	int iError = 0 ;
	// TODO: Add your message handler code here and/or call default
	CGtlDoc* xpDoc = GetDocument() ;
	xpDoc->m_xpGtlData->AcceptClick(this,
	                                cMousePoint, CLICK_LDOUBLE) ;

	CView::OnLButtonDblClk(nFlags, cMousePoint);
	JXELEAVE(CGtlView::OnLButtonDblClk) ;
}
#endif

void CGtlView::OnLButtonDown(UINT nFlags, CPoint cMousePoint) {
	CGtlDoc* xpDoc = GetDocument() ;

	xpDoc->m_xpGtlData->AcceptClick(this, cMousePoint, CLICK_LDOWN) ;

	if (bExitMetaDLL)
		return;

	CView::OnLButtonDown(nFlags, cMousePoint);
}

void CGtlView::OnLButtonUp(UINT nFlags, CPoint cMousePoint) {
	CGtlDoc* xpDoc = GetDocument() ;

	xpDoc->m_xpGtlData->AcceptClick(this, cMousePoint, CLICK_LUP) ;

	if (m_xpFrame && !m_xpFrame->m_bExitDll)
		CView::OnLButtonUp(nFlags, cMousePoint);
}

#ifdef NODEEDIT
void CGtlView::OnRButtonDblClk(UINT nFlags, CPoint cMousePoint) {
	JXENTER(CGtlView::OnRButtonDblClk) ;
	int iError = 0 ;

	HWND hParentWnd = ::GetParent(m_hWnd) ;
	DWORD dwStyle = ::GetWindowLong(hParentWnd, GWL_STYLE) ;

	dwStyle |= WS_CLIPCHILDREN ;
	::SetWindowLong(hParentWnd, GWL_STYLE, dwStyle) ;

	// TODO: Add your message handler code here and/or call default
	CGtlDoc* xpDoc = GetDocument() ;
	xpDoc->m_xpGtlData->AcceptClick(this, cMousePoint, CLICK_RDOUBLE) ;

	CView::OnRButtonDblClk(nFlags, cMousePoint);
	JXELEAVE(CGtlView::OnRButtonDblClk) ;
}
#endif

void CGtlView::OnRButtonDown(UINT nFlags, CPoint cMousePoint) {
	CGtlDoc* xpDoc = GetDocument() ;
	xpDoc->m_xpGtlData->AcceptClick(this, cMousePoint, CLICK_RDOWN) ;

	CView::OnRButtonDown(nFlags, cMousePoint);
}

void CGtlView::OnRButtonUp(UINT nFlags, CPoint cMousePoint) {
	CGtlDoc* xpDoc = GetDocument() ;
	xpDoc->m_xpGtlData->AcceptClick(this, cMousePoint, CLICK_RUP) ;

	CView::OnRButtonUp(nFlags, cMousePoint);
}

#ifdef NODEEDIT

void CGtlView::OnSize(UINT nType, int cx, int cy) {
	JXENTER(CGtlView::OnSize) ;
	int iError = 0 ;
	CGtlDoc* xpDoc = GetDocument() ;

	if (m_bBsuInit && xpDoc->m_xpGtlData) {
		xpDoc->m_xpGtlData->m_bChangeSize = TRUE ;
		xpDoc->m_xpGtlData->SpecifyUpdate(this) ;
	}

	UpdateDialogs();

	CView::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here

	JXELEAVE(CGtlView::OnSize) ;
}


void CGtlView::OnSetFocus(CWnd *pOldWnd) {
	CGtlDoc* xpDoc = GetDocument() ;

	if (xpDoc) {
		xpDoc->m_xpcLastFocusView = this ;
		xpDoc->FixChecks() ;
	}

	if (m_xpFrame)
		m_xpFrame->m_xpcLastFocusView = this ;

	UpdateDialogs() ;

	CView::OnSetFocus(pOldWnd);
}


void CGtlView::UpdateDialogs(void) {
	JXENTER(CGtlView::UpdateDialogs) ;
	int iError = 0 ;
	CGtlDoc* xpDoc = GetDocument() ;

	if (xpDoc->m_xpGtlData)
		xpDoc->m_xpGtlData->UpdateDialogs(FALSE) ;

	JXELEAVE(CGtlView::UpdateDialogs) ;
}

void CGtlView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {
	JXENTER(CGtlView::OnHScroll) ;
	int iError = 0 ;
	// TODO: Add your message handler code here and/or call default

	#if NEWSCROLL
	m_cViewBsuSet.OnScroll(nSBCode, nPos, pScrollBar, BSCT_HORZ) ;
	#else
	MFC_VIEW::OnHScroll(nSBCode, nPos, pScrollBar);
	#endif
	JXELEAVE(CGtlView::OnHScroll) ;
}

void CGtlView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {
	JXENTER(CGtlView::OnVScroll) ;
	int iError = 0 ;
	// TODO: Add your message handler code here and/or call default
	#if NEWSCROLL
	m_cViewBsuSet.OnScroll(nSBCode, nPos, pScrollBar, BSCT_VERT) ;
	#else

	MFC_VIEW::OnVScroll(nSBCode, nPos, pScrollBar);
	#endif
	JXELEAVE(CGtlView::OnVScroll) ;
}
#endif


long CGtlView::OnMCINotify(WPARAM wParam, LPARAM lParam) {
	CSound  *pSound;

	pSound = CSound::OnMCIStopped(wParam, lParam);
	if (pSound != nullptr)
		OnSoundNotify(pSound);
	return (0L);
}


long CGtlView::OnMMIONotify(WPARAM wParam, LPARAM lParam) {
	CSound  *pSound;

	pSound = CSound::OnMMIOStopped(wParam, lParam);
	if (pSound != nullptr)
		OnSoundNotify(pSound);
	return (0L);
}


void CGtlView::OnSoundNotify(CSound *) {
	//
	// Add your code to process explicit notification of a sound "done" event here.
	// pSound is a pointer to a CSound object for which you requested SOUND_NOTIFY.
	//
}


void CGtlView::FlushInputEvents(void) {
	MSG msg;

	while (TRUE) {                      // find and remove all keyboard events
		if (!PeekMessage(&msg, m_hWnd, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
			break;
	}

	while (TRUE) {                      // find and remove all mouse events
		if (!PeekMessage(&msg, m_hWnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
			break;
	}
}


extern CBgbMgr *gpBgbMgr;

void CGtlView::OnTimer(UINT nEventId) {
	// there can only be one animation timer going
	assert(nEventId == ANIMATION_TIMER_ID);

	if (gpBgbMgr->AnimationsActive()) {

		gpBgbMgr->DoAnimations();
	}
}


BOOL CGtlView::OnSetCursor(CWnd *pWnd, UINT /*nHitTest*/, UINT /*message*/) {
	if ((*pWnd).m_hWnd == (*this).m_hWnd)
		return (TRUE);
	else
		return (FALSE);
}

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
