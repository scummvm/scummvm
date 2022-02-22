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
#include "chewy/rooms/room62.h"

namespace Chewy {
namespace Rooms {

void Room62::entry() {
	_G(SetUpScreenFunc) = setup_func;
	_G(spieler_mi)[P_HOWARD].Mode = true;
	_G(spieler).R62LauraVerwandlung = 0;
	_G(spieler).room_e_obj[91].Attribut = 255;
	if (!_G(spieler).R62FirstEntry) {
		_G(spieler).R62TShow = false;
		_G(cur_hide_flag) = false;
		hideCur();
		_G(spieler).R62FirstEntry = true;
		set_person_pos(49, 113, P_HOWARD, P_RIGHT);
		_G(det)->start_detail(6, 255, ANI_VOR);
		start_aad_wait(345, -1);
		_G(det)->stop_detail(6);
		_G(det)->set_static_ani(5, -1);
		go_auto_xy(199, 59, P_HOWARD, ANI_WAIT);
		set_person_spr(0, P_HOWARD);
		_G(det)->del_static_ani(5);
		_G(det)->start_detail(5, 255, ANI_VOR);
		start_aad_wait(346, -1);
		_G(det)->stop_detail(5);
		_G(det)->start_detail(6, 255, ANI_VOR);
		start_aad_wait(347, -1);
		_G(det)->stop_detail(6);
		start_detail_wait(7, 1, ANI_VOR);
		_G(det)->set_static_ani(5, -1);
		_G(det)->start_detail(0, 255, ANI_VOR);
		start_aad_wait(348, -1);
		_G(det)->stop_detail(0);
		_G(det)->del_static_ani(0);
		_G(det)->start_detail(1, 1, ANI_VOR);
		auto_move(0, P_CHEWY);
		_G(spieler_mi)[P_CHEWY].Mode = true;
		go_auto_xy(160, 240, P_CHEWY, ANI_WAIT);
		_G(spieler_mi)[P_CHEWY].Mode = false;
		_G(det)->del_static_ani(5);
		start_detail_wait(7, 1, ANI_VOR);
		showCur();
		_G(spieler).R64Moni1Ani = 3;
		_G(spieler).R64Moni2Ani = 4;
		set_person_pos(187, 43, P_CHEWY, P_RIGHT);
		_G(spieler).R62TShow = true;
		switch_room(64);
	} else {
		_G(det)->del_static_ani(0);
		_G(spieler).PersonHide[P_HOWARD] = true;
		_G(det)->set_static_ani(4, -1);
		_G(det)->set_static_ani(8, -1);
		_G(r62Delay) = 0;
		_G(r62TalkAni) = 8;

	}
}

void Room62::setup_func() {
	if (_G(r62Delay) <= 0 && _G(spieler).R62TShow) {
		_G(r62Delay) = (_G(spieler).DelaySpeed + 1) * 60;
		_G(det)->stop_detail(_G(r62TalkAni));
		if (_G(r62TalkAni) == 4)
			_G(r62TalkAni) = 8;
		else
			_G(r62TalkAni) = 4;
		_G(det)->start_detail(_G(r62TalkAni), 255, ANI_VOR);
	} else
		--_G(r62Delay);
}

int16 Room62::use_laura() {
	int16 action_ret = false;
	if (is_cur_inventar(GERAET_INV)) {
		action_ret = true;
		hideCur();
		auto_move(2, P_CHEWY);
		auto_scroll(0, 0);
		_G(SetUpScreenFunc) = nullptr;
		_G(det)->del_static_ani(8);
		_G(det)->stop_detail(8);
		_G(spieler).PersonHide[P_CHEWY] = true;
		_G(det)->start_detail(2, 255, ANI_VOR);
		_G(det)->start_detail(6, 255, ANI_VOR);
		start_aad_wait(399, -1);
		_G(spieler).PersonHide[P_CHEWY] = false;
		flic_cut(FCUT_077);
		showCur();
		_G(spieler).R64Moni1Ani = 0;
		_G(spieler).R64Moni2Ani = 0;
		_G(spieler).R62TShow = false;
		_G(spieler).R62LauraVerwandlung = true;
		switch_room(63);
	}
	return action_ret;
}

} // namespace Rooms
} // namespace Chewy
