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
#include "chewy/rooms/room42.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room42::entry() {
	if (!_G(gameState).R42StationEmployeeAway) {
		_G(det)->playSound(0, 0);
		_G(det)->startDetail(0, 255, ANI_FRONT);
	}

	if (_G(gameState)._personRoomNr[P_HOWARD] == 42) {
		_G(SetUpScreenFunc) = setup_func;

		if (!_G(flags).LoadGame) {
			_G(det)->stopDetail(0);
			_G(timer_nr)[0] = _G(room)->set_timer(8, 5);
			_G(det)->set_static_ani(8, -1);
			_G(gameState).R42StationEmployeeAway = true;
			_G(det)->stopSound(0);

			_G(SetUpScreenFunc) = setup_func;

			setPersonPos(80, 43, P_HOWARD, P_LEFT);
			_G(atds)->set_all_ats_str(263, 1, ATS_DATA);
			_G(atds)->set_all_ats_str(264, 1, ATS_DATA);
		}

		if (_G(obj)->checkInventory(HOTEL_INV) && _G(obj)->checkInventory(TICKET_INV) && !_G(gameState).R42LetterOk)
			startAadWait(302);

		if (_G(obj)->checkInventory(HOTEL_INV) && _G(obj)->checkInventory(TICKET_INV) && _G(gameState).R42LetterOk)
			startAadWait(301);
	}
}

void Room42::xit() {
	if (_G(gameState)._personRoomNr[P_HOWARD] == 42) {
		_G(atds)->set_all_ats_str(264, 1, ATS_DATA);
		stop_spz();
		_G(gameState).R42HoToBeamter = false;
		_G(gameState)._personRoomNr[P_HOWARD] = 40;
	}
}

void Room42::setup_func() {
	if (!_G(gameState).R42HoToBeamter)
		calc_person_look();
}

void Room42::getPumpkin(int16 aad_nr) {
	hideCur();
	autoMove(1, P_CHEWY);
	_G(gameState)._personHide[P_CHEWY] = true;
	startSetAILWait(7, 1, ANI_BACK);
	_G(gameState)._personHide[P_CHEWY] = false;
	start_spz(CH_TALK5, 255, ANI_FRONT, P_CHEWY);
	startAadWait(aad_nr);

	_G(gameState)._personHide[P_CHEWY] = true;
	startSetAILWait(7, 1, ANI_FRONT);
	_G(gameState)._personHide[P_CHEWY] = false;
	showCur();
}

int16 Room42::useMailBag() {
	int16 action_flag = false;

	if (_G(menu_item) == CUR_HOWARD)
		return action_flag;

	hideCur();
	if (!_G(gameState).R42StationEmployeeAway && !_G(cur)->usingInventoryCursor()) {
		action_flag = true;
		getPumpkin(136);
	} else if (_G(gameState).R42HoToBeamter && !_G(cur)->usingInventoryCursor() && !_G(gameState).R42StampOk) {
		action_flag = true;
		autoMove(3, P_CHEWY);
		_G(gameState)._personHide[P_CHEWY] = true;
		startSetAILWait(10, 1, ANI_FRONT);
		_G(gameState)._personHide[P_CHEWY] = false;
		startAadWait(187);
		_G(gameState)._personHide[P_CHEWY] = true;
		startSetAILWait(9, 1, ANI_FRONT);
		_G(gameState)._personHide[P_CHEWY] = false;
		new_invent_2_cur(BMARKE_INV);
		startAadWait(181);
		_G(gameState).R42StampOk = true;
		autoMove(4, P_CHEWY);
		startAadWait(185);
		_G(gameState).R42HoToBeamter = false;
	} else if (isCurInventory(STAMPEDLETTER_INV)) {
		action_flag = true;
		autoMove(3, P_CHEWY);
		_G(gameState)._personHide[P_CHEWY] = true;
		startSetAILWait(10, 1, ANI_FRONT);
		_G(gameState)._personHide[P_CHEWY] = false;
		delInventory(_G(cur)->getInventoryCursor());
		startAadWait(183);
		_G(obj)->calc_rsi_flip_flop(SIB_BKASTEN_R28);
		_G(atds)->set_all_ats_str(206, 1, ATS_DATA);
		_G(gameState).R28LetterBox = true;
		_G(gameState).R40TrainMove = true;
		_G(gameState).R28PostCar = true;
		_G(gameState).R42LetterOk = true;
	} else if (isCurInventory(LETTER_INV)) {
		action_flag = true;
		startAadWait(182);
	}

	showCur();
	return action_flag;
}

int16 Room42::useStationEmployee() {
	int16 action_flag = false;

	if (_G(menu_item) == CUR_HOWARD) {
		action_flag = true;
		_G(atds)->set_all_ats_str(264, 2, ATS_DATA);
		hideCur();
		autoMove(4, P_CHEWY);
		startAadWait(184);
		_G(gameState).R42HoToBeamter = true;
		_G(menu_item) = CUR_WALK;
		cursorChoice(_G(menu_item));
		showCur();
		start_spz(HO_TALK_L, 255, ANI_FRONT, P_HOWARD);
		_G(flags).MouseLeft = false;
	}

	return action_flag;
}

void Room42::talkToStationEmployee() {
	int16 dia_nr;
	autoMove(1, P_CHEWY);

	if (!_G(gameState).R42StationEmployeeAway) {
		dia_nr = 10;
	} else if (!_G(gameState).R42StampOk) {
		dia_nr = 13;
	} else {
		dia_nr = 14;
	}

	startDialogCloseupWait(dia_nr);
}

void Room42::dialogWithStationEmployee(int16 str_end_nr) {
	hideCur();

	if (str_end_nr != 4) {
		while (_G(atds)->aadGetStatus() != -1) {
			setupScreen(DO_SETUP);
			SHOULD_QUIT_RETURN;
		}

		_G(gameState)._personHide[P_CHEWY] = true;
		_G(flags).NoDiaBox = true;

		switch (str_end_nr) {
		case 1:
			startSetAILWait(3, 4, ANI_FRONT);
			break;

		case 2:
			_G(det)->playSound(4, 0);
			startSetAILWait(4, 13, ANI_FRONT);
			break;

		case 3:
			startSetAILWait(5, 2, ANI_FRONT);
			break;

		default:
			break;
		}
	} else {
		while (_G(atds)->aadGetStatus() < 2) {
			setupScreen(DO_SETUP);
			SHOULD_QUIT_RETURN;
		}

		_G(det)->stopDetail(0);
		_G(det)->stopSound(0);
		startSetAILWait(1, 1, ANI_FRONT);
		_G(det)->startDetail(2, 255, ANI_FRONT);

		while (_G(atds)->aadGetStatus() != -1) {
			setupScreen(DO_SETUP);
			SHOULD_QUIT_RETURN;
		}

		_G(det)->stopDetail(2);
		_G(det)->startDetail(0, 255, ANI_FRONT);
		_G(det)->playSound(0, 0);
		_G(det)->playSound(0, 0);
	}

	_G(gameState)._personHide[P_CHEWY] = true;
	_G(det)->startDetail(6, 255, ANI_FRONT);
	startAadWait(135);
	_G(det)->stopDetail(6);
	_G(flags).NoDiaBox = false;
	_G(gameState)._personHide[P_CHEWY] = false;
	showCur();
}

} // namespace Rooms
} // namespace Chewy
