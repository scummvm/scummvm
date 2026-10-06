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
#include "mads/core/mcga.h"
#include "mads/core/pal.h"
#include "mads/core/timer.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section3.h"
#include "mads/nebular/rooms/dialog.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int32 _dropTimer;
	int32 _lastFrame;
	int32 _animMode;
	int32 _internCounter;
	int32 _counter;
	bool _dialogFl;
	bool _internTalkingFl;
	bool _internWalkingFl;
	bool _internVisibleFl;
	bool _explosionFl;
	int32 _lastFrameCounter;
	Dialog _dialog1;
};

static Scratch local;


static void handleRexDialogs(int quote) {
	kernel_message_purge();

	char *curQuote = quote_string(kernel.quotes, quote);
	if (g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing) > 200) {
		static char subQuote1[34], subQuote2[34];
		quote_split_string(curQuote, subQuote1, subQuote2);

		kernel_message_add(subQuote1, 138, 59, 0x1110, 240, 0, 32);
		kernel_message_add(subQuote2, 138, 73, 0x1110, 180, 1, 32);
	} else
		kernel_message_add(curQuote, 138, 73, 0x1110, 120, 1, 32);
}

static void handleInternDialog(int quoteId, int quoteNum, uint32 timeout) {
	int height = quoteNum * 14;
	int posY;
	if (height < 85)
		posY = 87 - height;
	else
		posY = 2;

	int curQuoteId = quoteId;

	int maxWidth = 0;
	for (int i = 0; i < quoteNum; i++) {
		maxWidth = MAX(maxWidth, g_engine->getMessageTextWidth(kernel_message_font, quote_string(kernel.quotes, curQuoteId), -1));
		curQuoteId++;
	}

	int posX = MIN(319 - maxWidth, 178 - (maxWidth >> 1));
	curQuoteId = quoteId;

	kernel_message_purge();
	local._internTalkingFl = true;

	// WORKAROUND: In case the player launches multiple talk selections with the
	// intern before previous ones have finished, take care of removing any
	int seqIndex;
	while ((seqIndex = kernel_seq_find_by_trigger(63)) != -1)
		kernel_seq_delete(seqIndex);

	for (int i = 0; i < quoteNum; i++) {
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(180, 63);
		kernel_message_add(quote_string(kernel.quotes, curQuoteId), posX, posY, 0xFDFC, timeout, 0, 0);
		posY += 14;
		curQuoteId++;
	}
}

static void handleDialog() {
	if (!kernel.trigger) {
		player.commands_allowed = false;
		handleRexDialogs(player2.words[0]);
	} else if (kernel.trigger == 2) {
		int synxIdx = g_sequence_ids[2];
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 1);
		kernel_seq_depth(g_sequence_ids[2], 1);
		kernel_seq_loc(g_sequence_ids[2], 142, 121);
		kernel_seq_timeout(synxIdx, g_sequence_ids[2]);
		g_engine->_soundManager->command(3, 0);
		kernel_set_interface_mode(INTER_BUILDING_SENTENCES);
		player.commands_allowed = true;
	} else {
		if (player2.words[0] < words_grass)
			local._dialog1.write(player2.words[0], false);

		switch (player2.words[0]) {
		case words_wear:
			handleInternDialog(0x19E, 2, INDEFINITE_TIMEOUT);
			local._dialog1.write(0x192, true);
			break;

		case words_weather_station:
			handleInternDialog(0x1A0, 5, INDEFINITE_TIMEOUT);
			local._dialog1.write(0x193, true);
			break;

		case words_weather_vane:
			handleInternDialog(0x1A5, 4, INDEFINITE_TIMEOUT);
			local._dialog1.write(0x194, true);
			break;

		case words_weight_machine:
			handleInternDialog(0x1A9, 6, INDEFINITE_TIMEOUT);
			local._dialog1.write(0x195, true);
			local._dialog1.write(0x196, true);
			local._dialog1.write(0x19D, false);
			break;

		case words_western_cliff_face:
			handleInternDialog(0x1AF, 7, INDEFINITE_TIMEOUT);
			if (!local._dialog1.read(0x196))
				local._dialog1.write(0x197, true);
			break;

		case words_wheel:
			handleInternDialog(0x1B6, 5, INDEFINITE_TIMEOUT);
			if (!local._dialog1.read(0x195))
				local._dialog1.write(0x197, true);
			break;

		case words_window:
			handleInternDialog(0x1BB, 5, INDEFINITE_TIMEOUT);
			break;

		case words_witchdoctor_hut:
			handleInternDialog(0x1C0, 5, INDEFINITE_TIMEOUT);
			local._dialog1.write(0x19A, true);
			break;

		case words_workbench:
			handleInternDialog(0x1C5, 3, INDEFINITE_TIMEOUT);
			break;

		case words_write_on:
			handleInternDialog(0x1C8, 5, INDEFINITE_TIMEOUT);
			local._dialog1.write(0x19B, true);
			break;

		case words_yellow_birdy:
			handleInternDialog(0x1CD, 3, INDEFINITE_TIMEOUT);
			break;

		case words_grass:
		case words_bouncing_reptile:
			kernel_seq_delete(g_sequence_ids[2]);
			g_sequence_ids[2] = kernel_seq_backward(g_sprite_ids[2], false, 8, 0, 0, 1);
			kernel_seq_depth(g_sequence_ids[2], 1);
			kernel_seq_loc(g_sequence_ids[2], 142, 121);
			kernel_seq_range(g_sequence_ids[2], 6, 8);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 2);

			local._dialogFl = false;
			handleInternDialog(0x1D0, 1, 120);
			if (local._dialog1.read(0) || (player2.words[0] == words_bouncing_reptile)) {
				local._explosionFl = true;
				local._internCounter = 3420;
			}
			break;

		default:
			break;
		}

		if (player2.words[0] < words_grass) {
			local._dialog1.start();
			player.commands_allowed = true;
		}

	}
}

static void room_318_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('x', 0), 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('k', -1), 0);

	if (global[kAfterHavoc]) {
		kernel_run_animation(kernel_name('f', -1), 0);
		kernel_anim[0].repeat = true;
	} else if (!global[kHasSeenProfPyro]) {
		kernel_flip_hotspot(words_professors_gurney, false);
		kernel_flip_hotspot(words_professor, false);
		kernel_flip_hotspot(words_tape_player, false);
	}

	if (object_is_here(OBJ_SCALPEL)) {
		g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 6, 120, 0, 0);
		kernel_seq_depth(g_sequence_ids[3], 4);
		kernel_add_dynamic(words_scalpel, words_take, 0, g_sequence_ids[3], 0, 0, 0, 0);
	}

	if (previous_room == 357) {
		player.x = 15;
		player.y = 110;
	}
	else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 214;
		player.y = 152;
	}

	local._dialog1.setup(0x47, 0x191, 0x192, 0x193, 0x194, 0x195, 0x196, 0x197, 0x198, 0x199, 0x19A, 0x19B, 0x19C, 0x19D, 0);

	if (!player.been_here_before) {
		local._dialog1.set(0x191, 0x198, 0x199, 0x19C, 0);
		if (kernel.cheating >= 2)
			local._dialog1.write(0x19D, true);
	}

	if (previous_room == 307) {
		player.walker_visible = false;
		player.commands_allowed = false;
		kernel_run_animation(kernel_name('a', -1), 60);
		local._animMode = 1;
	}

	local._lastFrame = 0;
	kernel_flip_hotspot(words_intern, false);

	if (previous_room != KERNEL_RESTORING_GAME) {
		local._dialogFl = false;
		local._internWalkingFl = false;
		local._counter = 0;
		local._internCounter = 0;
		local._internVisibleFl = true;
		local._explosionFl = false;
	}

	kernel.quotes = quote_load(396, 397, 398, 399, 401, 402, 403, 404, 405, 406,
		407, 408, 409, 410, 411, 412, 414, 415, 416, 417, 418, 419,
		420, 421, 422, 423, 424, 425, 426, 427, 428, 429, 430, 431,
		432, 433, 434, 435, 436, 437, 438, 439, 440, 441, 442, 443,
		444, 445, 446, 447, 448, 449, 450, 451, 452, 453, 454, 455,
		456, 457, 458, 459, 460, 461, 462, 463, 464, 465, 466, 467,
		400, 413, 0);

	if ((previous_room == KERNEL_RESTORING_GAME) || (((previous_room == 318) ||
		(previous_room == KERNEL_STARTING_GAME)) && (!global[kAfterHavoc]))) {
		if (!global[kAfterHavoc]) {
			player.walker_visible = false;
			g_sprite_ids[2] = kernel_load_series(kernel_name('g', -1), 0);
			local._animMode = 2;

			if (player_has_been_in_room(319) || !local._internVisibleFl) {
				local._internVisibleFl = false;
				local._dialogFl = false;
			} else {
				kernel_run_animation(kernel_name('b', -1), 61);
				kernel_flip_hotspot(words_intern, true);
			}

			if (local._dialogFl) {
				local._dialog1.start();
				g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 8);
			} else
				g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 1);

			kernel_seq_loc(g_sequence_ids[2], 142, 121);
			kernel_seq_depth(g_sequence_ids[2], 1);
		}
	}

	if (previous_room == 319) {
		player.commands_allowed = false;
		player.walker_visible = false;
		local._animMode = 4;
		if (!global[kHasSeenProfPyro]) {
			kernel_run_animation(kernel_name('d', -1), 64);
			global[kHasSeenProfPyro] = true;
		} else {
			kernel_run_animation(kernel_name('e', -1), 64);
		}
	}

	local._internTalkingFl = false;
	pal_change_color(252, 63, 63, 10);
	pal_change_color(253, 45, 45, 05);

	local._dropTimer = kernel.clock;
	section_3_music();

	if (local._dialogFl)
		g_engine->_soundManager->command(15, 0);
}

static void room_318_daemon() {
	if ((kernel_anim[0].anim != nullptr) && (local._animMode == 2)) {
		if (local._lastFrame != kernel_anim[0].frame) {
			local._lastFrame = kernel_anim[0].frame;
			int nextFrame = -1;

			switch (local._lastFrame) {
			case 20:
			case 30:
			case 40:
			case 50:
			case 60:
			case 70:
			case 80:
			case 90:
			case 100:
			case 110:
			case 120:
			case 130:
			case 140:
			case 149:
				if (local._internWalkingFl) {
					nextFrame = 149;
				} else if (local._internTalkingFl) {
					nextFrame = 149;
				} else if (local._lastFrame == 149) {
					nextFrame = 4;
				}
				break;

			case 151:
				if (local._internWalkingFl)
					nextFrame = 183;
				break;

			case 167:
			case 184:
				if (local._internWalkingFl) {
					nextFrame = 184;
				} else if (!local._internTalkingFl) {
					nextFrame = 0;
				} else if (g_engine->getRandomNumber(1, 100) <= 50) {
					nextFrame = 151;
				} else {
					nextFrame = 167;
				}

				if (nextFrame == 184) {
					handleInternDialog(0x1D1, 3, 240);
					kernel_flip_hotspot(words_intern, false);
					local._internVisibleFl = false;
				}
				break;

			default:
				break;
			}

			if ((nextFrame >= 0) && (nextFrame != kernel_anim[0].frame)) {
				kernel_reset_animation(0, nextFrame);
				local._lastFrame = nextFrame;
			}
		}
	}

	switch (kernel.trigger) {
	case 60:
		g_engine->_soundManager->command(3, 0);
		local._animMode = 2;
		kernel.force_restart = true;
		break;

	case 61:
		local._counter = 0;
		break;

	case 62:
		new_room = 319;
		break;

	case 63:
		local._internTalkingFl = false;
		break;

	case 64:
		g_engine->_soundManager->command(3, 0);
		new_room = 307;
		break;

	default:
		break;
	}

	uint32 tmpFrame = timer_read();
	int32 diffFrame = tmpFrame - local._lastFrameCounter;
	local._lastFrameCounter = tmpFrame;

	if ((local._animMode == 2) && !local._internVisibleFl && player.commands_allowed) {
		if ((diffFrame >= 0) && (diffFrame <= 4))
			local._counter += diffFrame;
		else
			local._counter++;

		int extraCounter = player_has(OBJ_SCALPEL) ? 900 : 0;

		if (local._counter + extraCounter >= 1800) {
			kernel_abort_animation(0);
			player.commands_allowed = false;
			kernel_run_animation(kernel_name('c', -1), 62);
			local._animMode = 3;
		}
	} else if ((local._animMode == 2) && local._explosionFl && local._internVisibleFl && !local._dialogFl
		&& !local._internWalkingFl && (inter_input_mode != INTER_CONVERSATION)) {
		if ((diffFrame >= 0) && (diffFrame <= 4))
			local._internCounter += diffFrame;
		else
			local._internCounter++;

		if (local._internCounter >= 3600) {
			g_engine->_soundManager->command(59, 0);
			mcga_shakes = 20;
			local._internWalkingFl = true;
		}
	}

	if ((kernel.clock - local._dropTimer) > 600) {
		g_engine->_soundManager->command(51, 0);
		g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 14, 0, 0, 1);
		kernel_seq_depth(g_sequence_ids[1], 10);
		local._dropTimer = kernel.clock;
	}
}

static void room_318_pre_parser() {
	if (player.need_to_walk)
		player.need_to_walk = player.walker_visible;

	if (player_said_2(walk_down, corridor_to_west))
		player.walk_off_edge_to_room = 357;
}

static void room_318_parser() {
	if (inter_input_mode == INTER_CONVERSATION) {
		handleDialog();
		player.command_ready = false;
		return;
	}

	if (player_said_2(talkto, intern)) {
		switch (kernel.trigger) {
		case 0:
		{
			local._dialogFl = true;
			g_engine->_soundManager->command(15, 0);
			player.commands_allowed = false;
			handleRexDialogs(g_engine->getRandomNumber(0x18C, 0x18E));

			kernel_seq_delete(g_sequence_ids[2]);
			g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 8, 80, 0, 1);
			kernel_seq_depth(g_sequence_ids[2], 1);
			kernel_seq_loc(g_sequence_ids[2], 142, 121);
			kernel_seq_range(g_sequence_ids[2], 6, 8);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 2);
		}
		break;

		case 1:
			player.commands_allowed = true;
			handleInternDialog(0x18F, 1, INDEFINITE_TIMEOUT);
			local._dialog1.start();
			break;

		case 2:
		{
			int oldIdx = g_sequence_ids[2];
			g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 8);
			kernel_seq_depth(g_sequence_ids[2], 1);
			kernel_seq_loc(g_sequence_ids[2], 142, 121);
			kernel_seq_timeout(oldIdx, g_sequence_ids[2]);
		}
		break;

		default:
			break;
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(take, scalpel) && (object_is_here(OBJ_SCALPEL) || kernel.trigger)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			kernel_seq_delete(g_sequence_ids[2]);
			g_sequence_ids[2] = kernel_seq_pingpong(g_sprite_ids[2], false, 8, 80, 0, 2);
			kernel_seq_depth(g_sequence_ids[2], 1);
			kernel_seq_loc(g_sequence_ids[2], 142, 121);
			kernel_seq_range(g_sequence_ids[2], 2, 5);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_SPRITE, 5, 1);
			kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			if (local._internVisibleFl)
				handleInternDialog(0x190, 1, 120);
			else {
				inter_give_to_player(OBJ_SCALPEL);
				object_examine(OBJ_SCALPEL, 0x7C5D, 0);
				kernel_seq_delete(g_sequence_ids[3]);
			}
			break;

		case 2:
		{
			int oldIdx = g_sequence_ids[2];
			g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 1);
			kernel_seq_depth(g_sequence_ids[2], 1);
			kernel_seq_loc(g_sequence_ids[2], 142, 121);
			kernel_seq_timeout(oldIdx, g_sequence_ids[2]);
			kernel_timing_trigger(60, 3);
		}
		break;

		case 3:
			player.commands_allowed = true;
			break;

		default:
			break;
		}
		player.command_ready = false;
		return;
	}

	if (player.walker_visible) {
		if (player_said_2(walk_down, corridor_to_south)) {
			new_room = 407;
			player.command_ready = false;
			return;
		}

		if (player_said_2(take, tape_player)) {
			if (object_is_here(OBJ_AUDIO_TAPE)) {
				object_examine(OBJ_AUDIO_TAPE, 0x7C5B, 0);
				inter_give_to_player(OBJ_AUDIO_TAPE);
			} else
				text_show(31834);

			player.command_ready = false;
			return;
		}

		if (player_said_2(look, tape_player)) {
			if (object_is_here(OBJ_AUDIO_TAPE))
				text_show(31833);
			else
				text_show(31834);

			player.command_ready = false;
			return;
		}

		if (player_said_2(walk_into, doctors_office)) {
			text_show(31831);
			player.command_ready = false;
			return;
		}

		if (player_said_2(look, gurney)) {
			text_show(31823);
			player.command_ready = false;
			return;
		}

		if (player_said_2(look, instrument_table)) {
			text_show(31825);
			player.command_ready = false;
			return;
		}
	} else { // Not visible
		if (player_said_2(look, gurney)) {
			text_show(31822);
			player.command_ready = false;
			return;
		}

		if (player_said_2(look, instrument_table)) {
			text_show(31824);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(look, wall))
		text_show(31810);
	else if (player_said_2(look, floor))
		text_show(31811);
	else if (player_said_2(look, corridor_to_west))
		text_show(31812);
	else if (player_said_2(look, corridor_to_south))
		text_show(31813);
	else if (player_said_2(look, faucet))
		text_show(31814);
	else if (player_said_2(look, sink))
		text_show(31815);
	else if (player_said_2(look, conveyor_belt))
		text_show(31816);
	else if (player_said_2(look, large_blade))
		text_show(31817);
	else if (player_said_2(look, monitor))
		text_show(31818);
	else if (player_said_2(look, cabinets))
		text_show(31819);
	else if (player_said_2(look, equipment))
		text_show(31820);
	else if (player_said_2(look, shelf))
		text_show(31821);
	else if (player_said_2(open, cabinets))
		text_show(31829);
	else if (player_said_2(look, intern))
		text_show(31830);
	else if (player_said_2(look, professor))
		text_show(31832);
	else if (player_said_2(look, professors_gurney))
		text_show(31836);
	else if (player.look_around) {
		if (player.walker_visible || player_has(OBJ_SCALPEL))
			text_show(31828);
		else if (local._internVisibleFl)
			text_show(31826);
		else
			text_show(31827);
	} else
		return;

	player.command_ready = false;
}

void room_318_synchronize(Common::Serializer &s) {
	s.syncAsUint32LE(local._dropTimer);
	s.syncAsSint32LE(local._lastFrame);
	s.syncAsSint32LE(local._animMode);
	s.syncAsSint32LE(local._internCounter);
	s.syncAsSint32LE(local._counter);

	s.syncAsByte(local._dialogFl);
	s.syncAsByte(local._internTalkingFl);
	s.syncAsByte(local._internWalkingFl);
	s.syncAsByte(local._internVisibleFl);
	s.syncAsByte(local._explosionFl);
	s.syncAsUint32LE(local._lastFrameCounter);
}

void room_318_preload() {
	room_init_code_pointer = room_318_init;
	room_pre_parser_code_pointer = room_318_pre_parser;
	room_parser_code_pointer = room_318_parser;
	room_daemon_code_pointer = room_318_daemon;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
