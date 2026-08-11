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
#include "mads/nebular/rooms/section4.h"
#include "mads/nebular/rooms/dialog.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

struct Scratch {
	bool _lightOn;
	bool _blowingSmoke;
	bool _leftWomanMoving;
	bool _rightWomanMoving;
	bool _firstTalkToGirlInChair;
	bool _waitingGinnyMove;
	bool _ginnyLooking;
	bool _bigBeatFl;
	bool _roxOnStool;
	bool _bartenderSteady;
	bool _bartenderHandsHips;
	bool _bartenderLooksLeft;
	bool _bartenderReady;
	bool _bartenderTalking;
	bool _bartenderCalled;
	bool _conversationFl;
	bool _activeTeleporter;
	bool _activeArrows;
	bool _activeArrow1;
	bool _activeArrow2;
	bool _activeArrow3;
	bool _cutSceneReady;
	bool _cutSceneNeeded;
	bool _helgaReady;
	bool _refuseAlienLiquor;

	int32 _drinkTimer;
	int32 _beatCounter;
	int32 _bartenderMode;
	int32 _bartenderDialogNode;
	int32 _bartenderCurrentQuestion;
	int32 _helgaTalkMode;
	int32 _roxMode;
	int32 _rexMode;
	int32 _talkTimer;

	Dialog _dialog1;
	Dialog _dialog2;
	Dialog _dialog3;
	Dialog _dialog4;
};

static Scratch local;


static void setDialogNode(int node) {
	if (node > 0)
		local._bartenderDialogNode = node;

	player.commands_allowed = true;

	switch (node) {
	case 0:
		kernel_set_interface_mode(INTER_BUILDING_SENTENCES);
		local._conversationFl = false;
		local._bartenderDialogNode = 0;
		break;

	case 1:
		local._dialog1.start();
		local._bartenderDialogNode = 1;
		break;

	case 2:
		local._dialog2.start();
		local._bartenderDialogNode = 2;
		break;

	case 3:
		local._dialog3.start();
		local._bartenderDialogNode = 3;
		break;

	case 4:
		local._dialog4.start();
		local._bartenderDialogNode = 4;
		break;

	default:
		break;
	}
}

static void handleConversation1() {
	switch (player2.words[0]) {
	case 0x214:
	{
		int quoteId = 0;
		int quotePosX = 0;
		switch (g_engine->getRandomNumber(1, 3)) {
		case 1:
			quoteId = 0x1E4;
			local._bartenderCurrentQuestion = 4;
			quotePosX = 205;
			break;

		case 2:
			quoteId = 0x1E5;
			local._bartenderCurrentQuestion = 5;
			quotePosX = 203;
			break;

		case 3:
			quoteId = 0x1E6;
			local._bartenderCurrentQuestion = 6;
			quotePosX = 260;
			break;

		default:
			break;
		}
		kernel_message_purge();
		kernel_message_add(quote_string(kernel.quotes, quoteId), quotePosX, 41, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(1, 100);
		local._talkTimer = 120;
		setDialogNode(2);
	}
	break;

	case 0x215:
		kernel_message_purge();
		kernel_message_add(quote_string(kernel.quotes, 0x1EC), 260, 41, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(1, 100);
		local._talkTimer = 120;
		local._bartenderCurrentQuestion = 1;
		setDialogNode(3);
		break;

	case 0x237:
		kernel_message_purge();
		kernel_message_add(quote_string(kernel.quotes, 0x1FD), 208, 41, 0xFDFC, 100, 0, 0);
		setDialogNode(0);
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(1, 100);
		local._talkTimer = 1120;
		break;

	default:
		break;
	}
}

static void handleConversation2() {
	switch (player2.words[0]) {
	case 0x216:
		local._dialog2.write(0x216, false);
		local._dialog2.write(0x21D, true);
		break;

	case 0x219:
		local._dialog2.write(0x219, false);
		local._dialog2.write(0x220, true);
		break;

	case 0x21A:
		local._dialog2.write(0x21A, false);
		local._dialog2.write(0x223, true);
		break;

	case 0x21B:
		local._dialog2.write(0x21B, false);
		local._dialog2.write(0x224, true);
		break;

	case 0x21D:
		local._dialog2.write(0x21D, false);
		local._dialog2.write(0x227, true);
		break;

	case 0x220:
		local._dialog2.write(0x220, false);
		local._dialog2.write(0x22A, true);
		break;

	case 0x223:
		local._dialog2.write(0x223, false);
		local._dialog2.write(0x22D, true);
		break;

	case 0x224:
		local._dialog2.write(0x224, false);
		local._dialog2.write(0x230, true);
		break;

	case 0x227:
		local._dialog2.write(0x227, false);
		break;

	case 0x22A:
		local._dialog2.write(0x22A, false);
		break;

	case 0x22D:
		local._dialog2.write(0x22D, false);
		break;

	case 0x230:
		local._dialog2.write(0x230, false);
		break;

	case 0x21C:
		setDialogNode(0);
		break;

	default:
		break;

	}

	if (player2.words[0] != 0x21C) {
		switch (g_engine->getRandomNumber(1, 3)) {
		case 1:
			kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
			kernel_timing_trigger(1, 100);
			local._talkTimer = 180;
			kernel_message_purge();
			kernel_message_add(quote_string(kernel.quotes, 0x1E7), 198, 27, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
			kernel_message_add(quote_string(kernel.quotes, 0x1E8), 201, 41, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
			local._bartenderCurrentQuestion = 7;
			break;

		case 2:
			kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
			kernel_timing_trigger(1, 100);
			local._talkTimer = 180;
			kernel_message_purge();
			kernel_message_add(quote_string(kernel.quotes, 0x1E9), 220, 27, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
			kernel_message_add(quote_string(kernel.quotes, 0x1EA), 190, 41, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
			local._bartenderCurrentQuestion = 8;
			break;

		case 3:
			kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
			kernel_timing_trigger(1, 100);
			local._talkTimer = 150;
			kernel_message_purge();
			kernel_message_add(quote_string(kernel.quotes, 0x1EB), 196, 41, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
			local._bartenderCurrentQuestion = 9;
			break;

		default:
			break;
		}
		local._dialog2.start();
	} else {
		kernel_message_purge();
		kernel_message_add(quote_string(kernel.quotes, 0x1FD), 208, 41, 0xFDFC, 100, 0, 0);
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(1, 100);
		local._talkTimer = 1120;
	}
}

static void handleConversation3() {
	switch (player2.words[0]) {
	case 0x233:
	case 0x234:
	case 0x235:
	case 0x236:
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(1, 86);
		kernel_message_purge();
		kernel_message_add(quote_string(kernel.quotes, 0x1ED), 188, 27, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
		kernel_message_add(quote_string(kernel.quotes, 0x1EE), 199, 41, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
		setDialogNode(4);
		local._bartenderCurrentQuestion = 2;
		break;

	case 0x237:
		kernel_message_purge();
		kernel_message_add(quote_string(kernel.quotes, 0x1FD), 208, 41, 0xFDFC, 100, 0, 0);
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(1, 100);
		local._talkTimer = 1120;
		setDialogNode(0);
		break;

	default:
		break;
	}
}

static void handleConversation4() {
	switch (player2.words[0]) {
	case 0x238:
		kernel_message_purge();
		setDialogNode(0);
		player.commands_allowed = false;
		kernel_message_add(quote_string(kernel.quotes, 0x1F0), 196, 13, 0xFDFC, 180, 0, 0);
		kernel_message_add(quote_string(kernel.quotes, 0x1F1), 184, 27, 0xFDFC, 180, 0, 0);
		kernel_message_add(quote_string(kernel.quotes, 0x1F2), 200, 41, 0xFDFC, 180, 0, 0);
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(1, 100);
		local._talkTimer = 1100;
		local._dialog4.write(0x238, false);
		local._bartenderMode = 22;
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(100, 95);
		local._refuseAlienLiquor = true;
		break;

	case 0x239:
		player.commands_allowed = false;
		local._roxMode = 21;
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(1, 92);
		kernel_set_interface_mode(INTER_BUILDING_SENTENCES);
		local._conversationFl = false;
		break;

	case 0x23A:
		setDialogNode(0);
		player.commands_allowed = false;
		kernel_message_add(quote_string(kernel.quotes, 0x1F4), 193, 27, 0xFDFC, 150, 0, 0);
		kernel_message_add(quote_string(kernel.quotes, 0x1F5), 230, 41, 0xFDFC, 150, 0, 0);
		local._dialog4.write(0x23A, false);
		global[kHasSaidTimer] = true;
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(1, 100);
		local._talkTimer = 1100;
		local._bartenderMode = 22;
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(100, 95);
		local._refuseAlienLiquor = true;
		break;

	case 0x23D:
		setDialogNode(0);
		player.commands_allowed = false;
		kernel_message_add(quote_string(kernel.quotes, 0x1F6), 153, 27, 0xFDFC, 150, 0, 0);
		kernel_message_add(quote_string(kernel.quotes, 0x1F7), 230, 41, 0xFDFC, 150, 0, 0);
		local._dialog4.write(0x23D, false);
		global[kHasSaidBinocs] = true;
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(1, 100);
		local._talkTimer = 1100;
		local._bartenderMode = 22;
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(100, 95);
		local._refuseAlienLiquor = true;
		break;

	case 0x23E:
		kernel_message_purge();
		setDialogNode(0);
		player.commands_allowed = false;
		kernel_message_add(quote_string(kernel.quotes, 0x1F8), 205, 41, 0xFDFC, 100, 0, 0);
		local._bartenderMode = 22;
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(1, 100);
		local._talkTimer = 1050;
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(50, 95);
		local._refuseAlienLiquor = true;
		break;

	default:
		break;
	}
}

static void handleDialogs() {
	if (kernel.trigger == 0) {
		kernel_message_purge();
		player.commands_allowed = false;
		char *curQuote = quote_string(kernel.quotes, player2.words[0]);
		if (font_string_width(kernel_message_font, curQuote, kernel_message_spacing) > 200) {
			static char subQuote1[34], subQuote2[34];
			quote_split_string(curQuote, subQuote1, subQuote2);
			kernel_message_add(subQuote1, 230, 42, 0x1110, 140, 0, 32);
			kernel_message_add(subQuote2, 230, 56, 0x1110, 140, 0, 32);
			kernel_timing_trigger(160, 120);
		} else {
			kernel_message_add(curQuote, 230, 56, 0x1110, 140, 1, 32);
			kernel_timing_trigger(160, 120);
		}
	} else if (kernel.trigger == 120) {
		player.commands_allowed = true;
		switch (local._bartenderDialogNode) {
		case 1:
			handleConversation1();
			break;

		case 2:
			handleConversation2();
			break;

		case 3:
			handleConversation3();
			break;

		case 4:
			handleConversation4();
			break;

		default:
			break;
		}
	}
}

static void room_402_init() {
	g_sprite_ids[0] = kernel_load_series(kernel_name('n', -1), 0);
	g_sprite_ids[1] = kernel_load_series(kernel_name('x', 2), 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('x', 0), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('x', 1), 0);
	g_sprite_ids[4] = kernel_load_series(kernel_name('g', 0), 0);
	g_sprite_ids[5] = kernel_load_series(kernel_name('g', 1), 0);
	g_sprite_ids[6] = kernel_load_series(kernel_name('a', 0), 0);
	g_sprite_ids[7] = kernel_load_series(kernel_name('a', 1), 0);
	g_sprite_ids[15] = kernel_load_series(kernel_name('x', 5), 0);
	g_sprite_ids[8] = kernel_load_series(kernel_name('x', 4), 0);
	g_sprite_ids[9] = kernel_load_series(kernel_name('b', 0), 0);
	g_sprite_ids[10] = kernel_load_series(kernel_name('b', 1), 0);
	g_sprite_ids[11] = kernel_load_series(kernel_name('b', 2), 0);
	g_sprite_ids[12] = kernel_load_series(kernel_name('b', 3), 0);
	g_sprite_ids[13] = kernel_load_series(kernel_name('c', 0), 0);
	g_sprite_ids[14] = kernel_load_series(kernel_name('l', 0), 0);
	g_sprite_ids[16] = kernel_load_series(kernel_name('h', 0), 0);
	g_sprite_ids[17] = kernel_load_series(kernel_name('z', 0), 0);
	g_sprite_ids[18] = kernel_load_series(kernel_name('z', 1), 0);
	g_sprite_ids[19] = kernel_load_series(kernel_name('z', 2), 0);
	g_sprite_ids[20] = kernel_load_series(kernel_name('x', 6), 0);
	g_sprite_ids[21] = kernel_load_series("*ROXRC_9", 0);
	g_sprite_ids[22] = kernel_load_series("*ROXCL_8", 0);

	if (previous_room == 401) {
		player.x = 160;
		player.y = 150;
		player.facing = FACING_NORTH;
		local._roxOnStool = false;
		local._bartenderDialogNode = 1;
		local._conversationFl = false;
	} else if (previous_room != KERNEL_RESTORING_GAME) {
		player.x = 160;
		player.y = 150;
		player.facing = FACING_NORTH;
		inter_give_to_player(OBJ_CREDIT_CHIP);
		inter_give_to_player(OBJ_BINOCULARS);
		inter_give_to_player(OBJ_TIMER_MODULE);
		local._roxOnStool = false;
		local._bartenderDialogNode = 1;
		local._conversationFl = false;
	}

	kernel.quotes = quote_load(471, 472, 473, 474, 475, 476, 477, 478, 479, 482, 483, 486, 485, 487,
		488, 489, 490, 495, 496, 497, 498, 499, 500, 501, 502, 503, 504, 505, 506, 507,
		508, 491, 492, 493, 494, 484, 509, 480, 481, 510, 511, 512, 513, 514, 515, 516,
		517, 518, 519, 520, 521, 522, 523, 524, 527, 525, 526, 528, 529, 530, 531, 532,
		533, 567, 534, 537, 538, 539, 540, 541, 544, 547, 548, 551, 554, 557, 560, 563,
		564, 565, 566, 568, 569, 570, 573, 574, 575, 0);

	pal_change_color(250, 47, 41, 40);
	pal_change_color(251, 50, 63, 55);
	pal_change_color(252, 38, 34, 25);
	pal_change_color(253, 45, 41, 35);

	local._dialog1.setup(0x60, 0x214, 0x215, 0x237, 0);
	local._dialog2.setup(0x61, 0x216, 0x219, 0x21A, 0x21B, 0x21D, 0x220, 0x223, 0x224, 0x227, 0x22A, 0x22D, 0x230, 0x21C, 0);
	local._dialog3.setup(0x62, 0x233, 0x234, 0x235, 0x236, 0x237, -1);
	local._dialog4.setup(0x63, 0x238, 0x239, 0x23A, 0x23D, 0x23E, 0);

	if (!player.been_here_before) {
		local._dialog2.set(0x61, 0x216, 0x219, 0x21A, 0x21B, 0x21C, 0);
		local._dialog4.set(0x63, 0x238, 0x23E, 0);
		local._dialog1.set(0x60, 0x214, 0x215, 0x237, 0);
	}

	if (player_has(OBJ_CREDIT_CHIP))
		local._dialog4.write(0x239, true);
	else
		local._dialog4.write(0x239, false);

	if (player_has(OBJ_BINOCULARS) && !global[kHasSaidBinocs])
		local._dialog4.write(0x23D, true);
	else
		local._dialog4.write(0x23D, false);

	if (player_has(OBJ_TIMER_MODULE) && !global[kHasSaidTimer])
		local._dialog4.write(0x23A, true);
	else
		local._dialog4.write(0x23A, false);

	if (local._dialog2.read(0) <= 1)
		local._dialog1.write(0x214, false);

	if (local._conversationFl) {
		switch (local._bartenderDialogNode) {
		case 0:
			kernel_set_interface_mode(INTER_BUILDING_SENTENCES);
			local._bartenderDialogNode = 1;
			break;

		case 1:
			local._dialog1.start();
			break;

		case 2:
			local._dialog2.start();
			break;

		case 3:
			local._dialog3.start();
			break;

		case 4:
			local._dialog4.start();
			break;

		default:
			break;
		}

		switch (local._bartenderCurrentQuestion) {
		case 1:
			kernel_message_add(quote_string(kernel.quotes, 0x1EC), 260, 41, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
			break;

		case 2:
			kernel_message_add(quote_string(kernel.quotes, 0x1ED), 188, 27, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
			kernel_message_add(quote_string(kernel.quotes, 0x1EE), 199, 41, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
			break;

		case 3:
			kernel_message_add(quote_string(kernel.quotes, 0x1EF), 177, 41, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
			break;

		case 4:
			kernel_message_add(quote_string(kernel.quotes, 0x1E4), 205, 41, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
			break;

		case 5:
			kernel_message_add(quote_string(kernel.quotes, 0x1E5), 203, 41, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
			break;

		case 6:
			kernel_message_add(quote_string(kernel.quotes, 0x1E6), 260, 41, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
			break;

		case 7:
			kernel_message_add(quote_string(kernel.quotes, 0x1E7), 198, 27, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
			kernel_message_add(quote_string(kernel.quotes, 0x1E8), 201, 41, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
			break;

		case 8:
			kernel_message_add(quote_string(kernel.quotes, 0x1E9), 220, 27, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
			kernel_message_add(quote_string(kernel.quotes, 0x1EA), 190, 41, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
			break;

		case 9:
			kernel_message_add(quote_string(kernel.quotes, 0x1EB), 196, 41, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
			break;

		case 10:
			kernel_message_add(quote_string(kernel.quotes, 0x1E2), 198, 27, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
			kernel_message_add(quote_string(kernel.quotes, 0x1E3), 199, 41, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
			break;

		default:
			break;
		}
	}

	local._firstTalkToGirlInChair = false;
	local._lightOn = false;
	local._blowingSmoke = false;
	local._leftWomanMoving = false;
	local._rightWomanMoving = false;
	local._ginnyLooking = false;
	local._beatCounter = 0;
	local._waitingGinnyMove = false;
	local._bigBeatFl = true;
	local._bartenderHandsHips = false;
	local._bartenderSteady = true;
	local._bartenderLooksLeft = false;
	local._activeTeleporter = false;
	local._activeArrows = false;
	local._activeArrow1 = false;
	local._activeArrow2 = false;
	local._activeArrow3 = false;
	local._cutSceneReady = false;
	local._cutSceneNeeded = false;
	local._helgaReady = true;
	local._bartenderReady = true;
	local._drinkTimer = 0;
	local._bartenderTalking = false;
	local._bartenderCalled = false;
	local._helgaTalkMode = 0;
	local._rexMode = 0;
	local._refuseAlienLiquor = false;

	kernel_run_animation(kernel_full_name(402, 'd', 1, "", KERNEL_AA), 0);
	kernel_anim[0].repeat = true;

	g_sequence_ids[5] = kernel_seq_stamp(g_sprite_ids[5], false, 1);
	kernel_seq_depth(g_sequence_ids[5], 1);

	g_sequence_ids[13] = kernel_seq_stamp(g_sprite_ids[13], false, 2);
	kernel_seq_depth(g_sequence_ids[13], 8);

	if (!player_has(OBJ_REPAIR_LIST)) {
		g_sequence_ids[14] = kernel_seq_stamp(g_sprite_ids[14], false, 1);
		kernel_seq_depth(g_sequence_ids[14], 7);
		kernel_seq_loc(g_sequence_ids[14], 210, 80);
		int idx = kernel_add_dynamic(words_repair_list, words_look_at, 0, g_sequence_ids[14], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 0, 0, FACING_NONE);
	}

	{
		g_sequence_ids[9] = kernel_seq_stamp(g_sprite_ids[9], false, 1);
		kernel_seq_depth(g_sequence_ids[9], 8);
		int idx = kernel_add_dynamic(words_bartender, words_walkto, 0, g_sequence_ids[9], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 228, 83, FACING_SOUTH);
	}

	if (!object_is_here(OBJ_ALIEN_LIQUOR) && !player_has(OBJ_CREDIT_CHIP)) {
		g_sequence_ids[15] = kernel_seq_stamp(g_sprite_ids[15], false, 1);
		kernel_seq_depth(g_sequence_ids[15], 8);
		kernel_seq_loc(g_sequence_ids[15], 250, 80);
		int idx = kernel_add_dynamic(words_credit_chip, words_take, 0, g_sequence_ids[15], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 0, 0, FACING_NONE);
	}

	g_sequence_ids[20] = kernel_seq_stamp(g_sprite_ids[20], false, 1);
	kernel_seq_depth(g_sequence_ids[20], 7);
	kernel_seq_loc(g_sequence_ids[20], 234, 72);

	if (global[kBottleDisplayed]) {
		g_sequence_ids[8] = kernel_seq_stamp(g_sprite_ids[8], false, 4);
		kernel_seq_depth(g_sequence_ids[8], 7);
	}

	if (local._roxOnStool) {
		g_sequence_ids[6] = kernel_seq_stamp(g_sprite_ids[6], false, 11);
		kernel_seq_depth(g_sequence_ids[6], 5);
		player.walker_visible = false;
	}

	section_4_music();
}

static void room_402_daemon() {
	if (kernel.trigger == 104) {
		player.clock = kernel.clock + player.frame_delay;
		player.walker_visible = true;
		kernel_seq_delete(g_sequence_ids[15]);
		inter_give_to_player(OBJ_CREDIT_CHIP);
		object_examine(OBJ_CREDIT_CHIP, 40242, 0);
		player.commands_allowed = true;
	}

	if ((g_engine->getRandomNumber(1, 1500) == 1) && (!local._activeTeleporter) && (player.x < 150)) {
		g_engine->_soundManager->command(30, 0);
		g_sequence_ids[4] = kernel_seq_forward(g_sprite_ids[4], false, 13, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[4], 1, 11);
		kernel_seq_depth(g_sequence_ids[4], 14);
		kernel_seq_trigger(g_sequence_ids[4], KERNEL_TRIGGER_EXPIRE, 0, 48);
		local._activeTeleporter = true;
		global[kSomeoneHasExploded] = true;
	}

	if (kernel.trigger == 48)
		local._activeTeleporter = false;

	if (kernel.trigger == 100) {
		local._bartenderReady = false;
		if (local._bartenderHandsHips) {
			kernel_seq_delete(g_sequence_ids[10]);
			local._bartenderHandsHips = false;
		} else if (local._bartenderLooksLeft) {
			kernel_seq_delete(g_sequence_ids[11]);
			local._bartenderLooksLeft = false;
		} else if (local._bartenderSteady) {
			kernel_seq_delete(g_sequence_ids[9]);
			local._bartenderSteady = false;
		}

		if (!local._bartenderTalking) {
			g_sequence_ids[10] = kernel_seq_pingpong(g_sprite_ids[10], false, 7, 0, 0, 0);
			kernel_seq_range(g_sequence_ids[10], 3, 4);
			kernel_seq_depth(g_sequence_ids[10], 8);
			int idx = kernel_add_dynamic(words_bartender, words_walkto, 0, g_sequence_ids[10], 0, 0, 0, 0);
			kernel_dynamic_walk(idx, 228, 83, FACING_SOUTH);
			local._bartenderTalking = true;
			if (local._talkTimer > 1000)
				kernel_timing_trigger(local._talkTimer - 1000, 101);
			else
				kernel_timing_trigger(local._talkTimer, 101);
		}
	}

	if ((kernel.trigger == 101) && local._bartenderTalking) {
		kernel_seq_delete(g_sequence_ids[10]);
		g_sequence_ids[9] = kernel_seq_stamp(g_sprite_ids[9], false, 1);
		int idx = kernel_add_dynamic(words_bartender, words_walkto, 0, g_sequence_ids[9], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 228, 83, FACING_SOUTH);
		kernel_seq_depth(g_sequence_ids[9], 8);
		local._bartenderSteady = true;
		local._bartenderTalking = false;
		if (local._talkTimer < 1000)
			local._bartenderReady = true;
	}

	if (kernel.trigger == 28)
		player.commands_allowed = true;

	switch (kernel.trigger) {
	case 92:
		player.commands_allowed = false;
		kernel_seq_delete(g_sequence_ids[6]);
		g_sequence_ids[7] = kernel_seq_forward(g_sprite_ids[7], false, 10, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[7], 1, 2);
		kernel_seq_depth(g_sequence_ids[7], 5);
		kernel_seq_trigger(g_sequence_ids[7], KERNEL_TRIGGER_EXPIRE, 0, 93);
		break;

	case 93:
	{
		int seqIdx = g_sequence_ids[7];
		switch (local._roxMode) {
		case 20:
			g_engine->_soundManager->command(57, 0);
			kernel_seq_delete(g_sequence_ids[15]);
			inter_give_to_player(OBJ_CREDIT_CHIP);
			object_examine(OBJ_CREDIT_CHIP, 40242, 0);
			break;

		case 22:
			g_engine->_soundManager->command(57, 0);
			kernel_seq_delete(g_sequence_ids[8]);
			inter_give_to_player(OBJ_ALIEN_LIQUOR);
			global[kBottleDisplayed] = false;
			object_examine(OBJ_ALIEN_LIQUOR, 40241, 0);
			break;

		case 21:
			g_sequence_ids[15] = kernel_seq_stamp(g_sprite_ids[15], false, 1);
			kernel_seq_depth(g_sequence_ids[15], 8);
			kernel_seq_loc(g_sequence_ids[15], 250, 80);
			break;

		default:
			break;
		}
		g_sequence_ids[7] = kernel_seq_backward(g_sprite_ids[7], false, 10, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[7], 1, 3);
		kernel_seq_depth(g_sequence_ids[7], 5);
		kernel_seq_timeout(seqIdx, g_sequence_ids[7]);
		kernel_seq_trigger(g_sequence_ids[7], KERNEL_TRIGGER_EXPIRE, 0, 94);

		if (local._roxMode == 21) {
			if (player_has(OBJ_CREDIT_CHIP))
				inter_move_object(OBJ_CREDIT_CHIP, NOWHERE);

			local._bartenderMode = 20;
			kernel_timing_trigger(60, 95);
		}
	}
	break;

	case 94:
	{
		int seqIdx = g_sequence_ids[7];
		g_sequence_ids[6] = kernel_seq_stamp(g_sprite_ids[6], false, 11);
		kernel_seq_depth(g_sequence_ids[6], 5);
		kernel_seq_timeout(seqIdx, g_sequence_ids[6]);
		if (local._roxMode == 22) {
			kernel_message_purge();
			kernel_message_add(quote_string(kernel.quotes, 0x23F), 230, 56, 0x1110, 120, 0, 32);
			local._bartenderMode = 21;
			global[kHasPurchased] = true;
			kernel_timing_trigger(140, 95);
		} else if (local._roxMode == 20)
			player.commands_allowed = true;

	}
	break;

	default:
		break;
	}

	switch (kernel.trigger) {
	case 95:
		local._bartenderReady = false;
		player.commands_allowed = false;
		if (local._bartenderHandsHips || local._bartenderTalking) {
			kernel_seq_delete(g_sequence_ids[10]);
			local._bartenderHandsHips = false;
			local._bartenderTalking = false;
		}

		if (local._bartenderLooksLeft) {
			kernel_seq_delete(g_sequence_ids[11]);
			local._bartenderLooksLeft = false;
		}

		if (local._bartenderSteady) {
			kernel_seq_delete(g_sequence_ids[9]);
			local._bartenderSteady = false;
		}
		g_sequence_ids[12] = kernel_seq_forward(g_sprite_ids[12], false, 10, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[12], 1, 4);
		kernel_seq_depth(g_sequence_ids[12], 8);
		kernel_seq_trigger(g_sequence_ids[12], KERNEL_TRIGGER_EXPIRE, 0, 96);
		break;

	case 96:
	{
		int seqIdx = g_sequence_ids[12];
		g_sequence_ids[12] = kernel_seq_forward(g_sprite_ids[12], false, 10, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[12], 6, 7);
		kernel_seq_depth(g_sequence_ids[12], 8);
		kernel_seq_timeout(seqIdx, g_sequence_ids[12]);
		kernel_seq_trigger(g_sequence_ids[12], KERNEL_TRIGGER_EXPIRE, 0, 97);
	}
	break;

	case 97:
	{
		int seqIdx = g_sequence_ids[12];
		switch (local._bartenderMode) {
		case 20:
			kernel_seq_delete(g_sequence_ids[15]);
			break;

		case 21:
		{
			g_sequence_ids[15] = kernel_seq_stamp(g_sprite_ids[15], false, 1);
			kernel_seq_depth(g_sequence_ids[15], 8);
			kernel_seq_loc(g_sequence_ids[15], 250, 80);
			int idx = kernel_add_dynamic(words_credit_chip, words_take, 0, g_sequence_ids[15], 0, 0, 0, 0);
			kernel_dynamic_walk(idx, 0, 0, FACING_NONE);
		}
		break;

		case 22:
			kernel_seq_delete(g_sequence_ids[8]);
			global[kBottleDisplayed] = false;
			break;

		default:
			break;
		}

		g_sequence_ids[12] = kernel_seq_stamp(g_sprite_ids[12], false, 6);
		kernel_seq_timeout(seqIdx, g_sequence_ids[12]);
		kernel_seq_depth(g_sequence_ids[12], 8);
		kernel_timing_trigger(10, 98);
	}
	break;

	case 98:
		kernel_seq_delete(g_sequence_ids[12]);
		g_sequence_ids[12] = kernel_seq_backward(g_sprite_ids[12], false, 10, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[12], 1, 4);
		kernel_seq_depth(g_sequence_ids[12], 8);
		kernel_seq_trigger(g_sequence_ids[12], KERNEL_TRIGGER_EXPIRE, 0, 99);
		break;

	case 99:
	{
		int seqIdx = g_sequence_ids[12];
		g_sequence_ids[9] = kernel_seq_stamp(g_sprite_ids[9], false, 1);
		local._bartenderSteady = true;
		int idx = kernel_add_dynamic(words_bartender, words_walkto, 0, g_sequence_ids[9], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 228, 83, FACING_SOUTH);
		kernel_seq_timeout(seqIdx, g_sequence_ids[9]);
		kernel_seq_depth(g_sequence_ids[9], 8);
		if (local._bartenderMode == 20) {
			kernel_message_purge();
			kernel_message_add(quote_string(kernel.quotes, 0x1F3), 210, 41, 0xFDFC, 100, 0, 0);
			kernel_timing_trigger(5, 100);
			local._talkTimer = 180;
			local._roxMode = 22;
			kernel_timing_trigger(65, 92);
		} else if ((local._bartenderMode == 21) || (local._bartenderMode == 22)) {
			player.commands_allowed = true;
			local._bartenderReady = true;
		}

	}
	break;

	default:
		break;
	}

	switch (kernel.trigger) {
	case 86:
		local._bartenderReady = false;
		player.commands_allowed = false;
		if ((local._bartenderHandsHips) || (local._bartenderTalking)) {
			kernel_seq_delete(g_sequence_ids[10]);
			local._bartenderHandsHips = false;
			local._bartenderTalking = false;
		} else if (local._bartenderLooksLeft) {
			kernel_seq_delete(g_sequence_ids[11]);
			local._bartenderLooksLeft = false;
		} else if (local._bartenderSteady) {
			kernel_seq_delete(g_sequence_ids[9]);
			local._bartenderSteady = false;
		}
		g_sequence_ids[9] = kernel_seq_forward(g_sprite_ids[9], false, 10, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[9], 1, 9);
		kernel_seq_depth(g_sequence_ids[9], 8);
		kernel_seq_trigger(g_sequence_ids[9], KERNEL_TRIGGER_EXPIRE, 0, 87);
		break;

	case 87:
	{
		int seqIdx = g_sequence_ids[9];
		g_sequence_ids[9] = kernel_seq_stamp(g_sprite_ids[9], false, 9);
		kernel_seq_depth(g_sequence_ids[9], 8);
		kernel_seq_timeout(seqIdx, g_sequence_ids[9]);
		kernel_timing_trigger(10, 89);

		g_sequence_ids[8] = kernel_seq_forward(g_sprite_ids[8], false, 10, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[8], 1, 4);
		kernel_seq_depth(g_sequence_ids[8], 7);
		kernel_seq_trigger(g_sequence_ids[8], KERNEL_TRIGGER_EXPIRE, 0, 88);
	}
	break;

	case 88:
		g_sequence_ids[8] = kernel_seq_stamp(g_sprite_ids[8], false, 4);
		kernel_seq_depth(g_sequence_ids[8], 7);
		global[kBottleDisplayed] = true;
		break;

	case 89:
		kernel_seq_delete(g_sequence_ids[9]);
		g_sequence_ids[9] = kernel_seq_stamp(g_sprite_ids[9], false, 6);
		kernel_seq_depth(g_sequence_ids[9], 8);
		kernel_timing_trigger(10, 90);
		break;

	case 90:
		kernel_seq_delete(g_sequence_ids[9]);
		g_sequence_ids[9] = kernel_seq_stamp(g_sprite_ids[9], false, 5);
		kernel_seq_depth(g_sequence_ids[9], 8);
		kernel_timing_trigger(10, 91);
		break;

	case 91:
	{
		kernel_seq_delete(g_sequence_ids[9]);
		g_sequence_ids[9] = kernel_seq_stamp(g_sprite_ids[9], false, 1);
		int idx = kernel_add_dynamic(words_bartender, words_walkto, 0, g_sequence_ids[9], 0, 0, 0, 0);
		kernel_dynamic_walk(idx, 228, 83, FACING_SOUTH);
		kernel_seq_depth(g_sequence_ids[9], 8);
		local._bartenderSteady = true;
		player.commands_allowed = true;
		local._bartenderReady = true;
	}
	break;

	default:
		break;
	}

	if (!local._waitingGinnyMove && !local._ginnyLooking) {
		local._waitingGinnyMove = true;
		++local._beatCounter;
		if (local._beatCounter >= 20) {
			local._ginnyLooking = true;
			local._beatCounter = 0;
			kernel_timing_trigger(60, 54);
		} else {
			kernel_timing_trigger(30, 75);
		}
	}

	switch (kernel.trigger) {
	case 75:
		kernel_seq_delete(g_sequence_ids[5]);
		local._bigBeatFl = !local._bigBeatFl;

		if (local._bigBeatFl) {
			g_sequence_ids[5] = kernel_seq_stamp(g_sprite_ids[5], false, 5);
			kernel_timing_trigger(8, 130);
		} else {
			g_sequence_ids[5] = kernel_seq_stamp(g_sprite_ids[5], false, 5);
			kernel_timing_trigger(8, 53);
		}

		kernel_seq_depth(g_sequence_ids[5], 1);
		break;

	case 130:
		kernel_seq_delete(g_sequence_ids[5]);
		g_sequence_ids[5] = kernel_seq_stamp(g_sprite_ids[5], false, 6);
		kernel_seq_depth(g_sequence_ids[5], 1);
		kernel_timing_trigger(8, 131);
		break;

	case 131:
		kernel_seq_delete(g_sequence_ids[5]);
		g_sequence_ids[5] = kernel_seq_stamp(g_sprite_ids[5], false, 5);
		kernel_seq_depth(g_sequence_ids[5], 1);
		kernel_timing_trigger(8, 53);
		break;

	case 53:
		kernel_seq_delete(g_sequence_ids[5]);
		g_sequence_ids[5] = kernel_seq_stamp(g_sprite_ids[5], false, 1);
		kernel_seq_depth(g_sequence_ids[5], 1);
		local._waitingGinnyMove = false;
		break;

	default:
		break;
	}

	if ((kernel.trigger == 54) && local._ginnyLooking) {
		++local._beatCounter;
		if (local._beatCounter >= 10) {
			local._ginnyLooking = false;
			local._waitingGinnyMove = false;
			local._beatCounter = 0;
			local._bigBeatFl = true;
		} else {
			kernel_seq_delete(g_sequence_ids[5]);
			g_sequence_ids[5] = kernel_seq_stamp(g_sprite_ids[5], false, g_engine->getRandomNumber(1, 4));
			kernel_seq_depth(g_sequence_ids[5], 1);
			kernel_timing_trigger(60, 54);
		}
	}

	if (local._bartenderReady) {
		if (g_engine->getRandomNumber(1, 250) == 1) {
			if (local._bartenderLooksLeft) {
				kernel_seq_delete(g_sequence_ids[11]);
				local._bartenderLooksLeft = false;
			} else if (local._bartenderHandsHips) {
				kernel_seq_delete(g_sequence_ids[10]);
				local._bartenderHandsHips = false;
			} else if (local._bartenderSteady) {
				kernel_seq_delete(g_sequence_ids[9]);
				local._bartenderSteady = false;
			}

			switch (g_engine->getRandomNumber(1, 3)) {
			case 1:
			{
				g_sequence_ids[10] = kernel_seq_stamp(g_sprite_ids[10], false, 4);
				kernel_seq_depth(g_sequence_ids[10], 8);
				int idx = kernel_add_dynamic(words_bartender, words_walkto, 0, g_sequence_ids[10], 0, 0, 0, 0);
				kernel_dynamic_walk(idx, 228, 83, FACING_SOUTH);
				local._bartenderHandsHips = true;
			}
			break;

			case 2:
			{
				g_sequence_ids[11] = kernel_seq_stamp(g_sprite_ids[11], false, 2);
				kernel_seq_depth(g_sequence_ids[11], 8);
				int idx = kernel_add_dynamic(words_bartender, words_walkto, 0, g_sequence_ids[11], 0, 0, 0, 0);
				kernel_dynamic_walk(idx, 228, 83, FACING_SOUTH);
				local._bartenderLooksLeft = true;
			}
			break;

			case 3:
			{
				g_sequence_ids[9] = kernel_seq_stamp(g_sprite_ids[9], false, 1);
				kernel_seq_depth(g_sequence_ids[9], 8);
				int idx = kernel_add_dynamic(words_bartender, words_walkto, 0, g_sequence_ids[9], 0, 0, 0, 0);
				kernel_dynamic_walk(idx, 228, 83, FACING_SOUTH);
				local._bartenderSteady = true;
			}
			break;

			default:
				break;
			}
		}
	}

	if (kernel.trigger == 76) {
		int seqIdx = g_sequence_ids[6];
		g_sequence_ids[6] = kernel_seq_stamp(g_sprite_ids[6], false, 11);
		kernel_seq_depth(g_sequence_ids[6], 5);
		kernel_seq_timeout(seqIdx, g_sequence_ids[6]);
		if (!global[kBeenThruHelgaScene]) {
			player.commands_allowed = false;
			local._cutSceneNeeded = true;
		} else {
			player.commands_allowed = true;
		}
		local._roxOnStool = true;
	}

	switch (kernel.trigger) {
	case 77:
		player.commands_allowed = false;
		kernel_seq_delete(g_sequence_ids[6]);
		g_sequence_ids[7] = kernel_seq_forward(g_sprite_ids[7], false, 10, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[7], 10, 12);
		kernel_seq_depth(g_sequence_ids[7], 5);
		kernel_seq_trigger(g_sequence_ids[7], KERNEL_TRIGGER_EXPIRE, 0, 78);
		break;

	case 78:
	{
		g_engine->_soundManager->command(57, 0);
		int seqIdx = g_sequence_ids[7];
		inter_give_to_player(OBJ_REPAIR_LIST);
		kernel_seq_delete(g_sequence_ids[14]);
		g_sequence_ids[7] = kernel_seq_backward(g_sprite_ids[7], false, 10, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[7], 10, 12);
		kernel_seq_depth(g_sequence_ids[7], 5);
		kernel_seq_timeout(seqIdx, g_sequence_ids[7]);
		kernel_seq_trigger(g_sequence_ids[7], KERNEL_TRIGGER_EXPIRE, 0, 79);
	}
	break;

	case 79:
	{
		int seqIdx = g_sequence_ids[7];
		g_sequence_ids[6] = kernel_seq_stamp(g_sprite_ids[6], false, 11);
		kernel_seq_depth(g_sequence_ids[6], 5);
		kernel_seq_timeout(seqIdx, g_sequence_ids[6]);
		kernel_timing_trigger(20, 180);
	}
	break;

	case 180:
		object_examine(OBJ_REPAIR_LIST, 40240, 0);
		player.commands_allowed = true;
		break;

	default:
		break;
	}

	if (local._cutSceneNeeded && local._cutSceneReady) {
		local._cutSceneNeeded = false;
		kernel_timing_trigger(20, 55);
		local._helgaReady = false;
		local._bartenderReady = false;
	}

	if (g_engine->getRandomNumber(1, 25) == 1) {
		if (local._lightOn) {
			kernel_seq_delete(g_sequence_ids[0]);
			local._lightOn = false;
		} else {
			g_sequence_ids[0] = kernel_seq_stamp(g_sprite_ids[0], false, 1);
			local._lightOn = true;
		}
	}

	if (!local._blowingSmoke && (g_engine->getRandomNumber(1, 300) == 1)) {
		g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 8, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[1], 1, 14);
		kernel_seq_depth(g_sequence_ids[1], 14);
		kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 30);
		local._blowingSmoke = true;
	}

	switch (kernel.trigger) {
	case 30:
		g_sequence_ids[1] = kernel_seq_backward(g_sprite_ids[1], false, 8, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[1], 1, 5);
		kernel_seq_depth(g_sequence_ids[1], 14);
		kernel_seq_trigger(g_sequence_ids[1], KERNEL_TRIGGER_EXPIRE, 0, 31);
		break;

	case 31:
		local._blowingSmoke = false;
		break;

	default:
		break;
	}

	if (!local._leftWomanMoving) {
		if (g_engine->getRandomNumber(1, 1000) == 1) {
			switch (g_engine->getRandomNumber(1, 2)) {
			case 1:
				g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 12, 0, 0, 1);
				kernel_seq_range(g_sequence_ids[2], 1, 8);
				kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 33);
				local._leftWomanMoving = true;
				break;

			case 2:
				g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 1);
				kernel_timing_trigger(12, 35);
				local._leftWomanMoving = true;
				break;

			default:
				break;
			}
		}
	}

	switch (kernel.trigger) {
	case 33:
	{
		int seqIdx = g_sequence_ids[2];
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 9);
		kernel_seq_timeout(seqIdx, g_sequence_ids[2]);
		kernel_timing_trigger(g_engine->getRandomNumber(60, 250), 34);
	}
	break;

	case 34:
		kernel_seq_delete(g_sequence_ids[2]);
		g_sequence_ids[2] = kernel_seq_backward(g_sprite_ids[2], false, 12, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[2], 1, 8);
		kernel_seq_trigger(g_sequence_ids[2], KERNEL_TRIGGER_EXPIRE, 0, 38);
		break;

	case 35:
		kernel_seq_delete(g_sequence_ids[2]);
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 2);
		kernel_timing_trigger(g_engine->getRandomNumber(60, 300), 36);
		break;

	case 36:
		kernel_seq_delete(g_sequence_ids[2]);
		g_sequence_ids[2] = kernel_seq_stamp(g_sprite_ids[2], false, 1);
		kernel_timing_trigger(12, 37);
		break;

	case 37:
		kernel_seq_delete(g_sequence_ids[2]);
		local._leftWomanMoving = false;
		break;

	case 38:
		local._leftWomanMoving = false;
		break;

	default:
		break;
	}

	if (!local._rightWomanMoving) {
		if (g_engine->getRandomNumber(1, 300) == 1) {
			switch (g_engine->getRandomNumber(1, 4)) {
			case 1:
				g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 12, 0, 0, 1);
				kernel_seq_range(g_sequence_ids[3], 1, 4);
				kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 32);
				local._rightWomanMoving = true;
				break;

			case 2:
				g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 12, 0, 0, 2);
				kernel_seq_range(g_sequence_ids[3], 4, 5);
				kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 32);
				local._rightWomanMoving = true;
				break;

			case 3:
				g_sequence_ids[3] = kernel_seq_pingpong(g_sprite_ids[3], false, 12, 0, 0, 2);
				kernel_seq_range(g_sequence_ids[3], 4, 5);
				kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 32);
				local._rightWomanMoving = true;
				break;

			case 4:
				g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 12, 0, 0, 2);
				kernel_seq_range(g_sequence_ids[3], 3, 4);
				kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 32);
				local._rightWomanMoving = true;
				break;

			default:
				break;
			}
		}
	}

	if (kernel.trigger == 32)
		local._rightWomanMoving = false;

	if (kernel_anim[0].frame == 1) {
		switch (g_engine->getRandomNumber(1, 50)) {
		case 1:
			kernel_reset_animation(0, 2);
			break;

		case 2:
			kernel_reset_animation(0, 7);
			break;

		case 3:
			kernel_reset_animation(0, 11);
			break;

		default:
			kernel_reset_animation(0, 0);
			break;
		}
	}

	if ((kernel_anim[0].frame == 4) && (local._drinkTimer < 10)) {
		++local._drinkTimer;
		kernel_reset_animation(0, 3);
	}

	if (local._drinkTimer == 10) {
		local._drinkTimer = 0;
		kernel_reset_animation(0, 4);
		kernel_anim[0].frame = 5;
	}


	switch (kernel_anim[0].frame) {
	case 6:
	case 10:
	case 14:
		kernel_reset_animation(0, 0);
		break;

	default:
		break;
	}

	switch (kernel.trigger) {
	case 39:
		kernel_message_purge();
		kernel_message_add(quote_string(kernel.quotes, 0x1D8), 89, 67, 0xFDFC, 120, 0, 32);
		kernel_timing_trigger(150, 40);
		break;

	case 40:
		kernel_message_add(quote_string(kernel.quotes, 0x1D9), 89, 67, 0xFDFC, 120, 0, 32);
		kernel_timing_trigger(150, 41);
		break;

	case 41:
		kernel_message_add(quote_string(kernel.quotes, 0x1DA), 89, 67, 0xFDFC, 120, 0, 32);
		player.commands_allowed = true;
		break;

	case 42:
		kernel_message_purge();
		kernel_message_add(quote_string(kernel.quotes, 0x1DC), 89, 67, 0xFDFC, 120, 0, 32);
		kernel_timing_trigger(150, 43);
		break;

	case 43:
		kernel_message_add(quote_string(kernel.quotes, 0x1DD), 89, 67, 0xFDFC, 120, 0, 32);
		player.commands_allowed = true;
		break;

	case 44:
		player.commands_allowed = true;
		break;

	default:
		break;
	}

	switch (kernel.trigger) {
	case 55:
		if (local._bartenderHandsHips) {
			kernel_seq_delete(g_sequence_ids[10]);
			local._bartenderHandsHips = false;
			g_sequence_ids[9] = kernel_seq_stamp(g_sprite_ids[9], false, 1);
			int idx = kernel_add_dynamic(words_bartender, words_walkto, 0, g_sequence_ids[9], 0, 0, 0, 0);
			kernel_dynamic_walk(idx, 228, 83, FACING_SOUTH);
			local._bartenderSteady = true;
			kernel_seq_depth(g_sequence_ids[9], 8);
		} else if (local._bartenderLooksLeft) {
			kernel_seq_delete(g_sequence_ids[11]);
			local._bartenderLooksLeft = false;
			g_sequence_ids[9] = kernel_seq_stamp(g_sprite_ids[9], false, 1);
			int idx = kernel_add_dynamic(words_bartender, words_walkto, 0, g_sequence_ids[9], 0, 0, 0, 0);
			kernel_dynamic_walk(idx, 228, 83, FACING_SOUTH);
			local._bartenderSteady = true;
			kernel_seq_depth(g_sequence_ids[9], 8);
		}
		player.commands_allowed = false;
		kernel_message_purge();
		kernel_message_add(quote_string(kernel.quotes, 0x1FE), 180, 47, 0xFBFA, 100, 0, 0);
		kernel_timing_trigger(120, 56);
		break;

	case 56:
		kernel_seq_delete(g_sequence_ids[13]);
		g_sequence_ids[13] = kernel_seq_forward(g_sprite_ids[13], false, 10, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[13], 2, 5);
		kernel_seq_depth(g_sequence_ids[13], 8);
		kernel_seq_trigger(g_sequence_ids[13], KERNEL_TRIGGER_EXPIRE, 0, 57);
		break;

	case 57:
	{
		int seqIdx = g_sequence_ids[13];
		kernel_seq_delete(g_sequence_ids[20]);
		g_sequence_ids[13] = kernel_seq_forward(g_sprite_ids[13], false, 10, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[13], 6, 9);
		kernel_seq_depth(g_sequence_ids[13], 8);
		kernel_seq_timeout(seqIdx, g_sequence_ids[13]);
		kernel_seq_trigger(g_sequence_ids[13], KERNEL_TRIGGER_EXPIRE, 0, 58);
	}
	break;

	case 58:
	{
		int seqIdx = g_sequence_ids[13];
		g_sequence_ids[13] = kernel_seq_backward(g_sprite_ids[13], false, 10, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[13], 5, 9);
		kernel_seq_depth(g_sequence_ids[13], 8);
		kernel_seq_timeout(seqIdx, g_sequence_ids[13]);
		kernel_seq_trigger(g_sequence_ids[13], KERNEL_TRIGGER_EXPIRE, 0, 59);
	}
	break;

	case 59:
	{
		int seqIdx = g_sequence_ids[13];
		g_sequence_ids[13] = kernel_seq_backward(g_sprite_ids[13], false, 10, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[13], 1, 4);
		kernel_seq_depth(g_sequence_ids[13], 8);
		kernel_seq_timeout(seqIdx, g_sequence_ids[13]);
		kernel_seq_trigger(g_sequence_ids[13], KERNEL_TRIGGER_EXPIRE, 0, 60);
		g_sequence_ids[20] = kernel_seq_stamp(g_sprite_ids[20], false, 1);
		kernel_seq_depth(g_sequence_ids[20], 8);
		kernel_seq_loc(g_sequence_ids[20], 234, 72);
	}
	break;

	case 60:
	{
		int seqIdx = g_sequence_ids[13];
		g_sequence_ids[13] = kernel_seq_stamp(g_sprite_ids[13], false, 2);
		kernel_seq_timeout(seqIdx, g_sequence_ids[13]);
		kernel_seq_depth(g_sequence_ids[13], 8);
		kernel_timing_trigger(10, 61);
	}
	break;

	case 61:
		kernel_message_purge();
		kernel_message_add(quote_string(kernel.quotes, 0x1FF), 181, 33, 0xFBFA, 130, 0, 0);
		kernel_message_add(quote_string(kernel.quotes, 0x200), 171, 47, 0xFBFA, 130, 0, 0);
		kernel_timing_trigger(150, 63);
		kernel_seq_delete(g_sequence_ids[13]);
		g_sequence_ids[13] = kernel_seq_pingpong(g_sprite_ids[13], false, 30, 0, 0, 4);
		kernel_seq_range(g_sequence_ids[13], 10, 11);
		kernel_seq_depth(g_sequence_ids[13], 8);
		kernel_seq_trigger(g_sequence_ids[13], KERNEL_TRIGGER_EXPIRE, 0, 62);
		break;

	case 62:
	{
		int seqIdx = g_sequence_ids[13];
		g_sequence_ids[13] = kernel_seq_stamp(g_sprite_ids[13], false, 2);
		kernel_seq_timeout(seqIdx, g_sequence_ids[13]);
		kernel_seq_depth(g_sequence_ids[13], 8);
	}
	break;

	case 63:
		kernel_message_purge();
		kernel_message_add(quote_string(kernel.quotes, 0x201), 160, 33, 0xFBFA, 130, 0, 0);
		kernel_message_add(quote_string(kernel.quotes, 0x202), 165, 47, 0xFBFA, 130, 0, 0);
		kernel_timing_trigger(150, 64);
		break;

	case 64:
		kernel_message_purge();
		kernel_message_add(quote_string(kernel.quotes, 0x1E0), 210, 27, 0xFDFC, 130, 0, 0);
		kernel_message_add(quote_string(kernel.quotes, 0x1E1), 198, 41, 0xFDFC, 130, 0, 0);
		kernel_timing_trigger(150, 65);
		kernel_timing_trigger(1, 100);
		local._talkTimer = 1130;
		break;

	case 65:
		g_engine->_soundManager->command(30, 0);
		g_sequence_ids[16] = kernel_seq_forward(g_sprite_ids[16], false, 10, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[16], 1, 6);
		kernel_seq_depth(g_sequence_ids[16], 9);
		kernel_seq_trigger(g_sequence_ids[16], KERNEL_TRIGGER_EXPIRE, 0, 66);
		break;

	case 66:
	{
		int seqIdx = g_sequence_ids[16];
		g_sequence_ids[16] = kernel_seq_forward(g_sprite_ids[16], false, 10, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[16], 7, 37);
		kernel_seq_depth(g_sequence_ids[16], 9);
		kernel_seq_timeout(seqIdx, g_sequence_ids[16]);
		kernel_seq_trigger(g_sequence_ids[16], KERNEL_TRIGGER_EXPIRE, 0, 68);

		kernel_seq_delete(g_sequence_ids[13]);
		g_sequence_ids[13] = kernel_seq_forward(g_sprite_ids[13], false, 10, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[13], 12, 13);
		kernel_seq_depth(g_sequence_ids[13], 8);
		kernel_seq_trigger(g_sequence_ids[13], KERNEL_TRIGGER_EXPIRE, 0, 67);
	}
	break;

	case 67:
	{
		int seqIdx = g_sequence_ids[13];
		g_sequence_ids[13] = kernel_seq_stamp(g_sprite_ids[13], false, 13);
		kernel_seq_timeout(seqIdx, g_sequence_ids[13]);
		kernel_seq_depth(g_sequence_ids[13], 8);
	}
	break;

	case 68:
		kernel_seq_delete(g_sequence_ids[13]);
		g_sequence_ids[13] = kernel_seq_backward(g_sprite_ids[13], false, 10, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[13], 12, 13);
		kernel_seq_depth(g_sequence_ids[13], 8);
		kernel_seq_trigger(g_sequence_ids[13], KERNEL_TRIGGER_EXPIRE, 0, 69);
		break;

	case 69:
	{
		int seqIdx = g_sequence_ids[13];
		g_sequence_ids[13] = kernel_seq_pingpong(g_sprite_ids[13], false, 25, 0, 0, 4);
		kernel_seq_range(g_sequence_ids[13], 10, 11);
		kernel_seq_depth(g_sequence_ids[13], 8);
		kernel_seq_timeout(seqIdx, g_sequence_ids[13]);

		kernel_message_purge();
		kernel_message_add(quote_string(kernel.quotes, 0x203), 179, 33, 0xFBFA, 130, 0, 0);
		kernel_message_add(quote_string(kernel.quotes, 0x204), 167, 47, 0xFBFA, 130, 0, 0);
		kernel_timing_trigger(150, 71);
		kernel_seq_trigger(g_sequence_ids[13], KERNEL_TRIGGER_EXPIRE, 0, 70);
	}
	break;

	case 70:
		g_sequence_ids[13] = kernel_seq_pingpong(g_sprite_ids[13], false, 25, 0, 0, 4);
		kernel_seq_range(g_sequence_ids[13], 10, 12);
		kernel_seq_depth(g_sequence_ids[13], 8);
		kernel_seq_trigger(g_sequence_ids[13], KERNEL_TRIGGER_EXPIRE, 0, 72);
		break;

	case 71:
		kernel_timing_trigger(210, 73);
		kernel_message_purge();
		kernel_message_add(quote_string(kernel.quotes, 0x205), 168, 33, 0xFBFA, 180, 0, 0);
		kernel_message_add(quote_string(kernel.quotes, 0x206), 151, 47, 0xFBFA, 180, 0, 0);
		if (!player_has(OBJ_REPAIR_LIST))
			local._activeArrows = true;
		break;

	case 72:
	{
		int seqIdx = g_sequence_ids[13];
		g_sequence_ids[13] = kernel_seq_stamp(g_sprite_ids[13], false, 2);
		kernel_seq_timeout(seqIdx, g_sequence_ids[13]);
		kernel_seq_depth(g_sequence_ids[13], 8);
	}
	break;

	case 73:
		kernel_message_purge();
		kernel_message_add(quote_string(kernel.quotes, 0x207), 177, 33, 0xFBFA, 150, 0, 0);
		kernel_message_add(quote_string(kernel.quotes, 0x208), 172, 47, 0xFBFA, 150, 0, 0);
		local._bartenderSteady = true;
		player.commands_allowed = true;
		local._helgaReady = true;
		local._bartenderReady = true;
		global[kBeenThruHelgaScene] = true;
		local._activeArrows = false;
		break;

	default:
		break;
	}

	if (local._helgaReady) {
		int rndVal = g_engine->getRandomNumber(1, 1000);
		if (rndVal < 6)
			switch (rndVal) {
			case 1:
				local._cutSceneReady = false;
				local._helgaReady = false;
				kernel_seq_delete(g_sequence_ids[13]);
				g_sequence_ids[13] = kernel_seq_forward(g_sprite_ids[13], false, 10, 0, 0, 1);
				kernel_seq_range(g_sequence_ids[13], 2, 5);
				kernel_seq_depth(g_sequence_ids[13], 8);
				kernel_seq_trigger(g_sequence_ids[13], KERNEL_TRIGGER_EXPIRE, 0, 82);
				break;

			case 2:
				local._cutSceneReady = false;
				local._helgaReady = false;
				kernel_seq_delete(g_sequence_ids[13]);
				g_sequence_ids[13] = kernel_seq_pingpong(g_sprite_ids[13], false, 15, 0, 0, 2);
				kernel_seq_range(g_sequence_ids[13], 11, 13);
				kernel_seq_depth(g_sequence_ids[13], 8);
				kernel_seq_trigger(g_sequence_ids[13], KERNEL_TRIGGER_EXPIRE, 0, 26);
				break;

			case 3:
				local._cutSceneReady = false;
				local._helgaReady = false;
				kernel_seq_delete(g_sequence_ids[13]);
				g_sequence_ids[13] = kernel_seq_forward(g_sprite_ids[13], false, 10, 0, 0, 1);
				kernel_seq_range(g_sequence_ids[13], 10, 11);
				kernel_seq_depth(g_sequence_ids[13], 8);
				kernel_seq_trigger(g_sequence_ids[13], KERNEL_TRIGGER_EXPIRE, 0, 26);
				break;

			case 4:
				local._cutSceneReady = false;
				local._helgaReady = false;
				kernel_seq_delete(g_sequence_ids[13]);
				g_sequence_ids[13] = kernel_seq_pingpong(g_sprite_ids[13], false, 15, 0, 0, 2);
				kernel_seq_range(g_sequence_ids[13], 14, 15);
				kernel_seq_depth(g_sequence_ids[13], 8);
				kernel_seq_trigger(g_sequence_ids[13], KERNEL_TRIGGER_EXPIRE, 0, 26);
				break;

			case 5:
				local._cutSceneReady = false;
				local._helgaReady = false;
				kernel_seq_delete(g_sequence_ids[13]);
				g_sequence_ids[13] = kernel_seq_forward(g_sprite_ids[13], false, 10, 0, 0, 1);
				kernel_seq_range(g_sequence_ids[13], 16, 19);
				kernel_seq_depth(g_sequence_ids[13], 8);
				kernel_seq_trigger(g_sequence_ids[13], KERNEL_TRIGGER_EXPIRE, 0, 80);
				break;

			default:
				break;
			}
	}

	if (kernel.trigger == 80) {
		int seqIdx = g_sequence_ids[13];
		g_sequence_ids[13] = kernel_seq_stamp(g_sprite_ids[13], false, 19);
		kernel_seq_timeout(seqIdx, g_sequence_ids[13]);
		kernel_seq_depth(g_sequence_ids[13], 8);
		kernel_timing_trigger(g_engine->getRandomNumber(60, 120), 81);
	}

	if (kernel.trigger == 81) {
		kernel_seq_delete(g_sequence_ids[13]);
		g_sequence_ids[13] = kernel_seq_backward(g_sprite_ids[13], false, 10, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[13], 16, 19);
		kernel_seq_depth(g_sequence_ids[13], 8);
		kernel_seq_trigger(g_sequence_ids[13], KERNEL_TRIGGER_EXPIRE, 0, 26);
	}

	if (kernel.trigger == 26) {
		int seqIdx = g_sequence_ids[13];
		g_sequence_ids[13] = kernel_seq_stamp(g_sprite_ids[13], false, 2);
		kernel_seq_timeout(seqIdx, g_sequence_ids[13]);
		kernel_seq_depth(g_sequence_ids[13], 8);
		local._cutSceneReady = true;

		if (!local._cutSceneNeeded)
			local._helgaReady = true;
	}

	if (kernel.trigger == 82) {
		kernel_seq_delete(g_sequence_ids[20]);
		int seqIdx = g_sequence_ids[13];
		g_sequence_ids[13] = kernel_seq_forward(g_sprite_ids[13], false, 10, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[13], 6, 9);
		kernel_seq_depth(g_sequence_ids[13], 8);
		kernel_seq_timeout(seqIdx, g_sequence_ids[13]);
		kernel_seq_trigger(g_sequence_ids[13], KERNEL_TRIGGER_EXPIRE, 0, 83);
	}

	if (kernel.trigger == 83) {
		int seqIdx = g_sequence_ids[13];
		g_sequence_ids[13] = kernel_seq_backward(g_sprite_ids[13], false, 10, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[13], 5, 9);
		kernel_seq_depth(g_sequence_ids[13], 8);
		kernel_seq_timeout(seqIdx, g_sequence_ids[13]);
		kernel_seq_trigger(g_sequence_ids[13], KERNEL_TRIGGER_EXPIRE, 0, 84);
	}

	if (kernel.trigger == 84) {
		int seqIdx = g_sequence_ids[13];
		g_sequence_ids[13] = kernel_seq_backward(g_sprite_ids[13], false, 10, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[13], 1, 4);
		kernel_seq_depth(g_sequence_ids[13], 8);
		kernel_seq_timeout(seqIdx, g_sequence_ids[13]);
		kernel_seq_trigger(g_sequence_ids[13], KERNEL_TRIGGER_EXPIRE, 0, 85);

		g_sequence_ids[20] = kernel_seq_stamp(g_sprite_ids[20], false, 1);
		kernel_seq_depth(g_sequence_ids[20], 8);
		kernel_seq_loc(g_sequence_ids[20], 234, 72);
	}

	if (kernel.trigger == 85) {
		int seqIdx = g_sequence_ids[13];
		g_sequence_ids[13] = kernel_seq_stamp(g_sprite_ids[13], false, 2);
		kernel_seq_timeout(seqIdx, g_sequence_ids[13]);
		kernel_seq_depth(g_sequence_ids[13], 8);
		local._cutSceneReady = true;
		if (!local._cutSceneNeeded)
			local._helgaReady = true;
	}

	if (kernel.trigger == 102) {
		kernel_seq_delete(g_sequence_ids[6]);
		player.clock = kernel.clock - player.frame_delay;
		g_sequence_ids[7] = kernel_seq_forward(g_sprite_ids[7], false, 8, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[7], 14, 18);
		kernel_seq_depth(g_sequence_ids[7], 5);
		kernel_seq_trigger(g_sequence_ids[7], KERNEL_TRIGGER_EXPIRE, 0, 103);
	} else if (kernel.trigger == 103) {
		player.clock = kernel.clock + player.frame_delay;
		local._roxOnStool = false;
		player.facing = FACING_SOUTH;
		player_select_series();
		player.walker_visible = true;
		player.commands_allowed = true;
		player.ready_to_walk = true;
	}

	if (local._activeArrows) {
		if (!local._activeArrow1) {
			g_sequence_ids[17] = kernel_seq_stamp(g_sprite_ids[17], false, 1);
			kernel_seq_depth(g_sequence_ids[17], 1);
			kernel_timing_trigger(g_engine->getRandomNumber(10, 15), 49);
			local._activeArrow1 = true;
		}

		if (!local._activeArrow2) {
			g_sequence_ids[18] = kernel_seq_stamp(g_sprite_ids[18], false, 1);
			kernel_seq_depth(g_sequence_ids[18], 1);
			kernel_timing_trigger(g_engine->getRandomNumber(10, 15), 50);
			local._activeArrow2 = true;
		}

		if (!local._activeArrow3) {
			g_sequence_ids[19] = kernel_seq_stamp(g_sprite_ids[19], false, 1);
			kernel_seq_depth(g_sequence_ids[19], 1);
			kernel_timing_trigger(g_engine->getRandomNumber(10, 15), 51);
			local._activeArrow3 = true;
		}
	}

	if (kernel.trigger == 49) {
		kernel_seq_delete(g_sequence_ids[17]);
		kernel_timing_trigger(g_engine->getRandomNumber(10, 15), 45);
	}

	if (kernel.trigger == 45)
		local._activeArrow1 = false;

	if (kernel.trigger == 50) {
		kernel_seq_delete(g_sequence_ids[18]);
		kernel_timing_trigger(g_engine->getRandomNumber(10, 15), 46);
	}

	if (kernel.trigger == 46)
		local._activeArrow2 = false;

	if (kernel.trigger == 51) {
		kernel_seq_delete(g_sequence_ids[19]);
		kernel_timing_trigger(g_engine->getRandomNumber(10, 15), 47);
	}

	if (kernel.trigger == 47)
		local._activeArrow3 = false;
}

static void room_402_pre_parser() {
	if (player_said_2(sit_on, bar_stool) && (player.prepare_walk_x != 248))
		player_walk(232, 112, FACING_EAST);

	if (player_said_2(walkto, woman_on_balcony))
		player.need_to_walk = player.ready_to_walk;

	if (!local._roxOnStool && player_said_2(take, credit_chip) && !player_has(OBJ_CREDIT_CHIP))
		player_walk(246, 108, FACING_NORTH);

	if (player_said_1(take))
		player.need_to_walk = false;

	if (player_said_2(take, credit_chip) && !local._roxOnStool)
		player.need_to_walk = true;

	if (local._roxOnStool) {
		if (player_said_1(look) || player_said_1(bar_stool) || player_said_1(talkto))
			player.need_to_walk = false;

		if (player_said_2(take, repair_list) || player_said_2(take, credit_chip))
			player.need_to_walk = false;

		if (player_said_2(talkto, woman_in_chair) || player_said_2(talkto, woman_in_alcove))
			player.need_to_walk = player.ready_to_walk;

		if (player.need_to_walk) {
			player.facing = FACING_SOUTH;
			player.ready_to_walk = false;
			player.commands_allowed = false;
			kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
			kernel_timing_trigger(1, 102);
		}
	}

	if (player_said_2(take, repair_list) && !local._roxOnStool && !player_has(OBJ_REPAIR_LIST))
		player_walk(191, 99, FACING_NORTHEAST);

	if (player_said_2(talkto, bartender) && !local._roxOnStool)
		player_walk(228, 83, FACING_SOUTH);

	if (player_said_2(talkto, repair_woman) && !local._roxOnStool)
		player_walk(208, 102, FACING_NORTHEAST);
}

static void room_402_parser() {
	if (player_said_2(take, repair_list) && object_is_here(OBJ_REPAIR_LIST) && local._roxOnStool) {
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(1, 77);
		player.need_to_walk = false;
	} else if (player_said_2(take, repair_list) && object_is_here(OBJ_REPAIR_LIST) && !local._roxOnStool) {
		if (kernel.trigger == 0) {
			player.commands_allowed = false;
			player.walker_visible = false;
			g_sequence_ids[21] = kernel_seq_pingpong(g_sprite_ids[21], false, 7, 0, 0, 2);
			kernel_seq_range(g_sequence_ids[21], 1, 2);
			kernel_seq_player(g_sequence_ids[21], false);
			kernel_seq_trigger(g_sequence_ids[21], KERNEL_TRIGGER_SPRITE, 2, 165);
			kernel_seq_trigger(g_sequence_ids[21], KERNEL_TRIGGER_EXPIRE, 0, 166);
		} else if (kernel.trigger == 165) {
			kernel_seq_delete(g_sequence_ids[14]);
			inter_give_to_player(OBJ_REPAIR_LIST);
		}
	} else if (kernel.trigger == 166) {
		player.clock = kernel.clock + player.frame_delay;
		player.walker_visible = true;
		kernel_timing_trigger(20, 167);
	} else if (kernel.trigger == 167) {
		object_examine(OBJ_REPAIR_LIST, 40240, 0);
		player.commands_allowed = true;
	} else if (inter_input_mode == INTER_CONVERSATION)
		handleDialogs();
	else if (player_said_2(walk_into, corridor_to_south))
		new_room = 401;
	else if (player_said_2(walk_onto, dance_floor))
		; // just... nothing
	else if (player_said_2(talkto, repair_woman)) {
		switch (kernel.trigger) {
		case 0:
		{
			player.commands_allowed = false;
			int random = g_engine->getRandomNumber(1, 3);
			if (local._helgaTalkMode == 0)
				random = 1;

			int centerFlag;
			Common::Point centerPos;
			if (local._roxOnStool) {
				centerFlag = 0;
				centerPos = Common::Point(230, 56);
			} else {
				centerFlag = 2;
				centerPos = Common::Point(0, 0);
			}

			switch (random) {
			case 1:
				kernel_message_add(quote_string(kernel.quotes, 0x211), centerPos.x, centerPos.y, 0x1110, 90, 0, 32 | centerFlag);
				kernel_timing_trigger(110, 25);
				break;

			case 2:
				kernel_message_add(quote_string(kernel.quotes, 0x212), centerPos.x, centerPos.y, 0x1110, 90, 0, 32 | centerFlag);
				kernel_timing_trigger(110, 25);
				break;

			case 3:
				kernel_message_add(quote_string(kernel.quotes, 0x213), centerPos.x, centerPos.y, 0x1110, 90, 0, 32 | centerFlag);
				kernel_timing_trigger(110, 25);
				break;

			default:
				break;
			}
		}
		break;

		case 25:
			switch (local._helgaTalkMode) {
			case 0:
				player.commands_allowed = false;
				local._helgaTalkMode = 1;
				kernel_message_add(quote_string(kernel.quotes, 0x209), 177, 33, 0xFBFA, 130, 0, 0);
				kernel_message_add(quote_string(kernel.quotes, 0x20A), 182, 47, 0xFBFA, 130, 0, 0);
				kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
				kernel_timing_trigger(130, 28);
				break;

			case 1:
				player.commands_allowed = false;
				local._helgaTalkMode = 2;
				kernel_message_add(quote_string(kernel.quotes, 0x20B), 157, 47, 0xFBFA, 100, 0, 0);
				kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
				kernel_timing_trigger(100, 28);
				break;

			case 2:
				player.commands_allowed = false;
				local._helgaTalkMode = 3;
				kernel_message_add(quote_string(kernel.quotes, 0x20C), 172, 47, 0xFBFA, 100, 0, 0);
				kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
				kernel_timing_trigger(100, 28);
				break;

			case 3:
				player.commands_allowed = true;
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
	} else if (player_said_2(talkto, woman_in_chair) && !local._firstTalkToGirlInChair) {
		kernel_message_add(quote_string(kernel.quotes, 0x1D7), 0, 0, 0x1110, 120, 0, 34);
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(150, 39);
		player.commands_allowed = false;
		local._firstTalkToGirlInChair = true;
	} else if (player_said_2(talkto, woman_in_chair) && local._firstTalkToGirlInChair) {
		kernel_message_add(quote_string(kernel.quotes, 0x1DB), 0, 0, 0x1110, 120, 0, 34);
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(150, 42);
		player.commands_allowed = false;
	} else if (player_said_2(talkto, woman_in_alcove) || player_said_2(walkto, woman_in_alcove)) {
		kernel_message_add(quote_string(kernel.quotes, 0x1DE), 102, 48, 0xFBFA, 120, 0, 0);
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(120, 44);
		player.commands_allowed = false;
	} else if (player_said_2(sit_on, bar_stool) && (player.target_x == 248)) {
		kernel_message_add(quote_string(kernel.quotes, 0x20D), 0, -14, 0x1110, 120, 0, 34);
		kernel_message_add(quote_string(kernel.quotes, 0x20E), 0, 0, 0x1110, 120, 0, 34);
	} else if (player_said_2(sit_on, bar_stool) && !local._roxOnStool && (player.target_x != 248)) {
		player.walker_visible = false;
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		g_sequence_ids[6] = kernel_seq_forward(g_sprite_ids[6], false, 7, 0, 0, 1);
		kernel_seq_range(g_sequence_ids[6], 1, 11);
		kernel_seq_depth(g_sequence_ids[6], 5);
		kernel_seq_trigger(g_sequence_ids[6], KERNEL_TRIGGER_EXPIRE, 0, 76);
		player.commands_allowed = false;
	} else if (player_said_2(take, credit_chip) && !player_has(OBJ_CREDIT_CHIP) && local._roxOnStool) {
		local._roxMode = 20;
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_timing_trigger(1, 92);
	} else if (player_said_2(take, credit_chip) && !player_has(OBJ_CREDIT_CHIP) && !local._roxOnStool) {
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		player.commands_allowed = false;
		player.walker_visible = false;
		g_sequence_ids[22] = kernel_seq_pingpong(g_sprite_ids[22], false, 7, 0, 0, 2);
		kernel_seq_range(g_sequence_ids[22], 1, 2);
		kernel_seq_loc(g_sequence_ids[22], player.x, player.y + 1);
		kernel_seq_depth(g_sequence_ids[22], 5);
		kernel_seq_scale(g_sequence_ids[22], 88);
		kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
		kernel_seq_trigger(g_sequence_ids[22], KERNEL_TRIGGER_EXPIRE, 0, 104);
	} else if (player_said_2(talkto, bartender)) {
		switch (kernel.trigger) {
		case 0:
		{
			int centerFlag;
			Common::Point centerPos;
			if (local._roxOnStool) {
				centerFlag = 0;
				centerPos = Common::Point(230, 56);
			} else {
				centerFlag = 2;
				centerPos = Common::Point(0, 0);
			}

			player.commands_allowed = false;
			int quoteId;
			if (local._bartenderCalled) {
				quoteId = 0x210;
			} else {
				quoteId = 0x20F;
				local._bartenderCalled = true;
			}
			kernel_message_purge();
			kernel_message_add(quote_string(kernel.quotes, quoteId), centerPos.x, centerPos.y, 0x1110, 90, 0, 32 | centerFlag);
			kernel_timing_trigger(110, 29);
			kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
			kernel_timing_trigger(90, 28);
		}
		break;

		case 29:
			kernel_message_purge();
			if (!local._roxOnStool) {
				if (object_is_here(OBJ_ALIEN_LIQUOR)) {
					kernel_message_add(quote_string(kernel.quotes, 0x1DF), 177, 41, 0xFDFC, 120, 0, 0);
					kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
					kernel_timing_trigger(1, 100);
					local._talkTimer = 120;
				} else if (local._rexMode == 0) {
					kernel_message_add(quote_string(kernel.quotes, 0x1F9), 175, 13, 0xFDFC, 180, 0, 0);
					kernel_message_add(quote_string(kernel.quotes, 0x1FA), 184, 27, 0xFDFC, 180, 0, 0);
					kernel_message_add(quote_string(kernel.quotes, 0x1FB), 200, 41, 0xFDFC, 180, 0, 0);
					kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
					kernel_timing_trigger(1, 100);
					local._talkTimer = 180;
					local._rexMode = 1;
				} else if (local._rexMode == 1) {
					kernel_message_add(quote_string(kernel.quotes, 0x1FC), 205, 41, 0xFDFC, 120, 0, 0);
					player.commands_allowed = true;
					kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
					kernel_timing_trigger(1, 100);
					local._talkTimer = 120;
					local._rexMode = 3;
				} else {
					player.commands_allowed = true;
				}
			} else {
				if (object_is_here(OBJ_ALIEN_LIQUOR)) {
					if (!local._refuseAlienLiquor) {
						kernel_message_purge();
						kernel_message_add(quote_string(kernel.quotes, 0x1E2), 198, 27, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
						kernel_message_add(quote_string(kernel.quotes, 0x1E3), 199, 41, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
						local._bartenderCurrentQuestion = 10;
						kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
						kernel_timing_trigger(1, 100);
						local._talkTimer = 120;
						local._conversationFl = true;
						local._bartenderDialogNode = 1;
						if (local._dialog2.read(0) <= 1)
							local._dialog1.write(0x214, false);

						local._dialog1.start();
					} else {
						kernel_message_add(quote_string(kernel.quotes, 0x1EF), 177, 41, 0xFDFC, INDEFINITE_TIMEOUT, 0, 0);
						kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
						kernel_timing_trigger(1, 100);
						local._talkTimer = 120;
						local._bartenderCurrentQuestion = 3;
						local._conversationFl = true;
						local._bartenderDialogNode = 1;
						if (local._dialog2.read(0) <= 1)
							local._dialog1.write(0x214, false);

						local._dialog1.start();
						player.commands_allowed = true;
					}
				} else {
					if (local._rexMode == 0) {
						kernel_message_add(quote_string(kernel.quotes, 0x1F9), 175, 13, 0xFDFC, 180, 0, 0);
						kernel_message_add(quote_string(kernel.quotes, 0x1FA), 184, 27, 0xFDFC, 180, 0, 0);
						kernel_message_add(quote_string(kernel.quotes, 0x1FB), 200, 41, 0xFDFC, 180, 0, 0);
						kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
						kernel_timing_trigger(1, 100);
						local._talkTimer = 180;
						local._rexMode = 1;
					} else if (local._rexMode == 1) {
						kernel_message_add(quote_string(kernel.quotes, 0x1FC), 205, 41, 0xFDFC, 120, 0, 0);
						player.commands_allowed = true;
						kernel.trigger_setup_mode = KERNEL_TRIGGER_DAEMON;
						kernel_timing_trigger(1, 100);
						local._talkTimer = 120;
						local._rexMode = 3;
					} else {
						player.commands_allowed = true;
					}
				}
			}
			break;

		default:
			break;
		}
	} else if (player_said_2(look, dance_floor))
		text_show(40210);
	else if (player_said_2(look, teleporter)) {
		if (global[kSomeoneHasExploded])
			text_show(40212);
		else
			text_show(40211);
	} else if (player_said_2(look, bar))
		text_show(40213);
	else if (player_said_2(look, bartender))
		text_show(40214);
	else if (player_said_2(look, woman_in_alcove))
		text_show(40215);
	else if (player_said_2(look, woman_on_balcony))
		text_show(40216);
	else if (player_said_2(look, woman_in_chair))
		text_show(40217);
	else if (player_said_2(look, repair_woman))
		text_show(40218);
	else if (player_said_2(look, corridor_to_south))
		text_show(40219);
	else if (player.look_around)
		text_show(40220);
	else if (player_said_2(look, women))
		text_show(40221);
	else if (player_said_2(push, repair_woman) || player_said_2(pull, repair_woman))
		text_show(40222);
	else if (player_said_2(talkto, women))
		text_show(40223);
	else if (player_said_2(talkto, woman_on_balcony))
		text_show(40224);
	else if (player_said_2(look, railing))
		text_show(40225);
	else if (player_said_2(look, table))
		text_show(40226);
	else if (player_said_2(take, table))
		text_show(40227);
	else if (player_said_2(look, sign))
		text_show(40228);
	else if (player_said_2(take, sign))
		text_show(40229);
	else if (player_said_2(look, bar_stool))
		text_show(40230);
	else if (player_said_2(take, bar_stool))
		text_show(40231);
	else if (player_said_2(look, cactus))
		text_show(40232);
	else if (player_said_2(take, cactus))
		text_show(40233);
	else if (player_said_2(look, disco_ball))
		text_show(40234);
	else if (player_said_2(look, upper_dance_floor))
		text_show(40235);
	else if (player_said_2(look, tree))
		text_show(40236);
	else if (player_said_2(look, plant))
		text_show(40237);
	else if (player_said_2(take, plant))
		text_show(40238);
	else if (player_said_2(look, repair_list) && object_is_here(OBJ_REPAIR_LIST))
		text_show(40239);
	else
		return;

	player.command_ready = false;
}

void room_402_synchronize(Common::Serializer &s) {
	s.syncAsByte(local._lightOn);
	s.syncAsByte(local._blowingSmoke);
	s.syncAsByte(local._leftWomanMoving);
	s.syncAsByte(local._rightWomanMoving);
	s.syncAsByte(local._firstTalkToGirlInChair);
	s.syncAsByte(local._waitingGinnyMove);
	s.syncAsByte(local._ginnyLooking);
	s.syncAsByte(local._bigBeatFl);
	s.syncAsByte(local._roxOnStool);
	s.syncAsByte(local._bartenderSteady);
	s.syncAsByte(local._bartenderHandsHips);
	s.syncAsByte(local._bartenderLooksLeft);
	s.syncAsByte(local._bartenderReady);
	s.syncAsByte(local._bartenderTalking);
	s.syncAsByte(local._bartenderCalled);
	s.syncAsByte(local._conversationFl);
	s.syncAsByte(local._activeTeleporter);
	s.syncAsByte(local._activeArrows);
	s.syncAsByte(local._activeArrow1);
	s.syncAsByte(local._activeArrow2);
	s.syncAsByte(local._activeArrow3);
	s.syncAsByte(local._cutSceneReady);
	s.syncAsByte(local._cutSceneNeeded);
	s.syncAsByte(local._helgaReady);
	s.syncAsByte(local._refuseAlienLiquor);

	s.syncAsSint16LE(local._drinkTimer);
	s.syncAsSint16LE(local._beatCounter);
	s.syncAsSint16LE(local._bartenderMode);
	s.syncAsSint16LE(local._bartenderDialogNode);
	s.syncAsSint16LE(local._bartenderCurrentQuestion);
	s.syncAsSint16LE(local._helgaTalkMode);
	s.syncAsSint16LE(local._roxMode);
	s.syncAsSint16LE(local._rexMode);
	s.syncAsSint16LE(local._talkTimer);
}

void room_402_preload() {
	room_init_code_pointer = room_402_init;
	room_pre_parser_code_pointer = room_402_pre_parser;
	room_parser_code_pointer = room_402_parser;
	room_daemon_code_pointer = room_402_daemon;

	section_4_walker();
	section_4_interface();

	vocab_make_active(words_bartender);
	vocab_make_active(words_alien_liquor);
	vocab_make_active(words_drink);
	vocab_make_active(words_binoculars);
	vocab_make_active(words_walkto);
	vocab_make_active(words_credit_chip);
	vocab_make_active(words_take);
	vocab_make_active(words_repair_list);
	vocab_make_active(words_look_at);
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
