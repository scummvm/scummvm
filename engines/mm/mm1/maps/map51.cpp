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

#include "mm/mm1/maps/map51.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

void Map51::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 20; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[71 + i]) {			
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	// All other cells on the map are encounters
	g_maps->clearSpecial();
	g_globals->_encounters.execute();
}

void Map51::special00() {
	send(SoundMessage(STRING["maps.wall_painted"]));
}

void Map51::special01() {
	if (!g_globals->_party.hasItem(THUNDRANIUM_ID)) {
		g_maps->_mapPos = Common::Point(
			getRandomNumber(14), getRandomNumber(14));
		updateGame();
		send(SoundMessage(STRING["maps.map51.substance"]));
	}
}

void Map51::special02() {
	visitedExit();

	send(SoundMessage(
		STRING["maps.stairs_down"],
		[]() {
			g_maps->changeMap(0xe00, 3);
		}
	));
}

void Map51::special03() {
	visitedExit();

	send(SoundMessage(
		STRING["maps.stairs_up"],
		[]() {
			g_maps->changeMap(0xf01, 3);
		}
	));
}

void Map51::special05() {
	encounter(&_data[201], &_data[208]);
}

void Map51::special06() {
	encounter(&_data[215], &_data[230]);
}

void Map51::special07() {
	encounter(&_data[245], &_data[260]);
}

void Map51::special08() {
	encounter(&_data[275], &_data[290]);
}

void Map51::special09() {
	encounter(&_data[305], &_data[320]);
}

void Map51::special10() {
	encounter(&_data[335], &_data[350]);
}

void Map51::special11() {
	encounter(&_data[365], &_data[380]);
}

void Map51::special12() {
	encounter(&_data[395], &_data[410]);
}

void Map51::special13() {
	encounter(&_data[425], &_data[440]);
}

void Map51::special14() {
	encounter(&_data[455], &_data[470]);
}

void Map51::special17() {
	encounter(&_data[000], &_data[000]);
}

void Map51::special18() {
	special19();
}

void Map51::special19() {
	Sound::sound(SOUND_2);
	none160();
}

} // namespace Maps
} // namespace MM1
} // namespace MM
