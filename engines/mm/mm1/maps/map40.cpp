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

#include "mm/mm1/maps/map40.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 363
#define VAL2 831
#define GOLD 832

void Map40::special() {
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

	if (_data[VAL1]) {
		checkPartyDead();
	} else {
		_data[VAL2]++;
		g_maps->_mapPos.y++;
		updateGame();
		send(SoundMessage(STRING["maps.map40.conveyor_belt"]));
	}
}

void Map40::special00() {
	send(SoundMessage(STRING["maps.map40.message2"]));
}

void Map40::special01() {
	g_maps->clearSpecial();

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		g_globals->_party[i]._flags[7] |= CHARFLAG7_40;
	}

	g_events->addView("LordArcher");
}

void Map40::special02() {
	reduceHP();
	send(SoundMessage(STRING["maps.map40.giants"]));
}

void Map40::special03() {
	visitedExit();

	send(SoundMessage(
		STRING["maps.stairs_up"],
		[]() {
			g_maps->changeMap(0xf02, 3);
		}
	));
}

void Map40::special04() {
	send(SoundMessage(
		STRING["maps.map40.button"],
		[]() {
			Map40 &map = *static_cast<Map40 *>(g_maps->_currentMap);
			map[VAL1]++;
		}
	));
}

void Map40::special05() {
	encounter(&_data[588], &_data[601]);
}

void Map40::special06() {
	encounter(&_data[614], &_data[621]);
}

void Map40::special07() {
	encounter(&_data[628], &_data[640]);
}

void Map40::special08() {
	encounter(&_data[652], &_data[658]);
}

void Map40::special09() {
	encounter(&_data[664], &_data[675]);
}

void Map40::special10() {
	encounter(&_data[686], &_data[700]);
}

void Map40::special11() {
	encounter(&_data[714], &_data[726]);
}

void Map40::special12() {
	encounter(&_data[738], &_data[750]);
}

void Map40::special13() {
	encounter(&_data[762], &_data[776]);
}

void Map40::special14() {
	encounter(&_data[790], &_data[802]);
}

void Map40::special15() {
	if (_data[VAL2]) {
		_data[VAL2] = 0;
		reduceHP();
		reduceHP();
		send(SoundMessage(STRING["maps.map40.squish"]));

	} else {
		none160();
	}
}

void Map40::special16() {
	send(SoundMessage(STRING["maps.map40.boulder"]));
}

void Map40::special17() {
	send(SoundMessage(STRING["maps.map40.test1"]));
}

void Map40::special18() {
	send(SoundMessage(STRING["maps.map40.test2"]));
}

void Map40::special19() {
	send(SoundMessage(STRING["maps.map40.test3"]));
}

void Map40::special20() {
	send(SoundMessage(STRING["maps.map40.test4"]));
}

void Map40::archerResist() {
	Game::Encounter &enc = g_globals->_encounters;

	enc.clearMonsters();
	for (int i = 0; i < 6; ++i)
		enc.addMonster(12, 10);
	enc.addMonster(15, 12);

	enc._manual = true;
	enc._levelIndex = 112;
	enc.execute();
}

void Map40::archerSubmit() {
	// As long as even one character has gold, Archer will take
	// all of the party's gold. However, if the party is penniless,
	// then Archer will actually give each character 5000 gold
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		if (g_globals->_party[i]._gold) {
			WRITE_LE_UINT16(&_data[GOLD], 0);
			break;
		}
	}

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		g_globals->_party[i]._gold = READ_LE_UINT16(&_data[GOLD]);
	}

	g_maps->_mapPos = Common::Point(8, 5);
	g_maps->changeMap(0x604, 1);
}

} // namespace Maps
} // namespace MM1
} // namespace MM
