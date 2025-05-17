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

#ifndef BAGEL_METAGAME_BGEN_BSUTL_H
#define BAGEL_METAGAME_BGEN_BSUTL_H

#include "bagel/hodjnpodj/metagame/bgen/stdafx.h"
#include "bagel/hodjnpodj/metagame/bgen/bgen.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {


// define scroll bar types (should not conflict with IDC_xxxx codes
//	from resource.h)
#define BSCT_GEN 1      
/* general (no special type) */
#define BSCT_HORZ 2     
/* horizontal scroll bar for window */
#define BSCT_VERT 3     
/* vertical scroll bar for window */
#define BSCT_EDIT 4     
/* tied to numeric value in edit box */

// window regions defined according to the following diagram:
//		123
//		456
//		789
#define BSCR1 0x001
#define BSCR2 0x002
#define BSCR3 0x004
#define BSCR4 0x008
#define BSCR5 0x010
#define BSCR6 0x020
#define BSCR7 0x040
#define BSCR8 0x080
#define BSCR9 0x100

#define BSCR123 (BSCR1 | BSCR2 | BSCR3)
#define BSCR456 (BSCR4 | BSCR5 | BSCR6)
#define BSCR789 (BSCR7 | BSCR8 | BSCR9)
#define BSCR147 (BSCR1 | BSCR4 | BSCR7)
#define BSCR258 (BSCR2 | BSCR5 | BSCR8)
#define BSCR369 (BSCR3 | BSCR6 | BSCR9)
#define BSCRALL (BSCR123 | BSCR456 | BSCR789)

class CBsuBar ;
class CBsuInfo ;
class CRPoint ;
class CRRect ;

// CBsuSet -- boffo scroll utility set
class CBsuSet {
public:
	char m_cStartData ;
	class CBsuBar *m_xpBarChain ;   // chain of scroll bar objects
	BOOL m_bInUpdateBars ;  // prevent UpdateBars recursion
	CDialog *m_xpDlg ;      // dialog to be updated
	CWnd *m_xpWnd ;     // window to be scrolled
	#ifdef NODEEDIT
	int m_iScrollCount ;    // # of times scrolled
	#endif
	BOOL m_bDebugMessages ;
	BOOL m_bPrimary ;   // primary in linked sets
	BOOL m_bScrollView ;    // window uses CScrollView
	BOOL m_bScrollBars ;    // window to have windows scroll bars
	CBsuSet *m_xpSetLink ;  // one alternate set of scroll bars
	char m_cEndData ;

// methods
public:
	CBsuSet() {
		TRACECONSTRUCTOR(CBsuSet) ;
		memset(&m_cStartData, 0,
		       &m_cEndData - &m_cStartData) ;
	}
	#ifdef NODEEDIT
	int GetScrollCount(void) {
		return (m_iScrollCount) ;
	}
	#endif

// bsutl.cpp : Boffo scroll bar utilities

//- ~CBsuSet -- destructor
public:
	~CBsuSet(void) ;
//- InitWndBsuSet -- initialize bsu set for a window
public:
	BOOL InitWndBsuSet(CWnd * xpWnd,
	                            BOOL bScrollView PDFT(FALSE), BOOL bScrollBars PDFT(FALSE),
	                            CBsuSet * xpLinkSet PDFT(NULL)) ;
//- InitDlgBsuSet -- initialize bsu set for dialog box
public:
	BOOL InitDlgBsuSet(CDialog * xpDlg,
	                            CBsuSet * xpLinkSet PDFT(NULL)) ;
//- AddBarToSet -- add scroll bar to scroll bar set
public:
	BOOL AddBarToSet(int iId, int iWndScrollCode,
	                          int iBarType PDFT(0)) ;
//- PrepareWndBsuSet -- prepare window scroll bar set
//		by filling in the device fields
public:
	BOOL PrepareWndBsuSet(CSize cDocSize, CRect cScrollRect) ;
//- UpdateWndDeviceExtents -- update window devices coordinates
private:
	BOOL UpdateWndDeviceExtents(void) ;
//- LinkWndBsuSet -- link window/dialog bsu sets
public:
	BOOL LinkWndBsuSet(void) ;
//- PrepareDc -- replace OnPrepareDC -- set the viewport and
//	the clip rectangle to the specified region
public:
	BOOL PrepareDc(CDC *xpDc, BOOL bRelocatable PDFT(TRUE));
//- OnScroll -- handle OnHScroll and OnVScroll messages
public:
	BOOL OnScroll(UINT nSBCode, UINT nPos,
	                       CScrollBar* xpScrollBar, int iBarType PDFT(0)) ;
//- GetBar -- get bsu scroll bar object
private:
	CBsuBar *GetBar(int iBarType) ;
//- ScrollWindowToPoint -- scroll window to spec point
public:
	BOOL ScrollWindowToPoint(CPoint cScrollPosition,
	                                  BOOL bScrollWindow PDFT(TRUE)) ;
//- EdgeToCenter -- if point is on edge, scroll it to center
public:
	BOOL EdgeToCenter(CPoint cPoint, BOOL bScroll PDFT(FALSE));
//- SetSubWindowRect -- set rectangle to portion of window
//		(logical coordinates)
public:
	BOOL SetSubWindowRect(LPRECT lpRect, int iBsRegion);
//- TestRect -- test whether rectangle is in window
public:
	BOOL TestRect(CRRect crTestRect,
	                       BOOL & bPhysical, BOOL & bEdge) ;
//- GetWindowBars -- set rectangle to portion of window
//		(device coordinates)
public:
	BOOL GetWindowBars(CBsuBar * &xpHBar,
	                            CBsuBar *&xpVBar, BOOL bErrorRtn PDFT(TRUE)) ;
//- PointLogical -- convert device point to logical coords
public:
	CRPoint PointLogical(CPoint cPoint) ;
//- GetInfo -- get information about scroll set
public:
	BOOL GetInfo(CBsuInfo * xpBsuInfo) ;
//- DumpInfo -- dump information about scroll set
public:
	BOOL DumpInfo(LPSTR lpStart PDFT(NULL)) ;

} ;


// CBsuBar -- scroll bar
class CBsuBar {
	friend class CBsuSet ;

	char m_cStartData ;
	CBsuBar *m_xpNextBar ;  // next bar for this set
	int m_iBarType ;        // BSCT_xxxx -- scroll bar type
	int m_iWndScrollCode ;  // windows scroll code --
	// SB_HORZ (=0) or SB_VERT (=1)
	int m_iId ;         // control id
//  CScrollBar * m_xpScrollBar ;    // MFC scroll bar object
	int m_iMin, m_iMax, m_iPosition ;   // scroll bar values
//  int m_iDevPosition ;    // position in device units
	LPINT m_lpiVariable ;   // ptr to integer variable
	int m_iDocSize ;        // document size (logical units)
//  int m_iDevDocSize ;     // ditto (device units)
	int m_iDevWndSize ;     // window size (device units)
	int m_iWndSize ;        // window size (logical units)
	int m_iMargin1, m_iMargin2 ;    // left/top and right/bottom
	// scroll margins (logical units)
	int m_iDevMargin1, m_iDevMargin2 ;  // ditto (device units)

	char m_cEndData ;

// methods
public:
	CBsuBar() {
		memset(&m_cStartData, 0,
		       &m_cEndData - &m_cStartData) ;
	}
};

// CBsuInfo -- information returned by GetInfo
class CBsuInfo {
public:
	char m_cStartData ;
	CSize m_cWndSize, m_cTotalSize ;
	CSize m_cDevWndSize, cDevTotalSize ;
	CRect m_cScrollRangeRect ;
	CPoint m_cScrollPosition ;
	#ifdef NODEEDIT
	int m_iScrollCount ;
	#endif
	char m_cEndData ;
} ;

// CRPoint -- relocatable point
class CRPoint : public CPoint {
public:
	BOOL m_bRelocatable ;

	CRPoint(BOOL bRelocatable = TRUE) {
		m_bRelocatable = (bRelocatable != 0) ;
	}

	CRPoint(CPoint cPt, BOOL bRelocatable = TRUE) : CPoint(cPt) {
		m_bRelocatable = (bRelocatable != 0) ;
	}

	CRPoint(int iX, int iY, BOOL bRelocatable = TRUE) : CPoint(iX, iY) {
		m_bRelocatable = (bRelocatable != 0) ;
	}

	BOOL IfRelocatable(void) {
		return (m_bRelocatable != 0) ;
	}

	CRPoint Offset(CPoint cPt) {
		return (CRPoint(x + cPt.x, y + cPt.y, m_bRelocatable)) ;
	}

} ;

// CRRect -- relocatable rectangle
class CRRect : public CRect {
public:
	BOOL m_bRelocatable ;

	CRRect(BOOL bRelocatable = TRUE) {
		m_bRelocatable = (bRelocatable != 0) ;
	}

	CRRect(CRect cRc, BOOL bRelocatable = TRUE) : CRect(cRc) {
		m_bRelocatable = (bRelocatable != 0) ;
	}

	CRRect(CRPoint cPt, CSize cSz) : CRect(cPt, cSz) {
		m_bRelocatable = (cPt.m_bRelocatable != 0) ;
	}

	CRRect(int iL, int iT, int iR, int iB,
	       BOOL bRelocatable = TRUE) : CRect(iL, iT, iR, iB) {
		m_bRelocatable = (bRelocatable != 0) ;
	}

	BOOL IfRelocatable(void) {
		return (m_bRelocatable != 0) ;
	}

	BOOL PtInRect(CRPoint crPoint) {
		return (EQV(m_bRelocatable, crPoint.m_bRelocatable)
		        && CRect::PtInRect(crPoint)) ;
	}
} ;

#define HINT_UPDATE_RECT 1
#define HINT_UPDATE_FULL 2
#define HINT_INIT_METAGAME 3
#define HINT_SIZE 4
#define HINT_SCROLL 5

// class CGtlHint -- hints for updating views -- used in calls
//		to CView::OnUpdate
class CGtlHint : public CObject {
public:
	char m_cStartData ;
//    BOOL m_bFull ;    // full screen invalidate
	BOOL m_bWmPaint ;   // update with WM_PAINT message
	CRect cHintRect ;
	char m_cEndData ;

	CGtlHint() {
		memset(&m_cStartData,
		       0, &m_cEndData - &m_cStartData) ;
	}
//    DECLARE_SERIAL(CGtlHint) ;
} ;

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
