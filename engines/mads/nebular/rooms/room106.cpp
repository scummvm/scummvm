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
#include "mads/core/matte.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section1.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _backToShipFl = false;
	bool _shadowFl = false;
	bool _firstEmergingFl = false;
	long _positionY = 0;
};

static Scratch local;


static void room_106_init() {
	g_sprite_ids[0] = kernel_load_series(kernel_name('H', -1), 0);

	if (player_has(OBJ_REBREATHER) || (previous_room != 102) || kernel.teleported_in) {
		g_sprite_ids[1] = kernel_load_series(kernel_name('A', 0), 0);
		g_sprite_ids[3] = kernel_load_series(kernel_name('A', 1), 0);
	}

	g_sprite_ids[2] = kernel_load_series(kernel_name('G', -1), 0);
	g_sequence_ids[2] = kernel_seq_pingpong(g_sprite_ids[2], false, 21, 0, 0, 0);
	g_sprite_ids[4] = kernel_load_series(kernel_name('I', -1), 0);
	g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 6, 47, 32, 0);

	if (previous_room == 102) {
		g_sequence_ids[0] = kernel_seq_forward(g_sprite_ids[0], false, 6, 0, 4, 1);
		kernel_seq_trigger(g_sequence_ids[0], KERNEL_TRIGGER_EXPIRE, 0, 70);
		player.walker_visible = false;
		player.commands_allowed = false;
		player.facing = FACING_EAST;
		player.x = 106;
		player.y = 69;
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		if (previous_room == 107) {
			player.x = 319;
			player.y = 84;
			player.facing = player.prepare_walk_facing = FACING_WEST;
		} else {
			player.x = 319;
			player.y = 44;
			player.facing = player.prepare_walk_facing = FACING_SOUTHWEST;
			series_list[player.series_base + 3]->walker->velocity = 24;
		}

		player.prepare_walk_x = 246;
		player.prepare_walk_y = 69;
		player.need_to_walk = true;
		player.ready_to_walk = true;
	}

	if (previous_room != 102) {
		g_sequence_ids[0] = kernel_seq_forward(g_sprite_ids[0], false, 6, 0, 0, 0);
		kernel_seq_range(g_sequence_ids[0], -2, -2);
		kernel_seq_depth(g_sequence_ids[0], 14);
	}

	local._backToShipFl = false;
	local._shadowFl = false;
	local._firstEmergingFl = false;

	kernel.quotes = quote_load(49, 50, 52, 77, 78, 79, 80, 81, 0);
	section_1_music();
}

static void room_106_daemon() {
	if (kernel.trigger != 0) warning("%d", kernel.trigger);
	if (kernel.trigger == 70) {
		g_sequence_ids[0] = kernel_seq_forward(g_sprite_ids[0], false, 6, 0, 0, 0);
		kernel_seq_range(g_sequence_ids[0], -2, -2);
		kernel_seq_depth(g_sequence_ids[0], 14);

		if (!player_has(OBJ_REBREATHER) && !kernel.teleported_in) {
			kernel_run_animation(kernel_full_name(106, 'A', -1, "", KERNEL_AA), 75);
		} else {
			g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 4, 0, 0, 1);
			kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_SPRITE, 28, 71);
		}
	}

	if (kernel.trigger == 71) {
		player.prepare_walk_x = 246;
		player.prepare_walk_y = 69;
		player.prepare_walk_facing = FACING_EAST;
		player.need_to_walk = true;
		player.ready_to_walk = true;
		player.walker_visible = true;

		if (player.been_here_before) {
			player.commands_allowed = true;
		} else {
			player.prepare_walk_facing = FACING_SOUTHWEST;
			local._firstEmergingFl = true;
			kernel_run_animation(kernel_full_name(106, 'B', -1, "", KERNEL_AA), 80);

			// FIXME: trigger 80 isn't happening for demo
			if (g_engine->isDemo())
				kernel_timing_trigger(100, 87);
		}
	}

	if (local._firstEmergingFl && (kernel_anim[0].frame >= 19)) {
		local._firstEmergingFl = false;
		kernel_message_add(quote_string(kernel.quotes, 52), 0, 0, 0x1110, 120, 0, 34);
	}

	if ((kernel.trigger >= 80) && (kernel.trigger <= 87)) {
		int tmpVal = kernel.trigger - 80;
		int msgId = -1;
		switch (tmpVal) {
		case 0:
			local._positionY = 26;
			msgId = 49;
			break;

		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			msgId = 76 + tmpVal;
			break;

		case 6:
			msgId = 50;
			break;

		default:
			msgId = -1;
			player.commands_allowed = true;
			break;
		}

		if (msgId >= 0) {
			int nextTrigger = kernel.trigger + 1;
			kernel_message_add(quote_string(kernel.quotes, msgId), 15, local._positionY, 0x1110, 360, 0, 0);
			kernel_timing_trigger(150, nextTrigger);
			local._positionY += 14;
		}
	}

	if (local._backToShipFl) {
		if (!local._shadowFl) {
			if (player.x < 204) {
				local._shadowFl = true;
				g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 4, 0, 0, 1);
				kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 72);
				kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_SPRITE, 44, 73);
			}
		} else if (kernel.trigger == 73)
			player.walker_visible = false;
		else if (kernel.trigger == 72)
			kernel_timing_trigger(24, 74);
		else if (kernel.trigger == 74)
			new_room = 102;
	}

	if (kernel.trigger == 75) {
		player_undiscover_room();
		new_room = 102;
	}
}

static void room_106_pre_parser() {
	if (player_said_2(swim_towards, sea_cliff) || player_said_2(swim_towards, seaweed_bank)) {
		if (g_engine->isDemo()) {
			text_show(99);
			player.command_ready = false;
		} else {
			player.commands_allowed = false;
			series_list[player.series_base + 1]->walker->velocity = 24;
			player.walk_off_edge_to_room = 104;
		}
	}

	if (player_said_2(swim_towards, open_area_to_east))
		player.walk_off_edge_to_room = 107;
}

static void room_106_parser() {
	if (player.look_around)
		text_show(10614);
	else if (player_said_2(swim_to, main_airlock)) {
		player.commands_allowed = false;
		player.prepare_walk_x = 95;
		player.prepare_walk_y = 72;
		player.prepare_walk_facing = FACING_WEST;
		player.need_to_walk = true;
		player.ready_to_walk = true;
		player.sprite = 9;
		local._backToShipFl = true;
	} else if (player_said_2(look, anemone) || player_said_2(look_at, anemone))
		text_show(10601);
	else if (player_said_2(take, anemone))
		text_show(10602);
	else if (player_said_2(look, seaweed) || player_said_2(look, seaweed_bank))
		text_show(10603);
	else if (player_said_2(take, seaweed) || player_said_2(take, seaweed_bank))
		text_show(10604);
	else if (player_said_2(look, open_area_to_east))
		text_show(10605);
	else if (player_said_2(look, pile_of_rocks) || player_said_2(look_at, pile_of_rocks))
		text_show(10606);
	else if (player_said_1(pile_of_rocks) && (player_said_1(push) || player_said_1(pull) || player_said_1(take)))
		text_show(10607);
	else if (player_said_2(look, ship) || player_said_2(look_at, ship))
		text_show(10608);
	else if (player_said_2(look, main_airlock))
		text_show(10609);
	else if (player_said_2(open, main_airlock))
		text_show(10610);
	else if (player_said_2(close, main_airlock))
		text_show(10611);
	else if (player_said_2(look, sea_cliff))
		text_show(10612);
	else if (player_said_2(look, ocean_floor))
		text_show(10613);
	else
		return;

	player.command_ready = false;
}

void room_106_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._backToShipFl);
	s.syncAsByte(local._shadowFl);
	s.syncAsByte(local._firstEmergingFl);
	s.syncAsSint32LE(local._positionY);
}

void room_106_preload() {
	room_init_code_pointer = room_106_init;
	room_pre_parser_code_pointer = room_106_pre_parser;
	room_parser_code_pointer = room_106_parser;
	room_daemon_code_pointer = room_106_daemon;

	section_1_walker();
	section_1_interface();

	if ((previous_room == 102) && !player_has(OBJ_REBREATHER) && !kernel.teleported_in)
		*player.series_name = '\0';

	text_default_y = 100;

}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
