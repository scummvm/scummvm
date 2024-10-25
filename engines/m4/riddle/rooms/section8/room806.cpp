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

#include "m4/riddle/rooms/section8/room806.h"

#include "m4/adv_r/adv_file.h"
#include "m4/adv_r/other.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/riddle.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room806::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
}

void Room806::init() {
	_unkVar1 = 1;
	_unkVar2 = 0;

	_806rp03Series = series_load("806RP03", -1, nullptr);
	_ripKneelAndExamineWallSeries = series_load("RIP KNEEL AND EXAMINE WALL", -1, nullptr);
	_ripTrekLookDownPos3Series = series_load("RIP TREK LOOK DOWN POS3", -1, nullptr);
	_ripLookUpPos3Series = series_load("RIP LOOKS UP POS3", -1, nullptr);
	_ripMedHighReacherPos2Series = series_load("RIP MED HIGH REACHER POS2", -1, nullptr);
	_ripHighReacherPos2Series = series_load("RIP HIGH REACHER POS2", -1, nullptr);
	_ripTrekTalkerPos3Series = series_load("RIP TREK TALKER POS3", -1, nullptr);
	_ripTrekHandTalkPos3Series = series_load("RIP TREK HAND TALK POS3", -1, nullptr);
	_ripTrekHeadTurnPos3Series = series_load("RIP TREK HEAD TURN POS3", -1, nullptr);
	_meiTalkPos3Series = series_load("MEI TALKS POS3", -1, nullptr);
	_meiTrekHeadTurnPos3Series = series_load("MEI TREK HEAD TURN POS3", -1, nullptr);
	_ripPos3LookAroundSeries = series_load("RIP POS 3 LOOK AROUND", -1, nullptr);
	_meiTrekHandsOnHipsPos3Series = series_load("MEI TREK HANDS ON HIPS POS3", -1, nullptr);

	series_play("LIT URN 1 ", 32767, 0, -1, 7, -1, 100, 0, 0, 0, -1);
	series_play("LIT URN 2", 32767, 0, -1, 7, -1, 100, 0, 0, 0, -1);

	if (!_G(flags[V265]))
		series_play("806 flash light flicker", 32767, 0, -1, 7, 2, 100, 0, 0, 0, -1);

	if (inv_object_is_here("CRANK")) {
		_806CrankMach = series_show("806CRANK", 32767, 16, -1, -1, 0, 100, 0, 0);
		hotspot_set_active(_G(currentSceneDef).hotspots, "SLOT", false);
	} else {
		hotspot_set_active(_G(currentSceneDef).hotspots, "CRANK", false);
	}

	if (_G(flags[V270]) == 806) {
		hotspot_set_active(_G(currentSceneDef).hotspots, "CHARIOT", false);
	} else {
		digi_stop(2);
		digi_unload("950_s33");
		_806ChartMach = series_show("806chart", 3840, 16, -1, -1, 0, 100, 0, 0);
		kernel_load_variant("806lock1");
		digi_preload("806r20", -1);
	}

	if (_G(flags[V266])) {
		series_show("806BOLTS", 32767, 16, -1, -1, 0, 100, 0, 0);
	} else {
		_806rp02Series = series_load("806RP02", -1, nullptr);
		_806mc02Series = series_load("806MC02", -1, nullptr);

		hotspot_set_active(_G(currentSceneDef).hotspots, "CROSSBOW BOLT  ", false);
		digi_preload("806m08", -1);
		digi_preload("806_s01", -1);
		digi_preload("950_s52", -1);
		digi_preload("950_s53", -1);
	}

	if (_G(flags[V276]) == 0) {
		ws_walk_load_walker_series(S8_SHADOW_DIRS1, S8_SHADOW_NAMES1);
		ws_walk_load_walker_series(S8_SHADOW_DIRS2, S8_SHADOW_NAMES2);
	}

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		digi_preload("950_s29", -1);
		if (_G(flags[V276])) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN", false);
			hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN ", false);
		} else {
			if (_unkVar3) {
				_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 624, 306, 9, triggerMachineByHashCallback3000, "mc_trek");
				hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN", false);
				hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN ", true);
			} else {
				_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 225, 306, 3, triggerMachineByHashCallback3000, "mc_trek");
				hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN ", false);
				hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN", true);
			}

			if (!player_been_here(809))
				kernel_timing_trigger(imath_ranged_rand(1200, 1800), 201, nullptr);

			kernel_timing_trigger(imath_ranged_rand(1200, 1800), 701, nullptr);
		}

		_unkVar1 = 0;

		break;

	case 805:
		if (!_G(flags[V265])) {
			_806rp01Series = series_load("806RP01", -1, nullptr);
			_806mc01Series = series_load("806MC01", -1, nullptr);
			digi_preload("806m01");
			digi_preload("806r01");
			digi_preload("806m02");
			digi_preload("806r02");
			digi_preload("806m03");
			digi_preload("806r03");
		}

		player_set_commands_allowed(false);
		ws_demand_facing(_G(my_walker), 3);
		ws_demand_location(_G(my_walker), -30, 316);

		if (_G(flags[V276])) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN ", false);
			hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN", false);
		} else {
			_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, -60, 306, 3, triggerMachineByHashCallback3000, "mc_trek");
			hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN ", false);
			hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN", true);
			_unkVar3 = 0;
		}

		if (_G(flags[V270]) == 806) {
			terminateMachine(_806ChartMach);
			_806ChartMach = series_show("806chart", 1536, 16, -1, -1, 0, 100, 0, 0);
			if (!_G(flags[V265])) {
				ws_walk(_G(my_walker), 221, 316, nullptr, 12001, 3, true);
			} else {
				if (!player_been_here(809))
					kernel_timing_trigger(imath_ranged_rand(1200, 1800), 201, nullptr);

				kernel_timing_trigger(imath_ranged_rand(1200, 1800), 701, nullptr);
			}

			if (!_G(flags[V276]))
				sendWSMessage_10000(_mcTrekMach, 30, 296, 0, 501, 1);

		} else if (!_G(flags[V265])) {
			sendWSMessage_10000(_mcTrekMach, 225, 306, 3, 101, 1);
			ws_walk(221, 316, nullptr, 12001, 3, true);
		} else {
			if (!_G(flags[V276]))
				sendWSMessage_10000(_mcTrekMach, 225, 306, 3, -1, 1);

			ws_walk(221, 316, nullptr, 999, 3, true);

			if (!_G(flags[V276])) {
				if (!player_been_here(809))
					kernel_timing_trigger(imath_ranged_rand(1200, 1800), 201, nullptr);

				kernel_timing_trigger(imath_ranged_rand(1200, 1800), 701, nullptr);
			}
		}
		break;

	case 807:
		player_set_commands_allowed(false);
		ws_demand_facing(_G(my_walker), 9);
		ws_demand_location(_G(my_walker), 670, 315);
		ws_walk(_G(my_walker), 594, 315, nullptr, 999, 9, true);
		if (_G(flags[V276])) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN ", false);
			hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN", false);
		} else {
			_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 710, 306, 9, triggerMachineByHashCallback3000, "mc_trek");
			sendWSMessage_10000(_mcTrekMach, 624, 306, 9, -1, 1);
			hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN ", true);
			hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN", false);

			if (!player_been_here(809))
				kernel_timing_trigger(imath_ranged_rand(1200, 1800), 201, nullptr);

			kernel_timing_trigger(imath_ranged_rand(1200, 1800), 701, nullptr);
		}

		break;

	default:
		digi_preload("950_s29", -1);
		ws_demand_facing(_G(my_walker), 3);
		ws_demand_location(_G(my_walker), 221, 316);
		if (_G(flags[V276])) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN ", true);
			hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN", false);
			_unkVar1 = 0;
		} else {
			if (_unkVar3) {
				_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 624, 306, 9, triggerMachineByHashCallback3000, "mc_trek");
				hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN", false);
				hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN ", true);
			} else {
				_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 225, 306, 3, triggerMachineByHashCallback3000, "mc_trek");
				hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN ", false);
				hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN", true);
			}

			if (!player_been_here(806))
				kernel_timing_trigger(imath_ranged_rand(1200, 1800), 201, nullptr);

			kernel_timing_trigger(imath_ranged_rand(1200, 1800), 701, nullptr);
		}

		_unkVar1 = 0;

		break;
	}

	digi_play_loop("950_s29", 3, 75, -1, -1);
}

void Room806::pre_parser() {
	bool lookFl = player_said_any("look", "look at");
	bool walkFl = player_said("walk to");
	bool takeFl = player_said("take");
	bool gearFl = player_said_any("push", "pull", "gear", "open", "close");

	if (player_said_any(" ", "WALL", "URN")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (lookFl && player_said_any("  ", "    ")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (walkFl && player_said_any("  ", "   ")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (player_said("west") && player_said("     ")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (player_said_any("    ", "      ") && _G(flags[266]) == 0) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (player_said_any("    ", "      ") && _G(flags[266]) != 0 && !walkFl && !player_said("east")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (takeFl && player_said("CROSSBOW BOLT ")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if ((takeFl && player_said("CRANK")) || (player_said("CRANK") && player_said("SLOT")) || (gearFl && player_said("CRANK"))) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (player_said_any("MEI CHEN", "MEI CHEN ")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (gearFl && player_said("CHARIOT")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (player_said("west") && _G(flags[276]) == 0) {
		sendWSMessage_10000(_mcTrekMach, 225, 306, 9, -1, 1);
		_unkVar3 = 1;
		hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN", true);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN ", false);
	}

	if (player_said("east") && _G(flags[266]) && _G(flags[276]) == 0) {
		sendWSMessage_10000(_mcTrekMach, 624, 306, 3, -1, 1);
		_unkVar3 = 0;
		hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN ", true);
	}

}

void Room806::parser() {
	bool lookFl = player_said("look at");
	bool walkFl = player_said("walk to");
	bool takeFl = player_said("take");
	bool gearFl = player_said("gear");

	if (lookFl && player_said_any(" ", "   "))
		digi_play("806r04", 1, 255, -1, -1);

	else if (lookFl && player_said("WALL")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_unkVar1 = 1;
			player_update_info(_G(my_walker), &_G(player_info));
			if (_G(MouseState).CursorColumn >= _G(player_info).x)
				ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, 2, 3, true);
			else
				ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, 2, 9, true);

			break;

		case 2:
			setGlobals1(_ripTrekHeadTurnPos3Series, 1, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), 3);

			break;

		case 3:
			digi_play("806r05", 1, 255, 8, -1);
			sendWSMessage_140000(_G(my_walker), 4);

			break;

		case 4:
			setGlobals1(_ripTrekHeadTurnPos3Series, 6, 9, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), 6);

			break;

		case 6:
			sendWSMessage_140000(_G(my_walker), 7);
			setGlobals1(_ripTrekTalkerPos3Series, 1, 1, 1, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), -1);

			break;

		case 7:
			setGlobals1(_ripTrekTalkerPos3Series, 1, 1, 1, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), -1);

			break;

		case 8:
			sendWSMessage_150000(_G(my_walker), 10);

			break;

		case 10:
			player_set_commands_allowed(true);
			_unkVar1 = 0;
			_G(flags[V106]) = 1;

			break;

		default:
			break;
		}
	} // if (ecx && player_said("WALL"))

	else if (lookFl && player_said("HOLE")) {
		switch (_G(kernel).trigger) {
		case -1:
			_unkVar1 = 1;
			player_update_info(_G(my_walker), &_G(player_info));
			ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, 1, 3, true);

			break;

		case 1:
			player_set_commands_allowed(false);
			ws_hide_walker(_G(my_walker));
			series_play("RIP KNEEL AND EXAMINE WALL", 2048, 0, 3, 5, 0, 100, 0, 0, 0, -1);

			break;

		case 3:
			digi_play("806r06", 1, 255, 8, -1);
			series_play("RIP KNEEL AND EXAMINE WALL", 2048, 2, 9, 5, 0, 100, 0, 0, 0, -1);

			break;

		case 9 :
			ws_unhide_walker(_G(my_walker));
			player_set_commands_allowed(true);
			_G(flags[V106]) = 1;
			_unkVar1 = 0;

			break;

		default:
			break;
		}

	} // if (ecx && player_said("HOLE"))

	else if (lookFl && player_said_any("CROSSBOW BOLT", "CROSSBOW BOLT ", "CROSSBOW BOLT  "))
		digi_play("806r07", 1, 255, -1, -1);

	else if (lookFl && player_said_any("  ", "    ", "      ")) {
		switch (_G(kernel).trigger) {
		case -1:
			_unkVar1 = 1;
			player_update_info(_G(my_walker), &_G(player_info));

			if (_G(MouseState).CursorColumn >= _G(player_info).x)
				ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, 1, 3, true);
			else
				ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, 1, 9, true);

			break;

		case 1:
			player_set_commands_allowed(false);
			setGlobals1(_ripTrekLookDownPos3Series, 1, 9, 9, 9, 0, 12, 21, 21, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), 2);

			if (_G(flags[V105]) == 0 && _G(flags[V276]) == 0)
				digi_play("806r08", 1, 255, 8, -1);
			else
				digi_play("806r08a", 1, 255, 8, -1);

			break;

		case 2:
			if (_G(flags[V105]) == 0 && _G(flags[V276]) == 0)
				sendWSMessage_120000(_G(my_walker), imath_ranged_rand(3, 4));
			else
				sendWSMessage_120000(_G(my_walker), 8);

			break;

		case 3:
			sendWSMessage_150000(_G(my_walker), 5);
			break;

		case 4:
			sendWSMessage_150000(_G(my_walker), 6);
			break;

		case 5:
			setGlobals1(_ripTrekTalkerPos3Series, 1, 1, 1, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), -1);

			break;

		case 6:
			setGlobals1(_ripTrekHandTalkPos3Series, 1, 16, 16, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), 3);

			break;

		case 8:
			sendWSMessage_150000(_G(my_walker), 9);
			break;

		case 9:
			_G(flags[V105]) = 1;
			player_set_commands_allowed(true);
			_unkVar1 = 0;

			break;

		default:
			break;
		}
	} // if (ecx && player_said_any("  ", ...)

	else if (lookFl && player_said("CRANK") && !inv_player_has("CRANK"))
		digi_play("806r09", 1, 255, -1, -1);
	else if (lookFl && player_said("SLOT"))
		digi_play("806r24", 1, 255, -1, -1);
	else if (lookFl && player_said("KUANG"))
		digi_play("806r10", 1, 255, -1, -1);
	else if (lookFl && player_said("CHARIOT"))
		digi_play("806r11", 1, 255, -1, -1);
	else if (lookFl && player_said("URN")) {
		switch (_G(kernel).trigger) {
		case -1:
			_unkVar1 = 1;
			player_update_info(_G(my_walker), &_G(player_info));

			if (_G(MouseState).CursorColumn >= _G(player_info).x)
				ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, 1, 3, true);
			else
				ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, 1, 9, true);

			break;

		case 1:
			player_set_commands_allowed(false);
			setGlobals1(_ripLookUpPos3Series, 1, 6, 6, 6, 0, 6, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), 3);
			digi_play("com060", 1, 255, -1, 997);

			break;

		case 3:
			sendWSMessage_120000(_G(my_walker), 5);
			break;

		case 5:
			sendWSMessage_120000(_G(my_walker), 7);
			break;


		case 7:
			player_set_commands_allowed(true);
			_unkVar1 = 0;

			break;
		default:
			break;
		}
	} // if (ecx && player_said("URN"))

	else if (takeFl && player_said("URN"))
		digi_play("com067", 1, 255, -1, 997);
	else if (gearFl && player_said("URN"))
		digi_play("com072", 1, 255, -1, 997);
	else if (lookFl && player_said_any("MEI CHEN", "MEI CHEN "))
		digi_play("806r14", 1, 255, -1, -1);
	else if (lookFl && player_said("FLASHLIGHT")) {
		if (_G(flags[V267]) == 0) {
			digi_play("806r25", 1, 255, -1, -1);
			_G(flags[V267]) = 1;
		} else
			digi_play("806r25a", 1, 255, -1, -1);
	} // if (ecx && player_said("FLASHLIGHT"))

	else if (lookFl && player_said("     ")) {
		player_update_info(_G(my_walker), &_G(player_info));
		ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, -1, 9, true);
		digi_play("805r02", 1, 255, -1, 805);
	} else if (lookFl && player_said("KUANG'S HAT"))
		digi_play("806r27", 1, 255, -1, -1);
	else if ((takeFl || gearFl) && player_said("KUANG'S HAT"))
		digi_play("806r28", 1, 255, -1, -1);
	else if (takeFl && player_said("CROSSBOW BOLT ")) {
		switch (_G(kernel).trigger) {
		case -1:
			_unkVar1 = 1;
			ws_walk(_G(my_walker), 281, 287, nullptr, 1, 2, true);

			break;

		case 1:
			player_set_commands_allowed(false);
			setGlobals1(_ripHighReacherPos2Series, 1, 12, 12, 12, 0, 12, 14, 14, 14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), 3);

			break;

		case 3:
			sendWSMessage_120000(_G(my_walker), 5);
			break;

		case 5:
			sendWSMessage_110000(_G(my_walker), 7);
			digi_play("806r15", 1, 255, -1, -1);
			break;

		case 7:
			sendWSMessage_120000(_G(my_walker), 9);
			break;

		case 9:
			sendWSMessage_110000(_G(my_walker), 11);
			break;

		case 11:
			sendWSMessage_140000(_G(my_walker), 15);
			break;

		case 15:
			ws_walk(_G(my_walker), 266, 299, nullptr, -1, 0, true);
			player_set_commands_allowed(true);
			_unkVar1 = 0;

			break;

		default:
			break;
		}
	} // if (esi && player_said("CROSSBOW BOLT "))

	else if (takeFl && player_said("CROSSBOW BOLT  ")) {
		switch (_G(kernel).trigger) {
		case -1:
			_unkVar1 = 1;
			player_set_commands_allowed(false);
			setGlobals1(_ripMedHighReacherPos2Series, 1, 11, 11, 11, 0, 11, 12, 12, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), 3);

			break;

		case 3:
			sendWSMessage_120000(_G(my_walker), 5);
			break;

		case 5:
			sendWSMessage_110000(_G(my_walker), 7);
			digi_play("806r15", 1, 255, -1, -1);

			break;

		case 7:
			sendWSMessage_120000(_G(my_walker), 9);
			break;

		case 9:
			sendWSMessage_110000(_G(my_walker), 11);
			break;

		case 11:
			sendWSMessage_140000(_G(my_walker), 15);
			break;

		case 15:
			player_set_commands_allowed(true);
			_unkVar1 = 0;

			break;

		default:
			break;
		}
	} // if (esi && player_said("CROSSBOW BOLT  "))

	else if (takeFl && player_said("CROSSBOW BOLT"))
		digi_play("806r16", 1, 255, -1, -1);
	else if (takeFl && player_said("CRANK") && inv_object_is_here("CRANK")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_unkVar1 = 1;
			ws_walk(_G(my_walker), 237, 295, nullptr, 1, 1, true);

			break;

		case 1:
			ws_hide_walker(_G(my_walker));
			player_update_info(_G(my_walker), &_G(player_info));
			_safariShadow3Mach = series_show("SAFARI SHADOW 1", 2304, 16, -1, -1, 0, _G(player_info).scale + 1, _G(player_info).x, _G(player_info).y);
			_806rp01Mach = series_play("806RP03", 2048, 16, 2, 5, 0, 100, 0, 0, 0, 6);

			break;

		case 2:
			terminateMachine(_806CrankMach);
			hotspot_set_active(_G(currentSceneDef).hotspots, "CRANK", false);
			hotspot_set_active(_G(currentSceneDef).hotspots, "SLOT", true);
			kernel_examine_inventory_object("ping crank", _G(master_palette), 5, 1, 212, 150, 3, nullptr, -1);

			break;

		case 3:
			terminateMachine(_806rp01Mach);
			series_play("806RP03", 2048, 2, 5, 5, 0, 100, 0, 0, 0, 6);

			break;

		case 5:
			terminateMachine(_safariShadow3Mach);
			ws_unhide_walker(_G(my_walker));
			player_set_commands_allowed(true);
			inv_give_to_player("CRANK");
			_unkVar1 = 0;

			break;

		default:
			break;
		}
	} // if (esi && player_said("CRANK") && inv_object_is_here("CRANK"))

	else if (takeFl && player_said("CHARIOT"))
		digi_play("806r23", 1, 255, -1, -1);
	else if ((takeFl || gearFl) && player_said("FLASHLIGHT"))
		digi_play("806r26", 1, 255, -1, -1);
	else if (takeFl) {
		switch (imath_ranged_rand(1, 6)) {
		case 1:
			digi_play("com006", 1, 255, -1, 997);
			break;

		case 2:
			digi_play("com007", 1, 255, -1, 997);
			break;

		case 3:
			digi_play("com008", 1, 255, -1, 997);
			break;

		case 4:
			digi_play("com009", 1, 255, -1, 997);
			break;

		case 5:
			digi_play("com010", 1, 255, -1, 997);
			break;

		case 6:
			digi_play("com011", 1, 255, -1, 997);
			break;

		default:
			break;
		}
	} // if (esi)

	else if (gearFl && player_said("CRANK") && !inv_player_has("CRANK")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_unkVar1 = 1;
			ws_walk(_G(my_walker), 237, 295, nullptr, 1, 1, true);

			break;

		case 1:
			ws_hide_walker(_G(my_walker));
			player_update_info(_G(my_walker), &_G(player_info));
			_safariShadow3Mach = series_show("SAFARI SHADOW 1", 2304, 16, -1, -1, 0, _G(player_info).scale + 1, _G(player_info).x, _G(player_info).y);
			series_play("806RP03", 2048, 0, 2, 5, 0, 100, 0, 0, 0, 16);

			break;

		case 2:
			series_play("806RP03", 2048, 2, 3, 5, 0, 100, 0, 0, 15, 16);
			break;

		case 3:
			series_play("806RP03", 2048, 0, 4, 5, 0, 100, 0, 0, 15, 16);
			break;

		case 4:
			series_play("806RP03", 2048, 2, 9, 5, 0, 100, 0, 0, 0, 16);
			break;

		case 9:
			digi_play("806r17", 1, 255, -1, -1);
			terminateMachine(_safariShadow3Mach);
			ws_unhide_walker(_G(my_walker));
			player_set_commands_allowed(true);
			_unkVar1 = 0;

			break;

		default:
			break;

		}
	} // if (edi && player_said("CRANK") && !inv_player_has("CRANK"))

	else if (player_said("CRANK") && player_said("SLOT")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_unkVar1 = 1;
			digi_preload("806_s03", -1);
			ws_walk(_G(my_walker), 237, 295, nullptr, 1, 1, true);

			break;

		case 1:
			ws_hide_walker(_G(my_walker));
			player_update_info(_G(my_walker), &_G(player_info));
			_safariShadow3Mach = series_show("SAFARI SHADOW 1", 2304, 16, -1, -1, 0, _G(player_info).scale + 1, _G(player_info).x, _G(player_info).y);
			series_play("806RP03", 2048, 0, 2, 5, 0, 100, 0, 0, 0, 6);

			break;

		case 2:
			digi_play("806_s03", 2, 255, -1, -1);
			_806CrankMach = series_show("806CRANK", 32767, 16, -1, -1, 0, 100, 0, 0);
			hotspot_set_active(_G(currentSceneDef).hotspots, "CRANK", true);
			hotspot_set_active(_G(currentSceneDef).hotspots, "SLOT", false);
			inv_move_object("CRANK", 806);
			series_play("806RP03", 2048, 2, 5, 5, 0, 100, 0, 0, 0, 6);

			break;

		case 5:
			terminateMachine(_safariShadow3Mach);
			ws_unhide_walker(_G(my_walker));
			player_set_commands_allowed(true);
			_unkVar1 = 0;

			break;

		default:
			break;
		}

	} // if (player_said("CRANK") && player_said("SLOT"))

	else if (player_said("CRANK") && player_said("HOLE"))
		digi_play("806r18", 1, 255, -1, -1);
	else if (!lookFl && !takeFl && !player_said("JOURNAL") && player_said_any("TWO SOLDIERS' SHIELDS", "SOLDIER'S SHIELD")) {
		if (_G(flags[V106]) == 0)
			digi_play("805r23", 1, 255, -1, 805);
		else
			digi_play("806r19", 1, 255, -1, -1);
	} else if (player_said("CRANK") && player_said_any("MEI CHEN", "MEI CHEN "))
		digi_play("806r21", 1, 255, -1, -1);
	else if (gearFl && player_said("CHARIOT")) {
		player_set_commands_allowed(false);
		_G(player).disable_hyperwalk = true;
		_unkVar1 = 1;

		switch (_G(kernel).trigger) {
		case -1:
			series_unload(_806rp02Series);
			series_unload(_806mc02Series);
			series_unload(_806rp03Series);
			series_unload(_ripKneelAndExamineWallSeries);
			series_unload(_ripTrekLookDownPos3Series);
			series_unload(_ripLookUpPos3Series);
			series_unload(_ripHighReacherPos2Series);
			series_unload(_ripTrekTalkerPos3Series);
			series_unload(_ripTrekHandTalkPos3Series);
			series_unload(_ripTrekHeadTurnPos3Series);
			series_unload(_meiTalkPos3Series);
			series_unload(_meiTrekHeadTurnPos3Series);

			digi_unload("806m08");
			digi_unload("806_s01");
			digi_unload("950_s52");
			digi_unload("950_s53");

			ws_walk(_G(my_walker), 20, 320, nullptr, 5, 3, true);
			digi_play("806r20", 1, 255, 1, -1);

			if (_unkVar2 != 0)
				sendWSMessage_140000(_mcTrekMach, 2);

			break;

		case 1:
			if (_unkVar2 != 0)
				kernel_timing_trigger(10, 1, nullptr);
			else
				sendWSMessage_10000(_mcTrekMach, 200, 288, 5, -1, 1);

			break;

		case 2:
			_unkVar2 = 0;
			break;

		case 5:
			digi_unload("806r20");
			digi_preload("806_s01", -1);
			terminateMachine(_806ChartMach);
			digi_preload("950_s33", -1);
			digi_play_loop("950_s33", 1, 255, -1, -1);
			_806rp01Mach = series_stream("806RP05 RIP PUSHES CHARIOT", 5, 512, -1);
			series_stream_break_on_frame(_806rp01Mach, 137, 12);

			break;

		case 12:
			digi_play("806_s01", 2, 255, -1, -1);
			series_stream_break_on_frame(_806rp01Mach, 181, 13);

			break;

		case 13:
			digi_play("806_s01", 2, 255, -1, -1);
			series_stream_break_on_frame(_806rp01Mach, 217, 14);

			break;

		case 14:
			digi_play("806_s01", 2, 255, -1, -1);
			series_stream_break_on_frame(_806rp01Mach, 266, 21);

			break;

		case 21:
			disable_player_commands_and_fade_init(31);
			series_stream_break_on_frame(_806rp01Mach, 276, -1);

			break;

		case 31:
			_G(flags[V266]) = 1;
			_G(flags[V270]) = 807;
			digi_unload("806_s01");
			adv_kill_digi_between_rooms(false);
			digi_play_loop("950_s33", 1, 255, -1, -1);
			digi_play_loop("950_s29", 3, 93, -1, -1);
			_G(game).new_room = 807;

			break;

		default:
			break;
		}

	} // if (edi && player_said("CHARIOT"))

	else if (player_said("talk to", "MEI CHEN") || player_said("talk to", "MEI CHEN ")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			if (_unkVar2) {
				sendWSMessage_140000(_mcTrekMach, -1);
				_unkVar2 = 0;
			}

			_unkVar1 = 1;
			player_update_info(_G(my_walker), &_G(player_info));
			if (_G(player_info).facing >= 1 && _G(player_info).facing <= 5)
				ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, 1, 3, true);
			else if (_G(player_info).facing >= 7 && _G(player_info).facing <= 11)
				ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, 1, 9, true);

			break;

		case 1:
			setGlobals1(_ripTrekTalkerPos3Series, 1, 1, 1, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), -1);
			switch (imath_ranged_rand(1, 4)) {
			case 1:
				digi_play("com034", 1, 255, 3, 997);
				break;

			case 2:
				digi_play("com035", 1, 255, 3, 997);
				break;

			case 3:
				digi_play("com036", 1, 255, 3, 997);
				break;

			case 4:
				digi_play("com037", 1, 255, 3, 997);
				break;

			default:
				break;
			}

			break;

		case 3:
			sendWSMessage_150000(_G(my_walker), 5);
			break;

		case 5:
			setGlobals1(_meiTalkPos3Series, 1, 1, 1, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_mcTrekMach, -1);
			switch (imath_ranged_rand(1, 3)) {
			case 1:
				digi_play("com039", 1, 255, 7, 997);
				break;

			case 2:
				digi_play("com040", 1, 255, 7, 997);
				break;

			case 3:
				digi_play("com041", 1, 255, 7, 997);
				break;

			default:
				break;
			}

			break;

		case 7:
			sendWSMessage_150000(_mcTrekMach, 9);
			break;

		case 9:
			player_set_commands_allowed(true);
			_unkVar1 = 0;
			break;

		default:
			break;

		}
	} // if (player_said("talk to", "MEI CHEN") || player_said("talk to", "MEI CHEN "))

	else if (!lookFl && player_said("JOURNAL"))
		digi_play("com042", 1, 255, -1, 997);

	else if (!gearFl && player_said_any("MEI CHEN", "MEI CHEN "))
		digi_play("com017", 1, 255, 997);

	else if (((walkFl && player_said("    ")) || (!walkFl && player_said("east"))) && _G(flags[266]) == 0) {
		switch (_G(kernel).trigger) {
		case -1:
			_G(player).disable_hyperwalk = true;
			player_set_commands_allowed(false);
			ws_walk(_G(my_walker), 446, 315, nullptr, 1, 0, true);
			_candlemanShadow3Mach = series_show("CANDLEMAN SHADOW3", 1792, 16, -1, -1, 0, 84, 225, 306);
			if (_unkVar2 != 0)
				sendWSMessage_140000(_mcTrekMach, -1);

			_unkVar1 = 1;
			break;

		case 1:
			ws_hide_walker(_G(my_walker));
			digi_play("806_s02", 2, 255, -1, -1);
			series_play("806RP02", 256, 0, 3, 5, 0, 100, 0, 0, 0, 8);

			break;

		case 3:
			_806rp01Mach = series_play("806RP02", 256, 0, 5, 5, 0, 100, 0, 0, 9, 18);
			digi_play("806m08", 1, 255, -1, -1);
			digi_play("950_s52", 2, 255, -1, 950);
			sendWSMessage_60000(_mcTrekMach);
			_806mc01Mach = series_play("806MC02", 256, 16, -1, 5, 0, 100, 0, 0, 0, -1);

			break;

		case 5:
			_806rp01Mach = series_play("806RP02", 256, 16, 7, 5, 0, 100, 0, 0, 19, -1);
			digi_play("950_s53", 2, 255, -1, 950);

			break;

		case 7:
			kernel_timing_trigger(60, 9, nullptr);
			break;

		case 9:
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(11);

			break;

		case 11:
			ws_unhide_walker(_G(my_walker));
			_G(player).disable_hyperwalk = false;
			ws_demand_location(_G(my_walker), 221, 316);
			ws_demand_facing(_G(my_walker), 3);
			other_save_game_for_resurrection();
			_G(game).new_section = 4;
			_G(game).new_room = 413;

			break;

		default:
			break;
		}
	} // if (((walkFl && player_said("    ")) || (!walkFl && player_said("east"))) && _G(flags[266]) == 0)

	else if (player_said("west") && (_G(kernel).trigger == -1 || _G(kernel).trigger == 1)) {
		if (_G(kernel).trigger == -1) {
			player_set_commands_allowed(false);
			_unkVar1 = 1;
			disable_player_commands_and_fade_init(1);
		} else if (_G(kernel).trigger == 1) {
			adv_kill_digi_between_rooms(false);
			digi_play_loop("950_s29", 3, 93, -1, -1);
			_G(game).new_room = 805;
		}
	} // if (player_said("west") && (_G(kernel).trigger == -1 || _G(kernel).trigger == 1))

	else if (player_said("east")) {
		if (_G(kernel).trigger == -1) {
			player_set_commands_allowed(false);
			_unkVar1 = 1;
			disable_player_commands_and_fade_init(1);
		} else if (_G(kernel).trigger == 1) {
			adv_kill_digi_between_rooms(false);
			digi_play_loop("950_s29", 3, 93, -1, -1);
			_G(game).new_room = 807;
		}
	}


	_G(player).command_ready = 0;
}

void Room806::daemon() {
	switch (_G(kernel).trigger) {
	case 101:
		ws_hide_walker(_mcTrekMach);
		_candlemanShadow3Mach = series_show("CANDLEMAN SHADOW3", 32767, 16, -1, -1, 0, 84, 225, 306);
		series_play("806MC01", 1792, 0, 102, 5, 0, 100, 0, 0, 0, 23);

		break;

	case 102:
		digi_play("806m01", 1, 255, 106, -1);
		series_play("806MC01", 1792, 0, 103, 5, 0, 100, 0, 0, 24, 39);

		break;

	case 103:
		terminateMachine(_806rp01Mach);
		_806rp01Mach = series_play("806RP01", 256, 16, -1, 5, 0, 100, 0, 0, 3, 6);
		_806mc01Mach = series_play("806MC01", 1792, 16, -1, 5, 0, 100, 0, 0, 40, -1);

		break;

	case 106:
		digi_play("806r01", 1, 255, 107, -1);
		break;

	case 107:
		digi_play("806m02", 1, 255, 111, -1);
		terminateMachine(_806mc01Mach);
		series_play("806MC01", 1792, 2, 108, 5, 0, 100, 0, 0, 41, -1);

		break;

	case 108:
		terminateMachine(_806rp01Mach);
		_806rp01Mach = series_play("806RP01", 256, 16, -1, 5, 0, 100, 0, 0, 7, 14);
		series_play("806MC01", 1792, 2, 110, 5, 0, 100, 0, 0, 24, 40);

		break;

	case 110:
		_806mc01Mach = series_show("806MC01", 1792, 16, -1, -1, 24, 100, 0, 0);
		break;

	case 111:
		digi_play("806r02", 1, 255, 112, -1);
		break;

	case 112:
		digi_play("806m03", 1, 255, 12014, -1);
		kernel_timing_trigger(1, 114, nullptr);
		terminateMachine(_806mc01Mach);
		series_play("806MC01", 1792, 2, 113, 5, 0, 100, 0, 0, 0, 23);

		break;

	case 113:
		terminateMachine(_candlemanShadow3Mach);
		ws_unhide_walker(_G(my_walker));

		break;

	case 114:
		terminateMachine(_806rp01Mach);
		series_play("806RP01", 256, 0, 115, 5, 0, 100, 0, 0, 14, -1);

		break;

	case 115:
		terminateMachine(_safariShadow3Mach);
		setGlobals1(_ripPos3LookAroundSeries, 1, 20, 20, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		ws_unhide_walker(_G(my_walker));
		sendWSMessage_110000(_G(my_walker), 116);

		break;

	case 116:
		sendWSMessage_140000(_G(my_walker), 117);
		break;

	case 117:
		_G(flags[265]) = 1;
		_G(flags[256]) = 1;
		series_unload(_806rp01Series);
		series_unload(_806mc01Series);
		digi_unload("806m01");
		digi_unload("806r01");
		digi_unload("806m02");
		digi_unload("806r02");
		digi_unload("806m03");
		player_set_commands_allowed(true);
		_unkVar1 = 0;
		_unkVar3 = 0;
		kernel_timing_trigger(imath_ranged_rand(1200, 1800), 201, nullptr);
		kernel_timing_trigger(imath_ranged_rand(1200, 1800), 701, nullptr);

		break;

	case 201:
		player_update_info(_G(my_walker), &_G(player_info));
		if (!_unkVar1 && player_commands_allowed() && checkStrings()) {
			_unkVar1 = 1;
			player_set_commands_allowed(false);
			intr_cancel_sentence();
			switch (imath_ranged_rand(1, 4)) {
			case 1:
				digi_play("950_s15", 2, 255, -1);
				break;

			case 2:
				digi_play("950_s16", 2, 255, -1);
				break;

			case 3:
				digi_play("950_s17", 2, 255, -1);
				break;

			case 4:
				digi_play("950_s18", 2, 255, -1);
				break;

			default:
				break;
			}

			if (_G(player_info).facing >= 1 && _G(player_info).facing <= 5)
				ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, 205, 3, true);
			else if (_G(player_info).facing > 5 && _G(player_info).facing <= 11)
				ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, 205, 9, true);

		} else if (_G(flags[269]))
			kernel_timing_trigger(60, 201, nullptr);

		break;

	case 205:
		switch (imath_ranged_rand(1, 5)) {
		case 1:
			_unkVar4 = 1;
			kernel_timing_trigger(1, 250, nullptr);

			break;

		case 2:
			setGlobals1(_meiTrekHeadTurnPos3Series, 1, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_mcTrekMach, 210);

			break;

		case 3:
			setGlobals1(_meiTrekHeadTurnPos3Series, 6, 9, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_mcTrekMach, 211);

			break;

		case 4:
			setGlobals1(_meiTrekHeadTurnPos3Series, 1, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_mcTrekMach, 212);

			break;

		case 5:
			setGlobals1(_meiTrekHeadTurnPos3Series, 1, 20, 20, 20, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_mcTrekMach, 220);

			break;

		default:
			break;
		}

		if (_unkVar2) {
			sendWSMessage_140000(_mcTrekMach, 50);
			_unkVar5 = 1;
		} else {
			switch (imath_ranged_rand(1, 3)) {
			case 1:
				_unkVar5 = 1;
				kernel_timing_trigger(1, 250, nullptr);
				break;

			case 2:
				setGlobals1(_meiTrekHeadTurnPos3Series, 1, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
				sendWSMessage_110000(_mcTrekMach, 213);
				break;

			case 3:
				setGlobals1(_meiTrekHeadTurnPos3Series, 6, 9, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
				sendWSMessage_110000(_mcTrekMach, 214);
				break;

			default:
				break;
			}
		}

		break;

	case 210:
		sendWSMessage_140000(_G(my_walker), 215);
		break;

	case 211:
		sendWSMessage_140000(_G(my_walker), 216);
		break;

	case 212:
		_unkVar4 = 1;
		sendWSMessage_150000(_G(my_walker), 250);
		break;

	case 213:
		sendWSMessage_140000(_mcTrekMach, 218);
		break;

	case 214:
		sendWSMessage_140000(_mcTrekMach, 219);
		break;

	case 215:
		switch (imath_ranged_rand(1, 2)) {
		case 1:
			setGlobals1(_ripTrekHeadTurnPos3Series, 6, 9, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), 220);

			break;

		case 2:
			_unkVar4 = 1;
			kernel_timing_trigger(1, 250, nullptr);

			break;

		default:
			break;
		}

		break;

	case 216:
		switch (imath_ranged_rand(1, 2)) {
		case 1:
			setGlobals1(_ripTrekHeadTurnPos3Series, 1, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), 220);

			break;

		case 2:
			_unkVar4 = 1;
			kernel_timing_trigger(1, 250, nullptr);

			break;

		default:
			break;
		}

		break;

	case 218:
		switch (imath_ranged_rand(1, 2)) {
		case 1:
			setGlobals1(_ripTrekHeadTurnPos3Series, 6, 9, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_mcTrekMach, 223);

			break;

		case 2:
			_unkVar5 = 1;
			kernel_timing_trigger(1, 250, nullptr);

			break;

		default:
			break;
		}

		break;

	case 219:
		switch (imath_ranged_rand(1, 2)) {
		case 1:
			setGlobals1(_ripTrekHeadTurnPos3Series, 1, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_mcTrekMach, 224);

			break;

		case 2:
			_unkVar5 = 1;
			kernel_timing_trigger(1, 250, nullptr);

			break;

		default:
			break;
		}

		break;

	case 220:
		_unkVar4 = 1;
		sendWSMessage_140000(_G(my_walker), 250);

		break;

	case 223:
	case 224:
		_unkVar5 = 1;
		sendWSMessage_140000(_mcTrekMach, 250);
		break;

	case 250:
		player_set_commands_allowed(true);
		if (_unkVar4 && _unkVar5) {
			_unkVar4 = 0;
			_unkVar5 = 0;
			kernel_timing_trigger(1, imath_ranged_rand(260, 263));
		}

		break;

	case 260:
		setGlobals1(_meiTalkPos3Series, 1, 1, 1, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_mcTrekMach, -1);
		digi_play("com052", 1, 255, 270, 997);

		break;

	case 261:
		setGlobals1(_ripTrekTalkerPos3Series, 1, 1, 1, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), -1);
		digi_play("com054", 1, 255, 271, 997);

		break;

	case 262:
		setGlobals1(_ripTrekTalkerPos3Series, 1, 1, 1, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), -1);
		digi_play("com056", 1, 255, 272, 997);

		break;

	case 263:
		setGlobals1(_meiTalkPos3Series, 1, 1, 1, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_mcTrekMach, -1);
		digi_play("com057", 1, 255, 273, 997);

		break;

	case 270:
		sendWSMessage_150000(_mcTrekMach, 280);
		break;

	case 271:
		sendWSMessage_150000(_G(my_walker), 281);
		break;

	case 272:
	case 290:
	case 293:
		sendWSMessage_150000(_G(my_walker), 299);
		break;

	case 273:
		sendWSMessage_150000(_mcTrekMach, 283);
		break;

	case 280:
		setGlobals1(_ripTrekTalkerPos3Series, 1, 1, 1, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), -1);
		digi_play("com053", 1, 255, 290, 997);

		break;

	case 281:
		setGlobals1(_meiTalkPos3Series, 1, 1, 1, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_mcTrekMach, -1);
		digi_play("com055", 1, 255, 291, 997);

		break;

	case 283:
		setGlobals1(_ripTrekTalkerPos3Series, 1, 1, 1, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), -1);
		digi_play("com058", 1, 255, 293, 997);

		break;

	case 291:
		sendWSMessage_150000(_mcTrekMach, 299);
		break;

	case 299:
		player_set_commands_allowed(true);
		_unkVar1 = 0;
		kernel_timing_trigger(imath_ranged_rand(1200, 1800), 201);

		break;

	case 501:
		sendWSMessage_10000(_mcTrekMach, 200, 296, 0, 502, 0);
		break;

	case 502:
		terminateMachine(_806ChartMach);
		_806ChartMach = series_show("806chart", 3840, 16, -1, -1, 0, 100, 0, 0);

		if (_G(flags[265]))
			sendWSMessage_10000(_mcTrekMach, 225, 306, 3, -1, 1);
		else
			sendWSMessage_10000(_mcTrekMach, 225, 306, 3, 101, 1);

		_unkVar3 = 0;

		break;

	case 701:
		if (!_unkVar2 && !_unkVar1) {
			setGlobals1(_unkSeries1, 1, 22, 22, 22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_mcTrekMach, 703);
			_unkVar2 = 1;
		}

		kernel_timing_trigger(imath_ranged_rand(1200, 1800), 702, nullptr);

		break;

	case 702:
		if (_unkVar2) {
			sendWSMessage_140000(_mcTrekMach, 703);
			_unkVar2 = 0;
		}

		kernel_timing_trigger(imath_ranged_rand(1200, 1800), 701, nullptr);

		break;

	case 703:
		_unkVar1 = 0;
		break;

	case 999:
		if (_G(flags[265])) {
			player_set_commands_allowed(true);
			_unkVar1 = 0;
		}

		break;

	case 12001:
		ws_hide_walker(_G(my_walker));
		player_update_info(_G(my_walker), &_G(player_info));
		_safariShadow3Mach = series_show("SAFARI SHADOW 3", 32767, 16, -1, -1, 0, _G(player_info).scale + 1, 221, 316);
		series_play("806RP01", 256, 2, 12002, 5, 0, 100, 0, 0, 12, 16);

		break;

	case 12002:
		_806rp01Mach = series_show("806RP01", 256, 16, -1, -1, 12, 100, 0, 0);
		break;

	case 12014:
		digi_play("806r03", 1, 255, 12016, -1);
		break;

	case 12016:
		digi_unload("806r03");
		break;

	default:
		break;
	}

}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
