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
#include "mads/madsv2/phantom/rooms/room407.h"
#include "mads/madsv2/phantom/catacombs.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_407_init() {
	global_catacombs_init();

	/* =================== Load sprites series =================== */

	ss[fx_bend_down_9] = kernel_load_series("*RRD_9", false);
	ss[fx_red_frame] = kernel_load_series(kernel_name('f', 0), false);
	ss[fx_green_frame] = kernel_load_series(kernel_name('f', 1), false);
	ss[fx_blue_frame] = kernel_load_series(kernel_name('f', 2), false);
	ss[fx_yellow_frame] = kernel_load_series(kernel_name('f', 3), false);

	if (global_catacombs_exit(WEST) == -1) {
		kernel_flip_hotspot_loc(words_more_catacombs, false, CAT_LEFT_1_X, CAT_LEFT_1_Y);
		kernel_flip_hotspot_loc(words_more_catacombs, false, CAT_LEFT_2_X, CAT_LEFT_2_Y);
		kernel_flip_hotspot_loc(words_more_catacombs, false, CAT_LEFT_3_X, CAT_LEFT_3_Y);
		kernel_flip_hotspot_loc(words_more_catacombs, false, CAT_LEFT_4_X, CAT_LEFT_4_Y);
	}

	if (global_catacombs_exit(EAST) == -1) {
		kernel_flip_hotspot_loc(words_more_catacombs, false, CAT_RIGHT_1_X, CAT_RIGHT_1_Y);
		kernel_flip_hotspot_loc(words_more_catacombs, false, CAT_RIGHT_2_X, CAT_RIGHT_2_Y);
		kernel_flip_hotspot_loc(words_more_catacombs, false, CAT_RIGHT_3_X, CAT_RIGHT_3_Y);
		kernel_flip_hotspot_loc(words_more_catacombs, false, CAT_RIGHT_4_X, CAT_RIGHT_4_Y);
	}

	if (previous_room != KERNEL_RESTORING_GAME) {

		switch (global[catacombs_from]) {

		case NORTH:
			player.x = NORTH_X;
			player.y = NORTH_Y;
			player.facing = FACING_WEST;
			player_walk(WALK_TO_NORTH_X, WALK_TO_NORTH_Y, FACING_WEST);
			break;

		case SOUTH:
			player.x = SOUTH_X;
			player.y = SOUTH_Y;
			player.facing = FACING_EAST;
			player_walk(WALK_TO_SOUTH_X, WALK_TO_SOUTH_Y, FACING_EAST);
			break;

		case EAST:
			player_first_walk(EAST_X, EAST_Y, FACING_WEST, WALK_TO_EAST_X, WALK_TO_EAST_Y, FACING_WEST, true);
			break;

		case WEST:
			player_first_walk(WEST_X, WEST_Y, FACING_WEST, WALK_TO_WEST_X, WALK_TO_WEST_Y, FACING_WEST, true);
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
	section_4_music();
}

void room_407_pre_parser() {
	if (player_said_2(exit_to, more_catacombs)) {
		if (player.y > 30) {
			if (inter_point_x < 100) {
				global_catacombs_move(WEST);
			} else {
				global_catacombs_move(EAST);
			}
		} else {
			player.need_to_walk = false;
		}
	}

	if (player_said_1(walk_through)) {
		if (player.y > 30) {
			player.need_to_walk = false;
		}
	}

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
			if ((local->frame_is_here) || (player.y < 30) || (inter_point_y < 30)) {
				player.need_to_walk = false;

			} else {
				player_walk(series_list[ss[fx_red_frame]]->index[0].x + 12,
					series_list[ss[fx_red_frame]]->index[0].y, FACING_NORTHWEST);
			}
		}
	}

	if (player_said_1(take)) {
		if (player_said_1(red_frame) || player_said_1(green_frame) ||
			player_said_1(blue_frame) || player_said_1(yellow_frame)) {

			if (local->frame_here_for_taking) {
				if (player.y < 30) {
					player.need_to_walk = false;
				}
			}
		}
	}

	if (player_said_2(walk_across, floor)) {
		if ((player.y < 30) && (inter_point_y > 29)) {
			player.need_to_walk = false;
		}
	}

	if (player_said_2(walk_to, lake) && (player.y < 30)) {
		player_walk(LAKE_X, LAKE_Y, FACING_SOUTH);
	}

	if (player_said_1(walk_to) && (inter_point_x < 130) && (player.y < 30)) {
		player.need_to_walk = false;
	}

	if (player_said_1(walk_to) && (inter_point_x > 203) && (player.y < 30)) {
		player.need_to_walk = false;
	}

	if (player_said_2(walk_to, wall) && (player.y > 30)) {
		if ((inter_point_x > 160) && (inter_point_x < 190)) {
			player.need_to_walk = false;
		}
	}
}

void room_407_parser() {
	if (player_said_2(walk_to, wall) && (player.y > 30)) {
		if ((inter_point_x > 160) && (inter_point_x < 190)) {
			text_show(text_407_18);
			goto handled;
		}
	}

	if (player_said_1(walk_to) && (inter_point_x < 130) && (player.y < 30)) {
		text_show(text_407_18);
		goto handled;
	}

	if (player_said_1(walk_to) && (inter_point_x > 203) && (player.y < 30)) {
		text_show(text_407_18);
		goto handled;
	}

	if (player_said_2(walk_across, floor)) {
		if ((player.y < 30) && (inter_point_y > 29)) {
			text_show(text_407_18);
			goto handled;

		} else if ((player.y > 29) && (inter_point_y < 30)) {
			text_show(text_407_18);
			goto handled;
			/* you can't cross the lake */
		}
	}

	if (player_said_1(put) && player_said_1(floor)) {
		if (player_said_1(red_frame) ||
			player_said_1(blue_frame) ||
			player_said_1(yellow_frame) ||
			player_said_1(green_frame)) {

			if ((player.y < 30) && (inter_point_y < 30)) {
				text_show(text_407_17);
				goto handled;
				/* you are on far side and want to put frame down on far side */

			} else if ((player.y < 30) && (inter_point_y > 29)) {
				text_show(text_407_18);
				goto handled;
				/* you are on far side and want to put frame down on near side */

			} else if (local->frame_is_here) {
				text_show(text_000_29);
				/* a frame is already here */
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
				if (player.y < 30) {
					text_show(text_407_18);
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
	}

	if (player_said_2(walk_through, archway_to_west)) {
		if (player.y < 30) {
			global_catacombs_move(SOUTH);
		} else {
			text_show(text_407_18);
		}
		goto handled;
	}

	if (player_said_2(walk_through, archway_to_east)) {
		if (player.y < 30) {
			global_catacombs_move(NORTH);
		} else {
			text_show(text_407_18);
		}
		goto handled;
	}

	if (player_said_2(exit_to, more_catacombs)) {
		if (player.y < 30) {
			text_show(text_407_18);
			goto handled;
		}
	}

	if (player.look_around) {
		text_show(text_407_10);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(wall)) {
			text_show(text_407_11);
			goto handled;
		}

		if (player_said_1(floor)) {
			text_show(text_407_12);
			goto handled;
		}

		if (player_said_1(archway)) {
			text_show(text_407_13);
			goto handled;
		}

		if (player_said_1(more_catacombs)) {
			text_show(text_407_14);
			goto handled;
		}

		if (player_said_1(column)) {
			text_show(text_407_15);
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
				object_examine(green_frame, text_008_18, 0);
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

		if (player_said_1(lake)) {
			text_show(text_407_16);
			goto handled;
		}
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_407_preload() {
	room_init_code_pointer = room_407_init;
	room_pre_parser_code_pointer = room_407_pre_parser;
	room_parser_code_pointer = room_407_parser;
	room_daemon_code_pointer = NULL;

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
