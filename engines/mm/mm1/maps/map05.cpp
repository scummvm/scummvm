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

#include "mm/mm1/maps/map05.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define MONSTER_ID 169

void Map05::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 21; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[72 + i]) {	
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

void Map05::special00() {
	Sound::sound(SOUND_2);
	send(InfoMessage(
		STRING["maps.map05.stairs"],
		[]() {
			g_maps->changeMap(0x604, 1);
		}
	));
}

void Map05::special01() {
	encounter(1);
}

void Map05::special02() {
	encounter(9);
}

void Map05::special03() {
	encounter(14);
}

void Map05::special04() {
	encounter(20);
}

void Map05::special05() {
	Game::Encounter &enc = g_globals->_encounters;
	g_maps->clearSpecial();

	enc.clearMonsters();
	for (int i = 0; i < 10; ++i)
		enc.addMonster(14, 1);

	enc._levelIndex = 80;
	enc._flag = true;
	enc.execute();
}

void Map05::special06() {
	// TODO: special
}

void Map05::special07() {
	Sound::sound(SOUND_2);
	send(InfoMessage(
		STRING["maps.map05.portal"],
		[]() {
			g_maps->_mapPos.x = 8;
			g_maps->_mapPos.y = 8;
			g_maps->changeMap(1, 1);
		}
	));
}

void Map05::special08() {
	Sound::sound(SOUND_2);
	send(InfoMessage(
		2, 1, STRING["maps.sign"],
		10, 2, STRING["maps.map05.arena"]
	));
}

void Map05::special09() {
	Sound::sound(SOUND_2);
	send(InfoMessage(
		0, 1, STRING["maps.map05.arena_inside"],
		[](const Common::KeyState &keyState) {
			if (keyState.keycode == Common::KEYCODE_y) {
				(*g_maps->_currentMap)[MAP_47] = 2;
				(*g_maps->_currentMap)[MAP_33] = 6;
				(*g_maps->_currentMap)[MAP_MAX_MONSTERS] = 15;
				g_globals->_encounters._encounterType = Game::FORCE_SURPRISED;
				g_globals->_encounters.execute();

			} else if (keyState.keycode == Common::KEYCODE_n) {
				(*g_maps->_currentMap)[MAP_47] = 1;
				(*g_maps->_currentMap)[MAP_33] = 4;
				(*g_maps->_currentMap)[MAP_MAX_MONSTERS] = 10;
				Common::fill(&g_globals->_treasure[0], &g_globals->_treasure[TREASURE_COUNT], 0);

//				TODO: Space key - is it used in-game?
//				g_events->addAction(32);
			}
		}
	));
}

void Map05::special10() {
	showMessage(STRING["maps.map05.message2"]);
}

void Map05::special11() {
	if (g_maps->_forwardMask == DIRMASK_S) {
		g_maps->clearSpecial();
		g_globals->_encounters.execute();
	} else {
		showMessage(STRING["maps.map05.message3"]);
	}
}

void Map05::special14() {
	g_maps->clearSpecial();
	if (getRandomNumber(2) == 2)
		g_globals->_encounters.execute();
}

void Map05::encounter(int monsterId) {
	Game::Encounter &enc = g_globals->_encounters;
	_data[MONSTER_ID] = monsterId;
	g_maps->clearSpecial();

	int monsterCount = getRandomNumber(5);
	enc.clearMonsters();
	for (int i = 0; i < monsterCount; ++i)
		enc.addMonster(monsterId, 1);

	enc._levelIndex = 80;
	enc._flag = true;
	enc.execute();
}

void Map05::showMessage(const Common::String &msg) {
	Sound::sound(SOUND_2);
	send(InfoMessage(
		0, 1, STRING["maps.map05.message1"],
		0, 2, msg
	));
}

} // namespace Maps
} // namespace MM1
} // namespace MM
