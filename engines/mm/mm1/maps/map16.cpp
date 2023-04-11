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

#include "mm/mm1/maps/map16.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 83

void Map16::special() {
	Game::Encounter &enc = g_globals->_encounters;

	// Scan for special actions on the map cell
	for (uint i = 0; i < 8; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[59 + i]) {			
				(this->*SPECIAL_FN[i])();
			} else {
				checkPartyDead();
			}
			return;
		}
	}

	if (getRandomNumber(100) == 100) {
		Character &c = g_globals->_party[0];
		g_globals->_currCharacter = &c;
		int id1 = getRandomNumber(c._level >= 12 ? 14 : c._level) + 2;
		int monsterCount = getRandomNumber(id1 < 15 ? 13 : 4);

		enc.clearMonsters();
		for (int i = 0; i < monsterCount; ++i)
			enc.addMonster(id1, 11);

		enc._manual = true;
		enc._levelIndex = 80;
		enc.execute();
		
	} else {
		none160();
	}
}

void Map16::special00() {
	if (_data[VAL1]) {
		g_maps->clearSpecial();
		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			Character &c = g_globals->_party[i];
			g_globals->_currCharacter = &c;
			c._flags[2] |= CHARFLAG2_1;
		}

		none160();

	} else {
		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			Character &c = g_globals->_party[i];
			g_globals->_currCharacter = &c;

			if (c._flags[2] & CHARFLAG2_1) {
				g_maps->clearSpecial();
				g_events->addAction(KEYBIND_SEARCH);
				return;
			}
		}

		_data[VAL1]++;
		SoundMessage msg(STRING["maps.map16.water"],
			[]() {
				Game::Encounter &enc = g_globals->_encounters;

				enc.clearMonsters();
				enc.addMonster(4, 12);
				for (int i = 1; i < 12; ++i)
					enc.addMonster(12, 11);

				enc._levelIndex = 80;
				enc._manual = true;
				enc.execute();
			}
		);
		msg._delaySeconds = 5;
		send(msg);
	}
}

void Map16::special01() {
	send(SoundMessage(STRING["maps.map16.wheel"],
		[]() {
			for (int i = 0; i < 20; ++i)
				Sound::sound(SOUND_1);

			g_events->addView("WheelSpin");
		}
	));
}

void Map16::special02() {
	_data[VAL1] = 0;
	none160();
}

void Map16::special03() {
	g_maps->clearSpecial();
	g_globals->_encounters.execute();
}

} // namespace Maps
} // namespace MM1
} // namespace MM
