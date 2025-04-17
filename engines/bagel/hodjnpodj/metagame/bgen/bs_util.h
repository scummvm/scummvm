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

#ifndef HODJNPODJ_METAGAME_BGEN_BS_UTIL_H
#define HODJNPODJ_METAGAME_BGEN_BS_UTIL_H

#include "common/algorithm.h"
#include "common/rect.h"
#include "bagel/boflib/stdinc.h"
#include "bagel/hodjnpodj/gfx/scrollbar.h"
#include "bagel/hodjnpodj/views/view.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

// define scroll bar types (should not conflict with IDC_xxxx codes
//	from resource.h)
#define BSCT_GEN 1		/* general (no special type) */
#define BSCT_HORZ 2		/* horizontal scroll bar for window */
#define BSCT_VERT 3		/* vertical scroll bar for window */
#define BSCT_EDIT 4		/* tied to numeric value in edit box */

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

struct CSize {
	int cx;
	int cy;

	CSize(int xv = 0, int yv = 0) : cx(xv), cy(yv) {
	}
};

struct CPoint {
	int cx;
	int cy;

	CPoint(int xv = 0, int yv = 0) : cx(xv), cy(yv) {
	}
	CPoint(const Common::Point &pt) : cx(pt.x), cy(pt.y) {
	}
};

// CBsuSet -- boffo scroll utility set
class CBsuSet {
public:
    CBsuBar *m_xpBarChain = nullptr;	// chain of scroll bar objects
    bool m_bInUpdateBars = false;		// prevent UpdateBars recursion
    CDialog *m_xpDlg = nullptr;			// dialog to be updated
    CWnd *m_xpWnd = nullptr;			// window to be scrolled
#ifdef NODEEDIT
    int m_iScrollCount ;	// # of times scrolled
#endif
    bool m_bDebugMessages = false;
    bool m_bPrimary = false;			// primary in linked sets
    bool m_bScrollView = false;			// window uses CScrollView
    bool m_bScrollBars = false;			// window to have windows scroll bars
    CBsuSet *m_xpSetLink = nullptr;		// one alternate set of scroll bars

// methods
public:
    CBsuSet() {}
#ifdef NODEEDIT
    int GetScrollCount(void) {return(m_iScrollCount) ;}
#endif

// bsutl.cpp : Boffo scroll bar utilities

//- ~CBsuSet -- destructor
public:
	~CBsuSet();
//- InitWndBsuSet -- initialize bsu set for a window
public:
	bool InitWndBsuSet(View *xpWnd,
		bool bScrollView PDFT(false), bool bScrollBars PDFT(false),
	CBsuSet * xpLinkSet PDFT(nullptr)) ;
//- InitDlgBsuSet -- initialize bsu set for dialog box
public: bool InitDlgBsuSet(CDialog * xpDlg,
	CBsuSet * xpLinkSet PDFT(nullptr)) ;
//- AddBarToSet -- add scroll bar to scroll bar set
public: bool AddBarToSet(int iId, int iWndScrollCode,
			int iBarType PDFT(0)) ;
//- PrepareWndBsuSet -- prepare window scroll bar set
//		by filling in the device fields
public: bool PrepareWndBsuSet(CSize cDocSize, const Common::Rect &cScrollRect) ;
//- UpdateWndDeviceExtents -- update window devices coordinates
private: bool UpdateWndDeviceExtents(void);
//- LinkWndBsuSet -- link window/dialog bsu sets
public: bool LinkWndBsuSet(void) ;
//- PrepareDc -- replace OnPrepareDC -- set the viewport and
//	the clip rectangle to the specified region
public: bool PrepareDc(CDC *xpDc, bool bRelocatable PDFT(true));
//- OnScroll -- handle OnHScroll and OnVScroll messages
public: bool OnScroll(uint nSBCode, uint nPos,
		ScrollBar *xpScrollBar, int iBarType PDFT(0)) ;
//- GetBar -- get bsu scroll bar object
private: CBsuBar * GetBar(int iBarType) ;
//- ScrollWindowToPoint -- scroll window to spec point
public: bool ScrollWindowToPoint(const Common::Point &cScrollPosition,
		bool bScrollWindow PDFT(true)) ;
//- EdgeToCenter -- if point is on edge, scroll it to center
public: bool EdgeToCenter(const Common::Point &cPoint, bool bScroll PDFT(false));
//- SetSubWindowRect -- set rectangle to portion of window
//		(logical coordinates)
public: bool SetSubWindowRect(Common::Rect *lpRect, int iBsRegion);
//- TestRect -- test whether rectangle is in window
public: bool TestRect(CRRect crTestRect,
       			bool & bPhysical, bool & bEdge) ;
//- GetWindowBars -- set rectangle to portion of window
//		(device coordinates)
public: bool GetWindowBars(CBsuBar *& xpHBar,
		CBsuBar *& xpVBar, bool bErrorRtn PDFT(true)) ;
//- PointLogical -- convert device point to logical coords
public: CRPoint PointLogical(const Common::Point &cPoint) ;
//- GetInfo -- get information about scroll set
public: bool GetInfo(CBsuInfo * xpBsuInfo) ;
//- DumpInfo -- dump information about scroll set
public: bool DumpInfo(const char *lpStart PDFT(nullptr)) ;

} ;


// CBsuBar -- scroll bar
class CBsuBar {
public:
	friend class CBsuSet;

    CBsuBar *m_xpNextBar = nullptr;	// next bar for this set
    int m_iBarType = 0;			// BSCT_xxxx -- scroll bar type
    int m_iWndScrollCode = 0;	// windows scroll code --
			// SB_HORZ (=0) or SB_VERT (=1)
    int m_iId = 0;				// control id
//  CScrollBar * m_xpScrollBar ;	// MFC scroll bar object
    int m_iMin = 0, m_iMax = 0, m_iPosition = 0;	// scroll bar values
//  int m_iDevPosition ;	// position in device units
    int *m_lpiVariable = nullptr;	// ptr to integer variable
    int m_iDocSize = 0;		// document size (logical units)
//  int m_iDevDocSize ;		// ditto (device units)
    int m_iDevWndSize = 0;		// window size (device units)
    int m_iWndSize = 0;		// window size (logical units)
    int m_iMargin1 = 0, m_iMargin2 = 0;	// left/top and right/bottom
				// scroll margins (logical units)
    int m_iDevMargin1 = 0, m_iDevMargin2 = 0;	// ditto (device units)
} ;

// CBsuInfo -- information returned by GetInfo
class CBsuInfo {
public:
    CSize m_cWndSize, m_cTotalSize;
    CSize m_cDevWndSize, cDevTotalSize;
    Common::Rect m_cScrollRangeRect;
    Common::Point m_cScrollPosition;
#ifdef NODEEDIT
    int m_iScrollCount ;
#endif
};

// CRPoint -- relocatable point
class CRPoint : public CPoint {
public:
    bool m_bRelocatable;

    CRPoint(bool bRelocatable = true) :
		m_bRelocatable(bRelocatable) {
	}

    CRPoint(const Common::Point &cPt,
		bool bRelocatable = true) : CPoint(cPt),
		m_bRelocatable(bRelocatable) {
	}

    CRPoint(int iX, int iY, bool bRelocatable = true) : CPoint(iX, iY),
		m_bRelocatable(bRelocatable) {
	}

    bool IfRelocatable() const {
		return m_bRelocatable;
	}

    CRPoint Offset(const Common::Point &cPt) {
		return CRPoint(cx+cPt.x, cy+cPt.y, m_bRelocatable);
	}
};

// CRRect -- relocatable rectangle
class CRRect : public Common::Rect {
public:
    bool m_bRelocatable;

    CRRect(bool bRelocatable = true) : Common::Rect(),
		m_bRelocatable(bRelocatable) {
	}

    CRRect(const Common::Rect &cRc, bool bRelocatable = true) :
		Common::Rect(cRc), m_bRelocatable(bRelocatable) {
	}

    CRRect(const CRPoint &cPt, const CSize &cSz) :
		Common::Rect(cPt.cx, cPt.cy, cPt.cx + cSz.cx, cPt.cy + cSz.cy),
		m_bRelocatable(cPt.m_bRelocatable) {
	}

    CRRect(int iL, int iT, int iR, int iB,
		bool bRelocatable = true) :
		Common::Rect(iL, iT, iR, iB), m_bRelocatable(bRelocatable) {
	}

	bool IfRelocatable() const {
		return m_bRelocatable;
	}

    bool PtInRect(const CRPoint &crPoint) {
		return m_bRelocatable == crPoint.m_bRelocatable
			&& Common::Rect::contains(crPoint.cx, crPoint.cy);
	}
};

#define HINT_UPDATE_RECT 1
#define HINT_UPDATE_FULL 2
#define HINT_INIT_METAGAME 3
#define HINT_SIZE 4
#define HINT_SCROLL 5

// class CGtlHint -- hints for updating views -- used in calls
//		to CView::OnUpdate
class CGtlHint : public CObject {
public:
    bool m_bWmPaint = false;	// update with WM_PAINT message
    Common::Rect cHintRect;
};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
