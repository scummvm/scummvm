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

#include "m4/riddle/rooms/section6/room604.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"
#include "m4/adv_r/adv_file.h"
#include "m4/adv_r/other.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room604::preload() {
	_G(player).walker_type = 1;
	_G(player).shadow_type = 1;
}

void Room604::init() {
	_val1 = 0;

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_val2 = 0;
		_val3 = 0;
		_val4 = 0;
		_val5 = 0;
	}

	static const char *DIGI[14] = {
		"GASPOUR", "610s01", "610_s02", "610_s02a",
		"610_s03", "610_s03a", "610_s03b", "610k07",
		"610k08", "610k09", "610k10", "610k11",
		"610k12", "610k13"
	};
	for (int i = 0; i < 14; ++i)
		digi_preload(DIGI[i]);

	if (_G(flags)[V203] == 8) {
		hotspot_set_active("door ", false);

		if (inv_object_is_here("SPARK PLUG TOOL")) {
			_sparkPlugTool = series_show("604 spark plug tool", 0xd00, 16);
		} else {
			hotspot_set_active("spark plug tool", false);
		}

		kernel_timing_trigger(300, 135);
	} else {
		hotspot_set_active("door", false);
		hotspot_set_active("spark plug tool", false);
	}

	_sparkPlugWire = series_show("604 spark plug wire on", 0xd00, 16);
	_ripLowReach1 = series_load("RIP LOW REACH POS1");
	_ripLowReach2 = series_load("RIP TREK LOW REACH POS2");
	_ripMedReach1 = series_load("RIP TREK MED REACH HAND POS1");

	hotspot_set_active(_G(flags)[V203] == 8 ? "window" : "window ", false);

	if (inv_object_is_here("LIGHTER")) {
		_flame = series_play("FLAME ON FLOOR", 0xd00, 4, -1, 5, -1, 100, 0, 0, 0, 7);
	} else {
		hotspot_set_active("LIGHTER", false);
	}

	if (_G(flags)[V203] == 8) {
		series_load("SHED DOOR OPENS");
		_shedDoor = series_show("SHED DOOR OPENS", 0xf00, 16);
	}

	hotspot_set_active("wire", false);
	hotspot_set_active("wire ", false);

	if (_G(flags)[V189] == 3)
		hotspot_set_active("wire ", true);
	else
		hotspot_set_active("wire", true);

	hotspot_set_active("PULL CORD", false);
	hotspot_set_active("PULL CORD ", false);
	hotspot_set_active("PULL CORD  ", false);
	hotspot_set_active("PULL CORD   ", false);
	hotspot_set_active("PULL CORD", false);
	hotspot_set_active("  ", false);

	if (inv_object_is_here("PULL CORD")) {
		switch (_G(flags)[V189]) {
		case 0:
			_pullCord1 = series_show("604 genset pull cord", 0xd00, 16);
			hotspot_set_active("PULL CORD", true);
			break;
		case 1:
			_pullCord2 = series_show("ONE FRAME PULL CORD", 0xd00, 16);
			hotspot_set_active("PULL CORD ", true);
			hotspot_set_active("plug", false);
			break;
		case 2:
			_doorWireGone = series_show("door wire be gone", 0xe00, 16);
			_cord = series_show("604cord2", 0xd00, 16);
			hotspot_set_active("PULL CORD  ", true);
			break;
		case 3:
			_doorWireGone = series_show("door wire be gone", 0xe00, 16);
			_tiedWires = series_show("604 wires tied together", 0xd00, 16);
			hotspot_set_active("PULL CORD   ", true);
			hotspot_set_active("plug", false);
			hotspot_set_active("  ", true);
			kernel_load_variant("604lock1");
			break;
		default:
			break;
		}
	}

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		if (_G(flags)[V203] == 8)
			midi_play("tensions", 255, 1, -1, 949);

		_shedDoor = series_show("SHED DOOR OPENS", 0xf00, 16);

		if (_G(flags)[GLB_TEMP_5])
			digi_preload("genrun");
		break;

	case 603:
		player_set_commands_allowed(false);
		_shedDoor = series_play("SHED DOOR OPENS", 0xf00, 16, 50, 11);
		ws_demand_location(156, 338, 3);
		digi_play("DOOROPEN", 2);
		break;

	default:
		_shedDoor = series_show("SHED DOOR OPENS", 0xf00, 16);

		if (_G(flags)[V203] == 8) {
			ws_demand_location(380, 304, 3);
			ws_hide_walker();
			series_load("rip crawls through window");
			_ripley = series_play("RIP CRAWLS THROUGH WINDOW", 0x100, 2, 10, 6);
			player_set_commands_allowed(false);

		} else {
			ws_demand_location(381, 329, 10);
		}
		break;
	}

	series_load("the generator cord");
	_generatorCord = series_show_sprite("the generator cord", 0, 0xf00);

	if (_G(flags)[GLB_TEMP_5])
		digi_play_loop("genrun", 3);
}

void Room604::daemon() {
	daemon1();

	switch (_G(kernel).trigger) {
	case 10:
		ws_unhide_walker();
		ws_walk(374, 330, nullptr, 11, 10);
		break;

	case 11:
		player_set_commands_allowed(true);
		break;

	case 12:
		player_set_commands_allowed(true);
		digi_play("603_s03", 2);
		break;

	case 50:
		ws_walk(238, 339, nullptr, 52, 3);
		break;

	case 52:
		terminateMachineAndNull(_shedDoor);
		_shedDoor = series_play("SHED DOOR OPENS", 0xf00, 18, 12, 11);
		break;

	case 666:
		if (_val3) {
			if (++_val4 < 15) {
				kernel_timing_trigger(60, 666);
			} else {
				Common::strcpy_s(_G(player).verb, "kill rip");
				_G(kernel).trigger_mode = KT_PARSE;
				kernel_timing_trigger(1, 667);
			}
		}
		break;

	default:
		break;
	}
}

void Room604::pre_parser() {
	bool takeFlag = player_said("take");
	bool useFlag = player_said_any("push", "pull", "gear", "open", "close");

	if (player_said("PULL CORD", "generator set"))
		_G(player).resetWalk();
	if (player_said("LIGHTER", " ") || player_said("LIT LIGHTER", " "))
		_G(player).resetWalk();
	if (takeFlag && player_said("LIGHTER") && inv_object_is_here("LIGHTER"))
		_G(player).resetWalk();
	if (useFlag && player_said("PULL CORD") && inv_object_is_here("PULL CORD"))
		_G(player).resetWalk();
	if (useFlag && player_said("pull cord ") && _G(flags)[V189] == 1)
		intr_freshen_sentence(65);
	if (useFlag && player_said("pull cord  ") && _G(flags)[V189] == 2)
		intr_freshen_sentence(65);
	if (useFlag && player_said("WHALE BONE HORN") && _G(flags)[V203] == 8) {
		digi_stop(3);
		_val5 = 1;
	}

	if (_val2) {
		_val3 = 0;
		if (player_said("exit")) {
			player_set_commands_allowed(false);
		} else {
			_G(player).resetWalk();
			Common::strcpy_s(_G(player).verb, "kill rip");
		}
	}
}

void Room604::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");
	bool useFlag = player_said_any("push", "pull", "gear", "open", "close");

	if (useFlag && player_said("WHALE BONE HORN") && useWhaleBoneHorn()) {
		// No implementation
	} else if (player_said("kill rip")) {
		killRipley();
	} else if (takeFlag && player_said("PULL CORD") && takePullCord()) {
		// No implementation
	} else if (player_said("PULL CORD", "generator set")) {
		switch (_G(kernel).trigger) {
		case -1:
			ws_walk(362, 316, nullptr, 1, 1);
			break;
		case 1:
			player_set_commands_allowed(false);
			setGlobals1(_ripLowReach1, 1, 10, 10, 10);
			sendWSMessage_110000(2);
			break;
		case 2:
			_pullCord1 = series_show("604 genset pull cord", 0xd00, 16);
			hotspot_set_active("PULL CORD", true);
			inv_move_object("PULL CORD", 604);
			sendWSMessage_140000(5);
			break;
		case 5:
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (player_said("SPARK PLUG TOOL", "GENERATOR SET")) {
		digi_play("com118", 1, 255, -1, 997);
	} else if (player_said("SPARK PLUG TOOL", "GAS TANK")) {
		digi_play("com011", 1, 255, -1, 997);
	} else if (takeFlag && player_said("SPARK PLUG TOOL") && takeSparkPlugTool()) {
		// No implementation
	} else if (useFlag && player_said("generator set") &&
			!inv_object_is_here("PULL CORD")) {
		digi_play("604r43", 1);
	} else if (useFlag && player_said("generator set") &&
			inv_object_is_here("PULL CORD") && _G(flags)[V189]) {
		digi_play("com011", 1);
	} else if (useFlag && player_said("PULL CORD") &&
			!inv_object_is_here("PULL CORD")) {
		digi_play("com011", 1);
	} else if (useFlag && player_said("PULL CORD") &&
			inv_object_is_here("PULL CORD") && _G(flags)[V189]) {
		digi_play("com011", 1);
	} else if (_G(kernel).trigger == 1995) {
		digi_unload("genstart");

		if (_G(flags)[V203] == 8) {
			digi_play("genrun", 2);
		} else {
			digi_play_loop("genrun", 3);
		}
	} else if (useFlag && player_said("generator set") && _G(flags)[GLB_TEMP_5]) {
		digi_play("com115", 1, 255, -1, 997);
	} else if (useFlag && (
		(player_said("PULL CORD") && inv_object_is_here("PULL CORD")) ||
		(player_said("generator set") && inv_object_is_here("PULL CORD"))
	)) {
		if (_G(flags)[GLB_TEMP_5]) {
			digi_play("com115", 1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				ws_walk(357, 311, nullptr, 1, 1);
				break;
			case 1:
				if (_G(flags)[V203] == 8) {
					digi_stop(3);
					_val5 = 1;
				}

				player_set_commands_allowed(false);
				player_update_info();
				ws_hide_walker();
				digi_preload("genstart");
				digi_preload("genrun");

				if (_G(flags)[V203] == 8)
					digi_preload("genshut");

				_ripAction = series_load("RIP STARTS GENERATOR");
				series_play("RIP STARTS GENERATOR", 0x100, 0, 17, 5, 0, 100, 0, 0, 0, 17);
				_shadow = series_play("SAFARI SHADOW 1", 0x200, 0, -1, 600, -1, 100,
					_G(player_info).x, _G(player_info).y, 0, 0);
				break;

			case 2:
				ws_unhide_walker();
				series_unload(_ripAction);
				terminateMachineAndNull(_shadow);
				series_unload(_ripAction);

				if (_G(flags)[V203] == 8) {
					kernel_timing_trigger(180, 3);
				} else {
					player_set_commands_allowed(true);
					_G(flags)[GLB_TEMP_5] = 1;
				}
				break;

			case 3:
				terminateMachineAndNull(_shedDoor);
				_shedDoor = series_play("SEHD DOOR OPENS", 0x300, 16, 4);
				break;

			case 4:
				ws_walk(381, 329, nullptr, 5, 9);
				break;

			case 5:
				ws_hide_walker();
				_ripley = series_play("604 RIP GETS SHOT", 0, 16, 6, 6);
				break;

			case 6:
				kernel_timing_trigger(40, 7);
				break;

			case 7:
				disable_player_commands_and_fade_init(-1);
				interface_hide();
				digi_play("genshut", 2, 255, 8);
				break;

			case 8:
				kernel_timing_trigger(30, 9);
				break;

			case 9:
				digi_play("950_s14", 1, 255, 10);
				break;

			case 10:
				other_save_game_for_resurrection();
				_G(game).setRoom(413);
				break;

			case 17:
				series_play("RIP STARTS GENERATOR", 0x100, 0, 2, 5, 0,
					100, 0, 0, 18, 30);
				digi_play("genstart", 3, 255, 1995);
				break;

			default:
				break;
			}
		}
	} else if (takeFlag && player_said("gas tank cap")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			ws_hide_walker();
			digi_preload("604_s02");
			_ripAction = series_load("RIP TAKES OFF GAS CAP");
			player_update_info();
			_ripley = series_play("RIP TAKES OFF GAS CAP", 0x100, 0, 2, 6, 0, 100, 0, 0, 0, 13);
			_shadow = series_play("SAFARI SHADOW 1", _G(player_info).depth + 1,
				0, -1, 600, -1, _G(player_info).scale, _G(player_info).x, _G(player_info).y, 1, 1);
			break;
		case 2:
			_ripley = series_play("RIP TAKES OFF GAS CAP", 0x100, 0, 3, 6, 0, 100, 0, 0, 14, 36);
			digi_play("604_s02", 2);
			break;
		case 3:
			ws_unhide_walker();
			series_unload(_ripAction);
			digi_stop(2);
			digi_unload("604_s02");
			terminateMachineAndNull(_shadow);
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if ((takeFlag || useFlag) && player_said("power cable")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			ws_hide_walker();
			digi_preload("wirepull");
			_ripAction = series_load("RIP PULLS WIRE");
			player_update_info();
			_ripley = series_play("RIP PULLS WIRE", 0xf00, 0, 1, 6, 0, 100, 0, 0, 0, 12);
			_shadow = series_play("SAFARI SHADOW 1", 0xf00, 0, -1, 600, -1,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y, 1, 1);
			break;
		case 1:
			_ripley = series_play("RIP PULLS WIRE", 0xf00, 0, 3, 6, 0, 100, 0, 0, 13, 24);

			if (_G(flags)[V203] == 8) {
				digi_stop(3);
				_val5 = 1;
			}

			digi_play("wirepull", 2, 255, 2);
			break;
		case 2:
			digi_unload("wirepull");
			break;
		case 3:
			ws_unhide_walker();
			series_unload(_ripAction);
			terminateMachineAndNull(_shadow);

			if (_G(flags)[V203] == 8) {
				kernel_timing_trigger(30, 5);
			} else {
				player_set_commands_allowed(true);
			}
			break;
		case 5:
			_ripley = series_play("BAD GUYS LOOK TO WIRE", 0, 0, 6, 6);
			break;
		case 6:
			kernel_timing_trigger(30, 7);
			break;
		case 7:
			digi_play("604k01", 1);
			_val2 = 1;
			_val3 = 1;
			_val4 = 0;
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_timing_trigger(60, 666);
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (takeFlag && player_said("wire")) {
		takeWire();
	} else if (player_said("LIGHTER", " ") ||
			player_said("LIT LIGHTER", " ")) {
		if (_G(flags)[V203] == 8) {
			switch (_G(kernel).trigger) {
			case -1:
				ws_walk(331, 323, nullptr, 1, 10);
				break;
			case 1:
				player_set_commands_allowed(false);
				setGlobals1(_ripLowReach2, 1, 16, 16, 16);
				sendWSMessage_110000(2);

				if (player_said("LIGHTER"))
					kernel_timing_trigger(40, 3);
				break;
			}
		} else {
			digi_play("com118", 1, 255, -1, 997);
		}
	} else if (takeFlag && player_said("LIGHTER") && takeLighter()) {
		// No implementation
	} else if (player_said("pull cord", "plug") && pullCordPlug()) {
		// No implementation
	} else if (player_said("plug", "pull cord  ")) {
		switch (_G(kernel).trigger) {
		case -1:
			if (_G(flags)[V190])
				ws_walk(289, 312, nullptr, 1, 11);
			else
				digi_play("604r12", 1);
			break;

		case 1:
			player_set_commands_allowed(false);
			setGlobals1(_ripLowReach1, 1, 15, 16, 16, 0, 17, 24, 24, 24, 0, 16, 1, 1, 1);
			sendWSMessage_110000(2);
			break;

		case 2:
			sendWSMessage_120000(4);
			terminateMachineAndNull(_cord);
			series_play("604TWANG", 0x300, 0, 3, 5);
			break;

		case 3:
			_tiedWires = series_show("604 wires tied together", 0xd00, 16);
			break;

		case 4:
			_G(flags)[V189] = 3;
			hotspot_set_active("  ", true);
			hotspot_set_active("pull cord   ", true);
			hotspot_set_active("pull cord  ", false);
			hotspot_set_active("plug", false);
			hotspot_set_active("WIRE", false);
			hotspot_set_active("WIRE ", true);
			sendWSMessage_130000(5);
			break;

		case 5:
			sendWSMessage_150000(-1);
			kernel_load_variant("604lock1");
			digi_play("604r48", 1);
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
	} else if (player_said("SPARK PLUG TOOL", "plug") &&
			inv_player_has("SPARK PLUG TOOL")) {
		switch (_G(kernel).trigger) {
		case -1:
			if (_G(flags)[V190])
				digi_play("604r32a", 1);
			else
				kernel_timing_trigger(1, 1);
			break;
		case 1:
			player_set_commands_allowed(false);
			setGlobals1(_ripLowReach1, 1, 24, 24, 24, 0, 16, 1, 1, 1);
			sendWSMessage_110000(2);
			break;
		case 2:
			_G(flags)[V190] = 1;
			sendWSMessage_120000(5);
			digi_play("604r32", 1);
			break;
		case 5:
			sendWSMessage_150000(-1);
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (player_said("pull cord ", "wire") && inv_object_is_here("pull cord")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			player_update_info();
			ws_hide_walker();
			terminateMachineAndNull(_pullCord2);
			series_load("door wire be gone");

			_shedDoor = series_show("door wire be gone", 0xe00, 16);
			_ripAction = series_load("RIP TIES CORD AND WIRE TOGETHER");
			_ripley = series_play("RIP TIES CORD AND WIRE TOGETHER", 0x100, 0, 2, 6);
			_shadow = series_play("SAFARI SHADOW 3", 0x200, 128, -1, 600, -1,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y, 0, 0);
			break;
		case 2:
			ws_unhide_walker();
			series_unload(_ripAction);
			terminateMachineAndNull(_shadow);
			series_load("604 wires tied together");
			_tiedWires = series_show("604 wires tied together", 0xd00, 16);
			_G(flags)[V189] = 3;
			hotspot_set_active("  ", true);
			hotspot_set_active("PULL CORD ", false);
			hotspot_set_active("PULL CORD   ", true);
			hotspot_set_active("WIRE", false);
			hotspot_set_active("WIRE ", true);
			kernel_load_variant("604lock1");
			digi_play("604r48", 1);
			break;
		default:
			break;
		}
	} else if (player_said("pull cord", "wire") && inv_player_has("pull cord")) {
		if (_G(flags)[V203] == 8) {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				player_update_info();
				ws_hide_walker();
				series_load("door wire be gone");
				_shedDoor = series_show("door wire be gone", 0xe00, 16);
				_ripAction = series_load("RIP TIES CORDS 2");
				_ripley = series_play("RIP TIES CORDS 2", 0x100, 0, 2);
				_shadow = series_play("SAFARI SHADOW 3", 0x200, 128, -1, 600, -1);
				break;
			case 2:
				ws_unhide_walker();
				series_unload(_ripAction);
				terminateMachineAndNull(_shadow);
				series_load("604cord2");
				_cord = series_show("604cord2", 0xc00, 16);
				_G(flags)[V189] = 2;

				inv_move_object("PULL CORD", 604);
				hotspot_set_active("pull cord  ", true);
				player_set_commands_allowed(true);
				break;
			default:
				break;
			}
		} else {
			digi_play("com017", 1, 255, -1, 997);
		}
	} else if (player_said("exit")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			ws_hide_walker();
			_ripley = series_play("RIP CRAWLS THROUGH WINDOW", 0x100, 0, 1);
			break;
		case 1:
			if (_G(flags)[V189] == 3 && inv_object_is_here("LIGHTER") && _val2) {
				interface_hide();
				disable_player_commands_and_fade_init(3);
				_G(flags)[V191] = 1;
			} else {
				disable_player_commands_and_fade_init(2);
			}
			break;
		case 2:
			_G(game).setRoom(610);
			break;
		case 3:
			if (_val2 == 2)
				_G(flags)[V042] = 1;
			_G(game).setRoom(603);
			break;
		default:
			break;
		}
	} else if (player_said("left")) {
		if (_G(flags)[V203] == 4) {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				_ripMedReach2 = series_load("rip trek med reach pos3");
				setGlobals1(_ripMedReach2, 1, 10, 10, 10);
				sendWSMessage_110000(-1);
				disable_player_commands_and_fade_init(1);
				break;
			case 1:
				adv_kill_digi_between_rooms(false);

				if (_G(flags)[GLB_TEMP_5]) {
					digi_play_loop("genrun", 3);
				} else {
					digi_preload("950_s28");
					digi_play_loop("950_s28", 3);
				}
				break;
			default:
				break;
			}
		}
	} else if (lookFlag && player_said("LIGHTER") &&
			inv_object_is_here("LIGHTER")) {
		digi_play("604r44", 1);
	} else if (lookFlag && player_said_any("wire", "wire ")) {
		if (_G(flags)[V203] != 8)
			digi_play("604r01", 1);
		else if (_G(flags)[V189] == 2 || _G(flags)[V189] == 3)
			digi_play("604r38", 1);
		else
			digi_play("604r25", 1);
	} else if (lookFlag && player_said("generator set")) {
		digi_play(_G(flags)[V203] == 8 ? "604r26" : "604r02", 1);
	} else if (lookFlag && player_said("gas tank")) {
		if (_G(flags)[V189] == 1 || _G(flags)[V189] == 3)
			digi_play("604r36", 1);
		else
			digi_play("604r03", 1);
	} else if (lookFlag && player_said("gas tank cap")) {
		digi_play("604r04", 1);
	} else if (lookFlag && player_said("plug")) {
		digi_play("604r05", 1);
	} else if (lookFlag && player_said("spark plug tool") &&
			inv_object_is_here("SPARK PLUG TOOL")) {
		if (_G(kernel).trigger == 1) {
			digi_play("604r27a", 1);
			_G(flags)[V188] = 1;
			player_set_commands_allowed(true);
		} else if (_G(flags)[V188]) {
			digi_play("604r27", 1);
		} else {
			digi_play("604r27", 1);
			player_set_commands_allowed(false);
		}
	} else if (lookFlag && player_said("pull cord") &&
			inv_object_is_here("PULL CORD")) {
		digi_play("604r06", 1);
	} else if (lookFlag && player_said("pull cord ")) {
		digi_play("604r36", 1);
	} else if (lookFlag && player_said("pull cord  ")) {
		digi_play("604r38", 1);
	} else if (lookFlag && player_said("pull cord   ")) {
		digi_play("604r39", 1);
	} else if (lookFlag && player_said("power cable")) {
		digi_play("604r07", 1);
	} else if (lookFlag && player_said("door")) {
		digi_play("604r28", 1);
	} else if (lookFlag && player_said("door ")) {
		digi_play("604r09", 1);
	} else if (useFlag && player_said("window")) {
		digi_play(_G(flags)[V203] == 8 ? "604r30" : "604r47", 1);
	} else if (lookFlag && player_said_any("window", "window ")) {
		digi_play(_G(flags)[V203] == 8 ? "604r29" : "604r10", 1);
	} else if (takeFlag && player_said("generator set")) {
		digi_play(_G(flags)[V203] == 8 ? "604r31" : "604r11", 1);
	} else if (takeFlag && player_said("generator set") &&
			_G(flags)[V203] == 8) {
		digi_play("604r31", 1);
	} else if ((takeFlag || useFlag) && player_said("plug")) {
		digi_play(_G(flags)[V190] ? "604r32a" : "604r12", 1);
	} else if (useFlag && player_said_any("wire", "wire ")) {
		digi_play("604r15", 1);
	} else if (useFlag && player_said_any("window", "window ")) {
		digi_play(_G(flags)[V190] ? "604r30" : "604r46", 1);
	} else if (lookFlag && player_said(" ")) {
		if (_G(flags)[V203] != 8)
			digi_play("604r08", 1);
		else if (_G(flags)[V189] == 3 && inv_object_is_here("LIGHTER"))
			digi_play("604r41", 1);
		else if (_G(flags)[V189] == 1 || _G(flags)[V189] == 2 ||
			inv_object_is_here("LIGHTER"))
			digi_play("604r41a", 1);
		else if (_G(flags)[V189] == 3)
			digi_play("604r41a", 1);
		else
			digi_play("604r24", 1);
	} else {
		return;
	}

	_G(player).command_ready = false;
}

bool Room604::useWhaleBoneHorn() {
	switch (_G(kernel).trigger) {
	case 5:
		_ripley = series_play("BAD GUYS LOOK TO SHED", 0, 0, 6, 6);
		break;

	case 6:
		kernel_timing_trigger(30, 7);
		break;

	case 7:
		digi_play("604k01", 1);
		_val2 = 2;
		_val3 = 1;
		_val4 = 0;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(60, 666);
		player_set_commands_allowed(true);
		return true;

	default:
		break;
	}

	return false;
}

void Room604::killRipley() {
	switch (_G(kernel).trigger) {
	case -1:
	case 667:
		player_set_commands_allowed(false);
		digi_play("ftsteps", 2, 255, 555);
		break;

	case 1:
		if (_G(flags)[V189] == 3) {
			series_play("glass flowing out", 0x700, 0, 2, 5);

			if (inv_object_is_here("LIGHTER")) {
				kernel_timing_trigger(20, 30);
				break;
			}
		}

		ws_walk(381, 329, nullptr, 3, 9);
		break;

	case 2:
		series_play("gas flowing out", 0x700, 4, -1, 5, -1, 100, 0, 0, 14, 18);
		break;

	case 3:
		ws_hide_walker();
		_ripley = series_play("604 RIP GETS SHOT", 0, 16, 5);
		break;

	case 5:
		kernel_timing_trigger(40, 6);
		break;

	case 6:
		disable_player_commands_and_fade_init(7);
		interface_hide();
		break;

	case 7:
		kernel_timing_trigger(30, 8);
		break;

	case 8:
		if (_G(spleenSpraying))
			digi_play("950_t10", 1, 255, 10);
		else
			digi_play("950_s14", 1, 255, 9);
		break;

	case 9:
	case 32:
		other_save_game_for_resurrection();
		_G(game).setRoom(413);
		break;

	case 10:
		digi_play("16_03p02", 1, 255, 11, 701);
		break;

	case 11:
		kernel_timing_trigger(10, 13);
		break;

	case 13:
		digi_play("950_t11", 1);
		kernel_timing_trigger(20, 14);
		break;

	case 14:
		digi_play("950_t11", 2);
		kernel_timing_trigger(15, 15);
		break;

	case 15:
		digi_play("950_t11", 3);
		kernel_timing_trigger(20, 16);
		break;

	case 16:
		digi_play("950_t11", 1);
		kernel_timing_trigger(25, 17);
		break;

	case 17:
		digi_play("950_t11", 2);
		kernel_timing_trigger(15, 18);
		break;

	case 18:
		digi_play("950_t11", 3);
		kernel_timing_trigger(20, 19);
		break;

	case 19:
		digi_play("950_t11", 3);
		kernel_timing_trigger(18, 20);
		break;

	case 21:
		digi_play("950_t11", 3);
		kernel_timing_trigger(18, 22);
		break;

	case 22:
		kernel_timing_trigger(30, 23);
		break;

	case 23:
		digi_play("304_s10", 1, 255, 24);
		break;

	case 24:
		digi_play("304_s05", 2);
		kernel_timing_trigger(20, 25);
		break;

	case 25:
		digi_play("950_t12", 3, 255, 9);
		break;

	case 30:
		disable_player_commands_and_fade_init(31);
		interface_hide();
		break;

	case 31:
		digi_play("explode2", 2, 255, 32);
		break;

	case 555:
		terminateMachineAndNull(_shedDoor);
		digi_play("dooropn2", 3);

		switch (_G(flags)[V189]) {
		case 1:
			_shedDoor = series_play("SHED DOOR OPENS", 0x300, 16, 1, 11);
			break;
		case 2:
			terminateMachineAndNull(_doorWireGone);
			terminateMachineAndNull(_cord);
			_shedDoor = series_play("604 DOOR OPEN PLUG OUT", 0x300, 16, 1, 11);
			break;
		case 3:
			terminateMachineAndNull(_doorWireGone);
			terminateMachineAndNull(_tiedWires);
			_shedDoor = series_play("604 DOOR OPEN PLUG IN", 0x300, 0, 556, 5, 0, 100, 0, 0, 0, 1);
			break;
		default:
			break;
		}
		break;

	case 556:
		digi_play("GASPOUR", 2);
		_shedDoor = series_play("604 DOOR OPEN PLUG IN", 0x300, 16, 1, 11, 0, 100, 0, 0, 2, 5);
		break;

	default:
		break;
	}
}

bool Room604::takePullCord() {
	switch (_G(kernel).trigger) {
	case -1:
		if (inv_object_is_here("PULL CORD") && !_G(flags)[V189]) {
			player_set_commands_allowed(false);
			setGlobals1(_ripLowReach1, 1, 10, 10, 10);
			sendWSMessage_110000(2);
		}
		break;

	case 2:
		terminateMachineAndNull(_pullCord1);
		hotspot_set_active("PULL CORD", false);
		inv_give_to_player("PULL CORD");
		kernel_examine_inventory_object("ping pull cord", 5, 1, 312, 350, 3);
		break;

	case 3:
		sendWSMessage_140000(5);
		break;

	case 5:
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}

	return false;
}

bool Room604::takeSparkPlugTool() {
	switch (_G(kernel).trigger) {
	case -1:
		if (inv_object_is_here("SPARK PLUG TOOL")) {
			player_set_commands_allowed(false);
			setGlobals1(_ripLowReach1, 1, 10, 10, 10);
			sendWSMessage_110000(2);
			return true;
		}
		break;

	case 2:
		hotspot_set_active("SPARK PLUG TOOL", false);
		inv_give_to_player("SPARK PLUG TOOL");
		kernel_examine_inventory_object("ping spark plug tool",
			5, 1, 282, 247, 3);
		terminateMachineAndNull(_sparkPlugTool);
		return true;

	case 3:
		sendWSMessage_140000(5);
		return true;

	case 5:
		player_set_commands_allowed(true);
		return true;

	default:
		break;
	}

	return false;
}

void Room604::takeWire() {
	if (_G(flags)[V189] == 0 || _G(flags)[V189] == 1) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			player_update_info();
			ws_hide_walker();
			_ripAction = series_load("RIP PULLS DOOR WIRE");
			_doorWireGone = series_show("door wire be gone", 0xe00, 16);
			_ripley = series_play("RIP PULLS DOOR WIRE", 0x100, 0, 2);
			_shadow = series_play("SAFARI SHADOW 3", 0x200, 128, -1, 600, -1,
				_G(player_info).scale, _G(player_info).x, _G(player_info).y, 0, 0);
			break;

		case 2:
			ws_unhide_walker();
			series_unload(_ripAction);
			terminateMachineAndNull(_shadow);
			terminateMachineAndNull(_doorWireGone);
			player_set_commands_allowed(true);
			digi_play("604r14", 1);
			break;

		default:
			break;
		}
	} else {
		digi_play("604r14", 1);
	}
}

bool Room604::takeLighter() {
	switch (_G(kernel).trigger) {
	case -1:
		if (!inv_object_is_here("LIGHTER"))
			return false;

		ws_walk(331, 323, nullptr, 1, 10);
		break;

	case 1:
		player_set_commands_allowed(false);
		setGlobals1(_ripLowReach2, 1, 16, 16, 16);
		sendWSMessage_110000(2);
		break;

	case 2:
		terminateMachineAndNull(_flame);
		hotspot_set_active("LIGHTER", false);
		inv_give_to_player("LIGHTER");
		sendWSMessage_140000(5);
		break;

	case 5:
		player_set_commands_allowed(true);
		break;

	default:
		return false;
	}

	return true;
}

bool Room604::pullCordPlug() {
	switch (_G(kernel).trigger) {
	case -1:
		if (_G(flags)[V190]) {
			if (!inv_player_has("PULL CORD"))
				return false;
			ws_walk(289, 312, nullptr, 1, 11);
		} else {
			digi_play("604r12", 1);
		}
		break;

	case 1:
		player_set_commands_allowed(false);
		setGlobals1(_ripLowReach1, 1, 24, 24, 24, 0, 16, 1, 1, 1);
		sendWSMessage_110000(2);
		break;

	case 2:
		series_load("one frame pull cord");
		_pullCord2 = series_show("ONE FRAME PULL CORD", 0xd00, 16);
		_G(flags)[V189] = 1;
		inv_move_object("PULL CORD", 604);
		hotspot_set_active("pull cord ", true);
		hotspot_set_active("plug", false);
		sendWSMessage_120000(5);
		break;

	case 5:
		sendWSMessage_150000(-1);
		break;

	default:
		return false;
	}

	return true;
}

void Room604::daemon1() {
	if (_val5)
		return;

	static const char *DIGI[3] = { "610_s03a", "610_s03b", "610_s03" };

	switch (_G(kernel).trigger) {
	case 126:
		digi_play(DIGI[imath_ranged_rand(0, 2)], 3, 130);
		kernel_timing_trigger(300, 135);
		break;

	case 135:
		if (++_val1 >= 8)
			_val1 = 1;

		digi_play(Common::String::format("610k%.2d", _val1 + 6).c_str(), 3, 130, 136);
		break;

	case 136:
		kernel_timing_trigger(300, 135);
		break;

	case 137:
		kernel_timing_trigger(60, 138);
		break;

	case 138:
		if (imath_ranged_rand(1, 2) == 1)
			digi_play("610_s02", 3, 130);
		else
			digi_play("610_s02a", 3, 130);

		kernel_timing_trigger(20, 126);
		break;

	default:
		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
