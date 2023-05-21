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

#include "mm/mm1/views/interactions/lord_ironfist.h"
#include "mm/mm1/maps/map43.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Interactions {

LordIronfist::LordIronfist() : TextView("LordIronfist") {
	_bounds = getLineBounds(20, 24);
}

bool LordIronfist::msgFocus(const FocusMessage &msg) {
	g_globals->_currCharacter = &g_globals->_party[0];
	_canAccept = !g_globals->_currCharacter->_quest;
	Sound::sound(SOUND_2);

	return TextView::msgFocus(msg);
}

void LordIronfist::draw() {
	MM1::Maps::Map43 &map = *static_cast<MM1::Maps::Map43 *>(g_maps->_currentMap);
	clearSurface();

	if (_canAccept) {
		Sound::sound2(SOUND_2);
		writeString(0, 1, STRING["maps.map43.ironfist1"]);
		writeString(0, 2, STRING["maps.map43.ironfist2"]);

	} else {
		int questNum = g_globals->_party[0]._quest;
		Common::String line;

		if (questNum < 8)
			line = map.checkQuestComplete();
		else
			line = STRING["maps.map43.ironfist4"];

		g_maps->_mapPos.x++;
		map.redrawGame();

		clearSurface();
		send(SoundMessage(
			0, 1, STRING["maps.map43.ironfist1"],
			0, 2, line
		));
		close();
	}
}

bool LordIronfist::msgKeypress(const KeypressMessage &msg) {
	MM1::Maps::Map43 &map = *static_cast<MM1::Maps::Map43 *>(g_maps->_currentMap);

	if (_canAccept) {
		if (msg.keycode == Common::KEYCODE_y) {
			close();
			map.acceptQuest();

			send(InfoMessage(
				0, 1, STRING["maps.map43.ironfist1"],
				0, 2, STRING[Common::String::format(
					"maps.map43.quests.%d",
					g_globals->_party[0]._quest)]
			));

		} else if (msg.keycode == Common::KEYCODE_n) {
			close();
		}
	}

	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace MM1
} // namespace MM
