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

#include "mm/mm1/maps/map53.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 379

void Map53::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 15; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[66 + i]) {			
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

void Map53::special00() {
	send(SoundMessage(STRING["maps.wall_painted"]));
	if (!_data[VAL1]) {
		g_globals->_treasure._items[2] = W_QUEEN_IDOL_ID;
		_data[VAL1]++;
	}
}

void Map53::special01() {
	send(SoundMessage(STRING["maps.map53.sign"]));
}

void Map53::special03() {
	visitedExit();

	send(SoundMessage(
		STRING["maps.map53.ladder"],
		[]() {
			g_maps->changeMap(0xf01, 3);
		}
	));
}

void Map53::special04() {
	g_globals->_encounters.execute();
}

void Map53::special05() {
	encounter(&_data[182], &_data[190]);
}

void Map53::special06() {
	encounter(&_data[198], &_data[205]);
}

void Map53::special07() {
	encounter(&_data[212], &_data[220]);
}

void Map53::special08() {
	encounter(&_data[228], &_data[234]);
}

void Map53::special09() {
	encounter(&_data[240], &_data[252]);
}

void Map53::special10() {
	encounter(&_data[264], &_data[274]);
}

void Map53::special11() {
	encounter(&_data[284], &_data[288]);
}

void Map53::special12() {
	encounter(&_data[292], &_data[296]);
}

void Map53::special13() {
	encounter(&_data[300], &_data[304]);
}

void Map53::special14() {
	encounter(&_data[308], &_data[312]);
}

} // namespace Maps
} // namespace MM1
} // namespace MM
