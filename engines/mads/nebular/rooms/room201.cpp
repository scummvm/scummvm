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
#include "mads/core/pal.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"	
#include "mads/nebular/rooms/section2.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _pterodactylFlag;
};

static Scratch local;


static void room_201_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('x', 0), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('x', 1), 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('m', -1), 0);
	g_sprite_ids[4] = kernel_load_series(kernel_name('b', -1), 0);
	g_sprite_ids[5] = kernel_load_series("*SC002Z1", 0);
	g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 6, 0, 1, 0);
	g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 15, 50, 0, 0);
	g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 4, 0, 0, 0);
	g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 6, 0, 0, 0);
	kernel_seq_depth(g_sequence_ids[4], 8);
	kernel_seq_loc(g_sequence_ids[4], 185, 46);

	int idx = kernel_add_dynamic(words_birds, words_look_at, 0, g_sequence_ids[4], 0, 0, 0, 0);
	kernel_dynamic_walk(idx, 186, 81, FACING_NORTH);

	if ((previous_room == 202) || (previous_room == KERNEL_STARTING_GAME)) {
		player.x = 165;
		player.y = 152;
	} else {
		player.x = 223;
		player.y = 149;
		player.facing = FACING_SOUTH;
	}

	if (global[kTeleporterCommand]) {
		player.walker_visible = false;
		player.commands_allowed = false;
		int sepChar = (global[kSexOfRex] == SEX_MALE) ? 't' : 'u';
		// Guess values. What is the default value used by the compiler?
		int suffixNum = -1;
		int endTrigger = -1;
		switch (global[kTeleporterCommand]) {
		case 1:
			suffixNum = 3;
			endTrigger = 76;
			global[kTeleporterUnderstood] = true;
			break;
		case 2:
			suffixNum = 1;
			endTrigger = 77;
			break;
		case 3:
			player.walker_visible = true;
			player.commands_allowed = true;
			break;
		case 4:
			suffixNum = 2;
			endTrigger = 78;
			break;
		default:
			break;
		}
		global[kTeleporterCommand] = 0;
		if (suffixNum >= 0)
			kernel_run_animation(kernel_name(sepChar, suffixNum), endTrigger);
	}

	if ((previous_room == 202) && (global[kMeteorologistStatus] == METEOROLOGIST_PRESENT) && !kernel.teleported_in) {
		g_sprite_ids[6] = kernel_load_series(kernel_name('a', 0), 0);
		g_sprite_ids[7] = kernel_load_series(kernel_name('a', 1), 0);
		kernel.quotes = quote_load(90, 91, 0);
		player.commands_allowed = false;
		g_sequence_ids[6] = kernel_seq_forward(g_sprite_ids[6], false, 7, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[6], -1, 12);
		kernel_seq_trigger(g_sequence_ids[6], KERNEL_TRIGGER_SPRITE, 12, 70);
		kernel_seq_depth(g_sequence_ids[6], 1);
		local._pterodactylFlag = false;
		player_walk(157, 143, FACING_NORTH);
		pal_change_color(252, 45, 63, 45);
		pal_change_color(253, 20, 45, 20);
		kernel_message_add(quote_string(kernel.quotes, 90), 0, 0, 0x1110, 120, 0, 2);
	} else
		local._pterodactylFlag = true;

	if (global[kTeleporterUnderstood])
		kernel_flip_hotspot(words_strange_device, false);

	section_2_music();
}

static void room_201_daemon() {
	if (local._pterodactylFlag && (g_engine->getRandomNumber(5000) == 9)) {
		g_sequence_ids[5] = kernel_seq_forward(g_sprite_ids[5], false, 5, 0, 6, 1);
		int idx = kernel_add_dynamic(words_swooping_creature, words_walkto, 0, g_sequence_ids[5], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 270, 80, FACING_EAST);
		kernel_seq_depth(g_sequence_ids[5], 8);
		g_engine->_soundManager->command(14, 0);
		local._pterodactylFlag = false;
	}

	if (kernel.trigger == 70) {
		g_sequence_ids[6] = kernel_seq_forward(g_sprite_ids[6], false, 9, 0, 0, 1);
		player.walker_visible = false;
		kernel_seq_range(g_sequence_ids[6], 12, 16);
		g_sequence_ids[7] = kernel_seq_forward(g_sprite_ids[7], false, 9, 0, 0, 1);
		g_engine->_soundManager->command(42, 0);
		kernel_seq_depth(g_sequence_ids[6], 1);
		kernel_seq_depth(g_sequence_ids[7], 1);
		kernel_seq_trigger(g_sequence_ids[7], KERNEL_TRIGGER_SPRITE, 3, 81);
		kernel_seq_trigger(g_sequence_ids[7], KERNEL_TRIGGER_EXPIRE, 0, 71);
		kernel_seq_trigger(g_sequence_ids[6], KERNEL_TRIGGER_EXPIRE, 0, 73);
	}

	if (kernel.trigger == 81) {
		kernel_message_purge();
	}

	if (kernel.trigger == 71) {
		g_sequence_ids[7] = kernel_seq_forward(g_sprite_ids[7], false, 9, 0, 0, 0);
		kernel_seq_range(g_sequence_ids[7], -2, -2);
		kernel_seq_depth(g_sequence_ids[7], 1);
	}

	if (kernel.trigger == 73) {
		g_sequence_ids[6] = kernel_seq_forward(g_sprite_ids[6], false, 9, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[6], 17, -2);
		kernel_seq_trigger(g_sequence_ids[6], KERNEL_TRIGGER_EXPIRE, 0, 74);
		kernel_seq_depth(g_sequence_ids[6], 1);
	}

	if (kernel.trigger == 74) {
		g_engine->_soundManager->command(40, 0);

		kernel_message_add(quote_string(kernel.quotes, 91), 125, 56, 0xFDFC, 180, 82, 32);
		g_sequence_ids[6] = kernel_seq_forward(g_sprite_ids[6], false, 9, 0, 0, 0);
		kernel_seq_depth(g_sequence_ids[6], 1);
		kernel_seq_range(g_sequence_ids[6], -2, -2);
		kernel_timing_trigger(180, 75);
	}

	if (kernel.trigger == 75) {
		global[kMeteorologistEverSeen] = 0;
		new_room = 202;
	}

	if (kernel.trigger == 76) {
		player.commands_allowed = true;
		player.walker_visible = true;
		player.clock = kernel.clock - player.frame_delay;
	}

	if (kernel.trigger == 77) {
		global[kTeleporterCommand] = 1;
		new_room = global[kTeleporterDestination];
		kernel.force_restart = true;
	}

	if (kernel.trigger == 78) {
		g_engine->_soundManager->command(40, 0);
		text_show(20114);
		kernel.force_restart = true;
	}
}

static void room_201_parser() {
	if (player.look_around == false) {
		if (player_said_2(walk_towards, field_to_south))
			new_room = 202;
		else if (player_said_2(climb_up, steps) || (player_said_2(walk_inside, teleporter)) || (player_said_2(walk_inside, strange_device))) {
			if (kernel.trigger == 0) {
				player.commands_allowed = false;
				player.walker_visible = false;
				int sepChar = (global[kSexOfRex] == SEX_MALE) ? 't' : 'u';
				kernel_run_animation(kernel_name(sepChar, 0), 1);
			} else if (kernel.trigger == 1) {
				new_room = 213;
			}
		} else if (player_said_2(look, grassy_field)) {
			text_show(20101);
		} else if (player_said_2(look, rocks)) {
			text_show(20102);
		} else if (player_said_2(look, thorny_bush)) {
			text_show(20103);
		} else if (player_said_2(look, sky)) {
			text_show(20104);
		} else if (player_said_2(look, water)) {
			text_show(20105);
		} else if (player_said_2(look, island_in_distance)) {
			text_show(20106);
		} else if (player_said_2(look, weather_station)) {
			text_show(20107);
		} else if (player_said_2(look, path)) {
			text_show(20108);
		} else if (player_said_2(look, field_to_south)) {
			text_show(20110);
		} else if (player_said_2(look, strange_device)) {
			if (global[kMeteorologistEverSeen])
				text_show(20112);
			else
				text_show(20109);
		} else if (player_said_2(look, teleporter)) {
			text_show(20113);
		} else
			return;
	} else {
		text_show(20111);
	}
	player.command_ready = false;
}

void room_201_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._pterodactylFlag);
}

void room_201_preload() {
	room_init_code_pointer = room_201_init;
	room_parser_code_pointer = room_201_parser;
	room_daemon_code_pointer = room_201_daemon;

	section_2_walker();
	section_2_interface();

	vocab_make_active(words_swooping_creature);
	vocab_make_active(words_birds);
	vocab_make_active(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
