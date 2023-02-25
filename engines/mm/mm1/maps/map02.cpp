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

#include "mm/mm1/maps/map02.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

static const byte PORTAL_DEST_X[4] = { 8, 15, 0, 5 };
static const byte PORTAL_DEST_Y[4] = { 4, 0, 0, 14 };
static const uint16 PORTAL_DEST_ID[4] = { 0xc03, 0xa00, 0xf01, 0x604 };
static const byte PORTAL_DEST_SECTION[4] = { 3, 0, 1, 4 };

void Map02::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 22; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[73 + i]) {
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

void Map02::special00() {
	inn();
}

void Map02::special01() {
	Common::String line1 = STRING["maps.map02.zom1"];
	Common::String line2 = STRING["maps.map02.zom3"];

	bool hasCourier = false;
	for (uint i = 0; i < g_globals->_party.size() && !hasCourier; ++i) {
		hasCourier = (g_globals->_party[i]._flags[0] & CHARFLAG0_COURIER3) != 0;
	}

	if (hasCourier) {
		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			Character &c = g_globals->_party[i];
			c._flags[0] |= CHARFLAG0_COURIER3 | CHARFLAG0_ZOM_CLUE;
		}

		line2 = STRING["maps.map02.zom2"];
	}

	send(InfoMessage(
		0, 1, line1,
		0, 3, line2
	));

}

void Map02::special02() {
	blacksmith();
}

void Map02::special03() {
	market();
}

void Map02::special04() {
	visitedExit();
	send(SoundMessage(
		STRING["maps.passage_outside2"],
		[]() {
			g_maps->_mapPos = Common::Point(7, 7);
			g_maps->changeMap(0x801, 2);
		}
	));
}

void Map02::special05() {
	tavern();
}

void Map02::special06() {
	temple();
}

void Map02::special07() {
	training();
}

void Map02::special08() {
	g_events->addView("Resistances");
}

void Map02::special09() {
	showSign(STRING["maps.map02.docks"]);
}

void Map02::special10() {
	showSign(STRING["maps.map02.market"]);
}

void Map02::special11() {
	showSign(STRING["maps.map02.blacksmith"]);
}

void Map02::special12() {
	showSign(STRING["maps.map02.inn"]);
}

void Map02::special13() {
	send(SoundMessage(0, 1, STRING["maps.map02.zom0"]));
}

void Map02::special14() {
	send(SoundMessage(
		STRING["maps.map02.pit"],
		[]() {
			g_maps->_mapPos.x++;
			g_globals->_encounters.execute();
		}
	));
}

void Map02::special15() {
	visitedExit();
	send(SoundMessage(
		STRING["maps.map02.portal"],
		[]() {
			int index = g_maps->_mapPos.x - 9;
			g_maps->_mapPos.x = PORTAL_DEST_X[index];
			g_maps->_mapPos.y = PORTAL_DEST_Y[index];

			uint16 id = PORTAL_DEST_ID[index];
			byte section = PORTAL_DEST_SECTION[index];
			g_maps->changeMap(id, section);
		}
	));
}

void Map02::special18() {
	showSign(STRING["maps.map02.temple"]);
}

void Map02::special20() {
	showSign(STRING["maps.map02.tavern"]);
}

void Map02::special21() {
	showSign(STRING["maps.map02.training"]);
}

} // namespace Maps
} // namespace MM1
} // namespace MM
