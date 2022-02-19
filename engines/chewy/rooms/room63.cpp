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
#include "chewy/rooms/room63.h"

namespace Chewy {
namespace Rooms {

void Room63::entry() {
	_G(zoom_horizont) = 76;
	_G(spieler).ScrollxStep = 2;
	_G(r63Schalter) = false;
	_G(r63RunDia) = 0;
	if (_G(spieler).R63FxMannWeg)
		_G(det)->del_static_ani(5);
	if (!_G(spieler).R63Uhr) {
		_G(cur_hide_flag) = false;
		hide_cur();
		_G(det)->start_detail(12, 255, ANI_VOR);
		_G(det)->start_detail(10, 255, ANI_VOR);
		_G(det)->start_detail(18, 255, ANI_VOR);
		auto_move(4, P_CHEWY);
		_G(det)->stop_detail(10);
		_G(det)->start_detail(11, 255, ANI_VOR);
		auto_scroll(58, 0);
		start_aad_wait(355, -1);
		_G(det)->stop_detail(11);
		_G(det)->start_detail(10, 255, ANI_VOR);
		show_cur();
	} else if (!_G(spieler).R63Feuer) {
		_G(det)->show_static_spr(10);
		_G(det)->show_static_spr(12);
	} else if (_G(spieler).R62LauraVerwandlung) {
		_G(atds)->set_steuer_bit(383, ATS_AKTIV_BIT, ATS_DATEI);
		_G(SetUpScreenFunc) = setup_func;
		cur_2_inventory();
		_G(spieler).scrollx = 176;
		set_person_pos(424, 78, P_CHEWY, P_LEFT);
		_G(spieler).PersonHide[P_CHEWY] = true;
		_G(spieler).room_e_obj[95].Attribut = 255;
		_G(r63ChewyAni) = 0;
		_G(det)->start_detail(0, 1, ANI_VOR);
	}
}

void Room63::setup_func() {
	if (!_G(spieler).R62LauraVerwandlung)
		return;

	cur_2_inventory();
	_G(menu_item) = CUR_USE;
	cursor_wahl(CUR_USE);
	
	switch (_G(r63ChewyAni)) {
	case 0:
		if (_G(det)->get_ani_status(0) == false) {
			if (_G(r63RunDia) < 4)
				++_G(r63RunDia);
			start_aad(370 + _G(r63RunDia));
			_G(det)->start_detail(1, 1, ANI_VOR);
			_G(r63ChewyAni) = 1;
		}
		break;

	case 1:
		if (_G(det)->get_ani_status(1) == false) {
			_G(spieler).ScrollxStep = 4;
			set_person_pos(0, 0, P_CHEWY, P_RIGHT);
			_G(det)->start_detail(22, 1, ANI_VOR);
			_G(det)->start_detail(2, 1, ANI_VOR);
			_G(atds)->stop_aad();
			_G(r63ChewyAni) = 2;
		}
		break;

	case 2:
		if (_G(det)->get_ani_status(2) == false) {
			_G(det)->start_detail(3, 1, ANI_VOR);
			_G(r63ChewyAni) = 3;
		}
		break;

	case 3:
		if (_G(det)->get_ani_status(3) == false) {
			if (!_G(flags).AutoAniPlay) {
				_G(flags).AutoAniPlay = true;
				_G(flags).NoScroll = true;

				_G(spieler).ScrollxStep = 16;
				_G(spieler).scrollx -= _G(spieler).scrollx % 16;
				auto_scroll(176, 0);
				set_person_pos(424, 78, P_CHEWY, P_LEFT);
				_G(flags).NoScroll = false;
				_G(spieler).ScrollxStep = 4;
				if (!_G(r63Schalter)) {
					_G(det)->start_detail(0, 1, ANI_VOR);
					_G(r63ChewyAni) = 0;
				} else
					bork_platt();
				_G(flags).AutoAniPlay = false;
			}
		}
		break;

	default:
		break;
	}
}

void Room63::bork_platt() {
	_G(spieler).R62LauraVerwandlung = false;
	_G(r63Schalter) = false;
	_G(spieler).room_e_obj[95].Attribut = AUSGANG_OBEN;
	flic_cut(FCUT_081);
	flic_cut(FCUT_082);
	_G(spieler).PersonHide[P_CHEWY] = false;
	check_shad(4, 1);
	_G(spieler_mi)[P_CHEWY].Mode = true;
	auto_move(6, P_CHEWY);
	_G(spieler_mi)[P_CHEWY].Mode = false;
	start_aad_wait(370, -1);
	start_detail_wait(4, 1, ANI_VOR);
	_G(det)->show_static_spr(13);
	start_aad_wait(361, -1);
	_G(out)->cls();
	_G(flags).NoPalAfterFlc = true;
	flic_cut(FCUT_079);
	_G(fx_blend) = BLEND3;
	show_cur();
	_G(flags).MainInput = true;
	_G(spieler).R62Flucht = true;
	_G(spieler).PersonRoomNr[P_HOWARD] = 56;
	switch_room(56);
}

void Room63::talk_hunter() {
	hide_cur();
	auto_move(3, P_CHEWY);
	if (_G(spieler).R63Uhr)
		_G(det)->hide_static_spr(10);
	_G(det)->start_detail(10, 255, ANI_VOR);
	start_aad_wait(356, -1);
	if (_G(spieler).R63Uhr) {
		_G(det)->show_static_spr(10);
		_G(det)->stop_detail(10);
	}
	show_cur();
}

void Room63::talk_regie() {
	hide_cur();
	auto_move(3, P_CHEWY);
	if (_G(spieler).R63Uhr)
		_G(det)->hide_static_spr(12);
	_G(det)->stop_detail(18);
	_G(det)->start_detail(19, 255, ANI_VOR);
	start_aad_wait(357, -1);
	_G(det)->stop_detail(19);
	if (_G(spieler).R63Uhr) {
		_G(det)->show_static_spr(12);
	} else {
		_G(det)->start_detail(18, 255, ANI_VOR);
	}
	show_cur();
}

void Room63::talk_fx_man() {
	hide_cur();
	auto_move(1, P_CHEWY);
	start_aad_wait(358, -1);
	show_cur();
}

int16 Room63::use_fx_man() {
	int16 action_ret = false;
	if (is_cur_inventar(MASKE_INV)) {
		action_ret = true;
		hide_cur();
		auto_move(1, P_CHEWY);
		del_inventar(_G(spieler).AkInvent);
		remove_inventory(34);
		start_aad_wait(359, -1);
		_G(det)->del_static_ani(5);
		start_detail_wait(6, 1, ANI_VOR);
		_G(det)->start_detail(7, 255, ANI_VOR);
		start_aad_wait(362, -1);
		_G(det)->stop_detail(7);
		start_detail_wait(8, 1, ANI_VOR);
		_G(spieler).R63FxMannWeg = true;
		_G(atds)->set_steuer_bit(384, ATS_AKTIV_BIT, ATS_DATEI);
		show_cur();
	}
	return action_ret;
}

int16 Room63::use_schalter() {
	int16 action_ret = false;
	if (!_G(spieler).inv_cur) {
		action_ret = true;
		if (_G(spieler).R63FxMannWeg) {
			if (_G(spieler).R62LauraVerwandlung) {
				_G(r63Schalter) = true;
				hide_cur();
				_G(flags).MainInput = false;
			} else {
				hide_cur();
				auto_move(1, P_CHEWY);
				start_spz_wait(CH_ROCK_GET2, 1, false, P_CHEWY);
				_G(det)->show_static_spr(2);
				set_person_spr(P_LEFT, P_CHEWY);
				start_detail_wait(21, 1, ANI_VOR);
				_G(det)->show_static_spr(14);
				wait_show_screen(18);
				_G(det)->hide_static_spr(14);
				start_detail_wait(24, 1, ANI_VOR);
				_G(det)->show_static_spr(1);
				_G(det)->hide_static_spr(2);
				start_aad_wait(364, -1);
				_G(atds)->set_ats_str(385, 1, ATS_DATEI);
				show_cur();
			}
		} else {
			hide_cur();
			start_aad_wait(363, -1);
			show_cur();
		}
	}
	return action_ret;
}

void Room63::talk_girl() {
	auto_move(2, P_CHEWY);
	_G(det)->stop_detail(12);
	start_detail_wait(13, 1, ANI_VOR);
	_G(det)->set_static_ani(14, -1);
	start_ads_wait(17);
	_G(det)->del_static_ani(14);
	_G(det)->start_detail(12, 255, ANI_VOR);
}

int16 Room63::use_girl() {
	int16 action_ret = false;
	if (is_cur_inventar(UHR_INV)) {
		action_ret = true;
		hide_cur();
		auto_move(2, P_CHEWY);
		del_inventar(_G(spieler).AkInvent);
		_G(det)->stop_detail(12);
		start_detail_wait(13, 1, ANI_VOR);
		_G(det)->set_static_ani(14, -1);
		start_aad_wait(365, -1);
		_G(det)->del_static_ani(14);
		_G(det)->start_detail(15, 255, ANI_VOR);
		start_aad_wait(360, -1);
		_G(det)->stop_detail(15);
		start_detail_wait(16, 1, ANI_VOR);
		_G(spieler).R63Uhr = true;
		_G(det)->stop_detail(10);
		_G(det)->stop_detail(18);
		_G(det)->show_static_spr(10);
		_G(det)->show_static_spr(12);
		_G(atds)->set_ats_str(381, 1, ATS_DATEI);
		_G(atds)->set_ats_str(382, 1, ATS_DATEI);
		_G(atds)->set_steuer_bit(380, ATS_AKTIV_BIT, ATS_DATEI);
		start_aad_wait(367, -1);
		show_cur();
	}
	return action_ret;
}

int16 Room63::use_aschenbecher() {
	int16 action_ret = false;
	if (is_cur_inventar(ASCHE_INV)) {
		action_ret = true;
		_G(cur_hide_flag) = false;
		hide_cur();
		if (_G(spieler).R63Uhr) {
			if (_G(spieler).R63FxMannWeg) {
				auto_move(5, P_CHEWY);
				del_inventar(_G(spieler).AkInvent);
				_G(flags).NoScroll = true;
				auto_scroll(70, 0);
				auto_move(1, P_CHEWY);
				_G(spieler).PersonHide[P_CHEWY] = true;
				_G(det)->start_detail(20, 255, ANI_VOR);
				_G(det)->start_detail(10, 255, ANI_VOR);
				_G(det)->start_detail(18, 255, ANI_VOR);
				_G(det)->hide_static_spr(10);
				_G(det)->hide_static_spr(12);
				start_aad_wait(368, -1);
				flic_cut(FCUT_080);
				_G(atds)->set_steuer_bit(381, ATS_AKTIV_BIT, ATS_DATEI);
				_G(atds)->set_steuer_bit(382, ATS_AKTIV_BIT, ATS_DATEI);
				_G(atds)->set_ats_str(383, 1, ATS_DATEI);
				_G(spieler).R63Feuer = true;
				_G(spieler).PersonHide[P_CHEWY] = false;
				_G(spieler).scrollx = 0;
				set_person_pos(187, 42, P_CHEWY, P_RIGHT);
				switch_room(64);
				_G(flags).NoScroll = false;
			} else
				start_aad_wait(369, -1);
		} else
			start_aad_wait(366, -1);
		
		show_cur();
	}
	return action_ret;
}

} // namespace Rooms
} // namespace Chewy
