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

#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_sys.h"
#include "m4/platform/keys.h"
#include "m4/riddle/rooms/section3/room304.h"
#include "m4/riddle/vars.h"
#include "m4/riddle/riddle.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room304::init() {
	interface_show();
	_val1 = 0;

	if (_G(game).previous_room != KERNEL_RESTORING_GAME)
		_flag1 = false;

	if (_G(game).previous_room == 303 && _G(flags)[V001])
		_G(game).previous_room = 354;

	if (_G(game).previous_room == KERNEL_RESTORING_GAME && _G(flags)[V001])
		_G(game).previous_room = 354;

	hotspot_set_active(_G(flags)[V001] ? "RUG" : "RUG ", false);

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		if (_G(flags)[V001])
			midi_play("thinker2", 255, 1, -1, 949);

		if (_G(flags)[V084] == 2 || player_been_here(201))
			_sword = series_show_sprite("one frame sword", 0, 0xa00);
		else
			hotspot_set_active("SAMURAI SWORD", false);
		break;

	case 303:
		player_set_commands_allowed(false);
		ws_demand_location(458, 263, 8);

		if (_G(flags)[V084] == 2 || player_been_here(201)) {
			_sword = series_show_sprite("one frame sword", 0, 0xa00);
			kernel_timing_trigger(1, 51);
		} else {
			hotspot_set_active("SAMURAI SWORD", false);
		}
		break;

	default:
		AddSystemHotkey(KEY_ESCAPE, Hotkeys::escape_key_pressed);
		AddSystemHotkey(KEY_F2, Hotkeys::saveGame);
		AddSystemHotkey(KEY_F3, Hotkeys::loadGame);

		if (_G(flags)[V001]) {
			mouse_show();
			_G(flags)[V001] = 1;
			digi_preload("304r52");
			digi_preload("304_s01");
			digi_preload("304_s04");
			player_set_commands_allowed(false);

			_useSword = _useHandlingStick = false;
			_val4 = 0;
			ws_demand_location(452, 285, 9);
			kernel_timing_trigger(1, 49);
			_trunk = series_show_sprite("one frame trunk", 0, 0);
			_stick = series_show_sprite("one frame stick", 0, 0xf00);

			LoadWSAssets("304 MC SCRIPT", _G(master_palette));

			_mei1 = series_load("MEI STANDS AND LOOKS");
			_G(globals)[GLB_TEMP_1] = _mei1 << 24;
			_G(globals)[GLB_TEMP_2] = 0xf050000;

			_mei2 = series_load("MEI TURNS INTO WALKER");
			_G(globals)[GLB_TEMP_3] = _mei2 << 24;
			_G(globals)[GLB_TEMP_4] = 0xf050000;

			_mei3 = series_load("MEI CHEN NY WALKER POS4");
			_G(globals)[GLB_TEMP_5] = _mei3 << 24;

			_mei = TriggerMachineByHash(32, nullptr, -1, -1, intrMsgNull, false, "MACHINE mc");

			LoadWSAssets("304 FL SN SCRIPT", _G(master_palette));
			AddWSAssetCELS("test3", 2, nullptr);
			AddWSAssetCELS("ripsh3", 12, nullptr);
			_smoke = series_load("LF AND COBRA FIRE SMOKE", -1, _G(master_palette));
			_G(globals)[GLB_TEMP_1] = _smoke << 24;
			_G(globals)[GLB_TEMP_2] = 0x10000;

			_machine2 = TriggerMachineByHash(45, nullptr, -1, -1, intrMsg, 0, "MACHINE fl snake rock");
		}

		_sword = series_show_sprite("one frame sword", 0, 0xa00);
		break;
	}

	if (!_G(flags)[V001]) {
		hotspot_set_active("MEI CHEN", false);
		hotspot_set_active("FENG LI", false);

		if (_G(flags)[V084] != 1 || player_been_here(301)) {
			hotspot_set_active("HANDLING STICK", false);
		} else {
			_stick = series_show_sprite("one frame stick", 0, 0xf00);
		}
	}
}

void Room304::daemon() {
	switch (_G(kernel).trigger) {
	case 49:
		ws_walk(387, 292, nullptr, 50, 9);
		digi_play("304m01", 1, 255, 60);
		break;

	case 50:
		player_set_commands_allowed(true);
		break;

	case 51:
		ws_walk(431, 287, nullptr, 50, 8);
		break;

	case 60:
		midi_play("thinker2", 255, 1, -1, 949);
		break;

	default:
		break;
	}
}

void Room304::pre_parser() {
	bool takeFlag = player_said("take");
	bool useFlag = player_said("gear");

	if (_flag1) {
		terminateMachineAndNull(_machine3);
		_flag1 = false;
		intr_cancel_sentence();
		hotspot_restore_all();
		interface_show();
	}

	// At the very start of the game, you can't freely move around
	// the room until the Cobra has been dealt with
	if (_G(flags)[V001]) {
		_G(player).resetWalk();

		if ((takeFlag || useFlag) && player_said("samurai sword")) {
			player_set_commands_allowed(false);
			_useSword = true;
			digi_preload("304_s10");
			digi_preload("304_s05");
			_cutSnake = series_load("CUT SNAKE");
			sendWSMessage(0xa0000, 0, _machine2, 0, nullptr, 1);
		}

		if ((takeFlag || useFlag) && player_said("handling stick")) {
			digi_preload("304_s03");
			digi_preload("304_s08");
			digi_preload("304_s02");
			digi_preload("304_s11");
			_useHandlingStick = true;
		}
	}
}

void Room304::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");
	bool useFlag = player_said_any("push", "pull", "gear", "open", "close");

	if (lookFlag && player_said("cartoon")) {
		if (_G(flags)[V001]) {
			digi_play("304r13", 1);

		} else {
			hotspot_hide_all();
			interface_hide();
			intr_cancel_sentence();
			hotspot_add_dynamic("x", " ", 0, 0, 1500, 374, 0);

			_flag1 = true;
			_pu = series_show_sprite(_G(flags)[V000] ? "394pu99" : "304pu99", 0, 0);
			digi_play("304r59", 1);
		}
	} else if (_G(kernel).trigger == 749) {
		midi_stop();
	} else if (_G(flags)[V001] && _val4
		&& _G(kernel).trigger >= 49 && _G(kernel).trigger <= 54) {
		switch (_G(kernel).trigger) {
		case 49:
			ws_hide_walker();
			_cobraKills = series_ranged_play("COBRA KILLS RIP AND LF",
				1, 0, 1, 4, 100, 0x200, 5, 50);
			break;
		case 50:
			_cobraKills = series_ranged_play("COBRA KILLS RIP AND LF",
				1, 0, 5, 19, 100, 0x200, 5, 51);
			digi_play("304_s07", 1);
			break;
		case 51:
			_cobraKills = series_ranged_play("COBRA KILLS RIP AND LF",
				1, 0, 20, 41, 100, 0x200, 5, 54);
			digi_play("304_s07", 1);
			break;
		case 52:
			_cobraKills = series_ranged_play("COBRA KILLS RIP AND LF",
				1, 0, 51, 51, 100, 0x200, 3000, -1);
			disable_player_commands_and_fade_init(-1);
			midi_fade_volume(0, 120);
			kernel_timing_trigger(120, 53);
			break;
		case 53:
			_G(game).setRoom(413);
			break;
		case 54:
			_cobraKills = series_ranged_play("COBRA KILLS RIP AND LF",
				1, 0, 42, 51, 100, 0x200, 5, 52);
			digi_play("304_s12", 2);
			break;
		default:
			break;
		}
	} else if (_G(flags)[V001] && (takeFlag || useFlag) && player_said("samurai sword")) {
		if (_G(flags)[V001]) {
			switch (_G(kernel).trigger) {
			case 11:
				terminateMachineAndNull(_sword);
				series_ranged_play("CUT SNAKE", 1, 0, 11, 22, 100, 0x200, 4, 12);
				digi_play("304_s10", 1);
				break;
			case 12:
				series_ranged_play("CUT SNAKE", 1, 0, 23, 55, 100, 0x200, 4, 13);
				break;
			case 13:
				series_ranged_play("CUT SNAKE", 1, 0, 56, 69, 100, 0x200, 4, 14);
				digi_play("304_s09", 2);
				break;
			case 14:
				midi_fade_volume(0, 120);
				kernel_timing_trigger(120, 749);
				Common::strcpy_s(_G(player).noun, "HANDLING STICK");
				series_ranged_play("CUT SNAKE", 1, 0, 70, 89, 100, 0x200, 4, 17);
				digi_play("com125", 1);
				break;
			default:
				break;
			}
		} else {
			digi_play("304r64", 1);
		}
	} else if (_G(flags)[V001] && (takeFlag || useFlag) && player_said("handling stick")) {
		// Catching snake with the handling stick, or killing it
		// with the sword
		handlingStickAndSword();
	
	} else if (lookFlag && player_said_any("native mask", "shield")) {
		digi_play("304r05", 1);
	} else if (lookFlag && player_said("mailbag")) {
		digi_play("304r15", 1);
	} else if (lookFlag && player_said("picture")) {
		digi_play("304r16", 1);
	} else if (lookFlag && player_said("globe")) {
		digi_play("304r18", 1);
	} else if (lookFlag && player_said("vase")) {
		digi_play("304r19", 1);
	} else if (lookFlag && player_said_any("rug", "rug ")) {
		digi_play("304r20", 1);
	} else if (lookFlag && player_said("light switch")) {
		digi_play("304r21", 1);
	} else if (lookFlag && player_said("failing cabinet")) {
		digi_play("304r29", 1);
	} else if (takeFlag && player_said("flat file")) {
		digi_play("304r30", 1);
	} else if (takeFlag && player_said("cartoon")) {
		digi_play("304r35", 1);
	} else if (takeFlag && player_said("picture")) {
		digi_play("304r36", 1);
	} else if (useFlag && player_said("flat file")) {
		digi_play(_G(flags)[V001] ? "304r46" : "304r76", 1);
	} else if (useFlag && player_said("filing cabinet")) {
		digi_play(_G(flags)[V001] ? "304r45" : "304r75", 1);
	} else if (useFlag && player_said("trunk")) {
		digi_play(_G(flags)[V001] ? "304r47" : "304r70", 1);
	} else if (lookFlag && player_said("feng li")) {
		digi_play("304r03", 1);
	} else if (lookFlag && player_said("mei chen")) {
		digi_play("304r04", 1);
	} else if (lookFlag && player_said("beer stein") && _G(kernel).trigger == -1) {
		digi_play("304r06", 1, 255, _G(flags)[V001] ? 1 : -1);
	} else if (lookFlag && player_said("beer stein") && _G(kernel).trigger == 1) {
		digi_play("304r07", 1);
	} else if (lookFlag && player_said("desk")) {
		digi_play(_G(flags)[V001] ? "304r08" : "304r56", 1);
	} else if (lookFlag && player_said("filing cabinet")) {
		digi_play(_G(flags)[V001] ? "304r09" : "304r57", 1);
	} else if (lookFlag && player_said("flat file")) {
		digi_play(_G(flags)[V001] ? "304r10" : "304r58", 1);
	} else if (lookFlag && player_said("samurai sword")) {
		digi_play("304r11", 1);
	} else if (lookFlag && player_said("handling stick")) {
		digi_play(_G(flags)[V001] ? "304r12" : "304r82", 1);
	} else if (lookFlag && player_said("trunk")) {
		if (_G(flags)[V001])
			digi_play("304r14", 1);
		else if (_G(flags)[V000] || _G(flags)[V084] == 1)
			digi_play("304r61", 1);
		else
			digi_play("304r60", 1);
	} else if (lookFlag && player_said("bookshelf")) {
		digi_play(_G(flags)[V001] ? "304r17" : "304r62", 1);
	} else if (player_said("exit")) {
		if (_G(flags)[V001]) {
			digi_play("304r01", 1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				disable_player_commands_and_fade_init(1);
				break;
			case 1:
				_G(game).setRoom(303);
				break;
			default:
				break;
			}
		}
	} else if (takeFlag && player_said("native mask")) {
		digi_play(_G(flags)[V001] ? "304r25" : "304r66", 1);
	} else if (takeFlag && player_said("shield")) {
		digi_play(_G(flags)[V001] ? "304r26" : "304r67", 1);
	} else if (takeFlag && player_said("samurai sword") && _G(flags)[V084] == 2) {
		digi_play("304r64", 1);
	} else if (takeFlag && player_said("handling stick") && _G(flags)[V084] == 1) {
		digi_play("304r65", 1);
	} else if (takeFlag && player_said("beer stein")) {
		digi_play(_G(flags)[V001] ? "304r27" : "304r68", 1);
	} else if (takeFlag && player_said("desk")) {
		digi_play(_G(flags)[V001] ? "304r28" : "304r69", 1);
	} else if (takeFlag && player_said("trunk")) {
		digi_play(_G(flags)[V001] ? "304r31" : "304r70", 1);
	} else if (takeFlag && player_said("mailbag")) {
		digi_play("304r32", 1);
	} else if (takeFlag && player_said("globe")) {
		digi_play(_G(flags)[V001] ? "304r33" : "304r71", 1);
	} else if (takeFlag && player_said("vase")) {
		digi_play(_G(flags)[V001] ? "304r34" : "304r72", 1);
	} else if (takeFlag && player_said("bookshelf")) {
		digi_play(_G(flags)[V001] ? "304r37" : "304r37a", 1);
	} else if (takeFlag && player_said("rug")) {
		digi_play("304r73", 1);
	} else if (takeFlag && player_said("rug ")) {
		digi_play("304r38", 1);
	} else if (useFlag && player_said("cartoon") && _G(player).click_x > 534) {
		digi_play(_G(flags)[V001] ? "304r13" : "304r77", 1);
	} else if (takeFlag && _G(player).click_y <= 374 && _G(flags)[V001]) {
		switch (imath_ranged_rand(1, 3)) {
		case 1:
			digi_play("304r39", 1);
			break;
		case 2:
			digi_play("304r40", 1);
			break;
		case 3:
			digi_play("304r41", 1);
			break;
		default:
			break;
		}
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room304::intrMsg(frac16 myMessage, struct machine *sender) {
	Room304 *r = static_cast<Room304 *>(g_engine->_activeRoom);
	auto oldMode = _G(kernel).trigger_mode;

	if ((myMessage >> 16) == 57) {
		if ((_G(globals)[GLB_TEMP_1] >> 16) == 1) {
			player_set_commands_allowed(false);
			terminateMachineAndNull(r->_machine2);
			series_unload(r->_smoke);
			r->_cobra = series_load("COBRA KILLS RIP AND LF");
			digi_preload("304_s07");

			_G(kernel).trigger_mode = KT_PARSE;
			r->_machine2 = series_ranged_play("LF AND COBRA FIRE SMOKE", 1, 0, 25, 49, 100, 0x200, 5, 49);
			_G(kernel).trigger_mode = oldMode;

			digi_play("304_s06", 1);
			r->_val4 = 1;
			sendWSMessage(0x200000, 0, r->_mei, 0, nullptr, 1);
			return;
		}

		if (r->_useHandlingStick) {
			ws_demand_location(382, 295);
			ws_hide_walker();
			player_set_commands_allowed(false);
			terminateMachineAndNull(r->_machine2);
			terminateMachineAndNull(r->_stick);
			digi_stop(1);
			terminateMachineAndNull(r->_trunk);

			r->_safe3 = series_stream("SNAKE HANDLE", 5, 0, 17);
			series_stream_break_on_frame(r->_safe3, 10, 13);
			sendWSMessage(0x200000, 0, r->_mei, 0, nullptr, 1);
			_G(flags)[V084] = 2;
			return;
		}
	}

	if ((myMessage >> 16) == 58 && r->_useSword) {
		ws_demand_location(382, 295);
		ws_hide_walker();
		player_set_commands_allowed(false);
		terminateMachineAndNull(r->_machine2);
		series_unload(r->_smoke);
		digi_stop(1);

		_G(kernel).trigger_mode = KT_PARSE;
		series_ranged_play("CUT SNAKE", 1, 0, 0, 10, 100, 0x200, 4, 11);
		_G(kernel).trigger_mode = oldMode;

		sendWSMessage(0x200000, 0, r->_mei, 0, nullptr, 1);
		_G(flags)[V084] = 1;
	}
}

void Room304::handlingStickAndSword() {
	switch (_G(kernel).trigger) {
	case 13:
		series_stream_break_on_frame(_safe3, 29, 14);
		digi_play("304_s03", 1);
		break;
	case 14:
		series_stream_break_on_frame(_safe3, 106, 15);
		digi_play("304_s08", 1);
		break;
	case 15:
		series_stream_break_on_frame(_safe3, 111, 16);
		digi_play("304_s02", 1);
		break;
	case 16:
		series_stream_break_on_frame(_safe3, 145, 19);
		midi_stop();
		digi_play("304_s11", 1);
		break;

	case 17:
		if (_G(flags)[V084] == 1) {
			digi_unload("304_s10");
			digi_unload("304_s05");
			series_show_sprite("one frame dead snake", 0, 0x100);
		} else if (_G(flags)[V084] == 2) {
			digi_unload("304_s03");
			digi_unload("304_s08");
			digi_unload("304_s02");
			digi_unload("304_s11");
		}

		terminateMachineAndNull(_mei);
		series_unload(_mei1);
		series_unload(_mei2);

		ws_unhide_walker();
		player_update_info();
		ws_demand_location(_G(player_info).x - 28,
			_G(player_info).y + 9);

		ws_walk_load_walker_series(S3_NORMAL_DIRS2, S3_NORMAL_NAMES2);
		ws_walk_load_shadow_series(S3_SHADOW_DIRS2, S3_SHADOW_NAMES2);

		_headUp1 = series_load("fl returns to head up");
		_headUp2 = series_ranged_play("fl returns to head up",
			1, 0, 0, 0, 100, 0x100, 250, 31);
		_mei = triggerMachineByHash_3000(8, 3, *S3_NORMAL_DIRS2, *S3_SHADOW_DIRS2,
			464, 283, 8, triggerMachineByHashCallback3000, "mc");
		ws_demand_facing(_mei, 8);
		kernel_timing_trigger(1, 74);
		break;

	case 18:
		sendWSMessage_10000(_mei, 73, 275, 3, 20, 0);
		break;

	case 19:
		digi_play("com125", 1);
		break;

	case 20:
		sendWSMessage_10000(_mei, 29, 295, 3, 21, 1);
		break;

	case 21:
		series_unload(_suit1);
		terminateMachineAndNull(_headUp2);
		series_unload(_headUp1);

		_headUp1 = series_load("lf talk 1");
		ws_hide_walker(_mei);
		series_unload(221);
		series_unload(222);

		_suit1 = series_load("suit chin in hand pos3");
		_suit2 = series_load("mc hands out talk pos3");
		_suit3 = series_load("MC UNTIES LF");

		series_ranged_play("MC UNTIES LF", 1, 0, 1, 21, 100, 0xa05, 5, 222);
		break;

	case 22:
		ws_unhide_walker(_mei);
		setGlobals1(_suit1, 1, 15, 15, 15, 1, 16, 19, 20, 23, 1);
		sendWSMessage_110000(-1);

		_headUp2 = series_ranged_play("MC UNTIES LF", -1, 0, 82, 82, 100, 0x200, 250);
		setGlobals3(_suit2, 1, 24);
		sendWSMessage_F0000(_mei, 23);
		digi_play("304m04", 1);
		break;

	case 23:
		terminateMachineAndNull(_headUp2);
		series_unload(_suit2);
		series_ranged_play("MC UNITES LF", 1, 1, 83, 98, 100, 0xf05, 7, 25);
		digi_play("304f02", 1);
		break;

	case 25:
		series_unload(_suit3);
		_headUp2 = series_ranged_play("MC UNTIES LF", -1, 0, 98, 98, 100, 0x200, 250);
		kernel_timing_trigger(20, 26);
		break;

	case 26:
		digi_play("304r48", 1, 255, 27);
		sendWSMessage_120000(-1);
		break;

	case 27:
		terminateMachineAndNull(_headUp2);
		sendWSMessage_110000(28);
		_headUp2 = series_ranged_play("lf talk 2", 1, 1, 5, 10, 100, 0x100, 6, 33);
		digi_play("304f03", 1);
		break;

	case 28:
		sendWSMessage_140000(29);
		break;

	case 29:
		series_unload(_suit1);
		_suit1 = series_load("rip suit arms x point pos3");
		setGlobals1(_suit1, 1, 13, 14, 14, 1, 14, 14, 14, 19, 1,
			14, 44, 44, 44, 1, 0, 0, 0, 0, 0);
		sendWSMessage_110000(34);
		digi_play("304r49", 1, 255, 35);
		break;

	case 31:
		terminateMachineAndNull(_headUp2);
		series_ranged_play("fl returns to head up", 1, 0, 1, 6, 100, 0x100, 7, 32);
		break;

	case 32:
		_headUp2 = series_ranged_play("fl returns to head up", -1, 0, 6, 6, 100, 0x100, 300);
		break;

	case 33:
		_headUp2 = series_ranged_play("fl talk 1", -1, 0, 0, 0, 100, 0x200, 250);
		break;

	case 34:
		sendWSMessage_120000(-1);
		break;

	case 35:
		sendWSMessage_110000(36);
		break;

	case 36:
		terminateMachineAndNull(_headUp2);
		_headUp2 = series_ranged_play("lf talk 1", 1, 1, 0, 14, 100, 0xa00, 6, 37);
		digi_play("304f04", 1, 255);
		break;

	case 37:
		_headUp2 = series_ranged_play("lf talk 1", -1, 0, 0, 0, 100, 0x200, 250);
		_suit3 = series_load("mc stand and talk pos3");
		setGlobals1(_suit3, 1, 1, 1, 8, 1);
		sendWSMessage_110000(_mei, 38);
		digi_play("304m05", 1, 255, 38);
		break;

	case 38:
		if (_ctr1 >= 1) {
			_ctr1 = 0;
			sendWSMessage_140000(_mei, 39);
		} else {
			++_ctr1;
		}
		break;

	case 39:
		series_unload(_suit3);
		sendWSMessage_160000(3, 44);
		kernel_timing_trigger(30, 67);
		break;

	case 40:
		series_unload(_suit1);
		_safe1 = series_load("rip opens safe part 1");
		_safe2 = series_load("rip opens safe part 2");
		ws_walk(510, 300, 0, 43, 3);
		digi_preload("304r51");
		digi_play("304r51", 1, 255, 79);
		break;

	case 43:
		player_update_info();
		ws_hide_walker();
		digi_preload("304_s04");
		digi_preload("304f05");
		digi_preload("304r52");
		series_plain_play("rip opens safe part 1", 1, 0, 100, 0, 5, 45);
		break;

	case 44:
		sendWSMessage_150000(40);
		break;

	case 45:
		series_plain_play("rip opens safe part 2", 1, 0, 100, 0, 5, 46);
		digi_play("304_s01", 2);
		break;

	case 46:
		_safe3 = series_stream("RIP OPENS SAFE PART 3", 5, 0, 49);
		series_stream_break_on_frame(_safe3, 44, 53);
		digi_play("304_s04", 2);
		break;

	case 47:
		terminateMachineAndNull(_headUp2);
		_headUp2 = series_ranged_play("lf talk 1", 1, 1, 0, 4, 100, 0xa00, 6, 48);
		digi_play("304f05", 1, 255, 54);
		break;

	case 48:
		_headUp2 = series_ranged_play("lf talk 1", -1, 0, 0, 0, 100, 0x200, 250);
		break;

	case 49:
		ws_unhide_walker();
		series_unload(_safe1);
		series_unload(_safe2);
		_safe1 = series_load("one frame safe open");
		series_show_sprite("one frame safe open", 0, 0xa00);
		_suit3 = series_load("mc hand to chin pos3");
		ws_walk(260, 279, nullptr, 80, 8);
		break;

	case 51:
		digi_unload("304_s04");
		digi_unload("304_f05");
		digi_unload("304r52");
		sendWSMessage_140000(-1);
		ws_hide_walker(_mei);

		_field60 = series_ranged_play_xy("mc hand to chin pos3",
			1, 0, 0, 17, 29, 295, 100, 0x100, 9);
		digi_play("304m06", 1, 255, 55);
		break;

	case 53:
		inv_give_to_player("POST MARKED ENVELOPE");
		break;

	case 54:
		digi_play("304r52", 1, 255, 51);
		break;

	case 55:
		if (_val1) {
			kernel_timing_trigger(1, 56);
		} else {
			kernel_timing_trigger(60, 55);
		}
		break;

	case 56:
		_suit1 = series_load("rip suit stander talk pos4");
		setGlobals1(_suit1, 1, 1, 1, 5, 1);
		sendWSMessage_110000(58);
		digi_play("304r53", 1, 255, 58);
		break;

	case 58:
		if (_ctr1 >= 1) {
			_ctr1 = 0;
			sendWSMessage_140000(59);
		} else {
			++_ctr1;
		}
		break;

	case 59:
		series_unload(_suit1);
		terminateMachineAndNull(_field60);
		series_unload(_suit3);

		_suit3 = series_load("mc hand to chin nod pos3");
		series_ranged_play_xy("mc hand to chin nod pos3", 2, 1,
			0, 7, 29, 295, 100, 0xa00, 7, 63);
		digi_play("304m07", 1, 255, 60);
		break;

	case 60:
		_suit1 = series_load("RIP LFT GEST RT PNT");
		setGlobals3(_suit1, 1, 45);
		sendWSMessage_F0000(75);
		digi_play("304r54", 1, 255, 61);
		break;

	case 61:
		_suit3 = series_load("mc hands out talk pos3");
		kernel_timing_trigger(40, 65);
		break;

	case 63:
		series_unload(_suit3);
		_suit3 = series_load("mc hand to chin pos3");
		_field60 = series_ranged_play_xy("mc hand to chin pos3",
			1, 2, 0, 17, 29, 295, 100, 0x100, 7, 64);
		break;

	case 64:
		ws_unhide_walker(_mei);
		series_unload(_suit3);
		break;

	case 65:
		setGlobals1(_suit3, 1, 10, 11, 16, 1, 17, 24, 1, 1, 1);
		sendWSMessage_110000(_mei, -1);
		digi_play("304m08", 1, 255, 66);
		kernel_timing_trigger(60, 69);
		break;

	case 66:
		sendWSMessage_120000(_mei, -1);
		_G(game).setRoom(303);
		break;

	case 67:
		digi_play("304r50", 1);
		break;

	case 69:
		ws_walk(458, 263, 0, -1, 2);
		kernel_timing_trigger(30, 678);
		break;

	case 71:
		if (_ctr1 >= 1) {
			_ctr1 = 0;
			sendWSMessage_140000(_mei, 72);
		} else {
			++_ctr1;
		}
		break;

	case 72:
		series_unload(_suit3);
		_suit1 = series_load("rip suit rt hand gest talk pos3");
		setGlobals1(_suit1, 1, 11, 12, 15, 1);
		sendWSMessage_110000(73);
		digi_play(_useHandlingStick ? "304r24" : "304r23", 1, 255, 73);
		sendWSMessage_10000(_mei, 242, 274, 3, 18, 0);
		break;

	case 73:
		if (_ctr1 >= 1) {
			_ctr1 = 0;
			sendWSMessage_140000(-1);
		} else {
			++_ctr1;
		}
		break;

	case 74:
		_ctr1 = 0;
		_suit3 = series_load("mei ny hands out talk pos4");
		setGlobals1(_suit3, 1, 9, 10, 15, 1);
		sendWSMessage_110000(_mei, 71);
		digi_play(_useHandlingStick ? "304m03" : "304m02", 1, 255, 71);
		break;

	case 75:
		series_unload(_suit1);
		break;

	case 78:
		digi_unload("304r51a");
		kernel_timing_trigger(60, 47);
		break;

	case 79:
		digi_unload("304r51");
		digi_preload("304r51a");
		digi_play("304r51a", 1, 255, 78);
		break;

	case 80:
		_val1 = 1;
		break;

	case 222:
		digi_play("304_s13", 2);
		series_ranged_play("MC UNTIES LF", 1, 0, 22, 82, 100, 0xa05, 5, 22);
		break;

	case 678:
		disable_player_commands_and_fade_init(-1);
		break;

	default:
		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
