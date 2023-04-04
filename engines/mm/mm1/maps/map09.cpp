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

#include "mm/mm1/maps/map09.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

static const byte MAP_DEST_X[4] = { 8, 15, 15, 7 };
static const byte MAP_DEST_Y[4] = { 5, 1, 0, 0 };
static const uint16 MAP_DEST_ID[4] = { 0x604, 0x51b, 0x601, 0xa00 };

void Map09::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 28; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[79 + i]) {	
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

void Map09::special00() {
	visitedExit();
	send(SoundMessage(
		STRING["maps.stairs_up"],
		[]() {
			g_maps->changeMap(0x802, 1);
		}
	));
}

void Map09::special01() {
	visitedExit();
	send(SoundMessage(
		STRING["maps.map09.passage_outside"],
		[]() {
			g_maps->_mapPos = Common::Point(0, 0);
			g_maps->changeMap(0xf05, 3);
		}
	));
}

void Map09::special02() {
	portal(0);
}

void Map09::special03() {
	portal(1);
}

void Map09::special04() {
	portal(2);
}

void Map09::special05() {
	portal(3);
}

void Map09::special06() {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		if (!(c._flags[11] & CHARFLAG11_GOT_ACCURACY)) {
			c._flags[11] |= CHARFLAG11_GOT_ACCURACY;
			c._might._base = MIN((int)c._accuracy._base + 4, 255);
		}
	}

	g_events->send(SoundMessage(0, 1, STRING["maps.map09.accuracy"]));
}

void Map09::special07() {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		if (!(c._flags[11] & CHARFLAG11_GOT_SPEED)) {
			c._flags[11] |= CHARFLAG11_GOT_SPEED;
			c._might._base = MIN((int)c._speed._base + 4, 255);
		}
	}

	g_events->send(SoundMessage(0, 1, STRING["maps.map09.agility"]));
}

void Map09::special08() {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		c._flags[5] |= CHARFLAG5_8;
	}

	send(SoundMessage(
		0, 1, STRING["maps.map09.shrine1"],
		[]() {
			const Character &leader = g_globals->_party[0];

			if (leader._alignment == leader._alignmentInitial) {
				SoundMessage msg(
					STRING["maps.map09.shrine2"],
					[]() {
						g_globals->_treasure._items[2] = g_events->getRandomNumber(26) + 120;
						g_globals->_treasure.setGold(120);
						g_events->addAction(KEYBIND_SEARCH);
					}
				);
				msg._delaySeconds = 2;
				g_events->send(msg);

			} else {
				SoundMessage msg(
					STRING["maps.map09.shrine3"],
					[]() {
						// TODO: Check this is right. Original set y twice
						g_maps->_mapPos = Common::Point(5, 13);
						g_maps->changeMap(0x201, 3);
					}
				);
				msg._delaySeconds = 2;
				g_events->send(msg);
			}
		}
	));
}

void Map09::special09() {
	send(SoundMessage(0, 1, STRING["maps.map09.stalactites"]));
	reduceHP();
}

void Map09::special14() {
	g_maps->clearSpecial();

	if (g_globals->_activeSpells._s.levitate) {
		send(SoundMessage(
			0, 1, STRING["maps.map09.pit"],
			0, 2, STRING["maps.map09.levitation"]
		));
	} else {
		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			Character &c = g_globals->_party[i];
			if (!g_globals->_activeSpells._s.poison &&
				!(c._condition & BAD_CONDITION))
				c._condition = POISONED;
			c._hpCurrent /= 2;
		}

		SoundMessage msg(
			STRING["maps.map09.pit"],
			[]() {
				g_globals->_encounters.execute();
			}
		);
		msg._delaySeconds = 2;
		send(msg);
	}
}

void Map09::special18() {
	if (g_globals->_activeSpells._s.psychic_protection) {
		send(SoundMessage(
			0, 1, STRING["maps.map09.psychic_blast"],
			0, 2, STRING["maps.map09.protection"]
		));
	} else {
		g_globals->_currCharacter = &g_globals->_party[
			getRandomNumber(g_globals->_party.size()) - 1
		];

		if (!(g_globals->_currCharacter->_condition & BAD_CONDITION)) {
			// Chosen character is okay, so blast them
			g_globals->_currCharacter->_condition = BAD_CONDITION | DEAD;
		} else {
			// Chosen character is disabled, so instead
			// remove the SP from all members of the party
			for (uint i = 0; i < g_globals->_party.size(); ++i) {
				g_globals->_party[i]._sp._current = 0;
			}
		}

		g_globals->_encounters.execute();
	}
}

void Map09::special25() {
	send(SoundMessage(0, 1, STRING["maps.map09.scrawl"]));
}

void Map09::special26() {
	send(SoundMessage(0, 1, STRING["maps.map09.corak_was_here"]));
}

void Map09::special27() {
	send(SoundMessage(0, 1, STRING["maps.map09.message"]));
}

void Map09::portal(int index) {
	visitedExit();
	_portalIndex = index;

	send(SoundMessage(
		STRING["maps.map09.portal"],
		[]() {
			int idx = static_cast<Map09 *>(g_maps->_currentMap)->_portalIndex;
			g_maps->_mapPos = Common::Point(MAP_DEST_X[idx], MAP_DEST_Y[idx]);
			g_maps->changeMap(MAP_DEST_ID[idx], 1);
		}
	));
}

} // namespace Maps
} // namespace MM1
} // namespace MM
