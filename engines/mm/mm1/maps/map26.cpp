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

#include "mm/mm1/maps/map26.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 67

void Map26::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 4; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[55 + i]) {
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	desert();
}

void Map26::special00() {
	if (_data[VAL1]) {
		addFlag();

	} else {
		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			if (g_globals->_party[i]._flags[2] & CHARFLAG2_2) {
				g_maps->clearSpecial();
				none160();
				return;
			}
		}

		SoundMessage msg(
			STRING["maps.map26.scorpion"],
			[]() {
				Map26 &map = *static_cast<Map26 *>(g_maps->_currentMap);
				Game::Encounter &enc = g_globals->_encounters;
				map[VAL1]++;

				enc.clearMonsters();
				enc.addMonster(1, 12);
				for (int i = 1; i < 14; ++i)
					enc.addMonster(5, 5);

				enc._levelIndex = 80;
				enc._manual = true;
				enc._encounterType = Game::FORCE_SURPRISED;
				enc.execute();
			}
		);
		msg._delaySeconds = 4;
		send(msg);
	}
}

void Map26::special01() {
	_data[VAL1] = 0;
	none160();
}

void Map26::special02() {
	send(SoundMessage(
		STRING["maps.map26.trading_post"],
		[]() {
			Character &c = g_globals->_party[0];
			if (c._backpack.empty()) {
				g_events->send(SoundMessage(STRING["maps.map26.nothing_to_trade"]));
			} else {
				c._backpack[0]._id = CACTUS_NECTAR_ID;
				c._backpack[0]._charges = 10;
			}
		}
	));
}

void Map26::special03() {
	send(SoundMessage(STRING["maps.map26.kilburn"]));
}

void Map26::addFlag() {
	for (uint i = 0; i < g_globals->_party.size(); ++i)
		g_globals->_party[i]._flags[2] |= CHARFLAG2_2;

	g_maps->clearSpecial();
	none160();
}

} // namespace Maps
} // namespace MM1
} // namespace MM
