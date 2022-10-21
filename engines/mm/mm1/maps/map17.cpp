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

#include "mm/mm1/maps/map17.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 509
#define VAL2 510
#define VAL3 511

void Map17::special() {
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

	if (getRandomNumber(100) == 100) {
		Character &c = g_globals->_party[0];
		g_globals->_currCharacter = &c;
		int id1 = getRandomNumber(c._level >= 12 ? 14 : c._level) + 2;
		int monsterCount = getRandomNumber(id1 < 15 ? 13 : 4);

		enc.clearMonsters();
		for (int i = 0; i < monsterCount; ++i)
			enc.addMonster(id1, 11);

		enc._flag = true;
		enc._levelIndex = 80;
		enc.execute();

	} else if (getRandomNumber(30) == 10) {
		g_maps->_mapPos = Common::Point(15, 15);
		send(SoundMessage(STRING["maps.map17.wave"]));
		g_events->send("Game", GameMessage("UPDATE"));

	} else {
		g_events->addAction(KEYBIND_SEARCH);
	}
}

void Map17::special00() {
	send(SoundMessage("maps.map17.islands"));
}

void Map17::special01() {
}

void Map17::special02() {
	if (_data[VAL3]) {
		g_globals->_treasure[5] = 236;
		g_events->addAction(KEYBIND_SEARCH);
	} else {
		g_events->addKeypress((Common::KeyCode)211);
	}
}

void Map17::special03() {
	g_maps->clearSpecial();
	g_globals->_encounters.execute();
}

} // namespace Maps
} // namespace MM1
} // namespace MM
