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

#include "m4/riddle/rooms/section7/room702.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room702::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
}

void Room702::init() {
	_alreadyBeen702Fl = player_been_here(702);
	_G(flags[V212]) = 0;

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_field40 = 0;
		_field44_mode = 0;
		_field48_should = 0;
		_field4C_triggerNum = -1;
		_field50_mode = 0;
		_field54_should = 0;
		_field58 = -1;
	}

	if (_G(game).previous_room == 703) {
		_safariShadow1Series = series_load("SAFARI SHADOW 1", -1, nullptr);
		_guardTalksAndBowsSeries = series_load("GUARD TALKS AND BOWS", -1, nullptr);
		_guardStepsAsideTalksSeries = series_load("GUARD STEPS ASIDE TALKS", -1, nullptr);
		_702GuardShadow1Series = series_load("702 GUARD SHADOW1", -1, nullptr);
		_702GuardShadow2Series = series_load("702 GUARD SHADOW2", -1, nullptr);
		_ripShowsRingSeries = series_load("RIP SHOWS RING", -1, nullptr);
		_ripSafariWalkerPos1Series = series_load("RIP SAFARI WALKER POSITION 1", -1, nullptr);
		_ringCloseupSeries = series_load("Ring closeup", -1, nullptr);
		_G(flags[V210]) = 0;
		_field40 = 0;

		if (_G(flags[V224]) == 0) {
			_guardMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 768, false, callback, "guard");
			sendWSMessage_10000(1, _guardMach, _guardTalksAndBowsSeries, 1, 1, -1, _guardTalksAndBowsSeries, 1, 1, 0);
			_guardShadowMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 487, 326, 100, 1024, false, callback, "guard shadow");
			sendWSMessage_10000(1, _guardShadowMach, _702GuardShadow1Series, 1, 1, -1, _702GuardShadow1Series, 1, 1, 0);
		} else {
			hotspot_set_active(_G(currentSceneDef).hotspots, "GUARD", false);
			hotspot_set_active(_G(currentSceneDef).hotspots, "RING", false);
		}

		player_first_walk(519, 322, 5, 534, 346, 1, true);
	} else {
		digi_preload("950_s39");
		_safariShadow1Series = series_load("SAFARI SHADOW 1", -1, nullptr);
		_guardTalksAndBowsSeries = series_load("GUARD TALKS AND BOWS", -1, nullptr);
		_guardStepsAsideTalksSeries = series_load("GUARD STEPS ASIDE TALKS", -1, nullptr);
		_702GuardShadow1Series = series_load("702 GUARD SHADOW1", -1, nullptr);
		_702GuardShadow2Series = series_load("702 GUARD SHADOW2", -1, nullptr);
		_ripShowsRingSeries = series_load("RIP SHOWS RING", -1, nullptr);
		_ripSafariWalkerPos1Series = series_load("RIP SAFARI WALKER POSITION 1", -1, nullptr);
		_ringCloseupSeries = series_load("Ring closeup", -1, nullptr);
		_G(flags[V210]) = 0;
		_field40 = 0;
		if (_G(flags[V224]) == 0) {
			_guardMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 768, false, callback, "guard");
			sendWSMessage_10000(1, _guardMach, _guardTalksAndBowsSeries, 1, 1, -1, _guardTalksAndBowsSeries, 1, 1, 0);
			_guardShadowMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 487, 326, 100, 1024, false, callback, "guard shadow");
			sendWSMessage_10000(1, _guardShadowMach, _702GuardShadow1Series, 1, 1, -1, _702GuardShadow1Series, 1, 1, 0);
		} else {
			hotspot_set_active(_G(currentSceneDef).hotspots, "GUARD", false);
			hotspot_set_active(_G(currentSceneDef).hotspots, "RING", false);
		}

		if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
			player_first_walk(660, 347, 11, 524, 342, 11, true);
		}
	}

	digi_play_loop("950_s39", 3, 255, -1, -1);
}

void Room702::pre_parser() {
	if (player_said_any("  ", "   ") && _G(flags[V211]) && _G(flags[V210]) && _G(flags[V212])) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (_G(flags[V224]) && player_said_any("  ", "   ")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}
}

void Room702::parser() {
	bool lookFl = player_said_any("look", "look at");
	bool talkFl = player_said_any("talk", "talk to");
	bool takeFl = player_said("take");
	bool gearFl = player_said_any("push", "pull", "gear", "open", "close");

	if (player_said("CIGAR BAND", "GUARD") && inv_player_has("CIGAR BAND") && !_G(flags[V211])) {
		_G(flags[V211]) = 1;
		_G(flags[V228]) = 1;
		_G(kernel).trigger_mode = KT_DAEMON;
		kernel_timing_trigger(10, 40, nullptr);
		_G(kernel).trigger_mode = KT_PARSE;
	} else if (player_said("conv702a")) {
		if (_G(kernel).trigger != 1) {
			conv702a();
			_G(flags[V211]) = 1;
			_G(flags[V228]) = 1;
		} else {
			int32 who = conv_whos_talking();
			if (who <= 0) {
				_field54_should = 2101;
			} else if (who == 1) {
				_field48_should = 1103;
			}
			conv_resume(conv_get_handle());
		}
	} else if (talkFl && player_said("GUARD")) {
		player_set_commands_allowed(false);
		if (!_field40) {
			_field58 = -1;
			_field44_mode = 1000;
			_field48_should = 1100;
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_trigger_dispatchx(kernel_trigger_create(100));
			_G(kernel).trigger_mode = KT_PARSE;
		} else {
			_field40 = 0;
			sendWSMessage_10000(1, _guardMach, _guardStepsAsideTalksSeries, 27, 1, 100, _guardStepsAsideTalksSeries, 1, 1, 0);
			sendWSMessage_10000(1, _guardShadowMach, _702GuardShadow2Series, 1, 1, -1, _702GuardShadow2Series, 1, 1, 0);
			_G(flags[V212]) = 0;
			_field58 = -1;
			_field44_mode = 1000;
			_field48_should = 1100;
		}
	} else if (lookFl && player_said("TEMPLE"))
		digi_play("702R02", 1, 255, -1, -1);
	else if (lookFl && player_said("WALL"))
		digi_play("702R03", 1, 255, -1, -1);
	else if (lookFl && player_said("CEREMONIAL TORANA"))
		digi_play("702R04", 1, 255, -1, -1);
	else if (lookFl && player_said("Ring")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			_ringCloseupMach = series_place_sprite("Ring closeup", 0, 0, -53, 100, 0);
			digi_play("702_S01", 1, 255, 2, -1);

			break;

		case 2:
			terminateMachine(_ringCloseupMach);
			break;

		default:
			break;
		}
	} else if (lookFl && player_said("GUARD")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			if (!_G(flags[V214]) && !_G(flags[V228])) {
				digi_play("702R05", 1, 255, 2, -1);
			} else {
				digi_play("702R05", 1, 255, 3, -1);
			}

			break;

		case 2:
			digi_play("702R05A", 1, 255, 3, -1);
			break;

		case 3:
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
	} // ecx && player_said("GUARD")
	else if (takeFl && _G(player).click_y <= 374) {
		switch (imath_ranged_rand(1, 6)) {
		case 1:
			digi_play("com006", 1, 255, -1, -1);
			break;
		case 2:
			digi_play("com007", 1, 255, -1, -1);
			break;
		case 3:
			digi_play("com008", 1, 255, -1, -1);
			break;
		case 4:
			digi_play("com009", 1, 255, -1, -1);
			break;
		case 5:
			digi_play("com010", 1, 255, -1, -1);
			break;
		case 6:
			digi_play("com011", 1, 255, -1, -1);
			break;

		default:
			break;
		}
	} // esi && _G(player).click_y <= 374
	else if (player_said("  ") && _G(flags[V224])) {
		player_set_commands_allowed(false);
		switch (_G(kernel).trigger) {
		case -1:
			ws_walk(_G(my_walker), 500, 287, nullptr, 2, 11, false);
			break;

		case 2:
			ws_walk(_G(my_walker), 396, 184, nullptr, -1, 11, true);
			kernel_timing_trigger(180, 3, nullptr);

			break;

		case 3:
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
	} // player_said("  ") && _G(flags[V224])
	if (player_said("exit")) {
		switch (_G(kernel).trigger) {
		case -1:
			disable_player_commands_and_fade_init(4);
			break;

		case 4:
			if (_G(flags[V211]) && !_G(flags[V210]))
				_G(flags[V213]) = 1;

			_G(game).new_room = 701;
			break;

		default:
			break;
		}
	} // player_said("exit")
	else if (player_said_any("  ", "   ") || (player_said("CIGAR BAND", "GUARD") && _G(flags[V211]))) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			if (_G(flags[V224]))
				kernel_timing_trigger(10, 84, nullptr);
			else if (!_G(flags[V211]))
				kernel_timing_trigger(10, 5, nullptr);
			else if (_G(flags[V210]))
				kernel_timing_trigger(10, _G(flags[V212]) ? 84 : 83, nullptr);
			else {
				setGlobals1(_ripShowsRingSeries, 1, 10, 10, 10, 0, 10, 19, 19, 19, 0, 19, 30, 30, 30, 0, 0, 0, 0, 0, 0);
				sendWSMessage_110000(_G(my_walker), 80);
			}

			break;

		case 2:
			ws_walk(_G(my_walker), 396, 184, nullptr, -1, 11, true);
			kernel_timing_trigger(180, 3, nullptr);

			break;

		case 3:
			disable_player_commands_and_fade_init(4);
			break;

		case 4:
			adv_kill_digi_between_rooms(false);
			digi_play_loop("950_s39", 3, 255, -1, -1);
			if (_G(flags[V211]) && !_G(flags[V210])) {
				_G(flags[V213]) = 1;
			}

			_G(game).new_room = 703;

			break;

		case 5:
			digi_play("702G20", 1, 255, 6, -1);
			sendWSMessage_10000(1, _guardMach, _guardTalksAndBowsSeries, 8, 11, -1, _guardTalksAndBowsSeries, 8, 11, 1);
			sendWSMessage_10000(1, _guardShadowMach, _702GuardShadow1Series, 8, 11, -1, _702GuardShadow1Series, 8, 11, 1);

			break;

		case 6:
			sendWSMessage_10000(1, _guardMach, _guardTalksAndBowsSeries, 8, 8, -1, _guardTalksAndBowsSeries, 8, 8, 0);
			sendWSMessage_10000(1, _guardShadowMach, _702GuardShadow1Series, 8, 8, -1, _702GuardShadow1Series, 8, 8, 0);
			kernel_timing_trigger(10, 8, nullptr);

			break;

		case 7:
			sendWSMessage_150000(_G(my_walker), 8);
			break;

		case 8:
			player_set_commands_allowed(true);
			break;

		case 80:
			sendWSMessage_120000(_G(my_walker), 81);
			break;

		case 81:
			sendWSMessage_130000(_G(my_walker), 82);
			break;

		case 82:
			sendWSMessage_150000(_G(my_walker), 83);
			break;

		case 83:
			sendWSMessage_10000(1, _guardMach, _guardStepsAsideTalksSeries, 1, 27, 84, _guardStepsAsideTalksSeries, 27, 27, 0);
			sendWSMessage_10000(1, _guardShadowMach, _702GuardShadow2Series, 1, 27, -1, _702GuardShadow2Series, 27, 27, 0);

			break;

		case 84:
			ws_walk(_G(my_walker), 500, 287, nullptr, 2, 11, false);
			break;

		default:
			break;
		}
	} // player_said_any("  ", "   ") || (player_said("CIGAR BAND", "GUARD") && _G(flags[V211]))
	else if (lookFl && player_said(" ")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			digi_play("702R01", 1, 255, 2, -1);
			break;

		case 2:
			if (_G(flags[V227])) {
				kernel_timing_trigger(10, 3, nullptr);
			} else {
				_G(flags[V227]) = 1;
				digi_play("702R01A", 1, 255, 3, -1);
			}
			break;

		case 3:
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
	} // ecx && player_said(" ")
	else if (player_said("GUARD", "CIGAR") || player_said("GUARD", "CIGAR WITHOUT BAND"))
		digi_play("com083", 1, 255, -1, -1);
	else if (!takeFl && !gearFl && player_said("GUARD"))
		digi_play("702R21", 1, 255, -1, -1);
	else
		return;

	_G(player).command_ready = false;
}

void Room702::daemon() {
	switch (_G(kernel).trigger) {
	case 40:
		player_set_commands_allowed(false);
		setGlobals1(1, _ripShowsRingSeries, 10, 10, 10, 0, 10, 19, 19, 19, 0, 19, 30, 30, 30, 0, 0, 0, 0, 0, 0);
		if (_G(flags[V210])) {
			sendWSMessage_110000(_G(my_walker), 41);
		} else {
			_G(flags[V210]) = 1;
			sendWSMessage_110000(_G(my_walker), 60);
		}

		break;

	case 41:
		digi_play("702R06", 1, 255, 42, -1);
		break;

	case 42:
		sendWSMessage_120000(_G(my_walker),43);
		break;

	case 43:
		digi_play("702R07",1,255,44,-1);
		break;

	case 44:
		sendWSMessage_130000(_G(my_walker), 45);
		break;

	case 45:
		sendWSMessage_150000(_G(my_walker), 46);
		break;

	case 46:
		kernel_timing_trigger(1,47,nullptr);
		break;

	case 47:
		sendWSMessage_10000(1, _guardMach, _guardTalksAndBowsSeries, 36, 46, 48, _guardTalksAndBowsSeries,46,46,0);
		sendWSMessage_10000(1, _guardShadowMach, _702GuardShadow1Series, 36, 46, -1, _702GuardShadow1Series,46,46,0);

		break;

	case 48:
		sendWSMessage_10000(1, _guardMach, _guardTalksAndBowsSeries, 46, 36, 49, _guardTalksAndBowsSeries, 1, 1, 0);
		sendWSMessage_10000(1, _guardShadowMach, _702GuardShadow1Series, 46, 36, -1, _702GuardShadow1Series, 1, 1, 0);

		break;

	case 49:
		digi_play("702G03", 1, 255, 50, -1);
		sendWSMessage_10000(1, _guardMach, _guardStepsAsideTalksSeries, 1, 27, -1, _guardStepsAsideTalksSeries,27,29,1);
		sendWSMessage_10000(1, _guardShadowMach, _702GuardShadow2Series, 1, 27, -1, _702GuardShadow2Series,27,29,1);

		break;

	case 50:
		sendWSMessage_10000(1, _guardMach, _guardStepsAsideTalksSeries, 29, 40, -1, _guardStepsAsideTalksSeries, 40, 40, 0);
		sendWSMessage_10000(1, _guardShadowMach, _702GuardShadow2Series, 29, 40, -1, _702GuardShadow2Series, 40, 40, 0);
		_G(flags[V212]) = 1;

		digi_play("702R11",1,255,51,-1);

		break;

	case 51:
		digi_play("702G04", 1, 255, 52, -1);
		sendWSMessage_10000(1, _guardMach, _guardStepsAsideTalksSeries, 41, 46, -1, _guardStepsAsideTalksSeries, 46, 41, 1);
		sendWSMessage_10000(1, _guardShadowMach, _702GuardShadow2Series, 41, 46, -1, _702GuardShadow2Series, 46, 41, 1);

		break;

	case 52:
		sendWSMessage_10000(1, _guardMach, _guardStepsAsideTalksSeries, 46, 46, -1, _guardStepsAsideTalksSeries, 46, 46, 0);
		sendWSMessage_10000(1, _guardShadowMach, _702GuardShadow2Series, 46, 46, -1, _702GuardShadow2Series, 46, 46, 0);
		_field40 = 1;
		player_set_commands_allowed(true);

		break;

	case 60:
		digi_play("702R08", 1, 255, 61, -1);
		break;

	case 61:
		sendWSMessage_120000(_G(my_walker), 62);
		break;

	case 62:
		digi_play("702G01", 1, 255, 63, -1);
		sendWSMessage_10000(1, _guardMach, _guardTalksAndBowsSeries, 8, 11, -1, _guardTalksAndBowsSeries, 8, 11, 1);
		sendWSMessage_10000(1, _guardShadowMach, _702GuardShadow1Series, 8, 11, -1, _702GuardShadow1Series, 8, 11, 1);
		break;

	case 63:
		sendWSMessage_10000(1, _guardMach, _guardTalksAndBowsSeries, 11, 11, -1, _guardTalksAndBowsSeries, 11, 11, 0);
		sendWSMessage_10000(1, _guardShadowMach, _702GuardShadow1Series, 11, 11, -1, _702GuardShadow1Series, 11, 11, 0);
		digi_play("702R09", 1, 255, 64, -1);
		sendWSMessage_130000(_G(my_walker), -1);

		break;

	case 64:
		digi_play("702G02", 1, 255, 65, -1);
		sendWSMessage_10000(1, _guardMach, _guardTalksAndBowsSeries, 8, 11, -1, _guardTalksAndBowsSeries, 8, 11, 1);
		sendWSMessage_10000(1, _guardShadowMach, _702GuardShadow1Series, 8, 11, -1, _702GuardShadow1Series, 8, 11, 1);
		sendWSMessage_150000(_G(my_walker), -1);

		break;

	case 65:
		sendWSMessage_10000(1, _guardMach, _guardTalksAndBowsSeries, 11, 11, -1, _guardTalksAndBowsSeries, 11, 11, 0);
		sendWSMessage_10000(1, _guardShadowMach, _702GuardShadow1Series, 11, 11, -1, _702GuardShadow1Series, 11, 11, 0);
		digi_play("702R10", 1, 255, 47, -1);

		break;

	case 100:
		if (_field4C_triggerNum != -1) {
			kernel_trigger_dispatchx(_field4C_triggerNum);
			_field4C_triggerNum = -1;
		}

		kernel_timing_trigger(1, 101, nullptr);

		break;

	case 101:
		if (_field44_mode == 1000) {
			switch (_field48_should) {
			case 1100:
				ws_hide_walker(_G(my_walker));
				player_set_commands_allowed(false);
				player_update_info(_G(my_walker), &_G(player_info));
				_ripTalksGuardMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 256, true, callback, "rip talks guard");
				_ripTalksGuardShadowMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 256, false, callback, "rip talks guard SHADOW");
				sendWSMessage_10000(1, _ripTalksGuardMach, _ripShowsRingSeries, 1, 1, -1, _ripShowsRingSeries, 1, 1, 0);
				sendWSMessage_10000(1, _ripTalksGuardShadowMach, _safariShadow1Series, 1, 1, 100, _safariShadow1Series, 1, 1, 0);

				_field48_should = 1102;
				_field50_mode = 2000;
				_field54_should = 2100;

				kernel_trigger_dispatchx(kernel_trigger_create(110));

				break;

			case 1102:
				conv_load("conv702a",10,10,103);
				conv_export_value_curr(_G(flags[V211]), 0);
				conv_export_value_curr(_G(flags[V213]), 1);
				conv_export_value_curr(_G(flags[V222]), 2);
				conv_play(conv_get_handle());

				_field48_should = 1103;

				kernel_trigger_dispatchx(kernel_trigger_create(100));

				break;

			case 1103:
			case 1104:
				sendWSMessage_10000(1, _ripTalksGuardMach, _ripShowsRingSeries, 1, 1, 100, _ripShowsRingSeries, 1, 1, 0);
				break;

			case 1105:
				terminateMachine(_ripTalksGuardMach);
				terminateMachine(_ripTalksGuardShadowMach);

				_field54_should = 2106;

				ws_unhide_walker(_G(my_walker));
				player_set_commands_allowed(true);

				break;

			default:
				break;
			}
		}

		break;

	case 102:
		kernel_trigger_dispatchx(kernel_trigger_create(100));
		break;

	case 103:
		_field48_should = 1105;
		break;

	case 110:
		if (_field50_mode == 2000) {
			switch (_field54_should) {
			case 2100:
			case 2101:
			case 2103:
			case 2104:
			case 2106:
			case 2109:
				kernel_timing_trigger(1, 111, nullptr);
				break;

			default:
				break;

			}
		}

		break;

	case 111:
		if (_field50_mode == 2000) {
			switch (_field54_should) {
			case 2100:
			case 2101:
				sendWSMessage_10000(1, _guardMach, _guardTalksAndBowsSeries, 8, 8, 110, _guardTalksAndBowsSeries, 8, 8, 0);
				sendWSMessage_10000(1, _guardShadowMach, _702GuardShadow1Series, 8, 8, -1, _702GuardShadow1Series, 8, 8, 0);
				_field54_should = 2101;

				break;

			case 2103:
				_G(kernel).trigger_mode = KT_PARSE;
				digi_play(conv_sound_to_play(), 1, 255, 1, -1);
				_G(kernel).trigger_mode = KT_DAEMON;
				_field54_should = 2104;
				kernel_timing_trigger(1, 111, nullptr);

				break;
			case 2104: {
				int32 rnd = imath_ranged_rand(8, 10);
				sendWSMessage_10000(1, _guardMach, _guardTalksAndBowsSeries, rnd, rnd, 110, _guardTalksAndBowsSeries, rnd, rnd, 0);
				sendWSMessage_10000(1, _guardShadowMach, _702GuardShadow1Series, rnd, rnd, -1, _702GuardShadow1Series, rnd, rnd, 0);
				}

				break;

			case 2106:
				sendWSMessage_10000(1, _guardMach, _guardTalksAndBowsSeries, 8, 8, -1, _guardTalksAndBowsSeries, 8, 8, 0);
				sendWSMessage_10000(1, _guardShadowMach, _702GuardShadow1Series, 8, 8, -1, _702GuardShadow1Series, 8, 8, 0);

				break;

			case 2107:
				sendWSMessage_10000(1, _guardMach, _guardTalksAndBowsSeries, 48, 58, 110, _guardTalksAndBowsSeries, 58, 58, 0);
				sendWSMessage_10000(1, _guardShadowMach, _702GuardShadow1Series, 48, 58, -1, _702GuardShadow1Series, 58, 58, 0);
				_field54_should = 2108;

				break;

			case 2108:
				digi_play("702_S01", 1, 255, -1, -1);
				sendWSMessage_10000(1, _guardMach, _guardTalksAndBowsSeries, 58, 82, 110, _guardTalksAndBowsSeries, 82, 82, 0);
				sendWSMessage_10000(1, _guardShadowMach, _702GuardShadow1Series, 58, 82, -1, _702GuardShadow1Series, 82, 82, 0);
				_field54_should = 2109;

				break;

			case 2109:
				terminateMachine(_ringCloseupMach);
				_field54_should = 2103;
				kernel_timing_trigger(1, 111, nullptr);

				break;

			default:
				break;
			}
		}
		break;

	default:
		break;
	}
}

void Room702::callback(frac16 myMessage, machine *sender) {
	kernel_trigger_dispatchx(myMessage);
}

void Room702::conv702a() {
	if (conv_sound_to_play() == nullptr) {
		conv_resume(conv_get_handle());
		return;
	}

	int32 who = conv_whos_talking();
	if (who == 0) {
		int32 node = conv_current_node();
		int32 entry = conv_current_entry();

		if (node == 1 && (entry == 1 || entry == 2)) {
			_field54_should = 2107;
		} else {
			_field54_should = 2103;
		}
	} else if (who == 1) {
		_field48_should = 1104;
		digi_play(conv_sound_to_play(), 1, 255, 1, -1);
	}
}

void Room702::syncGame(Common::Serializer &s) {
	s.syncAsSint32LE(_field40);
	s.syncAsSint32LE(_field44_mode);
	s.syncAsSint32LE(_field48_should);
	s.syncAsSint32LE(_field4C_triggerNum);
	s.syncAsSint32LE(_field50_mode);
	s.syncAsSint32LE(_field54_should);
	s.syncAsSint32LE(_field58);
}
} // namespace Rooms
} // namespace Riddle
} // namespace M4
