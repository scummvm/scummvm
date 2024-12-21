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

bool Game::msgFocus(const FocusMessage &msg) {
	if (_firstTime) {
		initialize();
		_firstTime = false;
	}

	return true;
}

bool Game::msgUnfocus(const UnfocusMessage &msg) {
	return true;
}

void Game::draw() {
	GfxSurface s = getSurface();
	s.clear();
}

bool Game::msgKeypress(const KeypressMessage &msg) {
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
	return true;
}

bool Game::tick() {
	// There are many things in original game code that can trigger
	// changes on screen, and for simplicity they each no longer have
	// the code that partially updates the screen. Due to this,
	// we set to redraw the screen every frame in case of updates
	redraw();

	return false;
}

} // namespace Views
} // namespace Got
