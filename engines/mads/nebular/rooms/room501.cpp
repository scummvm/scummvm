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
	int32 _mainSequenceId;
	int32 _mainSpriteId;
	int32 _doorHotspotid;
	bool _rexPunched;
};

static Scratch local;

static void handleSlotActions() {
	switch (kernel.trigger) {
	case 0:
		player.commands_allowed = false;
		player.walker_visible = false;
		int numTicks, frameIndex;
		if (global[kSexOfRex] == REX_MALE) {
			local._mainSpriteId = g_sprite_ids[4];
			numTicks = 8;
			frameIndex = 3;
		} else {
			local._mainSpriteId = g_sprite_ids[5];
			numTicks = 10;
			frameIndex = 2;
		}

		local._mainSequenceId = kernel_seq_pingpong(local._mainSpriteId, false, numTicks, 0, 0, 1);
		kernel_seq_range(local._mainSequenceId, 1, frameIndex);
		kernel_seq_player(local._mainSequenceId, false);
		g_engine->_soundManager->command(10, 0);
		kernel_seq_trigger(local._mainSequenceId, KERNEL_TRIGGER_SPRITE, frameIndex, 1);
		kernel_seq_trigger(local._mainSequenceId, KERNEL_TRIGGER_EXPIRE, 0, 2);
		break;

	case 1:
		g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 12, 0, 0, 6);
		break;

	case 2:
		kernel_seq_timeout(local._mainSequenceId, -1);
		player.walker_visible = true;
		kernel_timing_trigger(15, 3);
		break;

	case 3:
		player_walk(282, 110, FACING_NORTH);
		kernel_timing_trigger(60, 4);
		break;

	default:
		break;
	}
}

static void room_501_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('x', 1), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('c', 0), 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('x', 0), 0);

	if (global[kSexOfRex] == REX_MALE) {
		g_sprite_ids[4] = kernel_load_series("*RXMRC_9", 0);
		g_sprite_ids[6] = kernel_load_series("*RXCD_7", 0);
	} else {
		g_sprite_ids[5] = kernel_load_series("*ROXRC_9", 0);
		g_sprite_ids[7] = kernel_load_series("*ROXCD_7", 0);
	}

	g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 1);
	int idx = kernel_add_dynamic(words_door, words_walk_through, 0, g_sequence_ids[3], 0, 0, 0, 0);
	kernel_dynamic_walk(idx, 282, 110, FACING_NORTH);
	local._doorHotspotid = idx;
	kernel_dynamic_cursor(local._doorHotspotid, CURSOR_UP);
	kernel_seq_depth(g_sequence_ids[3], 7);
	g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, -1);
	kernel_seq_depth(g_sequence_ids[2], 4);
	local._rexPunched = true;

	if (previous_room == 504) {
		player.commands_allowed = false;
		player.x = 74;
		player.y = 121;
		player.facing = FACING_NORTHWEST;
		player.walker_visible = false;
		player.commands_allowed = false;
		kernel_seq_delete(g_sequence_ids[2]);
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, -2);
		kernel_seq_depth(g_sequence_ids[2], 4);
		if (global[kSexOfRex] == REX_MALE)
			kernel_run_animation(kernel_name('G', 2), 70);
		else
			kernel_run_animation(kernel_name('R', 2), 70);
	} else if (previous_room == 503) {
		player.x = 317;
		player.y = 102;
		player.facing = FACING_SOUTHWEST;
		kernel_timing_trigger(15, 80);
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 299;
		player.y = 131;
	}

	if (kernel.teleported_in) {
		inter_give_to_player(OBJ_FAKE_ID);
		inter_give_to_player(OBJ_SECURITY_CARD);
		inter_give_to_player(OBJ_ID_CARD);
	}

	section_5_music();
	kernel.quotes = quote_load(629, 630, 631, 0);

	if (!player.been_here_before)
		kernel_timing_trigger(2, 90);
}

static void room_501_daemon() {
	if (kernel.trigger == 90)
		text_show(50127);

	if (kernel.trigger >= 80) {
		switch (kernel.trigger) {
		case 80:
			player.commands_allowed = false;
			kernel_seq_delete(g_sequence_ids[3]);
			g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 12, 0, 0, 6);
			g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 9, 0, 0, 1);
			kernel_seq_depth(g_sequence_ids[3], 7);
			g_engine->_soundManager->command(11, 0);
			kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 81);
			break;

		case 81:
			kernel_delete_dynamic(local._doorHotspotid);
			player_walk(276, 110, FACING_SOUTHWEST);
			kernel_timing_trigger(120, 82);
			break;

		case 82:
			g_sequence_ids[3] = kernel_seq_backward(g_sprite_ids[3], false, 9, 0, 0, 1);
			kernel_seq_depth(g_sequence_ids[3], 7);
			g_engine->_soundManager->command(12, 0);
			local._doorHotspotid = kernel_add_dynamic(words_door, words_walk_through, 0, g_sequence_ids[3], 0, 0, 0, 0);
			kernel_dynamic_walk(g_sequence_ids[3], 282, 110, FACING_NORTH);
			kernel_dynamic_cursor(local._doorHotspotid, CURSOR_UP);
			kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 83);
			break;

		case 83:
			player.commands_allowed = true;
			g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, -1);
			kernel_seq_depth(g_sequence_ids[3], 7);
			break;

		default:
			break;
		}
	}

	if (kernel.trigger >= 70 && kernel.trigger <= 73) {
		switch (kernel.trigger) {
		case 70:
			player.walker_visible = true;
			player.clock = kernel.clock - player.frame_delay;
			kernel_timing_trigger(15, 71);
			break;

		case 71:
			player_walk(92, 130, FACING_SOUTH);
			kernel_timing_trigger(30, 72);
			break;

		case 72:
			kernel_seq_delete(g_sequence_ids[2]);
			g_sequence_ids[2] = kernel_seq_backward(g_sprite_ids[2], false, 6, 0, 0, 1);
			kernel_seq_depth(g_sequence_ids[2], 4);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 73);
			break;

		case 73:
			player.commands_allowed = true;
			g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, -1);
			kernel_seq_depth(g_sequence_ids[2], 4);
			break;

		default:
			break;
		}
	}
}

static void room_501_pre_parser() {
	if (player_said_1(walk_down) && (player_said_1(street_to_east) || player_said_1(sidewalk_to_east)))
		player.walk_off_edge_to_room = 551;
}

static void room_501_parser() {
	if (player_said_2(get_into, car)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			kernel_seq_delete(g_sequence_ids[2]);
			g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 6, 0, 0, 1);
			kernel_seq_depth(g_sequence_ids[2], 4);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
		{
			int syncIdx = g_sequence_ids[2];
			g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, -2);
			kernel_seq_depth(g_sequence_ids[2], 4);
			kernel_seq_timeout(syncIdx, g_sequence_ids[2]);
			kernel_timing_trigger(15, 2);
		}
		break;

		case 2:
			player.walker_visible = false;
			if (global[kSexOfRex] == REX_MALE)
				local._mainSpriteId = g_sprite_ids[6];
			else
				local._mainSpriteId = g_sprite_ids[7];

			local._mainSequenceId = kernel_seq_forward(local._mainSpriteId, false, 8, 0, 0, 1);
			kernel_seq_player(local._mainSequenceId, false);
			kernel_seq_trigger(local._mainSequenceId, KERNEL_TRIGGER_EXPIRE, 0, 3);
			break;

		case 3:
		{
			int syncIdx = local._mainSequenceId;
			local._mainSequenceId = kernel_seq_stamp(local._mainSpriteId, false, -2);
			kernel_seq_player(local._mainSequenceId, false);
			kernel_seq_timeout(syncIdx, local._mainSequenceId);
			kernel_timing_trigger(30, 4);
		}
		break;

		case 4:
			new_room = 504;
			break;

		default:
			break;
		}
	} else if (player_said_3(put, security_card, card_slot))
		text_show(50113);
	else if (player_said_3(put, fake_id, card_slot)) {
		switch (kernel.trigger) {
		case 0:
		case 1:
		case 2:
		case 3:
			handleSlotActions();
			break;

		case 4:
			if (global[kSexOfRex] == REX_MALE) {
				player.walker_visible = false;
				g_engine->_soundManager->command(13, 0);
				kernel_run_animation(kernel_name('G', 1), 5);
			} else {
				local._rexPunched = false;
				kernel_message_purge();
				kernel_message_add(quote_string(kernel.quotes, 0x277), 0, 0, 0x1110, 120, 6, 34);
			}
			break;

		case 5:
			player.walker_visible = true;
			player.clock = kernel.clock - player.frame_delay;
			kernel_timing_trigger(30, 6);
			break;

		case 6:
			if (global[kSexOfRex] == REX_MALE) {
				if (local._rexPunched) {
					kernel_message_purge();
					kernel_message_add(quote_string(kernel.quotes, 0x275), 0, 0, 0x1110, 120, 0, 34);
					local._rexPunched = false;
				} else {
					kernel_message_purge();
					kernel_message_add(quote_string(kernel.quotes, 0x276), 0, 0, 0x1110, 120, 0, 34);
				}
			}
			player.commands_allowed = true;
			break;

		default:
			break;
		}
	} else if (player_said_3(put, id_card, card_slot)) {
		switch (kernel.trigger) {
		case 0:
		case 1:
		case 2:
		case 3:
			handleSlotActions();
			break;

		case 4:
			kernel_seq_delete(g_sequence_ids[3]);
			kernel_delete_dynamic(local._doorHotspotid);
			g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 9, 0, 0, 1);
			kernel_seq_depth(g_sequence_ids[3], 7);
			g_engine->_soundManager->command(11, 0);
			kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 6);
			break;

		case 6:
			player_walk(317, 102, FACING_NORTHEAST);
			kernel_timing_trigger(120, 7);
			break;

		case 7:
		{
			g_engine->_soundManager->command(12, 0);
			int syncIdx = g_sequence_ids[3];
			g_sequence_ids[3] = kernel_seq_backward(g_sprite_ids[3], false, 9, 0, 0, 1);
			kernel_seq_depth(g_sequence_ids[3], 7);
			kernel_seq_timeout(syncIdx, g_sequence_ids[3]);
			g_engine->_soundManager->command(12, 0);
			kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 8);
		}
		break;

		case 8:
		{
			int syncIdx = g_sequence_ids[3];
			g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 1);
			kernel_seq_depth(g_sequence_ids[3], 7);
			kernel_seq_timeout(syncIdx, g_sequence_ids[3]);
			new_room = 503;
		}
		break;

		default:
			break;
		}
	} else if (player.look_around || player_said_2(look, street))
		text_show(50121);
	else if (player_said_2(look, door))
		text_show(50110);
	else if (player_said_2(look, card_slot))
		text_show(50112);
	else if (player_said_2(look, sign))
		text_show(50114);
	else if (player_said_2(take, sign))
		text_show(50115);
	else if (player_said_2(look, sidewalk_to_east))
		text_show(50118);
	else if (player_said_2(look, sidewalk_to_west) || player_said_2(look, street_to_west)
		|| player_said_2(walk_down, sidewalk_to_west) || player_said_2(walk_down, street_to_west))
		text_show(50119);
	else if (player_said_2(look, building))
		text_show(50120);
	else if (player_said_2(open, door))
		text_show(50122);
	else if (player_said_2(look, fire_hydrant))
		text_show(50123);
	else if (player_said_2(open, fire_hydrant))
		text_show(50124);
	else if (player_said_2(look, equipment_overhead))
		text_show(50125);
	else if (player_said_2(look, pipes) || player_said_2(look, pipe))
		text_show(50126);
	else if (player_said_2(look, car)) {
		if (!player_has_been_in_room(504))
			text_show(50116);
		else
			text_show(50117);
	} else
		return;

	player.command_ready = false;
}

void room_501_synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(local._mainSequenceId);
	s.syncAsSint16LE(local._mainSpriteId);
	s.syncAsSint16LE(local._doorHotspotid);
	s.syncAsByte(local._rexPunched);
}

void room_501_preload() {
	room_init_code_pointer = room_501_init;
	room_daemon_code_pointer = room_501_daemon;
	room_pre_parser_code_pointer = room_501_pre_parser;
	room_parser_code_pointer = room_501_parser;

	section_5_walker();
	section_5_interface();
	vocab_make_active(words_door);
	vocab_make_active(words_walk_through);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
