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

#include "mm/mm1/maps/map18.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 235
#define VAL2 196

void Map18::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 10; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[61 + i]) {				
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

void Map18::special00() {
	visitedExit();
	send(SoundMessage(
		STRING["maps.map18.passage"],
		[]() {
			g_maps->_mapPos = Common::Point(0, 7);
			g_maps->changeMap(0xb1a, 1);
		}
	));
}

void Map18::special01() {
	visitedExit();
	if (_data[VAL1] & 0x80) {
		g_maps->_mapPos = Common::Point(0, 7);
		g_maps->changeMap(0x508, 3);

	} else if (_data[VAL1] != 0) {
		_data[VAL1] = 0;

	} else {
		send("View", DrawGraphicMessage(65 + 6));
		send(SoundMessage(
			STRING["maps.map18.castle_south"],
			[]() {
				Map18 &map = *static_cast<Map18 *>(g_maps->_currentMap);
				map[VAL1] = 0xff;
				map.updateGame();
			},
			[]() {
				Map18 &map = *static_cast<Map18 *>(g_maps->_currentMap);
				map[VAL1]++;
				map.updateGame();
			}
		));
	}
}

void Map18::special02() {
	visitedExit();
	if (_data[VAL2] & 0x80) {
		g_maps->_mapPos = Common::Point(7, 15);
		g_maps->changeMap(0xf08, 3);

	} else if (_data[VAL2] != 0) {
		_data[VAL2] = 0;

	} else {
		send("View", DrawGraphicMessage(65 + 6));
		send(SoundMessage(
			STRING["maps.map18.castle_north"],
			[]() {
				Map18 &map = *static_cast<Map18 *>(g_maps->_currentMap);
				map[VAL2] = 0xff;
				map.updateGame();
			},
			[]() {
				Map18 &map = *static_cast<Map18 *>(g_maps->_currentMap);
				map[VAL2]++;
				map.updateGame();
			}
		));
	}
}

void Map18::special03() {
	visitedExit();
	send(SoundMessage(STRING["maps.map18.ruins"],
		[]() {
			g_maps->_mapPos = Common::Point(2, 2);
			g_maps->changeMap(0xf03, 3);
		}
	));
}

void Map18::special04() {
	send(SoundMessage(STRING["maps.map18.sign1"]));
}

void Map18::special05() {
	send(SoundMessage(STRING["maps.map18.sign2"]));
}

void Map18::special06() {
	send(SoundMessage(STRING["maps.map18.sign3"]));
}

void Map18::special07() {
	send(SoundMessage(STRING["maps.map18.gates"]));

	bool hasWonGame = false;
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		g_globals->_currCharacter = &c;

		if (c._flags[13] & CHARFLAG13_80)
			hasWonGame = true;
	}

	if (hasWonGame)
		g_events->addView("WonGame");
}

void Map18::special08() {
	visitedExit();
	send(SoundMessage(
		STRING["maps.map18.cave"],
		[]() {
			g_maps->_mapPos = Common::Point(15, 7);
			g_maps->changeMap(0x202, 1);
		}
	));
}

void Map18::special09() {
	g_maps->clearSpecial();
	g_globals->_treasure._items[2] = SILVER_KEY_ID;
	g_globals->_treasure._trapType = 4;
	g_globals->_treasure._container = IRON_BOX;
	g_globals->_treasure.setGold(2400);
	g_events->addAction(KEYBIND_SEARCH);
}

} // namespace Maps
} // namespace MM1
} // namespace MM
