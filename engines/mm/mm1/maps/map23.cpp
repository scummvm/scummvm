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

#include "mm/mm1/maps/map23.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 107
#define VAL2 108

void Map23::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 14; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[65 + i]) {
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	g_maps->clearSpecial();
	InfoMessage msg(
		14, 2, STRING["maps.map23.look_out"],
		[]() {
			g_globals->_encounters.execute();
		}
	);
	msg._delaySeconds = 2;
	send(msg);
}

void Map23::special00() {
	visitedExit();
	send(SoundMessage(
		STRING["maps.map23.passage"],
		[]() {
			g_maps->_mapPos = Common::Point(12, 0);
			g_maps->changeMap(0x604, 1);
		}
	));
}

void Map23::special01() {
	visitedExit();
	send(SoundMessage(
		STRING["maps.map23.cave"],
		[]() {
			g_maps->_mapPos = Common::Point(7, 0);
			g_maps->changeMap(1, 1);
		}
	));
}

void Map23::special02() {
	g_events->addView("Gypsy");
}

void Map23::special03() {
	if (g_globals->_activeSpells._s.levitate) {
		Common::String line1 = Common::String::format("%s %s",
			STRING["maps.map23.pit"].c_str(),
			STRING["maps.map23.levitation"].c_str()
		);

		send(SoundMessage(line1));

	} else {
		reduceHP();

		Common::String line1 = Common::String::format("%s %s",
			STRING["maps.map23.pit"].c_str(),
			STRING["maps.map23.ambush"].c_str()
		);
		SoundMessage msg(line1,
			[]() {
				g_globals->_encounters.execute();
			}
		);
		msg._delaySeconds = 2;
		send(msg);
	}
}

void Map23::special08() {
	send(SoundMessage(
		STRING["maps.map23.column"],
		[]() {
			g_maps->_mapPos = Common::Point(
				g_events->getRandomNumber(15),
				g_events->getRandomNumber(15));
			g_maps->_currentMap->updateGame();
		}
	));
}

void Map23::special09() {
	send(SoundMessage(
		STRING["maps.map23.statues"],
		[]() {
			Map23 &map = *static_cast<Map23 *>(g_maps->_currentMap);
			Game::Encounter &enc = g_globals->_encounters;

			g_maps->clearSpecial();
			map._states[32]--;
			g_maps->_currentState = map._states[32];

			enc.clearMonsters();
			for (int i = 0; i < 6; ++i)
				enc.addMonster(2, 4);

			enc._levelIndex = 80;
			enc._manual = true;
			enc.execute();
		}
	));
}

void Map23::special10() {
	if (_data[VAL1]) {
		if (g_maps->_forwardMask != DIRMASK_E) {
			g_maps->clearSpecial();
			return;
		}
	} else {
		_data[VAL1] = 1;
	}

	fountain();
}

void Map23::special11() {
	if (_data[VAL2]) {
		if (g_maps->_forwardMask != DIRMASK_E) {
			g_maps->clearSpecial();
			return;
		}
	} else {
		_data[VAL2] = 1;
	}

	fountain();
}

void Map23::special12() {
	send(SoundMessage(14, 2, STRING["maps.map23.avalanche"]));
	g_maps->_currentState = 209;
	_states[g_maps->_mapOffset] = 0xff;
	_walls[g_maps->_mapOffset] = 162;

	if (g_events->isKeypressPending()) {
		send(InfoMessage());
		g_maps->clearSpecial();
	}
}

void Map23::special13() {
	g_maps->clearSpecial();
	g_globals->_treasure._container = IRON_BOX;
	g_globals->_treasure._items[0] = 171;
	g_globals->_treasure._items[1] = 183;
	g_globals->_treasure._items[2] = 191;
	g_globals->_treasure.setGold(12);
	g_events->addAction(KEYBIND_SEARCH);
}

void Map23::fountain() {
	send(SoundMessage(
		STRING["maps.map23.fountain"],
		[]() {
			for (uint i = 0; i < g_globals->_party.size(); ++i) {
				Character &c = g_globals->_party[i];
				int attrNum = g_events->getRandomNumber(8) - 1;
				c.getAttribute(attrNum)._current = 30;
			}

			g_events->send(InfoMessage(16, 2, STRING["maps.map23.cheers"]));
		}
	));
}

} // namespace Maps
} // namespace MM1
} // namespace MM
