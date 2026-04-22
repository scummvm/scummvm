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
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/phantom/mads/inventory.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/mads/speeches.h"
#include "mads/madsv2/phantom/rooms/section3.h"
#include "mads/madsv2/phantom/rooms/room309.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/phantom/catacombs.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_309_init() {
	int id;

	kernel_flip_hotspot(words_Christine, false);
	kernel_flip_hotspot(words_boat, false);

	local->anim_0_running = false;
	local->boat_action = ACTION_OTHER;

	conv_get(CONV_MISC);

	/* ====================== Load sprites ======================= */

	ss[fx_door] = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_boat] = kernel_load_series(kernel_name('x', 1), false);
	ss[fx_take_9] = kernel_load_series("*RDR_9", false);

	if (previous_room == KERNEL_RESTORING_GAME) {

		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 10);

		if (player_has_been_in_room(310)) {
			local->anim_0_running = true;

			aa[0] = kernel_run_animation(kernel_name('b', 1), ROOM_309_OUT_OF_BOAT);
			kernel_reset_animation(aa[0], 184); /* put Chris by door */
			id = kernel_add_dynamic(words_Christine, words_walk_to, SYNTAX_SINGULAR_FEM, KERNEL_NONE,
				0, 0, 0, 0);
			kernel_dynamic_walk(id, DYNAMIC_CHR_WALK_TO_X, DYNAMIC_CHR_WALK_TO_Y, FACING_NORTHWEST);
			kernel_dynamic_hot[id].prep = PREP_ON;

			kernel_dynamic_anim(id, aa[0], 10);
			kernel_dynamic_anim(id, aa[0], 11);
			kernel_dynamic_anim(id, aa[0], 12);
			kernel_dynamic_anim(id, aa[0], 13);
			kernel_dynamic_anim(id, aa[0], 14);
			kernel_dynamic_anim(id, aa[0], 15);
			kernel_dynamic_anim(id, aa[0], 16);
			kernel_dynamic_anim(id, aa[0], 17);
			kernel_dynamic_anim(id, aa[0], 18);
			kernel_flip_hotspot(words_Christine, true);
			kernel_flip_hotspot(words_boat, true);
		}
	}


	/* ===================== Previous rooms ====================== */

	if (previous_room == 404) {
		player.x = PLAYER_X_FROM_404;
		player.y = PLAYER_Y_FROM_404;
		player.facing = FACING_SOUTHWEST;
		player_walk(WALK_TO_X_FROM_404, WALK_TO_Y_FROM_404, FACING_SOUTHWEST);
		seq[fx_door] = kernel_seq_stamp
		(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 10);

	} else if (previous_room == 310) {
		player.x = PLAYER_X_FROM_310;
		player.y = PLAYER_Y_FROM_310;
		player.facing = FACING_SOUTH;
		player.commands_allowed = false;
		player.walker_visible = false;
		local->anim_0_running = true;

		aa[0] = kernel_run_animation(kernel_name('b', 1), ROOM_309_OUT_OF_BOAT);
		id = kernel_add_dynamic(words_Christine, words_walk_to, SYNTAX_SINGULAR_FEM, KERNEL_NONE,
			0, 0, 0, 0);
		kernel_dynamic_walk(id, DYNAMIC_CHR_WALK_TO_X, DYNAMIC_CHR_WALK_TO_Y, FACING_NORTHWEST);
		kernel_dynamic_hot[id].prep = PREP_ON;

		kernel_dynamic_anim(id, aa[0], 10);
		kernel_dynamic_anim(id, aa[0], 11);
		kernel_dynamic_anim(id, aa[0], 12);
		kernel_dynamic_anim(id, aa[0], 13);
		kernel_dynamic_anim(id, aa[0], 14);
		kernel_dynamic_anim(id, aa[0], 15);
		kernel_dynamic_anim(id, aa[0], 16);
		kernel_dynamic_anim(id, aa[0], 17);
		kernel_dynamic_anim(id, aa[0], 18);
		kernel_flip_hotspot(words_Christine, true);
		kernel_flip_hotspot(words_boat, true);
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 10);

	} else if ((previous_room == 308) || (previous_room != KERNEL_RESTORING_GAME)) {
		player.x = PLAYER_X_FROM_308;
		player.y = PLAYER_Y_FROM_308;
		player.facing = FACING_SOUTHEAST;
		player_walk(WALK_TO_X_FROM_308, WALK_TO_Y_FROM_308, FACING_SOUTHEAST);
		player_walk_trigger(ROOM_309_DOOR_CLOSES);
		player.commands_allowed = false;
		seq[fx_door] = kernel_seq_stamp
		(ss[fx_door], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_door], 10);
	}
	section_3_music();
}

void room_309_pre_parser() {
	if (player_said_2(walk_through, stone_archway)) {
		if (global[lantern_status] == LANTERN_IS_OFF) {
			player.need_to_walk = false;
			text_show(text_309_18);
		}
	}

	if (player_said_2(walk_through, stone_archway)) {
		if (global[right_door_is_open_504]) {
			player_walk(WALK_TO_CAT_NO_X, WALK_TO_CAT_NO_Y, FACING_NORTHEAST);
		}
	}

	if (player_said_2(open, door)) {
		player_walk(DOOR_X, DOOR_Y, FACING_NORTHEAST);
	}
}

void room_309_parser() {
	int temp;  /* for synching purposes */

	if (player_said_2(walk_through, stone_archway)) {
		if (global[right_door_is_open_504]) {
			conv_run(CONV_MISC);
			conv_export_value(1);
			local->boat_action = ACTION_TALK;
			local->chris_talk_count = 0;
		} else if (global[lantern_status] == LANTERN_IS_ON) {
			global_enter_catacombs(0);
		}
		goto handled;
	}

	if (player_said_2(talk_to, Christine)) {
		conv_run(CONV_MISC);
		conv_export_value(1);
		local->boat_action = ACTION_TALK;
		local->chris_talk_count = 0;
		goto handled;
	}

	if (player_said_2(walk_through, door) || player_said_2(open, door)) {
		switch (kernel.trigger) {
		case (0):
			player.commands_allowed = false;
			player.walker_visible = false;
			seq[fx_take_9] = kernel_seq_pingpong(ss[fx_take_9], false,
				5, 0, 0, 2);
			kernel_seq_range(seq[fx_take_9], 1, 4);
			kernel_seq_player(seq[fx_take_9], true);
			kernel_seq_trigger(seq[fx_take_9],
				KERNEL_TRIGGER_SPRITE, 4, ROOM_309_DOOR_OPENS);
			kernel_seq_trigger(seq[fx_take_9],
				KERNEL_TRIGGER_EXPIRE, 0, ROOM_309_DOOR_OPENS + 2);
			break;

		case ROOM_309_DOOR_OPENS:
			kernel_seq_delete(seq[fx_door]);
			seq[fx_door] = kernel_seq_forward(ss[fx_door], false,
				8, 0, 0, 1);
			kernel_seq_depth(seq[fx_door], 14);
			kernel_seq_range(seq[fx_door], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_door],
				KERNEL_TRIGGER_EXPIRE, 0, ROOM_309_DOOR_OPENS + 1);
			sound_play(N_DoorOpens);
			break;

		case ROOM_309_DOOR_OPENS + 1:
			temp = seq[fx_door];
			seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, 5);
			kernel_synch(KERNEL_SERIES, seq[fx_door], KERNEL_SERIES, temp);
			kernel_seq_depth(seq[fx_door], 14);
			player_walk(PLAYER_X_FROM_308, PLAYER_Y_FROM_308, FACING_NORTHWEST);
			player_walk_trigger(ROOM_309_DOOR_OPENS + 3);
			break;

		case ROOM_309_DOOR_OPENS + 2:
			player.walker_visible = true;
			break;

		case ROOM_309_DOOR_OPENS + 3:
			if (!global[right_door_is_open_504]) {
				kernel_seq_delete(seq[fx_door]);
				seq[fx_door] = kernel_seq_backward(ss[fx_door], false,
					8, 0, 0, 1);
				kernel_seq_depth(seq[fx_door], 1);
				kernel_seq_range(seq[fx_door], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_trigger(seq[fx_door],
					KERNEL_TRIGGER_EXPIRE, 0, ROOM_309_DOOR_OPENS + 4);
				sound_play(N_DoorCloses);

			} else {
				kernel_reset_animation(aa[0], 186);
			}

			break;

		case ROOM_309_DOOR_OPENS + 4:
			seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, 5);
			kernel_seq_depth(seq[fx_door], 1);
			new_room = 308;
			break;
		}
		goto handled;
	}

	if (player.look_around) {
		text_show(text_309_10);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(stone_wall)) {
			text_show(text_309_11);
			goto handled;
		}

		if (player_said_1(lake)) {
			text_show(text_309_12);
			goto handled;
		}

		if (player_said_1(stone_column)) {
			text_show(text_309_13);
			goto handled;
		}

		if (player_said_1(doorway)) {
			text_show(text_309_14);
			goto handled;
		}

		if (player_said_1(stone_archway)) {
			text_show(text_309_15);
			goto handled;
		}

		if (player_said_1(stone_floor)) {
			text_show(text_309_16);
			goto handled;
		}

		if (player_said_1(catacombs)) {
			text_show(text_309_17);
			goto handled;
		}

		if (player_said_1(Christine)) {
			text_show(text_309_19);
			goto handled;
		}

		if (player_said_1(boat)) {
			text_show(text_309_21);
			goto handled;
		}
	}

	if (player_said_2(climb_into, boat)) {
		text_show(text_309_20);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

static void handle_animation_boat() {
	int boat_reset_frame;

	if (kernel_anim[aa[0]].frame != local->boat_frame) {
		local->boat_frame = kernel_anim[aa[0]].frame;
		boat_reset_frame = -1;

		switch (local->boat_frame) {
		case 72:   /* Turn on Raoul's walker */
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
			player.walker_visible = true;
			break;

		case 130:
			player.commands_allowed = true;
			break;

		case 185:  /* keep Chris by door */
			if (local->boat_action == ACTION_TALK) {
				boat_reset_frame = 244;
			} else {
				boat_reset_frame = 184;
			}

			break;

		case 244:
			new_room = 308;
			break;

		case 245: /* end of talk 1 */
		case 246: /* end of talk 2 */
		case 247: /* end of talk 3 */
			boat_reset_frame = imath_random(244, 246);
			++local->chris_talk_count;
			if (local->chris_talk_count > 10) {
				boat_reset_frame = 184;
				local->boat_action = ACTION_OTHER;
			}
			break;
		}

		if (boat_reset_frame >= 0) {
			kernel_reset_animation(aa[0], boat_reset_frame);
			local->boat_frame = boat_reset_frame;
		}
	}
}


void room_309_daemon() {
	if (local->anim_0_running) {
		handle_animation_boat();
	}

	switch (kernel.trigger) {
	case ROOM_309_DOOR_CLOSES:
		kernel_seq_delete(seq[fx_door]);
		seq[fx_door] = kernel_seq_backward(ss[fx_door], false,
			8, 0, 0, 1);
		kernel_seq_depth(seq[fx_door], 10);
		kernel_seq_range(seq[fx_door], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_door],
			KERNEL_TRIGGER_EXPIRE, 0, ROOM_309_DOOR_CLOSES + 1);
		break;

	case ROOM_309_DOOR_CLOSES + 1:
		sound_play(N_DoorCloses);
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 10);
		player.commands_allowed = true;
		break;
	}
}

void room_309_preload() {
	room_init_code_pointer = room_309_init;
	room_pre_parser_code_pointer = room_309_pre_parser;
	room_parser_code_pointer = room_309_parser;
	room_daemon_code_pointer = room_309_daemon;

	section_3_walker();
	section_3_interface();

	if (global[right_door_is_open_504]) {
		kernel_initial_variant = 1;
	}
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
