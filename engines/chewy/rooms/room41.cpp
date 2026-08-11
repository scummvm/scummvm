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
#include "chewy/rooms/room41.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

static const AniBlock ABLOCK32[2] = {
	{ 6, 255, ANI_FRONT, ANI_GO, 0 },
	{ 2, 1, ANI_FRONT, ANI_WAIT, 0 },
};


void Room41::entry() {
	hideCur();

	if (!_G(gameState).R41LolaOk) {
		_G(det)->playSound(6, 0);

		if (!_G(flags).LoadGame) {
			_G(room)->set_timer_status(0, TIMER_STOP);
			_G(det)->del_static_ani(0);
			startAniBlock(2, ABLOCK32);
			_G(room)->set_timer_status(0, TIMER_START);
			_G(det)->set_static_ani(0, -1);
			startAadWait(127);
		} else {
			_G(det)->startDetail(6, 255, ANI_FRONT);
		}
	}

	if (_G(gameState).R41Einbruch) {
		_G(atds)->delControlBit(271, ATS_ACTIVE_BIT);
		_G(det)->showStaticSpr(6);
		_G(det)->showStaticSpr(7);
	}

	if (_G(gameState).ChewyAni == CHEWY_ROCKER) {
		_G(atds)->setControlBit(269, ATS_ACTIVE_BIT);
	}

	if (!_G(flags).LoadGame)
		setPersonPos(241, 113, P_HOWARD, P_RIGHT);

	_G(SetUpScreenFunc) = setup_func;
	showCur();
}

void Room41::xit() {
	if (_G(gameState).R41TrainCount == 0) {
		_G(gameState).R41TrainCount = 3;
		_G(gameState).R40TrainMove = true;
	}

	_G(gameState).R41TrainCount--;
}

void Room41::setup_func() {
	if (_G(gameState)._personRoomNr[P_HOWARD] == 41) {
		calc_person_look();
		if (_G(moveState)->Xypos[P_HOWARD] == 160) {
			goAutoXy(258, 75, P_HOWARD, ANI_GO);
		} else {
			goAutoXy(246, 120, P_HOWARD, ANI_GO);
		}
	}
}

void Room41::talk_hoggy1() {
	stop_hoggy();
	autoMove(1, P_CHEWY);

	if (!_G(gameState).R41FirstTalk) {
		first_talk();

	} else if (!_G(gameState).R41Einbruch) {
		showCur();
		startDialogCloseupWait(11);

	} else if (_G(gameState).R41Einbruch) {
		if (!_G(gameState).R41BruchInfo) {
			_G(gameState).R41BruchInfo = true;
			startAadWait(132);
			startAadWait(128);
		} else if (_G(gameState).R31SurFurz && !_G(gameState).R41KuerbisInfo) {
			_G(gameState).R41KuerbisInfo = true;
			startAadWait(131);
			autoMove(5, P_CHEWY);
			new_invent_2_cur(TICKET_INV);
		} else {
			startAadWait(130);
		}
	} else {
		startAadWait(130);
	}

	start_hoggy();
}

void Room41::talk_hoggy2() {
	stop_hoggy();
	autoMove(2, P_CHEWY);

	if (!_G(gameState).R41FirstTalk) {
		first_talk();

	} else if (_G(gameState).R41BruchInfo) {
		if (_G(gameState).R31SurFurz && !_G(gameState).R41KuerbisInfo) {
			_G(gameState).R41KuerbisInfo = true;
			startAadWait(131);
			autoMove(5, P_CHEWY);
			new_invent_2_cur(TICKET_INV);

		} else {
			startAadWait(129);
		}
	} else {
		startAadWait(129);
	}

	start_hoggy();
}

void Room41::first_talk() {
	_G(gameState).R41FirstTalk = true;
	startAadWait(134);
	_G(atds)->set_ats_str(266, TXT_MARK_NAME, 1, ATS_DATA);
	_G(atds)->set_ats_str(265, TXT_MARK_NAME, 1, ATS_DATA);
}

void Room41::start_hoggy() {
	showCur();

	for (int16 i = 0; i < 2; i++) {
		_G(room)->set_timer_status(i, TIMER_START);
		_G(det)->set_static_ani(i, -1);
		_G(det)->del_static_ani(i + 3);
	}
}

void Room41::stop_hoggy() {
	hideCur();

	for (int16 i = 0; i < 2; i++) {
		_G(room)->set_timer_status(i, TIMER_STOP);
		_G(det)->del_static_ani(i);
		_G(det)->stopDetail(i);
		_G(det)->set_static_ani(i + 3, -1);
	}
}

int16 Room41::use_kasse() {
	int16 action_flag = false;

	if (!_G(cur)->usingInventoryCursor()) {
		action_flag = true;
		stop_hoggy();
		autoMove(1, P_CHEWY);
		startAadWait(133);
		start_hoggy();
	}

	return action_flag;
}

int16 Room41::use_lola() {
	int16 action_flag = false;

	if (!_G(cur)->usingInventoryCursor() && !_G(gameState).R41LolaOk && _G(gameState).R41RepairInfo) {
		hideCur();
		action_flag = true;
		_G(gameState).R41LolaOk = true;
		autoMove(4, P_CHEWY);
		_G(det)->stopSound(0);
		flic_cut(FCUT_057);
		setPersonPos(127, 112, P_CHEWY, P_LEFT);
		_G(det)->stopDetail(6);

		_G(atds)->delControlBit(267, ATS_ACTIVE_BIT);
		_G(atds)->set_all_ats_str(267, 1, ATS_DATA);
		_G(atds)->hideDialogCloseupItem(11, 0, 3);
		showCur();
	}

	return action_flag;
}

int16 Room41::use_brief() {
	int16 action_flag = false;
	hideCur();

	if (isCurInventory(LETTER_INV)) {
		action_flag = true;
		stop_hoggy();
		autoMove(6, P_CHEWY);
		startAadWait(126);
		start_hoggy();

	} else if (isCurInventory(STAMPEDLETTER_INV)) {
		action_flag = true;
		autoMove(6, P_CHEWY);
		delInventory(_G(cur)->getInventoryCursor());
		stop_hoggy();
		startAadWait(186);
		start_hoggy();
		_G(atds)->set_all_ats_str(206, 1, ATS_DATA);
		_G(gameState).R28LetterBox = true;
		_G(gameState).R40TrainMove = true;
		_G(gameState).R28PostCar = true;
		_G(gameState).R42LetterOk = true;
	}

	showCur();
	return action_flag;
}

void Room41::sub_dia() {
	aadWait(-1);
	startAadWait(161);

	if (_G(gameState).R41LolaOk) {
		startAadWait(163);
		_G(atds)->hideDialogCloseupItem(11, 0, 2);
		stopDialogCloseupDialog();
		autoMove(5, P_CHEWY);
		new_invent_2_cur(PAPIER_INV);

	} else {
		startAadWait(162);
	}
}

} // namespace Rooms
} // namespace Chewy
