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
#include "mm/mm1/metaengine.h"
#include "mm/mm1/mm1.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

Game::Game() : TextView("Game"),
		_view(this),
		_commands(this),
		_party(this) {
	_view.setBounds(Common::Rect(8, 15, 224, 130));
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
	Graphics::ManagedSurface s = getSurface();
	s.blitFrom(g_globals->_gameBackground);

	UIElement::draw();
}

bool Game::msgKeypress(const KeypressMessage &msg) {
	switch (msg.keycode) {
	case Common::KEYCODE_F5:
		if (g_engine->canSaveGameStateCurrently())
			g_engine->saveGameDialog();
		break;
	case Common::KEYCODE_F7:
		if (g_engine->canLoadGameStateCurrently())
			g_engine->loadGameDialog();
		break;
	default:
		break;
	}

	return true;
}

bool Game::msgMouseDown(const MouseDownMessage &msg) {
	if (Common::Rect(109, 137, 122, 147).contains(msg._pos) &&
			msg._button == MouseDownMessage::MB_LEFT) {
		g_engine->openMainMenuDialog();
		return true;
	} else {
		return Views::TextView::msgMouseDown(msg);
	}
}

bool Game::msgAction(const ActionMessage &msg) {
	switch (msg._action) {
	case KEYBIND_BASH:
		send("Bash", GameMessage("SHOW"));
		break;
	case KEYBIND_MAP:
		addView("MapPopup");
		return true;
	case KEYBIND_PROTECT:
		addView("Protect");
		return true;
	case KEYBIND_QUICKREF:
		addView("QuickRef");
		return true;
	case KEYBIND_REST:
		addView("Rest");
		return true;
	case KEYBIND_SEARCH:
		send("Search", GameMessage("SHOW"));
		break;
	case KEYBIND_SPELL:
		addView("CastSpell");
		return true;
	case KEYBIND_UNLOCK:
		send("Unlock", GameMessage("SHOW"));
		break;
	//case KEYBIND_ORDER:
	// Enhanced mode uses Exchange button from Char Info view
	default:
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
