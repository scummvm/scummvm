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
#include "chewy/rooms/room46.h"

namespace Chewy {
namespace Rooms {

void Room46::entry(int16 eib_nr) {
	if (_G(spieler).PersonRoomNr[P_HOWARD] == 45) {
		_G(spieler).PersonRoomNr[P_HOWARD] = 46;
		SetUpScreenFunc = setup_func;

		if (eib_nr == 79) {
			set_person_pos(140, 90, P_HOWARD, P_LEFT);
			auto_move(3, P_CHEWY);
			set_person_spr(P_LEFT, P_CHEWY);
		}
	}

	if (_G(spieler).PersonRoomNr[P_HOWARD] == 46) {
		spieler_mi[P_HOWARD].Mode = true;
		SetUpScreenFunc = setup_func;

		if (_G(spieler).flags33_8) {
			_G(maus_links_click) = false;
			hide_cur();
			set_person_pos(175, 73, P_HOWARD, P_LEFT);
			set_person_pos(147, 109, P_CHEWY, P_LEFT);
			_G(spieler).flags33_8 = false;
			_G(spieler).SVal1 = 46;
			wait_show_screen(10);

			start_spz_wait(64, 1, false, P_CHEWY);
			start_spz(65, 255, false, P_CHEWY);
			start_aad_wait(494, -1);
			_G(spieler).SVal2 = 504;
			switch_room(92);
			start_spz(65, 255, false, P_CHEWY);
			start_aad_wait(495, -1);
			_G(spieler).room_e_obj[78].Exit = 90;
			show_cur();
		}
	}

	if (_G(spieler).R47SchlossOk && !_G(spieler).R46GetLeder)
		bodo();
}

void Room46::xit() {
	if (_G(spieler).PersonRoomNr[P_HOWARD] == 46) {
		spieler_mi[P_HOWARD].Mode = false;
	}
}

void Room46::setup_func() {
	calc_person_look();
	int16 y = 64;
	const int16 ch_x = spieler_vector[P_CHEWY].Xypos[0];

	int16 x;
	if (ch_x > 160) {
		x = 260;
	} else {
		x = 120;
	}

	if (HowardMov && flags.ExitMov) {
		x = 160;
		y = 200;
	}

	go_auto_xy(x, y, P_HOWARD, ANI_GO);
}

void Room46::bodo() {
	hide_cur();
	det->show_static_spr(0);
	person_end_phase[P_CHEWY] = P_RIGHT;
	start_aad_wait(240, -1);
	spieler_mi[P_CHEWY].Mode = true;
	go_auto_xy(42, 76, P_CHEWY, ANI_WAIT);
	start_aad_wait(250, -1);
	auto_move(1, P_CHEWY);
	set_person_spr(P_RIGHT, P_CHEWY);
	start_aad_wait(251, -1);
	auto_move(2, P_CHEWY);

	spieler_mi[P_CHEWY].Mode = false;
	start_detail_wait(0, 1, ANI_VOR);
	det->start_detail(5, 255, ANI_VOR);
	start_aad_wait(241, -1);
	det->stop_detail(5);
	det->show_static_spr(5);
	start_aad_wait(609, -1);
	det->hide_static_spr(5);
	start_detail_wait(2, 1, ANI_VOR);
	start_detail_wait(3, 1, ANI_VOR);
	det->start_detail(4, 255, ANI_VOR);
	start_aad_wait(242, -1);
	det->stop_detail(4);

	det->show_static_spr(6);
	start_aad_wait(610, -1);
	det->hide_static_spr(6);
	start_detail_wait(6, 1, ANI_VOR);
	det->show_static_spr(3);
	_G(spieler).PersonHide[P_CHEWY] = true;
	start_detail_wait(0, 1, ANI_VOR);
	flic_cut(FCUT_065, CFO_MODE);
	out->setze_zeiger(nullptr);
	out->cls();
	_G(spieler).PersonHide[P_CHEWY] = false;

	det->hide_static_spr(0);
	det->hide_static_spr(3);
	load_chewy_taf(CHEWY_ROCKER);
	set_person_pos(213, 118, P_CHEWY, P_RIGHT);
	start_aad_wait(243, -1);
	invent_2_slot(UHR_INV);
	invent_2_slot(RING_INV);
	_G(spieler).R46GetLeder = true;
	show_cur();
}

void Room46::kloppe() {
	for (int16 i = 0; i < 4; i++) {
		mem->file->select_pool_item(Ci.Handle, FCUT_065);

		flc->play(Ci.Handle, Ci.VirtScreen, Ci.TempArea);
		out->setze_zeiger(nullptr);
		out->cls();
		start_aad(244 + i, -1);
		int16 delay = _G(spieler).DelaySpeed * 50;
		atds->print_aad(0, 0);

		while (in->get_switch_code() == 0 && delay) {
			out->skip_frame(1);
			--delay;
			SHOULD_QUIT_RETURN;
		}
	}

	mem->file->select_pool_item(Ci.Handle, 66);
	flc->custom_play(&Ci);
	ailsnd->fade_out(0);
	out->cls();

	while (ailsnd->music_playing() && !SHOULD_QUIT) {
		EVENTS_UPDATE;
	}
}

int16 Room46::use_schloss() {
	int16 action_ret = false;

	if (!_G(spieler).inv_cur) {
		action_ret = true;

		if (!_G(spieler).R46GetLeder) {
			auto_move(1, P_CHEWY);
			switch_room(47);
		} else {
			start_aad_wait(252, -1);
		}
	}

	return action_ret;
}

} // namespace Rooms
} // namespace Chewy
