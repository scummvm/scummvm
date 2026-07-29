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

#include "math/utils.h"
#include "mads/core/game.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section5.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _handingLine;
	bool _lineMoving;
	int16 _lineAnimationMode;
	int16 _lineFrame;
	int16 _lineAnimationPosition;
};

static Scratch local;


static void room_511_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('c', 0), 0);
	g_sprite_ids[4] = kernel_load_series("*RXCD_6", 0);

	// WORKARUND: Doing this earlier to allow to ensure hotspot bounds will get set from it's image
	int frame = 0;
	if (global[kLineStatus] == 2)
		frame = -1;
	else if (global[kLineStatus] == 3)
		frame = -2;

	if (global[kLineStatus] == 2 || global[kLineStatus] == 3) {
		g_sprite_ids[7] = kernel_load_series(kernel_name('b', 4), 0);
		g_sequence_ids[7] = kernel_seq_stamp(g_sprite_ids[7], false, frame);
		int idx = kernel_add_dynamic(words_fishing_line, words_walkto, 0, g_sequence_ids[7], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 26, 153, FACING_NORTHEAST);
		kernel_seq_depth(g_sequence_ids[7], 3);
		if (global[kBoatRaised])
			kernel_load_variant(2);
	}

	if (previous_room != KERNEL_RESTORING_GAME)
		local._handingLine = false;

	if (global[kBoatRaised]) {
		g_sprite_ids[2] = kernel_load_series(kernel_name('b', 0), 0);
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 1);
		kernel_seq_depth(g_sequence_ids[2], 3);
		kernel_flip_hotspot(words_boat, false);
		int idx = kernel_add_dynamic(words_boat, words_walkto, 0, g_sequence_ids[2], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 75, 124, FACING_NORTH);
		kernel_flip_hotspot(words_rope, false);
	} else {
		g_sprite_ids[5] = kernel_load_series(kernel_name('b', 2), 0);
		g_sprite_ids[6] = kernel_load_series(kernel_name('b', 3), 0);
		g_sprite_ids[3] = kernel_load_series(kernel_name('b', 1), 0);

		g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 1, 0, 0, 1);
		kernel_seq_depth(g_sequence_ids[3], 5);

		g_sequence_ids[5] = kernel_seq_stamp(g_sprite_ids[5], false, 1);
		kernel_seq_depth(g_sequence_ids[5], 4);

		g_sequence_ids[6] = kernel_seq_stamp(g_sprite_ids[6], false, 1);
		kernel_seq_depth(g_sequence_ids[6], 5);

		kernel_flip_hotspot(words_rope, true);
		kernel_flip_hotspot(words_boat, true);
		kernel_load_variant(1);
	}

	local._lineFrame = -1;
	local._lineMoving = false;

	g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, -2);
	kernel_seq_depth(g_sequence_ids[1], 1);

	if (previous_room == 512) {
		player.x = 60;
		player.y = 112;
		player.facing = FACING_SOUTHEAST;
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 55;
		player.y = 152;
		player.facing = FACING_NORTHWEST;
		player.walker_visible = false;
		player.commands_allowed = false;
		kernel_seq_delete(g_sequence_ids[1]);
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, -1);
		kernel_seq_depth(g_sequence_ids[1], 1);
		kernel_run_animation(kernel_name('R', 1), 70);
	} else if (local._handingLine) {
		player.walker_visible = false;
		local._lineAnimationMode = 1;
		local._lineAnimationPosition = 1;
		kernel_run_animation(kernel_name('R', -1), 0);
		local._lineFrame = 2;
	}
	section_5_music();
}

static void room_511_daemon() {
	if ((local._lineAnimationMode == 1) && (kernel_anim[0].anim != nullptr)) {
		if (local._lineFrame != kernel_anim[0].frame) {
			local._lineFrame = kernel_anim[0].frame;
			int resetFrame = -1;

			if ((local._lineAnimationPosition == 2) && (local._lineFrame == 14))
				local._lineMoving = false;

			if (local._lineAnimationPosition == 1) {
				if (local._lineFrame == 3) {
					local._lineMoving = false;
					resetFrame = 2;
				}

				if (local._handingLine)
					resetFrame = 2;
			}

			if ((resetFrame >= 0) && (resetFrame != kernel_anim[0].frame)) {
				kernel_reset_animation(0, resetFrame);
				local._lineFrame = resetFrame;
			}
		}
	}

	switch (kernel.trigger) {
	case 70:
		player.walker_visible = true;
		player.clock = kernel_anim[0].next_clock - player.frame_delay;
		kernel_timing_trigger(6, 71);
		break;

	case 71:
		kernel_seq_delete(g_sequence_ids[1]);
		g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 6, 0, 0, 1);
		kernel_seq_depth(g_sequence_ids[1], 1);
		kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 72);
		break;

	case 72:
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, -2);
		kernel_seq_depth(g_sequence_ids[1], 1);
		player.commands_allowed = true;
		break;

	default:
		break;
	}
}

static void room_511_pre_parser() {
	if (!local._handingLine)
		return;

	if (player_said_1(look) || player_said_1(fishing_line) || player_said_1(talkto))
		player.need_to_walk = false;

	if ((!player_said_3(tie, fishing_line, boat) || !player_said_3(attach, fishing_line, boat)) && player.need_to_walk) {
		if (kernel.trigger == 0) {
			player.ready_to_walk = false;
			player.commands_allowed = false;
			kernel_abort_animation(0);
			local._lineAnimationMode = 2;
			kernel_run_animation(kernel_name('R', 2), 1);
		} else if (kernel.trigger == 1) {
			player.walker_visible = true;
			player.clock = kernel_anim[0].next_clock - player.frame_delay;
			inter_move_object(OBJ_FISHING_LINE, 1);
			local._handingLine = false;
			player.commands_allowed = true;
			player.ready_to_walk = true;
		}
	}
}

static void room_511_parser() {
	if (player_said_2(walk_into, restaurant))
		new_room = 512;
	else if (player_said_2(get_into, car)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			kernel_seq_delete(g_sequence_ids[1]);
			g_sequence_ids[1] = kernel_seq_backward(g_sprite_ids[1], false, 6, 0, 0, 1);
			kernel_seq_depth(g_sequence_ids[1], 1);
			kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
		{
			int syncIdx = g_sequence_ids[1];
			g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, -1);
			kernel_seq_depth(g_sequence_ids[1], 1);
			kernel_seq_timeout(syncIdx, g_sequence_ids[1]);
			kernel_timing_trigger(6, 2);
		}
		break;

		case 2:
			player.walker_visible = false;
			g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 8, 0, 0, 1);
			kernel_seq_player(g_sequence_ids[4], false);
			kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 3);
			break;

		case 3:
		{
			int syncIdx = g_sequence_ids[4];
			g_sequence_ids[4] = kernel_seq_stamp(g_sprite_ids[4], false, -2);
			kernel_seq_player(g_sequence_ids[4], false);
			kernel_seq_timeout(syncIdx, g_sequence_ids[4]);
			new_room = 504;
		}
		break;

		default:
			break;
		}
	} else 	if (player_said_2(take, fishing_line)) {
		if (!global[kBoatRaised]) {
			if (global[kLineStatus] == 2) {
				if (kernel.trigger == 0) {
					player.commands_allowed = false;
					player.walker_visible = false;
					player_set_image();
					local._lineAnimationMode = 1;
					local._lineAnimationPosition = 1;
					local._lineMoving = true;
					kernel_run_animation(kernel_name('R', -1), 0);
					kernel_timing_trigger(1, 1);
				} else if (kernel.trigger == 1) {
					if (local._lineMoving) {
						kernel_timing_trigger(1, 1);
					} else {
						inter_give_to_player(OBJ_FISHING_LINE);
						local._lineMoving = true;
						local._handingLine = true;
						player.commands_allowed = true;
					}
				}
			} else {
				return;
			}
		} else {
			text_show(51130);
		}
	} else if (player_said_3(tie, fishing_line, boat) ||
		player_said_3(attach, fishing_line, boat)) {
		if (global[kBoatRaised])
			text_show(51131);
		else if (global[kLineStatus] == 1)
			text_show(51130);
		else if (!global[kBoatRaised] && local._handingLine) {
			if (global[kLineStatus] != 3) {
				if (kernel.trigger == 0) {
					player.commands_allowed = false;
					kernel_seq_delete(g_sequence_ids[7]);
					local._lineMoving = true;
					local._lineAnimationPosition = 2;
					kernel_timing_trigger(1, 1);
				} else if (kernel.trigger == 1) {
					if (local._lineMoving)
						kernel_timing_trigger(1, 1);
					else {
						player.walker_visible = true;
						g_sequence_ids[7] = kernel_seq_stamp(g_sprite_ids[7], false, -2);
						kernel_seq_depth(g_sequence_ids[7], 4);
						int idx = kernel_add_dynamic(words_fishing_line, words_walkto, 0, g_sequence_ids[7], 0, 0, 0, 0);
						kernel_dynamic_walk(idx, 26, 153, FACING_NORTHEAST);
						inter_take_from_player(OBJ_FISHING_LINE, 1);
						local._handingLine = false;
						local._lineMoving = true;
						global[kLineStatus] = 3;
						player.commands_allowed = true;
					}
				}
			}
		}
	} else if (player_said_2(look, street) || player.look_around) {
		if (global[kLineStatus] == 2)
			text_show(51110);
		else {
			if (global[kLineStatus] == 3)
				text_show(51111);
			else
				text_show(51112);
		}
	} else if (player_said_2(look, car))
		text_show(51113);
	else if (player_said_2(look, sidewalk))
		text_show(51114);
	else if (player_said_2(walk_down, sidewalk_to_east) || player_said_2(walk_down, sidewalk_to_west) || player_said_2(walk_down, street_to_east))
		text_show(51115);
	else if (player_said_2(look, pleasure_dome))
		text_show(51116);
	else if (player_said_2(look, ticket_booth))
		text_show(51117);
	else if (player_said_2(look, dome_entrance))
		text_show(51118);
	else if (player_said_3(unlock, padlock_key, dome_entrance) || player_said_3(unlock, door_key, dome_entrance))
		text_show(51119);
	else if ((player_said_1(put) || player_said_1(throw))
		&& (player_said_1(timebomb) || player_said_1(bomb) || player_said_1(bombs))
		&& player_said_1(dome_entrance))
		text_show(51120);
	else if (player_said_2(look, restaurant)) {
		if (global[kBoatRaised])
			text_show(51121);
		else
			text_show(51128);
	} else if (player_said_2(look, porthole))
		text_show(51122);
	else if (player_said_2(look, fishing_line) && (player.main_object_source == STROKE_INTERFACE) && (global[kLineStatus] == 2))
		text_show(51126);
	else if (player_said_2(look, fishing_line) && (player.main_object_source == STROKE_INTERFACE) && (global[kLineStatus] == 3))
		text_show(51133);
	else if (player_said_2(look, statue))
		text_show(51127);
	else if (player_said_2(look, boat))
		if (global[kBoatRaised])
			text_show(51123);
		else if (global[kLineStatus] != 3)
			text_show(51124);
		else
			text_show(51125);
	else if (player_said_2(look, fishing_line) && (global[kLineStatus] == 3))
		text_show(51125);
	else
		return;

	player.command_ready = false;
}

void room_511_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._handingLine);
	s.syncAsByte(local._lineMoving);

	s.syncAsSint16LE(local._lineAnimationMode);
	s.syncAsSint16LE(local._lineFrame);
	s.syncAsSint16LE(local._lineAnimationPosition);
}

void room_511_preload() {
	room_init_code_pointer = room_511_init;
	room_daemon_code_pointer = room_511_daemon;
	room_pre_parser_code_pointer = room_511_pre_parser;
	room_parser_code_pointer = room_511_parser;

	section_5_walker();
	section_5_interface();
	vocab_make_active(words_boat);
	vocab_make_active(words_fishing_line);
	vocab_make_active(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
