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
#include "chewy/ani_dat.h"
#include "chewy/room.h"
#include "chewy/rooms/room50.h"

namespace Chewy {
namespace Rooms {

static const AniBlock ABLOCK36[2] = {
	{ 6, 1, ANI_VOR, ANI_WAIT, 0 },
	{ 7, 1, ANI_VOR, ANI_WAIT, 0 },
};

bool Room50::_wasser;


void Room50::entry(int16 eib_nr) {
	_G(spieler).ScrollxStep = 2;

	if (_G(spieler).flags32_10) {
		hide_cur();
		stop_page();
		_G(maus_links_click) = 0;
		set_person_pos(0, 64, 0, 1);
		set_person_pos(92, 123, 0, 0);
		start_aad_wait(510, -1);
		out->setze_zeiger(nullptr);
		out->cls();
		flags.NoPalAfterFlc = true;
		flic_cut(108, 0);
		show_cur();
		switch_room(51);

	} else {
		if (_G(spieler).R50Zigarre) {
			stop_cigar();
		} else {
			det->start_detail(0, 255, ANI_VOR);
		}

		if (!_G(spieler).R50KeyOK) {
			_G(timer_nr)[0] = room->set_timer(1, 8);
			_wasser = false;
		} else {
			det->show_static_spr(4);
		}

		SetUpScreenFunc = setup_func;

		if (_G(spieler).PersonRoomNr[P_HOWARD] == 50) {
			spieler_mi[P_HOWARD].Mode = true;

			if (!flags.LoadGame) {
				if (eib_nr == 85)
					set_person_pos(298, 56, P_HOWARD, P_RIGHT);
				else
					set_person_pos(1, 64, P_HOWARD, P_RIGHT);
			}
		}
	}
}

void Room50::xit(int16 eib_nr) {
	_G(spieler).ScrollxStep = 1;

	if (_G(spieler).PersonRoomNr[P_HOWARD] == 50) {
		if (eib_nr == 83)
			_G(spieler).PersonRoomNr[P_HOWARD] = 49;
		else
			_G(spieler).PersonRoomNr[P_HOWARD] = 51;

		spieler_mi[P_HOWARD].Mode = false;
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
	room->set_timer_status(0, TIMER_STOP);
	det->del_static_ani(0);
	atds->set_ats_str(328, 1, ATS_DATEI);
	det->stop_detail(0);
	_G(spieler).R50Zigarre = true;
}

void Room50::calc_wasser() {
	if (!_wasser) {
		stop_page();
		det->start_detail(4, 1, ANI_VOR);
		_wasser = true;
	}
}

void Room50::stop_page() {
	room->set_timer_status(3, TIMER_STOP);
	det->del_static_ani(3);
	det->stop_detail(3);
	det->stop_detail(4);
}

void Room50::go_page() {
	det->set_static_ani(3, -1);
	room->set_timer_status(3, TIMER_START);
}

void Room50::calc_treppe() {
	if (!flags.AutoAniPlay) {
		flags.AutoAniPlay = true;

		if (!_G(spieler).R50KeyOK) {
			hide_cur();
			stop_person(P_CHEWY);
			room->set_timer_status(1, TIMER_STOP);
			_wasser = false;
			flags.NoScroll = true;
			auto_scroll(40, 0);
			stop_page();
			det->set_static_ani(5, -1);
			start_aad_wait(272, -1);
			auto_move(3, P_CHEWY);
			aad_page(273, 5);
		}

		flags.AutoAniPlay = false;
	}
}

int16 Room50::use_gutschein() {
	int16 action_ret = false;

	if (is_cur_inventar(HOTEL_INV)) {
		action_ret = true;
		if (!_G(spieler).R50KeyOK) {
			hide_cur();
			auto_move(3, P_CHEWY);
			room->set_timer_status(1, TIMER_STOP);
			_wasser = false;
			stop_page();
			del_inventar(_G(spieler).AkInvent);
			start_ani_block(2, ABLOCK36);
			aad_page(274, 8);
		} else {
			start_aad_wait(276, -1);
		}
	}

	return action_ret;
}

int16 Room50::use_gum() {
	int16 action_ret = false;

	if (is_cur_inventar(GUM_INV)) {
		action_ret = true;
		hide_cur();
		_G(spieler).R50KeyOK = true;
		room->set_timer_status(1, TIMER_STOP);
		_wasser = false;
		stop_page();
		start_detail_wait(6, 1, ANI_VOR);
		det->set_static_ani(5, -1);
		auto_move(3, P_CHEWY);
		spieler_mi[P_CHEWY].Mode = true;
		go_auto_xy(75, 92, P_CHEWY, ANI_WAIT);
		SetUpScreenFunc = 0;
		go_auto_xy(112, 57, P_HOWARD, ANI_WAIT);

		set_person_spr(P_LEFT, P_HOWARD);
		del_inventar(_G(spieler).AkInvent);
		hide_person();
		start_detail_wait(2, 1, ANI_VOR);
		det->show_static_spr(4);
		start_detail_wait(2, 1, ANI_RUECK);
		show_person();
		set_person_spr(P_LEFT, P_CHEWY);
		start_aad_wait(275, -1);
		det->del_static_ani(5);
		start_detail_wait(9, 1, ANI_VOR);
		det->del_static_ani(5);
		det->set_static_ani(10, -1);
		start_aad_wait(277, -1);
		SetUpScreenFunc = setup_func;
		auto_move(3, P_CHEWY);
		spieler_mi[P_CHEWY].Mode = true;
		det->del_static_ani(10);
		go_page();

		obj->add_inventar(KEY_INV, &room_blk);
		inventory_2_cur(KEY_INV);
		atds->set_ats_str(323, 1, ATS_DATEI);
		atds->set_ats_str(327, 1, ATS_DATEI);
		_G(spieler).room_e_obj[84].Attribut = AUSGANG_OBEN;
		show_cur();
	}

	return action_ret;
}

void Room50::talk_page() {
	start_aad_wait(276, -1);
}

void Room50::aad_page(int16 aad_nr, int16 ani_nr) {
	det->set_static_ani(ani_nr, -1);
	start_aad_wait(aad_nr, -1);
	det->del_static_ani(ani_nr);
	start_detail_wait(6, 1, ANI_RUECK);
	go_page();

	if (!_G(spieler).R50KeyOK)
		room->set_timer_status(1, TIMER_START);

	show_cur();
}

void Room50::setup_func() {
	int16 x, y;
	int16 ch_x;

	if (_wasser && !det->get_ani_status(4)) {
		_wasser = false;
		go_page();
	}

	if (_G(spieler).PersonRoomNr[P_HOWARD] == 50) {
		calc_person_look();
		x = spieler_vector[P_HOWARD].Xypos[0];
		y = 64;
		ch_x = spieler_vector[P_CHEWY].Xypos[0];

		if (ch_x < 72) {
			x = 1;
			y = 64;
		} else if (ch_x < 275 && spieler_vector[P_CHEWY].Xypos[1] > 72) {
			x = 143;
			y = 57;
		} else {
			x = 162;
			y = 115;
		}

		if (HowardMov && flags.ExitMov) {
			SetUpScreenFunc = nullptr;
			HowardMov = 0;
			auto_move(4, P_HOWARD);
		} else {
			go_auto_xy(x, y, P_HOWARD, ANI_GO);
		}
	}
}

} // namespace Rooms
} // namespace Chewy
