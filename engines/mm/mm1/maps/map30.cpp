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

#include "mm/mm1/maps/map30.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 137

static const byte MONSTER_ID1[8] = { 2, 3, 8, 10, 14, 16, 17, 7 };
static const byte MONSTER_ID2[8] = { 5, 6, 3, 8, 10, 12, 12, 12 };

void Map30::special() {
	Game::Encounter &enc = g_globals->_encounters;

	// Scan for special actions on the map cell
	for (uint i = 0; i < 4; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[65 + i]) {				
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	if (_states[g_maps->_mapOffset] != 0xff) {
		g_maps->clearSpecial();
		int monsterCount = getRandomNumber(8) + 6;
		int idx = getRandomNumber(8) - 1;
		byte id1 = MONSTER_ID1[idx];
		byte id2 = MONSTER_ID2[idx];

		enc.clearMonsters();
		for (int i = 0; i < monsterCount; ++i)
			enc.addMonster(id1, id2);

		enc._flag = true;
		enc._levelIndex = 96;
		enc.execute();
		return;
	} 

	SoundMessage msg;
	msg._lines.push_back(Line(0, 1, STRING["maps.map30.its_hot"]));

	// Check whether party has the desert map
	bool hasMap = g_globals->_party.hasItem(MAP_OF_DESERT_ID);
	if (!hasMap) {
		msg._lines.push_back(Line(0, 2, STRING["maps.map30.lost"]));
		lost();
	}

	_data[VAL1] = 0;
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];

		if (c._food) {
			// Take away food first
			c._food--;

		} else if (c._endurance) {
			// Then decrease endurance when the food runs out
			c._endurance._current--;

		} else {
			// When endurance reaches, character becomes living impaired
			c._condition = DEAD | BAD_CONDITION;
			_data[VAL1]++;
		}
	}

	if (_data[VAL1])
		g_events->findView("GameParty")->redraw();

	switch (getRandomNumber(200)) {
	case 20:
		msg._lines.push_back(Line(0, msg._lines.back().y + 1,
			STRING["maps.map30.whirlwind"]));
		Sound::sound(SOUND_3);
		g_maps->_mapPos = Common::Point(
			getRandomNumber(15), getRandomNumber(15));

		send(msg);
		updateGame();
		return;

	case 30:
		msg._lines.push_back(Line(0, msg._lines.back().y + 1,
			STRING["maps.map30.sandstorm"]));
		Sound::sound(SOUND_3);
		reduceHP();
		break;

	case 199:
	case 200:
		g_globals->_encounters.execute();
		break;

	default:
		break;
	}

	send(msg);
}

void Map30::special00() {
	send(SoundMessage(
		STRING["maps.map30.passage"],
		[]() {
			g_maps->_mapPos = Common::Point(15, 7);
			g_maps->changeMap(0x802, 1);
		}
	));
}

void Map30::special01() {
	send(SoundMessage(
		STRING["maps.map30.ruins"],
		[]() {
			g_maps->_mapPos = Common::Point(7, 15);
			g_maps->changeMap(0x107, 3);
		}
	));
}

void Map30::special02() {
	g_events->addView("Giant");
}

void Map30::special03() {
	send(SoundMessage(
		STRING["maps.map30.hourglass"],
		[]() {
			g_maps->clearSpecial();
			Sound::sound(SOUND_3);

			for (uint i = 0; i < g_globals->_party.size(); ++i) {
				Character &c = g_globals->_party[i];
				c._age._current = c._age._base =
					MAX((int)c._age._base - 20, 18);
			}

			none160();
		}
	));
}

void Map30::lost() {
	if (getRandomNumber(2) == 1) {
		g_maps->turnLeft();
	} else {
		g_maps->turnRight();
	}
}

} // namespace Maps
} // namespace MM1
} // namespace MM
