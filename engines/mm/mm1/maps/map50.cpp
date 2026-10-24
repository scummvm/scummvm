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

#include "mm/mm1/maps/map50.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"
#include "mm/mm1/utils/strings.h"

namespace MM {
namespace MM1 {
namespace Maps {

void Map50::special() {
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

void Map50::special00() {
	send(SoundMessage(g_engine->isEnhanced() ?
		"Etched in gold, message 7 reads:\n"
			"These-and-that-must-can-\n"
			"different-shall-have-from-a-jvc" :
		"Etched in gold, message 7 reads:\n"
			"These-and-that-must-can-different-\n"
			"shall-have-from-a-jvc"));
}

void Map50::special01() {
	visitedExit();

	send(SoundMessage(
		"A passage leads outside, take it (Y/N)?",
		[]() {
			g_maps->_mapPos = Common::Point(10, 5);
			g_maps->changeMapFromExit(0x11b, 2);
		}
	));
}

void Map50::special02() {
	visitedExit();

	send(SoundMessage(
		"Stairs going down! Take them (Y/N)?",
		[]() {
			g_maps->changeMapFromExit(0x701, 3);
		}
	));
}

void Map50::special03() {
	visitedExit();

	send(SoundMessage(
		"Stairs going down! Take them (Y/N)?",
		[]() {
			g_maps->changeMapFromExit(0xe00, 3);
		}
	));
}

void Map50::special04() {
	visitedExit();

	send(SoundMessage(
		"Stairs going down! Take them (Y/N)?",
		[]() {
			g_maps->changeMapFromExit(0x201, 3);
		}
	));
}

void Map50::special05() {
	encounter(&_data[427], &_data[440]);
}

void Map50::special06() {
	encounter(&_data[453], &_data[464]);
}

void Map50::special07() {
	encounter(&_data[475], &_data[486]);
}

void Map50::special08() {
	encounter(&_data[497], &_data[500]);
}

void Map50::special09() {
	encounter(&_data[503], &_data[516]);
}

void Map50::special10() {
	encounter(&_data[529], &_data[542]);
}

void Map50::special11() {
	encounter(&_data[555], &_data[569]);
}

void Map50::special12() {
	encounter(&_data[583], &_data[597]);
}

void Map50::special13() {
	encounter(&_data[611], &_data[626]);
}

void Map50::special14() {
	encounter(&_data[641], &_data[647]);
}

void Map50::special15() {
	encounter(&_data[653], &_data[664]);
}

void Map50::special16() {
	send(SoundMessage(mapText("Club Dead...\nThe antidote to humanization!")));
}

void Map50::special17() {
	send(SoundMessage("Temple of the Old Order"));
}

void Map50::special18() {
	send(SoundMessage("Glass Room"));
}

void Map50::special19() {
	send(SoundMessage("Ruby Room"));
}

void Map50::special20() {
	send(SoundMessage("Twinkle Room"));
}

void Map50::special21() {
	send(SoundMessage("Rainbow Room"));
}

void Map50::special22() {
	send(SoundMessage(mapText("The wall is painted from ceiling to\n"
		"floor in a black and white checkered\n"
		"pattern!")));
}

void Map50::special23() {
	if (!(g_globals->_party.hasItem(CRYSTAL_KEY_ID))) {
		g_maps->_mapPos.y--;
		updateGame();

		send(SoundMessage("The crystal grate repels you!"));
	}
}

} // namespace Maps
} // namespace MM1
} // namespace MM
