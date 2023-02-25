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

#include "mm/mm1/maps/map44.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

void Map44::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 11; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[62 + i]) {				
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

void Map44::special00() {
	g_events->addView("MutatedPrisoner");
}

void Map44::special01() {
	visitedExit();

	send(SoundMessage(
		STRING["maps.passage_outside1"],
		[]() {
			g_maps->_mapPos = Common::Point(12, 12);
			g_maps->changeMap(0x112, 2);
		}
	));
}

void Map44::special02() {
	visitedExit();

	send(SoundMessage(
		STRING["maps.stairs_down"],
		[]() {
			g_maps->changeMap(0xf05, 3);
		}
	));
}

void Map44::special03() {
	send(SoundMessage(STRING["maps.map44.message_f"]));
}

void Map44::special04() {
	send(SoundMessage(STRING["maps.map44.clover"]));

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		if (!(c._flags[11] & CHARFLAG11_GOT_LUCK)) {
			c._flags[11] |= CHARFLAG11_GOT_LUCK;
			c._luck._current = c._luck._base = c._luck._base + 4;
		}
	}
}

void Map44::special05() {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		c._flags[5] |= CHARFLAG5_10;
	}

	send(SoundMessage(
		STRING["maps.map44.fountain"],
		[]() {
			Map44 &map = *static_cast<Map44 *>(g_maps->_currentMap);

			for (uint i = 0; i < g_globals->_party.size(); ++i) {
				Character &c = g_globals->_party[i];
				c._exp += c._gold;
				c._gold = 0;
			}

			map.none160();
		}
	));
}

void Map44::special06() {
	send(SoundMessage(STRING["maps.map44.message"]));
}

void Map44::special07() {
	send(SoundMessage(STRING["maps.map44.bones"]));
}

} // namespace Maps
} // namespace MM1
} // namespace MM
