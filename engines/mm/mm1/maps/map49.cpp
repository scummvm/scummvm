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

#include "mm/mm1/maps/map49.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 952
#define VAL2 154

void Map49::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 23; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[74 + i]) {			
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

void Map49::special00() {
	g_events->addView("MaidenPrisoner");
}

void Map49::special01() {
	visitedExit();

	send(SoundMessage(
		STRING["maps.passage_outside1"],
		[]() {
			g_maps->_mapPos = Common::Point(14, 7);
			g_maps->changeMap(0xb1a, 2);
		}
	));
}

void Map49::special02() {
	visitedExit();

	g_maps->_mapPos = Common::Point(15, 7);
	g_maps->changeMap(0xb1a, 2);
	send(SoundMessage(STRING["maps.map49.chute"]));
}

void Map49::special03() {
	send(SoundMessage(STRING["maps.map49.message_e"]));
}

void Map49::special04() {
	if (!g_globals->_party.hasItem(MERCHANTS_PASS_ID)) {
		send(SoundMessage(
			STRING["maps.map49.guards2"],
			[](const Common::KeyState &) {
				Map49 &map = *static_cast<Map49 *>(g_maps->_currentMap);
				g_maps->_mapPos = Common::Point(
					map[MAP_SURFACE_X], map[MAP_SURFACE_Y]);
				g_maps->changeMap(
					READ_LE_UINT16(&map[MAP_SURFACE_ID]),
					map[MAP_SURFACE_SECTION]);
			}
		));
	}
}

void Map49::special05() {
	if (!g_globals->_party.hasItem(KINGS_PASS_ID)) {
		g_maps->_mapPos.x++;
		updateGame();
		send(SoundMessage(STRING["maps.map49.guards2"]));

	} else {
		checkPartyDead();
	}
}

void Map49::special06() {
	if (_data[VAL1]) {
		g_maps->_mapPos = Common::Point(6, 0);
		g_maps->changeMap(0x412, 3);

	} else {
		send("View", DrawGraphicMessage(7 + 65));
		g_events->addView("Alamar");
	}
}

void Map49::special07() {
	visitedExit();

	reduceHP();
	reduceHP();
	g_maps->_mapPos = Common::Point(4, 7);
	g_maps->changeMap(0xb1a, 2);

	send(SoundMessage(STRING["maps.map49.catapult"]));
}

void Map49::special08() {
	// WORKAROUND: Fixed acid trap to check for acid protection
	if (!g_globals->_activeSpells._s.acid)
		reduceHP();
	reduceHP();

	messageEncounter(STRING["maps.map49.trap"]);
}

void Map49::special09() {
	// WORKAROUND: Fixed explosion trap to check for fire protection
	if (!g_globals->_activeSpells._s.fire)
		reduceHP();
	reduceHP();

	messageEncounter(STRING["maps.map49.explosion"]);
}

void Map49::special16() {
	send(SoundMessage(STRING["maps.map49.scream"]));
}

void Map49::special20() {
	g_globals->_treasure.setGold(12000);
	g_events->addAction(KEYBIND_SEARCH);
}

void Map49::special22() {
	send(SoundMessage(STRING["maps.map49.throne_room"]));
}

void Map49::messageEncounter(const Common::String &line) {
	SoundMessage msg(line,
		[]() {
			g_globals->_encounters.execute();
		}
	);
	msg._delaySeconds = 4;
	send(msg);
}

} // namespace Maps
} // namespace MM1
} // namespace MM
