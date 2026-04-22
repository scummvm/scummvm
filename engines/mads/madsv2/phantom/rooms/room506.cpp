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
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/phantom/rooms/section5.h"
#include "mads/madsv2/phantom/rooms/room506.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_506_init() {
	int id;

	kernel.disable_fastwalk = true;

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
		local->prevent = false;
		local->ascending = false;
	}

	conv_get(CONV_MISC);

	/* ===================== Load Sprite Series ================== */

	ss[fx_left_flame] = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_middle_flame] = kernel_load_series(kernel_name('x', 1), false);
	ss[fx_right_flame] = kernel_load_series(kernel_name('x', 2), false);
	ss[fx_door] = kernel_load_series(kernel_name('x', 3), false);
	ss[fx_oar] = kernel_load_series(kernel_name('p', 0), false);
	ss[fx_raoul_ascend] = kernel_load_series(kernel_name('a', 1), false);
	ss[fx_raoul_decend] = kernel_load_series(kernel_name('a', 0), false);

	ss[fx_christine] = kernel_load_series(kernel_name('b', 0), PAL_MAP_ALL_TO_CLOSEST |
		PAL_MAP_ANY_TO_CLOSEST);

	ss[fx_take_6] = kernel_load_series("*RDR_9", false);

	ss[fx_christine] = kernel_load_series("*CHR_6", PAL_MAP_ALL_TO_CLOSEST |
		PAL_MAP_ANY_TO_CLOSEST);
	ss[fx_christine] = kernel_load_series("*CHR_3", PAL_MAP_ALL_TO_CLOSEST |
		PAL_MAP_ANY_TO_CLOSEST);
	ss[fx_christine] = kernel_load_series("*CHR_2", PAL_MAP_ALL_TO_CLOSEST |
		PAL_MAP_ANY_TO_CLOSEST);

	/* ======================== Stamp oar ======================== */

	if (object_is_here(oar)) {
		seq[fx_oar] = kernel_seq_stamp
		(ss[fx_oar], false, 1);
		kernel_seq_depth(seq[fx_oar], 14);

	} else {
		kernel_flip_hotspot(words_oar, false);
	}

	/* ======================= Stamp flames ====================== */

	seq[fx_left_flame] = kernel_seq_forward(ss[fx_left_flame], false, 6, 0, 0, 0);
	kernel_seq_depth(seq[fx_left_flame], 14);
	kernel_seq_range(seq[fx_left_flame], KERNEL_FIRST, KERNEL_LAST);

	seq[fx_middle_flame] = kernel_seq_forward(ss[fx_middle_flame], false, 6, 0, 0, 0);
	kernel_seq_depth(seq[fx_middle_flame], 14);
	kernel_seq_range(seq[fx_middle_flame], KERNEL_FIRST, KERNEL_LAST);

	seq[fx_right_flame] = kernel_seq_forward(ss[fx_right_flame], false, 6, 0, 0, 0);
	kernel_seq_depth(seq[fx_right_flame], 14);
	kernel_seq_range(seq[fx_right_flame], KERNEL_FIRST, KERNEL_LAST);

	/* ========================= Previous Rooms ================== */

	if (previous_room == KERNEL_RESTORING_GAME) {
		seq[fx_door] = kernel_seq_stamp
		(ss[fx_door], false, 1);
		kernel_seq_depth(seq[fx_door], 14);
		if (!global[christine_is_in_boat]) {
			local->anim_0_running = true;
			aa[0] = kernel_run_animation(kernel_name('r', 1), 0);
			kernel_reset_animation(aa[0], 239);
			id = kernel_add_dynamic(words_Christine, words_walk_to, SYNTAX_SINGULAR_FEM, KERNEL_NONE,
				0, 0, 0, 0);
			kernel_dynamic_hot[id].prep = PREP_ON;
			kernel_dynamic_walk(id, DYNAMIC_CHR_WALK_TO_X, DYNAMIC_CHR_WALK_TO_Y, FACING_SOUTHWEST);
			kernel_dynamic_anim(id, aa[0], 6);
		}

	} else if (previous_room == 504) {

		seq[fx_door] = kernel_seq_stamp
		(ss[fx_door], false, 1);
		kernel_seq_depth(seq[fx_door], 14);

		if (player.been_here_before) {
			player.x = END_OF_DECEND_X;
			player.y = END_OF_DECEND_Y;
			player.facing = FACING_SOUTHWEST;
			player.commands_allowed = false;
			player.walker_visible = false;

			if (!global[christine_is_in_boat]) {
				local->anim_0_running = true;
				aa[0] = kernel_run_animation(kernel_name('r', 1), 0);
				kernel_reset_animation(aa[0], 239);
				id = kernel_add_dynamic(words_Christine, words_walk_to, SYNTAX_SINGULAR_FEM, KERNEL_NONE,
					0, 0, 0, 0);
				kernel_dynamic_hot[id].prep = PREP_ON;
				kernel_dynamic_walk(id, DYNAMIC_CHR_WALK_TO_X, DYNAMIC_CHR_WALK_TO_Y, FACING_SOUTHWEST);
				kernel_dynamic_anim(id, aa[0], 6);
			}

			aa[1] = kernel_run_animation(kernel_name('r', 2), ROOM_506_DONE_GOING_DOWN);

		} else {
			player.x = PLAYER_X_FROM_504;
			player.y = PLAYER_Y_FROM_504;
			player.facing = FACING_SOUTHWEST;
			player.commands_allowed = false;
			player.walker_visible = false;
			local->anim_0_running = true;
			aa[0] = kernel_run_animation(kernel_name('r', 1), 0);
			id = kernel_add_dynamic(words_Christine, words_walk_to, SYNTAX_SINGULAR_FEM, KERNEL_NONE,
				0, 0, 0, 0);
			kernel_dynamic_hot[id].prep = PREP_ON;
			kernel_dynamic_walk(id, DYNAMIC_CHR_WALK_TO_X, DYNAMIC_CHR_WALK_TO_Y, FACING_SOUTHWEST);
			kernel_dynamic_anim(id, aa[0], 2);
			kernel_dynamic_anim(id, aa[0], 3);
			kernel_dynamic_anim(id, aa[0], 4);
			kernel_dynamic_anim(id, aa[0], 5);
			kernel_dynamic_anim(id, aa[0], 6);
		}

	} else if ((previous_room == 501) || (previous_room != KERNEL_RESTORING_GAME)) {
		player.x = PLAYER_X_FROM_501;
		player.y = PLAYER_Y_FROM_501;
		player.facing = FACING_EAST;
		player.commands_allowed = false;
		player_walk(WALK_TO_X_FROM_501, WALK_TO_Y_FROM_501, FACING_EAST);
		player_walk_trigger(ROOM_506_DOOR_CLOSES);
	}
	section_5_music();
}

void room_506_pre_parser() {
	if (player_said_2(unlock, door) || player_said_2(lock, door)) {
		player_walk(FRONT_OF_DOOR_X, FRONT_OF_DOOR_Y, FACING_NORTHWEST);
	}

	if (player_said_2(open, door)) {
		if (inter_point_x < 150) {
			player_walk(FRONT_OF_DOOR_X, FRONT_OF_DOOR_Y, FACING_NORTHWEST);
		} else {
			player_walk(START_OF_ASCEND_X, START_OF_ASCEND_Y, FACING_EAST);
		}
	}
}

void room_506_parser() {
	if (player_said_2(talk_to, Christine)) {
		conv_run(CONV_MISC);
		conv_export_value(1);
		kernel_reset_animation(aa[0], 290);
		goto handled;
	}

	if (player_said_2(take, oar)) {
		switch (kernel.trigger) {
		case (0):
			if (object_is_here(oar)) {
				player.commands_allowed = false;
				player.walker_visible = false;
				seq[fx_take_6] = kernel_seq_pingpong(ss[fx_take_6], true,
					5, 0, 0, 2);
				kernel_seq_range(seq[fx_take_6], 1, 4);
				kernel_seq_player(seq[fx_take_6], true);
				kernel_seq_trigger(seq[fx_take_6],
					KERNEL_TRIGGER_SPRITE, 4, 1);
				kernel_seq_trigger(seq[fx_take_6],
					KERNEL_TRIGGER_EXPIRE, 0, 2);
			}
			break;

		case 1:
			kernel_seq_delete(seq[fx_oar]);
			kernel_flip_hotspot(words_oar, false);
			inter_give_to_player(oar);
			sound_play(N_TakeObjectSnd);
			break;

		case 2:
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_6]);
			player.walker_visible = true;
			kernel_timing_trigger(20, 3);
			break;

		case 3:
			object_examine(oar, text_008_24, 0);
			/* You pick up the oar */
			player.commands_allowed = true;
			break;
		}
		goto handled;
	}

	if (player_said_2(walk_through, door) || player_said_2(open, door)) {
		if (inter_point_x < 150) {
			switch (kernel.trigger) {
			case (0):
				player.commands_allowed = false;
				player.walker_visible = false;
				seq[fx_take_6] = kernel_seq_pingpong(ss[fx_take_6], true,
					5, 0, 0, 2);
				kernel_seq_range(seq[fx_take_6], 1, 4);
				kernel_seq_player(seq[fx_take_6], true);
				kernel_seq_trigger(seq[fx_take_6],
					KERNEL_TRIGGER_SPRITE, 4, ROOM_506_DOOR_OPENS);
				kernel_seq_trigger(seq[fx_take_6],
					KERNEL_TRIGGER_EXPIRE, 0, ROOM_506_DOOR_OPENS + 2);
				break;

			case ROOM_506_DOOR_OPENS:
				kernel_seq_delete(seq[fx_door]);
				seq[fx_door] = kernel_seq_forward(ss[fx_door], false,
					8, 0, 0, 1);
				kernel_seq_depth(seq[fx_door], 14);
				kernel_seq_range(seq[fx_door], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_trigger(seq[fx_door],
					KERNEL_TRIGGER_EXPIRE, 0, ROOM_506_DOOR_OPENS + 1);
				sound_play(N_DoorOpens);
				break;

			case ROOM_506_DOOR_OPENS + 1:
				player_walk(PLAYER_X_FROM_501, PLAYER_Y_FROM_501, FACING_WEST);
				player_walk_trigger(ROOM_506_DOOR_OPENS + 3);
				break;

			case ROOM_506_DOOR_OPENS + 2:
				player.walker_visible = true;
				break;

			case ROOM_506_DOOR_OPENS + 3:
				if (global[christine_is_in_boat]) {
					new_room = 501;
				} else {
					kernel_reset_animation(aa[0], 241);
				}
				break;
			}

		} else {
			switch (kernel.trigger) {
			case (0):
				if (!global[christine_is_in_boat]) {
					conv_run(CONV_MISC);
					conv_export_value(2);
					if (kernel_anim[aa[0]].frame == 240 ||
						kernel_anim[aa[0]].frame == 239) {
						kernel_reset_animation(aa[0], 290);
					}
					local->ascending = true;
				}
				player.commands_allowed = false;
				player.walker_visible = false;
				seq[fx_raoul_ascend] = kernel_seq_forward(ss[fx_raoul_ascend], false,
					5, 0, 0, 1);
				kernel_seq_range(seq[fx_raoul_ascend], KERNEL_FIRST, KERNEL_LAST);
				kernel_synch(KERNEL_SERIES, seq[fx_raoul_ascend], KERNEL_PLAYER, 0);
				kernel_seq_trigger(seq[fx_raoul_ascend],
					KERNEL_TRIGGER_EXPIRE, 0, ROOM_506_DONE_GOING_UP);
				break;

			case ROOM_506_DONE_GOING_UP:
				conv_abort();
				new_room = 504;
				break;
			}
		}
		goto handled;
	}

	if (conv_control.running == CONV_MISC) {
		goto handled;
	}

	if (player.look_around) {
		text_show(text_506_10);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(floor)) {
			text_show(text_506_11);
			goto handled;
		}

		if (player_said_1(wall)) {
			text_show(text_506_12);
			goto handled;
		}

		if (player_said_1(torch)) {
			text_show(text_506_13);
			goto handled;
		}

		if (player_said_1(column)) {
			text_show(text_506_14);
			goto handled;
		}

		if (player_said_1(ceiling)) {
			text_show(text_506_15);
			goto handled;
		}

		if (player_said_1(ramp)) {
			text_show(text_506_16);
			goto handled;
		}

		if (player_said_1(door)) {
			if (inter_point_x < 150) {
				text_show(text_506_17);
			} else {
				text_show(text_506_18);
			}
			goto handled;
		}

		if (player_said_1(oar) && object_is_here(oar)) {
			text_show(text_506_19);
			goto handled;
		}

		if (player_said_1(Christine)) {
			if (kernel_anim[aa[0]].frame < 235) {
				text_show(text_506_21);
			} else {
				text_show(text_506_20);
			}
			goto handled;
		}
	}

	if (player_said_2(take, torch)) {
		text_show(text_506_13);
		goto handled;
	}

	if (player_said_2(take, Christine)) {
		text_show(text_506_22);
		goto handled;
	}


	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_506_daemon() {
	switch (kernel.trigger) {

	case ROOM_506_DONE_GOING_DOWN:
		player.walker_visible = true;
		player.commands_allowed = true;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[1]);
		break;

	case ROOM_506_DOOR_CLOSES:
		seq[fx_door] = kernel_seq_backward(ss[fx_door], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_door], 14);
		kernel_seq_range(seq[fx_door], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_door], KERNEL_TRIGGER_EXPIRE, 0, ROOM_506_DOOR_CLOSES + 1);
		break;

	case ROOM_506_DOOR_CLOSES + 1:
		seq[fx_door] = kernel_seq_stamp
		(ss[fx_door], false, 1);
		kernel_seq_depth(seq[fx_door], 14);
		player.commands_allowed = true;
		break;
	}

	if (local->anim_0_running) {
		if ((kernel_anim[aa[0]].frame == 141) && (!local->prevent)) {
			player.walker_visible = true;
			local->prevent = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
		}

		if (kernel_anim[aa[0]].frame == 240) {
			kernel_reset_animation(aa[0], 239);
		}

		if (kernel_anim[aa[0]].frame == 300) {
			kernel_reset_animation(aa[0], 239);
		}

		if (kernel_anim[aa[0]].frame == 168) {
			player.commands_allowed = true;
		}

		if (kernel_anim[aa[0]].frame == 289) {
			new_room = 501;
		}
	}

	if (local->ascending) {
		if (conv_control.running != CONV_MISC) {
			local->ascending = false;
			player.commands_allowed = false;
		}
	}
}

void room_506_preload() {
	room_init_code_pointer = room_506_init;
	room_pre_parser_code_pointer = room_506_pre_parser;
	room_parser_code_pointer = room_506_parser;
	room_daemon_code_pointer = room_506_daemon;

	section_5_walker();
	section_5_interface();

	vocab_make_active(words_Christine);
	vocab_make_active(words_look_at);

	if (!global[christine_is_in_boat]) {
		kernel_initial_variant = 1;
	}
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
