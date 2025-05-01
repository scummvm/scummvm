/*****************************************************************
*
*  Copyright (c) 1994 by Boffo Games, All Rights Reserved
*
*
*  restgame.cpp  -  Restore routines for Hodj 'n' Podj
*
*  HISTORY
*
*       1.00        08/19/94    BCW     Initial Design
*
*  MODULE DESCRIPTION:
*
*       [Describe the purpose of this module]
*
*  LOCALS:
*
*       MyFunction()            Description of MyFunction
*
*  GLOBALS:
*
*       MyFunction()            Description of MyFunction
*
*  RELEVANT DOCUMENTATION:
*
*       [Specifications, documents, test plans, etc./]
*
*  FILES USED:
*
*       [Discuss files created, or used]
*
****************************************************************/
#include <afxwin.h>
#include <afxext.h>
#include <stdio.h>
#include <assert.h>
#include <stdinc.h>
#include <misc.h>
#include <errors.h>

#include "resource.h"
#include "bfc.h"
#include "item.h"
#include "invent.h"
#include "restgame.h"
#include "restdlg.h"

STATIC CHAR *gpszSaveGameFileName;

STATIC CHAR szDescriptions[MAX_SAVEGAMES][40];
STATIC CHAR *pszDesc[MAX_SAVEGAMES];

struct stInventoryInfo {
    char m_szInventory[40];
    char m_szTradingPost[40];
    char m_szBlackMarket[40];
    char m_szGenStore[40];
} stPlayerBuf[MAX_PLAYERS];



// Local prototypes
//
ERROR_CODE GetSaveGameDescriptions(VOID);
ERROR_CODE RestoreSlot(INT, CBfcMgr *);
ERROR_CODE ReadSavedGame(FILE *, INT, SAVEGAME_INFO *);
BOOL       ValidateSGInfo(SAVEGAME_INFO *);
ERROR_CODE ConvertFromSGI(CBfcMgr *, SAVEGAME_INFO *);

VOID       WipeBFC(CBfcMgr *);


/*****************************************************************************
*
*  RestoreGame      - Loads a Restore-Game dialog box
*
*  DESCRIPTION:     User can select which game they want to restore.
*
*  SAMPLE USAGE:
*  errCode = RestoreGame(psFileName, pBfcMgr, pWnd, pPalette);
*  const CHAR *pszFileName;                 File to restore from
*  CBfMgr *pBfcMgr;                         SaveGame info
*  CWnd *pWnd;                              Parent window to RestoreGame dialog box
*  CPalette *pPalette;                      Palette to use for dialog Bmp
*
*  RETURNS:  ERROR_CODE = Error return code.
*
*****************************************************************************/
BOOL CALLBACK RestoreGame(const CHAR *pszFileName, CBfcMgr *pBfcMgr, CWnd *pWnd, CPalette *pPalette, ERROR_CODE *pErrCode)
{
    INT iGameNum;
    BOOL bRestored;

    // validate input
    assert(pszFileName != NULL);
    assert(pBfcMgr != NULL);
    assert(pWnd != NULL);
    assert(pPalette != NULL);
    assert(pErrCode != NULL);

    // assume no error
    *pErrCode = ERR_NONE;

    // assume Restore will work
    bRestored = TRUE;

    // keep track of the .SAV file name
    gpszSaveGameFileName = (CHAR *)pszFileName;

    if (FileExists(gpszSaveGameFileName)) {

        *pErrCode = GetSaveGameDescriptions();

        if (*pErrCode == ERR_NONE) {

            //
            // open the Restore Game dialog box
            //
            CRestoreDlg dlgRestore(pszDesc, pWnd, pPalette);

            if ((iGameNum = dlgRestore.DoModal()) != -1) {

                *pErrCode = RestoreSlot(iGameNum, pBfcMgr);
            } else {
                bRestored = FALSE;
            }
        }
    }

    if (*pErrCode != ERR_NONE) {
        bRestored = FALSE;
    }

    return(bRestored);
}


ERROR_CODE GetSaveGameDescriptions(VOID)
{
    SAVEGAME_INFO *pSaveGameInfo;
    FILE *pFile;
    INT i;
    ERROR_CODE errCode;

    // assume no error
    errCode = ERR_NONE;

    // can't restore a game unless the .SAV file exists
    //
    if (FileExists(gpszSaveGameFileName)) {

        if ((pSaveGameInfo = (SAVEGAME_INFO *)malloc(sizeof(SAVEGAME_INFO))) != NULL) {

            // open the .SAV file
            //
            if ((pFile = fopen(gpszSaveGameFileName, "rb")) != NULL) {

                for (i = 0; i < MAX_SAVEGAMES; i++) {

                    if ((errCode = ReadSavedGame(pFile, i, pSaveGameInfo)) == ERR_NONE) {

                        assert(strlen(pSaveGameInfo->m_szSaveGameDescription) < 40);
                        strcpy(pszDesc[i] = szDescriptions[i], pSaveGameInfo->m_szSaveGameDescription);

                        if (!pSaveGameInfo->m_bUsed) {
                            szDescriptions[i][0] = '\0';
                            pszDesc[i] = NULL;
                        }

                    } else {
                        break;
                    }
                }
                fclose(pFile);
            } else {
                errCode = ERR_FOPEN;
            }

            free(pSaveGameInfo);
        } else {
            errCode = ERR_MEMORY;
        }

    } else {
        ErrorLog("ERROR.LOG", "%s not found.  Can't restore game.", gpszSaveGameFileName);
    }

    return(errCode);
}


/*****************************************************************************
*
*  RestoreSlot      - one line discription
*
*  DESCRIPTION:
*
*
*  SAMPLE USAGE:
*  errCode = RestoreSlot(iGameNum, pBfcMgr);
*  INT iGameNum;                            Game slot to retore game from
*  CBfcMgr *pBfcMgr;                        destination object of restored info
*
*  RETURNS:  ERROR_CODE = Error return code.
*
*****************************************************************************/
ERROR_CODE RestoreSlot(INT iGameNum, CBfcMgr *pBfcMgr)
{
    SAVEGAME_INFO *pSaveGameInfo;
    FILE *pFile;
    ERROR_CODE errCode;

    // assume no error
    errCode = ERR_NONE;

    assert(iGameNum >= 0 && iGameNum < MAX_SAVEGAMES);
    assert(pBfcMgr != NULL);

    assert(FileExists(gpszSaveGameFileName));

    if ((pSaveGameInfo = (SAVEGAME_INFO *)malloc(sizeof(SAVEGAME_INFO))) != NULL) {

        if ((pFile = fopen(gpszSaveGameFileName, "rb")) != NULL) {

#ifdef _DEBUG
            memset(pSaveGameInfo, 0, sizeof(SAVEGAME_INFO));
#endif

            if ((errCode = ReadSavedGame(pFile, iGameNum, pSaveGameInfo)) == ERR_NONE) {

                if (ValidateSGInfo(pSaveGameInfo)) {

                    errCode = ConvertFromSGI(pBfcMgr, pSaveGameInfo);
                } else {
                    errCode = ERR_FTYPE;
                }
            }

            fclose(pFile);
        } else {
            errCode = ERR_FOPEN;
        }

        free(pSaveGameInfo);
    } else {
        errCode = ERR_MEMORY;
    }

#ifdef _DEBUG
    if (errCode != ERR_NONE) {
        ErrorLog("ERROR.LOG", "errCode = %d in %s at line %d", errCode, __FILE__, __LINE__);
    }
#endif

    return(errCode);
}


ERROR_CODE ReadSavedGame(FILE *pFile, INT iGameNum, SAVEGAME_INFO *pSaveGameInfo)
{
    ERROR_CODE errCode;

    // asume no error
    errCode = ERR_NONE;

    // validate explicit input
    assert(pFile != NULL);
    assert(iGameNum >= 0 && iGameNum < MAX_SAVEGAMES);
    assert(pSaveGameInfo != NULL);

    // seek to correct save game number (slot)
    //
    if (fseek(pFile, iGameNum * sizeof(SAVEGAME_INFO), SEEK_SET) == 0) {

        // write the save game to that slot
        //
        if (fread(pSaveGameInfo, sizeof(SAVEGAME_INFO), 1, pFile) == 1) {

            Decrypt(pSaveGameInfo, sizeof(SAVEGAME_INFO));

            // if this fails, then we are trying to restore an old saved game file
            // and the SAVEGAME_INFO structure has changed. (i.e. the .SAV file is
            // incompatable with this version of the game)
            //
            assert(pSaveGameInfo->m_nFixedRecordSize == sizeof(SAVEGAME_INFO));

        } else {
            errCode = ERR_FREAD;
        }

    } else {
        errCode = ERR_FSEEK;
    }

    return(errCode);
}


BOOL ValidateSGInfo(SAVEGAME_INFO *pSaveGameInfo)
{
    // can't acess a null pointer
    assert(pSaveGameInfo != NULL);

    // this must be a valid saved game
    assert(pSaveGameInfo->m_bUsed);

    if (pSaveGameInfo->m_lBoffoGameID != BOFFO_GAME_ID) {
        return(FALSE);
    }

    if (pSaveGameInfo->m_nFixedRecordSize != sizeof(SAVEGAME_INFO))
        return(FALSE);

    /*if (pSaveGameInfo->m_stPlayerInfo[0].m_lCrowns < 0)
        return(FALSE);

    if (pSaveGameInfo->m_stPlayerInfo[1].m_lCrowns < 0)
        return(FALSE);*/

    switch (pSaveGameInfo->m_iGameTime) {

        case SHORT_GAME:
        case MEDIUM_GAME:
        case LONG_GAME:
            break;

        // not a valid game type
        default:
            return(FALSE);
    }

    if (pSaveGameInfo->m_iMishMoshLoc < MG_LOC_BASE || pSaveGameInfo->m_iMishMoshLoc > MG_LOC_MAX) {
        return(FALSE);
    }

    return(TRUE);
}


ERROR_CODE ConvertFromSGI(CBfcMgr *pBfcMgr, SAVEGAME_INFO *pSaveGameInfo)
{
    CHodjPodj *pPlayer;
    CInventory *pInv;
    CItem *pItem;
    PLAYER_INFO *pSavePlayer;
    INVENTORY *pSaveInv;
    INT i, j, k;
    ERROR_CODE errCode;

    // assume no error
    errCode = ERR_NONE;

    assert(pBfcMgr != NULL);
    assert(pSaveGameInfo != NULL);

    // clean out any info currently in the CBfcMgr
    WipeBFC(pBfcMgr);

    pBfcMgr->m_bInventories = TRUE;
    pBfcMgr->m_bRestart = TRUE;
    pBfcMgr->m_bGameOver = FALSE;

    pBfcMgr->m_bChanged = FALSE;

    memcpy(&pBfcMgr->m_bTraps, &pSaveGameInfo->m_bTraps, sizeof(BOOL) * 240);

    pBfcMgr->m_iFunctionCode = 0;

    pBfcMgr->m_stGameStruct.bSoundEffectsEnabled = pSaveGameInfo->m_bSoundEffectsEnabled;
    pBfcMgr->m_stGameStruct.bMusicEnabled = pSaveGameInfo->m_bMusicEnabled;
    pBfcMgr->m_stGameStruct.bPlayingMetagame = TRUE;
    pBfcMgr->m_bScrolling = pSaveGameInfo->m_bScrolling;

    // need to allocate new Mish/Mosh items
    if (pSaveGameInfo->m_bNewMishMosh) {

        pBfcMgr->m_pMishItem = new CItem(MG_OBJ_MISH);
        pBfcMgr->m_pMoshItem = new CItem(MG_OBJ_MOSH);
    }

    pBfcMgr->m_bRestoredGame = TRUE;

    pBfcMgr->m_iGameTime = pSaveGameInfo->m_iGameTime;
    pBfcMgr->m_iMishMoshLoc = pSaveGameInfo->m_iMishMoshLoc;

    // Hodj/Podj info
    //
    pPlayer = &pBfcMgr->m_cHodj;
    for (i = 0; i < MAX_PLAYERS; i++) {

        if (i == 1) {
            pPlayer = &pBfcMgr->m_cPodj;
        }
        pSavePlayer = &pSaveGameInfo->m_stPlayerInfo[i];

        pPlayer->m_bMoving = pSavePlayer->m_bMoving;
        pPlayer->m_bComputer = pSavePlayer->m_bComputer;
        pPlayer->m_bHaveMishMosh = pSavePlayer->m_bHaveMishMosh;

        pPlayer->m_iSectorCode = pSavePlayer->m_iSectorCode;
        pPlayer->m_iNode = pSavePlayer->m_iNode;
        pPlayer->m_iSkillLevel = pSavePlayer->m_iSkillLevel;

        pPlayer->m_iWinInfoWon = pSavePlayer->m_iWinInfoWon;
        pPlayer->m_iWinInfoNeed = pSavePlayer->m_iWinInfoNeed;

        memcpy(&pPlayer->m_iWinInfoTable, &pSavePlayer->m_iWinInfoTable, sizeof(INT) * MAX_GAME_TABLE);

        pPlayer->m_iSecondaryInfoWon = pSavePlayer->m_iSecondaryInfoWon;
        pPlayer->m_iSecondaryInfoNeed = pSavePlayer->m_iSecondaryInfoNeed;

        memcpy(&pPlayer->m_iSecondaryInfoTable, &pSavePlayer->m_iSecondaryInfoTable, sizeof(INT) * MAX_GAME_TABLE);

        pPlayer->m_iRequiredObjectsCount = pSavePlayer->m_iRequiredObjectsCount;

        memcpy(&pPlayer->m_iRequiredObjectsTable, &pSavePlayer->m_iRequiredObjectsTable, sizeof(INT) * MAX_GAME_TABLE);

        pPlayer->m_iRequiredMoney = pSavePlayer->m_iRequiredMoney;

        memcpy(&pPlayer->m_iSecondaryLoc, &pSavePlayer->m_iSecondaryLoc, sizeof(INT) * MAX_GAME_TABLE);

        memcpy(&pPlayer->m_iGameHistory, &pSavePlayer->m_iGameHistory, sizeof(INT) * 20);

        pPlayer->m_iTargetLocation = pSavePlayer->m_iTargetLocation;
        pPlayer->m_iSpecialTravelCode = pSavePlayer->m_iSpecialTravelCode;
        pPlayer->m_iNumberBoatTries = pSavePlayer->m_iNumberBoatTries;
        pPlayer->m_iFurlongs = pSavePlayer->m_iFurlongs;
        pPlayer->m_nTurns = pSavePlayer->m_nTurns;

        strcpy(stPlayerBuf[i].m_szTradingPost, pSavePlayer->m_stTradingPost.m_szTitle);
        strcpy(stPlayerBuf[i].m_szBlackMarket, pSavePlayer->m_stBlackMarket.m_szTitle);
        strcpy(stPlayerBuf[i].m_szGenStore, pSavePlayer->m_stGenStore.m_szTitle);
        strcpy(stPlayerBuf[i].m_szInventory, pSavePlayer->m_stInventory.m_szTitle);

        if ((pPlayer->m_pTradingPost = new CInventory(stPlayerBuf[i].m_szTradingPost)) != NULL) {

            if ((pPlayer->m_pBlackMarket = new CInventory(stPlayerBuf[i].m_szBlackMarket)) != NULL) {

                if ((pPlayer->m_pGenStore = new CInventory(stPlayerBuf[i].m_szGenStore)) != NULL) {

                    if ((pPlayer->m_pInventory = new CInventory(stPlayerBuf[i].m_szInventory)) != NULL) {

                        for (j = 0; j < 4; j++) {

                            switch(j) {

                                case 0:
                                    pInv = pPlayer->m_pInventory;
                                    pSaveInv = &pSavePlayer->m_stInventory;
                                    break;

                                case 1:
                                    pInv = pPlayer->m_pGenStore;
                                    pSaveInv = &pSavePlayer->m_stGenStore;
                                    break;

                                case 2:
                                    pInv = pPlayer->m_pBlackMarket;
                                    pSaveInv = &pSavePlayer->m_stBlackMarket;
                                    break;

                                default:
                                    assert(j == 3);
                                    pInv = pPlayer->m_pTradingPost;
                                    pSaveInv = &pSavePlayer->m_stTradingPost;
                                    break;
                            }

                            // add specified items to the inventory
                            //
                            for (k = 0; k < pSaveInv->m_nItems; k++) {

                                // Mish/Mosh can't be in the inventory if we
                                // were supposed to allocate them
                                //assert(pSaveGameInfo->m_bNewMishMosh || ((pSaveInv->m_aItemList[k] != MG_OBJ_MOSH) && (pSaveInv->m_aItemList[k] != MG_OBJ_MOSH)));

                                pInv->AddItem( pSaveInv->m_aItemList[k], 1);
                            }

                            // make sure we added the correct number of items
                            assert(pInv->ItemCount() == pSaveInv->m_nItems);
                        }

                        assert(pSavePlayer->m_lCrowns >= 0);
                        pPlayer->m_pInventory->FindItem(MG_OBJ_CROWN)->SetQuantity(pSavePlayer->m_lCrowns);

                    } else {
                        errCode = ERR_MEMORY;
                    }

                } else {
                    errCode = ERR_MEMORY;
                }

            } else {
                errCode = ERR_MEMORY;
            }

        } else {
            errCode = ERR_MEMORY;
        }
        
        pItem = pPlayer->m_pInventory->FindItem(i == 0 ? MG_OBJ_HODJ_NOTEBOOK : MG_OBJ_PODJ_NOTEBOOK);
        (*pItem).SetActionCode(ITEM_ACTION_NOTEBOOK);

        for ( j = 0; j < NUMBER_OF_CLUES; j++ ) {

            pPlayer->m_aClueArray[j].pNote = new CNote(pSavePlayer->m_bClueArray[j].iNoteID,
                    CMgStatic::cHodjClueTable[j].m_iCluePicCode - MG_CLUEPIC_BASE + NOTE_ICON_BASE,
                    CMgStatic::cHodjClueTable[j].m_iCluePicCount, pSavePlayer->m_bClueArray[j].iPersonID,
                    pSavePlayer->m_bClueArray[j].iPlaceID);
            pPlayer->m_aClueArray[j].bUsed = pSavePlayer->m_bClueArray[j].bUsed;

            // must add this clue to the players notebook
            if (pSavePlayer->m_bClueArray[j].bUsed) {

                pPlayer->m_aClueArray[j].pNote->SetID(pSavePlayer->m_bClueArray[j].iNoteID);
                pPlayer->m_aClueArray[j].pNote->SetPersonID(pSavePlayer->m_bClueArray[j].iPersonID);
                pPlayer->m_aClueArray[j].pNote->SetPlaceID(pSavePlayer->m_bClueArray[j].iPlaceID);

                pPlayer->m_pInventory->FindItem(i == 0 ? MG_OBJ_HODJ_NOTEBOOK : MG_OBJ_PODJ_NOTEBOOK)->AddNote(pPlayer->m_aClueArray[j].pNote);
            }
        }
    }

    return(errCode);
}


VOID WipeBFC(CBfcMgr *pBfcMgr)
{
    int i;

    assert(pBfcMgr != NULL);

    pBfcMgr->m_bRestoredGame = FALSE;

    // delete any Mish/Mosh items
    //
    if (pBfcMgr->m_pMishItem != NULL) {
        delete pBfcMgr->m_pMishItem;
        pBfcMgr->m_pMishItem = NULL;
    }
    if (pBfcMgr->m_pMoshItem != NULL) {
        delete pBfcMgr->m_pMoshItem;
        pBfcMgr->m_pMoshItem = NULL;
    }

    if ( pBfcMgr->m_cPodj.m_pBlackMarket != NULL ) {
        delete pBfcMgr->m_cPodj.m_pBlackMarket;
        pBfcMgr->m_cPodj.m_pBlackMarket = NULL;
    }
    if ( pBfcMgr->m_cHodj.m_pBlackMarket != NULL ) {
        delete pBfcMgr->m_cHodj.m_pBlackMarket;
        pBfcMgr->m_cHodj.m_pBlackMarket = NULL;
    }

    if ( pBfcMgr->m_cPodj.m_pGenStore != NULL ) {
        delete pBfcMgr->m_cPodj.m_pGenStore;
        pBfcMgr->m_cPodj.m_pGenStore = NULL;
    }

    if ( pBfcMgr->m_cHodj.m_pGenStore != NULL ) {
        delete pBfcMgr->m_cHodj.m_pGenStore;
        pBfcMgr->m_cHodj.m_pGenStore = NULL;
    }

    if ( pBfcMgr->m_cPodj.m_pTradingPost != NULL ) {
        delete pBfcMgr->m_cPodj.m_pTradingPost;
        pBfcMgr->m_cPodj.m_pTradingPost = NULL;
    }

    if ( pBfcMgr->m_cHodj.m_pTradingPost != NULL ) {
        delete pBfcMgr->m_cHodj.m_pTradingPost;
        pBfcMgr->m_cHodj.m_pTradingPost = NULL;
    }

    if ( pBfcMgr->m_cPodj.m_pInventory != NULL ) {
        delete pBfcMgr->m_cPodj.m_pInventory;
        pBfcMgr->m_cPodj.m_pInventory = NULL;
    }

    if ( pBfcMgr->m_cHodj.m_pInventory != NULL ) {
        delete pBfcMgr->m_cHodj.m_pInventory;
        pBfcMgr->m_cHodj.m_pInventory = NULL;
    }

    // This deallocation MUST be after the delete pInventorys
    //
    for ( i = 0; i < NUMBER_OF_CLUES; i++ ) {

        if (!pBfcMgr->m_cHodj.m_aClueArray[i].bUsed) {

            if (pBfcMgr->m_cHodj.m_aClueArray[i].pNote != NULL) {
                delete pBfcMgr->m_cHodj.m_aClueArray[i].pNote;
                pBfcMgr->m_cHodj.m_aClueArray[i].pNote = NULL;
            }
        }

        if (!pBfcMgr->m_cPodj.m_aClueArray[i].bUsed) {

            if (pBfcMgr->m_cPodj.m_aClueArray[i].pNote != NULL) {
                delete pBfcMgr->m_cPodj.m_aClueArray[i].pNote;
                pBfcMgr->m_cPodj.m_aClueArray[i].pNote = NULL;
            }
        }
    }
}
