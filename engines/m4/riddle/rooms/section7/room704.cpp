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

#include "m4/riddle/rooms/section7/room704.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_vmng_screen.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room704::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
}

void Room704::init() {
	digi_preload("950_S40", -1);
	digi_preload("950_S40A", -1);
	_ripTrekMedReachHandPos1Series = series_load("RIP TREK MED REACH HAND POS1", -1, nullptr);

	if (_G(flags[V224])) {
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #5", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #6", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #7", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #8", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "EMPTY NICHE", false);
	}

	hotspot_set_active(_G(currentSceneDef).hotspots, _G(flags[V218]) ? "EMPTY NICHE" : "PRAYER WHEEL #6", false);

	switch (_G(flags[V218])) {
	case 1:
		_prayerWheelMach = series_place_sprite("704 PEACE PRAYER WHEEL", 0, 0, 0, 100, 1280);
		break;

	case 2:
		_prayerWheelMach = series_place_sprite("704 INSIGHT PRAYER WHEEL", 0, 0, 0, 100, 1280);
		break;

	case 3:
		_prayerWheelMach = series_place_sprite("704 SERENITY PRAYER WHEEL", 0, 0, 0, 100, 1280);
		break;

	case 4:
		_prayerWheelMach = series_place_sprite("704 TRUTH PRAYER WHEEL", 0, 0, 0, 100, 1280);
		break;

	case 5:
		_prayerWheelMach = series_place_sprite("704 WISDOM PRAYER WHEEL", 0, 0, 0, 100, 1280);
		break;

	default:
		break;
	}

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		// Same as default?
		digi_preload("950_s39", -1);
		break;

	case 703:
		player_first_walk(0, 300, 3, 130, 318, 3, true);
		break;

	case 705:
		MoveScreenDelta(_G(game_buff_ptr), -640, 0);
		ws_demand_location(_G(my_walker), 1299, 300);
		ws_demand_facing(_G(my_walker), 9);
		ws_walk(_G(my_walker), 1170, 318, nullptr, -1, 0, true);

		break;

	case 707:
		MoveScreenDelta(_G(game_buff_ptr), -320, 0);
		ws_demand_location(_G(my_walker), 645, 356);
		ws_demand_facing(_G(my_walker), 5);
		_ripGoesDownStairsSeries = series_load("704 RIP GOES DOWN STAIRS", -1, nullptr);
		player_set_commands_allowed(false);
		ws_hide_walker(_G(my_walker));
		_ripStairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 320, 0, 100, 0, false, callback, "rip stairs machine");
		sendWSMessage_10000(1, _ripStairsMach, _ripGoesDownStairsSeries, 4, 39, 140, _ripGoesDownStairsSeries, 39, 39, 0);

		break;

	default:
		digi_preload("950_s39", -1);
		break;
	}

	digi_play_loop("950_s39", 3, 255, -1, -1);
}

void Room704::pre_parser() {
	// No implementation
}

void Room704::parser() {
	const bool lookFl = player_said_any("look", "look at");
	const bool talkFl = player_said_any("talk", "talk to");
	const bool takeFl = player_said("take");
	const bool gearFl = player_said_any("push", "pull", "gear", "open", "close");

	if (player_said("conv704a")) {
		if (_G(kernel).trigger == 90)
			conv_resume(conv_get_handle());
		else
			conv704a();
	} else if (player_said("PRAYER WHEEL BROCHURE", "EMPTY NICHE") && inv_player_has("PRAYER WHEEL BROCHURE"))
		digi_play("com123", 1, 255, -1, 997);
	else if (player_said("PRAYER WHEEL BROCHURE", "PRAYER WHEEL #5") && inv_player_has("PRAYER WHEEL BROCHURE"))
		playCheckBrochureAnim(_G(kernel).trigger, "704r04");
	else if (player_said("PRAYER WHEEL BROCHURE", "PRAYER WHEEL #6") && inv_player_has("PRAYER WHEEL BROCHURE")) {
		switch (_G(flags[V218])) {
		case 1:
			playCheckBrochureAnim(_G(kernel).trigger, "704r09");
			break;

		case 2:
			playCheckBrochureAnim(_G(kernel).trigger, "705r04");
			break;

		case 3:
			playCheckBrochureAnim(_G(kernel).trigger, "703r11");
			break;

		case 4:
			playCheckBrochureAnim(_G(kernel).trigger, "705r14a");
			break;

		case 5:
			playCheckBrochureAnim(_G(kernel).trigger, "706r10");
			break;

		default:
			break;
		}
	} // player_said("PRAYER WHEEL BROCHURE", "PRAYER WHEEL #6") && inv_player_has("PRAYER WHEEL BROCHURE")

	else if (player_said("PRAYER WHEEL BROCHURE", "PRAYER WHEEL #7") && inv_player_has("PRAYER WHEEL BROCHURE"))
		playCheckBrochureAnim(_G(kernel).trigger, "704r11");
	else if (player_said("PRAYER WHEEL BROCHURE", "PRAYER WHEEL #8") && inv_player_has("PRAYER WHEEL BROCHURE"))
		playCheckBrochureAnim(_G(kernel).trigger, "704r12");
	else if (player_said("PRAYER WHEEL BROCHURE", "SERENITY WHEEL") && inv_player_has("PRAYER WHEEL BROCHURE") && inv_player_has("SERENITY WHEEL")) {
		player_set_commands_allowed(false);
		digi_play("703r11", 1, 255, -1, -1);
		player_set_commands_allowed(true);
	} else if (player_said("PRAYER WHEEL BROCHURE", "INSIGHT WHEEL") && inv_player_has("PRAYER WHEEL BROCHURE") && inv_player_has("INSIGHT WHEEL")) {
		player_set_commands_allowed(false);
		digi_play("705r04", 1, 255, -1, -1);
		player_set_commands_allowed(true);
	} else if (player_said("PRAYER WHEEL BROCHURE", "PEACE WHEEL") && inv_player_has("PRAYER WHEEL BROCHURE") && inv_player_has("PEACE WHEEL")) {
		player_set_commands_allowed(false);
		digi_play("704r09", 1, 255, -1, -1);
		player_set_commands_allowed(true);
	} else if (player_said("PRAYER WHEEL BROCHURE", "TRUTH WHEEL") && inv_player_has("PRAYER WHEEL BROCHURE") && inv_player_has("TRUTH WHEEL")) {
		player_set_commands_allowed(false);
		digi_play("705r14a", 1, 255, -1, -1);
		player_set_commands_allowed(true);
	} else if (player_said("PRAYER WHEEL BROCHURE", "WISDOM WHEEL") && inv_player_has("PRAYER WHEEL BROCHURE") && inv_player_has("WISDOM WHEEL")) {
		player_set_commands_allowed(false);
		digi_play("706r10", 1, 255, -1, -1);
		player_set_commands_allowed(true);
	} else if (talkFl && player_said_any("MONK #5", "MONK #6", "MONK #7", "MONK #8") && _G(flags[V286]))
		digi_play("com142", 1, 255, -1, -1);
	else if (talkFl && player_said("MONK #5")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			digi_play("com081", 1, 255, 2, -1);
			break;

		case 2:
			_704Monk1Series = series_load("704 MONK 1", -1, nullptr);
			_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 256, false, callback, "monk");
			sendWSMessage_10000(1, _monkMach, _704Monk1Series, 1, 13, 3, _704Monk1Series, 13, 13, 0);

			break;

		case 3:
			sendWSMessage_10000(1, _monkMach, _704Monk1Series, 14, 18, -1, _704Monk1Series, 19, 22, 4);
			sendWSMessage_1a0000(_monkMach, 9);
			digi_play("704E01", 1, 255, 4, -1);

			break;

		case 4:
			sendWSMessage_10000(1, _monkMach, _704Monk1Series, 23, 26, 5, _704Monk1Series, 26, 26, 0);
			break;

		case 5:
			terminateMachine(_monkMach);
			series_unload(_704Monk1Series);
			player_set_commands_allowed(true);

			break;

		default:
			break;
		}
	} // talkFl && player_said("MONK #5")

	else if (talkFl && player_said("MONK #6")) {
		switch (_G(kernel).trigger) {
		case 1:
			player_set_commands_allowed(false);
			digi_play("com081", 1, 255, 2, -1);
			break;

		case 2:
			_704Monk2Series = series_load("704 MONK 2", -1, nullptr);
			_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 256, false, callback, "monk");
			sendWSMessage_10000(1, _monkMach, _704Monk2Series, 1, 17, 3, _704Monk2Series, 17, 17, 4);

			break;

		case 3:
			sendWSMessage_10000(1, _monkMach, _704Monk2Series, 18, 21, -1, _704Monk2Series, 22, 24, 4);
			sendWSMessage_1a0000(_monkMach, 9);
			digi_play("704F01", 1, 255, 4, -1);

			break;

		case 4:
			sendWSMessage_10000(1, _monkMach, _704Monk2Series, 25, 28, 5, _704Monk2Series, 28, 28, 0);
			break;

		case 5:
			terminateMachine(_monkMach);
			series_unload(_704Monk2Series);
			player_set_commands_allowed(true);
			_field58_mode = 124;
			_field5C_should = 125;
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_timing_trigger(10, 127, nullptr);
			_G(kernel).trigger_mode = KT_PARSE;

			break;

		default:
			break;
		}
	} // talkFl && player_said("MONK #6")

	else if (talkFl && player_said("MONK #7")) {
		switch (_G(kernel).trigger) {
		case 1:
			player_set_commands_allowed(false);
			digi_play("com081", 1, 255, 2, -1);
			break;

		case 2:
			_704Monk3Series = series_load("704 MONK 3", -1, nullptr);
			_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 640, -53, 100, 256, false, callback, "monk");
			sendWSMessage_10000(1, _monkMach, _704Monk3Series, 1, 22, 3, _704Monk3Series, 22, 22, 0);

			break;

		case 3:
			sendWSMessage_10000(1, _monkMach, _704Monk3Series, 23, 28, -1, _704Monk3Series, 29, 31, 4);
			sendWSMessage_1a0000(_monkMach, 9);
			digi_play("704H01", 1, 255, 4, -1);

			break;

		case 4:
			sendWSMessage_10000(1, _monkMach, _704Monk3Series, 32, 35, 5, _704Monk3Series, 35, 35, 0);
			break;

		case 5:
			terminateMachine(_monkMach);
			series_unload(_704Monk3Series);
			player_set_commands_allowed(true);

			break;

		default:
			break;
		}
	} // talkFl && player_said("MONK #7")

	else if (talkFl && player_said("MONK #8")) {
		switch (_G(kernel).trigger) {
		case 1:
			player_set_commands_allowed(false);
			digi_play("com081", 1, 255, 2, -1);
			break;

		case 2:
			_704Monk4Series = series_load("704 MONK 4", -1, nullptr);
			_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 640, -53, 100, 256, false, callback, "monk");
			sendWSMessage_10000(1, _monkMach, _704Monk4Series, 1, 12, 3, _704Monk4Series, 12, 12, 0);
			sendWSMessage_190000(_monkMach, 9);

			break;

		case 3:
			sendWSMessage_10000(1, _monkMach, _704Monk4Series, 13, 13, -1, _704Monk4Series, 13, 15, 4);
			sendWSMessage_1a0000(_monkMach, 9);
			digi_play("704I01", 1, 255, 4, -1);

			break;

		case 4:
			sendWSMessage_10000(1, _monkMach, _704Monk4Series, 16, 19, 5, _704Monk4Series, 16, 19, 0);
			break;

		case 5:
			terminateMachine(_monkMach);
			series_unload(_704Monk4Series);
			player_set_commands_allowed(true);

			break;

		default:
			break;
		}
	} // talkFl && player_said("MONK #8")

	else if (lookFl && player_said("GRATE")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_update_info(_G(my_walker), &_G(player_info));
			switch (_G(player_info).facing) {
			case 1:
			case 11:
				_ripLooksDownSeries = series_load("RIP LOOKS DOWN POS1", -1, nullptr);
				setGlobals1(_ripLooksDownSeries, 1, 14, 14, 14, 0, 14, 29, 29, 29, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
				sendWSMessage_110000(_G(my_walker), 2);
				break;

			case 2:
			case 10:
				_ripLooksDownSeries = series_load("RIP LOOKS DOWN POS2", -1, nullptr);
				setGlobals1(_ripLooksDownSeries, 1, 14, 14, 14, 0, 14, 29, 29, 29, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
				sendWSMessage_110000(_G(my_walker), 2);
				break;

			case 3:
			case 9:
				_ripLooksDownSeries = series_load("RIP TREK LOOK DOWN POS3", -1, nullptr);
				setGlobals1(_ripLooksDownSeries, 1, 11, 11, 11, 0, 11, 22, 22, 22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
				sendWSMessage_110000(_G(my_walker), 2);
				break;

			case 4:
			case 8:
				_ripLooksDownSeries = series_load("RIP LOOKS DOWN POS4", -1, nullptr);
				setGlobals1(_ripLooksDownSeries, 1, 16, 16, 16, 0, 16, 33, 33, 33, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
				sendWSMessage_110000(_G(my_walker), 2);
				break;

			case 5:
			case 7:
				_ripLooksDownSeries = series_load("RIP LOOKS DOWN POS5", -1, nullptr);
				setGlobals1(_ripLooksDownSeries, 1, 15, 15, 15, 0, 15, 31, 31, 31, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
				sendWSMessage_110000(_G(my_walker), 2);
				break;

			default:
				// No value set for _ripLookDownSeries, most like an impossible value so I skipped
				break;
			}
			break;

		case 2:
			digi_play("707r04", 1, 255, -1, -1);
			kernel_timing_trigger(10, 3, nullptr);
			break;

		case 3:
			sendWSMessage_120000(_G(my_walker), 4);
			break;

		case 4:
			sendWSMessage_150000(_G(my_walker), 5);
			break;

		case 5:
			series_unload(_ripLooksDownSeries);
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
	} // ecx && player_said("GRATE")

	else if (player_said("GRATE")) {
		digi_play("707r02", 1, 255, -1, -1);
	} else if (lookFl && player_said("MONK #5")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_704Monk1Series = series_load("704 MONK 1", -1, nullptr);
			_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0, false, callback, "monk");
			sendWSMessage_10000(1, _monkMach, _704Monk1Series, 1, 26, 2, _704Monk1Series, 26, 26, 0);

			break;

		case 2:
			terminateMachine(_monkMach);
			series_unload(_704Monk1Series);
			player_set_commands_allowed(true);

			break;

		default:
			break;
		}
	} // ecx && player_said("MONK #5")

	else if (lookFl && player_said("MONK #6")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_704Monk2Series = series_load("704 MONK 2", -1, nullptr);
			_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0, false, callback, "monk");
			sendWSMessage_10000(1, _monkMach, _704Monk2Series, 1, 28, 2, _704Monk2Series, 28, 28, 0);

			break;

		case 2:
			terminateMachine(_monkMach);
			series_unload(_704Monk2Series);
			player_set_commands_allowed(true);

			break;

		default:
			break;
		}
	} // ecx && player_said("MONK #6")

	else if (lookFl && player_said("MONK #7")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_704Monk3Series = series_load("704 MONK 3", -1, nullptr);
			_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 640, -53, 100, 0, false, callback, "monk");
			sendWSMessage_10000(1, _monkMach, _704Monk3Series, 1, 35, 2, _704Monk3Series, 35, 35, 0);

			break;

		case 2:
			terminateMachine(_monkMach);
			series_unload(_704Monk3Series);
			player_set_commands_allowed(true);

			break;

		default:
			break;
		}
	} // ecx && player_said("MONK #7")

	else if (lookFl && player_said("MONK #8")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_704Monk4Series = series_load("704 MONK 4", -1, nullptr);
			_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 640, -53, 100, 0, false, callback, "monk");
			sendWSMessage_10000(1, _monkMach, _704Monk4Series, 1, 19, 2, _704Monk4Series, 19, 19, 0);

			break;

		case 2:
			terminateMachine(_monkMach);
			series_unload(_704Monk4Series);
			player_set_commands_allowed(true);

			break;

		default:
			break;
		}
	} // ecx && player_said("MONK #8")

	else if (player_said("TRUTH WHEEL", "EMPTY NICHE")) {
		if (_G(kernel).trigger != -1 || inv_player_has("TRUTH WHEEL")) {
			useWheelOnNiche(_G(kernel).trigger, 4);
		} else {
			player_set_commands_allowed(true);
		}
	} else if (player_said("PEACE WHEEL", "EMPTY NICHE")) {
		if (_G(kernel).trigger != -1 || inv_player_has("PEACE WHEEL")) {
			useWheelOnNiche(_G(kernel).trigger, 1);
		} else {
			player_set_commands_allowed(true);
		}
	} else if (player_said("WISDOM WHEEL", "EMPTY NICHE")) {
		if (_G(kernel).trigger != -1 || inv_player_has("WISDOM WHEEL")) {
			useWheelOnNiche(_G(kernel).trigger, 5);
		} else {
			player_set_commands_allowed(true);
		}
	} else if (player_said("INSIGHT WHEEL", "EMPTY NICHE")) {
		if (_G(kernel).trigger != -1 || inv_player_has("INSIGHT WHEEL")) {
			useWheelOnNiche(_G(kernel).trigger, 2);
		} else {
			player_set_commands_allowed(true);
		}
	} else if (player_said("SERENITY WHEEL", "EMPTY NICHE")) {
		if (_G(kernel).trigger != -1 || inv_player_has("SERENITY WHEEL")) {
			useWheelOnNiche(_G(kernel).trigger, 3);
		} else {
			player_set_commands_allowed(true);
		}
	} else if (player_said("TRUTH WHEEL", "PRAYER WHEEL #5") || player_said("PEACE WHEEL", "PRAYER WHEEL #5") || player_said("WISDOM WHEEL", "PRAYER WHEEL #5") || player_said("INSIGHT WHEEL", "PRAYER WHEEL #5") || player_said("SERENITY WHEEL", "PRAYER WHEEL #5")
	           || player_said("TRUTH WHEEL", "PRAYER WHEEL #6") || player_said("PEACE WHEEL", "PRAYER WHEEL #6") || player_said("WISDOM WHEEL", "PRAYER WHEEL #6") || player_said("INSIGHT WHEEL", "PRAYER WHEEL #6") || player_said("SERENITY WHEEL", "PRAYER WHEEL #6")
	           || player_said("TRUTH WHEEL", "PRAYER WHEEL #7") || player_said("PEACE WHEEL", "PRAYER WHEEL #7") || player_said("WISDOM WHEEL", "PRAYER WHEEL #7") || player_said("INSIGHT WHEEL", "PRAYER WHEEL #7") || player_said("SERENITY WHEEL", "PRAYER WHEEL #7")
	           || player_said("TRUTH WHEEL", "PRAYER WHEEL #8") || player_said("PEACE WHEEL", "PRAYER WHEEL #8") || player_said("WISDOM WHEEL", "PRAYER WHEEL #8") || player_said("INSIGHT WHEEL", "PRAYER WHEEL #8") || player_said("SERENITY WHEEL", "PRAYER WHEEL #8")
	) {
		digi_play("com080", 1, 255, -1, -1);
	} else if (lookFl && player_said("EMPTY NICHE")) {
		digi_play("com127", 1, 255, -1, -1);
	} else if (lookFl && player_said_any("PRAYER WHEEL #5", "PRAYER WHEEL #7", "PRAYER WHEEL #8")) {
		digi_play("com076", 1, 255, -1, -1);
	} else if (lookFl && player_said("PRAYER WHEEL #6") && _G(flags[V218])) {
		digi_play("com076", 1, 255, -1, -1);
	} else if (takeFl && !player_said_any("PRAYER WHEEL #5", "PRAYER WHEEL #6", "PRAYER WHEEL #7", "PRAYER WHEEL #8") && _G(flags[V286])) {
		digi_play(_G(flags[V224]) ? "706r26" : "com143", 1, 255, -1, -1);
	} else if (takeFl && player_said_any("PRAYER WHEEL #5", "PRAYER WHEEL #6", "PRAYER WHEEL #7", "PRAYER WHEEL #8")) {
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_trigger_dispatchx(kernel_trigger_create(player_said("PRAYER WHEEL #6") ? 70 : 80));
		_G(kernel).trigger_mode = KT_DAEMON;
	} else if (lookFl && player_said(" ")) {
		digi_play(_G(flags[V224]) ? "706r24" : "com075", 1, 255, -1, -1);
	} else if (player_said("CUPOLA")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_ripGoesUpStairsSeries = series_load("704 RIP GOES UP STAIRS", -1, nullptr);
			ws_hide_walker(_G(my_walker));
			_ripStairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 320, 0, 100, 0, false, callback, "rip stairs machine");
			sendWSMessage_10000(1, _ripStairsMach, _ripGoesUpStairsSeries, 1, 29, 2, _ripGoesUpStairsSeries, 29, 29, 0);

			break;

		case 2:
			sendWSMessage_10000(1, _ripStairsMach, _ripGoesUpStairsSeries, 30, 39, -1, _ripGoesUpStairsSeries, 39, 39, 0);
			disable_player_commands_and_fade_init(3);

			break;

		case 3:
			terminateMachine(_ripStairsMach);
			player_set_commands_allowed(true);
			adv_kill_digi_between_rooms(false);
			digi_play_loop("950_s39", 3, 255, -1, -1);
			_G(game).new_room = 707;

			break;

		default:
			break;
		}
	} // player_said("CUPOLA")

	else if (player_said("North Face")) {
		switch (_G(kernel).trigger) {
		case -1:
			disable_player_commands_and_fade_init(4);
			break;

		case 4:
			adv_kill_digi_between_rooms(false);
			digi_play_loop("950_s39", 3, 255, -1, -1);
			_G(game).new_room = 705;

			break;

		default:
			break;
		}
	} else if (player_said("South Face")) {
		switch (_G(kernel).trigger) {
		case -1:
			disable_player_commands_and_fade_init(4);
			break;

		case 4:
			adv_kill_digi_between_rooms(false);
			digi_play_loop("950_s39", 3, 255, -1, -1);
			_G(game).new_room = 703;

			break;

		default:
			break;
		}
	} else if (!gearFl && !takeFl && !player_said_any("PRAYER WHEEL #5", "PRAYER WHEEL #6", "PRAYER WHEEL #7", "PRAYER WHEEL #8")) {
		digi_play("com017", 1, 255, -1, -1);
	} else
		return;

	_G(player).command_ready = 0;
}

void Room704::daemon() {
	const bool takeFl = player_said("take");

	switch (_G(kernel).trigger) {
	case 70:
		player_set_commands_allowed(false);
		setGlobals1(_ripTrekMedReachHandPos1Series, 1, 5, 5, 5, 0, 5, 10, 10, 10, 0, 10, 1, 1, 1, 0, 0, 0, 0, 0, 0);
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
		switch (_G(flags[V218])) {
		case 1:
			inv_give_to_player("PEACE WHEEL");
			_G(flags[V218]) = 0;
			terminateMachine(_prayerWheelMach);
			player_update_info(_G(my_walker), &_G(player_info));
			digi_play("950_S40", 2, 255, -1, 950);
			kernel_examine_inventory_object("PING PEACE WHEEL", _G(master_palette), 5, 1, _G(player_info).camera_x + 353, 225, 75, nullptr, -1);

			break;

		case 2:
			inv_give_to_player("INSIGHT WHEEL");
			_G(flags[V218]) = 0;
			terminateMachine(_prayerWheelMach);
			player_update_info(_G(my_walker), &_G(player_info));
			digi_play("950_S40", 2, 255, -1, 950);
			kernel_examine_inventory_object("PING INSIGHT WHEEL", _G(master_palette), 5, 1, _G(player_info).camera_x + 353, 225, 75, nullptr, -1);

			break;

		case 3:
			inv_give_to_player("SERENITY WHEEL");
			_G(flags[V218]) = 0;
			terminateMachine(_prayerWheelMach);
			player_update_info(_G(my_walker), &_G(player_info));
			digi_play("950_S40", 2, 255, -1, 950);
			kernel_examine_inventory_object("PING SERENITY WHEEL", _G(master_palette), 5, 1, _G(player_info).camera_x + 353, 225, 75, nullptr, -1);

			break;

		case 4:
			inv_give_to_player("TRUTH WHEEL");
			_G(flags[V218]) = 0;
			terminateMachine(_prayerWheelMach);
			player_update_info(_G(my_walker), &_G(player_info));
			digi_play("950_S40", 2, 255, -1, 950);
			kernel_examine_inventory_object("PING TRUTH WHEEL", _G(master_palette), 5, 1, _G(player_info).camera_x + 353, 225, 75, nullptr, -1);

			break;

		case 5:
			inv_give_to_player("WISDOM WHEEL");
			_G(flags[V218]) = 0;
			terminateMachine(_prayerWheelMach);
			player_update_info(_G(my_walker), &_G(player_info));
			digi_play("950_S40", 2, 255, -1, 950);
			kernel_examine_inventory_object("PING WISDOM WHEEL", _G(master_palette), 5, 1, _G(player_info).camera_x + 353, 225, 75, nullptr, -1);

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
		player_set_commands_allowed(true);
		hotspot_set_active(_G(currentSceneDef).hotspots, "EMPTY NICHE", true);
		hotspot_set_active(_G(currentSceneDef).hotspots, "PRAYER WHEEL #6", false);

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
		if (_field58_mode != 124)
			break;

		switch (_field5C_should) {
		case 121:
			_704Eye2aSeries = series_load("704EYE2a", -1, nullptr);
			_field5C_should = 922;
			_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 256, false, callback, "monk");
			sendWSMessage_10000(1, _monkMach, _704Eye2aSeries, 1, 5, 123, _704Eye2aSeries, 5, 5, 0);

			break;

		case 122:
			terminateMachine(_monkMach);
			series_unload(_704Eye2aSeries);
			conv_resume(conv_get_handle());

			break;

		case 125:
			conv_load("conv704a", 10, 10, 91);
			conv_export_value_curr(0, 0);
			conv_export_value_curr(_G(flags[V211]), 1);
			conv_export_value_curr(_G(flags[V213]), 2);
			conv_export_value_curr((_G(flags[V218]) == 3) ? 1 : 0, 0);
			conv_play(conv_get_handle());

			break;

		case 922:
			switch (imath_ranged_rand(1, 2)) {
			case 1:
				_field5C_should = 923;
				break;

			case 2:
				_field5C_should = 924;
				break;

			default:
				break;
			}

			kernel_timing_trigger(10, 123, nullptr);

			break;

		case 923:
			_field5C_should = 925;
			sendWSMessage_10000(1, _monkMach, _704Eye2aSeries, 6, 7, -1, _704Eye2aSeries, 11, 13, 4);
			sendWSMessage_1a0000(_monkMach, 9);
			digi_play(conv_sound_to_play(), 1, 255, 123, -1);

			break;

		case 924:
			_field5C_should = 925;
			sendWSMessage_10000(1, _monkMach, _704Eye2aSeries, 6, 10, -1, _704Eye2aSeries, 11, 13, 4);
			sendWSMessage_1a0000(_monkMach, 9);
			digi_play(conv_sound_to_play(), 1, 255, 123, -1);

			break;

		case 925:
			_field5C_should = 926;
			sendWSMessage_10000(1, _monkMach, _704Eye2aSeries, 14, 17, 123, _704Eye2aSeries, 17, 17, 0);

			break;

		case 926:
			_field5C_should = 122;
			kernel_timing_trigger(10, 123, nullptr);

			break;

		default:
			break;
		}

		break;

	case 127:
		if (_field58_mode != 124)
			break;

		switch (_field5C_should) {
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

		break;

	case 140:
		player_set_commands_allowed(true);
		ws_unhide_walker(_G(my_walker));
		terminateMachine(_ripStairsMach);
		kernel_timing_trigger(10, 141, nullptr);

		break;

	case 141:
		series_unload(_ripGoesDownStairsSeries);
		break;

	default:
		break;

	}
}


void Room704::callback(frac16 myMessage, machine *sender) {
	kernel_trigger_dispatchx(myMessage);
}

void Room704::conv704a() {
	const char *digiName = conv_sound_to_play();

	if (digiName == nullptr) {
		conv_resume(conv_get_handle());
		return;
	}

	const int32 who = conv_whos_talking();
	if (who <= 0) {
		_field5C_should = 121;
		kernel_timing_trigger(10, 127, nullptr);
		_G(kernel).trigger_mode = KT_PARSE;
	} else if (who == 1) {
		digi_play(digiName, 1, 255, 90, -1);
	}
}

void Room704::playCheckBrochureAnim(int32 trigger, const char *digiName) {
	switch (trigger) {
	case -1:
		player_set_commands_allowed(false);
		_ripChecksBrochureSeries = series_load("RIP CHECKS BROCHURE", -1, nullptr);
		setGlobals1(_ripChecksBrochureSeries, 1, 25, 25, 25, 0, 25, 31, 31, 31, 0, 31, 1, 1, 1, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 41);

		break;

	case 41:
		kernel_timing_trigger(60, 43, nullptr);
		break;

	case 43:
		sendWSMessage_120000(_G(my_walker), -1);
		digi_play(digiName, 1, 255, 44, -1);

		break;

	case 44:
		kernel_timing_trigger(10, 45, nullptr);
		break;

	case 45:
		sendWSMessage_130000(_G(my_walker), 48);
		break;

	case 48:
		sendWSMessage_150000(_G(my_walker), 49);
		break;

	case 49:
		series_unload(_ripChecksBrochureSeries);
		player_set_commands_allowed(true);

		break;

	default:
		break;
	}
}

void Room704::useWheelOnNiche(int32 trigger, int val1) {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		if (_G(flags[V218])) {
			digi_play("com080", 1, 255, 114, -1);
		} else {
			setGlobals1(_ripTrekMedReachHandPos1Series, 1, 5, 5, 5, 0, 5, 10, 10, 10, 0, 10, 1, 1, 1, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), 101);
		}

		break;

	case 101:
		kernel_timing_trigger(5, 102, nullptr);
		break;

	case 102:
		sendWSMessage_120000(_G(my_walker), 103);
		break;

	case 103:
		kernel_timing_trigger(5, 104, nullptr);
		break;

	case 104:
		switch (_G(flags[V218])) {
		case 0:
			break;
		case 1:
			inv_move_object("PEACE WHEEL", 704);
			_G(flags[V218]) = 1;
			digi_play("950_S40A", 2, 255, -1, 950);
			_prayerWheelMach = series_place_sprite("704 PEACE PRAYER WHEEL", 0, 0, 0, 100, 1280);
			kernel_timing_trigger(5, 105);

			break;

		case 2:
			inv_move_object("INSIGHT WHEEL", 704);
			_G(flags[V218]) = 2;
			digi_play("950_S40A", 2, 255, -1, 950);
			_prayerWheelMach = series_place_sprite("704 INSIGHT PRAYER WHEEL", 0, 0, 0, 100, 1280);
			kernel_timing_trigger(5, 105);

			break;

		case 3:
			inv_move_object("SERENITY WHEEL", 704);
			_G(flags[V218]) = 3;
			digi_play("950_S40A", 2, 255, -1, 950);
			_prayerWheelMach = series_place_sprite("704 SERENITY PRAYER WHEEL", 0, 0, 0, 100, 1280);
			kernel_timing_trigger(5, 105);

			break;

		case 4:
			inv_move_object("TRUTH WHEEL", 704);
			_G(flags[V218]) = 4;
			digi_play("950_S40A", 2, 255, -1, 950);
			_prayerWheelMach = series_place_sprite("704 TRUTH PRAYER WHEEL", 0, 0, 0, 100, 1280);
			kernel_timing_trigger(5, 105);

			break;

		case 5:
			inv_move_object("WISDOM WHEEL", 704);
			_G(flags[V218]) = 5;
			digi_play("950_S40A", 2, 255, -1, 950);
			_prayerWheelMach = series_place_sprite("704 WISDOM PRAYER WHEEL", 0, 0, 0, 100, 1280);
			kernel_timing_trigger(5, 105);

			break;

		default:
			break;
		}

		break;

	case 105:
		switch (val1) {
		case 1:
			digi_play("705r06", 1, 255, -1, 705);
			break;
		case 2:
			digi_play("705r15", 1, 255, -1, 705);
			break;

		case 3:
			sendWSMessage_130000(_G(my_walker), 106);
			return;

		case 4:
			digi_play("706r11", 1, 255, -1, 706);
			break;

		case 5:
			digi_play("703r12", 1, 255, -1, 703);
			break;

		default:
			break;
		}
		sendWSMessage_130000(_G(my_walker), 113);

		break;

	case 106:
		digi_play("704r10", 1, 255, 107, -1);
		break;

	case 107:
		_704Monk2Series = series_load("704 MONK 2", -1, nullptr);
		_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0, false, callback, "monk");
		sendWSMessage_10000(1, _monkMach, _704Monk2Series, 1, 18, 908, _704Monk2Series, 18, 18, 0);

		break;

	case 108:
		terminateMachine(_monkMach);
		series_unload(_704Monk2Series);
		sendWSMessage_150000(_G(my_walker), -1);
		ws_walk(_G(my_walker), 6, 344, nullptr, 109, 11, true);

		break;

	case 109:
		ws_walk(_G(my_walker), 4, 290, nullptr, -1, 11, true);
		kernel_timing_trigger(60, 111, nullptr);

		break;

	case 111:
		disable_player_commands_and_fade_init(112);
		break;

	case 112:
		adv_kill_digi_between_rooms(false);
		digi_play_loop("950_s39", 3, 255, -1, -1);
		_G(game).new_room = 705;

		break;

	case 113:
		if (_G(flags[V218]) == 3) {
			terminateMachine(_monkMach);
			series_unload(_704Monk2Series);
		}

		sendWSMessage_150000(_G(my_walker), 114);

		break;

	case 114:
		hotspot_set_active(_G(currentSceneDef).hotspots, "EMPTY NICHE", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "PRAYER WHEEL #6", true);
		player_set_commands_allowed(true);

		break;

	case 908:
		sendWSMessage_10000(1, _monkMach, _704Monk2Series, 19, 21, -1, _704Monk2Series, 22, 24, 4);
		sendWSMessage_1a0000(_monkMach, 9);
		digi_play("704f02", 1, 255, 912, -1);

		break;

	case 912:
		sendWSMessage_10000(1, _monkMach, _704Monk2Series, 25, 28, 113, _704Monk2Series, 28, 28, 0);
		break;

	default:
		break;

	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
