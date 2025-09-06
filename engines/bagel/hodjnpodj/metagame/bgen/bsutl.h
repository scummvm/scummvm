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
namespace Gtl {

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

class CBsuBar;
class CBsuInfo;
class CRPoint;
class CRRect;

// CBsuSet -- boffo scroll utility set
class CBsuSet {
public:
	char m_cStartData;
	class CBsuBar *m_xpBarChain;   // chain of scroll bar objects
	bool m_bInUpdateBars;  // prevent UpdateBars recursion
	CDialog *m_xpDlg;      // dialog to be updated
	CWnd *m_xpWnd;     // window to be scrolled

	bool m_bDebugMessages;
	bool m_bPrimary;   // primary in linked sets
	bool m_bScrollView;    // window uses CScrollView
	bool m_bScrollBars;    // window to have windows scroll bars
	CBsuSet *m_xpSetLink;  // one alternate set of scroll bars
	char m_cEndData;

public:
	CBsuSet() {
		TRACECONSTRUCTOR(CBsuSet);
		memset(&m_cStartData, 0,
			&m_cEndData - &m_cStartData);
	}

	~CBsuSet();
	//- InitWndBsuSet -- initialize bsu set for a window
	bool InitWndBsuSet(CWnd *xpWnd,
		bool bScrollView PDFT(false), bool bScrollBars PDFT(false),
		CBsuSet *xpLinkSet PDFT(nullptr));
	//- InitDlgBsuSet -- initialize bsu set for dialog box
	bool InitDlgBsuSet(CDialog *xpDlg,
		CBsuSet *xpLinkSet PDFT(nullptr));
	//- AddBarToSet -- add scroll bar to scroll bar set
	bool AddBarToSet(int iId, int iWndScrollCode,
		int iBarType PDFT(0));
	//- PrepareWndBsuSet -- prepare window scroll bar set
	//      by filling in the device fields
	bool PrepareWndBsuSet(CSize cDocSize, CRect cScrollRect);
	//- UpdateWndDeviceExtents -- update window devices coordinates
	bool UpdateWndDeviceExtents();
	//- LinkWndBsuSet -- link window/dialog bsu sets
	bool LinkWndBsuSet();
	//- PrepareDc -- replace OnPrepareDC -- set the viewport and
	//  the clip rectangle to the specified region
	bool PrepareDc(CDC *xpDc, bool bRelocatable PDFT(true));
	//- OnScroll -- handle OnHScroll and OnVScroll messages
	bool OnScroll(unsigned int nSBCode, unsigned int nPos,
		CScrollBar *xpScrollBar, int iBarType PDFT(0));
	//- GetBar -- get bsu scroll bar object
	CBsuBar *GetBar(int iBarType);
	//- ScrollWindowToPoint -- scroll window to spec point
	bool ScrollWindowToPoint(CPoint cScrollPosition,
		bool bScrollWindow PDFT(true));
	//- EdgeToCenter -- if point is on edge, scroll it to center
	bool EdgeToCenter(CPoint cPoint, bool bScroll PDFT(false));
	//- SetSubWindowRect -- set rectangle to portion of window
	//      (logical coordinates)
	bool SetSubWindowRect(LPRECT lpRect, int iBsRegion);
	//- TestRect -- test whether rectangle is in window
	bool TestRect(CRRect crTestRect,
		bool &bPhysical, bool &bEdge);
	//- GetWindowBars -- set rectangle to portion of window
	//      (device coordinates)
	bool GetWindowBars(CBsuBar *&xpHBar,
		CBsuBar *&xpVBar, bool bErrorRtn PDFT(true));
	//- PointLogical -- convert device point to logical coords
	CRPoint PointLogical(CPoint cPoint);
	//- GetInfo -- get information about scroll set
	bool GetInfo(CBsuInfo *xpBsuInfo);
	//- DumpInfo -- dump information about scroll set
	bool DumpInfo(char *lpStart PDFT(nullptr));
};


// CBsuBar -- scroll bar
class CBsuBar {
private:
	friend class CBsuSet;

	CBsuBar *m_xpNextBar = nullptr;  // next bar for this set
	int m_iBarType = 0;        // BSCT_xxxx -- scroll bar type
	int m_iWndScrollCode = 0;  // windows scroll code --
	// SB_HORZ (=0) or SB_VERT (=1)
	int m_iId = 0;         // control id
	int m_iMin = 0, m_iMax = 0, m_iPosition = 0;   // scroll bar values
	int *m_lpiVariable = 0;   // ptr to integer variable
	int m_iDocSize = 0;        // document size (logical units)
	int m_iDevWndSize = 0;     // window size (device units)
	int m_iWndSize = 0;        // window size (logical units)
	int m_iMargin1 = 0, m_iMargin2 = 0;    // left/top and right/bottom
	// scroll margins (logical units)
	int m_iDevMargin1 = 0, m_iDevMargin2 = 0;  // ditto (device units)
};

// CBsuInfo -- information returned by GetInfo
class CBsuInfo {
public:
	char m_cStartData;
	CSize m_cWndSize, m_cTotalSize;
	CSize m_cDevWndSize, cDevTotalSize;
	CRect m_cScrollRangeRect;
	CPoint m_cScrollPosition;

	char m_cEndData;
};

// CRPoint -- relocatable point
class CRPoint : public CPoint {
public:
	bool m_bRelocatable;

	CRPoint(bool bRelocatable = true) {
		m_bRelocatable = (bRelocatable != 0);
	}

	CRPoint(CPoint cPt, bool bRelocatable = true) : CPoint(cPt) {
		m_bRelocatable = (bRelocatable != 0);
	}

	CRPoint(int iX, int iY, bool bRelocatable = true) : CPoint(iX, iY) {
		m_bRelocatable = (bRelocatable != 0);
	}

	bool IfRelocatable() const {
		return m_bRelocatable != 0;
	}

	CRPoint Offset(const CPoint &cPt) const {
		return CRPoint(x + cPt.x, y + cPt.y, m_bRelocatable);
	}
};

// CRRect -- relocatable rectangle
class CRRect : public CRect {
public:
	bool m_bRelocatable;

	CRRect(bool bRelocatable = true) {
		m_bRelocatable = (bRelocatable != 0);
	}

	CRRect(CRect cRc, bool bRelocatable = true) : CRect(cRc) {
		m_bRelocatable = (bRelocatable != 0);
	}

	CRRect(CRPoint cPt, CSize cSz) : CRect(cPt, cSz) {
		m_bRelocatable = (cPt.m_bRelocatable != 0);
	}

	CRRect(int iL, int iT, int iR, int iB,
		bool bRelocatable = true) : CRect(iL, iT, iR, iB) {
		m_bRelocatable = (bRelocatable != 0);
	}

	bool IfRelocatable() const {
		return m_bRelocatable != 0;
	}

	bool PtInRect(const CRPoint &crPoint) {
		return EQV(m_bRelocatable, crPoint.m_bRelocatable)
			&& CRect::PtInRect(crPoint);
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
	char m_cStartData;
	//    bool m_bFull ;    // full screen invalidate
	bool m_bWmPaint;   // update with WM_PAINT message
	CRect cHintRect;
	char m_cEndData;

	CGtlHint() {
		memset(&m_cStartData,
			0, &m_cEndData - &m_cStartData);
	}
	//    DECLARE_SERIAL(CGtlHint) ;
};

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
