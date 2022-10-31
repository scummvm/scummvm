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

#include "mm/mm1/views/maps/access_code.h"
#include "mm/mm1/maps/map08.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Maps {

#define MAX_CODE_LENGTH 10
#define ANSWER_OFFSET 147
#define VAL1 163
#define VAL2 408

static const byte OFFSETS1[8] = { 146, 98, 150, 102, 153, 105, 157, 109 };
static const byte OFFSETS2[8] = { 130, 82, 134, 86, 137, 89, 141, 93 };

AccessCode::AccessCode() :
		AnswerEntry("AccessCode", Common::Point(16, 3), 8) {
	_bounds = getLineBounds(20, 24);
}

void AccessCode::draw() {
	clearSurface();
	writeString(0, 1, STRING["maps.map08.enter_code"]);
	AnswerEntry::draw();
}

void AccessCode::answerEntered() {
	MM1::Maps::Map &map = *g_maps->_currentMap;
	Common::String properCode;
	for (int i = 0; i < 10 && map[ANSWER_OFFSET + i]; ++i)
		properCode += map[ANSWER_OFFSET + i] + 0x1f;

	if (_answer == properCode)
		correctCode();
	else
		incorrectCode();
}

void AccessCode::correctCode() {
	MM1::Maps::Map &map = *g_maps->_currentMap;
	Sound::sound(SOUND_2);
	clearSurface();
	writeString(0, 1, STRING["maps.map08.good_code"]);

	map[VAL1] = 0;
	for (int i = 0; i < 8; ++i)
		map._states[OFFSETS1[i]] ^= 4;
	for (int i = 0; i < 8; ++i)
		map._states[OFFSETS2[i]] ^= 0x40;
	map._states[119] ^= 0x10;
	map._states[120] ^= 1;
}

void AccessCode::incorrectCode() {
	MM1::Maps::Map &map = *g_maps->_currentMap;
	Sound::sound(SOUND_2);
	clearSurface();
	writeString(0, 1, STRING["maps.map08.bad_code"]);

	if (map[VAL2] != 2 && map[VAL2] < 20) {
		g_maps->_mapPos.y--;
		g_events->send("Game", GameMessage("UPDATE"));
		return;
	} else if (map[VAL2] != 2) {
		map[MM1::Maps::MAP_31] = 10;
	}

	writeString(17, 2, STRING["maps.map08.alarm"]);
	Sound::sound(SOUND_3);
	delaySeconds(2);
}

} // namespace Maps
} // namespace Views
} // namespace MM1
} // namespace MM
