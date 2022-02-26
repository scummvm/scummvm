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

#include "chewy/defines.h"
#include "chewy/events.h"
#include "chewy/globals.h"
#include "chewy/room.h"
#include "chewy/rooms/room67.h"

namespace Chewy {
namespace Rooms {

void Room67::entry() {
	_G(spieler).ScrollxStep = 2;
	_G(SetUpScreenFunc) = setup_func;
	_G(spieler_mi)[P_HOWARD].Mode = true;
	_G(spieler_mi)[P_NICHELLE].Mode = true;
	_G(spieler).DiaAMov = 7;
	if (_G(spieler).R67KommodeAuf)
		_G(det)->show_static_spr(9);
	if (!_G(spieler).R67PapageiWeg) {
		_G(timer_nr)[0] = _G(room)->set_timer(1, 10);
		_G(det)->set_static_ani(1, -1);
	} else
		_G(det)->show_static_spr(0);
	if (!_G(flags).LoadGame) {
		hideCur();
		set_person_pos(102, 132, P_CHEWY, P_RIGHT);
		if (_G(spieler).PersonRoomNr[P_HOWARD] == 67) {
			set_person_pos(12, 100, P_HOWARD, P_RIGHT);
			set_person_pos(47, 106, P_NICHELLE, P_RIGHT);
			goAutoXy(214, 112, P_NICHELLE, ANI_GO);
		}
		autoMove(7, P_CHEWY);
		showCur();
	}
}

void Room67::xit() {
	_G(spieler).ScrollxStep = 1;

	if (_G(spieler).PersonRoomNr[P_HOWARD] == 67) {
		_G(spieler).PersonRoomNr[P_HOWARD] = 66;
		_G(spieler).PersonRoomNr[P_NICHELLE] = 66;
	}
}

void Room67::setup_func() {
	calc_person_look();
	const int16 y = 117;
	int16 x;
	if (_G(spieler_vector)[P_CHEWY].Xypos[0] > 320) {
		x = 178;
	} else {
		x = 137;
	}
	goAutoXy(x, y, P_HOWARD, ANI_GO);
	goAutoXy(214, 112, P_NICHELLE, ANI_GO);
}

int16 Room67::use_grammo() {
	int16 action_flag = false;
	if (is_cur_inventar(SCHALL_INV)) {
		hideCur();
		action_flag = true;
		autoMove(6, P_CHEWY);
		start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
		_G(det)->start_detail(0, 255, ANI_FRONT);
		if (_G(spieler).PersonRoomNr[P_HOWARD] == 67)
			start_aad_wait(376, -1);
		else
			start_aad_wait(614, -1);

		start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
		_G(det)->stop_detail(0);
		showCur();
	}
	return action_flag;
}

void Room67::look_brief() {
	hideCur();
	autoMove(3, P_CHEWY);
	start_aad_wait(379, -1);
	showCur();
}

int16 Room67::use_kommode() {
	int16 action_flag = false;
	if (!_G(spieler).inv_cur) {
		hideCur();
		if (!_G(spieler).R67KommodeAuf) {
			action_flag = true;
			_G(spieler).R67KommodeAuf = true;
			autoMove(6, P_CHEWY);
			start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
			_G(det)->show_static_spr(9);
			_G(atds)->set_ats_str(400, 1, ATS_DATEI);
		} else if (!_G(spieler).R67KostuemWeg) {
			action_flag = true;
			_G(spieler).R67KostuemWeg = true;
			autoMove(6, P_CHEWY);
			start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
			_G(atds)->set_ats_str(400, 2, ATS_DATEI);
			new_invent_2_cur(GALA_INV);
		}
		showCur();
	}
	return action_flag;
}

void Room67::kostuem_aad(int16 aad_nr) {
	hideCur();
	if (_G(spieler).DiaAMov != -1) {
		autoMove(_G(spieler).DiaAMov, P_CHEWY);
	}
	start_aad_wait(aad_nr, -1);
	showCur();
}

int16 Room67::talk_papagei() {
	int16 action_flag = false;
	if (!_G(spieler).R67PapageiWeg && !_G(spieler).inv_cur) {
		action_flag = true;
		hideCur();
		_G(room)->set_timer_status(1, TIMER_STOP);
		if (_G(menu_item) == CUR_HOWARD) {
			showCur();

			_G(ssi)[2].X = 270 - _G(spieler).scrollx;
			_G(ssi)[2].Y = 10;
			_G(atds)->set_split_win(2, &_G(ssi)[2]);
			start_ads_wait(19);
			_G(room)->set_timer_status(1, TIMER_START);
		} else if (_G(menu_item) == CUR_NICHELLE) {
			start_aad_wait(380, -1);
			_G(room)->set_timer_status(1, TIMER_START);
		} else if (_G(menu_item) == CUR_TALK) {
			autoMove(5, P_CHEWY);
			showCur();

			_G(ssi)[2].X = 270 - _G(spieler).scrollx;
			_G(ssi)[2].Y = 10;
			_G(atds)->set_split_win(2, &_G(ssi)[2]);
			start_ads_wait(18);
			_G(room)->set_timer_status(1, TIMER_START);
		} else if (_G(menu_item) == CUR_USE) {
			hideCur();
			autoMove(4, P_CHEWY);
			start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
			_G(spieler).R67PapageiWeg = true;
			_G(det)->stop_detail(1);
			_G(det)->del_static_ani(1);
			_G(det)->show_static_spr(0);
			invent_2_slot(PAPAGEI_INV);
			showCur();
			_G(atds)->set_steuer_bit(394, ATS_AKTIV_BIT, ATS_DATEI);
		}
		showCur();
	}
	return action_flag;
}

} // namespace Rooms
} // namespace Chewy
