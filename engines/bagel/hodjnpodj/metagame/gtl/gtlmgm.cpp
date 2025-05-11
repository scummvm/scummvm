// gtlmgm.cpp -- meta game
// Written by John J. Xenakis for Boffo Games Inc., 1994

#include "stdafx.h"

#include <string.h>

#include "bagel/boflib/misc.h"
#include "bagel/hodjnpodj/hnplibs/sprite.h"

#include "gtl.h"

#include "gtldoc.h"
#include "gtlview.h"
#include "gtlfrm.h"
#include "mgstat.h"
#include "c1btndlg.h"

extern CGtlFrame    *pMainWindow;
extern CBfcMgr      *lpMetaGameStruct;
extern BOOL			bExitMetaDLL;

int gnFurlongs = 10;

extern char *szGameSounds[MG_SOUND_COUNT];

#define MAX_SPECIAL_LINKS 14

struct SPECIAL_TRAVEL {
    char *pszLink[MAX_SPECIAL_LINKS];
    char *pszFinalNode;
    char *pszFileName;
    int   iHodjArrivalSoundID;
    int   iPodjArrivalSoundID;
};

static struct SPECIAL_TRAVEL aTravelArray[MG_SPECIAL_VISIT_COUNT] = {
    {{"st35", "st36", "st37", "st38", "st39", "st40", NULL}, "Boat2", ".\\ART\\?0BOAT.BMP", 0, 0},
    {{"st40", "st39", "st38", "st37", "st36", "st35", NULL}, "Boat1", ".\\ART\\?0BOAT.BMP", 0, 0},
    {{"st30", "st31", "st32", "st33", "st34", "st35", NULL}, "Boat1", ".\\ART\\?0BOAT.BMP", 0, 0},
    {{"st35", "st34", "st33", "st32", "st31", "st30", NULL}, "Boat3", ".\\ART\\?0BOAT.BMP", 0, 0},

    {{"st01", "st02", "st03", "st04", NULL, NULL, NULL}, "Sky2", ".\\ART\\?0SKY.BMP", 0, 0},
    {{"st04", "st03", "st02", "st01", NULL, NULL, NULL}, "Sky1", ".\\ART\\?0SKY.BMP", 0, 0},

    {{"st22", "st21", "st20", "st19", "st18", "st17", "st15", "st14", "st13", "st11", "st10", "Aerie", NULL}, NULL, ".\\ART\\?0CAR.BMP", MG_SOUND_TRAN1, MG_SOUND_TRAN2},
    {{NULL, NULL, NULL, NULL, NULL, NULL, NULL}, NULL, ".\\ART\\?0WALK.BMP", 0, 0},
    {{NULL, NULL, NULL, NULL, NULL, NULL, NULL}, "Secret2", ".\\ART\\?0WALK.BMP", MG_SOUND_TRAN5, MG_SOUND_TRAN6},
    {{NULL, NULL, NULL, NULL, NULL, NULL, NULL}, "Secret1" , ".\\ART\\?0WALK.BMP", MG_SOUND_TRAN5, MG_SOUND_TRAN6}
};

// CGtlData::SetMetaGame -- set meta game on or off
BOOL CGtlData::SetMetaGame(BOOL bOn)
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::SetMetaGame);
    int iError = 0 ;            // error code
    BOOL bFound = FALSE ;               // node position found
    CPoint cLocation ;

    // turn meta game on
    if (bOn && (!m_bMetaGame || !m_bInitMetaGame || m_bStartMetaGame)) {
        m_bStartMetaGame = FALSE ;
        m_bMetaGame = m_bInitMetaGame = TRUE ;
        ((CGtlDoc *)m_xpcGtlDoc)->UpdateAllViews(NULL, HINT_INIT_METAGAME, NULL) ;
    }

    // turn off meta game
    if (!bOn && m_bMetaGame) {
//      dbgtrc = TRUE ;

        m_bMetaGame = FALSE ;
        m_bInitMetaGame = TRUE ;
        InitMetaGame(NULL, FALSE) ;             // release sprites
        ((CGtlDoc *)m_xpcGtlDoc)->UpdateAllViews(NULL, HINT_INIT_METAGAME, NULL);
    }

// cleanup:

    JXELEAVE(CGtlData::SetMetaGame) ;
    RETURN(iError != 0) ;
}

//* CGtlData::InitMetaGame -- init or release sprites for Meta Game
BOOL CGtlData::InitMetaGame(CGtlView *xpGtlView, BOOL bInit)
// xpGtlView -- view window (only for init)
// bInit -- initialize if TRUE, release if FALSE
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::InitMetaGame) ;
    int iError = 0 ;            // error code
    CXodj * xpXodj ;            // character block pointer
    BOOL bCenter = FALSE ;      // flag: centered
    CNode FAR *lpNode = NULL;
    BOOL bNodeFound ;
    int iN ;                    // loop variable

    // if we want to initialize, and initialization hasn't previously occurred
    //
    if (bInit && m_bInitMetaGame) {

//      m_cBgbMgr.InitDc(xpGtlView) ;

        m_cBgbMgr.InitDc(xpGtlView, &xpGtlView->m_cViewBsuSet) ;
        m_cBgbMgr.AdjustLockCount(+1) ;

        AdjustToView(xpGtlView) ;
        m_bInitMetaGame = FALSE ;       // no more need to initialize

        m_cBgbMgr.AdjustLockCount(-1) ;

        m_cBgbMgr.ReleaseDc() ;

        ReturnFromInterface() ;
        
        if (!m_iMishMoshLoc)    // if problem not yet set up
            InitProblem() ;     // initialize the problem

        for (xpXodj = m_xpXodjChain ; xpXodj ; xpXodj = xpXodj->m_xpXodjNext)

            if (xpXodj->m_lpcCharSprite) {
                bNodeFound = FALSE ;
                if (xpXodj->m_iCharNode < m_iNodes && !(lpNode = m_lpNodes + xpXodj->m_iCharNode)->m_bDeleted && lpNode->m_bRelocatable)
                    bNodeFound = TRUE ;

                for (iN = 0 ; !bNodeFound && iN < m_iNodes ; iN++)
                    if ( !(lpNode = m_lpNodes + iN)->m_bDeleted && lpNode->m_bRelocatable)
                        xpXodj->m_iCharNode = iN, bNodeFound = TRUE ;

                if (bNodeFound) {
                    xpXodj->m_lpcCharSprite->m_crPosition = NodeToPoint(lpNode, &xpXodj->m_lpcCharSprite->m_cSize) ;
                    //CallUpdate(xpXodj->m_lpcCharSprite) ;
                                // invalidate char's rectangle
                    //if (!bCenter)
                        //xpGtlView->m_cViewBsuSet.EdgeToCenter(xpXodj->m_lpcCharSprite->m_crPosition) ;
                    bCenter = TRUE ;
                }
            }
        //ReturnFromInterface() ;
#ifdef NODEEDIT
        ModifySelectNode() ;    // deselect any node
        SelectLink() ;          // deselect any link
#endif

        // Centered screen around current player
        //
        if (!m_bJustPlayedMiniGame || (m_xpCurXodj->m_nTurns > 0)) {
            xpGtlView->m_cViewBsuSet.EdgeToCenter(m_xpCurXodj->m_lpcCharSprite->m_crPosition, TRUE);
        } else {
            CXodj *pOtherXodj;

            pOtherXodj = m_xpXodjChain;
            if (m_xpCurXodj == m_xpXodjChain)
                pOtherXodj = pOtherXodj->m_xpXodjNext;
            xpGtlView->m_cViewBsuSet.EdgeToCenter(pOtherXodj->m_lpcCharSprite->m_crPosition, TRUE);
        }

        //ClearInhibitDraw() ;    // clear draw inhibit flag, if set
        ProcessMove(NULL) ;     // play computer moves, if any
//      EndMoveProcessing(xpGtlView) ;
    }

    // if it's time to release
    //
    if (!bInit) {

        for (xpXodj = m_xpXodjChain ; xpXodj ; xpXodj = xpXodj->m_xpXodjNext) {
            if (xpXodj->m_lpcCharSprite) {
                CallUpdate(xpXodj->m_lpcCharSprite) ;
                                // invalidate char's rectangle
                m_cBgbMgr.ClearBitmapObject(xpXodj->m_lpcCharSprite) ;
            }
        }
    }

// cleanup:

    JXELEAVE(CGtlData::InitMetaGame) ;
    RETURN(iError != 0) ;
}

//* CGtlData::ProcessMove -- handle move processing
BOOL CGtlData::ProcessMove(CNode FAR *lpTargetNode)
// lpTargetNode -- node to be moved to
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::ProcessMove) ;
    int iError = 0 ;            // error code
    CNode FAR * lpNode ;
    CGtlApp * xpGtlApp = (CGtlApp *)AfxGetApp() ; // get application

    // if current character is moving
    if (lpTargetNode && !m_bGameOver) {

		xpGtlApp->DoWaitCursor(1);
        // make current move to that node
        MoveCharToNode(lpTargetNode);
		xpGtlApp->DoWaitCursor(-1);
    }

    EndMoveProcessing();

    // as long as the computer's on the move, then compute move and
    // play it
    while ((m_xpCurXodj != NULL) && (m_xpCurXodj->m_bComputer) && (m_bGameOver == FALSE) && (bExitMetaDLL == FALSE)) {

        if ((( m_xpCurXodj->m_bHodj ) && ( lpMetaGameStruct->m_cHodj.m_bHaveMishMosh )) ||
            (( m_xpCurXodj->m_bHodj == FALSE ) && ( lpMetaGameStruct->m_cPodj.m_bHaveMishMosh )))
            m_xpCurXodj->m_iTargetLocation = MG_LOC_CASTLE;
        else {

    		if (( lpMetaGameStruct->m_cHodj.m_bHaveMishMosh ) && ( m_xpCurXodj->m_bHodj == FALSE )) {
				m_xpCurXodj->m_iTargetLocation = MG_LOC_HODJ;
			}
			else {
            	SelectBestMove(m_xpCurXodj);
            }
        }
        // are we going to target?
        if (m_xpCurXodj->m_iTargetLocation) {

            lpNode = LocationToNode(m_xpCurXodj->m_iTargetLocation) ;
            if (lpNode) {
				xpGtlApp->DoWaitCursor(1);
                MoveCharToNode(lpNode) ;
				xpGtlApp->DoWaitCursor(-1);
                EndMoveProcessing() ;
            }
        }
    }

    // test debugging option to give hint for next human move
    //
    if (xpGtlApp->m_cBdbgMgr.GetDebugInt("givehints")) {

        SelectBestMove(m_xpCurXodj) ;           // do best move analysis
        m_xpCurXodj->m_iTargetLocation = 0 ;    // but ignore result
    }

// cleanup:

    JXELEAVE(CGtlData::ProcessMove) ;
    RETURN(iError != 0) ;
}

//* CGtlData::MoveCharToNode -- move current character to specified node
BOOL CGtlData::MoveCharToNode(CNode FAR *lpTargetNode)
// lpTargetNode -- node to be moved to
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::MoveCharToNode) ;
    int iError = 0 ;            // error code
    CNode FAR * lpNode, * lpNextNode ;
    CBgbObject FAR * lpChar ;
    CMap *lpMap;
    CPoint cOldPosition, cNewPosition ;
    int iK ;            // loop variable
    BOOL bDone = FALSE ;        // flag: done with move

    LPINT lpiShortPath = NULL ; // shortest path between nodes

    gnFurlongs = 0;

    // no current char sprite
    if ((m_xpCurXodj == NULL) || ((lpChar = m_xpCurXodj->m_lpcCharSprite) == NULL)) {
        ::MessageBox(NULL, "No current char sprite.", NULL, MB_OK);
        iError = 100 ;
        goto cleanup ;
    }

    lpNode = m_lpNodes + m_xpCurXodj->m_iCharNode ;

    if ((lpiShortPath = FindShortestPath(lpNode, lpTargetNode)) == NULL) {
	    C1ButtonDialog dlg1Button((CWnd *)pMainWindow, m_cBgbMgr.m_xpGamePalette, "&OK", "You can't get there", "from here...");
		(void) dlg1Button.DoModal();
        iError = 101 ;
        goto cleanup ;
    }

    // lpiShortPath[0] contains the number of nodes in the shortest
    // path, and lpiShortPath[1] contains the total weight.
    // The remaining array elements contain the path nodes.
    for (iK = 3 ; m_bMetaGame && !bDone && iK <= lpiShortPath[0] ; ++iK) {

        lpNextNode = m_lpNodes + lpiShortPath[iK] ;
        bDone = (m_xpCurXodj->m_iFurlongs < lpNextNode->m_iWeight) ;
        if (!bDone) {

            m_xpCurXodj->m_iFurlongs -= lpNextNode->m_iWeight;
            gnFurlongs = m_xpCurXodj->m_iFurlongs;
            lpMetaGameStruct->m_bChanged = TRUE;
            SetFurlongs(m_xpCurXodj);
            cOldPosition = NodeToPoint(lpNode, &lpChar->m_cSize) ;
            cNewPosition = NodeToPoint(lpNextNode, &lpChar->m_cSize) ;

            LoadCharDirection(lpChar, cOldPosition, cNewPosition);

            m_cBgbMgr.AnimateSprite(lpChar, cOldPosition, cNewPosition) ;
            m_xpCurXodj->m_iCharNode = lpNextNode - m_lpNodes ;

            // update the furlongs at each step
            //
            if (!lpMetaGameStruct->m_bScrolling) {

                if ((m_xpCurXodj->m_iFurlongs >= 0) && (m_xpCurXodj->m_iFurlongs < 24) && ((lpMap = m_lpFurlongMaps[m_xpCurXodj->m_iFurlongs]) != NULL) && lpMap->m_lpcBgbObject)
                    DrawABitmap(NULL, lpMap->m_lpcBgbObject);
            }
        }
        lpNode = lpNextNode ;
        if (iK == 3)
            PositionCharacters() ;

        // don't allow player to stop on a pass thru node
        if (m_xpCurXodj->m_iFurlongs <= 0)
            break;
    }

cleanup:
    if (lpiShortPath)
        delete [] lpiShortPath ;

    gnFurlongs = 10;

    JXELEAVE(CGtlData::MoveCharToNode) ;
    RETURN(iError != 0) ;
}


/*****************************************************************************
*
*  DoSpecialTravel  - Moves player between special travel nodes
*
*  DESCRIPTION:
*
*
*  SAMPLE USAGE:
*  iNodeId = DoSpecialTravel(iStrartNodeId);
*  int iStartNodeId;                        Index of starting node
*
*  RETURNS:  int = index of destination node.
*
*****************************************************************************/
int CGtlData::DoSpecialTravel(int iVisitId, BOOL bHodj)
{
    char szOldFileName[32];
    CPoint ptNew, ptOld;
    CPoint ptOffset(0, 0);
    CBgbObject *pCurPlayer;
    int i, nId, iSpecialIndex;
    int iLinkThis, iLinkNext;
    int iHodjSound, iPodjSound;
    char chPlayerChar;
    char *pSoundFile;
    CSound *pSound;
    CGtlApp * xpGtlApp = (CGtlApp *)AfxGetApp() ; // get application
    BOOL bDoTransport;

	xpGtlApp->DoWaitCursor(1);

    // validate the input
    assert((iVisitId >= MG_SPECIAL_VISIT_BASE) && (iVisitId <= MG_SPECIAL_VISIT_MAX));

    // get the index into the Special travel array
    iSpecialIndex = iVisitId - MG_SPECIAL_VISIT_BASE;

    // must have a valid player object
    //
    assert(m_xpCurXodj != NULL);
    assert(m_xpCurXodj->m_lpcCharSprite != NULL);

    pCurPlayer = m_xpCurXodj->m_lpcCharSprite;

    // object must be in memory to get access to the sprite
    m_cBgbMgr.CacheLoadObject(pCurPlayer);

    // if there is no DC, then get one
    if (m_cBgbMgr.m_xpDc == NULL) {
        m_cBgbMgr.ReInitDc();
    }

    assert(m_cBgbMgr.m_xpBsuSet != NULL);
    if (pCurPlayer->IfRelocatable()) {
        m_cBgbMgr.m_xpBsuSet->PrepareDc(m_cBgbMgr.m_xpDc, TRUE);
    } else {
        m_cBgbMgr.m_xpBsuSet->PrepareDc(m_cBgbMgr.m_xpDc, FALSE);
    }

    // save old filename
    strcpy(szOldFileName, pCurPlayer->m_szFileName);

    // load the appropriate cel strip (new filename)
    //
    chPlayerChar = pCurPlayer->m_szFileName[6];
    strcpy(pCurPlayer->m_szFileName, aTravelArray[iSpecialIndex].pszFileName);
    pCurPlayer->m_szFileName[6] = chPlayerChar;

    // erase old image
    ((CSprite *)pCurPlayer->m_pObject)->EraseSprite(m_cBgbMgr.m_xpDc);

    if (!FileExists(pCurPlayer->m_szFileName))
        ErrorLog("ERROR.LOG", "File not found: %s", pCurPlayer->m_szFileName);

    assert(FileExists(pCurPlayer->m_szFileName));
    ((CSprite *)pCurPlayer->m_pObject)->LoadCels(m_cBgbMgr.m_xpDc, pCurPlayer->m_szFileName, pCurPlayer->m_nCels);

    // move from node to node
    //

    for (i = 0; i < MAX_SPECIAL_LINKS - 1; i++) {
     
        // stop if at end of itinerary
        //
        if (aTravelArray[iSpecialIndex].pszLink[i] == NULL || aTravelArray[iSpecialIndex].pszLink[i+1] == NULL)
            break;

        // find the node id's that correspond to these node labels
        //
        iLinkThis = FindNodeId(aTravelArray[iSpecialIndex].pszLink[i]);
        iLinkNext = FindNodeId(aTravelArray[iSpecialIndex].pszLink[i+1]);

        // those label had to be valid
        assert((iLinkThis != -1) && (iLinkNext != -1));

        ptOld = NodeToPoint(m_lpNodes + iLinkThis, &pCurPlayer->m_cSize);
        ptNew = NodeToPoint(m_lpNodes + iLinkNext, &pCurPlayer->m_cSize);

        // handle special instance when we want player to get out of mining
        // car and walk the rest of the way to the Aerie
        //
        if (stricmp(aTravelArray[iSpecialIndex].pszLink[i+1], "Aerie") == 0) {

            // put back old filename
            strcpy(pCurPlayer->m_szFileName, szOldFileName);
            assert(FileExists(pCurPlayer->m_szFileName));

            // set direction index to a bogus number so a new BMP is loaded
            pCurPlayer->m_szFileName[7] = 9;

            // erase old image
            ((CSprite *)pCurPlayer->m_pObject)->EraseSprite(m_cBgbMgr.m_xpDc);
        }

        LoadCharDirection(pCurPlayer, ptOld, ptNew);

        m_cBgbMgr.AnimateSprite(pCurPlayer, ptOld, ptNew);
    }

    nId = -1;
    // get player's new final location
    if (aTravelArray[iSpecialIndex].pszFinalNode != NULL)
        nId = FindNodeId(aTravelArray[iSpecialIndex].pszFinalNode);

    // if random transport
    //
    bDoTransport = FALSE;
    if (nId == -1) {

        bDoTransport = TRUE;

        switch (iVisitId) {

            // Aerie Transport
            //
            case MG_VISIT_CAR1:

                switch (brand() % 5) {
                    case 0:
                        nId = FindNodeId("AerieBT1");
                        break;

                    case 1:
                        nId = FindNodeId("AerieBT2");
                        break;

                    case 2:
                        nId = FindNodeId("AerieBT3");
                        break;

                    case 3:
                        nId = FindNodeId("AerieBT4");
                        break;

                    default:
                        nId = FindNodeId("AerieBT5");
                        break;

                }
                break;

            // Oasis Transport
            //
            case MG_VISIT_OASIS:
                switch (brand() % 8) {

                    case 0:
                        nId = FindNodeId("OasisBT1");
                        break;

                    case 1:
                        nId = FindNodeId("OasisBT2");
                        break;

                    case 2:
                        nId = FindNodeId("OasisBT3");
                        break;

                    case 3:
                        nId = FindNodeId("OasisBT4");
                        break;

                    case 4:
                        nId = FindNodeId("OasisBT5");
                        break;

                    case 5:
                        nId = FindNodeId("OasisBT6");
                        break;

                    case 6:
                        nId = FindNodeId("OasisBT7");
                        break;

                    default:
                        nId = FindNodeId("OasisBT8");
                        break;
                }
                break;

            // invalid MG_VISIT_xxx ID
            //
            default:
                assert(0);
                break;
        }
    }
    ptNew = NodeToPoint(m_lpNodes + nId, &pCurPlayer->m_cSize);

    // play arrival narration
    //
    iHodjSound = aTravelArray[iSpecialIndex].iHodjArrivalSoundID;
    iPodjSound = aTravelArray[iSpecialIndex].iPodjArrivalSoundID;

    if ((iHodjSound != 0) && (iPodjSound != 0)) {

        // validate the arrival sound IDs
        assert((iHodjSound >= MG_SOUND_BASE) && (iHodjSound <= MG_SOUND_MAX));
        assert((iPodjSound >= MG_SOUND_BASE) && (iPodjSound <= MG_SOUND_MAX));

        pSoundFile = szGameSounds[iPodjSound - MG_SOUND_BASE];
        if (bHodj)
            pSoundFile = szGameSounds[iHodjSound - MG_SOUND_BASE];

        pSound = new CSound(m_xpcGtlDoc->m_xpcLastFocusView, pSoundFile, SOUND_WAVE | SOUND_QUEUE | SOUND_ASYNCH | SOUND_AUTODELETE);
        pSound->SetDrivePath(lpMetaGameStruct->m_chCDPath);
        pSound->Play();
        CSound::waitWaveSounds();
    }

    // if there is no DC, then get one
    if (m_cBgbMgr.m_xpDc == NULL) {
        m_cBgbMgr.ReInitDc();
    }

    assert(m_cBgbMgr.m_xpBsuSet != NULL);
    if (pCurPlayer->IfRelocatable()) {
        m_cBgbMgr.m_xpBsuSet->PrepareDc(m_cBgbMgr.m_xpDc, TRUE);
    } else {
        m_cBgbMgr.m_xpBsuSet->PrepareDc(m_cBgbMgr.m_xpDc, FALSE);
    }

    // put back old filename
    strcpy(pCurPlayer->m_szFileName, szOldFileName);
    assert(FileExists(pCurPlayer->m_szFileName));

    if (bDoTransport) {
        DoTransport(m_xpCurXodj, nId);
    } else {

        // erase old image
        ((CSprite *)pCurPlayer->m_pObject)->EraseSprite(m_cBgbMgr.m_xpDc);
        ((CSprite *)pCurPlayer->m_pObject)->LoadCels(m_cBgbMgr.m_xpDc, pCurPlayer->m_szFileName, pCurPlayer->m_nCels);

        m_cBgbMgr.SetPosition(pCurPlayer, ptNew);

        // Re-center new current player
        m_xpcGtlDoc->m_xpcLastFocusView->m_cViewBsuSet.EdgeToCenter(pCurPlayer->m_crPosition, TRUE);

        // pause at this location for 1 sec.
        Sleep(1000);
    }

	xpGtlApp->DoWaitCursor(-1);

    return(nId);
}


int CGtlData::FindNodeId(char *pszLabel)
{
    CNode *pNode;
    int i, iNodeID;

    assert(pszLabel != NULL);

    iNodeID = -1;
    for (i = 0 ; i < m_iNodes ; ++i) {
        pNode = m_lpNodes + i;

        if (!pNode->m_bDeleted) {

            if (stricmp(pNode->m_szLabel, pszLabel) == 0) {
                iNodeID = i;
                break;
            }
        }
    }

    return(iNodeID);
}

VOID CGtlData::LoadCharDirection(CBgbObject *pBgbSprite, CPoint ptOld, CPoint ptNew)
{
#if 1
    CHAR chOldChar, chNewChar = '1';
    INT x, y;

    // can't access null pointers
    assert(pBgbSprite != NULL);

    if (pBgbSprite->m_bInit) {

        // make sure this really is a sprite
        assert(pBgbSprite->m_iBgbType == BGBT_SPRITE);

        // only take action if the character is actually going to move
        //
        if (ptNew != ptOld) {

            x = ptNew.x - ptOld.x;
            y = ptNew.y - ptOld.y;

            if (y >= 0) {

                if (x > 0) {

                    if (abs(x) * 2 < abs(y)) {
                        chNewChar = '4';
                    } else if (abs(y) * 2 >= abs(x)) {
                        chNewChar = '3';
                    } else {
                        chNewChar = '2';
                    }

                } else {

                    if (abs(x) * 2 < abs(y)) {
                        chNewChar = '4';
                    } else if (abs(y) * 2 >= abs(x)) {
                        chNewChar = '5';
                    } else {
                        chNewChar = '6';
                    }
                }

            } else if (y < 0) {

                if (x > 0) {

                    if (abs(x) * 2 < abs(y)) {
                        chNewChar = '0';
                    } else if (abs(y) * 2 >= abs(x)) {
                        chNewChar = '1';
                    } else {
                        chNewChar = '2';
                    }

                } else {

                    if (abs(x) * 2 < abs(y)) {
                        chNewChar = '0';
                    } else if (abs(y) * 2 >= abs(x)) {
                        chNewChar = '7';
                    } else {
                        chNewChar = '6';
                    }
                }
            }

            // if this is a new direction
            //
            chOldChar = pBgbSprite->m_szFileName[7];
            if (chNewChar != chOldChar) {

                // make sure this sprite is loaded
                m_cBgbMgr.CacheLoadObject(pBgbSprite);
                
                if (m_cBgbMgr.m_xpDc == NULL) {
                    m_cBgbMgr.ReInitDc();
                }

                // load the new cel strip
                //
                pBgbSprite->m_szFileName[7] = chNewChar;

                // load new pictures
                ((CSprite *)pBgbSprite->m_pObject)->LoadCels(m_cBgbMgr.m_xpDc, pBgbSprite->m_szFileName, pBgbSprite->m_nCels);
            }
        }
    }


#else
    CVector vector, vOrigin(0, -1);
    DOUBLE angle;
    CHAR chOldChar, chNewChar;

    // can't access null pointers
    assert(pBgbSprite != NULL);

    if (pBgbSprite->m_bInit) {

        // make sure this really is a sprite
        assert(pBgbSprite->m_iBgbType == BGBT_SPRITE);

        // only take action if the character is actually going to move
        //
        if (ptNew != ptOld) {

            vector.x = ptNew.x - ptOld.x;
            vector.y = ptNew.y - ptOld.y;

            // determine the angle of Hodj's vector
            angle = vector.RealAngle(vOrigin);

            //
            // change position bitmap relative to movement vector
            //

            // if moving UP
            //
            if ((angle >= 0 && angle < Deg2Rad(40)) || (angle >= Deg2Rad(340))) {
                chNewChar = '0';

            // if moving UP and RIGHT
            //
            } else if ((angle >= Deg2Rad(40)) && (angle < Deg2Rad(75))) {
                chNewChar = '1';

            // if moving RIGHT
            //
            } else if ((angle >= Deg2Rad(75)) && (angle < Deg2Rad(105))) {
                chNewChar = '2';

            // if moving DOWN and RIGHT
            //
            } else if ((angle >= Deg2Rad(105)) && (angle < Deg2Rad(165))) {
                chNewChar = '3';

            // if moving DOWN
            //
            } else if ((angle >= Deg2Rad(165)) && (angle < Deg2Rad(220))) {

                chNewChar = '4';

            // if moving DOWN and LEFT
            //
            } else if ((angle >= Deg2Rad(220)) && (angle < Deg2Rad(255))) {
                chNewChar = '5';

            // if moving LEFT
            //
            } else if ((angle >= Deg2Rad(255)) && (angle < Deg2Rad(285))) {
                chNewChar = '6';

            // if moving UP and LEFT
            //
            } else if ((angle >= Deg2Rad(285)) && (angle < Deg2Rad(340))) {
                chNewChar = '7';

            } else {

                // ERROR: angle MUST be between 0 and 360
                assert(0);
            }


            // if this is a new direction
            //
            chOldChar = pBgbSprite->m_szFileName[7];
            if (chNewChar != chOldChar) {

                // make sure this sprite is loaded
                m_cBgbMgr.CacheLoadObject(pBgbSprite);
                
                if (m_cBgbMgr.m_xpDc == NULL) {
                    m_cBgbMgr.ReInitDc();
                }

                // load the new cel strip
                //
                pBgbSprite->m_szFileName[7] = chNewChar;

                // load new pictures
                ((CSprite *)pBgbSprite->m_pObject)->LoadCels(m_cBgbMgr.m_xpDc, pBgbSprite->m_szFileName, pBgbSprite->m_nCels);
            }
        }
    }
#endif
}


struct DIST {
    int m_iDistance ;   // shortest distance so far
    int m_iWeight ;     // weight of this node
    int m_iLength ;     // # nodes in path
    int m_iFrom ;       // "from" node for shortest distance
    int m_iCount ;      // # paths with shortest distance

};

//* CGtlData::EstimatePathDistance -- between two nodes
int CGtlData::EstimatePathDistance(CNode FAR * lpNode1, CNode FAR * lpNode2)
// lpNode1, lpNode2 -- nodes for which path distance is estimated
// returns: estimated path length
{
    CPoint cStartPoint, cTargetPoint ;

    long lXDiff, lYDiff, lDistance = MAXPOSINT;

    if (lpNode1 && lpNode2) {
        cStartPoint = NodeToPoint(lpNode1, NULL) ;
        cTargetPoint = NodeToPoint(lpNode2, NULL) ;

        lXDiff = ( cTargetPoint.x - cStartPoint.x ) / 100;
        lYDiff = ( cTargetPoint.y - cStartPoint.y ) / 100;
        lDistance = (lXDiff * lXDiff + lYDiff * lYDiff);
    }

    if ( lDistance > MAXPOSINT )
    	lDistance = MAXPOSINT;
    	
    return((int)lDistance);

}

//* CGtlData::FindShortestPath -- between two nodes
LPINT CGtlData::FindShortestPath(CNode FAR * lpNode1,
                                CNode FAR * lpNode2)
// lpNode1, lpNode2 -- nodes for which path is to be found
// returns: pointer to integer array, first element is length,
//      remaining elements are path nodes; delete ptr when done
{
    JXENTER(CGtlData::FindShortestPath) ;
    int iError = 0 ;            // error code
    int iStartNode = lpNode1 - m_lpNodes ;
    int iTargetNode = lpNode2 - m_lpNodes ;
    int iDistance, iNewDistance, iOldDistance ; 
    int i, iLength;
                        // current distance being processed
    BOOL bChange, bLastChange ; // loop change flags
    int iK, iL ; //, iT ;       // loop variables
    int iLink ; // , iTest ;    // current link number
    CNode FAR * lpNode ;        // current node
    CNode FAR * lpTestNode ;    // current node
    LPINT lpiPath = NULL ;      // pointer to result path array
    BOOL        bClipBounds = TRUE; 
    CPoint      cStartPoint, cTargetPoint, cThisPoint;
    CRect       cBoundingRect;
    int iDistanceDX, iDistanceDY;
    struct _timeb stStartTime, stEndTime ;   // current time
    long lTimeDiff ;            // time difference
#ifdef _DEBUG
    char szMsg[150] ;
#endif

    struct DIST FAR * lpDist = new FAR struct DIST[m_iNodes] ;

    cStartPoint = NodeToPoint(lpNode1, NULL) ;
    cTargetPoint = NodeToPoint(lpNode2, NULL) ;

#ifdef _DEBUG
    // if debug5 is 1, then FindShortestPath dumps debugging information
    if (CBdbgMgr::GetPointer()->m_iDebugValues[5])
    {
        sprintf(szMsg, "Finding shortest path from node #%d "
                "(%d,%d) to node #%d (%d,%d).\n",
                iStartNode, iTargetNode,
                cStartPoint.x, cStartPoint.y,
                cTargetPoint.x, cTargetPoint.y) ;
        JXOutputDebugString(szMsg) ;
    }
#endif

    _ftime(&stStartTime) ;                // save current time

    iDistanceDX = abs(cStartPoint.x - cTargetPoint.x);
    iDistanceDY = abs(cStartPoint.y - cTargetPoint.y);
    iDistance = ((iDistanceDX > iDistanceDY)
                                ? iDistanceDX : iDistanceDY);
    cBoundingRect.left = cStartPoint.x;
    cBoundingRect.top = cStartPoint.y;
    cBoundingRect.right = cTargetPoint.x;
    cBoundingRect.bottom = cTargetPoint.y;
    CGenUtil::NormalizeCRect(cBoundingRect);
    cBoundingRect.InflateRect(iDistance,iDistance);

retry:
    for (iK = 0 ; iK < m_iNodes ; ++iK) {
        lpDist[iK].m_iDistance = lpDist[iK].m_iLength = MAXPOSINT ;
        lpDist[iK].m_iWeight = (m_lpNodes + iK)->m_iWeight ;
    }

    lpDist[iStartNode].m_iFrom = iStartNode ;
    lpDist[iStartNode].m_iDistance = 0 ;
    lpDist[iStartNode].m_iLength = 0 ;

    for (bChange = FALSE, bLastChange = TRUE ; bLastChange ;
                bLastChange = bChange, bChange = FALSE, ++iDistance)

        for (iK = 0 ; iK < m_iNodes ; ++iK) {
            if (iK == iTargetNode)
                continue;   

            iDistance = lpDist[iK].m_iDistance;

            if (iDistance < MAXPOSINT) {

                if ((lpDist[iTargetNode].m_iDistance < MAXPOSINT) &&
                        (iDistance > lpDist[iTargetNode].m_iDistance))
                    continue;

                lpNode = m_lpNodes + iK ;

                if (lpNode->m_iWeight >= 99)
                    continue;

                if (bClipBounds) {

                    cThisPoint = NodeToPoint(lpNode, NULL) ;
                    if (!cBoundingRect.PtInRect(cThisPoint))
                        continue;
                }
                for (iL = 0 ; iL < lpNode->m_iNumLinks ; ++iL) {

                    iLink = lpNode->m_iLinks[iL] ;
                    lpTestNode = m_lpNodes + iLink ;
                    if (bClipBounds) {
                        cThisPoint = NodeToPoint(lpTestNode, NULL) ;
                        if (!cBoundingRect.PtInRect(cThisPoint))
                            continue;
                    }
                    iOldDistance = lpDist[iLink].m_iDistance;
                    iNewDistance = iDistance + lpDist[iLink].m_iWeight;
                    if ( iOldDistance >= iNewDistance )
                    {
                        if (iNewDistance == MAXPOSINT)
                            ;   // skip

                        else if (iOldDistance > iNewDistance)
                                    ///// || lpDist[iLink].m_iWeight == 0)
                        {
                            bChange = TRUE ;
                            lpDist[iLink].m_iCount = 1 ;
                            lpDist[iLink].m_iDistance = iNewDistance ;
                            lpDist[iLink].m_iFrom = iK ;
                            lpDist[iLink].m_iLength =
                                        lpDist[iK].m_iLength + 1 ;
                            if ((iLink == iTargetNode) &&
                                    (lpTestNode->m_iNumLinks == 1))
                                goto done;
                            iK += 1;
                            break;
                        }

                        else if (lpDist[iLink].m_iWeight > 0 &&
                                !m_cGenUtil.RandomInteger(0,
                                            lpDist[iLink].m_iCount++))
                        {
                            lpDist[iLink].m_iFrom = iK ;
                            lpDist[iLink].m_iLength =
                                        lpDist[iK].m_iLength + 1 ;
                        }
                    }
                }
            }
        }

done:
    if ((lpDist[iStartNode].m_iDistance >= MAXPOSINT) ||
        (lpDist[iTargetNode].m_iDistance >= MAXPOSINT)) {
         if (bClipBounds){
             bClipBounds = FALSE;
             goto retry;
         }
         goto cleanup ;
    }

    iK = iTargetNode;
    lpDist[iTargetNode].m_iLength = 1;
    while(iK != iStartNode) {
        if (lpDist[iK].m_iDistance >= MAXPOSINT)
            goto cleanup;
        lpDist[iTargetNode].m_iLength += 1;
        iK = lpDist[iK].m_iFrom ;
    }

     // lpiPath[0] contains the number of nodes in the shortest
     // path, and lpiPath[1] contains the total weight.
     // The remaining array elements contain the path nodes.

    iLength = lpDist[iTargetNode].m_iLength ;
    lpiPath = new int [iLength + 2] ;
    lpiPath[iLength + 1] = iTargetNode ;
    for (iK = iLength ; iK > 1 ; --iK)
        lpiPath[iK] = lpDist[lpiPath[iK+1]].m_iFrom ;

    lpiPath[0] = 0;
    lpiPath[1] = 0 ;
    for (i = 2; i < iLength + 2; i++) {
        lpDist[lpiPath[i]].m_iLength = lpiPath[0];
        lpDist[lpiPath[i]].m_iDistance = lpiPath[1];
    
        lpiPath[0] += 1;
        lpiPath[1] += lpDist[lpiPath[i]].m_iWeight;
    }

    lpiPath[0] += 1;

    _ftime(&stEndTime) ;                // get ending time
    lTimeDiff = 1000L * (stEndTime.time - stStartTime.time)
            + stEndTime.millitm - stEndTime.millitm ;

#ifdef _DEBUG
    // if debug5 is 1, then FindShortestPath dumps debugging information
    if (CBdbgMgr::GetPointer()->m_iDebugValues[5]) {
        _snprintf(szMsg, sizeof(szMsg),
                "   Shortest path found in %ld milliseconds: "
                "number nodes = %d, "
                "weighted distance = %d:\n        ",
                lTimeDiff,
                iLength, lpDist[iTargetNode].m_iDistance) ;
        JXOutputDebugString(szMsg) ;

        for (iK = 3 ; iK <= lpiPath[0] ; ++iK)
        {
            if (iK == 3)
                szMsg[0] = 0 ;

            else if (iK % 12)
                strcpy(szMsg, ", ") ;

            else
                strcpy(szMsg, ",\n            ") ;

            itoa(lpiPath[iK], szMsg + strlen(szMsg), 10) ;
            JXOutputDebugString(szMsg) ;
        }
        JXOutputDebugString("\n") ;

    }

/*
    {               // ****** debugging
        struct DIST lpxDist[600] ;
        int lpxPath[600], iK = 0 ;
        for (iK = 0 ; iK < m_iNodes ; ++iK)
            lpxDist[iK] = lpDist[iK] ;

        for (iK = 0 ; iK < lpiPath[0] + 1 ; ++iK)
            lpxPath[iK] = lpiPath[iK] ;

        iK = 0 ;
    }
*/
#endif  // _DEBUG

cleanup:

    if (lpDist) {
        delete [] lpDist ;
        lpDist = NULL ;
    }

    if (iError && lpiPath) {
        delete [] lpiPath ;
        lpiPath = NULL ;
    }

    JXELEAVE(CGtlData::FindShortestPath) ;
    RETURN(lpiPath) ;
}

//* CGtlData::PositionCharacters -- set positions for Hodj and Podj
BOOL CGtlData::PositionCharacters(void)
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::PositionCharacters) ;
    int iError = 0 ;            // error code
    CXodj * xpXodj ;
    CXodj * xpXodj1 = NULL, * xpXodj2 = NULL ;  // character objects
    BOOL bSame, bSamePrev ;

    if ((xpXodj1 = m_xpXodjChain) != NULL)
        xpXodj2 = xpXodj1->m_xpXodjNext;

    bSamePrev = (xpXodj1 && xpXodj2 && xpXodj1->m_iCharNode == xpXodj2->m_iCharNode) ;

    PositionACharacter(xpXodj1, -1 * bSamePrev) ;
    PositionACharacter(xpXodj2, 1 * bSamePrev) ;

    bSame = (xpXodj1 && xpXodj2
                && xpXodj1->m_iCharNode == xpXodj2->m_iCharNode) ;

    if (bSame != bSamePrev) {
        PositionACharacter(xpXodj1, -1 * bSame) ;
        PositionACharacter(xpXodj2, 1 * bSame) ;
    }

    // if chars are/were in same position, we have to repaint both
    if (bSame || m_bSamePlace)
        for (xpXodj = m_xpXodjChain ; xpXodj ; xpXodj = xpXodj->m_xpXodjNext)
            if (xpXodj->m_lpcCharSprite)
                m_cBgbMgr.PaintBitmapObject(xpXodj->m_lpcCharSprite) ;
//              CallUpdate(xpXodj->m_lpcCharSprite) ;
//                              // invalidate char's rectangle
    m_bSamePlace = bSame ;

// cleanup:

    JXELEAVE(CGtlData::PositionCharacters) ;
    RETURN(iError != 0) ;
}

//* CGtlData::PositionACharacter -- find positions for Hodj and Podj
BOOL CGtlData::PositionACharacter(CXodj * xpXodj, int iShift)
// xpXodj -- character object
// iShift -- amount to shift left or right
// returns: TRUE if error, FALSE otherwise
{
    JXENTER(CGtlData::PositionACharacter) ;
    int iError = 0 ;            // error code
    CNode FAR * lpNode ;
    BOOL bNodeFound = FALSE ;
    int iN ;            // loop variable 
    CSize * lpSize ;    // size of character
    CRPoint crPosition ;        // position of node

    if (xpXodj) {
        if (xpXodj->m_iCharNode >= 0 && xpXodj->m_iCharNode < m_iNodes && !(lpNode = m_lpNodes + xpXodj->m_iCharNode)->m_bDeleted)
            bNodeFound = TRUE ;

        for (iN = 0 ; !bNodeFound && iN < m_iNodes ; iN++)
            if ( !(lpNode = m_lpNodes + iN)->m_bDeleted && !_fstricmp(lpNode->m_szLabel, "castle"))
                xpXodj->m_iCharNode = iN, bNodeFound = TRUE ;

        for (iN = 0 ; !bNodeFound && iN < m_iNodes ; iN++)
            if ( !(lpNode = m_lpNodes + iN)->m_bDeleted)
                xpXodj->m_iCharNode = iN, bNodeFound = TRUE ;

        if (bNodeFound && xpXodj->m_lpcCharSprite) {
            lpSize = &xpXodj->m_lpcCharSprite->m_cSize ;
            crPosition = NodeToPoint(lpNode, lpSize) ;
            crPosition.x += iShift * (lpSize->cx / 2 + 1) ;
            m_cBgbMgr.SetPosition(xpXodj->m_lpcCharSprite,crPosition);
        }
    }

// cleanup:

    JXELEAVE(CGtlData::PositionACharacter) ;
    RETURN(iError != 0) ;
}


//* CGtlData::LocationToNode -- find node for game location
CNode FAR * CGtlData::LocationToNode(int iLocationCode)
// iLocationCode -- MG_LOC_xxxx -- location code
// returns: pointer to node, or NULL if not found
{
    JXENTER(CGtlData::LocationToNode) ;
    int iError = 0 ;            // error code
    int iN ;            // loop variable
    CNode FAR *lpNode = NULL ; // return value

    if (iLocationCode >= MG_LOC_BASE && iLocationCode <= MG_LOC_MAX) {

		if ( iLocationCode == MG_LOC_HODJ ) {
            lpNode = m_lpNodes + lpMetaGameStruct->m_cHodj.m_iNode;   // compute node pointer

        } else {

	        // search for node in this location
	        for (iN = 0 ; iN < m_iNodes && (m_lpNodes + iN)->m_iLocationCode != iLocationCode ; ++iN)
	            ;   // null loop body
		
            if (iN < m_iNodes)          // if it was found
                lpNode = m_lpNodes + iN ;   // compute node pointer
        }
    }

//cleanup:

    JXELEAVE(CGtlData::LocationToNode) ;
    RETURN(lpNode) ;
}


// HACK-O-RAMA
VOID CGtlData::SetFurlongs(CXodj *pXodj)
{
    assert(pXodj != NULL);

    if (pXodj->m_bHodj) {
        lpMetaGameStruct->m_cHodj.m_iFurlongs = pXodj->m_iFurlongs;
    } else {
        lpMetaGameStruct->m_cPodj.m_iFurlongs = pXodj->m_iFurlongs;
    }
}
