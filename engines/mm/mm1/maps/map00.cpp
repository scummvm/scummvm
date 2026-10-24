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
#include "mm/mm1/utils/strings.h"

namespace MM {
namespace MM1 {
namespace Maps {

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
		line2 = "\"Eulards Fine Foods\"";
		x = 6;
		break;
	case DIRMASK_W:
		line2 = "\"B and B Blacksmithing\"";
		break;
	default:
		line2 = "\"The Inn of Sorpigal\"";
		break;
	}

	send(SoundMessage(
		2, 0, mapText("A sign above the door reads:\n"),
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
		"A passage leads outside, take it (Y/N)?",
		[]() {
			g_maps->_mapPos = Common::Point(10, 10);
			g_maps->changeMapFromExit(0xa11, 2);
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
	g_events->addView("Leprechaun");
}

void Map00::special09() {
	visitedExit();
	send(SoundMessage(
		"Stairs going down! Take them (Y/N)?",
		[]() {
			g_maps->changeMapFromExit(0xa11, 1);
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
		2, 0, mapText("A sign above the door reads:\n"),
		6, 1, "Temple Moonshadow"
	));
}

void Map00::special19() {
	send(SoundMessage(
		2, 0, mapText("A sign above the door reads:\n"),
		6, 1, "Jail. Keep Out!"
	));
}

void Map00::special20() {
	send(SoundMessage(
		2, 0, mapText("A sign above the door reads:\n"),
		6, 1, "Ye Olde Hogge Tavern"
	));
}
void Map00::special21() {
	send(SoundMessage(
		2, 0, mapText("A sign above the door reads:\n"),
		6, 1, "Otto's Training"
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
	Common::String msg = "Trap door!";
	if (g_globals->_activeSpells._s.levitate)
		msg += ", levitation saves you!";

	send(SoundMessage(msg,
		[](const Common::KeyState &keyState) {
			g_events->focusedView()->close();
			if (!g_globals->_activeSpells._s.levitate)
				g_maps->changeMapFromExit(0xa11, 1);
		}
	));
}

void Map00::searchStatue() {
	send(SoundMessage(
		"There is a statue here. Search it (Y/N)?",
		[]() {
			Map &map = *g_maps->_currentMap;
			g_events->send("Statue", GameMessage("STATUE", map[STATUE_VAL]));
		}
	));
}

} // namespace Maps
} // namespace MM1
} // namespace MM
