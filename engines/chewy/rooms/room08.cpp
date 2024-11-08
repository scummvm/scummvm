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

#include "chewy/cursor.h"
#include "chewy/defines.h"
#include "chewy/events.h"
#include "chewy/globals.h"
#include "chewy/ani_dat.h"
#include "chewy/room.h"
#include "chewy/rooms/room08.h"
#include "chewy/dialogs/inventory.h"

namespace Chewy {
namespace Rooms {

static const AniBlock ABLOCK12[2] = {
	{ 8, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 9, 2, ANI_FRONT, ANI_GO, 0 },
};

static const AniBlock ABLOCK13[2] = {
	{10, 1, ANI_FRONT, ANI_WAIT, 0},
	{11, 255, ANI_FRONT, ANI_GO, 0},
};

void Room8::entry() {
	_G(gameState).R7ChewyFlug = true;
	if (!_G(gameState).R8Folter)
		start_folter();
	else
		stop_folter();
	_G(atds)->set_all_ats_str(60, 0, ATS_DATA);
}

void Room8::start_folter() {
	_G(atds)->set_all_ats_str(67, 1, ATS_DATA);
	_G(det)->stopDetail(19);
	_G(det)->startDetail(13, 255, ANI_FRONT);
}

void Room8::stop_folter() {
	_G(atds)->set_all_ats_str(67, 0, ATS_DATA);
	_G(det)->startDetail(19, 255, ANI_FRONT);

	_G(det)->stopDetail(13);

	_G(gameState).R8Folter = true;
	_G(obj)->hide_sib(SIB_FOLTER_R8);
}

void Room8::hole_kohle() {
	if (_G(gameState).R8Kohle) {
		startAadWait(604);
	} else {
		hideCur();
		_G(gameState).R8Kohle = true;
		autoMove(4, P_CHEWY);
		_G(gameState)._personHide[P_CHEWY] = true;
		startSetAILWait(12, 1, ANI_FRONT);
		cur_2_inventory();
		invent_2_slot(KOHLE_HEISS_INV);
		_G(gameState)._personHide[P_CHEWY] = false;
		showCur();
	}
}

void Room8::start_verbrennen() {
	hideCur();

	if (!_G(cur)->usingInventoryCursor()) {
		autoMove(3, P_CHEWY);
		start_aad(102, 0);
		_G(gameState)._personHide[P_CHEWY] = true;
		startAniBlock(2, ABLOCK12);

		while (_G(det)->get_ani_status(9)) {
			setupScreen(DO_SETUP);
			SHOULD_QUIT_RETURN;

			if (_G(minfo).button == 1 || g_events->_kbInfo._keyCode == Common::KEYCODE_RETURN) {
				if (g_events->_mousePos.x > 146 && g_events->_mousePos.x < 208 &&
					g_events->_mousePos.y > 107 && g_events->_mousePos.y < 155)
					break;
			}
		}

		_G(det)->stopDetail(9);
		setPersonPos(129, 246, P_CHEWY, P_RIGHT);
		startAniBlock(2, ABLOCK13);
		_G(atds)->set_ats_str(60, TXT_MARK_LOOK, 1, ATS_DATA);
		_G(gameState)._personHide[P_CHEWY] = false;
	}

	showCur();
}

bool Room8::gips_wurf() {
	bool actionFl = false;
	
	if (isCurInventory(GIPS_EIMER_INV)) {
		hideCur();
		actionFl = true;
		_G(det)->load_taf_seq(116, 30, nullptr);
		autoMove(2, P_CHEWY);
		_G(mouseLeftClick) = false;

		_G(gameState)._personHide[P_CHEWY] = true;
		delInventory(GIPS_EIMER_INV);
		startSetAILWait(4, 1, ANI_FRONT);
		_G(gameState)._personHide[P_CHEWY] = false;

		startDetailFrame(5, 1, ANI_FRONT, 16);
		startSetAILWait(6, 1, ANI_FRONT);
		_G(obj)->show_sib(33);
		_G(det)->showStaticSpr(14);
		waitDetail(5);
		_G(gameState).R8GipsWurf = true;
		_G(gameState).room_m_obj[MASKE_INV].ZEbene = 0;
		_G(obj)->setInventory(MASKE_INV, 181, 251, 8, &_G(room_blk));
		_G(det)->del_taf_tbl(116, 30, nullptr);
		autoMove(8, P_CHEWY);
		_G(flags).AtsAction = false;
		_G(menu_item) = CUR_USE;
		Dialogs::Inventory::look_screen(INVENTORY_NORMAL, 178);
		_G(flags).AtsAction = true;
		_G(gameState)._personHide[P_CHEWY] = true;
		startSetAILWait(20, 1, ANI_FRONT);
		_G(gameState)._personHide[P_CHEWY] = false;
		invent_2_slot(MASKE_INV);
		cursorChoice(_G(menu_item));
		showCur();
	}

	return actionFl;
}

void Room8::open_gdoor() {
	_G(gameState)._personHide[P_CHEWY] = true;
	_G(det)->showStaticSpr(17);
	setupScreen(DO_SETUP);
	startSetAILWait(7, 1, ANI_FRONT);
	_G(det)->showStaticSpr(15);
	_G(det)->hideStaticSpr(17);
	_G(gameState)._personHide[P_CHEWY] = false;
	setPersonPos(204, 274, P_CHEWY, P_LEFT);
	_G(atds)->delControlBit(69, ATS_ACTIVE_BIT);
	_G(obj)->hide_sib(31);
	_G(gameState).R8GTuer = true;
	_G(gameState).room_e_obj[15].Attribut = EXIT_BOTTOM;
}

void Room8::talk_nimoy() {
	autoMove(9, P_CHEWY);
	_G(flags).NoScroll = true;
	auto_scroll(0, 120);
	if (_G(gameState).R8Folter) {
		int16 diaNr = _G(gameState).R8GipsWurf ? 2 : 1;

		if (!_G(gameState).R8GTuer)
			loadDialogCloseup(diaNr);
		else
			startAadWait(61);
	} else {
		startAadWait(603);
		loadDialogCloseup(6);
	}

	_G(flags).NoScroll = false;
}

} // namespace Rooms
} // namespace Chewy
