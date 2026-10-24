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
#include "mm/mm1/utils/strings.h"

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
		"A passage leads outside, take it (Y/N)?",
		[]() {
			g_maps->_mapPos = Common::Point(12, 12);
			g_maps->changeMapFromExit(0x112, 2);
		}
	));
}

void Map44::special02() {
	visitedExit();

	send(SoundMessage(
		"Stairs going down! Take them (Y/N)?",
		[]() {
			g_maps->changeMapFromExit(0xf05, 3);
		}
	));
}

void Map44::special03() {
	send(SoundMessage(mapText("Etched in silver, message F reads:\ntstst,e1,d-ds15a1drh")));
}

void Map44::special04() {
	send(SoundMessage(mapText("The green clover grants those who\nare worthy +4 luck!")));

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		if (!(c._flags[11] & CHARFLAG11_GOT_LUCK)) {
			c._flags[11] |= CHARFLAG11_GOT_LUCK;
			if (c._luck._base < 30) {
				c._luck._current = c._luck._base =
					c._luck._base + 4;
			}
		}
	}
}

void Map44::special05() {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		c._flags[5] |= CHARFLAG5_10;
	}

	send(SoundMessage(
		mapText("The Fabulous Fabled Fountain of\n"
			"Dragadune converts all gold into\n"
			"experience, accept (Y/N)?"),
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
	send(SoundMessage(mapText("Carved in stone, a trivial message\nreads: i be me")));
}

void Map44::special07() {
	send(SoundMessage("Decayed bones crunch beneath you!"));
}

} // namespace Maps
} // namespace MM1
} // namespace MM
