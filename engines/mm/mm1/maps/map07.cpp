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

#include "mm/mm1/maps/map07.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

static const byte MONSTER_IDS1[5] = { 2, 4, 1, 6, 4 };
static const byte MONSTER_IDS2[5] = { 2, 5, 2, 3, 3 };
static const byte MONSTER_IDS3[6] = {  2, 3, 3, 8, 8, 8 };
static const byte MONSTER_IDS4[6] = { 10, 9, 9, 7, 7, 7 };

void Map07::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 18; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[69 + i]) {
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	g_maps->clearSpecial();
	int idx = getRandomNumber(5) - 1;
	setMonsters(MONSTER_IDS1[idx], MONSTER_IDS2[idx]);
	g_globals->_encounters.execute();
}

void Map07::special00() {
	visitedExit();
	send(SoundMessage(
		STRING["maps.stairs_up"],
		[]() {
			g_maps->_mapPos = Common::Point(0, 15);
			g_maps->changeMap(0xc03, 1);
		}
	));
}

void Map07::special01() {
	visitedExit();
	send(SoundMessage(
		STRING["maps.map07.portal"],
		[]() {
			g_maps->_mapPos = Common::Point(2, 7);
			g_maps->changeMap(0x202, 1);
		}
	));
}

void Map07::special02() {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		const Character &c = g_globals->_party[i];

		for (byte itemId = BRONZE_KEY_ID;
				itemId <= DIAMOND_KEY_ID; ++itemId) {
			if (c.hasItem(itemId)) {
				// Someone has a key
				none160();
				return;
			}
		}
	}

	send(SoundMessage(0, 1, STRING["maps.map07.gate"]));
	g_maps->_mapPos.y--;
	updateGame();
}

void Map07::special03() {
	send(SoundMessage(0, 1, STRING["maps.map07.do_not_disturb"]));
}

void Map07::special04() {
	Game::Encounter &enc = g_globals->_encounters;
	g_maps->clearSpecial();

	enc.clearMonsters();
	for (int i = 0; i < 6; ++i)
		enc.addMonster(MONSTER_IDS3[i], MONSTER_IDS4[i]);

	for (int i = 6; i < 13; ++i)
		enc.addMonster(1, 2);

	enc.execute();
}

void Map07::special05() {
	g_maps->clearSpecial();
	setMonsters();
	g_globals->_encounters.addMonster(9, 2);
	g_globals->_encounters.execute();
}

void Map07::special07() {
	g_maps->clearSpecial();
	setMonsters();
	g_globals->_encounters.addMonster(10, 4);
	g_globals->_encounters.execute();
}

void Map07::special09() {
	Game::Encounter &enc = g_globals->_encounters;

	g_maps->clearSpecial();
	setMonsters();

	for (int i = 6; i < 10; ++i)
		enc.addMonster(9, 2);
	for (int i = 10; i < 13; ++i)
		enc.addMonster(10, 4);
	enc.addMonster(11, 4);
	enc.execute();
}

void Map07::special11() {
	Game::Encounter &enc = g_globals->_encounters;

	g_maps->clearSpecial();
	setMonsters();

	for (int i = 6; i < 12; ++i)
		enc.addMonster(11, 4);

	enc.addMonster(9, 5);
	enc.execute();
}

void Map07::special13() {
	poolYN(
		[]() {
			for (uint i = 0; i < g_globals->_party.size(); ++i) {
				Character &c = g_globals->_party[i];
				c._sex = (c._sex == MALE) ? FEMALE : MALE;
				c.loadFaceSprites();
			}

			g_events->send(SoundMessage(0, 1, STRING["maps.map07.reversal"]));
			Sound::sound(SOUND_3);
		}
	);
}

void Map07::special14() {
	applyCondition(POISONED);
}

void Map07::special15() {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		if (!(c._flags[11] & CHARFLAG11_GOT_MIGHT)) {
			c._flags[11] |= CHARFLAG11_GOT_MIGHT;
			c._might._base = MIN((int)c._might._base + 4, 255);
		}
	}

	g_events->send(SoundMessage(0, 1, STRING["maps.map07.might"]));
	Sound::sound(SOUND_3);
}

void Map07::special16() {
	applyCondition(DISEASED);
}

void Map07::special17() {
	poolYN([]() {
		g_globals->_treasure._items[2] = g_events->getRandomNumber(12) + 24;
		g_globals->_treasure.setGems(20);
		g_events->addAction(KEYBIND_SEARCH);
	});
}

void Map07::setMonsters(int id1, int id2) {
	Game::Encounter &enc = g_globals->_encounters;
	enc.clearMonsters();

	for (int i = 0; i < 6; ++i)
		enc.addMonster(id1, id2);

	enc._manual = true;
	enc._levelIndex = 48;
}

void Map07::poolYN(YNCallback callback) {
	send(SoundMessage(STRING["maps.map07.pool"], callback));
}

void Map07::applyCondition(Condition cond) {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		if (!(c._condition & BAD_CONDITION))
			c._condition = cond;
	}

	Sound::sound(SOUND_3);
	send(InfoMessage(0, 1, STRING["maps.map07.toxic"]));
}

} // namespace Maps
} // namespace MM1
} // namespace MM
