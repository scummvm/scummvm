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
#include "chewy/rooms/room80.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room80::entry() {
	_G(maus_links_click) = false;
	_G(spieler).scrollx = 0;
	_G(spieler).scrolly = 0;
	g_engine->_sound->playSound(0, 0);
	g_engine->_sound->playSound(0, 1);
	g_engine->_sound->playSound(0, 2);
	g_engine->_sound->playSound(0);
	g_engine->_sound->playSound(0, 1, false);
	g_engine->_sound->playSound(0, 2, false);

	if (_G(spieler).gottenDiary) {
		_G(atds)->del_steuer_bit(476, ATS_AKTIV_BIT, ATS_DATEI);
	} else {
		_G(atds)->set_steuer_bit(476, ATS_AKTIV_BIT, ATS_DATEI);
	}

	if (_G(spieler).flags32_1) {
		_G(spieler).scrollx = 39;
		return;
	}

	set_person_pos(37, 10, P_CHEWY, P_RIGHT);
	set_person_pos(22, -1, P_HOWARD, P_RIGHT);
	set_person_pos(6, 2, P_NICHELLE, P_RIGHT);
	_G(spieler).scrollx = 10;
	_G(flags).NoScroll = true;
	_G(spieler).ZoomXy[P_HOWARD][0] = 24;
	_G(spieler).ZoomXy[P_HOWARD][1] = 40;
	_G(spieler).ZoomXy[P_NICHELLE][0] = 24;
	_G(spieler).ZoomXy[P_NICHELLE][1] = 40;
	_G(zoom_horizont) = 0;

	if (_G(spieler).r88DestRoom == 84)
		_G(det)->show_static_spr(3);
	else
		_G(det)->show_static_spr(4);

	_G(SetUpScreenFunc) = setup_func;
}

void Room80::setup_func() {
	for (int i = 0; i < 3; ++i)
		_G(det)->hide_static_spr(i);

	if (_G(spieler).flags32_1 || !_G(flags).ShowAtsInvTxt || _G(menu_display))
		return;

	_G(menu_item) = CUR_USE;
	cur_2_inventory();
	cursorChoice(CUR_ZEIGE);
	int vec = _G(det)->maus_vector(_G(spieler).scrollx + _G(minfo).x, _G(minfo).y);
	if (vec == -1)
		return;

	if (vec != 0 && vec != 2) {
		if (vec != 1 || !_G(spieler).gottenDiary)
			return;
	}

	_G(det)->show_static_spr(vec);
	if (!_G(maus_links_click))
		return;

	int nextRoom;
	switch (vec) {
	case 0:
		nextRoom = 82;
		break;
	case 1:
		if (_G(spieler).R88UsedMonkey)
			nextRoom = 85;
		else
			nextRoom = 84;
		break;
	case 2:
		nextRoom = 81;
		break;
	default:
		nextRoom = -1;
		break;
	}

	if (nextRoom == -1)
		return;

	_G(SetUpScreenFunc) = nullptr;
	_G(det)->hide_static_spr(vec);
	_G(menu_item) = CUR_WALK;
	cursorChoice(CUR_WALK);
	_G(spieler).flags30_1 = true;
	_G(maus_links_click) = false;
	set_up_screen(DO_SETUP);
	
	for (int i = P_CHEWY; i <= P_NICHELLE; ++i) {
		if (_G(spieler).R79Val[i] != 0) {
			_G(spieler).PersonHide[i] = false;
			_G(spieler).R79Val[i] = 0;
		}
	}

	if (_G(spieler).PersonRoomNr[P_HOWARD] == 80)
		_G(spieler).PersonRoomNr[P_HOWARD] = nextRoom;

	if (_G(spieler).PersonRoomNr[P_NICHELLE] == 80)
		_G(spieler).PersonRoomNr[P_NICHELLE] = nextRoom;

	_G(flags).NoScroll = false;
	switch_room(nextRoom);
}

} // namespace Rooms
} // namespace Chewy
