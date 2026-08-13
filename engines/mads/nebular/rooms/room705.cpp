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
#include "mads/nebular/rooms/section7.h"
#include "mads/nebular/sound/mac_sound.h"
#include "mads/nebular/rooms/dialog.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	Dialog _dialog1;
};

static Scratch local;


static void handleBottleInterface() {
	switch (global[kBottleStatus]) {
	case 0:
		local._dialog1.write(0x311, true);
		local._dialog1.write(0x312, true);
		local._dialog1.write(0x313, true);
		local._dialog1.write(0x314, true);
		local._dialog1.write(0x315, true);
		break;

	case 1:
		local._dialog1.write(0x311, false);
		local._dialog1.write(0x312, true);
		local._dialog1.write(0x313, true);
		local._dialog1.write(0x314, true);
		local._dialog1.write(0x315, true);
		break;

	case 2:
		local._dialog1.write(0x311, false);
		local._dialog1.write(0x312, false);
		local._dialog1.write(0x313, true);
		local._dialog1.write(0x314, true);
		local._dialog1.write(0x315, true);
		break;

	case 3:
		local._dialog1.write(0x311, false);
		local._dialog1.write(0x312, false);
		local._dialog1.write(0x313, false);
		local._dialog1.write(0x314, true);
		local._dialog1.write(0x315, true);
		break;

	default:
		break;
	}
}

static void setBottleSequence() {
	kernel_set_interface_mode(INTER_BUILDING_SENTENCES);
	player.commands_allowed = false;
	kernel_seq_delete(g_sequence_ids[3]);
	kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
	kernel_run_animation(kernel_name('F', -1), 90);
}

static void handleFillBottle(int quote) {
	switch (quote) {
	case 0x311:
		global[kBottleStatus] = 1;
		setBottleSequence();
		break;

	case 0x312:
		global[kBottleStatus] = 2;
		setBottleSequence();
		break;

	case 0x313:
		global[kBottleStatus] = 3;
		setBottleSequence();
		break;

	case 0x314:
		global[kBottleStatus] = 4;
		setBottleSequence();
		break;

	case 0x315:
		kernel_set_interface_mode(INTER_BUILDING_SENTENCES);
		break;

	default:
		break;
	}
}

static void room_705_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('b', 0), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('b', 1), 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('a', 0), 0);

	player.walker_visible = false;

	if (previous_room == 706) {
		player.commands_allowed = false;
		g_sequence_ids[3] = kernel_seq_backward(g_sprite_ids[3], false, 9, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[3], 1, 4);
		kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 71);
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.commands_allowed = false;
		kernel_timing_trigger(1, 80);
		if (!Sound::commandMacintoshSound(Sound::kMacSoundPlay, 7028,
				0, 0, 0, true))
			g_engine->_soundManager->command(28, 0);
	} else
		g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 1);

	if (kernel.teleported_in)
		inter_give_to_player(OBJ_BOTTLE);

	kernel.quotes = quote_load(785, 786, 787, 788, 789, 0);
	local._dialog1.setup(0x98, 0x311, 0x312, 0x313, 0x314, 0x315, 0);
	section_7_music();
}

static void room_705_daemon() {
	switch (kernel.trigger) {
	case 70:
		g_sequence_ids[3] = kernel_seq_backward(g_sprite_ids[3], false, 9, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[3], 1, 4);
		kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 71);
		break;

	case 71:
	{
		int syncIdx = g_sequence_ids[3];
		g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 1);
		kernel_seq_timeout(syncIdx, g_sequence_ids[3]);
		player.commands_allowed = true;
	}
	break;

	default:
		break;
	}

	switch (kernel.trigger) {
	case 80:
		g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 9, 0, 0, 1);
		kernel_seq_depth(g_sequence_ids[1], 2);
		kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 81);
		break;

	case 81:
	{
		g_engine->_soundManager->command(19, 0);
		int syncIdx = g_sequence_ids[1];
		g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 1);
		kernel_seq_timeout(syncIdx, g_sequence_ids[3]);
		player.commands_allowed = true;
	}
	break;

	default:
		break;
	}

	switch (kernel.trigger) {
	case 90:
		g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 1);
		kernel_timing_trigger(30, 91);
		break;

	case 91:
		switch (global[kBottleStatus]) {
		case 0:
			text_show(432);
			break;

		case 1:
			text_show(70324);
			break;

		case 2:
			text_show(70325);
			break;

		case 3:
			text_show(70326);
			break;

		case 4:
			text_show(70327);
			break;

		default:
			break;
		}
		player.commands_allowed = true;
		break;

	default:
		break;
	}
}

static void room_705_parser() {
	if (inter_input_mode == INTER_CONVERSATION)
		handleFillBottle(player2.words[0]);
	else if (player_said_2(steer_towards, open_water_to_south)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			kernel_seq_delete(g_sequence_ids[3]);
			g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 6, 0, 0, 1);
			kernel_seq_depth(g_sequence_ids[2], 2);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 1);
			g_engine->_soundManager->command(18, 0);
			break;

		case 1:
		{
			int syncIdx = g_sequence_ids[2];
			g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, -2);
			kernel_seq_depth(g_sequence_ids[2], 2);
			kernel_seq_timeout(syncIdx, g_sequence_ids[2]);
			new_room = 704;
			player.commands_allowed = true;
		}
		break;

		default:
			break;
		}
	} else if (player_said_2(climb_through, window)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			kernel_seq_delete(g_sequence_ids[3]);
			g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 6, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[3], 1, 16);
			kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
		{
			int syncIdx = g_sequence_ids[3];
			g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, 16);
			kernel_seq_timeout(syncIdx, g_sequence_ids[3]);
			new_room = 706;
			player.commands_allowed = true;
		}
		break;

		default:
			break;
		}
	} else if (player_said_3(fill, bottle, water) || player_said_3(put, bottle, water)) {
		if (global[kBottleStatus] != 4) {
			handleBottleInterface();
			local._dialog1.start();
		} else
			text_show(70323);
	} else if (player.look_around || player_said_2(look, water))
		text_show(70511);
	else if (player_said_2(look, volcano_rim))
		text_show(70512);
	else if (player_said_2(look, open_water_to_south))
		text_show(70513);
	else if (player_said_2(look, sky))
		text_show(70514);
	else if (player_said_2(look, building))
		text_show(70515);
	else if (player_said_2(look, window))
		text_show(70516);
	else
		return;

	player.command_ready = false;
}

void room_705_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_705_preload() {
	room_init_code_pointer = room_705_init;
	room_daemon_code_pointer = room_705_daemon;
	room_parser_code_pointer = room_705_parser;

	*player.series_name = '\0';
	section_7_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
