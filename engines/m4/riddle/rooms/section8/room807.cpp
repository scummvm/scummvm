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

#include "m4/riddle/rooms/section8/room807.h"

#include "m4/adv_r/other.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/riddle.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {


// TODO : Refactor - This array is also present in walker.cpp
static const char *SAFARI_SHADOWS[5] = {
	"safari shadow 1", "safari shadow 2", "safari shadow 3",
	"safari shadow 4", "safari shadow 5"
};

void Room807::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
}

void Room807::init() {
	if (inv_object_in_scene("wooden beam", 807)) {
		_807PostMach = series_show("807post", 4095, 0, -1, -1, 0, 100, 0, 0);
	} else {
		hotspot_set_active(_G(currentSceneDef).hotspots, "wooden beam", false);
	}

	if (inv_object_in_scene("wooden post", 807)) {
		_807BeamMach = series_show("807beam", 4095, 0, -1, -1, 0, 100, 0, 0);
	} else {
		hotspot_set_active(_G(currentSceneDef).hotspots, "wooden post", false);
	}

	if (_G(flags[V274])) {
		_807DoorMach = series_show("807door", 4095, 0, -1, -1, 0, 100, 0, 0);
		hotspot_set_active(_G(currentSceneDef).hotspots, "stone block", true);
		hotspot_set_active(_G(currentSceneDef).hotspots, "corridor", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "chariot ", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "north", false);
	} else {
		hotspot_set_active(_G(currentSceneDef).hotspots, "stone block", false);

		if (player_been_here(807)) {
			_807DoorMach = series_show("807kart", 4095, 0, -1, -1, 100, 0, 0);
		}
	}

	_ripLowReachPos1Series = series_load("rip low reach pos1", -1, nullptr);
	_ripTrekHiReach2HndSeries = series_load("rip trek hi reach 2hnd", -1, nullptr);
	_ripTalkerPos5Series = series_load("RIP TALKER POS 5", -1, nullptr);
	_mctd82aSeries = series_load("mctd82a", -1, nullptr);
	_ripPos3LookAroundSeries = series_load("RIP POS 3 LOOK AROUND", -1, nullptr);
	_ripLooksAroundInAweSeries = series_load("RIP LOOKS AROUND IN AWE", -1, nullptr);

	series_play("807fire1", 4095, 0, -1, 7, -1, 100, 0, 0, 0, -1);
	series_play("807fire2", 4095, 0, -1, 7, -1, 100, 0, 0, 0, -1);

	_field34 = 0;

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		player_set_commands_allowed(true);
		digi_preload("950_s29", -1);

		if (_field38 != 0) {
			ws_demand_location(_G(my_walker), 476, 318);
			ws_demand_facing(_G(my_walker), 11);
			ws_hide_walker(_G(my_walker));
			_807Crnk2Mach = series_show("807rp05", 256, 0, -1, -1, 12, 100, 0, 0);
			player_update_info(_G(my_walker), &_G(player_info));
			_safariShadowMach = series_place_sprite(*SAFARI_SHADOWS, 0, 476, 318, _G(player_info).scale, 257);
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_trigger_dispatchx((kernel_trigger_create(6)));
			_G(kernel).trigger_mode = KT_PREPARSE;
			hotspot_set_active(_G(currentSceneDef).hotspots, "slot", false);
		} else if (inv_object_in_scene("crank", 807)) {
			_807Crnk2Mach = series_show("807crnk2", 4095, 0, -1, -1, 9, 100, 0, 0);
			hotspot_set_active(_G(currentSceneDef).hotspots, "slot", false);
		} else {
			hotspot_set_active(_G(currentSceneDef).hotspots, "crank", false);
		}

		if (_G(flags[V276]) != 0) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "mei chen", false);
		} else {
			ws_walk_load_shadow_series(S8_SHADOW_DIRS1, S8_SHADOW_NAMES1);
			ws_walk_load_walker_series(S8_SHADOW_DIRS2, S8_SHADOW_NAMES2, false);
			_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 560, 400, 11, Walker::player_walker_callback, "mc_trek");
		}

		break;

	case 808:
		player_set_commands_allowed(false);
		if (inv_object_in_scene("crank", 807)) {
			_807Crnk2Mach = series_show("807crnk2", 4095, 0, -1, -1, 9, 100, 0, 0);
			hotspot_set_active(_G(currentSceneDef).hotspots, "slot", false);
		} else {
			hotspot_set_active(_G(currentSceneDef).hotspots, "crank", false);
		}

		hotspot_set_active(_G(currentSceneDef).hotspots, "mei chen", false);
		_field38 = 0;
		ws_demand_location(_G(my_walker), 273, 270);
		ws_demand_facing(_G(my_walker), 5);

		if (_G(flags[V276]) != 0) {
			ws_walk(_G(my_walker), 250, 345, nullptr, 5, 2, true);
		} else {
			ws_walk_load_walker_series(S8_SHADOW_DIRS1, S8_SHADOW_NAMES1);
			_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 295, 250, 5, Walker::player_walker_callback, "mc_trek");
			ws_walk(_mcTrekMach, 560, 400, nullptr, 5, 11, true);
			ws_walk(_G(my_walker), 250, 345, nullptr, -1, 2, true);
		}

		break;

	default:
		player_set_commands_allowed(false);
		ws_walk_load_shadow_series(S8_SHADOW_DIRS1, S8_SHADOW_NAMES1);
		ws_walk_load_walker_series(S8_SHADOW_DIRS2, S8_SHADOW_NAMES2, false);
		if (inv_object_in_scene("crank", 807)) {
			_807Crnk2Mach = series_show("807crnk2", 4095, 0, -1, -1, 9, 100, 0, 0);
			hotspot_set_active(_G(currentSceneDef).hotspots, "slot", false);
		} else {
			hotspot_set_active(_G(currentSceneDef).hotspots, "crank", false);
		}

		hotspot_set_active(_G(currentSceneDef).hotspots, "mei chen", false);
		_field38 = 0;

		if (!player_been_here(807)) {
			_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 450, 60, 1, Walker::player_walker_callback, "mc_trek");
			ws_demand_location(_G(my_walker), 366, 345);
			ws_demand_facing(_G(my_walker), 11);
			ws_hide_walker(_G(my_walker));
			digi_preload("950_S33", -1);
			digi_preload("807_S01", -1);
			digi_play("950_S33", 2, 255, -1, -1);
			_807DoorMach = series_stream("807crush", 5, 0, 0);
			series_stream_break_on_frame(_807DoorMach, 60, 3);
		} else {
			ws_demand_location(_G(my_walker), 366, 500);
			ws_demand_facing(_G(my_walker), 1);

			if (_G(flags[V276]) != 0) {
				ws_walk(_G(my_walker), 366, 345, nullptr, 5, 2, true);
			} else {
				_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 450, 600, 1, Walker::player_walker_callback, "mc_trek");
				ws_walk(_G(my_walker), 366, 345, nullptr, -1, 2, true);
				ws_walk(_mcTrekMach, 560, 400, nullptr, 5, 11, true);
			}
		}

		break;
	}

	digi_play_loop("950_s29", 2, 127, -1, -1);
}

void Room807::pre_parser() {
	if (_G(flags[V274]) || inv_object_in_scene("wooden post", 807) || inv_object_in_scene("wooden beam", 807)) {
		if (player_said("gear", "stone block")) {
			_G(player).need_to_walk = false;
			_G(player).ready_to_walk = true;
			_G(player).waiting_for_walk = false;
		} else if (player_said("look at", "corridor")) {
			_G(player).walk_x = 285;
			_G(player).walk_y = 319;
		} else if (player_said("go", "south") || player_said("go", "north")) {
			_G(player).need_to_walk = false;
			_G(player).ready_to_walk = true;
			_G(player).waiting_for_walk = false;
		}
	} else if (player_said("talk to", "mei chen") || player_said("wooden post", "crank")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	} else {
		intr_cancel_sentence();
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_trigger_dispatchx(kernel_trigger_create(7));
		_G(kernel).trigger_mode = KT_PREPARSE;
	}
}

void Room807::parser() {
	_G(player).command_ready = false;

	if (_G(kernel).trigger == 747) {
		player_set_commands_allowed(true);
		_field34 = 0;
		return;
	}

	int opCode = -1;

	if (player_said_any("look", "look at"))
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
		opCode = 8;
	else if (player_said("wooden beam"))
		opCode = 6;
	else if (player_said("wooden post"))
		opCode = 7;
	else if (player_said("journal"))
		opCode = 9;
	else if (player_said("conv807a"))
		opCode = 10;


	switch (opCode) {
	case 0:
		if (player_said("stone block") && _G(flags[V276]) == 0) {
			switch (_G(kernel).trigger) {
			case -1:
				ws_walk(_G(my_walker), 305, 305, nullptr, 10, 1, true);
				break;

			case 10:
				player_set_commands_allowed(false);
				setGlobals3(_ripLowReachPos1Series, 1, 24);
				sendWSMessage_3840000(_G(my_walker), 20);

				break;

			case 20:
				kernel_timing_trigger(30, 30, "rip attempts stone block lift");
				break;

			case 30:
				digi_play("807m02", 1, 255, 40, -1);
				break;

			case 40:
				digi_play("807r14", 1, 255, -1, -1);
				setGlobals3(_ripLowReachPos1Series, 24, 1);
				sendWSMessage_3840000(_G(my_walker), 50);

				break;

			case 50:
				player_set_commands_allowed(true);
				ws_demand_facing(_G(my_walker), 1);

				break;

			default:
				break;
			}
		} else if (player_said("crank") && inv_object_in_scene("crank", 807)) {
			if (!_G(flags[V274])) {
				digi_play("com078", 1, 255, -1, 997);
			} else if (inv_object_in_scene("wooden post", 807)) {
				digi_play("807r23a", 1, 255, -1, -1);
			} else {
				switch (_G(kernel).trigger) {
				case -1:
					ws_walk(_G(my_walker), 476, 318, nullptr, 10, 11, true);
					break;

				case 10:
					player_set_commands_allowed(false);
					ws_hide_walker(_G(my_walker));
					series_play("807rp04", 256, 0, 13, 5, 0, 100, 0, 0, 0, -1);
					player_update_info(_G(my_walker), &_G(player_info));
					_safariShadowMach = series_place_sprite(*SAFARI_SHADOWS, 0, 476, 318, _G(player_info).scale, 257);

					break;

				case 13:
					terminateMachine(_807Crnk2Mach);
					_807Crnk2Mach = series_play("807rp05", 256, 16, 20, 10, 4, 100, 0, 0, 0, -1);
					terminateMachine(_807DoorMach);
					digi_preload("807_s03");
					series_stream("807open", 20, 495, 15);
					digi_play_loop("807_s03", 2, 255, -1, -1);

					break;

				case 15:
					_807DoorMach = series_show("807kart", 4095, 0, -1, -1, 0, 100, 0, 0);
					digi_stop(2);
					digi_unload("807_s03");

					break;

				case 20:
					player_set_commands_allowed(true);
					_G(flags[V274]) = 0;
					_field38 = 1;
					hotspot_set_active(_G(currentSceneDef).hotspots, "stone block", false);
					hotspot_set_active(_G(currentSceneDef).hotspots, "corridor", true);
					hotspot_set_active(_G(currentSceneDef).hotspots, "chariot ", true);
					hotspot_set_active(_G(currentSceneDef).hotspots, "north", true);

					_G(kernel).trigger_mode = KT_DAEMON;
					kernel_trigger_dispatchx(kernel_trigger_create(6));
					_G(kernel).trigger_mode = KT_PARSE;

					break;

				default:
					break;

				}
			}
		} else {
			player_set_commands_allowed(true);
		}

		break;

	case 1:
		if (player_said(" ")) {
			digi_play("807r02", 1, 255, -1, -1);
		} else if (player_said_any("stone block", "doorway")) {
			digi_play("807r03", 1, 255, -1, -1);
		} else if (player_said("slot")) {
			digi_play("807r05", 1, 255, -1, -1);
		} else if (player_said_any("chariot", "chariot ")) {
			digi_play("807r06", 1, 255, -1, -1);
		} else if (player_said("lit urn")) {
			digi_play("com060", 1, 255, -1, 997);
		} else if (player_said("unlit urn")) {
			digi_play("com061", 1, 255, -1, 997);
		} else if (player_said("corridor")) {
			digi_play("807r09", 1, 255, -1, -1);
		} else if (player_said("wooden beam") && inv_object_in_scene("wooden beam", 807)) {
			digi_play("807r11", 1, 255, -1, 997);
		} else if (player_said("wooden post") && inv_object_in_scene("wooden post", 807)) {
			if (inv_object_in_scene("crank", 807) && _G(flags[V274]) == 0) {
				digi_play("807r10", 1, 255, -1, -1);
			} else {
				digi_play("com021", 1, 255, -1, 997);
			}
		} else if (player_said("crank") && inv_object_in_scene("crank", 807)) {
			digi_play("807r12", 1, 255, -1, -1);
		} else if (player_said("mei chen")) {
			digi_play("807r13", 1, 255, -1, -1);
		} else
			_G(player).command_ready = true;

		break;

	case 2:
		if (player_said("wooden post")) {
			switch (_G(kernel).trigger) {
			case -1:
				if (inv_object_in_scene("wooden post", 807)) {
					player_set_commands_allowed(false);
					ws_hide_walker(_G(my_walker));
					player_update_info(_G(my_walker), &_G(player_info));
					_safariShadowMach = series_place_sprite(*SAFARI_SHADOWS, 0, 476, 318, _G(player_info).scale, 257);
					series_play("807rp06", 256, 2, 5, 5, 0, 100, 0, 0, 21, 39);
					hotspot_set_active(_G(currentSceneDef).hotspots, "wooden post", false);
				} else {
					_G(player).command_ready = true;
				}

				break;

			case 5:
				if (_G(flags[V274]) == 0 && !inv_object_in_scene("wooden beam", 807)) {
					inv_give_to_player("wooden post");
					kernel_examine_inventory_object("PING WOODEN POST", _G(master_palette), 5, 1, 400, 245, 13, nullptr, -1);
					terminateMachine(_807BeamMach);
					series_play("807rp06", 256, 2, 10, 5, 0, 100, 0, 0, 0, 20);
					_G(flags[V274]) = 1;
				} else {
					inv_give_to_player("wooden post");
					kernel_examine_inventory_object("PING WOODEN POST", _G(master_palette), 5, 1, 400, 245, -1, nullptr, -1);
					terminateMachine(_807BeamMach);
					series_play("807rp06", 256, 2, 20, 5, 0, 100, 0, 0, 0, 20);
				}

				break;

			case 10:
				ws_unhide_walker(_G(my_walker));
				ws_demand_facing(_G(my_walker), 11);
				terminateMachine(_safariShadowMach);

				break;

			case 13:
				terminateMachine(_807Crnk2Mach);
				terminateMachine(_807DoorMach);
				series_play("807close", 4095, 0, 15, 0, 0, 100, 0, 0, 0, -1);
				digi_play("807_s04", 2, 255, -1, -1);

				break;

			case 15:
				digi_play("807_s04a", 2, 255, -1, -1);
				player_set_commands_allowed(true);
				_807DoorMach = series_show("807door", 4095, 0, -1, -1, 0, 100, 0, 0);
				_807Crnk2Mach = series_show("807crnk2", 4095, 0, -1, -1, 9, 100, 0, 0);
				hotspot_set_active(_G(currentSceneDef).hotspots, "stone block", true);
				hotspot_set_active(_G(currentSceneDef).hotspots, "corridor", false);
				hotspot_set_active(_G(currentSceneDef).hotspots, "chariot ", false);
				hotspot_set_active(_G(currentSceneDef).hotspots, "north", false);

				break;

			case 20:
				player_set_commands_allowed(true);
				ws_unhide_walker(_G(my_walker));
				ws_demand_facing(_G(my_walker), 11);
				terminateMachine(_safariShadowMach);

				break;

			default:
				break;
			}
		} else if (player_said("crank")) {
			switch (_G(kernel).trigger) {
			case -1:
				if (inv_object_in_scene("crank", 807)) {
					if (inv_object_in_scene("wooden post", 807) && _G(flags[V274]) == 0) {
						digi_play("807r23", 1, 255, -1, -1);
					} else {
						player_set_commands_allowed(false);
						ws_hide_walker(_G(my_walker));
						terminateMachine(_807Crnk2Mach);
						_807Crnk2Mach = series_play("807rp04", 256, 16, 10, 5, 0, 100, 0, 0, 0, -1);
						player_update_info(_G(my_walker), &_G(player_info));
						_safariShadowMach = series_place_sprite(*SAFARI_SHADOWS, 0, 476, 318, _G(player_info).scale, 257);
					}
				} else {
					_G(player).command_ready = true;
				}

				break;

			case 10:
				inv_give_to_player("CRANK");
				kernel_examine_inventory_object("PING CRANK", _G(master_palette), 5, 1, 400, 245, 20, nullptr, -1);

				break;

			case 20:
				terminateMachine(_807Crnk2Mach);
				_807Crnk2Mach = series_play("807rp04", 256, 2, 30, 5, 0, 100, 0, 0, 0, -1);

				break;

			case 30:
				player_set_commands_allowed(true);
				terminateMachine(_safariShadowMach);
				hotspot_set_active(_G(currentSceneDef).hotspots, "slot", true);
				hotspot_set_active(_G(currentSceneDef).hotspots, "crank", false);
				ws_unhide_walker(_G(my_walker));
				ws_demand_facing(_G(my_walker), 11);

			default:
				break;
			}
		} else if (player_said("wooden beam")) {
			switch (_G(kernel).trigger) {
			case -1:
				if (inv_object_in_scene("wooden beam", 807)) {
					if (inv_object_in_scene("wooden post", 807)) {
						player_set_commands_allowed(false);
						setGlobals3(_ripTrekHiReach2HndSeries, 1, 13);
						sendWSMessage_3840000(_G(my_walker), 10);
					} else {
						digi_play("807r26", 1, 255, -1, -1);
					}
				} else {
					_G(player).command_ready = true;
				}

				break;

			case 10:
				inv_give_to_player("WOODEN BEAM");
				kernel_examine_inventory_object("PING WOODEN BEAM", _G(master_palette), 5, 1, 170, 135, 15, nullptr, -1);

				break;

			case 15:
				terminateMachine(_807PostMach);
				setGlobals3(_ripTrekHiReach2HndSeries, 13, 1);
				sendWSMessage_3840000(_G(my_walker), 20);

				break;

			case 20:
				player_set_commands_allowed(true);
				hotspot_set_active(_G(currentSceneDef).hotspots, "wooden beam", false);
				ws_demand_facing(_G(my_walker), 11);

				break;

			default:
				break;

			}
		} else if (player_said_any("chariot", "chariot ")) {
			digi_play("807r27", 1, 255, -1, -1);
		} else
			_G(player).command_ready = true;

		break;

	case 3:
		if (_G(flags[V274]) == 0 && !inv_object_in_scene("wooden post", 807) && !inv_object_in_scene("wooden beam", 807)) {
			_field34 = 1;
			conv_load("conv807a", 10, 10, 747);
			conv_play(conv_get_handle());
		} else {
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
				switch (imath_ranged_rand(1, 4)) {
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
		}

		break;
	case 4:
		// No implementation
		break;

	case 5:
		if (player_said("north")) {
			switch (_G(kernel).trigger) {
			case -1:
				ws_walk(_G(my_walker), 325, 275, nullptr, 10, 2, false);
				break;

			case 10:
				player_set_commands_allowed(false);
				ws_walk(_G(my_walker), 305, 190, nullptr, -1, 2, true);
				if (_G(flags[V276]) == 0) {
					ws_walk(_mcTrekMach, 305, 190, nullptr, -1, -1, true);
				}

				disable_player_commands_and_fade_init(20);

				break;

			case 20:
				_G(game).new_room = 808;
				adv_kill_digi_between_rooms(false);
				digi_preload("950_s29", -1);
				digi_play_loop("950_s29", 3, 255, -1, -1);

				break;

			default:
				break;
			}
		} else {
			switch (_G(kernel).trigger) {
			case -1: {
				player_update_info(_G(my_walker), &_G(player_info));
				const int32 destX = CLIP(_G(player_info).x, (int32)247, (int32)400);
				const int32 destY = MAX(_G(player_info).y, (int32)342);

				if (_G(flags[V276]) == 0 && _G(flags[V275]) == 0) {
					_G(flags[V275]) = 1;
					ws_walk(_G(my_walker), destX, destY, nullptr, 10, -1, true);
				} else {
					ws_walk(_G(my_walker), destX, destY, nullptr, 35, -1, false);
				}

				}
				break;

			case 10:
				player_set_commands_allowed(false);
				digi_play("807m10", 1, 255, 20, -1);
				ws_turn_to_face(_G(my_walker), 5, -1);

				break;

			case 20:
				digi_play_loop("807r25", 1, 255, 30, -1);
				setGlobals1(_ripTalkerPos5Series, 1, 4, 1, 4, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
				sendWSMessage_110000(_G(my_walker), -1);

				break;

			case 30:
				sendWSMessage_150000(_G(my_walker), -1);
				ws_walk(_mcTrekMach, 320, 560, nullptr, -1, -1, true);
				ws_walk(_G(my_walker), 320, 560, nullptr, -1, -1, true);
				disable_player_commands_and_fade_init(40);

				break;

			case 35:
				if (_G(flags[V276]) == 0) {
					ws_walk(_mcTrekMach, 320, 560, nullptr, -1, -1, true);
				}

				ws_walk(_G(my_walker), 320, 560, nullptr, -1, -1, true);
				disable_player_commands_and_fade_init(40);

				break;

			case 40:
				_G(game).new_room = 806;
				adv_kill_digi_between_rooms(false);
				digi_preload("950_s29", -1);
				digi_play_loop("950_s29", 2, 255, -1, -1);

				break;

			default:
				break;
			}
		}

		break;

	case 6:
		if (player_said("stone block")) {
			digi_play("807r22", 1, 255, -1, -1);
		} else if (player_said("corridor")) {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				inv_move_object("wooden beam", 807);
				hotspot_set_active(_G(currentSceneDef).hotspots, "wooden beam", true);
				ws_hide_walker(_G(my_walker));
				series_play("807rp02", 4095, 0, 5, 5, 0, 100, 0, 0, 0, 24);

				break;

			case 5:
				series_play("807rp02", 4095, 0, 10, 5, 0, 100, 0, 0, 25, -1);
				digi_play("807_s06", 2, 255, -1, -1);

				break;

			case 10:
				player_set_commands_allowed(true);
				ws_unhide_walker(_G(my_walker));
				ws_demand_facing(_G(my_walker), 11);
				_807PostMach = series_show("807post", 4095, 0, -1, -1, 0, 100, 0, 0);

				break;

			default:
				break;

			}
		} else if (player_said("mei chen")) {
			digi_play("com017", 1, 255, -1, 997);
		} else
			_G(player).command_ready = true;

		break;

	case 7:
		if ((player_said("crank") && inv_object_in_scene("crank", 807)) || player_said("slot")) {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				inv_move_object("wooden post", 807);
				_field38 = 0;
				hotspot_set_active(_G(currentSceneDef).hotspots, "wooden post", true);
				if (_G(flags[V274]) == 0 && !inv_object_in_scene("wooden beam", 807)) {
					terminateMachine(_807Crnk2Mach);
					_807Crnk2Mach = series_show("807crnk2", 4095, 0, -1, -1, 9, 100, 0, 0);
					series_load("807rp06", -1, nullptr);
					series_play("807rp04", 256, 2, 10, 5, 0, 100, 0, 0, 0, -1);
				} else {
					ws_hide_walker(_G(my_walker));
					series_play("807rp06", 256, 0, 15, 5, 0, 100, 0, 0, 0, 20);
					player_update_info(_G(my_walker), &_G(player_info));
					_safariShadowMach = series_place_sprite(*SAFARI_SHADOWS, 0, 476, 318, _G(player_info).scale, 257);
				}

				break;

			case 10:
				series_play("807rp06", 256, 0, 15, 5, 0, 100, 0, 0, 0, 20);
				break;

			case 15:
				series_play("807rp06", 256, 0, 20, 5, 0, 100, 0, 0, 21, 30);
				digi_play("807_s05", 2, 255, -1, -1);
				_807BeamMach = series_show("807beam", 4095, 0, -1, -1, 0, 100, 0, 0);

				break;

			case 20:
				player_set_commands_allowed(true);
				ws_unhide_walker(_G(my_walker));
				ws_demand_facing(_G(my_walker), 11);
				terminateMachine(_safariShadowMach);

				break;

			default:
				break;
			}
		} else if (player_said("mei chen")) {
			digi_play("com017", 1, 255, -1, 997);
		} else
			_G(player).command_ready = true;

		break;

	case 8:
		if (player_said("slot")) {
			switch (_G(kernel).trigger) {
			case -1:
				ws_walk(_G(my_walker), 476, 318, 0, 10, 11, true);
				break;

			case 10:
				player_set_commands_allowed(false);
				ws_hide_walker(_G(my_walker));
				_807Crnk2Mach = series_play("807rp04", 256, 16, 20, 5, 0, 100, 0, 0, 0, -1);
				player_update_info(_G(my_walker), &_G(player_info));
				_safariShadowMach = series_place_sprite(*SAFARI_SHADOWS, 0, 476, 318, _G(player_info).scale, 257);
				inv_move_object("CRANK", 807);

				hotspot_set_active(_G(currentSceneDef).hotspots, "slot", false);
				hotspot_set_active(_G(currentSceneDef).hotspots, "crank", true);

				break;

			case 20:
				kernel_timing_trigger(20, 30, "rip places crank");
				digi_play("807_s02", 2, 255, -1, -1);

				break;

			case 30:
				terminateMachine(_807Crnk2Mach);
				_807Crnk2Mach = series_play("807rp04", 256, 2, 40, 5, 0, 100, 0, 0, 0, -1);

				break;

			case 40:
				player_set_commands_allowed(true);
				terminateMachine(_safariShadowMach);
				_807Crnk2Mach = series_show("807crnk2", 4095, 0, -1, -1, 9, 100, 0, 0);
				ws_unhide_walker(_G(my_walker));
				ws_demand_facing(_G(my_walker), 11);

				break;

			default:
				break;
			}
		} else if (player_said("mei chen")) {
			digi_play("807r24", 1, 255, -1, -1);
		} else
			_G(player).command_ready = true;

		break;

	case 9:
		digi_play("com042", 1, 255, -1, 997);
		break;

	case 10:
		switch (_G(kernel).trigger) {
		case -1: {
			const char *conv = conv_sound_to_play();
			if (conv)
				digi_play(conv, 1, 255, 10, -1);
			else
				conv_resume(conv_get_handle());

			}

			break;

		case 10:
			if (conv_current_node() != 0) {
				conv_resume(conv_get_handle());
			} else {
				if (conv_current_entry() != 0) {
					conv_resume(conv_get_handle());
				} else if (conv_whos_talking() == 1) {
					ws_walk(_mcTrekMach, 476, 318, nullptr, 20, 11, true);
				}
			}

			break;

		case 20:
			ws_hide_walker(_mcTrekMach);
			_807Mc01Mach = series_play("807mc01", 0, 16, -1, 5, 0, 100, 0, 0, 0, 5);
			terminateMachine(_807Crnk2Mach);
			series_play("807rp04", 256, 2, 25, 5, 0, 100, 0, 0, 0, -1);

			break;

		case 25:
			terminateMachine(_safariShadowMach);
			ws_unhide_walker(_G(my_walker));
			ws_walk(_G(my_walker), 320, 294, nullptr, 30, 1, true);
			terminateMachine(_807Mc01Mach);
			_807Mc01Mach = series_play("807mc01", 0, 16, 27, 5, 0, 100, 0, 0, 6, 10);

			break;

		case 27:
			terminateMachine(_807Mc01Mach);
			_807Mc01Mach = series_play("807mc01", 0, 16, -1, 5, 0, 100, 0, 0, 11, 21);
			conv_resume(conv_get_handle());

			break;

		case 30:
			terminateMachine(_807Mc01Mach);
			_807Mc01Mach = series_play("807mc01", 0, 16, -1, 5, 0, 100, 0, 0, 22, 41);
			ws_hide_walker(_807Mc01Mach);
			_G(kernel).trigger_mode = KT_DAEMON;
			series_play("807rp03", 4095, 16, 11, 5, 0, 100, 0, 0, 0, -1);
			_G(kernel).trigger_mode = KT_PARSE;
			digi_play("807_s04", 2, 255, -1, -1);
			conv_resume(conv_get_handle());

			break;

		default:
			break;
		}

		break;

	default:
		if (player_said("mei chen"))
			digi_play("com017", 1, 255, -1, 997);
		else
			player_set_commands_allowed(true);

		break;
	}
}

void Room807::daemon() {
	switch (_G(kernel.trigger)) {
	case 0:
		ws_unhide_walker(_G(my_walker));
		_807DoorMach = series_show("807door", 4095, 0, -1, -1, 0, 100, 0, 0);
		_G(flags[V274]) = 1;
		hotspot_set_active(_G(currentSceneDef).hotspots, "stone block", true);
		hotspot_set_active(_G(currentSceneDef).hotspots, "corridor", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "chariot ", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "north", false);
		ws_walk(_mcTrekMach, 560, 400, nullptr, 1, 11, true);

		break;

	case 1:
		digi_play("807m01", 1, 255, 2, -1);
		break;

	case 2:
		digi_play("807r01", 1, 255, 5, -1);
		break;

	case 3:
		digi_stop(2);
		digi_unload("950_s33");
		digi_play("807_s01", 2, 255, 4, -1);

		break;

	case 4:
		digi_unload("807_s01");
		break;

	case 5:
		player_set_commands_allowed(true);
		if (_G(flags[V276]) == 0) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "mei chen", true);
			kernel_timing_trigger(imath_ranged_rand(1200, 1800), 13, nullptr);
		}

		break;

	case 6:
		kernel_timing_trigger(600, 7, "thunk!");
		break;

	case 7:
		if ((_G(flags[V274]) == 0) && !inv_object_in_scene("wooden beam", 807) && !inv_object_in_scene("wooden post", 807)) {
			if (_field34)
				kernel_timing_trigger(60, 7, "thunk!");
			else {
				player_set_commands_allowed(false);
				_G(flags[V274]) = 1;
				hotspot_set_active(_G(currentSceneDef).hotspots, "stone block", true);
				hotspot_set_active(_G(currentSceneDef).hotspots, "corridor", false);
				hotspot_set_active(_G(currentSceneDef).hotspots, "chariot ", false);
				hotspot_set_active(_G(currentSceneDef).hotspots, "north", false);

				terminateMachine(_807Crnk2Mach);

				series_play("807rp04", 256, 2, 8, 5, 0, 100, 0, 0, 0, -1);

				terminateMachine(_807DoorMach);

				series_play("807close", 4095, 0, 10, 0, 0, 100, 0, 0, 0, -1);
				digi_play("807_s04", 2, 255, -1, -1);

				_field38 = 0;
			}
		}

		break;

	case 8:
		terminateMachine(_safariShadowMach);
		ws_unhide_walker(_G(my_walker));
		ws_demand_location(_G(my_walker), 476, 318);
		ws_demand_facing(_G(my_walker), 11);

		break;

	case 10:
		player_set_commands_allowed(true);
		digi_play("807_s04a", 2, 255, -1, -1);
		_807DoorMach = series_show("807door", 4095, 0, -1, -1, 0, 100, 0, 0);
		_807Crnk2Mach = series_show("807crnk2", 4095, 0, -1, -1, 9, 100, 0, 0);

		break;

	case 11:
		_G(flags[V274]) = 1;
		disable_player_commands_and_fade_init(12);

		break;

	case 12:
		_field38 = 1;
		_G(flags[V274]) = 0;
		other_save_game_for_resurrection();
		_G(game).new_section = 4;
		_G(game).new_room = 413;

		break;

	case 13:
		if (player_commands_allowed() && checkStrings() && (_G(flags[V274]) != 0 || inv_object_in_scene("wooden post", 807) || inv_object_in_scene("wooden beam", 807))) {
			player_set_commands_allowed(false);
			intr_cancel_sentence();
			switch (imath_ranged_rand(1, 4)) {
			case 1:
				digi_play("950_s15", 2, 255, 14, -1);
				break;

			case 2:
				digi_play("950_s16", 2, 255, 14, -1);
				break;

			case 3:
				digi_play("950_s17", 2, 255, 14, -1);
				break;

			case 4:
			default:
				digi_play("950_s18", 2, 255, 14, -1);
				break;

			}

		} else {
			kernel_timing_trigger(60, 13, nullptr);
		}


		break;

	case 14:
		player_update_info(_G(my_walker), &_G(player_info));
		switch (_G(player_info).facing) {
		case 1:
		case 2:
		case 3:
		case 4:
			ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, 15, 3, true);
			_807newFacing = 3;

			break;

		case 5:
			kernel_timing_trigger(30, 15, "phantom reaction");
			_807newFacing = 5;

			break;

		case 7:
			kernel_timing_trigger(30, 15, "phantom reaction");
			_807newFacing = 7;

			break;

		case 8:
		case 9:
		case 10:
		case 11:
			ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, 15, 9, true);
			_807newFacing = 9;

			break;

		default:
			break;
		}


		break;

	case 15:
		_dword1A194C = 0;
		_dword1A1954 = imath_ranged_rand(1, 4);
		switch (_dword1A1954) {
		case 1:
			digi_play("COM052", 1, 255, 16, 997);
			break;

		case 2:
			digi_play("COM054", 1, 255, 16, 997);
			break;

		case 3:
			digi_play("COM056", 1, 255, 16, 997);
			break;

		case 4:
			digi_play("COM057", 1, 255, 16, 997);
			break;

		default:
			break;
		}

		setGlobals3(_mctd82aSeries, 1, 22);
		sendWSMessage_3840000(_mcTrekMach, 18);
		switch (_807newFacing) {
		case 3:
		case 9:
			setGlobals3(_ripPos3LookAroundSeries, 1, 20);
			break;

		default:
			setGlobals3(_ripLooksAroundInAweSeries, 1, 14);
			break;
		}

		sendWSMessage_3840000(_G(my_walker), 17);

		break;

	case 16:
		switch (_dword1A1954) {
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

	case 17:
		kernel_timing_trigger(imath_ranged_rand(90, 120), 19, nullptr);
		break;

	case 18:
		kernel_timing_trigger(imath_ranged_rand(90, 120), 20, nullptr);
		break;

	case 19:
		switch (_807newFacing) {
		case 3:
		case 9:
			setGlobals3(_ripPos3LookAroundSeries, 19, 1);
			break;

		default:
			setGlobals3(_ripLooksAroundInAweSeries, 13, 1);
			break;
		}

		sendWSMessage_3840000(_G(my_walker), 21);

		break;

	case 20:
		setGlobals3(_mctd82aSeries, 22, 1);
		sendWSMessage_3840000(_mcTrekMach, 21);

		break;

	case 21:
		++_dword1A194C;
		if (_dword1A194C == 2) {
			player_set_commands_allowed(true);
			ws_demand_facing(_G(my_walker), _807newFacing);
			kernel_timing_trigger(imath_ranged_rand(7200, 14400), 13, nullptr);
		}

		break;

	default:
		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
