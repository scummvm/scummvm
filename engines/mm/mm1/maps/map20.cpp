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

#include "mm/mm1/maps/map20.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 177

void Map20::special() {
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

	if (_states[g_maps->_mapOffset]) {
		g_maps->clearSpecial();
		enc.execute();

	} else if (getRandomNumber(100) != 100) {
		none160();

	} else {
		int id1 = getRandomNumber(5);
		int monsterCount = getRandomNumber(13);

		enc.clearMonsters();
		for (int i = 0; i < monsterCount; ++i)
			enc.addMonster(id1, 11);

		enc._flag = true;
		enc._levelIndex = 80;
		enc.execute();
	}
}

void Map20::special00() {
	send(SoundMessage(
		STRING["maps.map20.stairs_down"],
		[]() {
			g_maps->_mapPos = Common::Point(1, 15);
			g_maps->changeMap(0xc03, 1);
		}
	));
}

void Map20::special01() {
	send(SoundMessage(
		STRING["maps.map20.cave"],
		[]() {
			g_maps->_mapPos = Common::Point(15, 0);
			g_maps->changeMap(0x51b, 1);
		}
	));
}

void Map20::special02() {
	if (_data[VAL1] & 0x80) {
		g_maps->_mapPos = Common::Point(15, 8);
		g_maps->changeMap(0xa11, 3);

	} else if (_data[VAL1]) {
		_data[VAL1] = 0;

	} else {
		send(SoundMessage(
			STRING["maps.map20.cave"],
			[](const Common::KeyState &ks) {
				Map20 &map = *static_cast<Map20 *>(g_maps->_currentMap);
				if (ks.keycode == Common::KEYCODE_y) {
					g_events->close();
					map[VAL1] = 0xff;
				} else if (ks.keycode == Common::KEYCODE_n) {
					g_events->close();
					map[VAL1]++;
				}
			}
		));
	}
}

void Map20::special03() {
	// TODO
}

void Map20::special04() {
	send(SoundMessage(STRING["maps.map20.sign1"]));
}

void Map20::special05() {
	send(SoundMessage(STRING["maps.map20.sign2"]));
}

void Map20::special06() {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		g_globals->_currCharacter = &g_globals->_party[i];
		g_globals->_currCharacter->_flags[5] |= CHARFLAG5_2;
	}

	SoundMessage info(0, 0, STRING["maps.map20.peak"]);
	info._largeMessage = true;
	send(info);
}

void Map20::special07() {
	g_maps->_mapPos.y = 6;
	g_events->send("Game", GameMessage("UPDATE"));
}

void Map20::special08() {
	g_maps->_mapPos.x = 9;
	g_events->send("Game", GameMessage("UPDATE"));
}

} // namespace Maps
} // namespace MM1
} // namespace MM
