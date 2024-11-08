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
#include "chewy/rooms/room67.h"

namespace Chewy {
namespace Rooms {

void Room67::entry() {
	_G(gameState).ScrollxStep = 2;
	_G(SetUpScreenFunc) = setup_func;
	_G(spieler_mi)[P_HOWARD].Mode = true;
	_G(spieler_mi)[P_NICHELLE].Mode = true;
	_G(gameState).DiaAMov = 7;
	if (_G(gameState).R67KommodeAuf)
		_G(det)->showStaticSpr(9);
	if (!_G(gameState).R67PapageiWeg) {
		_G(timer_nr)[0] = _G(room)->set_timer(1, 10);
		_G(det)->set_static_ani(1, -1);
	} else
		_G(det)->showStaticSpr(0);
	if (!_G(flags).LoadGame) {
		hideCur();
		setPersonPos(102, 132, P_CHEWY, P_RIGHT);
		if (_G(gameState)._personRoomNr[P_HOWARD] == 67) {
			setPersonPos(12, 100, P_HOWARD, P_RIGHT);
			setPersonPos(47, 106, P_NICHELLE, P_RIGHT);
			goAutoXy(214, 112, P_NICHELLE, ANI_GO);
		}
		autoMove(7, P_CHEWY);
		showCur();
	}
}

void Room67::xit() {
	_G(gameState).ScrollxStep = 1;

	if (_G(gameState)._personRoomNr[P_HOWARD] == 67) {
		_G(gameState)._personRoomNr[P_HOWARD] = 66;
		_G(gameState)._personRoomNr[P_NICHELLE] = 66;
	}
}

void Room67::setup_func() {
	calc_person_look();
	const int16 y = 117;
	int16 x;
	if (_G(moveState)[P_CHEWY].Xypos[0] > 320) {
		x = 178;
	} else {
		x = 137;
	}
	goAutoXy(x, y, P_HOWARD, ANI_GO);
	goAutoXy(214, 112, P_NICHELLE, ANI_GO);
}

int16 Room67::use_grammo() {
	int16 action_flag = false;
	if (isCurInventory(SCHALL_INV)) {
		hideCur();
		action_flag = true;
		autoMove(6, P_CHEWY);
		start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
		_G(det)->startDetail(0, 255, ANI_FRONT);
		if (_G(gameState)._personRoomNr[P_HOWARD] == 67)
			startAadWait(376);
		else
			startAadWait(614);

		start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
		_G(det)->stopDetail(0);
		showCur();
	}
	return action_flag;
}

void Room67::look_brief() {
	hideCur();
	autoMove(3, P_CHEWY);
	startAadWait(379);
	showCur();
}

int16 Room67::use_kommode() {
	int16 action_flag = false;
	if (!_G(cur)->usingInventoryCursor()) {
		hideCur();
		if (!_G(gameState).R67KommodeAuf) {
			action_flag = true;
			_G(gameState).R67KommodeAuf = true;
			autoMove(6, P_CHEWY);
			start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
			_G(det)->showStaticSpr(9);
			_G(atds)->set_all_ats_str(400, 1, ATS_DATA);
		} else if (!_G(gameState).R67KostuemWeg) {
			action_flag = true;
			_G(gameState).R67KostuemWeg = true;
			autoMove(6, P_CHEWY);
			start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
			_G(atds)->set_all_ats_str(400, 2, ATS_DATA);
			new_invent_2_cur(GALA_INV);
		}
		showCur();
	}
	return action_flag;
}

void Room67::kostuem_aad(int16 aad_nr) {
	hideCur();
	if (_G(gameState).DiaAMov != -1) {
		autoMove(_G(gameState).DiaAMov, P_CHEWY);
	}
	startAadWait(aad_nr);
	showCur();
}

int16 Room67::talk_papagei() {
	int16 action_flag = false;
	if (!_G(gameState).R67PapageiWeg && !_G(cur)->usingInventoryCursor()) {
		action_flag = true;
		hideCur();
		_G(room)->set_timer_status(1, TIMER_STOP);
		if (_G(menu_item) == CUR_HOWARD) {
			showCur();

			_G(atds)->set_split_win(2, 270 - _G(gameState).scrollx, 10);
			startDialogCloseupWait(19);
			_G(room)->set_timer_status(1, TIMER_START);
		} else if (_G(menu_item) == CUR_NICHELLE) {
			startAadWait(380);
			_G(room)->set_timer_status(1, TIMER_START);
		} else if (_G(menu_item) == CUR_TALK) {
			autoMove(5, P_CHEWY);
			showCur();

			_G(atds)->set_split_win(2, 270 - _G(gameState).scrollx, 10);
			startDialogCloseupWait(18);
			_G(room)->set_timer_status(1, TIMER_START);
		} else if (_G(menu_item) == CUR_USE) {
			hideCur();
			autoMove(4, P_CHEWY);
			start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
			_G(gameState).R67PapageiWeg = true;
			_G(det)->stopDetail(1);
			_G(det)->del_static_ani(1);
			_G(det)->showStaticSpr(0);
			invent_2_slot(PARROT_INV);
			showCur();
			_G(atds)->setControlBit(394, ATS_ACTIVE_BIT);
		}
		showCur();
	}
	return action_flag;
}

} // namespace Rooms
} // namespace Chewy
