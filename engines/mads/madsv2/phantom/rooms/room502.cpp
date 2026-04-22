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

#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/camera.h"
#include "mads/madsv2/core/cycle.h"
#include "mads/madsv2/core/env.h"
#include "mads/madsv2/core/error.h"
#include "mads/madsv2/core/fileio.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/phantom/mads/speeches.h"
#include "mads/madsv2/phantom/rooms/section5.h"
#include "mads/madsv2/phantom/rooms/room502.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

static void room_502_initialize_panels() {
	int count;
	int count_2 = 1;
	int figure = 2;
	int ss_type;

	for (count = 0; count < 16; count++) {

		if (previous_room != KERNEL_RESTORING_GAME) {
			local->puzzle_picture[count] = (byte)imath_random(1, 4);
		}

		figure += (local->puzzle_picture[count] * 3) - 3;
		local->puzzle_sprite[count] = (byte)figure;

		switch (count) {
		case 0:
		case 1:
		case 2:
		case 3: ss_type = ss[fx_row_1]; break;

		case 4:
		case 5:
		case 6:
		case 7: ss_type = ss[fx_row_2]; break;

		case 8:
		case 9:
		case 10:
		case 11: ss_type = ss[fx_row_3]; break;

		default: ss_type = ss[fx_row_4]; break;
		}

		seq[fx_panel] = kernel_seq_stamp(ss_type, false, figure);
		kernel_seq_depth(seq[fx_panel], 14);
		local->puzzle_sequence[count] = seq[fx_panel];
		/* save the handle for later deletion */

		++count_2;
		if (count_2 >= 5) {
			count_2 = 1;
		}

		switch (count_2) {
		case 1: figure = 2;  break;
		case 2: figure = 14; break;
		case 3: figure = 26; break;
		case 4: figure = 38; break;
		}
	}
}

static void room_502_load_cycling_info() {
	int error_code = 0;
	int count;
	int cycle;
	int num_colors;
	int total_colors = 0;
	byte *chunk = NULL;
	Common::SeekableReadStream *handle = NULL;
	RGBcolor *color_marker;

	mem_save_free();

	local->cycle_pointer = NULL;

	chunk = (byte *)mem_get(mem_get_avail() - 256);
	if (chunk == NULL) goto done;

	color_marker = (RGBcolor *)chunk;

	handle = env_open("*RM502.CYC", "rb");
	if (handle == NULL) {
		error_code = 100;
		goto done;
	}

	for (count = 0; count < num_cycle_stages; count++) {

		if (!fileio_fread_f(&local->cycle_list[count], sizeof(CycleList), 1, handle)) {
			error_code = 200 + count;
			goto done;
		}

		num_colors = 0;
		for (cycle = 0; cycle < local->cycle_list[count].num_cycles; cycle++) {
			local->cycle_list[count].table[cycle].first_list_color = (byte)num_colors;
			num_colors += local->cycle_list[count].table[cycle].num_colors;
		}

		local->cycle_color[count] = color_marker;

		if (!fileio_fread_f(color_marker, sizeof(RGBcolor) * num_colors, 1, handle)) {
			error_code = 300 + count;
			goto done;
		}

		color_marker += num_colors;
		total_colors += num_colors;
	}

	mem_adjust(chunk, (total_colors * sizeof(RGBcolor)));

	local->cycle_pointer = chunk;
	local->cycle_bookkeep = 0;

done:
	delete handle;
	if (local->cycle_pointer == NULL) {
		error_report(ERROR_BREAK_POINT, ERROR, MODULE_UNKNOWN, 502, error_code);
	}
	mem_restore_free();
}

static void get_panel_info(int *walk_to_x, int *walk_to_y, int *panel, int mouse_x, int mouse_y, int *interim_x, int *interim_y) {
	/* walk_to_x will return the x coordinate for walking to that panel */
	/* walk_to_y will return the y coordinate for walking to that panel */
	/* panel will return the number of the panel (top row is 1-4, bottom row is 13-16) */
	/* pass in mouse_x and mouse_y to figure all this shit out */

	*walk_to_y = COLUMN_Y;

	if ((mouse_x >= 120) && (mouse_x <= 139)) {
		*interim_x = 129;
		if ((mouse_y >= 75) && (mouse_y <= 90)) {
			*panel = 0;  *interim_y = 90;  *walk_to_x = COLUMN_1_X;
		}
		if ((mouse_y >= 91) && (mouse_y <= 106)) {
			*panel = 4;  *interim_y = 106; *walk_to_x = COLUMN_1_X;
		}
		if ((mouse_y >= 107) && (mouse_y <= 122)) {
			*panel = 8;  *interim_y = 122; *walk_to_x = COLUMN_1_X;
		}
		if ((mouse_y >= 123) && (mouse_y <= 137)) {
			*panel = 12; *interim_y = 138; *walk_to_x = COLUMN_1_X;
		}
	}

	if ((mouse_x >= 140) && (mouse_x <= 159)) {
		*interim_x = 149;
		if ((mouse_y >= 75) && (mouse_y <= 90)) {
			*panel = 1;  *interim_y = 90;  *walk_to_x = COLUMN_2_X;
		}
		if ((mouse_y >= 91) && (mouse_y <= 106)) {
			*panel = 5;  *interim_y = 106; *walk_to_x = COLUMN_2_X;
		}
		if ((mouse_y >= 107) && (mouse_y <= 122)) {
			*panel = 9;  *interim_y = 122; *walk_to_x = COLUMN_2_X;
		}
		if ((mouse_y >= 123) && (mouse_y <= 137)) {
			*panel = 13; *interim_y = 138; *walk_to_x = COLUMN_2_X;
		}
	}

	if ((mouse_x >= 160) && (mouse_x <= 179)) {
		*interim_x = 169;
		if ((mouse_y >= 75) && (mouse_y <= 90)) {
			*panel = 2;  *interim_y = 90;  *walk_to_x = COLUMN_3_X;
		}
		if ((mouse_y >= 91) && (mouse_y <= 106)) {
			*panel = 6;  *interim_y = 106; *walk_to_x = COLUMN_3_X;
		}
		if ((mouse_y >= 107) && (mouse_y <= 122)) {
			*panel = 10; *interim_y = 122; *walk_to_x = COLUMN_3_X;
		}
		if ((mouse_y >= 123) && (mouse_y <= 137)) {
			*panel = 14; *interim_y = 138; *walk_to_x = COLUMN_3_X;
		}
	}

	if ((mouse_x >= 180) && (mouse_x <= 199)) {
		*interim_x = 189;
		if ((mouse_y >= 75) && (mouse_y <= 90)) {
			*panel = 3;  *interim_y = 90;  *walk_to_x = COLUMN_4_X;
		}
		if ((mouse_y >= 91) && (mouse_y <= 106)) {
			*panel = 7;  *interim_y = 106; *walk_to_x = COLUMN_4_X;
		}
		if ((mouse_y >= 107) && (mouse_y <= 122)) {
			*panel = 11; *interim_y = 122; *walk_to_x = COLUMN_4_X;
		}
		if ((mouse_y >= 123) && (mouse_y <= 137)) {
			*panel = 15; *interim_y = 138; *walk_to_x = COLUMN_4_X;
		}
	}
}

static void animate_panels() {
	int temp;  /* for synching purposes */
	int new_sprite;
	int count;
	int yippie = true;

	switch (kernel.trigger) {
	case ROOM_502_ANIMATE_PANELS:
		sound_play(N_PanelClank);
		kernel_seq_delete(local->puzzle_sequence[local->panel_pushed]);
		switch (local->panel_pushed) {
		case 0:
		case 1:
		case 2:
		case 3:
			seq[fx_row_1] = kernel_seq_stamp(ss[fx_row_1], false, local->puzzle_sprite[local->panel_pushed] - 1);
			kernel_seq_depth(seq[fx_row_1], 14);
			kernel_timing_trigger(5, ROOM_502_ANIMATE_PANELS + 1);
			break;

		case 4:
		case 5:
		case 6:
		case 7:
			seq[fx_row_2] = kernel_seq_stamp(ss[fx_row_2], false, local->puzzle_sprite[local->panel_pushed] - 1);
			kernel_seq_depth(seq[fx_row_2], 14);
			kernel_timing_trigger(5, ROOM_502_ANIMATE_PANELS + 1);
			break;

		case 8:
		case 9:
		case 10:
		case 11:
			seq[fx_row_3] = kernel_seq_stamp(ss[fx_row_3], false, local->puzzle_sprite[local->panel_pushed] - 1);
			kernel_seq_depth(seq[fx_row_3], 14);
			kernel_timing_trigger(5, ROOM_502_ANIMATE_PANELS + 1);
			break;

		default:
			seq[fx_row_4] = kernel_seq_stamp(ss[fx_row_4], false, local->puzzle_sprite[local->panel_pushed] - 1);
			kernel_seq_depth(seq[fx_row_4], 14);
			kernel_timing_trigger(5, ROOM_502_ANIMATE_PANELS + 1);
			break;
		}
		break;

	case ROOM_502_ANIMATE_PANELS + 1:
		switch (local->panel_pushed) {
		case 0:
		case 1:
		case 2:
		case 3:
			kernel_seq_delete(seq[fx_row_1]);
			break;

		case 4:
		case 5:
		case 6:
		case 7:
			kernel_seq_delete(seq[fx_row_2]);
			break;

		case 8:
		case 9:
		case 10:
		case 11:
			kernel_seq_delete(seq[fx_row_3]);
			break;

		default:
			kernel_seq_delete(seq[fx_row_4]);
			break;
		}

		seq[fx_interim] = kernel_seq_forward(ss[fx_interim], false, 5, 0, 0, 1);
		kernel_seq_depth(seq[fx_interim], 14);
		kernel_seq_loc(seq[fx_interim], local->interim_x, local->interim_y);
		kernel_seq_range(seq[fx_interim], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_interim], KERNEL_TRIGGER_EXPIRE, 0, ROOM_502_ANIMATE_PANELS + 2);
		break;

	case ROOM_502_ANIMATE_PANELS + 2:
		temp = seq[fx_interim];

		switch (local->panel_pushed) {
		case 0:
		case 4:
		case 8:
		case 12:
			new_sprite = local->puzzle_sprite[local->panel_pushed] + 4;
			if (new_sprite > 12) {
				new_sprite = 3;
			}
			break;

		case 1:
		case 5:
		case 9:
		case 13:
			new_sprite = local->puzzle_sprite[local->panel_pushed] + 4;
			if (new_sprite > 24) {
				new_sprite = 15;
			}
			break;

		case 2:
		case 6:
		case 10:
		case 14:
			new_sprite = local->puzzle_sprite[local->panel_pushed] + 4;
			if (new_sprite > 36) {
				new_sprite = 27;
			}
			break;

		default:
			new_sprite = local->puzzle_sprite[local->panel_pushed] + 4;
			if (new_sprite > 48) {
				new_sprite = 39;
			}
			break;
		}

		switch (local->panel_pushed) {
		case 0:
		case 1:
		case 2:
		case 3:
			seq[fx_row_1] = kernel_seq_stamp(ss[fx_row_1], false, new_sprite);
			kernel_seq_depth(seq[fx_row_1], 14);
			kernel_synch(KERNEL_SERIES, seq[fx_row_1], KERNEL_SERIES, temp);
			kernel_timing_trigger(5, ROOM_502_ANIMATE_PANELS + 3);
			break;

		case 4:
		case 5:
		case 6:
		case 7:
			seq[fx_row_2] = kernel_seq_stamp(ss[fx_row_2], false, new_sprite);
			kernel_seq_depth(seq[fx_row_2], 14);
			kernel_synch(KERNEL_SERIES, seq[fx_row_2], KERNEL_SERIES, temp);
			kernel_timing_trigger(5, ROOM_502_ANIMATE_PANELS + 3);
			break;

		case 8:
		case 9:
		case 10:
		case 11:
			seq[fx_row_3] = kernel_seq_stamp(ss[fx_row_3], false, new_sprite);
			kernel_seq_depth(seq[fx_row_3], 14);
			kernel_synch(KERNEL_SERIES, seq[fx_row_3], KERNEL_SERIES, temp);
			kernel_timing_trigger(5, ROOM_502_ANIMATE_PANELS + 3);
			break;

		default:
			seq[fx_row_4] = kernel_seq_stamp(ss[fx_row_4], false, new_sprite);
			kernel_seq_depth(seq[fx_row_4], 14);
			kernel_synch(KERNEL_SERIES, seq[fx_row_4], KERNEL_SERIES, temp);
			kernel_timing_trigger(5, ROOM_502_ANIMATE_PANELS + 3);
			break;
		}
		break;

	case ROOM_502_ANIMATE_PANELS + 3:


		switch (local->panel_pushed) {
		case 0:
		case 1:
		case 2:
		case 3:
			kernel_seq_delete(seq[fx_row_1]);
			break;

		case 4:
		case 5:
		case 6:
		case 7:
			kernel_seq_delete(seq[fx_row_2]);
			break;

		case 8:
		case 9:
		case 10:
		case 11:
			kernel_seq_delete(seq[fx_row_3]);
			break;

		default:
			kernel_seq_delete(seq[fx_row_4]);
			break;
		}

		new_sprite = local->puzzle_sprite[local->panel_pushed] + 3;

		switch (local->panel_pushed) {
		case 0:
		case 4:
		case 8:
		case 12:
			if (new_sprite > 12) {
				new_sprite = 2;
			}
			break;

		case 1:
		case 5:
		case 9:
		case 13:
			if (new_sprite > 24) {
				new_sprite = 14;
			}
			break;

		case 2:
		case 6:
		case 10:
		case 14:
			if (new_sprite > 36) {
				new_sprite = 26;
			}
			break;

		default:
			if (new_sprite > 48) {
				new_sprite = 38;
			}
			break;
		}
		local->puzzle_sprite[local->panel_pushed] = (byte)new_sprite;
		++local->puzzle_picture[local->panel_pushed];
		if (local->puzzle_picture[local->panel_pushed] >= 5) {
			local->puzzle_picture[local->panel_pushed] = 1;
		}

		switch (local->panel_pushed) {
		case 0:
		case 1:
		case 2:
		case 3:
			seq[fx_row_1] = kernel_seq_stamp(ss[fx_row_1], false, new_sprite);
			kernel_seq_depth(seq[fx_row_1], 14);
			local->puzzle_sequence[local->panel_pushed] = seq[fx_row_1];
			break;

		case 4:
		case 5:
		case 6:
		case 7:
			seq[fx_row_2] = kernel_seq_stamp(ss[fx_row_2], false, new_sprite);
			kernel_seq_depth(seq[fx_row_2], 14);
			local->puzzle_sequence[local->panel_pushed] = seq[fx_row_2];
			break;

		case 8:
		case 9:
		case 10:
		case 11:
			seq[fx_row_3] = kernel_seq_stamp(ss[fx_row_3], false, new_sprite);
			kernel_seq_depth(seq[fx_row_3], 14);
			local->puzzle_sequence[local->panel_pushed] = seq[fx_row_3];
			break;

		default:
			seq[fx_row_4] = kernel_seq_stamp(ss[fx_row_4], false, new_sprite);
			kernel_seq_depth(seq[fx_row_4], 14);
			local->puzzle_sequence[local->panel_pushed] = seq[fx_row_4];
			break;
		}

		for (count = 0; count < 16; count++) {
			if (local->puzzle_picture[count] != 1) {
				yippie = false;
			}
		}

		if (yippie) {
			if (!local->yippie) {
				local->yippie = true;
				kernel_seq_delete(seq[fx_trap_door]);
				aa[0] = kernel_run_animation(kernel_name('t', 1), ROOM_502_TRAP_DOOR_OPEN);
			}
		}
		break;
	}
}


void room_502_init() {
	room_502_load_cycling_info();  /* Top of init code */

	global_speech_load(speech_raoul_fire);

	local->panel_pushed = -1;
	local->turning_panel = false;
	local->fire_1_on = false;
	local->fire_2_on = false;
	local->fire_3_on = false;
	local->fire_4_on = false;

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->hot_clock = kernel.clock;
		local->cycle_stage = 0;
		local->hot_timer = 0;
		local->death_timer = 0;
		local->message_stage = 1;
		local->room_getting_hotter = true;
		local->yippie = false;
	}

	kernel_flip_hotspot(words_rope, false);
	kernel_flip_hotspot_loc(words_trap_door, false, TRAP_DOOR_HS_X, TRAP_DOOR_HS_Y);

	/* ===================== Load Sprite Series ================== */

	ss[fx_fire_1] = kernel_load_series(kernel_name('x', 2), false);
	ss[fx_fire_2] = kernel_load_series(kernel_name('x', 3), false);
	ss[fx_fire_3] = kernel_load_series(kernel_name('x', 4), false);
	ss[fx_fire_4] = kernel_load_series(kernel_name('x', 5), false);
	ss[fx_death] = kernel_load_series(kernel_name('a', 2), false);
	ss[fx_door] = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_trap_door] = kernel_load_series(kernel_name('x', 1), false);
	ss[fx_rope_thrower] = kernel_load_series(kernel_name('a', 1), false);
	ss[fx_rope_climber] = kernel_load_series(kernel_name('a', 3), false);
	ss[fx_rope] = kernel_load_series(kernel_name('x', 6), false);
	ss[fx_pusher] = kernel_load_series(kernel_name('a', 0), false);
	ss[fx_row_1] = kernel_load_series(kernel_name('j', 0), false);
	ss[fx_row_2] = kernel_load_series(kernel_name('k', 0), false);
	ss[fx_row_3] = kernel_load_series(kernel_name('l', 0), false);
	ss[fx_row_4] = kernel_load_series(kernel_name('m', 0), false);
	ss[fx_interim] = kernel_load_series(kernel_name('h', 0), false);


	/* =================== Stamp morphing door =================== */

	if (previous_room != KERNEL_RESTORING_GAME) {
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, 1);
		kernel_seq_depth(seq[fx_door], 14);

	} else {
		kernel_draw_to_background(ss[fx_door], KERNEL_LAST, KERNEL_HOME, KERNEL_HOME, 0, 100);
	}

	/* ========================= Previous Rooms ================== */

	if ((previous_room == 501) || (previous_room != KERNEL_RESTORING_GAME)) {

		if (!player.been_here_before) {

			if (player_has(rope)) {
				global[cable_hook_was_seperate] = true;
			} else {
				global[cable_hook_was_seperate] = false;
			}

		} else {

			if (global[cable_hook_was_seperate]) {
				inter_give_to_player(rope);
				inter_give_to_player(cable_hook);
				inter_move_object(rope_with_hook, NOWHERE);
			} else {
				inter_move_object(rope, NOWHERE);
				inter_move_object(cable_hook, NOWHERE);
				inter_give_to_player(rope_with_hook);
			}
		}

		player.x = PLAYER_X_FROM_501;
		player.y = PLAYER_Y_FROM_501;
		player.facing = FACING_EAST;
		player.commands_allowed = false;
		player_walk(WALK_TO_X_FROM_501, WALK_TO_Y_FROM_501, FACING_EAST);
		player_walk_trigger(ROOM_502_ENTER);
	}

	room_502_initialize_panels();

	/* ===================== Stamp trap door ===================== */

	if (local->yippie) {
		seq[fx_trap_door] = kernel_seq_stamp(ss[fx_trap_door], false, 6);
		kernel_seq_depth(seq[fx_trap_door], 1);
		kernel_flip_hotspot(words_trap_door, false);
		kernel_flip_hotspot_loc(words_trap_door, true, TRAP_DOOR_HS_X, TRAP_DOOR_HS_Y);
		if (!player_has(rope_with_hook) && !player_has(cable_hook)) {
			seq[fx_rope] = kernel_seq_stamp(ss[fx_rope], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_rope], 12);
			kernel_flip_hotspot(words_rope, true);
		}

	} else {
		seq[fx_trap_door] = kernel_seq_stamp(ss[fx_trap_door], false, 1);
		kernel_seq_depth(seq[fx_trap_door], 14);
	}

	section_5_music();
}

void room_502_shutdown() {
	if (local->cycle_pointer != NULL) {
		mem_free(local->cycle_pointer);
	}
}

void room_502_pre_parser() {
	int walk_to_x;
	int walk_to_y;
	int panel;
	int interim_x;
	int interim_y;

	if (player_said_2(push, panel)) {
		get_panel_info(&walk_to_x, &walk_to_y, &panel, inter_point_x, inter_point_y, &interim_x, &interim_y);
		player_walk(walk_to_x, walk_to_y, FACING_NORTH);
	}

	if (player_said_2(climb, rope) ||
		player_said_2(climb_through, trap_door)) {
		if (local->yippie) {
			player_walk(TRAP_DOOR_X, TRAP_DOOR_Y, FACING_NORTH);
		}
	}

	if (player_said_3(throw, rope_with_hook, trap_door) ||
		player_said_2(grapple, trap_door)) {
		if (local->yippie) {
			player_walk(THROW_UP_X, THROW_UP_Y, FACING_NORTH);
		}
	}
}

void room_502_parser() {
	int temp;  /* for synching purposes */
	int walk_to_x;
	int walk_to_y;
	int panel;
	int interim_x;
	int interim_y;

	if (kernel.trigger >= ROOM_502_ANIMATE_PANELS) {
		animate_panels();
		goto handled;
	}

	switch (kernel.trigger) {
	case ROOM_502_ROW_1:
		player.commands_allowed = false;
		player.walker_visible = false;
		seq[fx_pusher] = kernel_seq_pingpong(ss[fx_pusher], false, 7, 0, 0, 2);
		kernel_seq_player(seq[fx_pusher], true);
		kernel_seq_depth(seq[fx_pusher], 1);
		kernel_seq_range(seq[fx_pusher], 14, 18);
		kernel_seq_trigger(seq[fx_pusher], KERNEL_TRIGGER_EXPIRE, 0, ROOM_502_ROW_1 + 1);
		kernel_seq_trigger(seq[fx_pusher], KERNEL_TRIGGER_SPRITE, 18, ROOM_502_ANIMATE_PANELS);
		goto handled;
		break;

	case ROOM_502_ROW_1 + 1:
		temp = seq[fx_pusher];
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, temp);
		player.walker_visible = true;
		player.commands_allowed = true;
		kernel_timing_trigger(5, ROOM_502_ROW_3 + 2);
		goto handled;
		break;

	case ROOM_502_ROW_2:
		player.commands_allowed = false;
		player.walker_visible = false;
		seq[fx_pusher] = kernel_seq_pingpong(ss[fx_pusher], false, 7, 0, 0, 2);
		kernel_seq_player(seq[fx_pusher], true);
		kernel_seq_depth(seq[fx_pusher], 1);
		kernel_seq_range(seq[fx_pusher], 8, 13);
		kernel_seq_trigger(seq[fx_pusher], KERNEL_TRIGGER_EXPIRE, 0, ROOM_502_ROW_2 + 1);
		kernel_seq_trigger(seq[fx_pusher], KERNEL_TRIGGER_SPRITE, 13, ROOM_502_ANIMATE_PANELS);
		goto handled;
		break;

	case ROOM_502_ROW_2 + 1:
		temp = seq[fx_pusher];
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, temp);
		player.walker_visible = true;
		player.commands_allowed = true;
		kernel_timing_trigger(5, ROOM_502_ROW_3 + 2);
		goto handled;
		break;

	case ROOM_502_ROW_3:
		player.commands_allowed = false;
		player.walker_visible = false;
		seq[fx_pusher] = kernel_seq_pingpong(ss[fx_pusher], false, 9, 0, 0, 2);
		kernel_seq_player(seq[fx_pusher], true);
		kernel_seq_depth(seq[fx_pusher], 1);
		kernel_seq_range(seq[fx_pusher], 5, 7);
		kernel_seq_trigger(seq[fx_pusher], KERNEL_TRIGGER_EXPIRE, 0, ROOM_502_ROW_3 + 1);
		kernel_seq_trigger(seq[fx_pusher], KERNEL_TRIGGER_SPRITE, 7, ROOM_502_ANIMATE_PANELS);
		goto handled;
		break;

	case ROOM_502_ROW_3 + 1:
		temp = seq[fx_pusher];
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, temp);
		player.walker_visible = true;
		kernel_timing_trigger(5, ROOM_502_ROW_3 + 2);
		goto handled;
		break;

	case ROOM_502_ROW_3 + 2:
		local->turning_panel = false;
		player.commands_allowed = true;
		goto handled;
		break;

	case ROOM_502_ROW_4:
		player.commands_allowed = false;
		player.walker_visible = false;
		seq[fx_pusher] = kernel_seq_pingpong(ss[fx_pusher], false, 8, 0, 0, 2);
		kernel_seq_player(seq[fx_pusher], true);
		kernel_seq_depth(seq[fx_pusher], 1);
		kernel_seq_range(seq[fx_pusher], 1, 4);
		kernel_seq_trigger(seq[fx_pusher], KERNEL_TRIGGER_EXPIRE, 0, ROOM_502_ROW_4 + 1);
		kernel_seq_trigger(seq[fx_pusher], KERNEL_TRIGGER_SPRITE, 4, ROOM_502_ANIMATE_PANELS);
		goto handled;
		break;

	case ROOM_502_ROW_4 + 1:
		temp = seq[fx_pusher];
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, temp);
		player.walker_visible = true;
		kernel_timing_trigger(5, ROOM_502_ROW_3 + 2);
		goto handled;
		break;

	case ROOM_502_TRAP_DOOR_OPEN:
		seq[fx_trap_door] = kernel_seq_stamp(ss[fx_trap_door], false, 6);
		kernel_flip_hotspot_loc(words_rope, true, TRAP_DOOR_HS_X, TRAP_DOOR_HS_Y);
		kernel_seq_depth(seq[fx_trap_door], 1);
		kernel_flip_hotspot(words_trap_door, false);
		kernel_flip_hotspot_loc(words_trap_door, true, TRAP_DOOR_HS_X, TRAP_DOOR_HS_Y);
		if (!local->turning_panel) {
			text_show(text_502_16);
		}
		goto handled;
		break;
	}

	if (player_said_2(push, panel)) {
		if (local->turning_panel) {
			goto handled;
		}

		get_panel_info(&walk_to_x, &walk_to_y, &panel, inter_point_x, inter_point_y, &interim_x, &interim_y);

		local->panel_pushed = (byte)panel;
		local->interim_x = interim_x;
		local->interim_y = (byte)interim_y;
		local->turning_panel = true;

		switch (panel) {
		case 0:
		case 1:
		case 2:
		case 3:
			kernel_timing_trigger(1, ROOM_502_ROW_1);
			break;

		case 4:
		case 5:
		case 6:
		case 7:
			kernel_timing_trigger(1, ROOM_502_ROW_2);
			break;

		case 8:
		case 9:
		case 10:
		case 11:
			kernel_timing_trigger(1, ROOM_502_ROW_3);
			break;

		default:
			kernel_timing_trigger(1, ROOM_502_ROW_4);
			break;
		}
		goto handled;
	}

	if (player_said_3(throw, rope_with_hook, trap_door) ||
		player_said_2(grapple, trap_door)) {
		if (local->yippie) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible = false;
				local->turning_panel = true;
				seq[fx_rope_thrower] = kernel_seq_forward(ss[fx_rope_thrower], false, 6, 0, 0, 1);
				kernel_seq_depth(seq[fx_rope_thrower], 13);
				kernel_seq_range(seq[fx_rope_thrower], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_trigger(seq[fx_rope_thrower], KERNEL_TRIGGER_EXPIRE, 0, ROOM_502_THROW_UP);
				kernel_seq_trigger(seq[fx_rope_thrower], KERNEL_TRIGGER_SPRITE, 10, ROOM_502_THROW_UP + 1);
				inter_move_object(rope_with_hook, NOWHERE);
				break;

			case ROOM_502_THROW_UP + 1:
				sound_play(N_LassoThrow);
				break;

			case ROOM_502_THROW_UP:
				temp = seq[fx_rope_thrower];
				player.commands_allowed = true;
				player.walker_visible = true;
				local->turning_panel = false;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, temp);
				seq[fx_rope] = kernel_seq_stamp(ss[fx_rope], false, KERNEL_LAST);
				kernel_seq_depth(seq[fx_rope], 13);
				kernel_flip_hotspot(words_rope, true);
				break;
			}

		} else {
			text_show(text_502_29);
			/* throw rope with hook at trap door when it is closed */
		}
		goto handled;
	}

	if (player_said_2(climb_through, trap_door)) {
		if (player_has(rope_with_hook) || player_has(cable_hook)) {
			text_show(text_502_28);
			/* climb through trap door when rope is not installed */
			goto handled;
		}
	}

	if (player_said_3(throw, rope, trap_door)) {
		text_show(text_502_26);
		/* throw just the rope at trap door */
		goto handled;
	}

	if (player_said_3(throw, cable_hook, trap_door)) {
		text_show(text_502_27);
		/* throw just the cable hook at trap door */
		goto handled;
	}

	if (player_said_2(climb, rope) || player_said_2(climb_through, trap_door)) {
		switch (kernel.trigger) {
		case 0:
			global[player_score] += 5;
			player.commands_allowed = false;
			player.walker_visible = false;
			local->turning_panel = true;
			seq[fx_rope_climber] = kernel_seq_forward(ss[fx_rope_climber], false, 6, 0, 0, 1);
			kernel_seq_depth(seq[fx_rope_climber], 10);
			kernel_seq_range(seq[fx_rope_climber], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_rope_climber], KERNEL_TRIGGER_EXPIRE, 0, ROOM_502_THROW_UP);
			break;

		case ROOM_502_THROW_UP:
			new_room = 504;
			break;
		}
		goto handled;
	}

	if (player.look_around) {
		text_show(text_502_10);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(floor)) {
			text_show(text_502_17);
			goto handled;
		}

		if (player_said_1(trap_door)) {
			if (local->yippie) {
				text_show(text_502_20);
			} else {
				text_show(text_502_25);
			}
			goto handled;
		}

		if (player_said_1(wall)) {
			text_show(text_502_19);
			goto handled;
		}

		if (player_said_1(door)) {
			text_show(text_502_21);
			goto handled;
		}

		if (player_said_1(panels)) {
			text_show(text_502_22);
			goto handled;
		}

		if (player_said_1(panel)) {
			text_show(text_502_23);
			goto handled;
		}

		if (player_said_1(ceiling)) {
			text_show(text_502_24);
			goto handled;
		}

		if (player_said_1(rope)) {
			if (!player_has(rope) && !player_has(cable_hook) && !player_has(rope_with_hook)) {
				text_show(text_502_33);
				goto handled;
			}
		}
	}

	if (player_said_2(open, trap_door)) {
		if (local->yippie) {
			text_show(text_502_30);
		} else {
			text_show(text_502_28);
		}
		goto handled;
	}

	if (player_said_2(close, trap_door)) {
		if (local->yippie) {
			text_show(text_502_28);
		} else {
			text_show(text_502_31);
		}
		goto handled;
	}

	if (player_said_2(take, rope)) {
		if (!player_has(rope) && !player_has(cable_hook) && !player_has(rope_with_hook)) {
			text_show(text_502_34);
			goto handled;
		}
	}

	if (player_said_2(lasso, trap_door)) {
		text_show(text_502_32);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

static void animate_fire_bursts() {
	int trig;

	if (local->room_getting_hotter) {
		trig = imath_random(1, 50);
	} else {
		trig = imath_random(1, 400);
	}

	if (trig == 1) {

		trig = imath_random(1, 4);

		switch (trig) {
		case 1:
			if (!local->fire_1_on) {
				kernel_timing_trigger(imath_random(FIVE_SECONDS, TEN_SECONDS), ROOM_502_FIRE_BURST_1);
				local->fire_1_on = true;
			}
			break;

		case 2:
			if (!local->fire_2_on) {
				kernel_timing_trigger(imath_random(FIVE_SECONDS, TEN_SECONDS), ROOM_502_FIRE_BURST_2);
				local->fire_2_on = true;
			}
			break;

		case 3:
			if (!local->fire_3_on) {
				kernel_timing_trigger(imath_random(FIVE_SECONDS, TEN_SECONDS), ROOM_502_FIRE_BURST_3);
				local->fire_3_on = true;
			}
			break;

		case 4:
			if (!local->fire_4_on) {
				kernel_timing_trigger(imath_random(FIVE_SECONDS, TEN_SECONDS), ROOM_502_FIRE_BURST_4);
				local->fire_4_on = true;
			}
			break;
		}
	}

	switch (kernel.trigger) {
	case ROOM_502_FIRE_BURST_1:
		if ((player.x < 198) || (player.y > 150)) {
			seq[fx_fire_1] = kernel_seq_forward(ss[fx_fire_1], false, 5, 0, 0, 1);
			kernel_seq_depth(seq[fx_fire_1], 14);
			kernel_seq_range(seq[fx_fire_1], 1, 10);
			kernel_seq_trigger(seq[fx_fire_1],
				KERNEL_TRIGGER_EXPIRE, 0, ROOM_502_FIRE_BURST_1 + 1);
			/* local->fire_1_on = true; */
		}
		break;

	case ROOM_502_FIRE_BURST_1 + 1:
		local->fire_1_on = false;
		break;

	case ROOM_502_FIRE_BURST_2:
		if ((player.x > 127) || (player.y < 150)) {
			seq[fx_fire_2] = kernel_seq_forward(ss[fx_fire_2], false, 5, 0, 0, 1);
			kernel_seq_depth(seq[fx_fire_2], 1);
			kernel_seq_range(seq[fx_fire_2], 1, 10);
			kernel_seq_trigger(seq[fx_fire_2],
				KERNEL_TRIGGER_EXPIRE, 0, ROOM_502_FIRE_BURST_2 + 1);
			/* local->fire_2_on = true; */
		}
		break;

	case ROOM_502_FIRE_BURST_2 + 1:
		local->fire_2_on = false;
		break;

	case ROOM_502_FIRE_BURST_3:
		if (player.x < 198) {
			seq[fx_fire_3] = kernel_seq_forward(ss[fx_fire_3], false, 5, 0, 0, 1);
			kernel_seq_depth(seq[fx_fire_3], 1);
			kernel_seq_range(seq[fx_fire_3], 1, 10);
			kernel_seq_trigger(seq[fx_fire_3],
				KERNEL_TRIGGER_EXPIRE, 0, ROOM_502_FIRE_BURST_3 + 1);
			/* local->fire_3_on = true; */
		}
		break;

	case ROOM_502_FIRE_BURST_3 + 1:
		local->fire_3_on = false;
		break;

	case ROOM_502_FIRE_BURST_4:
		if ((player.x > 110) || (player.y > 150)) {
			seq[fx_fire_4] = kernel_seq_forward(ss[fx_fire_4], false, 5, 0, 0, 1);
			kernel_seq_depth(seq[fx_fire_4], 14);
			kernel_seq_range(seq[fx_fire_4], 1, 10);
			kernel_seq_trigger(seq[fx_fire_4],
				KERNEL_TRIGGER_EXPIRE, 0, ROOM_502_FIRE_BURST_4 + 1);
			/* local->fire_4_on = true; */
		}
		break;

	case ROOM_502_FIRE_BURST_4 + 1:
		local->fire_4_on = false;
		break;
	}
}

static void fastcall room_502_set_cycle() {
	int save_cycling;
	int cycle;
	int color_offset;
	int num_colors;
	int num_first_colors;
	int num_second_colors;
	CycleListPtr my_cycle;
	RGBcolor *color_base;
	RGBcolor *color_marker;
	RGBcolor *palette_base;
	RGBcolor *palette_marker;

	if (local->cycle_stage != local->cycle_bookkeep) {
		save_cycling = cycling_active;
		cycling_active = false;

		my_cycle = &local->cycle_list[local->cycle_stage];
		color_base = local->cycle_color[local->cycle_stage];

		for (cycle = 0; cycle < my_cycle->num_cycles; cycle++) {
			color_marker = color_base + my_cycle->table[cycle].first_list_color;

			palette_base = &cycling_palette[0];
			palette_base += cycle_list.table[cycle].first_palette_color;

			color_offset = cycle_list.table[cycle].first_list_color;

			num_colors = my_cycle->table[cycle].num_colors;
			num_first_colors = num_colors - color_offset;
			num_second_colors = num_colors - num_first_colors;

			palette_marker = palette_base + color_offset;

			memcpy(palette_marker, color_marker, sizeof(RGBcolor) * num_first_colors);

			color_marker += num_first_colors;

			if (num_second_colors) {
				memcpy(palette_base, color_marker, sizeof(RGBcolor) * num_second_colors);
			}

			cycle_list.table[cycle].ticks = my_cycle->table[cycle].ticks;
		}

		local->cycle_bookkeep = local->cycle_stage;

		cycling_active = save_cycling;
	}
}

void room_502_daemon() {
	long dif;

	if (local->room_getting_hotter) {
		dif = kernel.clock - local->hot_clock;
		if ((dif >= 0) && (dif <= 4)) {
			local->hot_timer += dif;
			local->death_timer += dif;
		} else {
			local->hot_timer += 1;
			local->death_timer += 1;
		}
		local->hot_clock = kernel.clock;

		if (local->hot_timer >= CYCLE_SWITCH_TICKS) {
			local->hot_timer = 0;
			if (local->cycle_stage < (num_cycle_stages - 1)) {
				local->cycle_stage++;
			}
		}
	}


	if (local->death_timer >= LENGTH_OF_LIFE) {
		if (!local->turning_panel) {
			text_show(text_502_15);
			player_walk(DEATH_X, DEATH_Y, FACING_NORTH);
			player_walk_trigger(ROOM_502_DEATH);
			player.commands_allowed = false;
			local->turning_panel = true;
			local->death_timer = 0;
		}
	}

	if ((local->death_timer > FIFTEEN_SECONDS) && (local->message_stage == 1)) {
		if (!local->turning_panel) {
			local->message_stage = 2;
			text_show(text_502_12);
		}
	}

	if ((local->death_timer > SIXTY_SECONDS) && (local->message_stage == 2)) {
		if (!local->turning_panel) {
			local->message_stage = 3;
			text_show(text_502_13);
		}
	}

	if ((local->death_timer > NINETY_SECONDS) && (local->message_stage == 3)) {
		if (!local->turning_panel) {
			local->message_stage = 4;
			text_show(text_502_14);
		}
	}

	switch (kernel.trigger) {
	case ROOM_502_DEATH:
		player.commands_allowed = false;
		player.walker_visible = false;
		seq[fx_death] = kernel_seq_forward(ss[fx_death], false, 7, 0, 0, 1);
		kernel_seq_depth(seq[fx_death], 1);
		kernel_seq_range(seq[fx_death], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_death], KERNEL_TRIGGER_EXPIRE, 0, ROOM_502_DEATH + 1);
		kernel_seq_trigger(seq[fx_death], KERNEL_TRIGGER_SPRITE, 44, ROOM_502_DEATH + 2);
		kernel_seq_trigger(seq[fx_death], KERNEL_TRIGGER_SPRITE, 51, ROOM_502_DEATH + 3);
		kernel_seq_trigger(seq[fx_death], KERNEL_TRIGGER_SPRITE, 32, ROOM_502_DEATH + 4);
		break;

	case ROOM_502_DEATH + 1:
		seq[fx_death] = kernel_seq_stamp(ss[fx_death], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_death], 1);
		inter_turn_off_object();
		inter_screen_update();
		kernel_timing_trigger(TWO_SECONDS, ROOM_502_DEATH + 5);
		break;

	case ROOM_502_DEATH + 5:
		kernel.force_restart = true;
		break;

	case ROOM_502_DEATH + 4:
		global_speech_go(speech_raoul_fire);
		break;

	case ROOM_502_DEATH + 2:
		sound_play(N_AllFade);
		sound_play(N_HeatHiss);
		break;

	case ROOM_502_DEATH + 3:
		sound_play(N_PlayerDies);
		break;

	case ROOM_502_ENTER:
		kernel_seq_delete(seq[fx_door]);
		seq[fx_door] = kernel_seq_forward(ss[fx_door], false, 7, 0, 0, 1);
		kernel_seq_depth(seq[fx_door], 1);
		kernel_seq_range(seq[fx_door], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_door], KERNEL_TRIGGER_EXPIRE, 0, ROOM_502_ENTER + 1);
		break;

	case ROOM_502_ENTER + 1:
		text_show(text_502_11);
		kernel_draw_to_background(ss[fx_door], KERNEL_LAST, KERNEL_HOME, KERNEL_HOME, 0, 100);
		player.commands_allowed = true;
		break;
	}

	if (!local->yippie) {
		animate_fire_bursts();
	}

	room_502_set_cycle();
}

void room_502_preload() {
	room_init_code_pointer = room_502_init;
	room_pre_parser_code_pointer = room_502_pre_parser;
	room_parser_code_pointer = room_502_parser;
	room_daemon_code_pointer = room_502_daemon;
	room_shutdown_code_pointer = room_502_shutdown;

	section_5_walker();
	section_5_interface();
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
