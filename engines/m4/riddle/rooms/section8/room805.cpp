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

#include "m4/riddle/rooms/section8/room805.h"

#include "m4/adv_r/adv_file.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/riddle.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room805::initSub2() {
	series_play("BRAZIER 1 (L TO R) BURNING", 768, 0, -1, 5, -1, 100, 0, 0, 0, -1);
	series_play("BRAZIER 2 (L TO R) BURNING", 1536, 0, -1, 5, -1, 100, 0, 0, 0, -1);
	series_play("BRAZIER 3 (L TO R) BURNING", 3840, 0, -1, 5, -1, 100, 0, 0, 0, -1);
	series_play("BRAZIER 4&5 (L TO R) BURNING", 2560, 0, -1, 5, -1, 100, 0, 0, 0, -1);
	series_play("BRAZIER 6 (L TO R) BURNING", 0, 0, -1, 5, -1, 100, 0, 0, 0, -1);

	initHotspots();

	switch (_G(game).previous_room) {
	case -2:
		if (_G(flags)[V276] == 0) {
			_mcMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 276, 201, 10, Walker::player_walker_callback, "mc");
		}

		break;

	case 804:
		ws_demand_location(_G(my_walker), 0, 264);
		if (_G(flags)[V276] == 0) {
			player_set_commands_allowed(false);
			_mcMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 0, 264, 3, Walker::player_walker_callback, "mc");
			ws_walk(_G(my_walker), 53, 264, nullptr, 102, 3, true);
		} else {
			ws_walk(_G(my_walker), 295, 305, nullptr, -1, 3, true);
		}
		break;

	case 806:
		ws_demand_location(_G(my_walker), 395, 202);
		ws_demand_facing(_G(my_walker), 9);
		if (_G(flags)[V276] == 0) {
			player_set_commands_allowed(false);
			_mcMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 420, 201, 0, Walker::player_walker_callback, "mc");
			ws_walk(_G(my_walker), 370, 201, nullptr, 109, 3, true);
		} else {
			ws_walk(_G(my_walker), 295, 305, nullptr, -1, 3, true);
		}
		break;

	case 814:
		ws_demand_location(_G(my_walker), 119, 194);
		if (_G(flags)[V276] == 0) {
			player_set_commands_allowed(false);
			_mcMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 119, 194, 0, Walker::player_walker_callback, "mc");
			ws_walk(_G(my_walker), 169, 194, nullptr, 103, 3, true);
		} else {
			ws_walk(_G(my_walker), 295, 305, nullptr, -1, 3, true);
		}

		break;

	case 824:
		ws_demand_location(_G(my_walker), 61, 221);
		if (_G(flags)[V276] == 0) {
			_mcMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 61, 221, 0, Walker::player_walker_callback, "mc");
		}

		if (_G(flags)[V270] == 805 && _G(flags)[V262] == 0) {
			ws_hide_walker(_G(my_walker));
			player_set_commands_allowed(false);
			digi_preload("950_s33", -1);
			digi_play_loop("950_s33", 2, 255, -1, -1);
			kernel_timing_trigger(1, 110, nullptr);
		} else if (_G(flags)[V276] == 0) {
			player_set_commands_allowed(false);
			ws_walk(_G(my_walker), 295, 305, nullptr, 104, 3, true);
		} else {
			ws_walk(_G(my_walker), 295, 305, nullptr, -1, 3, true);
		}

		break;

	case 834:
		ws_demand_location(_G(my_walker), 156, 164);
		if (_G(flags)[V276] == 0) {
			_mcMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 156, 164, 3, Walker::player_walker_callback, "mc");
			player_set_commands_allowed(false);
			ws_walk(_G(my_walker), 206, 164, nullptr, 107, 3, true);
		} else {
			ws_walk(_G(my_walker), 295, 305, nullptr, -1, 3, true);
		}

		break;

	case 844:
		ws_demand_location(_G(my_walker), 170, 154);
		if (_G(flags)[V276] == 0) {
			player_set_commands_allowed(false);
			_mcMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 170, 154, 3, Walker::player_walker_callback, "mc");
			ws_walk(_G(my_walker), 240, 154, nullptr, 108, 3, true);
		} else {
			ws_walk(_G(my_walker), 295, 305, nullptr, -1, 3, true);
		}

		break;

	default:
		ws_demand_facing(_G(my_walker), 3);
		ws_demand_location(_G(my_walker), 295, 305);
		if (_G(flags)[V276] == 0) {
			_mcMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 276, 201, 10, Walker::player_walker_callback, "mc");
		}
		break;
	}
}

void Room805::initHotspots() {
	if (inv_object_is_here("TWO SOLDIERS' SHIELDS")) {
		_G(flags[V257]) = 0;
		_G(flags[V258]) = 0;

		hotspot_set_active(_G(currentSceneDef).hotspots, "SHIELD", true);
		hotspot_set_active(_G(currentSceneDef).hotspots, "SHIELD ", true);
	}

	if (_G(flags)[V257] == 0)
		_farSoldiersShieldMach = series_play("805 FAR SOLDIERS SHIELD", 2304, 0, -1, 0, -1, 100, 0, 0, 0, -1);
	else
		hotspot_set_active(_G(currentSceneDef).hotspots, "SHIELD", false);

	if (_G(flags)[V258] == 0)
		_nearSoldiersShieldMach = series_play("805 NEAR SOLDIERS SHIELD", 1280, 0, -1, 0, -1, 100, 0, 0, 0, -1);
	else
		hotspot_set_active(_G(currentSceneDef).hotspots, "SHIELD ", false);

	if (inv_object_is_here("WOODEN POST"))
		_fallenBeamOnFloorMach = series_play("805 FALLEN BEAM ON FLOOR", 0, 0, -1, 0, -1, 100, 0, 0, 0, -1);
	else
		hotspot_set_active(_G(currentSceneDef).hotspots, "WOODEN POST", false);

	if (_G(flags)[V272] == 1) {
		hotspot_set_active(_G(currentSceneDef).hotspots, "JADE DOOR", false);
		_jadeDoorsOpenMach = series_play("805 JADE DOORS OPEN", 1280, 0, -1, 0, -1, 100, 0, 0, 0, -1);
		hotspot_set_active(_G(currentSceneDef).hotspots, "URN ", false);
	} else
		hotspot_set_active(_G(currentSceneDef).hotspots, "JADE DOOR ", false);

	if (!player_been_here(805))
		_G(flags)
		[V262] = 0;

	if (_G(flags)[V270] == 805 && _G(flags)[V262] == 1) {
		_chariotRestMach = series_play("805 CHARIOT REST", 1280, 0, -1, 0, -1, 100, 0, 0, 0, -1);
		kernel_load_variant("805LOCK1");
		hotspot_set_active(_G(currentSceneDef).hotspots, "CHARIOT", true);
	} else {
		hotspot_set_active(_G(currentSceneDef).hotspots, "CHARIOT", false);
	}

	_unkFlag1 = false;
}

void Room805::daemonSub1() {
	switch (_G(kernel).trigger) {
	case 100:
		player_set_commands_allowed(true);
		_unkFlag1 = false;

		break;

	case 101:
		sendWSMessage_10000(_mcMach, 276, 201, 10, 100, 1);
		break;

	case 102:
	case 103:
	case 108:
		ws_walk(_G(my_walker), 295, 305, nullptr, -1, 3, true);
		ws_walk(_mcMach, 250, 201, nullptr, 101, 3, true);

		break;

	case 104:
		sendWSMessage_10000(_mcMach, 185, 221, 1, 105, 1);
		break;

	case 105:
		sendWSMessage_10000(_mcMach, 200, 201, 3, 101, 1);
		break;

	case 109:
		ws_walk(_G(my_walker), 295, 305, nullptr, -1, 3, true);
		kernel_timing_trigger(1, 101, nullptr);

		break;

	case 111:
		_unkFlag1 = true;
		ws_unhide_walker(_G(my_walker));
		ws_demand_facing(_G(my_walker), 3);
		ws_demand_location(_G(my_walker), 240, 215);
		digi_stop(1);
		digi_stop(2);
		digi_unload("950_s33");
		_chariotRestMach = series_play("805 CHARIOT REST", 1280, 0, -1, 0, -1, 100, 0, 0, 0, -1);
		_G(flags[V262]) = 1;
		kernel_load_variant("805LOCK1");
		hotspot_set_active(_G(currentSceneDef).hotspots, "CHARIOT", true);
		ws_walk(_G(my_walker), 295, 305, nullptr, -1, 3, true);
		if (_G(flags[V276]) == 0)
			kernel_timing_trigger(10, 104, nullptr);
		else
			player_set_commands_allowed(true);

		break; // case 111

	default:
		break;
	}
}

void Room805::daemonSub2(const char *seriesName1, const char *seriesName2) {
	switch (_G(kernel).trigger) {
	case 30:
		ws_turn_to_face(_G(my_walker), 3, -1);
		ws_turn_to_face(_mcMach, 10, 31);

		break;

	case 31:
		setGlobals1(_meiHandsBehindBack, 1, 17, 17, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_mcMach, 32);

		break;

	case 32:
		sendWSMessage_150000(_mcMach, -1);
		setGlobals1(_meiSpookedPos2, 1, 49, 49, 49, 0, 49, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_mcMach, 33);
		if (strlen(seriesName1) != 0)
			digi_play(seriesName1, 1, 255, -1, 997);

		break;

	case 33:
		sendWSMessage_120000(_mcMach, -1);
		setGlobals1(_ripPos3LookAround, 1, 19, 19, 19, 0, 19, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 34);

		break;

	case 34:
		digi_play(seriesName2, 1, 255, 35, 997);
		break;

	case 35:
		sendWSMessage_120000(_G(my_walker), 36);
		break;

	case 36:
		sendWSMessage_150000(_G(my_walker), -1);
		sendWSMessage_150000(_mcMach, -1);
		sendWSMessage_140000(_G(my_walker), -1);
		setGlobals1(_meiHandsBehindBack, 17, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_mcMach, 37);

		break;

	case 37:
		sendWSMessage_150000(_mcMach, 38);
		break;

	case 38:
		ws_turn_to_face(_mcMach, 10, -1);
		player_set_commands_allowed(true);

		break;

	default:
		break;
	}

}

void Room805::daemonSub3(const char *seriesName1, const char *seriesName2) {
	switch (_G(kernel).trigger) {
	case 30:
		ws_turn_to_face(_G(my_walker), 3, 32);
		ws_turn_to_face(_mcMach, 10, 31);

		break;

	case 31:
		setGlobals1(_meiHandsBehindBack, 1, 17, 17, 17, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_mcMach, -1);

		break;

	case 32:
		setGlobals1(_ripPos3LookAround, 1, 19, 19, 19, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 33);
		if (strlen(seriesName1) > 0)
			digi_play(seriesName1, 1, 255, -1, 997);
		break;

	case 33:
		sendWSMessage_150000(_mcMach, -1);
		setGlobals1(_meiSpookedPos2, 1, 13, 13, 13, 0, 13, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_mcMach, 34);
		digi_play(seriesName2, 1, 255, -1, 997);

		break;

	case 34:
		sendWSMessage_120000(_mcMach, 35);

		break;

	case 35:
		sendWSMessage_150000(_mcMach, -1);
		setGlobals1(_meiHandsBehindBack, 17, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_mcMach, 36);
		sendWSMessage_140000(_G(my_walker), -1);

		break;

	case 36:
		sendWSMessage_150000(_mcMach, 37);

		break;

	case 37:
		ws_turn_to_face(_mcMach, 10, -1);
		player_set_commands_allowed(true);

		break;

	default:
		break;
	}
}

void Room805::daemonSub4(const char *seriesName1) {
	switch (_G(kernel).trigger) {
	case 30:
		ws_turn_to_face(_mcMach, 3, 31);

		break;

	case 31:
		setGlobals1(_ripPos3LookAround, 1, 19, 19, 19, 0, 19, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 32);
		break;

	case 32:
		digi_play(seriesName1, 1, 255, 33, 997);

		break;

	case 33:
		sendWSMessage_120000(_G(my_walker), 34);

		break;

	case 34:
		sendWSMessage_150000(_G(my_walker), -1);
		player_set_commands_allowed(true);

		break;

	default:
		break;
	}
}

void Room805::parserSub1(const char *name, uint channel, int32 vol, int32 trigger, int32 room_num) {
	if (_G(kernel).trigger == -1) {
		_unkFlag1 = true;
		digi_play(name, channel, vol, trigger, room_num);
	} else
		_unkFlag1 = false;
}

void Room805::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	LoadWSAssets("other script", _G(master_palette));
}

void Room805::init() {
	digi_preload("950_s29", -1);
	_meiHandsBehindBack = series_load("MEI CHIEN HANDS BEHIND BACK", -1, nullptr);
	_meiSpookedPos2 = series_load("MEI SPOOKED POS2", -1, nullptr);
	_meiTalkPos3 = series_load("Mei talks pos3", -1, nullptr);
	_ripPos3LookAround = series_load("RIP POS 3 LOOK AROUND", -1, nullptr);
	_ripHeadTurnPos3 = series_load("RIP TREK HEAD TURN POS3", -1, nullptr);
	_ripLookDownPos3 = series_load("RIP TREK LOOK DOWN POS3", -1, nullptr);
	_ripLowReachPos2 = series_load("RIP TREK LOW REACH POS2", -1, nullptr);
	_ripTalkerPos3 = series_load("RIP TREK TALKER POS3", -1, nullptr);
	_ripMedHiReachPos2 = series_load("RIP MED HI REACH POS2", -1, nullptr);
	_ripMedHiReachPos3 = series_load("RIP MED HI REACH POS3", -1, nullptr);

	if (_G(flags)[V276] == 0) {
		ws_walk_load_walker_series(S8_SHADOW_DIRS1, S8_SHADOW_NAMES1, false);
		ws_walk_load_walker_series(S8_SHADOW_DIRS2, S8_SHADOW_NAMES2, false);
	} else {
		hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN", false);
	}

	if (!player_been_here(809) && _G(flags)[V276] == 0)
		kernel_timing_trigger(29, imath_ranged_rand(3600, 7200));

	initSub2();

	digi_play_loop("950_s29", 3, 127, -1, 950);
}

void Room805::parser() {
	bool lookFl = player_said_any("look", "look at");
	bool takeFl = player_said("take");
	bool talkFl = player_said_any("talk", "talk to");
	bool gearFl = player_said("gear");
	bool goFl = player_said("go");

	if (lookFl && player_said(" "))
		parserSub1("805R01", 1, 255, 1, -1);

	else if (lookFl && (player_said("JADE DOOR") || player_said("JADE DOOR ")))
		parserSub1("805R02", 1, 255, 1, -1);

	else if (lookFl && (player_said("URN") || player_said("URN ")))
		parserSub1("COM060", 1, 255, 1, 997);

	else if (lookFl && player_said("UNLIT URN"))
		parserSub1("COM061", 1, 255, 1, 997);

	else if (lookFl && player_said("SOLDIER")) {
		if (_G(flags[V257]))
			parserSub1("COM074", 1, 255, 1, -1);
		else if (_G(flags[V258]))
			parserSub1("805R10a", 1, 255, 1, -1);
		else
			parserSub1("805R10", 1, 255, 1, -1);
	} // if (lookFl && player_said("SOLDIER"))

	else if (lookFl && player_said("SOLDIER ")) {
		if (_G(flags[V258]))
			parserSub1("COM074", 1, 255, 1, -1);
		else if (_G(flags[V257]))
			parserSub1("805R10a", 1, 255, 1, -1);
		else
			parserSub1("805R10", 1, 255, 1, -1);
	} // if (lookFl && player_said("SOLDIER "))

	else if (lookFl && (player_said("SHIELD") || player_said("SHIELD ")))
		parserSub1("805R11", 1, 255, 1, -1);

	else if (lookFl && (player_said("MURAL") || player_said("BROKEN BEAM")))
		parserSub1("805R12", 1, 255, 1, -1);

	else if (lookFl && player_said("CHARIOT"))
		parserSub1("805R13", 1, 255, 1, -1);

	else if (lookFl && player_said("MEI CHEN"))
		parserSub1("COM043", 1, 255, 1, 997);

	else if (lookFl && player_said("WOODEN POST") && inv_object_is_here("WOODEN POST")) {
		_unkFlag1 = true;
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			setGlobals1(_unkSeries1, 1, 9, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), 10);
			digi_stop(1);

			break;

		case 2:
			player_set_commands_allowed(true);
			_unkFlag1 = false;

			break;

		case 10:
			sendWSMessage_140000(_G(my_walker), -1);
			digi_play("805r07", 1, 255, 2, -1);

			break;

		default:
			break;
		}

	} // if (lookFl && player_said("WOODEN POST") && inv_object_is_here("WOODEN POST"))

	else if (lookFl && player_said("HOLE IN ROOF")) {
		if (_G(flags[V261]) != 1 && _G(flags[V276]) == 0 && _G(flags[V256]) != 1) {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				ws_hide_walker(_G(my_walker));
				_unkSeries4 = series_load("805 RIP LOOKS UP", -1, nullptr);
				_ripSiftsDirtMach = series_play("805 RIP LOOKS UP", 256, 16, 2, 5, 0, 100, 0, 0, 0, 11);
				digi_stop(1);

				break;

			case 2:
				terminateMachine(_ripSiftsDirtMach);
				_ripSiftsDirtMach = series_play("805 RIP LOOKS UP", 256, 18, 3, 5, 0, 100, 0, 0, 0, 11);

				break;

			case 3:
				ws_unhide_walker(_G(my_walker));
				terminateMachine(_ripSiftsDirtMach);
				setGlobals1(_ripHeadTurnPos3, 1, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
				sendWSMessage_110000(_G(my_walker), -1);
				digi_play("805R03", 1, 255, 4, -1);

				break;

			case 4:
				series_unload(_unkSeries4);
				sendWSMessage_140000(_G(my_walker), 5);
				ws_turn_to_face(_mcMach, 4, -1);

				break;

			case 5:
				ws_hide_walker(_G(my_walker));
				_ripSiftsDirtSeries = series_load("RIP SIFTS DIRT WITH HAND", -1, nullptr);
				_ripSiftsDirtMach = series_play("RIP SIFTS DIRT WITH HAND", 256, 16, 6, 5, 0, 100, 0, 0, 0, -1);
				break;

			case 6:
				digi_play("805R04", 1, 255, 7, -1);

				break;

			case 7:
				_meiTrekTalkerSeries = series_load("MEI TREK TALKER POS4", -1, nullptr);
				setGlobals1(_meiTrekTalkerSeries, 1, 4, 1, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
				sendWSMessage_110000(_mcMach, -1);
				digi_play("805M01", 1, 255, 8, -1);

				break;

			case 8:
				sendWSMessage_140000(_mcMach, -1);
				terminateMachine(_ripSiftsDirtMach);
				_ripSiftsDirtMach = series_play("RIP SIFTS DIRT WITH HAND", 256, 18, 9, 5, 0, 100, 0, 0, 0, 25);

				break;

			case 9:
				digi_play("805R05", 1, 255, 10, -1);

				break;

			case 10:
				terminateMachine(_ripSiftsDirtMach);
				ws_unhide_walker(_G(my_walker));
				setGlobals1(_ripHeadTurnPos3, 6, 9, 9, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
				sendWSMessage_110000(_G(my_walker), 12);

				break;

			case 12:
				series_unload(_ripSiftsDirtSeries);
				kernel_timing_trigger(15, 13, nullptr);

				break;

			case 13:
				sendWSMessage_140000(_G(my_walker), 14);
				break;

			case 14:
				setGlobals1(_ripHeadTurnPos3, 1, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
				sendWSMessage_110000(_G(my_walker), -1);
				digi_play("805R05A", 1, 255, 16, -1);

				break;

			case 16:
				ws_walk(_mcMach, 276, 201, nullptr, 100, 10, true);
				sendWSMessage_140000(_G(my_walker), 17);
				break;

			case 17:
				series_unload(_ripHeadTurnPos3);
				series_unload(_meiTrekTalkerSeries);
				_G(flags[V261]) = 1;
				ws_walk(_mcMach, 276, 201, nullptr, 100, 10, true);
				player_set_commands_allowed(true);
				break;

			default:
				break;
			}
		} else if (_G(flags[V256]) == 1) {
			parserSub1("805R06A", 1, 255, 1, -1);
		} else {
			parserSub1("805R06", 1, 255, 1, -1);
		}
	} // if (lookFl && player_said("HOLE IN ROOF")

	else if (takeFl && (player_said("SOLDIER") || player_said("SOLDIER ")))
		parserSub1("805R15", 1, 255, 1, -1);

	else if (takeFl && (player_said("URN") || player_said("URN ")))
		parserSub1("COM067", 1, 255, 1, 997);

	else if (takeFl && player_said("UNLIT URN"))
		parserSub1("COM068", 1, 255, 1, 997);

	else if (takeFl && player_said("CHARIOT"))
		parserSub1("805R36", 1, 255, 1, -1);

	else if (takeFl && player_said("SHIELD")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			setGlobals1(_unkSeries5, 1, 13, 13, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), 10);
			digi_stop(1);

			break;

		case 10:
			if (_unkFlag1)
				kernel_timing_trigger(5, 1, nullptr);
			_G(flags[V257]) = 1;

			if (_G(flags[V258]) == 0) {
				kernel_examine_inventory_object("PING SOLDIER'S SHIELD", _G(master_palette), 5, 1, 310, 130, 20, nullptr, 1);
				inv_give_to_player("SOLDIER'S SHIELD");
				inv_put_thing_in("TWO SOLDIERS' SHIELDS", 999);
			} else {
				kernel_examine_inventory_object("PING SOLDIER'S SHIELD", _G(master_palette), 5, 1, 310, 130, 20, nullptr, 1);
				inv_put_thing_in("SOLDIER'S SHIELD", 999);
				inv_give_to_player("TWO SOLDIERS' SHIELDS");
			}

			terminateMachine(_farSoldiersShieldMach);

			break;

		case 20:
			sendWSMessage_140000(_G(my_walker), 30);
			break;

		case 30:
			hotspot_set_active(_G(currentSceneDef).hotspots, "SHIELD", false);
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
	} // if (takeFl && player_said("SHIELD"))

	else if (takeFl && player_said("SHIELD ")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			setGlobals1(_unkSeries6, 1, 12, 12, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), 10);
			digi_stop(1);

			break;

		case 10:
			if (_unkFlag1)
				kernel_timing_trigger(5, 1, nullptr);
			_G(flags[V258]) = 1;

			if (_G(flags[V257]) == 0) {
				kernel_examine_inventory_object("PING SOLDIER'S SHIELD", _G(master_palette), 5, 1, 330, 170, 20, nullptr, 1);
				inv_give_to_player("SOLDIER'S SHIELD");
				inv_put_thing_in("TWO SOLDIERS' SHIELDS", 999);
			} else {
				kernel_examine_inventory_object("PING SOLDIER'S SHIELD", _G(master_palette), 5, 1, 330, 170, 20, nullptr, 1);
				inv_put_thing_in("SOLDIER'S SHIELD", 999);
				inv_give_to_player("TWO SOLDIERS' SHIELDS");
			}

			terminateMachine(_nearSoldiersShieldMach);

			break;

		case 20:
			sendWSMessage_140000(_G(my_walker), 30);
			break;

		case 30:
			hotspot_set_active(_G(currentSceneDef).hotspots, "SHIELD ", false);
			player_set_commands_allowed(true);

			break;

		default:
			break;
		}
	} // if (takeFl && player_said("SHIELD "))

	else if (takeFl && player_said("WOODEN POST")) {
		switch (_G(kernel).trigger) {
		case -1:
			ws_demand_facing(_G(my_walker), 2);
			if (inv_object_is_here("WOODEN POST")) {
				player_set_commands_allowed(false);
				player_update_info(_G(my_walker), &_G(player_info));
				player_set_commands_allowed(false);
				setGlobals1(_unkSeries7, 1, 16, 16, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
				sendWSMessage_110000(_G(my_walker), 10);
			}

			digi_stop(1);

			break;

		case 10:
			inv_give_to_player("WOODEN POST");
			_unkFlag1 = true;
			kernel_examine_inventory_object("PING WOODEN POST", _G(master_palette), 5, 1, 386, 279, 20, nullptr, -1);
			terminateMachine(_fallenBeamOnFloorMach);

			break;

		case 20:
			sendWSMessage_140000(_G(my_walker), 30);
			break;

		case 30:
			_unkFlag1 = false;
			hotspot_set_active(_G(currentSceneDef).hotspots, "WOODEN POST", false);
			player_set_commands_allowed(true);

			break;

		default:
			break;
		}
	} // if (takeFl && player_said("WOODEN POST"))

	else if (takeFl && _unkFlag1) {
		switch (imath_ranged_rand(1, 6)) {
		case 1:
			parserSub1("COM006", 1, 255, 1, 997);
			break;

		case 2:
			parserSub1("COM007", 1, 255, 1, 997);
			break;

		case 3:
			parserSub1("COM008", 1, 255, 1, 997);
			break;

		case 4:
			parserSub1("COM009", 1, 255, 1, 997);
			break;

		case 5:
			parserSub1("COM010", 1, 255, 1, 997);
			break;

		case 6:
			parserSub1("COM011", 1, 255, 1, 997);
			break;

		default:
			break;
		}
	} // if (takeFl && _unkFlag1)

	else if (talkFl && player_said("MEI CHEN")) {
		player_set_commands_allowed(false);
		_G(kernel.trigger_mode) = KT_DAEMON;
		switch (imath_ranged_rand(1, 4)) {
		case 1:
			digi_play("COM044", 1, 255, 63, 997);
			break;

		case 2:
			digi_play("COM045", 1, 255, 63, 997);
			break;

		case 3:
			digi_play("COM046", 1, 255, 63, 997);
			break;

		case 4:
			digi_play("COM047", 1, 255, 63, 997);
			break;

		default:
			break;
		}

		ws_demand_facing(_G(my_walker), 3);
		setGlobals1(_unkSeries8, 1, 5, 1, 5, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), -1);
	} // if (talkFl && player_said("MEI CHEN"))

	else if (player_said("go", "jade door"))
		parserSub1("805r34", 1, 255, 1, -1);

	else if (gearFl && (player_said("JADE DOOR") || player_said("JADE DOOR ")) && (_G(flags[V271]) == 1))
		parserSub1("805R18", 1, 255, 1, -1);

	else if (gearFl && (player_said("JADE DOOR") || player_said("JADE DOOR ")) && (_G(flags[V272]) == 1))
		parserSub1("805R19", 1, 255, 1, -1);

	else if (gearFl && player_said("CHARIOT")) {
		if (_G(flags[V272])!= 1) {
			parserSub1("805R20", 1, 255, 1, -1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				ws_hide_walker(_G(my_walker));
				terminateMachine(_chariotRestMach);
				digi_preload("950_s33", -1);
				_ripSiftsDirtMach = series_stream("805rp04a", 5, 0, -1);
				series_stream_break_on_frame(_ripSiftsDirtMach, 15, 7);
				digi_stop(1);

				break;

			case 2:
				adv_kill_digi_between_rooms(false);
				digi_play_loop("950_s29", 3, 180, -1, 950);
				digi_play_loop("950_s33", 2, 255, -1, -1);
				_G(flags[V270]) = 806;
				_G(flags[V262]) = 0;
				_G(game).new_room = 806;

				break;

			case 7:
				series_stream_break_on_frame(_ripSiftsDirtMach, 50, 10);
				digi_play_loop("950_s33", 2, 255, -1, -1);

				break;

			case 10:
				disable_player_commands_and_fade_init(2);
				break;

			default:
				break;

			}
		}
	} // if (gearFl && player_said("CHARIOT"))

	warning("STUB - Room805::parser");

	_G(player).command_ready = false;

}

void Room805::daemon() {
	switch (_G(kernel).trigger) {
	case 29:
		if (!_unkFlag1 && player_commands_allowed() && !_G(player).need_to_walk && !_G(player).ready_to_walk && !_G(player).waiting_for_walk) {
			if (player_been_here(809))
				kernel_timing_trigger(imath_ranged_rand(3600, 7200), 29, nullptr);

			player_set_commands_allowed(false);

			_lastRnd = imath_ranged_rand(1, 4);
			switch (_lastRnd) {
			case 1:
				digi_play("950_s15", 1, 255, 30, -1);
				break;

			case 2:
				digi_play("950_s16", 1, 255, 30, -1);
				break;

			case 3:
				digi_play("950_s17", 1, 255, 30, -1);
				break;

			default:
				digi_play("950_s18", 1, 255, 30, -1);
				break;
			}
		} else
			kernel_timing_trigger(60, 39, nullptr);

		break; // case 29

	case 39:
		if (!_unkFlag1 && player_commands_allowed() && !_G(player).need_to_walk && !_G(player).ready_to_walk && !_G(player).waiting_for_walk)
			kernel_timing_trigger(1, 29, nullptr);
		else
			kernel_timing_trigger(60, 39, nullptr);

		break; // case 39

	case 55:
		sendWSMessage_150000(_mcMach, -1);
		player_set_commands_allowed(true);

		break; // case 55

	case 56:
		sendWSMessage_150000(_mcMach, -1);

		break; // case 56

	case 63:
		sendWSMessage_150000(_G(my_walker), -1);
		switch (imath_ranged_rand(1, 4)) {
		case 1:
			digi_play("COM048", 1, 255, 55, 997);
			break;

		case 2:
			digi_play("COM049", 1, 255, 55, 997);
			break;

		case 3:
			digi_play("COM050", 1, 255, 55, 997);
			break;

		case 4:
			digi_play("COM051", 1, 255, 55, 997);
			break;

		default:
			break;
		}
		_unkFlag1 = false;
		setGlobals1(_meiTalkPos3, 1, 4, 1, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_mcMach, -1);

		break; // case 63

	case 100:
	case 101:
	case 102:
	case 103:
	case 104:
	case 105:
	case 108:
	case 109:
	case 111:
		daemonSub1();

		break;

	case 110:
		digi_preload("950_s33", -1);
		digi_play_loop("950_s33", 2, 255, -1, -1);
		_ripChariotInMach = series_stream("805 RIP CHARIOT IN", 5, 1280, 111);
		_G(flags[V262]) = 1;

		break; // case 110

	default:
		if (_G(kernel).trigger > 29 && _G(kernel).trigger < 39) {
			switch (_lastRnd) {
			case 1:
				daemonSub2("COM052", "COM053");

				break;
			case 2:
				daemonSub3("COM054", "COM055");

				break;
			case 3:
				daemonSub4("COM056");

				break;
			case 4:
				daemonSub2("COM057", "COM058");

				break;
			default:
				break;
			}
		}

		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
