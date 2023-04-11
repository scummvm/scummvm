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

#include "mm/mm1/maps/map15.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 350
#define VAL2 118
#define ITEM_ID 361
#define GEMS 362

void Map15::special() {
	Game::Encounter &enc = g_globals->_encounters;

	// Scan for special actions on the map cell
	for (uint i = 0; i < 9; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[60 + i]) {			
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	if (g_maps->_mapPos.y >= 3) {
		g_globals->_treasure.clear();

		if (getRandomNumber(20) == 20) {
			if (_data[VAL1] > 14)
				_data[VAL1] = 14;

			g_globals->_activeSpells._s.fire = 0;
			enc.clearMonsters();
			for (uint i = 0; i < _data[VAL1]; ++i)
				enc.addMonster(7, 8);

			enc._manual = true;
			enc._levelIndex = 80;
			enc.execute();

		} else {
			send(SoundMessage(16, 1, STRING["maps.map15.its_hot"]));

			if (!g_globals->_activeSpells._s.fire) {
				for (uint i = 0; i < g_globals->_party.size(); ++i) {
					Character &c = g_globals->_party[i];
					c._hpCurrent = MAX((int)c._hpCurrent - 15, 0);
				}
			}
		}
	} else if (getRandomNumber(100) != 100) {
		none160();

	} else {
		Character &c = g_globals->_party[0];
		g_globals->_currCharacter = &c;
		int id1 = getRandomNumber(2 + ((c._level < 12) ? c._level : 14));
		int monsterCount = getRandomNumber((id1 < 15) ? 13 : 4);

		enc.clearMonsters();
		for (int i = 0; i < monsterCount; ++i)
			enc.addMonster(id1, 11);

		enc._manual = true;
		enc._levelIndex = 80;
		enc.execute();
	}
}

void Map15::special00() {
	send(SoundMessage(STRING["maps.map15.lava"]));
}

void Map15::special01() {
	Game::Encounter &enc = g_globals->_encounters;

	if (_data[VAL2]) {
		SoundMessage msg(
			STRING["maps.map15.body"],
			[]() {
				g_globals->_treasure._items[2] = DRAGONS_TOOTH_ID;
				g_events->addAction(KEYBIND_SEARCH);
			}
		);
		msg._delaySeconds = 5;
		send(msg);

	} else {
		_data[VAL2]++;

		enc.clearMonsters();
		enc.addMonster(15, 9);
		enc._levelIndex = 5;
		enc._manual = true;
		enc.execute();
	}
}

void Map15::special02() {
	Game::Encounter &enc = g_globals->_encounters;
	g_maps->clearSpecial();
	_data[VAL2]++;

	int monsterCount = getRandomNumber(4) + 1;
	enc.clearMonsters();
	for (int i = 0; i < monsterCount; ++i)
		enc.addMonster(15, 9);

	enc._levelIndex = 48;
	enc._manual = true;
	enc.execute();
}

void Map15::special03() {
	Game::Encounter &enc = g_globals->_encounters;
	g_maps->clearSpecial();
	_data[VAL2]++;

	enc.clearMonsters();
	for (int i = 0; i < 10; ++i)
		enc.addMonster(15, 9);

	enc._levelIndex = 48;
	enc._manual = true;
}

void Map15::special04() {
	_data[ITEM_ID] = PIRATES_MAP_A_ID;
	_data[GEMS] = 100;
	cove();
}

void Map15::special05() {
	_data[ITEM_ID] = PIRATES_MAP_B_ID;
	_data[GEMS] = 200;
	cove();
}

void Map15::special06() {
	g_maps->clearSpecial();
	send(SoundMessage(STRING["maps.map15.percella1"],
		[]() {
			g_globals->_treasure._items[2] = KINGS_PASS_ID;
			g_events->addAction(KEYBIND_SEARCH);
		},
		[]() {
			SoundMessage msg(
				STRING["maps.map15.percella2"],
				[]() {
					g_maps->_mapPos = Common::Point(14, 2);
					g_maps->_currentMap->updateGame();
				}
			);
			msg._delaySeconds = 5;
			g_events->send(msg);
		}
	));
}

void Map15::special08() {
	_data[VAL2] = 0;
	none160();
}

void Map15::cove() {
	send(SoundMessage(STRING["maps.map15.cove"],
		[]() {
			Map15 &map = *static_cast<Map15 *>(g_maps->_currentMap);
			g_maps->clearSpecial();

			for (uint i = 0; i < g_globals->_party.size(); ++i) {
				Character &c = g_globals->_party[i];
				int idx = c._backpack.indexOf(map[ITEM_ID]);
				if (idx != -1) {
					c._backpack.removeAt(idx);
					g_globals->_treasure.setGold(2000);
					g_globals->_treasure.setGems(map[GEMS]);
					break;
				}
			}

			g_events->addAction(KEYBIND_SEARCH);
		}
	));
}

} // namespace Maps
} // namespace MM1
} // namespace MM
