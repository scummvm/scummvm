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
#include "bagel/hodjnpodj/metagame/saves/savegame.h"
#include "bagel/hodjnpodj/metagame/saves/savedlg.h"
#include "bagel/hodjnpodj/metagame/saves/restdlg.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Saves {

#define IDC_SAVE_BUSY 2030

STATIC char szDescriptions[MAX_SAVEGAMES][40];
STATIC char *pszDesc[MAX_SAVEGAMES];

char szDescBuf[40];

struct stInventoryInfo {
	char m_szInventory[40];
	char m_szTradingPost[40];
	char m_szBlackMarket[40];
	char m_szGenStore[40];
} stPlayerBuf[MAX_PLAYERS];

// Local prototypes
static ERROR_CODE GetSaveGameDescriptions();
static void WipeBFC(CBfcMgr *pBfcMgr);

bool SaveGame(CWnd *pWnd, CPalette *pPalette, ERROR_CODE *pErrCode) {
	CWinApp *pMyApp;
	HCURSOR hCursor;
	int iGameNum;
	bool bSaved;
	ERROR_CODE errCode;

	// Validate input
	assert(pWnd != nullptr);
	assert(pPalette != nullptr);

	// If not keeping track of errors externally
	if (pErrCode == nullptr)
		pErrCode = &errCode;

	// Assume no error
	*pErrCode = ERR_NONE;

	// Assume the save will work
	bSaved = true;

	if ((*pErrCode = GetSaveGameDescriptions()) == ERR_NONE) {
		// Open the Save Game dialog box
		CSaveDlg dlgSave(pszDesc, pWnd, pPalette);

		if ((iGameNum = dlgSave.DoModal()) != -1) {
			// Save users description of this saved game
			assert(iGameNum < MAX_SAVEGAMES);
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
			if (g_engine->saveGameState(iGameNum + 1, desc).getCode() != Common::kNoError)
				*pErrCode = ERR_FOPEN;

			// Change cursor back to arrow
			hCursor = pMyApp->LoadStandardCursor(IDC_ARROW);
			assert(hCursor != nullptr);
			MFC::SetCursor(hCursor);

		} else {
			bSaved = false;
		}
	}

	if (*pErrCode != ERR_NONE) {
		bSaved = false;
		C1ButtonDialog dlgOk(pWnd, pPalette, "&OK", "An error occured", "while saving.");
		dlgOk.DoModal();
	}

	if (bSaved) {
		g_engine->_bfcMgr.m_bChanged = false;

		// Show this dialog box 10% of the time
		if (ProbableTrue(10)) {
			C1ButtonDialog dlgOk(pWnd, pPalette, "&Yes", "Don't you wish", "saving Mish and Mosh", "was that easy?");
			dlgOk.DoModal();
		}
	}

	return bSaved;
}

bool RestoreGame(CWnd *pWnd, CPalette *pPalette, ERROR_CODE *pErrCode, int iGameNum) {
	bool bRestored;

	// Validate input
	assert(pWnd != nullptr);
	assert(pPalette != nullptr);
	assert(pErrCode != nullptr);

	// Assume no error
	*pErrCode = ERR_NONE;

	// Assume Restore will work
	bRestored = true;

	*pErrCode = GetSaveGameDescriptions();

	if (*pErrCode == ERR_NONE) {
		if (iGameNum == -1) {
			// Open the Restore Game dialog box
			CRestoreDlg dlgRestore(pszDesc, pWnd, pPalette);
			iGameNum = dlgRestore.DoModal();
			if (iGameNum != -1)
				++iGameNum;
		}

		if (iGameNum != -1) {
			if (g_engine->loadGameState(iGameNum).getCode() != Common::kNoError)
				*pErrCode = ERR_FOPEN;
		} else {
			bRestored = false;
		}
	}

	if (*pErrCode != ERR_NONE) {
		bRestored = false;
	}

	return bRestored;
}

void ConvertToSGI(CBfcMgr *pBfcMgr, SAVEGAME_INFO *pSaveGameInfo) {
	CHodjPodj *pPlayer;
	CInventory *pInv;
	CItem *pItem;
	PLAYER_INFO *pSavePlayer;
	INVENTORY *pSaveInv;
	int i, j, k;

	assert(pBfcMgr != nullptr);
	assert(pSaveGameInfo != nullptr);

	// reset entire info structure
	*pSaveGameInfo = SAVEGAME_INFO();

	pSaveGameInfo->m_lBoffoGameID = BOFFO_GAME_ID;
	pSaveGameInfo->m_nFixedRecordSize = sizeof(SAVEGAME_INFO);
	pSaveGameInfo->m_bUsed = true;

	pSaveGameInfo->m_iGameTime = pBfcMgr->m_iGameTime;
	pSaveGameInfo->m_iMishMoshLoc = pBfcMgr->m_iMishMoshLoc;

	memcpy(&pSaveGameInfo->m_bTraps, &pBfcMgr->m_bTraps, sizeof(bool) * 240);

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

		memcpy(&pSavePlayer->m_iWinInfoTable, &pPlayer->m_iWinInfoTable, sizeof(int) * MAX_GAME_TABLE);

		pSavePlayer->m_iSecondaryInfoWon = pPlayer->m_iSecondaryInfoWon;
		pSavePlayer->m_iSecondaryInfoNeed = pPlayer->m_iSecondaryInfoNeed;

		memcpy(&pSavePlayer->m_iSecondaryInfoTable, &pPlayer->m_iSecondaryInfoTable, sizeof(int) * MAX_GAME_TABLE);

		pSavePlayer->m_iRequiredObjectsCount = pPlayer->m_iRequiredObjectsCount;

		memcpy(&pSavePlayer->m_iRequiredObjectsTable, &pPlayer->m_iRequiredObjectsTable, sizeof(int) * MAX_GAME_TABLE);

		pSavePlayer->m_iRequiredMoney = pPlayer->m_iRequiredMoney;

		memcpy(&pSavePlayer->m_iSecondaryLoc, &pPlayer->m_iSecondaryLoc, sizeof(int) * MAX_GAME_TABLE);

		memcpy(&pSavePlayer->m_iGameHistory, &pPlayer->m_iGameHistory, sizeof(int) * 20);

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


bool ValidateSGInfo(SAVEGAME_INFO *pSaveGameInfo) {
	// can't acess a null pointer
	assert(pSaveGameInfo != nullptr);

	// this must be a valid saved game
	assert(pSaveGameInfo->m_bUsed);

	if (pSaveGameInfo->m_lBoffoGameID != BOFFO_GAME_ID) {
		return false;
	}

	switch (pSaveGameInfo->m_iGameTime) {
	case SHORT_GAME:
	case MEDIUM_GAME:
	case LONG_GAME:
		break;

	// not a valid game type
	default:
		return false;
	}

	if (pSaveGameInfo->m_iMishMoshLoc < MG_LOC_BASE || pSaveGameInfo->m_iMishMoshLoc > MG_LOC_MAX) {
		return false;
	}

	return true;
}

ERROR_CODE ConvertFromSGI(CBfcMgr *pBfcMgr, SAVEGAME_INFO *pSaveGameInfo) {
	CHodjPodj *pPlayer;
	CInventory *pInv;
	CItem *pItem;
	PLAYER_INFO *pSavePlayer;
	INVENTORY *pSaveInv;
	int i, j, k;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	assert(pBfcMgr != nullptr);
	assert(pSaveGameInfo != nullptr);

	// clean out any info currently in the CBfcMgr
	WipeBFC(pBfcMgr);

	pBfcMgr->m_bInventories = true;
	pBfcMgr->m_bRestart = true;
	pBfcMgr->m_bGameOver = false;

	pBfcMgr->m_bChanged = false;

	memcpy(&pBfcMgr->m_bTraps, &pSaveGameInfo->m_bTraps, sizeof(bool) * 240);

	pBfcMgr->m_iFunctionCode = 0;

	pBfcMgr->m_stGameStruct.bSoundEffectsEnabled = pSaveGameInfo->m_bSoundEffectsEnabled;
	pBfcMgr->m_stGameStruct.bMusicEnabled = pSaveGameInfo->m_bMusicEnabled;
	pBfcMgr->m_stGameStruct.bPlayingMetagame = true;
	pBfcMgr->m_bScrolling = pSaveGameInfo->m_bScrolling;

	// need to allocate new Mish/Mosh items
	if (pSaveGameInfo->m_bNewMishMosh) {

		pBfcMgr->m_pMishItem = new CItem(MG_OBJ_MISH);
		pBfcMgr->m_pMoshItem = new CItem(MG_OBJ_MOSH);
	}

	pBfcMgr->m_bRestoredGame = true;

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

		memcpy(&pPlayer->m_iWinInfoTable, &pSavePlayer->m_iWinInfoTable, sizeof(int) * MAX_GAME_TABLE);

		pPlayer->m_iSecondaryInfoWon = pSavePlayer->m_iSecondaryInfoWon;
		pPlayer->m_iSecondaryInfoNeed = pSavePlayer->m_iSecondaryInfoNeed;

		memcpy(&pPlayer->m_iSecondaryInfoTable, &pSavePlayer->m_iSecondaryInfoTable, sizeof(int) * MAX_GAME_TABLE);

		pPlayer->m_iRequiredObjectsCount = pSavePlayer->m_iRequiredObjectsCount;

		memcpy(&pPlayer->m_iRequiredObjectsTable, &pSavePlayer->m_iRequiredObjectsTable, sizeof(int) * MAX_GAME_TABLE);

		pPlayer->m_iRequiredMoney = pSavePlayer->m_iRequiredMoney;

		memcpy(&pPlayer->m_iSecondaryLoc, &pSavePlayer->m_iSecondaryLoc, sizeof(int) * MAX_GAME_TABLE);

		memcpy(&pPlayer->m_iGameHistory, &pSavePlayer->m_iGameHistory, sizeof(int) * 20);

		pPlayer->m_iTargetLocation = pSavePlayer->m_iTargetLocation;
		pPlayer->m_iSpecialTravelCode = pSavePlayer->m_iSpecialTravelCode;
		pPlayer->m_iNumberBoatTries = pSavePlayer->m_iNumberBoatTries;
		pPlayer->m_iFurlongs = pSavePlayer->m_iFurlongs;
		pPlayer->m_nTurns = pSavePlayer->m_nTurns;

		Common::strcpy_s(stPlayerBuf[i].m_szTradingPost, pSavePlayer->m_stTradingPost.m_szTitle);
		Common::strcpy_s(stPlayerBuf[i].m_szBlackMarket, pSavePlayer->m_stBlackMarket.m_szTitle);
		Common::strcpy_s(stPlayerBuf[i].m_szGenStore, pSavePlayer->m_stGenStore.m_szTitle);
		Common::strcpy_s(stPlayerBuf[i].m_szInventory, pSavePlayer->m_stInventory.m_szTitle);

		if ((pPlayer->m_pTradingPost = new CInventory(stPlayerBuf[i].m_szTradingPost)) != nullptr) {

			if ((pPlayer->m_pBlackMarket = new CInventory(stPlayerBuf[i].m_szBlackMarket)) != nullptr) {

				if ((pPlayer->m_pGenStore = new CInventory(stPlayerBuf[i].m_szGenStore)) != nullptr) {

					if ((pPlayer->m_pInventory = new CInventory(stPlayerBuf[i].m_szInventory)) != nullptr) {

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

							// add specified items to the inventory
							//
							for (k = 0; k < pSaveInv->m_nItems; k++) {

								// Mish/Mosh can't be in the inventory if we
								// were supposed to allocate them
								//assert(pSaveGameInfo->m_bNewMishMosh || ((pSaveInv->m_aItemList[k] != MG_OBJ_MOSH) && (pSaveInv->m_aItemList[k] != MG_OBJ_MOSH)));

								pInv->AddItem(pSaveInv->m_aItemList[k], 1);
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

		for (j = 0; j < NUMBER_OF_CLUES; j++) {

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

	return errCode;
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

static void WipeBFC(CBfcMgr *pBfcMgr) {
	int i;

	assert(pBfcMgr != nullptr);

	pBfcMgr->m_bRestoredGame = false;

	// Delete any Mish/Mosh items
	if (pBfcMgr->m_pMishItem != nullptr) {
		delete pBfcMgr->m_pMishItem;
		pBfcMgr->m_pMishItem = nullptr;
	}
	if (pBfcMgr->m_pMoshItem != nullptr) {
		delete pBfcMgr->m_pMoshItem;
		pBfcMgr->m_pMoshItem = nullptr;
	}

	if (pBfcMgr->m_cPodj.m_pBlackMarket != nullptr) {
		delete pBfcMgr->m_cPodj.m_pBlackMarket;
		pBfcMgr->m_cPodj.m_pBlackMarket = nullptr;
	}
	if (pBfcMgr->m_cHodj.m_pBlackMarket != nullptr) {
		delete pBfcMgr->m_cHodj.m_pBlackMarket;
		pBfcMgr->m_cHodj.m_pBlackMarket = nullptr;
	}

	if (pBfcMgr->m_cPodj.m_pGenStore != nullptr) {
		delete pBfcMgr->m_cPodj.m_pGenStore;
		pBfcMgr->m_cPodj.m_pGenStore = nullptr;
	}

	if (pBfcMgr->m_cHodj.m_pGenStore != nullptr) {
		delete pBfcMgr->m_cHodj.m_pGenStore;
		pBfcMgr->m_cHodj.m_pGenStore = nullptr;
	}

	if (pBfcMgr->m_cPodj.m_pTradingPost != nullptr) {
		delete pBfcMgr->m_cPodj.m_pTradingPost;
		pBfcMgr->m_cPodj.m_pTradingPost = nullptr;
	}

	if (pBfcMgr->m_cHodj.m_pTradingPost != nullptr) {
		delete pBfcMgr->m_cHodj.m_pTradingPost;
		pBfcMgr->m_cHodj.m_pTradingPost = nullptr;
	}

	if (pBfcMgr->m_cPodj.m_pInventory != nullptr) {
		delete pBfcMgr->m_cPodj.m_pInventory;
		pBfcMgr->m_cPodj.m_pInventory = nullptr;
	}

	if (pBfcMgr->m_cHodj.m_pInventory != nullptr) {
		delete pBfcMgr->m_cHodj.m_pInventory;
		pBfcMgr->m_cHodj.m_pInventory = nullptr;
	}

	// This deallocation MUST be after the delete pInventorys
	for (i = 0; i < NUMBER_OF_CLUES; i++) {
		if (!pBfcMgr->m_cHodj.m_aClueArray[i].bUsed) {
			if (pBfcMgr->m_cHodj.m_aClueArray[i].pNote != nullptr) {
				delete pBfcMgr->m_cHodj.m_aClueArray[i].pNote;
				pBfcMgr->m_cHodj.m_aClueArray[i].pNote = nullptr;
			}
		}

		if (!pBfcMgr->m_cPodj.m_aClueArray[i].bUsed) {

			if (pBfcMgr->m_cPodj.m_aClueArray[i].pNote != nullptr) {
				delete pBfcMgr->m_cPodj.m_aClueArray[i].pNote;
				pBfcMgr->m_cPodj.m_aClueArray[i].pNote = nullptr;
			}
		}
	}
}

} // namespace Saves
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
