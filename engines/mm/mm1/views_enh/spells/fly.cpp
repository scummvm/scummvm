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

#include "mm/mm1/views_enh/spells/fly.h"
#include "mm/mm1/globals.h"

#define TEXT_X1 160
#define TEXT_X2 195

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Spells {

#define TEXT_X 120

Fly::Fly() : ScrollView("Fly") {
	setBounds(Common::Rect(0, 144, 234, 200));
	addButton(&g_globals->_escSprites, Common::Point(5, 28), 0, KEYBIND_ESCAPE, true);
}

bool Fly::msgFocus(const FocusMessage &msg) {
	ScrollView::msgFocus(msg);

	_mode = SELECT_X;
	_xIndex = _yIndex = 0;
	return 0;
}

void Fly::draw() {
	ScrollView::draw();

	setReduced(true);
	writeString(20, 30, STRING["enhdialogs.misc.go_back"]);

	writeLine(0, STRING["dialogs.spells.fly_to_x"], ALIGN_RIGHT, TEXT_X);
	writeChar((_mode == SELECT_X) ? '_' : 'A' + _xIndex);

	if (_mode == SELECT_Y || _mode == CAST) {
		writeLine(1, STRING["dialogs.spells.fly_to_y"], ALIGN_RIGHT, TEXT_X);
		writeChar((_mode == SELECT_Y) ? '_' : '1' + _yIndex);
	}

	setReduced(false);
}

bool Fly::msgKeypress(const KeypressMessage &msg) {
	if (_mode == SELECT_X && msg.keycode >= Common::KEYCODE_a
		&& msg.keycode <= Common::KEYCODE_d) {
		// X map selected
		_mode = SELECT_Y;
		_xIndex = msg.keycode - Common::KEYCODE_a;
		redraw();

	} else if (_mode == SELECT_Y && msg.keycode >= Common::KEYCODE_1
		&& msg.keycode <= Common::KEYCODE_4) {
		// Y map selected
		_mode = CAST;
		_yIndex = msg.keycode - Common::KEYCODE_1;

		delaySeconds(1);
		redraw();
	}

	return true;
}

bool Fly::msgAction(const ActionMessage &msg) {
	if (endDelay())
		return true;

	if (msg._action == KEYBIND_ESCAPE) {
		cancelDelay();
		g_events->replaceView("Game", true);
		fly(-1);
		return true;
	}

	return false;
}

void Fly::timeout() {
	// Spell was cast
	g_events->replaceView("Game", true);
	int mapIndex = _yIndex * 5 + _xIndex;
	fly(mapIndex);
}

} // namespace Spells
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
