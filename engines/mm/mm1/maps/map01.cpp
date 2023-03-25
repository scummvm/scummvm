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

#include "mm/mm1/maps/map01.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

void Map01::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 17; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[68 + i]) {
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

void Map01::special00() {
	inn();
}

void Map01::special01() {
	Common::String line1 = STRING["maps.map01.zam1"];
	Common::String line2 = STRING["maps.map01.zam3"];

	bool hasCourier = false;
	for (uint i = 0; i < g_globals->_party.size() && !hasCourier; ++i) {
		hasCourier = (g_globals->_party[i]._flags[0] & CHARFLAG0_COURIER3) != 0;
	}

	if (hasCourier) {
		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			Character &c = g_globals->_party[i];
			c._flags[0] |= CHARFLAG0_COURIER3 | CHARFLAG0_ZAM_CLUE;
		}

		line2 = STRING["maps.map01.zam2"];
	}

	send(InfoMessage(
		0, 1, line1,
		0, 3, line2
	));
}

void Map01::special02() {
	blacksmith();
}

void Map01::special03() {
	market();
}

void Map01::special04() {
	visitedExit();
	send(SoundMessage(
		STRING["maps.passage_outside2"],
		[]() {
			g_maps->_mapPos = Common::Point(3, 3);
			g_maps->changeMap(0x101, 2);
		}
	));
}

void Map01::special05() {
	tavern();
}

void Map01::special06() {
	temple();
}

void Map01::special07() {
	training();
}

void Map01::special08() {
	// Paralyze all the men
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		if (c._sex == MALE && !(c._condition & BAD_CONDITION))
			c._condition = PARALYZED;
	}

	redrawGame();
	
	// Show the message and wait for a keypress
	send(SoundMessage(
		STRING["maps.map01.secret"],
		[](const Common::KeyState &) {
			g_events->close();
			Game::Encounter &enc = g_globals->_encounters;
			enc.clearMonsters();

			uint count = g_events->getRandomNumber(4) + 4;
			enc.addMonster(6, 12);
			for (uint i = 0; i < count; ++i)
				enc.addMonster(4, 9);

			enc._manual = true;
			enc._levelIndex = 80;
			enc.execute();
		}
	));
}

void Map01::special09() {
	visitedExit();
	send(SoundMessage(
		STRING["maps.stairs_down"],
		[]() {
			g_maps->_mapPos = Common::Point(8, 0);
			g_maps->changeMap(0xc01, 1);
		}
	));
}

void Map01::special10() {
	showSign(STRING["maps.map01.market"]);
}

void Map01::special11() {
	showSign(STRING["maps.map01.blacksmith"]);
}

void Map01::special12() {
	showSign(STRING["maps.map01.inn"]);
}

void Map01::special13() {
	showSign(STRING["maps.map01.temple"]);
}

void Map01::special14() {
	send(SoundMessage(
		0, 1, STRING["maps.map01.zam0"]
	));
}

void Map01::special15() {
	showSign(STRING["maps.map01.tavern"]);
}

void Map01::special16() {
	showSign(STRING["maps.map01.training"]);
}

} // namespace Maps
} // namespace MM1
} // namespace MM
