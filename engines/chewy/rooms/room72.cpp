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
#include "chewy/rooms/room72.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room72::entry(int16 eib_nr) {
	g_engine->_sound->playSound(0, 0);
	g_engine->_sound->playSound(0);
	_G(spieler_mi)[P_HOWARD].Mode = true;
	_G(spieler_mi)[P_NICHELLE].Mode = true;
	
	if (_G(spieler).flags28_10) {
		_G(zoom_horizont) = 110;
		_G(det)->hide_static_spr(0);
	} else {
		_G(spieler).ZoomXy[P_HOWARD][0] = 20;
		_G(spieler).ZoomXy[P_HOWARD][1] = 40;
		_G(spieler).ZoomXy[P_NICHELLE][0] = 20;
		_G(spieler).ZoomXy[P_NICHELLE][1] = 40;
		_G(zoom_horizont) = 140;
		_G(spieler_mi)[P_CHEWY].Mode = true;
		hide_cur();
	}

	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 3;

	if (_G(flags).LoadGame || _G(spieler).flags28_10)
		return;

	switch (eib_nr) {
	case 112:
		set_person_pos(50, 129, P_CHEWY, P_RIGHT);
		set_person_pos(15, 103, P_HOWARD, P_RIGHT);
		set_person_pos(-15, 102, P_NICHELLE, P_RIGHT);
		go_auto_xy(185, 130, P_CHEWY, ANI_GO);
		go_auto_xy(147, 103, P_HOWARD, ANI_GO);
		go_auto_xy(120, 104, P_NICHELLE, ANI_WAIT);
		proc1(84);
		register_cutscene(23);
		break;
	case 113:
		set_person_pos(213, 129, P_CHEWY, P_LEFT);
		set_person_pos(262, 103, P_HOWARD, P_LEFT);
		set_person_pos(293, 102, P_NICHELLE, P_LEFT);
		go_auto_xy(120, 130, P_CHEWY, ANI_GO);
		go_auto_xy(154, 102, P_HOWARD, ANI_GO);
		go_auto_xy(186, 102, P_NICHELLE, ANI_WAIT);
		proc1(85);
		register_cutscene(23);
		break;
	default:
		break;
	}
}

void Room72::xit(int16 eib_nr) {
	_G(spieler).ScrollxStep = 1;
}

void Room72::proc1(int16 flicNr) {
	_G(atds)->set_steuer_bit(389, ATS_AKTIV_BIT, ATS_DATEI);
	_G(spieler).flags28_10 = true;
	wait_show_screen(7);
	start_aad_wait(437, -1);
	flic_cut(flicNr, CFO_MODE);
	_G(spieler).PersonRoomNr[P_HOWARD] = 76;
	_G(spieler).PersonRoomNr[P_NICHELLE] = 76;
	_G(spieler).PersonHide[P_CHEWY] = true;
	_G(det)->hide_static_spr(0);
	set_up_screen(DO_SETUP);
	_G(spieler).PersonHide[P_CHEWY] = false;
	_G(spieler_mi)[P_CHEWY].Mode = false;
	show_cur();
	switch_room(76);
}

} // namespace Rooms
} // namespace Chewy
