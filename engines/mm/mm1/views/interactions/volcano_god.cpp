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

#include "mm/mm1/views/interactions/volcano_god.h"
#include "mm/mm1/maps/map11.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Interactions {

VolcanoGod::VolcanoGod() :
		AnswerEntry("VolcanoGod", Common::Point(9, 3), 8) {
}

void VolcanoGod::draw() {
	clearSurface();

	switch (_mode) {
	case CHOOSE_OPTION:
		writeString(0, 1, STRING["maps.map11.volcano_god"]);
		break;

	case ENTER_RESPONSE:
		writeString(0, 1, STRING["maps.map11.question"]);
		AnswerEntry::draw();
		break;

	default:
		break;
	}
}

bool VolcanoGod::msgKeypress(const KeypressMessage &msg) {
	if (endDelay())
		return true;

	switch (_mode) {
	case CHOOSE_OPTION:
		switch (msg.keycode) {
		case Common::KEYCODE_a:
			challenge();
			break;
		case Common::KEYCODE_b:
			riddle();
			break;
		case Common::KEYCODE_c:
			clue();
			break;
		default:
			break;
		}
		break;

	case ENTER_RESPONSE:
		return AnswerEntry::msgKeypress(msg);
	}

	return true;
}

void VolcanoGod::challenge() {
	MM1::Maps::Map11 &map = *static_cast<MM1::Maps::Map11 *>(g_maps->_currentMap);
	map.challenge();
}

void VolcanoGod::riddle() {
	Sound::sound(SOUND_2);
	_mode = ENTER_RESPONSE;
	redraw();
}

void VolcanoGod::clue() {
	MM1::Maps::Map11 &map = *static_cast<MM1::Maps::Map11 *>(g_maps->_currentMap);
	close();
	map.clue();
}

void VolcanoGod::answerEntered() {
	MM1::Maps::Map11 &map = *static_cast<MM1::Maps::Map11 *>(g_maps->_currentMap);
	close();
	map.riddleAnswer(_answer);
}

} // namespace Interactions
} // namespace Views
} // namespace MM1
} // namespace MM
