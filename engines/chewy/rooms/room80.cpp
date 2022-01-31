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
#include "chewy/rooms/room80.h"

namespace Chewy {
namespace Rooms {

void Room80::entry() {
	_G(maus_links_click) = false;
	_G(spieler).scrollx = 0;
	_G(spieler).scrolly = 0;
	det->enable_sound(0, 0);
	det->enable_sound(0, 1);
	det->enable_sound(0, 2);
	det->play_sound(0, 0);
	det->play_sound(0, 1);
	det->play_sound(0, 2);

	if (_G(spieler).gottenDiary) {
		atds->del_steuer_bit(476, ATS_AKTIV_BIT, ATS_DATEI);
	} else {
		atds->set_steuer_bit(476, ATS_AKTIV_BIT, ATS_DATEI);
	}

	if (_G(spieler).flags32_1) {
		_G(spieler).scrollx = 39;
		return;
	}

	set_person_pos(37, 10, P_CHEWY, P_RIGHT);
	set_person_pos(22, -1, P_HOWARD, P_RIGHT);
	set_person_pos(6, 2, P_NICHELLE, P_RIGHT);
	_G(spieler).scrollx = 10;
	flags.NoScroll = true;
	_G(spieler).ZoomXy[P_HOWARD][0] = 24;
	_G(spieler).ZoomXy[P_HOWARD][1] = 40;
	_G(spieler).ZoomXy[P_NICHELLE][0] = 24;
	_G(spieler).ZoomXy[P_NICHELLE][1] = 40;
	_G(zoom_horizont) = 0;

	if (_G(spieler).r88DestRoom == 84)
		det->show_static_spr(3);
	else
		det->show_static_spr(4);

	SetUpScreenFunc = setup_func;
}

void Room80::setup_func() {
	for (int i = 0; i < 3; ++i)
		det->hide_static_spr(i);

	if (_G(spieler).flags32_1 || !flags.ShowAtsInvTxt || menu_display)
		return;

	menu_item = CUR_USE;
	cur_2_inventory();
	cursor_wahl(CUR_ZEIGE);
	int vec = det->maus_vector(_G(spieler).scrollx + minfo.x, minfo.y);
	if (vec == -1)
		return;

	if (vec != 0 && vec != 2) {
		if (vec != 1 || !_G(spieler).gottenDiary)
			return;
	}

	det->show_static_spr(vec);
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

	SetUpScreenFunc = nullptr;
	det->hide_static_spr(vec);
	menu_item = CUR_WALK;
	cursor_wahl(CUR_WALK);
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

	flags.NoScroll = false;
	switch_room(nextRoom);
}

} // namespace Rooms
} // namespace Chewy
