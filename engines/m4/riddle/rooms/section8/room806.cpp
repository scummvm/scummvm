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
}

void Room806::daemon() {
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
