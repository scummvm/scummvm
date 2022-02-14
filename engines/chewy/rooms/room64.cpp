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
#include "chewy/rooms/room64.h"

namespace Chewy {
namespace Rooms {

void Room64::entry() {
	_G(SetUpScreenFunc) = setup_func;
	_G(r62Delay) = 0;
	_G(r64TalkAni) = _G(spieler).R64Moni1Ani;

	if (_G(spieler).flags38_1)
		_G(det)->start_detail(0, 255, false);
	
	if (!_G(spieler).R64ManWeg) {
		_G(timer_nr)[0] = _G(room)->set_timer(1, 10);
		_G(det)->set_static_ani(1, -1);
		chewy_entry();
		if (_G(spieler).R63Feuer) {
			_G(flc)->set_flic_user_function(cut_sev);
			flic_cut(FCUT_078, CFO_MODE);
			_G(flc)->remove_flic_user_function();
			_G(spieler).R64ManWeg = true;
			_G(det)->del_static_ani(1);
			_G(room)->set_timer_status(1, TIMER_STOP);
			_G(atds)->set_steuer_bit(376, ATS_AKTIV_BIT, ATS_DATEI);
			_G(spieler).R64Moni1Ani = 5;
			calc_monitor();
			hide_cur();
			start_aad_wait(354, -1);
			show_cur();
		}
	} else
		chewy_entry();
}

int16 Room64::cut_sev(int16 frame) {
	const int16 spr_nr = _G(chewy_ph)[_G(spieler_vector)[P_CHEWY].Phase * 8 + _G(spieler_vector)[P_CHEWY].PhNr];
	const int16 x = _G(spieler_mi)[P_CHEWY].XyzStart[0] + _G(chewy_kor)[spr_nr * 2] - _G(spieler).scrollx;
	const int16 y = _G(spieler_mi)[P_CHEWY].XyzStart[1] + _G(chewy_kor)[spr_nr * 2 + 1] - _G(spieler).scrolly;
	
	calc_zoom(_G(spieler_mi)[P_CHEWY].XyzStart[1], (int16)_G(room)->_roomInfo->ZoomFak, (int16)_G(room)->_roomInfo->ZoomFak, &_G(spieler_vector)[P_CHEWY]);
	_G(out)->scale_set(_G(chewy)->image[spr_nr], x, y, _G(spieler_vector)[P_CHEWY].Xzoom, _G(spieler_vector)[P_CHEWY].Yzoom, _G(scr_width));
	return 0;
}

void Room64::chewy_entry() {
	calc_monitor();
	if (!_G(flags).LoadGame) {
		hide_cur();
		_G(det)->show_static_spr(3);
		auto_move(2, P_CHEWY);
		_G(det)->hide_static_spr(3);
		show_cur();
	}
}

void Room64::calc_monitor() {
	int16 str_nr = 0;
	switch (_G(spieler).R64Moni1Ani) {
	case 0:
		str_nr = 2;
		break;

	case 3:
		str_nr = 0;
		_G(det)->set_static_ani(3, -1);
		break;

	case 5:
		str_nr = 1;
		_G(det)->start_detail(5, 255, ANI_VOR);
		break;

	default:
		break;
	}

	_G(atds)->set_ats_str(373, str_nr, ATS_DATEI);
	switch (_G(spieler).R64Moni2Ani) {
	case 0:
		str_nr = 1;
		break;

	case 4:
		str_nr = 0;
		_G(det)->set_static_ani(4, -1);
		break;

	default:
		break;
	}
	_G(atds)->set_ats_str(374, str_nr, ATS_DATEI);
}

void Room64::setup_func() {
	if (_G(r62Delay) <= 0 && _G(spieler).R62TShow) {
		_G(r62Delay) = (_G(spieler).DelaySpeed + 1) * 60;
		if (_G(r64TalkAni) == 3 || _G(r64TalkAni) == 4)
			_G(det)->stop_detail(_G(r64TalkAni));
		
		if (_G(r64TalkAni) == 4)
			_G(r64TalkAni) = _G(spieler).R64Moni1Ani;
		else
			_G(r64TalkAni) = _G(spieler).R64Moni2Ani;

		if (_G(r64TalkAni) != 0)
			_G(det)->start_detail(_G(r64TalkAni), 255, ANI_VOR);
	} else
		--_G(r62Delay);
}

void Room64::talk_man() {
	talk_man(350);
}

void Room64::talk_man(int16 aad_nr) {
	if (!_G(spieler).R64ManWeg) {
		hide_cur();
		auto_move(4, P_CHEWY);
		_G(room)->set_timer_status(1, TIMER_STOP);
		_G(det)->del_static_ani(1);
		_G(det)->stop_detail(1);
		_G(det)->set_static_ani(2, -1);
		start_aad_wait(aad_nr, -1);
		_G(det)->del_static_ani(2);
		_G(room)->set_timer_status(1, TIMER_START);
		_G(det)->set_static_ani(1, -1);
		show_cur();
	}
}

int16 Room64::use_tasche() {
	int16 action_ret = false;
	hide_cur();
	if (!_G(spieler).inv_cur) {
		if (_G(spieler).R64ManWeg) {
			if (!_G(atds)->get_steuer_bit(375, ATS_AKTIV_BIT, ATS_DATEI)) {
				auto_move(3, P_CHEWY);
				start_spz_wait(CH_ROCK_GET1, 1, false, P_CHEWY);
				new_invent_2_cur(GERAET_INV);
				_G(atds)->set_steuer_bit(375, ATS_AKTIV_BIT, ATS_DATEI);
				start_aad_wait(353, -1);
			} else {
				show_cur();
				return 0;
			}
		} else {
			auto_move(3, P_CHEWY);
			_G(room)->set_timer_status(1, TIMER_STOP);
			_G(det)->del_static_ani(1);
			_G(det)->stop_detail(1);
			_G(det)->start_detail(6, 255, false);
			start_aad_wait(352, -1);
			_G(det)->stop_detail(6);
			_G(room)->set_timer_status(0, TIMER_START);
			_G(det)->set_static_ani(1, -1);
		}
		
		action_ret = true;
	}
	show_cur();
	return action_ret;
}

} // namespace Rooms
} // namespace Chewy
