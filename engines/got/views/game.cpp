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

#include "got/views/game.h"
#include "got/game/back.h"
#include "got/game/init.h"
#include "got/game/move.h"
#include "got/game/object.h"
#include "got/metaengine.h"
#include "got/vars.h"

namespace Got {
namespace Views {

Game::Game() : View("Game") {
	_children.push_back(&_content);
	_children.push_back(&_status);
	_content.setBounds(Common::Rect(0, 0, 320, 240 - 48));
	_status.setBounds(Common::Rect(0, 240 - 48, 320, 240));
}

bool Game::msgKeypress(const KeypressMessage &msg) {
	if (_G(gameMode) != MODE_NORMAL)
		return false;

	switch (msg.keycode) {
	case Common::KEYCODE_F1:
		odin_speaks(2008, -1);
		return true;

	case Common::KEYCODE_s:
		g_engine->saveGameDialog();
		break;

	case Common::KEYCODE_l:
		g_engine->loadGameDialog();
		break;

	default:
		break;
	}

	return false;
}

bool Game::msgAction(const ActionMessage &msg) {
	if (_G(gameMode) != MODE_NORMAL)
		return false;

	switch (msg._action) {
	case KEYBIND_FIRE:
		thor_shoots();
		break;

	case KEYBIND_SELECT:
		select_item();
		return true;

	case KEYBIND_THOR_DIES:
		_content.send(GameMessage("THOR_DIES"));
		return true;

	case KEYBIND_ESCAPE:
		addView("OptionsMenu");
		return true;

	default:
		break;
	}

	return false;
}

bool Game::tick() {
	// There are many things in original game code that can trigger
	// changes on screen, and for simplicity they each no longer have
	// the code that partially updates the screen. Due to this,
	// we set to redraw the screen every frame in case of updates
	redraw();

	return View::tick();
}

} // namespace Views
} // namespace Got
