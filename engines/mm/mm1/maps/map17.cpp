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

#include "mm/mm1/maps/map17.h"
#include "mm/mm1/maps/maps.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Maps {

#define VAL1 509
#define CORRECT_ANSWERS 511

void Map17::special() {
	Game::Encounter &enc = g_globals->_encounters;

	// Scan for special actions on the map cell
	for (uint i = 0; i < 9; ++i) {
		if (g_maps->_mapOffset == _data[51 + i]) {
			// Found a specially handled cell, but it
			// only triggers in designated direction(s)
			if (g_maps->_forwardMask & _data[60 + i]) {			
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

	} else if (getRandomNumber(30) == 10) {
		g_maps->_mapPos = Common::Point(15, 15);
		updateGame();
		send(SoundMessage(STRING["maps.map17.wave"]));

	} else {
		none160();
	}
}

void Map17::special00() {
	send(SoundMessage(STRING["maps.map17.islands"]));
}

void Map17::special01() {
	SoundMessage msg(STRING["maps.map17.bridge"],
		[]() {
			g_events->send("ColorQuestions", GameMessage("DISPLAY"));
		}
	);

	msg._largeMessage = true;
	send(msg);
}

void Map17::special02() {
	if (_data[CORRECT_ANSWERS]) {
		g_globals->_treasure._items[2] = CORAL_KEY_ID;
		g_events->addAction(KEYBIND_SEARCH);
	} else {
		none160();
	}
}

void Map17::special03() {
	g_maps->clearSpecial();
	g_globals->_encounters.execute();
}
/*
void Map17::askQuestion(uint partyIndex) {
	if (partyIndex >= g_globals->_party.size()) {
		// Entire party has answered the question
		if (_data[CORRECT_ANSWERS]) {
			g_maps->_mapPos.y = 2;
			updateGame();
		} else {
			none160();
		}
		return;
	}

	_data[VAL1] = partyIndex;
	g_globals->_currCharacter = &g_globals->_party[partyIndex];

	if (!(g_globals->_currCharacter->_condition & BAD_CONDITION)) {
		InfoMessage msg(
			0, 0, STRING["maps.map17.color"],
			0, 2, STRING["maps.map17.options"],
			[](const Common::KeyState &ks) {
				Map17 &map = *static_cast<Map17 *>(g_maps->_currentMap);
				if (ks.keycode >= Common::KEYCODE_1 &&
						ks.keycode <= Common::KEYCODE_9) {
					map[COLOR] = ks.ascii - '1';

					Common::String line;
					Character &c = *g_globals->_currCharacter;
					int color = c._flags[2] & 0xf;

					// If a color hasn't been designated yet from talking to Gypsy,
					// or it has but the wrong color is selected, eradicate them
					if (!color || (color & 7) != map[COLOR]) {
						c._condition = ERADICATED;
						line = STRING["maps.map17.wrong"];
					} else {
						map[CORRECT_ANSWERS]++;
						c._flags[4] |= CHARFLAG4_80;
						line = STRING["maps.map17.correct"];
					}

					Sound::sound(SOUND_3);
					InfoMessage msg2;
					msg2._largeMessage = true;
					msg2._delaySeconds = 1;
					msg2._lines.push_back(Line(0, 0, STRING["maps.map17.color"]));
					msg2._lines.push_back(Line(0, 2, STRING["maps.map17.options"]));
					msg2._lines.push_back(Line(16, 6, line));
					msg2._timeoutCallback = []() {
						Map17 &map17 = *static_cast<Map17 *>(g_maps->_currentMap);
						map17.askQuestion(map17[VAL1] + 1);
					};

					g_events->send(msg2);
				}
			}
		);
		msg._largeMessage = true;
		send(msg);
	}
}
*/
} // namespace Maps
} // namespace MM1
} // namespace MM
