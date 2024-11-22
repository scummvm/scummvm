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

#include "m4/gui/hotkeys.h"
#include "m4/gui/gui_sys.h"
#include "m4/gui/gui_vmng.h"
#include "m4/platform/keys.h"
#include "m4/adv_r/adv_walk.h"
#include "m4/burger/burger.h"
#include "m4/burger/vars.h"
#include "m4/riddle/riddle.h"
#include "m4/riddle/vars.h"

namespace M4 {

// Since we include the vars of both games, we need to reset the _G
// to only point to the common shared variables
#undef _G
#define _G(X) (g_vars->_##X)

Dialog *Hotkeys::_changeGlobalDialog;
int Hotkeys::_globalToChange;
Dialog *Hotkeys::_teleportDialog;

Hotkeys::Hotkeys() {
	_globalToChange = 0;
	_changeGlobalDialog = nullptr;
	_teleportDialog = nullptr;
}

void Hotkeys::disable_hot_keys() {
	RemoveSystemHotkey(KEY_HOME);
	RemoveSystemHotkey(KEY_PAGE_UP);
	RemoveSystemHotkey(KEY_PAGE_DOWN);
	RemoveSystemHotkey(KEY_END);

	RemoveSystemHotkey(KEY_ALT_UP);
	RemoveSystemHotkey(KEY_ALT_DOWN);
	RemoveSystemHotkey(KEY_ALT_LEFT);
	RemoveSystemHotkey(KEY_ALT_RIGHT);

	RemoveSystemHotkey(KEY_CTRL_UP);
	RemoveSystemHotkey(KEY_CTRL_DOWN);
	RemoveSystemHotkey(KEY_CTRL_LEFT);
	RemoveSystemHotkey(KEY_CTRL_RIGHT);

	RemoveSystemHotkey(KEY_F4);

	RemoveSystemHotkey('[');
	RemoveSystemHotkey(']');
	RemoveSystemHotkey('{');
	RemoveSystemHotkey('}');

	//RemoveSystemHotkey( KEY_ALT_B);
	RemoveSystemHotkey(KEY_ALT_C);
	RemoveSystemHotkey(KEY_ALT_D);
	RemoveSystemHotkey(KEY_ALT_F);
	RemoveSystemHotkey(KEY_ALT_G);
	RemoveSystemHotkey(KEY_ALT_I);
	RemoveSystemHotkey(KEY_ALT_B);
	RemoveSystemHotkey(KEY_ALT_M);
	RemoveSystemHotkey(KEY_ALT_P);
	RemoveSystemHotkey(KEY_ALT_R);
	RemoveSystemHotkey(KEY_ALT_S);
	RemoveSystemHotkey(KEY_ALT_T);
	RemoveSystemHotkey(KEY_ALT_W);
	RemoveSystemHotkey(KEY_ALT_Z);
}

void Hotkeys::add_hot_keys() {
	AddSystemHotkey(KEY_F2, saveGame);
	AddSystemHotkey(KEY_F3, loadGame);
	AddSystemHotkey(KEY_F5, saveGame);
	AddSystemHotkey(KEY_F7, loadGame);

	AddSystemHotkey(KEY_ALT_X, exit_program);
	AddSystemHotkey(KEY_CTRL_X, exit_program);
	AddSystemHotkey(KEY_ALT_Q, exit_program);
	AddSystemHotkey(KEY_CTRL_Q, exit_program);
	AddSystemHotkey(KEY_SPACE, adv_hyperwalk_to_final_destination);
	AddSystemHotkey('f', adv_hyperwalk_to_final_destination);

	adv_enable_system_hot_keys();
}

void Hotkeys::adv_enable_system_hot_keys() {
	term_message("System Cheats On");

	AddSystemHotkey(KEY_HOME, saveScreenshot);
	AddSystemHotkey(KEY_PAGE_UP, debug_memory_next_column);
	AddSystemHotkey(KEY_PAGE_DOWN, debug_memory_prev_column);
	AddSystemHotkey(KEY_END, debug_memory_last_column);

	AddSystemHotkey(KEY_ALT_UP, player_step_up);
	AddSystemHotkey(KEY_ALT_DOWN, player_step_down);
	AddSystemHotkey(KEY_ALT_LEFT, player_step_left);
	AddSystemHotkey(KEY_ALT_RIGHT, player_step_right);

	AddSystemHotkey(KEY_CTRL_UP, player_jump_up);
	AddSystemHotkey(KEY_CTRL_DOWN, player_jump_down);
	AddSystemHotkey(KEY_CTRL_LEFT, player_jump_left);
	AddSystemHotkey(KEY_CTRL_RIGHT, player_jump_right);

	AddSystemHotkey(KEY_F4, term_next_mode);

	AddSystemHotkey('[', camera_step_left);
	AddSystemHotkey(']', camera_step_right);
	AddSystemHotkey('{', camera_jump_left);
	AddSystemHotkey('}', camera_jump_right);

	AddSystemHotkey(KEY_ALT_B, toggle_inv_visible);
	AddSystemHotkey(KEY_ALT_C, toggle_commands_allowed);
	AddSystemHotkey(KEY_ALT_D, debug_memory_dumpcore_to_disk);
	AddSystemHotkey(KEY_ALT_E, scale_editor_toggle);	// Was Alt-S, but ScummVM reserves that
	AddSystemHotkey(KEY_ALT_F, dbg_mem_set_search);
	AddSystemHotkey(KEY_ALT_G, changeGlobal);
	AddSystemHotkey(KEY_ALT_I, toggleInfoDialog);
	AddSystemHotkey(KEY_ALT_B, other_cheat_with_inventory_objects);
	AddSystemHotkey(KEY_ALT_M, debug_memory_toggle);
	AddSystemHotkey(KEY_ALT_P, dbg_pal_toggle);
	AddSystemHotkey(KEY_ALT_R, f_io_report);
	AddSystemHotkey(KEY_ALT_T, teleport);
	AddSystemHotkey(KEY_ALT_W, paint_walk_codes);
	AddSystemHotkey(KEY_ALT_Z, pal_override);
}

void Hotkeys::exit_program(void *, void *) {
	_G(kernel).going = false;
}

void Hotkeys::saveGame(void *, void *) {
	g_engine->showSaveScreen();
}

void Hotkeys::loadGame(void *, void *) {
	g_engine->showLoadScreen(M4Engine::kLoadFromHotkey);
}

void Hotkeys::adv_hyperwalk_to_final_destination(void *a, void *b) {
	M4::adv_hyperwalk_to_final_destination(a, b);
}

void Hotkeys::f_io_report(void *, void *) {
	// IO file list not available in ScummVM
}

void Hotkeys::saveScreenshot(void *, void *) {
	g_system->saveScreenshot();
}

void Hotkeys::debug_memory_next_column(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::debug_memory_prev_column(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::debug_memory_last_column(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::player_step_up(void *, void *) {
	player_step(0, -1);
}

void Hotkeys::player_step_down(void *, void *) {
	player_step(0, 1);
}

void Hotkeys::player_step_left(void *, void *) {
	player_step(-1, 0);
}

void Hotkeys::player_step_right(void *, void *) {
	player_step(1, 0);
}

void Hotkeys::player_jump_up(void *, void *) {
	player_step(0, -50);
}

void Hotkeys::player_jump_down(void *, void *) {
	player_step(0, 50);
}

void Hotkeys::player_jump_left(void *, void *) {
	player_step(-50, 0);
}

void Hotkeys::player_jump_right(void *, void *) {
	player_step(50, 0);
}

void Hotkeys::player_step(int xDelta, int yDelta) {
	player_update_info();
	_G(player_info).x += xDelta;
	_G(player_info).y += yDelta;
	ws_demand_location(_G(player_info).x, _G(player_info).y);
}

void Hotkeys::term_next_mode(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::camera_step_left(void *, void *) {
	ScreenContext *sc = vmng_screen_find(_G(gameDrawBuff), nullptr);
	MoveScreenDelta(sc, 10, 0);
}

void Hotkeys::camera_step_right(void *, void *) {
	ScreenContext *sc = vmng_screen_find(_G(gameDrawBuff), nullptr);
	MoveScreenDelta(sc, -10, 0);
}

void Hotkeys::camera_jump_left(void *, void *) {
	ScreenContext *sc = vmng_screen_find(_G(gameDrawBuff), nullptr);
	MoveScreenDelta(sc, 100, 0);
}

void Hotkeys::camera_jump_right(void *, void *) {
	ScreenContext *sc = vmng_screen_find(_G(gameDrawBuff), nullptr);
	MoveScreenDelta(sc, -100, 0);
}

void Hotkeys::toggle_inv_visible(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::toggle_commands_allowed(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::debug_memory_dumpcore_to_disk(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::dbg_mem_set_search(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::changeGlobal(void *, void *) {
	if (!_changeGlobalDialog) {
		gr_font_set(_G(font_tiny));
		_changeGlobalDialog = DialogCreateAbsolute(250, 120, 450, 220, 242);
		_changeGlobalDialog->addButton(60, 40, " Change Global Variable ",
			changeGlobalChange, 1);
		_changeGlobalDialog->addButton(10, 40, " Cancel ", changeGlobalCancel, 2);
		char sep[5+1] = "-----";
		_changeGlobalDialog->addTextField(50, 7, gr_font_string_width(sep) + 50, sep, nullptr, 3, 5);
		_changeGlobalDialog->configure(3, 1, 2);
		_changeGlobalDialog->show();
	}
}

void Hotkeys::changeGlobalChange(void *, void *) {
	// Get the global number to change
	Item *textField = _changeGlobalDialog->getItem(3);
	_globalToChange = atoi(textField->prompt);

	if (!_globalToChange) {
		changeGlobalCancel(nullptr, nullptr);
		return;
	}

	// Destroy the current dialog
	_changeGlobalDialog->destroy();

	// Create secondary dialog to get value to set global to
	int globalVal = (g_engine->getGameType() == GType_Burger) ?
		Burger::g_vars->_flags[(Burger::Flag)_globalToChange] :
		Riddle::g_vars->_flags[(Riddle::Flag)_globalToChange];

	_changeGlobalDialog = DialogCreateAbsolute(250, 120, 450, 220, 242);
	_changeGlobalDialog->addButton(60, 40,
		Common::String::format("Assign new value to #%d ", _globalToChange).c_str(),
		changeGlobalDoChange, 1);
	_changeGlobalDialog->addButton(10, 40, " Cancel ", changeGlobalCancel, 2);
	char val[5+1] = "MMMMM";
	_changeGlobalDialog->addTextField(50, 7,
		gr_font_string_width(val) + 50,
		Common::String::format("%5d", globalVal).c_str(),
		nullptr, 3, 5);
	_changeGlobalDialog->configure(3, 1, 2);
	_changeGlobalDialog->show();
}

void Hotkeys::changeGlobalDoChange(void *, void *) {
	Item *textField = _changeGlobalDialog->getItem(3);
	int globalVal = atoi(textField->prompt);

	if (g_engine->getGameType() == GType_Burger)
		Burger::g_vars->_flags[(Burger::Flag)_globalToChange] = globalVal;
	else
		Riddle::g_vars->_flags[(Riddle::Flag)_globalToChange] = globalVal;

	_changeGlobalDialog->destroy();
	_changeGlobalDialog = nullptr;
}

void Hotkeys::changeGlobalCancel(void *, void *) {
	_changeGlobalDialog->destroy();
	_changeGlobalDialog = nullptr;
}

void Hotkeys::toggleInfoDialog(void *, void *) {
	if (!_G(showMousePos)) {
		vmng_screen_show(_G(mousePosDialog));
		_G(showMousePos) = true;
		pal_override();
	} else {
		vmng_screen_hide(_G(mousePosDialog));
		_G(showMousePos) = false;
	}
}

void Hotkeys::other_cheat_with_inventory_objects(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::debug_memory_toggle(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::dbg_pal_toggle(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::scale_editor_toggle(void *, void *) {
	M4::scale_editor_toggle();
}

void Hotkeys::teleport(void *, void *) {
	if (!_teleportDialog) {
		gr_font_set(_G(font_tiny));
		_teleportDialog = DialogCreateAbsolute(260, 150, 385, 200, 61);
		_teleportDialog->addButton(60, 30, " Teleport ", teleportOk, 1);
		_teleportDialog->addButton(10, 30, " Cancel ", teleportCancel, 2);
		char sep_placeholder[4+1] = "MMMM";
		_teleportDialog->addTextField(100, 10, gr_font_string_width(sep_placeholder) + 100,
			"---", nullptr, 3, 3);
		_teleportDialog->addMessage(10, 12, "Teleport where?", 4);
		_teleportDialog->configure(3, 1, 2);
		_teleportDialog->show();
		pal_override(nullptr, nullptr);
	}
}

void Hotkeys::teleportOk(void *, void *) {
	Item *textField = _teleportDialog->getItem(3);
	_G(game).setRoom(atoi(textField->prompt));
	_G(kernel).teleported_in = true;

	_teleportDialog->destroy();
	_teleportDialog = nullptr;
}

void Hotkeys::teleportCancel(void *, void *) {
	_teleportDialog->destroy();
	_teleportDialog = nullptr;
}

void Hotkeys::paint_walk_codes(void *, void *) {
	Buffer *bgBuff = _G(game_bgBuff)->get_buffer();
	Buffer *drawBuff = _G(gameDrawBuff)->get_buffer();

	for (int yp = 0; yp < bgBuff->h; ++yp) {
		const byte *bgLine = gr_buffer_pointer(bgBuff, 0, yp);
		byte *drawLine = gr_buffer_pointer(drawBuff, 0, yp);

		for (int xp = 0; xp < bgBuff->w; ++xp, ++bgLine, ++drawLine) {
			if (*bgLine & 0x10)
				*drawLine = gr_pal_get_ega_color(1);
		}
	}

	_G(game_bgBuff)->release();
	_G(gameDrawBuff)->release();
	RestoreScreens(0, 0, 639, 479);
}

void Hotkeys::pal_override(void *, void *) {
	gr_pal_interface(_G(master_palette));
	gr_pal_set(_G(master_palette));
	Dialog_Refresh_All();
}

} // End of namespace M4
