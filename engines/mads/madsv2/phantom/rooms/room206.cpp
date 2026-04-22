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
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/rail.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/quote.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/buffer.h"
#include "mads/madsv2/phantom/global.h"
#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/phantom/mads/inventory.h"
#include "mads/madsv2/phantom/mads/quotes.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/mads/speeches.h"
#include "mads/madsv2/phantom/mads/text.h"
#include "mads/madsv2/phantom/mads/words.h"
#include "mads/madsv2/phantom/rooms/section2.h"
#include "mads/madsv2/phantom/rooms/room206.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_206_daemon() {
	switch (kernel.trigger) {
	case ROOM_206_FROM_308:
		sound_play(N_DoorOpens);
		seq[fx_panel_closes] = kernel_seq_stamp(ss[fx_panel_closes], false, 1);
		kernel_seq_depth(seq[fx_panel_closes], 14);
		seq[fx_panel_opens] = kernel_seq_backward(ss[fx_panel_opens], false, 8, 0, 0, 1);
		kernel_seq_depth(seq[fx_panel_opens], 13);
		kernel_seq_range(seq[fx_panel_opens], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_panel_opens],
		                   KERNEL_TRIGGER_EXPIRE, 0, ROOM_206_FROM_308 + 1);
		break;

	case ROOM_206_FROM_308 + 1:
		seq[fx_panel_appears] = kernel_seq_stamp(ss[fx_panel_appears], false, 3);
		kernel_seq_depth(seq[fx_panel_appears], 13);
		player.commands_allowed = true;
		break;

	case ROOM_206_HIT_ON_HEAD:
		player.x              = HIT_HEAD_X;
		player.y              = HIT_HEAD_Y;
		player.walker_visible = true;
		player.commands_allowed = true;
		local->anim_0_running = false;
		player_demand_facing(FACING_WEST);
		break;
	}

	if (local->anim_0_running && !local->prevent) {
		if (kernel_anim[aa[0]].frame == 118) {
			global[knocked_over_head] = true;
			local->prevent = true;
			kernel_timing_trigger(1, ROOM_206_FROM_308 + 2);
		}
	}

	if (local->anim_0_running && !local->prevent_2) {
		if (kernel_anim[aa[0]].frame == 61) {
			if (global_prefer_roland) {
				sound_play(N_WomanScream002);
			} else {
				global_speech_go(speech_woman_scream);
			}
			local->prevent_2 = true;
		}
	}

	switch (kernel.trigger) {
	case ROOM_206_FROM_308 + 2:
		sound_play(N_DoorOpens);
		seq[fx_panel_closes] = kernel_seq_stamp(ss[fx_panel_closes], false, 1);
		kernel_seq_depth(seq[fx_panel_closes], 14);
		seq[fx_panel_opens] = kernel_seq_backward(ss[fx_panel_opens], false, 8, 0, 0, 1);
		kernel_seq_depth(seq[fx_panel_opens], 13);
		kernel_seq_range(seq[fx_panel_opens], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_panel_opens],
		                   KERNEL_TRIGGER_EXPIRE, 0, ROOM_206_FROM_308 + 3);
		break;

	case ROOM_206_FROM_308 + 3:
		seq[fx_panel_appears] = kernel_seq_stamp(ss[fx_panel_appears], false, 3);
		kernel_seq_depth(seq[fx_panel_appears], 13);
		break;
	}
}

void room_206_pre_parser() {
	if (player_said_2(look, left_column)) {
		player.need_to_walk = true;
	}

	if (player_said_2(unlock, panel) || player_said_2(walk_behind, panel) ||
	    player_said_2(lock, panel)   || player_said_2(open, panel)) {
		player_walk(PANEL_X, PANEL_Y, FACING_NORTHWEST);
	}

	if (player_said_1(left_column)) {
		if (global[panel_in_206] == PANEL_UNDISCOVERED) {
			player_walk(PANEL_X - 5, PANEL_Y, FACING_NORTHWEST);
		}
	}
}

void room_206_parser() {
	int temp;

	if (player_said_2(exit_to, loge_corridor)) {
		if (global[right_door_is_open_504]) {
			if (global_prefer_roland) {
				sound_play(N_WomanScream002);
			} else {
				global_speech_go(speech_woman_scream);
			}
			conv_run(CONV_MISC);
			conv_export_value(4);
		} else {
			new_room = 205;
		}
		goto handled;
	}

	if (player_said_2(take, crumpled_note) &&
	    (object_is_here(crumpled_note) || kernel.trigger)) {
		switch (kernel.trigger) {
		case 0:
			global[player_score] += 5;
			player.commands_allowed = false;
			player.walker_visible   = false;
			seq[fx_take_9] = kernel_seq_pingpong(ss[fx_take_9], false, 5, 0, 0, 2);
			kernel_seq_range(seq[fx_take_9], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_player(seq[fx_take_9], true);
			kernel_seq_trigger(seq[fx_take_9],
			                   KERNEL_TRIGGER_SPRITE, 4, ROOM_206_TAKE_NOTE);
			kernel_seq_trigger(seq[fx_take_9],
			                   KERNEL_TRIGGER_EXPIRE, 0, ROOM_206_TAKE_NOTE + 1);
			goto handled;
			break;

		case ROOM_206_TAKE_NOTE:
			kernel_seq_delete(seq[fx_note]);
			kernel_flip_hotspot(words_crumpled_note, false);
			inter_give_to_player(crumpled_note);
			sound_play(N_TakeObjectSnd);
			goto handled;
			break;

		case ROOM_206_TAKE_NOTE + 1:
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_9]);
			player.walker_visible = true;
			kernel_timing_trigger(20, ROOM_206_TAKE_NOTE + 2);
			goto handled;
			break;

		case ROOM_206_TAKE_NOTE + 2:
			object_examine(crumpled_note, text_008_16, 6);
			/* You pick up the crumpled note */
			player.commands_allowed = true;
			goto handled;
			break;

		default:
			break;
		}
	}

	if (player_said_2(walk_behind, panel) || player_said_2(open, panel) ||
	    ((kernel.trigger >= ROOM_206_ENTER_PANEL) && (kernel.trigger < ROOM_206_TAKE_NOTE))) {
		if (global[panel_in_206] == PANEL_UNLOCKED) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible   = false;
				seq[fx_take_9] = kernel_seq_pingpong(ss[fx_take_9], true, 7, 0, 0, 2);
				kernel_seq_range(seq[fx_take_9], 1, 3);
				kernel_seq_player(seq[fx_take_9], true);
				kernel_seq_trigger(seq[fx_take_9],
				                   KERNEL_TRIGGER_SPRITE, 3, ROOM_206_ENTER_PANEL);
				kernel_seq_trigger(seq[fx_take_9],
				                   KERNEL_TRIGGER_EXPIRE, 0, ROOM_206_ENTER_PANEL + 1);
				break;

			case ROOM_206_ENTER_PANEL:
				sound_play(N_DoorOpens);
				kernel_seq_delete(seq[fx_panel_appears]);
				seq[fx_panel_opens] = kernel_seq_forward(ss[fx_panel_opens], false, 8, 0, 0, 1);
				kernel_seq_depth(seq[fx_panel_opens], 13);
				kernel_seq_range(seq[fx_panel_opens], KERNEL_FIRST, KERNEL_LAST);
				break;

			case ROOM_206_ENTER_PANEL + 1:
				seq[fx_panel_opens] = kernel_seq_stamp(ss[fx_panel_opens], false, KERNEL_LAST);
				kernel_seq_depth(seq[fx_panel_opens], 14);
				player.commands_allowed = true;
				player.walker_visible   = true;
				player_walk(BEHIND_PANEL_X, BEHIND_PANEL_Y, FACING_NORTHWEST);
				player_walk_trigger(ROOM_206_ENTER_PANEL + 2);
				break;

			case ROOM_206_ENTER_PANEL + 2:
				sound_play(N_DoorOpens);
				kernel_seq_delete(seq[fx_panel_opens]);
				seq[fx_panel_opens] = kernel_seq_backward(ss[fx_panel_opens], false, 8, 0, 0, 1);
				kernel_seq_depth(seq[fx_panel_opens], 13);
				kernel_seq_range(seq[fx_panel_opens], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_trigger(seq[fx_panel_opens],
				                   KERNEL_TRIGGER_EXPIRE, 0, ROOM_206_ENTER_PANEL + 3);
				break;

			case ROOM_206_ENTER_PANEL + 3:
				seq[fx_panel_appears] = kernel_seq_stamp(ss[fx_panel_appears], false, KERNEL_LAST);
				kernel_seq_depth(seq[fx_panel_appears], 14);
				new_room = 308;
				break;
			}

		} else {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible   = false;
				seq[fx_take_9] = kernel_seq_pingpong(ss[fx_take_9], true, 7, 0, 0, 2);
				kernel_seq_range(seq[fx_take_9], 1, 3);
				kernel_seq_player(seq[fx_take_9], true);
				kernel_seq_trigger(seq[fx_take_9],
				                   KERNEL_TRIGGER_EXPIRE, 0, ROOM_206_ENTER_PANEL + 4);
				kernel_seq_trigger(seq[fx_take_9],
				                   KERNEL_TRIGGER_SPRITE, 3, ROOM_206_ENTER_PANEL + 5);
				break;

			case ROOM_206_ENTER_PANEL + 5:
				sound_play(N_DoorHandle002);
				break;

			case ROOM_206_ENTER_PANEL + 4:
				player.commands_allowed = true;
				player.walker_visible   = true;
				text_show(text_206_25);
				/* the panel is locked */
				break;
			}
		}
		goto handled;
	}

	if (player_said_2(unlock, panel)  || player_said_2(lock, panel) || player_said_3(put, key, keyhole) ||
	    player_said_2(unlock, keyhole) || player_said_2(lock, keyhole) ||
	    (kernel.trigger >= ROOM_206_SCREW_WITH_PANEL)) {
		if (global[panel_in_206] >= PANEL_DISCOVERED) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible   = false;
				seq[fx_take_9] = kernel_seq_forward(ss[fx_take_9], true, 7, 0, 0, 1);
				kernel_seq_range(seq[fx_take_9], 1, 3);
				kernel_seq_player(seq[fx_take_9], true);
				kernel_seq_trigger(seq[fx_take_9],
				                   KERNEL_TRIGGER_EXPIRE, 0, ROOM_206_SCREW_WITH_PANEL);
				break;

			case ROOM_206_SCREW_WITH_PANEL:
				sound_play(N_KeyTurnSnd);
				temp = seq[fx_take_9];
				seq[fx_take_9] = kernel_seq_stamp(ss[fx_take_9], true, KERNEL_LAST);
				kernel_synch(KERNEL_SERIES, seq[fx_take_9], KERNEL_SERIES, temp);
				kernel_seq_player(seq[fx_take_9], false);
				kernel_timing_trigger(HALF_SECOND, ROOM_206_SCREW_WITH_PANEL + 1);
				break;

			case ROOM_206_SCREW_WITH_PANEL + 1:
				kernel_seq_delete(seq[fx_take_9]);
				seq[fx_take_9] = kernel_seq_backward(ss[fx_take_9], true, 7, 0, 0, 1);
				kernel_seq_range(seq[fx_take_9], 1, 3);
				kernel_seq_player(seq[fx_take_9], false);
				kernel_seq_trigger(seq[fx_take_9],
				                   KERNEL_TRIGGER_EXPIRE, 0, ROOM_206_SCREW_WITH_PANEL + 2);
				break;

			case ROOM_206_SCREW_WITH_PANEL + 2:
				if (player_said_1(lock)) {
					global[panel_in_206] = PANEL_LOCKED;
					text_show(text_206_29);
				} else if (player_said_1(unlock)) {
					global[panel_in_206] = PANEL_UNLOCKED;
					text_show(text_206_28);
				}

				if (player_said_1(put)) {
					if (global[panel_in_206] <= PANEL_LOCKED) {
						global[panel_in_206] = PANEL_UNLOCKED;
						text_show(text_206_28);
					} else {
						global[panel_in_206] = PANEL_LOCKED;
						text_show(text_206_29);
					}
				}

				kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_9]);
				player.commands_allowed = true;
				player.walker_visible   = true;
				break;
			}
			goto handled;
		}
	}

	if (player.look_around) {
		text_show(text_206_10);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(wall)) {
			text_show(text_206_11);
			goto handled;
		}

		if (player_said_1(floor)) {
			text_show(text_206_12);
			goto handled;
		}

		if (player_said_1(right_column)) {
			text_show(text_206_14);
			goto handled;
		}

		if (player_said_1(keyhole)) {
			text_show(text_206_15);
			goto handled;
		}

		if (player_said_1(rail)) {
			text_show(text_206_16);
			goto handled;
		}

		if (player_said_1(seat)) {
			text_show(text_206_17);
			goto handled;
		}

		if (player_said_1(loge_corridor)) {
			text_show(text_206_18);
			goto handled;
		}

		if (player_said_1(stage)) {
			if (global[jacques_status]) {
				text_show(text_206_30);
			} else {
				text_show(text_206_19);
			}
			goto handled;
		}

		if (player_said_1(house)) {
			text_show(text_206_20);
			goto handled;
		}

		if (player_said_1(ceiling)) {
			text_show(text_206_21);
			goto handled;
		}

		if (player_said_1(house_light)) {
			text_show(text_206_22);
			goto handled;
		}

		if (player_said_1(panel)) {
			if (global[panel_in_206] == PANEL_UNLOCKED) {
				text_show(text_206_24);
			} else {
				text_show(text_206_26);
			}
			goto handled;
		}

		if ((player_said_1(left_column)) && (!kernel.trigger)) {
			if (global[panel_in_206] == PANEL_UNDISCOVERED) {
				text_show(text_206_13);
				aa[0] = kernel_run_animation(kernel_name('k', 1), ROOM_206_KNOCK);
				player.commands_allowed = false;
				player.walker_visible   = false;

			} else {
				text_show(text_206_23);
			}
			goto handled;
		}

		if (player_said_1(crumpled_note) && object_is_here(crumpled_note)) {
			text_show(text_206_27);
			goto handled;
		}
	}

	switch (kernel.trigger) {
	case ROOM_206_KNOCK:
		player.walker_visible = true;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
		kernel_flip_hotspot(words_panel, true);
		kernel_flip_hotspot(words_keyhole, true);
		seq[fx_panel_appears] = kernel_seq_stamp(ss[fx_panel_appears], false, 3);
		kernel_seq_depth(seq[fx_panel_appears], 13);
		if (!(global[player_score_flags] & SCORE_HOLLOW_COLUMN)) {
			global[player_score_flags] = global[player_score_flags] | SCORE_HOLLOW_COLUMN;
			global[player_score] += 5;
		}
		kernel_timing_trigger(QUARTER_SECOND, ROOM_206_KNOCK + 1); /* a slight delay so anim totally ends */
		goto handled;
		break;

	case ROOM_206_KNOCK + 1:
		player.commands_allowed = true;
		global[panel_in_206] = PANEL_DISCOVERED;
		text_show(text_206_23);
		goto handled;
		break;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_206_init() {
	local->anim_0_running = false;
	local->prevent        = false;
	local->prevent_2      = false;

	global_speech_load(speech_woman_scream);

	conv_get(CONV_MISC);

	/* Load sprites */

	ss[fx_trap_door]     = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_panel_closes]  = kernel_load_series(kernel_name('x', 1), false);
	ss[fx_panel_appears] = kernel_load_series(kernel_name('x', 2), false);
	ss[fx_panel_opens]   = kernel_load_series(kernel_name('x', 3), false);
	ss[fx_take_9]        = kernel_load_series("*RDR_9", false);


	/* Do knocked on panel stuff */

	if (previous_room != 308) {
		switch (global[panel_in_206]) {
		case PANEL_UNDISCOVERED:
			seq[fx_panel_closes] = kernel_seq_stamp(ss[fx_panel_closes], false, 1);
			kernel_seq_depth(seq[fx_panel_closes], 14);
			break;

		default:
			seq[fx_panel_closes] = kernel_seq_stamp(ss[fx_panel_closes], false, 1);
			kernel_seq_depth(seq[fx_panel_closes], 14);
			seq[fx_panel_appears] = kernel_seq_stamp(ss[fx_panel_appears], false, 3);
			kernel_seq_depth(seq[fx_panel_appears], 13);
			break;
		}
	}

	if (global[panel_in_206] == PANEL_UNDISCOVERED) {
		kernel_flip_hotspot(words_panel, false);
		kernel_flip_hotspot(words_keyhole, false);
	}


	if (object_is_here(crumpled_note)) {
		ss[fx_note]  = kernel_load_series(kernel_name('p', 0), false);
		seq[fx_note] = kernel_seq_stamp(ss[fx_note], false, 1);
		kernel_seq_depth(seq[fx_note], 10);

	} else {
		kernel_flip_hotspot(words_crumpled_note, false);
	}


	/* If trap door is open, stamp it */

	if (global[trap_door_status] == TRAP_DOOR_IS_OPEN) {
		seq[fx_trap_door] = kernel_seq_stamp(ss[fx_trap_door], false, 1);
		kernel_seq_depth(seq[fx_trap_door], 14);
	}


	/* Previous Rooms */

	if (global[right_door_is_open_504] && !global[knocked_over_head]) {
		aa[0] = kernel_run_animation(kernel_name('h', 1), ROOM_206_HIT_ON_HEAD);
		player.walker_visible   = false;
		player.commands_allowed = false;
		local->anim_0_running   = true;

	} else if (previous_room == 308) {
		player.x                = BEHIND_PANEL_X;
		player.y                = BEHIND_PANEL_Y;
		player.commands_allowed = false;
		player_walk(PANEL_X, PANEL_Y, FACING_EAST);
		player_walk_trigger(ROOM_206_FROM_308);

	} else if ((previous_room == 205) || (previous_room != KERNEL_RESTORING_GAME)) {
		player.x      = PLAYER_X_FROM_205;
		player.y      = PLAYER_Y_FROM_205;
		player.facing = FACING_NORTH;
	}

	section_2_music();
}

void room_206_preload() {
	room_init_code_pointer = room_206_init;
	room_pre_parser_code_pointer = room_206_pre_parser;
	room_parser_code_pointer = room_206_parser;
	room_daemon_code_pointer = room_206_daemon;

	section_2_walker();
	section_2_interface();
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
