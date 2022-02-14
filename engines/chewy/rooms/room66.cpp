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
#include "chewy/rooms/room66.h"

namespace Chewy {
namespace Rooms {

void Room66::entry(int16 eib_nr) {
	_G(spieler).ScrollxStep = 2;
	_G(spieler_mi)[P_HOWARD].Mode = true;
	_G(spieler_mi)[P_NICHELLE].Mode = true;
	_G(spieler).ZoomXy[P_HOWARD][0] = 30;
	_G(spieler).ZoomXy[P_HOWARD][0] = 50;
	_G(spieler).ZoomXy[P_NICHELLE][0] = 36;
	_G(spieler).ZoomXy[P_NICHELLE][0] = 50;
	_G(zoom_horizont) = 130;
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 3;
	if (!_G(spieler).flags26_4) {
		_G(det)->show_static_spr(4);
		_G(det)->show_static_spr(8);
		_G(cur_hide_flag) = false;
		hide_cur();
		_G(spieler).flags26_4 = true;
		_G(spieler).scrollx = 476;
		set_person_pos(598, 101, P_CHEWY, P_RIGHT);
		set_person_pos(644, 82, P_NICHELLE, P_LEFT);
		set_person_pos(623, 81, P_HOWARD, P_LEFT);
		start_aad_wait(413, -1);
		auto_move(9, P_CHEWY);
		_G(SetUpScreenFunc) = setup_func;
		start_aad_wait(403, -1);
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_detail_wait(0, 1, ANI_VOR);
		_G(det)->show_static_spr(14);
		wait_show_screen(15);
		_G(det)->hide_static_spr(14);
		start_detail_wait(1, 1, ANI_VOR);
		load_chewy_taf(CHEWY_NORMAL);
		_G(spieler).PersonHide[P_CHEWY] = false;
		start_spz(CH_TALK12, 255, false, P_CHEWY);
		start_aad_wait(404, -1);
		start_aad_wait(415, -1);
		show_cur();
	} else if (!_G(flags).LoadGame && _G(spieler).PersonRoomNr[P_HOWARD] == 66) {
		switch (eib_nr) {
		case 96:
			set_person_pos(488, 114, P_HOWARD, P_RIGHT);
			set_person_pos(520, 114, P_NICHELLE, P_RIGHT);
			break;
		case 97:
			set_person_pos(22, 114, P_HOWARD, P_RIGHT);
			set_person_pos(50, 114, P_NICHELLE, P_RIGHT);
			break;
		case 101:
			set_person_pos(150, 114, P_HOWARD, P_RIGHT);
			set_person_pos(182, 114, P_NICHELLE, P_RIGHT);
			break;
		default:
			break;
		}
		
	}
	_G(SetUpScreenFunc) = setup_func;
}

void Room66::xit(int16 eib_nr) {
	_G(spieler).ScrollxStep = 1;
	_G(atds)->set_steuer_bit(415, ATS_AKTIV_BIT, ATS_DATEI);
	_G(atds)->set_steuer_bit(417, ATS_AKTIV_BIT, ATS_DATEI);
	if (_G(spieler).PersonRoomNr[P_HOWARD] != 66)
		return;

	switch (eib_nr) {
	case 98:
		_G(spieler).PersonRoomNr[P_HOWARD] = 69;
		_G(spieler).PersonRoomNr[P_NICHELLE] = 69;
		break;

	case 99:
		_G(spieler).PersonRoomNr[P_HOWARD] = 68;
		_G(spieler).PersonRoomNr[P_NICHELLE] = 68;
		break;

	case 100:
		_G(spieler).PersonRoomNr[P_HOWARD] = 67;
		_G(spieler).PersonRoomNr[P_NICHELLE] = 67;
		break;

	default:
		break;
	}
}

void Room66::setup_func() {
	calc_person_look();
	const int posX = _G(spieler_vector)[P_CHEWY].Xypos[0];

	int edx, esi;
	if (posX < 30) {
		edx = 57;
		esi = 97;
	} else if (posX < 260){
		edx = 170;
		esi = 263;
	} else if (posX < 370) {
		edx = 314;
		esi = 398;
	} else if (posX < 500) {
		edx = 517;
		esi = 556;
	} else {
		edx = 607;
		esi = 690;
	}

	go_auto_xy(edx, 114, P_HOWARD, ANI_GO);
	go_auto_xy(esi, 114, P_NICHELLE, ANI_GO);

	if (posX >= 500 || _G(spieler).flags26_8)
		return;

	_G(spieler).flags26_8 = true;
	_G(det)->start_detail(9, 5, false);
	start_aad_wait(405, -1);
}

void Room66::talk1() {
	hide_cur();
	auto_move(5, P_CHEWY);
	start_aad_wait(407, -1);
	show_cur();
}

void Room66::talk2() {
	proc8(6, 10, 11, 408);
}

void Room66::talk3() {
	proc8(8, 6, 7, 409);
}

void Room66::talk4() {
	proc8(7, 2, 3, _G(spieler).flags26_20 ? 414 : 410);
}

int Room66::proc2() {
	hide_cur();
	auto_move(0, P_CHEWY);
	_G(spieler).flags26_40 = true;
	_G(spieler).room_e_obj[100].Attribut = 3;
	_G(atds)->set_ats_str(423, 1, ANI_GO);
	show_cur();
	
	return 0;
}

int Room66::proc7() {
	if (!is_cur_inventar(ARTE_INV))
		return 0;

	hide_cur();
	auto_move(7, P_CHEWY);
	if (_G(spieler).flags26_10) {
		del_inventar(_G(spieler).AkInvent);
		invent_2_slot(92);
		invent_2_slot(93);
		invent_2_slot(94);
		_G(spieler).flags26_20 = true;
	}

	proc8(7, 2, 3, 411 + (_G(spieler).flags26_10 ? 1 : 0));
	_G(cur_hide_flag) = 0;
	hide_cur();
	if (_G(spieler).flags26_20)
		start_detail_wait(4, 1, ANI_VOR);
	show_cur();

	return 1;
}

void Room66::proc8(int chewyAutoMovNr, int restartAniNr, int transitionAniNr, int transitionDiaNr) {
	hide_cur();

	if (chewyAutoMovNr != -1)
		auto_move(chewyAutoMovNr, P_CHEWY);

	_G(room)->set_timer_status(restartAniNr, TIMER_STOP);
	_G(det)->del_static_ani(restartAniNr);
	_G(det)->set_static_ani(transitionAniNr, -1);
	start_aad_wait(transitionDiaNr, -1);
	_G(det)->del_static_ani(transitionAniNr);
	_G(det)->set_static_ani(restartAniNr, -1);
	_G(room)->set_timer_status(restartAniNr, TIMER_START);
	show_cur();
}

} // namespace Rooms
} // namespace Chewy
