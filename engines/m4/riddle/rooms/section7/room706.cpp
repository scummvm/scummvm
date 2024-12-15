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

#include "m4/riddle/rooms/section7/room706.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_vmng_screen.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room706::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
}

void Room706::init() {
	digi_preload("950_S40", -1);
	digi_preload("950_S40A", -1);
	_ripTrekMedReachHandPos1Series = series_load("RIP TREK MED REACH HAND POS1", -1, nullptr);

	if (_G(flags[V224])) {
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #13", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #14", false);
		// Suspicious but present in the original. Should it be #16?
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #14", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #15", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "PRAYER WHEEL #14", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "EMPTY NICHE", false);

		series_show("THE ALL IMPORTANT NOTE", 3840, 16, -1, -1, 0, 100, 0, 0);
	}

	hotspot_set_active(_G(currentSceneDef).hotspots, "  ", false);
	hotspot_set_active(_G(currentSceneDef).hotspots, "Note", false);

	if (_G(flags[V219])) {
		hotspot_set_active(_G(currentSceneDef).hotspots, "EMPTY NICHE", false);
	} else {
		hotspot_set_active(_G(currentSceneDef).hotspots, "PRAYER WHEEL #15", false);
	}

	switch (_G(flags[V219])) {
	case 1:
		_prayerWheelMach = series_place_sprite("706 PEACE PRAYER WHEEL", 0, 640, 0, 100, 1280);
		break;

	case 2:
		_prayerWheelMach = series_place_sprite("706 INSIGHT PRAYER WHEEL", 0, 640, 0, 100, 1280);
		break;

	case 3:
		_prayerWheelMach = series_place_sprite("706 SERENITY PRAYER WHEEL", 0, 640, 0, 100, 1280);
		break;

	case 4:
		_prayerWheelMach = series_place_sprite("706 TRUTH PRAYER WHEEL", 0, 640, 0, 100, 1280);
		break;

	case 5:
		_prayerWheelMach = series_place_sprite("706 WISDOM PRAYER WHEEL", 0, 640, 0, 100, 1280);
		break;

	default:
		break;
	}

	if (_G(flags[V222])) {
		series_show("706 ENLIGHTEN DOOR OPEN", 3840, 16, -1, -1, 0, 100, 0, 0);

		hotspot_set_active(_G(currentSceneDef).hotspots, "  ", true);
		hotspot_set_active(_G(currentSceneDef).hotspots, "PRAYER WHEEL #14", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "empty cell", false);
	}

	if (_G(flags[V224])) {
		hotspot_set_active(_G(currentSceneDef).hotspots, "Note", true);
	}

	switch (_G(game).previous_room) {
	case 703:
		MoveScreenDelta(_G(game_buff_ptr), -640, 0);
		player_first_walk(1299, 300, 9, 1170, 318, 9, true);
		break;

	case 705:
		player_first_walk(0, 300, 3, 130, 318, 3, true);
		break;

	case 707:
		MoveScreenDelta(_G(game_buff_ptr), -320, 0);
		player_set_commands_allowed(false);
		ws_demand_location(_G(my_walker), 635, 357);
		ws_demand_facing(_G(my_walker), 7);
		ws_hide_walker(_G(my_walker));
		_706RipGoesDownStairsSeries = series_load("706 RIP GOES DOWN STAIRS", -1, nullptr);
		_ripStairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 320, 0, 100, 0, false, triggerMachineByHashCallback, "rip stairs machine");
		sendWSMessage_10000(1, _ripStairsMach, _706RipGoesDownStairsSeries, 4, 39, 140, _706RipGoesDownStairsSeries, 39, 39, 0);
		break;

	case 709:
	case 711:
		digi_preload("950_s39", -1);
		player_set_commands_allowed(false);
		ws_demand_location(_G(my_walker), 420, 288);
		ws_demand_facing(_G(my_walker), 4);
		_ripShieldsFaceSeries = series_load("RIP SHIELDS FACE", -1, nullptr);
		setGlobals1(_ripShieldsFaceSeries, 1, 14, 14, 14, 0, 14, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 131);

		break;

	default: // Including KERNEL_RESTORING_GAME
		digi_preload("950_s39", -1);
		break;
	}

	digi_play_loop("950_s39", 3, 255, -1, -1);
}

void Room706::pre_parser() {
	// Nothing
}

void Room706::parser() {
	const bool lookFl = player_said_any("look", "look at");
	const bool talkFl = player_said_any("talk", "talk to");
	const bool takeFl = player_said("take");
	const bool gearFl = player_said_any("push", "pull", "gear", "open", "close");

	if (player_said("conv706a")) {
		if (_G(kernel).trigger == 90) {
			conv_resume(conv_get_handle());
		} else {
			conv706a();
		}
	} else if (player_said("PRAYER WHEEL BROCHURE", "EMPTY NICHE") && inv_player_has("PRAYER WHEEL BROCHURE")) {
		digi_play("com123", 1, 255, -1, 997);
	} else if (player_said("PRAYER WHEEL BROCHURE", "PRAYER WHEEL #13") && inv_player_has("PRAYER WHEEL BROCHURE")) {
		playCheckBrochureAnim(_G(kernel).trigger, "706r04");
	} else if (player_said("PRAYER WHEEL BROCHURE", "PRAYER WHEEL #14") && inv_player_has("PRAYER WHEEL BROCHURE")) {
		playCheckBrochureAnim(_G(kernel).trigger, "706r09");
	} else if (player_said("PRAYER WHEEL BROCHURE", "PRAYER WHEEL #16") && inv_player_has("PRAYER WHEEL BROCHURE")) {
		playCheckBrochureAnim(_G(kernel).trigger, "706r12");
	} else if (player_said("PRAYER WHEEL BROCHURE", "PRAYER WHEEL #15") && inv_player_has("PRAYER WHEEL BROCHURE")) {
		switch (_G(flags[V219])) {
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
	} else if (player_said("PRAYER WHEEL BROCHURE", "SERENITY WHEEL") && inv_player_has("PRAYER WHEEL BROCHURE") && inv_player_has("SERENITY WHEEL")) {
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
	} else if (talkFl && player_said_any("MONK #13", "MONK #14", "MONK #15", "MONK #16") && _G(flags[V219])) {
		digi_play("com142", 1, 255, -1, -1);
	} else if (talkFl && player_said("MONK #13")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			digi_play("com081", 1, 255, 2, -1);
			break;

		case 2:
			_706Monk1Series = series_load("706 MONK 1", -1, nullptr);
			_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 256, false, triggerMachineByHashCallback, "monk");
			sendWSMessage_10000(1, _monkMach, _706Monk1Series, 1, 16, 3, _706Monk1Series, 16, 16, 0);

			break;

		case 3:
			sendWSMessage_10000(1, _monkMach, _706Monk1Series, 17, 17, -1, _706Monk1Series, 17, 18, 4);
			sendWSMessage_1a0000(_monkMach, 13);
			digi_play("706O01", 1, 255, 4, -1);

			break;

		case 4:
			sendWSMessage_10000(1, _monkMach, _706Monk1Series, 19, 22, 5, _706Monk1Series, 22, 22, 0);
			break;

		case 5:
			terminateMachine(_monkMach);
			series_unload(_706Monk1Series);
			player_set_commands_allowed(true);

			break;

		default:
			break;

		}
	} // talkFl && player_said("MONK #13")

	else if (talkFl && player_said("MONK #14")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			digi_play("com081", 1, 255, 2, -1);
			break;

		case 2:
			_706Monk3Series = series_load("706 MONK 3", -1, nullptr);
			_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 640, -53, 100, 256, false, triggerMachineByHashCallback, "monk");
			sendWSMessage_10000(1, _monkMach, _706Monk3Series, 1, 11, 3, _706Monk3Series, 11, 11, 4);

			break;

		case 3:
			sendWSMessage_10000(1, _monkMach, _706Monk3Series, 12, 16, -1, _706Monk3Series, 17, 18, 4);
			sendWSMessage_1a0000(_monkMach, 9);
			digi_play("706P01", 1, 255, 4, -1);

			break;

		case 4:
			sendWSMessage_10000(1, _monkMach, _706Monk3Series, 19, 22, 5, _706Monk3Series, 22, 22, 0);
			break;

		case 5:
			terminateMachine(_monkMach);
			series_unload(_706Monk3Series);
			_field78_mode = 124;
			_field7C_should =125;
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_timing_trigger(10, 127, nullptr);
			_G(kernel).trigger_mode = KT_PARSE;

			break;

		default:
			break;
		}
	} // talkFl && player_said("MONK #14")

	else if (talkFl && player_said("MONK #15")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			digi_play("com081", 1, 255, 2, -1);
			break;

		case 2:
			_706Monk4Series = series_load("706 MONK 4", -1, nullptr);
			_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 256, false, triggerMachineByHashCallback, "monk");
			sendWSMessage_10000(1, _monkMach, _706Monk4Series, 1, 26, 3, _706Monk4Series, 26, 26, 0);

			break;

		case 3:
			sendWSMessage_10000(1, _monkMach, _706Monk4Series, 27, 28, -1, _706Monk4Series, 29, 30, 4);
			sendWSMessage_1a0000(_monkMach, 9);
			digi_play("706Q01", 1, 255, 4, -1);

			break;

		case 4:
			sendWSMessage_10000(1, _monkMach, _706Monk4Series, 31, 33, 5, _706Monk4Series, 31, 33, 0);
			break;

		case 5:
			terminateMachine(_monkMach);
			series_unload(_706Monk4Series);
			player_set_commands_allowed(true);

			break;

		default:
			break;
		}
	} // talkFl && player_said("MONK #15")

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
				_ripLooksDownSeries = series_load("RIP LOOKS DOWN POS3", -1, nullptr);
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
				_ripLooksDownSeries = series_load("RIP LOOKS DOWN POS4", -1, nullptr);
				setGlobals1(_ripLooksDownSeries, 1, 15, 15, 15, 0, 15, 31, 31, 31, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
				sendWSMessage_110000(_G(my_walker), 2);

				break;

			default:
				// Should never happen. Original calls setGlobals1 without setting the key numbers
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

	else if (player_said("GRATE"))
		digi_play("707r02", 1, 255, -1, -1);
	else if (lookFl && player_said("MONK #13")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_706Monk1Series = series_load("706 MONK 1", -1, nullptr);
			_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0, false, triggerMachineByHashCallback, "monk");
			sendWSMessage_10000(1, _monkMach, _706Monk1Series, 1, 22, 2, _706Monk1Series, 22, 22, 0);

			break;

		case 2:
			terminateMachine(_monkMach);
			series_unload(_706Monk1Series);
			player_set_commands_allowed(true);

			break;

		default:
			break;
		}
	} // ecx && player_said("MONK #13")

	else if (lookFl && player_said("empty cell")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_706Monk2Series = series_load("706 MONK 2", -1, nullptr);
			_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, false, triggerMachineByHashCallback, "monk");
			sendWSMessage_10000(1, _monkMach, _706Monk2Series, 1, 5, 2, _706Monk2Series, 5, 5, 0);

			break;

		case 2:
			sendWSMessage_10000(1, _monkMach, _706Monk2Series, 5, 5, -1, _706Monk2Series, 5, 5, 0);
			kernel_timing_trigger(120, 3, nullptr);

			break;

		case 3:
			sendWSMessage_10000(1, _monkMach, _706Monk2Series, 5, 1, 4, _706Monk2Series, 1, 1, 0);
			break;

		case 4:
			terminateMachine(_monkMach);
			series_unload(_706Monk2Series);
			player_set_commands_allowed(true);

			break;

		default:
			break;
		}
	} // ecx && player_said("empty cell")

	else if (lookFl && player_said("MONK #14")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_706Monk3Series = series_load("706 MONK 3", -1, nullptr);
			_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 640, -53, 100, 0, false, triggerMachineByHashCallback, "monk");
			sendWSMessage_10000(1, _monkMach, _706Monk3Series, 1, 22, 2, _706Monk3Series, 22, 22, 0);

			break;

		case 2:
			terminateMachine(_monkMach);
			series_unload(_706Monk3Series);
			player_set_commands_allowed(true);

			break;
		default:
			break;

		}

	} // ecx && player_said("MONK #14")

	else if (lookFl && player_said("MONK #15")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_monkMach = series_stream("706 MONK 4", 5, 0, 2);

			break;

		case 2:
			player_set_commands_allowed(true);

			break;
		default:
			break;
		}

	} // ecx && player_said("MONK #15")

	else if (lookFl && player_said("Note"))
		digi_play("706R25", 1, 255, -1, -1);
	else if (takeFl && player_said("Note"))
		digi_play("706R27", 1, 255, -1, -1);
	else if (player_said("TRUTH WHEEL", "EMPTY NICHE")) {
		if (_G(kernel).trigger == -1 && !inv_player_has("TRUTH WHEEL"))
			player_set_commands_allowed(true);
		else
			useWheelOnNiche(_G(kernel).trigger, 4);
	} else if (player_said("PEACE WHEEL", "EMPTY NICHE")) {
		if (_G(kernel).trigger == -1 && !inv_player_has("PEACE WHEEL"))
			player_set_commands_allowed(true);
		else
			useWheelOnNiche(_G(kernel).trigger, 1);
	} else if (player_said("WISDOM WHEEL", "EMPTY NICHE")) {
		if (_G(kernel).trigger == -1 && !inv_player_has("WISDOM WHEEL"))
			player_set_commands_allowed(true);
		else
			useWheelOnNiche(_G(kernel).trigger, 5);
	} else if (player_said("INSIGHT WHEEL", "EMPTY NICHE")) {
		if (_G(kernel).trigger == -1 && !inv_player_has("INSIGHT WHEEL"))
			player_set_commands_allowed(true);
		else
			useWheelOnNiche(_G(kernel).trigger, 2);
	} else if (player_said("SERENITY WHEEL", "EMPTY NICHE")) {
		if (_G(kernel).trigger == -1 && !inv_player_has("SERENITY WHEEL"))
			player_set_commands_allowed(true);
		else
			useWheelOnNiche(_G(kernel).trigger, 3);
	} else if (player_said("TRUTH WHEEL", "PRAYER WHEEL #13") || player_said("PEACE WHEEL", "PRAYER WHEEL #13") || player_said("WISDOM WHEEL", "PRAYER WHEEL #13") || player_said("INSIGHT WHEEL", "PRAYER WHEEL #13") || player_said("SERENITY WHEEL", "PRAYER WHEEL #13")
			|| player_said("TRUTH WHEEL", "PRAYER WHEEL #14") || player_said("PEACE WHEEL", "PRAYER WHEEL #14") || player_said("WISDOM WHEEL", "PRAYER WHEEL #14") || player_said("INSIGHT WHEEL", "PRAYER WHEEL #14") || player_said("SERENITY WHEEL", "PRAYER WHEEL #14")
			|| player_said("TRUTH WHEEL", "PRAYER WHEEL #15") || player_said("PEACE WHEEL", "PRAYER WHEEL #15") || player_said("WISDOM WHEEL", "PRAYER WHEEL #15") || player_said("INSIGHT WHEEL", "PRAYER WHEEL #15") || player_said("SERENITY WHEEL", "PRAYER WHEEL #15")
			|| player_said("TRUTH WHEEL", "PRAYER WHEEL #16") || player_said("PEACE WHEEL", "PRAYER WHEEL #16") || player_said("WISDOM WHEEL", "PRAYER WHEEL #16") || player_said("INSIGHT WHEEL", "PRAYER WHEEL #16") || player_said("SERENITY WHEEL", "PRAYER WHEEL #16")
		)
		digi_play("com080", 1, 255, -1, -1);
	else if (lookFl && player_said("EMPTY NICHE"))
		digi_play("com127", 1, 255, -1, 997);
	else if (lookFl && (player_said_any("PRAYER WHEEL #13", "PRAYER WHEEL #14", "PRAYER WHEEL #16") || (player_said("PRAYER WHEEL #15") && _G(flags[V219]))))
		digi_play("com076", 1, 255, -1, -1);
	else if (takeFl && player_said_any("PRAYER WHEEL #13", "PRAYER WHEEL #14", "PRAYER WHEEL #15", "PRAYER WHEEL #16") && _G(flags[V286]))
		digi_play(_G(flags[V224]) ? "706r26" : "com143", 1, 255, -1, -1);
	else if (takeFl && player_said_any("PRAYER WHEEL #13", "PRAYER WHEEL #14", "PRAYER WHEEL #16")) {
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_trigger_dispatchx(kernel_trigger_create(80));
		_G(kernel).trigger_mode = KT_PARSE;
	} else if (takeFl && player_said("PRAYER WHEEL #15")) {
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_trigger_dispatchx(kernel_trigger_create(70));
		_G(kernel).trigger_mode = KT_PARSE;
	} else if (lookFl && player_said(" ")) {
		digi_play(_G(flags[V224]) ? "706r24" : "com075", 1, 255, -1, -1);
	} else if (player_said("CUPOLA")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			ws_hide_walker(_G(my_walker));
			_706RipGoesUpStairsSeries = series_load("706 RIP GOES UP STAIRS", -1, nullptr);
			_ripStairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 320, 0, 100, 0, false, triggerMachineByHashCallback, "rip stairs machine");
			sendWSMessage_10000(1, _ripStairsMach, _706RipGoesUpStairsSeries, 1, 29, 2, _706RipGoesUpStairsSeries, 29, 29, 0);

			break;

		case 2:
			sendWSMessage_10000(1, _ripStairsMach, _706RipGoesUpStairsSeries, 30, 39, -1, _706RipGoesUpStairsSeries, 39, 39, 0);
			disable_player_commands_and_fade_init(4);

			break;

		case 4:
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
	} // player_said("North Face")

	else if (player_said("South Face")) {
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
	} // player_said("South Face")

	else if (lookFl && player_said("  "))
		digi_play("709r01", 1, 255, -1, 709);
	else if (!takeFl && player_said("  ")) {
		switch (_G(kernel).trigger) {
		case -1:
			_ripStepsDownSeries = series_load("RIP STEPS DOWN", -1, nullptr);
			ws_hide_walker(_G(my_walker));
			_downToMazeMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 256, false, triggerMachineByHashCallback, "down to maze");
			sendWSMessage_10000(1, _downToMazeMach, _ripStepsDownSeries, 1, 13, -1, _ripStepsDownSeries, 13, 13, 0);
			kernel_timing_trigger(60, 2, nullptr);

			break;

		case 2:
			disable_player_commands_and_fade_init(3);
			break;

		case 3:
			_G(game).new_room = 709;
			break;

		default:
			break;
		}
	} // !esi && player_said("  ")

	else if (!gearFl && !takeFl && player_said_any("MONK #13", "MONK #14", "MONK #16")) {
		// The original is testing 2 times the monk #14 and is missing the monk #16, I think it's better that way
		digi_play("com017", 1, 255, -1, -1);
	} else
		return;

	_G(player).command_ready = false;
}

void Room706::daemon() {
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
		kernel_timing_trigger(5, 73, nullptr);
		break;

	case 74:
		switch (_G(flags[V219])) {
		case 1:
			inv_give_to_player("PEACE WHEEL");
			_G(flags[V219]) = 0;
			player_update_info(_G(my_walker), &_G(player_info));
			digi_play("950_S40", 2, 255, -1, 950);
			kernel_examine_inventory_object("PING PEACE WHEEL", _G(master_palette), 5, 1, _G(player_info).camera_x + 800, 225, 75, nullptr, -1);

			break;

		case 2:
			inv_give_to_player("INSIGHT WHEEL");
			_G(flags[V219]) = 0;
			player_update_info(_G(my_walker), &_G(player_info));
			digi_play("950_S40", 2, 255, -1, 950);
			kernel_examine_inventory_object("PING INSIGHT WHEEL", _G(master_palette), 5, 1, _G(player_info).camera_x + 800, 225, 75, nullptr, -1);

			break;

		case 3:
			inv_give_to_player("SERENITY WHEEL");
			_G(flags[V219]) = 0;
			player_update_info(_G(my_walker), &_G(player_info));
			digi_play("950_S40", 2, 255, -1, 950);
			kernel_examine_inventory_object("PING SERENITY WHEEL", _G(master_palette), 5, 1, _G(player_info).camera_x + 800, 225, 75, nullptr, -1);

			break;

		case 4:
			inv_give_to_player("TRUTH WHEEL");
			_G(flags[V219]) = 0;
			player_update_info(_G(my_walker), &_G(player_info));
			digi_play("950_S40", 2, 255, -1, 950);
			kernel_examine_inventory_object("PING TRUTH WHEEL", _G(master_palette), 5, 1, _G(player_info).camera_x + 800, 225, 75, nullptr, -1);

			break;

		case 5:
			inv_give_to_player("WISDOM WHEEL");
			_G(flags[V219]) = 0;
			player_update_info(_G(my_walker), &_G(player_info));
			digi_play("950_S40", 2, 255, -1, 950);
			kernel_examine_inventory_object("PING WISDOM WHEEL", _G(master_palette), 5, 1, _G(player_info).camera_x + 800, 225, 75, nullptr, -1);

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
		hotspot_set_active(_G(currentSceneDef).hotspots, "PRAYER WHEEL #15", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "EMPTY NICHE", true);
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
		if (!takeFl || _G(player).click_y > 374)
			break;

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
		if (_field78_mode != 124)
			break;

		switch (_field7C_should) {
		case 121:
			_706eye3aSeries = series_load("706eye3a", -1, nullptr);
			_field7C_should = 922;
			_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 640, -53, 100, 256, false, triggerMachineByHashCallback, "monk");
			sendWSMessage_10000(1, _monkMach, _706eye3aSeries, 1, 5, 123, _706eye3aSeries, 5, 5, 0);

			break;

		case 122:
			terminateMachine(_monkMach);
			series_unload(_706eye3aSeries);
			conv_resume(conv_get_handle());

			break;

		case 125:
			conv_load("conv706a", 10, 10, 91);
			conv_export_value_curr(0, 0);
			conv_export_value_curr(_G(flags[V211]), 1);
			conv_export_value_curr(_G(flags[V213]), 2);
			conv_export_value_curr((_G(flags[V219]) == 4) ? 1 : 0, 0);

			conv_play(conv_get_handle());

			break;

		case 922:
			switch (imath_ranged_rand(1, 2)) {
			case 1:
				_field7C_should = 923;
				break;

			case 2:
				_field7C_should = 924;
				break;

			default:
				break;
			}

			kernel_timing_trigger(10, 123, nullptr);

			break;

		case 923:
			_field7C_should = 925;
			sendWSMessage_10000(1, _monkMach, _706eye3aSeries, 6, 9, -1, _706eye3aSeries, 10, 11, 4);
			sendWSMessage_1a0000(_monkMach, 9);
			digi_play(conv_sound_to_play(), 1, 255, 123, -1);

			break;

		case 924:
			_field7C_should = 925;
			sendWSMessage_10000(1, _monkMach, _706eye3aSeries, 10, 10, -1, _706eye3aSeries, 10, 11, 4);
			sendWSMessage_1a0000(_monkMach, 9);
			digi_play(conv_sound_to_play(), 1, 255, 123, -1);

			break;

		case 925:
			_field7C_should = 926;
			sendWSMessage_10000(1, _monkMach, _706eye3aSeries, 12, 15, 123, _706eye3aSeries, 15, 15, 0);
			break;

		case 926:
			_field7C_should = 122;
			kernel_timing_trigger(10, 123, nullptr);

			break;

		default: // including case 126
			break;
		}

		break;

	case 127:
		if (_field78_mode != 124)
			break;

		switch (_field7C_should) {
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

	case 131:
		sendWSMessage_120000(_G(my_walker), 132);
		break;

	case 132:
		digi_play("706r23", 1, 255, 133, -1);
		break;

	case 133:
		sendWSMessage_150000(_G(my_walker), 135);
		break;

	case 135:
		series_unload(_ripShieldsFaceSeries);
		player_set_commands_allowed(true);

		break;

	case 140:
		player_set_commands_allowed(true);
		ws_unhide_walker(_G(my_walker));
		terminateMachine(_ripStairsMach);
		kernel_timing_trigger(10, 141, nullptr);

		break;

	case 141:
		series_unload(_706RipGoesDownStairsSeries);
		break;

	default:
		break;
	}
}

void Room706::conv706a() {
	const char *digiName = conv_sound_to_play();

	if (digiName == nullptr) {
		conv_resume(conv_get_handle());
		return;
	}

	const int32 who = conv_whos_talking();

	if (who <= 0) {
		_field7C_should = 121;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_trigger_dispatchx(kernel_trigger_create(127));
		_G(kernel).trigger_mode = KT_PARSE;
	} else if (who == 1) {
		digi_play(digiName, 1, 255, 90);
	}

}

void Room706::playCheckBrochureAnim(int32 trigger, const char *digiName) {
	switch (trigger) {
	case -1:
		player_set_commands_allowed(false);
		player_update_info(_G(my_walker), &_G(player_info));
		ws_walk(_G(my_walker), _G(player_info).x + 1, _G(player_info).y - 1, nullptr, 41, 1, true);

		break;

	case 41:
		_ripChecksBrochureSeries = series_load("RIP CHECKS BROCHURE", -1, nullptr);
		setGlobals1(_ripChecksBrochureSeries, 1, 25, 25, 25, 0, 25, 31, 31, 31, 0, 31, 1, 1, 1, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), -1);
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
		// CHECKME: Loading the series is totally illogical, despite it's in the original game.
		// At this point, we usually unload the series (+ it's already loaded). I suspect it could lead to a memory leak.
		_ripChecksBrochureSeries = series_load("RIP CHECKS BROCHURE");
		player_set_commands_allowed(true);

		break;

	default:
		break;

	}
}

void Room706::useWheelOnNiche(int32 trigger, int32 val1) {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		if (_G(flags[V219]) == 0) {
			setGlobals1(_ripTrekMedReachHandPos1Series, 1, 5, 5, 5, 0, 5, 10, 10, 10, 0, 10, 1, 1, 1, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), 101);
		} else {
			digi_play("com080", 1, 255, 114, -1);
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
		if (_G(flags[V219]) != 0)
			return;

		switch (val1) {
		case 1:
			inv_move_object("PEACE WHEEL", 706);
			_G(flags[V219]) = 1;
			digi_play("950_S40A", 2, 255, -1, 950);
			_prayerWheelMach = series_place_sprite("706 PEACE PRAYER WHEEL", 0, 640, 0, 100, 1280);

			break;

		case 2:
			inv_move_object("INSIGHT WHEEL", 706);
			_G(flags[V219]) = 2;
			digi_play("950_S40A", 2, 255, -1, 950);
			_prayerWheelMach = series_place_sprite("706 INSIGHT PRAYER WHEEL", 0, 640, 0, 100, 1280);

			break;

		case 3:
			inv_move_object("SERENITY WHEEL", 706);
			_G(flags[V219]) = 3;
			digi_play("950_S40A", 2, 255, -1, 950);
			_prayerWheelMach = series_place_sprite("706 SERENITY PRAYER WHEEL", 0, 640, 0, 100, 1280);

			break;

		case 4:
			inv_move_object("TRUTH WHEEL", 706);
			_G(flags[V219]) = 4;
			digi_play("950_S40A", 2, 255, -1, 950);
			_prayerWheelMach = series_place_sprite("706 TRUTH PRAYER WHEEL", 0, 640, 0, 100, 1280);

			break;

		case 5:
			inv_move_object("WISDOM WHEEL", 706);
			_G(flags[V219]) = 5;
			digi_play("950_S40A", 2, 255, -1, 950);
			_prayerWheelMach = series_place_sprite("706 WISDOM PRAYER WHEEL", 0, 640, 0, 100, 1280);

			break;

		default:
			return;
		}

		kernel_timing_trigger(5, 105, nullptr);

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
			digi_play("704r10", 1, 255, -1, 704);
			break;

		case 4:
			sendWSMessage_130000(_G(my_walker), 106);
			return;

		case 5:
			digi_play("703r12", 1, 255, -1, 703);
			break;

		default:
			break;
		}

		sendWSMessage_130000(_G(my_walker), 113);

		break;

	case 106:
		digi_play("706r11", 1, 255, 107, -1);
		break;

	case 107:
		_706Monk3Series = series_load("706 MONK 3", -1, nullptr);
		_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 640, -53, 100, 256, false, triggerMachineByHashCallback, "monk");
		sendWSMessage_10000(1, _monkMach, _706Monk3Series, 1, 11, 908, _706Monk3Series, 11, 11, 0);

		break;

	case 108:
		terminateMachine(_monkMach);
		series_unload(_706Monk3Series);
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
		if (_G(flags[V219]) == 4) {
			terminateMachine(_monkMach);
			series_unload(_706Monk3Series);
		}

		sendWSMessage_150000(_G(my_walker), 114);

		break;


	case 114:
		hotspot_set_active(_G(currentSceneDef).hotspots, "PRAYER WHEEL #15", true);
		hotspot_set_active(_G(currentSceneDef).hotspots, "EMPTY NICHE", false);
		player_set_commands_allowed(true);

		break;

	case 908:
		sendWSMessage_10000(1, _monkMach, _706Monk3Series, 11, 16, -1, _706Monk3Series, 17, 18, 0);
		sendWSMessage_1a0000(_monkMach, 9);
		digi_play("706p02", 1, 255, 912, -1);

		break;

	case 912:
		sendWSMessage_10000(1, _monkMach, _706Monk3Series, 19, 22, 113, _706Monk3Series, 19, 22, 0);
		break;

	default:
		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
