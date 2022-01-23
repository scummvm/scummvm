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
#include "chewy/rooms/room79.h"

namespace Chewy {
namespace Rooms {

void Room79::entry() {
	det->enable_sound(0, 0);
	det->play_sound(0, 0);
	_G(spieler).ScrollxStep = 2;
	spieler_mi[P_HOWARD].Mode = true;
	spieler_mi[P_NICHELLE].Mode = true;
	_G(spieler).PersonHide[P_CHEWY] = false;
	_G(spieler).PersonHide[P_HOWARD] = false;
	_G(spieler).PersonHide[P_NICHELLE] = false;
	_G(spieler).PersonRoomNr[P_HOWARD] = 79;
	_G(spieler).PersonRoomNr[P_NICHELLE] = 79;
	if (!flags.LoadGame) {
		set_person_pos(459, 114, P_CHEWY, P_LEFT);
		set_person_pos(568, 65, P_HOWARD, P_LEFT);
		set_person_pos(534, 75, P_NICHELLE, P_LEFT);
		_G(spieler).scrollx = 300;
		hide_cur();
		start_aad_wait(484, -1);
		_G(spieler).PersonGlobalDia[P_HOWARD] = 10026;
		_G(spieler).PersonDiaRoom[P_HOWARD] = true;
		show_cur();
	}

	SetUpScreenFunc = setup_func;
}

void Room79::xit() {
	_G(spieler).R79Val[P_CHEWY] = 1;
	_G(spieler).R79Val[P_HOWARD] = 1;
	_G(spieler).PersonRoomNr[P_HOWARD] = 80;
	_G(spieler).R79Val[P_NICHELLE] = 1;
	_G(spieler).PersonRoomNr[P_NICHELLE] = 80;
}

void Room79::setup_func() {
	calc_person_look();
	const int posX = spieler_vector[P_CHEWY].Xypos[0];

	int howDestX, howDestY, nicDestX, nicDestY;

	if (posX < 160) {
		howDestX = 228;
		howDestY = 102;
		nicDestX = 191;
		nicDestY = 97;
	} else if (posX < 280) {
		howDestX = 346;
		howDestY = 98;
		nicDestX = 307;
		nicDestY = 100;
	} else if (posX < 390) {
		howDestX = 463;
		howDestY = 96;
		nicDestX = 424;
		nicDestY = 94;
	} else {
		howDestX = 568;
		howDestY = 65;
		nicDestX = 534;
		nicDestY = 75;
	}

	go_auto_xy(howDestX, howDestY, P_HOWARD, ANI_GO);
	go_auto_xy(nicDestX, nicDestY, P_NICHELLE, ANI_GO);
}

} // namespace Rooms
} // namespace Chewy
