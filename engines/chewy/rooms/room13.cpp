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
#include "chewy/rooms/room13.h"

namespace Chewy {
namespace Rooms {

#define R13_BORK_OBJ 0

static const int16 R13_BORK_PHASEN[4][2] = {
	{ 92, 97 },
	{ 86, 91 },
	{ 86, 91 },
	{ 68, 73 }
};

static const MovLine R13_BORK_MPKT[3] = {
	{ { 168, 140, 162 }, 0, 6 },
	{ {  50, 140, 162 }, 0, 6 },
	{ {  50, 107, 130 }, 2, 6 }
};

void Room13::entry() {
	if (!_G(gameState).R12ChewyBork && !_G(gameState).R13BorkOk) {
		_G(out)->cls();
		flic_cut(FCUT_012);
		setPersonPos(106, 65, P_CHEWY, P_RIGHT);
		switchRoom(11);
		start_spz(CH_TALK12, 255, false, P_CHEWY);
		startAadWait(27);
	} else {
		if (_G(gameState).R13MonitorStatus)
			_G(det)->showStaticSpr(11 - _G(gameState).R13MonitorStatus);

		if (!_G(gameState).R13BorkOk) {
			_G(det)->showStaticSpr(12);
			_G(obj)->hide_sib(SIB_BANDBUTTON_R13);
			_G(gameState).R13Bandlauf = true;

			_G(atds)->set_ats_str(94, TXT_MARK_LOOK, _G(gameState).R13Bandlauf, ATS_DATA);	// conveyor belt
			_G(atds)->set_ats_str(97, TXT_MARK_LOOK, _G(gameState).R13Bandlauf, ATS_DATA);	// lever
			_G(atds)->set_ats_str(93, TXT_MARK_LOOK, _G(gameState).R13Bandlauf, ATS_DATA);	// monitor
		} else {
			_G(atds)->setControlBit(122, ATS_ACTIVE_BIT);
			_G(atds)->delControlBit(92, ATS_ACTIVE_BIT);
			_G(obj)->show_sib(SIB_BANDBUTTON_R13);
		}

		if (_G(gameState).R13Bandlauf) {
			for (int i = 0; i < 5; ++i)
				_G(det)->startDetail(i, 255, false);
		}
		
		if (!_G(flags).LoadGame && _G(gameState).R13Band) {
			_G(gameState).room_e_obj[25].Attribut = EXIT_TOP;
			_G(atds)->setControlBit(100, ATS_ACTIVE_BIT);
			_G(gameState).R13Band = false;
		}

		if (_G(gameState).R21GarbageGrid)
			_G(det)->hideStaticSpr(6);
	}
}

void Room13::xit() {
	_G(gameState).room_e_obj[25].Attribut = EXIT_TOP;
	_G(atds)->setControlBit(100, ATS_ACTIVE_BIT);
	_G(gameState).R13Band = false;
}

void Room13::gedAction(int index) {
	if (index == 2 && _G(gameState).R12ChewyBork) {
		stopPerson(P_CHEWY);
		talk_bork();
	}
}

void Room13::talk_bork() {
	if (!_G(gameState).R13BorkOk) {
		_G(gameState).R13BorkOk = true;
		_G(gameState).R12ChewyBork = false;
		_G(det)->showStaticSpr(13);
		_G(det)->setDetailPos(10, _G(moveState)[P_CHEWY].Xypos[0], _G(moveState)[P_CHEWY].Xypos[1]);
		_G(det)->setStaticPos(12, _G(moveState)[P_CHEWY].Xypos[0], _G(moveState)[P_CHEWY].Xypos[1], false, true);
		_G(gameState)._personHide[P_CHEWY] = true;
		startAadWait(33);
		_G(det)->stopDetail(9);
		_G(det)->load_taf_seq(86, 12, nullptr);
		_G(gameState)._personHide[P_CHEWY] = false;
		_G(det)->hideStaticSpr(12);
		_G(auto_obj) = 1;

		_G(mov_phasen)[R13_BORK_OBJ].AtsText = 122;
		_G(mov_phasen)[R13_BORK_OBJ].Lines = 3;
		_G(mov_phasen)[R13_BORK_OBJ].Repeat = 1;
		_G(mov_phasen)[R13_BORK_OBJ].ZoomFak = 0;
		_G(auto_mov_obj)[R13_BORK_OBJ].Id = AUTO_OBJ0;
		_G(auto_mov_vector)[R13_BORK_OBJ].Delay = _G(gameState).DelaySpeed;
		_G(auto_mov_obj)[R13_BORK_OBJ].Mode = true;
		init_auto_obj(R13_BORK_OBJ, &R13_BORK_PHASEN[0][0], 3, (const MovLine *)R13_BORK_MPKT);
		autoMove(9, P_CHEWY);
		wait_auto_obj(R13_BORK_OBJ);
		autoMove(11, P_CHEWY);
		_G(flags).NoScroll = true;
		auto_scroll(41, 0);

		start_spz(68, 255, false, P_CHEWY);
		startAadWait(248);
		flic_cut(FCUT_013);
		load_chewy_taf(CHEWY_NORMAL);
		_G(auto_obj) = 0;
		_G(flags).NoScroll = false;

		_G(atds)->setControlBit(122, ATS_ACTIVE_BIT);
		_G(atds)->delControlBit(92, ATS_ACTIVE_BIT);
		_G(obj)->show_sib(SIB_BANDBUTTON_R13);
		setPersonPos(153, 138, P_CHEWY, P_LEFT);

		startAadWait(34);
		startAadWait(249);

		_G(obj)->show_sib(SIB_TBUTTON1_R11);
		_G(obj)->show_sib(SIB_SCHLITZ_R11);
		_G(obj)->show_sib(SIB_TBUTTON2_R11);
		_G(obj)->show_sib(SIB_TBUTTON3_R11);

		if (_G(gameState).R6DoorRightB)
			_G(gameState).room_e_obj[20].Attribut = EXIT_LEFT;
		_G(gameState).room_e_obj[21].Attribut = EXIT_TOP;
	}
}

void Room13::jmp_band() {
	if (!_G(gameState).R13Band && !_G(gameState).R12ChewyBork) {
		if (!_G(gameState).R13Bandlauf) {
			_G(obj)->hide_sib(SIB_BANDBUTTON_R13);
			_G(gameState).room_e_obj[25].Attribut = 255;
			_G(atds)->delControlBit(100, ATS_ACTIVE_BIT);
			_G(gameState).R13Band = true;
			autoMove(3, P_CHEWY);
			_G(gameState)._personHide[P_CHEWY] = true;
			startSetAILWait(8, 1, ANI_FRONT);
			_G(gameState)._personHide[P_CHEWY] = false;
			setPersonPos(292, 98, P_CHEWY, P_RIGHT);

		} else {
			_G(gameState).R13Surf = true;
			_G(mouseLeftClick) = false;
			autoMove(12, P_CHEWY);
			startAadWait(117);
			_G(flags).NoScroll = true;
			auto_scroll(76, 0);
			flic_cut(FCUT_014);
			_G(flags).NoScroll = false;
			setPersonPos(195, 226, P_CHEWY, P_LEFT);

			_G(gameState).R13Bandlauf = false;
			_G(atds)->set_ats_str(94, TXT_MARK_LOOK, _G(gameState).R13Bandlauf, ATS_DATA);	// conveyor belt
			_G(atds)->set_ats_str(97, TXT_MARK_LOOK, _G(gameState).R13Bandlauf, ATS_DATA);	// lever
			_G(atds)->set_ats_str(93, TXT_MARK_LOOK, _G(gameState).R13Bandlauf, ATS_DATA);	// monitor
			_G(obj)->calc_rsi_flip_flop(SIB_BANDBUTTON_R13);
			_G(obj)->hide_sib(SIB_BANDBUTTON_R13);
			switchRoom(14);

			flic_cut(FCUT_017);
			register_cutscene(6);
			_G(gameState).scrollx = 92;
			_G(gameState).scrolly = 120;
			_G(gameState)._personHide[P_CHEWY] = true;
			waitShowScreen(20);
			_G(gameState)._personHide[P_CHEWY] = false;
		}
	}
}

void Room13::jmp_floor() {
	if (_G(gameState).R13Band) {
		if (!_G(gameState).R13Surf)
			_G(obj)->show_sib(SIB_BANDBUTTON_R13);

		_G(gameState).room_e_obj[25].Attribut = EXIT_TOP;
		_G(atds)->setControlBit(100, ATS_ACTIVE_BIT);
		_G(gameState).R13Band = false;
		autoMove(5, P_CHEWY);
		_G(gameState)._personHide[P_CHEWY] = true;
		startSetAILWait(7, 1, ANI_FRONT);
		_G(gameState)._personHide[P_CHEWY] = false;
		setPersonPos(176, 138, P_CHEWY, P_LEFT);
	}
}

int16 Room13::monitor_button() {
	int16 action_flag = false;
	
	if (!_G(cur)->usingInventoryCursor()) {
		if (_G(gameState).R13Band) {
			action_flag = true;
			startAadWait(620);

		} else if (!_G(gameState).R12ChewyBork) {
			action_flag = true;
			autoMove(8, P_CHEWY);
			_G(gameState)._personHide[P_CHEWY] = true;
			startSetAILWait(6, 1, ANI_FRONT);
			_G(gameState)._personHide[P_CHEWY] = false;

			if (_G(gameState).R13MonitorStatus)
				_G(det)->hideStaticSpr(11 - _G(gameState).R13MonitorStatus);

			++_G(gameState).R13MonitorStatus;
			if (_G(gameState).R13MonitorStatus > 4)
				_G(gameState).R13MonitorStatus = 0;
			else
				_G(det)->showStaticSpr(11 - _G(gameState).R13MonitorStatus);

			_G(atds)->set_ats_str(96, TXT_MARK_LOOK, _G(gameState).R13MonitorStatus, ATS_DATA);
		}
	}

	return action_flag;
}

} // namespace Rooms
} // namespace Chewy
