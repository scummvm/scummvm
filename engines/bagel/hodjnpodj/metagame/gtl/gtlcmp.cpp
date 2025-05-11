// gtlcmp.cpp -- meta game computer play
// Written by John J. Xenakis for Boffo Games Inc., 1994

#include "stdafx.h"

#include "gtl.h"

#include "gtldoc.h"
#include "gtlview.h"
#include "gtlfrm.h"

extern CBfcMgr      *lpMetaGameStruct;
///DEFS gtldat.h

//* CGtlData::SelectBestMove -- select best move for computer play
//      (can also be used for human player for debugging or giving hints)
BOOL CGtlData::SelectBestMove(CXodj * xpXodj)
// xpXodj -- player
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::SelectBestMove) ;
    int iError = 0 ;            // error code
    CStrategyInfo * xpStrategyInfo = xpXodj->m_xpStrategyInfo ;
    CGtlApp * xpGtlApp = (CGtlApp *)AfxGetApp() ; // get application
    char szMsg[100] ;

    // we only allocate the strategy info storage when absolutely
    // necessary
    if ((xpStrategyInfo = xpXodj->m_xpStrategyInfo) == NULL) {

        if ((xpStrategyInfo = xpXodj->m_xpStrategyInfo = new CStrategyInfo) == NULL) {
            iError = 100 ;
            goto cleanup ;
        }

        // if no target location
        if (!xpXodj->m_iTargetLocation) {
            // need to gather info
            xpXodj->m_bGatherInformation = TRUE ;
        }
    }

/*
    if (( xpXodj->m_pInventory->FindItem( MG_OBJ_MISH ) != NULL ) ||
        ( xpXodj->m_pInventory->FindItem( MG_OBJ_MOSH ) != NULL ) )
        ::MessageBox( NULL, "I have Mish", "HELLO", MB_OK );
*/

    if (xpXodj->m_bGatherInformation) {

        xpXodj->m_bGatherInformation = FALSE ;
        xpXodj->m_iTargetLocation = 0 ; // no target location
        xpStrategyInfo->xpTargetLocInfo = NULL ;
                                // target location has to
                        // be recomputed
        if ((iError = GatherInformation(xpXodj)) != 0)
            goto cleanup ;
        DetermineWeights(xpXodj) ;
    }

    // if we have no target location
    if (!xpXodj->m_iTargetLocation) {

        xpStrategyInfo->xpTargetLocInfo = NULL ;
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

    if (xpGtlApp->m_bDumpGamePlay)
    {
        DumpBestMoveData(xpXodj) ;
        if (iError)
        {
            sprintf(szMsg, "    Error %d in computing best move.\n",
                                iError) ;
            JXOutputDebugString(szMsg) ;
        }
    }
        


    JXELEAVE(CGtlData::SelectBestMove) ;
    RETURN(iError != 0) ;
}


//* CGtlData::GatherInformation -- gather info to decide computer's move
BOOL CGtlData::GatherInformation(CXodj * xpXodj)
// xpXodj -- char for whom data is being gathered
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::GatherInformation) ;
    int iError = 0 ;            // error code
    int iK ;                // loop variables
    CStrategyInfo * xpStrategyInfo = xpXodj->m_xpStrategyInfo ;
    CStratLocInfo * xpStratLocInfo ;
    CGameTable * xpGameEntry ;  // pointer to game table entry
//  int iWinCode ;              // MG_WIN_xxxx (win money, object, info)
    int iObjectCode ;

    // initialize computer location array so all entries are
    // zeroed except for the location code
    for (iK = 0 ; iK < MG_LOC_MAX + 1 ; ++iK)
    {
        xpStratLocInfo = &xpStrategyInfo->m_cStratLocInfo[iK] ;
        memset(&xpStratLocInfo->m_cStartData, 0,
                &xpStratLocInfo->m_cEndData
                        - &xpStratLocInfo->m_cStartData) ;
        if (iK < MG_LOC_MAX)
        {
            xpStratLocInfo->m_iLocCode = MG_LOC_BASE + iK ;
            xpStratLocInfo->m_lpNode = LocationToNode(
                                xpStratLocInfo->m_iLocCode) ;
                        // find node corresponding to location
        }

        if (xpStratLocInfo->m_lpNode)    // if there's a node
        {
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
                if ((xpGameEntry = CMgStatic::FindGame(xpStratLocInfo->m_iValueCode)) == NULL) {

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
                                xpXodj->m_iRequiredObjectsTable[iK]) ))
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
BOOL CGtlData::DetermineWeights(CXodj * xpXodj)
// xpXodj -- char for whom weights are being determined
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::DetermineWeights) ;
    int iError = 0 ;            // error code
    int iK ;            // loop variable
    CStrategyInfo * xpStrategyInfo = xpXodj->m_xpStrategyInfo ;
    CStratLocInfo * xpStratLocInfo ;
    int		i,j;
    int     nObjectCode = -1;
    BOOL    bNeedObject = FALSE;
    BOOL	bNeedMoney;
	BOOL	bCanBuyObject;
    CNode FAR * lpNode = NULL;
	CItem	*pItem = NULL;
	BOOL	bObjInGenStore;
	BOOL	bCanSellObject;

    // For each eligible location, determine weight
    for (iK = 0 ; iK < MG_LOC_MAX ; ++iK) {
        xpStratLocInfo = &xpStrategyInfo->m_cStratLocInfo[iK] ;
        xpStratLocInfo->m_iWeight = 0 ;         // assume that
                                // this loc has no weight

        switch ( xpStratLocInfo->m_iValueCode )
        {

            case MG_VISIT_MISHMOSH:
                xpStratLocInfo->m_iWeight = MAXPOSINT;
                break ;

            case MG_VISIT_CASTLE:
                xpStratLocInfo->m_iWeight = MAXPOSINT;
                break ;

            case MG_WIN_INFO:
				if (  xpXodj->m_iWinInfoWon < xpXodj->m_iWinInfoNeed ) {
	                xpStratLocInfo->m_iWeight = 100;
				}
				else {
	                xpStratLocInfo->m_iWeight = 0;
				}
                break ;

            case MG_WIN_MONEY:
				bNeedObject = FALSE;
				bNeedMoney = FALSE;
				
				for ( i = 0; i < xpXodj->m_iWinInfoWon; i++ ) {
				
					if ( xpXodj->m_bHodj ) {
						nObjectCode = CMgStatic::cHodjClueTable[xpXodj->m_iWinInfoTable[i]].m_iArgument;
					}
					else {
						nObjectCode = CMgStatic::cPodjClueTable[xpXodj->m_iWinInfoTable[i]].m_iArgument;
					}
				
					if (( nObjectCode >= MG_OBJ_BASE ) && ( nObjectCode <= MG_OBJ_MAX )) {
						if ( GetGameObjectCount(xpXodj, nObjectCode ) == 0 ) {
							bNeedObject = TRUE;
							break;
						}
					}
				}
				
				if ( bNeedObject == FALSE ) {
					for ( i = 0; i < xpXodj->m_iSecondaryInfoWon; i++ ) {
						if ( xpXodj->m_bHodj ) {
							nObjectCode = CMgStatic::cHodjClueTable[xpXodj->m_iSecondaryInfoTable[i]].m_iArgument;
						}
						else {
							nObjectCode = CMgStatic::cPodjClueTable[xpXodj->m_iSecondaryInfoTable[i]].m_iArgument;
						}
					
						if (( nObjectCode >= MG_OBJ_BASE ) && ( nObjectCode <= MG_OBJ_MAX )) {
							if ( GetGameObjectCount(xpXodj, nObjectCode ) == 0 ) {
								bNeedObject = TRUE;
								break;
							}
						}
					}
				}
				if ( bNeedObject ) {
					pItem = xpXodj->m_pGenStore->FindItem( nObjectCode );
						
					if ( pItem == NULL )
						pItem = xpXodj->m_pBlackMarket->FindItem( nObjectCode );
	
					if (( pItem != NULL ) &&( pItem->GetValue() > GetGameObjectCount(xpXodj, MG_OBJ_CROWN )))
						bNeedMoney = TRUE;
				}
				
				if ( bNeedMoney == FALSE ) {
					if ( xpXodj->m_iSecondaryInfoWon < xpXodj->m_iSecondaryInfoNeed ) {
		                bNeedMoney = TRUE;
					}
				}

			    if ( bNeedMoney ) {
	                xpStratLocInfo->m_iWeight = 60;
			    }
				else {
	                xpStratLocInfo->m_iWeight = 0;
				}
                break ;

            case MG_WIN_OBJECT:
				for ( i = 0; i < xpXodj->m_iWinInfoWon; i++ ) {
				
					if ( xpXodj->m_bHodj ) {
						nObjectCode = CMgStatic::cHodjClueTable[xpXodj->m_iWinInfoTable[i]].m_iArgument;
					}
					else {
						nObjectCode = CMgStatic::cPodjClueTable[xpXodj->m_iWinInfoTable[i]].m_iArgument;
					}
				
					if (( nObjectCode >= MG_OBJ_BASE ) && ( nObjectCode <= MG_OBJ_MAX )) {
						if ( GetGameObjectCount(xpXodj, nObjectCode ) == 0 ) {
							bNeedObject = TRUE;
							break;
						}
					}
				}
				
				if ( bNeedObject == FALSE ) {
					for ( i = 0; i < xpXodj->m_iSecondaryInfoWon; i++ ) {
						if ( xpXodj->m_bHodj ) {
							nObjectCode = CMgStatic::cHodjClueTable[xpXodj->m_iSecondaryInfoTable[i]].m_iArgument;
						}
						else {
							nObjectCode = CMgStatic::cPodjClueTable[xpXodj->m_iSecondaryInfoTable[i]].m_iArgument;
						}
					
						if (( nObjectCode >= MG_OBJ_BASE ) && ( nObjectCode <= MG_OBJ_MAX )) {
							if ( GetGameObjectCount(xpXodj, nObjectCode ) == 0 ) {
								bNeedObject = TRUE;
								break;
							}
						}
					}
				}
			    
			    if ( bNeedObject ) {
	                xpStratLocInfo->m_iWeight = 40;
			    }
				else {
	                xpStratLocInfo->m_iWeight = 0;
				}
                break ;

            case MG_VISIT_PAWNSHOP: // sell objects 
				bNeedObject = FALSE;
				bNeedMoney = FALSE;
				bCanSellObject = FALSE;
				
				for ( i = 0; i < xpXodj->m_iWinInfoWon; i++ ) {
				
					if ( xpXodj->m_bHodj ) {
						nObjectCode = CMgStatic::cHodjClueTable[xpXodj->m_iWinInfoTable[i]].m_iArgument;
					}
					else {
						nObjectCode = CMgStatic::cPodjClueTable[xpXodj->m_iWinInfoTable[i]].m_iArgument;
					}
				
					if (( nObjectCode >= MG_OBJ_BASE ) && ( nObjectCode <= MG_OBJ_MAX )) {
						if ( GetGameObjectCount(xpXodj, nObjectCode ) == 0 ) {
							bNeedObject = TRUE;
							break;
						}
					}
				}
				
				if ( bNeedObject == FALSE ) {
					for ( i = 0; i < xpXodj->m_iSecondaryInfoWon; i++ ) {
						if ( xpXodj->m_bHodj ) {
							nObjectCode = CMgStatic::cHodjClueTable[xpXodj->m_iSecondaryInfoTable[i]].m_iArgument;
						}
						else {
							nObjectCode = CMgStatic::cPodjClueTable[xpXodj->m_iSecondaryInfoTable[i]].m_iArgument;
						}
					
						if (( nObjectCode >= MG_OBJ_BASE ) && ( nObjectCode <= MG_OBJ_MAX )) {
							if ( GetGameObjectCount(xpXodj, nObjectCode ) == 0 ) {
								bNeedObject = TRUE;
								break;
							}
						}
					}
				}
				if ( bNeedObject ) {
					pItem = xpXodj->m_pGenStore->FindItem( nObjectCode );
						
					if ( pItem == NULL )
						pItem = xpXodj->m_pBlackMarket->FindItem( nObjectCode );
	
					if (( pItem != NULL ) &&( pItem->GetValue() > GetGameObjectCount(xpXodj, MG_OBJ_CROWN )))
						bNeedMoney = TRUE;
				}
				
				if ( bNeedMoney == FALSE ) {
					if ( xpXodj->m_iSecondaryInfoWon < xpXodj->m_iSecondaryInfoNeed ) {
		                bNeedMoney = TRUE;
					}
				}


			    if ( bNeedMoney ) {
					bCanSellObject = TRUE;
					if ( xpXodj->m_pInventory->ItemCount() > 2 ) {
						for ( i = 0; i < xpXodj->m_pInventory->ItemCount(); i++ ) {
							pItem = xpXodj->m_pInventory->FetchItem( i );
							if (( pItem->m_nID != MG_OBJ_CROWN ) &&
								( pItem->m_nID != MG_OBJ_HODJ_NOTEBOOK ) &&
								( pItem->m_nID != MG_OBJ_PODJ_NOTEBOOK ) &&
								( pItem->m_nID != MG_OBJ_MISH ) &&
								( pItem->m_nID != MG_OBJ_MOSH )) {
								
								bCanSellObject = TRUE;
								for ( j = 0; j < xpXodj->m_iRequiredObjectsCount; j++ ) {
									if ( xpXodj->m_iRequiredObjectsTable[j] == pItem->m_nID ) {
										bCanSellObject = FALSE;
										break;
									}
								}
							}
							else {
								bCanSellObject = FALSE;
							}
							pItem = NULL;
							if ( bCanSellObject )
								break;
						}
					}
					else {
						bCanSellObject = FALSE;
					}
				}
				
			    if ( bCanSellObject ) {
			    CNode	*pNode = xpXodj->m_iCharNode + m_lpNodes;
					if ( pNode->m_iLocationCode == MG_LOC_PAWNSHOP )
						bCanSellObject = FALSE;
				}
			    
			    if ( bCanSellObject ) {
	                xpStratLocInfo->m_iWeight = 70;
			    }
				else {
	                xpStratLocInfo->m_iWeight = 0;
				}
                break ;
				
            case MG_VISIT_GENERALSTR: /* general store: buy objects */
            case MG_VISIT_BLACKMARKET: /* buy objects */

				bNeedObject = FALSE;
				bCanBuyObject = FALSE;
				
				//	if I know I need an object
				//		and I don't have that object
				//		and I know I that I need it
				//	then go to store if I can afford the item 
				
				for ( i = 0; i < xpXodj->m_iWinInfoWon; i++ ) {
				
					if ( xpXodj->m_bHodj ) {
						nObjectCode = CMgStatic::cHodjClueTable[xpXodj->m_iWinInfoTable[i]].m_iArgument;
					}
					else {
						nObjectCode = CMgStatic::cPodjClueTable[xpXodj->m_iWinInfoTable[i]].m_iArgument;
					}
				
					if (( nObjectCode >= MG_OBJ_BASE ) && ( nObjectCode <= MG_OBJ_MAX )) {
						if ( GetGameObjectCount(xpXodj, nObjectCode ) == 0 ) {
							bNeedObject = TRUE;
							break;
						}
					}
				}
				
				if ( bNeedObject == FALSE ) {
					for ( i = 0; i < xpXodj->m_iSecondaryInfoWon; i++ ) {
						if ( xpXodj->m_bHodj ) {
							nObjectCode = CMgStatic::cHodjClueTable[xpXodj->m_iSecondaryInfoTable[i]].m_iArgument;
						}
						else {
							nObjectCode = CMgStatic::cPodjClueTable[xpXodj->m_iSecondaryInfoTable[i]].m_iArgument;
						}
					
						if (( nObjectCode >= MG_OBJ_BASE ) && ( nObjectCode <= MG_OBJ_MAX )) {
							if ( GetGameObjectCount(xpXodj, nObjectCode ) == 0 ) {
								bNeedObject = TRUE;
								break;
							}
						}
					}
				}
				
				//	if I want to go to store
				//		and I can afford the item
				//		and I've been to a store
				//		and I know that this store doesn't have then item
				// then don't go to the store
				
				if ( bNeedObject ) {
				
					bObjInGenStore = TRUE;
					pItem = xpXodj->m_pGenStore->FindItem( nObjectCode );
					
					if ( pItem == NULL ) {
						pItem = xpXodj->m_pBlackMarket->FindItem( nObjectCode );
						bObjInGenStore = FALSE;
					}
					
					if (( pItem != NULL ) && ( pItem->GetValue() <= GetGameObjectCount(xpXodj, MG_OBJ_CROWN ))) {
				
						bCanBuyObject = TRUE;
						if ( lpMetaGameStruct->m_bVisitedStore )	{
							if ( bObjInGenStore ) {
								if ( xpStratLocInfo->m_iValueCode == MG_VISIT_BLACKMARKET ) {
									bCanBuyObject = FALSE;
								}
							}
							else{
								if ( xpStratLocInfo->m_iValueCode == MG_VISIT_GENERALSTR ) {
									bCanBuyObject = FALSE;
								}
							}
						} 
					}
				
				}

			    if ( bCanBuyObject ) {
			    CNode	*pNode = xpXodj->m_iCharNode + m_lpNodes;
					if (( pNode->m_iLocationCode == MG_LOC_GENERALSTR ) && ( xpStratLocInfo->m_iValueCode == MG_VISIT_GENERALSTR ))
						bCanBuyObject = FALSE;
					if (( pNode->m_iLocationCode == MG_LOC_BANDITHIDE ) && ( xpStratLocInfo->m_iValueCode == MG_VISIT_BLACKMARKET ))
						bCanBuyObject = FALSE;
				}
			    
			    if ( bCanBuyObject ) {
	                xpStratLocInfo->m_iWeight = 80;
			    }
				else {
	                xpStratLocInfo->m_iWeight = 0;
				}

                break ;


            case MG_VISIT_INFO:
// if I'm eligible then
				if ( xpXodj->m_iSecondaryInfoWon != xpXodj->m_iSecondaryInfoNeed ) {
	                xpStratLocInfo->m_iWeight = 100;
				}
				else {
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
BOOL CGtlData::DetermineDistances(CXodj * xpXodj)
// xpXodj -- char for whom data is being gathered
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::DetermineDistances) ;
    int iError = 0 ;            // error code
    int iK ;            // loop variable
    CStrategyInfo * xpStrategyInfo = xpXodj->m_xpStrategyInfo ;
    CStratLocInfo * xpStratLocInfo ;
    LPINT lpiShortPath = NULL ;
    CNode FAR * lpNode, FAR * lpTargetNode ;

    lpNode = m_lpNodes + xpXodj->m_iCharNode ;

    // maximum distance to an eligible location
    xpStrategyInfo->m_iMaximumDistance = 0 ;

    // For each eligible location, determine distance to it.
    //
    for (iK = 0 ; iK < MG_LOC_MAX ; ++iK) {

        xpStratLocInfo = &xpStrategyInfo->m_cStratLocInfo[iK] ;

        xpStratLocInfo->m_iDistance = 0;
        if ((lpTargetNode = xpStratLocInfo->m_lpNode) != NULL) {

            if (( xpStratLocInfo->m_iValueCode == MG_VISIT_CASTLE ) ||
                ( xpStratLocInfo->m_iValueCode == MG_VISIT_MISHMOSH )) {
            	
            	xpStratLocInfo->m_iDistance = 0;
            }
            else {
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
BOOL CGtlData::AdjustWeightByDistance(CXodj * xpXodj)
// xpXodj -- char for whom weight is being adjusted
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::AdjustWeightByDistance) ;
    int iError = 0 ;            // error code
    int iK ;            // loop variable
    CStrategyInfo * xpStrategyInfo = xpXodj->m_xpStrategyInfo ;
    CStratLocInfo * xpStratLocInfo ;

    xpStrategyInfo->m_iMaxAdjustedWeight = 0 ;

    // For each eligible location, adjust weight by distance
    for (iK = 0 ; iK < MG_LOC_MAX ; ++iK)
    {
        xpStratLocInfo = &xpStrategyInfo->m_cStratLocInfo[iK] ;

        if (( xpStratLocInfo->m_iValueCode != 0) && ( xpStratLocInfo->m_iWeight != 0 ))
        {
            xpStratLocInfo->m_iAdjustedWeight = xpStratLocInfo->m_iWeight - xpStratLocInfo->m_iDistance;
		}
		else {
            xpStratLocInfo->m_iAdjustedWeight = -1 * ( MAXPOSINT - 1 );
		}
    }

// cleanup:

    JXELEAVE(CGtlData::AdjustWeightByDistance) ;
    RETURN(iError != 0) ;
}

//* CGtlData::FindTopLocations -- find locations with top weights
BOOL CGtlData::FindTopLocations(CXodj * xpXodj)
// xpXodj -- char for whom weight is being adjusted
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::FindTopLocations) ;
    int iError = 0 ;            // error code
    int iK ;            // loop variable
    CStrategyInfo * xpStrategyInfo = xpXodj->m_xpStrategyInfo ;
    CStratLocInfo * xpStratLocInfo ;
    CStratLocInfo * xpStratTemp[3];
    int i;

    xpStrategyInfo->m_iTopLocCount = 0 ;  // not top locations yet
    xpStrategyInfo->xpTargetLocInfo = NULL ;
                        // no chosen location yet

//	xpStratTemp = &xpStrategyInfo->m_cStratLocInfo[0];
	xpStratTemp[0] = &xpStrategyInfo->m_cStratLocInfo[MG_LOC_SILO-MG_LOC_BASE];
	xpStratTemp[1] = &xpStrategyInfo->m_cStratLocInfo[MG_LOC_SILO-MG_LOC_BASE];
	xpStratTemp[2] = &xpStrategyInfo->m_cStratLocInfo[MG_LOC_SILO-MG_LOC_BASE];

    // Make a list of all the locations above the 90'th percentile
    for (iK = 0 ; iK < MG_LOC_MAX ; ++iK)
    {
        xpStratLocInfo = &xpStrategyInfo->m_cStratLocInfo[iK] ;
		
        if ( xpStratLocInfo->m_iValueCode ) {
			
			if ( xpStratLocInfo->m_iAdjustedWeight > xpStratTemp[0]->m_iAdjustedWeight ) {
				xpStratTemp[2] = xpStratTemp[1];
				xpStratTemp[1] = xpStratTemp[0];
				xpStratTemp[0] = xpStratLocInfo;
			}
			else {
				if ( xpStratLocInfo->m_iAdjustedWeight > xpStratTemp[1]->m_iAdjustedWeight ) {
					xpStratTemp[2] = xpStratTemp[1];
					xpStratTemp[1] = xpStratLocInfo;
				}
				else {
					if ( xpStratLocInfo->m_iAdjustedWeight > xpStratTemp[2]->m_iAdjustedWeight ) {
						xpStratTemp[2] = xpStratLocInfo;
					}
				}
			}

		}
	}


	i = brand() % 10;

	if ( xpStratTemp[0]->m_iValueCode == MG_VISIT_MISHMOSH ) {
		i = 9;
	}

	if ( i < 1 ) {
		xpStrategyInfo->xpTargetLocInfo	= xpStratTemp[2];
	}else {
		if ( i < 3 ) {
			xpStrategyInfo->xpTargetLocInfo	= xpStratTemp[1];
		}else {
			xpStrategyInfo->xpTargetLocInfo	= xpStratTemp[0];
		}
	}

#ifdef _DEBUG
{
char	cTemp[128];

	lstrcpy( cTemp, xpStratTemp[0]->m_lpNode->m_szLabel );
	lstrcat( cTemp, " : " );
	lstrcat( cTemp, xpStratTemp[1]->m_lpNode->m_szLabel );
	lstrcat( cTemp, " : " );
	lstrcat( cTemp, xpStratTemp[2]->m_lpNode->m_szLabel );
	 
//	Common::sprintf_s( cTemp, "%s : %s : %s", xpStratTemp[0].m_lpNode->m_szLabel, xpStratTemp[1].m_lpNode->m_szLabel, xpStratTemp[2].m_lpNode->m_szLabel);
	::MessageBox( NULL, cTemp, xpStrategyInfo->xpTargetLocInfo->m_lpNode->m_szLabel, MB_OK );
}
#endif


//cleanup:

    JXELEAVE(CGtlData::FindTopLocations) ;
    RETURN(iError != 0) ;
}


//* CGtlData::DumpBestMoveData -- dump data used in best move computation
BOOL CGtlData::DumpBestMoveData(CXodj *xpXodj)
// xpXodj -- char for whom data is being dumped
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::DumpBestMoveData) ;
    int iError = 0 ;            // error code
#ifdef _DEBUG
    int iK ;            // loop variable
    CStrategyInfo * xpStrategyInfo = xpXodj->m_xpStrategyInfo ;
    CStratLocInfo * xpStratLocInfo ;
    CLocTable * xpLoc ; // pointer to location table entry
    char szMsg[150] ;
    CGtlApp * xpGtlApp = (CGtlApp *)AfxGetApp() ; // get application

    if (xpGtlApp->m_bDumpGamePlay) {

        sprintf(szMsg, "\n  %s strategy tables:\n", xpXodj->m_szName) ;
        JXOutputDebugString(szMsg) ;

        // For each eligible location, adjust weight by distance
        for (iK = 0 ; iK < MG_LOC_MAX ; ++iK) {

            xpStratLocInfo = &xpStrategyInfo->m_cStratLocInfo[iK] ;

            xpLoc = CMgStatic::FindLoc(xpStratLocInfo->m_iLocCode) ;

            if (xpStratLocInfo->m_iValueCode)
            {
                _snprintf(szMsg, sizeof(szMsg),
                        "        Location %d = %s = node #%d: "
                        "Eligibility:%s, "
                        "Dist:%d, Weight:%d, Adjusted:%d.\n",

                        iK, xpLoc->m_lpszName,
                            xpStratLocInfo->m_lpNode ?
                                xpStratLocInfo->m_lpNode - m_lpNodes : -1,
                        xpStratLocInfo->m_iValueCode ? "TRUE" : "FALSE",
                        xpStratLocInfo->m_iDistance,
                        xpStratLocInfo->m_iWeight,
                        xpStratLocInfo->m_iAdjustedWeight) ;

                JXOutputDebugString(szMsg) ;
            }
        }

        sprintf(szMsg, "%d locations close to max adjusted "
                        " weight of %d:\n",
                xpStrategyInfo->m_iTopLocCount,
                xpStrategyInfo->m_iMaxAdjustedWeight) ;
        JXOutputDebugString(szMsg) ;

        for (iK = 0 ; iK < xpStrategyInfo->m_iTopLocCount ; ++iK)
        {
            xpStratLocInfo = &xpStrategyInfo->m_cStratLocInfo[
                        xpStrategyInfo->m_iTopLocTable[iK]] ;
            xpLoc = CMgStatic::FindLoc(xpStratLocInfo->m_iLocCode) ;

            if (xpLoc)
            {
                _snprintf(szMsg, sizeof(szMsg),
                        "        Location %d = %s = node #%d: "
                        "Eligibility:%s, "
                        "Dist:%d, Weight:%d, Adjusted:%d.%s\n",

                        iK, xpLoc->m_lpszName,
                            xpStratLocInfo->m_lpNode ?
                                xpStratLocInfo->m_lpNode - m_lpNodes : -1,
                        xpStratLocInfo->m_iValueCode ? "TRUE" : "FALSE",
                        xpStratLocInfo->m_iDistance,
                        xpStratLocInfo->m_iWeight,
                        xpStratLocInfo->m_iAdjustedWeight,
                        (xpStratLocInfo->m_iLocCode
                                        == xpXodj->m_iTargetLocation)
                                ? " (Selected target)" : "") ;
            }
            else
                sprintf(szMsg, "Invalid location code %d.\n",
                        xpStratLocInfo->m_iLocCode) ;

            JXOutputDebugString(szMsg) ;
        }
    }

// cleanup:
#endif
    JXELEAVE(CGtlData::DumpBestMoveData) ;
    RETURN(iError != 0) ;
}
