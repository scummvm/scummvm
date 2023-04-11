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

#include "mm/mm1/maps/map19.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 123
#define VAL2 161
#define VAL3 162
#define VAL4 163
#define VAL5 164
#define VAL6 165
#define VAL7 166
#define ANSWER_OFFSET 167

void Map19::special() {
	// Scan for special actions on the map cell
	for (uint i = 0; i < 6; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[57 + i]) {	
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

void Map19::special00() {
	g_events->addView("IcePrincess");
}

void Map19::special01() {
	visitedExit();
	send(SoundMessage(
		STRING["maps.map19.cave"],
		[]() {
			g_maps->_mapPos = Common::Point(8, 0);
			g_maps->changeMap(0xa00, 1);
		}
	));
}

void Map19::special02() {
	visitedExit();
	send(SoundMessage(
		STRING["maps.map19.stairs_down"],
		[]() {
			g_maps->_mapPos = Common::Point(7, 1);
			g_maps->changeMap(0xf02, 3);
		}
	));
}

void Map19::special03() {
	send(SoundMessage(STRING["maps.map19.carving"]));
}

void Map19::special04() {
	if (_data[VAL2])
		g_maps->clearSpecial();

	_data[VAL2]++;
	_data[VAL4] = 10;
	_data[VAL5] = 10;
	_data[VAL7] = 10;
	_data[VAL6] = 7;
	encounter();
}

void Map19::special05() {
	if (_data[VAL3])
		g_maps->clearSpecial();

	_data[VAL3]++;
	_data[VAL4] = 10;
	_data[VAL6] = 7;
	_data[VAL5] = 1;
	_data[VAL7] = 1;
	encounter();
}

void Map19::encounter() {
	Game::Encounter &enc = g_globals->_encounters;
	int monsterCount = getRandomNumber(5) + 3;

	enc.clearMonsters();
	for (int i = 0; i < monsterCount; ++i)
		enc.addMonster(_data[VAL7], _data[VAL6]);
	enc.addMonster(_data[VAL5], _data[VAL4]);

	enc._manual = true;
	enc._levelIndex = 80;
	enc.execute();
}

void Map19::riddleAnswer(const Common::String &answer) {
	Common::String properAnswer;
	_data[VAL1] = answer.size();

	for (int i = 0; i < 4; ++i)
		properAnswer += (_data[ANSWER_OFFSET + i] & 0x7f) + 64;

	if (answer.equalsIgnoreCase(properAnswer)) {
		InfoMessage msg(
			16, 2, STRING["maps.map19.correct"],
			[]() {
				g_maps->clearSpecial();

				if (g_globals->_party.hasItem(DIAMOND_KEY_ID)) {
					g_globals->_treasure._items[2] = BRONZE_KEY_ID;
					g_events->addAction(KEYBIND_SEARCH);
					return;
				}

				g_globals->_treasure._items[2] = DIAMOND_KEY_ID;
				g_events->addAction(KEYBIND_SEARCH);
			}
		);

		msg._delaySeconds = 2;
		send(msg);
		Sound::sound(SOUND_3);
		Sound::sound(SOUND_3);

	} else {
		g_maps->_mapPos.x = 15;
		updateGame();
		send(SoundMessage(STRING["maps.map19.incorrect"]));
	}
}

} // namespace Maps
} // namespace MM1
} // namespace MM
