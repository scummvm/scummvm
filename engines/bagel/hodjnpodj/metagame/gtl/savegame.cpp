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

#include "common/system.h"
#include "common/savefile.h"
#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/hnplibs/stdinc.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/boflib/misc.h"
#include "bagel/boflib/error.h"
#include "bagel/hodjnpodj/metagame/bgen/bfc.h"
#include "bagel/hodjnpodj/metagame/bgen/c1btndlg.h"
#include "bagel/hodjnpodj/metagame/gtl/savegame.h"
#include "bagel/hodjnpodj/metagame/gtl/savedlg.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

#define IDC_SAVE_BUSY 2030

STATIC CHAR szDescriptions[MAX_SAVEGAMES][40];
STATIC CHAR *pszDesc[MAX_SAVEGAMES];

CHAR szDescBuf[40];

// Local prototypes
static ERROR_CODE GetSaveGameDescriptions(VOID);

BOOL CALLBACK SaveGame(CBfcMgr *pBfcMgr, CWnd *pWnd, CPalette *pPalette, ERROR_CODE *pErrCode) {
	CWinApp *pMyApp;
	HCURSOR hCursor;
	INT iGameNum;
	BOOL bSaved;
	ERROR_CODE errCode;
	assert(pBfcMgr == &g_engine->_bfcMgr);

	// Validate input
	assert(pWnd != nullptr);
	assert(pPalette != nullptr);

	// If not keeping track of errors externally
	if (pErrCode == nullptr)
		pErrCode = &errCode;

	// Assume no error
	*pErrCode = ERR_NONE;

	// Assume the save will work
	bSaved = TRUE;

	if ((*pErrCode = GetSaveGameDescriptions()) == ERR_NONE) {
		// Open the Save Game dialog box
		CSaveDlg dlgSave(pszDesc, pWnd, pPalette);

		if ((iGameNum = dlgSave.DoModal()) != -1) {
			// Save users description of this saved game
			Common::strcpy_s(szDescriptions[iGameNum], szDescBuf);

			// Change cursor to a disk to indicate saving
			pMyApp = AfxGetApp();
			hCursor = pMyApp->LoadCursor(IDC_SAVE_BUSY);
			assert(hCursor != nullptr);
			MFC::SetCursor(hCursor);

			// Pause for a 2 seconds to make it look good
			Sleep(2000);

			// Do the actual save
			Common::String desc = szDescriptions[iGameNum];
			assert(iGameNum >= 0 && iGameNum < MAX_SAVEGAMES);
			g_engine->saveGameState(iGameNum + 1, desc);

			// Change cursor back to arrow
			hCursor = pMyApp->LoadStandardCursor(IDC_ARROW);
			assert(hCursor != nullptr);
			MFC::SetCursor(hCursor);

		} else {
			bSaved = FALSE;
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
		if (ProbableTrue(10)) {
			C1ButtonDialog dlgOk(pWnd, pPalette, "&Yes", "Don't you wish", "saving Mish and Mosh", "was that easy?");
			dlgOk.DoModal();
		}
	}

	return bSaved;
}

VOID ConvertToSGI(CBfcMgr *pBfcMgr, SAVEGAME_INFO *pSaveGameInfo) {
	CHodjPodj *pPlayer;
	CInventory *pInv;
	CItem *pItem;
	PLAYER_INFO *pSavePlayer;
	INVENTORY *pSaveInv;
	INT i, j, k;

	assert(pBfcMgr != nullptr);
	assert(pSaveGameInfo != nullptr);

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

	pSaveGameInfo->m_bNewMishMosh = (pBfcMgr->m_pMishItem != nullptr);

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

		assert(pPlayer->m_pInventory != nullptr);

		pSavePlayer->m_lCrowns = pPlayer->m_pInventory->FindItem(MG_OBJ_CROWN)->GetQuantity();

		// make sure player's crown inventory item is valid
		assert(pSavePlayer->m_lCrowns >= 0);

		for (j = 0; j < 4; j++) {

			switch (j) {

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

			if (pInv != nullptr) {

				Common::strcpy_s(pSaveInv->m_szTitle, pInv->GetTitle());

				k = 0;
				pItem = pInv->FirstItem();
				while (pItem != nullptr) {
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

/**
 * Gets the list of savegame descriptions
**/
static ERROR_CODE GetSaveGameDescriptions() {
	ERROR_CODE errCode = ERR_NONE;

	SaveStateList saves = g_engine->listSaves();

	for (int i = 0; i < MAX_SAVEGAMES; ++i) {
		szDescriptions[i][0] = '\0';
		pszDesc[i] = nullptr;
	}

	for (auto it = saves.begin(); it != saves.end(); ++it) {
		const auto saveSlot = *it;
		const int i = it->getSaveSlot();
		if (i >= 1 && i <= MAX_SAVEGAMES) {
			pszDesc[i - 1] = szDescriptions[i - 1];
			Common::String desc = saveSlot.getDescription();
			Common::strcpy_s(szDescriptions[i - 1], desc.c_str());
		}
	}

	return errCode;
}

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
