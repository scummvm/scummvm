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

#include "mm/mm1/maps/map06.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 446
#define VAL2 447
#define VAL3 329

static const byte MONSTER_ID1[13] = {
	2, 1, 8, 9, 5, 9, 2, 7, 15, 4, 11, 1, 5
};
static const byte MONSTER_ID2[13] = {
	3, 4, 3, 4, 4, 3, 4, 3, 3, 4, 4, 3, 3
};

void Map06::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 27; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[78 + i]) {				
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	if (_data[VAL1]) {
		_data[VAL2]++;

		Common::Point &pos = g_maps->_mapPos;
		if (pos.y == 9) {
			if (pos.x == 13 || pos.x == 14)
				pos.x++;
			else
				pos.y++;
		} else if (pos.x < 6) {
			if (pos.y == 0) {
				if (pos.x == 0)
					pos.y++;
				else
					pos.x--;
			} else if (pos.y != 10 || pos.x == 3) {
				pos.y++;
			} else {
				pos.x++;
			}
		} else if (pos.x == 6) {
			if (pos.y == 0)
				pos.x--;
			else
				pos.y--;
		} else {
			pos.y++;
		}

		g_events->send("Game", GameMessage("UPDATE"));
	} else {
		checkPartyDead();
	}
}

void Map06::special00() {
	Sound::sound(SOUND_2);
	send(InfoMessage(
		STRING["maps.map06.passage_outside"],
		[]() {
			g_maps->_mapPos = Common::Point(15, 11);
			g_maps->changeMap(0xa11, 2);
		}
	));
}

void Map06::special01() {
	Sound::sound(SOUND_2);
	send(InfoMessage(
		STRING["maps.map06.portal"],
		[]() {
			g_maps->_mapPos = Common::Point(7, 0);
			g_maps->changeMap(0xc01, 1);
		}
	));
}

void Map06::special02() {
	if (_data[VAL1]) {
		slide();
	} else {
		checkPartyDead();
	}
}

void Map06::special03() {
	if (_data[VAL1]) {
		g_maps->_mapPos.y++;
		slide();
	} else {
		checkPartyDead();
	}
}

void Map06::special04() {
	if (_data[VAL2]) {
		_data[VAL2] = 0;
		Sound::sound(SOUND_2);
		send(InfoMessage(0, 1, STRING["maps.map06.acid"]));

		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			Character &c = g_globals->_party[i];
			c._hpBase = MAX((int)c._hpBase - 15, 0);
			if (!c._hpBase) {
				if (!(c._condition & BAD_CONDITION))
					c._condition = UNCONSCIOUS;
			}
		}
	} else {
		// TODO: Space key - is it used in-game?
		g_events->addKeypress(Common::KEYCODE_SPACE);
	}
}

void Map06::special06() {
	Sound::sound(SOUND_2);
	send(InfoMessage(
		STRING["maps.map06.button"],
		[]() {
			byte &val1 = (*static_cast<Map06 *>(g_maps->_currentMap))[VAL1];
			val1 = val1 ? 0 : 1;
			g_events->addKeypress(Common::KEYCODE_SPACE);
		}
	));
}

void Map06::special07() {
	Sound::sound(SOUND_2);
	send(InfoMessage(17, 1, STRING["maps.poof"]));
	g_maps->_mapPos.x = 10;
	g_maps->_mapPos.y = 11;
	g_events->send("Game", GameMessage("UPDATE"));
}

void Map06::special09() {
	Sound::sound(SOUND_2);
	send(InfoMessage(0, 1, STRING["maps.map06.banner"]));
}

void Map06::special10() {
	g_maps->_mapPos.x++;
	g_globals->_encounters.execute();
}

void Map06::special13() {
	Game::Encounter &enc = g_globals->_encounters;
	g_maps->clearSpecial();
	_data[VAL3]++;

	int monsterCount = getRandomNumber(5);

	enc._levelIndex = 20;
	int id1 = MONSTER_ID1[g_maps->_mapPos.y];
	int id2 = MONSTER_ID2[g_maps->_mapPos.y];

	enc.clearMonsters();
	for (int i = 0; i < monsterCount; ++i)
		enc.addMonster(id1, id2);

	enc.execute();
}

void Map06::special26() {
	if (_data[VAL3]) {
		Sound::sound(SOUND_2);
		send(InfoMessage(STRING["maps.map06.wizard"]));
	} else {
		g_globals->_treasure[6] = 224;
		g_globals->_treasure[7] = 46;
		g_globals->_treasure[4] = 237;
		g_globals->_treasure[5] = getRandomNumber(12) + 182;
		g_events->addAction(KEYBIND_SEARCH);
	}
}

void Map06::slide() {
	Sound::sound(SOUND_2);
	send(InfoMessage(16, 1, STRING["maps.map06.slide"]));
	_data[VAL2]++;

	g_events->send("Game", GameMessage("UPDATE"));
}

} // namespace Maps
} // namespace MM1
} // namespace MM
