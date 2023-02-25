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

#include "mm/mm1/maps/map34.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 1238

void Map34::special() {
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

void Map34::special00() {
	g_events->addView("ChildPrisoner");
}

void Map34::special01() {
	visitedExit();

	send(SoundMessage(
		STRING["maps.passage_outside1"],
		[]() {
			g_maps->_mapPos = Common::Point(7, 15);
			g_maps->changeMap(0xf01, 2);
		}
	));
}

void Map34::special02() {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		if ((c._flags[13] & CHARFLAG13_ALAMAR) || c.hasItem(EYE_OF_GOROS_ID)) {
			g_maps->clearSpecial();
			g_globals->_encounters.execute();
			return;
		}
	}

	Sound::sound2(SOUND_2);

	InfoMessage msg(
		0, 0, STRING["maps.map34.alamar1"],
		0, 7, STRING["maps.map34.alamar2"]
	);
	msg._largeMessage = true;
	send(msg);
}

void Map34::special03() {
	send(SoundMessage(STRING["maps.map34.message3"]));
}

void Map34::special04() {
	send(SoundMessage(STRING["maps.map34.message4"]));
}

void Map34::special05() {
	send(SoundMessage(STRING["maps.map34.message5"]));
}

void Map34::special06() {
	send(SoundMessage(STRING["maps.map34.message6"]));
}

void Map34::special07() {
	if (g_globals->_party.hasItem(GOLD_KEY_ID)) {
		checkPartyDead();
	} else {
		send(SoundMessage(STRING["maps.map34.door"]));
		g_maps->_mapPos.y--;
		updateGame();
	}
}

void Map34::special08() {
	send(SoundMessage(STRING["maps.map34.statue"]));
}

void Map34::special09() {
	send(SoundMessage(STRING["maps.map34.banner"]));
}

void Map34::special10() {
	g_maps->_mapPos.x++;
	g_maps->_mapPos.y--;
	updateGame();
}

void Map34::special12() {
	g_maps->_mapPos.x--;
	g_maps->_mapPos.y++;
	updateGame();
}

void Map34::special13() {
	send(SoundMessage(STRING["maps.map34.sign"]));
}

void Map34::special16() {
	send(SoundMessage(STRING["maps.map34.machine"]));
}

void Map34::special17() {
	send(SoundMessage(STRING["maps.map34.box"]));
	_data[MAP_47] = 9;
	_data[VAL1] = 9;
}

void Map34::special18() {
	send(SoundMessage(STRING["maps.map34.message1"]));
}

void Map34::special19() {
	send(SoundMessage(STRING["maps.map34.message2"]));
}

void Map34::special20() {
	if (_data[VAL1]) {
		g_maps->clearSpecial();
		g_globals->_encounters.execute();
	} else {
		none160();
	}
}

void Map34::special21() {
	send(SoundMessage(STRING["maps.map34.message7"]));
}

} // namespace Maps
} // namespace MM1
} // namespace MM
