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
#include "chewy/rooms/room87.h"

namespace Chewy {
namespace Rooms {

void Room87::entry() {
	_G(spieler).ScrollxStep = 2;
	_G(SetUpScreenFunc) = setup_func;
	_G(spieler_mi)[P_HOWARD].Mode = true;
	_G(spieler_mi)[P_NICHELLE].Mode = true;
	_G(spieler).ZoomXy[P_HOWARD][0] = 18;
	_G(spieler).ZoomXy[P_HOWARD][1] = 28;
	_G(spieler).ZoomXy[P_NICHELLE][0] = 18;
	_G(spieler).ZoomXy[P_NICHELLE][1] = 28;
	_G(zoom_horizont) = 176;
	if (_G(spieler).flags32_4)
		_G(det)->show_static_spr(3);

	if (_G(spieler).flags32_8)
		_G(det)->show_static_spr(4);

	if (flags.LoadGame)
		return;

	set_person_pos(11, 70, P_HOWARD, P_RIGHT);
	set_person_pos(-3, 81, P_NICHELLE, P_RIGHT);
	auto_move(2, P_CHEWY);
}

void Room87::setup_func() {
	calc_person_look();
	int howDestX = 0;
	int howDestY = 0;
	int nicDestX = 0;
	int nicDestY = 0;

	
	if (_G(spieler_vector)[P_CHEWY].Xypos[0] > 186) {
		howDestX = 103;
		howDestY = 128;
		nicDestX = 134;
		nicDestY = 123;
	} else if (_G(spieler_vector)[P_CHEWY].Xypos[0] > 20) {
		howDestX = 27;
		howDestY = 98;
		nicDestX = -2;
		nicDestY = 110;
	}

	if (_G(HowardMov) == 1) {
		howDestX = 11;
		howDestY = 70;
		nicDestX = -3;
		nicDestY = 81;
	}

	go_auto_xy(howDestX, howDestY, P_HOWARD, ANI_GO);
	go_auto_xy(nicDestX, nicDestY, P_NICHELLE, ANI_GO);
}

void Room87::xit(int16 eib_nr) {
	if (eib_nr != 129)
		return;

	_G(spieler_mi)[P_CHEWY].Mode = true;
	_G(zoom_horizont) = 0;
	_G(room)->set_zoom(25);
	_G(HowardMov) = 1;
	flags.ZoomMov = true;
	_G(zoom_mov_fak) = 2;
	go_auto_xy(20, 56, P_CHEWY, ANI_WAIT);
	go_auto_xy(31, 56, P_CHEWY, ANI_WAIT);
	go_auto_xy(9, 53, P_CHEWY, ANI_WAIT);
	_G(spieler_mi)[P_CHEWY].Mode = false;
	_G(spieler).ScrollxStep = 1;
	if (_G(spieler).PersonRoomNr[P_HOWARD] == 87)
		_G(spieler).PersonRoomNr[P_HOWARD] = 86;

	if (_G(spieler).PersonRoomNr[P_NICHELLE] == 87)
		_G(spieler).PersonRoomNr[P_NICHELLE] = 86;
}

int Room87::proc2(int16 txt_nr) {
	if (!is_cur_inventar(107) && !is_cur_inventar(108))
		return 0;

	hide_cur();

	int diaNr = 0;
	int movNr;
	if (txt_nr == 501) {
		movNr = 3;
		if (_G(spieler).flags32_4)
			diaNr = 473;
		else
			_G(spieler).flags32_4 = true;
	} else {
		movNr = 4;
		if (_G(spieler).flags32_8)
			diaNr = 473;
		else
			_G(spieler).flags32_8 = true;
	}

	if (diaNr) {
		start_spz(CH_TALK1, 255, false, P_CHEWY);
		start_aad_wait(diaNr, -1);
	} else {
		auto_move(movNr, P_CHEWY);
		del_inventar(_G(spieler).AkInvent);
		start_spz_wait(14, 1, false, P_CHEWY);
		_G(atds)->set_ats_str(txt_nr, 1, ATS_DATEI);
		_G(det)->show_static_spr(movNr);
		if (_G(spieler).flags32_4 && _G(spieler).flags32_8)
			_G(atds)->del_steuer_bit(502, ATS_AKTIV_BIT, ATS_DATEI);
	}
	show_cur();
	return 1;
}

int16 Room87::proc3(int16 key) {
	return key < 12 ? 0 : -1;
}

int16 Room87::proc5(int16 key) {
	_G(atds)->print_aad(_G(spieler).scrollx, _G(spieler).scrolly);
	return 0;
}

int Room87::proc4() {
	if (_G(spieler).inv_cur)
		return 0;

	hide_cur();
	auto_move(1, P_CHEWY);
	flic_cut(93, CFO_MODE);
	_G(flc)->set_custom_user_function(proc3);
	flic_cut(94, CFO_MODE);
	_G(flc)->remove_custom_user_function();
	_G(flc)->set_custom_user_function(proc5);
	start_aad(472);
	flic_cut(95, CFO_MODE);
	_G(flc)->remove_custom_user_function();
	flic_cut(96, CFO_MODE);
	_G(det)->hide_static_spr(2);
	_G(det)->start_detail(2, 255, false);
	start_aad_wait(471, -1);
	flic_cut(97, CFO_MODE);
	flic_cut(98, CFO_MODE);
	flic_cut(99, CFO_MODE);
	flic_cut(100, CFO_MODE);
	_G(spieler).PersonGlobalDia[P_HOWARD] = -1;
	_G(spieler).PersonDiaRoom[P_HOWARD] = false;
	_G(spieler).flags32_10 = true;

	switch_room(86);
	show_cur();
	return 1;
}

} // namespace Rooms
} // namespace Chewy
