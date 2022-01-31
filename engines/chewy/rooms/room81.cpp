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
#include "chewy/rooms/room81.h"

namespace Chewy {
namespace Rooms {

void Room81::entry() {
	det->enable_sound(0, 0);
	det->play_sound(0, 0);
	spieler_mi[P_HOWARD].Mode = true;
	spieler_mi[P_NICHELLE].Mode = true;
	_G(zoom_horizont) = 140;
	flags.ZoomMov = true;
	_G(zoom_mov_fak) = 3;
	_G(spieler).DiaAMov = 0;
	if (_G(spieler).R88Val1 == 84) {
		det->show_static_spr(0);
		atds->del_steuer_bit(488, ATS_AKTIV_BIT, ATS_DATEI);
	}

	proc3();

	if (flags.LoadGame || !_G(spieler).flags30_1)
		return;

	set_person_pos(155, 146, P_CHEWY, P_LEFT);
	set_person_pos(103, 115, P_HOWARD, P_RIGHT);
	set_person_pos(62, 112, P_NICHELLE, P_RIGHT);
	_G(spieler).flags30_1 = false;
	_G(maus_links_click) = false;
	_G(spieler).scrollx = 0;
}

void Room81::xit(int16 eib_nr) {
	_G(spieler).R79Val[P_CHEWY] = 1;
	_G(spieler).R79Val[P_HOWARD] = 1;
	_G(spieler).PersonRoomNr[P_HOWARD] = 80;
	_G(spieler).R79Val[P_NICHELLE] = 1;
	_G(spieler).PersonRoomNr[P_NICHELLE] = 80;
}

void Room81::proc1() {
	int diaNr = -1;
	hide_cur();
	auto_move(1, P_CHEWY);
	if (!_G(spieler).flags30_2)
		diaNr = 458;
	else if (_G(spieler).flags30_4)
		diaNr = 460;
	else {
		spieler_mi[P_CHEWY].Mode = true;
		go_auto_xy(222, 97, P_CHEWY, ANI_WAIT);
		go_auto_xy(100, 96, P_CHEWY, ANI_WAIT);
		go_auto_xy(171, 93, P_CHEWY, ANI_WAIT);
		go_auto_xy(143, 62, P_CHEWY, ANI_WAIT);
		go_auto_xy(112, 60, P_CHEWY, ANI_WAIT);
		start_aad_wait(461, -1);
		go_auto_xy(143, 62, P_CHEWY, ANI_WAIT);
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_detail_wait(0, 1, ANI_VOR);
		det->start_detail(1, 255, false);
		start_aad_wait(459, -1);
		det->stop_detail(1);
		start_detail_wait(0, 1, ANI_GO);
		_G(spieler).PersonHide[P_CHEWY] = false;
		go_auto_xy(171, 93, P_CHEWY, ANI_WAIT);
		go_auto_xy(100, 96, P_CHEWY, ANI_WAIT);
		go_auto_xy(222, 97, P_CHEWY, ANI_WAIT);
		auto_move(1, P_CHEWY);
		spieler_mi[P_CHEWY].Mode = false;
		invent_2_slot(DIARY_INV);
		_G(spieler).flags30_2 = true;
		_G(spieler).flags31_4 = true;
	}
	
	if (diaNr != -1) {
		start_spz(16, 255, false, P_CHEWY);
		start_aad_wait(diaNr, -1);
	}

	show_cur();
}

int Room81::proc2() {
	int retVal = 0;
	int diaNr = -1;
	int aniId = -1;

	hide_cur();
	if (is_cur_inventar(110)) {
		auto_move(1, P_CHEWY);
		start_spz_wait(13, 1, false, P_CHEWY);
		aniId = 5;
		diaNr = 462;
		del_inventar(_G(spieler).AkInvent);
		_G(spieler).flags30_2 = true;
	} else if (is_cur_inventar(104)) {
		aniId = 4;
		diaNr = 463;
	} else if (is_cur_inventar(102)) {
		aniId = 4;
		diaNr = 464;
	}

	if (diaNr != -1) {
		retVal = 1;
		start_spz(aniId, 255, false, P_CHEWY);
		start_aad_wait(diaNr, -1);
		proc3();
	}

	show_cur();
	return retVal;
}

void Room81::proc3() {
	if (!_G(spieler).flags30_2) {
		det->start_detail(2, 255, false);
		return;
	}

	for (int i = 0; i < 3; ++i)
		det->start_detail(3 + i, 255, false);

	det->stop_detail(2);
	atds->del_steuer_bit(486, ATS_AKTIV_BIT, ATS_DATEI);
	atds->set_steuer_bit(490, ATS_AKTIV_BIT, ATS_DATEI);
}

} // namespace Rooms
} // namespace Chewy
