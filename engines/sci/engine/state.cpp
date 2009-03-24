/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "sci/engine/state.h"

namespace Sci {

EngineState::EngineState() : _dirseeker(this) {
	savegame_version = 0;

	widget_serial_counter = 0;

	resmgr = 0;

	game_version = 0;

	gfx_state = 0;
	old_screen = 0;

	memset(&sound, 0, sizeof(sound));	// FIXME: Remove this once/if we C++ify sfx_state_t
	sfx_init_flags = 0;
	sound_volume = 0;
	sound_mute = 0;

	restarting_flags = 0;
	have_mouse_flag = 0;

	pic_not_valid = 0;
	pic_is_new = 0;
	onscreen_console = 0;
	osc_backup = 0;

	pic_priority_table = 0;

	status_bar_foreground = 0;
	status_bar_background = 0;

	game_time = 0;

	save_dir_copy = NULL_REG;
	save_dir_edit_offset = 0;
	save_dir_copy_buf = 0;

	mouse_pointer_view = 0;
	mouse_pointer_loop = 0;
	mouse_pointer_cel = 0;
	save_mouse_pointer_view = 0;
	save_mouse_pointer_loop = 0;
	save_mouse_pointer_cel = 0;

	port_serial = 0;
	port = 0;

	memset(ega_colors, 0, sizeof(ega_colors));

	visual = 0;

	titlebar_port = 0;
	wm_port = 0;
	picture_port = 0;
	iconbar_port = 0;

	memset(&pic_visible_map, 0, sizeof(pic_visible_map));	// FIXME: Remove this once/if we C++ify gfx_map_mask_t
	pic_animate = 0;

	dyn_views_list_serial = 0;
	dyn_views = 0;

	drop_views_list_serial = 0;
	drop_views = 0;

	animation_delay = 0;
	animation_granularity = 0;

	_menubar = 0;

	priority_first = 0;
	priority_last = 0;

	pics_drawn_nr = 0;
	pics_nr = 0;
	pics = 0;

	last_wait_time = 0;

	version_lock_flag = 0;
	version = 0;
	max_version = 0;
	min_version = 0;

	_fileHandles.resize(5);

	kernel_opt_flags = 0;

	execution_stack = 0;
	execution_stack_size = 0;
	execution_stack_pos = 0;
	execution_stack_base = 0;
	execution_stack_pos_changed = 0;

	r_acc = NULL_REG;
	r_amp_rest = 0;
	r_prev = NULL_REG;

	stack_segment = 0;
	stack_base = 0;
	stack_top = 0;

	parser_segment = 0;
	parser_base = NULL_REG;
	parser_event = NULL_REG;
	script_000_segment = 0;
	script_000 = 0;

	string_frag_segment = 0;

	parser_lastmatch_word = 0;
	bp_list = 0;
	have_bp = 0;
	debug_mode = 0;
	sys_strings_segment = 0;
	sys_strings = 0;
	parser_words = 0;
	parser_words_nr = 0;
	parser_branches = 0;
	parser_rules = 0;
	parser_branches_nr = 0;
	memset(parser_nodes, 0, sizeof(parser_nodes));

	parser_valid = 0;

	synonyms = 0;
	synonyms_nr = 0;

	game_obj = NULL_REG;

	classtable_size = 0;
	classtable = 0;

	seg_manager = 0;
	gc_countdown = 0;

	kernel_names_nr = 0;
	kernel_names = 0;

	kfunct_table = 0;
	kfunct_nr = 0;

	opcodes = 0;

	memset(&selector_map, 0, sizeof(selector_map));	// FIXME: Remove this once/if we C++ify selector_map_t

	successor = 0;
}

EngineState::~EngineState() {
}


} // End of namespace Sci
