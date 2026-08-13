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
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section1.h"
#include "mads/nebular/sound/mac_sound.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

#define QUOTE_ID(INDEX) ((g_engine->isDemo() ? 11 : 59) + INDEX)

struct Scratch {
	byte _fridgeOpenedFl;
	byte _fridgeOpenedDescr;
	byte _fridgeFirstOpenFl;
	byte _chairDescrFl;
	byte _drawerDescrFl;
	byte _activeMsgFl;

	int16 _fridgeCommentCount;
};

static Scratch local;

static void addRandomMessage() {
	kernel_message_purge();
	kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
	int quoteId = g_engine->getRandomNumber(QUOTE_ID(6), QUOTE_ID(10));
	kernel_message_add(quote_string(kernel.quotes, quoteId), 0, 0, 0x1110, 120, 73, 34);
	local._activeMsgFl = true;
}

static void room_102_init() {
	section_1_music();

	g_sprite_ids[1] = kernel_load_series(kernel_name('x', 1), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('x', 2), 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('x', 3), 0);
	g_sprite_ids[4] = kernel_load_series(kernel_name('x', 4), 0);
	g_sprite_ids[5] = kernel_load_series(kernel_name('x', 5), 0);
	g_sprite_ids[6] = kernel_load_series(kernel_name('b', -1), 0);
	g_sprite_ids[7] = kernel_load_series(kernel_name('c', -1), 0);
	g_sprite_ids[8] = kernel_load_series(kernel_name('e', -1), 0);
	g_sprite_ids[9] = kernel_load_series(kernel_name('n', -1), 0);
	g_sprite_ids[10] = kernel_load_series(kernel_name('g', -1), 0);
	g_sprite_ids[11] = kernel_load_series("*RXMRC_8", 0);
	g_sprite_ids[13] = kernel_load_series(kernel_name('x', 0), 0);

	g_sequence_ids[1] = kernel_seq_pingpong(g_sprite_ids[1], false, 8, 0, 0, 0);
	g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 170, 6, 1, 0);
	g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 11, 3, 2, 0);
	g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 4, 0, 1, 0);
	g_sequence_ids[5] = kernel_seq_forward(g_sprite_ids[5], false, 3, 5, 0, 0);

	if (object_is_here(OBJ_BINOCULARS))
		g_sequence_ids[9] = kernel_seq_forward(g_sprite_ids[9], false, 24, 24, 0, 0);
	else
		kernel_flip_hotspot(words_binoculars, false);

	kernel_flip_hotspot(words_burger, false);

	if (global[kMedicineCabinetOpen]) {
		g_sequence_ids[8] = kernel_seq_forward(g_sprite_ids[8], false, 6, 0, 0, 0);
		kernel_seq_range(g_sequence_ids[8], -2, -2);
	}

	if (g_engine->isDemo() && previous_room == KERNEL_STARTING_GAME) {
		player.x = 169;
		player.y = 115;
		player.facing = FACING_SOUTH;
		inter_move_object(OBJ_REBREATHER, PLAYER);
	} else if (previous_room == 101) {
		player.x = 229;
		player.y = 109;
		player.commands_allowed = false;
		g_sequence_ids[6] = kernel_seq_backward(g_sprite_ids[6], false, 6, 0, 2, 1);
		kernel_seq_trigger(g_sequence_ids[6], KERNEL_TRIGGER_EXPIRE, 0, 70);
	} else if (previous_room == 103) {
		player.x = 47;
		player.y = 152;
	}
	else if (previous_room != KERNEL_RESTORING_GAME) {
		player.facing = FACING_NORTHWEST;
		player.x = 32;
		player.y = 129;
	}

	if (previous_room != 106) {
		if (global[kWaterInAPuddle]) {
			g_sequence_ids[13] = kernel_seq_forward(g_sprite_ids[13], false, 6, 0, 0, 0);
			kernel_seq_range(g_sequence_ids[13], -2, -2);
			kernel_seq_depth(g_sequence_ids[13], 5);
		}
	} else {
		player.commands_allowed = false;
		player.walker_visible = false;
		g_sequence_ids[13] = kernel_seq_forward(g_sprite_ids[13], false, 6, 0, 0, 1);
		kernel_seq_trigger(g_sequence_ids[13], KERNEL_TRIGGER_EXPIRE, 0, 72);
		kernel_seq_depth(g_sequence_ids[13], 5);
		if (!Sound::commandMacintoshSound(Sound::kMacSoundPlay, 1024,
				0, 0, 0, true))
			g_engine->_soundManager->command(24, 0);
		if (!Sound::commandMacintoshSound(Sound::kMacSoundPlay, 1028,
				0, 0, 0, true))
			g_engine->_soundManager->command(28, 0);
	}

	local._fridgeOpenedFl = false;
	local._fridgeOpenedDescr = false;
	local._fridgeCommentCount = 0;
	local._fridgeFirstOpenFl = true;
	local._chairDescrFl = false;
	local._activeMsgFl = false;

	int quote = QUOTE_ID(0);
	kernel.quotes = quote_load(
		quote + 0, quote + 1, quote + 2, quote + 3, quote + 4, quote + 5,
		quote + 6, quote + 7, quote + 8, quote + 9, quote + 10, 0);

	if (previous_room == 101) {
		if (!Sound::commandMacintoshSound(Sound::kMacSoundPlay, 1020,
				0, 0, 0, true))
			g_engine->_soundManager->command(20, 0);
	}
}

static void room_102_daemon() {
	if (kernel.trigger == 70)
		player.commands_allowed = true;

	if (kernel.trigger == 72) {
		g_sequence_ids[13] = kernel_seq_forward(g_sprite_ids[13], false, 6, 0, 0, 0);
		kernel_seq_range(g_sequence_ids[13], -2, -2);
		kernel_seq_depth(g_sequence_ids[13], 5);
		kernel_timing_trigger(48, 90);
	}

	if (kernel.trigger >= 90) {
		if (kernel.trigger >= 94) {
			kernel_run_animation(kernel_name('B', -1), 71);
			player.commands_allowed = false;
			player.walker_visible = false;

			global[kWaterInAPuddle] = true;
			g_engine->_soundManager->command(24, 0);
		} else {
			g_engine->_soundManager->command(23, 0);
			kernel_timing_trigger(48, kernel.trigger + 1);
		}
	}

	if (kernel.trigger == 71) {
		player.commands_allowed = true;
		player.walker_visible = true;
		player.clock = kernel.clock - player.frame_delay;
	}

	if (local._fridgeOpenedFl && !local._fridgeOpenedDescr) {
		local._fridgeCommentCount++;
		if (local._fridgeCommentCount > 16384) {
			local._fridgeOpenedDescr = true;
			text_show(10213);
		}
	}

	if (!local._activeMsgFl && (Common::Point(player.x, player.y) == Common::Point(177, 114)) && (player.facing == FACING_NORTH)
		&& (g_engine->getRandomNumber(1, 5000) == 1)) {
		kernel_message_purge();
		local._activeMsgFl = false;
		addRandomMessage();
	}

	if (kernel.trigger == 73)
		local._activeMsgFl = false;
}

static void room_102_pre_parser() {
	if (player_said_1(refrigerator) || player_said_1(poster))
		player.need_to_walk = player.ready_to_walk;

	if (local._fridgeOpenedFl && !player_said_1(refrigerator)) {
		switch (kernel.trigger) {
		case 0:
			if (player.need_to_walk) {
				kernel_seq_delete(g_sequence_ids[7]);
				g_sequence_ids[7] = kernel_seq_backward(g_sprite_ids[7], false, 6, 0, 0, 1);
				kernel_seq_trigger(g_sequence_ids[7], KERNEL_TRIGGER_EXPIRE, 0, 1);
				kernel_seq_depth(g_sequence_ids[7], 15);
				player.commands_allowed = false;
				g_engine->_soundManager->command(20, 0);
			}
			break;

		case 1:
			if (object_is_here(OBJ_BURGER)) {
				kernel_seq_delete(g_sequence_ids[10]);
				kernel_flip_hotspot(words_burger, false);
			}
			local._fridgeOpenedFl = false;
			player.commands_allowed = true;
			break;

		default:
			break;
		}
	}

	if (player.need_to_walk)
		kernel_message_purge();
}

static void room_102_parser() {
	bool justOpenedFl = false;

	if (player.look_around) {
		text_show(10234);
		goto handled;
	}

	if (player_said_1(refrigerator) && !local._fridgeOpenedFl) {
		switch (kernel.trigger) {
		case 0:
			g_sequence_ids[7] = kernel_seq_forward(g_sprite_ids[7], false, 6, 0, 0, 1);
			kernel_seq_depth(g_sequence_ids[7], 15);
			kernel_seq_trigger(g_sequence_ids[7], KERNEL_TRIGGER_EXPIRE, 0, 1);
			if (object_is_here(OBJ_BURGER)) {
				g_sequence_ids[10] = kernel_seq_forward(g_sprite_ids[10], false, 7, 0, 0, 0);
				kernel_seq_depth(g_sequence_ids[10], 14);
			}
			player.commands_allowed = false;
			g_engine->_soundManager->command(20, 0);
			goto handled;

		case 1:
			g_sequence_ids[7] = kernel_seq_forward(g_sprite_ids[7], false, 6, 0, 0, 0);
			kernel_seq_range(g_sequence_ids[7], -2, -2);
			kernel_seq_depth(g_sequence_ids[7], 15);
			int delay;
			if (player_said_1(walkto) && !local._fridgeFirstOpenFl)
				delay = 0;
			else
				delay = 48;
			kernel_timing_trigger(delay, 2);
			goto handled;

		case 2:
			local._fridgeOpenedFl = true;
			local._fridgeOpenedDescr = false;
			local._fridgeCommentCount = 0;
			player.commands_allowed = true;
			justOpenedFl = true;
			if (object_is_here(OBJ_BURGER))
				kernel_flip_hotspot(words_burger, true);
			break;

		default:
			break;
		}
	}

	if (player_said_2(look, refrigerator) || player_said_2(open, refrigerator)) {
		if (object_is_here(OBJ_BURGER))
			text_show(10230);
		else
			text_show(10229);

		local._fridgeFirstOpenFl = false;
		goto handled;
	}

	if (player_said_2(walkto, refrigerator) && justOpenedFl) {
		local._fridgeFirstOpenFl = false;
		int quoteId = g_engine->getRandomNumber(QUOTE_ID(0), QUOTE_ID(4));
		char *curQuote = quote_string(kernel.quotes, quoteId);
		int width = font_string_width(kernel_message_font, curQuote, -1);
		kernel_message_purge();
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_message_add(curQuote, 210, 60, 0x1110, 120, 73, 0);
		kernel_message_add(quote_string(kernel.quotes, QUOTE_ID(5)), 214 + width, 60, 0x1110, 120, 73, 0);
		local._activeMsgFl = true;
		goto handled;
	}

	if (player_said_2(close, refrigerator)) {
		text_show(10213);
		goto handled;
	}

	if (player_said_2(take, refrigerator)) {
		text_show(8);
		goto handled;
	}

	if (player_said_2(walk_through, door)) {
		if (g_engine->isDemo()) {
			// Not available in demo
			text_show(99);
			goto handled;
		}

		switch (kernel.trigger) {
		case 0:
			g_sequence_ids[6] = kernel_seq_forward(g_sprite_ids[6], false, 6, 0, 0, 1);
			kernel_seq_trigger(g_sequence_ids[6], KERNEL_TRIGGER_EXPIRE, 0, 1);
			player.commands_allowed = false;
			g_engine->_soundManager->command(20, 0);
			break;

		case 1:
			new_room = 101;
			break;

		default:
			break;
		}
		goto handled;
	}

	if (player_said_2(walkto, engineering_section)) {
		if (g_engine->isDemo())
			// Not available in demo
			text_show(99);
		else
			new_room = 103;
		goto handled;
	}

	if (player_said_2(walkto, poster) || player_said_2(look, poster) || player_said_2(walkto, binoculars)) {
		addRandomMessage();
		goto handled;
	}

	if (player_said_2(look, weight_machine)) {
		text_show(10212);
		goto handled;
	}

	if (player_said_2(look, engineering_section)) {
		text_show(10205);
		goto handled;
	}

	if (player_said_2(look, door)) {
		text_show(10204);
		goto handled;
	}

	if (player_said_2(stare_at, ceiling) || player_said_2(look, ceiling)) {
		text_show(10203);
		goto handled;
	}

	if (player_said_2(stare_at, overhead_lamp) || player_said_2(look, overhead_lamp)) {
		text_show(10202);
		goto handled;
	}

	if (player_said_2(look, robo_kitchen)) {
		text_show(10215);
		goto handled;
	}

	if (player_said_3(put, burger, robo_kitchen) && player_has(OBJ_BURGER)) {
		text_show(10216);
		goto handled;
	}

	if (player_said_2(put, refrigerator) && player_has(object_named(player2.words[1]))) {
		text_show(10217);
		goto handled;
	}

	if (player_said_3(put, dead_fish, robo_kitchen) || player_said_3(put, stuffed_fish, robo_kitchen)) {
		text_show(10230);
		goto handled;
	}

	if (player_said_2(open, robo_kitchen)) {
		text_show(10218);
		goto handled;
	}

	if (player_said_2(look, closet)) {
		text_show(10219);
		goto handled;
	}

	if ((player_said_1(ladder) || player_said_1(hatchway)) && (player_said_1(look) || player_said_1(climb_up) || player_said_1(climb_through))) {
		if (player_has(OBJ_REBREATHER)) {
			if (!player_said_1(climb_up) && !player_said_1(climb_through)) {
				text_show(10231);
				goto handled;
			}
		} else if (player_said_1(look) || (game.difficulty != DIFFICULTY_HARD)) {
			text_show(10222);
			goto handled;
		}
	}

	if ((player_said_1(ladder) || player_said_1(hatchway)) && (player_said_1(climb_up) || player_said_1(climb_through))) {
		switch (kernel.trigger) {
		case 0:
			kernel_run_animation(kernel_name('A', -1), 1);
			player.commands_allowed = false;
			player.walker_visible = false;
			break;

		case 1:
			g_engine->_soundManager->command(24, 0);
			kernel_timing_trigger(48, 2);
			break;

		case 2:
		case 3:
		case 4:
			g_engine->_soundManager->command(23, 0);
			kernel_timing_trigger(48, kernel.trigger + 1);
			break;

		case 5:
			g_engine->_soundManager->command(24, 0);
			kernel_timing_trigger(48, kernel.trigger + 1);
			break;

		case 6:
			if (player_has(OBJ_REBREATHER) && !player_has_been_in_room(106) && !g_engine->isDemo())
				text_show(10237);
			new_room = 106;
			break;

		default:
			break;
		}
		goto handled;
	}

	if (player_said_2(look, power_status_panel)) {
		text_show(10226);
		goto handled;
	}

	if (player_said_2(look, window) || player_said_2(look_through, window)) {
		text_show(10227);
		goto handled;
	}

	if (player_said_2(look, doorway) || player_said_2(walkto, doorway)) {
		text_show(10228);
		goto handled;
	}

	if (player_said_2(look, drawer) || ((player_said_2(close, drawer) || player_said_2(push, drawer)) && !local._drawerDescrFl)) {
		text_show(10220);
		local._drawerDescrFl = true;
		goto handled;
	}

	if (player_said_2(close, drawer) || player_said_2(push, drawer)) {
		text_show(10221);
		goto handled;
	}

	if (player_said_2(open, drawer)) {
		text_show(10236);
		goto handled;
	}

	if (player_said_2(look, chair) || (player_said_2(sit_in, chair) && !local._chairDescrFl)) {
		local._chairDescrFl = true;
		text_show(10210);
		goto handled;
	}

	if (player_said_2(sit_in, chair)) {
		text_show(10211);
		goto handled;
	}

	if (player_said_2(look, medicine_cabinet)) {
		if (global[kMedicineCabinetOpen])
			text_show(10207);
		else
			text_show(10206);

		goto handled;
	}

	if (player_said_2(close, medicine_cabinet) && global[kMedicineCabinetOpen]) {
		switch (kernel.trigger) {
		case 0:
			kernel_seq_delete(g_sequence_ids[8]);
			g_sequence_ids[8] = kernel_seq_backward(g_sprite_ids[8], false, 6, 0, 0, 1);
			kernel_seq_trigger(g_sequence_ids[8], KERNEL_TRIGGER_EXPIRE, 0, 1);
			player.commands_allowed = false;
			g_engine->_soundManager->command(21, 0);
			break;

		case 1:
			kernel_timing_trigger(48, 2);
			break;

		case 2:
			player.commands_allowed = true;
			global[kMedicineCabinetOpen] = false;
			text_show(10209);
			break;

		default:
			break;
		}
		goto handled;
	}

	if (player_said_2(open, medicine_cabinet) && !global[kMedicineCabinetOpen]) {
		switch (kernel.trigger) {
		case 0:
			g_sequence_ids[8] = kernel_seq_forward(g_sprite_ids[8], false, 6, 0, 0, 1);
			kernel_seq_trigger(g_sequence_ids[8], KERNEL_TRIGGER_EXPIRE, 0, 1);
			player.commands_allowed = false;
			g_engine->_soundManager->command(21, 0);
			break;

		case 1:
			g_sequence_ids[8] = kernel_seq_forward(g_sprite_ids[8], false, 6, 0, 0, 0);
			kernel_seq_range(g_sequence_ids[8], -2, -2);
			kernel_timing_trigger(48, 2);
			break;

		case 2:
			player.commands_allowed = true;
			global[kMedicineCabinetOpen] = true;
			if (global[kMedicineCabinetVirgin]) {
				text_show(10208);
			} else {
				text_show(10207);
			}
			global[kMedicineCabinetVirgin] = false;
			break;

		default:
			break;
		}
		goto handled;
	}

	if (player_said_2(take, binoculars) && object_is_here(OBJ_BINOCULARS)) {
		switch (kernel.trigger) {
		case 0:
			g_sequence_ids[11] = kernel_seq_pingpong(g_sprite_ids[11], false, 3, 0, 0, 1);
			kernel_seq_player(g_sequence_ids[11], false);
			kernel_seq_trigger(g_sequence_ids[11], KERNEL_TRIGGER_EXPIRE, 0, 1);
			player.walker_visible = false;
			player.commands_allowed = false;
			break;

		case 1:
			inter_give_to_player(OBJ_BINOCULARS);
			kernel_seq_delete(g_sequence_ids[9]);
			kernel_flip_hotspot(words_binoculars, false);
			player.walker_visible = true;
			player.commands_allowed = true;
			g_engine->_soundManager->command(22, 0);
			object_examine(OBJ_BINOCULARS, 10201, 0);
			break;

		default:
			break;
		}
		goto handled;
	}

	if (player_said_2(take, burger) && object_is_here(OBJ_BURGER)) {
		if (kernel.trigger == 0) {
			object_examine(OBJ_BURGER, 10235, 0);
			kernel_seq_delete(g_sequence_ids[10]);
			inter_give_to_player(OBJ_BURGER);
			kernel_flip_hotspot(words_burger, false);
			g_engine->_soundManager->command(22, 0);
			player.walker_visible = true;
			player.commands_allowed = true;
		}
		goto handled;
	}

	if (player_said_2(take, poster)) {
		text_show(10224);
		goto handled;
	}

	if ((player_said_1(push) || player_said_1(pull)) && player_said_1(weight_machine)) {
		text_show(10225);
		goto handled;
	}

	if (player_said_2(look, floor)) {
		text_show(10232);
		goto handled;
	}

	if (player_said_2(look, binoculars) && !player_has(OBJ_BINOCULARS)) {
		text_show(10233);
		goto handled;
	}

	if (player_said_2(look, burger) && (player.main_object_source == STROKE_INTERFACE)) {
		text_show(801);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;
done:
	;
}

static void room_102_error() {
	if (player_said_2(put, robo_kitchen) && player_has(object_named(player2.words[1]))) {
		text_show(10217);
		player.command_ready = false;
	}
}

void room_102_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._fridgeOpenedFl);
	s.syncAsByte(local._fridgeOpenedDescr);
	s.syncAsByte(local._fridgeFirstOpenFl);
	s.syncAsByte(local._chairDescrFl);
	s.syncAsByte(local._drawerDescrFl);
	s.syncAsByte(local._activeMsgFl);

	s.syncAsSint16LE(local._fridgeCommentCount);
}

void room_102_preload() {
	room_init_code_pointer = room_102_init;
	room_pre_parser_code_pointer = room_102_pre_parser;
	room_parser_code_pointer = room_102_parser;
	room_daemon_code_pointer = room_102_daemon;
	room_error_code_pointer = room_102_error;

	anim_himem_preload(kernel_name('A', -1), 3);

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
