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

#include "common/file.h"
#include "mm/mm1/views_enh/game.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/meta_engine.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

Game::Game() : TextView("Game"),
		_view(this) {
}

bool Game::msgFocus(const FocusMessage &msg) {
	MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_NORMAL);
	return TextView::msgFocus(msg);
}

bool Game::msgUnfocus(const UnfocusMessage &msg) {
	MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_MENUS);
	return true;
}

void Game::draw() {
	if (_needsRedraw) {
		// Load the Xeen background
		Common::File f;
		if (!f.open("back.raw"))
			error("Could not load background");

		Graphics::Surface s = getSurface();
		f.read((byte *)s.getPixels(), s.w * s.h);
	}

	UIElement::draw();
}

bool Game::msgKeypress(const KeypressMessage &msg) {
	return true;
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
