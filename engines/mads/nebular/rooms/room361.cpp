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

#include "mads/core/game.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section3.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

static void handleRexAction() {
	switch (kernel.trigger) {
	case 0:
		player.commands_allowed = false;
		player.walker_visible = false;
		kernel_seq_delete(g_sequence_ids[1]);
		g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 50, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[1], 3, -2);
		kernel_seq_loc(g_sequence_ids[1], 165, 76);
		kernel_seq_depth(g_sequence_ids[1], 15);

		g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 15, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[2], -1, 4);
		kernel_seq_player(g_sequence_ids[2], false);
		kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 1);
		kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 2);
		break;

	case 1:
	{
		int seqIdx = g_sequence_ids[1];
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 4);
		kernel_seq_loc(g_sequence_ids[1], 165, 76);
		kernel_seq_depth(g_sequence_ids[1], 15);
		kernel_seq_timeout(seqIdx, g_sequence_ids[1]);
	}
	break;

	case 2:
	{
		int seqIdx = g_sequence_ids[2];
		g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 12, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[2], 4, 10);
		kernel_seq_player(g_sequence_ids[2], false);
		kernel_seq_timeout(seqIdx, g_sequence_ids[2]);
		kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 3);
	}
	break;

	case 3:
	{
		kernel_seq_delete(g_sequence_ids[1]);
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 3);
		kernel_seq_loc(g_sequence_ids[1], 165, 76);
		kernel_seq_depth(g_sequence_ids[1], 1);

		int seqIdx = g_sequence_ids[2];
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 11);
		kernel_seq_player(g_sequence_ids[2], false);
		kernel_seq_loc(g_sequence_ids[2], 167, 100);
		kernel_seq_timeout(seqIdx, g_sequence_ids[2]);
		kernel_timing_trigger(15, 4);
	}
	break;

	case 4:
		kernel_seq_delete(g_sequence_ids[1]);
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 2);
		kernel_seq_loc(g_sequence_ids[1], 165, 76);
		kernel_seq_depth(g_sequence_ids[1], 1);

		kernel_player_expire(g_sequence_ids[2]);
		g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 12, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[2], 12, 14);
		kernel_seq_player(g_sequence_ids[2], false);
		kernel_seq_loc(g_sequence_ids[2], 167, 100);
		kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 5);
		break;

	case 5:
	{
		int seqIdx = g_sequence_ids[2];
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 15);
		kernel_seq_player(g_sequence_ids[2], false);
		kernel_seq_loc(g_sequence_ids[2], 167, 100);
		kernel_seq_timeout(seqIdx, g_sequence_ids[2]);
		kernel_timing_trigger(15, 6);
	}
	break;

	case 6:
		kernel_player_expire(g_sequence_ids[2]);
		kernel_seq_delete(g_sequence_ids[1]);
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 1);
		kernel_seq_loc(g_sequence_ids[1], 165, 76);
		kernel_seq_depth(g_sequence_ids[1], 1);
		kernel_timing_trigger(48, 7);
		break;

	case 7:
		new_room = 313;
		break;

	default:
		break;
	}
}

static void handleRoxAction() {
	switch (kernel.trigger) {
	case 0:
		player.commands_allowed = false;
		player.walker_visible = false;
		kernel_seq_delete(g_sequence_ids[1]);
		g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 18, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[1], 2, 4);
		kernel_seq_loc(g_sequence_ids[1], 165, 76);
		kernel_seq_depth(g_sequence_ids[1], 15);

		g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 18, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[4], -1, 3);
		kernel_seq_player(g_sequence_ids[4], false);
		kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 1);
		kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 2);
		break;

	case 1:
	{
		int tmpIdx = g_sequence_ids[1];
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 4);
		kernel_seq_loc(g_sequence_ids[1], 165, 76);
		kernel_seq_depth(g_sequence_ids[1], 15);
		kernel_seq_timeout(tmpIdx, g_sequence_ids[1]);
	}
	break;

	case 2:
	{
		int tmpIdx = g_sequence_ids[4];
		g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 12, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[4], 4, 8);
		kernel_seq_player(g_sequence_ids[4], false);
		kernel_seq_timeout(tmpIdx, g_sequence_ids[4]);
		kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 3);
	}
	break;

	case 3:
	{
		kernel_seq_delete(g_sequence_ids[1]);
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 3);
		kernel_seq_loc(g_sequence_ids[1], 165, 76);
		kernel_seq_depth(g_sequence_ids[1], 1);

		int tmpIdx = g_sequence_ids[4];
		g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 12, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[4], 9, 10);
		kernel_seq_player(g_sequence_ids[4], false);
		kernel_seq_loc(g_sequence_ids[4], 167, 100);
		kernel_seq_timeout(tmpIdx, g_sequence_ids[4]);
		kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 4);
	}
	break;

	case 4:
	{
		kernel_seq_delete(g_sequence_ids[1]);
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 2);
		kernel_seq_loc(g_sequence_ids[1], 165, 76);
		kernel_seq_depth(g_sequence_ids[1], 1);

		int tmpIdx = g_sequence_ids[4];
		g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 12, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[4], 11, 15);
		kernel_seq_player(g_sequence_ids[4], false);
		kernel_seq_loc(g_sequence_ids[4], 167, 100);
		kernel_seq_timeout(tmpIdx, g_sequence_ids[4]);
		kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 5);
	}
	break;

	case 5:
	{
		int tmpIdx = g_sequence_ids[4];
		g_sequence_ids[4] = kernel_seq_stamp(g_sprite_ids[4], false, 16);
		kernel_seq_player(g_sequence_ids[4], false);
		kernel_seq_loc(g_sequence_ids[4], 167, 100);
		kernel_seq_timeout(tmpIdx, g_sequence_ids[4]);
		kernel_timing_trigger(48, 6);
	}
	break;

	case 6:
		kernel_player_expire(g_sequence_ids[4]);

		kernel_seq_delete(g_sequence_ids[1]);
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 1);
		kernel_seq_loc(g_sequence_ids[1], 165, 76);
		kernel_seq_depth(g_sequence_ids[1], 1);
		kernel_timing_trigger(48, 7);
		break;

	case 7:
		new_room = 313;
		break;

	default:
		break;
	}
}

static void room_361_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_full_name(307, 'X', 0, "", KERNEL_SS), 0);

	if (global[kSexOfRex] == REX_MALE) {
		g_sprite_ids[2] = kernel_load_series("*RXCL_8", 0);
		g_sprite_ids[3] = kernel_load_series("*RXCL_2", 0);
	} else
		g_sprite_ids[4] = kernel_load_series("*ROXCL_8", 0);

	g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 1);
	kernel_seq_loc(g_sequence_ids[1], 165, 76);
	kernel_seq_depth(g_sequence_ids[1], 15);

	if (previous_room == 391) {
		global[kSexOfRex] = REX_MALE;
		player.commands_allowed = false;
		player.walker_visible = false;
		player.facing = FACING_SOUTH;
		player.x = 166;
		player.y = 101;
		kernel_timing_trigger(120, 70);
	} else if (previous_room == 360) {
		player.x = 302;
		player.y = 145;
	}
	else if (previous_room == 320) {
		player.x = 129;
		player.y = 113;
		player.facing = FACING_SOUTH;
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 13;
		player.y = 145;
	}

	kernel.quotes = quote_load(251, 252, 0);

	if (previous_room == 320)
		kernel_message_teletype(kernel_message_player(0xFB, 0x78, 0), 4, true);

	section_3_music();
}

static void room_361_daemon() {
	if (kernel.trigger >= 70) {
		switch (kernel.trigger) {
		case 70:
			kernel_seq_delete(g_sequence_ids[1]);
			g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 2);
			kernel_seq_loc(g_sequence_ids[1], 165, 76);
			kernel_seq_depth(g_sequence_ids[1], 1);

			g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 1);
			kernel_seq_player(g_sequence_ids[3], false);
			kernel_timing_trigger(15, 71);
			break;

		case 71:
			kernel_player_expire(g_sequence_ids[3]);
			g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 2);
			kernel_seq_player(g_sequence_ids[3], false);
			kernel_timing_trigger(15, 72);
			break;

		case 72:
			kernel_seq_delete(g_sequence_ids[1]);
			g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 3);
			kernel_seq_loc(g_sequence_ids[1], 165, 76);
			kernel_seq_depth(g_sequence_ids[1], 1);

			kernel_player_expire(g_sequence_ids[3]);
			g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 3);
			kernel_seq_player(g_sequence_ids[3], false);
			kernel_timing_trigger(15, 73);
			break;

		case 73:
			kernel_player_expire(g_sequence_ids[3]);
			g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 12, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[3], 4, 5);
			kernel_seq_player(g_sequence_ids[3], false);
			kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 74);
			break;

		case 74:
		{
			int seqIdx = g_sequence_ids[3];
			g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 6);
			kernel_seq_player(g_sequence_ids[3], false);
			kernel_seq_timeout(seqIdx, g_sequence_ids[3]);
			kernel_timing_trigger(15, 75);
		}
		break;

		case 75:
			kernel_player_expire(g_sequence_ids[3]);
			g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 7);
			kernel_seq_player(g_sequence_ids[3], false);
			kernel_timing_trigger(15, 76);
			break;

		case 76:
			kernel_seq_delete(g_sequence_ids[1]);
			g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 2);
			kernel_seq_loc(g_sequence_ids[1], 165, 76);
			kernel_seq_depth(g_sequence_ids[1], 1);

			kernel_player_expire(g_sequence_ids[3]);
			g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 8);
			kernel_seq_player(g_sequence_ids[3], false);
			kernel_timing_trigger(15, 77);
			break;

		case 77:
			kernel_seq_delete(g_sequence_ids[1]);
			g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 1);
			kernel_seq_loc(g_sequence_ids[1], 165, 76);
			kernel_seq_depth(g_sequence_ids[1], 15);

			kernel_player_expire(g_sequence_ids[3]);
			g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 9);
			kernel_seq_player(g_sequence_ids[3], false);
			kernel_timing_trigger(15, 78);
			break;

		case 78:
			kernel_player_expire(g_sequence_ids[3]);
			g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 12, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[3], 10, -2);
			kernel_seq_player(g_sequence_ids[3], false);
			kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 79);
			break;

		case 79:
			kernel_seq_timeout(g_sequence_ids[3], -1);
			player.commands_allowed = true;
			player.walker_visible = true;
			break;

		default:
			break;
		}
	}
}

static void room_361_pre_parser() {
	if (player_said_2(walk_down, corridor_to_east))
		player.walk_off_edge_to_room = 360;

	if (player_said_2(walk_down, corridor_to_west))
		player.walk_off_edge_to_room = 354;
}

static void room_361_parser() {
	if (player.look_around)
		text_show(36119);
	else if (player_said_2(sit_at, desk)) {
		kernel_message_purge();
		kernel_message_player(252, 120, 0);
	} else if (player_said_2(climb_into, air_vent)) {
		if (global[kSexOfRex] == REX_FEMALE)
			handleRoxAction();
		else
			handleRexAction();
	} else if (player_said_2(look, desk))
		text_show(36110);
	else if (player_said_2(look, wall))
		text_show(36111);
	else if (player_said_2(look, lighting_fixture) || player_said_2(stare_at, lighting_fixture))
		text_show(36112);
	else if (player_said_2(look, lights) || player_said_2(stare_at, lights))
		text_show(36113);
	else if (player_said_2(take, lights))
		text_show(36114);
	else if (player_said_2(look, light_bulb) || player_said_2(stare_at, light_bulb))
		text_show(36115);
	else if (player_said_2(take, light_bulb))
		text_show(36116);
	else if (player_said_2(look, corridor_to_west))
		text_show(36117);
	else if (player_said_2(look, corridor_to_east))
		text_show(36118);
	else if (player_said_2(look, air_vent))
		text_show(36120);
	else
		return;

	player.command_ready = false;
}

void room_361_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_361_preload() {
	room_init_code_pointer = room_361_init;
	room_pre_parser_code_pointer = room_361_pre_parser;
	room_parser_code_pointer = room_361_parser;
	room_daemon_code_pointer = room_361_daemon;

	if (room_id == 391)
		global[kSexOfRex] = REX_MALE;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
