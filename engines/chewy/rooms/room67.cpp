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
#include "chewy/global.h"
#include "chewy/room.h"
#include "chewy/rooms/room67.h"

namespace Chewy {
namespace Rooms {

void Room67::entry() {
	_G(spieler).ScrollxStep = 2;
	SetUpScreenFunc = setup_func;
	spieler_mi[P_HOWARD].Mode = true;
	spieler_mi[P_NICHELLE].Mode = true;
	_G(spieler).DiaAMov = 7;
	if (_G(spieler).R67KommodeAuf)
		det->show_static_spr(9);
	if (!_G(spieler).R67PapageiWeg) {
		_G(timer_nr)[0] = room->set_timer(1, 10);
		det->set_static_ani(1, -1);
	} else
		det->show_static_spr(0);
	if (!flags.LoadGame) {
		hide_cur();
		set_person_pos(102, 132, P_CHEWY, P_RIGHT);
		if (_G(spieler).PersonRoomNr[P_HOWARD] == 67) {
			set_person_pos(12, 100, P_HOWARD, P_RIGHT);
			set_person_pos(47, 106, P_NICHELLE, P_RIGHT);
			go_auto_xy(214, 112, P_NICHELLE, ANI_GO);
		}
		auto_move(7, P_CHEWY);
		show_cur();
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
	if (spieler_vector[P_CHEWY].Xypos[0] > 320) {
		x = 178;
	} else {
		x = 137;
	}
	go_auto_xy(x, y, P_HOWARD, ANI_GO);
	go_auto_xy(214, 112, P_NICHELLE, ANI_GO);
}

int16 Room67::use_grammo() {
	int16 action_flag = false;
	if (is_cur_inventar(SCHALL_INV)) {
		hide_cur();
		action_flag = true;
		auto_move(6, P_CHEWY);
		start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
		det->start_detail(0, 255, ANI_VOR);
		if (_G(spieler).PersonRoomNr[P_HOWARD] == 67)
			start_aad_wait(376, -1);
		else
			start_aad_wait(614, -1);

		start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
		det->stop_detail(0);
		show_cur();
	}
	return action_flag;
}

void Room67::look_brief() {
	hide_cur();
	auto_move(3, P_CHEWY);
	start_aad_wait(379, -1);
	show_cur();
}

int16 Room67::use_kommode() {
	int16 action_flag = false;
	if (!_G(spieler).inv_cur) {
		hide_cur();
		if (!_G(spieler).R67KommodeAuf) {
			action_flag = true;
			_G(spieler).R67KommodeAuf = true;
			auto_move(6, P_CHEWY);
			start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
			det->show_static_spr(9);
			atds->set_ats_str(400, 1, ATS_DATEI);
		} else if (!_G(spieler).R67KostuemWeg) {
			action_flag = true;
			_G(spieler).R67KostuemWeg = true;
			auto_move(6, P_CHEWY);
			start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
			atds->set_ats_str(400, 2, ATS_DATEI);
			new_invent_2_cur(GALA_INV);
		}
		show_cur();
	}
	return action_flag;
}

void Room67::kostuem_aad(int16 aad_nr) {
	hide_cur();
	if (_G(spieler).DiaAMov != -1) {
		auto_move(_G(spieler).DiaAMov, P_CHEWY);
	}
	start_aad_wait(aad_nr, -1);
	show_cur();
}

int16 Room67::talk_papagei() {
	int16 action_flag = false;
	if (!_G(spieler).R67PapageiWeg && !_G(spieler).inv_cur) {
		action_flag = true;
		hide_cur();
		room->set_timer_status(1, TIMER_STOP);
		if (_G(menu_item) == CUR_HOWARD) {
			show_cur();

			ssi[2].X = 270 - _G(spieler).scrollx;
			ssi[2].Y = 10;
			atds->set_split_win(2, &ssi[2]);
			start_ads_wait(19);
			room->set_timer_status(1, TIMER_START);
		} else if (_G(menu_item) == CUR_NICHELLE) {
			start_aad_wait(380, -1);
			room->set_timer_status(1, TIMER_START);
		} else if (_G(menu_item) == CUR_TALK) {
			auto_move(5, P_CHEWY);
			show_cur();

			ssi[2].X = 270 - _G(spieler).scrollx;
			ssi[2].Y = 10;
			atds->set_split_win(2, &ssi[2]);
			start_ads_wait(18);
			room->set_timer_status(1, TIMER_START);
		} else if (_G(menu_item) == CUR_USE) {
			hide_cur();
			auto_move(4, P_CHEWY);
			start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
			_G(spieler).R67PapageiWeg = true;
			det->stop_detail(1);
			det->del_static_ani(1);
			det->show_static_spr(0);
			invent_2_slot(PAPAGEI_INV);
			show_cur();
			atds->set_steuer_bit(394, ATS_AKTIV_BIT, ATS_DATEI);
		}
		show_cur();
	}
	return action_flag;
}

} // namespace Rooms
} // namespace Chewy
