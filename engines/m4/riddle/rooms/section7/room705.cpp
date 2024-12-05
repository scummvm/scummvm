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

#include "m4/riddle/rooms/section7/room705.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_vmng_screen.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room705::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
}

void Room705::init() {
	digi_preload("950_S40", -1);
	digi_preload("950_S40A", -1);
	_ripTrekMedReachPos1Series = series_load("RIP TREK MED REACH HAND POS1", -1, nullptr);

	if (_G(flags[V224])) {
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #9", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #10", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #11", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #12", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "EMPTY NICHE", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "EMPTY NICHE ", false);
	}

	hotspot_set_active(_G(currentSceneDef).hotspots, _G(flags[V220]) ? "EMPTY NICHE" : "PRAYER WHEEL #9", false);

	switch (_G(flags[V220])) {
	case 1:
		_leftWheelMach = series_place_sprite("705 LEFT PEACE WHEEL", 0, 0, 0, 100, 1280);
		break;

	case 2:
		_leftWheelMach = series_place_sprite("705 LEFT INSIGHT WHEEL", 0, 0, 0, 100, 1280);
		break;

	case 3:
		_leftWheelMach = series_place_sprite("705 LEFT SERENITY WHEEL", 0, 0, 0, 100, 1280);
		break;

	case 4:
		_leftWheelMach = series_place_sprite("705 LEFT TRUTH WHEEL", 0, 0, 0, 100, 1280);
		break;

	case 5:
		_leftWheelMach = series_place_sprite("705 LEFT WISDOM WHEEL", 0, 0, 0, 100, 1280);
		break;

	default:
		break;
	}

	hotspot_set_active(_G(currentSceneDef).hotspots, _G(flags[V221]) ? "EMPTY NICHE " : "PRAYER WHEEL #12", false);

	switch (_G(flags[V221])) {
	case 1:
		_rightWheelMach = series_place_sprite("705 RT PEACE WHEEL", 0, 640, 0, 100, 1280);
		break;

	case 2:
		_rightWheelMach = series_place_sprite("705 RT INSIGHT WHEEL", 0, 640, 0, 100, 1280);
		break;

	case 3:
		_rightWheelMach = series_place_sprite("705 RT SERENITY WHEEL", 0, 640, 0, 100, 1280);
		break;

	case 4:
		_rightWheelMach = series_place_sprite("705 RT TRUTH WHEEL", 0, 0, 640, 100, 1280);
		break;

	case 5:
		_rightWheelMach = series_place_sprite("705 RT WISDOM WHEEL", 0, 0, 640, 100, 1280);
		break;

	default:
		break;
	}

	switch (_G(game).previous_room) {
	case 704:
		player_first_walk(0, 300, 3, 130, 318, 3, true);
		break;

	case 706:
		MoveScreenDelta(_G(game_buff_ptr), -640, 0);
		player_first_walk(1299, 300, 9, 1170, 318, 9, true);
		break;

	case 707:
		MoveScreenDelta(_G(game_buff_ptr), -320, 0);
		ws_demand_location(_G(my_walker), 643, 357);
		ws_demand_facing(_G(my_walker), 5);
		player_set_commands_allowed(false);
		ws_hide_walker(_G(my_walker));
		_ripGoesDownStairsSeries = series_load("705 RIP GOES DOWN STAIRS", -1, nullptr);
		_ripStairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 320, 0, 100, 0, 0, triggerMachineByHashCallback, "rip stairs machine");
		sendWSMessage_10000(1, _ripStairsMach, _ripGoesDownStairsSeries, 4, 39, 160, _ripGoesDownStairsSeries, 39, 39, 0);

		break;

	default: // includes case KERNEL_RESTORE (-2)
		digi_preload("950_s39", -1);
		break;
	}

	digi_play_loop("950_s39", 3, 255, -1, -1);
}

void Room705::pre_parser() {
	// Empty
}

void Room705::parser() {
	// TODO not yet implemented
}

void Room705::daemon() {
	bool esi = player_said("take");

	switch (_G(kernel).trigger) {
	case 70:
		player_set_commands_allowed(false);
		setGlobals1(_field24Series, 1, 5, 5, 5, 0, 5, 10, 10, 10, 0, 10, 1, 1, 1, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 71);

		break;

	case 71:
		kernel_timing_trigger(5, 72, nullptr);
		break;

	case 72:
		sendWSMessage_120000(_G(my_walker), 73);
		break;

	case 73:
		kernel_timing_trigger(5, 74);
		break;

	case 74:
		switch (_G(flags[V220])) {
		case 1:
			inv_give_to_player("PEACE WHEEL");
			_G(flags[V220]) = 0;
			terminateMachine(_leftWheelMach);
			digi_play("950_S40", 2, 255, -1, 950);
			kernel_examine_inventory_object("PING PEACE WHEEL", _G(master_palette), 5, 1, 145, 225, 75, nullptr, -1);

			break;

		case 2:
			inv_give_to_player("INSIGHT WHEEL");
			_G(flags[V220]) = 0;
			terminateMachine(_leftWheelMach);
			digi_play("950_S40", 2, 255, -1, 950);
			kernel_examine_inventory_object("PING INSIGHT WHEEL", _G(master_palette), 5, 1, 145, 225, 75, nullptr, -1);

			break;

		case 3:
			inv_give_to_player("SERENITY WHEEL");
			_G(flags[V220]) = 0;
			terminateMachine(_leftWheelMach);
			digi_play("950_S40", 2, 255, -1, 950);
			kernel_examine_inventory_object("PING SERENITY WHEEL", _G(master_palette), 5, 1, 145, 225, 75, nullptr, -1);

			break;

		case 4:
			inv_give_to_player("TRUTH WHEEL");
			_G(flags[V220]) = 0;
			terminateMachine(_leftWheelMach);
			digi_play("950_S40", 2, 255, -1, 950);
			kernel_examine_inventory_object("PING TRUTH WHEEL", _G(master_palette), 5, 1, 145, 225, 75, nullptr, -1);

			break;

		case 5:
			inv_give_to_player("WISDOM WHEEL");
			_G(flags[V220]) = 0;
			terminateMachine(_leftWheelMach);
			digi_play("950_S40", 2, 255, -1, 950);
			kernel_examine_inventory_object("PING WISDOM WHEEL", _G(master_palette), 5, 1, 145, 225, 75, nullptr, -1);

			break;

		default:
			break;
		}

		break;

	case 75:
		sendWSMessage_130000(_G(my_walker), 76);
		break;

	case 76:
		sendWSMessage_150000(_G(my_walker), 77);
		break;

	case 77:
		hotspot_set_active(_G(currentSceneDef).hotspots, "PRAYER WHEEL #9", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "EMPTY NICHE", true);
		player_set_commands_allowed(true);

		break;

	case 80:
		player_set_commands_allowed(false);
		setGlobals1(_field24Series, 1, 5, 5, 5, 0, 5, 10, 10, 10, 0, 10, 1, 1, 1, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 81);

		break;

	case 81:
		kernel_timing_trigger(5, 82);
		break;

	case 82:
		sendWSMessage_120000(_G(my_walker), 83);
		break;

	case 83:
		if (esi && _G(player).click_y <= 374) {
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
		sendWSMessage_130000(_G(my_walker), 86);
		break;

	case 86:
		sendWSMessage_150000(_G(my_walker), 87);
		break;

	case 87:
	case 91:
	case 94:
		player_set_commands_allowed(true);
		break;

	case 123:
		if (_field60_mode == 124) {
			switch (_field64_should) {
			case 121:
				_705Eye1aSeries = series_load("705eye1a", -1, nullptr);
				_field64_should = 822;
				_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 256, false, triggerMachineByHashCallback, "monk");
				sendWSMessage_10000(1, _monkMach, _705Eye1aSeries, 1, 4, 123, _705Eye1aSeries, 5, 10, 4);
				sendWSMessage_1a0000(_monkMach, 9);

				break;

			case 122:
				terminateMachine(_monkMach);
				series_unload(_705Eye1aSeries);
				conv_resume(conv_get_handle());

				break;

			case 125:
				conv_load("conv705a", 10, 10, 91);
				conv_export_value_curr(0, 0);
				conv_export_value_curr(_G(flags[V211]), 1);
				conv_export_value_curr(_G(flags[V213]), 2);
				conv_export_value_curr(_G(flags[V220]) == 1 ? 1 : 0, 0);
				conv_play(conv_get_handle());

				break;

			case 822:
				_field64_should = 823;
				digi_play(conv_sound_to_play(), 1, 255, 123, -1);

				break;

			case 823:
				_field64_should = 824;
				sendWSMessage_10000(1, _monkMach, _705Eye1aSeries, 11, 14, 123, _705Eye1aSeries, 14, 14, 0);

				break;

			case 824:
				_field64_should = 122;
				kernel_timing_trigger(10, 123, nullptr);

				break;

			default: // includes case 126
				break;
			}

		}

		break;

	case 127:
		if (_field60_mode == 124) {
			switch (_field64_should) {
			case 121:
			case 122:
			case 124:
			case 125:
			case 126:
			case 127:
			case 822:
			case 823:
			case 824:
				kernel_timing_trigger(1, 123, nullptr);
				break;

			default:
				break;
			}
		}

		break;

	case 133:
		if (_field60_mode == 134) {
			switch (_field64_should) {
			case 131:
				_705Eye4aSeries = series_load("705eye4a", -1, nullptr);
				_field64_should = 832;
				_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 640, -53, 100, 256, false, triggerMachineByHashCallback, "monk");
				sendWSMessage_10000(1, _monkMach, _705Eye4aSeries, 1, 5, 133, _705Eye4aSeries, 5, 5, 0);

				break;

			case 132:
				terminateMachine(_monkMach);
				series_unload(_705Eye4aSeries);
				conv_resume(conv_get_handle());

				break;

			case 135:
				conv_load("conv705b", 10, 10, 94);
				conv_export_value_curr(0, 0);
				conv_export_value_curr(_G(flags[V211]), 1);
				conv_export_value_curr(_G(flags[V213]), 2);
				conv_export_value_curr(_G(flags[V221]) == 2 ? 1 : 0, 0);
				conv_play(conv_get_handle());

				break;

			case 832:
				switch (imath_ranged_rand(1, 2)) {
				case 1:
					_field64_should = 833;
					break;

				case 2:
					_field64_should = 834;
					break;

				default:
					break;
				}

				kernel_timing_trigger(10, 133);

				break;

			case 833:
				_field64_should = 835;
				sendWSMessage_10000(1, _monkMach, _705Eye4aSeries, 6, 7, -1, _705Eye4aSeries, 8, 12, 4);
				sendWSMessage_1a0000(_monkMach, 9);
				digi_play(conv_sound_to_play(), 1, 255, 133, -1);

				break;

			case 834:
				_field64_should = 835;
				sendWSMessage_10000(1, _monkMach, _705Eye4aSeries, 8, 8, -1, _705Eye4aSeries, 8, 12, 4);
				sendWSMessage_1a0000(_monkMach, 9);
				digi_play(conv_sound_to_play(), 1, 255, 133, -1);

				break;

			case 835:
				_field64_should = 836;
				sendWSMessage_10000(1, _monkMach, _705Eye4aSeries, 13, 16, 133, _705Eye4aSeries, 16, 16, 0);

				break;

			case 836:
				_field64_should = 132;
				kernel_timing_trigger(10, 133);
				break;

			default: // including 136
				break;

			}
		}

		break;

	case 137:
		if (_field60_mode == 134) {
			switch (_field60_mode) {
			case 131:
			case 132:
			case 134:
			case 135:
			case 136:
			case 137:
			case 832:
			case 833:
			case 834:
			case 835:
			case 836:
				kernel_timing_trigger(1, 133, nullptr);
				break;

			default:
				break;
			}
		}

		break;

	case 140:
		player_set_commands_allowed(false);
		setGlobals1(_field24Series, 1, 5, 5, 5, 0, 5, 10, 10, 10, 0, 10, 1, 1, 1, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 141);

		break;

	case 141:
		kernel_timing_trigger(5, 142, nullptr);
		break;

	case 142:
		sendWSMessage_120000(_G(my_walker), 143);
		break;

	case 143:
		kernel_timing_trigger(5, 144, nullptr);
		break;

	case 144:
		switch (_G(flags[V221])) {
		case 1:
			inv_give_to_player("PEACE WHEEL");
			_G(flags[V221]) = 0;
			terminateMachine(_rightWheelMach);
			digi_play("950_S40", 2, 255, -1, 950);
			kernel_examine_inventory_object("PING PEACE WHEEL", _G(master_palette), 5, 1, 388, 225, 145, nullptr, -1);

			break;

		case 2:
			inv_give_to_player("INSIGHT WHEEL");
			_G(flags[V221]) = 0;
			terminateMachine(_rightWheelMach);
			digi_play("950_S40", 2, 255, -1, 950);
			kernel_examine_inventory_object("PING INSIGHT WHEEL", _G(master_palette), 5, 1, 388, 225, 145, nullptr, -1);

			break;

		case 3:
			inv_give_to_player("SERENITY WHEEL");
			_G(flags[V221]) = 0;
			terminateMachine(_rightWheelMach);
			digi_play("950_S40", 2, 255, -1, 950);
			kernel_examine_inventory_object("PING SERENITY WHEEL", _G(master_palette), 5, 1, 388, 225, 145, nullptr, -1);

			break;

		case 4:
			inv_give_to_player("TRUTH WHEEL");
			_G(flags[V221]) = 0;
			terminateMachine(_rightWheelMach);
			digi_play("950_S40", 2, 255, -1, 950);
			kernel_examine_inventory_object("PING TRUTH WHEEL", _G(master_palette), 5, 1, 388, 225, 145, nullptr, -1);

			break;

		case 5:
			inv_give_to_player("WISDOM WHEEL");
			_G(flags[V221]) = 0;
			terminateMachine(_rightWheelMach);
			digi_play("950_S40", 2, 255, -1, 950);
			kernel_examine_inventory_object("PING WISDOM WHEEL", _G(master_palette), 5, 1, 388, 225, 145, nullptr, -1);

			break;

		default:
			break;
		}

		break;

	case 145:
		sendWSMessage_130000(_G(my_walker), 146);
		break;

	case 146:
		sendWSMessage_150000(_G(my_walker), 147);
		break;

	case 147:
		hotspot_set_active(_G(currentSceneDef).hotspots, "PRAYER WHEEL #12", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "EMPTY NICHE ", true);
		player_set_commands_allowed(true);

		break;

	case 150:
		digi_play("705R07", 1, 255, 151, -1);
		break;

	case 151:
		_705Monk1Series = series_load("705 MONK 1", -1, nullptr);
		_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 256, false, triggerMachineByHashCallback, "monk");
		sendWSMessage_10000(1, _monkMach, _705Monk1Series, 1, 7, 951, _705Monk1Series, 7, 7, 4);

		break;

	case 152:
		sendWSMessage_10000(1, _monkMach, _705Monk1Series, 16, 19, 952, _705Monk1Series, 19, 19, 0);
		break;

	case 153:
		digi_play("705J08", 1, 255, 154, -1);
		break;

	case 154:
		_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 256, false, triggerMachineByHashCallback, "monk");
		sendWSMessage_10000(1, _monkMach, _705Monk1Series, 1, 7, 954, _705Monk1Series, 7, 7, 4);

		break;

	case 155:
		sendWSMessage_10000(1, _monkMach, _705Monk1Series, 16, 19, 955, _705Monk1Series, 19, 19, 0);
		digi_play("706_s01", 2, 127, -1, -1);
		_G(flags[V222]) = 1;
		player_set_commands_allowed(true);

		break;

	case 160:
		player_set_commands_allowed(true);
		ws_unhide_walker(_G(my_walker));
		terminateMachine(_ripStairsMach);
		kernel_timing_trigger(10, 161, nullptr);

		break;

	case 161:
		series_unload(_ripGoesDownStairsSeries);
		break;

	case 951:
		sendWSMessage_10000(1, _monkMach, _705Monk1Series, 8, 8, -1, _705Monk1Series, 9, 15, 4);
		sendWSMessage_1a0000(_monkMach, 9);
		digi_play("705J03", 1, 255, 152, -1);

		break;

	case 952:
		terminateMachine(_monkMach);
		digi_play("705R08", 1, 255, 154, -1);

		break;

	case 954:
		sendWSMessage_10000(1, _monkMach, _705Monk1Series, 8, 8, -1, _705Monk1Series, 9, 15, 4);
		sendWSMessage_1a0000(_monkMach, 9);
		digi_play("705J04", 1, 255, 155, -1);

		break;

	case 955:
		terminateMachine(_monkMach);
		break;

	default:
		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
