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

#include "m4/riddle/rooms/section5/room510.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"
#include "m4/adv_r/adv_file.h"
#include "m4/adv_r/other.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room510::init() {
	digi_preload("509_s01");
	_val1 = 0;

	static const char *HOTSPOTS[] = {
		"ROPE ", "GREEN VINE ", "BROWN VINE ", "VINES ",
		"ROPE", "GREEN VINE", "BROWN VINE", "VINES"
	};
	for (int i = 0; i < 8; ++i)
		hotspot_set_active(HOTSPOTS[i], false);

	switch (_G(flags)[V169]) {
	case 0:
		_statue = series_play("510 STATUE", 0xa00, 16, -1, 5);
		hotspot_set_active("ALTAR POST ", false);
		kernel_load_variant("510lock0");
		break;

	case 1:
		_statue = series_play("510 STATUE LAYED DOWN", 0xa00, 16, -1, 5);
		hotspot_set_active("ALTAR POST", false);
		kernel_load_variant("510lock1");
		break;

	case 2:
		_statue = series_play(" 510 STATUE LAYED DOWN", 0xa00, 16, -1, 5);

		if (inv_object_is_here("ROPE")) {
			hotspot_set_active("ROPE ", true);
			_rope = series_play("510 STATUE AND ROPE", 0xf00, 16, -1, 5, 0, 100, 0, 0, 38, 38);
		}

		if (inv_object_is_here("BROWN VINE")) {
			hotspot_set_active("BROWN VINE ", true);
			_rope = series_play("510 STATUE AND ANY VINE", 0xf00, 16, -1, 5, 0, 100, 0, 0, 38, 38);
		}

		if (inv_object_is_here("GREEN VINE")) {
			hotspot_set_active("GREEN VINE ", true);
			_rope = series_play("510 STATUE AND ANY VINE", 0xf00, 16, -1, 5, 0, 100, 0, 0, 38, 38);
		}

		if (inv_object_is_here("VINES")) {
			hotspot_set_active("VINES  ", true);
			_rope = series_play("510 STATUE AND COMBO VINES", 0xf00, 16, -1, 5, 0, 100, 0, 0, 38, 38);
		}

		hotspot_set_active("ALTAR POST", false);
		kernel_load_variant("510lock1");
		break;

	default:
		break;
	}

	if (inv_object_is_here("WOODEN LADDER"))
		_ladder = series_play("510 LADDER", 0xf00, 16, -1, 5);
	else
		hotspot_set_active("WOODEN LADDER", false);

	if (_G(flags)[V169] != 2) {
		if (inv_object_is_here("ROPE")) {
			hotspot_set_active("ROPE", true);
			_rope = series_play("ROPE UNWINDS DOWN THE TOWER", 0x100, 16, -1, 5, 0, 100, 0, 0, 35, 35);
		}

		if (inv_object_is_here("GREEN VINE")) {
			hotspot_set_active("GREEN VINE", true);
			_rope = series_play("510 SHORTER VINE UNWINDS", 0x100, 16, -1, 5, 0, 100, 0, 0, 35, 35);
		}

		if (inv_object_is_here("BROWN VINE")) {
			hotspot_set_active("BROWN VINE", true);
			_rope = series_play("510 LONGER VINE UNWINDS", 0x100, 16, -1, 5, 0, 100, 0, 0, 35, 35);
		}

		if (inv_object_is_here("VINES")) {
			hotspot_set_active("VINES", true);
			_rope = series_play("ROPE UNWINDS DOWN THE TOWER", 0x100, 16, -1, 5, 0, 100, 0, 0, 35, 35);
		}
	}

	hotspot_set_active("SKULL", false);

	if (inv_object_is_here("CRYSTAL SKULL"))
		_skull = series_play("SKULL SPARKLES", 0x100, 16, -1, 15);

	if (_G(flags)[V170] == 1 && inv_object_is_here("CRYSTAL SKULL")) {
		hotspot_set_active("SKULL", true);
		hotspot_set_active("NICHE", false);
		_G(flags)[V292] = 0;
	}

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		if (_G(flags)[V170]) {
			ws_hide_walker();
			_steps = series_play("RIP DANGLES FROM ROPE", 0x100, 16, -1, 5);
		}

		kernel_timing_trigger(60, 999);

	} else {
		player_set_commands_allowed(false);
		_stepsSeries = series_load("510 UP STEPS");
		_steps = series_play("510 UP STEPS", 0xf00, 0, 110, 5);
		ws_demand_location(409, 86, 11);
		ws_hide_walker();
	}

	digi_play_loop("509_s01", 3, 127);
}

void Room510::daemon() {
	switch (_G(kernel).trigger) {
	case 110:
		terminateMachineAndNull(_steps);
		series_unload(_stepsSeries);
		ws_walk(407, 97, nullptr, 999, 8);
		break;

	case 999:
		player_set_commands_allowed(true);

		if (!_G(player).been_here_before)
			digi_play("510R01", 1);
		break;

	default:
		break;
	}
}

void Room510::pre_parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");
	bool useFlag = player_said("gear");

	if (!useFlag && !lookFlag && !takeFlag)
		return;
	if (!_G(flags)[V170])
		return;

	if (takeFlag && player_said("SKULL"))
		return;
	if (useFlag && player_said("ROPE"))
		return;

	if (lookFlag && player_said("NICHE")) {
		digi_play("com127", 1);
		intr_cancel_sentence();
		return;
	}

	if (lookFlag && player_said("ROPE"))
		return;

	if (lookFlag && player_said("WOODEN LADDER")) {
		digi_play("com107", 1);
	} else {
		if (lookFlag && player_said("TOWER"))
			return;
		if (lookFlag && player_said("SKULL"))
			return;
		if (!lookFlag && !takeFlag && !useFlag)
			return;

		if (player_said_any("ALTAR POST", "ALTAR POST ", "TROUGH", "STAIRS")) {
			switch (imath_ranged_rand(1, 5)) {
			case 1:
				digi_play("301r23", 1, 255, -1, 301);
				break;
			case 2:
				digi_play("301r25", 1, 255, -1, 301);
				break;
			case 3:
				digi_play("301r26", 1, 255, -1, 301);
				break;
			case 4:
				digi_play("301r35", 1, 255, -1, 301);
				break;
			case 5:
				digi_play("301r36", 1, 255, -1, 301);
				break;
			default:
				break;
			}
		}
	}
}

#define HERE(ITEM) (player_said(ITEM) && inv_object_is_here(ITEM))
#define HAS(ITEM) (player_said(ITEM) && inv_player_has(ITEM))

void Room510::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");
	bool useFlag = player_said("gear");

	if (lookFlag && player_said(" ")) {
		digi_play("510R02", 1);
	} else if (lookFlag && player_said_any("ALTAR POST", "ALTAR POST ", "ALTAR")) {
		digi_play(_G(flags)[V165] == 1 ? "510R03A" : "510R03", 1);
	} else if (lookFlag && player_said("TROUGH")) {
		if (_G(flags)[V166] == 1) {
			digi_play("510R04A", 1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				digi_play("510R04", 1);
				break;
			case 1:
				digi_play("510_s01", 1);
				kernel_timing_trigger(300, 2);
				break;
			case 2:
				digi_play("510r05", 2);
				_G(flags)[V166] = 1;
				break;
			case 7:
				digi_play("510R04A", 1);
				break;
			default:
				break;
			}
		}
	} else if (lookFlag && player_said("NICHE")) {
		switch (_G(kernel).trigger) {
		case 1:
			digi_play("510r06", 1, 255, 2);
			break;
		case -1:
			if (_G(flags)[V170]) {
				digi_play("com127", 1);
			} else {
				player_set_commands_allowed(false);
				_ripLooksDown = series_load("RIP LOOKS DOWN POS4");
				kernel_timing_trigger(1, 5);
			}
			break;
		case 2:
			sendWSMessage_140000(3);
			break;
		case 3:
			kernel_timing_trigger(1, 4);
			break;
		case 4:
			series_unload(_ripLooksDown);
			break;
		case 5:
			setGlobals1(_ripLooksDown, 1, 12, 12, 12);
			sendWSMessage_110000(1);
			break;
		case 7:
			if (_G(flags)[V170])
				digi_play("com127", 1);
			break;
		default:
			break;
		}
	} else if (lookFlag && player_said("SKULL")) {
		digi_play("510R11", 1);
	} else if (lookFlag && player_said("DOME")) {
		digi_play("510R20", 1);
	} else if (lookFlag && HERE("WOODEN LADDER")) {
		digi_play("com107", 1);
	} else if (lookFlag && HERE("ROPE")) {
		digi_play("COM132", 1, 255, -1, 997);
	} else if (lookFlag && HERE("GREEN VINE")) {
		digi_play("COM133", 1, 255, -1, 997);
	} else if (lookFlag && HERE("BROWN VINE")) {
		digi_play("COM133", 1, 255, -1, 997);
	} else if (lookFlag && HERE("VINES")) {
		digi_play("COM134", 1, 255, -1, 997);
	} else if (lookFlag && player_said("STAIRS")) {
		digi_play("509r06", 1, 255, -1, 509);
	} else if (takeFlag && player_said_any("ALTAR POST", "ALTAR POST ")) {
		digi_play("510R09", 1);
	} else if (takeFlag && player_said("SKULL")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			terminateMachineAndNull(_steps);
			terminateMachineAndNull(_rope);
			_ripDangles = series_play("RIP DANGLES FROM ROPE", 0x100, 16, 1, 5, 0, 100, 0, 0, 0, 41);
			break;
		case 1:
			hotspot_set_active("SKULL", false);
			hotspot_set_active("NICHE", true);

			if (inv_object_is_here("CRYSTAL SKULL")) {
				kernel_examine_inventory_object("PING CRYSTAL SKULL", 5, 1, 330, 280, 2, 0, 2);
				inv_give_to_player("CRYSTAL SKULL");
				terminateMachineAndNull(_skull);
			}

			digi_preload("510_s07");
			break;
		case 2:
			terminateMachineAndNull(_ripDangles);
			_ripDangles = series_play("RIP DANGLES FROM ROPE", 0x100, 16, 3, 5, 0, 100, 0, 0, 42, 58);
			break;
		case 3:
			terminateMachineAndNull(_ripDangles);
			terminateMachineAndNull(_statue);
			terminateMachineAndNull(_ladder);

			_ripStartsDownWall = series_load("Rip starts down wall");
			_pu03 = series_stream("510pu03", 5, 0, 7);
			series_stream_break_on_frame(_pu03, 58, 4);
			break;
		case 4:
			_rope = series_play("ROPE UNWINDS DOWN THE TOWER", 0x100, 16, -1, 5, 0, 100, 0, 0, 35, 35);
			_statue = series_play(" 510 STATUE LAYED DOWN", 0xa00, 16, -1, 5);
			_ladder = series_play("510 LADDER", 0xf00, 16, -1, 5);
			_steps = series_play("Rip starts down wall", 0x200, 18, 5, 5, 0, 100, 0, 0, 0, 34);
			digi_play("510_s07", 2);
			break;
		case 5:
			digi_unload("510_s07");
			terminateMachineAndNull(_steps);
			_G(flags)[V170] = 0;
			_G(flags)[V292] = 1;

			if (inv_object_is_here("CRYSTAL SKULL")) {
				hotspot_set_active("SKULL", false);
				hotspot_set_active("NICHE", true);
			}

			ws_unhide_walker();
			ws_demand_location(425, 128, 11);
			kernel_timing_trigger(1, 6);
			break;
		case 6:
			_G(flags)[V292] = 1;
			series_unload(_ripStartsDownWall);
			break;
		case 7:
			terminateMachineAndNull(_pu03);
			break;
		default:
			break;
		}
	} else if (takeFlag && player_said_any("VINES ", "ROPE ", "GREEN VINE ", "BROWN VINE ")) {
		if (!parserSub())
			return;
	} else if (takeFlag && _G(flags)[V170] != 1 &&
		player_said_any("VINES", "ROPE", "GREEN VINE", "BROWN VINE") && takeVinesRope()) {
		// No implementation
	} else if (takeFlag && player_said("WOODEN LADDER")) {
		switch (_G(kernel).trigger) {
		case -1:
			if (inv_object_is_here("WOODEN LADDER") && !_G(flags)[V169]) {
				ws_walk(395, 121, nullptr, 1, 5);
			}
			break;
		case 1:
			player_set_commands_allowed(false);
			if (!_G(flags)[V169]) {
				if (!inv_object_is_here("VINES") && !inv_object_is_here("ROPE") &&
					!inv_object_is_here("GREEN VINE") && !inv_object_is_here("BROWN VINE") &&
					!_G(flags)[V170]) {
					kernel_timing_trigger(10, 8);
				} else {
					kernel_timing_trigger(10, 2);
				}
			} else {
				return;
			}
			break;
		case 2:
			ws_hide_walker();
			_steps = series_play("Rip ties rope to ladder", 0x100, 16, 3, 5, 0, 100, 0, 0, 0, 23);
			break;
		case 3:
			terminateMachineAndNull(_steps);
			_steps = series_play("Rip ties rope to ladder", 0x100, 16, -1, 5, 0, 100, 0, 0, 23, 32);

			if (inv_object_is_here("GREEN VINE")) {
				hotspot_set_active("GREEN VINE", false);
				terminateMachineAndNull(_rope);
				_rope = series_play("510 SHORTER VINE UNWINDS", 0x100, 18, 4, 5);
				digi_play("510_s06", 1);
			}
			if (inv_object_is_here("BROWN VINE")) {
				hotspot_set_active("BROWN VINE", false);
				terminateMachineAndNull(_rope);
				_rope = series_play("510 LONGER VINE UNWINDS", 0x100, 18, 4, 5);
				digi_play("510_s06", 1);
			}
			if (inv_object_is_here("ROPE")) {
				hotspot_set_active("ROPE", false);
				terminateMachineAndNull(_rope);
				_rope = series_play("ROPE UNWINDS DOWN THE TOWER", 0x100, 18, 4, 5);
				digi_play("510_s06", 1);
			}
			if (inv_object_is_here("VINES")) {
				hotspot_set_active("VINES", false);
				terminateMachineAndNull(_rope);
				_rope = series_play("ROPE UNWINDS DOWN THE TOWER", 0x100, 18, 4, 5);
				digi_play("510_s06", 1);
			}
			break;
		case 4:
			terminateMachineAndNull(_steps);
			terminateMachineAndNull(_rope);
			_steps = series_play("Rip ties rope to ladder", 0x100, 18, 5, 5, 0, 100, 0, 0, 0, 23);
			break;
		case 5:
			terminateMachineAndNull(_steps);
			ws_unhide_walker();

			if (inv_object_is_here("GREEN VINE")) {
				inv_give_to_player("GREEN VINE");
				kernel_examine_inventory_object("PING GREEN VINE", 5, 1, 325, 200, 8);
			}
			if (inv_object_is_here("BROWN VINE")) {
				inv_give_to_player("BROWN VINE");
				kernel_examine_inventory_object("PING BROWN VINE", 5, 1, 325, 200, 8);
			}
			if (inv_object_is_here("ROPE")) {
				inv_give_to_player("ROPE");
				kernel_examine_inventory_object("PING ROPE", 5, 1, 325, 200, 8);
			}
			if (inv_object_is_here("VINES")) {
				inv_give_to_player("VINES");
				kernel_examine_inventory_object("PING VINES", 5, 1, 325, 200, 8);
			}
			break;
		case 7:
			return;
		case 8:
			kernel_timing_trigger(1, 12);
			break;
		case 9:
			terminateMachineAndNull(_ladder);
			inv_give_to_player("WOODEN LADDER");
			kernel_examine_inventory_object("PING WOODEN LADDER", 5, 1, 300, 80, 10, nullptr, 10);
			break;
		case 10:
			sendWSMessage_140000(11);
			break;
		case 11:
			series_unload(_ripLowReach);
			player_set_commands_allowed(true);
			break;
		case 12:
			hotspot_set_active("WOODEN LADDER", false);
			_ripLowReach = series_load("Rip trek low reacher pos5");
			setGlobals1(_ripLowReach, 1, 26, 26, 26);
			sendWSMessage_110000(9);
			break;
		default:
			break;
		}
	} else if (useFlag && player_said("ROPE ")) {
		digi_play("com112a", 1);
	} else if (useFlag && player_said_any("GREEN VINE ", "BROWN VINE ")) {
		digi_play("com113", 1);
	} else if (useFlag && player_said("VINES ")) {
		useVines();
	} else if (useFlag && player_said_any("ALTAR POST", "ALTAR POST ")) {
		switch (_G(kernel).trigger) {
		case -1:
		case 4:
		case 7:
		case 11:
		case 666:
			if (!parserSub())
				return;
			break;

		default:
			return;
		}
	} else if (player_said("WOODEN LADDER", "ALTAR") && inv_player_has("WOODEN LADDER")) {
		if (_G(flags)[V169])
			return;

		switch (_G(kernel).trigger) {
		case -1:
			ws_walk(395, 121, nullptr, 1, 7);
			break;
		case 1:
			player_set_commands_allowed(false);
			_ripLowReach2 = series_load("Rip trek low reacher pos5");
			setGlobals1(_ripLowReach2, 1, 26, 26, 26);
			sendWSMessage_110000(2);
			break;
		case 2:
			digi_play("510_s03", 1);
			hotspot_set_active("WOODEN LADDER", true);
			sendWSMessage_140000(3);
			_ladder = series_play("510 LADDER", 0xf00, 16, -1, 5);
			break;
		case 3:
			kernel_timing_trigger(1, 4);
			break;
		case 4:
			series_unload(_ripLowReach2);
			inv_put_thing_in("WOODEN LADDER", 510);
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (inv_object_is_here("WOODEN LADDER") && (
		player_said("ROPE", "WOODEN LADDER") ||
		player_said("VINES", "WOODEN LADDER") ||
		player_said("GREEN VINE", "WOODEN LADDER") ||
		player_said("BROWN VINE", "WOODEN LADDER")
		)) {
		woodenLadder();
	} else if ((player_said("LADDER/ROPE", "ALTAR") && inv_player_has("LADDER/ROPE")) ||
		(player_said("LADDER/VINES", "ALTAR") && inv_player_has("LADDER/VINES")) ||
		(player_said("LADDER/GREEN VINE", "ALTAR") && inv_player_has("LADDER/GREEN VINE")) ||
		(player_said("LADDER/BROWN VINE", "ALTAR") && inv_player_has("LADDER/BROWN VINE"))
		) {
		if (!_G(flags)[V169])
			altar();
	} else if (player_said("GREEN VINE", "BROWN VINE") && inv_player_has("GREEN VINE") &&
		inv_player_has("BROWN VINE")) {
		inv_put_thing_in("GREEN VINE", NOWHERE);
		inv_put_thing_in("BROWN VINE", NOWHERE);
		inv_give_to_player("VINES");
	} else if (useFlag && HERE("ROPE")) {
		if (_G(flags)[V169] == 0) {
			digi_play("510R10", 1);
		} else if (_G(flags)[V170] == 1) {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				terminateMachineAndNull(_steps);
				terminateMachineAndNull(_ladder);
				terminateMachineAndNull(_rope);
				terminateMachineAndNull(_statue);

				_pu03 = series_stream("510pu03", 5, 0, 3);
				series_stream_break_on_frame(_pu03, 58, 1);
				break;
			case 1:
				_rope = series_play("ROPE UNWINDS DOWN THE TOWER", 0x100, 16, -1, 5, 0, 100, 0, 0, 35, 35);
				_statue = series_play(" 510 STATUE LAYED DOWN", 0xa00, 16, -1, 5);
				_ladder = series_play("510 LADDER", 0xf00, 16, -1, 5);
				_steps = series_play("Rip starts down wall", 0x200, 18, 2, 5, 0, 100, 0, 0, 0, 34);
				break;
			case 2:
				terminateMachineAndNull(_steps);

				if (inv_object_is_here("CRYSTAL SKULL")) {
					hotspot_set_active("SKULL", false);
					hotspot_set_active("NICHE", true);
				}

				ws_unhide_walker();
				ws_demand_location(425, 128, 11);
				_G(flags)[V170] = 0;
				player_set_commands_allowed(true);
				break;
			case 3:
				terminateMachineAndNull(_pu03);
				break;
			default:
				break;
			}
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				ws_walk(425, 128, nullptr, 1, 11);
				break;
			case 1:
				player_set_commands_allowed(false);
				ws_hide_walker();
				_steps = series_play("Rip starts down wall", 0x200, 16, 2, 5, 0, 100, 0, 0, 0, 34);
				digi_preload("510_s07");
				break;
			case 2:
				terminateMachineAndNull(_steps);
				terminateMachineAndNull(_ladder);
				terminateMachineAndNull(_rope);
				terminateMachineAndNull(_statue);
				_dangling = series_load("RIP DANGLES FROM ROPE");
				digi_play("510_s07", 1);
				series_stream("510 RIP DOWN", 5, 0, 3);
				break;
			case 3:
				digi_unload("510_s07");
				_rope = series_play("ROPE UNWINDS DOWN THE TOWER", 0x100, 16, -1, 5, 0, 100, 0, 0, 35, 35);
				_statue = series_play(" 510 STATUE LAYED DOWN", 0xa00, 16, -1, 5);
				_ladder = series_play("510 LADDER", 0xf00, 16, -1, 5);
				_steps = series_play("RIP DANGLES FROM ROPE", 0x100, 16, -1, 5, 0, 100, 0, 0, 0, 0);
				kernel_timing_trigger(1, 4);
				break;
			case 4:
				_G(flags)[V170] = 1;
				_G(flags)[V292] = 0;

				if (inv_object_is_here("CRYSTAL SKULL")) {
					hotspot_set_active("SKULL", true);
					hotspot_set_active("NICHE", false);
				}

				player_set_commands_allowed(true);
				break;
			default:
				break;
			}
		}
	} else if (useFlag && (HERE("GREEN VINE") || HERE("BROWN VINE"))) {
		digi_play(_G(flags)[V169] ? "com113" : "510R10", 1);
	} else if ((player_said("ROPE", "ALTAR POST") && inv_player_has("ROPE")) ||
		(player_said("GREEN VINE", "ALTAR POST") && inv_player_has("GREEN VINE")) ||
		(player_said("BROWN VINE", "ALTAR POST") && inv_player_has("BROWN VINE")) ||
		(player_said("VINES", "ALTAR POST") && inv_player_has("VINES"))) {
		if (_G(flags)[V169])
			return;

		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_ripLowReach4 = series_load("RIP TREK LOW REACHER POS1");
			kernel_timing_trigger(1, 1);
			break;
		case 1:
			setGlobals1(_ripLowReach4, 1, 28, 28, 28);
			sendWSMessage_110000(2);
			break;
		case 2:
			if (player_said("ROPE"))
				_rope = series_play("510 STATUE AND ROPE", 0xf00, 16, 3, 5);
			if (player_said("GREEN VINE") || player_said("BROWN VINE"))
				_rope = series_play("510 STATUE AND ANY VINE", 0xf00, 16, 3, 5);
			if (player_said("VINES"))
				_rope = series_play("510 STATUE AND COMBO VINES", 0xf00, 16, 3, 5);
			break;
		case 3:
			sendWSMessage_140000(4);
			break;
		case 4:
			series_unload(_ripLowReach4);
			kernel_timing_trigger(150, 8);
			ws_hide_walker();
			terminateMachineAndNull(_statue);
			_statue = series_play("510 RIP LOWERS STATUE", 0xa00, 16, 5, 5);
			break;
		case 5:
			ws_unhide_walker();
			terminateMachineAndNull(_statue);
			_statue = series_play(" 510 STATUE LAYED DOWN", 0xa00, 16, -1, 5);
			kernel_timing_trigger(1, 7);
			hotspot_set_active("ALTAR POST", false);
			hotspot_set_active("ALTAR POST ", true);
			kernel_load_variant("510lock1");
			break;
		case 7:
			if (player_said("ROPE")) {
				inv_put_thing_in("ROPE", 510);
				hotspot_set_active("ROPE ", true);
			}

			if (player_said("GREEN VINE")) {
				inv_put_thing_in("GREEN VINE", 510);
				hotspot_set_active("GREEN VINE ", true);
			}

			if (player_said("BROWN VINE")) {
				inv_put_thing_in("BROWN VINE", 510);
				hotspot_set_active("BROWN VINE ", true);
			}

			if (player_said("VINES")) {
				inv_put_thing_in("VINES", 510);
				hotspot_set_active("VINES ", true);
			}

			_G(flags)[V169] = 2;
			player_set_commands_allowed(true);
			break;
		case 8:
			digi_play("510_s02", 2);
			break;
		default:
			break;
		}
	} else if (useFlag && HAS("VINES")) {
		inv_give_to_player("GREEN VINE");
		inv_give_to_player("BROWN VINE");
		inv_put_thing_in("VINES", NOWHERE);
	} else if (useFlag && HAS("LADDER/ROPE")) {
		inv_give_to_player("ROPE");
		inv_give_to_player("WOODEN LADDER");
		inv_put_thing_in("LADDER/ROPE", NOWHERE);
	} else if (player_said("ROPE", "WOODEN LADDER") && inv_player_has("ROPE") &&
		inv_player_has("WOODEN LADDER")) {
		inv_put_thing_in("ROPE", NOWHERE);
		inv_put_thing_in("WOODEN LADDER", NOWHERE);
		inv_give_to_player("LADDER/ROPE");
	} else if (useFlag && HAS("LADDER/GREEN VINE")) {
		inv_give_to_player("GREEN VINE");
		inv_give_to_player("WOODEN LADDER");
		inv_put_thing_in("LADDER/GREEN VINE", NOWHERE);
	} else if (player_said("GREEN VINE", "WOODEN LADDER") && inv_player_has("GREEN VINE") &&
		inv_player_has("WOODEN LADDER")) {
		inv_put_thing_in("GREEN VINE", NOWHERE);
		inv_put_thing_in("WOODEN LADDER", NOWHERE);
		inv_give_to_player("LADDER/GREEN VINE");
	} else if (useFlag && HAS("LADDER/BROWN VINE")) {
		inv_give_to_player("BROWN VINE");
		inv_give_to_player("WOODEN LADDER");
		inv_put_thing_in("LADDER/BROWN VINE", NOWHERE);
	} else if (player_said("BROWN VINE", "WOODEN LADDER") && inv_player_has("BROWN VINE") &&
		inv_player_has("WOODEN LADDER")) {
		inv_put_thing_in("BROWN VINE", NOWHERE);
		inv_put_thing_in("WOODEN LADDER", NOWHERE);
		inv_give_to_player("LADDER/BROWN VINE");
	} else if (useFlag && HAS("VINES LADDER")) {
		inv_give_to_player("VINES");
		inv_give_to_player("WOODEN LADDER");
		inv_put_thing_in("LADDER/VINES", NOWHERE);
	} else if (player_said("VINES", "WOODEN LADDER") && inv_player_has("VINES") &&
		inv_player_has("WOODEN LADDER")) {
		inv_put_thing_in("VINES", NOWHERE);
		inv_put_thing_in("WOODEN LADDER", NOWHERE);
		inv_give_to_player("LADDER/VINES");
	} else if (player_said("journal") && !takeFlag && !lookFlag &&
		!inv_player_has(_G(player).noun)) {
		if (_G(flags)[kMocaMocheCartoon]) {
			digi_play("com029", 1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				ws_walk(407, 97, nullptr, 1, 10);
				break;
			case 1:
				player_set_commands_allowed(false);

				if (_flag1) {
					sendWSMessage_multi(0);
				} else {
					digi_play("com028", 1, 255, 100);
					_flag1 = true;
				}
				break;
			case 6:
				_G(flags)[V089] = 1;
				_G(flags)[kMocaMocheCartoon] = 1;
				break;
			case 100:
				_G(kernel).trigger = -1;
				sendWSMessage_multi(0);
				break;
			default:
				sendWSMessage_multi(0);
				break;
			}
		}
	} else if (player_said("Stairs") && !lookFlag && !takeFlag) {
		switch (_G(kernel).trigger) {
		case -1:
			kernel_timing_trigger(15, 1);
			break;
		case 1:
			player_set_commands_allowed(false);
			ws_hide_walker();
			_steps = series_play("510 RIP DOWN STEPS", 0x100, 16, 2, 5, 0, 100, 0, 0, 0, 28);
			break;
		case 2:
			terminateMachineAndNull(_steps);
			_steps = series_play("510 RIP DOWN STEPS", 0x100, 16, 2, 5, 0, 100, 0, 0, 29, 38);
			disable_player_commands_and_fade_init(3);
			break;
		case 3:
			_G(game).setRoom(509);
			terminateMachineAndNull(_steps);
			break;
		default:
			break;
		}
	} else {
		return;
	}

	_G(player).command_ready = false;
}

bool Room510::parserSub() {
	// TODO
	return false;
}

bool Room510::takeVinesRope() {
	static const char *ITEMS[4] = { "VINES", "ROPE", "GREEN VINE", "BROWN VINE" };

	switch (_G(kernel).trigger) {
	case -1:
		for (int i = 0; i < 4; ++i) {
			if (player_said(ITEMS[i]) && inv_player_has(ITEMS[i]))
				return true;
		}

		ws_walk(395, 121, nullptr, 1, 5);
		break;

	case 1:
		player_set_commands_allowed(false);

		if (!inv_player_has(_G(player).noun)) {
			ws_hide_walker();
			_steps = series_play("Rip ties rope to ladder", 0, 16, 2, 5, 0, 100, 0, 0, 0, 23);
		}
		break;

	case 2:
		terminateMachineAndNull(_steps);
		_steps = series_play("Rip ties rope to ladder", 0, 16, -1, 5, 0, 100, 0, 0, 23, 32);

		if (player_said("GREEN VINE")) {
			hotspot_set_active("GREEN VINE", false);
			terminateMachineAndNull(_rope);
			_rope = series_play("510 SHORTER VINE UNWINDS", 0x100, 18, 3, 5);
			digi_play("510_s06", 1);
		}
		if (player_said("BROWN VINE")) {
			hotspot_set_active("BROWN VINE", false);
			terminateMachineAndNull(_rope);
			_rope = series_play("510 LONGER VINE UNWINDS", 0x100, 18, 3, 5);
			digi_play("510_s06", 1);
		}
		if (player_said("ROPE")) {
			hotspot_set_active("ROPE", false);
			terminateMachineAndNull(_rope);
			_rope = series_play("ROPE UNWINDS DOWN THE TOWER", 0x100, 18, 3, 5);
			digi_play("510_s06", 1);
		}
		if (player_said("VINES")) {
			hotspot_set_active("VINES", false);
			terminateMachineAndNull(_rope);
			_rope = series_play("ROPE UNWINDS DOWN THE TOWER", 0x100, 18, 3, 5);
			digi_play("510_s06", 1);
		}
		break;

	case 3:
		terminateMachineAndNull(_rope);
		terminateMachineAndNull(_steps);
		_steps = series_play("Rip ties rope to ladder", 0, 18, 4, 5, 0, 100, 0, 0, 0, 23);
		break;

	case 4:
		if (player_said("GREEN VINE")) {
			inv_give_to_player("GREEN VINE");
			kernel_examine_inventory_object("PING GREEN VINE", 5, 1, 325, 200, 5, nullptr, 10);
		}
		if (player_said("BROWN VINE")) {
			inv_give_to_player("BROWN VINE");
			kernel_examine_inventory_object("PING BROWN VINE", 5, 1, 325, 200, 5, nullptr, 10);
		}
		if (player_said("VINES")) {
			inv_give_to_player("VINES");
			kernel_examine_inventory_object("PING VINES", 5, 1, 325, 200, 5, nullptr, 10);
		}
		if (player_said("ROPE")) {
			inv_give_to_player("ROPE");
			kernel_examine_inventory_object("PING ROPE", 5, 1, 325, 200, 5, nullptr, 10);
		}
		break;

	case 5:
		terminateMachineAndNull(_steps);
		ws_unhide_walker();
		player_set_commands_allowed(true);
		break;

	default:
		return false;
	}

	return true;
}

void Room510::useVines() {
	switch (_G(kernel).trigger) {
	case -1:
		other_save_game_for_resurrection();
		ws_walk(425, 128, nullptr, 1, 11);
		break;

	case 1:
		ws_hide_walker();
		player_set_commands_allowed(false);
		_steps = series_play("Rip starts down wall", 0x200, 16, 2, 5);
		break;

	case 2:
		kernel_timing_trigger(20, 3);
		terminateMachineAndNull(_steps);

		if (inv_object_is_here("WOODEN LADDER"))
			terminateMachineAndNull(_ladder);

		terminateMachineAndNull(_rope);
		terminateMachineAndNull(_statue);
		series_stream("510 RIP DIES", 5, 0, 4);
		break;

	case 3:
		digi_play("510_s04", 1);
		digi_play("510_s05", 2);
		break;

	case 4:
		disable_player_commands_and_fade_init(5);
		break;

	case 5:
		_G(game).setRoom(413);
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room510::woodenLadder() {
	if (player_said("ROPE", "WOODEN LADDER")) {
		if (inv_object_is_here("GREEN VINE") || inv_object_is_here("BROWN VINE") ||
			inv_object_is_here("VINES"))
			return;

		hotspot_set_active("ROPE", true);
		inv_put_thing_in("ROPE", 510);
	}

	if (player_said("BROWN VINE", "WOODEN LADDER")) {
		if (inv_object_is_here("GREEN VINE") || inv_object_is_here("VINES") ||
			inv_object_is_here("ROPE"))
			return;

		hotspot_set_active("BROWN VINE", true);
		inv_put_thing_in("BROWN VINE", 510);
	}

	if (player_said("GREEN VINE", "WOODEN LADDER")) {
		if (inv_object_is_here("BROWN VINE") || inv_object_is_here("VINES") ||
			inv_object_is_here("ROPE"))
			return;

		hotspot_set_active("GREEN VINE", true);
		inv_put_thing_in("GREEN VINE", 510);
	}

	if (player_said("VINES", "WOODEN LADDER")) {
		if (inv_object_is_here("BROWN VINE") || inv_object_is_here("GREEN VINE") ||
			inv_object_is_here("ROPE"))
			return;

		hotspot_set_active("VINES", true);
		inv_put_thing_in("VINES", 510);
	}

	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		ws_hide_walker();
		_steps = series_play("Rip ties rope to ladder", 0x100, 16, 1, 5);
		break;

	case 1:
		if (player_said("GREEN VINE", "WOODEN LADDER")) {
			_rope = series_play("510 SHORTER VINE UNWINDS", 0x100, 16, 2, 5);
			digi_play("510_s06", 1);
		}

		if (player_said("BROWN VINE", "WOODEN LADDER")) {
			_rope = series_play("510 LONGER VINE UNWINDS", 0x100, 16, 2, 5);
			digi_play("510_s06", 1);
		}

		if (player_said("VINES", "WOODEN LADDER")) {
			_rope = series_play("ROPE UNWINDS DOWN THE TOWER", 0x100, 16, 2, 5);
			digi_play("510_s06", 1);
		}

		if (player_said("ROPE", "WOODEN LADDER")) {
			_rope = series_play("ROPE UNWINDS DOWN THE TOWER", 0x100, 16, 2, 5);
			digi_play("510_s06", 1);
		}
		break;

	case 2:
		terminateMachineAndNull(_steps);
		_steps = series_play("Rip ties rope to ladder", 0x100, 18, 3, 5, 0, 100, 0, 0, 0, 23);
		break;

	case 3:
		ws_unhide_walker();
		terminateMachineAndNull(_steps);
		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

void Room510::altar() {
	switch (_G(kernel).trigger) {
	case -1:
		ws_walk(395, 121, nullptr, 1, 7);
		break;

	case 1:
		player_set_commands_allowed(false);
		_ripLowReach3 = series_load("Rip trek low reacher pos5");
		setGlobals1(_ripLowReach3, 1, 26, 26, 26);
		sendWSMessage_110000(2);
		break;

	case 2:
		digi_play("510_s03", 1);
		hotspot_set_active("WOODEN LADDER", true);
		sendWSMessage_140000(3);
		_ladder = series_play("510 LADDER", 0xf00, 16, -1, 5);
		inv_put_thing_in("WOODEN LADDER", 510);
		break;

	case 3:
		ws_hide_walker();
		_steps = series_play("Rip ties rope to ladder", 0x100, 16, 4, 5);
		break;

	case 4:
		series_unload(_ripLowReach3);

		if (player_said("LADDER/GREEN VINE", "ALTAR")) {
			_rope = series_play("510 SHORTER VINE UNWINDS", 0x100, 16, 5, 5);
			digi_play("510_s06", 1);
			hotspot_set_active("GREEN VINE", true);
		}

		if (player_said("LADDER/BROWN VINE", "ALTAR")) {
			_rope = series_play("510 LONGER VINE UNWINDS", 0x100, 16, 5, 5);
			digi_play("510_s06", 1);
			hotspot_set_active("BROWN VINE", true);
		}

		if (player_said("LADDER/VINES", "ALTAR")) {
			_rope = series_play("ROPE UNWINDS DOWN THE TOWER", 0x100, 16, 5, 5);
			digi_play("510_s06", 1);
			hotspot_set_active("VINES", true);
		}

		if (player_said("LADDER/ROPE", "ALTAR")) {
			_rope = series_play("ROPE UNWINDS DOWN THE TOWER", 0x100, 16, 5, 5);
			digi_play("510_s06", 1);
			hotspot_set_active("ROPE", true);
		}
		break;

	case 5:
		terminateMachineAndNull(_steps);
		_steps = series_play("Rip ties rope to ladder", 0x100, 18, 6, 5, 0, 100, 0, 0, 0, 23);
		break;

	case 6:
		ws_unhide_walker();
		terminateMachineAndNull(_steps);

		if (player_said("LADDER/GREEN VINE", "ALTAR")) {
			inv_put_thing_in("GREEN VINE", 510);
			inv_put_thing_in("WOODEN LADDER", 510);
			inv_put_thing_in("LADDER/GREEN VINE", NOWHERE);
			hotspot_set_active("GREEN VINE", true);
		}

		if (player_said("LADDER/BROWN VINE", "ALTAR")) {
			inv_put_thing_in("BROWN VINE", 510);
			inv_put_thing_in("WOODEN LADDER", 510);
			inv_put_thing_in("LADDER/BROWN VINE", NOWHERE);
			hotspot_set_active("BROWN VINE", true);
		}

		if (player_said("LADDER/VINES", "ALTAR")) {
			inv_put_thing_in("VINES", 510);
			inv_put_thing_in("WOODEN LADDER", 510);
			inv_put_thing_in("LADDER/VINES", NOWHERE);
			hotspot_set_active("VINES", true);
		}

		if (player_said("LADDER/ROPE", "ALTAR")) {
			inv_put_thing_in("ROPE", 510);
			inv_put_thing_in("WOODEN LADDER", 510);
			inv_put_thing_in("LADDER/ROPE", NOWHERE);
			hotspot_set_active("ROPE", true);
		}

		player_set_commands_allowed(true);
		break;

	default:
		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
