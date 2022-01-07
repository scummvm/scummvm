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
#include "chewy/rooms/room66.h"

namespace Chewy {
namespace Rooms {

void Room66::entry(int16 eib_nr) {
	_G(spieler).ScrollxStep = 2;
	spieler_mi[P_HOWARD].Mode = true;
	spieler_mi[P_NICHELLE].Mode = true;
}

void Room66::xit(int16 eib_nr) {
	_G(spieler).ScrollxStep = 1;
	switch (eib_nr) {
	case 98:
		_G(spieler).PersonRoomNr[P_HOWARD] = 69;
		_G(spieler).PersonRoomNr[P_NICHELLE] = 69;
		break;

	case 99:
		_G(spieler).PersonRoomNr[P_HOWARD] = 68;
		_G(spieler).PersonRoomNr[P_NICHELLE] = 68;
		break;

	case 100:
		_G(spieler).PersonRoomNr[P_HOWARD] = 67;
		_G(spieler).PersonRoomNr[P_NICHELLE] = 67;
		break;

	}
}

void Room66::talk1() {
}

void Room66::talk2() {
}

void Room66::talk3() {
}

void Room66::talk4() {
}

int Room66::proc2() {
	return 0;
}

int Room66::proc7() {
	return 0;
}

} // namespace Rooms
} // namespace Chewy
