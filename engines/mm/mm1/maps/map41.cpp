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
#include "mm/mm1/sound.h"

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
	send(SoundMessage(STRING["maps.map41.message3"]));
}

void Map41::special01() {
	visitedExit();

	send(SoundMessage(
		STRING["maps.map41.stairs_up"],
		[]() {
			g_maps->_mapPos = Common::Point(14, 2);
			g_maps->changeMap(0x101, 2);
		}
	));
}

void Map41::special02() {
	visitedExit();

	send(SoundMessage(
		STRING["maps.stairs_down"],
		[]() {
			g_maps->changeMap(0x704, 3);
		}
	));
}

void Map41::special03() {
	special02();
}

void Map41::special04() {
	send(SoundMessage(STRING["maps.map41.dung"]));
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
	showSign(STRING["maps.map41.sign1"]);
}

void Map41::special17() {
	showSign(STRING["maps.map41.sign2"]);
}

void Map41::special18() {
	showSign(STRING["maps.map41.sign3"]);
}

void Map41::special19() {
	showSign(STRING["maps.map41.sign4"]);
}

void Map41::special20() {
	showSign(STRING["maps.map41.sign5"]);
}

void Map41::special21() {
	send(SoundMessage(STRING["maps.wall_painted"]));
}

void Map41::special22() {
	send(SoundMessage(STRING["maps.map41.tapestry1"]));
}

void Map41::special23() {
	send(SoundMessage(STRING["maps.map41.tapestry2"]));
}

void Map41::showSign(const Common::String &line) {
	send(SoundMessage(
		0, 1, STRING["maps.sign"],
		0, 2, line
	));
}

} // namespace Maps
} // namespace MM1
} // namespace MM
