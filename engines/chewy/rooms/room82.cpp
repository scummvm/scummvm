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

#include "chewy/rooms/room82.h"
#include "chewy/rooms/room66.h"

namespace Chewy {
namespace Rooms {

void Room82::entry() {
	det->enable_sound(0, 0);
	det->play_sound(0, 0);
	_G(spieler).ScrollxStep = 2;
	spieler_mi[P_HOWARD].Mode = true;
	spieler_mi[P_NICHELLE].Mode = true;

	if (_G(spieler).flags31_1) {
		det->show_static_spr(6);
		atds->set_steuer_bit(473, ATS_AKTIV_BIT, ATS_DATEI);
		atds->set_steuer_bit(467, ATS_AKTIV_BIT, ATS_DATEI);
	}

	if (_G(spieler).flags37_20)
		det->show_static_spr(7);

	if (flags.LoadGame) {
		SetUpScreenFunc = setup_func;
		return;
	}

	if (_G(spieler).flags30_1) {
		set_person_pos(635, 144, P_CHEWY, P_LEFT);
		set_person_pos(592, 115, P_HOWARD, P_RIGHT);
		set_person_pos(543, 11, P_NICHELLE, P_RIGHT);
		_G(spieler).flags30_1 = false;
		_G(spieler).scrollx = 479;
	} else if (_G(spieler).flags31_8) {
		set_person_pos(130, 138, P_CHEWY, P_LEFT);
		set_person_pos(104, 111, P_HOWARD, P_RIGHT);
		set_person_pos(153, 110, P_NICHELLE, P_RIGHT);
		_G(spieler).flags31_8 = false;
		_G(spieler).scrollx = 0;
	}

	if (_G(spieler).flags30_8) {
		SetUpScreenFunc = setup_func;
		proc4();
	}

	_G(maus_links_click) = false;
	SetUpScreenFunc = setup_func;
}

void Room82::xit(int16 eib_nr) {
	_G(spieler).ScrollxStep = 1;

	if (eib_nr == 122) {
		_G(spieler).R79Val[P_CHEWY] = 1;
		_G(spieler).R79Val[P_HOWARD] = 1;
		_G(spieler).R79Val[P_NICHELLE] = 1;
		_G(spieler).PersonRoomNr[P_HOWARD] = 80;
		_G(spieler).PersonRoomNr[P_NICHELLE] = 80;
	}
}

void Room82::setup_func() {
	calc_person_look();

	const int posX = spieler_vector[P_CHEWY].Xypos[0];
	int howDestX, nicDestX;
	
	if (posX < 200) {
		howDestX = 104;
		nicDestX = 150;
	} else if (posX < 387) {
		howDestX = 280;
		nicDestX = 379;
	} else {
		howDestX = 592;
		nicDestX = 543;
	}

	go_auto_xy(howDestX, 111, P_HOWARD, ANI_GO);
	go_auto_xy(nicDestX, 110, P_NICHELLE, ANI_GO);
}

void Room82::talk1() {
	if (_G(spieler).flags30_40)
		return;

	int transitionDiaNr, transitionAniNr;
	if (!_G(spieler).flags30_10) {
		transitionDiaNr = 446;
		transitionAniNr = 8;
	} else if (!_G(spieler).flags30_20) {
		transitionDiaNr = 454;
		transitionAniNr = 7;
	} else {
		transitionDiaNr = 449;
		transitionAniNr = 7;

		_G(spieler).flags30_40 = true;
		atds->set_ats_str(473, 1, ATS_DATEI);
		atds->set_ats_str(467, 1, ATS_DATEI);		
	}

	Room66::proc8(2, 7, transitionAniNr, transitionDiaNr);
}

void Room82::talk2() {
	hide_cur();
	auto_move(3, P_CHEWY);
	start_aad_wait(453, -1);
	show_cur();
}

void Room82::talk3() {
	Room66::proc8(4, 0, 1, 452);
}

int Room82::proc3() {
	if (!is_cur_inventar(101))
		return 0;

	hide_cur();
	auto_move(4, P_CHEWY);
	start_spz_wait(13, 1, false, P_CHEWY);
	room->set_timer_status(0, TIMER_STOP);
	det->del_static_ani(0);
	start_detail_wait(2, 1, ANI_VOR);
	start_detail_wait(3, 1, ANI_VOR);
	det->set_static_ani(0, -1);
	start_spz_wait(13, 1, false, P_CHEWY);

	del_inventar(_G(spieler).AkInvent);
	new_invent_2_cur(104);
	show_cur();

	return 1;
}

void Room82::proc4() {
	hide_cur();
	_G(spieler).flags30_8 = true;
	auto_move(5, P_CHEWY);
	flags.NoScroll = true;
	auto_scroll(270, 0);
	room->set_timer_status(7, TIMER_STOP);
	det->del_static_ani(7);
	det->set_static_ani(8, -1);
	room->set_timer_status(4, TIMER_STOP);
	det->del_static_ani(4);
	det->set_static_ani(5, 3);
	start_aad_wait(445, -1);
	det->del_static_ani(5);
	det->set_static_ani(4, -1);
	room->set_timer_status(4, TIMER_START);
	det->del_static_ani(8);
	det->set_static_ani(7, -1);
	room->set_timer_status(7, TIMER_START);
	flags.NoScroll = false;

	show_cur();
}

int Room82::proc6() {
	if (!is_cur_inventar(105) && !is_cur_inventar(106))
		return 0;

	hide_cur();
	auto_move(2, P_CHEWY);

	if (_G(spieler).flags30_10) {
		start_aad_wait(450, -1);
		out->ausblenden(0);
		out->set_teilpalette(pal, 255, 1);
		atds->set_unknown(0);
		start_aad_wait(598, -1);
		atds->set_unknown(1);
		det->show_static_spr(7);
		fx_blend = BLEND3;
		set_up_screen(DO_SETUP);
		Room66::proc8(2, 7, 7, 451);
		_G(spieler).flags30_20 = true;
		_G(spieler).flags37_20 = true;
		del_inventar(_G(spieler).AkInvent);
		remove_inventory(105);
		remove_inventory(106);
	} else {
		start_spz(16, 255, false, P_CHEWY);
		start_aad_wait(276, -1);
	}

	show_cur();
	return 1;
}

void Room82::proc8() {
	hide_cur();
	_G(spieler).flags30_10 = true;
	start_aad_wait(447, -1);
	auto_move(6, P_CHEWY);

	while (spieler_vector[P_NICHELLE].Count != 0)
		set_up_screen(DO_SETUP);

	_G(spieler).PersonHide[P_NICHELLE] = true;
	det->start_detail(10, 255, false);
	start_aad_wait(625, -1);
	det->stop_detail(10);
	_G(spieler).PersonHide[P_NICHELLE] = false;
	start_aad_wait(448, -1);
	det->del_static_ani(4);
	start_detail_wait(6, 2, ANI_VOR);
	det->set_static_ani(4, -1);
	show_cur();
}

int Room82::proc9() {
	if (_G(spieler).inv_cur || !_G(spieler).flags30_40)
		return 0;

	auto_move(7, P_CHEWY);
	_G(spieler).PersonRoomNr[P_HOWARD] = 88;
	_G(spieler).PersonRoomNr[P_NICHELLE] = 88;
	switch_room(88);
	menu_item = CUR_WALK;
	cursor_wahl(CUR_WALK);

	return 1;
}

} // namespace Rooms
} // namespace Chewy
