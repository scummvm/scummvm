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

#include "mm/mm1/maps/map47.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define ARRAY1 250

void Map47::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 26; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[77 + i]) {				
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	if (getRandomNumber(100) < 10) {
		g_globals->_encounters.execute();

	} else {
		g_maps->_mapPos = Common::Point(
			getRandomNumber(16) - 1, getRandomNumber(16) - 1);
		updateGame();
		send(SoundMessage(STRING["maps.map47.poof"]));
	}
}

void Map47::special00() {
	send(SoundMessage(STRING["maps.map47.message5"]));
}

void Map47::special01() {
	send(SoundMessage(
		STRING["maps.map47.gong"],
		[]() {
			Map47 &map = *static_cast<Map47 *>(g_maps->_currentMap);
			int toneNum = g_events->getRandomNumber(6);
			if (toneNum < 4) {
				g_events->send(SoundMessage(STRING[
					Common::String::format("maps.map47.tones.%d", toneNum)]));
			} else {
				map.poof();
			}
		}
	));
}

void Map47::special02() {
	if (_data[ARRAY1] && _data[ARRAY1 + 1] && _data[ARRAY1 + 2]) {
		send(SoundMessage(STRING["maps.map47.clerics3"]));
		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			Character &c = g_globals->_party[i];
			c._flags[11] = CHARFLAG11_CLERICS;
		}
	}
}

void Map47::special03() {
	visitedExit();

	send(SoundMessage(
		STRING["maps.stairs_up"],
		[]() {
			g_maps->changeMap(0xa00, 3);
		}
	));
}

void Map47::special04() {
	send(SoundMessage(STRING["maps.map47.painting"]));
}

void Map47::special05() {
	encounter(&_data[587], &_data[594]);
}

void Map47::special06() {
	encounter(&_data[601], &_data[608]);
}

void Map47::special07() {
	encounter(&_data[615], &_data[624]);
}

void Map47::special08() {
	encounter(&_data[633], &_data[644]);
}

void Map47::special09() {
	encounter(&_data[655], &_data[665]);
}

void Map47::special10() {
	encounter(&_data[675], &_data[684]);
}

void Map47::special11() {
	encounter(&_data[693], &_data[700]);
}

void Map47::special12() {
	encounter(&_data[707], &_data[718]);
}

void Map47::special13() {
	encounter(&_data[729], &_data[740]);
}

void Map47::special14() {
	encounter(&_data[751], &_data[762]);
}

void Map47::special15() {
	encounter(&_data[773], &_data[782]);
}

void Map47::special16() {
	encounter(&_data[791], &_data[801]);
}

void Map47::special17() {
	encounter(&_data[811], &_data[821]);
}

void Map47::special23() {
	Common::String line = Common::String::format(
		STRING["maps.map47.door_number"].c_str(),
		'0' + (g_maps->_mapPos.x - 5)
	);
	send(SoundMessage(line));
}

void Map47::poof() {
	if (getRandomNumber(100) < 10) {
		g_globals->_encounters.execute();

	} else {
		g_maps->_mapPos = Common::Point(
			getRandomNumber(16) - 1, getRandomNumber(16) - 1);
		updateGame();

		send(SoundMessage(STRING["maps.map47.poof"]));
	}
}

} // namespace Maps
} // namespace MM1
} // namespace MM
