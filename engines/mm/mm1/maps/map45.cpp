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

#include "mm/mm1/maps/map45.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

void Map45::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 22; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[73 + i]) {			
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

void Map45::special00() {
	send(SoundMessage(STRING["maps.map45.message8"]));
}

void Map45::special01() {
	visitedExit();

	send(SoundMessage(
		STRING["maps.map45.passage"],
		[]() {
			g_maps->_mapPos = Common::Point(15, 12);
			g_maps->changeMap(5, 1);
		}
	));
}

void Map45::special02() {
	special04();
}

void Map45::special03() {
	visitedExit();

	send(SoundMessage(
		STRING["maps.stairs_up"],
		[]() {
			g_maps->changeMap(0x107, 1);
		}
	));
}

void Map45::special04() {
	visitedExit();

	send(SoundMessage(
		STRING["maps.stairs_down"],
		[]() {
			g_maps->changeMap(0xa00, 3);
		}
	));
}

void Map45::special05() {
	encounter(&_data[600], &_data[611]);
}

void Map45::special06() {
	encounter(&_data[622], &_data[633]);
}

void Map45::special07() {
	encounter(&_data[644], &_data[657]);
}

void Map45::special08() {
	encounter(&_data[670], &_data[683]);
}

void Map45::special09() {
	encounter(&_data[696], &_data[710]);
}

void Map45::special10() {
	encounter(&_data[724], &_data[737]);
}

void Map45::special11() {
	encounter(&_data[750], &_data[761]);
}

void Map45::special12() {
	encounter(&_data[772], &_data[786]);
}

void Map45::special13() {
	encounter(&_data[800], &_data[814]);
}

void Map45::special14() {
	encounter(&_data[828], &_data[837]);
}

void Map45::special15() {
	send(SoundMessage(STRING["maps.map45.sign1"]));
}

void Map45::special16() {
	showSign(STRING["maps.map45.sign2"]);
}

void Map45::special18() {
	showSign(STRING["maps.map45.sign3"]);
}

void Map45::special19() {
	send(SoundMessage(STRING["maps.map45.walls"]));
}

void Map45::special20() {
	send(SoundMessage(STRING["maps.map45.message"]));
}

void Map45::special21() {
	send(SoundMessage(STRING["maps.wall_painted"]));
}

void Map45::showSign(const Common::String &line) {
	send(SoundMessage(
		0, 1, STRING["maps.sign"],
		0, 2, line
	));
}

} // namespace Maps
} // namespace MM1
} // namespace MM
