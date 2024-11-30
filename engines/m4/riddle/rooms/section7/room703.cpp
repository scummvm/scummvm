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
	bool lookFl = player_said_any("look", "look at");
	bool talkFl = player_said_any("talk", "talk to");
	bool takeFl = player_said("take");
	bool gearFl = player_said_any("push", "pull", "gear", "open", "close");

	if (player_said("conv703a")) {
		if (_G(kernel).trigger == 90)
			conv_resume(conv_get_handle());
		else
			conv703a();
	} else if (player_said("PRAYER WHEEL BROCHURE", "EMPTY NICHE") && inv_player_has("PRAYER WHEEL BROCHURE")) {
		digi_play("com123", 1, 255, -1, 997);
	} else if (player_said("PRAYER WHEEL BROCHURE", "PRAYER WHEEL #1") && inv_player_has("PRAYER WHEEL BROCHURE")) {
		playCheckBrochureAnim("703r04", _G(kernel).trigger);
	} else if (player_said("PRAYER WHEEL BROCHURE", "PRAYER WHEEL #2") && inv_player_has("PRAYER WHEEL BROCHURE")) {
		playCheckBrochureAnim("703r09", _G(kernel).trigger);
	} else if (player_said("PRAYER WHEEL BROCHURE", "PRAYER WHEEL #3") && inv_player_has("PRAYER WHEEL BROCHURE")) {
		playCheckBrochureAnim("703r10", _G(kernel).trigger);
	} else if (player_said("PRAYER WHEEL BROCHURE", "PRAYER WHEEL #4") && inv_player_has("PRAYER WHEEL BROCHURE")) {
		switch (_G(flags[V217])) {
		case 1:
			playCheckBrochureAnim("704r09", _G(kernel).trigger);
			break;

		case 2:
			playCheckBrochureAnim("705r04", _G(kernel).trigger);
			break;

		case 3:
			playCheckBrochureAnim("703r11", _G(kernel).trigger);
			break;

		case 4:
			playCheckBrochureAnim("705r14a", _G(kernel).trigger);
			break;

		case 5:
			playCheckBrochureAnim("706r10", _G(kernel).trigger);
			break;

		default:
			break;
		}
	} // player_said("PRAYER WHEEL BROCHURE", "PRAYER WHEEL #4") && inv_player_has("PRAYER WHEEL BROCHURE")
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
	} else if (talkFl && player_said_any("MONK #1", "MONK #2", "MONK #3", "MONK #4") && _G(flags[V286]))
		digi_play("com142", 1, 255, -1, -1);
	else if (talkFl && player_said("MONK #1")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			digi_play("com081", 1, 255, 2, -1);

			break;

		case 2:
			_monk1Series = series_load("703 MONK 1", -1, nullptr);
			_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 256, false, callback, "monk");
			sendWSMessage_10000(1, _monkMach, _monk1Series, 1, 19, 3, _monk1Series, 19, 19, 0);

			break;

		case 3:
			sendWSMessage_10000(1, _monkMach, _monk1Series, 20, 25, -1, _monk1Series, 25, 25, 0);
			digi_play("703A01", 1, 255, 4, -1);

			break;

		case 4:
			sendWSMessage_10000(1, _monkMach, _monk1Series, 26, 29, 5, _monk1Series, 29, 29, 0);
			break;

		case 5:
			terminateMachine(_monkMach);
			series_unload(_monk1Series);
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
	} // talkFl && player_said("MONK #1")
	else if (talkFl && player_said("MONK #2")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			digi_play("com081", 1, 255, 2, -1);
			break;

		case 2:
			_monk2Series = series_load("703 MONK 2", -1, nullptr);
			_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 256, false, callback, "monk");
			sendWSMessage_10000(1, _monkMach, _monk2Series, 1, 14, 3, _monk2Series, 14, 14, 0);

			break;
		case 3:
			sendWSMessage_10000(1, _monkMach, _monk2Series, 15, 18, -1, _monk2Series, 19, 22, 4);
			sendWSMessage_1a0000(_monkMach, 9);
			digi_play("703B01", 1, 255, 4, -1);

			break;

		case 4:
			sendWSMessage_10000(1, _monkMach, _monk2Series, 23, 26, 5, _monk2Series, 26, 26, 0);
			break;

		case 5:
			terminateMachine(_monkMach);
			series_unload(_monk2Series);
			player_set_commands_allowed(true);

			break;

		default:
			break;
		}
	} // talkFl && player_said("MONK #2")
	else if (talkFl && player_said("MONK #3")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			digi_play("com081", 1, 255, 2, -1);
			break;

		case 2:
			_monk3Series = series_load("703 MONK 3", -1, nullptr);
			_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 640, -53, 100, 256, false, callback, "monk");
			sendWSMessage_10000(1, _monkMach, _monk3Series, 1, 18, 3, _monk3Series, 18, 18, 0);

			break;
		case 3:
			sendWSMessage_10000(1, _monkMach, _monk3Series, 18, 19, -1, _monk3Series, 19, 19, 0);
			digi_play("703C01", 1, 255, 4, -1);

			break;

		case 4:
			sendWSMessage_10000(1, _monkMach, _monk3Series, 20, 23, 5, _monk3Series, 23, 23, 0);
			break;

		case 5:
			terminateMachine(_monkMach);
			series_unload(_monk3Series);
			player_set_commands_allowed(true);

			break;

		default:
			break;
		}
	} // talkFl && player_said("MONK #3")

	else if (talkFl && player_said("MONK #4")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			digi_play("com081", 1, 255, 2, -1);
			break;

		case 2:
			_monk4Series = series_load("703 MONK 4", -1, nullptr);
			_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 640, -53, 100, 256, false, callback, "monk");
			sendWSMessage_10000(1, _monkMach, _monk4Series, 21, 10, 3, _monk4Series, 9, 5, 4);
			sendWSMessage_1a0000(_monkMach, 9);

			break;
		case 3:
			digi_play("703D01", 1, 255, 4, -1);

			break;

		case 4:
			sendWSMessage_10000(1, _monkMach, _monk4Series, 4, 1, 5, _monk4Series, 1, 1, 0);
			break;

		case 5:
			terminateMachine(_monkMach);
			series_unload(_monk4Series);
			_field5C_mode = 124;
			_field60_should = 125;

			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_timing_trigger(10, 127, nullptr);
			_G(kernel).trigger_mode = KT_PARSE;

			break;

		default:
			break;
		}
	} // talkFl && player_said("MONK #4")

	else if (lookFl && player_said("GRATE")) {
		switch (_G(kernel).trigger) {
		case -1: {
			int32 keyValue = 0;

			player_update_info(_G(my_walker), &_G(player_info));
			switch (_G(player_info).facing) {
			case 1:
			case 11:
				_ripLooksDownSeries = series_load("RIP LOOKS DOWN POS1", -1, nullptr);
				keyValue = 29;
				break;

			case 2:
			case 10:
				_ripLooksDownSeries = series_load("RIP LOOKS DOWN POS2", -1, nullptr);
				keyValue = 29;

				break;

			case 3:
			case 9:
				_ripLooksDownSeries = series_load("RIP LOOKS DOWN POS3", -1, nullptr);
				keyValue = 22;

				break;

			case 4:
			case 8:
				_ripLooksDownSeries = series_load("RIP LOOKS DOWN POS4", -1, nullptr);
				keyValue = 33;

				break;

			case 5:
			case 7:
				_ripLooksDownSeries = series_load("RIP LOOKS DOWN POS5", -1, nullptr);
				keyValue = 31;

				break;

			default:
				// CHECKME: in the cases 6 and "other" the setGlobals is called without setting the keyValue.
				// My best guess is that this never happens as the series isn't loaded, and I just guessed a value of 0 rather randomly.

				break;
			}

			setGlobals1(_ripLooksDownSeries, 1, keyValue / 2, keyValue / 2, keyValue / 2, 0, keyValue / 2, keyValue, keyValue, keyValue, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), 2);
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
	} // lookFl && player_said("GRATE")
	else if (player_said("GRATE"))
		digi_play("707r02", 1, 255, -1, -1);
	else if (lookFl && player_said("MONK #1")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_monkMach = series_stream("703 MONK 1", 5, 0, 2);
			break;

		case 2:
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
	} else if (lookFl && player_said("MONK #2")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_monk2Series = series_load("703 MONK 2", -1, nullptr);
			_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0, false, callback, "monk");
			sendWSMessage_10000(1, _monkMach, _monk2Series, 1, 26, 2, _monk2Series, 26, 26, 0);

			break;

		case 2:
			terminateMachine(_monkMach);
			series_unload(_monk2Series);
			player_set_commands_allowed(true);

			break;

		default:
			break;
		}
	} else if (lookFl && player_said("MONK #3")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_monk3Series = series_load("703 MONK 3", -1, nullptr);
			_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 640, -53, 100, 0, false, callback, "monk");
			sendWSMessage_10000(1, _monkMach, _monk3Series, 1, 23, 2, _monk3Series, 23, 23, 0);

			break;

		case 2:
			terminateMachine(_monkMach);
			series_unload(_monk3Series);
			player_set_commands_allowed(true);

			break;

		default:
			break;
		}
	} else if (lookFl && player_said("MONK #4")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_monk4Series = series_load("703 MONK 4", -1, nullptr);
			_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 640, -53, 100, 256, false, callback, "monk");
			sendWSMessage_10000(1, _monkMach, _monk4Series, 1, 21, 2, _monk4Series, 21, 21, 0);

			break;

		case 2:
			terminateMachine(_monkMach);
			series_unload(_monk4Series);
			player_set_commands_allowed(true);

			break;

		default:
			break;
		}
	} else if (player_said("TRUTH WHEEL", "EMPTY NICHE")) {
		if (_G(kernel).trigger == -1 &&  ! inv_player_has("TRUTH WHEEL"))
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
	} else if (player_said("TRUTH WHEEL", "PRAYER WHEEL #1") || player_said("PEACE WHEEL", "PRAYER WHEEL #1") || player_said("WISDOM WHEEL", "PRAYER WHEEL #1") || player_said("INSIGHT WHEEL", "PRAYER WHEEL #1") || player_said("SERENITY WHEEL", "PRAYER WHEEL #1")
		|| player_said("TRUTH WHEEL", "PRAYER WHEEL #2") || player_said("PEACE WHEEL", "PRAYER WHEEL #2") || player_said("WISDOM WHEEL", "PRAYER WHEEL #2") || player_said("INSIGHT WHEEL", "PRAYER WHEEL #2") || player_said("SERENITY WHEEL", "PRAYER WHEEL #2")
		|| player_said("TRUTH WHEEL", "PRAYER WHEEL #3") || player_said("PEACE WHEEL", "PRAYER WHEEL #3") || player_said("WISDOM WHEEL", "PRAYER WHEEL #3") || player_said("INSIGHT WHEEL", "PRAYER WHEEL #3") || player_said("SERENITY WHEEL", "PRAYER WHEEL #3")
		|| player_said("TRUTH WHEEL", "PRAYER WHEEL #4") || player_said("PEACE WHEEL", "PRAYER WHEEL #4") || player_said("WISDOM WHEEL", "PRAYER WHEEL #4") || player_said("INSIGHT WHEEL", "PRAYER WHEEL #4") || player_said("SERENITY WHEEL", "PRAYER WHEEL #4")) {
		digi_play("com080", 1, 255, -1, -1);
	} else if (lookFl && player_said("EMPTY NICHE")) {
		digi_play("com127", 1, 255, -1, 997);
	} else if (lookFl && player_said_any("PRAYER WHEEL #1", "PRAYER WHEEL #2", "PRAYER WHEEL #3")) {
		digi_play("com076", 1, 255, -1, -1);
	} else if (lookFl && player_said("PRAYER WHEEL #4") && _G(flags[V217])) {
		digi_play("com076", 1, 255, -1, -1);
	} else if (takeFl && player_said_any("PRAYER WHEEL #1", "PRAYER WHEEL #2", "PRAYER WHEEL #3", "PRAYER WHEEL #4") && _G(flags[V286])) {
		if (_G(flags[V224]))
			digi_play("706r26", 1, 255, -1, -1);
		else
			digi_play("com143", 1, 255, -1, -1);
	} else if (takeFl && player_said_any("PRAYER WHEEL #1", "PRAYER WHEEL #2", "PRAYER WHEEL #3")) {
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_trigger_dispatchx(kernel_trigger_create(80));
		_G(kernel).trigger_mode = KT_PARSE;
	} else if (takeFl && player_said("PRAYER WHEEL #1")) {
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_trigger_dispatchx(kernel_trigger_create(70));
		_G(kernel).trigger_mode = KT_PARSE;
	} else if (takeFl && _G(player).click_y <= 374) {
		switch (imath_ranged_rand(1, 6)) {
		case 1:
			digi_play("207r09", 1, 255, -1, -1);
			break;

		case 2:
			digi_play("207r10", 1, 255, -1, -1);
			break;

		case 3:
			digi_play("207r11", 1, 255, -1, -1);
			break;

		case 4:
			digi_play("207r37", 1, 255, -1, -1);
			break;

		case 5:
			digi_play("207r38", 1, 255, -1, -1);
			break;

		case 6:
			digi_play("207r39", 1, 255, -1, -1);
			break;

		default:
			break;
		}
	} // takeFl && _G(player).click_y <= 374
	else if (lookFl && player_said(" ")) {
		if (_G(flags[V224])) {
			digi_play("706r24", 1, 255, -1, -1);
		} else {
			digi_play("com075", 1, 255, -1, -1);
		}
	} else if (player_said("CUPOLA")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			ws_hide_walker(_G(my_walker));
			_ripGoesUpStairsSeries = series_load("703 RIP GOES UP STAIRS", -1, nullptr);
			_ripStairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 320, 0, 100, 0, false, callback, "rip stairs machine");
			sendWSMessage_10000(1, _ripStairsMach, _ripGoesUpStairsSeries, 1, 29, 2, _ripGoesUpStairsSeries, 29, 29, 0);

			break;

		case 2:
			sendWSMessage_10000(1, _ripStairsMach, _ripGoesUpStairsSeries, 30, 39, -1, _ripGoesUpStairsSeries, 39, 39, 0);
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
	} else if (player_said("GATE")) {
		switch (_G(kernel).trigger) {
		case -1:
			ws_walk(_G(my_walker), 1200, 360, nullptr, 2, 11, true);
			break;

		case 2:
			ws_walk(_G(my_walker), 1288, 500, nullptr, -1, 11, true);
			kernel_timing_trigger(60, 3, nullptr);

			break;

		case 3:
			disable_player_commands_and_fade_init(4);
			break;

		case 4:
			adv_kill_digi_between_rooms(false);
			digi_play_loop("950_s39", 3, 255, -1, -1);
			_G(game).new_room = 702;

			break;

		default:
			break;
		}
	} else if (player_said("East Face")) {
		switch (_G(kernel).trigger) {
		case -1:
			disable_player_commands_and_fade_init(4);
			break;

		case 4:
			adv_kill_digi_between_rooms(false);
			digi_play_loop("950_s39", 3, 255, -1, -1);
			_G(game).new_room = 704;

			break;

		default:
			break;
		}
	} else if (player_said("West Face")) {
		switch (_G(kernel).trigger) {
		case -1:
			disable_player_commands_and_fade_init(4);
			break;

		case 4:
			adv_kill_digi_between_rooms(false);
			digi_play_loop("950_s39", 3, 255, -1, -1);
			_G(game).new_room = 706;

			break;

		default:
			break;
		}
	} else if (player_said("rm702")) {
		if (inv_player_has("WISDOM WHEEL") || inv_player_has("SERENITY WHEEL") || inv_player_has("PEACE WHEEL") || inv_player_has("INSIGHT WHEEL") || inv_player_has("TRUTH WHEEL")) {
			player_update_info(_G(my_walker), &_G(player_info));
			ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, -1, 5, true);
			digi_play("703r13", 1, 255, -1, -1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				ws_walk(_G(my_walker), 640, 450, nullptr, -1, 0, false);
				disable_player_commands_and_fade_init(2);

				break;

			case 2:
				adv_kill_digi_between_rooms(false);
				digi_play_loop("950_s39", 3, 255, -1, -1);
				_G(game).new_room = 702;

			default:
				break;
			}
		}
	} else if (!gearFl && !takeFl && player_said_any("MONK #1", "MONK #2", "MONK #3", "MONK #4")) {
		digi_play("com017", 1, 255, -1, -1);
	}
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

void Room703::conv703a() {
	const int32 who = conv_whos_talking();

	if (conv_sound_to_play() == nullptr) {
		conv_resume(conv_get_handle());
	} else if (who <= 0) {
		_field60_should = 121;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(10, 127, nullptr);
		_G(kernel).trigger_mode = KT_PARSE;
	} else if (who == 1) {
		digi_play(conv_sound_to_play(), 1, 255, 90, -1);
	}
}

void Room703::playCheckBrochureAnim(const char *digiName, int32 trigger) {
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

void Room703::useWheelOnNiche(int32 trigger, int val1) {
	switch (_G(kernel).trigger) {
	case -1:
		player_set_commands_allowed(false);
		if (_G(flags[V217])) {
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
		if (_G(flags[V217]))
			break;
		switch (val1) {
		case 1:
			inv_move_object("PEACE WHEEL", 703);
			_G(flags[V217]) = 1;
			digi_play("950_S40A", 2, 255, -1, 950);
			_prayerWheelMach = series_place_sprite("703 PEACE PRAYER WHEEL", 0, 640, 0, 100, 1280);

			break;

		case 2:
			inv_move_object("INSIGHT WHEEL", 703);
			_G(flags[V217]) = 2;
			digi_play("950_S40A", 2, 255, -1, 950);
			_prayerWheelMach = series_place_sprite("703 INSIGHT PRAYER WHEEL", 0, 640, 0, 100, 1280);

			break;

		case 3:
			inv_move_object("SERENITY WHEEL", 703);
			_G(flags[V217]) = 3;
			digi_play("950_S40A", 2, 255, -1, 950);
			_prayerWheelMach = series_place_sprite("703 SERENITY PRAYER WHEEL", 0, 640, 0, 100, 1280);

			break;

		case 4:
			inv_move_object("TRUTH WHEEL", 703);
			_G(flags[V217]) = 4;
			digi_play("950_S40A", 2, 255, -1, 950);
			_prayerWheelMach = series_place_sprite("703 TRUTH PRAYER WHEEL", 0, 640, 0, 100, 1280);

			break;

		case 5:
			inv_move_object("WISDOM WHEEL", 703);
			_G(flags[V217]) = 5;
			digi_play("950_S40A", 2, 255, -1, 950);
			_prayerWheelMach = series_place_sprite("703 WISDOM PRAYER WHEEL", 0, 640, 0, 100, 1280);

			break;

		default:
			// The 'return' is here on purpose, it's not a typo
			return;
		}

		kernel_timing_trigger(5, 105, nullptr);
		break;

	case 105:
		switch (val1) {
		case 1:
			digi_play("705r06", 1, 255, -1, 705);
			sendWSMessage_130000(_G(my_walker), 113);

			break;

		case 2:
			digi_play("705r15", 1, 255, -1, 705);
			sendWSMessage_130000(_G(my_walker), 113);

			break;

		case 3:
			digi_play("704r10", 1, 255, -1, 704);
			sendWSMessage_130000(_G(my_walker), 113);

			break;

		case 4:
			digi_play("706r11", 1, 255, -1, 706);
			sendWSMessage_130000(_G(my_walker), 113);

			break;

		case 5:
			sendWSMessage_130000(_G(my_walker), 106);
			break;

		default:
			break;
		}

		break;

	case 106:
		digi_play("703r12", 1, 255, 107, -1);
		break;

	case 107:
		_monk4Series = series_load("703 MONK 4", -1, nullptr);
		_monkMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 640, -53, 100, 256, false, callback, "monk");
		sendWSMessage_10000(1, _monkMach, _monk4Series, 21, 10, 908, _monk4Series, 10, 10, 0);
		break;

	case 108:
		terminateMachine(_monkMach);
		series_unload(_monk4Series);
		sendWSMessage_150000(_G(my_walker), -1);
		ws_walk(_G(my_walker), 1200, 360, nullptr, 109, 11, true);

		break;

	case 109:
		ws_walk(_G(my_walker), 1288, 500, nullptr, -1, 11, true);
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
		if (_G(flags[V217]) == 5) {
			series_unload(_monk4Series);
			terminateMachine(_monkMach);
		}
		sendWSMessage_150000(_G(my_walker), 114);

		break;

	case 114:
		hotspot_set_active(_G(currentSceneDef).hotspots, "EMPTY NICHE", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "PRAYER WHEEL #4", true);
		player_set_commands_allowed(true);

		break;

	case 908:
		sendWSMessage_10000(1, _monkMach, _monk4Series, 9, 9, -1, _monk4Series, 9, 5, 4);
		sendWSMessage_1a0000(_monkMach, 9);
		digi_play("703d02", 1, 255, 912, -1);

		break;

	case 912:
		sendWSMessage_10000(1, _monkMach, _monk4Series, 4, 1, 113, _monk4Series, 1, 1, 0);
		break;

	default:
		break;
	}
}


} // namespace Rooms
} // namespace Riddle
} // namespace M4
