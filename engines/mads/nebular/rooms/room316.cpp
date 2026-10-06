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

#include "mads/core/config.h"
#include "mads/core/game.h"
#include "mads/nebular/global.h"
#include "mads/nebular/mac_nebular.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section3.h"
#include "mads/nebular/sound/mac_sound.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

static void handleRexInGrate() {
	switch (kernel.trigger) {
	case 0:
		player.commands_allowed = false;
		player.walker_visible = false;
		g_sequence_ids[4] = kernel_seq_stamp(g_sprite_ids[4], false, 1);
		kernel_seq_player(g_sequence_ids[4], false);
		kernel_timing_trigger(15, 1);
		break;

	case 1:
		kernel_player_expire(g_sequence_ids[4]);
		g_sequence_ids[4] = kernel_seq_pingpong(g_sprite_ids[4], false, 12, 0, 0, 3);
		kernel_seq_range(g_sequence_ids[4], 2, 3);
		kernel_seq_player(g_sequence_ids[4], false);
		kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 2);
		break;

	case 2:
	{
		int oldIdx = g_sequence_ids[4];
		g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 12, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[4], 4, 8);
		kernel_seq_player(g_sequence_ids[4], false);
		kernel_seq_timeout(oldIdx, g_sequence_ids[4]);

		kernel_seq_delete(g_sequence_ids[3]);
		g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 12, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[3], 2, -2);
		kernel_seq_depth(g_sequence_ids[3], 12);
		kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 3);
		kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 4);
	}
	break;

	case 3:
	{
		int oldIdx = g_sequence_ids[3];
		g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, -2);
		kernel_seq_depth(g_sequence_ids[3], 12);
		kernel_seq_timeout(oldIdx, g_sequence_ids[3]);
	}
	break;

	case 4:
	{
		int oldIdx = g_sequence_ids[4];
		g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 12, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[4], 10, 11);
		kernel_seq_player(g_sequence_ids[4], false);
		kernel_seq_timeout(oldIdx, g_sequence_ids[4]);

		kernel_seq_delete(g_sequence_ids[3]);
		g_sequence_ids[3] = kernel_seq_backward(g_sprite_ids[3], false, 12, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[3], 5, -2);
		kernel_seq_depth(g_sequence_ids[3], 3);
		kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 5);
	}
	break;

	case 5:
	{
		int oldIdx = g_sequence_ids[3];
		g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 3);
		kernel_seq_depth(g_sequence_ids[3], 3);
		kernel_seq_timeout(oldIdx, g_sequence_ids[3]);

		oldIdx = g_sequence_ids[4];
		g_sequence_ids[4] = kernel_seq_stamp(g_sprite_ids[4], false, 12);
		kernel_seq_player(g_sequence_ids[4], false);
		kernel_seq_timeout(oldIdx, g_sequence_ids[4]);
		kernel_timing_trigger(15, 6);
	}
	break;

	case 6:
		kernel_player_expire(g_sequence_ids[4]);
		g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 12, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[4], 13, 14);
		kernel_seq_player(g_sequence_ids[4], false);

		kernel_seq_delete(g_sequence_ids[3]);
		g_sequence_ids[3] = kernel_seq_backward(g_sprite_ids[3], false, 12, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[3], 2, 3);
		kernel_seq_depth(g_sequence_ids[3], 3);
		kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 7);
		kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 8);
		break;

	case 7:
		kernel_player_expire(g_sequence_ids[4]);
		g_sequence_ids[4] = kernel_seq_stamp(g_sprite_ids[4], false, 15);
		kernel_seq_player(g_sequence_ids[4], false);
		kernel_seq_timeout(-1, g_sequence_ids[4]);
		break;

	case 8:
	{
		int oldIdx = g_sequence_ids[3];
		g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 2);
		kernel_seq_depth(g_sequence_ids[3], 3);
		kernel_seq_timeout(oldIdx, g_sequence_ids[3]);
		kernel_timing_trigger(15, 9);
	}
	break;

	case 9:
		kernel_player_expire(g_sequence_ids[4]);
		g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 1);
		kernel_seq_depth(g_sequence_ids[3], 3);
		kernel_timing_trigger(48, 10);
		break;

	case 10:
		new_room = 313;
		break;

	default:
		break;
	}
}

static void handleRoxInGrate() {
	int temp;
	int temp1;

	switch (kernel.trigger) {
	case 0:
		player.commands_allowed = false;
		player.walker_visible = false;
		g_sequence_ids[5] = kernel_seq_stamp(g_sprite_ids[5], false, 1);
		kernel_seq_player(g_sequence_ids[5], false);
		kernel_timing_trigger(15, 1);
		break;

	case 1:
		kernel_player_expire(g_sequence_ids[5]);
		g_sequence_ids[5] = kernel_seq_pingpong(g_sprite_ids[5], false, 17, 0, 0, 3);
		kernel_seq_range(g_sequence_ids[5], 2, 3);
		kernel_seq_player(g_sequence_ids[5], false);
		kernel_seq_trigger(g_sequence_ids[5], KERNEL_TRIGGER_EXPIRE, 0, 2);
		break;

	case 2:
		kernel_seq_delete(g_sequence_ids[3]);
		g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 17, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[3], 2, -2);
		kernel_seq_depth(g_sequence_ids[3], 12);

		temp = g_sequence_ids[5];
		g_sequence_ids[5] = kernel_seq_forward(g_sprite_ids[5], false, 17, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[5], 4, 8);
		kernel_seq_player(g_sequence_ids[5], false);
		kernel_seq_timeout(temp, g_sequence_ids[5]);
		kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 3);
		kernel_seq_trigger(g_sequence_ids[5], KERNEL_TRIGGER_EXPIRE, 0, 4);
		break;

	case 3:
		temp1 = g_sequence_ids[3];
		g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, -2);
		kernel_seq_depth(g_sequence_ids[3], 12);
		kernel_seq_timeout(temp1, g_sequence_ids[3]);
		break;

	case 4:
		temp = g_sequence_ids[5];
		g_sequence_ids[5] = kernel_seq_forward(g_sprite_ids[5], false, 17, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[5], 10, 11);
		kernel_seq_player(g_sequence_ids[5], false);
		kernel_seq_timeout(temp, g_sequence_ids[5]);

		kernel_seq_delete(g_sequence_ids[3]);
		g_sequence_ids[3] = kernel_seq_backward(g_sprite_ids[3], false, 17, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[3], 5, -2);
		kernel_seq_depth(g_sequence_ids[3], 3);
		kernel_seq_trigger(g_sequence_ids[5], KERNEL_TRIGGER_EXPIRE, 0, 5);
		break;

	case 5:
		temp = g_sequence_ids[3];
		g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 3);
		kernel_seq_depth(g_sequence_ids[3], 3);
		kernel_seq_timeout(temp, g_sequence_ids[3]);

		temp = g_sequence_ids[5];
		g_sequence_ids[5] = kernel_seq_stamp(g_sprite_ids[5], false, 12);
		kernel_seq_player(g_sequence_ids[5], false);
		kernel_seq_timeout(temp, g_sequence_ids[5]);
		kernel_timing_trigger(20, 6);
		break;

	case 6:
		kernel_player_expire(g_sequence_ids[5]);
		g_sequence_ids[5] = kernel_seq_forward(g_sprite_ids[5], false, 17, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[5], 13, 15);
		kernel_seq_player(g_sequence_ids[5], false);

		kernel_seq_delete(g_sequence_ids[3]);
		g_sequence_ids[3] = kernel_seq_backward(g_sprite_ids[3], false, 17, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[3], 2, 3);
		kernel_seq_depth(g_sequence_ids[3], 3);
		kernel_seq_trigger(g_sequence_ids[5], KERNEL_TRIGGER_EXPIRE, 0, 7);
		kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 8);
		break;

	case 7:
		kernel_player_expire(g_sequence_ids[5]);
		g_sequence_ids[5] = kernel_seq_stamp(g_sprite_ids[5], false, 16);
		kernel_seq_player(g_sequence_ids[5], false);
		kernel_seq_timeout(-1, g_sequence_ids[5]);
		break;

	case 8:
		temp = g_sequence_ids[3];
		g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 2);
		kernel_seq_depth(g_sequence_ids[3], 3);
		kernel_seq_timeout(temp, g_sequence_ids[3]);
		kernel_timing_trigger(20, 9);
		break;

	case 9:
		kernel_player_expire(g_sequence_ids[5]);
		g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 1);
		kernel_seq_depth(g_sequence_ids[3], 3);
		kernel_timing_trigger(48, 10);
		break;

	case 10:
		new_room = 313;
		break;

	default:
		break;
	}
}

static void room_316_init() {
	if (global[kSexOfRex] == REX_MALE) {
		g_sprite_ids[1] = kernel_load_series(kernel_name('g', -1), 0);
		g_sprite_ids[4] = kernel_load_series("*RXCL_8", 0);
		g_sprite_ids[6] = kernel_load_series("*RXCL_2", 0);
	} else {
		g_sprite_ids[2] = kernel_load_series(kernel_name('g', 0), 0);
		g_sprite_ids[5] = kernel_load_series("*ROXCL_8", 0);
	}

	g_sprite_ids[3] = kernel_load_series(kernel_name('v', 0), 0);
	g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 1);
	kernel_seq_depth(g_sequence_ids[3], 12);

	if (previous_room == 366) {
		player.commands_allowed = false;
		player.walker_visible = false;
		player.x = 78;
		player.y = 87;
		player.facing = FACING_SOUTH;
		kernel_timing_trigger(48, 70);
	} else if (previous_room == 321) {
		player.x = 153;
		player.y = 102;
		player.facing = FACING_SOUTH;
		player.commands_allowed = false;
		player.walker_visible = false;
		if (!Sound::commandMacintoshSound(Sound::kMacSoundPlay, 3044,
				0, 0, 0, true))
			g_engine->_soundManager->command(44, 0);
		int spriteIdx = (global[kSexOfRex] == REX_MALE) ? 1 : 2;
		g_sequence_ids[1] = kernel_seq_backward(g_sprite_ids[spriteIdx], false, 6, 0, 0, 1);
		kernel_seq_depth(g_sequence_ids[1], 2);
		kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 60);
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 291;
		player.y = 126;
	}

	setMacintoshMessageColors(63, 63, 0, 63, 63, 0);
	section_3_music();
	kernel.quotes = quote_load(253, 0);
}

static void room_316_daemon() {
	if (kernel.trigger == 60) {
		kernel_seq_timeout(g_sequence_ids[1], -1);
		player.walker_visible = true;
		player.commands_allowed = true;
	}

	if (kernel.trigger >= 70) {
		switch (kernel.trigger) {
		case 70:
			kernel_seq_delete(g_sequence_ids[3]);
			g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 12, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[3], 2, 6);
			kernel_seq_depth(g_sequence_ids[3], 3);

			g_sequence_ids[6] = kernel_seq_forward(g_sprite_ids[6], false, 12, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[6], 1, 5);
			kernel_seq_player(g_sequence_ids[6], false);
			kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 71);
			kernel_seq_trigger(g_sequence_ids[6], KERNEL_TRIGGER_EXPIRE, 0, 72);
			break;

		case 71:
		{
			int synxIdx = g_sequence_ids[3];
			g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, -2);
			kernel_seq_depth(g_sequence_ids[3], 3);
			kernel_seq_timeout(synxIdx, g_sequence_ids[3]);
		}
		break;

		case 72:
		{
			int synxIdx = g_sequence_ids[6];
			g_sequence_ids[6] = kernel_seq_forward(g_sprite_ids[6], false, 12, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[6], 6, 9);
			kernel_seq_player(g_sequence_ids[6], false);
			kernel_seq_timeout(synxIdx, g_sequence_ids[6]);
			kernel_seq_trigger(g_sequence_ids[6], KERNEL_TRIGGER_EXPIRE, 0, 73);
		}
		break;

		case 73:
			kernel_seq_delete(g_sequence_ids[3]);
			g_sequence_ids[3] = kernel_seq_backward(g_sprite_ids[3], false, 12, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[3], 1, 5);
			kernel_seq_depth(g_sequence_ids[3], 12);
			kernel_seq_player(g_sequence_ids[6], false);

			g_sequence_ids[6] = kernel_seq_forward(g_sprite_ids[6], false, 12, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[6], 10, -2);
			kernel_seq_player(g_sequence_ids[6], false);
			kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 74);
			kernel_seq_trigger(g_sequence_ids[6], KERNEL_TRIGGER_EXPIRE, 0, 75);
			break;

		case 74:
		{
			int synxIdx = g_sequence_ids[3];
			g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 1);
			kernel_seq_depth(g_sequence_ids[3], 12);
			kernel_seq_timeout(synxIdx, g_sequence_ids[3]);
		}
		break;

		case 75:
			kernel_seq_timeout(g_sequence_ids[6], -1);
			player.commands_allowed = true;
			player.walker_visible = true;
			break;

		default:
			break;
		}
	}
}

static void room_316_pre_parser() {
	if (player_said_2(walk_down, corridor_to_east)) {
		if (global[kAfterHavoc])
			player.walk_off_edge_to_room = 354;
		else
			player.walk_off_edge_to_room = 304;
	}
}

static void room_316_parser() {
	if (player_said_2(climb_into, air_vent)) {
		if (global[kSexOfRex] == REX_FEMALE)
			handleRoxInGrate();
		else
			handleRexInGrate();
	} else if (player_said_2(walk_up, ramp) || player_said_2(walk_onto, platform)) {
		switch (kernel.trigger) {
		case 0:
			if (global[kCityFlooded]) {
				text_show(31623);
			} else {
				g_engine->_soundManager->command(45, 0);
				player.commands_allowed = false;
				player.walker_visible = false;
				if (global[kSexOfRex] == REX_MALE) {
					g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 7, 0, 0, 1);
					kernel_seq_range(g_sequence_ids[1], -1, 7);
					kernel_seq_depth(g_sequence_ids[1], 2);
					kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 1);
				} else {
					g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 7, 0, 0, 1);
					kernel_seq_depth(g_sequence_ids[2], 2);
					kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 3);
				}
			}
			break;

		case 1:
			g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 7, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[1], 8, -2);
			kernel_seq_depth(g_sequence_ids[1], 2);
			kernel_message_purge();
			if (!player_has_been_in_room(321))
				kernel_message_add(quote_string(kernel.quotes, 253), 0, 0, 0x1110, 120, 0, 34);

			kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2:
			g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 7, 0, 0, 0);
			kernel_seq_range(g_sequence_ids[1], -2, -2);
			kernel_seq_depth(g_sequence_ids[1], 2);
			kernel_timing_trigger(48, 4);
			break;

		case 3:
			g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 7, 0, 0, 0);
			kernel_seq_range(g_sequence_ids[2], -2, -2);
			kernel_seq_depth(g_sequence_ids[2], 2);
			kernel_timing_trigger(48, 4);
			break;

		case 4:
			new_room = 321;
			break;

		default:
			break;
		}
	} else if (player_said_2(look, platform))
		text_show(31610);
	else if (player_said_2(look, strange_device)) {
		if (player_has_been_in_room(321))
			text_show(31612);
		else
			text_show(31611);
	} else if (player_said_2(look, controls))
		text_show(31613);
	else if (player_said_2(look, equipment))
		text_show(31614);
	else if (player_said_2(look, panel))
		text_show(31615);
	else if (player_said_2(look, monitor))
		text_show(31616);
	else if (player_said_2(look, ramp))
		text_show(31617);
	else if (player_said_2(look, air_vent))
		text_show(31618);
	else if (player_said_2(look, corridor_to_east)) {
		if (!global[kAfterHavoc]) {
			if (game.difficulty != DIFFICULTY_EASY)
				text_show(31620);
			else
				text_show(31619);
		}
	} else if (player_said_2(look, floor))
		text_show(31621);
	else if (player_said_2(look, support))
		text_show(31622);
	else
		return;

	setMacintoshMessageColors(63, 63, 0, 63, 63, 0);
	player.command_ready = false;
}

void room_316_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_316_preload() {
	room_init_code_pointer = room_316_init;
	room_pre_parser_code_pointer = room_316_pre_parser;
	room_parser_code_pointer = room_316_parser;
	room_daemon_code_pointer = room_316_daemon;

	if (room_id == 366)
		global[kSexOfRex] = REX_MALE;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
