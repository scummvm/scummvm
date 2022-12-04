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

#include "mm/mm1/views/maps/orango.h"
#include "mm/mm1/maps/map48.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Maps {

#define ANSWER_OFFSET 274

Orango::Orango() :
		AnswerEntry("Orango", Common::Point(9, 6), 15) {
	_bounds = getLineBounds(17, 24);
}

void Orango::draw() {
	clearSurface();
	writeString(0, 1, STRING["maps.map48.orango1"]);
	AnswerEntry::draw();
}

void Orango::answerEntered() {
	MM1::Maps::Map &map = *g_maps->_currentMap;
	Common::String properAnswer;
	close();

	for (int i = 0; i < 15 && map[ANSWER_OFFSET + i]; ++i)
		properAnswer += (map[ANSWER_OFFSET + i] & 0x7f) + 29;

	if (_answer.equalsIgnoreCase(properAnswer)) {
		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			Character &c = g_globals->_party[i];
			c._flags[13] |= CHARFLAG13_ALAMAR;
		}

		g_maps->_mapPos = Common::Point(8, 5);
		g_maps->changeMap(0x604, 1);
		g_events->send(SoundMessage(STRING["maps.map48.orango3"]));

	} else {
		g_maps->_mapPos.x++;
		map.updateGame();
		g_events->send(SoundMessage(13, 2, STRING["maps.map48.orango2"]));
	}
}

} // namespace Maps
} // namespace Views
} // namespace MM1
} // namespace MM
