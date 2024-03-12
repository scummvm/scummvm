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
#include "mm/shared/utils/strings.h"
#include "mm/mm1/views_enh/game_messages.h"
#include "mm/mm1/events.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

GameMessages::YesNo::YesNo() :
		ScrollView("MessagesYesNo", g_events) {
	_bounds = Common::Rect(234, 144, 320, 200);
	addButton(&g_globals->_confirmIcons, Common::Point(0, 0), 0,
		Common::KeyState(Common::KEYCODE_y, 'y'));
	addButton(&g_globals->_confirmIcons, Common::Point(26, 0), 2,
		Common::KeyState(Common::KEYCODE_n, 'n'));
}

bool GameMessages::YesNo::msgKeypress(const KeypressMessage &msg) {
	// Pass on any Y/N button presses to the messages area
	return send("GameMessages", msg);
}

/*------------------------------------------------------------------------*/

GameMessages::GameMessages() : ScrollText("GameMessages") {
	setBounds(Common::Rect(0, 144, 234, 200));
}

void GameMessages::draw() {
	ScrollText::draw();

	if (_callback && !isDelayActive()) {
		_yesNo.resetSelectedButton();
		_yesNo.draw();
	}
}

bool GameMessages::msgFocus(const FocusMessage &msg) {
	MetaEngine::setKeybindingMode(_callback || _keyCallback ?
		KeybindingMode::KBMODE_MENUS :
		KeybindingMode::KBMODE_NORMAL);
	return true;
}

bool GameMessages::msgInfo(const InfoMessage &msg) {
	// Do a first draw to show 3d view at new position
	g_events->redraw();
	g_events->draw();

	_callback = msg._callback;
	_nCallback = msg._nCallback;
	_keyCallback = msg._keyCallback;
	_fontReduced = msg._fontReduced;

	// Add the view
	addView(this);

	if (msg._largeMessage)
		setBounds(Common::Rect(0, 90, 234, 200));
	else
		setBounds(Common::Rect(0, 144, 234, 200));

	// Process the lines
	clear();
	for (const auto &line : msg._lines)
		addText(line._text, -1, 0, (line.x > 0) ? ALIGN_MIDDLE : line._align, 0);

	if (msg._delaySeconds)
		delaySeconds(msg._delaySeconds);

	return true;
}

bool GameMessages::msgKeypress(const KeypressMessage &msg) {
	if (_keyCallback) {
		_keyCallback(msg);

	} else if (_callback) {
		if (msg.keycode == Common::KEYCODE_n) {
			close();
			if (_nCallback)
				_nCallback();
		} else if (msg.keycode == Common::KEYCODE_y) {
			close();
			_callback();
		}
	} else {
		// Displayed message, any keypress closes the window
		// and passes control to the game
		close();

		if (msg.keycode != Common::KEYCODE_SPACE)
			send("Game", msg);
	}

	return true;
}

bool GameMessages::msgAction(const ActionMessage &msg) {
	if (_callback || _keyCallback) {
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
			} else {
				close();
				_callback();
			}
			return true;
		default:
			break;
		}
	} else {
		// Single turn message display
		close();
		if (msg._action != KEYBIND_SELECT)
			return send("Game", msg);
	}

	return false;
}

bool GameMessages::msgMouseDown(const MouseDownMessage &msg) {
	// If yes/no prompting, also pass events to buttons view
	if (_callback)
		return send("MessagesYesNo", msg);

	return msgAction(KeybindingAction(KEYBIND_SELECT));
}

bool GameMessages::msgMouseUp(const MouseUpMessage &msg) {
	// If yes/no prompting, also pass events to buttons view
	if (_callback)
		return send("MessagesYesNo", msg);
	return false;
}

void GameMessages::timeout() {
	close();

	if (_callback)
		_callback();
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
