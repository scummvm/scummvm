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

#include "mm/mm1/maps/map08.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 163

void Map08::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < _data[50]; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[75 + i]) {
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	g_maps->clearSpecial();

	if (g_maps->_mapPos.x == 0 || g_maps->_mapPos.x == 15) {
		g_globals->_encounters.execute();
	} else {
		addTreasure();
	}
}

void Map08::special00() {
	visitedExit();
	send(SoundMessage(
		STRING["maps.stairs_up"],
		[]() {
			g_maps->_mapPos = Common::Point(15, 7);
			g_maps->changeMap(0xb1a, 1);
		}
	));
}

void Map08::special01() {
	visitedExit();
	send(SoundMessage(
		STRING["maps.stairs_up"],
		[]() {
			g_maps->_mapPos = Common::Point(14, 1);
			g_maps->changeMap(0xa00, 2);
		}
	));
}

void Map08::special02() {
	if (_data[VAL1]) {
		g_events->addView("AccessCode");
	} else {
		checkPartyDead();
	}
}

void Map08::special04() {
	g_maps->_mapPos = Common::Point(15, 9);
	updateGame();
}

void Map08::special05() {
	g_maps->_mapPos = Common::Point(0, 9);
	updateGame();
}

void Map08::special06() {
	if (_data[VAL1]) {
		reduceHP();

		send(InfoMessage(18, 2, STRING["maps.map08.zap"]));
		Sound::sound(SOUND_3);
	} else {
		checkPartyDead();
	}
}

void Map08::special08() {
	if (_data[VAL1]) {
		send(InfoMessage(0, 1, STRING["maps.map08.dancing_lights"]));
		Sound::sound(SOUND_3);
	} else {
		checkPartyDead();
	}
}

void Map08::special20() {
	g_maps->clearSpecial();
	g_globals->_treasure._items[2] = getRandomNumber(48);
	g_globals->_treasure._trapType = 1;
	g_globals->_treasure._container = SILVER_BOX;
	addTreasure();
}

void Map08::addTreasure() {
	g_globals->_treasure.setGold(getRandomNumber(150) + 100);
	g_globals->_treasure.setGems(getRandomNumber(4));
	g_events->addAction(KEYBIND_SEARCH);
}

} // namespace Maps
} // namespace MM1
} // namespace MM
