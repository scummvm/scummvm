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

#include "mm/mm1/views/interactions/inspectron.h"
#include "mm/mm1/maps/map35.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Interactions {

#define VAL1 123
#define ANSWER_OFFSET 167

Inspectron::Inspectron() : TextView("Inspectron") {
	_bounds = getLineBounds(20, 24);
}

bool Inspectron::msgGame(const GameMessage &msg) {
	if (msg._name != "DISPLAY")
		return false;

	g_globals->_currCharacter = &g_globals->_party[0];
	_canAccept = !g_globals->_currCharacter->_quest;

	if (_canAccept) {
		// Open the view for display
		Sound::sound(SOUND_2);
		addView();

	} else {
		// There's an active quest, so any result will simply be
		// displayed as a game message
		MM1::Maps::Map35 &map = *static_cast<MM1::Maps::Map35 *>(g_maps->_currentMap);
		int questNum = g_globals->_party[0]._quest;
		Common::String line;

		if (questNum >= 8 && questNum <= 14)
			line = map.checkQuestComplete();
		else
			line = STRING["maps.map35.inspectron4"];

		g_maps->_mapPos.y++;
		map.redrawGame();

		send(SoundMessage(
			0, 1, STRING["maps.map35.inspectron1"],
			0, 2, line
		));
	}

	return true;
}

void Inspectron::draw() {
	clearSurface();
	writeString(0, 1, STRING["maps.map35.inspectron1"]);
	writeString(0, 2, STRING["maps.map35.inspectron2"]);
}

bool Inspectron::msgKeypress(const KeypressMessage &msg) {
	MM1::Maps::Map35 &map = *static_cast<MM1::Maps::Map35 *>(g_maps->_currentMap);

	if (msg.keycode == Common::KEYCODE_y) {
		map.acceptQuest();
		close();

		send(InfoMessage(
			0, 1, STRING["maps.map35.inspectron1"],
			0, 2, STRING[Common::String::format(
				"maps.map35.quests.%d",
				g_globals->_party[0]._quest - 8
			)]
		));

	} else if (msg.keycode == Common::KEYCODE_n) {
		close();
		map.redrawGame();
	}

	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace MM1
} // namespace MM
