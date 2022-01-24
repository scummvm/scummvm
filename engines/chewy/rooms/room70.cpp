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
#include "chewy/ani_dat.h"
#include "chewy/room.h"
#include "chewy/rooms/room70.h"

namespace Chewy {
namespace Rooms {

void Room70::entry(int16 eib_nr) {
}

void Room70::xit(int16 eib_nr) {
}

void Room70::setup_func() {
	calc_person_look();

	const int posX = spieler_vector[P_CHEWY].Xypos[0];

	int howDestX, nicDestX;
	if (posX > 40) {
		howDestX = 52;
		nicDestX = 100;
	} else if (posX < 230) {
		howDestX = 83;
		nicDestX = 163;
	} else if (posX < 445) {
		howDestX = 261;
		nicDestX = 329;
	} else {
		howDestX = 493;
		nicDestX = 543;
	}

	go_auto_xy(howDestX, 111, P_HOWARD, P_RIGHT);
	go_auto_xy(nicDestX, 110, P_NICHELLE, P_RIGHT);	
}

void Room70::proc1() {
	_G(cur_hide_flag) = 0;
	hide_cur();
	set_person_pos(236, 110, P_CHEWY, P_RIGHT);
	set_person_pos(263, 85, P_NICHELLE, P_RIGHT);
	set_person_pos(285, 78, P_HOWARD, P_RIGHT);
	go_auto_xy(266, 113, P_HOWARD, ANI_VOR);
	show_cur();
}

void Room70::proc2() {
	set_person_pos(31, 118, P_CHEWY, P_RIGHT);
	set_person_pos(71, 104, P_NICHELLE, P_RIGHT);
	set_person_pos(6, 111, P_HOWARD, P_RIGHT);
}

void Room70::proc3() {
	set_person_pos(587, 114, P_CHEWY, P_LEFT);
	set_person_pos(613, 103, P_NICHELLE, P_LEFT);
	set_person_pos(561, 112, P_HOWARD, P_LEFT);
}
} // namespace Rooms
} // namespace Chewy
