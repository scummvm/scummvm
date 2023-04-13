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

#include "mm/mm1/maps/map37.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 232

void Map37::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 20; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[71 + i]) {				
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	// Randomly spin the party
	int count = getRandomNumber(4) - 1;
	for (int i = 0; i < count; ++i)
		g_maps->turnLeft();

	send(SoundMessage(STRING["maps.map37.spins"]));
}

void Map37::special00() {
	SoundMessage msg(STRING["maps.map37.message1"]);
	msg._fontReduced = true;
	send(msg);
}

void Map37::special01() {
	if (_data[VAL1]) {
		none160();
	} else {
		visitedExit();

		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			g_globals->_party[i]._flags[5] |= CHARFLAG5_1;
		}

		send(SoundMessage(
			STRING["maps.map37.opening"],
			[]() {
				g_maps->_mapPos = Common::Point(13, 5);
				g_maps->changeMap(0xa00, 2);
			}
		));
	}
}

void Map37::special02() {
	visitedExit();

	send(SoundMessage(
		STRING["maps.stairs_down"],
		[]() {
			g_maps->_mapPos.x = 15;
			g_maps->changeMap(0x703, 3);
		}
	));
}

void Map37::special03() {
	special02();
}

void Map37::special04() {
	Game::Encounter &enc = g_globals->_encounters;
	g_maps->clearSpecial();

	if (_data[VAL1]) {
		g_events->addView("Ghost");

	} else {
		_data[VAL1]++;

		enc.clearMonsters();
		enc.addMonster(19, 12);
		for (int i = 1; i < 4; ++i)
			enc.addMonster(14, 8);
		enc.addMonster(16, 12);

		enc._manual = true;
		enc._levelIndex = 80;
		enc.execute();
	}
}

void Map37::special05() {
	encounter(&_data[553], &_data[562]);
}

void Map37::special06() {
	encounter(&_data[571], &_data[586]);
}

void Map37::special07() {
	encounter(&_data[601], &_data[615]);
}

void Map37::special08() {
	encounter(&_data[629], &_data[641]);
}

void Map37::special09() {
	encounter(&_data[653], &_data[662]);
}

void Map37::special10() {
	encounter(&_data[671], &_data[681]);
}

void Map37::special11() {
	encounter(&_data[691], &_data[699]);
}

void Map37::special12() {
	encounter(&_data[707], &_data[714]);
}

void Map37::special13() {
	encounter(&_data[721], &_data[730]);
}

void Map37::special14() {
	encounter(&_data[739], &_data[752]);
}

void Map37::special15() {
	send(SoundMessage(STRING["maps.wall_painted"]));

	if (!g_globals->_party.hasItem(B_QUEEN_IDOL_ID)) {
		g_globals->_treasure._items[2] = B_QUEEN_IDOL_ID;
	}
}

void Map37::special16() {
	g_maps->clearSpecial();
	g_globals->_treasure._container = GOLD_BOX;
	g_globals->_treasure.setGems(100);
	g_events->addAction(KEYBIND_SEARCH);
}

void Map37::special19() {
	send(SoundMessage(STRING["maps.map37.archway"]));
}

} // namespace Maps
} // namespace MM1
} // namespace MM
