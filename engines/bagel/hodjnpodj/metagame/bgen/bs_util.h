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

#define SB_HORZ 1
#define SB_VERT 2

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
	int x;
	int y;

	CPoint(int xv = 0, int yv = 0) : x(xv), y(yv) {
	}
	CPoint(const Common::Point &pt) : x(pt.x), y(pt.y) {
	}
};

// CBsuSet -- boffo scroll utility set
class CBsuSet {
private:
	/**
	 * Get bsu scroll bar object
	 * @param iBarType	BSCT_xxxx -- scroll bar type
	 * @returns			pointer to scroll bar object, or nullptr if not found
	 */
	CBsuBar *GetBar(int iBarType);

public:
    CBsuBar *m_xpBarChain = nullptr;	// Chain of scroll bar objects
    bool m_bInUpdateBars = false;		// Prevent UpdateBars recursion
    CDialog *m_xpDlg = nullptr;			// Dialog to be updated
    View *m_xpWnd = nullptr;			// Window to be scrolled

    bool m_bDebugMessages = false;
    bool m_bPrimary = false;			// Primary in linked sets
    bool m_bScrollView = false;			// Window uses CScrollView
    bool m_bScrollBars = false;			// Window to have windows scroll bars
    CBsuSet *m_xpSetLink = nullptr;		// One alternate set of scroll bars

public:
    CBsuSet() {}
	~CBsuSet();

	/**
	 * Initialize bsu set for a window
	 * @param xpWnd		Window for which scroll bar set is to be initialized
	 * @param bScrollView	true if window is derived from CScrollView
	 * @param bScrollBars	true if window should have scroll bars
	 * @param xpLinkSet -- bsu set for 2nd set of scroll bars for same window
	 * @returns			true if error, false otherwise
	 */
	bool InitWndBsuSet(View *xpWnd, bool bScrollView = false,
		bool bScrollBars = false, CBsuSet *xpLinkSet = nullptr);

	/**
	 * Initialize bsu set for dialog box
	 */
	bool InitDlgBsuSet(CDialog *xpDlg,
		CBsuSet *xpLinkSet = nullptr) ;

	/**
	 * Add scroll bar to scroll bar set
	 */
	bool AddBarToSet(int iId, int iWndScrollCode,
			int iBarType = 0) ;

	/**
	 * Prepare window scroll bar set by filling in
	 * the device fields
	 * @param cDocSize		Document size (logical units)
	 * @param cScrollRect	The rectangle defining the interior scrolling
	 * region in logical coordinates, with bottom/right being
	 * positive values indicating margins (logical units)
	 * @returns				true if error, false otherwise 
	 */
	bool PrepareWndBsuSet(CSize cDocSize, const Common::Rect &cScrollRect);

	/**
	 * Update window devices coordinates
	 */
	bool UpdateWndDeviceExtents();

	/**
	 * Link window/dialog bsu sets
	 */
	bool LinkWndBsuSet();

	/**
	 * Replace OnPrepareDC -- set the viewport and
	 * the clip rectangle to the specified region
	 * @param xpDc			Device context
	 * @param bRelocatable	If false, set to whole window; if true,
	 * set to the interior region, which is scrolled and might be remapped
	 * @param lpClipRect	Pointer to clipping rectangle; if nullptr, use whole window
	 * @param returns		true if error, false otherwise
	 */
	bool PrepareDc(CDC *xpDc, bool bRelocatable = true);

	/**
	 * Handle OnHScroll and OnVScroll messages
	 * @param nSBCode		SB_xxxx -- Windows scroll code
	 * @param nPos			Position on scroll bar (SB_THUMBTRACT/SB_THUMBPOSITION)
	 * @param xpScrollBar	Pointer to scroll bar control
	 * @param iBarType		BSCT_xxxx -- scroll bar type
	 * @returns				true if error, false otherwise
	 */
	bool OnScroll(uint nSBCode, uint nPos,
		ScrollBar *xpScrollBar, int iBarType = 0);

	/**
	 * Scroll window to spec point
	 * @param cScrollPosition	Point to which window is to be scrolled
	 * @param bScrollWindow		If true, then scroll window.  (If false, then just
	 * set the variables and scroll bars)
	 * @returns					true if error, false otherwise
	 */
	bool ScrollWindowToPoint(const Common::Point &cScrollPosition,
		bool bScrollWindow = true);

	/**
	 * If point is on edge, scroll it to center
	 * @param cPoint	The point to be tested
	 * @returns			true if error, false otherwise
	 */
	bool EdgeToCenter(const Common::Point &cPoint, bool bScroll = false);

	/**
	 * Set rectangle to portion of window
	 * (logical coordinates)
	 * @param lpRect	Where results are to be stored
	 * @param iBsRegion	Combination of BSCRn bits, based on the window margins,
	 *	according to the following diagram:
	 *		123
	 *		456
	 *		789
	 *	Typical values are: "123" for the top margin, "5" for the
	 *	interior scrolling region, and so forth.  Not all
	 *	combinations are supported.
	 * returns: true if error, false otherwise
	 */
	bool SetSubWindowRect(Common::Rect *lpRect, int iBsRegion);

	/**
	 * Test whether rectangle is in window
	 * @param crTestRect		Relocatable rectangle to be tested
	 * @param bPhysical			(output) -- if true, then rectangle lies at least
	 * partially within physical window
	 * @param bEdge (output)	if true, then rectangle is on the edge
	 * (partially in window, partially outside window)
	 * @returns					true if error, false otherwise
	 */
	bool TestRect(CRRect crTestRect,
		bool &bPhysical, bool &bEdge);

	/**
	 * Set rectangle to portion of window
	 * (device coordinates)
	 * @param xpHBar, xpVBar	(output) -- horizontal/vertical scroll bar object
	 * @param bErrorRtn			if false, don't generate "not allocated" error returns
	 * @returns		true if error, false otherwise
	 */
	bool GetWindowBars(CBsuBar *&xpHBar,
		CBsuBar *&xpVBar, bool bErrorRtn = true);

	/**
	 * Convert device point to logical coords
	 * @param cPoint	Point in device coordinates
	 * @returns			Relocatable point
	 */
	CRPoint PointLogical(const Common::Point &cPoint);

	/**
	 * Get information about scroll set
	 */
	bool GetInfo(CBsuInfo *xpBsuInfo);

	/**
	 * Dump information about scroll set
	 */
	bool DumpInfo(const char *lpStart = nullptr);
};

/**
 * CBsuBar -- scroll bar
 */
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

/**
 * CBsuInfo -- information returned by GetInfo
 */
class CBsuInfo {
public:
    CSize m_cWndSize, m_cTotalSize;
    CSize m_cDevWndSize, cDevTotalSize;
    Common::Rect m_cScrollRangeRect;
    Common::Point m_cScrollPosition;
};

/**
 * CRPoint -- relocatable point
 */
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
		return CRPoint(x+cPt.x, y+cPt.y, m_bRelocatable);
	}
};

/**
 * CRRect -- relocatable rectangle
 */
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
		Common::Rect(cPt.x, cPt.y, cPt.x + cSz.cx, cPt.y + cSz.cy),
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
			&& Common::Rect::contains(crPoint.x, crPoint.y);
	}
};

#define HINT_UPDATE_RECT 1
#define HINT_UPDATE_FULL 2
#define HINT_INIT_METAGAME 3
#define HINT_SIZE 4
#define HINT_SCROLL 5

/**
 * CGtlHint -- hints for updating views -- used in calls
 * to CView::OnUpdate
 */
class CGtlHint : public CObject {
public:
    bool m_bWmPaint = false;	// update with WM_PAINT message
    Common::Rect cHintRect;
};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
