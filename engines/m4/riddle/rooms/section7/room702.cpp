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
	_field3C = player_been_here(702) ? 1 : 0;
	_G(flags[V212]) = 0;

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_field40 = 0;
		_field44 = 0;
		_field48 = 0;
		_field4C_triggerNum = -1;
		_field50 = 0;
		_field54 = 0;
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
	//TODO
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
		if (_field44 == 1000) {
			switch (_field48) {
			case 1100:
				ws_hide_walker(_G(my_walker));
				player_set_commands_allowed(false);
				player_update_info(_G(my_walker), &_G(player_info));
				_ripTalksGuardMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 256, true, callback, "rip talks guard");
				_ripTalksGuardShadowMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 256, false, callback, "rip talks guard SHADOW");
				sendWSMessage_10000(1, _ripTalksGuardMach, _ripShowsRingSeries, 1, 1, -1, _ripShowsRingSeries, 1, 1, 0);
				sendWSMessage_10000(1, _ripTalksGuardShadowMach, _safariShadow1Series, 1, 1, 100, _safariShadow1Series, 1, 1, 0);

				_field48 = 1102;
				_field50 = 2000;
				_field54 = 2100;

				kernel_trigger_dispatchx(kernel_trigger_create(110));

				break;

			case 1102:
				conv_load("conv702a",10,10,103);
				conv_export_value_curr(_G(flags[V211]), 0);
				conv_export_value_curr(_G(flags[V213]), 1);
				conv_export_value_curr(_G(flags[V222]), 2);
				conv_play(conv_get_handle());

				_field48 = 1103;

				kernel_trigger_dispatchx(kernel_trigger_create(100));

				break;

			case 1103:
			case 1104:
				sendWSMessage_10000(1, _ripTalksGuardMach, _ripShowsRingSeries, 1, 1, 100, _ripShowsRingSeries, 1, 1, 0);
				break;

			case 1105:
				terminateMachine(_ripTalksGuardMach);
				terminateMachine(_ripTalksGuardShadowMach);

				_field54 = 2106;

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
		_field48 = 1105;
		break;

	case 110:
		if (_field50 == 2000) {
			switch (_field54) {
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
		if (_field50 == 2000) {
			switch (_field54) {
			case 2100:
			case 2101:
				sendWSMessage_10000(1, _guardMach, _guardTalksAndBowsSeries, 8, 8, 110, _guardTalksAndBowsSeries, 8, 8, 0);
				sendWSMessage_10000(1, _guardShadowMach, _702GuardShadow1Series, 8, 8, -1, _702GuardShadow1Series, 8, 8, 0);
				_field54 = 2101;

				break;

			case 2103:
				_G(kernel).trigger_mode = KT_PARSE;
				digi_play(conv_sound_to_play(), 1, 255, 1, -1);
				_G(kernel).trigger_mode = KT_DAEMON;
				_field54 = 2104;
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
				_field54 = 2108;

				break;

			case 2108:
				digi_play("702_S01", 1, 255, -1, -1);
				sendWSMessage_10000(1, _guardMach, _guardTalksAndBowsSeries, 58, 82, 110, _guardTalksAndBowsSeries, 82, 82, 0);
				sendWSMessage_10000(1, _guardShadowMach, _702GuardShadow1Series, 58, 82, -1, _702GuardShadow1Series, 82, 82, 0);
				_field54 = 2109;

				break;

			case 2109:
				terminateMachine(_field10_mach);
				_field54 = 2103;
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

} // namespace Rooms
} // namespace Riddle
} // namespace M4
