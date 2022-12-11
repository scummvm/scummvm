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

#include "mm/mm1/maps/map31.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 79

void Map31::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 7; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[58 + i]) {		
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	if (_walls[g_maps->_mapOffset] != 0xff) {
		if (getRandomNumber(100) < 25) {
			g_maps->clearSpecial();
			g_globals->_encounters.execute();
		} else {
			send(SoundMessage(STRING["maps.map31.poof"]));
			g_maps->_mapPos = Common::Point(
				getRandomNumber(15), getRandomNumber(15));
			updateGame();
		}

		return;
	}

	desert();
}

void Map31::special00() {
	send(SoundMessage(STRING["maps.map31.device"]));

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];

		if (!(c._flags[11] & CHARFLAG11_GOT_INTELLIGENCE)) {
			c._flags[11] |= CHARFLAG11_GOT_INTELLIGENCE;
			c._intelligence._current = c._intelligence._base =
				MIN(c._intelligence._base + 4, 30);
		}
	}
}

void Map31::special01() {
	g_events->addView("Alien");
}

void Map31::special02() {
	if (_data[VAL1]) {
		none160();
	} else {
		encounter();
	}
}

void Map31::special06() {
	if (_data[VAL1]) {
		g_globals->_treasure._items[2] = LASER_BLASTER_ID;
		g_events->addAction(KEYBIND_SEARCH);
	} else {
		none160();
	}
}

void Map31::encounter() {
	Game::Encounter &enc = g_globals->_encounters;
	int monsterCount = getRandomNumber(7) + 5;

	enc.clearMonsters();
	for (int i = 0; i < monsterCount; ++i)
		enc.addMonster(8, 12);

	enc._manual = true;
	enc._levelIndex = 80;
	enc.execute();
}

void Map31::hostile() {
	SoundMessage msg(
		STRING["maps.map31.flash"],
		[]() {
			for (uint i = 0; i < g_globals->_party.size(); ++i) {
				g_globals->_party[i]._condition = ERADICATED;
			}
		}
	);
	msg._delaySeconds = 2;
	send(msg);
}

void Map31::neutral() {
	_data[VAL1]--;
	encounter();
}

void Map31::friendly() {
	send(SoundMessage(STRING["maps.map31.varnlings"]));
	g_maps->clearSpecial();
}

} // namespace Maps
} // namespace MM1
} // namespace MM
