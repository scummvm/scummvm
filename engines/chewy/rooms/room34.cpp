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
#include "chewy/rooms/room34.h"

namespace Chewy {
namespace Rooms {

bool Room34::use_kuehlschrank() {
	bool result = false;

	if (!_G(spieler).inv_cur) {
		result = true;

		if (!flags.LoadGame) {
			auto_move(3, P_CHEWY);
			start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
		}

		_G(spieler).PersonHide[P_CHEWY] = true;
		flags.ChewyDontGo = true;
		if (!flags.LoadGame) {
			switch_room(34);
		}
		set_person_pos(160, 70, P_CHEWY, -1);
	}

	return result;
}

void Room34::xit_kuehlschrank() {
	_G(spieler).PersonHide[P_CHEWY] = false;
	set_person_pos(54, 111, P_CHEWY, -1);
	switch_room(33);
	flags.ChewyDontGo = false;
	_G(maus_links_click) = false;
}

} // namespace Rooms
} // namespace Chewy
