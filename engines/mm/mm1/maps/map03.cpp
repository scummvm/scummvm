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

#include "mm/mm1/maps/map03.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

static const byte MONSTER_ID1[8] = { 5, 10, 12, 2, 11, 16, 6, 12 };
static const byte MONSTER_ID2[8] = { 2, 7, 7, 7, 5, 4, 3, 8 };

void Map03::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 29; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[80 + i]) {				
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

void Map03::special00() {
	inn();
}

void Map03::special01() {
	bool hasCourier = false;
	for (uint i = 0; i < g_globals->_party.size() && !hasCourier; ++i) {
		hasCourier = (g_globals->_party[i]._flags[0] & CHARFLAG0_COURIER3) != 0;
	}

	bool hasScroll = false;
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		if (c.hasItem(VELLUM_SCROLL_ID)) {
			hasScroll = true;
			c._gold += 1500;
		}
	}

	if (hasCourier && hasScroll) {
		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			Character &c = g_globals->_party[i];
			g_globals->_currCharacter = &c;
			c._exp += 2500;

			int idx = c._equipped.indexOf(VELLUM_SCROLL_ID);
			if (idx != -1)
				c._equipped.removeAt(idx);
			idx = c._backpack.indexOf(VELLUM_SCROLL_ID);
			if (idx != -1)
				c._backpack.removeAt(idx);
		}

		InfoMessage info1(
			0, 0, STRING["maps.map03.telgoran1"],
			0, 1, STRING["maps.map03.telgoran2"],
			[](const Common::KeyState &) {
				InfoMessage info2(
					0, 1, STRING["maps.map03.telgoran3"],
					[](const Common::KeyState &) {
						g_events->close();
					}
				);

				info2._largeMessage = true;
				g_events->send(info2);
			}
		);
		info1._largeMessage = true;
		g_events->send(info1);

	} else {
		send(InfoMessage(
			0, 0, STRING["maps.map03.telgoran1"],
			0, 1, STRING["maps.map03.telgoran4"]
		));
	}

	none160();
}

void Map03::special02() {
	blacksmith();
}

void Map03::special03() {
	market();
}

void Map03::special04() {
	visitedExit();
	send(SoundMessage(
		STRING["maps.passage_outside1"],
		[]() {
			g_maps->_mapPos = Common::Point(9, 11);
			g_maps->changeMap(0x112, 2);
		}
	));
}

void Map03::special05() {
	tavern();
}

void Map03::special06() {
	temple();
}

void Map03::special07() {
	training();
}

void Map03::special08() {
	g_globals->_treasure._container = GOLD_CHEST;
	g_globals->_treasure._items[2] = 200;
	g_globals->_treasure.setGems(200);
	g_events->addAction(KEYBIND_SEARCH);
}

void Map03::special09() {
	visitedExit();
	send(SoundMessage(
		STRING["maps.stairs_down"],
		[]() {
			g_maps->_mapPos = Common::Point(14, 0);
			g_maps->changeMap(5, 1);
		}
	));
}

void Map03::special10() {
	showSign(STRING["maps.map03.market"]);
}

void Map03::special11() {
	showSign(STRING["maps.map03.blacksmith"]);
}

void Map03::special12() {
	showSign(STRING["maps.map03.inn"]);
}

void Map03::special13() {
	_data[0x1d] = 80;
	_data[0x2e] = 3;
	_data[0x2f] = 3;
}

void Map03::special14() {
	_data[0x1d] = 150;
	_data[0x2e] = 2;
	_data[0x2f] = 2;
}

void Map03::special15() {
	assert(g_maps->_mapPos.x < 8);
	g_maps->clearSpecial();
	int monsterCount = (g_maps->_mapPos.x < 3) ? 1 :
		getRandomNumber(8);

	Game::Encounter &enc = g_globals->_encounters;
	enc._levelIndex = 80;
	enc._manual = true;

	enc.clearMonsters();
	for (int i = 0; i < monsterCount; ++i)
		enc.addMonster(MONSTER_ID1[i], MONSTER_ID2[i]);

	enc.execute();
}

void Map03::special18() {
	showSign(STRING["maps.map03.temple"]);
}

void Map03::special20() {
	showSign(STRING["maps.map03.tavern"]);
}

void Map03::special21() {
	showSign(STRING["maps.map03.training"]);
}

void Map03::special27() {
	showSign(STRING["maps.map03.forbidden_crypt"]);
}

void Map03::special28() {
	showSign(STRING["maps.map03.eternal_rest"]);
}

} // namespace Maps
} // namespace MM1
} // namespace MM
