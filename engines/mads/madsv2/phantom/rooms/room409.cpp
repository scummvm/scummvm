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
#include "mads/madsv2/core/quote.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/phantom/mads/inventory.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/mads/speeches.h"
#include "mads/madsv2/phantom/rooms/section4.h"
#include "mads/madsv2/phantom/rooms/room409.h"
#include "mads/madsv2/phantom/catacombs.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_409_init() {
	global_catacombs_init();

	global_speech_load(speech_raoul_gas);

	kernel_flip_hotspot(words_sword, false);

	/* =================== Load sprites series =================== */

	ss[fx_take_9] = kernel_load_series("*RDR_9", false);
	ss[fx_bend_down_9] = kernel_load_series("*RRD_9", false);
	ss[fx_red_frame] = kernel_load_series(kernel_name('f', 0), false);
	ss[fx_green_frame] = kernel_load_series(kernel_name('f', 1), false);
	ss[fx_blue_frame] = kernel_load_series(kernel_name('f', 2), false);
	ss[fx_yellow_frame] = kernel_load_series(kernel_name('f', 3), false);
	ss[fx_door] = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_gas] = kernel_load_series(kernel_name('a', 0), false);

	if (object_is_here(sword)) {
		ss[fx_sword] = kernel_load_series(kernel_name('p', 0), false);
		ss[fx_bend_down_9] = kernel_load_series("*RRD_9", false);
		seq[fx_sword] = kernel_seq_stamp(ss[fx_sword], false, 1);
		kernel_seq_depth(seq[fx_sword], 14);
		kernel_flip_hotspot(words_sword, true);
	}


	if (!global[door_in_409_is_open]) {
		seq[fx_door] = kernel_seq_stamp
		(ss[fx_door], false, 1);
		kernel_seq_depth(seq[fx_door], 14);
		kernel_flip_hotspot(words_archway_to_north, false);
	} else {
		kernel_flip_hotspot(words_door, false);
	}


	/* ===================== Previous rooms ====================== */

	if (previous_room == 410) {
		player.facing = FACING_NORTH;
		player.x = PANEL_X;
		player.y = PANEL_Y;

		if (global[flicked_1] && global[flicked_2] && global[flicked_3] && global[flicked_4]) {
			if ((global[flicked_1] == 5) && (global[flicked_2] == 18) &&
				(global[flicked_3] == 9) && (global[flicked_4] == 11)) {
				if (!global[door_in_409_is_open]) {
					global[player_score] += 5;
					sound_play(N_DoorOpens);
					kernel_seq_delete(seq[fx_door]);
					player.commands_allowed = false;
					seq[fx_door] = kernel_seq_forward(ss[fx_door], false, 7, 0, 0, 1);
					kernel_seq_depth(seq[fx_door], 14);
					kernel_seq_range(seq[fx_door], KERNEL_FIRST, KERNEL_LAST);
					kernel_seq_trigger(seq[fx_door],
						KERNEL_TRIGGER_EXPIRE, 0, ROOM_409_DOOR_OPENS);
				}

			} else {
				global[flicked_1] = 0;
				global[flicked_2] = 0;
				global[flicked_3] = 0;
				global[flicked_4] = 0;
				player.commands_allowed = false;
				seq[fx_gas] = kernel_seq_forward(ss[fx_gas], false, 9, 0, 0, 1);
				kernel_seq_depth(seq[fx_gas], 1);
				kernel_seq_range(seq[fx_gas], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_trigger(seq[fx_gas],
					KERNEL_TRIGGER_SPRITE, 15, ROOM_409_DIE);
				kernel_seq_trigger(seq[fx_gas],
					KERNEL_TRIGGER_EXPIRE, 0, ROOM_409_DIE + 1);
				sound_play(N_PoisonGas);
			}

		} else {
			global[flicked_1] = 0;
			global[flicked_2] = 0;
			global[flicked_3] = 0;
			global[flicked_4] = 0;
		}

	} else if (previous_room != KERNEL_RESTORING_GAME) {

		switch (global[catacombs_from]) {

		case NORTH:
			player.x = NORTH_X;
			player.y = NORTH_Y;
			player.facing = FACING_SOUTH;
			player_walk(WALK_TO_NORTH_X, WALK_TO_NORTH_Y, FACING_SOUTH);
			break;

		case SOUTH:
			player.x = SOUTH_X;
			player.y = SOUTH_Y;
			player.facing = FACING_NORTH;
			break;
		}
	}


	/* ================= Stamp frame if in room ================== */

	if (object[red_frame].location == global[catacombs_room] + 600) {
		seq[fx_red_frame] = kernel_seq_stamp(ss[fx_red_frame], false, 1);
		kernel_seq_depth(seq[fx_red_frame], 14);
		local->dyn_red = kernel_add_dynamic(words_red_frame, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
			series_list[ss[fx_red_frame]]->index[0].x - 5,
			series_list[ss[fx_red_frame]]->index[0].y - 5, 10, 6);
		kernel_dynamic_walk(local->dyn_red, series_list[ss[fx_red_frame]]->index[0].x + 12,
			series_list[ss[fx_red_frame]]->index[0].y, FACING_NORTHWEST);
	}

	if (object[green_frame].location == global[catacombs_room] + 600) {
		seq[fx_green_frame] = kernel_seq_stamp(ss[fx_green_frame], false, 1);
		kernel_seq_depth(seq[fx_green_frame], 14);
		local->dyn_green = kernel_add_dynamic(words_green_frame, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
			series_list[ss[fx_red_frame]]->index[0].x - 5,
			series_list[ss[fx_red_frame]]->index[0].y - 5, 10, 6);
		kernel_dynamic_walk(local->dyn_green, series_list[ss[fx_red_frame]]->index[0].x + 12,
			series_list[ss[fx_red_frame]]->index[0].y, FACING_NORTHWEST);
	}

	if (object[blue_frame].location == global[catacombs_room] + 600) {
		seq[fx_blue_frame] = kernel_seq_stamp(ss[fx_blue_frame], false, 1);
		kernel_seq_depth(seq[fx_blue_frame], 14);
		local->dyn_blue = kernel_add_dynamic(words_blue_frame, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
			series_list[ss[fx_red_frame]]->index[0].x - 5,
			series_list[ss[fx_red_frame]]->index[0].y - 5, 10, 6);
		kernel_dynamic_walk(local->dyn_blue, series_list[ss[fx_red_frame]]->index[0].x + 12,
			series_list[ss[fx_red_frame]]->index[0].y, FACING_NORTHWEST);
	}

	if (object[yellow_frame].location == global[catacombs_room] + 600) {
		seq[fx_yellow_frame] = kernel_seq_stamp(ss[fx_yellow_frame], false, 1);
		kernel_seq_depth(seq[fx_yellow_frame], 14);
		local->dyn_yellow = kernel_add_dynamic(words_yellow_frame, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
			series_list[ss[fx_red_frame]]->index[0].x - 5,
			series_list[ss[fx_red_frame]]->index[0].y - 5, 10, 6);
		kernel_dynamic_walk(local->dyn_yellow, series_list[ss[fx_red_frame]]->index[0].x + 12,
			series_list[ss[fx_red_frame]]->index[0].y, FACING_NORTHWEST);
	}

	section_4_music();
}

void room_409_pre_parser() {
	local->frame_is_here = false;
	local->frame_here_for_taking = false;

	if (object[red_frame].location == global[catacombs_room] + 600) {
		local->frame_is_here = true;
		if (player_said_2(take, red_frame)) {
			local->frame_here_for_taking = true;
		}
	}

	if (object[yellow_frame].location == global[catacombs_room] + 600) {
		local->frame_is_here = true;
		if (player_said_2(take, yellow_frame)) {
			local->frame_here_for_taking = true;
		}
	}

	if (object[blue_frame].location == global[catacombs_room] + 600) {
		local->frame_is_here = true;
		if (player_said_2(take, blue_frame)) {
			local->frame_here_for_taking = true;
		}
	}

	if (object[green_frame].location == global[catacombs_room] + 600) {
		local->frame_is_here = true;
		if (player_said_2(take, green_frame)) {
			local->frame_here_for_taking = true;
		}
	}

	if (player_said_1(put) && player_said_1(floor)) {
		if (player_said_1(red_frame) ||
			player_said_1(blue_frame) ||
			player_said_1(yellow_frame) ||
			player_said_1(green_frame)) {
			if (local->frame_is_here) {
				player.need_to_walk = false;

			} else {
				player_walk(series_list[ss[fx_red_frame]]->index[0].x + 12,
					series_list[ss[fx_red_frame]]->index[0].y, FACING_NORTHWEST);
			}
		}
	}

	if (player_said_2(look, switch_panel)) {
		player_walk(PANEL_X, PANEL_Y, FACING_NORTH);
	}

	if (player_said_2(open, door)) {
		player_walk(DOOR_X, DOOR_Y, FACING_NORTHEAST);
	}

	if (player_said_2(open, grate)) {
		player.need_to_walk = true;
	}
}

void room_409_parser() {
	int temp; /* for synching purposes */

	if (player_said_2(walk_through, door) || player_said_2(open, door)) {
		switch (kernel.trigger) {
		case (0):
			player.commands_allowed = false;
			player.walker_visible = false;
			seq[fx_take_9] = kernel_seq_forward(ss[fx_take_9], false, 5, 0, 0, 1);
			kernel_seq_range(seq[fx_take_9], 1, 4);
			kernel_seq_player(seq[fx_take_9], true);
			kernel_seq_trigger(seq[fx_take_9], KERNEL_TRIGGER_EXPIRE, 0, 1);
			kernel_seq_trigger(seq[fx_take_9], KERNEL_TRIGGER_SPRITE, 4, 4);
			goto handled;
			break;

		case 1:
			temp = seq[fx_take_9];
			seq[fx_take_9] = kernel_seq_stamp(ss[fx_take_9], false, 4);
			kernel_synch(KERNEL_SERIES, seq[fx_take_9], KERNEL_SERIES, temp);
			kernel_seq_player(seq[fx_take_9], false);
			kernel_timing_trigger(HALF_SECOND, 2);
			goto handled;
			break;

		case 2:
			kernel_seq_delete(seq[fx_take_9]);
			seq[fx_take_9] = kernel_seq_backward(ss[fx_take_9], false, 5, 0, 0, 1);
			kernel_seq_range(seq[fx_take_9], 1, 4);
			kernel_seq_player(seq[fx_take_9], false);
			kernel_seq_trigger(seq[fx_take_9], KERNEL_TRIGGER_EXPIRE, 0, 3);
			goto handled;
			break;

		case 3:
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_9]);
			player.walker_visible = true;
			text_show(text_409_23);
			/* the door is locked */
			player.commands_allowed = true;
			goto handled;
			break;

		case 4:
			sound_play(N_DoorHandle004);
			goto handled;
			break;
		}
	}

	if (player_said_1(put) && player_said_1(floor)) {
		if (player_said_1(red_frame) ||
			player_said_1(blue_frame) ||
			player_said_1(yellow_frame) ||
			player_said_1(green_frame)) {
			if (local->frame_is_here) {
				text_show(text_000_29);
				goto handled;

			} else {

				switch (kernel.trigger) {
				case (0):
					player.commands_allowed = false;
					player.walker_visible = false;
					seq[fx_bend_down_9] = kernel_seq_pingpong(ss[fx_bend_down_9], true,
						5, 0, 0, 2);
					kernel_seq_range(seq[fx_bend_down_9], 1, 5);
					kernel_seq_player(seq[fx_bend_down_9], true);
					kernel_seq_trigger(seq[fx_bend_down_9],
						KERNEL_TRIGGER_SPRITE, 5, 1);
					kernel_seq_trigger(seq[fx_bend_down_9],
						KERNEL_TRIGGER_EXPIRE, 0, 2);
					break;

				case 1:
					if (player_said_1(red_frame)) {
						inter_move_object(red_frame, NOWHERE);
						object[red_frame].location = global[catacombs_room] + 600;
						seq[fx_red_frame] = kernel_seq_stamp(ss[fx_red_frame], false, 1);
						kernel_seq_depth(seq[fx_red_frame], 14);
						local->dyn_red = kernel_add_dynamic(words_red_frame, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
							series_list[ss[fx_red_frame]]->index[0].x - 5, series_list[ss[fx_red_frame]]->index[0].y - 5, 10, 6);
						kernel_dynamic_walk(local->dyn_red, series_list[ss[fx_red_frame]]->index[0].x + 12,
							series_list[ss[fx_red_frame]]->index[0].y, FACING_NORTHWEST);
					}

					if (player_said_1(green_frame)) {
						inter_move_object(green_frame, NOWHERE);
						object[green_frame].location = global[catacombs_room] + 600;
						seq[fx_green_frame] = kernel_seq_stamp(ss[fx_green_frame], false, 1);
						kernel_seq_depth(seq[fx_green_frame], 14);
						local->dyn_green = kernel_add_dynamic(words_green_frame, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
							series_list[ss[fx_red_frame]]->index[0].x - 5, series_list[ss[fx_red_frame]]->index[0].y - 5, 10, 6);
						kernel_dynamic_walk(local->dyn_green, series_list[ss[fx_red_frame]]->index[0].x + 12,
							series_list[ss[fx_red_frame]]->index[0].y, FACING_NORTHWEST);
					}

					if (player_said_1(blue_frame)) {
						inter_move_object(blue_frame, NOWHERE);
						object[blue_frame].location = global[catacombs_room] + 600;
						seq[fx_blue_frame] = kernel_seq_stamp(ss[fx_blue_frame], false, 1);
						kernel_seq_depth(seq[fx_blue_frame], 14);
						local->dyn_blue = kernel_add_dynamic(words_blue_frame, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
							series_list[ss[fx_red_frame]]->index[0].x - 5, series_list[ss[fx_red_frame]]->index[0].y - 5, 10, 6);
						kernel_dynamic_walk(local->dyn_blue, series_list[ss[fx_red_frame]]->index[0].x + 12,
							series_list[ss[fx_red_frame]]->index[0].y, FACING_NORTHWEST);
					}

					if (player_said_1(yellow_frame)) {
						inter_move_object(yellow_frame, NOWHERE);
						object[yellow_frame].location = global[catacombs_room] + 600;
						seq[fx_yellow_frame] = kernel_seq_stamp(ss[fx_yellow_frame], false, 1);
						kernel_seq_depth(seq[fx_yellow_frame], 14);
						local->dyn_yellow = kernel_add_dynamic(words_yellow_frame, words_walk_to, SYNTAX_SINGULAR, KERNEL_NONE,
							series_list[ss[fx_red_frame]]->index[0].x - 5, series_list[ss[fx_red_frame]]->index[0].y - 5, 10, 6);
						kernel_dynamic_walk(local->dyn_yellow, series_list[ss[fx_red_frame]]->index[0].x + 12,
							series_list[ss[fx_red_frame]]->index[0].y, FACING_NORTHWEST);
					}
					break;

				case 2:
					kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_bend_down_9]);
					player.walker_visible = true;
					player.commands_allowed = true;
					break;
				}
				goto handled;
			}
		}
	}


	if (player_said_1(take)) {
		if (player_said_1(red_frame) || player_said_1(green_frame) ||
			player_said_1(blue_frame) || player_said_1(yellow_frame)) {

			if ((local->frame_here_for_taking || kernel.trigger)) {
				switch (kernel.trigger) {
				case (0):
					player.commands_allowed = false;
					player.walker_visible = false;
					seq[fx_bend_down_9] = kernel_seq_pingpong(ss[fx_bend_down_9], true,
						5, 0, 0, 2);
					kernel_seq_range(seq[fx_bend_down_9], 1, 5);
					kernel_seq_player(seq[fx_bend_down_9], true);
					kernel_seq_trigger(seq[fx_bend_down_9],
						KERNEL_TRIGGER_SPRITE, 5, 1);
					kernel_seq_trigger(seq[fx_bend_down_9],
						KERNEL_TRIGGER_EXPIRE, 0, 2);
					break;

				case 1:
					if (player_said_1(red_frame)) {
						kernel_seq_delete(seq[fx_red_frame]);
						kernel_delete_dynamic(local->dyn_red);
						inter_give_to_player(red_frame);
					}

					if (player_said_1(green_frame)) {
						kernel_seq_delete(seq[fx_green_frame]);
						kernel_delete_dynamic(local->dyn_green);
						inter_give_to_player(green_frame);
					}

					if (player_said_1(blue_frame)) {
						kernel_seq_delete(seq[fx_blue_frame]);
						kernel_delete_dynamic(local->dyn_blue);
						inter_give_to_player(blue_frame);
					}

					if (player_said_1(yellow_frame)) {
						kernel_seq_delete(seq[fx_yellow_frame]);
						kernel_delete_dynamic(local->dyn_yellow);
						inter_give_to_player(yellow_frame);
					}

					sound_play(N_TakeObjectSnd);
					break;

				case 2:
					kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_bend_down_9]);
					player.walker_visible = true;
					player.commands_allowed = true;
					break;
				}
				goto handled;
			}
		}
	}

	if (player_said_2(walk_through, archway_to_north)) {
		global_enter_catacombs(true);
		goto handled;
	}

	if (player_said_2(exit_to, more_catacombs)) {
		global_enter_catacombs(false);
		goto handled;
	}

	if (player.look_around) {
		text_show(text_409_10);
		goto handled;
	}

	if (player_said_2(take, sword) &&
		(object_is_here(sword) || kernel.trigger)) {
		switch (kernel.trigger) {
		case (0):
			global[player_score] += 5;
			player.commands_allowed = false;
			player.walker_visible = false;
			seq[fx_bend_down_9] = kernel_seq_pingpong(ss[fx_bend_down_9], true,
				5, 0, 0, 2);
			kernel_seq_range(seq[fx_bend_down_9], 1, 5);
			kernel_seq_player(seq[fx_bend_down_9], true);
			kernel_seq_trigger(seq[fx_bend_down_9],
				KERNEL_TRIGGER_SPRITE, 5, 1);
			kernel_seq_trigger(seq[fx_bend_down_9],
				KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			kernel_seq_delete(seq[fx_sword]);
			kernel_flip_hotspot(words_sword, false);
			inter_give_to_player(sword);
			sound_play(N_TakeObjectSnd);
			break;

		case 2:
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_bend_down_9]);
			player.walker_visible = true;
			kernel_timing_trigger(20, 3);
			break;

		case 3:
			object_examine(sword, text_008_08, 0);
			/* You pick up the sword frame */
			player.commands_allowed = true;
			break;
		}
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(wall)) {
			text_show(text_409_11);
			goto handled;
		}

		if (player_said_1(floor)) {
			text_show(text_409_12);
			goto handled;
		}

		if (player_said_1(archway)) {
			text_show(text_409_13);
			goto handled;
		}

		if (player_said_1(exposed_brick)) {
			text_show(text_409_14);
			goto handled;
		}

		if (player_said_1(more_catacombs)) {
			text_show(text_409_15);
			goto handled;
		}

		if (player_said_1(blocked_archway)) {
			text_show(text_409_16);
			goto handled;
		}

		if (player_said_1(grate)) {
			text_show(text_409_17);
			goto handled;
		}

		if (player_said_1(unlucky_adventurer)) {
			text_show(text_409_20);
			goto handled;
		}

		if (player_said_1(red_frame)) {
			if (!player_has(red_frame)) {
				object_examine(red_frame, text_008_02, 0);
				goto handled;
			}
		}

		if (player_said_1(green_frame)) {
			if (!player_has(green_frame)) {
				object_examine(green_frame, text_008_19, 0);
				goto handled;
			}
		}

		if (player_said_1(blue_frame)) {
			if (!player_has(blue_frame)) {
				object_examine(blue_frame, text_008_17, 0);
				goto handled;
			}
		}

		if (player_said_1(yellow_frame)) {
			if (!player_has(yellow_frame)) {
				object_examine(yellow_frame, text_008_04, 0);
				goto handled;
			}
		}

		if (player_said_1(sword) && object_is_here(sword)) {
			text_show(text_409_21);
			goto handled;
		}
	}

	if (player_said_2(look, switch_panel)) {
		text_show(text_409_19);
		new_room = 410;
		goto handled;
	}

	if (player_said_2(walk_to, switch_panel)) {
		new_room = 410;
		goto handled;
	}

	if ((player_said_2(open, grate)) ||
		(player_said_2(push, grate)) ||
		(player_said_2(pull, grate))) {
		switch (kernel.trigger) {
		case (0):
			player.commands_allowed = false;
			player.walker_visible = false;
			seq[fx_bend_down_9] = kernel_seq_forward(ss[fx_bend_down_9], false,
				5, 0, 0, 1);
			kernel_seq_range(seq[fx_bend_down_9], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_player(seq[fx_bend_down_9], true);
			kernel_seq_trigger(seq[fx_bend_down_9],
				KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2:
			seq[fx_bend_down_9] = kernel_seq_stamp
			(ss[fx_bend_down_9], false, KERNEL_LAST);
			kernel_seq_player(seq[fx_bend_down_9], true);
			/* kernel_seq_depth (seq[fx_bend_down_9], 1); */
			kernel_timing_trigger(HALF_SECOND, 3);
			break;

		case 3:
			/* temp = seq[fx_bend_down_9]; */
			kernel_seq_delete(seq[fx_bend_down_9]);
			seq[fx_bend_down_9] = kernel_seq_backward(ss[fx_bend_down_9], false,
				5, 0, 0, 1);
			/* kernel_synch (KERNEL_SERIES, seq[fx_bend_down_9], KERNEL_SERIES, temp); */
			kernel_seq_range(seq[fx_bend_down_9], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_player(seq[fx_bend_down_9], false);
			kernel_seq_trigger(seq[fx_bend_down_9],
				KERNEL_TRIGGER_EXPIRE, 0, 4);
			break;

		case 4:
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_bend_down_9]);
			kernel_timing_trigger(TENTH_SECOND, 5);
			break;

		case 5:
			player.commands_allowed = true;
			text_show(text_409_18);
			break;
		}
		goto handled;
	}

	if (player_said_2(talk_to, unlucky_adventurer)) {
		text_show(text_409_24);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_409_daemon() {
	if (kernel.trigger == ROOM_409_DIE) {
		player.walker_visible = false;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_gas]);
		global_speech_go(speech_raoul_gas);
	}

	if (kernel.trigger == ROOM_409_DIE + 1) {
		seq[fx_gas] = kernel_seq_stamp
		(ss[fx_gas], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_gas], 1);
		kernel_timing_trigger(ONE_SECOND, ROOM_409_DIE + 2);
		sound_play(N_PlayerDies);
	}

	if (kernel.trigger == ROOM_409_DIE + 2) {
		kernel.force_restart = true;
	}

	if (kernel.trigger == ROOM_409_DOOR_OPENS) {
		player.commands_allowed = true;
		global[door_in_409_is_open] = true;
		kernel_flip_hotspot(words_door, false);
		kernel_flip_hotspot(words_archway_to_north, true);
	}
}

void room_409_preload() {
	room_init_code_pointer = room_409_init;
	room_pre_parser_code_pointer = room_409_pre_parser;
	room_parser_code_pointer = room_409_parser;
	room_daemon_code_pointer = room_409_daemon;

	section_4_walker();
	section_4_interface();

	vocab_make_active(words_red_frame);
	vocab_make_active(words_yellow_frame);
	vocab_make_active(words_blue_frame);
	vocab_make_active(words_green_frame);
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
