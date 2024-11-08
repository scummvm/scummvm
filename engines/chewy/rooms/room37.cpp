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
#include "chewy/rooms/room37.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

static const AniBlock ABLOCK31[3] = {
	{ 0, 1, ANI_FRONT, ANI_GO, 0 },
	{ 1, 2, ANI_FRONT, ANI_GO, 0 },
	{ 5, 1, ANI_BACK, ANI_WAIT, 0 },
};


void Room37::entry() {
	_G(zoom_horizont) = 100;
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 3;
	_G(SetUpScreenFunc) = setup_func;
	_G(obj)->show_sib(74);
	_G(obj)->show_sib(75);

	if (!_G(flags).LoadGame) {
		_G(gameState).scrollx = 124;
		setPersonPos(219, 66, P_CHEWY, P_RIGHT);
	}

	if (!_G(gameState).R37RoosterFoughtWithDog) {
		_G(timer_nr)[1] = _G(room)->set_timer(7, 5);
		_G(det)->set_static_ani(7, -1);

		if (!_G(gameState).R37DogAshamed) {
			_G(timer_nr)[0] = _G(room)->set_timer(3, 4);
			_G(det)->set_static_ani(3, -1);
			_G(det)->playSound(3, 0);
		}
	}

	if (_G(gameState).R37TakenDenturesFromGlass) {
		_G(det)->hideStaticSpr(9);

		if (_G(gameState).R37RoosterFoughtWithDog) {
			_G(det)->hideStaticSpr(8);
		} else if (_G(gameState).R37DogAshamed) {
			_G(det)->showStaticSpr(0);
			_G(atds)->set_all_ats_str(250, 1, ATS_DATA);
			_G(atds)->set_all_ats_str(256, 1, ATS_DATA);
			_G(atds)->delControlBit(251, ATS_ACTIVE_BIT);
		}
	}
}

void Room37::gedAction(int index) {
	if (!index) {
		dog_bell();

	} else if (index == 1) {
		if (_G(gameState).R37RoosterFoughtWithDog && !_G(gameState).R37Mes) {
			stopPerson(P_CHEWY);
			_G(gameState).R37Mes = true;
			start_spz(CH_TALK6, 255, ANI_FRONT, P_CHEWY);
			startAadWait(142);
		}
	}
}

void Room37::setup_func() {
	if (_G(mouseLeftClick) && !_G(gameState).R37RoosterFoughtWithDog &&
			_G(menu_item) == CUR_WALK) {
		if ((g_events->_mousePos.x + _G(gameState).scrollx > 380 && g_events->_mousePos.y > 120) ||
			(g_events->_mousePos.x + _G(gameState).scrollx > 482)) {
			// Don't allow moving into chicken coop area
			// until the rooster has left
			autoMove(7, P_CHEWY);
			_G(mouseLeftClick) = false;
		}
	}
}

short Room37::useSeesaw() {
	if (_G(cur)->usingInventoryCursor()) {
		if (isCurInventory(H_FUTTER_INV)) {
			hideCur();
			autoMove(0, P_CHEWY);
			_G(flags).NoScroll = true;
			auto_scroll(129, 0);
			start_spz(CH_TALK6, 255, ANI_FRONT, P_CHEWY);
			startAadWait(159);
			delInventory(_G(cur)->getInventoryCursor());
			flic_cut(FCUT_047);
			_G(flags).NoScroll = false;
			showCur();
			_G(gameState).scrollx = 269;
			setPersonPos(388, 119, P_CHEWY, P_RIGHT);
			switchRoom(29);
			_G(mouseLeftClick) = false;

		} else {
			start_spz(CH_TALK5, 255, ANI_FRONT, P_CHEWY);
			startAadWait(160);
		}

		return true;
	} else {
		return false;
	}
}

int16 Room37::useGlass() {
	if (!_G(gameState).R37TakenDenturesFromGlass) {
		if (isCurInventory(FISHING_ROD_INV)) {
			_G(flags).NoScroll = true;
			hideCur();
			autoMove(5, P_CHEWY);
			_G(flags).NoScroll = true;
			auto_scroll(146, 0);
			start_spz(CH_TALK6, 255, ANI_FRONT, P_CHEWY);
			startAadWait(147);
			delInventory(_G(cur)->getInventoryCursor());
			flic_cut(FCUT_048);
			flic_cut(FCUT_049);
			invent_2_slot(DENTURES_INV);
			_G(det)->hideStaticSpr(9);
			_G(atds)->set_all_ats_str(250, 1, ATS_DATA);
			_G(atds)->set_all_ats_str(256, 1, ATS_DATA);
			_G(atds)->delControlBit(251, ATS_ACTIVE_BIT);
			_G(obj)->show_sib(SIB_HFUTTER2_R37);
			_G(gameState).R37TakenDenturesFromGlass = true;
			start_spz(CH_TALK3, 255, ANI_FRONT, P_CHEWY);
			startAadWait(146);
			showCur();
			_G(flags).NoScroll = false;
			_G(det)->playSound(3, 0);
		} else {
			autoMove(4, P_CHEWY);
		}

		return true;
	} else {
		return false;
	}
}

void Room37::dog_bell() {
	int16 dia_nr = -1;
	int16 ani_nr = 0;
	hideCur();

	if (!_G(flags).AutoAniPlay) {
		_G(flags).AutoAniPlay = true;
		_G(det)->stopSound(0); // nr 3, sslot 0

		if (!_G(gameState).R37TakenDenturesFromGlass) {
			stopPerson(P_CHEWY);
			_G(flags).ChAutoMov = false;
			setPersonSpr(P_LEFT, P_CHEWY);
			_G(flags).NoScroll = true;
			auto_scroll(178, 0);
			disable_timer();
			_G(det)->stopDetail(3);
			_G(det)->del_static_ani(3);
			startSetAILWait(5, 1, ANI_FRONT);
			_G(det)->hideStaticSpr(9);
			startSetAILWait(6, 1, ANI_FRONT);
			_G(gameState)._personHide[P_CHEWY] = true;
			_G(det)->startDetail(11, 255, ANI_FRONT);
			flic_cut(FCUT_050);
			startSetAILWait(6, 1, ANI_BACK);
			_G(det)->stopDetail(11);
			setPersonPos(326, 85, P_CHEWY, P_LEFT);
			_G(gameState)._personHide[P_CHEWY] = false;
			_G(det)->showStaticSpr(9);
			startAniBlock(3, ABLOCK31);
			_G(det)->set_static_ani(3, -1);
			_G(det)->playSound(3, 0);
//			_G(det)->playSound(3);
			enable_timer();
			dia_nr = 149;
			ani_nr = CH_TALK12;

		} else if (!_G(gameState).R37DogAshamed) {
			stopPerson(P_CHEWY);
			setPersonSpr(P_LEFT, P_CHEWY);
			_G(flags).NoScroll = true;
			auto_scroll(178, 0);
			_G(room)->set_timer_status(3, TIMER_STOP);
			_G(det)->del_static_ani(3);
			_G(det)->stopDetail(3);
			startSetAILWait(4, 1, ANI_FRONT);
			flic_cut(FCUT_051);
			_G(gameState).scrollx = 104;
			flic_cut(FCUT_054);
			register_cutscene(10);
			_G(det)->showStaticSpr(0);

			_G(gameState).R37DogAshamed = true;
			dia_nr = 148;
			ani_nr = CH_TALK6;
		}

		_G(flags).AutoAniPlay = false;

		if (dia_nr != -1) {
			start_spz(ani_nr, 255, ANI_FRONT, P_CHEWY);
			startAadWait(dia_nr);
		}
	}

	_G(flags).NoScroll = false;
	showCur();
}

void Room37::talkWithRooster() {
	hideCur();
	autoMove(7, P_CHEWY);
	showCur();

	if (!_G(gameState).R37UsedTranslatorOnRooster) {
		hideCur();
		startAadWait(145);
		showCur();
	} else {
		roosterDialog();
	}
}

void Room37::useRooster() {
	if (isCurInventory(TRANSLATOR_INV)) {
		hideCur();
		autoMove(7, P_CHEWY);
		_G(gameState).R37UsedTranslatorOnRooster = true;
		start_spz_wait(CH_TRANS, 1, false, P_CHEWY);
		flic_cut(FCUT_052);
		cur_2_inventory();
		_G(menu_item) = CUR_TALK;
		cursorChoice(_G(menu_item));
		showCur();
		roosterDialog();
	} else if (_G(gameState).R37UsedTranslatorOnRooster) {
		if (isCurInventory(DENTURES_INV)) {
			_G(gameState).R37RoosterFoughtWithDog = true;
			hideCur();
			autoMove(6, P_CHEWY);
			g_engine->_sound->playRoomMusic(256);
			_G(room)->set_timer_status(7, TIMER_STOP);
			_G(det)->stopDetail(7);
			_G(det)->del_static_ani(7);
			_G(det)->startDetail(9, 1, ANI_FRONT);
			start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
			delInventory(DENTURES_INV);
			_G(flags).NoScroll = true;
			auto_scroll(177, 0);

			while (_G(det)->get_ani_status(9)) {
				setupScreen(DO_SETUP);
				SHOULD_QUIT_RETURN;
			}

			_G(det)->startDetail(4, 1, ANI_FRONT);
			_G(det)->hideStaticSpr(0);
			_G(det)->startDetail(10, 10, ANI_FRONT);
			autoMove(8, P_CHEWY);
			flic_cut(FCUT_053);
			_G(det)->stopDetail(10);

			_G(gameState).scrollx = 320;
			_G(flags).NoScroll = false;
			_G(atds)->setControlBit(251, ATS_ACTIVE_BIT);
			_G(atds)->setControlBit(250, ATS_ACTIVE_BIT);
			_G(atds)->setControlBit(256, ATS_ACTIVE_BIT);
			_G(det)->hideStaticSpr(8);
			start_spz(CH_TALK5, 255, ANI_FRONT, P_CHEWY);
			startAadWait(141);
			_G(obj)->addInventory(EGGS_INV, &_G(room_blk));
			inventory_2_cur(EGGS_INV);
			showCur();
		}
	} else if (_G(cur)->usingInventoryCursor()) {
		startAadWait(143);
	}
}

void Room37::roosterDialog() {
	_G(gameState)._personHide[P_CHEWY] = true;
	int16 tmp_scrollx = _G(gameState).scrollx;
	int16 tmp_scrolly = _G(gameState).scrolly;
	_G(gameState).scrollx = 0;
	_G(gameState).scrolly = 0;
	switchRoom(38);
	startDialogCloseupWait(9);
	_G(gameState)._personHide[P_CHEWY] = false;
	_G(flags).LoadGame = true;
	_G(gameState).scrollx = tmp_scrollx;
	_G(gameState).scrolly = tmp_scrolly;
	switchRoom(37);
	_G(flags).LoadGame = false;
}

} // namespace Rooms
} // namespace Chewy
