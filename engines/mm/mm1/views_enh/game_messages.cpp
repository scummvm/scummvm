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
	_bounds = Common::Rect(234, 18 * 8, 320, 200);
	addButton(&g_globals->_confirmIcons,
		Common::Point(14, 10), 0,
		Common::KeyState(Common::KEYCODE_y, 'y'));
	addButton(&g_globals->_confirmIcons,
		Common::Point(40, 10), 2,
		Common::KeyState(Common::KEYCODE_n, 'n'));
}

bool GameMessages::YesNo::msgKeypress(const KeypressMessage &msg) {
	// Pass on any Y/N button presses to the messages area
	return send("GameMessages", msg);
}

/*------------------------------------------------------------------------*/

GameMessages::GameMessages(UIElement *owner) :
		ScrollText("GameMessages", owner) {
}

void GameMessages::draw() {
	// Only draw non-focused messages for a single turn
	if (_show || g_events->focusedView() == this) {
		ScrollText::draw();
		if (_ynCallback) {
			_yesNo.resetSelectedButton();
			_yesNo.draw();
		}

		_show = false;
	}
}

bool GameMessages::msgInfo(const InfoMessage &msg) {
	if (msg._ynCallback || msg._keyCallback) {
		// Do a first draw to show 3d view at new position
		g_events->redraw();
		g_events->drawElements();

		addView(this);
	}

	setBounds(Common::Rect(0, 18 * 8, 234, 200));

	_show = true;
	_ynCallback = msg._ynCallback;
	_keyCallback = msg._keyCallback;

	// Process the lines
	clear();
	for (auto line : msg._lines)
		addText(line._text, line.y, 0, line._align, line.x * 8);

	redraw();
	return true;
}

bool GameMessages::msgKeypress(const KeypressMessage &msg) {
	if (g_events->focusedView() == this) {
		if (_keyCallback) {
			_keyCallback(msg);
		} else if (msg.keycode == Common::KEYCODE_n) {
			close();
			g_events->drawElements();
		} else if (msg.keycode == Common::KEYCODE_y) {
			close();
			g_events->drawElements();
			_ynCallback();
		}

		return true;
	}

	return false;
}

bool GameMessages::msgAction(const ActionMessage &msg) {
	if (g_events->focusedView()) {
		switch (msg._action) {
		case KEYBIND_ESCAPE:
			if (_keyCallback) {
				_keyCallback(Common::KeyState(Common::KEYCODE_ESCAPE));
			} else {
				close();
				g_events->drawElements();
			}
			return true;
		case KEYBIND_SELECT:
			if (_keyCallback) {
				_keyCallback(Common::KeyState(Common::KEYCODE_RETURN));
			} else {
				close();
				g_events->drawElements();
				_ynCallback();
			}
			return true;
		default:
			break;
		}
	}

	return false;
}

bool GameMessages::msgMouseDown(const MouseDownMessage &msg) {
	// If yes/no prompting, also pass events to buttons view
	if (_ynCallback)
		return send("MessagesYesNo", msg);
	return false;
}

bool GameMessages::msgMouseUp(const MouseUpMessage &msg) {
	// If yes/no prompting, also pass events to buttons view
	if (_ynCallback)
		return send("MessagesYesNo", msg);
	return false;
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
