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
#include "m4/events/keys.h"
#include "m4/vars.h"

namespace M4 {

void Hotkeys::restore_hot_keys() {
	AddSystemHotkey(KEY_HOME, capture_wrap);
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

	//AddSystemHotkey( KEY_ALT_B, toggle_inv_visible);
	AddSystemHotkey(KEY_ALT_C, toggle_commands_allowed);
	AddSystemHotkey(KEY_ALT_D, debug_memory_dumpcore_to_disk);
	AddSystemHotkey(KEY_ALT_F, dbg_mem_set_search);
	AddSystemHotkey(KEY_ALT_G, change_global_var);
	AddSystemHotkey(KEY_ALT_I, cb_MouseDialog);
	AddSystemHotkey(KEY_ALT_B, other_cheat_with_inventory_objects); // was O
	AddSystemHotkey(KEY_ALT_M, debug_memory_toggle);
	AddSystemHotkey(KEY_ALT_P, dbg_pal_toggle);
	AddSystemHotkey(KEY_ALT_R, f_io_report);
	AddSystemHotkey(KEY_ALT_S, scale_editor_toggle);
	AddSystemHotkey(KEY_ALT_T, teleport);
	AddSystemHotkey(KEY_ALT_W, paint_walk_codes);
	AddSystemHotkey(KEY_ALT_Z, pal_override);
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
	AddSystemHotkey(KEY_F2, cb_F2);
	AddSystemHotkey(KEY_F3, cb_F3);

	AddSystemHotkey('t', t_cb);
	AddSystemHotkey('u', u_cb);
	AddSystemHotkey('l', l_cb);

	AddSystemHotkey('T', t_cb);
	AddSystemHotkey('U', u_cb);
	AddSystemHotkey('L', l_cb);

	AddSystemHotkey('A', l_cb);
	AddSystemHotkey('S', t_cb);
	AddSystemHotkey('D', u_cb);
	AddSystemHotkey('F', adv_hyperwalk_to_final_destination);
	AddSystemHotkey('a', l_cb);
	AddSystemHotkey('s', t_cb);
	AddSystemHotkey('d', u_cb);
	AddSystemHotkey('f', adv_hyperwalk_to_final_destination);

	AddSystemHotkey(KEY_ALT_V, show_version);
	AddSystemHotkey(KEY_CTRL_V, show_version);
	AddSystemHotkey(KEY_ALT_X, exit_program);
	AddSystemHotkey(KEY_CTRL_X, exit_program);
	AddSystemHotkey(KEY_ALT_Q, exit_program);
	AddSystemHotkey(KEY_CTRL_Q, exit_program);
	AddSystemHotkey(KEY_SPACE, adv_hyperwalk_to_final_destination);

	if (_G(cheating_enabled)) {
		adv_enable_system_hot_keys();
	}
}

void Hotkeys::adv_enable_system_hot_keys(void) {
	term_message("System Cheats On");

	AddSystemHotkey(KEY_HOME, capture_wrap);
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
	AddSystemHotkey(KEY_ALT_F, dbg_mem_set_search);
	AddSystemHotkey(KEY_ALT_G, change_global_var);
	AddSystemHotkey(KEY_ALT_I, (HotkeyCB)cb_MouseDialog);
	AddSystemHotkey(KEY_ALT_B, other_cheat_with_inventory_objects); // was O
	AddSystemHotkey(KEY_ALT_M, debug_memory_toggle);
	AddSystemHotkey(KEY_ALT_P, dbg_pal_toggle);
	AddSystemHotkey(KEY_ALT_R, f_io_report);
	AddSystemHotkey(KEY_ALT_S, scale_editor_toggle);
	AddSystemHotkey(KEY_ALT_T, teleport);
	AddSystemHotkey(KEY_ALT_W, paint_walk_codes);
	AddSystemHotkey(KEY_ALT_Z, pal_override);
}

void Hotkeys::exit_program(void *, void *) {
	_G(kernel).going = false;
}

void Hotkeys::cb_F2(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::cb_F3(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::show_version(void *a, void *b) {
	warning("TODO: hotkey");
}

void Hotkeys::version_ok_button(void *a, void *b) {
	warning("TODO: hotkey");
}

void Hotkeys::t_cb(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::u_cb(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::l_cb(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::a_cb(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::adv_hyperwalk_to_final_destination(void *a, void *b) {
	warning("TODO: hotkey");
}

void Hotkeys::f_io_report(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::capture_wrap(void *, void *) {
	warning("TODO: hotkey");
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

void Hotkeys::player_step_up(void *, void *) {}

void Hotkeys::player_step_down(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::player_step_left(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::player_step_right(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::player_jump_up(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::player_jump_down(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::player_jump_left(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::player_jump_right(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::term_next_mode(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::camera_step_left(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::camera_step_right(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::camera_jump_left(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::camera_jump_right(void *, void *) {
	warning("TODO: hotkey");
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

void Hotkeys::change_global_var(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::cb_MouseDialog(void *, void *) {
	warning("TODO: hotkey");
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
	warning("TODO: hotkey");
}

void Hotkeys::teleport(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::paint_walk_codes(void *, void *) {
	warning("TODO: hotkey");
}

void Hotkeys::pal_override(void *, void *) {
	warning("TODO: hotkey");
}

} // End of namespace M4
