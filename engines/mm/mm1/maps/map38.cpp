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

#include "mm/mm1/maps/map38.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

void Map38::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 19; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[70 + i]) {
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	g_maps->_mapPos = Common::Point(getRandomNumber(16) - 1,
		getRandomNumber(16) - 1);
	send(SoundMessage(STRING["maps.map38.ringing"]));
}

void Map38::special00() {
	send(SoundMessage(STRING["maps.map38.message4"]));
}

void Map38::special01() {
	send(SoundMessage(STRING["maps.wall_painted"]));
}

void Map38::special02() {
	special03();
}

void Map38::special03() {
	visitedExit();

	send(SoundMessage(
		STRING["maps.stairs_up"],
		[]() {
			g_maps->_mapPos.x = 0;
			g_maps->changeMap(0xf03, 3);
		}
	));
}

void Map38::special04() {
	send(SoundMessage(STRING["maps.map38.banner"]));
}

void Map38::special05() {
	encounter(&_data[669], &_data[682]);
}

void Map38::special06() {
	encounter(&_data[690], &_data[705]);
}

void Map38::special07() {
	encounter(&_data[715], &_data[724]);
}

void Map38::special08() {
	encounter(&_data[733], &_data[743]);
}

void Map38::special09() {
	encounter(&_data[753], &_data[764]);
}

void Map38::special10() {
	encounter(&_data[775], &_data[788]);
}

void Map38::special11() {
	encounter(&_data[801], &_data[816]);
}

void Map38::special12() {
	encounter(&_data[831], &_data[840]);
}

void Map38::special13() {
	encounter(&_data[849], &_data[862]);
}

void Map38::special14() {
	encounter(&_data[875], &_data[890]);
}

void Map38::special15() {
	send(SoundMessage(STRING["maps.map38.face1"]));
}

void Map38::special16() {
	send(SoundMessage(STRING["maps.map38.face2"]));
}

void Map38::special17() {
	send(SoundMessage(STRING["maps.map38.face3"]));
}

void Map38::special18() {
	send(SoundMessage(STRING["maps.map38.face4"]));
}

} // namespace Maps
} // namespace MM1
} // namespace MM
