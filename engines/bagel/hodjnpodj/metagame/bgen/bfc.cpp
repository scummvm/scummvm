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

#include "bagel/hodjnpodj/metagame/bgen/bfc.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

static bool bHomeWriteLocked = false;
static bool bPathsDiffer = false;

void CBfcMgr::initBFCInfo() {
	static const char *pszTest;
	CClueTable *pClueTable;
	CHodjPodj *pPlayer;
	CItem *pItem;
	int i, j, k;

	freeBFCInfo();

	pPlayer = &m_cHodj;
	for (k = 0; k < 2; k++) {
		pPlayer->m_bComputer = false;
		pPlayer->m_bMoving = true;
		pPlayer->m_nTurns = 1;

		// Podj Specific stuff
		if (k == 1) {
			pPlayer = &m_cPodj;
			pPlayer->m_bComputer = true;
			pPlayer->m_bMoving = false;
			pPlayer->m_nTurns = 0;
		}

		pPlayer->m_iSkillLevel = SKILLLEVEL_LOW;

		pPlayer->m_bHaveMishMosh = false;

		pPlayer->m_pTradingPost = NULL;
		pPlayer->m_pBlackMarket = NULL;

		if ((pPlayer->m_pInventory = new CInventory(k == 0 ? "Hodj's Stuff" : "Podj's Stuff")) != NULL) {
			pPlayer->m_pInventory->AddItem(k == 0 ? MG_OBJ_HODJ_NOTEBOOK : MG_OBJ_PODJ_NOTEBOOK, 1);
			pItem = pPlayer->m_pInventory->FindItem(k == 0 ? MG_OBJ_HODJ_NOTEBOOK : MG_OBJ_PODJ_NOTEBOOK);
			(*pItem).SetActionCode(ITEM_ACTION_NOTEBOOK);
			pPlayer->m_pInventory->AddItem(MG_OBJ_CROWN, 20);
		}

		pszTest = "Corruption Test";

		if ((pPlayer->m_pGenStore = new CInventory("General Store")) != NULL) {
			for (i = MG_OBJ_BASE; i <= MG_OBJ_MAX; i++) {
				switch (i) {
				case MG_OBJ_HERRING:
				case MG_OBJ_MISH:
				case MG_OBJ_MOSH:
				case MG_OBJ_HODJ_NOTEBOOK:
				case MG_OBJ_PODJ_NOTEBOOK:
				case MG_OBJ_CROWN:
					break;
				default:
					pPlayer->m_pGenStore->AddItem(i, 1);
					break;
				}
			}

			if ((pPlayer->m_pBlackMarket = new CInventory("Black Market")) != NULL) {
				for (i = 0; i < ITEMS_IN_BLACK_MARKET; i++) {
					j = g_engine->getRandomNumber(pPlayer->m_pGenStore->ItemCount() - 1);
					pItem = pPlayer->m_pGenStore->FetchItem(j);
					pPlayer->m_pGenStore->RemoveItem(pItem);

					pPlayer->m_pBlackMarket->AddItem(pItem);
				}
			}
		}

		pClueTable = CMgStatic::cHodjClueTable;
		if (k == 1)
			pClueTable = CMgStatic::cPodjClueTable;

		for (i = 0; i < NUMBER_OF_CLUES; i++) {

			pPlayer->m_aClueArray[i].pNote = new CNote(-1,
				pClueTable[i].m_iCluePicCode - MG_CLUEPIC_BASE + NOTE_ICON_BASE,
				pClueTable[i].m_iCluePicCount,
				-1, -1);
			pPlayer->m_aClueArray[i].bUsed = false;
		}

		pPlayer->m_iFurlongs = 0;
	}

	m_bInventories = true;
	m_stGameStruct.lCrowns = 20;
	m_stGameStruct.lScore = 0;
	m_stGameStruct.nSkillLevel = SKILLLEVEL_MEDIUM;
	m_stGameStruct.bSoundEffectsEnabled = true;
	m_stGameStruct.bMusicEnabled = true;
	m_stGameStruct.bPlayingHodj = true;
	m_bRestart = false;

	m_bRestoredGame = true;

	m_stGameStruct.bPlayingMetagame = true;
	m_iGameTime = SHORT_GAME;
	m_bScrolling = g_engine->_bScrollingEnabled;
	m_bSlowCPU = g_engine->_bSlowCPU;
	m_bLowMemory = g_engine->_bLowMemory;
	m_bAnimations = (g_engine->_bSlowCPU || g_engine->_bLowMemory) ? false :
		g_engine->_bAnimationsEnabled;
	m_bChanged = false;
	m_dwFreeSpaceMargin = g_engine->_dwFreeSpaceMargin;
	m_dwFreePhysicalMargin = g_engine->_dwFreePhysicalMargin;

	if (bHomeWriteLocked)
		m_nInstallationCode = INSTALL_NONE;
	else
		m_nInstallationCode = (bPathsDiffer ? INSTALL_BASIC : INSTALL_MINIMAL);

	Common::strcpy_s(m_chHomePath, "");
	Common::strcpy_s(m_chCDPath, "");
	Common::strcpy_s(m_chMiniPath, "");

	// Create Mish/Mosh items
	assert(m_pMishItem == NULL);
	m_pMishItem = new CItem(MG_OBJ_MISH);

	assert(m_pMoshItem == NULL);
	m_pMoshItem = new CItem(MG_OBJ_MOSH);
}

void CBfcMgr::freeBFCInfo() {
	CHodjPodj *pPlayer;
	int i, k;

	m_bRestoredGame = false;

	// Delete any Mish/Mosh items
	if (m_pMishItem != NULL) {
		delete m_pMishItem;
		m_pMishItem = NULL;
	}
	if (m_pMoshItem != NULL) {
		delete m_pMoshItem;
		m_pMoshItem = NULL;
	}

	pPlayer = &m_cHodj;
	for (k = 0; k < 2; k++) {

		if (k == 1) {
			pPlayer = &m_cPodj;
		}

		if (pPlayer->m_pBlackMarket != NULL) {
			delete pPlayer->m_pBlackMarket;
			pPlayer->m_pBlackMarket = NULL;
		}

		if (pPlayer->m_pGenStore != NULL) {
			delete pPlayer->m_pGenStore;
			pPlayer->m_pGenStore = NULL;
		}

		if (pPlayer->m_pTradingPost != NULL) {
			delete pPlayer->m_pTradingPost;
			pPlayer->m_pTradingPost = NULL;
		}

		if (pPlayer->m_pInventory != NULL) {
			delete pPlayer->m_pInventory;
			pPlayer->m_pInventory = NULL;
		}

		// This deallocation MUST be after the delete pInventorys
		//
		for (i = 0; i < NUMBER_OF_CLUES; i++) {

			if (!pPlayer->m_aClueArray[i].bUsed) {

				if (pPlayer->m_aClueArray[i].pNote != NULL) {
					delete pPlayer->m_aClueArray[i].pNote;
					pPlayer->m_aClueArray[i].pNote = NULL;
				}
			}
		}
	}
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
