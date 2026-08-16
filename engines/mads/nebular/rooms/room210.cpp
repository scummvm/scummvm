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
#include "mads/nebular/rooms/dialog.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section2.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	int32 _curDialogNode;
	int32 _nextHandsPlace;
	int32 _twinkleAnimationType;
	int32 _twinklesCurrentFrame;
	bool _shouldTalk;
	bool _shouldFaceRex;
	bool _shouldMoveHead;
	bool _stopWalking;
	bool _twinklesTalking;
	bool _twinklesTalk2;
	int32 _doorway;
	char _subQuote2[256];
	Dialog _conv1;
	Dialog _conv2;
	Dialog _conv3;
	Dialog _conv5;
	Dialog _conv6;
	Dialog _conv7;
	Dialog _conv8;
};

static Scratch local;


static void handleTwinklesSpeech(int quoteId, int shiftX, uint32 delay) {
	kernel_message_add(quote_string(kernel.quotes, quoteId), 10, 70 + (shiftX * 14), 0xFDFC, (delay == 0) ? INDEFINITE_TIMEOUT : delay, 0, 0);
}

static void newNode(int node) {
	local._curDialogNode = node;

	switch (local._curDialogNode) {
	case 1:
		local._conv1.start();
		break;

	case 2:
		local._conv2.start();
		break;

	case 3:
		local._conv3.start();
		break;

	case 5:
		local._conv5.start();
		break;

	case 6:
		local._conv6.start();
		break;

	case 7:
		local._conv7.start();
		break;

	case 8:
		local._conv8.start();
		break;

	default:
		break;
	}
}

static void setDialogNode(int node) {
	switch (node) {
	case 0:
		kernel_set_interface_mode(INTER_BUILDING_SENTENCES);
		local._shouldFaceRex = false;
		local._shouldTalk = false;
		player.commands_allowed = true;
		local._curDialogNode = 0;
		break;

	case 2:
		switch (kernel.trigger) {
		case 1:
			local._nextHandsPlace = 1;
			local._shouldTalk = true;
			player.commands_allowed = false;
			if (local._twinklesTalking) {
				handleTwinklesSpeech(0xB9, -1, 0);
				kernel_timing_trigger(180, 2);
			} else {
				kernel_timing_trigger(6, 1);
			}
			break;

		case 2:
			local._nextHandsPlace = 0;
			handleTwinklesSpeech(186, 0, 0);
			kernel_timing_trigger(180, 3);
			break;

		default:
			local._nextHandsPlace = 0;
			local._shouldTalk = false;
			player.commands_allowed = true;
			newNode(2);
			break;
		}
		break;

	case 3:
		switch (kernel.trigger) {
		case 1:
			local._nextHandsPlace = 0;
			local._shouldTalk = true;
			player.commands_allowed = false;
			if (local._twinklesTalking) {
				handleTwinklesSpeech(0xBE, -2, 0);
				kernel_timing_trigger(180, 2);
			} else {
				kernel_timing_trigger(6, 1);
			}
			break;
		case 2:
			local._nextHandsPlace = 2;
			handleTwinklesSpeech(191, -1, 0);
			kernel_timing_trigger(180, 3);
			break;
		case 3:
			local._nextHandsPlace = 0;
			handleTwinklesSpeech(192, 0, 0);
			kernel_timing_trigger(180, 4);
			break;
		default:
			local._shouldTalk = false;
			player.commands_allowed = true;
			newNode(3);
			break;
		}
		break;

	case 4:
		if (kernel.trigger == 1) {
			local._nextHandsPlace = 1;
			local._shouldTalk = true;
			player.commands_allowed = false;

			int quote;
			if (config_file.naughtiness == NAUGHTY)
				quote = g_engine->getRandomNumber(199, 201);
			else
				quote = g_engine->getRandomNumber(197, 198);

			if (local._twinklesTalking) {
				handleTwinklesSpeech(quote, 0, 360);
				kernel_timing_trigger(120, 2);
			} else {
				kernel_timing_trigger(6, 1);
			}
		} else {
			kernel_set_interface_mode(INTER_BUILDING_SENTENCES);
			local._shouldFaceRex = false;
			local._shouldTalk = false;
			player.commands_allowed = true;
		}
		break;

	case 5:
		switch (kernel.trigger) {
		case 1:
			local._nextHandsPlace = 2;
			local._shouldTalk = true;
			player.commands_allowed = false;
			if (local._twinklesTalking) {
				handleTwinklesSpeech(0xCA, -1, 0);
				kernel_timing_trigger(180, 2);
			} else {
				kernel_timing_trigger(6, 1);
			}
			break;

		case 2:
			local._nextHandsPlace = 1;
			handleTwinklesSpeech(0xCB, 0, 0);
			kernel_timing_trigger(180, 3);
			break;

		default:
			local._nextHandsPlace = 2;
			local._shouldTalk = false;
			player.commands_allowed = true;
			newNode(5);
			break;
		}
		break;

	case 6:
		switch (kernel.trigger) {
		case 1:
			local._nextHandsPlace = 1;
			local._shouldTalk = true;
			player.commands_allowed = false;
			if (local._twinklesTalking) {
				handleTwinklesSpeech(0xD0, -2, 0);
				kernel_timing_trigger(180, 2);
			} else {
				kernel_timing_trigger(6, 1);
			}
			break;

		case 2:
			handleTwinklesSpeech(0xD1, -1, 0);
			kernel_timing_trigger(180, 3);
			break;

		case 3:
			local._nextHandsPlace = 1;
			handleTwinklesSpeech(0xD2, 0, 0);
			kernel_timing_trigger(180, 4);
			break;

		default:
			local._nextHandsPlace = 0;
			local._shouldTalk = false;
			player.commands_allowed = true;
			newNode(6);
			break;
		}
		break;

	case 7:
		switch (kernel.trigger) {
		case 1:
			local._nextHandsPlace = 2;
			local._shouldTalk = true;
			player.commands_allowed = false;
			if (local._twinklesTalking) {
				handleTwinklesSpeech(0xD6, -1, 0);
				kernel_timing_trigger(180, 2);
			} else {
				kernel_timing_trigger(6, 1);
			}
			break;

		case 2:
			handleTwinklesSpeech(0xD7, 0, 0);
			kernel_timing_trigger(180, 3);
			break;

		default:
			local._shouldTalk = false;
			player.commands_allowed = true;
			newNode(7);
			break;
		}
		break;

	case 8:
		switch (kernel.trigger) {
		case 1:
			local._nextHandsPlace = 2;
			local._shouldTalk = true;
			player.commands_allowed = false;
			if (local._twinklesTalking) {
				handleTwinklesSpeech(0xDD, -1, 0);
				kernel_timing_trigger(180, 2);
			} else {
				kernel_timing_trigger(6, 1);
			}
			break;

		case 2:
			handleTwinklesSpeech(0xDE, 0, 0);
			kernel_timing_trigger(180, 3);
			break;

		default:
			local._shouldTalk = false;
			player.commands_allowed = true;
			newNode(8);
			break;
		}
		break;


	case 9:
		switch (kernel.trigger) {
		case 1:
			local._nextHandsPlace = 0;
			local._shouldTalk = true;
			player.commands_allowed = false;
			if (local._twinklesTalking) {
				inter_reset_dialog();
				kernel_set_interface_mode(INTER_CONVERSATION);
				handleTwinklesSpeech(0xE4, -1, 0);
				kernel_timing_trigger(180, 2);
			} else {
				kernel_timing_trigger(6, 1);
			}
			break;

		case 2:
			handleTwinklesSpeech(0xE5, 0, 0);
			kernel_timing_trigger(180, 3);
			break;

		case 3:
			local._twinkleAnimationType = 2;
			global[kCurtainOpen] = true;
			player.walker_visible = false;
			pal_lock();
			kernel_message_purge();
			kernel_abort_animation(0);
			kernel_dump_all();

			g_sprite_ids[1] = kernel_load_series(kernel_name('c', -1), 0);
			kernel.quotes = quote_load(230, 233, 234, 231, 232, 0);
			kernel_run_animation(kernel_name('B', -1), 4);
			break;

		case 4:
		{
			g_sprite_ids[8] = kernel_seq_stamp(g_sprite_ids[1], false, 5);
			kernel_seq_depth(g_sprite_ids[8], 1);

			int msgIndex = kernel_message_add(quote_string(kernel.quotes, 231), 160, 20, 0x1110, 180, 5, 32);
			kernel_message_teletype(msgIndex, 4, true);
		}
		break;

		case 5:
		{
			int msgIndex = kernel_message_add(quote_string(kernel.quotes, 233), 160, 40, 0xFDFC, 180, 6, 32);
			kernel_message_teletype(msgIndex, 4, true);
		}
		break;

		case 6:
		{
			int msgIndex = kernel_message_add(quote_string(kernel.quotes, 232), 160, 60, 0x1110, 180, 7, 32);
			kernel_message_teletype(msgIndex, 4, true);
		}
		break;

		case 7:
		{
			int msgIndex = kernel_message_add(quote_string(kernel.quotes, 234), 160, 80, 0xFDFC, 180, 8, 32);
			kernel_message_teletype(msgIndex, 4, true);
		}
		break;

		case 8:
			global[kTwinklesStatus] = TWINKLES_GONE;
			new_room = 216;
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}
}

static void handleConversation1() {
	switch (player2.words[0]) {
	case words_ignite:
		setDialogNode(2);
		break;

	case words_inflate:
		setDialogNode(6);
		break;

	case words_insert:
		setDialogNode(4);
		break;

	case words_inspect:
		setDialogNode(9);
		break;

	case words_jungle:
		setDialogNode(0);
		break;

	default:
		break;
	}
}

static void handleConversation2() {
	switch (player2.words[0]) {
	case words_key_1:
		setDialogNode(3);
		break;

	case words_key_2:
		setDialogNode(4);
		break;

	case words_key_3:
		setDialogNode(0);
		break;

	default:
		break;
	}
}

static void handleConversation3() {
	switch (player2.words[0]) {
	case words_key_7:
		setDialogNode(6);
		break;

	case words_key_8:
		setDialogNode(5);
		break;

	case words_key_9:
		setDialogNode(4);
		break;

	case words_keypad:
		setDialogNode(0);
		break;

	default:
		break;
	}
}

static void handleConversation5() {
	switch (player2.words[0]) {
	case words_life_support_section:
		setDialogNode(6);
		break;

	case words_light:
	case words_load:
		setDialogNode(4);
		break;

	case words_lock:
		setDialogNode(0);
		break;

	default:
		break;
	}
}

static void handleConversation6() {
	switch (player2.words[0]) {
	case words_look_through:
		setDialogNode(7);
		break;

	case words_lounge_area:
		setDialogNode(4);
		break;

	case words_love_altar:
		setDialogNode(0);
		break;

	default:
		break;
	}
}

static void handleConversation7() {
	switch (player2.words[0]) {
	case words_magnet:
	case words_main_airlock:
	case words_marshy_wetlands:
		setDialogNode(4);
		break;

	case words_manta_ray:
		setDialogNode(8);
		break;

	case words_match:
		setDialogNode(0);
		break;

	default:
		break;
	}
}

static void handleConversation8() {
	switch (player2.words[0]) {
	case words_men_who_came_before:
	case words_mine:
		setDialogNode(4);
		break;

	case words_mirror:
	case words_monitor:
		setDialogNode(9);
		break;

	case words_monkey:
		setDialogNode(0);
		break;

	default:
		break;
	}
}

static void handleConversations() {
	if (kernel.trigger == 0) {
		kernel_message_purge();
		player.commands_allowed = false;
		char *curQuote = quote_string(kernel.quotes, player2.words[0]);
		if (g_engine->getMessageTextWidth(kernel_message_font, curQuote, kernel_message_spacing) > 200) {
			static char line1[40], line2[40];
			quote_split_string(curQuote, line1, line2);
			Common::strcpy_s(local._subQuote2, line2);
			kernel_message_add(line1, 0, -14, 0x1110, 240, 0, 34);
			kernel_timing_trigger(60, 50);
		} else {
			kernel_message_add(curQuote, 0, 0, 0x1110, 120, 1, 34);
		}
	} else if (kernel.trigger == 50) {
		kernel_message_add(local._subQuote2, 0, 0, 0x1110, 240, 0, 34);
		kernel_timing_trigger(180, 1);
	} else {
		if (kernel.trigger == 1)
			kernel_message_purge();

		switch (local._curDialogNode) {
		case 1:
			handleConversation1();
			break;

		case 2:
			handleConversation2();
			break;

		case 3:
			handleConversation3();
			break;

		case 5:
			handleConversation5();
			break;

		case 6:
			handleConversation6();
			break;

		case 7:
			handleConversation7();
			break;

		case 8:
			handleConversation8();
			break;

		default:
			break;
		}
	}
}

static void restoreDialogNode(int node, int msgId, int posY) {
	int curQuoteId = msgId;
	int curY = 1 - posY;
	for (int count = 0; count < posY; count++) {
		handleTwinklesSpeech(curQuoteId, curY, 0);
		curY++;
		curQuoteId++;
	}

	newNode(node);
}

static void room_210_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('c', -1), 0);

	if (!player.been_here_before)
		global[kCurtainOpen] = 0;

	if (previous_room == 205) {
		player.x = 277;
		player.y = 56;
	}
	else if (previous_room == 215) {
		player.x = 168;
		player.y = 128;
		player.facing = FACING_SOUTH;
		global[kCurtainOpen] = true;
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 308;
		player.y = 132;
	}

	if (!global[kCurtainOpen]) {
		g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 5);
		kernel_seq_depth(g_sequence_ids[1], 5);
	} else {
		int idx = kernel_add_dynamic(words_doorway, words_walk_through, 0, -1, 163, 87, 19, 36);
		kernel_dynamic_walk(idx, 168, 127, FACING_NORTH);
		local._doorway = idx;
		kernel_dynamic_cursor(local._doorway, CURSOR_UP);
	}

	kernel.quotes = quote_load(90, 115, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 184, 183,
		185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202,
		203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220,
		221, 222, 223, 224, 225, 226, 227, 228, 229, 0);

	local._conv1.setup(0x2E, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0);

	if (!player.been_here_before) {
		local._conv1.set(0x2E, 0xB4, 0xB5, 0xB6, 0xB8, 0);

		if (kernel.cheating >= 2)
			local._conv1.write(0xB7, true);
	}

	bool sceneRevisited = player.been_here_before;
	local._conv2.setup(0x2F, 0xBC, 0xBB, 0xBD, sceneRevisited ? 0 : -1);
	local._conv3.setup(0x30, 0xC3, 0xC2, 0xC1, 0xC4, sceneRevisited ? 0 : -1);
	local._conv5.setup(0x31, 0xCD, 0xCC, 0xCE, 0xCF, sceneRevisited ? 0 : -1);
	local._conv6.setup(0x32, 0xD3, 0xD4, 0xD5, sceneRevisited ? 0 : -1);
	local._conv7.setup(0x33, 0xD8, 0xDA, 0xD9, 0xDB, 0xDC, sceneRevisited ? 0 : -1);
	local._conv8.setup(0x34, 0xDF, 0xE0, 0xE1, 0xE2, 0xE3, sceneRevisited ? 0 : -1);

	local._twinkleAnimationType = 0;
	local._twinklesCurrentFrame = 0;

	if (previous_room != KERNEL_RESTORING_GAME) {
		local._shouldMoveHead = false;
		local._shouldFaceRex = false;
		local._shouldTalk = false;
		local._nextHandsPlace = 0;
		local._twinklesTalking = false;
		local._curDialogNode = 0;
		local._stopWalking = false;
		local._twinklesTalk2 = (global[kTwinklesApproached] > 0);
	}

	if (global[kTwinklesStatus] == 0) {
		kernel_run_animation(kernel_name('A', -1), 0);
		local._twinkleAnimationType = 1;
	} else
		kernel_flip_hotspot(476, false);

	if (local._curDialogNode) {
		int quote = 0;
		int number = 0;

		switch (local._curDialogNode) {
		case 1:
			quote = 0xB3;
			number = 1;
			break;
		case 2:
			quote = 0xB9;
			number = 2;
			break;
		case 3:
			quote = 0xBE;
			number = 3;
			break;
		case 5:
			quote = 0xCA;
			number = 2;
			break;
		case 6:
			quote = 0xD0;
			number = 3;
			break;
		case 7:
			quote = 0xD6;
			number = 2;
			break;
		case 8:
			quote = 0xDD;
			number = 2;
			break;
		default:
			break;
		}

		restoreDialogNode(local._curDialogNode, quote, number);
		if ((kernel_anim[0].anim != nullptr))
			kernel_reset_animation(0, 131);
	}

	pal_change_color(252, 63, 63, 10);
	pal_change_color(253, 45, 45, 5);

	section_2_music();
}

static void room_210_daemon() {
	if ((local._twinkleAnimationType == 1) && (kernel_anim[0].anim != nullptr)) {
		if (local._twinklesCurrentFrame != kernel_anim[0].frame) {
			local._twinklesCurrentFrame = kernel_anim[0].frame;
			int reset_frame = -1;
			int random = g_engine->getRandomNumber(1, 1000);

			switch (local._twinklesCurrentFrame) {
			case 31:
			case 58:
			case 74:
			case 108:
			case 190:
				if (local._shouldFaceRex)
					reset_frame = 108;
				else if ((random <= 100) || local._shouldMoveHead)
					reset_frame = 74;
				else if (random <= 300)
					reset_frame = 58;
				else if (random <= 500)
					reset_frame = 31;
				else
					reset_frame = 0;
				break;

			case 80:
			case 90:
				local._twinklesTalking = false;
				if (local._shouldMoveHead) {
					reset_frame = 90;
				} else if (local._twinklesCurrentFrame == 90) {
					if ((random <= 400) || local._shouldFaceRex)
						reset_frame = 99;
					else
						reset_frame = 79;
				}
				break;

			case 98:
				if (local._shouldMoveHead)
					reset_frame = 91;
				break;

			case 99:
				if ((random <= 400) && !local._shouldFaceRex)
					reset_frame = 79;
				break;

			case 124:
			case 136:
			case 174:
			case 143:
			case 152:
				if (local._nextHandsPlace == 3)
					reset_frame = 174;
				else if (!local._shouldFaceRex)
					reset_frame = 174;
				else if (local._nextHandsPlace == 1)
					reset_frame = 136;
				else if (local._nextHandsPlace == 2)
					reset_frame = 152;
				else if (local._shouldTalk)
					reset_frame = 124;
				else
					reset_frame = (random <= 200) ? 131 : 143;
				break;

			case 131:
				if (local._shouldTalk) {
					switch (local._nextHandsPlace) {
					case 1:
						reset_frame = 136;
						break;
					case 2:
						reset_frame = 152;
						break;
					default:
						reset_frame = 124;
						break;
					}
				}
				break;

			case 138:
			case 141:
				if ((local._nextHandsPlace == 0) || !local._shouldFaceRex)
					reset_frame = 141;
				else if (local._nextHandsPlace == 2)
					reset_frame = 152;
				else if (!local._shouldTalk || (local._twinklesCurrentFrame == 141))
					reset_frame = 137;
				break;

			case 153:
			case 169:
				if ((local._nextHandsPlace == 0) || !local._shouldFaceRex)
					reset_frame = 169;
				else if (local._nextHandsPlace == 1)
					reset_frame = 136;
				else if (local._shouldTalk)
					reset_frame = 153;
				else
					reset_frame = 162;
				break;

			case 162:
				if (local._shouldTalk) {
					switch (local._nextHandsPlace) {
					case 0:
						reset_frame = 124;
						break;
					case 1:
						reset_frame = 136;
						break;
					default:
						reset_frame = 153;
						break;
					}
				}
				break;

			default:
				break;
			}

			if (reset_frame >= 0) {
				if (reset_frame != kernel_anim[0].frame) {
					kernel_reset_animation(0, reset_frame);
					local._twinklesCurrentFrame = reset_frame;
				}

				if ((reset_frame == 90) || (reset_frame == 91) || (reset_frame == 124) || (reset_frame == 153)
					|| ((reset_frame == 137) && local._shouldTalk)) {
					local._twinklesTalking = true;
				} else {
					local._twinklesTalking = false;
				}
			}
		}
	}

	if ((local._twinkleAnimationType == 2) && (kernel_anim[0].anim != nullptr)) {
		if (local._twinklesCurrentFrame != kernel_anim[0].frame) {
			local._twinklesCurrentFrame = kernel_anim[0].frame;
			int reset_frame = -1;

			if (local._twinklesCurrentFrame == 53) {
				kernel_message_add(quote_string(kernel.quotes, 230), 151, 61, 0xFDFC, 180, 70, 32);
				local._shouldTalk = true;
			} else if ((local._twinklesCurrentFrame == 75) && local._shouldTalk)
				reset_frame = 60;

			if ((reset_frame >= 0) && (reset_frame != kernel_anim[0].frame)) {
				kernel_reset_animation(0, reset_frame);
				local._twinklesCurrentFrame = reset_frame;
			}
		}

		if (kernel.trigger == 70)
			local._shouldTalk = false;
	}

	if ((local._twinkleAnimationType == 1) && (player.next_special_code > 0)) {
		player_walk(214, 150, FACING_NORTHWEST);
		player.next_special_code = 0;
		local._stopWalking = true;
	}
}

static void room_210_pre_parser() {
	local._stopWalking = false;

	if (player_said_2(walk_down, path_to_east))
		player.walk_off_edge_to_room = 211;
}

static void room_210_parser() {
	if (player_said_3(look, binoculars, hut_to_north)) {
		text_show(21017);
	} else if (inter_input_mode == INTER_CONVERSATION) {
		handleConversations();
	} else if (player_said_2(talkto, native_woman) ||
		((Common::Point(player.x, player.y) == Common::Point(214, 150)) && (player.facing == FACING_NORTHWEST) && (local._twinkleAnimationType == 1) && local._stopWalking)) {
		switch (kernel.trigger) {
		case 0:
		{
			player.commands_allowed = false;
			int quote;
			if (global[kTwinklesApproached] == 0)
				quote = 90;
			else if (global[kTwinklesApproached] == 1)
				quote = 115;
			else
				quote = 171;

			local._shouldFaceRex = true;
			local._nextHandsPlace = 0;
			kernel_message_purge();
			kernel_message_player(quote, 120, 1);
		}
		break;

		case 1:
			local._shouldTalk = true;
			if (!local._twinklesTalking) {
				kernel_timing_trigger(6, 1);
			} else {
				if (global[kTwinklesApproached] == 0) {
					handleTwinklesSpeech(0xAF, -1, 0);
					handleTwinklesSpeech(0xB0, 0, 0);
				} else if (global[kTwinklesApproached] == 1) {
					handleTwinklesSpeech(0xB1, 0, 0);
				} else {
					int quote = local._twinklesTalk2 ? 0xB3 : 0xB2;
					local._twinklesTalk2 = true;
					handleTwinklesSpeech(quote, 0, 0);
				}
				kernel_timing_trigger(60, 3);
			}
			break;

		case 3:
			player.commands_allowed = true;
			local._shouldMoveHead = false;
			local._shouldTalk = false;

			if (global[kTwinklesApproached] < 2)
				global[kTwinklesApproached]++;

			local._conv1.start();
			local._curDialogNode = 1;
			break;

		default:
			break;
		}
	} else if (player_said_2(give, native_woman) && player_has(object_named(player2.words[1]))) {
		switch (kernel.trigger) {
		case 0:
		{
			int quote = g_engine->getRandomNumber(172, 174);
			local._shouldMoveHead = true;
			player.commands_allowed = false;
			handleTwinklesSpeech(quote, 0, 120);
			kernel_timing_trigger(120, 1);
		}
		break;

		case 1:
			player.commands_allowed = true;
			local._shouldMoveHead = false;
			break;

		default:
			break;
		}
	} else if (player_said_2(walk_down, path_to_north) || player_said_2(walk_towards, hut_to_north)) {
		new_room = 205;
	} else if (player_said_2(walk_through, doorway)) {
		new_room = 215;
	} else if ((player_said_2(pull, curtain) || player_said_2(open, curtain)) && !global[kCurtainOpen]) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			kernel_seq_delete(g_sequence_ids[1]);
			g_sequence_ids[1] = kernel_seq_backward(g_sprite_ids[1], false, 12, 0, 0, 1);
			kernel_seq_depth(g_sequence_ids[1], 5);
			kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
			player.commands_allowed = true;
			global[kCurtainOpen] = true;
			local._doorway = kernel_add_dynamic(words_doorway, words_walk_through, 0, -1, 163, 87, 19, 36);
			kernel_dynamic_walk(local._doorway, 168, 127, FACING_NORTH);
			kernel_dynamic_cursor(local._doorway, CURSOR_UP);
			break;

		default:
			break;
		}
	} else if ((player_said_2(pull, curtain) || player_said_2(close, curtain)) && global[kCurtainOpen]) {
		switch (kernel.trigger) {
		case 0:
			kernel_seq_delete(g_sequence_ids[1]);
			player.commands_allowed = false;
			player.facing = FACING_NORTH;
			g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 12, 1, 0, 0);
			kernel_seq_depth(g_sequence_ids[1], 5);
			kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 1);
			player.commands_allowed = false;
			break;
		case 1:
			g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, 5);
			kernel_seq_depth(g_sequence_ids[1], 5);
			kernel_timing_trigger(48, 2);
			break;
		case 2:
			kernel_delete_dynamic(local._doorway);
			player.commands_allowed = true;
			global[kCurtainOpen] = false;
			break;
		default:
			break;
		}
	} else if (player_said_2(look, hut)) {
		if (global[kTwinklesStatus] == TWINKLES_GONE) {
			if (config_file.naughtiness == NAUGHTY)
				text_show(21003);
			else
				text_show(21002);
		} else {
			text_show(21001);
		}
	} else if (player_said_2(look, bra)) {
		text_show(21004);
	} else if (player_said_2(look, hotpants)) {
		text_show(21005);
	} else if (player_said_2(take, hotpants) || player_said_2(take, bra)) {
		text_show(21006);
	} else if (player_said_2(look, stream)) {
		text_show(21007);
	} else if (player_said_2(look, bushy_fern)) {
		text_show(21008);
	} else if (player_said_2(look, village_path)) {
		text_show(21009);
	} else if (player_said_2(look, native_woman)) {
		text_show(21010);
	} else if (player_said_2(shoot, native_woman) || player_said_2(hose_down, native_woman)) {
		text_show(21011);
	} else if (player_said_2(look, path_to_north)) {
		text_show(21012);
	} else if (player_said_2(look, curtain)) {
		text_show(21013);
	} else if (player_said_2(look, clothesline)) {
		text_show(21014);
	} else if (player_said_2(take, clothesline)) {
		text_show(21015);
	} else if (player_said_2(look, hut_to_north)) {
		text_show(21016);
	} else {
		// Not handled
		return;
	}

	player.command_ready = false;
}

void room_210_synchronize(Common::Serializer &s) {
	s.syncAsSint32LE(local._curDialogNode);
	s.syncAsSint32LE(local._nextHandsPlace);
	s.syncAsSint32LE(local._twinkleAnimationType);
	s.syncAsSint32LE(local._twinklesCurrentFrame);

	s.syncAsByte(local._shouldTalk);
	s.syncAsByte(local._shouldFaceRex);
	s.syncAsByte(local._shouldMoveHead);
	s.syncAsByte(local._stopWalking);
	s.syncAsByte(local._twinklesTalking);
	s.syncAsByte(local._twinklesTalk2);

	s.syncAsSint32LE(local._doorway);

	Common::String subQuote2 = local._subQuote2;
	s.syncString(subQuote2);
	if (s.isLoading())
		Common::strcpy_s(local._subQuote2, subQuote2.c_str());
}

void room_210_preload() {
	room_init_code_pointer = room_210_init;
	room_pre_parser_code_pointer = room_210_pre_parser;
	room_parser_code_pointer = room_210_parser;
	room_daemon_code_pointer = room_210_daemon;

	section_2_walker();
	section_2_interface();
	vocab_make_active(words_doorway);
	vocab_make_active(words_walk_through);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
