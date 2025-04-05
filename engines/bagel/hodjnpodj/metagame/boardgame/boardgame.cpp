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

#include "bagel/hodjnpodj/metagame/boardgame/boardgame.h"
#include "bagel/hodjnpodj/metagame/boardgame/backpack.h"
#include "bagel/hodjnpodj/metagame/boardgame/general_store.h"
#include "bagel/hodjnpodj/metagame/boardgame/notebook.h"
#include "bagel/hodjnpodj/metagame/boardgame/pawn_shop.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/metaengine.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

static const char *m_szFilename = "meta/meta.gtl";


Boardgame::Boardgame() : View("Boardgame") {
}

bool Boardgame::msgOpen(const OpenMessage &msg) {
	clearVars();

	return true;
}

bool Boardgame::msgClose(const CloseMessage &msg) {
	return true;
}

bool Boardgame::msgAction(const ActionMessage &msg) {
	return false;
}

bool Boardgame::msgGame(const GameMessage &msg) {
	return false;
}

void Boardgame::draw() {
	GfxSurface s = getSurface();
	s.clear();
}

void Boardgame::showClue(CNote *note) {
	Notebook::show(NULL, note);
}

void Boardgame::showInventory(int nWhichDlg) {
	CHodjPodj *pPlayer;

	// which player
	pPlayer = &lpMetaGame->m_cPodj;
	if (lpMetaGame->m_cHodj.m_bMoving)
		pPlayer = &lpMetaGame->m_cHodj;

	switch (nWhichDlg) {
	case 4:
		// Black market
		GeneralStore::show(pPlayer->m_pBlackMarket,
			pPlayer->m_pInventory);
		break;

	case 3:
		// Pawn shop
		PawnShop::show(getRandomNumber(1) == 1 ?
				pPlayer->m_pGenStore : pPlayer->m_pBlackMarket,
			pPlayer->m_pInventory);
		break;

	case 2:
		// General store
		GeneralStore::show(pPlayer->m_pGenStore,
			pPlayer->m_pInventory);
		break;

	case 1:
	default:
		Backpack::show(pPlayer->m_pInventory);
		break;
	}
}

/*------------------------------------------------------------------------*/

void BoardgameVars::clearVars() {
	m_szIniFilename[0] = '\0';
	m_szIniSectionname[0] = '\0';
	m_bDebug = false;
	m_bDebugMessages = false;
	m_bTimeMessage = false;
	m_bTrack = m_bTrace = false;
	m_bTraceError = false;
	m_iConstructorMsgLevel = 0;
	m_bVerifyDc = false;
	Common::fill(m_iDebugValues, m_iDebugValues + 100, 0);
	m_iTraceObjectCount = 0;
	m_lpTraceObjects = nullptr;
	m_iTraceObjectCurrent = 0;
	m_iErrorCount = 0;
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
