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

extern BOOL     bExitMetaDLL;

///DEFS gtldat.h

//* CGtlData::AcceptClick -- process a mouse click by user
BOOL CGtlData::AcceptClick(CGtlView * xpGtlView, CPoint cClickPoint, int iClickType)
// xpGtlView -- CGtlView object
// cClickPoint -- position of mouse when click (button up) occurred
// iClickType -- CLICK_xxxx -- left/right up/move/down/double
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CGtlData::AcceptClick) ;
	CNode *lpFoundNode = nullptr;  // current node
	int iError = 0 ;            // error code
	int iButtonId = 0 ;         // handle button
	CRPoint crClickPoint ;      // relocatable click position

	uint32 stNowTime ;   // current time
	long lTimeDiff ;            // time difference
	BOOL bExitDll = FALSE;

	#ifdef NODEEDIT
	CNode *lpNode;
	// previously selected node, if any
	CNode FAR * lpSelectedNode = GetSelectedNode() ;
	#endif

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

		#ifdef NODEEDIT
		if (!m_bMetaGame) {
			m_lpLastSelectedNode = GetSelectedNode() ;
			ModifySelectNode() ;  // deselect current node
			SelectLink() ;    // deselect current link
		} else
		#endif
			if (!lpFoundNode)
				m_cBbtMgr.AcceptClick(crClickPoint, iClickType) ;

		break ;


	case CLICK_LUP:

		#ifdef NODEEDIT
		if (lpFoundNode && m_lpFoundNode == lpFoundNode
		        && lpFoundNode->m_bMenu
		        && m_xpcGtlDoc && m_xpcGtlDoc->m_xpGtlFrame
		        && !m_xpcGtlDoc->m_xpcMenuDlg
		        && ::GetKeyState(VK_SHIFT) < 0) {
			m_xpcGtlDoc->m_xpGtlFrame->ShowMenuDialog() ;
			lpFoundNode = m_lpFoundNode = nullptr ;
		}
		#endif
	// drop through

	case CLICK_LMOVE:

		#ifdef NODEEDIT
		if (m_lpFoundNode && lpFoundNode != m_lpFoundNode && (!m_lpFoundNode->m_bSelected || m_bMetaGame))
			m_lpFoundNode = nullptr ;

		if (iClickType == CLICK_LMOVE) {
			if (m_bMetaGame)
				m_cBbtMgr.AcceptClick(crClickPoint, iClickType) ;
			break ;
		}

		if (!m_bMetaGame) {

			lTimeDiff = 1000L * (stNowTime.time - m_stLDownTime.time) + stNowTime.millitm - m_stLDownTime.millitm ;

			// if click lasts more than a second, and there's
			// a previously selected node
			//
			if (lTimeDiff > 1000L && m_lpLastSelectedNode) {

				// then move the node
				MoveNode(m_lpLastSelectedNode, crClickPoint) ;

			} else if (lpFoundNode) {
				SelectNode(lpFoundNode) ;   // selected node found
			}
		} else
		#endif
			if (lpFoundNode) {

				if (lpFoundNode->m_iWeight < 99)
					ProcessMove(lpFoundNode) ;

				// there needs to be a check here to see if
				// I need to exit the DLL without continuing
				// if (bExitMetaDLL)
				//      return;
			} else
				iButtonId = m_cBbtMgr.AcceptClick(crClickPoint, iClickType);
		break ;

		#ifdef BAGEL_DEBUG
	case CLICK_LDOUBLE:
		break ;

	case CLICK_RDOWN:
		break ;

	case CLICK_RMOVE:
		break ;

	case CLICK_RDOUBLE:
		break ;

	case CLICK_UMOVE:
		break ;
		#endif

	case CLICK_RUP:

		#ifdef NODEEDIT
		if (!m_bMetaGame) {

			// if we're double clicking in empty space
			if ((lpNode = lpFoundNode) == nullptr) {

				// create new node and test for error
				if ((iError = CreateNode(lpNode, crClickPoint)) != 0)
					goto cleanup ;
			}

			// in either case, link to previously selected node, if any
			if (lpSelectedNode)
				if ((iError = CreateLink(lpNode, lpSelectedNode)) != 0)
					goto cleanup;

			ModifySelectNode(lpNode, TRUE) ;
		} else
		#endif
		{
			// in meta game
			xpGtlView->m_cViewBsuSet.EdgeToCenter(crClickPoint) ;
		}
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

	#ifdef NODEEDIT
	UpdateDialogs() ;
	#endif

	m_stAcceptClickActive = 0 ;

exit:

	JXELEAVE(CGtlData::AcceptClick) ;
	RETURN(iError != 0) ;
}


#ifdef NODEEDIT

//* CGtlData::AcceptDeleteKey -- process depressing delete key
BOOL CGtlData::AcceptDeleteKey(CGtlView * xpGtlView)
// xpGtlView -- CGtlView object
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CGtlData::AcceptDeleteKey) ;
	int iError = 0 ;            // error code

	CNode FAR * lpSelectedNode = GetSelectedNode() ;

	m_cBgbMgr.InitDc(xpGtlView, &xpGtlView->m_cViewBsuSet) ;

	if (!m_bMetaGame) {

		if (lpSelectedNode) {
			if (::MessageBox(nullptr, "Delete node and all links to it?", "Delete Key", MB_OKCANCEL) == IDOK)
				DeleteNode(lpSelectedNode) ;
		}

		if (m_bSelectedLink) {
			if (::MessageBox(nullptr, "Delete link?", "Delete Key", MB_OKCANCEL) == IDOK) {

				DeleteLink() ;
				m_xpcGtlDoc->SetModifiedFlag() ;
			}
		}
	}

// cleanup:
	m_cBgbMgr.ReleaseDc() ;

	UpdateDialogs() ;

	JXELEAVE(CGtlData::AcceptDeleteKey) ;
	RETURN(iError != 0) ;
}
#endif

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

#ifdef NODEEDIT

//* CGtlData::SelectNode -- algorithm when node is clicked
BOOL CGtlData::SelectNode(CNode FAR * lpNode)
// lpNode -- pointer to node clicked on
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CGtlData::SelectNode) ;
	int iError = 0 ;            // error code

//  CNode FAR * lpSelectedNode = GetSelectedNode() ;

//  if (lpSelectedNode)         // if there's already a selected node
//      ModifySelectNode(lpSelectedNode, FALSE) ; // deselect it

	if (m_bSelectedLink)
		SelectLink() ;          // unselect selected link

	// if new node is linked to (previously) selected node
	if (m_lpLastSelectedNode && IfLinked(lpNode, m_lpLastSelectedNode)) {

		// then select the link between them
		SelectLink(lpNode, m_lpLastSelectedNode) ;

		// they're not linked
	} else {

		// select the new node
		ModifySelectNode(lpNode) ;
	}

// cleanup:
	JXELEAVE(CGtlData::SelectNode) ;
	RETURN(iError != 0) ;
}


//* CGtlData::ModifySelectNode -- modify node select/deselect
BOOL CGtlData::ModifySelectNode(CNode FAR *lpNode, BOOL bSelect)
// lpNode -- node to be selected or deselected; if nullptr, then
//              deselect currently selected node
// bSelect -- select/deselect flag (if non-null lpNull)
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CGtlData::ModifySelectNode) ;
	int iError = 0 ;            // error code
	CNode FAR * lpSelectedNode = GetSelectedNode() ;

	// deselect a node
	if (!lpNode || !bSelect) {

		if (!lpNode)
			lpNode = lpSelectedNode ;

		if (lpNode && lpNode->m_bSelected) {
			lpNode->m_bSelected = FALSE ;       // deselect
			CallUpdate(lpNode) ;                // update view
		}
	}

	// select a node
	//
	if (lpNode && bSelect) {

		// if node is not already selected
		//
		if (lpNode != lpSelectedNode) {
			SelectLink() ;              // deselect link, if necessary
			lpNode->m_bSelected = TRUE;
			CallUpdate(lpNode) ;        // update view
		}
	}

// cleanup:
	JXELEAVE(CGtlData::ModifySelectNode) ;
	RETURN(iError != 0) ;
}


//* CGtlData::GetSelectedNode -- return selected node, or nullptr if none
CNode FAR *CGtlData::GetSelectedNode(void)
// returns: selected node if any, nullptr otherwise
{
	JXENTER(CGtlData::GetSelectedNode) ;
	int iError = 0 ;            // error code
	CNode FAR * lpNode, FAR * lpSelectedNode = nullptr ;

	int iK ;                    // loop variable

	for (iK = 0 ; !lpSelectedNode && iK < m_iNodes ; ++iK)
		if (!(lpNode = m_lpNodes + iK)->m_bDeleted && lpNode->m_bSelected)
			lpSelectedNode = lpNode ;

// cleanup:

	JXELEAVE(CGtlData::GetSelectedNode) ;
	RETURN(lpSelectedNode) ;
}


//* CGtlData::SelectLink -- select or deselect a link
BOOL CGtlData::SelectLink(CNode FAR *lpNode1, CNode FAR *lpNode2)
// lpNode1, lpNode2 -- endpoints of link to be selected if non-nullptr,
//      after de-selecting any currently selected link
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CGtlData::SelectLink) ;
	int iError = 0 ;            // error code

	int iIndex1, iIndex2 ;      // node indices

	// if there's a link selected
	if (m_bSelectedLink) {

		m_bSelectedLink = FALSE ;       // turn off selection
		CallUpdate(m_lpNodes + m_iSelLinkPt1, m_lpNodes + m_iSelLinkPt2) ;
		// update views
	}
	ModifySelectNode() ;        // deselect current node if necessary

	// if nodes specified
	if (lpNode1 || lpNode2) {

		if (!lpNode1 || !lpNode2 || lpNode1->m_bDeleted || lpNode2->m_bDeleted || !IfLinked(lpNode1, lpNode2)) {
			iError = 100 ;              // unspecified or deleted node,
			// or nodes not linked
			goto cleanup ;
		}
		iIndex1 = lpNode1 - m_lpNodes ;
		iIndex2 = lpNode2 - m_lpNodes ;

		// node that node indices are stored in increasing order
		m_iSelLinkPt1 = min(iIndex1, iIndex2) ;
		m_iSelLinkPt2 = max(iIndex1, iIndex2) ;
		m_bSelectedLink = TRUE ;
		CallUpdate(lpNode1, lpNode2) ;  // update view
	}

cleanup:
	JXELEAVE(CGtlData::SelectLink) ;
	RETURN(iError != 0) ;
}


//* CGtlData::IfLinked -- test whether two nodes are linked
BOOL CGtlData::IfLinked(CNode FAR *lpNode1, CNode FAR *lpNode2)
// lpNode1, lpNode2 -- nodes to be tested
// returns: TRUE if nodes are linked, FALSE otherwise
{
	JXENTER(CGtlData::IfLinked) ;
	int iError = 0 ;            // error code
	BOOL bLinked = FALSE ;      // return value
	int iL ;            // loop variable
	int iIndex2 = lpNode2 - m_lpNodes ; // index of lpNode2

	// if either node is deleted
	if (lpNode1->m_bDeleted || lpNode2->m_bDeleted) {

		iError = 100 ;
		goto cleanup ;
	}

	// loop through all the links for node 1 to see if node 2 is
	// among them
	for (iL = 0 ; !bLinked && iL < lpNode1->m_iNumLinks ; ++iL)
		if ((lpNode1->m_iLinks[iL]) == iIndex2)
			bLinked = TRUE ;

cleanup:
	JXELEAVE(CGtlData::IfLinked) ;
	RETURN(bLinked) ;
}

//* CGtlData::DeleteNode -- delete node and all connecting links
BOOL CGtlData::DeleteNode(CNode * lpNode)
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CGtlData::DeleteNode) ;
	int iError = 0 ;            // error code
	int iLink ;                 // link number

	if (lpNode->m_bDeleted) {
		iError = 100 ;
		goto cleanup ;
	}

	// loop as long as links are left
	while (lpNode->m_iNumLinks) {

		// get linking node
		iLink = lpNode->m_iLinks[0] ;

		// delete link
		DeleteLink(lpNode, m_lpNodes + iLink) ;
	}

	CallUpdate(lpNode) ;
	lpNode->m_bDeleted = TRUE ;
	m_xpcGtlDoc->SetModifiedFlag() ;

cleanup:
	JXELEAVE(CGtlData::DeleteNode) ;
	RETURN(iError != 0) ;
}

//* CGtlData::DeleteLink -- delete link between two nodes
BOOL CGtlData::DeleteLink(CNode FAR * lpNode1, CNode FAR * lpNode2)
// lpNode1, lpNode2 -- nodes whose link is to be deleted; if nullptr,
//              then delete selected link if any
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CGtlData::DeleteLink) ;
	int iError = 0 ;            // error code
	int iIndex1, iIndex2 ;      // indexes of endpoint nodes

	if (!lpNode1 && !lpNode2 && m_bSelectedLink) {

		lpNode1 = m_lpNodes + m_iSelLinkPt1 ;   // get endpoints
		// of selected link
		lpNode2 = m_lpNodes + m_iSelLinkPt2 ;
	}

	if (lpNode1 || lpNode2) {

		if (!lpNode1 || !lpNode2 || lpNode1->m_bDeleted || lpNode2->m_bDeleted) {
			iError = 100 ;      // one of nodes is nullptr or deleted
			goto cleanup ;
		}
		iIndex1 = lpNode1 - m_lpNodes ;
		iIndex2 = lpNode2 - m_lpNodes ;
		DeleteLinkIndex(lpNode1, iIndex2) ;
		DeleteLinkIndex(lpNode2, iIndex1) ;
		if (m_bSelectedLink && iIndex1 == m_iSelLinkPt1
		        && iIndex2 == m_iSelLinkPt2)
			// if we're deleting the selected link
			m_bSelectedLink = FALSE ;
		m_xpcGtlDoc->SetModifiedFlag() ;
		CallUpdate(lpNode1, lpNode2) ;  // update all views
	}

cleanup:
	JXELEAVE(CGtlData::DeleteLink) ;
	RETURN(iError != 0) ;
}

//* CGtlData::DeleteLinkIndex -- delete index from array of link
//              indices for a given node, if found
BOOL CGtlData::DeleteLinkIndex(CNode FAR * lpNode, int iLink)
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CGtlData::DeleteLinkIndex) ;
	int iError = 0 ;            // error code
	int iL ;            // loop index
	int iFound = -1 ;   // found link value

	for (iL = 0 ; iFound < 0 && iL < lpNode->m_iNumLinks ; ++iL)
		if (lpNode->m_iLinks[iL] == iLink)
			iFound = iL ;

	// if found
	if (iFound >= 0) {

		--lpNode->m_iNumLinks ; // reduce # links by 1

		for (iL = iFound ; iL < lpNode->m_iNumLinks ; ++iL)
			lpNode->m_iLinks[iL] = lpNode->m_iLinks[iL + 1] ;
	}

// cleanup:
	JXELEAVE(CGtlData::DeleteLinkIndex) ;
	RETURN(iError != 0) ;
}
#endif

//* CGtlData::CallUpdate -- call to update all views for this map
BOOL CGtlData::CallUpdate(CNode FAR *lpNode1, CNode FAR *lpNode2, BOOL bLinks, BOOL bWmPaint)
// lpNode1, lpNode2 -- node or link specifying range to be updated
// bLinks -- if TRUE, also update all links of lpNode1
// bWmPaint -- if TRUE, use WM_PAINT message.
// returns: TRUE if error, FALSE otherwise
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

//  SetModifiedFlag(TRUE) ;
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
BOOL CGtlData::CallUpdate(CBgbObject * lpcBgbObject)
// lpcBgbObject -- pointer to object
// returns: TRUE if error, FALSE otherwise
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

#ifdef NODEEDIT
//* CGtlData::CreateNode -- create a new node
BOOL CGtlData::CreateNode(CNode FAR * &lpNode, CRPoint crPosition)
// lpNode (output) -- created node
// crPosition -- position where node is to be created
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CGtlData::CreateNode) ;
	int iError = 0 ;            // error code
	if (m_iNodes >= MAX_NODES - 1) {
		::MessageBox(nullptr, "Too many nodes.", nullptr, MB_OK | MB_ICONINFORMATION);
		goto cleanup ;
	}
	lpNode = m_lpNodes + m_iNodes++ ;
	SetNodePosition(lpNode, crPosition) ;
	lpNode->m_iWeight = 1 ;
	m_xpcGtlDoc->SetModifiedFlag() ;
	CallUpdate(lpNode) ;

cleanup:
	JXELEAVE(CGtlData::CreateNode) ;
	RETURN(iError != 0) ;
}


//* CGtlData::MoveNode -- move node
BOOL CGtlData::MoveNode(CNode FAR * lpNode, CRPoint crPosition)
// lpNode -- node to be moved
// crPosition -- position where node is to be created
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CGtlData::MoveNode) ;
	int iError = 0 ;            // error code
	CallUpdate(lpNode, nullptr, TRUE, TRUE) ;
	SetNodePosition(lpNode, crPosition) ;
	CallUpdate(lpNode, nullptr, TRUE, TRUE) ;
	m_xpcGtlDoc->SetModifiedFlag() ;

// cleanup:
	JXELEAVE(CGtlData::MoveNode) ;
	RETURN(iError != 0) ;
}


//* CGtlData::CreateLink -- create link between two nodes
BOOL CGtlData::CreateLink(CNode FAR * lpNode1, CNode FAR * lpNode2)
// lpNode1, lpNode2 -- nodes to be linked
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CGtlData::CreateLink) ;
	int iError = 0 ;            // error code
	if (!IfLinked(lpNode1, lpNode2)) {

		if (lpNode1->m_iNumLinks >= DIMENSION(lpNode1->m_iLinks) || lpNode2->m_iNumLinks >= DIMENSION(lpNode1->m_iLinks)) {
			::MessageBox(nullptr, "Too many links for node.", nullptr, MB_OK | MB_ICONINFORMATION);
			goto cleanup ;
		}
		AddLink(lpNode1, lpNode2) ;
		m_xpcGtlDoc->SetModifiedFlag() ;
	}
	SelectLink(lpNode1, lpNode2) ;

cleanup:
	JXELEAVE(CGtlData::CreateLink) ;
	RETURN(iError != 0) ;
}


//* CGtlData::SetNodePosition -- set position relative to bitmap
BOOL CGtlData::SetNodePosition(CNode FAR *lpNode, CRPoint crPosition)
// lpNode -- node whose position is to be set
// crPosition -- position of node
// returns: TRUE if error, FALSE otherwise
{
	JXENTER(CGtlData::SetNodePosition) ;
	int iError = 0 ;            // error code
	int iK ;            // loop variable
	CMap FAR * lpMap ;  // pointer to map block
	CRect cMapRect ;    // rectangle occupied by map block
	BOOL bFound = FALSE ;       // TRUE when map block is found
	CBgbObject FAR * lpcBgbObject ;

	for (iK = 0 ; !bFound && iK < m_iMaps ; ++iK) {

		lpMap = m_lpMaps + iK ;
		if (!lpMap->m_bOverlay && ((lpcBgbObject = lpMap->m_lpcBgbObject) != nullptr)) {

			cMapRect = CRect(CPoint(lpMap->m_lpcBgbObject->m_crPosition.x,
			                        lpMap->m_lpcBgbObject->m_crPosition.y),
			                 CSize(lpMap->m_lpcBgbObject->m_cSize.cx,
			                       lpMap->m_lpcBgbObject->m_cSize.cy)) ;

			if (cMapRect.PtInRect(crPosition)) {
				bFound = TRUE ;
				lpNode->m_bRelative = TRUE ;
				lpNode->m_bRelocatable = crPosition.IfRelocatable() ;
				lpNode->m_iBitmap = iK ;
				lpNode->m_iX = crPosition.x - lpMap->m_lpcBgbObject->m_crPosition.x ;
				lpNode->m_iY = crPosition.y - lpMap->m_lpcBgbObject->m_crPosition.y ;
			}
		}
	}

	// point not in any bitmap blocks
	if (!bFound) {

		lpNode->m_bRelative = FALSE ;
		lpNode->m_iBitmap = 0 ;
		lpNode->m_iX = crPosition.x ;   // absolute position
		lpNode->m_iY = crPosition.y ;
		lpNode->m_bRelocatable = crPosition.IfRelocatable() ;
	}

// cleanup:
	JXELEAVE(CGtlData::SetNodePosition) ;
	RETURN(iError != 0) ;
}
#endif

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
