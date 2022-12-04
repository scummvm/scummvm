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

#include "mm/mm1/views/maps/ice_princess.h"
#include "mm/mm1/maps/map19.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Maps {

#define VAL1 123
#define ANSWER_OFFSET 167

IcePrincess::IcePrincess() :
		AnswerEntry("IcePrincess", Common::Point(9, 7), 10) {
	_bounds = getLineBounds(17, 24);
}

void IcePrincess::draw() {
	clearSurface();
	writeString(0, 1, STRING["maps.map19.ice_princess"]);
	AnswerEntry::draw();
}

void IcePrincess::answerEntered() {
	MM1::Maps::Map &map = *g_maps->_currentMap;
	Common::String properAnswer;
	map[VAL1] = _answer.size();
	close();

	for (int i = 0; i < 4; ++i)
		properAnswer += (map[ANSWER_OFFSET + i] & 0x7f) + 64;

	if (_answer.equalsIgnoreCase(properAnswer)) {
		InfoMessage msg(
			16, 2, STRING["maps.map19.correct"],
			[]() {
				g_maps->clearSpecial();

				for (uint i = 0; i < g_globals->_party.size(); ++i) {
					g_globals->_currCharacter = &g_globals->_party[i];
					if (g_globals->_currCharacter->_backpack.indexOf(DIAMOND_KEY_ID) != -1) {
						g_globals->_treasure._items[2] = 237;
						g_events->addAction(KEYBIND_SEARCH);
						return;
					}
				}

				g_globals->_treasure._items[2] = 240;
				g_events->addAction(KEYBIND_SEARCH);
			}
		);

		msg._delaySeconds = 2;
		send(msg);
		Sound::sound(SOUND_3);
		Sound::sound(SOUND_3);

	} else {
		g_maps->_mapPos.x = 15;
		g_events->send("Game", GameMessage("UPDATE"));
		send(SoundMessage(STRING["maps.map19.incorrect"]));
	}
}

} // namespace Maps
} // namespace Views
} // namespace MM1
} // namespace MM
