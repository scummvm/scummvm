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

extern bool     bExitMetaDLL;

///DEFS gtldat.h

//* CGtlData::AcceptClick -- process a mouse click by user
bool CGtlData::AcceptClick(CGtlView * xpGtlView, CPoint cClickPoint, int iClickType)
// xpGtlView -- CGtlView object
// cClickPoint -- position of mouse when click (button up) occurred
// iClickType -- CLICK_xxxx -- left/right up/move/down/double
// returns: true if error, false otherwise
{
	JXENTER(CGtlData::AcceptClick) ;
	CNode *lpFoundNode = nullptr;  // current node
	int iError = 0 ;            // error code
	int iButtonId = 0 ;         // handle button
	CRPoint crClickPoint ;      // relocatable click position

	uint32 stNowTime ;   // current time
	long lTimeDiff ;            // time difference
	bool bExitDll = false;

	// Save current time
	stNowTime = g_system->getMillis();

	// test for recursive call

	lTimeDiff = stNowTime - m_stAcceptClickActive;

	// if recursion lockout lasts more than 3 seconds, then just clear it
	if (lTimeDiff < 3000)
		goto exit ; // exit -- recursive call

	m_stAcceptClickActive = stNowTime;

	// convert device point to relocatable point
	crClickPoint = xpGtlView->m_cViewBsuSet.PointLogical(cClickPoint) ;

	// allocate device context
	m_cBgbMgr.InitDc(xpGtlView, &xpGtlView->m_cViewBsuSet) ;

	// prepare DC for access to correct area
	xpGtlView->m_cViewBsuSet.PrepareDc(m_cBgbMgr.m_xpDc, crClickPoint.IfRelocatable());

	lpFoundNode = PointToNode(crClickPoint) ;

	switch (iClickType) {

	case CLICK_LDOWN:
		m_stLDownTime = stNowTime ;  // save lft button down time
		m_lpFoundNode = lpFoundNode ;       // found node

		if (!lpFoundNode)
			m_cBbtMgr.AcceptClick(crClickPoint, iClickType);
		break;


	case CLICK_LUP:
	case CLICK_LMOVE:
		if (lpFoundNode) {

			if (lpFoundNode->m_iWeight < 99)
				ProcessMove(lpFoundNode) ;

		} else {
			iButtonId = m_cBbtMgr.AcceptClick(crClickPoint, iClickType);
		}
		break ;

	case CLICK_RUP:
		// in meta game
		xpGtlView->m_cViewBsuSet.EdgeToCenter(crClickPoint) ;
		break ;

	default:
		iError = 100 ;
		goto cleanup ;
		// break ;
	}

	switch (iButtonId) {

	case 0:         // no button
		break ;

	case BUTTON_MINIMAP:
		InitInterface(MG_DLLX_ZOOM, bExitDll);
		break ;

	case BUTTON_INVENTORY:
		// display inventory
		InitInterface(MG_DLLX_INVENTORY, bExitDll);
		break ;

	case BUTTON_SCROLL:
		// display main-menu
		InitInterface(MG_DLLX_SCROLL, bExitDll);
		break ;

	default:
		iError = 110 ;
		goto cleanup ;
		// break ;
	}

cleanup:
	m_cBgbMgr.ReleaseDc() ;

	m_stAcceptClickActive = 0 ;

exit:

	JXELEAVE(CGtlData::AcceptClick) ;
	RETURN(iError != 0) ;
}

//* CGtlData::PointToNode -- find node closest to point
CNode FAR *CGtlData::PointToNode(CRPoint crPoint)
// crPoint -- relocatable point
// returns: node nearest to point, or nullptr if none close enough
{
	JXENTER(CGtlData::PointToNode) ;
	long lSqDistance ;
	long lSqSensitivity ;
	long lSqMinDistance = MAXPOSLONG ;
	int iK ;            // loop variable
	CNode FAR * lpNode, FAR * lpClosestNode = nullptr ;
	CRPoint crNodePosition ;

	// search through nodes to find which node (if any) is selected
	for (iK = 0 ; iK < m_iNodes ; ++iK) {

		if (!(lpNode = m_lpNodes + iK)->m_bDeleted && (crPoint.IfRelocatable() == lpNode->IfRelocatable())) {

			crNodePosition = NodeToPoint(lpNode) ;
			lSqSensitivity = (_metaGame ? lpNode->m_iSensitivity : NODERADIUS + 1) ;
			lSqSensitivity *= lSqSensitivity ;

			if ((lSqDistance = LSQ(crNodePosition, crPoint)) < lSqSensitivity && lSqDistance < lSqMinDistance) {
				lSqMinDistance = lSqDistance ;
				lpClosestNode = lpNode ;
			}
		}
	}

// cleanup:

	JXELEAVE(CGtlData::PointToNode) ;
	RETURN(lpClosestNode) ;
}

//* CGtlData::CallUpdate -- call to update all views for this map
bool CGtlData::CallUpdate(CNode FAR *lpNode1, CNode FAR *lpNode2, bool bLinks, bool bWmPaint)
// lpNode1, lpNode2 -- node or link specifying range to be updated
// bLinks -- if true, also update all links of lpNode1
// bWmPaint -- if true, use WM_PAINT message.
// returns: true if error, false otherwise
{
	JXENTER(CGtlData::CallUpdate) ;
	int iError = 0 ;            // error code
	CGtlHint cHint ;
	CRPoint crPoint1, crPoint2 ;        // node locations
	long lHintParam = HINT_UPDATE_RECT ;
	int iRadius ;

	cHint.m_bWmPaint = bWmPaint ;

	if (lpNode1)
		crPoint1 = NodeToPoint(lpNode1) ;

	if (lpNode2)
		crPoint2 = NodeToPoint(lpNode2) ;

	if (!lpNode1)       // nothing specified
		lHintParam = HINT_UPDATE_FULL ; // update whole screem

	else if (!lpNode2) {        // just a node specified
		iRadius = NODERADIUS ;
		if (iRadius < lpNode1->m_iSensitivity)
			iRadius = lpNode1->m_iSensitivity ;
		iRadius += 5 ;  // just to be sure

		cHint.cHintRect = CRect(crPoint1.x - iRadius,
		                        crPoint1.y - iRadius, crPoint1.x + iRadius,
		                        crPoint1.y + iRadius) ;
	} else              // link between nodes 1 and 2
		CGenUtil::NormalizeCRect((cHint.cHintRect =
		                              CRect(crPoint1.x - NODERADIUS,
		                                    crPoint1.y - NODERADIUS,
		                                    crPoint2.x + NODERADIUS,
		                                    crPoint2.y + NODERADIUS))) ;

//  SetModifiedFlag(true) ;
	((CGtlDoc *)m_xpcGtlDoc)->UpdateAllViews(nullptr, lHintParam, &cHint) ;

	// want to update all node 1 links
	if (lpNode1 && bLinks) {

		int iL, iLink ;         // link index
		CNode FAR * lpLinkNode ;        // link node
		for (iL = 0 ; iL < lpNode1->m_iNumLinks ; ++iL) {

			iLink = lpNode1->m_iLinks[iL] ;
			lpLinkNode = m_lpNodes + iLink ;
			CallUpdate(lpNode1, lpLinkNode) ;
		}
	}

// cleanup:

	JXELEAVE(CGtlData::CallUpdate) ;
	RETURN(iError != 0) ;
}

//* CGtlData::CallUpdate -- call to update all views for this map
//              -- version which updates a bitmap object
bool CGtlData::CallUpdate(CBgbObject * lpcBgbObject)
// lpcBgbObject -- pointer to object
// returns: true if error, false otherwise
{
	JXENTER(CGtlData::CallUpdate) ;
	int iError = 0 ;            // error code
	CGtlHint cHint ;

	cHint.cHintRect = lpcBgbObject->GetRect() ;
	((CGtlDoc *)m_xpcGtlDoc)->UpdateAllViews(nullptr, HINT_UPDATE_RECT, &cHint);

// cleanup:

	JXELEAVE(CGtlData::CallUpdate) ;
	RETURN(iError != 0) ;
}

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
