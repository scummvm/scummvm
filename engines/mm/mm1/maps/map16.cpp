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
#define VAL2 84
#define VAL3 85
#define VAL4 87

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

		enc._flag = true;
		enc._levelIndex = 80;
		enc.execute();
		
	} else {
		g_events->addAction(KEYBIND_SEARCH);
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

		g_events->addAction(KEYBIND_SEARCH);

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
		send(SoundMessage(STRING["maps.map16.water"],
			[](const Common::KeyState &ks) {
				Game::Encounter &enc = g_globals->_encounters;

				enc.clearMonsters();
				enc.addMonster(4, 12);
				for (int i = 1; i < 12; ++i)
					enc.addMonster(12, 11);

				enc._levelIndex = 80;
				enc._flag = true;
				enc.execute();
			}
		));
	}
}

void Map16::special01() {
	send(SoundMessage(STRING["maps.map16.wheel"],
		[]() {
			// Original called sound 1 20 times. This seems excessive
			Sound::sound(SOUND_1);

			static_cast<Map16 *>(g_maps->_currentMap)->wheelSpin();
		}
	));
}

void Map16::special02() {
	_data[VAL1] = 0;
	g_events->addAction(KEYBIND_SEARCH);
}

void Map16::special03() {
	g_maps->clearSpecial();
	g_globals->_encounters.execute();
}

void Map16::wheelSpin() {
	Common::String line;
	int val;
	SoundMessage msg;
	msg._largeMessage = true;

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		Character &c = g_globals->_party[i];
		g_globals->_currCharacter = &c;
		_data[VAL2] = i;

		// Count set flags
		_data[VAL4] = 0;
		byte v = c._flags[2];
		for (int j = 0; j < 4; ++j, v >>= 1) {
			if (v & 1)
				_data[VAL4]++;
		}

		line = c._name;
		while (line.size() < 17)
			line += ' ';

		if (_data[VAL4] == 0) {
			line += STRING["maps.map16.loser"];
		} else {
			c._flags[2] |= CHARFLAG2_80;

			switch (getRandomNumber(6)) {
			case 1:
				val = 2000 << _data[VAL4];
				WRITE_LE_UINT16(&_data[VAL3], val);
				c._exp += val;
				line += Common::String::format("+%d %s", val,
					STRING["maps.map16.exp"].c_str());
				break;

			case 2:
				val = 500 << _data[VAL4];
				WRITE_LE_UINT16(&_data[VAL3], val);
				c._gold += val;
				line += Common::String::format("+%d %s", val,
					STRING["maps.map16.gold"].c_str());
				break;

			case 3:
				val = 15 << _data[VAL4];
				WRITE_LE_UINT16(&_data[VAL3], val);
				c._gems += val;
				line += Common::String::format("+%d %s", val,
					STRING["maps.map16.gems"].c_str());
				break;

			default:
				line += STRING["maps.map16.loser"];
				break;
			}
		}

		msg._lines.push_back(Line(0, i, line));
	}

	// Display the results
	send(msg);
}

} // namespace Maps
} // namespace MM1
} // namespace MM
