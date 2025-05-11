// gtlui.cpp -- data interface to Windows
// Written by John J. Xenakis for Boffo Games Inc., 1994

#include "stdafx.h"


#include "bagel/hodjnpodj/hnplibs/sprite.h"

#include "gtl.h"
#include "gtldoc.h"
#include "gtlview.h"
#include "gtlfrm.h"
#include "bgen.h"
#include "gtldat.h"

#define	STARTING_LOCATION	"Castle"

#define HODJS_WALKING_SOUND ".\\SOUND\\HODJST.MID"
#define HODJ_SOUND_START    2000
#define HODJ_SOUND_END      63000
#define HODJS_STARS_FILE    ".\\ART\\HSTAR.BMP"

#define PODJS_WALKING_SOUND ".\\SOUND\\PODJST.MID"
#define PODJ_SOUND_START    1000
#define PODJ_SOUND_END      64000
#define PODJS_STARS_FILE    ".\\ART\\PSTAR.BMP"

extern DWORD GetPhysicalMemory(void);


CNode::CNode(void)
{
    // the node table must stay under 32k
    assert((UINT)sizeof(CNode) * MAX_NODES < (UINT)0x7FFF);

#ifdef NODEEDIT
    memset(&m_szSector, 0, sizeof(char) * MAX_LABEL_LENGTH);
#endif
    memset(&m_szLabel, 0, sizeof(char) * MAX_LABEL_LENGTH);

    memset(&m_iLinks, 0, sizeof(int) * MAX_LINKS_PER_NODE);
    m_iLocationCode = 0;
    m_iSector = 0;
    m_iBitmap = 0;
    m_iX = 0;
    m_iY = 0;
    m_iNumLinks = 0;
    m_bDeleted = FALSE;
    m_bSelected = FALSE;
    m_bRelative = FALSE;
    m_bRelocatable = FALSE;
    m_bWgtSpec = FALSE;
    m_bSenSpec = FALSE;
    m_bMenu = FALSE;
    m_iWeight = 0;
    m_iSensitivity = 0;
}

//* CGtlData::CGtlData -- constructor -- zero out all fields, allocate
//      arrays, and set colors
CGtlData::CGtlData(void)
{
    JXENTER(CGtlData::CGtlData) ;
    int iError = 0 ;            // error code

    TRACECONSTRUCTOR(CGtlData) ;
    ClearData() ;
    m_cBbtMgr.m_lpBgbMgr = &m_cBgbMgr ;
    m_cMiniButton.m_iBbtId = BUTTON_MINIMAP ;
    m_cInvButton.m_iBbtId = BUTTON_INVENTORY ;
    m_cScrollButton.m_iBbtId = BUTTON_SCROLL ;

    m_lpMaps = new FAR CMap[MAX_BITMAPS] ; // allocate bitmap array
    m_lpNodes = new FAR CNode[MAX_NODES] ;     // allocate node array
    SetColors() ;       // set color fields
    m_bGtlDll = GTLDLL ;

    CreateOffScreenBmp();

// cleanup:

    JXELEAVE(CGtlData::CGtlData) ;
    RETURN_VOID ;
}

//* CGtlData::~CGtlData -- destructor
CGtlData::~CGtlData(void)
{
    JXENTER(CGtlData::~CGtlData) ;
    int iError = 0 ;            // error code
    CXodj *xpXodj, *xpXodjNext ;

    DeleteOffScreenBmp();

    TRACEDESTRUCTOR(CGtlData) ;
    delete [] m_lpMaps ;
    delete [] m_lpNodes ;

    // delete Xodj chain
    //
    for (xpXodj = m_xpXodjChain ; xpXodj ; xpXodj = xpXodjNext) {

        xpXodjNext = xpXodj->m_xpXodjNext ;
        delete xpXodj ;
    }


// cleanup:

    JXELEAVE(CGtlData::~CGtlData) ;
    RETURN_VOID ;
}

VOID CGtlData::CreateOffScreenBmp(VOID)
{
    CDC *pScreenDC;
    CGtlApp *xpGtlApp = (CGtlApp *)AfxGetApp();

    if (m_pOffScreenBmp == NULL) {

        if ((m_cBgbMgr.m_xpGamePalette != NULL) &&
        	(GetFreeSpace(0) >= (ULONG)250000) && (GlobalCompact(0) >= (ULONG)500000)) {

            if ((m_pOffScreenBmp = new CBitmap) != NULL) {

                pScreenDC = m_xpGtlView->GetDC();
                if (m_pOffScreenBmp->CreateCompatibleBitmap(pScreenDC, xpGtlApp->m_iWidth, xpGtlApp->m_iHeight) == FALSE) {

                    delete m_pOffScreenBmp;
                    m_pOffScreenBmp = NULL;
                }
                m_xpGtlView->ReleaseDC(pScreenDC);
            }
        }
    }
}

VOID CGtlData::DeleteOffScreenBmp(VOID)
{
    if (m_pOffScreenBmp != NULL) {
        delete m_pOffScreenBmp;
        m_pOffScreenBmp = NULL;
    }
}

VOID CGtlData::PaintOffScreenBmp(VOID)
{
    CDC *pScreenDC;
    CMemDC *pMemDC;
    CPalette *pScreenPalOld;
    CGtlApp *xpGtlApp = (CGtlApp *)AfxGetApp();

    if (m_pOffScreenBmp != NULL) {

        if ((pMemDC = GetMemDC()) != NULL) {

            if ((pScreenDC = m_xpGtlView->GetDC()) != NULL) {
                pScreenPalOld = pScreenDC->SelectPalette(m_cBgbMgr.m_xpGamePalette, FALSE);
                pScreenDC->BitBlt(0, 0, xpGtlApp->m_iWidth, xpGtlApp->m_iHeight, pMemDC->m_pDC, 0, 0, SRCCOPY);
                pScreenDC->SelectPalette(pScreenPalOld, FALSE);
                m_xpGtlView->ReleaseDC(pScreenDC);
            }
            ReleaseMemDC(pMemDC);
        }
    }
}
CMemDC::CMemDC(VOID)
{
    m_pDC = new CDC;
    assert(m_pDC != NULL);
}

CMemDC::~CMemDC(VOID)
{
    if (m_pDC != NULL) {
        delete m_pDC;
        m_pDC = NULL;
    }
}

CMemDC *CGtlData::GetMemDC(VOID)
{
    CMemDC *pMemDC;

    if ((pMemDC = new CMemDC) != NULL) {
        pMemDC->m_pDC->CreateCompatibleDC(NULL);
        assert(m_cBgbMgr.m_xpGamePalette != NULL);
        pMemDC->m_pPalOld = pMemDC->m_pDC->SelectPalette(m_cBgbMgr.m_xpGamePalette, FALSE);
        pMemDC->m_pDC->RealizePalette();
        pMemDC->m_hBmpOld = SelectBitmap(pMemDC->m_pDC->m_hDC, m_pOffScreenBmp->m_hObject);
    }
    assert(pMemDC != NULL);

    return(pMemDC);
}

VOID CGtlData::ReleaseMemDC(CMemDC *pMemDC)
{
    assert(pMemDC != NULL);

    if (pMemDC != NULL) {
        SelectBitmap(pMemDC->m_pDC->m_hDC, pMemDC->m_hBmpOld);
        pMemDC->m_pDC->SelectPalette(pMemDC->m_pPalOld, FALSE);
        delete pMemDC;
    }
}

//* CGtlData::SetColors -- set colors of various elements
BOOL CGtlData::SetColors(void)
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::SetColors) ;
    int iError = 0 ;            // error code

    m_cNodePenColor = m_cNodeBrushColor = RGBCOLOR_RED ;
    m_cSelNodePenColor = RGBCOLOR_MAGENTA ;
    m_cSelNodeBrushColor = RGBCOLOR_GREEN ;

    m_cNodeSensitivityColor = m_cNodePenColor ;
    m_cNodePassThruColor = RGBCOLOR_WHITE ;
    m_cNodeHeavyColor = RGBCOLOR_BLACK ;

    m_cLinkPenColor = RGBCOLOR_BLUE ;
    m_cSelLinkPenColor = RGBCOLOR_YELLOW ;
    m_cBackColor = RGBCOLOR_GREEN ;

// cleanup:

    JXELEAVE(CGtlData::SetColors) ;
    RETURN(iError != 0) ;
}

//* CGtlData::ClearInhibitDraw -- clear m_bInhibitDraw flag
BOOL CGtlData::ClearInhibitDraw(void)
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::ClearInhibitDraw) ;
    int iError = 0 ;		// error code
    CGtlView * xpGtlView = (CGtlView *)m_cBgbMgr.m_xpcView ;

    if (m_bInhibitDraw) {
        m_bInhibitDraw = FALSE ;
        if (xpGtlView)
            xpGtlView->Invalidate(FALSE) ;   // repaint window
    }

// cleanup:

    JXELEAVE(CGtlData::ClearInhibitDraw) ;
    RETURN(iError != 0) ;
}

//* CGtlData::Draw -- draw data
BOOL CGtlData::Draw(CGtlView *xpGtlView, CRect *xpClipRect, CDC *xpDc)
// xpGtlView -- CGtlView object
// xpClipRect -- clipping rectangle
// xpDc -- device context (provided only for WM_PAINT)
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::Draw) ;
    int iError = 0 ;            // error code
    CGtlApp * xpGtlApp = (CGtlApp *)AfxGetApp() ; // get application
    CRPoint crPoint1, crPoint2 ;        // link points
    CRPoint crPosition ;                // position within window
    BOOL bPaint = (xpDc != NULL) ;      // test for WM_PAINT
    CMemDC *pMemDC;
    CDC *pOldDC;

#ifdef NODEEDIT
    CNode *lpNode, *lpTargetNode;
    int iK, iL ;                // loop variables
    int iLink ;
    int iRadius ;
    BOOL bSelected ;            // flag: link is selected
#endif

    // rectangles for clip region,
    // destination area, and bitmap
    CRRect cClipRect, cDevClipRect, cDestRect, cBmpRect;

    // avoid recursion
    assert(!m_bDrawing);

    m_bDrawing = TRUE ;
    m_xpGtlView = xpGtlView ;
    m_bPaint = bPaint ;

    m_cBgbMgr.InitDc(xpGtlView, &xpGtlView->m_cViewBsuSet, xpDc) ;
    m_cBgbMgr.AdjustLockCount(+1) ;

    AdjustToView(xpGtlView);
    xpDc = m_cBgbMgr.m_xpDc ;

    // Get the invalidated rectangle of the view, or in the case
    // of printing, the clipping region of the printer dc.
    // We do everything in logical (not display) units -- see note to
    // PaintDIB call below.
    xpGtlView->m_cViewBsuSet.PrepareDc(xpDc, !bPaint) ;
    m_bRelocatable = !bPaint ;  // relocatable status of DC

    if (bPaint) {
        xpClipRect = &cClipRect ;
        xpDc->GetClipBox(&cDevClipRect) ; // clip rectangle in device
                                        // viewport coordinates
        *xpClipRect = cDevClipRect ;
        xpDc->DPtoLP(xpClipRect) ;

    } else {
        if (xpClipRect) {

            cDevClipRect = *xpClipRect ;
            xpDc->LPtoDP(&cDevClipRect) ;

        } else {

            xpGtlView->GetClientRect(&cDevClipRect) ;
            xpClipRect = &cClipRect ;
            *xpClipRect = cDevClipRect ;
            xpDc->DPtoLP(xpClipRect) ;
        }
    }

    m_xpClipRect = xpClipRect ; // save relocatable clipping rectangle
    m_xpDevClipRect = &cDevClipRect ;   // save device clipping rectangle

#ifdef NODEEDIT
    
    /// ***** delete the following later
    SwitchDc(xpDc, FALSE) ;     // make dev context non-relocatable

    if (m_bPaintBackground) {

        SwitchDc(xpDc, FALSE) ; // make dev context non-relocatable
        m_cBgbMgr.SetBrush(m_cBackColor) ;      // set device
                        // context brush to background color
        m_cBgbMgr.SetPen(m_cBackColor) ;        // and pen
        xpDc->Rectangle(&cDevClipRect) ;        // paint a rectangle
    }

    SwitchDc(xpDc, TRUE) ;      // make device constant relocatable
#endif

    if (!m_bInhibitDraw) {

        pMemDC = NULL;
        pOldDC = NULL;
        if (m_pOffScreenBmp == NULL) {
            CreateOffScreenBmp();
        }
        if (m_pOffScreenBmp != NULL) {
            pOldDC = m_cBgbMgr.m_xpDc;
            pMemDC = GetMemDC();
            xpDc = m_cBgbMgr.m_xpDc = pMemDC->m_pDC;
        }

        if (m_bPaint)
            CSprite::ClearBackgrounds();

        DrawBitmaps(xpDc, FALSE) ;  // draw non-overlay bitmaps

#ifdef NODEEDIT

        // draw all the links
        for (iK = 0 ; m_bShowNodes && iK < m_iNodes ; ++iK) {

            if (!(lpNode = m_lpNodes + iK)->m_bDeleted) {

                crPoint1 = NodeToPoint(lpNode) ;    // first point
                for (iL = 0 ; iL < lpNode->m_iNumLinks ; ++iL) {

                    // only paint links to previous nodes
                    if ( (iLink = lpNode->m_iLinks[iL]) < iK) {

                        lpTargetNode = m_lpNodes + iLink ;
                        crPoint2 = NodeToPoint(lpTargetNode) ;
                        cDestRect = CRect(crPoint1.x, crPoint1.y, crPoint2.x, crPoint2.y) ;
                        CGenUtil::NormalizeCRect(cDestRect) ;
                        xpDc->LPtoDP(&cDestRect) ;

                        if (cDestRect.IntersectRect(&cDestRect, &cDevClipRect)) {

                            // set cDestRect to the intersection,
                            // return true
                            // if result is not null rectangle

                            SwitchDc(xpDc, crPoint1.IfRelocatable()) ;
                            bSelected = m_bSelectedLink && iLink == m_iSelLinkPt1 && iK == m_iSelLinkPt2;

                            if (bSelected)
                                m_cBgbMgr.SetPen(m_cSelLinkPenColor, 4) ;
                            else
                                m_cBgbMgr.SetPen(m_cLinkPenColor, 4) ;

                            xpDc->MoveTo(crPoint1) ;
                            xpDc->LineTo(crPoint2) ;
                        }
                    }
                }
            }
        }

        // draw the nodes
        for (iK = 0 ; m_bShowNodes && iK < m_iNodes ; ++iK) {

            if (!(lpNode = m_lpNodes + iK)->m_bDeleted) {
                crPosition = NodeToPoint(lpNode) ;  // get node position
                iRadius = NODERADIUS ;
                if (iRadius < lpNode->m_iSensitivity)
                    iRadius = lpNode->m_iSensitivity ;
                cDestRect = CRect(crPosition.x - iRadius, crPosition.y - iRadius, crPosition.x + iRadius, crPosition.y + iRadius);

                xpDc->LPtoDP(&cDestRect) ; // logical to display units
                if (cDestRect.IntersectRect(&cDestRect, &cDevClipRect)) {
                        // set cDestRect to the intersection, return true
                        // if result is not null rectangle

                    SwitchDc(xpDc, crPosition.IfRelocatable()) ;

                    if (lpNode->m_bSelected) {
                        m_cBgbMgr.SetPen(m_cSelNodePenColor) ;
                        m_cBgbMgr.SetBrush(m_cSelNodeBrushColor) ;

                    } else {           // not selected

                        m_cBgbMgr.SetPen(m_cNodePenColor) ;
                        if (!lpNode->m_iWeight)     // passthru node
                            m_cBgbMgr.SetBrush(m_cNodePassThruColor) ;
                        else if (lpNode->m_iWeight > 1)     // heavy node
                            m_cBgbMgr.SetBrush(m_cNodeHeavyColor) ;
                        else
                            m_cBgbMgr.SetBrush(m_cNodeBrushColor) ;
                    }

                    cDestRect = CRect(crPosition.x - NODERADIUS,
                                    crPosition.y - NODERADIUS,
                                    crPosition.x + NODERADIUS,
                                    crPosition.y + NODERADIUS) ;
                    xpDc->Ellipse(&cDestRect) ;     // draw circle

                    if ((lpNode->m_bSelected || m_bShowSensitivity)
                                && lpNode->m_iWeight
                                && lpNode->m_iSensitivity > NODERADIUS)
                    {
                        m_cBgbMgr.SetBrush(0, BS_NULL) ;
                        m_cBgbMgr.SetPen(m_cNodeSensitivityColor) ;
                        cDestRect = CRect(
                                    crPosition.x - lpNode->m_iSensitivity,
                                    crPosition.y - lpNode->m_iSensitivity,
                                    crPosition.x + lpNode->m_iSensitivity,
                                    crPosition.y + lpNode->m_iSensitivity) ;
                        xpDc->Ellipse(&cDestRect) ; // draw circle
                    }
                }
            }
        }
#endif

        // paint sprites and moveable overlays
        DrawBitmaps(xpDc, TRUE) ;   // draw non-overlay bitmaps

        if (m_pOffScreenBmp != NULL) {
            ReleaseMemDC(pMemDC);
            PaintOffScreenBmp();

            m_cBgbMgr.m_xpDc = pOldDC;
        }

        m_cBgbMgr.ResumeAnimations();
    }

// cleanup:

    m_cBgbMgr.AdjustLockCount(-1) ;
    m_cBgbMgr.ReleaseDc() ;

#ifdef NODEEDIT
    UpdateDialogs();
#endif

    m_xpClipRect = m_xpDevClipRect = NULL ;

    m_bDrawing = FALSE ;

//exit:

    JXELEAVE(CGtlData::Draw) ;
    RETURN(iError != 0) ;
}

//* CGtlData::SwitchDc -- switch between relocatable/nonrelocatable
//              device context
BOOL CGtlData::SwitchDc(CDC *pDC, BOOL bRelocatable)
// pDC -- pointer to device context
// bRelocatable -- (non)relocatable flag
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::SwitchDc) ;
    int iError = 0 ;            // error code

    if (bRelocatable && !m_bRelocatable) {

        m_xpGtlView->m_cViewBsuSet.PrepareDc(pDC, TRUE) ;
        if (!m_bPaint && m_xpClipRect)
            pDC->IntersectClipRect(m_xpClipRect) ;
        m_bRelocatable = TRUE ;

    } else if (!bRelocatable && m_bRelocatable) {

        m_xpGtlView->m_cViewBsuSet.PrepareDc(pDC, FALSE) ;
        if (m_xpDevClipRect)
            pDC->IntersectClipRect(m_xpDevClipRect) ;
        m_bRelocatable = FALSE ;
    }

// cleanup:

    JXELEAVE(CGtlData::SwitchDc) ;
    RETURN(iError != 0) ;
}


//* CGtlData::DrawBitmaps -- draw bitmaps for Draw routine
BOOL CGtlData::DrawBitmaps(CDC *pDC, BOOL bOverlay)
// pDC -- device context
// bOverlay -- if FALSE, draw base bitmaps; if TRUE, draw overlays
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::DrawBitmaps) ;
    int iError = 0 ;            // error code
    int iK ;                    // loop variable
    CMap FAR * lpMap ;          // pointer to bitmap
    CBgbObject FAR * lpcBgbObject ;     // boffo graphic object
    CXodj * xpXodj ;    // hodj/podj character

    m_cBgbMgr.PauseAnimations();

    // paint sprites and moveable overlays
    for (iK = 0 ; iK < m_iMaps ; ++iK) {

        if ((iK & 0x17) == 0)
			CSound::HandleMessages();

        lpMap = m_lpMaps + iK ;

        if (!lpMap->m_bSpecialPaint && EQV(lpMap->m_bOverlay, bOverlay) && (m_bMetaGame || !lpMap->m_bMetaGame) && ((lpcBgbObject = lpMap->m_lpcBgbObject) != NULL))
            DrawABitmap(pDC, lpcBgbObject, TRUE);
    }

    m_cBgbMgr.ResumeAnimations();

    if (bOverlay) {

        // draw the hodj/podj bitmaps
        for (xpXodj = m_xpXodjChain ; xpXodj ; xpXodj = xpXodj->m_xpXodjNext)
            DrawABitmap(pDC, xpXodj->m_lpcCharSprite, TRUE);

        // draw the correct hodj/podj icon
        if (m_xpCurXodj)
            DrawABitmap(pDC, m_xpCurXodj->m_lpcIcon, TRUE);

        // draw buttons in current correct position
        DrawABitmap(pDC, m_cBbtMgr.GetCurrentBitmap(&m_cMiniButton), TRUE);
        DrawABitmap(pDC, m_cBbtMgr.GetCurrentBitmap(&m_cInvButton), TRUE);
        DrawABitmap(pDC, m_cBbtMgr.GetCurrentBitmap(&m_cScrollButton), TRUE);

        // draw number of furlongs
        if ((xpXodj = m_xpCurXodj) != NULL) {

            int iNumFurlongs = xpXodj->m_iFurlongs ;

            if (iNumFurlongs < 0)
                iNumFurlongs = 0 ;
            if (iNumFurlongs > 24)
                iNumFurlongs = 24 ;
            if (((lpMap = m_lpFurlongMaps[iNumFurlongs]) != NULL) && ((lpcBgbObject = lpMap->m_lpcBgbObject) != NULL))
                DrawABitmap(pDC, lpcBgbObject, TRUE);
        }
    }

// cleanup:

    JXELEAVE(CGtlData::DrawBitmaps) ;
    RETURN(iError != 0) ;
}


//* CGtlData::DrawABitmap -- draw a bitmap object for Draw routine
BOOL CGtlData::DrawABitmap(CDC *pDC, CBgbObject *pBgbObject, BOOL bPaint)
// pDC -- device context
// pBgbObject -- pointer to graphics object to draw
// bPaint -- if TRUE, called from WM_PAINT (Draw routine)
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::DrawABitmap) ;
    int iError = 0 ;            // error code
    CRRect crDestRect ; // relocatable destination rectangle
    BOOL bDrawBitmap = !bPaint ;

    assert(pBgbObject != NULL);

    if (!pDC) {

        m_cBgbMgr.ReInitDc() ;
        pDC = m_cBgbMgr.m_xpDc ;
    }

    if (pBgbObject) {

        SwitchDc(pDC, pBgbObject->IfRelocatable()) ;
        if (!bDrawBitmap) {

            crDestRect = pBgbObject->GetRect() ;
            pDC->LPtoDP(&crDestRect) ;

            // set crDestRect to the intersection, return true
            // if result is not null rectangle
            if (!m_xpDevClipRect || crDestRect.IntersectRect(&crDestRect, m_xpDevClipRect))
                bDrawBitmap = TRUE ;
        }
        if (bDrawBitmap) {
            m_cBgbMgr.PaintBitmapObject(pBgbObject, bPaint);
        }
    }

// cleanup:

    JXELEAVE(CGtlData::DrawABitmap) ;
    RETURN(iError != 0) ;
}


//* CGtlData::NodeToPoint -- return relocatable coordinates of node
//              adjusted by optional size object
CRPoint CGtlData::NodeToPoint(CNode *lpNode, CSize FAR *lpcSize)
// lpNode -- pointer to node
// lpcSize -- pointer to size of bitmap sitting on node
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::NodeToPoint) ;
    int iError = 0 ;            // error code
    CRPoint crPosition ;                // output: position
    CMap * lpMap ;              // pointer to relative bitmap

    crPosition = CRPoint(lpNode->m_iX, lpNode->m_iY, lpNode->m_bRelocatable);

    if (lpNode->m_bRelative) {
        lpMap = &m_lpMaps[lpNode->m_iBitmap] ;  // point to relative bitmap

        if (lpMap->m_lpcBgbObject) {
            crPosition.x += lpMap->m_lpcBgbObject->m_crPosition.x ;
            crPosition.y += lpMap->m_lpcBgbObject->m_crPosition.y ;
        }
    }

    if (lpcSize) {
        crPosition.x -= lpcSize->cx / 2 ;
        crPosition.y -= lpcSize->cy ;
    }

// cleanup:

    JXELEAVE(CGtlData::NodeToPoint) ;
    RETURN(crPosition) ;
}

//* CGtlData::SpecifyUpdate -- specify update rectangle
BOOL CGtlData::SpecifyUpdate(CGtlView * xpGtlView)
// xpGtlView -- CGtlView object
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::SpecifyUpdate) ;
    int iError = 0 ;            // error code

    if (m_bChangeSize) {
        xpGtlView->Invalidate(FALSE);

#if NEWSCROLL

        if (xpGtlView->m_bBsuInit)
            xpGtlView->m_cViewBsuSet.PrepareWndBsuSet(
                        CSize(m_iSizeX, m_iSizeY),
                        CRect(0, m_iMargin, 0, 0)) ;
#else
        xpGtlView->SetScrollSizes(MM_TEXT, CSize(m_iSizeX, m_iSizeY)) ;
#endif
        m_bChangeSize = FALSE ;
    }

// cleanup:

    JXELEAVE(CGtlData::SpecifyUpdate) ;
    RETURN(iError != 0) ;
}


#ifdef NODEEDIT

//* CGtlData::UpdateDialogs -- update modeless dialog boxes
BOOL CGtlData::UpdateDialogs(BOOL bRetrieve)
// bRetrieve -- if TRUE, retrieve data from dialog box; if FALSE,
//                              initialize dialog box
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::UpdateDialogs) ;
    int iError = 0 ;            // error code

    UpdateMenuDialog(bRetrieve) ;
    UpdateInfoDialog(bRetrieve) ;
    UpdateControlDialog(bRetrieve) ;
    UpdateNodeDialog(bRetrieve) ;

// cleanup:
    JXELEAVE(CGtlData::UpdateDialogs) ;
    RETURN(iError != 0) ;
}

//* CGtlData::UpdateMenuDialog -- update control dialog box
BOOL CGtlData::UpdateMenuDialog(BOOL bRetrieve)
// xpGtlView -- pointer to GtlView object, derived from CView
// bRetrieve -- if TRUE, retrieve data from dialog box; if FALSE,
//                              initialize dialog box
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::UpdateMenuDialog) ;
    int iError = 0 ;            // error code
    CGtlView * xpGtlView = m_xpcGtlDoc->m_xpcLastFocusView ;
    CPoint cPoint ;
    CGtlApp * xpGtlApp = (CGtlApp *)AfxGetApp() ; // get application

    CMenuDlg * xpMenuDlg = m_xpcGtlDoc->m_xpcMenuDlg ;

    if (xpMenuDlg && !bRetrieve)        // update dialog box
    {
        xpMenuDlg->m_bCtlDlg = (m_xpcGtlDoc->m_xpcCtlDlg != NULL) ;
        xpMenuDlg->m_bInfoDlg = (m_xpcGtlDoc->m_xpcInfDlg != NULL) ;
        xpMenuDlg->m_bNodeDlg = (m_xpcGtlDoc->m_xpcNodeDlg != NULL) ;

        if (xpMenuDlg->m_hWnd)  // if dialog box window created
            xpMenuDlg->UpdateData(FALSE) ;      // update dialog box
    }

    if (xpMenuDlg && bRetrieve) // update data from dialog box
    {
        if ((xpMenuDlg->m_bCtlDlg != 0) != (m_xpcGtlDoc->m_xpcCtlDlg != NULL)) {
            if (xpMenuDlg->m_bCtlDlg && m_xpcGtlDoc->m_xpGtlFrame)
                m_xpcGtlDoc->m_xpGtlFrame->ShowControlDialog() ;
            if (!xpMenuDlg->m_bCtlDlg)
                m_xpcGtlDoc->m_xpcCtlDlg->DestroyWindow() ;
        }

        if ((xpMenuDlg->m_bNodeDlg != 0) != (m_xpcGtlDoc->m_xpcNodeDlg != NULL)) {
            if (xpMenuDlg->m_bNodeDlg && m_xpcGtlDoc->m_xpGtlFrame)
                m_xpcGtlDoc->m_xpGtlFrame->ShowNodeDialog() ;
            if (!xpMenuDlg->m_bNodeDlg)
                m_xpcGtlDoc->m_xpcNodeDlg->DestroyWindow() ;
        }

        if ((xpMenuDlg->m_bInfoDlg != 0) != (m_xpcGtlDoc->m_xpcInfDlg != NULL)) {
            if (xpMenuDlg->m_bInfoDlg && m_xpcGtlDoc->m_xpGtlFrame)
                m_xpcGtlDoc->m_xpGtlFrame->ShowInfoDialog() ;
            if (!xpMenuDlg->m_bInfoDlg)
                m_xpcGtlDoc->m_xpcInfDlg->DestroyWindow() ;
        }

        // new file
        if (m_xpcGtlDoc->m_xpcMenuDlg->m_bNew) {

            m_xpcGtlDoc->m_xpcMenuDlg->m_bNew = FALSE ;
            if (xpGtlApp)
                xpGtlApp->CallOnFileNew() ;
        }

//      if (m_xpcGtlDoc->m_xpcMenuDlg->m_bOpen) // open file
//      {
//          m_xpcGtlDoc->m_xpcMenuDlg->m_bOpen = FALSE ;
//          if (xpGtlApp)
//              xpGtlApp->CallOnFileOpen() ;
//      }
//
//      if (m_xpcGtlDoc->m_xpcMenuDlg->m_bSave) // save file
//      {
//          m_xpcGtlDoc->m_xpcMenuDlg->m_bSave = FALSE ;
//          m_xpcGtlDoc->DoSave(NULL) ;
//      }

        // request to close dialog box?
        //
        if (xpMenuDlg->m_bClose) {

            if (m_xpcGtlDoc->m_xpGtlFrame && m_xpcGtlDoc->m_xpGtlFrame->GetMenu()) {

                // uncheck menu item in frame window
                m_xpcGtlDoc->m_xpGtlFrame->GetMenu()->CheckMenuItem(ID_VIEW_MENU_DLG, MF_UNCHECKED) ;
            }
            m_xpcGtlDoc->m_xpcMenuDlg = NULL ;
        }
    }

// cleanup:
    JXELEAVE(CGtlData::UpdateMenuDialog) ;
    RETURN(iError != 0) ;
}

//* CGtlData::UpdateInfoDialog -- update information dialog box
BOOL CGtlData::UpdateInfoDialog(BOOL bRetrieve)
// xpGtlView -- pointer to GtlView object, derived from CView
// bRetrieve -- if TRUE, retrieve data from dialog box; if FALSE,
//                              initialize dialog box
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::UpdateInfoDialog) ;
    int iError = 0 ;            // error code

    CGtlView * xpGtlView = m_xpcGtlDoc->m_xpcLastMouseView ;

    CPoint cPoint ;
    CSize cSize, cSizePage, cSizeLine ;
    CRect cRect ;
//  int iMin, iMax ;
    CBsuInfo cBsuInfo ;

    CInfDlg * xpInfDlg = m_xpcGtlDoc->m_xpcInfDlg ;

    if (xpInfDlg && xpGtlView && !bRetrieve) {

//      if (xpGtlView->GetParent())
//          xpGtlView->GetParent()->GetWindowText(xpInfDlg->m_strView) ;

        xpInfDlg->m_iXCoord = xpGtlView->m_cCurrentPosition.x ;
        xpInfDlg->m_iYCoord = xpGtlView->m_cCurrentPosition.y ;

        xpGtlView->m_cViewBsuSet.GetInfo(&cBsuInfo) ;

        xpInfDlg->m_iLogScrollPosX = cBsuInfo.m_cScrollPosition.x ;
        xpInfDlg->m_iLogScrollPosY = cBsuInfo.m_cScrollPosition.y ;

        xpInfDlg->m_iScrollRangeX = cBsuInfo.m_cScrollRangeRect.right - cBsuInfo.m_cScrollRangeRect.left ;
        xpInfDlg->m_iScrollRangeY = cBsuInfo.m_cScrollRangeRect.bottom - cBsuInfo.m_cScrollRangeRect.top ;

        xpInfDlg->m_iTotalSizeX = cBsuInfo.m_cTotalSize.cx ;
        xpInfDlg->m_iTotalSizeY = cBsuInfo.m_cTotalSize.cy ;

        xpInfDlg->m_iScrollCount = cBsuInfo.m_iScrollCount ;

        xpGtlView->GetClientRect(&cRect) ;
        xpInfDlg->m_iViewSizeX = cRect.Width() ;
        xpInfDlg->m_iViewSizeY = cRect.Height() ;

        if (xpInfDlg->m_hWnd)   // if dialog box window created
            xpInfDlg->UpdateData(bRetrieve) ;   // update dialog box
    }

    // update data from dialog box
    //
    if (xpInfDlg && bRetrieve) {

        // request to close dialog box?
        if (xpInfDlg->m_bClose) {

            // uncheck menu item in frame window
            if (m_xpcGtlDoc->m_xpGtlFrame && m_xpcGtlDoc->m_xpGtlFrame->GetMenu())
                m_xpcGtlDoc->m_xpGtlFrame->GetMenu()->CheckMenuItem(ID_VIEW_INFO_DLG, MF_UNCHECKED) ;

            m_xpcGtlDoc->m_xpcInfDlg = NULL ;
        }
    }

// cleanup:
    JXELEAVE(CGtlData::UpdateInfoDialog) ;
    RETURN(iError != 0) ;
}

//* CGtlData::UpdateControlDialog -- update control dialog box
BOOL CGtlData::UpdateControlDialog(BOOL bRetrieve)
// xpGtlView -- pointer to GtlView object, derived from CView
// bRetrieve -- if TRUE, retrieve data from dialog box; if FALSE,
//                              initialize dialog box
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::UpdateControlDialog) ;
    int iError = 0 ;            // error code
    CGtlView * xpGtlView = m_xpcGtlDoc->m_xpcLastFocusView ;
    CPoint cPoint ;

    CCtlDlg * xpCtlDlg = m_xpcGtlDoc->m_xpcCtlDlg ;

    if (xpCtlDlg && !bRetrieve) // update dialog box
    {
        xpCtlDlg->m_bPaintBackground = m_bPaintBackground ;
        xpCtlDlg->m_bShowNodes = m_bShowNodes ;
        xpCtlDlg->m_bShowSensitivity = m_bShowSensitivity ;
        xpCtlDlg->m_bMetaGame = m_bMetaGame ;

        xpCtlDlg->InitDialogBox() ;
                        // set scroll bars *** to be written ***

        if (xpCtlDlg->m_hWnd)   // if dialog box window created
            xpCtlDlg->UpdateData(FALSE) ;       // update dialog box
    }

    if (xpCtlDlg && bRetrieve)  // update data from dialog box
    {

        if (m_bPaintBackground != xpCtlDlg->m_bPaintBackground)
        {
            m_bPaintBackground = xpCtlDlg->m_bPaintBackground ;
            CallUpdate() ;      // repaint screen
        }

        if (m_bShowNodes != xpCtlDlg->m_bShowNodes)
        {
            m_bShowNodes = xpCtlDlg->m_bShowNodes ;
            CallUpdate() ;      // repaint screen
        }

        if (m_bShowSensitivity != xpCtlDlg->m_bShowSensitivity)
        {
            m_bShowSensitivity = xpCtlDlg->m_bShowSensitivity ;
            CallUpdate() ;      // repaint screen
        }

        if (m_bMetaGame != xpCtlDlg->m_bMetaGame)
            SetMetaGame(xpCtlDlg->m_bMetaGame) ; // set metagame on/off

        if (xpCtlDlg->m_bClose)         // request to close dialog box?
        {
            if (m_xpcGtlDoc->m_xpGtlFrame
                        && m_xpcGtlDoc->m_xpGtlFrame->GetMenu())
                m_xpcGtlDoc->m_xpGtlFrame->GetMenu()
                        ->CheckMenuItem(ID_VIEW_CTL_DLG, MF_UNCHECKED) ;
                        // uncheck menu item in frame window
            m_xpcGtlDoc->m_xpcCtlDlg = NULL ;
        }
    }

// cleanup:
    JXELEAVE(CGtlData::UpdateControlDialog) ;
    RETURN(iError != 0) ;
}

//* CGtlData::UpdateNodeDialog -- update node dialog box
BOOL CGtlData::UpdateNodeDialog(BOOL bRetrieve)
// xpGtlView -- pointer to GtlView object, derived from CView
// bRetrieve -- if TRUE, retrieve data from dialog box; if FALSE,
//                              initialize dialog box
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::UpdateNodeDialog) ;
    int iError = 0 ;            // error code
    CGtlView * xpGtlView = m_xpcGtlDoc->m_xpcLastFocusView ;
    CPoint cPoint ;
    CNode FAR * lpSelectedNode = GetSelectedNode() ;
    CSectorTable * xpSectorTable = CMgStatic::cSectorTable,
                * xpSectorEntry ;
    int iSector ;       // MG_SECTOR_xxxx sector code
    LPSTR lpszSector ;      // sector label

    CNodeDlg * xpNodeDlg = m_xpcGtlDoc->m_xpcNodeDlg ;

    // update dialog box
    //
    if (xpNodeDlg && !bRetrieve) {

        if (!lpSelectedNode) {

        xpNodeDlg->m_stSector = xpNodeDlg->m_stLoc
                = xpNodeDlg->m_stName = "" ;
            xpNodeDlg->m_iNodeIndex = xpNodeDlg->m_iX
                        = xpNodeDlg->m_iY = xpNodeDlg->m_iWeight
                        = xpNodeDlg->m_iSensitivity = 0 ;
            xpNodeDlg->m_bRelocatable = FALSE ;
            xpNodeDlg->m_iSectorButton = -1 ;
        }

        else            // there's a selected node
        {
            xpNodeDlg->m_iNodeIndex = lpSelectedNode - m_lpNodes ;
            xpNodeDlg->m_stName = lpSelectedNode->m_szLabel ;
//          xpNodeDlg->m_stAction = lpSelectedNode->m_szAction ;

            if (lpSelectedNode->m_bRelative)
                xpNodeDlg->m_stLoc = m_lpMaps[lpSelectedNode
                                ->m_iBitmap].m_szLabel ;
            else
                xpNodeDlg->m_stLoc = "" ;

            xpNodeDlg->m_iX = lpSelectedNode->m_iX ;
            xpNodeDlg->m_iY = lpSelectedNode->m_iY ;

//          xpNodeDlg->m_bPassThru =
//                      (lpSelectedNode->m_bPassThru != 0) ;

            xpNodeDlg->m_iWeight = lpSelectedNode->m_iWeight ;
//          xpNodeDlg->m_iSensitivity =
//                              lpSelectedNode->m_iSensitivity ;
//          xpNodeDlg->m_iSector = lpSelectedNode->m_iSector ;
//          xpNodeDlg->m_iSectorButton = lpSelectedNode->m_iSector ?
//                      lpSelectedNode->m_iSector - MG_SECTOR_BASE
//                                      : MG_SECTOR_ANY ;
//          xpNodeDlg->m_bRelocatable =
                lpSelectedNode->m_iSensitivity ;
//      xpNodeDlg->m_iSector = lpSelectedNode->m_iSector ;
        xpNodeDlg->m_stSector = lpSelectedNode->m_szSector ;

        for (xpSectorEntry = xpSectorTable ;
                xpSectorEntry->m_iSectorCode
            && stricmp(lpSelectedNode->m_szSector,
                    xpSectorEntry->m_lpszLabel) ;
            ++xpSectorEntry)
        ;   // null loop body

        if (xpSectorEntry->m_iSectorCode)
                // if this location is in table
        xpNodeDlg->m_iSectorButton
            = xpSectorEntry->m_iSectorCode - MG_SECTOR_BASE ;
        else
        xpNodeDlg->m_iSectorButton = MG_SECTOR_ANY ;

//      xpNodeDlg->m_iSectorButton = lpSelectedNode->m_iSector ?
//              lpSelectedNode->m_iSector - MG_SECTOR_BASE
//                      : MG_SECTOR_ANY ;
        xpNodeDlg->m_bRelocatable =
                        (lpSelectedNode->m_bRelocatable != 0) ;
        }

        if (xpNodeDlg->m_hWnd)  // if dialog box window created
            xpNodeDlg->UpdateData(FALSE) ;      // update dialog box
    }

    if (xpNodeDlg && bRetrieve) // update data from dialog box
    {

//      if (m_bPaintBackground != xpNodeDlg->m_bPaintBackground)
//      {
//          m_bPaintBackground = xpNodeDlg->m_bPaintBackground ;
//          CallUpdate() ;      // repaint screen
//      }

        CNode * lpNode ;
        int iBitmap = 0 ;
        BOOL bUpdateDialog = FALSE ;
        CMap FAR * lpMap ;
        CBgbObject FAR * lpcBgbObject ;

        if (xpNodeDlg->m_bApply && (xpNodeDlg->m_iNodeIndex < 0
                    || xpNodeDlg->m_iNodeIndex >= m_iNodes
                    || (lpNode = m_lpNodes + xpNodeDlg->m_iNodeIndex)
                                ->m_bDeleted))
        {
            ::MessageBox(NULL, "Invalid node number.", NULL,
                        MB_OK | MB_ICONINFORMATION) ;
            UpdateNodeDialog(FALSE) ;   // reset dialog box
            iError = 100 ;
            goto cleanup ;
        }

        if (xpNodeDlg->m_bApply && lpNode != lpSelectedNode)
        {
            ModifySelectNode(lpNode, TRUE) ;    // select the node
            lpSelectedNode = GetSelectedNode() ;
            UpdateNodeDialog(FALSE) ;   // reset dialog box
        }

        if (xpNodeDlg->m_bApply && lpSelectedNode
                && lpSelectedNode->m_szLabel != xpNodeDlg->m_stName)
            _fstrncpy(lpSelectedNode->m_szLabel, xpNodeDlg->m_stName,
                        MAX_LABEL_LENGTH - 1) ;

//      if (xpNodeDlg->m_bApply && lpSelectedNode
//              && lpSelectedNode->m_szAction != xpNodeDlg->m_stAction)
//          _fstrncpy(lpSelectedNode->m_szAction,
//                      xpNodeDlg->m_stAction , MAX_LABEL_LENGTH - 1) ;

        if (xpNodeDlg->m_bApply && lpSelectedNode
                && (((xpNodeDlg->m_stLoc != "") != 0)
                        != (lpSelectedNode->m_bRelative != 0)
                || (lpSelectedNode->m_bRelative &&
                        xpNodeDlg->m_stLoc != m_lpMaps[lpSelectedNode
                                ->m_iBitmap].m_szLabel)))
        {
            if ( (xpNodeDlg->m_stLoc != "") &&
                        (iError = GetLabel((const LPSTR)(const char *)
                                xpNodeDlg->m_stLoc, FALSE, iBitmap)) )
            {
                ::MessageBox(NULL, "Invalid bitmap label.", NULL,
                            MB_OK | MB_ICONINFORMATION) ;
                UpdateNodeDialog(FALSE) ;       // reset dialog box
                goto cleanup ;
            }

            CallUpdate(lpSelectedNode, NULL, TRUE, TRUE) ;
            lpSelectedNode->m_bRelative = (xpNodeDlg->m_stLoc != "") ;
            lpSelectedNode->m_iBitmap = iBitmap ;
            xpNodeDlg->m_iX = xpNodeDlg->m_iY = 0 ;
            bUpdateDialog = TRUE ;
            CallUpdate(lpSelectedNode, NULL, TRUE, TRUE) ;
        }

        if (xpNodeDlg->m_bApply && lpSelectedNode
                        && xpNodeDlg->m_iX != lpSelectedNode->m_iX)
        {
            if (lpSelectedNode->m_bRelative)
            {
                lpMap = m_lpMaps + lpSelectedNode->m_iBitmap ;
                lpcBgbObject = lpMap->m_lpcBgbObject ;

                if (xpNodeDlg->m_iX < 0 ||
                        xpNodeDlg->m_iX > lpcBgbObject->m_cSize.cx)
                {
                    ::MessageBox(NULL, "X coordinate out of range.", NULL,
                            MB_OK | MB_ICONINFORMATION) ;
                    UpdateNodeDialog(FALSE) ;   // reset dialog box
                    goto cleanup ;
                }
            }
            CallUpdate(lpSelectedNode, NULL, TRUE, TRUE) ;
            lpSelectedNode->m_iX = xpNodeDlg->m_iX ;
            CallUpdate(lpSelectedNode, NULL, TRUE, TRUE) ;
        }


        if (xpNodeDlg->m_bApply && lpSelectedNode
                && xpNodeDlg->m_iY != lpSelectedNode->m_iY)
        {
            if (lpSelectedNode->m_bRelative)
            {
                lpMap = m_lpMaps + lpSelectedNode->m_iBitmap ;
                lpcBgbObject = lpMap->m_lpcBgbObject ;

                if (xpNodeDlg->m_iY < 0 ||
                        xpNodeDlg->m_iY > lpcBgbObject->m_cSize.cy)
                {
                    ::MessageBox(NULL, "Y coordinate out of range.",
                                NULL, MB_OK | MB_ICONINFORMATION) ;
                    UpdateNodeDialog(FALSE) ;   // reset dialog box
                    goto cleanup ;
                }
            }
            CallUpdate(lpSelectedNode, NULL, TRUE, TRUE) ;
            lpSelectedNode->m_iY = xpNodeDlg->m_iY ;
            CallUpdate(lpSelectedNode, NULL, TRUE, TRUE) ;
        }

        if (xpNodeDlg->m_bApply && lpSelectedNode
                        && lpSelectedNode->IfRelocatable()
                            != (xpNodeDlg->m_bRelocatable != 0))
            lpSelectedNode->m_bRelocatable = xpNodeDlg->m_bRelocatable ;

        if (xpNodeDlg->m_bApply && lpSelectedNode &&
                xpNodeDlg->m_iWeight != lpSelectedNode->m_iWeight)
        {
            lpSelectedNode->m_bWgtSpec = TRUE ;
            lpSelectedNode->m_iWeight = xpNodeDlg->m_iWeight ;
        }

        if (xpNodeDlg->m_bApply && lpSelectedNode &&
                xpNodeDlg->m_iSensitivity !=
                                lpSelectedNode->m_iSensitivity)
        {
            lpSelectedNode->m_bSenSpec = TRUE ;
            lpSelectedNode->m_iSensitivity = xpNodeDlg->m_iSensitivity ;
        }

//      if (xpNodeDlg->m_bApply && lpSelectedNode &&
//              xpNodeDlg->m_iSector != lpSelectedNode->m_iSector)
//          lpSelectedNode->m_iSector = xpNodeDlg->m_iSector ;

        iSector = (xpNodeDlg->m_iSectorButton < 0) ? 0 :
                xpNodeDlg->m_iSectorButton + MG_SECTOR_BASE ;

    xpSectorEntry = CMgStatic::FindSector(iSector) ;
                // point to sector table entry
    lpszSector = xpSectorEntry ? xpSectorEntry->m_lpszLabel
                : "" ;

    if (xpNodeDlg->m_bApply && lpSelectedNode
        && lpSelectedNode->m_szSector != lpszSector)
        _fstrncpy(lpSelectedNode->m_szSector, lpszSector,
                MAX_LABEL_LENGTH - 1) ;

//      if (xpNodeDlg->m_bApply && lpSelectedNode
//              && iSector != lpSelectedNode->m_iSector)
//          lpSelectedNode->m_iSector = xpNodeDlg->m_iSector
//                              = iSector ;

        if (bUpdateDialog && xpNodeDlg->m_hWnd)
            xpNodeDlg->UpdateData(FALSE) ;      // update dialog box

        if (xpNodeDlg->m_bClose)        // request to close dialog box?
        {
            if (m_xpcGtlDoc->m_xpGtlFrame
                        && m_xpcGtlDoc->m_xpGtlFrame->GetMenu())
                m_xpcGtlDoc->m_xpGtlFrame->GetMenu()
                        ->CheckMenuItem(ID_VIEW_NODE_DLG, MF_UNCHECKED) ;
                        // uncheck menu item in frame window
            m_xpcGtlDoc->m_xpcNodeDlg = NULL ;
        }
    }

cleanup:

    if (xpNodeDlg)
        xpNodeDlg->m_bApply = FALSE ;   // cancel apply button
    JXELEAVE(CGtlData::UpdateNodeDialog) ;
    RETURN(iError != 0) ;
}

//* CGtlData::UpdateFocusRect -- update focus rectangle for hiliting
BOOL CGtlData::UpdateFocusRect(void)
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::UpdateFocusRect) ;
    int iError = 0 ;            // error code


// cleanup:

    JXELEAVE(CGtlData::UpdateFocusRect) ;
    RETURN(iError != 0) ;
}

#endif

// BCW Optimized this function Tue  08-02-1994  19:53:59
//

//* CGtlData::AdjustToView -- set up bitmaps for view
BOOL CGtlData::AdjustToView(CGtlView *xpGtlView)
// xpGtlView -- CView window
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::AdjustToView);
    int iError = 0;            // error code
    CBgbObject *lpcBgbObject;
    char szPath[200];
    CMap *lpMap;
    CNode *lpNode;
    int iK, iL = 0 ;            // loop variables
    BOOL bNewBitmap = FALSE;
    CLocTable *xpLocTable = CMgStatic::cLocTable, *xpLocEntry;

    for (iK = 0 ; iK < m_iMaps ; ++iK) {

        lpMap = m_lpMaps + iK ;

        if (m_bMetaGame || !lpMap->m_bMetaGame) {

            lpcBgbObject = lpMap->m_lpcBgbObject;

            // the object should have already been allocted in GTLCPL
            assert(lpcBgbObject != NULL);

            if (!lpcBgbObject->m_bInit) {

                bNewBitmap = TRUE;

                lpcBgbObject->m_iBgbType = lpMap->m_bSprite ? BGBT_SPRITE : BGBT_DIB ;
                lpcBgbObject->m_crPosition.m_bRelocatable = (lpMap->m_bRelocatable != 0) ;

                // next load in the bitmap segment and test
                strcpy(szPath, m_szBmpDirectory) ;
                _fstrcat(szPath, lpMap->m_szFilename) ;

                m_cBgbMgr.InitBitmapObject(lpcBgbObject, szPath);
            }
        }
    }

    NormalizeData(xpGtlView) ;  // normalize data so point (0,0)
                                // is in upper LH corner

    if (xpGtlView) {
        if (!xpGtlView->m_bBsuInit && !xpGtlView-> m_cViewBsuSet.InitWndBsuSet(xpGtlView))
            xpGtlView->m_bBsuInit = TRUE;

        xpGtlView->m_cViewBsuSet.PrepareWndBsuSet(CSize(m_iSizeX, m_iSizeY), CRect(0, m_iMargin, 0, 0));
    }

    for (iK = 0 ; iK < m_iNodes ; ++iK) {
        if (!(lpNode = m_lpNodes + iK)->m_bDeleted) {

            if (_fstricmp(lpNode->m_szLabel, "menu") == 0)
                lpNode->m_bMenu = TRUE;
#if 0
            // find this nodes sector
            //
            lpNode->m_iSector = MG_SECTOR_ANY;
            pSectorEntry = CMgStatic::cSectorTable;
            while (pSectorEntry->m_iSectorCode != 0) {
                if (stricmp(pSectorEntry->m_lpszLabel, lpNode->m_szSector) == 0) {
                    lpNode->m_iSector = pSectorEntry->m_iSectorCode;
                    break;
                }
                pSectorEntry++;
            }
#endif
            for (xpLocEntry = xpLocTable ; xpLocEntry->m_iLocCode && stricmp(lpNode->m_szLabel, xpLocEntry->m_lpszLabel) ; ++xpLocEntry)
                ;       // null loop body

            // if this location is in table
            //
            if (xpLocEntry->m_iLocCode)
                lpNode->m_iLocationCode = xpLocEntry->m_iLocCode ;
            
            if (lpNode->m_iSensitivity == 0 && !lpNode->m_bSenSpec) {
                if (lpNode->m_bMenu)
                    lpNode->m_iSensitivity = NODERADIUS ;
                else if (lpNode->m_iWeight > 0 && lpNode->m_bRelocatable && lpNode->m_iNumLinks > 0)
                    lpNode->m_iSensitivity = NODESENSITIVITY ;
            }
        }
    }

    // determine current positions of sprites and movable overlays
    for (iK = 0 ; iK < m_iMaps ; ++iK) {
        lpMap = m_lpMaps + iK ;

        if ((m_bMetaGame || !lpMap->m_bMetaGame) && ((lpcBgbObject = lpMap->m_lpcBgbObject) != NULL)) {

            // Adjust sprite positions in case anything's moved
            // Note: May have to add code here to erase sprite background
            //  under some circumstances.
            if (lpMap->m_bSprite && lpMap->m_iRelationType == KT_NODE) {

                lpNode = m_lpNodes + lpMap->m_iRelation ;

                // point to relative node
                m_cBgbMgr.SetPosition(lpcBgbObject, NodeToPoint(lpNode)) ;
            }


            if (stricmp(lpMap->m_szLabel, "f00") >= 0 && stricmp(lpMap->m_szLabel, "f24") <= 0) {

                int iFurlong = 10 * lpMap->m_szLabel[1] + lpMap->m_szLabel[2] - 11 * '0' ;

                if (iFurlong >= 0 && iFurlong < DIMENSION(m_lpFurlongMaps))
                    m_lpFurlongMaps[iFurlong] = lpMap ;

                lpMap->m_bSpecialPaint = TRUE ;

            } else if (stricmp(lpMap->m_szLabel, "Hodj") == 0 || stricmp(lpMap->m_szLabel, "Podj") == 0) {

                InitOverlay(lpMap) ;    // initialize sprite

            } else if (stricmp(lpMap->m_szLabel, "Minib1") == 0) {
                lpMap->m_bSpecialPaint = TRUE ;
                m_cBbtMgr.LinkButton(&m_cMiniButton, lpcBgbObject, NULL) ;

            } else if (stricmp(lpMap->m_szLabel, "Minib2") == 0) {
                lpMap->m_bSpecialPaint = TRUE;
                m_cBbtMgr.LinkButton(&m_cMiniButton, NULL, lpcBgbObject);

            } else if (stricmp(lpMap->m_szLabel, "Invb1") == 0) {
                lpMap->m_bSpecialPaint = TRUE ;
                m_cBbtMgr.LinkButton(&m_cInvButton, lpcBgbObject, NULL) ;

            } else if (stricmp(lpMap->m_szLabel, "Invb2") == 0) {
                lpMap->m_bSpecialPaint = TRUE ;
                m_cBbtMgr.LinkButton(&m_cInvButton, NULL, lpcBgbObject) ;

            } else if (stricmp(lpMap->m_szLabel, "Scrob1") == 0) {
                lpMap->m_bSpecialPaint = TRUE ;
                m_cBbtMgr.LinkButton(&m_cScrollButton, lpcBgbObject, NULL) ;

            } else if (stricmp(lpMap->m_szLabel, "Scrob2") == 0) {
                lpMap->m_bSpecialPaint = TRUE ;
                m_cBbtMgr.LinkButton(&m_cScrollButton, NULL, lpcBgbObject) ;

#ifndef NODEEDIT
            } else if (stricmp(lpMap->m_szLabel, "podjb1") == 0) {

                assert(m_xpXodjChain != NULL);
                m_xpXodjChain->m_lpcIcon = lpcBgbObject;

            } else if (stricmp(lpMap->m_szLabel, "hodjb1") == 0) {

                assert(m_xpXodjChain != NULL);
                assert(m_xpXodjChain->m_xpXodjNext != NULL);
                m_xpXodjChain->m_xpXodjNext->m_lpcIcon = lpcBgbObject;
#endif
            }
        }
    }

    if (bNewBitmap) {

        // this forces PositionCharacters to repaint
        m_bSamePlace = TRUE ;

        PositionCharacters();
    }

//cleanup:

    JXELEAVE(CGtlData::AdjustToView) ;
    RETURN(iError != 0) ;
}


//* CGtlData::InitOverlay -- initialize overlay
BOOL CGtlData::InitOverlay(CMap FAR * lpMap)
// lpMap -- bitmap object for overlay sprite
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::InitOverlay) ;
    int iError = 0 ;            // error code
    CXodj * xpXodj = NULL ;

    for (xpXodj = m_xpXodjChain ; xpXodj && _fstricmp(lpMap->m_szLabel, xpXodj->m_szName); xpXodj = xpXodj->m_xpXodjNext)
        ;       // null loop body

    if (!xpXodj) {              // existing character not found

        if ((xpXodj = new CXodj) == NULL) {
            iError = 100 ;      // can't allocate character
            goto cleanup ;
        }
        xpXodj->m_xpXodjNext = m_xpXodjChain ;
                                // put in front of chain
        m_xpXodjChain = xpXodj ;

        _fstrncpy(xpXodj->m_szName, lpMap->m_szLabel, sizeof(xpXodj->m_szName) - 1);
        xpXodj->m_szName[0] = (char)toupper(xpXodj->m_szName[0]);

        xpXodj->m_lpcCharSprite = lpMap->m_lpcBgbObject ;
        lpMap->m_bSpecialPaint = TRUE ;

        // GTB - 8/25 - I put this in to find and then place hodj and podj at the starting loc
        // you'll find STARTING_LOCATION at the top of this file.

		CNode	*pNode = m_lpNodes;
		int		nTemp = 0;
		do {
			if ( lstrcmp( pNode->m_szLabel, STARTING_LOCATION ) == 0 ){
				pNode = NULL;
				break;
			}
			else {
				pNode++;
				nTemp++;
			}
			
		} while ( pNode != NULL );
		
        xpXodj->m_iCharNode = nTemp ;
// GTB - 8/25 end of my change
        
//        xpXodj->m_iCharNode = -(lpMap - m_lpMaps) - 1 ;
                        // kludge forces char to start
                                        // at castle
        xpXodj->m_bHodj = (_fstricmp(lpMap->m_szLabel, "Hodj") == 0) ;

        // initialize theme music info
        //
        if (xpXodj->m_bHodj) {
            xpXodj->m_pszThemeFile = (char *)&HODJS_WALKING_SOUND;
            xpXodj->m_nThemeStart = HODJ_SOUND_START;
            xpXodj->m_nThemeEnd = HODJ_SOUND_END;
            xpXodj->m_pszStarsFile = (char *)&HODJS_STARS_FILE;
        } else {
            xpXodj->m_pszThemeFile = (char *)&PODJS_WALKING_SOUND;
            xpXodj->m_nThemeStart = PODJ_SOUND_START;
            xpXodj->m_nThemeEnd = PODJ_SOUND_END;
            xpXodj->m_pszStarsFile = (char *)&PODJS_STARS_FILE;
        }

    }

    if (!m_xpCurXodj)
        m_xpCurXodj = xpXodj ;

cleanup:

    JXELEAVE(CGtlData::InitOverlay) ;
    RETURN(iError != 0) ;
}

//* CGtlData::NormalizeData -- normalize data after loading
BOOL CGtlData::NormalizeData(CGtlView * xpGtlView)
// xpGtlView -- void pointer to current view
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::NormalizeData) ;
    int iError = 0 ;            // error code
    int iK ;                    // loop variable
    CMap FAR * lpMap, FAR * lpPrevMap ;
    int iMinX = MAXPOSINT, iMinY = MAXPOSINT ;  // min x,y coordinates
    int iMaxX = MINNEGINT, iMaxY = MINNEGINT ;  // max x,y coordinates
    BOOL bPositionFound = FALSE ;
    BOOL bChangeSize = FALSE, bChangeOrigin = FALSE ;
    CNode FAR * lpNode ;
    CBgbObject FAR * lpcBgbObject, FAR * lpcPrevBgbObject ;

    for (iK = 0 ; iK < m_iMaps ; ++iK) {
        lpMap = m_lpMaps + iK ;
        lpPrevMap = NULL ;
        lpcPrevBgbObject = NULL ;

        if (lpMap->m_iRelation >= 0 && lpMap->m_iRelation < m_iMaps) {
            lpPrevMap = m_lpMaps + lpMap->m_iRelation ;
                                // get previous map pointer
            lpcPrevBgbObject = lpPrevMap->m_lpcBgbObject ;
        }

        if (!lpMap->m_bPositionDetermined && lpMap->m_iRelationType && ((lpcBgbObject = lpMap->m_lpcBgbObject) != NULL)) {

            switch (lpMap->m_iRelationType) {

                case KT_LEFT:
                    if (lpcPrevBgbObject) {
                        lpcBgbObject->m_crPosition.x = lpcPrevBgbObject->m_crPosition.x - lpcBgbObject->m_cSize.cx ;
                        lpcBgbObject->m_crPosition.y = lpcPrevBgbObject->m_crPosition.y ;
                        lpMap->m_bPositionDetermined = TRUE ;
                    }
                    break ;

                case KT_RIGHT:
                    if (lpcPrevBgbObject) {
                        lpcBgbObject->m_crPosition.x = lpcPrevBgbObject->m_crPosition.x + lpcPrevBgbObject->m_cSize.cx ;
                        lpcBgbObject->m_crPosition.y = lpcPrevBgbObject->m_crPosition.y ;
                        lpMap->m_bPositionDetermined = TRUE ;
                    }
                    break ;

                case KT_ABOVE:
                    if (lpcPrevBgbObject) {
                        lpcBgbObject->m_crPosition.x = lpcPrevBgbObject->m_crPosition.x ;
                        lpcBgbObject->m_crPosition.y = lpcPrevBgbObject->m_crPosition.y - lpcBgbObject->m_cSize.cy ;
                        lpMap->m_bPositionDetermined = TRUE ;
                    }
                    break ;

                case KT_BELOW:
                    if (lpcPrevBgbObject) {
                        lpcBgbObject->m_crPosition.x = lpcPrevBgbObject->m_crPosition.x ;
                        lpcBgbObject->m_crPosition.y = lpcPrevBgbObject->m_crPosition.y + lpcPrevBgbObject->m_cSize.cy ;
                        lpMap->m_bPositionDetermined = TRUE ;
                    }
                    break ;

                case KT_NODE:           // ignore
                    if (lpMap->m_iRelation < 0 || lpMap->m_iRelation >= m_iNodes || (lpNode = m_lpNodes + lpMap->m_iRelation) ->m_bDeleted) {
                        iError = 200 + iK ;     // bitmap node
                                                // reference not found
                        goto cleanup ;
                    }
                    lpcBgbObject->m_crPosition = NodeToPoint(lpNode);
                    break ;

                default:
                    iError = 100 + iK ;         // invalid relationship
                    goto cleanup ;
                    // break ;
            }
            bPositionFound = TRUE ;
        }

        if (!lpMap->m_bOverlay && lpMap->m_lpcBgbObject && lpMap->m_lpcBgbObject->m_crPosition.IfRelocatable()) {
            if (iMinX > lpMap->m_lpcBgbObject->m_crPosition.x)
                iMinX = lpMap->m_lpcBgbObject->m_crPosition.x ;
            if (iMinY > lpMap->m_lpcBgbObject->m_crPosition.y)
                iMinY = lpMap->m_lpcBgbObject->m_crPosition.y ;

            if (iMaxX < lpMap->m_lpcBgbObject->m_crPosition.x + lpMap->m_lpcBgbObject->m_cSize.cx)
                iMaxX = lpMap->m_lpcBgbObject->m_crPosition.x + lpMap->m_lpcBgbObject->m_cSize.cx;
            if (iMaxY < lpMap->m_lpcBgbObject->m_crPosition.y + lpMap->m_lpcBgbObject->m_cSize.cy)
                iMaxY = lpMap->m_lpcBgbObject->m_crPosition.y + lpMap->m_lpcBgbObject->m_cSize.cy;
        }
    }

    for (iK = 0 ; iK < m_iNodes ; ++iK) {
        if (!(lpNode = m_lpNodes + iK)->m_bDeleted && !lpNode->m_bRelative && lpNode->IfRelocatable()) {
            if (iMinX > lpNode->m_iX - NODERADIUS)
                iMinX = lpNode->m_iX - NODERADIUS ;
            if (iMinY > lpNode->m_iY - NODERADIUS)
                iMinY = lpNode->m_iY - NODERADIUS ;
            if (iMaxX < lpNode->m_iX + NODERADIUS)
                iMaxX = lpNode->m_iX + NODERADIUS ;
            if (iMaxY < lpNode->m_iY + NODERADIUS)
                iMaxY = lpNode->m_iY + NODERADIUS ;
        }
    }

    bChangeOrigin = (bPositionFound && (iMinX || iMinY)) ;

    if (bChangeOrigin) {

        for (iK = 0 ; iK < m_iMaps ; ++iK) {
            lpMap = m_lpMaps + iK ;

            if (lpMap->m_lpcBgbObject && lpMap->m_lpcBgbObject->m_crPosition.IfRelocatable() && !lpMap->m_bPositionSpecified) {
                lpMap->m_lpcBgbObject->m_crPosition.x -= iMinX ;
                lpMap->m_lpcBgbObject->m_crPosition.y -= iMinY ;
            }
        }

        for (iK = 0 ; iK < m_iNodes ; ++iK) {
            if (!(lpNode = m_lpNodes + iK)->m_bDeleted && !lpNode->m_bRelative && lpNode->IfRelocatable()) {
                lpNode->m_iX -= iMinX ;
                lpNode->m_iY -= iMinY ;
            }
        }
    } else {
        if (iMaxX < 0)
            iMaxX = 1 ;
        if (iMaxY < 0)
            iMaxY = 1 ;
        iMinX = iMinY = 0 ;
    }

    if (m_iSizeX != iMaxX - iMinX)
        bChangeSize = TRUE, m_iSizeX = iMaxX - iMinX ;
    if (m_iSizeY != iMaxY - iMinY)
        bChangeSize = TRUE, m_iSizeY = iMaxY - iMinY ;

    m_iMargin = 24;

    if (bChangeSize)
        ((CGtlDoc *)m_xpcGtlDoc)->UpdateAllViews(xpGtlView, HINT_SIZE, NULL);

cleanup:

    JXELEAVE(CGtlData::NormalizeData) ;
    RETURN(iError != 0) ;
}
