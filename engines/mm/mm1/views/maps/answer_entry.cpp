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

#include "mm/mm1/views/maps/answer_entry.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Maps {

AnswerEntry::AnswerEntry(const Common::String &name,
	const Common::Point &pos, size_t maxLength) :
		TextView(name), _pos(pos), _maxLength(maxLength) {
	_bounds = getLineBounds(20, 24);
}

bool AnswerEntry::msgFocus(const FocusMessage &msg) {
	_answer = "";
	Sound::sound(SOUND_2);
	return TextView::msgFocus(msg);
}

void AnswerEntry::draw() {
	writeString(_pos.x, _pos.y, _answer);
	for (uint i = 0; i < (_maxLength - _answer.size()); ++i)
		writeChar(_blank);
}

bool AnswerEntry::msgKeypress(const KeypressMessage &msg) {
	if (!isDelayActive()) {
		if (msg.keycode == Common::KEYCODE_SPACE ||
			(msg.keycode >= Common::KEYCODE_0 &&
				msg.keycode <= Common::KEYCODE_z)) {
			_answer += toupper(msg.ascii);
			redraw();

			if (_answer.size() == _maxLength)
				answerEntered();

		} else if (msg.keycode == Common::KEYCODE_BACKSPACE && !_answer.empty()) {
			_answer.deleteLastChar();
			redraw();
		}
	}

	return true;
}

bool AnswerEntry::msgAction(const ActionMessage &msg) {
	if (!isDelayActive() && msg._action == KEYBIND_SELECT) {
		answerEntered();
		return true;
	}

	return false;
}

} // namespace Maps
} // namespace Views
} // namespace MM1
} // namespace MM
