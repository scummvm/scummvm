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

#include "mm/mm1/maps/map14.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 75
#define VAL2 148
#define VAL3 395

void Map14::special() {
	Game::Encounter &enc = g_globals->_encounters;

	// Scan for special actions on the map cell
	for (uint i = 0; i < 6; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[57 + i]) {
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	g_maps->clearSpecial();
	int monsterCount = getRandomNumber(3);
	int id1 = getRandomNumber(16);

	enc.clearMonsters();
	for (int i = 0; i < monsterCount; ++i)
		enc.addMonster(id1, 7);

	enc._levelIndex = 5;
	enc._manual = true;
	enc.execute();
}

void Map14::special00() {
	if (_data[VAL1]) {
		g_maps->clearSpecial();
		for (uint i = 0; i < g_globals->_party.size(); ++i)
			g_globals->_party[i]._flags[2] |= CHARFLAG2_8;
		none160();

	} else {
		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			if (g_globals->_party[i]._flags[2] & CHARFLAG2_8) {
				g_maps->clearSpecial();
				none160();
				return;
			}
		}

		send(SoundMessage(STRING["maps.map14.surrounded"],
			[]() {
				Map14 &map = *static_cast<Map14 *>(g_maps->_currentMap);
				map.encounter();
			},
			[]() {
				if (g_events->getRandomNumber(3) == 3) {
					g_maps->_mapPos = Common::Point(15, 10);
					g_maps->_currentMap->updateGame();

				} else {
					Map14 &map = *static_cast<Map14 *>(g_maps->_currentMap);
					map.encounter();
				}
			}
		));
	}
}

void Map14::encounter() {
	Game::Encounter &enc = g_globals->_encounters;

	_data[VAL1]++;
	enc.clearMonsters();
	enc.addMonster(2, 12);
	for (int i = 1; i < 12; ++i)
		enc.addMonster(13, 8);

	enc._levelIndex = 80;
	enc._manual = true;
	enc.execute();
};


void Map14::special01() {
	_data[VAL1] = 0;
	_data[VAL2]++;
	none160();
}

void Map14::special02() {
	if (_data[VAL3] & 0x80) {
		g_maps->_mapPos = Common::Point(7, 0);
		g_maps->changeMap(0x706, 3);
	} else if (_data[VAL3]) {
		_data[VAL3] = 0;
	} else {
		send("View", DrawGraphicMessage(65 + 6));

		send(SoundMessage(
			STRING["maps.map14.castle"],
			[]() {
				Map14 &map = *static_cast<Map14 *>(g_maps->_currentMap);
				map[VAL3] = 0xff;
				map.updateGame();
			},
			[]() {
				Map14 &map = *static_cast<Map14 *>(g_maps->_currentMap);
				map[VAL3]++;
				map.updateGame();
			}
		));
	}
}

void Map14::special03() {
	send(SoundMessage(STRING["maps.map14.words"]));
}

void Map14::special04() {
	visitedExit();
	if (_data[VAL2]) {
		send(SoundMessage(
			STRING["maps.map14.passage"],
			[]() {
				g_maps->_mapPos = Common::Point(4, 4);
				g_maps->changeMap(0x706, 3);
			}
		));
	} else {
		none160();
	}
}

void Map14::special05() {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		if (!(c._flags[11] & CHARFLAG11_GOT_ENDURANCE)) {
			c._flags[11] |= CHARFLAG11_GOT_ENDURANCE;
			int endurance = c._endurance._base + 4;
			if (endurance < 30) {
				c._endurance._base = c._endurance._current = endurance;
			}
		}
	}

	send(SoundMessage(STRING["maps.map14.pool"]));
}

} // namespace Maps
} // namespace MM1
} // namespace MM
