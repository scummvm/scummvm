
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

#ifndef M4_GUI_HOTKEYS_H
#define M4_GUI_HOTKEYS_H

#include "m4/m4_types.h"
#include "m4/gui/gui_dialog.h"

namespace M4 {

enum CursorChange {
	CURSCHANGE_NONE, CURSCHANGE_NEXT, CURSCHANGE_PREVIOUS, CURSCHANGE_TOGGLE
};

struct Hotkeys {
private:
	static Dialog *_teleportDialog;
	static Dialog *_changeGlobalDialog;
	static int _globalToChange;

private:
	static void exit_program(void *, void *);
	static void adv_hyperwalk_to_final_destination(void *a, void *b);
	static void saveScreenshot(void *, void *);
	static void debug_memory_next_column(void *, void *);
	static void debug_memory_prev_column(void *, void *);
	static void debug_memory_last_column(void *, void *);

	static void player_step_up(void *, void *);
	static void player_step_down(void *, void *);
	static void player_step_left(void *, void *);
	static void player_step_right(void *, void *);
	static void player_jump_up(void *, void *);
	static void player_jump_down(void *, void *);
	static void player_jump_left(void *, void *);
	static void player_jump_right(void *, void *);
	static void player_step(int xDelta, int yDelta);

	static void term_next_mode(void *, void *);

	static void camera_step_left(void *, void *);
	static void camera_step_right(void *, void *);
	static void camera_jump_left(void *, void *);
	static void camera_jump_right(void *, void *);

	static void toggle_inv_visible(void *, void *);
	static void toggle_commands_allowed(void *, void *);
	static void debug_memory_dumpcore_to_disk(void *, void *);
	static void dbg_mem_set_search(void *, void *);

	static void toggleInfoDialog(void *, void *);
	static void other_cheat_with_inventory_objects(void *, void *); // was O
	static void debug_memory_toggle(void *, void *);
	static void dbg_pal_toggle(void *, void *);
	static void f_io_report(void *, void *);
	static void scale_editor_toggle(void *, void *);

	static void teleport(void *, void *);
	static void teleportOk(void *, void *);
	static void teleportCancel(void *, void *);

	static void changeGlobal(void *, void *);
	static void changeGlobalCancel(void *, void *);
	static void changeGlobalChange(void *, void *);
	static void changeGlobalDoChange(void *, void *);

	static void paint_walk_codes(void *, void *);
	static void pal_override(void *a = nullptr, void *b = nullptr);

protected:
	virtual void adv_enable_system_hot_keys();

public:
	Hotkeys();
	virtual ~Hotkeys() {}

	virtual void add_hot_keys();
	void disable_hot_keys();

	static void saveGame(void *, void *);
	static void loadGame(void *, void *);

	virtual void toggle_through_cursors(CursorChange cursChange = CURSCHANGE_NEXT) = 0;
};

} // End of namespace M4

#endif
