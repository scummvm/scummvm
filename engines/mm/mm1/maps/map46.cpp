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

#include "mm/mm1/maps/map46.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

void Map46::special() {
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

	// All other cells on the map are encounters
	g_maps->clearSpecial();
	g_globals->_encounters.execute();
}

void Map46::special00() {
	send(SoundMessage(STRING["maps.wall_painted"]));
}

void Map46::special01() {
	send(SoundMessage(STRING["maps.map46.shakes"]));
}

void Map46::special02() {
	visitedExit();

	send(SoundMessage(
		STRING["maps.stairs_down"],
		[]() {
			g_maps->changeMap(0x705, 3);
		}
	));
}

void Map46::special03() {
	visitedExit();

	send(SoundMessage(
		STRING["maps.stairs_up"],
		[]() {
			g_maps->changeMap(0xf05, 3);
		}
	));
}

void Map46::special04() {
	special03();
}

void Map46::special05() {
	g_maps->_mapPos.x++;
	updateGame();
}

void Map46::special06() {
	send(SoundMessage(STRING["maps.map46.clerics"]));
}

} // namespace Maps
} // namespace MM1
} // namespace MM
