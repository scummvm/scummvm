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
#include "chewy/rooms/room69.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room69::entry(int16 eib_nr) {
	g_engine->_sound->playSound(0, 0);
	g_engine->_sound->playSound(0);
	
	_G(spieler).ScrollxStep = 2;
	_G(spieler).ZoomXy[P_HOWARD][0] = 46;
	_G(spieler).ZoomXy[P_HOWARD][1] = 90;
	_G(spieler).ZoomXy[P_NICHELLE][0] = 46;
	_G(spieler).ZoomXy[P_NICHELLE][1] = 90;
	_G(spieler_mi)[P_HOWARD].Mode = true;
	_G(spieler_mi)[P_NICHELLE].Mode = true;
	_G(zoom_horizont) = 110;
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 3;
	_G(SetUpScreenFunc) = setup_func;
	_G(spieler).DiaAMov = 2;
	if (_G(flags).LoadGame)
		return;

	_G(cur_hide_flag) = false;
	hide_cur();

	if (eib_nr == 102)
		proc1();
	else {
		set_person_pos(295, 118, P_CHEWY, P_RIGHT);
		set_person_pos(237, 101, P_NICHELLE, P_RIGHT);
		set_person_pos(347, 119, P_HOWARD, P_RIGHT);
		go_auto_xy(351, 97, P_HOWARD, ANI_WAIT);
		show_cur();
	}
}

void Room69::xit(int16 eib_nr) {
	_G(spieler).ScrollxStep = 1;

	if (_G(spieler).PersonRoomNr[P_HOWARD] == 69 && eib_nr == 97) {
		_G(spieler).PersonRoomNr[P_HOWARD] = 66;
		_G(spieler).PersonRoomNr[P_NICHELLE] = 66;
	}
}

void Room69::setup_func() {
	calc_person_look();
}

void Room69::look_schild() {
	hide_cur();
	auto_move(1, P_CHEWY);
	start_aad_wait(382, -1);
	show_cur();
}

int16 Room69::use_bruecke() {
	int16 action_flag = false;
	if (!_G(spieler).inv_cur) {
		hide_cur();
		action_flag = true;

		if (!_G(spieler).R68KarteDa) {
			calc_person_dia(P_HOWARD);
		} else {
			auto_move(3, P_CHEWY);
			_G(spieler_mi)[P_CHEWY].Mode = true;
			go_auto_xy(253, 103, P_CHEWY, ANI_WAIT);
			go_auto_xy(141, 103, P_CHEWY, ANI_WAIT);

			if (_G(spieler).PersonRoomNr[P_NICHELLE] == 69)
				go_auto_xy(255, 84, 2, ANI_GO);
			
			go_auto_xy(82, 95, P_CHEWY, ANI_WAIT);
			go_auto_xy(46, 83, P_CHEWY, ANI_WAIT);

			if (_G(spieler).PersonRoomNr[P_HOWARD] == 69) {
				go_auto_xy(258, 86, P_HOWARD, ANI_GO);
				go_auto_xy(147, 84, P_NICHELLE, ANI_WAIT);
				go_auto_xy(140, 86, P_HOWARD, ANI_GO);
				go_auto_xy(94, 77, P_NICHELLE, ANI_WAIT);
				go_auto_xy(94, 82, P_HOWARD, ANI_GO);
				go_auto_xy(59, 68, P_NICHELLE, ANI_WAIT);
				_G(spieler).PersonRoomNr[P_HOWARD] = 70;
				_G(spieler).PersonRoomNr[P_NICHELLE] = 70;
			}

			_G(spieler_mi)[P_CHEWY].Mode = false;
			switch_room(70);
		}
		show_cur();
	}
	return action_flag;
}

void Room69::proc1() {
	_G(spieler).scrollx = 0;
	_G(spieler_mi)[P_CHEWY].Mode = true;
	set_person_pos(46, 83, P_CHEWY, P_RIGHT);
	set_person_pos(59, 68, P_NICHELLE, P_RIGHT);
	set_person_pos(94, 82, P_HOWARD, P_RIGHT);
	go_auto_xy(59, 68, P_NICHELLE, ANI_GO);
	go_auto_xy(94, 82, P_HOWARD, ANI_WAIT);
	go_auto_xy(94, 77, P_NICHELLE, ANI_GO);
	go_auto_xy(140, 86, P_HOWARD, ANI_WAIT);
	go_auto_xy(82, 95, P_CHEWY, _G(spieler).PersonRoomNr[P_HOWARD] == 69 ? ANI_GO : ANI_WAIT);
	go_auto_xy(147, 84, P_NICHELLE, ANI_GO);
	go_auto_xy(278, 86, P_HOWARD, ANI_WAIT);
	go_auto_xy(255, 84, P_NICHELLE, ANI_WAIT);
	go_auto_xy(237, 101, P_NICHELLE, ANI_GO);
	go_auto_xy(351, 97, P_HOWARD, ANI_GO);
	go_auto_xy(141, 109, P_CHEWY, ANI_GO);
	go_auto_xy(253, 106, P_CHEWY, ANI_WAIT);
	auto_move(3, P_CHEWY);
	_G(spieler_mi)[P_CHEWY].Mode = false;
}

} // namespace Rooms
} // namespace Chewy
