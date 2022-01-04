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
#include "chewy/rooms/room2.h"

namespace Chewy {
namespace Rooms {

void Room2::entry() {
	if (!_G(spieler).R2KabelBork)
		det->start_detail(5, 255, 0);
}

void Room2::jump_out_r1(int16 nr) {
	_G(spieler).PersonHide[P_CHEWY] = true;
	start_detail_wait(nr, 1, ANI_VOR);
	set_up_screen(DO_SETUP);
	det->stop_detail(6);
	set_person_pos(32, 127, P_CHEWY, P_LEFT);
	_G(spieler).PersonHide[P_CHEWY] = false;
	clear_prog_ani();
	switch_room(1);
	check_shad(2, 1);
}

} // namespace Rooms
} // namespace Chewy
