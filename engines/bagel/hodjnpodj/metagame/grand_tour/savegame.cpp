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

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace GrandTour {

#include "bagel/afxwin.h"

#include <stdio.h>
#include <assert.h>
#include "bagel/hodjnpodj/hnplibs/stdinc.h"
#include "bagel/boflib/misc.h"
#include <errors.h>

#include "bagel/hodjnpodj/metagame/bgen/bfc.h"
#include "bagel/hodjnpodj/metagame/bgen/c1btndlg.h"
#include "savegame.h"
#include "savedlg.h"

#define IDC_SAVE_BUSY 2030

STATIC CHAR gszSaveGameFileName[256];

STATIC CHAR szDescriptions[MAX_SAVEGAMES][40];
STATIC CHAR *pszDesc[MAX_SAVEGAMES];

CHAR szDescBuf[40];

// Local prototypes
//
ERROR_CODE GetSaveGameDescriptions(VOID);
ERROR_CODE SaveSlot(INT, CBfcMgr *);
ERROR_CODE InitSaveGameFile(VOID);
ERROR_CODE WriteSavedGame(FILE *, INT, SAVEGAME_INFO *);
ERROR_CODE ReadSavedGame(FILE *, INT, SAVEGAME_INFO *);
VOID       ConvertToSGI(CBfcMgr *, SAVEGAME_INFO *);


/*****************************************************************************
*
*  SaveGame         - Loads a Save-Game dialog box
*
*  DESCRIPTION:     User can choose which slot they want to save a game into,
*                   and can give a description of the saved game.
*
*  SAMPLE USAGE:
*  errCode = SaveGame(psFileName, pBfcMgr, pWnd, pPalette);
*  const CHAR *pszFileName;                 File to save into
*  CBfMgr *pBfcMgr;                         SaveGame info
*  CWnd *pWnd;                              Parent window to SaveGame dialog box
*  CPalette *pPalette;                      Palette to use for dialog Bmp
*  ERROR_CODE *pErrCode;                    pointer to error return code
*
*  RETURNS:  BOOL = TRUE if the game was saved successfully
*
*****************************************************************************/
BOOL CALLBACK SaveGame(const CHAR *pszFileName, CBfcMgr *pBfcMgr, CWnd *pWnd, CPalette *pPalette, ERROR_CODE *pErrCode)
{
    CWinApp *pMyApp;
    HCURSOR hCursor;
    INT iGameNum, n;
    BOOL bSaved;
    ERROR_CODE errCode;

    // validate input
    assert(pszFileName != NULL);
    assert(pBfcMgr != NULL);
    assert(pWnd != NULL);
    assert(pPalette != NULL);

    // if not keeping track of errors externally
    if (pErrCode == NULL)
        pErrCode = &errCode;

    // assume no error
    *pErrCode = ERR_NONE;

    // assume the save will work
    bSaved = TRUE;

    // keep track of the .SAV file name
    //
    strcpy(gszSaveGameFileName, pBfcMgr->m_chHomePath);
    n = strlen(gszSaveGameFileName);
    if (gszSaveGameFileName[n - 1] != '\\')
        strcat(gszSaveGameFileName, "\\");
    strcat(gszSaveGameFileName, pszFileName);

    // if .SAV file does not exist
    //
    if (!FileExists(gszSaveGameFileName)) {

        // then create an empty save game file
        //
        *pErrCode = InitSaveGameFile();
    }

    if (*pErrCode == ERR_NONE) {

        if ((*pErrCode = GetSaveGameDescriptions()) == ERR_NONE) {

            //
            // open the Save Game dialog box
            //
            CSaveDlg dlgSave(pszDesc, pWnd, pPalette);

            if ((iGameNum = dlgSave.DoModal()) != -1) {

                // save users description of this saved game
                strcpy(szDescriptions[iGameNum], szDescBuf);

                // change cursor to a disk to indicate saving
                //
                pMyApp = AfxGetApp();
                hCursor = pMyApp->LoadCursor(IDC_SAVE_BUSY);
                assert(hCursor != NULL);
                ::SetCursor(hCursor);

                // pause for a 2 seconds to make it look good
                Sleep(2000);

                // do the actual save
                *pErrCode = SaveSlot(iGameNum, pBfcMgr);

                // change cursor back to arrow
                //
                hCursor = pMyApp->LoadStandardCursor(IDC_ARROW);
                assert(hCursor != NULL);
                ::SetCursor(hCursor);

            } else {
                bSaved = FALSE;
            }
        }
    }

    if (*pErrCode != ERR_NONE) {

        bSaved = FALSE;
        C1ButtonDialog dlgOk(pWnd, pPalette, "&OK", "An error occured", "while saving.");
        dlgOk.DoModal();
    }

    if (bSaved) {

        pBfcMgr->m_bChanged = FALSE;

        // Show this dialog box 10% of the time
        //
        if (ProbableTrue(10)) {
            C1ButtonDialog dlgOk(pWnd, pPalette, "&Yes", "Don't you wish", "saving Mish and Mosh", "was that easy?");
            dlgOk.DoModal();
        }
    }

//#ifdef _DEBUG
    if (*pErrCode != ERR_NONE) {
        ErrorLog("ERROR.LOG", "errCode = %d in %s at line %d", *pErrCode, __FILE__, __LINE__);
    }
//#endif

    return(bSaved);
}


/*****************************************************************************
*
*  GetrSaveGameDescriptions - Retrieves Game descriptions
*
*  DESCRIPTION:     Opens the .SAV file and gets the game description of each
*                   game.
*
*  SAMPLE USAGE:
*  errCode = GetSaveGameDescriptions();
*
*  RETURNS:  ERROR_CODE = error return code.
*
*****************************************************************************/
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
    if (FileExists(gszSaveGameFileName)) {

        if ((pSaveGameInfo = (SAVEGAME_INFO *)malloc(sizeof(SAVEGAME_INFO))) != NULL) {

            // open the .SAV file
            //
            if ((pFile = fopen(gszSaveGameFileName, "rb")) != NULL) {

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
        errCode = ERR_FFIND;
        ErrorLog("ERROR.LOG", "%s not found.  Can't restore game.", gszSaveGameFileName);
    }

    return(errCode);
}


/*****************************************************************************
*
*  SaveSlot         - one line discription
*
*  DESCRIPTION:
*
*
*  SAMPLE USAGE:
*  errCode = SaveSlot(iGameNum, pBfcMgr);
*  INT iGameNum;                            Game slot to save into
*  CBfcMgr *pBfcMgr;                        info to save
*
*  RETURNS:  ERROR_CODE = Error return code.
*
*****************************************************************************/
ERROR_CODE SaveSlot(INT iGameNum, CBfcMgr *pBfcMgr)
{
    SAVEGAME_INFO *pSaveGameInfo;
    FILE *pFile;
    ERROR_CODE errCode;

    // assume no error
    errCode = ERR_NONE;

    assert(iGameNum >= 0 && iGameNum < MAX_SAVEGAMES);
    assert(pBfcMgr != NULL);

    // if the save game file does not already exist then create one with
    // 10 empty slots
    //
    if (!FileExists(gszSaveGameFileName)) {

        errCode = InitSaveGameFile();
    }

    // allocate 1 SGI record
    if ((pSaveGameInfo = (SAVEGAME_INFO *)malloc(sizeof(SAVEGAME_INFO))) != NULL) {

        ConvertToSGI(pBfcMgr, pSaveGameInfo);

        // copy user's description of this saved game
        strcpy(pSaveGameInfo->m_szSaveGameDescription, szDescriptions[iGameNum]);

        if ((pFile = fopen(gszSaveGameFileName, "r+b")) != NULL) {

            errCode = WriteSavedGame(pFile, iGameNum, pSaveGameInfo);

            fclose(pFile);
        } else {
            errCode = ERR_FOPEN;
        }

       free(pSaveGameInfo);
    } else {
        errCode = ERR_MEMORY;
    }

//#ifdef _DEBUG
    if (errCode != ERR_NONE) {
        ErrorLog("ERROR.LOG", "errCode = %d in %s at line %d", errCode, __FILE__, __LINE__);
    }
//#endif

    return(errCode);
}


/*****************************************************************************
*
*  InitSaveGameFile - Creates a .SAV file if one does not already exist
*
*  DESCRIPTION:     Initializes a new .SAV file containing blank records.
*
*  SAMPLE USAGE:
*  errCode = InitSaveGameFile();
*
*  RETURNS:  ERROR_CODE = error return code.
*
*****************************************************************************/
ERROR_CODE InitSaveGameFile(VOID)
{
    SAVEGAME_INFO *pNewInfo;
    FILE *pFile;
    ERROR_CODE errCode;
    INT i;

    // assume no error
    errCode = ERR_NONE;

    // we don't want to overwrite a good .SAV file
    assert(!FileExists(gszSaveGameFileName));

    // allocate one save game record
    //
    if ((pNewInfo = (SAVEGAME_INFO *)malloc(sizeof(SAVEGAME_INFO))) != NULL) {

        memset(pNewInfo, 0, sizeof(SAVEGAME_INFO));

        pNewInfo->m_lBoffoGameID = BOFFO_GAME_ID;
        pNewInfo->m_nFixedRecordSize = sizeof(SAVEGAME_INFO);

        // this slot is not used
        pNewInfo->m_bUsed = FALSE;

        if ((pFile = fopen(gszSaveGameFileName, "w+b")) != NULL) {

            for (i = 0; i < MAX_SAVEGAMES; i++) {
                if ((errCode = WriteSavedGame(pFile, i, pNewInfo)) != ERR_NONE)
                    break;
            }

            fclose(pFile);
        } else {
            errCode = ERR_FOPEN;
        }

       free(pNewInfo);
    } else {
        errCode = ERR_MEMORY;
    }

    return(errCode);
}


/*****************************************************************************
*
*  WriteSavedGame   - Writes one save game record to the .SAV file
*
*  DESCRIPTION:
*                   
*
*  SAMPLE USAGE:
*  errCode = WriteSaveGame(pFile, iGameNum, pSaveGameInfo);
*  FILE *pFile;                             .SAV file
*  INT iGameNum;                            Slot to save into
*  SAVEGAME_INFO *pSaveGameInfo;            Record info to write
*
*  RETURNS:  ERROR_CODE = error return code.
*
*****************************************************************************/
ERROR_CODE WriteSavedGame(FILE *pFile, INT iGameNum, SAVEGAME_INFO *pSaveGameInfo)
{
    ERROR_CODE errCode;

    // asume no error
    errCode = ERR_NONE;

    // validate explicit input
    assert(pFile != NULL);
    assert(iGameNum >= 0 && iGameNum < MAX_SAVEGAMES);
    assert(pSaveGameInfo != NULL);

    Encrypt(pSaveGameInfo, sizeof(SAVEGAME_INFO));

    // seek to correct save game number (slot)
    //
    if (fseek(pFile, iGameNum * sizeof(SAVEGAME_INFO), SEEK_SET) == 0) {

        // write the save game to that slot
        //
        if (fwrite(pSaveGameInfo, sizeof(SAVEGAME_INFO), 1, pFile) != 1) {
            errCode = ERR_FWRITE;
        }

    } else {
        errCode = ERR_FSEEK;
    }

    Decrypt(pSaveGameInfo, sizeof(SAVEGAME_INFO));

    return(errCode);
}


/*****************************************************************************
*
*  ReadSavedGame    - Reads one save game record from the .SAV file
*
*  DESCRIPTION:
*                   
*
*  SAMPLE USAGE:
*  errCode = ReadSaveGame(pFile, iGameNum, pSaveGameInfo);
*  FILE *pFile;                             .SAV file
*  INT iGameNum;                            Slot to Read from
*  SAVEGAME_INFO *pSaveGameInfo;            Storage for record info
*
*  RETURNS:  ERROR_CODE = error return code.
*
*****************************************************************************/
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


/*****************************************************************************
*
*  ConvertToSGI     - Converts a CBfcMgr to a SAVEGAME_INFO
*
*  DESCRIPTION:
*                   
*
*  SAMPLE USAGE:
*  ConvertToSGI(pBfcMgr, pSaveGameInfo);
*  CBfcMgr *pBfcMgr;                        Meta Game info to be converted
*  SAVEGAME_INFO *pSaveGameInfo;            Storage for record info
*
*  RETURNS:  ERROR_CODE = error return code.
*
*****************************************************************************/
VOID ConvertToSGI(CBfcMgr *pBfcMgr, SAVEGAME_INFO *pSaveGameInfo)
{
    CHodjPodj *pPlayer;
    CInventory *pInv;
    CItem *pItem;
    PLAYER_INFO *pSavePlayer;
    INVENTORY *pSaveInv;
    INT i, j, k;

    assert(pBfcMgr != NULL);
    assert(pSaveGameInfo != NULL);

    // reset entire info structure
    memset(pSaveGameInfo, 0, sizeof(SAVEGAME_INFO));

    pSaveGameInfo->m_lBoffoGameID = BOFFO_GAME_ID;
    pSaveGameInfo->m_nFixedRecordSize = sizeof(SAVEGAME_INFO);
    pSaveGameInfo->m_bUsed = TRUE;

    pSaveGameInfo->m_iGameTime = pBfcMgr->m_iGameTime;
    pSaveGameInfo->m_iMishMoshLoc = pBfcMgr->m_iMishMoshLoc;

    memcpy(&pSaveGameInfo->m_bTraps, &pBfcMgr->m_bTraps, sizeof(BOOL) * 240);

    pSaveGameInfo->m_bSoundEffectsEnabled = pBfcMgr->m_stGameStruct.bSoundEffectsEnabled;
    pSaveGameInfo->m_bMusicEnabled = pBfcMgr->m_stGameStruct.bMusicEnabled;
    pSaveGameInfo->m_bScrolling = pBfcMgr->m_bScrolling;

    pSaveGameInfo->m_bNewMishMosh = (pBfcMgr->m_pMishItem != NULL);

    pPlayer = &pBfcMgr->m_cHodj;
    for (i = 0; i < MAX_PLAYERS; i++) {

        if (i == 1) {
            pPlayer = &pBfcMgr->m_cPodj;
        }
        pSavePlayer = &pSaveGameInfo->m_stPlayerInfo[i];

        pSavePlayer->m_bMoving = pPlayer->m_bMoving;
        pSavePlayer->m_bComputer = pPlayer->m_bComputer;
        pSavePlayer->m_bHaveMishMosh = pPlayer->m_bHaveMishMosh;

        pSavePlayer->m_iSectorCode = pPlayer->m_iSectorCode;
        pSavePlayer->m_iNode = pPlayer->m_iNode;
        pSavePlayer->m_iSkillLevel = pPlayer->m_iSkillLevel;

        pSavePlayer->m_iWinInfoWon = pPlayer->m_iWinInfoWon;
        pSavePlayer->m_iWinInfoNeed = pPlayer->m_iWinInfoNeed;

        memcpy(&pSavePlayer->m_iWinInfoTable, &pPlayer->m_iWinInfoTable, sizeof(INT) * MAX_GAME_TABLE);

        pSavePlayer->m_iSecondaryInfoWon = pPlayer->m_iSecondaryInfoWon;
        pSavePlayer->m_iSecondaryInfoNeed = pPlayer->m_iSecondaryInfoNeed;

        memcpy(&pSavePlayer->m_iSecondaryInfoTable, &pPlayer->m_iSecondaryInfoTable, sizeof(INT) * MAX_GAME_TABLE);

        pSavePlayer->m_iRequiredObjectsCount = pPlayer->m_iRequiredObjectsCount;

        memcpy(&pSavePlayer->m_iRequiredObjectsTable, &pPlayer->m_iRequiredObjectsTable, sizeof(INT) * MAX_GAME_TABLE);

        pSavePlayer->m_iRequiredMoney = pPlayer->m_iRequiredMoney;

        memcpy(&pSavePlayer->m_iSecondaryLoc, &pPlayer->m_iSecondaryLoc, sizeof(INT) * MAX_GAME_TABLE);

        memcpy(&pSavePlayer->m_iGameHistory, &pPlayer->m_iGameHistory, sizeof(INT) * 20);

        pSavePlayer->m_iTargetLocation = pPlayer->m_iTargetLocation;
        pSavePlayer->m_iSpecialTravelCode = pPlayer->m_iSpecialTravelCode;
        pSavePlayer->m_iNumberBoatTries = pPlayer->m_iNumberBoatTries;
        pSavePlayer->m_iFurlongs = pPlayer->m_iFurlongs;
        pSavePlayer->m_nTurns = pPlayer->m_nTurns;

        assert(pPlayer->m_pInventory != NULL);

        pSavePlayer->m_lCrowns = pPlayer->m_pInventory->FindItem(MG_OBJ_CROWN)->GetQuantity();

        // make sure player's crown inventory item is valid
        assert(pSavePlayer->m_lCrowns >= 0);

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

            if (pInv != NULL) {

                strcpy(pSaveInv->m_szTitle, pInv->GetTitle());

                k = 0;
                pItem = pInv->FirstItem();
                while (pItem != NULL) {
                    pSaveInv->m_aItemList[k] = pItem->GetID();

                    assert(pSaveInv->m_aItemList[k] >= MG_OBJ_BASE && pSaveInv->m_aItemList[k] <= MG_OBJ_MAX);

                    k++;
                    pItem = pItem->GetNext();
                }
                assert(k == pInv->ItemCount());

                pSaveInv->m_nItems = k;
            }
        }

        for (j = 0; j < NUMBER_OF_CLUES; j++) {
            pSavePlayer->m_bClueArray[j].bUsed = pPlayer->m_aClueArray[j].bUsed;
            pSavePlayer->m_bClueArray[j].iNoteID = pPlayer->m_aClueArray[j].pNote->GetID();
            pSavePlayer->m_bClueArray[j].iPersonID = pPlayer->m_aClueArray[j].pNote->GetPersonID();
            pSavePlayer->m_bClueArray[j].iPlaceID = pPlayer->m_aClueArray[j].pNote->GetPlaceID();
        }
    }
}

} // namespace GrandTour
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
