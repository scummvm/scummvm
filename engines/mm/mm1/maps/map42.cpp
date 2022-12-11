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

#include "mm/mm1/maps/map42.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define MONSTER_COUNT 151
#define VAL1 259

void Map42::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 25; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[76 + i]) {			
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	g_maps->_mapPos.y = 4;
	updateGame();
}

void Map42::special00() {
	g_events->addView("DogStatue");
}

void Map42::special01() {
	send(SoundMessage(STRING["maps.map42.message9"]));
}

void Map42::special02() {
	Game::Encounter &enc = g_globals->_encounters;

	if (_data[VAL1]) {
		send(SoundMessage(STRING["maps.map42.defeated"]));

		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			g_globals->_party[i]._flags[5] |= CHARFLAG5_40;
		}
	} else {
		_data[VAL1]++;

		enc.clearMonsters();
		enc.addMonster(14, 12);
		for (int i = 1; i < 13; ++i)
			enc.addMonster(8, 5);

		enc._manual = true;
		enc._levelIndex = 64;
		enc.execute();
	}
}

void Map42::special03() {
	send(SoundMessage(STRING["maps.map42.sign1"]));
}

void Map42::special04() {
	Sound::sound(SOUND_2);

	g_maps->_mapPos.x = getRandomNumber(5) + 5;
	redrawGame();
}

void Map42::special05() {
	encounter(&_data[611], &_data[624]);
}

void Map42::special06() {
	encounter(&_data[637], &_data[648]);
}

void Map42::special07() {
	encounter(&_data[659], &_data[668]);
}

void Map42::special08() {
	encounter(&_data[677], &_data[686]);
}

void Map42::special09() {
	encounter(&_data[695], &_data[704]);
}

void Map42::special10() {
	encounter(&_data[713], &_data[724]);
}

void Map42::special11() {
	encounter(&_data[735], &_data[743]);
}

void Map42::special12() {
	encounter(&_data[751], &_data[759]);
}

void Map42::special13() {
	encounter(&_data[767], &_data[777]);
}

void Map42::special14() {
	encounter(&_data[787], &_data[796]);
}

void Map42::special15() {
	visitedExit();

	send(SoundMessage(
		STRING["maps.stairs_up"],
		[]() {
			g_maps->changeMap(0xf04, 3);
		}
	));
}

void Map42::special17() {
	g_globals->_treasure.setGold(7500);
	g_globals->_treasure._container = GOLD_BOX;
	g_events->addAction(KEYBIND_SEARCH);
}

void Map42::dogSuccess() {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		c._flags[0] = CHARFLAG0_DOG_STATUE;
		c._exp += 10000;
	}

	g_globals->_treasure._items[2] = GOLD_KEY_ID;
	g_globals->_treasure._trapType = 2;
	none160();
}

void Map42::dogDesecrate() {
	Game::Encounter &enc = g_globals->_encounters;
	redrawGame();

	byte &count = _data[MONSTER_COUNT];
	count *= 2;
	if (count >= 16)
		count = 13;

	enc.clearMonsters();
	for (int i = 0; i < (int)count; ++i)
		enc.addMonster(4, 10);

	enc._manual = true;
	enc._levelIndex = 20;
	enc.execute();
}

} // namespace Maps
} // namespace MM1
} // namespace MM
