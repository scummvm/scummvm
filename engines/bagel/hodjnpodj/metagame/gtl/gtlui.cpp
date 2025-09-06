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
#include "bagel/hodjnpodj/hnplibs/sprite.h"
#include "bagel/hodjnpodj/metagame/gtl/gtl.h"
#include "bagel/hodjnpodj/metagame/gtl/gtldoc.h"
#include "bagel/hodjnpodj/metagame/gtl/gtlview.h"
#include "bagel/hodjnpodj/metagame/gtl/gtlfrm.h"
#include "bagel/hodjnpodj/metagame/bgen/bgen.h"
#include "bagel/hodjnpodj/metagame/gtl/gtldat.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

#define STARTING_LOCATION   "Castle"

#define HODJS_WALKING_SOUND ".\\SOUND\\HODJST.MID"
#define HODJ_SOUND_START    2000
#define HODJ_SOUND_END      63000
#define HODJS_STARS_FILE    ".\\ART\\HSTAR.BMP"

#define PODJS_WALKING_SOUND ".\\SOUND\\PODJST.MID"
#define PODJ_SOUND_START    1000
#define PODJ_SOUND_END      64000
#define PODJS_STARS_FILE    ".\\ART\\PSTAR.BMP"

extern uint32 GetPhysicalMemory(void);


CNode::CNode() {
	m_bDeleted = FALSE;
	m_bSelected = FALSE;
	m_bRelative = FALSE;
	m_bRelocatable = FALSE;
	m_bWgtSpec = FALSE;
	m_bSenSpec = FALSE;
	m_bMenu = FALSE;
}

//* CGtlData::CGtlData -- constructor -- zero out all fields, allocate
//      arrays, and set colors
CGtlData::CGtlData(void) {
	JXENTER(CGtlData::CGtlData) ;

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
CGtlData::~CGtlData(void) {
	JXENTER(CGtlData::~CGtlData) ;
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

void CGtlData::CreateOffScreenBmp(void) {
	CDC *pScreenDC;
	CGtlApp *xpGtlApp = (CGtlApp *)AfxGetApp();

	if (m_pOffScreenBmp == nullptr) {

		if ((m_cBgbMgr.m_xpGamePalette != nullptr) &&
		        (GetFreeSpace(0) >= (unsigned long)250000) && (GlobalCompact(0) >= (unsigned long)500000)) {

			if ((m_pOffScreenBmp = new CBitmap) != nullptr) {

				pScreenDC = m_xpGtlView->GetDC();
				if (m_pOffScreenBmp->CreateCompatibleBitmap(pScreenDC, xpGtlApp->m_iWidth, xpGtlApp->m_iHeight) == FALSE) {

					delete m_pOffScreenBmp;
					m_pOffScreenBmp = nullptr;
				}
				m_xpGtlView->ReleaseDC(pScreenDC);
			}
		}
	}
}

void CGtlData::DeleteOffScreenBmp(void) {
	if (m_pOffScreenBmp != nullptr) {
		delete m_pOffScreenBmp;
		m_pOffScreenBmp = nullptr;
	}
}

void CGtlData::PaintOffScreenBmp(void) {
	CDC *pScreenDC;
	CMemDC *pMemDC;
	CPalette *pScreenPalOld;
	CGtlApp *xpGtlApp = (CGtlApp *)AfxGetApp();

	if (m_pOffScreenBmp != nullptr) {

		if ((pMemDC = GetMemDC()) != nullptr) {

			if ((pScreenDC = m_xpGtlView->GetDC()) != nullptr) {
				pScreenPalOld = pScreenDC->SelectPalette(m_cBgbMgr.m_xpGamePalette, FALSE);
				pScreenDC->BitBlt(0, 0, xpGtlApp->m_iWidth, xpGtlApp->m_iHeight, pMemDC->m_pDC, 0, 0, SRCCOPY);
				pScreenDC->SelectPalette(pScreenPalOld, FALSE);
				m_xpGtlView->ReleaseDC(pScreenDC);
			}
			ReleaseMemDC(pMemDC);
		}
	}
}
CMemDC::CMemDC(void) {
	m_pDC = new CDC;
	assert(m_pDC != nullptr);
}

CMemDC::~CMemDC(void) {
	if (m_pDC != nullptr) {
		delete m_pDC;
		m_pDC = nullptr;
	}
}

CMemDC *CGtlData::GetMemDC(void) {
	CMemDC *pMemDC;

	if ((pMemDC = new CMemDC) != nullptr) {
		pMemDC->m_pDC->CreateCompatibleDC(nullptr);
		assert(m_cBgbMgr.m_xpGamePalette != nullptr);
		pMemDC->m_pPalOld = pMemDC->m_pDC->SelectPalette(m_cBgbMgr.m_xpGamePalette, FALSE);
		pMemDC->m_pDC->RealizePalette();
		pMemDC->m_hBmpOld = SelectBitmap(pMemDC->m_pDC->m_hDC, m_pOffScreenBmp->m_hObject);
	}
	assert(pMemDC != nullptr);

	return (pMemDC);
}

void CGtlData::ReleaseMemDC(CMemDC *pMemDC) {
	assert(pMemDC != nullptr);

	if (pMemDC != nullptr) {
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
	int iError = 0 ;        // error code
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
	CRPoint crPoint1, crPoint2 ;        // link points
	CRPoint crPosition ;                // position within window
	BOOL bPaint = (xpDc != nullptr) ;      // test for WM_PAINT
	CMemDC *pMemDC;
	CDC *pOldDC;

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

	if (!m_bInhibitDraw) {

		pMemDC = nullptr;
		pOldDC = nullptr;
		if (m_pOffScreenBmp == nullptr) {
			CreateOffScreenBmp();
		}
		if (m_pOffScreenBmp != nullptr) {
			pOldDC = m_cBgbMgr.m_xpDc;
			pMemDC = GetMemDC();
			xpDc = m_cBgbMgr.m_xpDc = pMemDC->m_pDC;
		}

		if (m_bPaint)
			CSprite::ClearBackgrounds();

		DrawBitmaps(xpDc, FALSE) ;  // draw non-overlay bitmaps

	
		// paint sprites and moveable overlays
		DrawBitmaps(xpDc, TRUE) ;   // draw non-overlay bitmaps

		if (m_pOffScreenBmp != nullptr) {
			ReleaseMemDC(pMemDC);
			PaintOffScreenBmp();

			m_cBgbMgr.m_xpDc = pOldDC;
		}

		m_cBgbMgr.ResumeAnimations();
	}

// cleanup:

	m_cBgbMgr.AdjustLockCount(-1) ;
	m_cBgbMgr.ReleaseDc() ;

	m_xpClipRect = m_xpDevClipRect = nullptr;
	m_bDrawing = FALSE;

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
			CSound::handleMessages();

		lpMap = m_lpMaps + iK ;

		if (!lpMap->m_bSpecialPaint && EQV(lpMap->m_bOverlay, bOverlay) && (_metaGame || !lpMap->m_bMetaGame) && ((lpcBgbObject = lpMap->m_lpcBgbObject) != nullptr))
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
		if ((xpXodj = m_xpCurXodj) != nullptr) {

			int iNumFurlongs = xpXodj->m_iFurlongs ;

			if (iNumFurlongs < 0)
				iNumFurlongs = 0 ;
			if (iNumFurlongs > 24)
				iNumFurlongs = 24 ;
			if (((lpMap = m_lpFurlongMaps[iNumFurlongs]) != nullptr) && ((lpcBgbObject = lpMap->m_lpcBgbObject) != nullptr))
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

	assert(pBgbObject != nullptr);

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
	int iK;
	BOOL bNewBitmap = FALSE;
	const CLocTable *xpLocTable = CMgStatic::cLocTable, *xpLocEntry;

	for (iK = 0 ; iK < m_iMaps ; ++iK) {

		lpMap = m_lpMaps + iK ;

		if (_metaGame || !lpMap->m_bMetaGame) {

			lpcBgbObject = lpMap->m_lpcBgbObject;

			// the object should have already been allocted in GTLCPL
			assert(lpcBgbObject != nullptr);

			if (!lpcBgbObject->m_bInit) {

				bNewBitmap = TRUE;

				lpcBgbObject->m_iBgbType = lpMap->m_bSprite ? BGBT_SPRITE : BGBT_DIB ;
				lpcBgbObject->m_crPosition.m_bRelocatable = (lpMap->m_bRelocatable != 0) ;

				// next load in the bitmap segment and test
				Common::strcpy_s(szPath, m_szBmpDirectory) ;
				Common::strcat_s(szPath, lpMap->m_szFilename) ;

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

			if (scumm_stricmp(lpNode->m_szLabel, "menu") == 0)
				lpNode->m_bMenu = TRUE;
			#if 0
			// find this nodes sector
			//
			lpNode->m_iSector = MG_SECTOR_ANY;
			pSectorEntry = CMgStatic::cSectorTable;
			while (pSectorEntry->m_iSectorCode != 0) {
				if (scumm_stricmp(pSectorEntry->m_lpszLabel, lpNode->m_szSector) == 0) {
					lpNode->m_iSector = pSectorEntry->m_iSectorCode;
					break;
				}
				pSectorEntry++;
			}
			#endif
			for (xpLocEntry = xpLocTable ; xpLocEntry->m_iLocCode && scumm_stricmp(lpNode->m_szLabel, xpLocEntry->m_lpszLabel) ; ++xpLocEntry)
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

		if ((_metaGame || !lpMap->m_bMetaGame) && ((lpcBgbObject = lpMap->m_lpcBgbObject) != nullptr)) {

			// Adjust sprite positions in case anything's moved
			// Note: May have to add code here to erase sprite background
			//  under some circumstances.
			if (lpMap->m_bSprite && lpMap->m_iRelationType == KT_NODE) {

				lpNode = m_lpNodes + lpMap->m_iRelation ;

				// point to relative node
				m_cBgbMgr.SetPosition(lpcBgbObject, NodeToPoint(lpNode)) ;
			}


			if (scumm_stricmp(lpMap->m_szLabel, "f00") >= 0 && scumm_stricmp(lpMap->m_szLabel, "f24") <= 0) {

				int iFurlong = 10 * lpMap->m_szLabel[1] + lpMap->m_szLabel[2] - 11 * '0' ;

				if (iFurlong >= 0 && iFurlong < DIMENSION(m_lpFurlongMaps))
					m_lpFurlongMaps[iFurlong] = lpMap ;

				lpMap->m_bSpecialPaint = TRUE ;

			} else if (scumm_stricmp(lpMap->m_szLabel, "Hodj") == 0 || scumm_stricmp(lpMap->m_szLabel, "Podj") == 0) {

				InitOverlay(lpMap) ;    // initialize sprite

			} else if (scumm_stricmp(lpMap->m_szLabel, "Minib1") == 0) {
				lpMap->m_bSpecialPaint = TRUE ;
				m_cBbtMgr.LinkButton(&m_cMiniButton, lpcBgbObject, nullptr) ;

			} else if (scumm_stricmp(lpMap->m_szLabel, "Minib2") == 0) {
				lpMap->m_bSpecialPaint = TRUE;
				m_cBbtMgr.LinkButton(&m_cMiniButton, nullptr, lpcBgbObject);

			} else if (scumm_stricmp(lpMap->m_szLabel, "Invb1") == 0) {
				lpMap->m_bSpecialPaint = TRUE ;
				m_cBbtMgr.LinkButton(&m_cInvButton, lpcBgbObject, nullptr) ;

			} else if (scumm_stricmp(lpMap->m_szLabel, "Invb2") == 0) {
				lpMap->m_bSpecialPaint = TRUE ;
				m_cBbtMgr.LinkButton(&m_cInvButton, nullptr, lpcBgbObject) ;

			} else if (scumm_stricmp(lpMap->m_szLabel, "Scrob1") == 0) {
				lpMap->m_bSpecialPaint = TRUE ;
				m_cBbtMgr.LinkButton(&m_cScrollButton, lpcBgbObject, nullptr) ;

			} else if (scumm_stricmp(lpMap->m_szLabel, "Scrob2") == 0) {
				lpMap->m_bSpecialPaint = TRUE ;
				m_cBbtMgr.LinkButton(&m_cScrollButton, nullptr, lpcBgbObject) ;

			} else if (scumm_stricmp(lpMap->m_szLabel, "podjb1") == 0) {
				assert(m_xpXodjChain != nullptr);
				m_xpXodjChain->m_lpcIcon = lpcBgbObject;

			} else if (scumm_stricmp(lpMap->m_szLabel, "hodjb1") == 0) {
				assert(m_xpXodjChain != nullptr);
				assert(m_xpXodjChain->m_xpXodjNext != nullptr);
				m_xpXodjChain->m_xpXodjNext->m_lpcIcon = lpcBgbObject;
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
	CXodj * xpXodj = nullptr ;

	for (xpXodj = m_xpXodjChain ; xpXodj && scumm_stricmp(lpMap->m_szLabel, xpXodj->m_szName); xpXodj = xpXodj->m_xpXodjNext)
		;       // null loop body

	if (!xpXodj) {              // existing character not found

		if ((xpXodj = new CXodj) == nullptr) {
			iError = 100 ;      // can't allocate character
			goto cleanup ;
		}
		xpXodj->m_xpXodjNext = m_xpXodjChain ;
		// put in front of chain
		m_xpXodjChain = xpXodj ;

		strncpy(xpXodj->m_szName, lpMap->m_szLabel, sizeof(xpXodj->m_szName) - 1);
		xpXodj->m_szName[0] = (char)toupper(xpXodj->m_szName[0]);

		xpXodj->m_lpcCharSprite = lpMap->m_lpcBgbObject ;
		lpMap->m_bSpecialPaint = TRUE ;

		// GTB - 8/25 - I put this in to find and then place hodj and podj at the starting loc
		// you'll find STARTING_LOCATION at the top of this file.

		CNode   *pNode = m_lpNodes;
		int     nTemp = 0;
		do {
			if (strcmp(pNode->m_szLabel, STARTING_LOCATION) == 0) {
				pNode = nullptr;
				break;
			} else {
				pNode++;
				nTemp++;
			}

		} while (pNode != nullptr);

		xpXodj->m_iCharNode = nTemp ;
// GTB - 8/25 end of my change

//        xpXodj->m_iCharNode = -(lpMap - m_lpMaps) - 1 ;
		// kludge forces char to start
		// at castle
		xpXodj->m_bHodj = (scumm_stricmp(lpMap->m_szLabel, "Hodj") == 0) ;

		// initialize theme music info
		//
		if (xpXodj->m_bHodj) {
			xpXodj->m_pszThemeFile = (const char *)&HODJS_WALKING_SOUND;
			xpXodj->m_nThemeStart = HODJ_SOUND_START;
			xpXodj->m_nThemeEnd = HODJ_SOUND_END;
			xpXodj->m_pszStarsFile = (const char *)&HODJS_STARS_FILE;
		} else {
			xpXodj->m_pszThemeFile = (const char *)&PODJS_WALKING_SOUND;
			xpXodj->m_nThemeStart = PODJ_SOUND_START;
			xpXodj->m_nThemeEnd = PODJ_SOUND_END;
			xpXodj->m_pszStarsFile = (const char *)&PODJS_STARS_FILE;
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
		lpPrevMap = nullptr ;
		lpcPrevBgbObject = nullptr ;

		if (lpMap->m_iRelation >= 0 && lpMap->m_iRelation < m_iMaps) {
			lpPrevMap = m_lpMaps + lpMap->m_iRelation ;
			// get previous map pointer
			lpcPrevBgbObject = lpPrevMap->m_lpcBgbObject ;
		}

		if (!lpMap->m_bPositionDetermined && lpMap->m_iRelationType && ((lpcBgbObject = lpMap->m_lpcBgbObject) != nullptr)) {

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
		((CGtlDoc *)m_xpcGtlDoc)->UpdateAllViews(xpGtlView, HINT_SIZE, nullptr);

cleanup:

	JXELEAVE(CGtlData::NormalizeData) ;
	RETURN(iError != 0) ;
}

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
