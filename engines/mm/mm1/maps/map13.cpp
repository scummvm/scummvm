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

#include "mm/mm1/maps/map13.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"
#include "mm/mm1/utils/strings.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 143
#define MONSTER_ID1 389
#define MONSTER_ID2 445

void Map13::special() {
	Game::Encounter &enc = g_globals->_encounters;

	// Scan for special actions on the map cell
	for (uint i = 0; i < 23; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[74 + i]) {		
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	g_maps->clearSpecial();
	int index;
	if (g_maps->_mapPos.y < 5) {
		index = 0;
	} else if (g_maps->_mapPos.y < 9) {
		index = 14;
	} else if (g_maps->_mapPos.x < 9) {
		index = 28;
	} else {
		index = 42;
	}

	int monsterCount = getRandomNumber(7) + 5;
	enc.clearMonsters();

	for (int i = 0; i < monsterCount; ++i)
		enc.addMonster(_data[MONSTER_ID1 + index + i],
			_data[MONSTER_ID2 + index + i]);

	enc._manual = true;
	enc._levelIndex = 48;
	enc.execute();
}

void Map13::special00() {
	visitedExit();
	_data[VAL1] = 0;
	send(SoundMessage(
		"A passage leads outdoors, exit (Y/N)?",
		[]() {
			g_maps->_mapPos = Common::Point(8, 4);
			g_maps->changeMapFromExit(0x703, 2);
		}
	));
}

void Map13::special01() {
	if (_data[VAL1]) {
		g_maps->clearSpecial();
		g_globals->_treasure._items[2] = MEDUSA_HEAD_ID;
		g_events->addAction(KEYBIND_SEARCH);
	} else {
		_data[VAL1]++;
		encounter(getRandomNumber(6) + 3, 9, 6);
	}
}

void Map13::special02() {
	g_maps->clearSpecial();

	if (g_globals->_activeSpells._s.levitate) {
		send(SoundMessage(
			0, 1, mapText("A trap! A pit full of poisonous spikes\nopens below the party!\n"),
			0, 2, g_engine->isEnhanced() ?
				"Luckily, your levitation spell saved you!" :
				"Luckily,your levitation spell saved you!"
		));
	} else if (!g_globals->_activeSpells._s.poison) {
		reduceHP();
		reduceHP();

		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			Character &c = g_globals->_party[i];
			if (!(c._condition & BAD_CONDITION))
				c._condition = POISONED;
		}

		send(SoundMessage("Snake pit! "));
	}
}

void Map13::special06() {
	send(SoundMessage(mapText("Smashed remains of stone statues\nseem suspiciously lifelike.")));
}

void Map13::special10() {
	g_maps->clearSpecial();
	Sound::sound(SOUND_2);
	Sound::sound(SOUND_3);

	if (g_globals->_activeSpells._s.levitate) {
		send(InfoMessage(
			0, 1, "Snake pit! ",
			0, 2, "Levitation saved you."
		));
		Sound::sound(SOUND_3);

	} else if (!g_globals->_activeSpells._s.poison) {
		reduceHP();

		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			Character &c = g_globals->_party[i];
			if (!(c._condition & BAD_CONDITION))
				c._condition = POISONED;
		}

		InfoMessage msg(
			0, 1, "Snake pit! ",
			[]() {
				static_cast<Map13 *>(g_maps->_currentMap)->encounter(
					g_events->getRandomNumber(3) + 10, 14, 1);
			}
		);
		msg._delaySeconds = 2;
		send(msg);
	}
}

void Map13::special18() {
	g_maps->clearSpecial();
	encounter(getRandomNumber(4) + 2, 2, 5);
}

void Map13::special22() {
	g_maps->clearSpecial();
	encounter(getRandomNumber(3) + 3, 9, 6);
}

void Map13::encounter(size_t count, byte id1, byte id2) {
	Game::Encounter &enc = g_globals->_encounters;

	enc.clearMonsters();
	for (size_t i = 0; i < count; ++i)
		enc.addMonster(id1, id2);

	enc._manual = true;
	enc._levelIndex = 64;
	enc.execute();
}

} // namespace Maps
} // namespace MM1
} // namespace MM
