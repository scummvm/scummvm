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


#include "m4/adv_r/other.h"
#include "m4/core/errors.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/riddle.h"
#include "m4/riddle/vars.h"
#include "m4/wscript/wst_regs.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

static const char *SAFARI_SHADOWS_2[6] = {
	"", "safari shadow 1", "safari shadow 2", "safari shadow 3", "safari shadow 4", "safari shadow 5"
};


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
		} else if (player_said_any("mei chen", "east") || !scumm_strnicmp(_G(player).noun, "wheel", 5) || !scumm_strnicmp(_G(player).noun, "bridge", 6)) {
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
	_G(player).command_ready = false;

	if (_G(kernel).trigger == 747)
		return;

	if (inv_player_has(_G(player).noun) && _G(kernel).trigger == -1) {
		_G(player).command_ready = true;
		return;
	}

	int32 opCode = -1;
	if (player_said_any("look", "look at", "no walk"))
		opCode = 1;
	else if (player_said_any("gear", "use"))
		opCode = 0;
	else if (player_said("take"))
		opCode = 2;
	else if (player_said("talk to"))
		opCode = 3;
	else if (player_said_any("walk to", "spleen"))
		opCode = 4;
	else if (player_said("go"))
		opCode = 5;
	else if (player_said("crank"))
		opCode = 6;
	else if (player_said("FARMER'S SHOVEL"))
		opCode = 7;
	else if (player_said("wooden post"))
		opCode = 8;
	else if (player_said("wooden beam"))
		opCode = 9;
	else if (player_said("journal"))
		opCode = 11;
	else if (player_said("conv808a"))
		opCode = 10;

	switch (opCode) {
	case 0:
		if (player_said("chain")) {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				ws_hide_walker(_G(my_walker));
				terminateMachine(_808ChainMach);
				series_play("808rp03", 256, 0, 5, 5, 0, 100, 0, 0, 0, 25);

				break;

			case 5:
				_808RipFallShovelNearSideMach = series_play("808rp03", 256, 16, 10, 5, 0, 100, 0, 0, 26, -1);
				digi_play("808_s05", 2, 255, -1, -1);

				break;

			case 10:
				kernel_timing_trigger(60, 20, "gong pause");
				digi_play("808_s05a", 2, 255, -1, -1);

				break;

			case 20:
				terminateMachine(_808RipFallShovelNearSideMach);
				series_play("808rp03", 256, 2, 30, 5, 0, 100, 0, 0, 0, -1);

				break;

			case 30:
				player_set_commands_allowed(true);
				digi_play("808r19", 1, 255, -1, -1);
				ws_unhide_walker(_G(my_walker));
				ws_demand_facing(_G(my_walker), 10);
				_808ChainMach = series_plain_play("808chain", 1, 0, 100, 0, 0, -1, true);

				break;

			default:
				break;

			}
		} else if (player_said("wheel") && _G(flags[V100])) {
			switch (_G(kernel).trigger) {
			case -1: {
				bool walkCheck = true;
				if (getWalkPath(_G(my_walker), 185, 156)) {
					player_update_info(_G(my_walker), &_G(player_info));
					walkCheck = intr_PathCrossesLine(_G(player_info).x, _G(player_info).y, _G(my_walker)->walkPath, 242, 139, 295, 149);
				}

				if (walkCheck)
					ws_turn_to_face(_G(my_walker), 8, 5);
				else
					ws_walk(_G(my_walker), 169, 171, nullptr, 10, 2, true);

				}

				break;

			case 5:
				digi_play("com077", 1, 255, -1, 997);
				break;

			case 10:
				player_set_commands_allowed(false);
				ws_hide_walker(_G(my_walker));
				terminateMachine(_808PosMach);
				_808PosMach = series_play("808spn01", 1281, 16, 20, 5, 0, 100, 0, 0, 0, 13);

				break;

			case 20:
				kernel_timing_trigger(60, 30, nullptr);
				break;

			case 30:
				terminateMachine(_808PosMach);
				series_play("808spn01", 0, 2, 40, 5, 0, 100, 0, 0, 0, 13);
				digi_play("com078", 1, 255, -1, 997);

				break;

			case 40:
				player_set_commands_allowed(true);
				ws_unhide_walker(_G(my_walker));
				ws_demand_facing(_G(my_walker), 2);
				_808PosMach = series_show_sprite("808pos1", 0, 32767);

				break;

			default:
				break;
			}
		} else if (player_said_any("wheel", "wheel ", "wheel  ", "wheel   ")) {
			switch (_G(kernel).trigger) {
			case -1:
				if (_G(flags[V095])) {
					_dword1A1964_facing = 1;
					ws_walk(_G(my_walker), 157, 166, nullptr, 5, 1, true);
				} else {
					_dword1A1964_facing = 2;
					ws_walk(_G(my_walker), 169, 171, nullptr, 5, 2, true);
				}

				break;

			case 5:
				player_set_commands_allowed(false);
				setBridgeHotspots(_G(flags[V094]), false);
				ws_hide_walker(_G(my_walker));
				terminateMachine(_808PosMach);
				room808_sub1();
				_808PosMach = series_stream(_posMachName, 5, 1281, 10);
				series_stream_break_on_frame(_808PosMach, 7, _posMachFrameNum);
				player_update_info(_G(my_walker), &_G(player_info));
				_808RipFallShovelNearSideMach = series_place_sprite(SAFARI_SHADOWS_2[_G(player_info).facing], 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 1282);

				break;

			case 7:
				digi_play("808_s04", 2, 255, -1, -1);
				break;

			case 10:
				player_set_commands_allowed(true);
				setBridgeHotspots(_G(flags[V094]), true);
				ws_unhide_walker(_G(my_walker));
				ws_demand_facing(_G(my_walker), _dword1A1964_facing);

				_808PosMach = series_show(inv_object_in_scene("FARMER'S SHOVEL", 808) ? "808pos2" : "808pos1", 1281, 0, -1, -1, _posMachIndex, 100, 0, 0);

				terminateMachine(_808RipFallShovelNearSideMach);

				break;

			default:
				break;

			}
		} else if (player_said("crank") && inv_object_in_scene("crank", 808) && _G(flags[V100])) {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				setGlobals3(_ripMedReach1HandPos2Series, 1, 17);
				sendWSMessage_3840000(_G(my_walker), 10);

				break;

			case 10:
				kernel_timing_trigger(120, 20, nullptr);
				break;

			case 20:
				setGlobals3(_ripMedReach1HandPos2Series, 17, 1);
				sendWSMessage_3840000(_G(my_walker), 30);
				digi_play("808r32", 1, 255, -1, -1);

				break;

			case 30:
				player_set_commands_allowed(true);
				ws_demand_facing(_G(my_walker), 10);

				break;

			default:
				break;

			}
		}

		break;

	case 1:
		if (player_said(" ")) {
			digi_play("808r01", 1, 255, -1, -1);
		} else if (!scumm_strnicmp(_G(player).noun, "wheel", 5) || !scumm_strnicmp(_G(player).noun, "bridge", 6)) {
			switch (_G(kernel).trigger) {
			case -1:
				player_update_info(_G(my_walker), &_G(player_info));
				if (_G(player).x >= 300) {
					ws_turn_to_face(_G(my_walker), 9, 10);
				} else {
					ws_walk(_G(my_walker), 202, 179, nullptr, 10, !scumm_strnicmp(_G(player).noun, "bridge", 6) ? 1 : 11, true);
				}

				break;

			case 10:
				if (!scumm_strnicmp(_G(player).noun, "bridge", 6)) {
					digi_play("808r02", 1, 255, -1, -1);
				} else {
					digi_play("808r03", 1, 255, -1, -1);
				}

				break;

			default:
				break;
			}
		} else if (!scumm_strnicmp(_G(player).noun, "FARMER'S SHOVEL", 15) && inv_object_in_scene("FARMER'S SHOVEL", 808)) {
			digi_play("808r33", 1, 255, -1, -1);
		} else if (player_said("chain")) {
			digi_play("808r04", 1, 255, -1, -1);
		} else if (player_said("slot")) {
			digi_play("808r05", 1, 255, -1, -1);
		} else if (player_said("crank") && inv_object_in_scene("crank", 808)) {
			digi_play("808r06", 1, 255, -1, -1);
		} else if (player_said("lit urn")) {
			digi_play("808r07", 1, 255, -1, -1);
		} else if (player_said("unlit urn")) {
			digi_play("com061", 1, 255, -1, 997);
		} else if (player_said("soldier")) {
			digi_play("808r09", 1, 255, -1, -1);
		} else if (player_said ("broken feet")) {
			digi_play("808r10", 1, 255, -1, -1);
		} else if (player_said("large hole")) {
			digi_play("808r11", 1, 255, -1, -1);
		} else if (player_said_any("corridor", "east")) {
			digi_play("808r12", 1, 255, -1, -1);
		} else if (player_said("lower staircase")) {
			digi_play("808r13", 1, 255, -1, -1);
		} else if (player_said("chasm")) {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				player_update_info(_G(my_walker), &_G(player_info));
				if (_G(player_info).facing == 2) {
					setGlobals3(_808Rp02Series, 1, 17);
				} else {
					setGlobals3(_808Rp01Series, 1, 23);
				}

				sendWSMessage_3840000(_G(my_walker), 10);

				break;

			case 10:
				kernel_timing_trigger(90, 20, "rip peers");
				break;

			case 20:
				digi_play("808r14", 1, 255, -1, -1);
				if (_G(player_info).facing == 2) {
					setGlobals3(_808Rp02Series, 1, 17);
				} else {
					setGlobals3(_808Rp01Series, 1, 19);
				}

				sendWSMessage_3840000(_G(my_walker), 30);

				break;

			case 30:
				player_set_commands_allowed(true);
				ws_demand_facing(_G(my_walker), _dword1A1964_facing);

				break;

			default:
				break;
			}
		} else if (player_said("hole in bridge")) {
			switch (_G(kernel).trigger) {
			case -1:
				if (_G(flags[V278]) && _G(flags[V277]))
					digi_play("808r16", 1, 255, -1, -1);
				else {
					_G(flags[V278]) = 1;
					digi_play("808r15", 1, 255, _G(flags[V278]) ? 10 : -1, -1);
				}
				break;

			case 10:
				digi_play("808r15a", 1, 255, -1, -1);
				break;

			default:
				break;
			}
		} else if (player_said("hole in chasm wall")) {
			switch (_G(kernel).trigger) {
			case -1:
				if (_G(flags[V278]) && _G(flags[V277]))
					digi_play("808r16", 1, 255, -1, -1);
				else {
					_G(flags[V277]) = 1;
					digi_play("808r17", 1, 255, _G(flags[V278])  ? 10 : -1, -1);
				}
				break;

			case 10:
				digi_play("808r17a", 1, 255, -1, -1);
				break;

			default:
				break;
			}
		} else if (player_said("mei chen")) {
			digi_play("808r18", 1, 255, -1, -1);
		} else
			break;

		return;

	case 2:
		if (player_said("FARMER'S SHOVEL   ")) {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				ws_hide_walker(_G(my_walker));
				terminateMachine(_808PosMach);
				_808PosMach = series_show("808pos1", 1281, 0, -1, -1, 3, 100, 0, 0);
				_808RipFallShovelNearSideMach = series_play("808rp07", 1, 18, 10, 5, 0, 100, 0, 0, 0, -1);
				player_update_info(_G(my_walker), &_G(player_info));
				_safariShadowMach = series_place_sprite(SAFARI_SHADOWS_2[_G(player_info).facing], 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 257);

				break;

			case 10:
				inv_give_to_player("FARMER'S SHOVEL");
				hotspot_set_active(_G(currentSceneDef).hotspots, "FARMER'S SHOVEL   ", false);
				kernel_examine_inventory_object("PING FARMER'S SHOVEL", _G(master_palette), 5, 1, 125, 125, 20, nullptr, -1);

				break;

			case 20:
				player_set_commands_allowed(true);
				terminateMachine(_808RipFallShovelNearSideMach);
				terminateMachine(_safariShadowMach);
				ws_unhide_walker(_G(my_walker));
				ws_demand_facing(_G(my_walker), 2);

				break;

			default:
				break;
			}
		} else if ((player_said("FARMER'S SHOVEL") && inv_object_in_scene("FARMER'S SHOVEL", 808)) || player_said_any("FARMER'S SHOVEL ", "FARMER'S SHOVEL  ", "FARMER'S SHOVEL    ")) {
			digi_play("808r31", 1, 255, -1, -1);
		} else if (player_said("crank")) {
			if (_G(flags[V100])) {
				switch (_G(kernel).trigger) {
				case -1:
					if (!inv_object_in_scene("crank", 808)) {
						_G(player).command_ready = true;
						return;
					}

					player_set_commands_allowed(false);
					setGlobals3(_ripMedReach1HandPos2Series, 1, 17);
					sendWSMessage_3840000(_G(my_walker), 10);

					break;

				case 10:
					inv_give_to_player("CRANK");
					kernel_examine_inventory_object("PING CRANK", _G(master_palette), 5, 1, 280, 85, 20, nullptr, -1);

					break;

				case 20:
					terminateMachine(_808HandleSpriteMach);
					setGlobals3(_ripMedReach1HandPos2Series, 17, 1);
					sendWSMessage_3840000(_G(my_walker), 30);

					break;

				case 30:
					player_set_commands_allowed(true);
					inv_give_to_player("crank");
					hotspot_set_active(_G(currentSceneDef).hotspots, "slot", true);
					hotspot_set_active(_G(currentSceneDef).hotspots, "crank", false);
					ws_demand_facing(_G(my_walker), 10);

					break;

				default:
					break;
				}
			}
		} else
			break;

		return;

	case 3:
		if (_G(flags[V097]) == 0 && _G(flags[V094]) == 4 && inv_object_in_scene("FARMER'S SHOVEL", 808)) {
			conv_load("conv808a", 10, 10, 747);
			conv_export_value_curr(_G(flags[V098]), 0);
			conv_play(conv_get_handle());
		} else if (_G(flags[V097]) == 0) {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				setGlobals1(_ripTalkerPos5Series, 1, 4, 1, 4, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
				sendWSMessage_110000(_G(my_walker), -1);
				switch (imath_ranged_rand(1, 4)) {
				case 1:
					digi_play("com034", 1, 255, 10, 997);
					break;

				case 2:
					digi_play("com035", 1, 255, 10, 997);
					break;

				case 3:
					digi_play("com036", 1, 255, 10, 997);
					break;

				case 4:
				default:
					digi_play("com037", 1, 255, 10, 997);
					break;
				}

				break;

			case 10:
				player_set_commands_allowed(true);
				sendWSMessage_150000(_G(my_walker), -1);

				switch (_G(kernel).trigger) {
				case 1:
					digi_play("com038", 1, 255, -1, 997);
					break;

				case 2:
					digi_play("com039", 1, 255, -1, 997);
					break;

				case 3:
					digi_play("com040", 1, 255, -1, 997);
					break;

				case 4:
				default:
					digi_play("com041", 1, 255, -1, 997);
					break;
				}

				break;

			default:
				break;
			}
		} else if (_G(flags[V097]) == 1 && inv_object_in_scene("crank", 808) && _G(flags[V100]) == 0) {
			switch (_G(kernel).trigger) {
			case -1:
				ws_turn_to_face(_G(my_walker), 1, 5);
				break;

			case 5:
				player_set_commands_allowed(false);
				if (_G(flags[V099]) == 0) {
					_G(flags[V099]) = 1;
					digi_play("808r26", 1, 255, 10, -1);
				} else {
					digi_play("808r27", 1, 255, 10, -1);
				}

				break;

			case 10:
				DisposePath(_mcTrekMach->walkPath);
				_mcTrekMach->walkPath = CreateCustomPath(320, 122, -1);
				ws_custom_walk(_mcTrekMach, 10, 20, true);

				break;

			case 20:
				ws_hide_walker(_mcTrekMach);
				if (_G(flags[V098]) == 0)
					terminateMachine(_808HandleSpriteMach);

				if (_G(flags[V094]) == 1 || _G(flags[V094]) == 2 || _G(flags[V094]) == 3) {
					series_load("808 bolt going in and out", -1, nullptr);
					series_play("808 mei chen cranks handle", 1536, 0, 30, 5, 0, 100, 0, 0, 0, 14);
				} else if (_G(flags[V094]) == 4 || (_G(flags[V094]) == 0 && inv_object_in_scene("FARMER'S SHOVEL", 808))) {
					series_play("808 mei chen cranks handle", 1536, 0, 21, 5, 0, 100, 0, 0, 0, 12);
				} else {
					series_play("808 mei chen cranks handle", 1536, 0, 40, 5, 0, 100, 0, 0, 0, 7);
				}

				digi_preload("808_s10", -1);

				break;

			case 21:
				series_play("808 mei chen cranks handle", 1536, 0, 22, 5, 0, 100, 0, 0, 13, 24);
				digi_play("808_s10", 2, 255, -1, -1);
				break;

			case 22:
				_808RipFallShovelNearSideMach = series_play("808 mei chen cranks handle", 1536, 16, -1, 5, 0, 100, 0, 0, 25, 29);
				digi_play("808_s09", 2, 255, -1, -1);
				kernel_timing_trigger(60, 26, nullptr);

				break;

			case 26:
				terminateMachine(_808RipFallShovelNearSideMach);
				series_play("808 mei chen cranks handle", 1536, 2, 38, 5, 0, 100, 0, 0, 8, 29);
				digi_play("808_s10", 2, 255, -1, -1);

				break;

			case 30:
				if (_G(flags[V098]) == 0) {
					terminateMachine(_808HandleSpriteMach);
				}

				_808RipFallShovelNearSideMach = series_play("808 mei chen cranks handle", 1536, 16, 35, 5, 0, 100, 0, 0, 15, 47);
				_safariShadowMach = series_play("808 bolt going in and out", 0, 16, -1, 15, 0, 100, 0, 0, 0, -1);
				_G(flags[V279]) = 1;
				digi_play("808_s07", 2, 255, -1, -1);
				digi_play("808_s10", 1, 255, -1, -1);

				break;

			case 35:
				kernel_timing_trigger(45, 37, "bolt showing");
				digi_stop(1);

				break;

			case 37:
				terminateMachine(_808RipFallShovelNearSideMach);
				terminateMachine(_safariShadowMach);
				series_play("808 mei chen cranks handle", 1536, 2, 38, 5, 0, 100, 0, 0, 8, 47);
				series_play("808 bolt going in and out", 0, 2, -1, 15, 0, 100, 0, 0, 0, -1);
				digi_play("808_s08", 2, 255, -1, -1);
				digi_play("808_s10", 1, 255, -1, -1);

				break;

			case 38:
				_808HandleSpriteMach = series_show("808 handle sprite", 4095, 0, -1, -1, 0, 100, 0, 0);
				series_play("808 mei chen cranks handle", 1536, 2, 39, 5, 0, 100, 0, 0, 0, 7);
				digi_stop(1);
				digi_stop(2);

				break;

			case 39:
				ws_unhide_walker(_mcTrekMach);
				DisposePath(_mcTrekMach->walkPath);
				_mcTrekMach->walkPath = CreateCustomPath(345, 115, -1);
				ws_custom_walk(_mcTrekMach, 5, 55, true);

				break;

			case 40:
				if (_G(flags[V098]) == 0) {
					terminateMachine(_808HandleSpriteMach);
				}

				series_play("808 mei chen cranks handle", 1536, 0, 41, 5, 0, 100, 0, 0, 8, 42);
				digi_play("808_s10", 2, 255, -1, -1);

				break;

			case 41:
				series_play("808 mei chen cranks handle", 1536, 0, 42, 5, 0, 100, 0, 0, 43, 58);
				digi_play("808_s03", 2, 255, -1, -1);

				break;

			case 42:
				_808HandleSpriteMach = series_show("808 handle sprite", 4095, 0, -1, -1, 0, 100, 0, 0);
				series_play("808 mei chen cranks handle", 1536, 0, 43, 5, 0, 100, 0, 0, 59, -1);

				break;

			case 43:
				ws_unhide_walker(_mcTrekMach);
				DisposePath(_mcTrekMach->walkPath);
				_mcTrekMach->walkPath = CreateCustomPath(345, 115, -1);
				ws_custom_walk(_mcTrekMach, 5, 50, true);

				break;

			case 50:
				player_set_commands_allowed(true);
				_G(flags[V100]) = 1;
				_G(flags[V276]) = 0;
				_G(flags[V098]) = 0;
				hotspot_set_active(_G(currentSceneDef).hotspots, "crank", true);
				hotspot_set_active(_G(currentSceneDef).hotspots, "slot", false);
				digi_unload("808_s10");

				break;

			case 55:
				player_set_commands_allowed(true);

				if (_G(flags[V094]) == 4 || (_G(flags[V094]) == 0 && inv_object_in_scene("FARMER'S SHOVEL", 808))) {
					digi_play(_G(flags[V279]) ? "808r29" : "808r28", 1, 255, -1, -1);
				}

				_G(flags[V098]) = 0;
				hotspot_set_active(_G(currentSceneDef).hotspots, "crank", true);
				hotspot_set_active(_G(currentSceneDef).hotspots, "slot", false);
				digi_unload("808_s10");

				break;

			default:
				break;
			}
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				if (_G(flags[V100]) == 0) {
					ws_turn_to_face(_G(my_walker), 1, 10);
				} else {
					ws_walk(_G(my_walker), 335, 121, nullptr, 10, 1, true);
				}

				break;
			case 10:
				player_set_commands_allowed(false);
				switch (imath_ranged_rand(1, 4)) {
				case 1:
					digi_play("com034", 1, 255, 20, 997);
					break;

				case 2:
					digi_play("com035", 1, 255, 20, 997);
					break;

				case 3:
					digi_play("com036", 1, 255, 20, 997);
					break;

				case 4:
				default:
					digi_play("com037", 1, 255, 20, 997);
					break;
				}

				break;

			case 20:
				player_set_commands_allowed(true);
				switch (imath_ranged_rand(1, 4)) {
				case 1:
					digi_play("com038", 1,255,-1,997);
					break;
				case 2:
					digi_play("com039", 1, 255, -1, 997);
					break;
				case 3:
					digi_play("com040", 1, 255, -1, 997);
					break;
				case 4:
				default:
					digi_play("com041", 1, 255, -1, 997);
					break;
				}

				break;
			default:
				break;
			}
		}


		return;

	case 4:
		// Empty
		return;

	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	default:
		break;
	}

	// TODO Not implemented yet
	_G(player).command_ready = true;
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
			sendWSMessage_3840000(_mcTrekMach, -1);
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
		disable_player_commands_and_fade_init(12);
		break;

	case 10:
		player_set_commands_allowed(false);
		ws_hide_walker(_G(my_walker));
		terminateMachine(_808PosMach);

		digi_preload("808_s04", -1);
		digi_preload("808_s02", -1);
		digi_preload("28_02n01", 807);

		if (inv_object_in_scene("farmer's shovel", 808)) {
			_808RipFallShovelNearSideMach = series_stream("808 RIP FALL WSHOVEL FAR SIDE", 5, 0, -1);
			series_stream_break_on_frame(_808RipFallShovelNearSideMach, 5, 13);
		} else {
			_808RipFallShovelNearSideMach = series_stream("808 RIP FALLS", 5, 0, -1);
			series_stream_break_on_frame(_808RipFallShovelNearSideMach, 5, 14);
		}

		digi_play("808_s04", 3, 255, -1, -1);

		break;

	case 11:
		if (inv_object_in_scene("farmer's shovel", 808)) {
			series_show("808pos2", 0, 0, -1, -1, 4, 100, 0, 0);
		}

		disable_player_commands_and_fade_init(12);

		break;

	case 12:
		if (_G(spleenSpraying)) {
			kernel_timing_trigger(200, 966, nullptr);
		} else {
			ws_unhide_walker(_G(my_walker));
			ws_demand_location(_G(my_walker), 202, 179);
			ws_demand_facing(_G(my_walker), 2);
			other_save_game_for_resurrection();

			_G(game).new_section = 4;
			_G(game).new_room = 413;
		}

		break;

	case 13:
		series_stream_break_on_frame(_808RipFallShovelNearSideMach, 21, 11);
		digi_play("808_s02", 2, 255, -1, -1);

		if (_G(flags[V097] == 0)) {
			setGlobals3(_meiChienHandsBehindBackSeries, 17, 1);
			sendWSMessage_3840000(_mcTrekMach, 16);
		} else {
			ws_hide_walker(_mcTrekMach);
			series_play("808mc99", 3840, 16, -1, 5, 0, 100, 0, 0, 0, -1);
		}

		kernel_timing_trigger(45, 17, nullptr);

		break;

	case 14:
		series_stream_break_on_frame(_808RipFallShovelNearSideMach, 24, 11);
		digi_play("808_s02", 2, 255, -1, -1);

		if (_G(flags[V097] == 0)) {
			setGlobals3(_meiChienHandsBehindBackSeries, 17, 1);
			sendWSMessage_3840000(_mcTrekMach, 16);
		} else {
			ws_hide_walker(_mcTrekMach);
			series_play("808mc99", 3840, 16, -1, 5, 0, 100, 0, 0, 0, -1);
		}

		kernel_timing_trigger(45, 17, nullptr);


		break;

	case 15:
		series_stream_break_on_frame(_808RipFallShovelNearSideMach, 24, 9);
		digi_play("808_s02", 2, 255, -1, -1);

		if (_G(flags[V097] == 0)) {
			setGlobals3(_meiChienHandsBehindBackSeries, 17, 1);
			sendWSMessage_3840000(_mcTrekMach, 16);
		} else {
			ws_demand_location(_mcTrekMach, -1000, -1000);
			ws_hide_walker(_mcTrekMach);
			series_play("808mc99", 3840, 16, -1, 5, 0, 100, 0, 0, 0, -1);
		}

		kernel_timing_trigger(45, 17, nullptr);

		break;

	case 16:
		series_play("808mc98", 0, 16, -1, 5, 0, 100, 0, 0, 0, -1);
		break;

	case 17:
		digi_play("28_02n01", 1, 255, -1, 807);
		break;

	case 18:
		if (_G(flags[V097] != 0))
			break;

		if (!player_commands_allowed() || !checkStrings()) {
			kernel_timing_trigger(60, 18, nullptr);
		} else {
			player_set_commands_allowed(false);
			intr_cancel_sentence();
			switch (imath_ranged_rand(1, 4)) {
			case 1:
				digi_play("950_s15", 2, 255, 19, -1);
				break;

			case 2:
				digi_play("950_s16", 2, 255, 19, -1);
				break;

			case 3:
				digi_play("950_s17", 2, 255, 19, -1);
				break;

			case 4:
			default:
				digi_play("950_s18", 2, 255, 19, -1);
				break;
			}
		}

		break;

	case 19:
		player_update_info(_G(my_walker), &_G(player_info));
		switch (_G(player_info).facing) {
		case 1:
		case 2:
		case 3:
		case 4:
			ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, 20, 3, true);
			_dword1A195C_facing = 3;

			break;

		case 5:
			kernel_timing_trigger(30, 20, "phantom reaction");
			_dword1A195C_facing = 5;

			break;

		case 7:
			kernel_timing_trigger(30, 20, "phantom reaction");
			_dword1A195C_facing = 7;

			break;

		case 8:
		case 9:
		case 10:
		case 11:
			ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, 20, 9, true);
			_dword1A195C_facing = 9;

			break;

		default:
			break;
		}

		break;

	case 20:
		_dword1A1958 = 0;
		_dword1A1960_rand4 = imath_ranged_rand(1, 4);

		switch(_dword1A1960_rand4) {
		case 1:
			digi_play("COM052", 1, 255, 21, 997);
			break;

		case 2:
			digi_play("COM054", 1, 255, 21, 997);
			break;

		case 3:
			digi_play("COM056", 1, 255, 21, 997);
			break;

		case 4:
			digi_play("COM057", 1, 255, 21, 997);
			break;

		default:
			break;
		}

		setGlobals3(_mctd82aSeries, 1, 22);
		sendWSMessage_3840000(_mcTrekMach, 23);

		if (_dword1A195C_facing == 3 || _dword1A195C_facing == 9) {
			setGlobals3(_ripPos3LookAroundSeries, 1, 20);
		} else {
			setGlobals3(_ripLooksAroundInAweSeries, 1, 14);
		}

		sendWSMessage_3840000(_G(my_walker), 22);

		break;

	case 21:
		switch (_dword1A1960_rand4) {
		case 1:
			digi_play("COM053", 1, 255, -1, 997);
			break;

		case 2:
			digi_play("COM055", 1, 255, -1, 997);
			break;

		case 4:
			digi_play("COM058", 1, 255, -1, 997);
			break;

		default:
			break;
		}
		break;

	case 22:
		kernel_timing_trigger(imath_ranged_rand(90, 120), 24, nullptr);
		break;

	case 23:
		kernel_timing_trigger(imath_ranged_rand(90, 120), 25, nullptr);
		break;

	case 24:


		if (_dword1A195C_facing == 3 || _dword1A195C_facing == 9) {
			setGlobals3(_ripPos3LookAroundSeries, 19, 1);
		} else {
			setGlobals3(_ripLooksAroundInAweSeries, 13, 1);
		}

		sendWSMessage_3840000(_G(my_walker), 26);

		break;

	case 25:
		setGlobals3(_mctd82aSeries, 22, 1);
		sendWSMessage_3840000(_mcTrekMach, 26);
		break;

	case 26:
		++_dword1A1958;
		if (_dword1A1958 == 2) {
			player_set_commands_allowed(true);
			ws_demand_facing(_G(my_walker), _dword1A195C_facing);
			kernel_timing_trigger(imath_ranged_rand(7200, 14400), 19, nullptr);
		}

		break;

	case 966:
		digi_play("950_s53", 2, 255, 967, -1);
		break;

	case 967:
		ws_unhide_walker(_G(my_walker));
		ws_demand_location(_G(my_walker), 202, 179);
		ws_demand_facing(_G(my_walker), 2);
		other_save_game_for_resurrection();
		_G(game).section_id = 4;
		_G(game).new_room = 413;

		break;

	default:
		break;
	}
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

void Room808::room808_sub1() {
	// TODO Not implemented yet
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
