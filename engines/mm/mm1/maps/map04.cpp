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

#include "mm/mm1/maps/map04.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

void Map04::special() {
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

	// Stealable treasure
	Sound::sound(SOUND_2);
	send(InfoMessage(
		STRING["maps.map04.treasure"],
		[]() {
			g_maps->clearSpecial();
			if ((*g_maps->_currentMap)[MAP04_TREASURE_STOLEN] < 255)
				(*g_maps->_currentMap)[MAP04_TREASURE_STOLEN]++;
			g_globals->_treasure[6] = 88;
			g_globals->_treasure[7] = 2;
			g_globals->_treasure[8] = 10;
			g_events->addAction(KEYBIND_SEARCH);
		}
	));
}

void Map04::special00() {
	inn();
}

void Map04::special01() {
	bool hasCourier = false;
	for (uint i = 0; i < g_globals->_party.size() && !hasCourier; ++i) {
		hasCourier = (g_globals->_party[i]._flags[0] & CHARFLAG0_COURIER1) != 0;
	}

	bool hasScroll = false;
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		if (c._equipped.indexOf(231) != -1) {
			hasScroll = true;
			c._gold += 1500;
			break;
		}
	}

	if (hasCourier && hasScroll) {
		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			Character &c = g_globals->_party[i];
			g_globals->_currCharacter = &c;
			c._exp += 1000;
			c._flags[0] &= (c._flags[0] & CHARFLAG0_COURIER1) | CHARFLAG0_COURIER2;
		}

		send(InfoMessage(
			0, 0, STRING["maps.map04.agar1"],
			0, 1, STRING["maps.map04.agar2"],
			[](const Common::KeyState &) {
			}
		));
	} else {
		send(InfoMessage(
			0, 0, STRING["maps.map03.telgoran1"],
			0, 1, STRING["maps.map03.agar3"]
		));
	}
}

void Map04::special02() {
	blacksmith();
}

void Map04::special03() {
	market();
}

void Map04::special04() {
	if (_data[MAP04_PASSAGE_OVERRIDE] || _data[MAP04_TREASURE_STOLEN] == 0) {
		Sound::sound(SOUND_2);
		send(InfoMessage(
			STRING["maps.map04.passage_outside"],
			[]() {
				g_maps->_mapPos.x = 13;
				g_maps->_mapPos.y = 1;
				g_maps->changeMap(0xa00, 2);
			}
		));
	} else {
		// Sucks to be you
		g_events->addView("Arrested");
	}
}

void Map04::special05() {
	tavern();
}

void Map04::special06() {
	temple();
}

void Map04::special07() {
	training();
}

void Map04::special08() {
	// Pick random run destination. This seems incredibly dangerous,
	// since couldn't it end up with the party in a wall?
	g_maps->_mapPos.x = getRandomNumber(15);
	g_maps->_mapPos.y = getRandomNumber(15);

	send(InfoMessage(0, 1, STRING["maps.poof"]));
}

void Map04::special09() {
	if (_data[MAP04_STAIRS_OVERRIDE] || _data[MAP04_TREASURE_STOLEN] == 0) {
		if (_data[MAP04_STAIRS_OVERRIDE] < 255)
			_data[MAP04_STAIRS_OVERRIDE]++;

		Sound::sound(SOUND_2);
		send(InfoMessage(
			STRING["maps.map04.stairs"],
			[]() {
				g_maps->_mapPos.x = 0;
				g_maps->_mapPos.y = 7;
				g_maps->changeMap(0x202, 2);
			}
		));
	} else {
		// Sucks to be you
		g_events->addView("Arrested");
	}
}

void Map04::special10() {
	showSign(STRING["maps.map04.market"]);
}

void Map04::special11() {
	showSign(STRING["maps.map04.blacksmith"]);
}

void Map04::special12() {
	showSign(STRING["maps.map04.inn"]);
}

void Map04::special13() {
	if (_data[MAP04_TREASURE_STOLEN]) {
		g_maps->clearSpecial();
		g_events->addView("Arrested");
	} else {
		// TODO: Sends key 160 to game_keypress, which doesn't seem to handle it
		//g_events->addAction();
	}
}

void Map04::special18() {
	showSign(STRING["maps.map04.temple"]);
}

void Map04::special20() {
	showSign(STRING["maps.map04.tavern"]);
}

void Map04::special21() {
	showSign(STRING["maps.map04.training"]);
}

} // namespace Maps
} // namespace MM1
} // namespace MM
