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

#include "m4/riddle/rooms/section7/room703.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_vmng_screen.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room703::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
}

void Room703::init() {
	_ripTrekMedReachHandPos1Series = series_load("RIP TREK MED REACH HAND POS1", -1, nullptr);
	digi_preload("950_S40", -1);
	digi_preload("950_S40a", -1);

	if (_G(flags[V224])) {
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #1", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #2", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #3", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #4", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "EMPTY NICHE", false);
	}

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		digi_preload("950_s39", -1);
		break;

	case 702:
		if (!player_been_here(703)) {
			_G(flags[V217]) = 3;
			_G(flags[V218]) = 1;
			_G(flags[V219]) = 5;
			_G(flags[V220]) = 2;
			_G(flags[V221]) = 4;
		}

		MoveScreenDelta(_G(game_buff_ptr), -320, 0);
		ws_demand_location(_G(my_walker), 630, 450);
		ws_demand_facing(_G(my_walker), 1);
		ws_walk(_G(my_walker), 625, 357, nullptr, -1, 1, true);

		break;

	case 704:
		MoveScreenDelta(_G(game_buff_ptr), -640, 0);
		player_first_walk(1299, 300, 9, 1170, 318, 9, true);

		break;

	case 706:
		player_first_walk(0, 300, 9, 130, 318, 8, true);
		break;

	case 707:
		MoveScreenDelta(_G(game_buff_ptr), -320, 0);
		ws_demand_location(_G(my_walker), 648, 358);
		ws_demand_facing(_G(my_walker), 5);
		player_set_commands_allowed(false);
		ws_hide_walker(_G(my_walker));
		_703RipGoesDownStairsSeries = series_load("703 RIP GOES DOWN STAIRS", -1, nullptr);
		_ripStairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 320, 0, 100, 0, false, callback, "rip stairs machine");
		sendWSMessage_10000(1, _ripStairsMach, _703RipGoesDownStairsSeries, 4, 39, 140, _703RipGoesDownStairsSeries, 39, 39, 0);

		break;

	default:
		digi_preload("950_s39");
		player_first_walk(1299, 400, 10, 970, 318, 10, true);
		break;
	}

	if (_G(flags[V217])) {
		hotspot_set_active(_G(currentSceneDef).hotspots, "EMPTY NICHE", false);
	} else {
		hotspot_set_active(_G(currentSceneDef).hotspots, "PRAYER WHEEL #4", false);
	}

	switch (_G(flags[V217])) {
	case 1:
		_prayerWheelMach = series_place_sprite("703 PEACE PRAYER WHEEL", 0, 640, 0, 100, 1280);
		break;

	case 2:
		_prayerWheelMach = series_place_sprite("703 INSIGHT PRAYER WHEEL", 0, 640, 0, 100, 1280);
		break;

	case 3:
		_prayerWheelMach = series_place_sprite("703 SERENITY PRAYER WHEEL", 0, 640, 0, 100, 1280);
		break;

	case 4:
		_prayerWheelMach = series_place_sprite("703 TRUTH PRAYER WHEEL", 0, 640, 0, 100, 1280);
		break;

	case 5:
		_prayerWheelMach = series_place_sprite("703 WISDOM PRAYER WHEEL", 0, 640, 0, 100, 1280);
		break;

	default:
		break;
	}

	digi_play_loop("950_s39", 3, 255, -1, -1);
}

void Room703::pre_parser() {
	if (player_said("rm702") && (inv_player_has("WISDOM WHEEL") || inv_player_has("SERENITY WHEEL") || inv_player_has("PEACE WHEEL") || inv_player_has("INSIGHT WHEEL") || inv_player_has("TRUTH WHEEL"))) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}
}

void Room703::parser() {
	//TODO Parser

}

void Room703::daemon() {
	const bool takeFl = player_said("take");

	switch (_G(kernel).trigger) {
	case 70:
		player_set_commands_allowed(false);
		setGlobals1(_ripTrekMedReachHandPos1Series, 1, 5, 7, 7, 0, 5, 10, 10, 10, 0, 10, 1, 1, 1, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 71);

		break;

	case 71:
		kernel_timing_trigger(5, 72, nullptr);
		break;

	case 72:
		sendWSMessage_120000(_G(my_walker), 73);
		break;

	case 73:
		kernel_timing_trigger(5, 74, nullptr);
		break;

	case 74:
		if (_G(flags[V217])) {
			terminateMachine(_prayerWheelMach);
			switch (_G(flags[V217])) {
			case 1:
				inv_give_to_player("PEACE WHEEL");
				_G(flags[V217]) = 0;
				player_update_info(_G(my_walker), &_G(player_info));
				digi_play("950_S40", 2, 255, -1, 950);
				kernel_examine_inventory_object("PING PEACE WHEEL", _G(master_palette), 5, 1, 362, 225, 75, nullptr, -1);

				break;

			case 2:
				inv_give_to_player("INSIGHT WHEEL");
				_G(flags[V217]) = 0;
				player_update_info(_G(my_walker), &_G(player_info));
				digi_play("950_S40", 2, 255, -1, 950);
				kernel_examine_inventory_object("PING INSIGHT WHEEL", _G(master_palette), 5, 1, 362, 225, 75, nullptr, -1);

				break;

			case 3:
				inv_give_to_player("SERENITY WHEEL");
				_G(flags[V217]) = 0;
				player_update_info(_G(my_walker), &_G(player_info));
				digi_play("950_S40", 2, 255, -1, 950);
				kernel_examine_inventory_object("PING SERENITY WHEEL", _G(master_palette), 5, 1, 362, 225, 75, nullptr, -1);

				break;

			case 4:
				inv_give_to_player("TRUTH WHEEL");
				_G(flags[V217]) = 0;
				player_update_info(_G(my_walker), &_G(player_info));
				digi_play("950_S40", 2, 255, -1, 950);
				kernel_examine_inventory_object("PING TRUTH WHEEL", _G(master_palette), 5, 1, 362, 225, 75, nullptr, -1);

				break;

			case 5:
				inv_give_to_player("WISDOM WHEEL");
				_G(flags[V217]) = 0;
				player_update_info(_G(my_walker), &_G(player_info));
				digi_play("950_S40", 2, 255, -1, 950);
				kernel_examine_inventory_object("PING WISDOM WHEEL", _G(master_palette), 5, 1, 362, 225, 75, nullptr, -1);

				break;

			default:
				break;
			}
		}
		break;

	case 75:
		sendWSMessage_130000(_G(my_walker), 76);
		break;

	case 76:
		sendWSMessage_150000(_G(my_walker), 77);
		break;

	case 77:
		hotspot_set_active(_G(currentSceneDef).hotspots, "EMPTY NICHE", true);
		hotspot_set_active(_G(currentSceneDef).hotspots, "PRAYER WHEEL #4", false);
		player_set_commands_allowed(true);

		break;

	case 80:
		player_set_commands_allowed(false);
		setGlobals1(_ripTrekMedReachHandPos1Series, 1, 5, 5, 5, 0, 5, 10, 10, 10, 0, 10, 1, 1, 1, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 81);

		break;

	case 81:
		kernel_timing_trigger(5, 82, nullptr);
		break;

	case 82:
		sendWSMessage_120000(_G(my_walker), 83);
		break;

	case 83:
		if (takeFl && _G(player).click_y <= 374) {
			switch (imath_ranged_rand(1, 3)) {
			case 1:
				digi_play("com077", 1, 255, 84, -1);
				break;

			case 2:
				digi_play("com078", 1, 255, 84, -1);
				break;

			case 3:
				digi_play("com079", 1, 255, 84, -1);
				break;

			default:
				break;
			}
		}

		break;

	case 84:
		sendWSMessage_130000(_G(my_walker), 85);
		break;

	case 85:
		sendWSMessage_150000(_G(my_walker), 86);
		break;

	case 86:
	case 91:
		player_set_commands_allowed(true);
		break;

	case 123:
		if (_field5C_mode == 124) {
			switch (_field60_should) {
			case 121:
				_703Eye4aSeries = series_load("703EYE4a", -1, nullptr);
				_field60_should = 922;
				_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 640, -53, 100, 256, false, callback, "monk");
				sendWSMessage_10000(1, _monkMach, _703Eye4aSeries, 1, 5, 123, _703Eye4aSeries, 5, 5, 0);

				break;

			case 122:
				terminateMachine(_monkMach);
				series_unload(_703Eye4aSeries);
				conv_resume(conv_get_handle());

				break;

			case 125:
				conv_load("conv703a", 10, 10, 91);
				conv_export_value_curr(0, 0);
				conv_export_value_curr(_G(flags[V211]), 1);
				conv_export_value_curr(_G(flags[V213]), 2);
				conv_export_value_curr((_G(flags[V217]) == 5) ? 1 : 0, 0);
				conv_play(conv_get_handle());

				break;

			case 922:
				switch (imath_ranged_rand(1, 2)) {
				case 1:
					_field60_should = 923;
					break;

				case 2:
					_field60_should = 924;
					break;

				default:
					break;
				}

				kernel_timing_trigger(10, 123, nullptr);

				break;

			case 923:
				_field60_should = 925;

				sendWSMessage_10000(1, _monkMach, _703Eye4aSeries, 6, 11, -1, _703Eye4aSeries, 12, 15, 4);
				sendWSMessage_1a0000(_monkMach, 9);
				digi_play(conv_sound_to_play(), 1, 255, 123, -1);

				break;

			case 924:
				_field60_should = 925;

				sendWSMessage_10000(1, _monkMach, _703Eye4aSeries, 6, 9, -1, _703Eye4aSeries, 12, 16, 4);
				sendWSMessage_1a0000(_monkMach, 9);
				digi_play(conv_sound_to_play(), 1, 255, 123, -1);

				break;

			case 925:
				_field60_should = 926;
				sendWSMessage_10000(1, _monkMach, _703Eye4aSeries, 17, 20, 123, _703Eye4aSeries, 20, 20, 0);
				break;

			case 926:
				_field60_should = 122;
				kernel_timing_trigger(10, 123, nullptr);

				break;

			default:

				break;
			}
		}

		break;

	case 127:
		if (_field5C_mode == 124) {
			switch (_field60_should) {
			case 121:
			case 122:
			case 124:
			case 125:
			case 126:
			case 127:
			case 922:
			case 923:
			case 924:
			case 925:
			case 926:
				kernel_timing_trigger(1, 123, nullptr);
				break;

			default:
				break;
			}
		}

		break;

	case 140:
		player_set_commands_allowed(true);
		ws_unhide_walker(_G(my_walker));
		terminateMachine(_ripStairsMach);
		kernel_timing_trigger(10, 141, nullptr);

		break;

	case 141:
		series_unload(_703RipGoesDownStairsSeries);
		break;

	default:
		break;
	}
}

void Room703::callback(frac16 myMessage, machine *sender) {
	kernel_trigger_dispatchx(myMessage);
}
} // namespace Rooms
} // namespace Riddle
} // namespace M4
