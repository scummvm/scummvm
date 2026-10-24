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

#include "mm/mm1/maps/map41.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"
#include "mm/mm1/utils/strings.h"

namespace MM {
namespace MM1 {
namespace Maps {

void Map41::special() {
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

	checkPartyDead();
}

void Map41::special00() {
	send(SoundMessage(g_engine->isEnhanced() ?
		"Etched in gold, message 3 reads:\n"
			"For-tasks-and-rating.-Be-from-\n"
			"6th-frustrations!-The-to-in" :
		"Etched in gold, message 3 reads:\n"
			"For-tasks-and-rating.-be-from-6th-\n"
			"frustrations!-the-to-in"));
}

void Map41::special01() {
	visitedExit();

	send(SoundMessage(
		g_engine->isEnhanced() ?
			"Spiralling stairs go up, take them (Y/N)?" :
			"Spiralling stairs go up,take them (Y/N)?",
		[]() {
			g_maps->_mapPos = Common::Point(14, 2);
			g_maps->changeMapFromExit(0x101, 2);
		}
	));
}

void Map41::special02() {
	visitedExit();

	send(SoundMessage(
		"Stairs going down! Take them (Y/N)?",
		[]() {
			g_maps->changeMapFromExit(0x704, 3);
		}
	));
}

void Map41::special03() {
	special02();
}

void Map41::special04() {
	send(SoundMessage("Minotaur dung slushes beneath you!"));
}

void Map41::special05() {
	encounter(&_data[509], &_data[519]);
}

void Map41::special06() {
	encounter(&_data[529], &_data[542]);
}

void Map41::special07() {
	encounter(&_data[555], &_data[563]);
}

void Map41::special08() {
	encounter(&_data[571], &_data[586]);
}

void Map41::special09() {
	encounter(&_data[601], &_data[613]);
}

void Map41::special10() {
	encounter(&_data[625], &_data[633]);
}

void Map41::special11() {
	encounter(&_data[641], &_data[650]);
}

void Map41::special12() {
	encounter(&_data[659], &_data[670]);
}

void Map41::special13() {
	encounter(&_data[683], &_data[691]);
}

void Map41::special14() {
	encounter(&_data[699], &_data[710]);
}

void Map41::special16() {
	showSign("The Enchanted Chamber");
}

void Map41::special17() {
	showSign("Cleptomian Guild, do not disturb!");
}

void Map41::special18() {
	showSign("The Council of Strength is in session");
}

void Map41::special19() {
	showSign("The Uglies Club - Members only!");
}

void Map41::special20() {
	showSign("Specimen Tank - keep closed");
}

void Map41::special21() {
	send(SoundMessage(mapText("The wall is painted from ceiling to\n"
		"floor in a black and white checkered\n"
		"pattern!")));
}

void Map41::special22() {
	send(SoundMessage(mapText("A tapestry on the wall depicts a gray\n"
		"minotaur stomping on a group of knights.")));
}

void Map41::special23() {
	send(SoundMessage(mapText("A tapestry on the wall depicts\na kingdom ruled by minotaurs")));
}

void Map41::showSign(const Common::String &line) {
	send(SoundMessage(
		0, 1, mapText("A sign above the door reads:\n"),
		0, 2, line
	));
}

} // namespace Maps
} // namespace MM1
} // namespace MM
