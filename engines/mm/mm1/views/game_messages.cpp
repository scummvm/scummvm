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

#include "common/util.h"
#include "mm/mm1/views/game_messages.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Views {

GameMessages::GameMessages(UIElement *owner) :
		TextView("GameMessages", owner) {
	_bounds = getLineBounds(21, 24);
}

void GameMessages::draw() {
	clearSurface();

	if (!_lines.empty()) {
		if (_lines[0].y == -1) {
			// No co-ordinates provided, just display
			for (uint i = 0; i < MIN(_lines.size(), 4U); ++i)
				writeString(0, i, _lines[i]._text);
		} else {
			// Write text at suggested co-ordinates
			for (uint i = 0; i < MIN(_lines.size(), 4U); ++i)
				writeString(_lines[i].x, _lines[i].y, _lines[i]._text);
		}

		_lines.clear();
	}
}

bool GameMessages::msgInfo(const InfoMessage &msg) {
	if (msg._ynCallback || msg._keyCallback) {
		// Do a first draw to show 3d view at new position
		g_events->redraw();
		g_events->drawElements();

		addView(this);
	}

	_lines = msg._lines;
	_ynCallback = msg._ynCallback;
	_keyCallback = msg._keyCallback;

	redraw();
	return true;
}

bool GameMessages::msgKeypress(const KeypressMessage &msg) {
	if (g_events->focusedView() == this) {
		if (_keyCallback) {
			_keyCallback(msg);
		} else if (msg.keycode == Common::KEYCODE_n) {
			close();
		} else if (msg.keycode == Common::KEYCODE_y) {
			close();
			_ynCallback();
		}

		return true;
	}

	return false;
}

} // namespace Views
} // namespace MM1
} // namespace MM
