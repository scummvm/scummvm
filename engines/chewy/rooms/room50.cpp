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
#include "chewy/room.h"
#include "chewy/rooms/room50.h"

namespace Chewy {
namespace Rooms {

static const AniBlock ABLOCK36[2] = {
	{ 6, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 7, 1, ANI_FRONT, ANI_WAIT, 0 },
};

bool Room50::_wasser;


void Room50::entry(int16 eib_nr) {
	_G(gameState).ScrollxStep = 2;

	if (_G(gameState).flags32_10) {
		hideCur();
		stop_page();
		_G(mouseLeftClick) = false;
		setPersonPos(1, 64, P_HOWARD, P_RIGHT);
		setPersonPos(92, 123, P_CHEWY, P_LEFT);
		startAadWait(510);
		_G(out)->setPointer(nullptr);
		_G(out)->cls();
		_G(flags).NoPalAfterFlc = true;
		flic_cut(FCUT_108);
		showCur();
		switchRoom(51);

	} else {
		if (_G(gameState).R50Zigarre) {
			stop_cigar();
		} else {
			_G(det)->startDetail(0, 255, ANI_FRONT);
		}

		if (!_G(gameState).R50KeyOK) {
			_G(timer_nr)[0] = _G(room)->set_timer(1, 8);
			_wasser = false;
		} else {
			_G(det)->showStaticSpr(4);
		}

		_G(SetUpScreenFunc) = setup_func;

		if (_G(gameState)._personRoomNr[P_HOWARD] == 50) {
			_G(spieler_mi)[P_HOWARD].Mode = true;

			if (!_G(flags).LoadGame) {
				if (eib_nr == 85)
					setPersonPos(298, 56, P_HOWARD, P_RIGHT);
				else
					setPersonPos(1, 64, P_HOWARD, P_RIGHT);
			}
		}
	}
}

void Room50::xit(int16 eib_nr) {
	_G(gameState).ScrollxStep = 1;

	if (_G(gameState)._personRoomNr[P_HOWARD] == 50) {
		if (eib_nr == 83)
			_G(gameState)._personRoomNr[P_HOWARD] = 49;
		else
			_G(gameState)._personRoomNr[P_HOWARD] = 51;

		_G(spieler_mi)[P_HOWARD].Mode = false;
	}
}

bool Room50::timer(int16 t_nr, int16 ani_nr) {
	if (t_nr == _G(timer_nr)[0])
		calc_wasser();

	return true;
}

void Room50::gedAction(int index) {
	if (!index)
		calc_treppe();
}

void Room50::stop_cigar() {
	_G(room)->set_timer_status(0, TIMER_STOP);
	_G(det)->del_static_ani(0);
	_G(atds)->set_all_ats_str(328, 1, ATS_DATA);
	_G(det)->stopDetail(0);
	_G(gameState).R50Zigarre = true;
}

void Room50::calc_wasser() {
	if (!_wasser) {
		stop_page();
		_G(det)->startDetail(4, 1, ANI_FRONT);
		_wasser = true;
	}
}

void Room50::stop_page() {
	_G(room)->set_timer_status(3, TIMER_STOP);
	_G(det)->del_static_ani(3);
	_G(det)->stopDetail(3);
	_G(det)->stopDetail(4);
}

void Room50::go_page() {
	_G(det)->set_static_ani(3, -1);
	_G(room)->set_timer_status(3, TIMER_START);
}

void Room50::calc_treppe() {
	if (!_G(flags).AutoAniPlay) {
		_G(flags).AutoAniPlay = true;

		if (!_G(gameState).R50KeyOK) {
			hideCur();
			stopPerson(P_CHEWY);
			_G(room)->set_timer_status(1, TIMER_STOP);
			_wasser = false;
			_G(flags).NoScroll = true;
			auto_scroll(40, 0);
			stop_page();
			_G(det)->set_static_ani(5, -1);
			startAadWait(272);
			autoMove(3, P_CHEWY);
			aad_page(273, 5);
			_G(flags).NoScroll = false;
		}

		_G(flags).AutoAniPlay = false;
	}
}

int16 Room50::use_gutschein() {
	int16 action_ret = false;

	if (isCurInventory(HOTEL_INV)) {
		action_ret = true;
		if (!_G(gameState).R50KeyOK) {
			hideCur();
			autoMove(3, P_CHEWY);
			_G(room)->set_timer_status(1, TIMER_STOP);
			_wasser = false;
			stop_page();
			delInventory(_G(cur)->getInventoryCursor());
			startAniBlock(2, ABLOCK36);
			aad_page(274, 8);
		} else {
			startAadWait(276);
		}
	}

	return action_ret;
}

int16 Room50::use_gum() {
	int16 action_ret = false;

	if (isCurInventory(GUM_INV)) {
		action_ret = true;
		hideCur();
		_G(gameState).R50KeyOK = true;
		_G(room)->set_timer_status(1, TIMER_STOP);
		_wasser = false;
		stop_page();
		startSetAILWait(6, 1, ANI_FRONT);
		_G(det)->set_static_ani(5, -1);
		autoMove(3, P_CHEWY);
		_G(spieler_mi)[P_CHEWY].Mode = true;
		goAutoXy(75, 92, P_CHEWY, ANI_WAIT);
		_G(SetUpScreenFunc) = nullptr;
		goAutoXy(112, 57, P_HOWARD, ANI_WAIT);

		setPersonSpr(P_LEFT, P_HOWARD);
		delInventory(_G(cur)->getInventoryCursor());
		hide_person();
		startSetAILWait(2, 1, ANI_FRONT);
		_G(det)->showStaticSpr(4);
		startSetAILWait(2, 1, ANI_BACK);
		show_person();
		setPersonSpr(P_LEFT, P_CHEWY);
		startAadWait(275);
		_G(det)->del_static_ani(5);
		startSetAILWait(9, 1, ANI_FRONT);
		_G(det)->del_static_ani(5);
		_G(det)->set_static_ani(10, -1);
		startAadWait(277);
		_G(SetUpScreenFunc) = setup_func;
		autoMove(3, P_CHEWY);
		_G(spieler_mi)[P_CHEWY].Mode = true;
		_G(det)->del_static_ani(10);
		go_page();

		_G(obj)->addInventory(KEY_INV, &_G(room_blk));
		inventory_2_cur(KEY_INV);
		_G(atds)->set_all_ats_str(323, 1, ATS_DATA);
		_G(atds)->set_all_ats_str(327, 1, ATS_DATA);
		_G(gameState).room_e_obj[84].Attribut = EXIT_TOP;
		showCur();
	}

	return action_ret;
}

void Room50::talk_page() {
	startAadWait(276);
}

void Room50::aad_page(int16 aad_nr, int16 ani_nr) {
	_G(det)->set_static_ani(ani_nr, -1);
	startAadWait(aad_nr);
	_G(det)->del_static_ani(ani_nr);
	startSetAILWait(6, 1, ANI_BACK);
	go_page();

	if (!_G(gameState).R50KeyOK)
		_G(room)->set_timer_status(1, TIMER_START);

	showCur();
}

void Room50::setup_func() {
	if (_wasser && !_G(det)->get_ani_status(4)) {
		_wasser = false;
		go_page();
	}

	if (_G(gameState)._personRoomNr[P_HOWARD] == 50) {
		calc_person_look();
		const int16 ch_x = _G(moveState)[P_CHEWY].Xypos[0];

		int16 x, y;
		if (ch_x < 72) {
			x = 1;
			y = 64;
		} else if (ch_x < 275 && _G(moveState)[P_CHEWY].Xypos[1] > 72) {
			x = 143;
			y = 57;
		} else {
			x = 162;
			y = 115;
		}

		if (_G(HowardMov) && _G(flags).ExitMov) {
			_G(SetUpScreenFunc) = nullptr;
			_G(HowardMov) = 0;
			autoMove(4, P_HOWARD);
		} else {
			goAutoXy(x, y, P_HOWARD, ANI_GO);
		}
	}
}

} // namespace Rooms
} // namespace Chewy
