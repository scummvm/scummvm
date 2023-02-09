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

#include "mm/mm1/views/spells/teleport.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Spells {

void Teleport::show() {
	UIElement *teleport = dynamic_cast<Teleport *>(g_events->findView("Teleport"));
	assert(teleport);

	teleport->open();
}

Teleport::Teleport() : SpellView("Teleport") {
	_bounds = getLineBounds(20, 24);
}

bool Teleport::msgFocus(const FocusMessage &msg) {
	SpellView::msgFocus(msg);

	_mode = SELECT_DIRECTION;
	return 0;
}

void Teleport::draw() {
	clearSurface();
	escToGoBack(0);

	writeString(10, 1, STRING["dialogs.spells.teleport_dir"]);
	writeChar((_mode == SELECT_DIRECTION) ? '_' : _direction);

	if (_mode != SELECT_DIRECTION) {
		writeString(11, 2, STRING["dialogs.spells.teleport_squares"]);
		writeChar((_mode == SELECT_SQUARES) ? '_' : '0' + _squares);
	}

	if (_mode == CAST)
		writeString(23, 4, STRING["spells.enter_to_cast"]);
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

	} else if (_mode == CAST && msg.keycode == Common::KEYCODE_RETURN) {
		teleport();
	}

	return true;
}

bool Teleport::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		close();
		return true;
	}

	return false;
}

void Teleport::teleport() {
	Maps::Maps &maps = *g_maps;
	Maps::Map &map = *maps._currentMap;

	if (map[Maps::MAP_FLAGS] & 2) {
		spellFailed();

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

		close();
		send("Game", GameMessage("UPDATE"));
	}
}

} // namespace Spells
} // namespace Views
} // namespace MM1
} // namespace MM
