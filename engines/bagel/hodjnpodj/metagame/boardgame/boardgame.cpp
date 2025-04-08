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
#include "bagel/hodjnpodj/views/rules.h"
#include "bagel/hodjnpodj/hodjnpodj.h"
#include "bagel/metaengine.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

Boardgame::Boardgame() : View("Boardgame"),
		_doc(&_app) {
}

bool Boardgame::msgOpen(const OpenMessage &msg) {

	return true;
}

bool Boardgame::msgClose(const CloseMessage &msg) {
	return true;
}

bool Boardgame::msgAction(const ActionMessage &msg) {
	if (!isInputAllowed())
		return false;

	return false;
}

bool Boardgame::msgKeypress(const KeypressMessage &msg) {
	if (!isInputAllowed())
		return false;

	switch (msg.keycode) {
	case Common::KEYCODE_F1:
		Rules::show("meta/metarule.txt", nullptr);
		break;

	case Common::KEYCODE_F2:
		addView("BoardgameOptions");
		break;

	case Common::KEYCODE_m:
		addView("Minimap");
		break;

	case Common::KEYCODE_i:
		showInventory(kDialogInventory);
		break;

	default:
		return false;
	}

	return true;
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
	case kDialogBlackMarket:
		// Black market
		GeneralStore::show(pPlayer->m_pBlackMarket,
			pPlayer->m_pInventory);
		break;

	case kDialogPawnShop:
		// Pawn shop
		PawnShop::show(getRandomNumber(1) == 1 ?
				pPlayer->m_pGenStore : pPlayer->m_pBlackMarket,
			pPlayer->m_pInventory);
		break;

	case kDialogGeneralStore:
		// General store
		GeneralStore::show(pPlayer->m_pGenStore,
			pPlayer->m_pInventory);
		break;

	case kDialogInventory:
	default:
		Backpack::show(pPlayer->m_pInventory);
		break;
	}
}

bool Boardgame::isInputAllowed() const {
	return true;
}

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
