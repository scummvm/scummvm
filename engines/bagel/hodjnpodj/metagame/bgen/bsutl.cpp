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
#include "bagel/hodjnpodj/metagame/bgen/bfc.h"
#include "bagel/hodjnpodj/metagame/bgen/bsutl.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

extern  CBfcMgr *lpMetaGameStruct;
extern int gnFurlongs;

static  bool    bScrollTemp;

///DEFS bsutl.h

//* CBsuSet::~CBsuSet -- destructor
CBsuSet::~CBsuSet()
// returns: true if error, false otherwise
{
	JXENTER(CBsuSet::~CBsuSet);
	CBsuBar *xpBar;

	TRACEDESTRUCTOR(CBsuSet);

	while ((xpBar = m_xpBarChain) != nullptr) {

		m_xpBarChain = xpBar->m_xpNextBar;
		delete xpBar;
	}

	if (m_xpSetLink)
		m_xpSetLink->m_xpSetLink = nullptr;

	m_xpSetLink = nullptr;

	// cleanup:

	JXELEAVE(CBsuSet::~CBsuSet);
	RETURN_VOID;
}

//* CBsuSet::InitWndBsuSet -- initialize bsu set for a window
bool CBsuSet::InitWndBsuSet(CWnd *xpWnd, bool bScrollView, bool bScrollBars, CBsuSet *xpLinkSet)
// xpWnd -- window for which scroll bar set is to be initialized
// bScrollView -- true if window is derived from CScrollView
// bScrollBars -- true if window should have scroll bars
// xpLinkSet -- bsu set for 2nd set of scroll bars for same window
// returns: true if error, false otherwise
{
	JXENTER(CBsuSet::InitWndBsuSet);
	int iError = 0;        // error code
	CBsuBar *xpHBar = nullptr, *xpVBar = nullptr;

	GetWindowBars(xpHBar, xpVBar, false);  // try to get existing
	// scroll bar objects, ignore error

	if (!xpHBar) {
		if ((xpHBar = new CBsuBar) == nullptr) {
			iError = 100;
			goto cleanup;
		} else {
			xpHBar->m_xpNextBar = m_xpBarChain;
			m_xpBarChain = xpHBar;
		}
	}

	if (!xpVBar) {
		if ((xpVBar = new CBsuBar) == nullptr) {
			iError = 101;
			goto cleanup;
		} else {
			xpVBar->m_xpNextBar = m_xpBarChain;
			m_xpBarChain = xpVBar;
		}
	}

	m_xpWnd = xpWnd;
	m_bScrollView = bScrollView;
	m_bScrollBars = bScrollBars;

	xpHBar->m_iBarType = BSCT_HORZ;
	xpVBar->m_iBarType = BSCT_VERT;

	xpHBar->m_iWndScrollCode = SB_HORZ;
	xpVBar->m_iWndScrollCode = SB_VERT;

	if (xpLinkSet) {
		xpLinkSet->m_xpSetLink = this;
		m_xpSetLink = xpLinkSet;
	}

	if ((xpLinkSet = m_xpSetLink) != nullptr) {

		if (!xpLinkSet->m_xpWnd)
			xpLinkSet->m_xpWnd = xpWnd;

		if (!xpWnd)
			m_xpWnd = xpLinkSet->m_xpWnd;

	} else {
		m_bPrimary = true;
	}

cleanup:

	JXELEAVE(CBsuSet::InitWndBsuSet);
	RETURN(iError != 0);
}

//* CBsuSet::PrepareWndBsuSet -- prepare window scroll bar set
//		by filling in the device fields
bool CBsuSet::PrepareWndBsuSet(CSize cDocSize, CRect cScrollRect)
// CDocSize -- document size (logical units)
// CScrollRect -- the rectangle defining the interior scrolling
//	region in logical coordinates, with bottom/right being
//	positive values indicating margins (logical units)
// returns: true if error, false otherwise
{
	JXENTER(CBsuSet::PrepareWndBsuSet);
	int iError = 0;        // error code
	int iCount;        // loop count
	CBsuSet *xpSet;       // loop variable
	CBsuBar *xpHBar = nullptr, *xpVBar = nullptr;

	for (iCount = 0, xpSet = this; iCount++ < 2 && xpSet; xpSet = m_xpSetLink) {
		if ((iError = xpSet->GetWindowBars(xpHBar, xpVBar)) != 0)
			goto cleanup;

		xpHBar->m_iDocSize = cDocSize.cx;
		xpVBar->m_iDocSize = cDocSize.cy;

		xpHBar->m_iMargin1 = cScrollRect.left;
		xpHBar->m_iMargin2 = cScrollRect.right;
		xpVBar->m_iMargin1 = cScrollRect.top;
		xpVBar->m_iMargin2 = cScrollRect.bottom;
	}

	UpdateWndDeviceExtents();

cleanup:

	JXELEAVE(CBsuSet::PrepareWndBsuSet);
	RETURN(iError != 0);
}

//* CBsuSet::UpdateWndDeviceExtents -- update window devices coordinates
bool CBsuSet::UpdateWndDeviceExtents()
// returns: true if error, false otherwise
{
	JXENTER(CBsuSet::UpdateWndDeviceExtents);
	int iError = 0;        // error code

	CDC *xpDc = nullptr;     // device context
	int iInCount, iCount;      // loop count
	CBsuSet *xpSet;       // loop variable
	CBsuBar *xpBar, *xpHBar = nullptr, *xpVBar = nullptr;
	CRect cWndRect, cDevWndRect;  // window rect: logical/device units
	CScrollBar *xpScrollBar;  // dialog box scroll bar

	if (m_bInUpdateBars)        // prevent recursion
		goto exit;

	m_bInUpdateBars = true;

	if (!m_xpWnd || ((xpDc = m_xpWnd->GetDC()) == nullptr)) {
		iError = 100;  // can't allocate device context
		goto cleanup;
	}

	m_xpWnd->GetClientRect(cDevWndRect);
	cWndRect = cDevWndRect;
	xpDc->DPtoLP(&cWndRect);       // get logical units

	for (iCount = 0, xpSet = this; iCount++ < 2 && xpSet; xpSet = m_xpSetLink)
		if (!xpSet->GetWindowBars(xpHBar, xpVBar)) {
			// CRect constructor order: left, top, right, bottom
			CRect cDevScrollRect(xpHBar->m_iMargin1,
				xpVBar->m_iMargin1,
				xpHBar->m_iMargin2, xpVBar->m_iMargin2);
			xpDc->LPtoDP(&cDevScrollRect);

			xpHBar->m_iWndSize = cWndRect.right - cWndRect.left;
			xpVBar->m_iWndSize = cWndRect.bottom - cWndRect.top;

			xpHBar->m_iDevMargin1 = cDevScrollRect.left;
			xpHBar->m_iDevMargin2 = cDevScrollRect.right;
			xpVBar->m_iDevMargin1 = cDevScrollRect.top;
			xpVBar->m_iDevMargin2 = cDevScrollRect.bottom;

			// if this is a CScrollView window, then tell viewscrl.cpp
			// what the document sizes are.  This will enable or
			// disable the window scroll bars
			if (xpSet->m_bScrollBars)
				((CScrollView *)xpSet->m_xpWnd)->SetScrollSizes(MM_TEXT,
					CSize(xpHBar->m_iDocSize, xpVBar->m_iDocSize));
			else if (xpSet->m_bScrollView)
				((CScrollView *)xpSet->m_xpWnd)->SetScrollSizes(MM_TEXT,
					CSize(1, 1));


			for (iInCount = 0, xpBar = xpHBar; iInCount++ < 2; xpBar = xpVBar) {
				if ((xpBar->m_iMax = xpBar->m_iDocSize - xpBar->m_iWndSize + xpBar->m_iMargin1 + xpBar->m_iMargin2) < 0)
					xpBar->m_iMax = 0;

				if (xpBar->m_iPosition > xpBar->m_iMax)
					xpBar->m_iPosition = xpBar->m_iMax;

				if (xpSet->m_xpDlg && ((xpScrollBar = (CScrollBar *)xpSet->m_xpDlg->GetDlgItem(xpBar->m_iId)) != nullptr)) {
					xpScrollBar->SetScrollRange(xpBar->m_iMin, xpBar->m_iMax, true);
					xpScrollBar->SetScrollPos(xpBar->m_iPosition);
				}

				if (xpBar->m_lpiVariable)
					*xpBar->m_lpiVariable = xpBar->m_iPosition;
			}
		}

cleanup:

	if (xpDc)
		m_xpWnd->ReleaseDC(xpDc);

	m_bInUpdateBars = false;

exit:

	JXELEAVE(CBsuSet::UpdateWndDeviceExtents);
	RETURN(iError != 0);
}

//* CBsuSet::PrepareDc -- replace OnPrepareDC -- set the viewport and
//	the clip rectangle to the specified region
bool CBsuSet::PrepareDc(CDC *xpDc, bool bRelocatable)
// xpDc -- device context
// bRelocatable -- if false, set to whole window; if true,
//		set to the interior
//		region, which is scrolled and might be remapped
// lpClipRect -- pointer to clipping rectangle; if nullptr, use whole window
// returns: true if error, false otherwise
{
	JXENTER(CBsuSet::PrepareDc);
	int iError = 0;        // error code
	CPoint cViewOrigin(0, 0);  // viewport origin
	CRect cScrollRegion;   // region to be scrolled
	CBsuBar *xpHBar, *xpVBar;     // horizontal/vertical bars

	if ((iError = GetWindowBars(xpHBar, xpVBar)) != 0)
		goto cleanup;      // error return

	if (bRelocatable) {
		SetSubWindowRect(&cScrollRegion, BSCR5);
		// get the non-border region of the window

		cViewOrigin.x += xpHBar->m_iMargin1 - xpHBar->m_iPosition;
		cViewOrigin.y += xpVBar->m_iMargin1 - xpVBar->m_iPosition;
	} else {
		// scroll region is entire window
		SetSubWindowRect(&cScrollRegion, BSCRALL);
	}

	xpDc->SetMapMode(MM_TEXT);
	xpDc->SetViewportOrg(0, 0);

	xpDc->SelectClipRgn(nullptr); // make entire window the clipping region

	xpDc->IntersectClipRect(&cScrollRegion);

	xpDc->SetViewportOrg(cViewOrigin);

cleanup:

	JXELEAVE(CBsuSet::PrepareDc);
	RETURN(iError != 0);
}


//* CBsuSet::OnScroll -- handle OnHScroll and OnVScroll messages
bool CBsuSet::OnScroll(unsigned int nSBCode, unsigned int nPos, CScrollBar *, int iBarType)
// nSBCode -- SB_xxxx -- Windows scroll code
// nPos -- position on scroll bar (SB_THUMBTRACT/SB_THUMBPOSITION)
// xpScrollBar -- pointer to scroll bar control
// iBarType -- BSCT_xxxx -- scroll bar type
// returns: true if error, false otherwise
{
	JXENTER(CBsuSet::OnScroll);
	int iError = 0;            // error code
	CBsuBar *xpBar;
	int iIncr = 0;             // scroll bar increment
	int iNewVal;               // new scroll value
	CScrollBar *xpDScrollBar;  // dialog scroll bar

	if ((xpBar = GetBar(iBarType)) == nullptr) {
		iError = 100;
		goto cleanup;
	}

	switch (nSBCode) {

	case SB_LEFT:       // = SB_TOP
		iIncr = xpBar->m_iMin - xpBar->m_iPosition;
		break;

	case SB_PAGELEFT:   // = SB_PAGEUP
		iIncr = -(xpBar->m_iMax - xpBar->m_iMin) / 8;
		break;

	case SB_LINELEFT:   // = SB_LINEUP
		iIncr = -5;
		break;

	case SB_RIGHT:      // = SB_BOTTOM
		iIncr = xpBar->m_iMax - xpBar->m_iPosition;
		break;

	case SB_PAGERIGHT:  // = SB_PAGERIGHT
		iIncr = (xpBar->m_iMax - xpBar->m_iMin) / 8;
		break;

	case SB_LINERIGHT:  // = SB_LINERIGHT
		iIncr = 1;
		break;

	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		iIncr = nPos - xpBar->m_iPosition;
		break;

	default:
		break;
	}

	iNewVal = xpBar->m_iPosition + iIncr;

	if (iNewVal < xpBar->m_iMin)
		iNewVal = xpBar->m_iMin;

	if (iNewVal > xpBar->m_iMax)
		iNewVal = xpBar->m_iMax;

	iIncr = iNewVal - xpBar->m_iPosition;

	// scroll bars are for a window (may or may not also be m_xpDlg)
	if (m_xpWnd) {
		CPoint cNewPoint;  // new scroll point
		CBsuBar *xpHBar, *xpVBar;

		if (GetWindowBars(xpHBar, xpVBar))
			goto cleanup;

		if (xpBar->m_iBarType == BSCT_HORZ)
			cNewPoint = CPoint(iNewVal, xpVBar->m_iPosition);
		else
			cNewPoint = CPoint(xpHBar->m_iPosition, iNewVal);

		ScrollWindowToPoint(cNewPoint);

		// not for a window
		//
	} else if (m_xpDlg) {
		xpBar->m_iPosition = iNewVal;

		if (xpBar->m_lpiVariable)
			*xpBar->m_lpiVariable = iNewVal;

		m_xpDlg->UpdateData(false);

		if (m_xpSetLink) {
			CBsuBar *xpLinkBar = m_xpSetLink->GetBar(iBarType);

			if (xpLinkBar && m_xpSetLink->m_xpDlg) {
				if ((xpDScrollBar = (CScrollBar *)m_xpSetLink->m_xpDlg->GetDlgItem(xpLinkBar->m_iId)) != nullptr)
					xpDScrollBar->SetScrollPos(iNewVal);

				xpLinkBar->m_iPosition = iNewVal;

				if (xpLinkBar->m_lpiVariable)
					*xpLinkBar->m_lpiVariable = iNewVal;
			}

			if (m_xpSetLink->m_xpDlg)
				m_xpSetLink->m_xpDlg->UpdateData(false);
		}
	}

cleanup:

	JXELEAVE(CBsuSet::OnScroll);
	RETURN(iError != 0);
}


//* CBsuSet::GetBar -- get bsu scroll bar object
CBsuBar *CBsuSet::GetBar(int iBarType)
// iBarType -- BSCT_xxxx -- scroll bar type
// returns: pointer to scroll bar object, or nullptr if not found
// returns: true if error, false otherwise
{
	JXENTER(CBsuSet::GetBar);
	CBsuBar *xpBar, *xpBarFound = nullptr;   // return value
	CScrollBar *xpScrollBar;

	for (xpBar = m_xpBarChain; !xpBarFound && xpBar; xpBar = xpBar->m_xpNextBar) {
		if (xpBar->m_iBarType == iBarType)
			xpBarFound = xpBar;
	}

	if (xpBarFound) {

		if (m_bScrollView && m_bScrollBars) {
			((CScrollView *)m_xpWnd)->GetScrollRange(xpBarFound->m_iWndScrollCode, &xpBarFound->m_iMin, &xpBarFound->m_iMax);
			CPoint cPosition = ((CScrollView *)m_xpWnd)->GetScrollPosition();
			xpBarFound->m_iPosition = (xpBarFound->m_iBarType == BSCT_HORZ) ? cPosition.x : cPosition.y;

		} else if (m_xpDlg && ((xpScrollBar = (CScrollBar *)m_xpDlg->GetDlgItem(xpBarFound->m_iId)) != nullptr)) {

			xpScrollBar->GetScrollRange(&xpBarFound->m_iMin, &xpBarFound->m_iMax);
			xpBarFound->m_iPosition = xpScrollBar->GetScrollPos();
		}
	}

	// cleanup:

	JXELEAVE(CBsuSet::GetBar);
	RETURN(xpBarFound);
}


//* CBsuSet::ScrollWindowToPoint -- scroll window to spec point
bool CBsuSet::ScrollWindowToPoint(CPoint cScrollPosition, bool bScrollWindow)
// cScrollPosition -- point to which window is to be scrolled
// bScrollWindow -- if true, then scroll window.  (If false, then just
//	set the variables and scroll bars)
// returns: true if error, false otherwise
{
	JXENTER(CBsuSet::ScrollWindowToPoint);
	int iError = 0;        // error code
	CDC *xpDc = nullptr;     // client device context
	CPoint cOldScrollPosition, cDevOldScrollPosition;
	int iCount;
	CBsuBar *xpBar, *xpHBar, *xpVBar;
	CRect cScrollRegion;           // region to be scrolled
	CRect cUncoverRect;            // rectangle for uncovered region
	CRect cHPaintRect, cVPaintRect; // more rects for uncovered region
	CBsuSet *xpSet;
	CScrollBar *xpScrollBar;
	static bool bFirstTime = true;

	AfxGetApp()->DoWaitCursor(1);

	if (GetWindowBars(xpHBar, xpVBar))
		goto cleanup;

	cOldScrollPosition = CPoint(xpHBar->m_iPosition, xpVBar->m_iPosition);

	if (cScrollPosition.x < xpHBar->m_iMin)
		cScrollPosition.x = xpHBar->m_iMin;
	if (cScrollPosition.x > xpHBar->m_iMax)
		cScrollPosition.x = xpHBar->m_iMax;

	if (cScrollPosition.y < xpVBar->m_iMin)
		cScrollPosition.y = xpVBar->m_iMin;
	if (cScrollPosition.y > xpVBar->m_iMax)
		cScrollPosition.y = xpVBar->m_iMax;

	bScrollTemp = lpMetaGameStruct->m_bScrolling;
	lpMetaGameStruct->m_bScrolling = false;

	if (bScrollWindow && m_xpWnd && (cScrollPosition.x != cOldScrollPosition.x || cScrollPosition.y != cOldScrollPosition.y)) {

		if (!xpDc && ((xpDc = m_xpWnd->GetDC()) == nullptr)) {

			iError = 100;  // can't allocate device context
			goto cleanup;
		}
		PrepareDc(xpDc, true);

		if (!bFirstTime && xpDc->GetClipBox(&cUncoverRect) != NULLREGION) {
			bFirstTime = false;
			// make sure screen is valid before we do a ScrollDC
			m_xpWnd->UpdateWindow();
		}

		// scroll right
		if (cScrollPosition.x < cOldScrollPosition.x) {
			cHPaintRect = CRect(cScrollPosition.x, 0, cOldScrollPosition.x, xpVBar->m_iDocSize);

			// scroll left
		} else {
			cHPaintRect = CRect(cOldScrollPosition.x + xpHBar->m_iWndSize - xpHBar->m_iMargin1 - xpHBar->m_iMargin2, 0, cScrollPosition.x + xpHBar->m_iWndSize - xpHBar->m_iMargin1 - xpHBar->m_iMargin2, xpVBar->m_iDocSize);
		}

		// scroll down
		if (cScrollPosition.y < cOldScrollPosition.y) {
			cVPaintRect = CRect(0, cScrollPosition.y, xpHBar->m_iDocSize, cOldScrollPosition.y);

			// scroll up
		} else {
			cVPaintRect = CRect(0, cOldScrollPosition.y + xpVBar->m_iWndSize - xpVBar->m_iMargin1 - xpVBar->m_iMargin2, xpHBar->m_iDocSize, cScrollPosition.y + xpVBar->m_iWndSize - xpVBar->m_iMargin1 - xpVBar->m_iMargin2);
		}

		SetSubWindowRect(&cScrollRegion, BSCR5);
		xpDc->DPtoLP(&cScrollRegion);

#if 1
		//xpDc->ScrollDC(cOldScrollPosition.x - cScrollPosition.x,cOldScrollPosition.y - cScrollPosition.y,&cScrollRegion, &cScrollRegion, nullptr, &cUncoverRect);

		//xpDc->LPtoDP(&cUncoverRect) ;
		//m_xpWnd->InvalidateRect(&cUncoverRect) ;

#else   // do x and y scrolling separately so repaint will be faster
		xpDc->ScrollDC(cOldScrollPosition.x - cScrollPosition.x, 0, &cScrollRegion, &cScrollRegion, nullptr, &cUncoverRect);

		//xpDc->LPtoDP(&cUncoverRect) ;
		//m_xpWnd->InvalidateRect(&cUncoverRect) ;
		m_xpWnd->InvalidateRect(&cHPaintRect);
		m_xpWnd->UpdateWindow();   // update between scrolls

		xpDc->ScrollDC(0, cOldScrollPosition.y - cScrollPosition.y, &cScrollRegion, &cScrollRegion, nullptr, &cUncoverRect);

		//xpDc->LPtoDP(&cUncoverRect) ;
		//m_xpWnd->InvalidateRect(&cUncoverRect) ;
		m_xpWnd->InvalidateRect(&cVPaintRect);
#endif

		lpMetaGameStruct->m_bScrolling = bScrollTemp;
	}

	for (iCount = 0, xpSet = this; iCount++ < 2 && xpSet; xpSet = m_xpSetLink) {
		//CBsuBar * xpHBar, *xpVBar ;
		int iInCount, iNewVal;

		if (xpSet->GetWindowBars(xpHBar, xpVBar))
			goto cleanup;

		for (iInCount = 0, xpBar = xpHBar, iNewVal = cScrollPosition.x; iInCount++ < 2; xpBar = xpVBar, iNewVal = cScrollPosition.y) {

			if (xpSet->m_bScrollView && xpSet->m_bScrollBars)
				((CScrollView *)xpSet->m_xpWnd)->SetScrollPos(xpBar->m_iWndScrollCode, iNewVal);

			else if (xpSet->m_xpDlg && ((xpScrollBar = (CScrollBar *)xpSet->m_xpDlg->GetDlgItem(xpBar->m_iId)) != nullptr))
				xpScrollBar->SetScrollPos(iNewVal);

			xpBar->m_iPosition = iNewVal;

			if (xpBar->m_lpiVariable)
				*xpBar->m_lpiVariable = iNewVal;

			if (xpSet->m_xpDlg)
				xpSet->m_xpDlg->UpdateData(false);
		}
	}

	if (bScrollWindow && m_xpWnd && (cScrollPosition.x != cOldScrollPosition.x || cScrollPosition.y != cOldScrollPosition.y)) {
		PrepareDc(xpDc, true); // call again to prepare the DC,
		// based on the new scroll position

#if 0
		xpDc->LPtoDP(&cHPaintRect);
		xpDc->LPtoDP(&cVPaintRect);

		m_xpWnd->InvalidateRect(&cHPaintRect);
		m_xpWnd->UpdateWindow();   // update between scrolls

		m_xpWnd->InvalidateRect(&cVPaintRect);

#else
		m_xpWnd->Invalidate(false);
		m_xpWnd->UpdateWindow();
#endif

	}

cleanup:

	if (xpDc)
		m_xpWnd->ReleaseDC(xpDc);

	AfxGetApp()->DoWaitCursor(-1);

	JXELEAVE(CBsuSet::ScrollWindowToPoint);
	RETURN(iError != 0);
}

//* CBsuSet::EdgeToCenter -- if point is on edge, scroll it to center
bool CBsuSet::EdgeToCenter(CPoint cPoint, bool bScroll)
// cPoint the point to be tested
// returns: true if error, false otherwise
{
	JXENTER(CBsuSet::EdgeToCenter);
	int iError = 0;        // error code
	int bCenter = false;   // flag: center the point
	int iCoord;
	int iInCount;      // loop variable
	CPoint cCenterPoint;
	XPINT xpiCenterCoord;

	CBsuBar *xpBar, *xpHBar, *xpVBar;

	if (GetWindowBars(xpHBar, xpVBar))
		goto cleanup;

	for (iInCount = 0, xpBar = xpHBar, iCoord = cPoint.x, xpiCenterCoord = &cCenterPoint.x; iInCount++ < 2; xpBar = xpVBar, iCoord = cPoint.y, xpiCenterCoord = &cCenterPoint.y) {

		// this computation can be done entirely in logical units.
		int iWidth = (xpBar->m_iWndSize - xpBar->m_iMargin1 - xpBar->m_iMargin2);
		if ((xpBar->m_iPosition > xpBar->m_iMin &&
			iCoord < xpBar->m_iPosition + iWidth / 8)
			|| (xpBar->m_iPosition < xpBar->m_iMax &&
				iCoord > xpBar->m_iPosition + iWidth * 7 / 8))
			bCenter = true;

		*xpiCenterCoord = iCoord - iWidth / 2;
	}

	if (bScroll || (bCenter && (gnFurlongs != 0)))
		ScrollWindowToPoint(cCenterPoint);

cleanup:

	JXELEAVE(CBsuSet::EdgeToCenter);
	RETURN(iError != 0);
}

//* CBsuSet::SetSubWindowRect -- set rectangle to portion of window
//		(logical coordinates)
bool CBsuSet::SetSubWindowRect(LPRECT lpRect, int iBsRegion)
// lpRect -- where results are to be stored
// iBsRegion -- combination of BSCRn bits, based on the window margins,
//	according to the following diagram:
//		123
//		456
//		789
//	Typical values are: "123" for the top margin, "5" for the
//	interior scrolling region, and so forth.  Not all
//	combinations are supported.
// returns: true if error, false otherwise
{
	JXENTER(CBsuSet::SetSubWindowRect);
	int iError = 0;        // error code
	CBsuBar *xpHBar, *xpVBar;     // horizontal/vertical bars

	if ((iError = GetWindowBars(xpHBar, xpVBar)) != 0)
		goto cleanup;      // error return

	if (iBsRegion == BSCR123) {
		lpRect->left = lpRect->top = 0;
		lpRect->right = xpHBar->m_iWndSize;
		lpRect->bottom = xpVBar->m_iMargin1;

		// entire window
	} else if (iBsRegion == BSCRALL) {

		lpRect->left = lpRect->top = 0;
		lpRect->right = xpHBar->m_iWndSize;
		lpRect->bottom = xpVBar->m_iWndSize;

	} else if (iBsRegion == BSCR5) {

		lpRect->left = xpHBar->m_iMargin1;
		lpRect->right = xpHBar->m_iWndSize - xpHBar->m_iMargin2;
		lpRect->top = xpVBar->m_iMargin1;
		lpRect->bottom = xpVBar->m_iWndSize - xpVBar->m_iMargin2;

	} else {

		// for unsupported, set rectangle to whole window
		lpRect->left = lpRect->top = 0;
		lpRect->right = xpHBar->m_iWndSize;
		lpRect->bottom = xpVBar->m_iWndSize;

		iError = 110;      // argument not supported
		goto cleanup;
	}

cleanup:

	JXELEAVE(CBsuSet::SetSubWindowRect);
	RETURN(iError != 0);
}


//* CBsuSet::TestRect -- test whether rectangle is in window
bool CBsuSet::TestRect(CRRect crTestRect, bool &bPhysical, bool &bEdge)
// crTestRect -- relocatable rectangle to be tested
// bPhysical (output) -- if true, then rectangle lies at least
//		partially within physical window
// bEdge (output) -- if true, then rectangle is on the edge
//		(partially in window, partially outside window)
// returns: true if error, false otherwise
{
	JXENTER(CBsuSet::TestRect);
	int iError = 0;        // error code
	CRect cWndRect;    // physical window (logical units)
	CRect cInterRect;  // intersection rectangle

	CBsuBar *xpHBar, *xpVBar;     // horizontal/vertical bars

	if ((iError = GetWindowBars(xpHBar, xpVBar)) != 0)
		goto cleanup;      // error return

	// if rectangle is relocatable
	//
	if (crTestRect.IfRelocatable()) {

		SetSubWindowRect(&cWndRect, BSCR5);    // get inside
		// window in logical units
		cWndRect.OffsetRect(xpHBar->m_iPosition - xpHBar->m_iMargin1, xpVBar->m_iPosition - xpVBar->m_iMargin1);
		bPhysical = (cInterRect.IntersectRect(&crTestRect, &cWndRect) != 0);
		bEdge = bPhysical && (cInterRect != crTestRect);

		// rectangle is not relocatable
		//
	} else {
		bPhysical = true, bEdge = false;
	}

cleanup:

	JXELEAVE(CBsuSet::TestRect);
	RETURN(iError != 0);
}


//* CBsuSet::GetWindowBars -- set rectangle to portion of window
//		(device coordinates)
bool CBsuSet::GetWindowBars(CBsuBar *&xpHBar, CBsuBar *&xpVBar, bool bErrorRtn)
// xpHBar, xpVBar (output) -- horizontal/vertical scroll bar object
// bErrorRtn -- if false, don't generate "not allocated" error returns
// returns: true if error, false otherwise
{
	JXENTER(CBsuSet::GetWindowBars);
	int iError = 0;        // error code
	CBsuBar *xpBar;       // loop variable

	xpHBar = xpVBar = nullptr;    // nothing yet
	for (xpBar = m_xpBarChain; xpBar; xpBar = xpBar->m_xpNextBar) {

		if (xpBar->m_iBarType == BSCT_HORZ) {

			if (xpHBar) {
				iError = 100;  // duplicate horizontal bars
				goto cleanup;
			}
			xpHBar = xpBar;

		} else if (xpBar->m_iBarType == BSCT_VERT) {

			if (xpVBar) {
				iError = 101;  // duplicate vertical bars
				goto cleanup;
			}
			xpVBar = xpBar;
		}
	}

	if (bErrorRtn && (!xpHBar || !xpVBar)) {
		iError = 110;
		goto cleanup;
	}

cleanup:

	JXELEAVE(CBsuSet::GetWindowBars);
	RETURN(iError != 0);
}

//* CBsuSet::PointLogical -- convert device point to logical coords
CRPoint CBsuSet::PointLogical(CPoint cPoint) {
// cPoint -- point in device coordinates
// returns: relocatable point
	JXENTER(CBsuSet::PointLogical);
	CRect cScrollRegion;   // scrolled region of window
	CRPoint crPoint(cPoint, false);    // return value
	CDC *xpDc = nullptr;

	CBsuBar *xpHBar = nullptr, *xpVBar = nullptr;

	if (GetWindowBars(xpHBar, xpVBar)) {
		//iError = 100;
		goto cleanup;
	}

	// get scroll region in device units
	SetSubWindowRect(&cScrollRegion, BSCR5);
	if ((crPoint.m_bRelocatable = cScrollRegion.PtInRect(cPoint)) != false) {

		if (!xpDc && ((xpDc = m_xpWnd->GetDC()) == nullptr)) {
			//iError = 101;  // can't allocate device context
			goto cleanup;
		}

		PrepareDc(xpDc, true); // set up relocatable mapping mode
		xpDc->DPtoLP(&crPoint);    // convert point to logical units
	}

cleanup:
	if (xpDc)
		m_xpWnd->ReleaseDC(xpDc);

	JXELEAVE(CBsuSet::PointLogical);
	RETURN(crPoint);
}

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
