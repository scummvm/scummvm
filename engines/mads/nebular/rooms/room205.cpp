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
#include "mads/nebular/rooms/dialog.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section2.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	long _lastFishTime;
	long _chickenTime;
	bool _beingKicked;
	int16 _kernelMessage;
	Dialog _dialog1;
};

static Scratch local;


static void room_205_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('x', 0), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('x', 1), 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('x', 2), 0);
	g_sprite_ids[5] = kernel_load_series(kernel_name('f', -1), 0);
	g_sprite_ids[4] = kernel_load_series(kernel_name('c', -1), 0);
	g_sprite_ids[6] = kernel_load_series(kernel_name('p', -1), 0);

	if (global[kSexOfRex] == SEX_MALE)
		g_sprite_ids[8] = kernel_load_series(kernel_name('a', 1), 0);

	g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 10, 3, 0, 0);
	int idx = kernel_add_dynamic(words_chicken, words_walkto, 0, g_sequence_ids[1], 0, 0, 0, 0);
	kernel_dynamic_walk(idx, 162, 120, FACING_NORTHEAST);

	g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 15, 0, 0, 0);
	idx = kernel_add_dynamic(words_chicken, words_walkto, 0, g_sequence_ids[1], 0, 0, 0, 0);
	kernel_dynamic_walk(idx, 162, 120, FACING_NORTHEAST);

	g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 9, 0, 0, 0);
	g_sequence_ids[5] = kernel_seq_forward(g_sprite_ids[5], false, 6, 0, 0, 0);
	kernel_seq_depth(g_sequence_ids[5], 11);

	if (!player.been_here_before) {
		g_sequence_ids[6] = kernel_seq_forward(g_sprite_ids[6], false, 7, 0, 0, 1);
		idx = kernel_add_dynamic(words_piranha, words_walkto, 0, g_sequence_ids[6], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 49, 86, FACING_NORTH);
	}

	if (object[12].location == 205) {
		g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 7, 0, 0, 0);
		kernel_seq_depth(g_sequence_ids[4], 11);
	} else {
		kernel_flip_hotspot(450, false);
	}

	local._beingKicked = false;
	kernel.quotes = quote_load(107, 112, 113, 114, 90, 116, 117, 118, 119, 120, 115, 121, 122, 123, 124,
		125, 126, 127, 128, 172, 173, 174, 108, 109, 110, 111, 2, 0);
	local._dialog1.setup(0x2A, 0x5A, 0x78, 0x74, 0x75, 0x76, 0x77, 0);

	if (!player.been_here_before)
		local._dialog1.set(0x5A, 0x74, 0x75, 0x77, 0);

	pal_change_color(250, 63, 50, 20);
	pal_change_color(251, 50, 40, 15);
	pal_change_color(252, 63, 63, 40);
	pal_change_color(253, 50, 50, 30);

	local._chickenTime = kernel.clock;

	if (global[kSexOfRex] == SEX_FEMALE)
		kernel_random_messages_init(3, 195, 264, 99, 134, 13, 2, 0xFDFC, 60, 108, 108, 109, 109, 110, 110, 111, 108, 0);

	if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 99;
		player.y = 152;
	}

	if (global[kSexOfRex] != SEX_MALE) {
		kernel_run_animation(kernel_name('a', -1), 0);
		kernel_anim[0].repeat = true;
	} else {
		local._beingKicked = true;
		g_sequence_ids[8] = kernel_seq_forward(g_sprite_ids[8], false, 8, 0, 0, 1);
		player.walker_visible = false;
		player.commands_allowed = false;
		kernel_seq_depth(g_sequence_ids[8], 2);
		kernel_seq_trigger(g_sequence_ids[8], KERNEL_TRIGGER_SPRITE, 6, 73);
		kernel_seq_trigger(g_sequence_ids[8], KERNEL_TRIGGER_SPRITE, 11, 74);
		kernel_seq_trigger(g_sequence_ids[8], KERNEL_TRIGGER_EXPIRE, 0, 71);
	}

	section_2_music();
}

static void room_205_daemon() {
	if (global[kSexOfRex] == SEX_FEMALE) {
		kernel_random_message_server();

		if (kernel.clock >= local._chickenTime) {
			int chanceMinor = kernel_check_random() + 1;
			if (kernel_generate_random_message(100, chanceMinor))
				g_engine->_soundManager->command(28, 0);

			local._chickenTime = kernel.clock + 2;
		}
	}

	if (kernel.clock - local._lastFishTime > 1300) {
		g_sequence_ids[6] = kernel_seq_forward(g_sprite_ids[6], false, 5, 0, 0, 1);
		int idx = kernel_add_dynamic(words_piranha, words_walkto, 0, g_sequence_ids[6], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 49, 86, FACING_NORTH);
		local._lastFishTime = kernel.clock;
	}

	if (kernel.trigger == 73)
		kernel_message_add(quote_string(kernel.quotes, 112), 160, 68, 0xFBFA, 60, 0, 32);

	if (kernel.trigger == 74) {
		g_engine->_soundManager->command(26, 0);
		kernel_message_add(quote_string(kernel.quotes, 113), 106, 90, 0x1110, 60, 0, 32);
	}

	if (kernel.trigger == 71) {
		g_sequence_ids[8] = kernel_seq_forward(g_sprite_ids[8], false, 6, 0, 0, 0);
		kernel_seq_depth(g_sequence_ids[8], 2);
		kernel_seq_range(g_sequence_ids[8], -2, -2);
		kernel_message_purge();
		kernel_message_add(quote_string(kernel.quotes, 114), 160, 68, 0xFBFA, 180, 72, 32);
	}

	if (kernel.trigger == 72)
		new_room = 211;
}

static void handleWomanSpeech(int quote) {
	local._kernelMessage = kernel_message_add(quote_string(kernel.quotes, quote), 186, 27, 0xFBFA, INDEFINITE_TIMEOUT, 0, 0);
}

static void room_205_parser() {
	if (inter_input_mode == INTER_CONVERSATION) {
		if (local._kernelMessage >= 0)
			kernel_message_delete(local._kernelMessage);
		local._kernelMessage = -1;

		if (kernel.trigger == 0) {
			player.commands_allowed = false;
			kernel_message_add(quote_string(kernel.quotes, player2.words[0]), 0, 0, 0x1110, 120, 1, 18);
		} else {
			if ((kernel.trigger > 1) || (player2.words[0] != words_eject))
				player.commands_allowed = true;

			switch (player2.words[0]) {
			case words_coral:
				handleWomanSpeech(0x7A);
				local._dialog1.write(0x78, true);
				local._dialog1.write(0x5A, false);
				break;

			case words_eastern_cliff_face:
				handleWomanSpeech(0x7C);
				local._dialog1.write(0x74, false);
				local._dialog1.write(0x76, true);
				break;

			case words_eat:
			case words_engineering_controls:
				handleWomanSpeech(0x7B);
				local._dialog1.write(player2.words[0], false);
				text_show(20501);
				break;

			case words_eject:
				if (kernel.trigger == 1) {
					handleWomanSpeech(0x7D);
					kernel_timing_trigger(120, 2);
				} else if (kernel.trigger == 2) {
					handleWomanSpeech(0x7E);
					local._dialog1.write(0x76, false);
					global[kChickenPermitted] = true;
				}
				break;

			case words_empty:
				kernel_message_add(quote_string(kernel.quotes, 0x7F), 186, 27, 0xFBFA, 120, 0, 0);
				kernel_set_interface_mode(INTER_BUILDING_SENTENCES);
				break;

			default:
				break;
			}

			if (player2.words[0] != words_empty)
				local._dialog1.start();
		}
	} else if (player.look_around)
		text_show(20502);
	else if (player_said_3(look, binoculars, opposite_bank))
		text_show(20518);
	else if (player_said_2(talkto, native_woman)) {
		if (kernel.trigger == 0) {
			player.commands_allowed = false;
			kernel_message_add(quote_string(kernel.quotes, 0x73), 0, 0, 0x1110, 120, 1, 18);
		} else if (kernel.trigger == 1) {
			player.commands_allowed = true;
			handleWomanSpeech(0x79);
			local._dialog1.write(0x5A, true);
			local._dialog1.write(0x75, true);
			local._dialog1.start();
		}
	} else if (player_said_2(give, native_woman) && player_has(object_named(player2.words[1]))) {
		if (kernel.trigger == 0) {
			player.commands_allowed = false;
			int rndVal = g_engine->getRandomNumber(0xAC, 0xAE);
			kernel_message_add(quote_string(kernel.quotes, rndVal), 186, 27, 0xFBFA, 120, 1, 32);
		} else if (kernel.trigger == 1)
			player.commands_allowed = true;
	} else if (player_said_2(walkto, opposite_bank)) {
		if (kernel.trigger == 0) {
			player.walker_visible = false;
			player.commands_allowed = false;
			pal_lock();
			kernel_message_purge();
			kernel_dump_walker_only();
			g_sprite_ids[9] = kernel_load_series(kernel_name('a', 0), 0);
			kernel_new_palette();
			g_sequence_ids[9] = kernel_seq_forward(g_sprite_ids[9], false, 6, 0, 0, 1);
			kernel_seq_trigger(g_sequence_ids[9], KERNEL_TRIGGER_EXPIRE, 0, 1);
			kernel_seq_timeout(-1, g_sequence_ids[9]);
			g_engine->_soundManager->command(27, 0);
		} else if (kernel.trigger == 1) {
			if (kernel_anim[0].anim != nullptr)
				kernel_anim[0].anim->num_series = 0;

			text_show(20516);
			kernel.force_restart = true;
		}
	} else {
		if (player_said_2(walk_down, path_to_south))
			new_room = 210;

		if (player_said_2(walkto, fire_pit) || player_said_2(walkto, chicken_on_spit)) {
			if (object_is_here(OBJ_CHICKEN)) {
				kernel_message_purge();
				kernel_message_add(quote_string(kernel.quotes, 0x6B), 0, 0, 0x1110, 120, 0, 34);
			}
		} else if (player_said_2(take, chicken_on_spit) && global[kChickenPermitted] && object_is_here(OBJ_CHICKEN)) {
			inter_give_to_player(OBJ_CHICKEN);
			kernel_seq_delete(g_sequence_ids[4]);
			kernel_flip_hotspot(words_chicken_on_spit, false);
			object_examine(OBJ_CHICKEN, 812, 0);
		} else if (player_said_2(take, chicken_on_spit) && (!global[kChickenPermitted]))
			kernel_message_add(quote_string(kernel.quotes, 0x80), 186, 27, 0xFBFA, 120, 0, 32);
		else if (player_said_2(look, native_woman))
			text_show(20503);
		else if (player_said_2(look, hut))
			text_show(20504);
		else if (player_said_2(look, chicken) && (player.main_object_source == STROKE_INTERFACE))
			text_show(20505);
		else if (player_said_2(take, chicken) && (player.main_object_source == STROKE_INTERFACE))
			text_show(20506);
		else if (player_said_2(look, chicken_on_spit))
			text_show(20507);
		else if (player_said_2(look, fire_pit))
			text_show(20508);
		else if (player_said_2(take, fire_pit))
			text_show(20509);
		else if (player_said_2(look, stream))
			text_show(20510);
		else if (player_said_2(look, opposite_bank))
			text_show(20511);
		else if (player_has(object_named(player2.words[1]))
			&& (player_said_2(give, stream) || player_said_2(throw, stream)
				|| player_said_2(give, piranha) || player_said_2(throw, piranha)))
			text_show(20512);
		else if (player_said_2(look, piranha))
			text_show(20513);
		else if (player_said_2(look, twinkifruit_bush))
			text_show(20514);
		else if (player_said_2(take, twinkifruit_bush))
			text_show(20515);
		else if (player_said_2(take, native_woman))
			text_show(20517);
		else
			return;
	}

	player.command_ready = false;
}

void room_205_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._beingKicked);
	s.syncAsSint16LE(local._kernelMessage);
}

void room_205_preload() {
	local._lastFishTime = kernel.clock;
	local._chickenTime = kernel.clock;
	local._beingKicked = false;
	local._kernelMessage = -1;

	room_init_code_pointer = room_205_init;
	room_parser_code_pointer = room_205_parser;
	room_daemon_code_pointer = room_205_daemon;

	section_2_walker();
	section_2_interface();
	vocab_make_active(words_walkto);
	vocab_make_active(words_chicken);
	vocab_make_active(words_piranha);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
