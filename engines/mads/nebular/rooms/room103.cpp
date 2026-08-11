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
#include "mads/core/matte.h"
#include "mads/core/pal.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section1.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	long _updateClock;
};

static Scratch local;


static void room_103_init() {
	g_sprite_ids[0] = kernel_load_series(kernel_name('x', 0), 0);
	g_sprite_ids[1] = kernel_load_series(kernel_name('x', 1), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('x', 2), 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('x', 3), 0);
	g_sprite_ids[4] = kernel_load_series(kernel_name('x', 4), 0);
	g_sprite_ids[5] = kernel_load_series(kernel_name('x', 5), 0);
	g_sprite_ids[6] = kernel_load_series(kernel_name('b', -1), 0);
	g_sprite_ids[7] = kernel_load_series(kernel_name('h', -1), 0);
	g_sprite_ids[8] = kernel_load_series(kernel_name('m', -1), 0);
	g_sprite_ids[9] = kernel_load_series(kernel_name('t', -1), 0);
	g_sprite_ids[10] = kernel_load_series(kernel_name('r', -1), 0);
	g_sprite_ids[11] = kernel_load_series(kernel_name('c', -1), 0);
	g_sprite_ids[12] = kernel_load_series("*RXMBD_2", 0);
	g_sprite_ids[13] = kernel_load_series("*RXMRD_3", 0);
	g_sprite_ids[15] = kernel_seq_forward(g_sprite_ids[0], false, 7, 0, 1, 0);

	g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 6, 0, 2, 0);
	kernel_seq_depth(g_sequence_ids[1], 0);

	g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 6, 25, 0, 0);
	kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_SPRITE, 2, 72);

	g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 6, 37, 1, 0);
	kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_SPRITE, 2, 73);

	g_sequence_ids[8] = kernel_seq_forward(g_sprite_ids[8], false, 8, 0, 0, 0);
	g_sequence_ids[7] = kernel_seq_forward(g_sprite_ids[7], false, 6, 0, 0, 0);
	g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 6, 0, 0, 0);
	g_sequence_ids[5] = kernel_seq_forward(g_sprite_ids[5], false, 6, 0, 0, 0);

	if (object_is_here(OBJ_TIMER_MODULE))
		g_sequence_ids[11] = kernel_seq_forward(g_sprite_ids[11], false, 6, 0, 0, 0);
	else
		kernel_flip_hotspot(371, false);

	if (object_is_here(OBJ_REBREATHER))
		g_sequence_ids[10] = kernel_seq_forward(g_sprite_ids[10], false, 6, 0, 0, 0);
	else
		kernel_flip_hotspot(289, false);

	if (global[kTurkeyExploded]) {
		g_sequence_ids[9] = kernel_seq_forward(g_sprite_ids[9], false, 6, 0, 0, 0);
		kernel_seq_range(g_sequence_ids[9], -2, -2);
		kernel_flip_hotspot(362, false);
	}

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 237;
		player.y = 74;
	}

	if (previous_room == 102) {
		player.commands_allowed = false;
		g_sequence_ids[6] = kernel_seq_backward(g_sprite_ids[6], false, 6, 0, 0, 1);
		kernel_seq_trigger(g_sequence_ids[6], KERNEL_TRIGGER_EXPIRE, 0, 70);
	}

	section_1_music();

	kernel.quotes = quote_load(70, 51, 71, 7, 73, 0);

	if (!player.been_here_before) {
		int msgIndex = kernel_message_add(quote_string(kernel.quotes, 70), 0, 0, 0x1110, 120, 0, 34);
		kernel_message_teletype(msgIndex, 4, true);
	}

	if (previous_room == 102)
		g_engine->_soundManager->command(20, 0);

	pal_change_color(252, 63, 63, 10);
	pal_change_color(253, 45, 45, 10);
	local._updateClock = kernel.clock;
}

static void room_103_daemon() {
	switch (kernel.trigger) {
	case 70:
		player.commands_allowed = true;
		break;

	case 72:
	{
		Common::Point pt = Common::Point(player.x, player.y);
		int dist = Math::hypotenuse(pt.x - 58, pt.y - 93);
		g_engine->_soundManager->command(27, (dist * -128 / 378) + 127);
	}
	break;

	case 73:
	{
		Common::Point pt = Common::Point(player.x, player.y);
		int dist = Math::hypotenuse(pt.x - 266, pt.y - 81);
		g_engine->_soundManager->command(27, (dist * -127 / 378) + 127);
	}
	break;

	default:
		break;
	}

	if (kernel.clock >= local._updateClock) {
		Common::Point pt = Common::Point(player.x, player.y);
		int dist = Math::hypotenuse(pt.x - 79, pt.y - 137);
		g_engine->_soundManager->command(29, (dist * -127 / 378) + 127);

		pt = Common::Point(player.x, player.y);
		dist = Math::hypotenuse(pt.x - 69, pt.y - 80);
		g_engine->_soundManager->command(30, (dist * -127 / 378) + 127);

		pt = Common::Point(player.x, player.y);
		dist = Math::hypotenuse(pt.x - 266, pt.y - 138);
		g_engine->_soundManager->command(32, (dist * -127 / 378) + 127);

		local._updateClock = kernel.clock + player.frame_delay;
	}
}

static void room_103_pre_parser() {
	// No implementation
}

static void room_103_parser() {
	if (player.look_around)
		text_show(10322);
	else if (player_said_2(walk_through, door)) {
		switch (kernel.trigger) {
		case 0:
			g_sequence_ids[6] = kernel_seq_forward(g_sprite_ids[6], false, 6, 0, 0, 1);
			kernel_seq_trigger(g_sequence_ids[6], KERNEL_TRIGGER_EXPIRE, 0, 1);
			player.commands_allowed = false;
			g_engine->_soundManager->command(20, 0);
			break;

		case 1:
			g_engine->_soundManager->command(1, 0);
			new_room = 102;
			player.commands_allowed = true;
			break;

		default:
			break;
		}
	} else if (player_said_2(take, timer_module) && object_is_here(OBJ_TIMER_MODULE)) {
		switch (kernel.trigger) {
		case 0:
			kernel_load_variant(1);
			g_sequence_ids[13] = kernel_seq_pingpong(g_sprite_ids[13], false, 3, 0, 0, 2);
			kernel_seq_player(g_sequence_ids[13], false);
			kernel_seq_trigger(g_sequence_ids[13], KERNEL_TRIGGER_SPRITE, 7, 1);
			kernel_seq_trigger(g_sequence_ids[13], KERNEL_TRIGGER_EXPIRE, 0, 2);
			player.walker_visible = false;
			player.commands_allowed = false;
			break;

		case 1:
			kernel_seq_delete(g_sequence_ids[11]);
			break;

		case 2:
			g_engine->_soundManager->command(22, 0);
			inter_give_to_player(OBJ_TIMER_MODULE);
			kernel_load_variant(0);
			matte_frame(MATTE_FX_NONE, false);
			kernel_flip_hotspot(371, false);
			player.walker_visible = true;
			player.commands_allowed = true;
			object_examine(OBJ_TIMER_MODULE, 805, 0);
			break;

		default:
			break;
		}
	} else if (player_said_2(take, rebreather) && object_is_here(OBJ_REBREATHER)) {
		switch (kernel.trigger) {
		case 0:
			g_sequence_ids[12] = kernel_seq_pingpong(g_sprite_ids[12], false, 3, 0, 0, 2);
			kernel_seq_player(g_sequence_ids[12], false);
			kernel_seq_trigger(g_sequence_ids[12], KERNEL_TRIGGER_SPRITE, 6, 1);
			kernel_seq_trigger(g_sequence_ids[12], KERNEL_TRIGGER_EXPIRE, 0, 2);
			player.walker_visible = false;
			player.commands_allowed = false;
			break;

		case 1:
			kernel_seq_delete(g_sequence_ids[10]);
			break;

		case 2:
			g_engine->_soundManager->command(22, 0);
			inter_give_to_player(OBJ_REBREATHER);
			kernel_flip_hotspot(289, false);
			player.walker_visible = true;
			player.commands_allowed = true;
			object_examine(OBJ_REBREATHER, 804, 0);
			break;

		default:
			break;
		}
	} else if (player_said_2(look, tasty_turkey))
		text_show(10301);
	else if (player_said_2(take, tasty_turkey)) {
		// Take Turkey
		if (!kernel.trigger)
			g_engine->_soundManager->command(31, 0);

		if (kernel.trigger < 2) {
			g_sequence_ids[9] = kernel_seq_forward(g_sprite_ids[9], false, 6, 0, 0, kernel.trigger < 1 ? 1 : 0);
			if (kernel.trigger) {
				// Lock the turkey into a permanent "exploded" frame
				kernel_seq_range(g_sequence_ids[9], -2, -2);

				// Rex says "Gads.."
				char *msg = quote_string(kernel.quotes, 51);
				kernel_message_add(msg, 0, 0, 0x1110, 60, 0, 18);
				kernel_timing_trigger(120, kernel.trigger + 1);
			} else {
				// Initial turky explosion
				kernel_seq_trigger(g_sequence_ids[9], KERNEL_TRIGGER_EXPIRE, 0, 1);
			}
		}

		// Re-enable player if sequence is ended, and set global flag
		player.commands_allowed = kernel.trigger == 2;
		global[kTurkeyExploded] = -1;

		if (kernel.trigger == 2) {
			// Show exposition dialog at end of sequence
			text_show(10302);
			kernel_flip_hotspot(362, false);
		}
	} else if (player_said_2(look, oven))
		text_show(!global[kTurkeyExploded] ? 10323 : 10303);
	else if (player_said_2(talkto, auxiliary_power)) {
		switch (kernel.trigger) {
		case 0:
		{
			player.commands_allowed = false;
			char *msg = quote_string(kernel.quotes, 71);
			kernel_message_add(msg, Common::Point().x, Common::Point().y, 0x1110, 120, 1, 18);
			break;
		}

		case 1:
		{
			char *msg = quote_string(kernel.quotes, 72);
			kernel_message_add(msg, 310, 132, 0xFDFC, 120, 2, 16);
			break;
		}

		case 2:
			kernel_message_purge();
			kernel_timing_trigger(1, 3);
			break;

		case 3:
			player.commands_allowed = true;
			text_show(10306);
			break;

		default:
			break;
		}
	} else if (player_said_2(look, auxiliary_power))
		text_show(10304);
	else if (player_said_2(look, big_pipes))
		text_show(10307);
	else if (player_said_2(look, burnt_out_warp_coil))
		text_show(10308);
	else if (player_said_2(take, shovel))
		text_show(10309);
	else if (player_said_2(take, coal))
		text_show(10310);
	else if (player_said_2(look, furnace))
		text_show(10312);
	else if (player_said_2(open, furnace))
		text_show(10313);
	else if (player_said_2(close, auxiliary_power))
		text_show(10314);
	else if (player_said_2(look, shield_generator))
		text_show(10315);
	else if (player_said_2(look, hyperdrive_jump_unit))
		text_show(10316);
	else if (player_said_2(look, pressure_gauge))
		text_show(10317);
	else if (player_said_2(look, engineering_controls))
		text_show(10318);
	else if (player_said_2(look, rebreather) && player_has(OBJ_REBREATHER))
		text_show(10319);
	else if (player_said_2(look, timer_module) && player_has(OBJ_TIMER_MODULE))
		text_show(10320);
	else if (player_said_2(look, floor))
		text_show(10321);
	else if (player_said_2(look, workbench))
		text_show(player_has(OBJ_TIMER_MODULE) ? 10324 : 10325);
	else
		return;

	player.command_ready = false;
}

void room_103_error() {
	if (player_said_1(auxiliary_power) && !player_said_1(walkto)) {
		text_show(10305);
		player.command_ready = false;
	} else if (player_said_3(put, coal, furnace)) {
		char *msg = quote_string(kernel.quotes, 73);
		kernel_message_add(msg, 0, 0, 0x1110, 120, 0, 34);
		player.command_ready = false;
	}
}

void room_103_synchronize(Common::Serializer &s) {
	byte dummy = 0;
	s.syncAsByte(dummy); // In order to avoid to break savegame compatibility
	s.syncAsUint32LE(local._updateClock);
}

void room_103_preload() {
	room_init_code_pointer = room_103_init;
	room_pre_parser_code_pointer = room_103_pre_parser;
	room_parser_code_pointer = room_103_parser;
	room_daemon_code_pointer = room_103_daemon;
	room_error_code_pointer = room_103_error;

	anim_himem_preload(kernel_name('A', -1), 3);

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
