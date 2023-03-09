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

#include "mm/mm1/maps/map00.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define TOWN_NUM 0x2fe
#define LEPRECHAUN_MAP_ID1 0x2ff
#define LEPRECHAUN_MAP_ID2 0x304
#define LEPRECHAUN_MAP_X 0x309
#define LEPRECHAUN_MAP_Y 0x30E
#define STATUE_VAL 0x412

void Map00::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 24; ++i) {
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

	// All other cells on the map are encounters
	g_maps->clearSpecial();
	g_globals->_encounters.execute();
}

void Map00::special00() {
	inn();
}

void Map00::special01() {
	Common::String line2;
	int x = 5;

	switch (g_maps->_forwardMask) {
	case DIRMASK_E:
		line2 = STRING["maps.map00.market"];
		x = 6;
		break;
	case DIRMASK_W:
		line2 = STRING["maps.map00.blacksmith"];
		break;
	default:
		line2 = STRING["maps.map00.inn"];
		break;
	}

	send(SoundMessage(
		2, 0, STRING["maps.sign"],
		x, 1, line2
	));
}

void Map00::special02() {
	blacksmith();
}

void Map00::special03() {
	market();
}

void Map00::special04() {
	visitedExit();
	send(SoundMessage(
		STRING["maps.passage_outside1"],
		[]() {
			g_maps->_mapPos = Common::Point(10, 10);
			g_maps->changeMap(0xa11, 2);
		}
	));
}

void Map00::special05() {
	tavern();
}

void Map00::special06() {
	temple();
}

void Map00::special07() {
	training();
}

void Map00::special08() {
	SoundMessage msg(
		STRING["maps.map00.leprechaun"],
		[](const Common::KeyState &keyState) {
			Maps &maps = *g_maps;
			Map &map = *g_maps->_currentMap;

			switch (keyState.keycode) {
			case Common::KEYCODE_ESCAPE:
				maps.turnAround();
				g_events->focusedView()->close();
				break;
			case Common::KEYCODE_1:
			case Common::KEYCODE_2:
			case Common::KEYCODE_3:
			case Common::KEYCODE_4:
			case Common::KEYCODE_5:
				g_events->focusedView()->close();
				map[TOWN_NUM] = keyState.ascii;

				for (uint i = 0; i < g_globals->_party.size(); ++i) {
					Character &c = g_globals->_party[i];
					if (c._gems) {
						c._gems--;

						int townIndex = map[TOWN_NUM] - Common::KEYCODE_1;
						maps._mapPos.x = map[LEPRECHAUN_MAP_X + townIndex];
						maps._mapPos.y = map[LEPRECHAUN_MAP_Y + townIndex];
						maps.changeMap(
							map[LEPRECHAUN_MAP_ID1 + townIndex] |
							(map[LEPRECHAUN_MAP_ID2 + townIndex] << 8),
							1);

						g_events->redraw();
						return;
					}
				}

				maps._mapPos = Common::Point(8, 5);
				break;
			default:
				break;
			}
		}
	);

	msg._largeMessage = true;
	send(msg);
}

void Map00::special09() {
	visitedExit();
	send(SoundMessage(
		STRING["maps.stairs_down"],
		[]() {
			g_maps->changeMap(0xa11, 1);
		}
	));
}

void Map00::special10() {
	Map &map = *g_maps->_currentMap;
	map[STATUE_VAL] = 0;
	searchStatue();
}

void Map00::special11() {
	Map &map = *g_maps->_currentMap;
	map[STATUE_VAL] = 1;
	searchStatue();
}

void Map00::special12() {
	Map &map = *g_maps->_currentMap;
	map[STATUE_VAL] = 2;
	searchStatue();
}

void Map00::special13() {
	Map &map = *g_maps->_currentMap;
	map[STATUE_VAL] = 3;
	searchStatue();
}

void Map00::special14() {
	Map &map = *g_maps->_currentMap;
	map[STATUE_VAL] = 4;
	searchStatue();
}

void Map00::special15() {
	Map &map = *g_maps->_currentMap;
	map[STATUE_VAL] = 5;
	searchStatue();
}

void Map00::special16() {
	Map &map = *g_maps->_currentMap;
	map[STATUE_VAL] = 6;
	searchStatue();
}

void Map00::special17() {
	Map &map = *g_maps->_currentMap;
	map[STATUE_VAL] = 7;
	searchStatue();
}

void Map00::special18() {
	send(SoundMessage(
		2, 0, STRING["maps.sign"],
		6, 1, STRING["maps.map00.temple"]
	));
}

void Map00::special19() {
	send(SoundMessage(
		2, 0, STRING["maps.sign"],
		6, 1, STRING["maps.map00.jail"]
	));
}

void Map00::special20() {
	send(SoundMessage(
		2, 0, STRING["maps.sign"],
		6, 1, STRING["maps.map00.tavern"]
	));
}
void Map00::special21() {
	send(SoundMessage(
		2, 0, STRING["maps.sign"],
		6, 1, STRING["maps.map00.training"]
	));
}

void Map00::special22() {
	Map &map = *g_maps->_currentMap;
	map[MM1::Maps::MAP_47] = 3;
	map[MM1::Maps::MAP_33] = 6;
	g_maps->clearSpecial();
}

void Map00::special23() {
	visitedExit();
	Common::String msg = STRING["maps.map00.trapdoor"];
	if (g_globals->_activeSpells._s.levitate)
		msg += STRING["maps.map00.levitate"];

	send(SoundMessage(msg,
		[](const Common::KeyState &keyState) {
			g_events->focusedView()->close();
			if (!g_globals->_activeSpells._s.levitate)
				g_maps->changeMap(0xa11, 1);
		}
	));
}

void Map00::searchStatue() {
	send(SoundMessage(
		STRING["maps.map00.statue"],
		[]() {
			Map &map = *g_maps->_currentMap;
			g_events->send("Statue", GameMessage("STATUE", map[STATUE_VAL]));
		}
	));
}

} // namespace Maps
} // namespace MM1
} // namespace MM
