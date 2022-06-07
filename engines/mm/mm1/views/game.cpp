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

#include "mm/mm1/views/game.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/meta_engine.h"
#include "mm/mm1/game/search.h"

namespace MM {
namespace MM1 {
namespace Views {

Game::Game() : TextView("Game"),
		_view(this), _commands(this), _messages(this), _party(this) {
	_view.setBounds(Common::Rect(0, 0, 245, 128));
}

bool Game::msgFocus(const FocusMessage &msg) {
	MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_NORMAL);
	return TextView::msgFocus(msg);
}

bool Game::msgUnfocus(const UnfocusMessage &msg) {
	MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_MENUS);
	return TextView::msgUnfocus(msg);
}

void Game::draw() {
	clearSurface();
}

bool Game::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_ORDER:
		g_events->msgGame(GameMessage("ORDER"));
		return true;
	case KEYBIND_SEARCH:
		MM1::Game::Search::execute();
		break;
	}

	return TextView::msgAction(msg);
}

bool Game::msgGame(const GameMessage &msg) {
	if (msg._name == "DISPLAY") {
		replaceView(this);
		return true;
	}

	return TextView::msgGame(msg);
}

} // namespace Views
} // namespace MM1
} // namespace MM
