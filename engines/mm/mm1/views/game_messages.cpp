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
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {

GameMessages::GameMessages(UIElement *owner) :
		TextView("GameMessages", owner) {
	_bounds = getLineBounds(21, 24);
}

bool GameMessages::msgUnfocus(const UnfocusMessage &msg) {
	clearSurface();
	return true;
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
	if (msg._yCallback || msg._keyCallback ||
			g_globals->_party.isPartyDead()) {
		addView(this);
	}

	_lines = msg._lines;
	_yCallback = msg._yCallback;
	_nCallback = msg._nCallback;
	_keyCallback = msg._keyCallback;

	if (msg._largeMessage)
		_bounds = getLineBounds(17, 24);
	else
		_bounds = getLineBounds(21, 24);

	if (msg._sound)
		Sound::sound(SOUND_2);

	redraw();

	if (msg._delaySeconds)
		delaySeconds(msg._delaySeconds);

	return true;
}

bool GameMessages::msgKeypress(const KeypressMessage &msg) {
	if (g_globals->_party.isPartyDead()) {
		// Party is dead, so now that players have read whatever
		// message was displayed, switch to the Dead screen
		g_events->clearViews();
		addView("Dead");

	} else if (g_events->focusedView()) {
		if (endDelay())
			return true;

		if (_keyCallback) {
			_keyCallback(msg);
		} else if (msg.keycode == Common::KEYCODE_n) {
			close();
			if (_nCallback)
				_nCallback();
		} else if (msg.keycode == Common::KEYCODE_y) {
			close();
			_yCallback();
		}

		return true;
	}

	return false;
}

bool GameMessages::msgAction(const ActionMessage &msg) {
	auto focusedView = g_events->focusedView();

	if (g_globals->_party.isPartyDead()) {
		// Party is dead, so now that players have read whatever
		// message was displayed, switch to the Dead screen
		g_events->clearViews();
		addView("Dead");

	} else if (focusedView == this) {
		if (endDelay())
			return true;

		switch (msg._action) {
		case KEYBIND_ESCAPE:
			if (_keyCallback) {
				_keyCallback(Common::KeyState(Common::KEYCODE_ESCAPE));
			} else {
				close();
				if (_nCallback)
					_nCallback();
			}
			return true;
		case KEYBIND_SELECT:
			if (_keyCallback) {
				_keyCallback(Common::KeyState(Common::KEYCODE_RETURN));
			} else if (_yCallback) {
				close();
				_yCallback();
			}
			return true;
		default:
			break;
		}
	} else if (msg._action == KEYBIND_SELECT) {
		clearSurface();
		return true;
	}

	return false;
}

void GameMessages::timeout() {
	if (_yCallback) {
		// _ynCallback is also used for timeout callbacks
		close();
		_yCallback();
	}
}

} // namespace Views
} // namespace MM1
} // namespace MM
