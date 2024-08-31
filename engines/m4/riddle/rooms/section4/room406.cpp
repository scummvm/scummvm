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
 * along with this program.  If not, see <http://www.gnu.org/licenses/ },.
 *
 */

#include "m4/riddle/rooms/section4/room406.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

static const char *const SAID[][2] = {
	{ "DART BOARD",            "406r03" },
	{ "MESSAGES",              "406r14" },
	{ "PAINTING",              "456r01" },
	{ "OVERHANGING LAMP",      "406r04" },
	{ "GAMES CABINET",         "406r06" },
	{ "GAMES CABINET DRAWER",  "406r47" },
	{ "BROKEN MIRROR",         "406r43" },
	{ "WRITING DESK",          "406r09a" },
	{ "WRITING DESK DRAWER",   "406r21" },
	{ "DESK DRAWER OPEN",      "406r09a" },
	{ "WRITING DESK ",         "406r09" },
	{ "LAMP",                  "406r11" },
	{ "TABLE",                 "406r11" },
	{ "CHAIR",                 "406r12" },
	{ "PAINTING ",             "406r13" },
	{ nullptr, nullptr }
};

void Room406::preload() {
	_G(player).walker_type = 1;
	_G(player).shadow_type = 1;
	LoadWSAssets("OTHER SCRIPT");
}

void Room406::init() {
	player_set_commands_allowed(false);

	switch (_G(flags)[V312]) {
	case 0:
		hotspot_set_active("STAIRS", false);
		hotspot_set_active("BILLIARD TABLE ", false);
		break;

	case 1:
		_billiardTable = series_place_sprite("406 BILLIARD TABLE UNLATCHED",
			0, 0, 0, 100, 0x200);
		hotspot_set_active("STAIRS", false);
		break;

	case 2:
		_billiardTable = series_place_sprite("406 BILLIARD TABLE UP",
			0, 0, 0, 100, 0x200);
		hotspot_set_active("BILLIARD TABLE", false);
		hotspot_set_active("BILLIARD TABLE ", false);
		break;

	default:
		break;
	}

	if (_G(flags)[kPaintingOpen]) {
		_painting = series_place_sprite("406 PAINTING OPEN", 0, 0, 0, 100, 0xf00);
		hotspot_set_active("PAINTING", false);
		hotspot_set_active("SMOKING HUTCH", true);
	} else {
		_painting = series_place_sprite("406 PAINTING CLOSED", 0, 0, 0, 100, 0xf00);
		hotspot_set_active("PAINTING", true);
		hotspot_set_active("SMOKING HUTCH", false);
	}

	if (_G(flags)[V316]) {
		hotspot_set_active("MIRROR", false);
		_mirror = series_place_sprite("CRACKED MIRROR IN BILLIARD RM",
			0, 0, 0, 100, 0xf00);
	} else {
		hotspot_set_active("BROKEN MIRROR", false);
	}

	if (_G(flags)[V310]) {
		_cardDoor = series_place_sprite("406 CARD DOOR OPEN BY PICT",
			0, 0, 0, 100, 0xf00);
		hotspot_set_active("ACE OF SPADES", false);
		hotspot_set_active("ACE OF SPADES ", true);
		hotspot_set_active("SWITCH", true);
	} else {
		hotspot_set_active("ACE OF SPADES", true);
		hotspot_set_active("ACE OF SPADES ", false);
		hotspot_set_active("SWITCH", false);
	}

	if (_G(flags)[V306]) {
		hotspot_set_active("GAMES CABINET", false);
		_gamesCabinet = series_place_sprite("406 GAMES CABINET OPEN",
			0, 0, 0, 100, 0xf00);
	} else {
		hotspot_set_active("GAMES CABINET ", false);
		hotspot_set_active("GAMES CABINET DRAWER", false);
		_gamesCabinet = series_place_sprite("406 GAMES CABINET CLOSED",
			0, 0, 0, 100, 0xf00);
	}

	if (_G(flags)[kWritingDeskDrawerOpen]) {
		hotspot_set_active("WRITING DESK", false);
		_desk = series_place_sprite("406 DESK OPEN", 0, 0, 0, 100, 0x600);
	} else {
		hotspot_set_active("WRITING DESK ", false);
		hotspot_set_active("WRITING DESK DRAWER", false);
		_desk = series_place_sprite("406 DESK CLOSE", 0, 0, 0, 100, 0x600);
	}

	if (_G(flags)[V321]) {
		_poolBall = series_place_sprite("406 POOL BALL IN CABINET",
			0, 0, 0, 100, 0xf00);
		hotspot_set_active("BILLIARD BALL", false);
	} else if (!inv_player_has("BILLIARD BALL")) {
		_poolBall = series_place_sprite("BILLIARD BALL", 0, 0, 0, 100, 0x200);
	} else {
		hotspot_set_active("BILLIARD BALL", false);
	}

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_val1 = 1001;
		_val2 = 1001;
		hotspot_set_active("CABINET DRAWER OPEN", false);
		hotspot_set_active("DESK DRAWER OPEN", false);
		hotspot_set_active("MESSAGES", false);
		hotspot_set_active("ENVELOPE", false);
		hotspot_set_active("KEYS", false);
	} else if (_val2 == 1000) {
		ws_demand_facing(1);
		_rptmhr = series_load("RPTMHR11");
		setGlobals1(_rptmhr, 1, 5, 5, 5, 0, 5, 1, 1, 1);
		sendWSMessage_110000(-1);
		_emptyDrawer = series_place_sprite("406 DESK DRAWER EMPTY",
			0, 0, 0, 100, 0x200);
		disableHotspots();

		hotspot_set_active(" ", true);
		hotspot_set_active("DESK DRAWER OPEN", true);
		hotspot_set_active("MESSAGES", true);

		if (!inv_player_has("ENVELOPE")) {
			_envelope = series_place_sprite("406 DESK DRAWER ENVELOPE",
				0, 0, 0, 100, 0x200);
			hotspot_set_active("ENVELOPE", true);
		}

		if (!inv_player_has("KEYS")) {
			_keys = series_place_sprite("406 DESK DRAWER WITH KEYS",
				0, 0, 0, 100, 0x200);
			hotspot_set_active("KEYS", true);
		}
	} else if (_val1 == 1000) {
		ws_demand_facing(11);
		_ripReachHand = series_load("RIP TREK MED REACH HAND POS1");
		setGlobals1(_ripReachHand, 1, 10, 10, 10, 0, 10, 1, 1, 1);
		sendWSMessage_110000(-1);
		_cards = series_place_sprite("406 GAMES DRAWER WITH CARDS",
			0, 0, 0, 100, 0x100);
		disableHotspots();

		hotspot_set_active(" ", true);
		hotspot_set_active("CABINET DRAWER OPEN", true);
	} else {
		disableHotspots();
		setHotspots();
	}

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		player_set_commands_allowed(true);

		if (_G(flags)[V322]) {
			digi_preload("456_s03a");
			digi_play_loop("456_s03a", 3, 255, -1, 456);
		}
		break;

	case 456:
		if (_G(flags)[V322])
			digi_play_loop("456_s03a", 3, 255, -1, 456);

		ws_demand_location(400, 340, 1);
		_ripHiHand = series_load("rip trek hi 1 hand");
		setGlobals1(_ripHiHand, 1, 5, 5, 5, 0, 5, 1, 1, 1);
		sendWSMessage_110000(310);
		break;

	default:
		if (_G(flags)[V322]) {
			digi_preload("456_s03a");
			digi_play_loop("456_s03a", 3, 255, -1, 456);
		}

		ws_demand_location(603, 327, 9);
		ws_walk(530, 332, nullptr, 300, 9);
		break;
	}
}

void Room406::daemon() {
	switch (_G(kernel).trigger) {
	case 10:
		player_set_commands_allowed(false);
		digi_play("406_s03", 2);
		terminateMachineAndNull(_cards);
		sendWSMessage_120000(12);
		break;

	case 12:
		setHotspots();
		sendWSMessage_150000(13);
		break;

	case 13:
		series_unload(_ripReachHand);
		player_set_commands_allowed(true);
		break;

	case 20:
		player_set_commands_allowed(false);
		digi_play("406_s03", 2);
		terminateMachineAndNull(_emptyDrawer);

		if (inv_object_is_here("ENVELOPE"))
			terminateMachineAndNull(_envelope);
		if (inv_object_is_here("KEYS"))
			terminateMachineAndNull(_keys);

		sendWSMessage_120000(22);
		return;

	case 22:
		sendWSMessage_150000(23);
		return;

	case 23:
		series_unload(_rptmhr);
		player_set_commands_allowed(true);
		break;

	case 300:
		player_set_commands_allowed(true);
		break;

	case 310:
		hotspot_set_active("PAINTING", true);
		hotspot_set_active("SMOKING HUTCH", false);
		digi_play("406_s07", 2, 255, 311);
		_paintingOpening = series_load("406 PAINTING OPENING");
		_painting = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, 0,
			triggerMachineByHashCallbackAlways, "HUTCH painting opens");
		sendWSMessage_10000(1, _painting, _paintingOpening, 30, 1, 312,
			_paintingOpening, 1, 1, 0);
		break;

	case 311:
		digi_play("406_s08", 2);
		break;

	case 312:
		terminateMachineAndNull(_painting);
		series_unload(_paintingOpening);
		_painting = series_place_sprite("406 PAINTING CLOSED", 0, 0, 0, 100, 0xf00);
		sendWSMessage_120000(313);
		break;

	case 313:
		sendWSMessage_150000(314);
		break;

	case 314:
		series_unload(_ripHiHand);
		_G(flags)[kPaintingOpen] = 0;
		player_set_commands_allowed(true);
		break;

	case 700:
		if (_G(flags)[V322])
			digi_play("456_s03", 3, 255, 700);
		break;

	default:
		break;
	}
}

void Room406::pre_parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");
	bool useFlag = player_said_any("push", "pull", "gear", "open", "close");

	if (useFlag && player_said_any("BILLIARD TABLE", "BILLIARD TABLE ")) {
		_G(player).resetWalk();
		_G(kernel).trigger_mode = KT_PARSE;
		kernel_timing_trigger(1, 69);
		_G(kernel).trigger_mode = KT_PREPARSE;
	}

	if (player_said("BILLIARD BALL", "BILLIARD TABLE") &&
			_G(kernel).trigger == -1) {
		_G(player).resetWalk();
		_G(kernel).trigger_mode = KT_PARSE;
		kernel_timing_trigger(1, 69);
		_G(kernel).trigger_mode = KT_PREPARSE;
	}

	if (_val1 == 1000) {
		_G(player).resetWalk();

		if (!player_said(" ") &&
			!(lookFlag && player_said("MESSAGE LOG")) &&
			!player_said("journal")) {
			if (!useFlag || !player_said("CABINET DRAWER OPEN"))
				return;
		}

		intr_cancel_sentence();
		_val1 = 1001;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 10);
		_G(kernel).trigger_mode = KT_PARSE;

	} else if (_val2 == 1000) {
		_G(player).resetWalk();

		if (!player_said(" ") &&
			!(lookFlag && player_said("MESSAGE LOG")) &&
			!player_said("journal")) {
			if (!useFlag || !player_said("DESK DRAWER OPEN"))
				return;
		}

		intr_cancel_sentence();
		_val1 = 1001;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(1, 20);
		_G(kernel).trigger_mode = KT_PARSE;

	} else if (player_said("journal") && !takeFlag && !lookFlag &&
			_G(kernel).trigger == -1) {
		_G(player).resetWalk();
	}
}

#define LOOK(ITEM) (lookFlag && player_said(ITEM) && inv_object_is_here(ITEM))
void Room406::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");
	bool useFlag = player_said_any("push", "pull", "gear", "open", "close");

	if (takeFlag && player_said("ENVELOPE")) {
		switch (_G(kernel).trigger) {
		case -1:
			if (inv_object_is_here("ENVELOPE")) {
				inv_give_to_player("ENVELOPE");
				hotspot_set_active("ENVELOPE", false);
				terminateMachineAndNull(_envelope);
				kernel_examine_inventory_object("PING ENVELOPE", _G(master_palette),
					5, 1, 398, 220, 8, 0, -1);
			}
			break;
		case 8:
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (takeFlag && player_said("KEYS") && takeKeys()) {
		// No implementation
	} else if (takeFlag && player_said("DESK DRAWER OPEN")) {
		digi_play("406r17", 1);
	} else if (takeFlag && player_said("CABINET DRAWER OPEN")) {
		digi_play("406r24", 1);
	} else if (takeFlag && player_said("BILLIARD BALL") && takeBilliardBall()) {
		// No implementation
	} else if (player_said("BILLIARD BALL", "BILLIARD TABLE") && billiardBallOnTable()) {
		// No implementation
	} else if (lookFlag && player_said_any("BILLIARD TABLE", "BILLIARD TABLE ")) {
		if (!_G(flags)[V030])
			digi_play("406r41", 1);
		else if (!_G(flags)[V321])
			digi_play("406r02", 1);
		else
			digi_play("406r28", 1);
	} else if (lookFlag && player_said("CUE CABINET")) {
		if (_G(flags)[V030])
			digi_play("406r42", 1);
		else if (_G(flags)[V321])
			digi_play("406r29", 1);
		else
			digi_play("406r05", 1);
	} else if (lookFlag && player_said("BALL RACK")) {
		if (_G(flags)[V030])
			digi_play("406r42", 1);
		else if (_G(flags)[V321])
			digi_play("406r29", 1);
		else
			digi_play("406r49", 1);
	} else if (lookFlag && player_said("GAMES CABINET ")) {
		switch (_G(kernel).trigger) {
		case -1:
			digi_play("406r25", 1);
			if (!_G(flags)[V317]) {
				player_set_commands_allowed(false);
				_G(flags)[V317] = 1;
			}
			break;
		case 2:
			digi_play("406r25a", 1, 255, 3);
			break;
		case 3:
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (lookFlag && player_said("CABINET DRAWER OPEN")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			if (_G(flags)[V311])
				digi_play("406r23", 1, 255, 2);
			else
				digi_play("406r23", 1, 255, 3);
			break;
		case 2:
			digi_play("406r23a", 1, 255, 3);
			break;
		case 3:
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (lookFlag && player_said("MIRROR")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			digi_preload("406_s13");
			_G(flags)[V316] = 1;
			_lookMirror = series_load("406 RIP LOOKS MIRROR");
			ws_hide_walker();
			_ripAction = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x700, 0,
				triggerMachineByHashCallbackAlways, "rip looks at mirror");
			sendWSMessage_10000(1, _ripAction, _lookMirror, 1, 17, 2,
				_lookMirror, 17, 17, 0);
			break;

		case 1:
			kernel_timing_trigger(10, 2);
			break;

		case 2:
			_mirror = series_place_sprite("CRACKED MIRROR IN BILLIARD RM",
				0, 0, 0, 100, 0xf00);
			digi_play("406_s13", 2, 255, 3);
			break;

		case 3:
			digi_play("406r07", 1);
			kernel_timing_trigger(40, 4);
			break;

		case 4:
			sendWSMessage_10000(1, _ripAction, _lookMirror, 17, 1, 5,
				_lookMirror, 1, 1, 0);
			break;

		case 5:
			terminateMachineAndNull(_ripAction);
			series_unload(_lookMirror);
			ws_unhide_walker();
			digi_play("406r07A", 1, 255, 6);
			break;

		case 6:
			hotspot_set_active("BROKEN MIRROR", true);
			hotspot_set_active("MIRROR", false);
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
	} else if (lookFlag && player_said("ACE OF SPADES")) {
		_G(flags)[V311] = 1;
		digi_play(_G(flags)[V030] ? "406r44" : "406r08", 1);
	} else if (lookFlag && player_said_any("SWITCH", "ACE OF SPADES ")) {
		digi_play(player_been_here(456) ? "406r44" : "456r03", 1);
	} else if (LOOK("BILLIARD BALL")) {
		digi_play("406r10", 1);
	} else if (lookFlag && _G(walker).ripley_said(SAID)) {
		// No implementation
	} else if (LOOK("ENVELOPE")) {
		digi_play("406r15", 1);
	} else if (LOOK("KEYS")) {
		digi_play("406r16", 1);
	} else if (lookFlag && player_said("MESSAGE")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			digi_play("406r18", 1, 255, 2);
			break;
		case 2:
			digi_play("406r18a", 1, 255, 3);
			break;
		case 3:
			digi_play("406r18b", 1, 255, 4);
			break;
		case 4:
			digi_play("406r18c", 1, 255, 5);
			break;
		case 5:
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (lookFlag && player_said("STAMP")) {
		digi_play("406r19", 1);
	} else if (lookFlag && player_said(" ")) {
		digi_play(_G(flags)[V030] ? "406r40" : "406r01", 1);
	} else if (useFlag && player_said("DART BOARD")) {
		digi_play("406r20", 1);
	} else if (useFlag && player_said_any("BILLIARD TABLE",
			"BILLIARD TABLE ") && _G(flags)[V312] == 1) {
		switch (_G(kernel).trigger) {
		case 1:
			_rptmr15 = series_load("RPTMR15");
			setGlobals1(_rptmr15, 1, 12, 12, 12, 0, 12, 1, 1, 1);
			sendWSMessage_110000(2);
			break;
		case 2:
			sendWSMessage_120000(4);
			_tableRaises = series_load("808 BILLIARDS TABLE RAISES");
			digi_play("406_s11", 2);
			terminateMachineAndNull(_billiardTable);
			_billiardTable = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x200, 0,
				triggerMachineByHashCallbackAlways, "Table raises");
			sendWSMessage_10000(1, _billiardTable, _tableRaises, 1, 24, 6,
				_tableRaises, 24, 24, 0);
			break;
		case 4:
			sendWSMessage_150000(5);
			break;
		case 5:
			series_unload(_rptmr15);
			break;
		case 6:
			terminateMachineAndNull(_billiardTable);
			series_unload(_tableRaises);
			hotspot_set_active("BILLIARD TABLE", false);
			hotspot_set_active("BILLIARD TABLE ", false);
			hotspot_set_active("STAIRS", true);
			_billiardTable = series_place_sprite("406 BILLIARD TABLE UP", 0, 0, 0, 100, 0x200);
			_G(flags)[V312] = 2;
			player_set_commands_allowed(true);
			break;
		case 69:
			player_set_commands_allowed(false);
			ws_walk(205, 333, nullptr, 1, 5);
			break;
		default:
			break;
		}
	} else if (useFlag && player_said("SWITCH")) {
		if (_G(flags)[kPaintingOpen])
			useSwitchPaintingOpen();
		else
			useSwitchPaintingClosed();
	} else if (useFlag && player_said_any("ACE OF SPADES", "ACE OF SPADES ")) {
		// Note: The original had two separate blocks for use ace of spades.
		// Since the second version could never be called, I've ommitted it
		if (_G(flags)[V310])
			useAceOfSpades1();
		else
			useAceOfSpades2();
	} else if (player_said("CUBE CABINET", "KEYS")) {
		digi_play("406r27", 1);
	} else if (player_said("DART BOARD", "KEYS")) {
		digi_play("406r26", 1);
	} else if (useFlag && player_said("CUE CABINET")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_ripReachHand = series_load("RIP TREK MED REACH HAND POS1");
			break;
		case 1:
			digi_play("406_s04", 2, 255, 2);
			break;
		case 2:
			digi_play(inv_player_has("KEYS") ? "406r27" : "406r20", 1);
			break;
		case 4:
			sendWSMessage_150000(5);
			break;
		case 5:
			series_unload(_ripReachHand);
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (useFlag && player_said("WRITING DESK DRAWER")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_rptmhr = series_load("RPTMHR11");
			setGlobals1(_rptmhr, 1, 5, 5, 5, 0, 5, 1, 1, 1);
			sendWSMessage_110000(1);
			break;
		case 1:
			_val2 = 1000;
			_emptyDrawer = series_place_sprite("406 DESK DRAWER EMPTY", 0, 0, 0, 100, 0x200);
			digi_play("406_s02", 2);
			hotspot_set_active(" ", true);
			hotspot_set_active("DESK DRAWER OPEN", true);
			hotspot_set_active("MESSAGES", true);

			if (inv_object_is_here("ENVELOPE")) {
				_envelope = series_place_sprite("406 DESK DRAWER ENVELOPE", 0, 0, 0, 100, 0x200);
				hotspot_set_active("ENVELOPE", true);
			}

			if (!inv_player_has("KEYS")) {
				_keys = series_place_sprite("406 DESK DRAWER WITH KEYS", 0, 0, 0, 100, 0x200);
				hotspot_set_active("KEYS", true);
			}

			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (useFlag && player_said("GAMES CABINET DRAWER")) {
		if (_G(flags)[V307]) {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				_ripReachHand = series_load("RIP TREK MED REACH HAND POS1");
				setGlobals1(_ripReachHand, 1, 10, 10, 10, 0, 10, 1, 1, 1);
				sendWSMessage_110000(1);
				break;
			case 1:
				if (_G(flags)[V308]) {
					_val1 = 1000;
					_cards = series_place_sprite("406 GAMES DRAWER WITH CARDS", 0, 0, 0, 100, 0x100);
					digi_play("406_s02", 2);
					hotspot_set_active(" ", true);
					hotspot_set_active("CABINET DRAW OPEN", true);
					player_set_commands_allowed(true);
				} else {
					digi_play("406r20", 1);
					sendWSMessage_120000(4);
				}
				break;
			case 4:
				sendWSMessage_150000(5);
				break;
			case 5:
				series_unload(_ripReachHand);
				player_set_commands_allowed(true);
				break;
			default:
				break;
			}
		} else {
			digi_play("406r20", 1);
		}
	} else if (player_said("GAMES CABINET DRAWER", "KEYS")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_ripReachHand = series_load("RIP TREK MED REACH HAND POS1");
			setGlobals1(_ripReachHand, 1, 10, 10, 10, 0, 10, 1, 1, 1);
			sendWSMessage_110000(1);
			break;
		case 1:
			if (_G(flags)[V308]) {
				_G(flags)[V308] = 0;
				digi_play("406_s06", 2);
			} else {
				digi_play("406_s05", 2);
				_G(flags)[V308] = 1;
			}
			break;
		case 4:
			sendWSMessage_150000(5);
			break;
		case 5:
			series_unload(_ripReachHand);
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (player_said("GAMES CABINET", "KEYS")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_ripReachHand = series_load("RIP TREK MED REACH HAND POS1");
			setGlobals1(_ripReachHand, 1, 10, 10, 10, 0, 10, 1, 1, 1);
			sendWSMessage_110000(1);
			break;
		case 1:
			if (_G(flags)[V307]) {
				_G(flags)[V307] = 0;
				digi_play("406_s06", 2);
			} else {
				digi_play("406_s05", 2);
				_G(flags)[V307] = 1;
			}
			break;
		case 4:
			sendWSMessage_150000(5);
			break;
		case 5:
			series_unload(_ripReachHand);
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (useFlag && player_said("BILLIARD BALL") && lookBilliardBall()) {
		// No implementation
	} else if (useFlag && player_said("WRITING DESK")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_ripReachHand = series_load("RIP TREK MED REACH HAND POS1");
			setGlobals1(_ripReachHand, 1, 10, 10, 10, 0, 10, 1, 1, 1);
			sendWSMessage_110000(1);
			break;
		case 1:
			terminateMachineAndNull(_desk);
			_deskOpening = series_load("406 DESK OPENING");
			digi_play("406_s05", 2, 255, -1, 406);
			_desk = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x500, 0,
				triggerMachineByHashCallbackAlways, "writing desk opens");
			sendWSMessage_10000(1, _desk, _deskOpening, 1, 15, 2,
				_deskOpening, 15, 15, 0);
			break;
		case 2:
			terminateMachineAndNull(_desk);
			_desk = series_place_sprite("406 DESK OPEN", 0, 0, 0, 100, 0x600);
			hotspot_set_active("WRITING DESK", false);
			hotspot_set_active("WRITING DESK ", true);
			hotspot_set_active("WRITING DESK DRAWER", true);
			sendWSMessage_120000(3);
			break;
		case 3:
			sendWSMessage_150000(4);
			break;
		case 4:
			series_unload(_deskOpening);
			series_unload(_ripReachHand);
			_G(flags)[kWritingDeskDrawerOpen] = 1;
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (useFlag && player_said("WRITING DESK ")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_ripReachHand = series_load("RIP TREK MED REACH HAND POS1");
			setGlobals1(_ripReachHand, 1, 10, 10, 10, 0, 10, 1, 1, 1);
			sendWSMessage_110000(1);
			break;
		case 1:
			terminateMachineAndNull(_desk);
			_deskOpening = series_load("406 DESK OPENING");
			digi_play("406_s05", 2, 255, -1, 406);
			_desk = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x500, 0,
				triggerMachineByHashCallbackAlways, "writing desk opens");
			sendWSMessage_10000(1, _desk, _deskOpening, 15, 1, 2,
				_deskOpening, 1, 1, 0);
			break;
		case 2:
			terminateMachineAndNull(_desk);
			_desk = series_place_sprite("406 DESK CLOSE", 0, 0, 0, 100, 0x600);
			hotspot_set_active("WRITING DESK", true);
			hotspot_set_active("WRITING DESK ", false);
			hotspot_set_active("WRITING DESK DRAWER", false);
			sendWSMessage_120000(3);
			break;
		case 3:
			sendWSMessage_150000(4);
			break;
		case 4:
			series_unload(_deskOpening);
			series_unload(_ripReachHand);
			_G(flags)[kWritingDeskDrawerOpen] = 0;
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (useFlag && player_said("GAMES CABINET")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_ripReachHand = series_load("RIP TREK MED REACH HAND POS1");
			setGlobals1(_ripReachHand, 1, 10, 10, 10, 0, 10, 1, 1, 1);
			sendWSMessage_110000(1);
			break;
		case 1:
			if (_G(flags)[V307]) {
				terminateMachineAndNull(_gamesCabinet);
				_cabinetOpens = series_load("406 GAMES CABINET OPENS");
				digi_play("950_s36", 2, 255, -1, 950);
				_gamesCabinet = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0xf00, 0,
					triggerMachineByHashCallbackAlways, "game cabinet opens");
				sendWSMessage_10000(1, _gamesCabinet, _cabinetOpens, 1, 10, 2,
					_cabinetOpens, 10, 10, 0);
			} else {
				digi_play("406r20", 1, 255, 3);
			}
			break;
		case 2:
			terminateMachineAndNull(_gamesCabinet);
			_gamesCabinet = series_place_sprite("406 GAMES CABINET OPEN", 0, 0, 0, 100, 0xf00);
			hotspot_set_active("GAMES CABINET", false);
			hotspot_set_active("GAMES CABINET ", true);
			hotspot_set_active("GAMES CABINET DRAWER", true);
			hotspot_set_active("CABINET DRAWER OPEN", false);
			_G(flags)[V306] = 1;
			kernel_timing_trigger(1, 3);
			break;
		case 3:
			sendWSMessage_120000(4);
			break;
		case 4:
			sendWSMessage_150000(5);
			break;
		case 5:
			series_unload(_ripReachHand);
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (useFlag && player_said("GAMES CABINET ")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_ripReachHand = series_load("RIP TREK MED REACH HAND POS1");
			setGlobals1(_ripReachHand, 1, 10, 10, 10, 0, 10, 1, 1, 1);
			sendWSMessage_110000(1);
			break;
		case 1:
			terminateMachineAndNull(_gamesCabinet);
			_cabinetOpens = series_load("406 GAMES CABINET OPENS");
			digi_play("950_s36", 2, 255, -1, 950);
			_gamesCabinet = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0xf00, 0,
				triggerMachineByHashCallbackAlways, "game cabinet opens");
			sendWSMessage_10000(1, _gamesCabinet, _cabinetOpens, 10, 1, 2,
				_cabinetOpens, 1, 1, 0);
			break;
		case 2:
			if (player_said("KEYS"))
				digi_play("406_s06", 2);

			terminateMachineAndNull(_gamesCabinet);
			_desk = series_place_sprite("406 GAMES CABINET CLOSED", 0, 0, 0, 100, 0xf00);
			hotspot_set_active("GAMES CABINET", true);
			hotspot_set_active("GAMES CABINET ", false);
			hotspot_set_active("GAMES CABINET DRAWER", false);
			sendWSMessage_120000(3);
			break;
		case 3:
			sendWSMessage_150000(4);
			break;
		case 4:
			series_unload(_cabinetOpens);
			series_unload(_ripReachHand);
			_G(flags)[V306] = 0;

			if (player_said("GAMES CABINET ", "KEYS"))
				_G(flags)[V307] = 0;

			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (player_said("GAMES CABINET ", "KEYS")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_ripReachHand = series_load("RIP TREK MED REACH HAND POS1");
			setGlobals1(_ripReachHand, 1, 10, 10, 10, 0, 10, 1, 1, 1);
			sendWSMessage_110000(1);
			break;
		case 1:
			terminateMachineAndNull(_gamesCabinet);
			_cabinetOpens = series_load("406 GAMES CABINET OPENS");
			digi_play("950_s36", 2, 255, -1, 950);
			_gamesCabinet = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0xf00, 0,
				triggerMachineByHashCallbackAlways, "game cabinet opens");
			sendWSMessage_10000(1, _gamesCabinet, _cabinetOpens, 10, 1, 2,
				_cabinetOpens, 1, 1, 0);
			break;
		case 2:
			if (player_said("KEYS"))
				digi_play("406_s06", 2);

			terminateMachineAndNull(_gamesCabinet);
			_gamesCabinet = series_place_sprite("406 GAMES CABINET CLOSED", 0, 0, 0, 100, 0xf00);
			hotspot_set_active("GAMES CABINET", true);
			hotspot_set_active("GAMES CABINET ", false);
			hotspot_set_active("GAMES CABINET DRAWER", false);
			sendWSMessage_120000(3);
			break;
		case 3:
			sendWSMessage_150000(5);
			break;
		case 4:
			series_unload(_cabinetOpens);
			series_unload(_ripReachHand);
			_G(flags)[V306] = 0;

			if (player_said("GAMES CABINET ", "KEYS"))
				_G(flags)[V307] = 0;

			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (player_said("DOWN", "STAIRS")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(2);
			break;
		case 2:
			_G(flags)[V312] = 0;
			_G(game).setRoom(407);
			break;
		default:
			break;
		}
	} else if ((useFlag || lookFlag) && player_said("SMOKING HUTCH")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(2);
			break;
		case 2:
			_G(game).setRoom(456);

			if (_G(flags)[V322]) {
				adv_kill_digi_between_rooms(false);
				digi_play_loop("456_s03a", 3, 255, 700, 456);
			}
			break;
		default:
			break;							
		}
	} else if (player_said("WALK TO", "DOOR")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(2);
			break;
		case 2:
			_G(game).setRoom(404);
			break;
		default:
			break;
		}
	} else if (player_said("journal") && !takeFlag && !lookFlag &&
			!inv_player_has(_G(player).noun)) {
		if (_G(flags)[kCastleCartoon])
			digi_play("com016", 1);
		else if (_G(kernel).trigger != 6)
			sendWSMessage_multi("com015");
		else {
			_G(flags)[kCastleCartoon] = 1;
			sendWSMessage_multi("com015");
		}
	} else {
		return;
	}

	_G(player).command_ready = false;
}
#undef LOOK

void Room406::setHotspots() {
	for (auto *hs = _G(currentSceneDef).hotspots; hs; hs = hs->next)
		hs->active = true;

	hotspot_set_active(_G(flags)[V316] ? "MIRROR" : "BROKEN MIRROR", false);

	if (_G(flags)[V306]) {
		hotspot_set_active("GAMES CABINET", false);
	} else {
		hotspot_set_active("GAMES CABINET ", false);
		hotspot_set_active("GAMES CABINET DRAWER", false);
	}

	if (_G(flags)[kWritingDeskDrawerOpen]) {
		hotspot_set_active("WRITING DESK", false);
	} else {
		hotspot_set_active("WRITING DESK ", false);
		hotspot_set_active("WRITING DESK DRAWER", false);
	}

	if (inv_player_has("BILLIARD BALL") || _G(flags)[V321])
		hotspot_set_active("BILLIARD BALL", false);

	hotspot_set_active("CABINET DRAWER OPEN", false);
	hotspot_set_active("DESK DRAWER OPEN", false);
	hotspot_set_active("MESSAGES", false);
	hotspot_set_active("ENVELOPE", false);
	hotspot_set_active("KEYS", false);

	if (_G(flags)[V310]) {
		hotspot_set_active("ACE OF SPADES", false);
	} else {
		hotspot_set_active("ACE OF SPADES ", false);
		hotspot_set_active("SWITCH", false);
	}

	switch (_G(flags)[V312]) {
	case 0:
		hotspot_set_active("STAIRS", false);
		hotspot_set_active("BILLIARD TABLE ", false);
		break;

	case 1:
		hotspot_set_active("STAIRS", false);
		break;

	case 2:
		hotspot_set_active("BILLIARD TABLE", false);
		hotspot_set_active("BILLIARD TABLE ", false);
		break;

	default:
		break;
	}

	hotspot_set_active(_G(flags)[kPaintingOpen] ? "PAINTING" : "SMOKING HUTCH", false);
	hotspot_set_active(_G(flags)[V316] ? "MIRROR" : "BROKEN MIRROR", false);
}

bool Room406::takeKeys() {
	switch (_G(kernel).trigger) {
	case -1:
		if (inv_object_is_here("KEYS")) {
			inv_give_to_player("KEYS");
			hotspot_set_active("KEYS", false);
			terminateMachineAndNull(_keys);
			kernel_examine_inventory_object("PING KEYS", _G(master_palette),
				5, 1, 460, 265, 2, "406_s01", -1);
			return true;
		}
		break;

	case 2:
		player_set_commands_allowed(true);
		return true;

	default:
		break;
	}

	return false;
}

bool Room406::takeBilliardBall() {
	switch (_G(kernel).trigger) {
	case -1:
		if (inv_object_is_here("BILLIARD BALL")) {
			player_set_commands_allowed(false);
			_pickupBall = series_load("406 RIP PICKUP BALL");
			ws_hide_walker();

			_ripAction = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, false,
				triggerMachineByHashCallbackAlways, "RIP picks up ball");
			sendWSMessage_10000(1, _ripAction, _pickupBall, 1, 14, 2,
				_pickupBall, 14, 14, 0);
			return true;
		}
		break;

	case 2:
		terminateMachineAndNull(_poolBall);
		inv_give_to_player("BILLIARD BALL");
		hotspot_set_active("BILLIARD BALL", false);
		kernel_examine_inventory_object("PING BILLIARD BALL", _G(master_palette),
			5, 1, 125, 240, 3, 0, -1);
		return true;

	case 3:
		sendWSMessage_10000(1, _ripAction, _pickupBall, 14, 1, 4,
			_pickupBall, 1, 1, 0);
		return true;

	case 4:
		terminateMachineAndNull(_ripAction);
		ws_unhide_walker();
		series_unload(_pickupBall);
		player_set_commands_allowed(true);
		return true;

	default:
		break;
	}

	return false;
}

bool Room406::billiardBallOnTable() {
	switch (_G(kernel).trigger) {
	case -1:
		return true;

	case 1:
		if (inv_player_has("BILLIARD BALL")) {
			player_set_commands_allowed(false);
			_pickupBall = series_load("406 RIP PICKUP BALL");
			ws_hide_walker();
			_ripAction = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, 0,
				triggerMachineByHashCallbackAlways, "RIP picks up ball");
			sendWSMessage_10000(1, _ripAction, _pickupBall, 1, 14, 2,
				_pickupBall, 14, 14, 0);
			return true;
		}
		break;

	default:
		break;
	}

	return false;
}

void Room406::useSwitchPaintingOpen() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_ripHiHand = series_load("rip trek hi 1 hand");
		setGlobals1(_ripHiHand, 1, 5, 5, 5, 0, 5, 1, 1, 1);
		sendWSMessage_110000(1);
		break;

	case 1:
		hotspot_set_active("PAINTING", true);
		hotspot_set_active("SMOKING HUTCH", false);
		terminateMachineAndNull(_painting);
		_paintingOpening = series_load("406 PAINTING OPENING");
		digi_play("406_s07", 2, 255, 69);
		_painting = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, 0,
			triggerMachineByHashCallbackAlways, "HUTCH painting opens");
		sendWSMessage_10000(1, _painting, _paintingOpening, 30, 1, 2,
			_paintingOpening, 1, 1, 0);
		break;

	case 2:
		terminateMachineAndNull(_painting);
		series_unload(_paintingOpening);
		_painting = series_place_sprite("406 PAINTING CLOSED",
			0, 0, 0, 100, 0xf00);
		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripHiHand);
		_G(flags)[kPaintingOpen] = 0;
		player_set_commands_allowed(true);
		break;

	case 69:
		digi_play("406_s09", 2);
		break;

	default:
		break;
	}
}

void Room406::useSwitchPaintingClosed() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_ripHiHand = series_load("rip trek hi 1 hand");
		setGlobals1(_ripHiHand, 1, 5, 5, 5, 0, 5, 1, 1, 1);
		sendWSMessage_110000(1);
		break;

	case 1:
		hotspot_set_active("PAINTING", false);
		hotspot_set_active("SMOKING HUTCH", true);
		terminateMachineAndNull(_painting);
		_paintingOpening = series_load("406 PAINTING OPENING");
		digi_play("406_s07", 2, 255, 69);
		_painting = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, 0,
			triggerMachineByHashCallbackAlways, "HUTCH painting opens");
		sendWSMessage_10000(1, _painting, _paintingOpening, 1, 30, 2,
			_paintingOpening, 30, 30, 0);
		break;

	case 2:
		terminateMachineAndNull(_painting);
		series_unload(_paintingOpening);
		_painting = series_place_sprite("406 PAINTING OPEN",
			0, 0, 0, 100, 0xf00);
		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripHiHand);
		_G(flags)[kPaintingOpen] = 1;
		player_set_commands_allowed(true);
		break;

	case 69:
		digi_play("406_s08", 2);
		break;

	default:
		break;
	}
}

void Room406::useAceOfSpades1() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_ripHiHand = series_load("rip trek hi 1 hand");
		setGlobals1(_ripHiHand, 1, 5, 5, 5, 0, 5, 1, 1, 1);
		sendWSMessage_110000(1);
		break;

	case 1:
		terminateMachineAndNull(_cardDoor);
		hotspot_set_active("ACE OF SPADES", true);
		hotspot_set_active("ACE OF SPADES ", false);
		hotspot_set_active("SWITCH", false);
		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripHiHand);
		_G(flags)[V310] = 0;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room406::useAceOfSpades2() {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		_ripHiHand = series_load("rip trek hi 1 hand");
		setGlobals1(_ripHiHand, 1, 5, 5, 5, 0, 5, 1, 1, 1);
		sendWSMessage_110000(1);
		break;

	case 1:
		_cardDoor = series_place_sprite("406 CARD DOOR OPEN BY PICT", 0, 0, 0, 100, 0xf00);
		hotspot_set_active("ACE OF SPADES", false);
		hotspot_set_active("ACE OF SPADES ", true);
		hotspot_set_active("SWITCH", true);
		sendWSMessage_120000(3);
		break;

	case 3:
		sendWSMessage_150000(4);
		break;

	case 4:
		series_unload(_ripHiHand);
		_G(flags)[V310] = 1;
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

bool Room406::lookBilliardBall() {
	switch (_G(kernel).trigger) {
	case -1:
		if (inv_object_is_here("BILLIARD BALL")) {
			player_set_commands_allowed(false);
			_ripThrowsBall = series_load("406 RIP THROWS BALL");
			digi_preload("406_s12");
			terminateMachineAndNull(_poolBall);
			ws_hide_walker();
			_poolBall = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, 0,
				triggerMachineByHashCallbackAlways, "RIP throws ball");
			sendWSMessage_10000(1, _poolBall, _ripThrowsBall, 1, 31, 2,
				_ripThrowsBall, 31, 31, 0);
			return true;
		}
		break;

	case 2:
		sendWSMessage_10000(1, _poolBall, _ripThrowsBall, 32, 66, 3,
			_ripThrowsBall, 66, 66, 0);
		digi_play("406_s12", 2);
		return true;

	case 3:
		terminateMachineAndNull(_poolBall);
		ws_unhide_walker();
		series_unload(_ripThrowsBall);
		_poolBall = series_place_sprite("BILLIARD BALL", 0, 0, 0, 100, 0x200);
		player_set_commands_allowed(true);
		return true;

	default:
		break;
	}

	return false;
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
