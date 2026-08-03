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
#include "mads/core/pal.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section6.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int32 _dogTimer;
	int32 _lastFrameTime;
	bool _dogLoop;
	bool _dogEatsRex;
	bool _dogBarking;
	bool _shopAvailable;
	int16 _animationMode;
	int16 _animationActive;
	int16 _counter;
};

static Scratch local;


static void room_607_init() {
	g_sprite_ids[2] = kernel_load_series(kernel_name('c', 0), 0);
	g_sprite_ids[3] = kernel_load_series("*RXCD_3", 0);

	if (!player.been_here_before && (previous_room != 608))
		global[kDogStatus] = DOG_PRESENT;

	if ((previous_room == 608) && (global[kDogStatus] < DOG_GONE))
		global[kDogStatus] = DOG_GONE;

	local._animationActive = 0;

	if ((global[kDogStatus] == DOG_PRESENT) && (game.difficulty != DIFFICULTY_EASY)) {
		g_sprite_ids[4] = kernel_load_series(kernel_name('g', 3), 0);
		g_sprite_ids[5] = kernel_load_series(kernel_name('g', 7), 0);
		g_sprite_ids[1] = kernel_load_series(kernel_name('g', 0), 0);

		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 1);
		kernel_seq_depth(g_sequence_ids[1], 6);
		local._dogBarking = false;
		local._dogLoop = false;
		local._shopAvailable = false;
		local._dogEatsRex = false;
		local._dogTimer = 0;
	} else
		kernel_flip_hotspot(words_obnoxious_dog, false);

	g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, -2);
	kernel_seq_depth(g_sequence_ids[2], 4);

	if (previous_room == 608) {
		player.x = 297;
		player.y = 50;
		player.facing = FACING_SOUTHEAST;
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 40;
		player.y = 104;
		player.facing = FACING_SOUTHEAST;
		player.walker_visible = false;
		player.commands_allowed = false;
		kernel_seq_delete(g_sequence_ids[2]);
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, -1);
		kernel_seq_depth(g_sequence_ids[2], 4);
		kernel_run_animation(kernel_name('R', 1), 80);
	} else if (global[kDogStatus] == DOG_LEFT) {
		g_sprite_ids[4] = kernel_load_series(kernel_name('g', 3), 0);
		g_sprite_ids[5] = kernel_load_series(kernel_name('g', 7), 0);
		g_sprite_ids[1] = kernel_load_series(kernel_name('g', 0), 0);
	}

	section_6_music();

	if (kernel.teleported_in)
		inter_give_to_player(OBJ_BONES);

	pal_change_color(252, 63, 44, 30);
	pal_change_color(253, 63, 20, 22);
	kernel.quotes = quote_load(760, 759, 758, 761, 762, 0);
}

static void room_607_daemon() {
	if (global[kDogStatus] == DOG_LEFT) {
		int32 diff = kernel.clock - local._lastFrameTime;
		if ((diff >= 0) && (diff <= 4))
			local._dogTimer += diff;
		else
			local._dogTimer++;

		local._lastFrameTime = kernel.clock;
	}

	if ((local._dogTimer >= 480) && !local._dogLoop && !local._shopAvailable && (global[kDogStatus] == DOG_LEFT) && !player.special_code) {
		g_engine->_soundManager->command(14, 0);
		local._dogLoop = true;
		g_sequence_ids[5] = kernel_seq_forward(g_sprite_ids[5], false, 10, 0, 0, 1);
		kernel_seq_depth(g_sequence_ids[5], 1);
		kernel_seq_trigger(g_sequence_ids[5], KERNEL_TRIGGER_EXPIRE, 0, 91);
		local._dogLoop = false;
		local._dogTimer = 0;
	}

	if (kernel.trigger == 91) {
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 1);
		kernel_seq_depth(g_sequence_ids[1], 6);
		local._dogBarking = false;
		global[kDogStatus] = DOG_PRESENT;
		kernel_flip_hotspot(words_obnoxious_dog, true);
	}

	if (!local._dogEatsRex && (game.difficulty != DIFFICULTY_EASY) && !local._animationActive && (global[kDogStatus] == DOG_PRESENT)
		&& !local._dogBarking && (g_engine->getRandomNumber(1, 50) == 10)) {
		local._dogBarking = true;
		kernel_seq_delete(g_sequence_ids[1]);
		g_sequence_ids[1] = kernel_seq_pingpong(g_sprite_ids[1], false, 5, 0, 0, 8);
		kernel_seq_depth(g_sequence_ids[1], 6);
		kernel_message_purge();
		kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_SPRITE, 2, 100);
		kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 70);
		local._counter = 0;
	}

	if ((kernel.trigger == 70) && !local._dogEatsRex && (global[kDogStatus] == DOG_PRESENT) && !local._animationActive) {
		int syncIdx = g_sequence_ids[1];
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 1);
		kernel_seq_depth(g_sequence_ids[1], 6);
		kernel_seq_timeout(syncIdx, g_sequence_ids[1]);
		kernel_message_purge();
		local._dogBarking = false;
	}

	if (kernel.trigger == 100) {
		local._counter++;
		g_engine->_soundManager->command(12, 0);

		if ((local._counter >= 1) && (local._counter <= 4)) {
			Common::Point pos(0, 0);
			switch (local._counter) {
			case 1:
				pos = Common::Point(237, 5);
				break;

			case 2:
				pos = Common::Point(270, 15);
				break;

			case 3:
				pos = Common::Point(237, 25);
				break;

			case 4:
				pos = Common::Point(270, 36);
				break;

			default:
				break;
			}
			kernel_message_add(quote_string(kernel.quotes, 0x2F9), pos.x, pos.y, 0xFDFC, 120, 0, 0);
		}
	}

	if (player.walking && (game.difficulty != DIFFICULTY_EASY) && !local._shopAvailable && (global[kDogStatus] == DOG_PRESENT) && (player.next_special_code > 0)) {
		player_cancel_command();
		player_start_walking(268, 72, FACING_NORTHEAST);
		player.next_special_code = 0;
	}

	if ((player.special_code > 0) && (game.difficulty != DIFFICULTY_EASY) && (global[kDogStatus] == DOG_PRESENT) && player.commands_allowed)
		player.commands_allowed = false;

	if ((game.difficulty != DIFFICULTY_EASY) && (global[kDogStatus] == DOG_PRESENT) && (Common::Point(player.x, player.y) == Common::Point(268, 72))
		&& (kernel.trigger || !local._dogEatsRex)) {
		local._dogEatsRex = true;
		switch (kernel.trigger) {
		case 91:
		case 0:
			local._animationActive = 1;
			player.walker_visible = false;
			kernel_seq_delete(g_sequence_ids[1]);
			g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 10, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[4], -1, 7);
			kernel_seq_depth(g_sequence_ids[4], 1);
			kernel_message_purge();
			kernel_message_add(quote_string(kernel.quotes, 0x2FA), 0, 0, 0x1110, 120, 0, 34);
			kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 60);
			kernel_timing_trigger(10, 64);
			break;

		case 60:
		{
			int syncIdx = g_sequence_ids[4];
			g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 5, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[4], 8, 45);
			kernel_seq_depth(g_sequence_ids[4], 1);
			kernel_seq_timeout(syncIdx, g_sequence_ids[4]);
			kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 61);
		}
		break;

		case 61:
		{
			int syncIdx = g_sequence_ids[4];
			g_sequence_ids[4] = kernel_seq_pingpong(g_sprite_ids[4], false, 15, 0, 0, 3);
			kernel_seq_range(g_sequence_ids[4], 46, -2);
			kernel_seq_depth(g_sequence_ids[4], 1);
			kernel_seq_timeout(syncIdx, g_sequence_ids[4]);
			kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 62);
		}
		break;

		case 62:
		{
			int syncIdx = g_sequence_ids[4];
			local._animationActive = 2;
			g_sequence_ids[4] = kernel_seq_stamp(g_sprite_ids[4], false, -2);
			kernel_seq_depth(g_sequence_ids[4], 1);
			kernel_seq_timeout(syncIdx, g_sequence_ids[4]);
			kernel_timing_trigger(60, 63);
		}
		break;

		case 63:
			text_show(60729);
			local._animationActive = 0;
			local._dogEatsRex = false;
			kernel.force_restart = true;
			player.commands_allowed = true;
			break;

		case 64:
			if (local._dogEatsRex && (local._animationActive == 1)) {
				g_engine->_soundManager->command(12, 0);
				kernel_timing_trigger(10, 64);
			}
			break;

		default:
			break;
		}
	}

	switch (kernel.trigger) {
	case 80:
		player.walker_visible = true;
		player.clock = kernel_anim[0].next_clock - player.frame_delay;
		kernel_timing_trigger(6, 81);
		break;

	case 81:
		kernel_seq_delete(g_sequence_ids[2]);
		g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 6, 0, 0, 1);
		kernel_seq_depth(g_sequence_ids[2], 4);
		kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 82);
		break;

	case 82:
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, -2);
		kernel_seq_depth(g_sequence_ids[2], 4);
		player.commands_allowed = true;
		break;

	default:
		break;
	}
}

static void handleThrowingBone() {
	local._animationActive = -1;
	switch (kernel.trigger) {
	case 0:
		player.commands_allowed = false;
		kernel_seq_delete(g_sequence_ids[1]);
		player.walker_visible = false;
		kernel_run_animation(kernel_name('D', local._animationMode), 1);
		break;

	case 1:
		player.walker_visible = true;
		player.clock = kernel_anim[0].next_clock - player.frame_delay;

		if (local._animationMode != 1)
			kernel_flip_hotspot(words_obnoxious_dog, false);
		else {
			g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 1);
			kernel_seq_depth(g_sequence_ids[1], 6);
		}

		local._dogBarking = false;
		if (player_has(OBJ_BONE)) {
			inter_move_object(OBJ_BONE, 1);
			if (local._animationMode == 1)
				global[kBone202Status] = 0;
		} else {
			inter_move_object(OBJ_BONES, 1);
			inter_give_to_player(OBJ_BONE);
		}

		kernel_timing_trigger(60, 2);
		break;

	case 2:
	{
		int quoteId = 0x2F8;
		if (local._animationMode == 1)
			quoteId = 0x2F7;

		if (local._animationMode == 2) {
			global[kDogStatus] = DOG_LEFT;
			local._dogTimer = 0;
		}

		kernel_message_purge();
		kernel_message_add(quote_string(kernel.quotes, quoteId), 0, 0, 0x1110, 120, 0, 34);
		kernel_timing_trigger(60, 3);
	}
	break;

	case 3:
		player.commands_allowed = true;
		local._animationActive = 0;
		break;

	default:
		break;
	}
}

static void room_607_pre_parser() {
	if (player_said_2(talkto, obnoxious_dog))
		player.need_to_walk = false;

	if (player_said_2(walk_through, side_entrance) && (global[kDogStatus] == DOG_LEFT) && (game.difficulty != DIFFICULTY_EASY)) {
		local._shopAvailable = true;
		local._dogTimer = 0;
	}

	if (player_said_3(throw, bones, obnoxious_dog) || player_said_3(throw, bone, obnoxious_dog))
		player_walk(193, 100, FACING_NORTHEAST);

	if (player_said_3(throw, bones, fence) || player_said_3(throw, bone, fence))
		player_walk(201, 107, FACING_SOUTHEAST);
}

static void room_607_parser() {
	if (player_said_2(walk_through, side_entrance))
		new_room = 608;
	else if (player_said_2(get_inside, car)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			kernel_seq_delete(g_sequence_ids[2]);
			g_sequence_ids[2] = kernel_seq_backward(g_sprite_ids[2], false, 6, 0, 0, 1);
			kernel_seq_depth(g_sequence_ids[2], 4);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
		{
			int syncIdx = g_sequence_ids[2];
			g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, -1);
			kernel_seq_timeout(syncIdx, g_sequence_ids[2]);
			kernel_timing_trigger(6, 2);
		}
		break;

		case 2:
			player.walker_visible = false;
			g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 10, 0, 0, 1);
			kernel_seq_player(g_sequence_ids[3], false);
			kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 3);
			break;

		case 3:
		{
			int syncIdx = g_sequence_ids[3];
			g_sequence_ids[3] = kernel_seq_stamp(g_sprite_ids[3], false, -2);
			kernel_seq_player(g_sequence_ids[3], false);
			kernel_seq_timeout(syncIdx, g_sequence_ids[3]);
			new_room = 504;
		}
		break;

		default:
			break;
		}
	} else if (player_said_3(throw, bones, obnoxious_dog) || player_said_3(throw, bone, obnoxious_dog)) {
		if (game.difficulty != DIFFICULTY_EASY) {
			local._animationMode = 1;
			kernel_message_purge();
			if (kernel.trigger == 0)
				kernel_message_add(quote_string(kernel.quotes, 0x2F6), 0, 0, 0x1110, 120, 0, 34);

			handleThrowingBone();
		}
	} else if ((player_said_3(throw, bones, fence) || player_said_3(throw, bone, fence)) && (game.difficulty != DIFFICULTY_EASY)
		&& ((global[kDogStatus] == DOG_PRESENT) || kernel.trigger)) {
		local._animationMode = 2;
		if (kernel.trigger == 0) {
			kernel_message_purge();
			kernel_message_add(quote_string(kernel.quotes, 0x2F6), 0, 0, 0x1110, 120, 0, 34);
		}
		handleThrowingBone();
	} else if (player.look_around || player_said_2(look, street)) {
		if ((global[kDogStatus] == DOG_PRESENT) || (game.difficulty == DIFFICULTY_EASY))
			text_show(60710);
		else
			text_show(60711);
	} else if (player_said_2(look, wall))
		text_show(60712);
	else if (player_said_2(look, fence))
		text_show(60713);
	else if (player_said_2(look, car))
		text_show(60714);
	else if (player_said_2(look, manhole))
		text_show(60715);
	else if (player_said_2(look, fire_hydrant) && (global[kDogStatus] == DOG_PRESENT))
		text_show(60716);
	else if (player_said_2(look, sign))
		text_show(60717);
	else if (player_said_2(look, broken_window))
		text_show(60718);
	else if (player_said_2(look, garage_door))
		text_show(60719);
	else if (player_said_2(look, sidewalk))
		text_show(60720);
	else if (player_said_2(look, air_hose))
		text_show(60721);
	else if (player_said_2(look, auto_shop)) {
		if (global[kDogStatus] == DOG_PRESENT)
			text_show(60723);
		else
			text_show(60722);
	} else if (player_said_2(look, side_entrance)) {
		if (global[kDogStatus] == DOG_PRESENT)
			text_show(60725);
		else
			text_show(60724);
	} else if (player_said_2(look, obnoxious_dog))
		text_show(60726);
	else if (player_said_2(talkto, obnoxious_dog))
		text_show(60727);
	else if (player_said_2(look, barricade))
		text_show(60728);
	else if (player_said_2(walk_down, street))
		text_show(60730);
	else if (player_said_1(garage_door) && (player_said_1(open) || player_said_1(push) || player_said_1(pull)))
		text_show(60731);
	else
		return;

	player.command_ready = false;
}

void room_607_synchronize(Common::Serializer &s) {
	s.syncAsUint32LE(local._dogTimer);
	s.syncAsUint32LE(local._lastFrameTime);

	s.syncAsByte(local._dogLoop);
	s.syncAsByte(local._dogEatsRex);
	s.syncAsByte(local._dogBarking);
	s.syncAsByte(local._shopAvailable);

	s.syncAsSint16LE(local._animationMode);
	s.syncAsSint16LE(local._animationActive);
	s.syncAsSint16LE(local._counter);
}

void room_607_preload() {
	room_init_code_pointer = room_607_init;
	room_daemon_code_pointer = room_607_daemon;
	room_pre_parser_code_pointer = room_607_pre_parser;
	room_parser_code_pointer = room_607_parser;

	section_6_walker();
	section_6_interface();
	vocab_make_active(words_obnoxious_dog);
	vocab_make_active(words_walkto);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
