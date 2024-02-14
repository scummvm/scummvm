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

#include "m4/burger/hotkeys.h"
#include "m4/burger/vars.h"
#include "m4/burger/gui/game_menu.h"
#include "m4/gui/gui_sys.h"
#include "m4/adv_r/other.h"
#include "m4/platform/keys.h"

namespace M4 {
namespace Burger {

Dialog *Hotkeys::_versionDialog;

Hotkeys::Hotkeys() : M4::Hotkeys() {
	_versionDialog = nullptr;
}

void Hotkeys::add_hot_keys() {
	M4::Hotkeys::add_hot_keys();

	AddSystemHotkey(KEY_ALT_V, show_version);
	AddSystemHotkey(KEY_CTRL_V, show_version);

	AddSystemHotkey('t', t_cb);
	AddSystemHotkey('u', u_cb);
	AddSystemHotkey('l', l_cb);
	AddSystemHotkey('T', t_cb);
	AddSystemHotkey('U', u_cb);
	AddSystemHotkey('L', l_cb);

	AddSystemHotkey('A', l_cb);
	AddSystemHotkey('S', t_cb);
	AddSystemHotkey('D', u_cb);
	AddSystemHotkey('a', l_cb);
	AddSystemHotkey('s', t_cb);
	AddSystemHotkey('d', u_cb);

	AddSystemHotkey('F', a_cb);
	AddSystemHotkey('f', a_cb);
}

void Hotkeys::toggle_through_cursors(CursorChange cursChange) {
	cursor_states newCursor = kARROW;

	switch (cursChange) {
	case CURSCHANGE_NEXT:
		newCursor = _G(cursor_state) == kUSE ? kARROW :
			static_cast<cursor_states>((int)_G(cursor_state) + 1);
		break;

	case CURSCHANGE_PREVIOUS:
		newCursor = _G(cursor_state) == kARROW ? kUSE :
			static_cast<cursor_states>((int)_G(cursor_state) - 1);
		break;

	case CURSCHANGE_TOGGLE:
		newCursor = (_G(cursor_state) == kARROW) ? kUSE : kARROW;
		break;

	default:
		break;
	}

	switch (newCursor) {
	case kARROW:
		Hotkeys::a_cb(nullptr, nullptr);
		break;
	case kLOOK:
		Hotkeys::l_cb(nullptr, nullptr);
		break;
	case kTAKE:
		Hotkeys::t_cb(nullptr, nullptr);
		break;
	case kUSE:
		Hotkeys::u_cb(nullptr, nullptr);
		break;
	default:
		break;
	}
}

void Hotkeys::escape_key_pressed(void *, void *) {
	// Decide what to do depending on what kind of game is playing
	switch (_G(executing)) {
	case JUST_OVERVIEW:
		// Stop playing the overview
		_G(kernel).going = false;
		break;

	case INTERACTIVE_DEMO:
	case MAGAZINE_DEMO:
		if (_G(game).room_id == 901) {
			// Already on the demo menu screen, so quit game
			_G(kernel).going = false;
		} else {
			// In-game, so go back to demo menu screen
			_G(game).new_section = 9;
			_G(game).new_room = 901;
		}
		break;

	case WHOLE_GAME:
		// The real thing is playing 
		switch (_G(game).room_id) {
		case 902:
		case 904:
		case 951:
		case 971:
			// Switch to main menu
			_G(game).new_section = 9;
			_G(game).new_room = 903;
			break;

		case 903:
			// Quit game
			_G(kernel).going = false;
			break;

		default:
			if (player_commands_allowed()) {
				// Bring up the main menu
				other_save_game_for_resurrection();
				GUI::CreateGameMenu(&_G(master_palette)[0]);
			}
			break;
		}
		break;
	}
}


void Hotkeys::t_cb(void *, void *) {
	g_vars->_interface.t_cb();
}

void Hotkeys::u_cb(void *, void *) {
	g_vars->_interface.u_cb();
}

void Hotkeys::l_cb(void *, void *) {
	g_vars->_interface.l_cb();
}

void Hotkeys::a_cb(void *, void *) {
	g_vars->_interface.a_cb();
}

void Hotkeys::show_version(void *a, void *b) {
	if (!_versionDialog) {
		gr_font_set(_G(font_tiny));
		_versionDialog = DialogCreateAbsolute(190, 35, 510, 105, 242);
		_versionDialog->addButton(115, 52, " OK ", version_ok_button, 1);
		_versionDialog->addMessage(10, 5, "Orion Burger", 1);
		_versionDialog->addMessage(10, 15,
			Common::String::format("Game Version %s - %s ",
				"Nudibranchs", "May 7, 1996").c_str(), 2);
		_versionDialog->addMessage(10, 25,
			Common::String::format("M4 Library Version %s - %s ",
				"v1.400 OB", "January 21, 1996").c_str(), 3);
		_versionDialog->addMessage(10, 35,
			"Copyright (c) 1996 by Sanctuary Woods Multimedia Corporation", 4);

		Dialog_Configure(_versionDialog, 1, 1, 1);
		vmng_screen_show(_versionDialog);
	}
}

void Hotkeys::version_ok_button(void *a, void *b) {
	_versionDialog->destroy();
	_versionDialog = nullptr;
}

} // namespace Burger
} // namespace M4
