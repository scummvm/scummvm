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
#include "bagel/hodjnpodj/hnplibs/rules.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

///DEFS gtlview.h

extern HWND     ghwndParent;
extern CBfcMgr  *lpMetaGameStruct;
extern bool     bExitMetaDLL;
extern CBgbMgr  *gpBgbMgr;


CGtlView *gpMyView;

/////////////////////////////////////////////////////////////////////////////
// CGtlView

IMPLEMENT_DYNCREATE(CGtlView, MFC_VIEW)

BEGIN_MESSAGE_MAP(CGtlView, MFC_VIEW)
	//{{AFX_MSG_MAP(CGtlView)
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

//* CGtlView::OnUpdate -- called when document changes to update view
void CGtlView::OnUpdate(CView *xpSender, LPARAM lHint, CObject *xpHint)
// returns: void
{
//    dbgtrc = true ;   // debugging code

	JXENTER(CGtlView::OnUpdate) ;
	CGtlHint *xpcGtlHint = (CGtlHint *)xpHint ;
	//bool bDone = false ;    // true when hint is fully identified

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
			//bDone = true ;
		}
		break ;

	case HINT_UPDATE_FULL:
		xpGtlData->Draw(this, nullptr);
		//bDone = true ;
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
			xpDoc->m_xpGtlData->m_bChangeSize = true;
		break;

	default:

		//xpGtlData->Draw(this, nullptr);
		xpGtlData->m_bStartMetaGame = false;
		xpGtlData->_metaGame = xpGtlData->m_bInitMetaGame = true;
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

//  A view's OnInitialUpdate() overrideable function is called immediately
//  after the frame window is created, and the view within the frame
//  window is attached to its document.  This provides the scroll view the
//  opportunity to set its size (m_totalSize) based on the document size.


//* CGtlView::OnInitialUpdate --
void CGtlView::OnInitialUpdate() {
	JXENTER(CGtlView::OnInitialUpdate) ;
	CGtlDoc *xpDoc = GetDocument() ;

	#if NEWSCROLL
//  m_cViewBsuSet.InitWndBsuSet(this, true) ;
	m_cViewBsuSet.InitWndBsuSet(this, SCROLLVIEW, false) ;
	m_bBsuInit = true ;
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
bool CGtlView::PreCreateWindow(CREATESTRUCT& cCs)
// returns: value returned by base class function
{
	JXENTER(CGtlView::PreCreateWindow) ;
	bool bRetval ;      // return value
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
// CGtlView message handlers

bool CGtlView::OnEraseBkgnd(CDC *) {
	return true ; // this tells MFC that the background has been erased
}

void CGtlView::OnSysKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
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

void CGtlView::OnKeyDown(unsigned int nChar, unsigned int nRepCnt, unsigned int nFlags) {
	JXENTER(CGtlView::OnKeyDown) ;
	CGtlDoc *xpDoc = GetDocument() ;
	bool    bJunk, bAnimations;
	CPoint  ptEdge(0, 0);
	CPoint  ptAdjustedEdge(0, 0);

	switch (nChar) {

	case VK_F1: {
		CRules RulesDlg(this, "metarule.txt", gpBgbMgr->m_xpGamePalette, nullptr);
		if ((bAnimations = gpBgbMgr->AnimationsActive()) != false)
			gpBgbMgr->PauseAnimations();
		gpBgbMgr->CacheOptimize(2000000);
		RulesDlg.DoModal();
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
			xpDoc->m_xpcLastFocusView->m_cViewBsuSet.EdgeToCenter(xpDoc->m_xpGtlData->m_xpXodjChain->m_lpcCharSprite->m_crPosition, true);
		else
			xpDoc->m_xpcLastFocusView->m_cViewBsuSet.EdgeToCenter(xpDoc->m_xpGtlData->m_xpXodjChain->m_xpXodjNext->m_lpcCharSprite->m_crPosition, true);
		break;

	case VK_P:
		if (xpDoc->m_xpGtlData->m_xpXodjChain->m_bHodj)
			xpDoc->m_xpcLastFocusView->m_cViewBsuSet.EdgeToCenter(xpDoc->m_xpGtlData->m_xpXodjChain->m_xpXodjNext->m_lpcCharSprite->m_crPosition, true);
		else
			xpDoc->m_xpcLastFocusView->m_cViewBsuSet.EdgeToCenter(xpDoc->m_xpGtlData->m_xpXodjChain->m_lpcCharSprite->m_crPosition, true);
		break;

	/*
	        case VK_SPACE:  // space down is identical to left button down
	            xpDoc->m_xpGtlData->AcceptClick(this,m_cCurrentPosition, CLICK_LDOWN) ;
	            break;
	*/

	case VK_SPACE:
	case VK_RETURN:
		xpDoc->m_xpcLastFocusView->m_cViewBsuSet.EdgeToCenter(xpDoc->m_xpGtlData->m_xpCurXodj->m_lpcCharSprite->m_crPosition, true);
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

	default:
		break;
	}

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
	JXELEAVE(CGtlView::OnKeyDown) ;
}

void CGtlView::OnLButtonDown(unsigned int nFlags, CPoint cMousePoint) {
	CGtlDoc* xpDoc = GetDocument() ;

	xpDoc->m_xpGtlData->AcceptClick(this, cMousePoint, CLICK_LDOWN) ;

	if (bExitMetaDLL)
		return;

	CView::OnLButtonDown(nFlags, cMousePoint);
}

void CGtlView::OnLButtonUp(unsigned int nFlags, CPoint cMousePoint) {
	CGtlDoc* xpDoc = GetDocument() ;

	xpDoc->m_xpGtlData->AcceptClick(this, cMousePoint, CLICK_LUP) ;

	if (m_xpFrame && !m_xpFrame->m_bExitDll)
		CView::OnLButtonUp(nFlags, cMousePoint);
}

void CGtlView::OnRButtonDown(unsigned int nFlags, CPoint cMousePoint) {
	CGtlDoc* xpDoc = GetDocument() ;
	xpDoc->m_xpGtlData->AcceptClick(this, cMousePoint, CLICK_RDOWN) ;

	CView::OnRButtonDown(nFlags, cMousePoint);
}

void CGtlView::OnRButtonUp(unsigned int nFlags, CPoint cMousePoint) {
	CGtlDoc* xpDoc = GetDocument() ;
	xpDoc->m_xpGtlData->AcceptClick(this, cMousePoint, CLICK_RUP) ;

	CView::OnRButtonUp(nFlags, cMousePoint);
}

LRESULT CGtlView::OnMCINotify(WPARAM wParam, LPARAM lParam) {
	CSound  *pSound;

	pSound = CSound::OnMCIStopped(wParam, lParam);
	if (pSound != nullptr)
		OnSoundNotify(pSound);
	return 0;
}


LRESULT CGtlView::OnMMIONotify(WPARAM wParam, LPARAM lParam) {
	CSound  *pSound;

	pSound = CSound::OnMMIOStopped(wParam, lParam);
	if (pSound != nullptr)
		OnSoundNotify(pSound);
	return 0;
}


void CGtlView::OnSoundNotify(CSound *) {
	//
	// Add your code to process explicit notification of a sound "done" event here.
	// pSound is a pointer to a CSound object for which you requested SOUND_NOTIFY.
	//
}


void CGtlView::FlushInputEvents() {
	MSG msg;

	while (true) {                      // find and remove all keyboard events
		if (!PeekMessage(&msg, m_hWnd, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
			break;
	}

	while (true) {                      // find and remove all mouse events
		if (!PeekMessage(&msg, m_hWnd, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
			break;
	}
}


extern CBgbMgr *gpBgbMgr;

void CGtlView::OnTimer(uintptr nEventID) {
	// there can only be one animation timer going
	assert(nEventID == ANIMATION_TIMER_ID);

	if (gpBgbMgr->AnimationsActive()) {

		gpBgbMgr->DoAnimations();
	}
}


bool CGtlView::OnSetCursor(CWnd *pWnd, unsigned int /*nHitTest*/, unsigned int /*message*/) {
	if ((*pWnd).m_hWnd == (*this).m_hWnd)
		return true;
	else
		return false;
}

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
