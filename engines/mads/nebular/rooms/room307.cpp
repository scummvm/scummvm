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
#include "mads/core/matte.h"
#include "mads/core/pal.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section3.h"
#include "mads/nebular/rooms/forcefield.h"
#include "mads/nebular/rooms/thunks.h"
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
	const char *curQuote = quote_string(kernel.quotes, quote);
	if (_scene->_kernelMessages._talkFont->getWidth(curQuote, kernel_message_spacing) > 200) {
		static char subQuote1[34], subQuote2[34];
		quote_split_string(curQuote, subQuote1, subQuote2);
		Common::strcpy_s(local._subQuote2, subQuote2);

		_scene->_kernelMessages.add(Common::Point(0, -14), 0x1110, 34, 0, 240, subQuote1);
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 1, 180, local._subQuote2);
	} else
		_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 1, 120, curQuote);
}

static void handlePrisonerSpeech(int firstQuoteId, int number, uint32 timeout) {
	int height = number * 14;
	int posY;

	if (height < 60)
		posY = 65 - height;
	else
		posY = 78 - (height / 2);

	_scene->_kernelMessages.reset();
	local._activePrisonerFl = true;

	int quoteId = firstQuoteId;
	for (int count = 0; count < number; count++) {
		kernel.trigger_setup_mode = SEQUENCE_TRIGGER_DAEMON;
		_scene->_kernelMessages.add(Common::Point(5, posY), 0xFDFC, 0, 81, timeout, quote_string(kernel.quotes, quoteId));
		posY += 14;
		quoteId++;
	}
}

static void setDialogNode(int node) {
	switch (node) {
	case 0:
		handlePrisonerSpeech(0x153, 2, 120);
		_scene->_userInterface.setup(kInputBuildingSentences);
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
		_scene->_userInterface.setup(kInputBuildingSentences);
		break;

	case 5:
		global[kKnowsBuddyBeast] = true;
		handlePrisonerSpeech(0x117, 2, INDEFINITE_TIMEOUT);
		local._dialog2.start();
		break;

	case 6:
		handlePrisonerSpeech(0x123, 1, 120);
		_scene->_userInterface.setup(kInputBuildingSentences);
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
		_scene->_userInterface.setup(kInputBuildingSentences);
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
		_scene->_kernelMessages.reset();
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

	g_sequence_ids[4] = _scene->_sequences.startCycle(g_sprite_ids[4], false, 1);
	_scene->_sequences.setPosition(g_sequence_ids[4], Common::Point(127, 78));
	_scene->_sequences.setDepth(g_sequence_ids[4], 15);

	local._animationMode = 0;
	local._fieldCollisionCounter = 0;

	kernel_load_variant(1);

	kernel.quotes = quote_load(0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0x10C, 0x104, 0x106, 0x107, 0x108, 0x105,
		0x109, 0x10A, 0x10B, 0x10D, 0x10E, 0x10F, 0x110, 0x111, 0x112, 0x113, 0x114, 0x115, 0x116, 0x117,
		0x118, 0x119, 0x11A, 0x11B, 0x11C, 0x11D, 0x11E, 0x11F, 0x120, 0x121, 0x122, 0x123, 0x124, 0x125,
		0x126, 0x127, 0x128, 0x129, 0x12A, 0x12B, 0x12C, 0x12D, 0x12E, 0x12F, 0x130, 0x131, 0x132, 0x133,
		0x134, 0x135, 0x136, 0x137, 0x138, 0x139, 0x13A, 0x13B, 0x13C, 0x13D, 0x13E, 0x13F, 0x140, 0x141,
		0x142, 0x143, 0x144, 0x145, 0x146, 0x147, 0x148, 0x149, 0x14A, 0x14B, 0x14C, 0x14D, 0x14E, 0x14F,
		0x150, 0x151, 0x152, 0x153, 0);

	local._dialog1.setup(0x3F, 0x113, 0x114, 0x115, -1);
	local._dialog2.setup(0x40, 0x11A, 0x11B, 0x11C, 0x11D, 0x11E, 0x11F, 0x120, 0x121, 0x122, 0);

	if (!player.been_here_before)
		local._dialog2.set(0x11A, 0x122, 0);
	else if (previous_room == 318)
		local._dialog2.write(0x11E, true);


	if (previous_room == RETURNING_FROM_DIALOG) {
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
		_scene->_hotspots.activate(17, false);

		int idx = _scene->_dynamicHotspots.add(words_air_vent, words_climb_into, -1, Common::Rect(117, 67, 117 + 19, 67 + 13));
		int hotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(129, 104), FACING_NORTH);
		_scene->_dynamicHotspots.setCursor(hotspotId, CURSOR_GO_UP);

		_scene->_sequences.remove(g_sequence_ids[4]);
		g_sequence_ids[4] = _scene->_sequences.startCycle(g_sprite_ids[4], false, 2);
		_scene->_sequences.setPosition(g_sequence_ids[4], Common::Point(127, 78));
		_scene->_sequences.setDepth(g_sequence_ids[4], 15);
	}

	pal_change_color(252, 63, 30, 20);
	pal_change_color(253, 45, 15, 12);

	section_3_music();

	if ((previous_room == 318) || (previous_room == 387))
		_scene->_kernelMessages.addQuote(0xF3, 0, 120);
}

static void room_307_daemon() {
	handle_forcefield(&local._forcefield, &g_sprite_ids[0]);

	if ((local._animationMode == 1) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() == 126) {
			local._forcefield._flag = false;
			g_engine->_soundManager->command(5, 0);
		}

		if (_scene->_animation[0]->getCurrentFrame() == 194) {
			local._forcefield._flag = true;
			g_engine->_soundManager->command(24, 0);
		}
	}

	if ((local._animationMode == 2) && (_scene->_animation[0] != nullptr)) {
		if (_scene->_animation[0]->getCurrentFrame() == 54)
			local._forcefield._flag = false;

		if (_scene->_animation[0]->getCurrentFrame() == 150) {
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

		if ((local._guardTime > 3000) && !local._duringPeeingFl && (_scene->_animation[0] == nullptr)
			&& (inter_input_mode != kInputConversation) && global[kMetBuddyBeast] && !local._activePrisonerFl) {
			if (!player_has(OBJ_SCALPEL) && !local._grateOpenedFl) {
				player.commands_allowed = false;
				player_walk(151, 119, FACING_SOUTHEAST);
				local._animationMode = 2;
				g_engine->_soundManager->command(11, 0);
				kernel_run_animation(kernel_name('b', -1), 70);
			}
			local._guardTime = 0;
		} else if ((local._prisonerTimer > 300) && (inter_input_mode != kInputConversation) && (_scene->_animation[0] == nullptr) && !local._activePrisonerFl) {
			if (!global[kMetBuddyBeast]) {
				if (local._prisonerMessageId == -1)
					local._prisonerMessageId = 0x104;

				int idx = _scene->_kernelMessages.add(Common::Point(5, 51), 0xFDFC, 0, 81, 120, quote_string(kernel.quotes, local._prisonerMessageId));
				_scene->_kernelMessages.setQuoted(idx, 4, true);
				local._prisonerMessageId++;
				if (local._prisonerMessageId > 0x10A)
					local._prisonerMessageId = 0x104;
			} else if (global[kKnowsBuddyBeast] && (local._dialog2.read(0) > 1) && (g_engine->getRandomNumber(1, 3) == 1)) {
				int idx = _scene->_kernelMessages.add(Common::Point(5, 51), 0xFDFC, 0, 81, 120, quote_string(kernel.quotes, 267));
				_scene->_kernelMessages.setQuoted(idx, 4, true);
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
	else if (inter_input_mode == kInputConversation)
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
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, quote_string(kernel.quotes, 239));
			_scene->_sequences.addTimer(120, 1);
			break;

		case 1:
			g_sprite_ids[5] = kernel_load_series("*RXCL_8", 0);
			player.walker_visible = false;
			g_sequence_ids[5] = _scene->_sequences.addSpriteCycle(g_sprite_ids[5], false, 12, 1, 0, 0);
			_scene->_sequences.setAnimRange(g_sequence_ids[5], -1, 3);
			_scene->_sequences.setMsgLayout(g_sequence_ids[5]);
			_scene->_sequences.addSubEntry(g_sequence_ids[5], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2:
		{
			int oldIdx = g_sequence_ids[5];
			g_sequence_ids[5] = _scene->_sequences.startPingPongCycle(g_sprite_ids[5], false, 12, 6, 0, 0);
			_scene->_sequences.setMsgLayout(g_sequence_ids[5]);
			_scene->_sequences.setAnimRange(g_sequence_ids[5], 2, 3);
			_scene->_sequences.updateTimeout(g_sequence_ids[5], oldIdx);
			_scene->_sequences.addSubEntry(g_sequence_ids[5], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
		}
		break;

		case 3:
		{
			int oldIdx = g_sequence_ids[5];
			g_sequence_ids[5] = _scene->_sequences.startCycle(g_sprite_ids[5], false, 1);
			_scene->_sequences.setMsgLayout(g_sequence_ids[5]);
			_scene->_sequences.updateTimeout(g_sequence_ids[5], oldIdx);
			_scene->_sequences.addTimer(48, 4);
		}
		break;

		case 4:
			g_engine->_soundManager->command(26, 0);
			_scene->_sequences.remove(g_sequence_ids[4]);
			g_sequence_ids[4] = _scene->_sequences.startCycle(g_sprite_ids[4], false, 2);
			_scene->_sequences.setPosition(g_sequence_ids[4], Common::Point(127, 78));
			_scene->_sequences.setDepth(g_sequence_ids[4], 15);
			_scene->_sequences.addTimer(90, 5);
			break;

		case 5:
			g_engine->_soundManager->command(10, 0);
			_scene->_kernelMessages.reset();
			_scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 0, 120, quote_string(kernel.quotes, 241));
			_scene->_sequences.addTimer(120, 6);
			break;

		case 6:
		{
			player.walker_visible = true;
			player.clock = kernel.clock - player.frame_delay;
			_scene->_sequences.remove(g_sequence_ids[5]);
			local._grateOpenedFl = true;
			_scene->_hotspots.activate(17, false);
			int idx = _scene->_dynamicHotspots.add(words_air_vent, words_climb_into, -1, Common::Rect(117, 67, 117 + 19, 67 + 13));
			int hotspotId = _scene->_dynamicHotspots.setPosition(idx, Common::Point(129, 104), FACING_NORTH);
			_scene->_dynamicHotspots.setCursor(hotspotId, CURSOR_GO_UP);
			inter_take_from_player(OBJ_SCALPEL, NOWHERE);
			_scene->_kernelMessages.addQuote(0xF2, 7, 120);
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
				_scene->_sequences.remove(g_sequence_ids[4]);
				g_sequence_ids[4] = _scene->_sequences.addSpriteCycle(g_sprite_ids[4], false, 60, 1, 0, 0);
				_scene->_sequences.setAnimRange(g_sequence_ids[4], 3, -2);
				_scene->_sequences.setPosition(g_sequence_ids[4], Common::Point(127, 78));
				_scene->_sequences.setDepth(g_sequence_ids[4], 15);
				g_sequence_ids[5] = _scene->_sequences.addSpriteCycle(g_sprite_ids[5], false, 18, 1, 0, 0);
				_scene->_sequences.setAnimRange(g_sequence_ids[5], -1, 4);
				_scene->_sequences.setMsgLayout(g_sequence_ids[5]);
				_scene->_sequences.addSubEntry(g_sequence_ids[4], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				_scene->_sequences.addSubEntry(g_sequence_ids[5], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				g_sequence_ids[4] = _scene->_sequences.startCycle(g_sprite_ids[4], false, -2);
				_scene->_sequences.setPosition(g_sequence_ids[4], Common::Point(127, 78));
				_scene->_sequences.setDepth(g_sequence_ids[4], 15);
				break;

			case 2:
			{
				int oldIdx = g_sequence_ids[5];
				g_sequence_ids[5] = _scene->_sequences.addSpriteCycle(g_sprite_ids[5], false, 12, 1, 0, 0);
				_scene->_sequences.setAnimRange(g_sequence_ids[5], 4, 10);
				_scene->_sequences.setMsgLayout(g_sequence_ids[5]);
				_scene->_sequences.updateTimeout(g_sequence_ids[5], oldIdx);
				_scene->_sequences.addSubEntry(g_sequence_ids[5], SEQUENCE_TRIGGER_EXPIRE, 0, 3);
			}
			break;

			case 3:
				_scene->_sequences.remove(g_sequence_ids[4]);
				g_sequence_ids[4] = _scene->_sequences.startCycle(g_sprite_ids[4], false, 3);
				_scene->_sequences.setPosition(g_sequence_ids[4], Common::Point(127, 78));
				_scene->_sequences.setDepth(g_sequence_ids[4], 1);
				g_sequence_ids[5] = _scene->_sequences.startCycle(g_sprite_ids[5], false, 11);
				_scene->_sequences.setMsgLayout(g_sequence_ids[5]);
				_scene->_sequences.setPosition(g_sequence_ids[5], Common::Point(129, 102));
				_scene->_sequences.addTimer(48, 4);
				break;

			case 4:
				_scene->_sequences.remove(g_sequence_ids[4]);
				g_sequence_ids[4] = _scene->_sequences.startCycle(g_sprite_ids[4], false, 2);
				_scene->_sequences.setPosition(g_sequence_ids[4], Common::Point(127, 78));
				_scene->_sequences.setDepth(g_sequence_ids[4], 1);
				_scene->_sequences.remove(g_sequence_ids[5]);
				g_sequence_ids[5] = _scene->_sequences.addSpriteCycle(g_sprite_ids[5], false, 12, 1, 0, 0);
				_scene->_sequences.setAnimRange(g_sequence_ids[5], 12, 14);
				_scene->_sequences.setMsgLayout(g_sequence_ids[5]);
				_scene->_sequences.setPosition(g_sequence_ids[5], Common::Point(129, 102));
				_scene->_sequences.addSubEntry(g_sequence_ids[5], SEQUENCE_TRIGGER_EXPIRE, 0, 5);
				break;

			case 5:
				g_sequence_ids[5] = _scene->_sequences.startCycle(g_sprite_ids[5], false, 15);
				_scene->_sequences.setMsgLayout(g_sequence_ids[5]);
				_scene->_sequences.setPosition(g_sequence_ids[5], Common::Point(129, 102));
				_scene->_sequences.addTimer(48, 6);
				break;

			case 6:
				_scene->_sequences.remove(g_sequence_ids[5]);
				_scene->_sequences.remove(g_sequence_ids[4]);
				g_sequence_ids[4] = _scene->_sequences.startCycle(g_sprite_ids[4], false, 1);
				_scene->_sequences.setPosition(g_sequence_ids[4], Common::Point(127, 78));
				_scene->_sequences.setDepth(g_sequence_ids[4], 1);
				_scene->_sequences.addTimer(48, 7);
				break;

			case 7:
				new_room = 313;
				break;

			default:
				break;
			}
		}
	} else if (player_said_2(use, toilet) && (config_file.naughtiness != STORYMODE_NAUGHTY))
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
				g_sequence_ids[3] = _scene->_sequences.addSpriteCycle(g_sprite_ids[3], false, 9, 1, 0, 0);
				_scene->_sequences.setAnimRange(g_sequence_ids[3], -1, 2);
				_scene->_sequences.setDepth(g_sequence_ids[3], 9);
				_scene->_sequences.addSubEntry(g_sequence_ids[3], SEQUENCE_TRIGGER_EXPIRE, 0, 1);
				break;

			case 1:
				g_sequence_ids[3] = _scene->_sequences.addSpriteCycle(g_sprite_ids[3], false, 9, 5, 0, 0);
				_scene->_sequences.setAnimRange(g_sequence_ids[3], 3, -2);
				_scene->_sequences.setDepth(g_sequence_ids[3], 9);
				_scene->_sequences.addSubEntry(g_sequence_ids[3], SEQUENCE_TRIGGER_EXPIRE, 0, 2);
				break;

			case 2:
				_scene->_sequences.updateTimeout(-1, g_sequence_ids[3]);
				player.walker_visible = true;
				_scene->_sequences.addTimer(48, 3);
				break;

			case 3:
			{
				matte_deallocate_series(g_sprite_ids[3], true);
				_scene->_kernelMessages.reset();
				int idx = _scene->_kernelMessages.add(Common::Point(0, 0), 0x1110, 34, 4, 120, quote_string(kernel.quotes, 237));
				_scene->_kernelMessages.setQuoted(idx, 4, true);
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
			_scene->_kernelMessages.reset();
			int idx = _scene->_kernelMessages.add(Common::Point(85, 39), 0x1110, 0, 0, 180, quote_string(kernel.quotes, 238));
			_scene->_kernelMessages.setQuoted(idx, 4, true);
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
