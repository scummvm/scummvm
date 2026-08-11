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
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

extern CBfcMgr      *lpMetaGameStruct;
///DEFS gtldat.h

//* CGtlData::SelectBestMove -- select best move for computer play
//      (can also be used for human player for debugging or giving hints)
bool CGtlData::SelectBestMove(CXodj * xpXodj)
// xpXodj -- player
// returns: true if error, false otherwise
{
	JXENTER(CGtlData::SelectBestMove) ;
	int iError = 0 ;            // error code
	CStrategyInfo * xpStrategyInfo = xpXodj->m_xpStrategyInfo ;
	CGtlApp * xpGtlApp = (CGtlApp *)AfxGetApp() ; // get application
	char szMsg[100] ;

	// we only allocate the strategy info storage when absolutely
	// necessary
	if ((xpStrategyInfo = xpXodj->m_xpStrategyInfo) == nullptr) {

		if ((xpStrategyInfo = xpXodj->m_xpStrategyInfo = new CStrategyInfo) == nullptr) {
			iError = 100 ;
			goto cleanup ;
		}

		// if no target location
		if (!xpXodj->m_iTargetLocation) {
			// need to gather info
			xpXodj->m_bGatherInformation = true ;
		}
	}

	/*
	    if (( xpXodj->m_pInventory->FindItem( MG_OBJ_MISH ) != nullptr ) ||
	        ( xpXodj->m_pInventory->FindItem( MG_OBJ_MOSH ) != nullptr ) )
	        ::MessageBox( nullptr, "I have Mish", "HELLO", MB_OK );
	*/

	if (xpXodj->m_bGatherInformation) {

		xpXodj->m_bGatherInformation = false ;
		xpXodj->m_iTargetLocation = 0 ; // no target location
		xpStrategyInfo->xpTargetLocInfo = nullptr ;
		// target location has to
		// be recomputed
		if ((iError = GatherInformation(xpXodj)) != 0)
			goto cleanup ;
		DetermineWeights(xpXodj) ;
	}

	// if we have no target location
	if (!xpXodj->m_iTargetLocation) {

		xpStrategyInfo->xpTargetLocInfo = nullptr ;
		DetermineDistances(xpXodj) ;    // set distances to all
		// eligible locations
		AdjustWeightByDistance(xpXodj) ;
		FindTopLocations(xpXodj) ;      // choose top locations,
		// and choose among them
		if (xpStrategyInfo->xpTargetLocInfo)
			// if there's a winner
			xpXodj->m_iTargetLocation =
			    xpStrategyInfo->xpTargetLocInfo->m_iLocCode ;
	}


cleanup:

	if (xpGtlApp->m_bDumpGamePlay) {
		DumpBestMoveData(xpXodj) ;
		if (iError) {
			Common::sprintf_s(szMsg, "    Error %d in computing best move.\n",
			                  iError) ;
			JXOutputDebugString(szMsg) ;
		}
	}



	JXELEAVE(CGtlData::SelectBestMove) ;
	RETURN(iError != 0) ;
}


//* CGtlData::GatherInformation -- gather info to decide computer's move
bool CGtlData::GatherInformation(CXodj * xpXodj)
// xpXodj -- char for whom data is being gathered
// returns: true if error, false otherwise
{
	JXENTER(CGtlData::GatherInformation) ;
	int iError = 0 ;            // error code
	int iK ;                // loop variables
	CStrategyInfo * xpStrategyInfo = xpXodj->m_xpStrategyInfo ;
	CStratLocInfo * xpStratLocInfo ;
	const CGameTable * xpGameEntry ;  // pointer to game table entry
//  int iWinCode ;              // MG_WIN_xxxx (win money, object, info)
	int iObjectCode ;

	// initialize computer location array so all entries are
	// zeroed except for the location code
	for (iK = 0 ; iK < MG_LOC_MAX + 1 ; ++iK) {
		xpStratLocInfo = &xpStrategyInfo->m_cStratLocInfo[iK] ;
		xpStratLocInfo->clear();

		if (iK < MG_LOC_MAX) {
			xpStratLocInfo->m_iLocCode = MG_LOC_BASE + iK ;
			xpStratLocInfo->m_lpNode = LocationToNode(
			                               xpStratLocInfo->m_iLocCode) ;
			// find node corresponding to location
		}

		if (xpStratLocInfo->m_lpNode) {  // if there's a node
			DetermineEligibility(xpXodj,
			                     xpStratLocInfo->m_iLocCode,
			                     xpStratLocInfo->m_iValueCode) ;
			// determine eligibility of this location, setting
			// m_iValueCode to function code if eligible or to
			// zero if not

			// if the code is a game
			if (xpStratLocInfo->m_iValueCode >= MG_GAME_BASE && xpStratLocInfo->m_iValueCode <= MG_GAME_MAX) {

				// if not a legal game code
				//
				if ((xpGameEntry = CMgStatic::FindGame(xpStratLocInfo->m_iValueCode)) == nullptr) {

					// location no longer eligible
					xpStratLocInfo->m_iValueCode = 0 ;

				} else {  // for game, change GAME code to WIN code
					xpStratLocInfo->m_iValueCode = xpGameEntry->m_iWinCode ;
				}
			}
		}
	}

	// determine what objects we still need, if any
	xpStrategyInfo->m_iRequiredObjectsCount = 0 ;
	for (iK = 0 ; iK < xpXodj->m_iRequiredObjectsCount ; ++iK)
		if (!GetGameObjectCount(xpXodj,
		                        (iObjectCode =
		                             xpXodj->m_iRequiredObjectsTable[iK])))
			// if we don't have the required object
			xpStrategyInfo->m_iRequiredObjectsTable[
			    xpStrategyInfo->m_iRequiredObjectsCount++]
			    = iObjectCode ;
	// insert object into strategy table

// cleanup:

	JXELEAVE(CGtlData::GatherInformation) ;
	RETURN(iError != 0) ;
}


//* CGtlData::DetermineWeights -- determine weight for each
//              location (independent of distance)
bool CGtlData::DetermineWeights(CXodj * xpXodj)
// xpXodj -- char for whom weights are being determined
// returns: true if error, false otherwise
{
	JXENTER(CGtlData::DetermineWeights) ;
	int iError = 0 ;            // error code
	int iK ;            // loop variable
	CStrategyInfo * xpStrategyInfo = xpXodj->m_xpStrategyInfo ;
	CStratLocInfo * xpStratLocInfo ;
	int     i, j;
	int     nObjectCode = -1;
	bool    bNeedObject = false;
	bool    bNeedMoney;
	bool    bCanBuyObject;
	CItem   *pItem = nullptr;
	bool    bObjInGenStore;
	bool    bCanSellObject;

	// For each eligible location, determine weight
	for (iK = 0 ; iK < MG_LOC_MAX ; ++iK) {
		xpStratLocInfo = &xpStrategyInfo->m_cStratLocInfo[iK] ;
		xpStratLocInfo->m_iWeight = 0 ;         // assume that
		// this loc has no weight

		switch (xpStratLocInfo->m_iValueCode) {

		case MG_VISIT_MISHMOSH:
		case MG_VISIT_CASTLE:
			xpStratLocInfo->m_iWeight = MAXPOSINT;
			break ;

		case MG_WIN_INFO:
			if (xpXodj->m_iWinInfoWon < xpXodj->m_iWinInfoNeed) {
				xpStratLocInfo->m_iWeight = 100;
			} else {
				xpStratLocInfo->m_iWeight = 0;
			}
			break ;

		case MG_WIN_MONEY:
			bNeedObject = false;
			bNeedMoney = false;

			for (i = 0; i < xpXodj->m_iWinInfoWon; i++) {

				if (xpXodj->m_bHodj) {
					nObjectCode = CMgStatic::cHodjClueTable[xpXodj->m_iWinInfoTable[i]].m_iArgument;
				} else {
					nObjectCode = CMgStatic::cPodjClueTable[xpXodj->m_iWinInfoTable[i]].m_iArgument;
				}

				if ((nObjectCode >= MG_OBJ_BASE) && (nObjectCode <= MG_OBJ_MAX)) {
					if (GetGameObjectCount(xpXodj, nObjectCode) == 0) {
						bNeedObject = true;
						break;
					}
				}
			}

			if (bNeedObject == false) {
				for (i = 0; i < xpXodj->m_iSecondaryInfoWon; i++) {
					if (xpXodj->m_bHodj) {
						nObjectCode = CMgStatic::cHodjClueTable[xpXodj->m_iSecondaryInfoTable[i]].m_iArgument;
					} else {
						nObjectCode = CMgStatic::cPodjClueTable[xpXodj->m_iSecondaryInfoTable[i]].m_iArgument;
					}

					if ((nObjectCode >= MG_OBJ_BASE) && (nObjectCode <= MG_OBJ_MAX)) {
						if (GetGameObjectCount(xpXodj, nObjectCode) == 0) {
							bNeedObject = true;
							break;
						}
					}
				}
			}
			if (bNeedObject) {
				pItem = xpXodj->m_pGenStore->FindItem(nObjectCode);

				if (pItem == nullptr)
					pItem = xpXodj->m_pBlackMarket->FindItem(nObjectCode);

				if ((pItem != nullptr) && (pItem->GetValue() > GetGameObjectCount(xpXodj, MG_OBJ_CROWN)))
					bNeedMoney = true;
			}

			if (bNeedMoney == false) {
				if (xpXodj->m_iSecondaryInfoWon < xpXodj->m_iSecondaryInfoNeed) {
					bNeedMoney = true;
				}
			}

			if (bNeedMoney) {
				xpStratLocInfo->m_iWeight = 60;
			} else {
				xpStratLocInfo->m_iWeight = 0;
			}
			break ;

		case MG_WIN_OBJECT:
			for (i = 0; i < xpXodj->m_iWinInfoWon; i++) {

				if (xpXodj->m_bHodj) {
					nObjectCode = CMgStatic::cHodjClueTable[xpXodj->m_iWinInfoTable[i]].m_iArgument;
				} else {
					nObjectCode = CMgStatic::cPodjClueTable[xpXodj->m_iWinInfoTable[i]].m_iArgument;
				}

				if ((nObjectCode >= MG_OBJ_BASE) && (nObjectCode <= MG_OBJ_MAX)) {
					if (GetGameObjectCount(xpXodj, nObjectCode) == 0) {
						bNeedObject = true;
						break;
					}
				}
			}

			if (bNeedObject == false) {
				for (i = 0; i < xpXodj->m_iSecondaryInfoWon; i++) {
					if (xpXodj->m_bHodj) {
						nObjectCode = CMgStatic::cHodjClueTable[xpXodj->m_iSecondaryInfoTable[i]].m_iArgument;
					} else {
						nObjectCode = CMgStatic::cPodjClueTable[xpXodj->m_iSecondaryInfoTable[i]].m_iArgument;
					}

					if ((nObjectCode >= MG_OBJ_BASE) && (nObjectCode <= MG_OBJ_MAX)) {
						if (GetGameObjectCount(xpXodj, nObjectCode) == 0) {
							bNeedObject = true;
							break;
						}
					}
				}
			}

			if (bNeedObject) {
				xpStratLocInfo->m_iWeight = 40;
			} else {
				xpStratLocInfo->m_iWeight = 0;
			}
			break ;

		case MG_VISIT_PAWNSHOP: // sell objects
			bNeedObject = false;
			bNeedMoney = false;
			bCanSellObject = false;

			for (i = 0; i < xpXodj->m_iWinInfoWon; i++) {

				if (xpXodj->m_bHodj) {
					nObjectCode = CMgStatic::cHodjClueTable[xpXodj->m_iWinInfoTable[i]].m_iArgument;
				} else {
					nObjectCode = CMgStatic::cPodjClueTable[xpXodj->m_iWinInfoTable[i]].m_iArgument;
				}

				if ((nObjectCode >= MG_OBJ_BASE) && (nObjectCode <= MG_OBJ_MAX)) {
					if (GetGameObjectCount(xpXodj, nObjectCode) == 0) {
						bNeedObject = true;
						break;
					}
				}
			}

			if (bNeedObject == false) {
				for (i = 0; i < xpXodj->m_iSecondaryInfoWon; i++) {
					if (xpXodj->m_bHodj) {
						nObjectCode = CMgStatic::cHodjClueTable[xpXodj->m_iSecondaryInfoTable[i]].m_iArgument;
					} else {
						nObjectCode = CMgStatic::cPodjClueTable[xpXodj->m_iSecondaryInfoTable[i]].m_iArgument;
					}

					if ((nObjectCode >= MG_OBJ_BASE) && (nObjectCode <= MG_OBJ_MAX)) {
						if (GetGameObjectCount(xpXodj, nObjectCode) == 0) {
							bNeedObject = true;
							break;
						}
					}
				}
			}
			if (bNeedObject) {
				pItem = xpXodj->m_pGenStore->FindItem(nObjectCode);

				if (pItem == nullptr)
					pItem = xpXodj->m_pBlackMarket->FindItem(nObjectCode);

				if ((pItem != nullptr) && (pItem->GetValue() > GetGameObjectCount(xpXodj, MG_OBJ_CROWN)))
					bNeedMoney = true;
			}

			if (bNeedMoney == false) {
				if (xpXodj->m_iSecondaryInfoWon < xpXodj->m_iSecondaryInfoNeed) {
					bNeedMoney = true;
				}
			}


			if (bNeedMoney) {
				bCanSellObject = true;
				if (xpXodj->m_pInventory->ItemCount() > 2) {
					for (i = 0; i < xpXodj->m_pInventory->ItemCount(); i++) {
						pItem = xpXodj->m_pInventory->FetchItem(i);
						if ((pItem->m_nID != MG_OBJ_CROWN) &&
						        (pItem->m_nID != MG_OBJ_HODJ_NOTEBOOK) &&
						        (pItem->m_nID != MG_OBJ_PODJ_NOTEBOOK) &&
						        (pItem->m_nID != MG_OBJ_MISH) &&
						        (pItem->m_nID != MG_OBJ_MOSH)) {

							bCanSellObject = true;
							for (j = 0; j < xpXodj->m_iRequiredObjectsCount; j++) {
								if (xpXodj->m_iRequiredObjectsTable[j] == pItem->m_nID) {
									bCanSellObject = false;
									break;
								}
							}
						} else {
							bCanSellObject = false;
						}
						pItem = nullptr;
						if (bCanSellObject)
							break;
					}
				} else {
					bCanSellObject = false;
				}
			}

			if (bCanSellObject) {
				CNode   *pNode = xpXodj->m_iCharNode + m_lpNodes;
				if (pNode->m_iLocationCode == MG_LOC_PAWNSHOP)
					bCanSellObject = false;
			}

			if (bCanSellObject) {
				xpStratLocInfo->m_iWeight = 70;
			} else {
				xpStratLocInfo->m_iWeight = 0;
			}
			break ;

		case MG_VISIT_GENERALSTR: /* general store: buy objects */
		case MG_VISIT_BLACKMARKET: /* buy objects */

			bNeedObject = false;
			bCanBuyObject = false;

			//  if I know I need an object
			//      and I don't have that object
			//      and I know I that I need it
			//  then go to store if I can afford the item

			for (i = 0; i < xpXodj->m_iWinInfoWon; i++) {

				if (xpXodj->m_bHodj) {
					nObjectCode = CMgStatic::cHodjClueTable[xpXodj->m_iWinInfoTable[i]].m_iArgument;
				} else {
					nObjectCode = CMgStatic::cPodjClueTable[xpXodj->m_iWinInfoTable[i]].m_iArgument;
				}

				if ((nObjectCode >= MG_OBJ_BASE) && (nObjectCode <= MG_OBJ_MAX)) {
					if (GetGameObjectCount(xpXodj, nObjectCode) == 0) {
						bNeedObject = true;
						break;
					}
				}
			}

			if (bNeedObject == false) {
				for (i = 0; i < xpXodj->m_iSecondaryInfoWon; i++) {
					if (xpXodj->m_bHodj) {
						nObjectCode = CMgStatic::cHodjClueTable[xpXodj->m_iSecondaryInfoTable[i]].m_iArgument;
					} else {
						nObjectCode = CMgStatic::cPodjClueTable[xpXodj->m_iSecondaryInfoTable[i]].m_iArgument;
					}

					if ((nObjectCode >= MG_OBJ_BASE) && (nObjectCode <= MG_OBJ_MAX)) {
						if (GetGameObjectCount(xpXodj, nObjectCode) == 0) {
							bNeedObject = true;
							break;
						}
					}
				}
			}

			//  if I want to go to store
			//      and I can afford the item
			//      and I've been to a store
			//      and I know that this store doesn't have then item
			// then don't go to the store

			if (bNeedObject) {

				bObjInGenStore = true;
				pItem = xpXodj->m_pGenStore->FindItem(nObjectCode);

				if (pItem == nullptr) {
					pItem = xpXodj->m_pBlackMarket->FindItem(nObjectCode);
					bObjInGenStore = false;
				}

				if ((pItem != nullptr) && (pItem->GetValue() <= GetGameObjectCount(xpXodj, MG_OBJ_CROWN))) {

					bCanBuyObject = true;
					if (lpMetaGameStruct->m_bVisitedStore) {
						if (bObjInGenStore) {
							if (xpStratLocInfo->m_iValueCode == MG_VISIT_BLACKMARKET) {
								bCanBuyObject = false;
							}
						} else {
							if (xpStratLocInfo->m_iValueCode == MG_VISIT_GENERALSTR) {
								bCanBuyObject = false;
							}
						}
					}
				}

			}

			if (bCanBuyObject) {
				CNode   *pNode = xpXodj->m_iCharNode + m_lpNodes;
				if ((pNode->m_iLocationCode == MG_LOC_GENERALSTR) && (xpStratLocInfo->m_iValueCode == MG_VISIT_GENERALSTR))
					bCanBuyObject = false;
				if ((pNode->m_iLocationCode == MG_LOC_BANDITHIDE) && (xpStratLocInfo->m_iValueCode == MG_VISIT_BLACKMARKET))
					bCanBuyObject = false;
			}

			if (bCanBuyObject) {
				xpStratLocInfo->m_iWeight = 80;
			} else {
				xpStratLocInfo->m_iWeight = 0;
			}

			break ;


		case MG_VISIT_INFO:
// if I'm eligible then
			if (xpXodj->m_iSecondaryInfoWon != xpXodj->m_iSecondaryInfoNeed) {
				xpStratLocInfo->m_iWeight = 100;
			} else {
				xpStratLocInfo->m_iWeight = 0;
			}
			break ;

		// location is ineligible
		default:
			xpStratLocInfo->m_iWeight = 0 ;
			break ;
		}
	}

// cleanup:

	JXELEAVE(CGtlData::DetermineWeights) ;
	RETURN(iError != 0) ;
}


//* CGtlData::DetermineDistances -- determine distance to each
//                      eligible location
bool CGtlData::DetermineDistances(CXodj * xpXodj)
// xpXodj -- char for whom data is being gathered
// returns: true if error, false otherwise
{
	JXENTER(CGtlData::DetermineDistances) ;
	int iError = 0 ;            // error code
	int iK ;            // loop variable
	CStrategyInfo * xpStrategyInfo = xpXodj->m_xpStrategyInfo ;
	CStratLocInfo * xpStratLocInfo ;
	CNode FAR * lpNode, FAR * lpTargetNode ;

	lpNode = m_lpNodes + xpXodj->m_iCharNode ;

	// maximum distance to an eligible location
	xpStrategyInfo->m_iMaximumDistance = 0 ;

	// For each eligible location, determine distance to it.
	//
	for (iK = 0 ; iK < MG_LOC_MAX ; ++iK) {

		xpStratLocInfo = &xpStrategyInfo->m_cStratLocInfo[iK] ;

		xpStratLocInfo->m_iDistance = 0;
		if ((lpTargetNode = xpStratLocInfo->m_lpNode) != nullptr) {

			if ((xpStratLocInfo->m_iValueCode == MG_VISIT_CASTLE) ||
			        (xpStratLocInfo->m_iValueCode == MG_VISIT_MISHMOSH)) {

				xpStratLocInfo->m_iDistance = 0;
			} else {
				xpStratLocInfo->m_iDistance = EstimatePathDistance(lpNode, lpTargetNode);
			}
		}
	}

// cleanup:

	JXELEAVE(CGtlData::DetermineDistances) ;
	RETURN(iError != 0) ;
}

//* CGtlData::AdjustWeightByDistance -- adjust each by the distance
//              of the node
bool CGtlData::AdjustWeightByDistance(CXodj * xpXodj)
// xpXodj -- char for whom weight is being adjusted
// returns: true if error, false otherwise
{
	JXENTER(CGtlData::AdjustWeightByDistance) ;
	int iError = 0 ;            // error code
	int iK ;            // loop variable
	CStrategyInfo * xpStrategyInfo = xpXodj->m_xpStrategyInfo ;
	CStratLocInfo * xpStratLocInfo ;

	xpStrategyInfo->m_iMaxAdjustedWeight = 0 ;

	// For each eligible location, adjust weight by distance
	for (iK = 0 ; iK < MG_LOC_MAX ; ++iK) {
		xpStratLocInfo = &xpStrategyInfo->m_cStratLocInfo[iK] ;

		if ((xpStratLocInfo->m_iValueCode != 0) && (xpStratLocInfo->m_iWeight != 0)) {
			xpStratLocInfo->m_iAdjustedWeight = xpStratLocInfo->m_iWeight - xpStratLocInfo->m_iDistance;
		} else {
			xpStratLocInfo->m_iAdjustedWeight = -1 * (MAXPOSINT - 1);
		}
	}

// cleanup:

	JXELEAVE(CGtlData::AdjustWeightByDistance) ;
	RETURN(iError != 0) ;
}

//* CGtlData::FindTopLocations -- find locations with top weights
bool CGtlData::FindTopLocations(CXodj * xpXodj)
// xpXodj -- char for whom weight is being adjusted
// returns: true if error, false otherwise
{
	JXENTER(CGtlData::FindTopLocations) ;
	int iError = 0 ;            // error code
	int iK ;            // loop variable
	CStrategyInfo * xpStrategyInfo = xpXodj->m_xpStrategyInfo ;
	CStratLocInfo * xpStratLocInfo ;
	CStratLocInfo * xpStratTemp[3];
	int i;

	xpStrategyInfo->m_iTopLocCount = 0 ;  // not top locations yet
	xpStrategyInfo->xpTargetLocInfo = nullptr ;
	// no chosen location yet

//	xpStratTemp = &xpStrategyInfo->m_cStratLocInfo[0];
	xpStratTemp[0] = &xpStrategyInfo->m_cStratLocInfo[MG_LOC_SILO - MG_LOC_BASE];
	xpStratTemp[1] = &xpStrategyInfo->m_cStratLocInfo[MG_LOC_SILO - MG_LOC_BASE];
	xpStratTemp[2] = &xpStrategyInfo->m_cStratLocInfo[MG_LOC_SILO - MG_LOC_BASE];

	// Make a list of all the locations above the 90'th percentile
	for (iK = 0 ; iK < MG_LOC_MAX ; ++iK) {
		xpStratLocInfo = &xpStrategyInfo->m_cStratLocInfo[iK] ;

		if (xpStratLocInfo->m_iValueCode) {

			if (xpStratLocInfo->m_iAdjustedWeight > xpStratTemp[0]->m_iAdjustedWeight) {
				xpStratTemp[2] = xpStratTemp[1];
				xpStratTemp[1] = xpStratTemp[0];
				xpStratTemp[0] = xpStratLocInfo;
			} else {
				if (xpStratLocInfo->m_iAdjustedWeight > xpStratTemp[1]->m_iAdjustedWeight) {
					xpStratTemp[2] = xpStratTemp[1];
					xpStratTemp[1] = xpStratLocInfo;
				} else {
					if (xpStratLocInfo->m_iAdjustedWeight > xpStratTemp[2]->m_iAdjustedWeight) {
						xpStratTemp[2] = xpStratLocInfo;
					}
				}
			}

		}
	}


	i = brand() % 10;

	if (xpStratTemp[0]->m_iValueCode == MG_VISIT_MISHMOSH) {
		i = 9;
	}

	if (i < 1) {
		xpStrategyInfo->xpTargetLocInfo = xpStratTemp[2];
	} else {
		if (i < 3) {
			xpStrategyInfo->xpTargetLocInfo = xpStratTemp[1];
		} else {
			xpStrategyInfo->xpTargetLocInfo = xpStratTemp[0];
		}
	}

//cleanup:

	JXELEAVE(CGtlData::FindTopLocations) ;
	RETURN(iError != 0) ;
}


//* CGtlData::DumpBestMoveData -- dump data used in best move computation
bool CGtlData::DumpBestMoveData(CXodj *xpXodj) {
	// No implementation
	return false;
}

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
