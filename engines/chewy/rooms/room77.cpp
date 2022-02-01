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
#include "chewy/rooms/room77.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room77::entry() {
	g_engine->_sound->playSound(0, 0);
	g_engine->_sound->playSound(0, 1);
	g_engine->_sound->playSound(0);
	g_engine->_sound->playSound(0, 1, false);
	_G(spieler).ScrollxStep = 2;
	if (_G(spieler).r76State == 1) {
		_G(spieler).flags29_4 = true;
		atds->set_ats_str(452, 1, ATS_DATEI);
	}

	if (_G(spieler).flags29_4) {
		det->set_static_ani(0, -1);

		for (int i = 0; i < 3; ++i) {
			det->show_static_spr(2 + i);
			atds->del_steuer_bit(460 + i, ATS_AKTIV_BIT, ATS_DATEI);
		}
	}

	// WORKAROUND: Loading saved game in this room don't properly
	// reset the flag used for cutscenes when returning to village
	if (flags.LoadGame) {
		_G(spieler).r76State = -1;

	} else if (_G(spieler).r76State == 1) {
		_G(spieler).r76State = -1;
		hide_cur();
		_G(spieler).scrollx = 62;
		set_person_pos(158, 127, P_CHEWY, P_RIGHT);
		start_aad_wait(440, -1);
		show_cur();
	}
}

void Room77::xit() {
	_G(spieler).ScrollxStep = 1;
}

int Room77::proc1() {
	if (_G(spieler).flags29_4 || !is_cur_inventar(103))
		return 0;

	hide_cur();
	auto_move(1, P_CHEWY);
	int diaNr, aniId;
	if (_G(spieler).flags29_8) {
		start_spz_wait(14, 1, false, P_CHEWY);
		atds->set_ats_str(463, 1, ATS_DATEI);
		_G(spieler).flags29_10 = true;
		_G(spieler).flags29_40 = true;
		diaNr = 441;
		aniId = 6;
	} else {
		diaNr = 439;
		aniId = 4;
	}

	cur_2_inventory();
	start_spz(aniId, 255, false, P_CHEWY);
	start_aad_wait(diaNr, -1);
	show_cur();
	if (_G(spieler).flags29_10)
		remove_inventory(103);

	return 1;
}

int Room77::proc2() {
	if (_G(spieler).flags29_4 || !is_cur_inventar(100))
		return 0;

	hide_cur();
	auto_move(2, P_CHEWY);

	int diaNr, aniId;
	if (_G(spieler).flags29_8) {
		start_spz_wait(14, 1, false, P_CHEWY);
		del_inventar(_G(spieler).AkInvent);
		atds->set_ats_str(464, 1, 1);
		_G(spieler).flags29_20 = true;
		_G(spieler).flags29_40 = true;
		diaNr = 442;
		aniId = 6;
	} else {
		diaNr = 439;
		aniId = 4;
	}

	start_spz(aniId, 255, false, P_CHEWY);
	start_aad_wait(diaNr, -1);

	show_cur();
	return 1;
}

} // namespace Rooms
} // namespace Chewy
