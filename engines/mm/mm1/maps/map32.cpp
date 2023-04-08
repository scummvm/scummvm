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

#include "mm/mm1/maps/map32.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 75
#define VAL2 111
#define PASSWORD_INDEX 393

void Map32::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 6; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[57 + i]) {		
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	if (_walls[g_maps->_mapOffset] == 0x55 && g_maps->_mapPos.x < 13) {
		send(SoundMessage(STRING["maps.map32.music"]));
	} else {
		g_maps->clearSpecial();
		g_globals->_encounters.execute();
	}
}

void Map32::special00() {
	visitedExit();

	if (!g_globals->_party.hasItem(DIAMOND_KEY_ID)) {
		send(SoundMessage(STRING["maps.map32.door"]));
	} else {
		send(SoundMessage(
			0, 1, STRING["maps.map32.door"],
			0, 2, STRING["maps.map32.key"],
			[]() {
				g_maps->_mapPos = Common::Point(7, 0);
				g_maps->changeMap(0xb1a, 3);
			}
		));
	}
}

void Map32::special01() {
	g_events->addView("Lion");
}

void Map32::special02() {
	visitedExit();

	if (_data[VAL2] & 0x80) {
		g_maps->_mapPos = Common::Point(0, 7);
		g_maps->changeMap(0xb07, 3);

	} else if (_data[VAL2] != 0) {
		_data[VAL2] = 0;

	} else {
		send("View", DrawGraphicMessage(65 + 6));
		send(SoundMessage(
			STRING["maps.map32.castle"],
			[]() {
				Map32 &map = *static_cast<Map32 *>(g_maps->_currentMap);
				map[VAL2] = 0xff;
				map.updateGame();
			},
			[]() {
				Map32 &map = *static_cast<Map32 *>(g_maps->_currentMap);
				map[VAL2]++;
				map.updateGame();
			}
		));
	}
}

void Map32::special04() {
	if (_data[PASSWORD_INDEX] & 0x80) {
		_data[PASSWORD_INDEX] = getRandomNumber(7) - 1;
	}

	Common::String line2 = Common::String::format("\"%s %s",
		STRING[Common::String::format("maps.map32.passwords.%d",
			_data[PASSWORD_INDEX])].c_str(),
		STRING["maps.map32.password"].c_str()
	);

	send(SoundMessage(
		0, 1, STRING["maps.map32.heratio"],
		0, 2, line2
	));
}

void Map32::special05() {
	if (!_data[VAL1]) {
		g_maps->_mapPos.x--;
		updateGame();
	}
}

void Map32::passwordEntered(const Common::String &password) {
	if ((_data[PASSWORD_INDEX] & 0x80) ||
		!password.equalsIgnoreCase(STRING[Common::String::format("maps.map32.passwords.%d",
			_data[PASSWORD_INDEX])])) {
		g_maps->_mapPos.x--;
		updateGame();

	} else {
		g_events->send(SoundMessage(STRING["maps.map32.correct"]));
		_data[VAL1]++;
		g_maps->clearSpecial();
	}
}

} // namespace Maps
} // namespace MM1
} // namespace MM
