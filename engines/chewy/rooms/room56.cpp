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
#include "chewy/rooms/room56.h"

namespace Chewy {
namespace Rooms {

void Room56::entry() {
	_G(spieler).ScrollxStep = 2;
	flags.ZoomMov = true;
	_G(zoom_mov_fak) = 4;
	spieler_mi[P_HOWARD].Mode = true;
	_G(timer_nr)[0] = room->set_timer(255, 25);
	if (!flags.LoadGame) {
		if (_G(spieler).R48TaxiEntry) {
			hide_cur();
			_G(spieler).R48TaxiEntry = false;
			_G(spieler).scrollx = 0;
			_G(spieler).scrolly = 0;
			_G(spieler).PersonHide[P_CHEWY] = true;
			_G(spieler).PersonHide[P_HOWARD] = true;
			det->hide_static_spr(2);
			_G(zoom_horizont) = 0;
			set_person_pos(-6, 16, P_HOWARD, P_RIGHT);
			set_person_pos(3, 42, P_CHEWY, P_RIGHT);
			start_detail_wait(7, 1, ANI_RUECK);
			start_detail_wait(8, 1, ANI_VOR);
			start_detail_wait(7, 1, ANI_VOR);
			set_up_screen(DO_SETUP);
			if (!_G(spieler).R56GetTabak) {
				flic_cut(FCUT_074, FLC_MODE);
				det->show_static_spr(2);
			}
			room->set_zoom(23);
			_G(spieler).ZoomXy[P_HOWARD][0] = 17;
			_G(spieler).ZoomXy[P_HOWARD][1] = 37;
			_G(spieler).PersonHide[P_CHEWY] = false;
			_G(spieler).PersonHide[P_HOWARD] = false;
			SetUpScreenFunc = setup_func;
			spieler_mi[P_CHEWY].Mode = true;
			auto_move(1, P_CHEWY);
			spieler_mi[P_CHEWY].Mode = false;
			_G(maus_links_click) = false;
			show_cur();
		} else if (_G(spieler).R62Flucht) {
			_G(maus_links_click) = false;
			_G(spieler).ZoomXy[P_HOWARD][0] = 40;
			_G(spieler).ZoomXy[P_HOWARD][1] = 86;
			_G(zoom_horizont) = 114;
			room->set_zoom(70);
			_G(spieler).R62Flucht = false;
			set_person_pos(308, 97, P_HOWARD, P_RIGHT);
			set_person_pos(429, 146, P_CHEWY, P_LEFT);
			_G(spieler).scrollx = 262;
			_G(spieler).PersonHide[P_HOWARD] = false;
			det->show_static_spr(9);
			det->show_static_spr(8);
			room->set_timer_status(0, TIMER_STOP);
			det->del_static_ani(0);
			det->set_static_ani(3, -1);
			_G(maus_links_click) = false;
			atds->stop_aad();
			start_aad_wait(306, -1);
			flic_cut(FCUT_076, FLC_MODE);
		}
	}
	SetUpScreenFunc = setup_func;
	_G(spieler).ZoomXy[P_HOWARD][0] = 40;
	_G(spieler).ZoomXy[P_HOWARD][1] = 86;
	_G(zoom_horizont) = 114;
	room->set_zoom(70);
}

void Room56::xit() {
	spieler_mi[P_HOWARD].Mode = false;
	_G(spieler).ScrollxStep = 1;
}

bool Room56::timer(int16 t_nr, int16 ani_nr) {
	if (t_nr == _G(timer_nr)[0])
		start_flug();
	else
		return true;

	return false;
}

int16 Room56::use_taxi() {
	int16 action_ret = false;
	if (!_G(spieler).inv_cur) {
		action_ret = true;
		hide_cur();
		auto_move(1, P_CHEWY);
		start_detail_wait(7, 1, ANI_RUECK);
		det->start_detail(8, 1, ANI_VOR);
		_G(zoom_horizont) = 0;
		room->set_zoom(23);
		_G(spieler).ZoomXy[P_HOWARD][0] = 17;
		_G(spieler).ZoomXy[P_HOWARD][1] = 37;
		spieler_mi[P_CHEWY].Mode = true;
		go_auto_xy(3, 42, P_CHEWY, ANI_WAIT);
		spieler_mi[P_CHEWY].Mode = false;
		_G(spieler).PersonHide[P_CHEWY] = true;
		_G(spieler).R48TaxiPerson[P_CHEWY] = true;
		if (_G(spieler).PersonRoomNr[P_HOWARD] == 56) {
			_G(spieler).PersonHide[P_HOWARD] = true;
			_G(spieler).R48TaxiPerson[P_HOWARD] = true;
			_G(spieler).PersonRoomNr[P_HOWARD] = 48;
		}
		show_cur();
		switch_room(48);
	}
	return action_ret;
}

void Room56::talk_man() {
	auto_move(3, P_CHEWY);
	if (!_G(spieler).R56AbfahrtOk) {
		start_ads_wait(16);
	} else if (!_G(spieler).R62Flucht) {
		start_aad_wait(343, -1);
	}
}

int16 Room56::use_man() {
	int16 action_ret = false;
	if (is_cur_inventar(FLASCHE_INV)) {
		action_ret = true;
		hide_cur();
		auto_move(3, P_CHEWY);
		start_spz_wait(CH_ROCK_GET2, 1, false, P_CHEWY);
		room->set_timer_status(0, TIMER_STOP);
		det->del_static_ani(0);

		if (!_G(spieler).R56WhiskyMix) {
			start_detail_wait(4, 1, ANI_VOR);

			det->set_static_ani(5, -1);
			start_aad_wait(304, -1);
			det->del_static_ani(5);
			room->set_timer_status(0, TIMER_START);
			det->set_static_ani(0, -1);
		} else {
			del_inventar(_G(spieler).AkInvent);
			_G(spieler).R56AbfahrtOk = true;
			start_detail_wait(6, 1, ANI_VOR);

			det->set_static_ani(1, -1);
			start_aad_wait(305, -1);
			det->del_static_ani(1);
		}
		room->set_timer_status(0, TIMER_START);
		det->set_static_ani(0, -1);
		show_cur();
	}
	return action_ret;
}

int16 Room56::use_kneipe() {
	int16 action_ret = false;
	if (!_G(spieler).inv_cur) {
		if (!atds->get_steuer_bit(362, ATS_AKTIV_BIT, ATS_DATEI)) {
			hide_cur();
			action_ret = true;
			if (!_G(spieler).R56Kneipe) {
				SetUpScreenFunc = nullptr;
				auto_move(4, P_CHEWY);
				_G(spieler).PersonHide[P_CHEWY] = true;
				go_auto_xy(160, 58, P_HOWARD, ANI_WAIT);
				_G(spieler).PersonHide[P_HOWARD] = true;
				_G(spieler).R56Kneipe = true;
				flags.NoScroll = true;
				auto_scroll(0, 0);
				start_detail_wait(12, 3, ANI_VOR);
				flic_cut(FCUT_075, FLC_MODE);
				det->start_detail(10, 6, ANI_VOR);
				start_aad_wait(307, -1);
				det->stop_detail(10);
				out->ausblenden(0);
				set_up_screen(DO_SETUP);
				_G(spieler).PersonHide[P_CHEWY] = false;
				_G(spieler).PersonHide[P_HOWARD] = false;
				_G(spieler).scrollx = 0;
				set_person_pos(23, 70, P_HOWARD, P_RIGHT);
				set_person_pos(50, 81, P_CHEWY, P_LEFT);

				fx_blend = BLEND3;
				start_aad_wait(308, -1);
				SetUpScreenFunc = setup_func;
				flags.NoScroll = false;

				if (obj->check_inventar(SACKGELD_INV)) {
					del_invent_slot(SACKGELD_INV);
					start_aad_wait(309, -1);
				}
			} else {
				start_aad_wait(344, -1);
			}
			show_cur();
		}
	}
	return action_ret;
}

void Room56::start_flug() {
	if (!_G(r56koch_flug)) {
		_G(r56koch_flug) = 12;
		det->start_detail(_G(r56koch_flug), 1, ANI_VOR);
	}
}

void Room56::setup_func() {
	int16 x, y;
	int16 ch_x;
	int16 ch_y;
	switch (_G(r56koch_flug)) {
	case 12:
		if (det->get_ani_status(12) == false) {
			det->start_detail(11, 1, ANI_VOR);
			_G(r56koch_flug) = 11;
		}
		break;

	case 11:
		if (det->get_ani_status(11) == false) {
			det->start_detail(10, 1, ANI_VOR);
			_G(r56koch_flug) = 10;
		}
		break;

	case 10:
		if (det->get_ani_status(10) == false) {
			_G(r56koch_flug) = 0;

			uhr->reset_timer(_G(timer_nr)[0], 0);
		}
		break;

	}
	if (_G(spieler).PersonRoomNr[P_HOWARD] == 56) {
		calc_person_look();
		ch_x = spieler_vector[P_CHEWY].Xypos[0];
		ch_y = spieler_vector[P_CHEWY].Xypos[1];
		if (ch_x < 196) {
			x = 23;
			y = 70;
		} else if (ch_x < 283) {
			x = 115;
			y = 61;
		} else if (ch_y < 115) {
			x = 254;
			y = 66;
		} else {
			x = 308;
			y = 97;
		}
		go_auto_xy(x, y, P_HOWARD, ANI_GO);
	}
	if (!atds->get_steuer_bit(362, ATS_AKTIV_BIT, ATS_DATEI)) {
		if (menu_item == CUR_WALK) {
			if (minfo.x + _G(spieler).scrollx >= 157 &&
				minfo.x + _G(spieler).scrollx <= 204 &&
				minfo.y >= 28 && minfo.y <= 89) {
				cursor_wahl(CUR_AUSGANG_OBEN);
			} else
				cursor_wahl(CUR_WALK);
		}
	}
}

} // namespace Rooms
} // namespace Chewy
