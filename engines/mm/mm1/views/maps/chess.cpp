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

#include "mm/mm1/views/maps/chess.h"
#include "mm/mm1/maps/map29.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Maps {

#define ANSWER_OFFSET 67

Chess::Chess() :
		AnswerEntry("Chess", Common::Point(10, 7), 23) {
	_bounds = getLineBounds(17, 24);
}

bool Chess::msgFocus(const FocusMessage &msg) {
	Sound::sound(SOUND_3);
	return AnswerEntry::msgFocus(msg);
}

void Chess::draw() {
	clearSurface();
	writeString(0, 0, STRING["maps.map29.chess"]);
	AnswerEntry::draw();
}

void Chess::answerEntered() {
	MM1::Maps::Map29 &map = *static_cast<MM1::Maps::Map29 *>(g_maps->_currentMap);
	Common::String properAnswer;

	for (int i = 0; i < 22; ++i)
		properAnswer += map[ANSWER_OFFSET + i] + 48;

	if (_answer == properAnswer) {
		InfoMessage msg(
			16, 2, STRING["maps.map19.correct"],
			[]() {
				MM1::Maps::Map29 &map = *static_cast<MM1::Maps::Map29 *>(g_maps->_currentMap);

				for (uint i = 0; i < g_globals->_party.size(); ++i) {
					Character &c = g_globals->_party[i];
					c._exp += 25000;
				}

				g_maps->_mapPos.y = 7;
				map.updateGame();
			}
		);

		msg._delaySeconds = 2;
		send(msg);
		Sound::sound(SOUND_3);
		Sound::sound(SOUND_3);

	} else {
		map.begone();
	}
}

} // namespace Maps
} // namespace Views
} // namespace MM1
} // namespace MM
