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
#include "chewy/rooms/room49.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

static const AniBlock ABLOCK34[2] = {
	{ 7, 1, ANI_VOR, ANI_WAIT, 0 },
	{ 6, 1, ANI_VOR, ANI_WAIT, 0 },
};


void Room49::entry(int16 eib_nr) {
	_G(zoom_horizont) = 110;
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 3;
	_G(spieler).ScrollxStep = 2;
	_G(SetUpScreenFunc) = setup_func;

	if (!_G(spieler).R49BoyWeg) {
		_G(timer_nr)[0] = _G(room)->set_timer(255, 2);
	} else {
		_G(det)->del_static_ani(0);
		_G(det)->hide_static_spr(5);
		_G(det)->del_static_ani(0);
		_G(det)->del_static_ani(1);
	}

	_G(spieler).ZoomXy[P_HOWARD][0] = 30;
	_G(spieler).ZoomXy[P_HOWARD][1] = 30;
	_G(spieler_mi)[P_HOWARD].Mode = true;

	if (!_G(flags).LoadGame) {
		if (_G(spieler).R48TaxiEntry) {
			_G(spieler).R48TaxiEntry = false;
			set_person_pos(527, 76, P_HOWARD, P_LEFT);
			set_person_pos(491, 98, P_CHEWY, P_LEFT);
			_G(maus_links_click) = false;
			_G(spieler).scrollx = 320;
			_G(spieler).scrolly = 0;
			_G(det)->start_detail(5, 1, ANI_VOR);
		} else if (eib_nr == 83) {
			set_person_pos(377, 78, P_HOWARD, P_LEFT);
		}
	}
}

void Room49::xit(int16 eib_nr) {
	_G(spieler).ScrollxStep = 1;

	if (_G(spieler).PersonRoomNr[P_HOWARD] == 49) {
		_G(spieler_mi)[P_HOWARD].Mode = false;
		if (eib_nr == 80) {
			_G(spieler).PersonRoomNr[P_HOWARD] = 50;
		} else if (eib_nr == 81) {
			_G(SetUpScreenFunc) = nullptr;
			start_aad_wait(268, -1);
		}
	}
}

bool Room49::timer(int16 t_nr, int16 ani_nr) {
	if (t_nr == _G(timer_nr)[0])
		calc_boy_ani();

	return false;
}

void Room49::gedAction(int index) {
	if (!index)
		calc_boy();
}

void Room49::calc_boy_ani() {
	if (!_G(spieler).R49BoyAniCount) {
		_G(spieler).R49BoyAniCount = 3;
		_G(spieler).R49BoyAni = true;
		_G(det)->del_static_ani(0);
	} else {
		--_G(spieler).R49BoyAniCount;
		_G(spieler).R49BoyAni = false;
		_G(det)->del_static_ani(1);
	}

	_G(det)->set_static_ani(_G(spieler).R49BoyAni ? 1 : 0, -1);
	_G(det)->start_detail(_G(spieler).R49BoyAni ? 1 : 0, 1, ANI_VOR);
	_G(uhr)->reset_timer(_G(timer_nr)[0], 0);
}

void Room49::calc_boy() {
	if (!_G(spieler).R49WegFrei) {
		hide_cur();
		_G(room)->set_timer_status(255, TIMER_STOP);
		_G(uhr)->reset_timer(_G(timer_nr)[0], 0);
		stop_person(P_CHEWY);
		stop_person(P_HOWARD);
		_G(person_end_phase)[P_CHEWY] = P_LEFT;
		_G(det)->stop_detail(_G(spieler).R49BoyAni ? 1 : 0);
		_G(det)->del_static_ani(_G(spieler).R49BoyAni ? 1 : 0);
		_G(det)->set_static_ani(2, -1);

		_G(SetUpScreenFunc) = nullptr;
		start_aad_wait(262, -1);
		_G(SetUpScreenFunc) = nullptr;
		auto_move(3, P_CHEWY);
		go_auto_xy(374, 79, P_HOWARD, ANI_WAIT);
		set_person_spr(P_LEFT, P_HOWARD);
		_G(det)->del_static_ani(2);
		start_detail_wait(3, 1, ANI_VOR);

		_G(det)->show_static_spr(9);
		_G(spieler).PersonHide[P_HOWARD] = true;
		start_detail_wait(8, 1, ANI_VOR);
		_G(spieler).PersonHide[P_HOWARD] = false;
		_G(det)->hide_static_spr(9);
		start_detail_wait(4, 1, ANI_GO);

		_G(SetUpScreenFunc) = setup_func;
		_G(det)->set_static_ani(_G(spieler).R49BoyAni ? 1 : 0, -1);
		_G(room)->set_timer_status(255, TIMER_START);
		show_cur();
	}
}

int16 Room49::use_boy() {
	int16 action_ret = false;

	if (is_cur_inventar(SPARK_INV)) {
		action_ret = true;
		hide_cur();
		auto_move(3, P_CHEWY);
		del_inventar(_G(spieler).AkInvent);
		talk_boy(265);
		_G(spieler).R49WegFrei = true;
		_G(spieler).room_e_obj[80].Attribut = AUSGANG_OBEN;
		show_cur();

	} else if (is_cur_inventar(CIGAR_INV)) {
		action_ret = true;
		use_boy_cigar();
	}

	return action_ret;
}

void Room49::use_boy_cigar() {
	hide_cur();
	del_inventar(_G(spieler).AkInvent);
	talk_boy(263);
	_G(SetUpScreenFunc) = nullptr;
	auto_move(5, P_CHEWY);

	const int16 zoom = _G(room)->_roomInfo->ZoomFak;
	_G(room)->set_zoom(zoom);
	go_auto_xy(416, 79, P_HOWARD, ANI_WAIT);
	set_person_spr(P_LEFT, P_HOWARD);
	_G(flags).NoScroll = true;

	auto_scroll(164, 0);
	flic_cut(FCUT_067);
	register_cutscene(17);
	
	_G(room)->set_timer_status(255, TIMER_STOP);
	_G(uhr)->reset_timer(_G(timer_nr)[0], 0);
	_G(det)->del_static_ani(_G(spieler).R49BoyAni ? 1 : 0);
	_G(det)->stop_detail(_G(spieler).R49BoyAni ? 1 : 0);

	_G(flags).NoScroll = false;
	set_person_spr(P_RIGHT, P_CHEWY);
	start_aad_wait(264, -1);
	_G(room)->set_zoom(zoom);

	_G(obj)->add_inventar(GUM_INV, &_G(room_blk));
	inventory_2_cur(GUM_INV);
	_G(atds)->set_steuer_bit(318, ATS_AKTIV_BIT, ATS_DATEI);

	_G(SetUpScreenFunc) = setup_func;
	_G(spieler).R49BoyWeg = true;
	show_cur();
}

void Room49::talk_boy() {
	if (!_G(spieler).R49BoyWeg) {
		auto_move(3, P_CHEWY);
		talk_boy(266 + (_G(spieler).R49WegFrei ? 1 : 0));
	}
}

void Room49::talk_boy(int16 aad_nr) {
	if (!_G(spieler).R49BoyWeg) {
		_G(room)->set_timer_status(255, TIMER_STOP);
		_G(uhr)->reset_timer(_G(timer_nr)[0], 0);
		_G(det)->stop_detail(_G(spieler).R49BoyAni ? 1 : 0);

		_G(det)->set_static_ani(_G(spieler).R49BoyAni ? 1 : 0, -1);
		_G(det)->set_static_ani(2, -1);
		_G(SetUpScreenFunc) = nullptr;
		stop_person(P_HOWARD);
		start_aad_wait(aad_nr, -1);
		_G(SetUpScreenFunc) = setup_func;
		_G(det)->del_static_ani(2);
		_G(det)->set_static_ani(_G(spieler).R49BoyAni ? 1 : 0, -1);
		_G(room)->set_timer_status(255, TIMER_START);
	}
}

void Room49::look_hotel() {
	if (_G(spieler).PersonRoomNr[P_HOWARD] == 49) {
		_G(SetUpScreenFunc) = nullptr;
		stop_person(P_HOWARD);
		start_aad_wait(261, -1);
		_G(SetUpScreenFunc) = setup_func;
	}
}

int16 Room49::use_taxi() {
	int16 action_ret = false;

	if (!_G(spieler).inv_cur) {
		action_ret = true;
		hide_cur();
		start_ani_block(2, ABLOCK34);
		_G(det)->show_static_spr(7);
		auto_move(2, P_CHEWY);
		_G(spieler).PersonHide[P_CHEWY] = true;
		_G(spieler).R48TaxiPerson[P_CHEWY] = true;

		if (_G(spieler).PersonRoomNr[P_HOWARD] == 49) {
			go_auto_xy(507, 74, P_HOWARD, ANI_WAIT);
			go_auto_xy(490, 58, P_HOWARD, ANI_WAIT);
			_G(spieler).PersonHide[P_HOWARD] = true;
			_G(spieler).R48TaxiPerson[P_HOWARD] = true;
			_G(spieler).PersonRoomNr[P_HOWARD] = 48;
		}

		_G(det)->hide_static_spr(7);
		start_detail_wait(5, 1, ANI_VOR);
		g_engine->_sound->stopSound(0);
		switch_room(48);
	}

	return action_ret;
}

void Room49::setup_func() {
	if (_G(spieler).PersonRoomNr[P_HOWARD] == 49) {
		calc_person_look();
		const int16 ch_x = _G(spieler_vector)[P_CHEWY].Xypos[0];

		int16 x, y;
		if (ch_x < 130) {
			x = 40;
			y = 97;
		} else if (ch_x < 312) {
			x = 221;
			y = 94;
		} else if (ch_x < 445) {
			x = 342;
			y = 93;
		} else {
			x = 536;
			y = 90;
		}

		go_auto_xy(x, y, P_HOWARD, ANI_GO);
	}
}

} // namespace Rooms
} // namespace Chewy
