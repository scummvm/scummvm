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

#include "m4/riddle/rooms/section8/room808.h"

#include "m4/core/errors.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/riddle.h"
#include "m4/riddle/vars.h"
#include "m4/wscript/wst_regs.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room808::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
}

void Room808::init() {
	player_set_commands_allowed(false);

	ws_walk_load_shadow_series(S8_SHADOW_DIRS1, S8_SHADOW_NAMES1);
	ws_walk_load_walker_series(S8_SHADOW_DIRS2, S8_SHADOW_NAMES2);

	_808Rp02Series = series_load("808rp02", -1, nullptr);
	_808Rp01Series = series_load("808rp01", -1, nullptr);
	_ripTalkerPos5Series = series_load("RIP TALKER POS 5", -1, nullptr);
	_rptmr15Series = series_load("RPTMR15", -1, nullptr);
	_ripMedReach1HandPos2Series = series_load("RIP MED REACH 1HAND POS2", -1, nullptr);
	_807Rp04Series = series_load("807rp04", -1, nullptr);
	_mctd61Series = series_load("mctd61", -1, nullptr);
	_mctd82aSeries = series_load("mctd82a", -1, nullptr);
	_ripPos3LookAroundSeries = series_load("RIP POS 3 LOOK AROUND", -1, nullptr);
	_ripLooksAroundInAweSeries = series_load("RIP LOOKS AROUND IN AWE", -1, nullptr);
	series_load("808mc98", -1, nullptr);
	series_load("808mc99", -1, nullptr);
	series_load("808pos1", -1, nullptr);
	series_load("808pos2", -1, nullptr);

	if (_G(flags[V096]) == 0) {
		// Unload the freshly loaded series? This doesn't make sense?!
		series_unload(series_load("808 RIP TEST BRIDGESHOVEL FAR", -1, nullptr));
		series_unload(series_load("808 RIP TEST BRIDGE", -1, nullptr));
		series_unload(series_load("808test3", -1, nullptr));
	}

	_808ChainMach = series_plain_play("808chain", -1, 0, 100, 0, 0, -1, true);
	series_play("LIT URN ", 767, 0, -1, 5, -1, 100, 0, 0, 0, -1);
	if (inv_object_in_scene("FARMER'S SHOVEL", 808)) {
		if (_G(flags[V095])) {
			switch (_G(flags[V094])) {
			case 1:
				_808PosMach = series_show("808pos2", 1281, 0, -1, -1, 6, 100, 0, 0);
				break;

			case 2:
				_808PosMach = series_show("808pos2", 1281, 0, -1, -1, 4, 100, 0, 0);
				break;

			case 3:
				_808PosMach = series_show("808pos2", 1281, 0, -1, -1, 3, 100, 0, 0);
				break;

			case 4:
				_808PosMach = series_show("808pos2", 1281, 0, -1, -1, 5, 100, 0, 0);
				break;

			default:
				break;
			}
		} else {
			_808PosMach = series_show("808pos2", 1281, 0, -1, -1, _G(flags[V094]), 100, 0, 0);
		}
	} else {
		if (_G(flags[V095])) {
			switch (_G(flags[V094])) {
			case 1:
				_808PosMach = series_show("808pos1", 1281, 0, -1, -1, 6, 100, 0, 0);
				break;

			case 2:
				_808PosMach = series_show("808pos1", 1281, 0, -1, -1, 4, 100, 0, 0);
				break;

			case 3:
				_808PosMach = series_show("808pos1", 1281, 0, -1, -1, 3, 100, 0, 0);
				break;

			case 4:
				_808PosMach = series_show("808pos1", 1281, 0, -1, -1, 5, 100, 0, 0);
				break;

			default:
				break;
			}
		} else {
			_808PosMach = series_show("808pos1", 1281, 0, -1, -1, _G(flags[V094]), 100, 0, 0);
		}
	}

	if (inv_object_in_scene("crank", 808) && _G(flags[V098]) == 0) {
		_808HandleSpriteMach = series_show("808 handle sprite", 4095, 0, -1, -1, 0, 100, 0, 0);
	}

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		digi_preload("950_s29", -1);
		if (_G(flags[V097])) {
			_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 345, 115, 5, Walker::player_walker_callback, "mc_trek");
		} else {
			_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 112, 238, 5, Walker::player_walker_callback, "mc_trek");
		}

		kernel_timing_trigger(1, 1, nullptr);
		break;

	case 809:
		ws_demand_facing(_G(my_walker), 5);
		ws_demand_location(_G(my_walker), 459, 36);
		ws_walk(_G(my_walker), 382, 116, nullptr, -1, 7, true);
		_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 448, 38, 5, Walker::player_walker_callback, "mc_trek");
		DisposePath(_mcTrekMach->walkPath);
		_mcTrekMach->walkPath = CreateCustomPath(427, 96, 345, 115, -1);
		ws_custom_walk(_mcTrekMach, 5, 1, true);

		break;

	default:
		_808RpupSeries = series_load("808rpup", -1, nullptr);
		ws_demand_facing(_G(my_walker), 2);
		ws_demand_location(_G(my_walker), 18, 216);
		ws_hide_walker(_G(my_walker));

		if (_G(flags[V276]) == 0) {
			_808McupSeries = series_load("808mcup", -1, nullptr);
			series_play("808rpup", 0, 0, 2, 5, 0, 100, 0, 0, 0, -1);
		} else {
			_G(flags[V276]) = 0;
			_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 345, 116, 5, Walker::player_walker_callback, "mc_trek");
			series_play("808rpup", 0, 0, 4, 5, 0, 100, 0, 0, 0, -1);
		}

		break;

	}

	digi_play_loop("950_s29", 3, 96, -1, -1);
}

void Room808::pre_parser() {
	if (inv_player_has(_G(player).noun)) {
		return;
	}

	int32 opCode = -1;

	if (player_said_any("look", "look at"))
		opCode = 1;
	else if (player_said_any("gear", "use"))
		opCode = 0;
	else if (player_said("take"))
		opCode = 2;
	else if (player_said("talk to"))
		opCode = 3;
	else if (player_said("go"))
		opCode = 5;
	else if (player_said("crank"))
		opCode = 6;
	else if (player_said("journal"))
		opCode = 11;

	bool doneFl = true;

	switch (opCode) {
	case 0:
		if (player_said_any("wheel", "wheel ", "wheel  ", "wheel   ")) {
			_G(player).need_to_walk = false;
			_G(player).ready_to_walk = true;
			_G(player).waiting_for_walk = false;
		} else
			doneFl = false;

		break;

	case 1:
		if (player_said("slot") && _G(flags[V097]) == 0) {
			_G(player).walk_x = 202;
			_G(player).walk_y = 179;
			_G(player).walk_facing = 2;
		} else if (player_said_any("mei chen", "east") || scumm_stricmp(_G(player).noun, "wheel") || scumm_stricmp(_G(player).noun, "bridge")) {
			_G(player).need_to_walk = false;
			_G(player).ready_to_walk = true;
			_G(player).waiting_for_walk = false;
		} else
			doneFl = false;

		break;
	case 5:
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;

		break;

	case 11:
		// Nothing
		break;

	default:
		doneFl = false;
	}


	if (doneFl || _G(flags[V100]))
		return;

	if (!getWalkPath(_G(my_walker), _G(player).walk_x, _G(player).walk_y))
		return;

	player_update_info(_G(my_walker), &_G(player_info));
	const bool check1Fl = intr_PathCrossesLine(_G(player_info).x, _G(player_info).y, _G(my_walker)->walkPath, 242, 139, 295, 149);
	const bool check2Fl = intr_PathCrossesLine(_G(player_info).x, _G(player_info).y, _G(my_walker)->walkPath, 170, 158, 223, 168);

	DisposePath(_G(my_walker)->walkPath);
	_G(my_walker)->walkPath = nullptr;

	if (!check1Fl && !check2Fl)
		return;

	if (_G(flags[V094]) != 0 && _G(flags[V094]) != 4) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	} else if (inv_object_in_scene("FARMER'S SHOVEL", 808) && (_G(flags[V094]) == 0 || _G(flags[V100]) == 0)) {
		intr_cancel_sentence();
		_G(kernel).trigger_mode = KT_DAEMON;
		if (_G(flags[V096]) == 0) {
			ws_walk(_G(my_walker), 185, 165, nullptr, 6, 2, true);
		} else {
			ws_walk(_G(my_walker), 192, 163, nullptr, 10, 2, true);
		}
		_G(kernel).trigger_mode = KT_PREPARSE;
	} else if (inv_object_in_scene("FARMER'S SHOVEL", 808) && _G(flags[V094]) == 4 && check1Fl) {
		intr_cancel_sentence();
		_G(kernel).trigger_mode = KT_DAEMON;
		ws_walk(_G(my_walker), 274, 142, nullptr, 8, 2, true);
		_G(kernel).trigger_mode = KT_PREPARSE;
	}

}

void Room808::parser() {
	// TODO Not implemented yet
}

void Room808::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		player_set_commands_allowed(true);
		setBridgeHotspots(0, false);
		setBridgeHotspots(1, false);
		setBridgeHotspots(2, false);
		setBridgeHotspots(3, false);
		setBridgeHotspots(4, false);
		setBridgeHotspots(_G(flags[V094]), true);
		addMcHotspot(_G(flags[V097]));

		if (_G(flags[V097]) == 0) {
			_meiChienHandsBehindBackSeries = series_load("MEI CHIEN HANDS BEHIND BACK", -1, nullptr);
			setGlobals3(_meiChienHandsBehindBackSeries, 1, 17);
			subD7916(_mcTrekMach, -1);
		}

		kernel_timing_trigger(imath_ranged_rand(1200, 1800), 18);

		break;

	case 2:
		ws_unhide_walker(_G(my_walker));
		ws_demand_location(_G(my_walker), 18, 216);
		ws_demand_facing(_G(my_walker), 2);
		DisposePath(_G(my_walker)->walkPath);
		_G(my_walker)->walkPath = CreateCustomPath(29, 209, 67, 209, 112, 195, -1);
		ws_custom_walk(_G(my_walker), 2, -1, true);
		series_unload(_808McupSeries);
		series_play("808mcup", 0, 0, 3, 5, 0, 100, 0, 0, 0, -1);

		break;

	case 3:
		_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 18, 216, 2, Walker::player_walker_callback, "mc_trek");
		DisposePath(_mcTrekMach->walkPath);
		_mcTrekMach->walkPath = CreateCustomPath(31, 211, 66, 215, 112, 238, -1);
		ws_custom_walk(_mcTrekMach, 2, 1, true);
		series_unload(_808McupSeries);

		break;

	case 4:
		ws_unhide_walker(_G(my_walker));
		ws_demand_location(_G(my_walker), 18, 216);
		ws_demand_facing(_G(my_walker), 2);
		DisposePath(_G(my_walker)->walkPath);
		_G(my_walker)->walkPath = CreateCustomPath(29, 209, 67, 209, 112, 195, -1);
		ws_custom_walk(_G(my_walker), 2, 1, true);

		break;

	case 6:
		player_set_commands_allowed(false);
		ws_hide_walker(_G(my_walker));
		terminateMachine(_808PosMach);
		_G(flags[V096]) = 1;

		if (inv_object_in_scene("farmer's shovel", 808)) {
			series_play("808 RIP TEST BRIDGESHOVEL FAR", 1, 0, 7, 5, 0, 100, 0, 0, 0, -1);
		} else if (_G(flags[V094]) == 4) {
			series_play("808test3", 1, 0, 7, 5, 0, 100, 0, 0, 0, -1);
		} else {
			series_play("808 RIP TEST BRIDGE", 1, 0, 7, 5, 0, 100, 0, 0, 0, -1);
		}

		break;

	case 7:
		player_set_commands_allowed(true);
		ws_unhide_walker(_G(my_walker));
		ws_demand_facing(_G(my_walker), 2);

		if (inv_object_in_scene("farmer's shovel", 808)) {
			_808PosMach = series_show("808pos2", 1281, 0, -1, -1, 0, 100, 0, 0);
		} else if (_G(flags[V094]) == 4) {
			_808PosMach = series_show("808pos1", 1281, 0, -1, -1, 4, 100, 0, 0);
		} else {
			_808PosMach = series_show("808pos1", 1281, 0, -1, -1, 0, 100, 0, 0);
		}


		break;

	case 8:
		player_set_commands_allowed(false);
		ws_hide_walker(_G(my_walker));
		terminateMachine(_808PosMach);
		digi_preload("808_s04", -1);
		digi_preload("808_s02", -1);
		digi_preload("28_02n01", 807);
		_808RipFallShovelNearSideMach = series_stream("808 RIP FALLSHOVEL NEAR SIDE ", 5, 0, -1);
		series_stream_break_on_frame(_808RipFallShovelNearSideMach, 5, 15);
		digi_play("808_s04", 3, 255, -1, -1);

		break;

	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
	case 18:
	case 19:
	case 20:
	case 21:
	case 22:
	case 23:
	case 24:
	case 25:
	case 26:
	case 966:
	case 967:

	default:
		break;
	}
	// TODO Not implemented yet
}

bool Room808::getWalkPath(machine *machine, int32 walk_x, int32 walk_y) {
	if (machine == nullptr || machine->myAnim8 == nullptr) {
		error_show(FL, 514, "ws_walk");
	}

	const int32 currPos_x = machine->myAnim8->myRegs[IDX_X] >> 16;
	const int32 currPos_y = machine->myAnim8->myRegs[IDX_Y] >> 16;
	Buffer* currBuffer = _G(screenCodeBuff)->get_buffer();

	const int32 currNode = AddRailNode(currPos_x, currPos_y, currBuffer, true);

	if (currNode < 0) {
		error_show(FL, 520, "Walker's curr posn: %ld %ld", currPos_x, currPos_y);
	}

	const int32 destNode = AddRailNode(walk_x, walk_y, currBuffer, true);

	if (destNode < 0) {
		error_show(FL, 520, "Trying to walk to: %ld %ld", walk_x, walk_y);
	}

	if (machine->walkPath)
		DisposePath(machine->walkPath);

	bool retVal = GetShortestPath(currNode, destNode, &machine->walkPath);

	RemoveRailNode(currNode, currBuffer, true);
	RemoveRailNode(destNode, currBuffer, true);
	_G(screenCodeBuff)->release();

	return retVal;
}

void Room808::setBridgeHotspots(int val1, bool activeFl) {
	switch (val1) {
	case 0:
		hotspot_set_active(_G(currentSceneDef).hotspots, "wheel", activeFl);
		hotspot_set_active(_G(currentSceneDef).hotspots, "bridge", activeFl);

		hotspot_set_active(_G(currentSceneDef).hotspots, "hole in chasm wall", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "hole in bridge", false);

		if (inv_object_in_scene("FARMER'S SHOVEL", 808) || activeFl == false)
			hotspot_set_active(_G(currentSceneDef).hotspots, "FARMER'S SHOVEL", false);
		else
			hotspot_set_active(_G(currentSceneDef).hotspots, "FARMER'S SHOVEL", true);

		if (inv_object_in_scene("crank", 808) || _G(flags[V098]) != 0) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "crank", false);
			hotspot_set_active(_G(currentSceneDef).hotspots, "slot", true);
		} else {
			hotspot_set_active(_G(currentSceneDef).hotspots, "crank", true);
			hotspot_set_active(_G(currentSceneDef).hotspots, "slot", false);
		}

		break;

	case 1:
		hotspot_set_active(_G(currentSceneDef).hotspots, "wheel ", activeFl);
		hotspot_set_active(_G(currentSceneDef).hotspots, "bridge ", activeFl);
		hotspot_set_active(_G(currentSceneDef).hotspots, "hole in chasm wall", activeFl);

		hotspot_set_active(_G(currentSceneDef).hotspots, "hole in bridge", false);

		if (inv_object_in_scene("FARMER'S SHOVEL", 808) || activeFl == false)
			hotspot_set_active(_G(currentSceneDef).hotspots, "FARMER'S SHOVEL ", false);
		else
			hotspot_set_active(_G(currentSceneDef).hotspots, "FARMER'S SHOVEL ", true);

		hotspot_set_active(_G(currentSceneDef).hotspots, "crank", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "slot", false);

		break;

	case 2:
		hotspot_set_active(_G(currentSceneDef).hotspots, "wheel  ", activeFl);
		hotspot_set_active(_G(currentSceneDef).hotspots, "bridge  ", activeFl);
		hotspot_set_active(_G(currentSceneDef).hotspots, "hole in chasm wall", activeFl);

		hotspot_set_active(_G(currentSceneDef).hotspots, "hole in bridge", false);

		if (inv_object_in_scene("FARMER'S SHOVEL", 808) || activeFl == false)
			hotspot_set_active(_G(currentSceneDef).hotspots, "FARMER'S SHOVEL  ", false);
		else
			hotspot_set_active(_G(currentSceneDef).hotspots, "FARMER'S SHOVEL  ", true);

		if (inv_object_in_scene("crank", 808) || _G(flags[V098]) != 0) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "crank", false);
			hotspot_set_active(_G(currentSceneDef).hotspots, "slot", true);
		} else {
			hotspot_set_active(_G(currentSceneDef).hotspots, "crank", true);
			hotspot_set_active(_G(currentSceneDef).hotspots, "slot", false);
		}

		break;

	case 3:
		hotspot_set_active(_G(currentSceneDef).hotspots, "wheel   ", activeFl);
		hotspot_set_active(_G(currentSceneDef).hotspots, "bridge   ", activeFl);
		hotspot_set_active(_G(currentSceneDef).hotspots, "hole in chasm wall", activeFl);
		hotspot_set_active(_G(currentSceneDef).hotspots, "hole in bridge", activeFl);

		if (inv_object_in_scene("FARMER'S SHOVEL", 808) || activeFl == false)
			hotspot_set_active(_G(currentSceneDef).hotspots, "FARMER'S SHOVEL   ", false);
		else
			hotspot_set_active(_G(currentSceneDef).hotspots, "FARMER'S SHOVEL   ", true);

		if (inv_object_in_scene("crank", 808) || _G(flags[V098]) != 0) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "crank", false);
			hotspot_set_active(_G(currentSceneDef).hotspots, "slot", true);
		} else {
			hotspot_set_active(_G(currentSceneDef).hotspots, "crank", true);
			hotspot_set_active(_G(currentSceneDef).hotspots, "slot", false);
		}

		break;

	case 4:
		hotspot_set_active(_G(currentSceneDef).hotspots, "wheel", activeFl);
		hotspot_set_active(_G(currentSceneDef).hotspots, "bridge", activeFl);

		hotspot_set_active(_G(currentSceneDef).hotspots, "hole in chasm wall", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "hole in bridge", false);

		if (inv_object_in_scene("FARMER'S SHOVEL", 808) || activeFl == false)
			hotspot_set_active(_G(currentSceneDef).hotspots, "FARMER'S SHOVEL    ", false);
		else
			hotspot_set_active(_G(currentSceneDef).hotspots, "FARMER'S SHOVEL    ", true);

		if (inv_object_in_scene("crank", 808) || _G(flags[V098]) != 0) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "crank", false);
			hotspot_set_active(_G(currentSceneDef).hotspots, "slot", true);
		} else {
			hotspot_set_active(_G(currentSceneDef).hotspots, "crank", true);
			hotspot_set_active(_G(currentSceneDef).hotspots, "slot", false);
		}

		break;

	default:
		break;
	}
}

void Room808::addMcHotspot(int32 val1) {
	for (HotSpotRec* curr = _G(currentSceneDef).hotspots; curr != nullptr; curr = curr->next) {
		if (scumm_stricmp(curr->vocab, "MEI CHEN")) {
			hotspot_delete_record(_G(currentSceneDef).hotspots, curr);
			break;
		}
	}

	HotSpotRec *newHotspot;
	switch (val1) {
	case 0:
		newHotspot = hotspot_new(100, 170, 124, 241);
		hotspot_newVerb(newHotspot, "TALK TO");
		hotspot_newVocab(newHotspot, "MEI CHEN");

		newHotspot->cursor_number = 7;
		newHotspot->feet_x = 128;
		newHotspot->feet_y = 221;
		newHotspot->facing = 7;

		break;

	case 1:
		newHotspot = hotspot_new(336, 74, 352, 117);
		hotspot_newVerb(newHotspot, "TALK TO");
		hotspot_newVocab(newHotspot, "MEI CHEN");

		newHotspot->cursor_number = 7;
		newHotspot->feet_x = 32767;
		newHotspot->feet_y = 32767;
		newHotspot->facing = 0;

		break;

	default:
		error("addMcHotspot - unexpected value");
	}

	_G(currentSceneDef).hotspots = hotspot_add(_G(currentSceneDef).hotspots, newHotspot, true);
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
