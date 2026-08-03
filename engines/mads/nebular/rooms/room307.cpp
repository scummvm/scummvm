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
#include "mads/core/matte.h"
#include "mads/core/pal.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section3.h"
#include "mads/nebular/rooms/forcefield.h"
#include "mads/nebular/rooms/dialog.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _afterPeeingFl;
	bool _duringPeeingFl;
	bool _grateOpenedFl;
	bool _activePrisonerFl;
 	int32 _animationMode;
	int32 _prisonerMessageId;
	int32 _fieldCollisionCounter;
	int32 _lastFrameTime;
	int32 _guardTime;
	int32 _prisonerTimer;
	char _subQuote2[256];

	Dialog _dialog1;
	Dialog _dialog2;
	Forcefield _forcefield;
};

static Scratch local;


static void handleRexDialog(int quote) {
	char *curQuote = quote_string(kernel.quotes, quote);
	if (font_string_width(kernel_message_font, curQuote, kernel_message_spacing) > 200) {
		static char subQuote1[34], subQuote2[34];
		quote_split_string(curQuote, subQuote1, subQuote2);
		Common::strcpy_s(local._subQuote2, subQuote2);

		kernel_message_add(subQuote1, 0, -14, 0x1110, 240, 0, 34);
		kernel_message_add(local._subQuote2, 0, 0, 0x1110, 180, 1, 34);
	} else
		kernel_message_add(curQuote, 0, 0, 0x1110, 120, 1, 34);
}

static void handlePrisonerSpeech(int firstQuoteId, int number, uint32 timeout) {
	int height = number * 14;
	int posY;

	if (height < 60)
		posY = 65 - height;
	else
		posY = 78 - (height / 2);

	kernel_message_purge();
	local._activePrisonerFl = true;

	int quoteId = firstQuoteId;
	for (int count = 0; count < number; count++) {
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_message_add(quote_string(kernel.quotes, quoteId), 5, posY, 0xFDFC, timeout, 81, 0);
		posY += 14;
		quoteId++;
	}
}

static void setDialogNode(int node) {
	switch (node) {
	case 0:
		handlePrisonerSpeech(0x153, 2, 120);
		kernel_set_interface_mode(INTER_BUILDING_SENTENCES);
		break;

	case 1:
		global[kMetBuddyBeast] = true;
		handlePrisonerSpeech(0x10F, 2, INDEFINITE_TIMEOUT);
		local._dialog1.start();
		break;

	case 2:
		global[kMetBuddyBeast] = true;
		handlePrisonerSpeech(0x111, 2, INDEFINITE_TIMEOUT);
		local._dialog1.start();
		break;

	case 4:
		handlePrisonerSpeech(0x116, 1, 120);
		kernel_set_interface_mode(INTER_BUILDING_SENTENCES);
		break;

	case 5:
		global[kKnowsBuddyBeast] = true;
		handlePrisonerSpeech(0x117, 2, INDEFINITE_TIMEOUT);
		local._dialog2.start();
		break;

	case 6:
		handlePrisonerSpeech(0x123, 1, 120);
		kernel_set_interface_mode(INTER_BUILDING_SENTENCES);
		break;

	case 7:
		global[kKnowsBuddyBeast] = true;
		handlePrisonerSpeech(0x124, 10, INDEFINITE_TIMEOUT);
		local._dialog2.write(0x11A, false);
		local._dialog2.write(0x11B, true);
		local._dialog2.write(0x120, true);
		local._dialog2.start();
		break;

	case 8:
		handlePrisonerSpeech(0x12E, 6, INDEFINITE_TIMEOUT);
		local._dialog2.write(0x11A, false);
		local._dialog2.write(0x11B, false);
		local._dialog2.write(0x11C, true);
		local._dialog2.write(0x11D, true);
		local._dialog2.write(0x11F, true);
		local._dialog2.start();
		break;

	case 9:
		handlePrisonerSpeech(0x134, 4, INDEFINITE_TIMEOUT);
		local._dialog2.write(0x11D, false);
		local._dialog2.start();
		break;

	case 10:
		handlePrisonerSpeech(0x138, 6, INDEFINITE_TIMEOUT);
		local._dialog2.write(0x11E, false);
		local._dialog2.start();
		break;

	case 11:
		handlePrisonerSpeech(0x13E, 6, INDEFINITE_TIMEOUT);
		local._dialog2.write(0x11F, false);
		local._dialog2.write(0x121, true);
		local._dialog2.start();
		break;

	case 12:
		handlePrisonerSpeech(0x144, 4, INDEFINITE_TIMEOUT);
		local._dialog2.write(0x11C, false);
		local._dialog2.start();
		break;

	case 13:
		handlePrisonerSpeech(0x148, 7, INDEFINITE_TIMEOUT);
		local._dialog2.write(0x120, false);
		local._dialog2.start();
		break;

	case 14:
		handlePrisonerSpeech(0x14F, 3, INDEFINITE_TIMEOUT);
		local._dialog2.write(0x121, false);
		local._dialog2.start();
		break;

	case 15:
		handlePrisonerSpeech(0x152, 1, 120);
		kernel_set_interface_mode(INTER_BUILDING_SENTENCES);
		break;

	case 16:
		global[kKnowsBuddyBeast] = true;
		handlePrisonerSpeech(0x10C, 1, INDEFINITE_TIMEOUT);
		local._dialog2.start();
		break;

	default:
		break;
	}
}

static void handlePrisonerEncounter() {
	switch (player2.words[0]) {
	case 275:
		setDialogNode(5);
		break;

	case 277:
		setDialogNode(4);
		break;

	case 276:
		setDialogNode(6);
		break;

	default:
		break;
	}
}

static void handlePrisonerDialog() {
	switch (player2.words[0]) {
	case 0x11A:
		setDialogNode(7);
		break;

	case 0x11B:
		setDialogNode(8);
		break;

	case 0x11C:
		setDialogNode(12);
		break;

	case 0x11D:
		setDialogNode(9);
		break;

	case 0x11E:
		setDialogNode(10);
		break;

	case 0x11F:
		setDialogNode(11);
		break;

	case 0x120:
		setDialogNode(13);
		break;

	case 0x121:
		setDialogNode(14);
		break;

	case 0x122:
		setDialogNode(15);
		break;

	default:
		break;
	}
}

static void handleDialog() {
	if (kernel.trigger == 0) {
		kernel_message_purge();
		player.commands_allowed = false;
		handleRexDialog(player2.words[0]);
	} else {
		player.commands_allowed = true;

		if (!global[kKnowsBuddyBeast]) {
			handlePrisonerEncounter();
		} else {
			handlePrisonerDialog();
		}
	}
}

static void room_307_init() {
	g_sprite_ids[1] = kernel_load_series("*SC003x0", 0);
	g_sprite_ids[0] = kernel_load_series("*SC003x1", 0);
	g_sprite_ids[2] = kernel_load_series("*SC003x2", 0);
	g_sprite_ids[4] = kernel_load_series(kernel_name('x', 0), 0);

	init_forcefield(&local._forcefield, true);

	g_sequence_ids[4] = kernel_seq_stamp(g_sprite_ids[4], false, 1);
	kernel_seq_loc(g_sequence_ids[4], 127, 78);
	kernel_seq_depth(g_sequence_ids[4], 15);

	local._animationMode = 0;
	local._fieldCollisionCounter = 0;

	kernel_load_variant(1);

	kernel.quotes = quote_load(237, 238, 239, 240, 241, 242, 243, 268, 260, 262, 263, 264, 261,
		265, 266, 267, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279,
		280, 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292, 293,
		294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307,
		308, 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320, 321,
		322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 334, 335,
		336, 337, 338, 339, 0);

	local._dialog1.setup(0x3F, 0x113, 0x114, 0x115, -1);
	local._dialog2.setup(0x40, 0x11A, 0x11B, 0x11C, 0x11D, 0x11E, 0x11F, 0x120, 0x121, 0x122, 0);

	if (!player.been_here_before)
		local._dialog2.set(0x11A, 0x122, 0);
	else if (previous_room == 318)
		local._dialog2.write(0x11E, true);


	if (previous_room == KERNEL_RESTORING_GAME) {
		if (local._grateOpenedFl)
			g_engine->_soundManager->command(10, 0);
		else
			g_engine->_soundManager->command(3, 0);
	} else {
		local._afterPeeingFl = false;
		local._duringPeeingFl = false;
		local._guardTime = 0;
		local._grateOpenedFl = false;
		local._activePrisonerFl = false;
		local._prisonerTimer = 0;
		local._prisonerMessageId = 0x104;

		if (previous_room == 308) {
			player.walker_visible = false;
			player.commands_allowed = false;
			player.x = 156;
			player.y = 113;
			player.facing = FACING_NORTH;
			local._animationMode = 1;
			g_engine->_soundManager->command(11, 0);
			kernel_run_animation(kernel_name('a', -1), 60);
		} else if (previous_room == 387) {
			player.x = 129;
			player.y = 108;
			player.facing = FACING_NORTH;
			g_engine->_soundManager->command(3, 0);
			local._grateOpenedFl = true;
		} else {
			player.x = 159;
			player.y = 109;
			player.facing = FACING_SOUTH;
			g_engine->_soundManager->command(3, 0);
		}
	}

	if (local._grateOpenedFl) {
		kernel_flip_hotspot(17, false);

		int idx = kernel_add_dynamic(words_air_vent, words_climb_into, 0, -1, 117, 67, 19, 13);
		kernel_dynamic_walk(idx, 129, 104, FACING_NORTH);
		int hotspotId = idx;
		kernel_dynamic_cursor(hotspotId, CURSOR_UP);

		kernel_seq_delete(g_sequence_ids[4]);
		g_sequence_ids[4] = kernel_seq_stamp(g_sprite_ids[4], false, 2);
		kernel_seq_loc(g_sequence_ids[4], 127, 78);
		kernel_seq_depth(g_sequence_ids[4], 15);
	}

	pal_change_color(252, 63, 30, 20);
	pal_change_color(253, 45, 15, 12);

	section_3_music();

	if ((previous_room == 318) || (previous_room == 387))
		kernel_message_player(0xF3, 120, 0);
}

static void room_307_daemon() {
	handle_forcefield(&local._forcefield, &g_sprite_ids[0]);

	if ((local._animationMode == 1) && (kernel_anim[0].anim != nullptr)) {
		if (kernel_anim[0].frame == 126) {
			local._forcefield._flag = false;
			g_engine->_soundManager->command(5, 0);
		}

		if (kernel_anim[0].frame == 194) {
			local._forcefield._flag = true;
			g_engine->_soundManager->command(24, 0);
		}
	}

	if ((local._animationMode == 2) && (kernel_anim[0].anim != nullptr)) {
		if (kernel_anim[0].frame == 54)
			local._forcefield._flag = false;

		if (kernel_anim[0].frame == 150) {
			player.walker_visible = false;
			player.clock = kernel.clock - player.frame_delay;
		}
	}

	if (kernel.trigger == 60) {
		player.walker_visible = true;
		player.commands_allowed = true;
		player.clock = kernel.clock - player.frame_delay;
		local._animationMode = 0;
		g_engine->_soundManager->command(9, 0);
	}

	if ((local._lastFrameTime != kernel.clock) && !local._duringPeeingFl) {
		int32 elapsedTime = local._lastFrameTime - kernel.clock;
		if ((elapsedTime > 0) && (elapsedTime <= 4)) {
			local._guardTime += elapsedTime;
			local._prisonerTimer += elapsedTime;
		} else {
			local._guardTime++;
			local._prisonerTimer++;
		}
		local._lastFrameTime = kernel.clock;

		if ((local._guardTime > 3000) && !local._duringPeeingFl && (kernel_anim[0].anim == nullptr)
			&& (inter_input_mode != INTER_CONVERSATION) && global[kMetBuddyBeast] && !local._activePrisonerFl) {
			if (!player_has(OBJ_SCALPEL) && !local._grateOpenedFl) {
				player.commands_allowed = false;
				player_walk(151, 119, FACING_SOUTHEAST);
				local._animationMode = 2;
				g_engine->_soundManager->command(11, 0);
				kernel_run_animation(kernel_name('b', -1), 70);
			}
			local._guardTime = 0;
		} else if ((local._prisonerTimer > 300) && (inter_input_mode != INTER_CONVERSATION) && (kernel_anim[0].anim == nullptr) && !local._activePrisonerFl) {
			if (!global[kMetBuddyBeast]) {
				if (local._prisonerMessageId == -1)
					local._prisonerMessageId = 0x104;

				int idx = kernel_message_add(quote_string(kernel.quotes, local._prisonerMessageId), 5, 51, 0xFDFC, 120, 81, 0);
				kernel_message_teletype(idx, 4, true);
				local._prisonerMessageId++;
				if (local._prisonerMessageId > 0x10A)
					local._prisonerMessageId = 0x104;
			} else if (global[kKnowsBuddyBeast] && (local._dialog2.read(0) > 1) && (g_engine->getRandomNumber(1, 3) == 1)) {
				int idx = kernel_message_add(quote_string(kernel.quotes, 267), 5, 51, 0xFDFC, 120, 81, 0);
				kernel_message_teletype(idx, 4, true);
			}
			local._prisonerTimer = 0;
		}
	}

	if (kernel.trigger == 70)
		new_room = 318;

	if (kernel.trigger == 81) {
		local._prisonerTimer = 0;
		if (local._activePrisonerFl && (local._guardTime > 2600))
			local._guardTime = 3000 - g_engine->getRandomNumber(1, 800);

		local._activePrisonerFl = false;
	}
}

static void room_307_parser() {
	if (player.look_around)
		text_show(30715);
	else if (inter_input_mode == INTER_CONVERSATION)
		handleDialog();
	else if (player_said_2(talkto, cell_wall) || player_said_2(talkto, wall) || player_said_2(talkto, toilet)) {
		int node, say;
		if (global[kKnowsBuddyBeast]) {
			say = 0x10E;
			node = 16;
		} else if (global[kMetBuddyBeast]) {
			say = 0x10E;
			node = 2;
		} else {
			say = 0x10D;
			node = 1;
		}

		switch (kernel.trigger) {
		case 0:
			handleRexDialog(say);
			break;

		case 1:
			setDialogNode(node);
			break;

		default:
			break;
		}
	} else if (player_said_3(pry, scalpel, air_vent)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			kernel_message_purge();
			kernel_message_add(quote_string(kernel.quotes, 239), 0, 0, 0x1110, 120, 0, 34);
			kernel_timing_trigger(120, 1);
			break;

		case 1:
			g_sprite_ids[5] = kernel_load_series("*RXCL_8", 0);
			player.walker_visible = false;
			g_sequence_ids[5] = kernel_seq_forward(g_sprite_ids[5], false, 12, 0, 0, 1);
			kernel_seq_range(g_sequence_ids[5], -1, 3);
			kernel_seq_player(g_sequence_ids[5], false);
			kernel_seq_trigger(g_sequence_ids[5], KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2:
		{
			int oldIdx = g_sequence_ids[5];
			g_sequence_ids[5] = kernel_seq_pingpong(g_sprite_ids[5], false, 12, 0, 0, 6);
			kernel_seq_player(g_sequence_ids[5], false);
			kernel_seq_range(g_sequence_ids[5], 2, 3);
			kernel_seq_timeout(oldIdx, g_sequence_ids[5]);
			kernel_seq_trigger(g_sequence_ids[5], KERNEL_TRIGGER_EXPIRE, 0, 3);
		}
		break;

		case 3:
		{
			int oldIdx = g_sequence_ids[5];
			g_sequence_ids[5] = kernel_seq_stamp(g_sprite_ids[5], false, 1);
			kernel_seq_player(g_sequence_ids[5], false);
			kernel_seq_timeout(oldIdx, g_sequence_ids[5]);
			kernel_timing_trigger(48, 4);
		}
		break;

		case 4:
			g_engine->_soundManager->command(26, 0);
			kernel_seq_delete(g_sequence_ids[4]);
			g_sequence_ids[4] = kernel_seq_stamp(g_sprite_ids[4], false, 2);
			kernel_seq_loc(g_sequence_ids[4], 127, 78);
			kernel_seq_depth(g_sequence_ids[4], 15);
			kernel_timing_trigger(90, 5);
			break;

		case 5:
			g_engine->_soundManager->command(10, 0);
			kernel_message_purge();
			kernel_message_add(quote_string(kernel.quotes, 241), 0, 0, 0x1110, 120, 0, 34);
			kernel_timing_trigger(120, 6);
			break;

		case 6:
		{
			player.walker_visible = true;
			player.clock = kernel.clock - player.frame_delay;
			kernel_seq_delete(g_sequence_ids[5]);
			local._grateOpenedFl = true;
			kernel_flip_hotspot(17, false);
			int idx = kernel_add_dynamic(words_air_vent, words_climb_into, 0, -1, 117, 67, 19, 13);
			kernel_dynamic_walk(idx, 129, 104, FACING_NORTH);
			int hotspotId = idx;
			kernel_dynamic_cursor(hotspotId, CURSOR_UP);
			inter_take_from_player(OBJ_SCALPEL, NOWHERE);
			kernel_message_player(0xF2, 120, 7);
		}
		break;

		case 7:
			matte_deallocate_series(g_sprite_ids[5], true);
			player.commands_allowed = true;
			break;

		default:
			break;
		}
	} else if (player_said_2(climb_into, air_vent)) {
		if (local._grateOpenedFl) {
			switch (kernel.trigger) {
			case 0:
				g_sprite_ids[5] = kernel_load_series("*RXCL_8", 0);
				player.commands_allowed = false;
				player.walker_visible = false;
				kernel_seq_delete(g_sequence_ids[4]);
				g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 60, 0, 0, 1);
				kernel_seq_range(g_sequence_ids[4], 3, -2);
				kernel_seq_loc(g_sequence_ids[4], 127, 78);
				kernel_seq_depth(g_sequence_ids[4], 15);
				g_sequence_ids[5] = kernel_seq_forward(g_sprite_ids[5], false, 18, 0, 0, 1);
				kernel_seq_range(g_sequence_ids[5], -1, 4);
				kernel_seq_player(g_sequence_ids[5], false);
				kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 1);
				kernel_seq_trigger(g_sequence_ids[5], KERNEL_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				g_sequence_ids[4] = kernel_seq_stamp(g_sprite_ids[4], false, -2);
				kernel_seq_loc(g_sequence_ids[4], 127, 78);
				kernel_seq_depth(g_sequence_ids[4], 15);
				break;

			case 2:
			{
				int oldIdx = g_sequence_ids[5];
				g_sequence_ids[5] = kernel_seq_forward(g_sprite_ids[5], false, 12, 0, 0, 1);
				kernel_seq_range(g_sequence_ids[5], 4, 10);
				kernel_seq_player(g_sequence_ids[5], false);
				kernel_seq_timeout(oldIdx, g_sequence_ids[5]);
				kernel_seq_trigger(g_sequence_ids[5], KERNEL_TRIGGER_EXPIRE, 0, 3);
			}
			break;

			case 3:
				kernel_seq_delete(g_sequence_ids[4]);
				g_sequence_ids[4] = kernel_seq_stamp(g_sprite_ids[4], false, 3);
				kernel_seq_loc(g_sequence_ids[4], 127, 78);
				kernel_seq_depth(g_sequence_ids[4], 1);
				g_sequence_ids[5] = kernel_seq_stamp(g_sprite_ids[5], false, 11);
				kernel_seq_player(g_sequence_ids[5], false);
				kernel_seq_loc(g_sequence_ids[5], 129, 102);
				kernel_timing_trigger(48, 4);
				break;

			case 4:
				kernel_seq_delete(g_sequence_ids[4]);
				g_sequence_ids[4] = kernel_seq_stamp(g_sprite_ids[4], false, 2);
				kernel_seq_loc(g_sequence_ids[4], 127, 78);
				kernel_seq_depth(g_sequence_ids[4], 1);
				kernel_seq_delete(g_sequence_ids[5]);
				g_sequence_ids[5] = kernel_seq_forward(g_sprite_ids[5], false, 12, 0, 0, 1);
				kernel_seq_range(g_sequence_ids[5], 12, 14);
				kernel_seq_player(g_sequence_ids[5], false);
				kernel_seq_loc(g_sequence_ids[5], 129, 102);
				kernel_seq_trigger(g_sequence_ids[5], KERNEL_TRIGGER_EXPIRE, 0, 5);
				break;

			case 5:
				g_sequence_ids[5] = kernel_seq_stamp(g_sprite_ids[5], false, 15);
				kernel_seq_player(g_sequence_ids[5], false);
				kernel_seq_loc(g_sequence_ids[5], 129, 102);
				kernel_timing_trigger(48, 6);
				break;

			case 6:
				kernel_seq_delete(g_sequence_ids[5]);
				kernel_seq_delete(g_sequence_ids[4]);
				g_sequence_ids[4] = kernel_seq_stamp(g_sprite_ids[4], false, 1);
				kernel_seq_loc(g_sequence_ids[4], 127, 78);
				kernel_seq_depth(g_sequence_ids[4], 1);
				kernel_timing_trigger(48, 7);
				break;

			case 7:
				new_room = 313;
				break;

			default:
				break;
			}
		}
	} else if (player_said_2(use, toilet) && (config_file.naughtiness != NAUGHTY))
		text_show(30723);
	else if (player_said_2(use, toilet)) {
		if (!local._afterPeeingFl) {
			switch (kernel.trigger) {
			case 0:
				g_engine->_soundManager->command(25, 0);
				g_sprite_ids[3] = kernel_load_series(kernel_name('a', 0), 0);
				local._duringPeeingFl = true;
				player.commands_allowed = false;
				player.walker_visible = false;
				g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 9, 0, 0, 1);
				kernel_seq_range(g_sequence_ids[3], -1, 2);
				kernel_seq_depth(g_sequence_ids[3], 9);
				kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1:
				g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 9, 0, 0, 5);
				kernel_seq_range(g_sequence_ids[3], 3, -2);
				kernel_seq_depth(g_sequence_ids[3], 9);
				kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 2);
				break;

			case 2:
				kernel_seq_timeout(g_sequence_ids[3], -1);
				player.walker_visible = true;
				kernel_timing_trigger(48, 3);
				break;

			case 3:
			{
				matte_deallocate_series(g_sprite_ids[3], true);
				kernel_message_purge();
				int idx = kernel_message_add(quote_string(kernel.quotes, 237), 0, 0, 0x1110, 120, 4, 34);
				kernel_message_teletype(idx, 4, true);
			}
			break;

			case 4:
				player.commands_allowed = true;
				local._duringPeeingFl = false;
				local._afterPeeingFl = true;
				break;

			default:
				break;
			}
		} else {
			kernel_message_purge();
			int idx = kernel_message_add(quote_string(kernel.quotes, 238), 85, 39, 0x1110, 180, 0, 0);
			kernel_message_teletype(idx, 4, true);
		}
	} else if (player_said_2(look, air_vent)) {
		if (!local._grateOpenedFl)
			text_show(30710);
		else
			text_show(30711);
	} else if (player_said_2(look, bed))
		text_show(30712);
	else if (player_said_2(look, sink))
		text_show(30713);
	else if (player_said_2(look, toilet))
		text_show(30714);
	else if (player_said_2(sharpen, scalpel))
		text_show(30716);
	else if (player_said_2(look, cell_wall))
		text_show(30717);
	else if (player_said_2(look, light))
		text_show(30718);
	else if (player_said_2(walk_into, corridor)) {
		switch (local._fieldCollisionCounter) {
		case 0:
			text_show(30719);
			local._fieldCollisionCounter = 1;
			break;

		case 1:
			text_show(30720);
			local._fieldCollisionCounter = 2;
			break;

		case 2:
			text_show(30721);
			local._fieldCollisionCounter = 3;
			break;

		case 3:
			text_show(30722);
			break;

		default:
			break;
		}
	} else
		return;

	player.command_ready = false;
}

void room_307_synchronize(Common::Serializer &s) {
	local._forcefield.synchronize(s);

	s.syncAsByte(local._afterPeeingFl);
	s.syncAsByte(local._duringPeeingFl);
	s.syncAsByte(local._grateOpenedFl);
	s.syncAsByte(local._activePrisonerFl);

	s.syncAsSint32LE(local._animationMode);
	s.syncAsSint32LE(local._prisonerMessageId);
	s.syncAsSint32LE(local._fieldCollisionCounter);

	s.syncAsUint32LE(local._lastFrameTime);
	s.syncAsUint32LE(local._guardTime);
	s.syncAsUint32LE(local._prisonerTimer);

	Common::String subQuote2 = local._subQuote2;
	s.syncString(subQuote2);
	if (s.isLoading())
		Common::strcpy_s(local._subQuote2, subQuote2.c_str());
}

void room_307_preload() {
	local._forcefield.init();

	room_init_code_pointer = room_307_init;
	room_parser_code_pointer = room_307_parser;
	room_daemon_code_pointer = room_307_daemon;

	section_3_walker();
	section_3_interface();
	vocab_make_active(words_air_vent);
	vocab_make_active(words_climb_into);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
