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
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/phantom/mads/inventory.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/rooms/section4.h"
#include "mads/madsv2/phantom/rooms/room404.h"
#include "mads/madsv2/phantom/catacombs.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_404_init() {
	global_catacombs_init();

	kernel_flip_hotspot(words_rat_s_nest, false);
	kernel_flip_hotspot(words_web, false);
	kernel_flip_hotspot(words_broken_pot, false);
	kernel_flip_hotspot(words_block, false);
	kernel_flip_hotspot(words_puddle, false);

	local->anim_0_running = false;

	/* =================== Load sprites series =================== */

	ss[fx_bend_down_9] = kernel_load_series("*RRD_9", false);
	ss[fx_red_frame] = kernel_load_series(kernel_name('f', 0), false);
	ss[fx_green_frame] = kernel_load_series(kernel_name('f', 1), false);
	ss[fx_blue_frame] = kernel_load_series(kernel_name('f', 2), false);
	ss[fx_yellow_frame] = kernel_load_series(kernel_name('f', 3), false);


	if (global_catacombs_exit(NORTH) == -1) {
		ss[fx_north] = kernel_load_series(kernel_name('c', 1), false);
		kernel_draw_to_background(ss[fx_north], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
		kernel_flip_hotspot(words_archway_to_north, false);
	}

	if (global_catacombs_exit(WEST) == -1) {
		ss[fx_west] = kernel_load_series(kernel_name('c', 0), false);
		kernel_draw_to_background(ss[fx_west], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
		kernel_flip_hotspot(words_archway_to_west, false);
	}

	if (global_catacombs_exit(EAST) == -1) {
		ss[fx_east] = kernel_load_series(kernel_name('c', 2), false);
		kernel_draw_to_background(ss[fx_east], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
		kernel_flip_hotspot(words_archway_to_east, false);
	}

	if (global_catacombs_exit(SOUTH) == -1) {
		kernel_flip_hotspot(words_more_catacombs, false);
	}


	if (global[catacombs_misc] & MAZE_RAT_NEST) {
		ss[fx_rat_nest] = kernel_load_series(kernel_name('c', 3), false);
		kernel_draw_to_background(ss[fx_rat_nest], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
		kernel_flip_hotspot(words_rat_s_nest, true);
	}

	if (global[catacombs_misc] & MAZE_WEB) {
		ss[fx_web] = kernel_load_series(kernel_name('c', 4), false);
		kernel_draw_to_background(ss[fx_web], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
		kernel_flip_hotspot(words_web, true);
	}

	if (global[catacombs_misc] & MAZE_POT) {
		ss[fx_pot] = kernel_load_series(kernel_name('c', 5), false);
		kernel_draw_to_background(ss[fx_pot], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
		kernel_flip_hotspot(words_broken_pot, true);
	}

	if (global[catacombs_misc] & MAZE_BLOCK) {
		ss[fx_block] = kernel_load_series(kernel_name('c', 7), false);
		kernel_draw_to_background(ss[fx_block], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
	}

	if (global[catacombs_misc] & MAZE_FALLEN_BLOCK) {
		ss[fx_fallen_block] = kernel_load_series(kernel_name('c', 8), false);
		kernel_draw_to_background(ss[fx_fallen_block], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
		kernel_flip_hotspot(words_block, true);
	}

	if (global[catacombs_misc] & MAZE_PUDDLE) {
		kernel_flip_hotspot(words_puddle, true);
		aa[0] = kernel_run_animation(kernel_name('d', 1), 0);
		local->anim_0_running = true;
	}

	if (previous_room != KERNEL_RESTORING_GAME) {

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

		case EAST:
			player.x = EAST_X;
			player.y = EAST_Y;
			player.facing = FACING_WEST;
			player_walk(WALK_TO_EAST_X, WALK_TO_EAST_Y, FACING_WEST);
			break;

		case WEST:
			player.x = WEST_X;
			player.y = WEST_Y;
			player.facing = FACING_SOUTH;
			player_walk(WALK_TO_WEST_X, WALK_TO_WEST_Y, FACING_EAST);
			break;
		}
	}

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

	if (global[priest_piston_puke]) {
		kernel_timing_trigger(TWO_SECONDS, ROOM_404_SHOW_TEXT);
	}

	section_4_music();
}

void room_404_pre_parser() {
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
}

void room_404_parser() {
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
		global_catacombs_move(NORTH);
		goto handled;
	}

	if (player_said_2(walk_through, archway_to_west)) {
		global_catacombs_move(WEST);
		goto handled;
	}

	if (player_said_2(exit_to, more_catacombs)) {
		global_catacombs_move(SOUTH);
		goto handled;
	}

	if (player_said_2(walk_through, archway_to_east)) {
		global_catacombs_move(EAST);
		goto handled;
	}

	if (player.look_around) {
		text_show(text_404_10);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(wall)) {
			text_show(text_404_11);
			goto handled;
		}

		if (player_said_1(floor)) {
			text_show(text_404_12);
			goto handled;
		}

		if (player_said_1(archway)) {
			text_show(text_404_13);
			goto handled;
		}

		if (player_said_1(exposed_brick)) {
			text_show(text_404_14);
			goto handled;
		}

		if (player_said_1(more_catacombs)) {
			text_show(text_404_15);
			goto handled;
		}

		if (player_said_1(puddle)) {
			text_show(text_404_17);
			goto handled;
		}

		if (player_said_1(rat_s_nest)) {
			text_show(text_404_18);
			goto handled;
		}

		if (player_said_1(broken_pot)) {
			text_show(text_404_21);
			goto handled;
		}

		if (player_said_1(web)) {
			text_show(text_404_24);
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

		if (player_said_1(block)) {
			text_show(text_404_30);
			goto handled;
		}
	}

	if (player_said_2(take, rat_s_nest)) {
		text_show(text_404_19);
		goto handled;
	}

	if (player_said_2(take, broken_pot)) {
		text_show(text_404_22);
		goto handled;
	}

	if (player_said_2(take, block)) {
		text_show(text_404_31);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_404_daemon() {
	if (local->anim_0_running) {
		if (kernel_anim[aa[0]].frame == 20) {
			kernel_reset_animation(aa[0], 0);
		}
	}

	if (kernel.trigger == ROOM_404_SHOW_TEXT) {
		text_show(text_000_30);
		global[priest_piston_puke] = false;
	}
}

void room_404_preload() {
	room_init_code_pointer = room_404_init;
	room_pre_parser_code_pointer = room_404_pre_parser;
	room_parser_code_pointer = room_404_parser;
	room_daemon_code_pointer = room_404_daemon;

	section_4_walker();
	section_4_interface();

	if (global[catacombs_misc] & MAZE_BLOCK) {
		kernel_initial_variant = 1;
	}

	vocab_make_active(words_red_frame);
	vocab_make_active(words_yellow_frame);
	vocab_make_active(words_blue_frame);
	vocab_make_active(words_green_frame);
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
