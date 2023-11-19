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

#include "mm/mm1/views_enh/spells/teleport.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Spells {

#define TEXT_X 120

Teleport::Teleport() : ScrollView("Teleport") {
	setBounds(Common::Rect(0, 144, 234, 200));
	addButton(&g_globals->_escSprites, Common::Point(5, 28), 0, KEYBIND_ESCAPE, true);
}

bool Teleport::msgFocus(const FocusMessage &msg) {
	ScrollView::msgFocus(msg);

	_mode = SELECT_DIRECTION;
	return 0;
}

void Teleport::draw() {
	ScrollView::draw();

	setReduced(true);
	writeString(20, 30, STRING["enhdialogs.misc.go_back"]);

	writeLine(0, STRING["dialogs.spells.teleport_dir"], ALIGN_RIGHT, TEXT_X);
	writeChar((_mode == SELECT_DIRECTION) ? '_' : _direction);

	if (_mode == SELECT_SQUARES || _mode == CAST) {
		writeLine(1, STRING["dialogs.spells.teleport_squares"], ALIGN_RIGHT, TEXT_X);
		writeChar((_mode == SELECT_SQUARES) ? '_' : '0' + _squares);
	}

	if (_mode == CAST)
		writeString(0, 30, STRING["spells.enter_to_cast"], ALIGN_RIGHT);

	setReduced(false);
}

bool Teleport::msgKeypress(const KeypressMessage &msg) {
	if (_mode == SELECT_DIRECTION && (
		msg.keycode == Common::KEYCODE_n ||
		msg.keycode == Common::KEYCODE_s ||
		msg.keycode == Common::KEYCODE_e ||
		msg.keycode == Common::KEYCODE_w)) {
		_direction = toupper(msg.ascii);
		_mode = SELECT_SQUARES;
		redraw();

	} else if (_mode == SELECT_SQUARES && (
		msg.keycode >= Common::KEYCODE_0 &&
		msg.keycode <= Common::KEYCODE_9)) {
		_squares = msg.keycode - Common::KEYCODE_0;
		_mode = CAST;
		redraw();
	}

	return true;
}

bool Teleport::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_ESCAPE:
		close();
		return true;

	case KEYBIND_SELECT:
		if (_mode == CAST)
			teleport();
		return true;

	default:
		break;
	}

	return false;
}

void Teleport::teleport() {
	Maps::Maps &maps = *g_maps;
	Maps::Map &map = *maps._currentMap;

	close();
	if (map[Maps::MAP_FLAGS] & 2) {
		g_events->send(SoundMessage(STRING["spells.failed"]));

	} else {
		switch (_direction) {
		case 'N':
			maps.step(Common::Point(0, _squares));
			break;
		case 'S':
			maps.step(Common::Point(0, -_squares));
			break;
		case 'E':
			maps.step(Common::Point(_squares, 0));
			break;
		case 'W':
			maps.step(Common::Point(-_squares, 0));
			break;
		default:
			return;
		}

		send("Game", GameMessage("UPDATE"));
	}
}

} // namespace Spells
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
