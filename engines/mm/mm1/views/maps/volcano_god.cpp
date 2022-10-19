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

#include "mm/mm1/views/maps/volcano_god.h"
#include "mm/mm1/maps/map11.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Maps {

#define MAX_ANSWER_LENGTH 8
#define ANSWER_OFFSET 636
#define VAL1 641

VolcanoGod::VolcanoGod() : TextView("VolcanoGod") {
	_bounds = getLineBounds(20, 24);
}

bool VolcanoGod::msgFocus(const FocusMessage &msg) {
	_answer = "";
	_mode = CHOOSE_OPTION;
	Sound::sound(SOUND_2);
	return TextView::msgFocus(msg);
}

void VolcanoGod::draw() {
	clearSurface();

	switch (_mode) {
	case CHOOSE_OPTION:
		writeString(0, 1, STRING["maps.map11.volcano_god"]);
		break;

	case ENTER_RESPONSE:
		writeString(0, 1, STRING["maps.map11.question"]);
		writeString(9, 3, _answer);
		break;

	default:
		break;
	}
}

bool VolcanoGod::msgKeypress(const KeypressMessage &msg) {
	if (!isDelayActive()) {
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

		case ENTER_RESPONSE:
			if (msg.keycode == Common::KEYCODE_RETURN) {
				responseEntered();
			} else if (msg.keycode == Common::KEYCODE_SPACE ||
				(msg.keycode >= Common::KEYCODE_0 &&
					msg.keycode <= Common::KEYCODE_z)) {
				_answer += toupper(msg.ascii);
				redraw();

				if (_answer.size() == MAX_ANSWER_LENGTH)
					responseEntered();
			} else if (msg.keycode == Common::KEYCODE_BACKSPACE && !_answer.empty()) {
				_answer.deleteLastChar();
				redraw();
			}
			break;
		}
	}

	return true;
}

void VolcanoGod::challenge() {
	Game::Encounter &enc = g_globals->_encounters;
	close();

	enc.clearMonsters();
	enc.addMonster(10, 12);
	for (int i = 1; i < 10; ++i)
		enc.addMonster(7, 8);

	enc._levelIndex = 96;
	enc._flag = true;
	enc.execute();
}

void VolcanoGod::riddle() {
	Sound::sound(SOUND_2);
	_mode = ENTER_RESPONSE;
	redraw();
}

void VolcanoGod::clue() {
	close();
	g_maps->_mapPos = Common::Point(0, 5);
	g_events->send("Game", GameMessage("UPDATE"));
}

void VolcanoGod::responseEntered() {
	MM1::Maps::Map &map = *g_maps->_currentMap;
	Common::String properAnswer;
	for (int i = 0; i < 8 && map[MAX_ANSWER_LENGTH + i]; ++i)
		properAnswer += map[ANSWER_OFFSET + i] + 30;

	if (_answer == properAnswer) {
		(*g_maps->_currentMap)[VAL1]++;
		g_events->addAction(KEYBIND_SEARCH);
	} else {
		g_maps->_mapPos = Common::Point(7, 2);
		g_maps->changeMap(0xf04, 2);
	}
}

} // namespace Maps
} // namespace Views
} // namespace MM1
} // namespace MM
