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
#include "chewy/rooms/room57.h"

namespace Chewy {
namespace Rooms {

void Room57::entry() {
	_G(zoom_horizont) = 180;
	flags.ZoomMov = true;
	_G(zoom_mov_fak) = 4;
	SetUpScreenFunc = setup_func;
	_G(spieler).ZoomXy[P_HOWARD][0] = 46;
	_G(spieler).ZoomXy[P_HOWARD][1] = 86;
	spieler_mi[P_HOWARD].Mode = true;
	if (_G(spieler).R57StudioAuf)
		det->hide_static_spr(4);
	
	if (!flags.LoadGame &&_G(spieler).R48TaxiEntry) {
		hide_cur();
		_G(spieler).R48TaxiEntry = false;
		_G(spieler).scrollx = 0;
		_G(spieler).scrolly = 0;
		set_person_pos(4, 144, P_HOWARD, P_LEFT);
		set_person_pos(40, 160, P_CHEWY, P_RIGHT);
		auto_move(2, P_CHEWY);
		_G(maus_links_click) = false;
		show_cur();
	}
}

void Room57::xit(int16 eib_nr) {
	if (_G(spieler).PersonRoomNr[P_HOWARD] == 57) {
		spieler_mi[P_HOWARD].Mode = false;
		if (eib_nr == 91)
			_G(spieler).PersonRoomNr[P_HOWARD] = 62;
	}
}

void Room57::setup_func() {
	if (_G(spieler).PersonRoomNr[P_HOWARD] == 57) {
		calc_person_look();
		const int16 ch_y = spieler_vector[P_CHEWY].Xypos[1];
		int16 x, y;
		if (ch_y < 145) {
			x = 176;
			y = 126;
		} else {
			x = 176;
			y = 142;
		}
		go_auto_xy(x, y, P_HOWARD, ANI_GO);
	}
}

int16 Room57::use_taxi() {
	int16 action_ret = false;
	if (!_G(spieler).inv_cur) {
		action_ret = true;
		hide_cur();
		auto_move(3, P_CHEWY);
		det->enable_sound(3, 0);
		det->play_sound(3, 0);
		det->show_static_spr(7);
		go_auto_xy(16, 160, P_CHEWY, ANI_WAIT);
		_G(spieler).PersonHide[P_CHEWY] = true;
		_G(spieler).R48TaxiPerson[P_CHEWY] = true;
		if (_G(spieler).PersonRoomNr[P_HOWARD] == 57) {
			SetUpScreenFunc = nullptr;
			go_auto_xy(11, 144, P_HOWARD, ANI_WAIT);
			_G(spieler).PersonHide[P_HOWARD] = true;
			_G(spieler).R48TaxiPerson[P_HOWARD] = true;
			_G(spieler).PersonRoomNr[P_HOWARD] = 48;
		}
		det->hide_static_spr(7);
		det->enable_sound(3, 1);
		det->play_sound(3, 1);
		room->set_timer_status(3, TIMER_STOP);
		det->del_static_ani(3);
		start_detail_wait(5, 1, ANI_VOR);
		det->disable_sound(5, 0);
		switch_room(48);
	}
	return action_ret;
}

int16 Room57::use_pfoertner() {
	int16 action_ret = false;
	room->set_timer_status(1, TIMER_STOP);
	det->del_static_ani(1);
	det->set_static_ani(3, -1);
	hide_cur();
	auto_move(1, P_CHEWY);
	if (is_cur_inventar(CUTMAG_INV)) {
		action_ret = true;
		if (_G(spieler).flags37_10)
			start_aad_wait(596, -1);
		else {
			_G(spieler).flags37_10 = true;
			start_aad_wait(339, -1);
			new_invent_2_cur(BESTELL_INV);
		}
	} else if (is_cur_inventar(JMKOST_INV)) {
		action_ret = true;
		start_aad_wait(340, -1);
	} else if (is_cur_inventar(EINLAD_INV)) {
		action_ret = true;
		SetUpScreenFunc = nullptr;
		go_auto_xy(132, 130, P_HOWARD, ANI_WAIT);
		if (_G(spieler).R56AbfahrtOk) {
			start_aad_wait(341, -1);
			go_auto_xy(176, 130, P_HOWARD, ANI_WAIT);
			del_inventar(_G(spieler).AkInvent);
			_G(spieler).R57StudioAuf = true;
			_G(spieler).room_e_obj[91].Attribut = AUSGANG_OBEN;
			det->hide_static_spr(4);
			start_detail_wait(6, 1, ANI_WAIT);
			det->disable_sound(6, 0);
			atds->set_steuer_bit(358, ATS_AKTIV_BIT, ATS_DATEI);
		} else {
			start_aad_wait(349, -1);
			go_auto_xy(176, 130, P_HOWARD, ANI_WAIT);
		}
		SetUpScreenFunc = setup_func;
	}
	show_cur();
	room->set_timer_status(1, TIMER_START);
	det->set_static_ani(1, -1);
	return action_ret;
}

void Room57::talk_pfoertner() {
	hide_cur();
	auto_move(1, P_CHEWY);
	room->set_timer_status(1, TIMER_STOP);
	det->del_static_ani(1);
	det->set_static_ani(3, -1);
	int16 aad_nr;
	if (!_G(spieler).R57StudioAuf) {
		aad_nr = 338;
	} else
		aad_nr = 342;
	start_aad_wait(aad_nr, -1);
	room->set_timer_status(1, TIMER_START);
	det->set_static_ani(1, -1);
	show_cur();
}

} // namespace Rooms
} // namespace Chewy
