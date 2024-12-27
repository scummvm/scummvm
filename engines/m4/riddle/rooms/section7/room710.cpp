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

#include "m4/riddle/rooms/section7/room710.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room710::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
}

void Room710::init() {
	_710Rpld2Series = series_load("710RPLD2", -1, nullptr);
	_710Rpld3Series = series_load("710RPLD3", -1, nullptr);
	_ripPullsRopeForLaderSeries = series_load("RIP PULLS ROPE FOR LADER", -1, nullptr);
	_laderComesDownSeries = series_load("LADER COMES DOWN", -1, nullptr);
	_ripTrekMedReachHandPos1Series = series_load("RIP TREK MED REACH HAND POS1", -1, nullptr);
	_710Rpro1Series = series_load("710RPRO1", -1, nullptr);
	_710Rpro2Series = series_load("710RPRO2", -1, nullptr);
	_710Rpro3Series = series_load("710RPRO3", -1, nullptr);
	_710Rpro4Series = series_load("710RPRO4", -1, nullptr);
	_mazeCentreDoorLiteSeries = series_load("MAZE CENTRE DOOR LITE", -1, nullptr);

	digi_preload("710_s02", -1);
	digi_preload("710_s01", -1);

	_field4C = 0;

	_ripContraptionMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 256, false, triggerMachineByHashCallback, "rip contraption machine");
	_ripReacherMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 1792, false, triggerMachineByHashCallback, "rip reacher machine");
	_710Rprs3Mach = series_place_sprite("710RPRS3", 0, 0, -53, 100, 2565);
	_710Rprs2Mach = series_place_sprite("710RPRS2", 0, 0, -53, 100, 2053);
	_710Rprs1Mach = series_place_sprite("710RPRS1", 0, 0, -53, 100, 517);
	_710Rprs4Mach = series_place_sprite("710RPRS4", 0, 0, -53, 100, 1029);

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		digi_preload("950_s41", -1);
		if (_G(flags[V223])) {
			_field1C = 1;
			_710Rpld4Mach = series_place_sprite("710rpld4", 0, 0, -53, 100, 1541);
			_710Rpld5Mach = series_place_sprite("710rpld5", 0, 0, -53, 100, 3840);
		} else {
			_field1C = 0;
			_710Rpld4Mach = series_place_sprite("710rpld3", 0, 0, -53, 100, 3840);
			_710Rpld5Mach = series_place_sprite("710rpld5", 0, 0, -53, 100, 3840);
			hotspot_set_active(_G(currentSceneDef).hotspots, "Ladder", false);
		}
		break;

	case 709:
		ws_demand_location(_G(my_walker), 320, 282);
		ws_demand_facing(_G(my_walker), 5);

		if (_G(flags[V223])) {
			_field1C = 1;
			_710Rpld4Mach = series_place_sprite("710rpld4", 0, 0, -53, 100, 1541);
			_710Rpld5Mach = series_place_sprite("710rpld5", 0, 0, -53, 100, 3840);
		} else {
			_field1C = 0;
			hotspot_set_active(_G(currentSceneDef).hotspots, "Ladder", false);
			_710Rpld4Mach = series_place_sprite("710rpld3", 0, 0, -53, 100, 3840);
			_710Rpld5Mach = series_place_sprite("710rpld5", 0, 0, -53, 100, 3840);
		}

		break;

	case 711:
		digi_preload("950_s41", -1);
		ws_demand_location(_G(my_walker), 320, 308);
		ws_demand_facing(_G(my_walker), 1);
		ws_hide_walker(_G(my_walker));
		player_set_commands_allowed(false);

		_710Rpld4Mach = series_place_sprite("710rpld4", 0, 0, -53, 100, 1541);
		_710Rpld5Mach = series_place_sprite("710rpld5", 0, 0, -53, 100, 4095);

		_field1C = 1;
		hotspot_set_active(_G(currentSceneDef).hotspots, "Ladder", true);
		sendWSMessage_10000(1, _ripContraptionMach, _710Rpld2Series, 86, 83, 30, _710Rpld2Series, 83, 83, 0);

		break;

	default:
		digi_preload("950_s41", -1);
		ws_demand_location(_G(my_walker), 282, 282);
		ws_demand_facing(_G(my_walker), 4);

		if (_G(flags[V223])) {
			_field1C = 1;
			_710Rpld4Mach = series_place_sprite("710rpld4", 0, 0, -53, 100, 1541);
			_710Rpld5Mach = series_place_sprite("710rpld5", 0, 0, -53, 100, 4095);
		} else {
			_field1C = 0;
			_710Rpld4Mach = series_place_sprite("710rpld3", 0, 0, -53, 100, 3840);
			_710Rpld5Mach = series_place_sprite("710rpld5", 0, 0, -53, 100, 4095);
			hotspot_set_active(_G(currentSceneDef).hotspots, "Ladder", false);
		}

		break;
	}

	if (player_been_here(710))
		midi_play("drama1", 127, 0, -1, 949);

	digi_play_loop("950_s41", 3, 255, -1, -1);
}

void Room710::pre_parser() {
	// No implementation
}

void Room710::parser() {
	bool ecx = player_said_any("look", "look at");
	bool edi = player_said_any("talk", "talk to", "take");
	bool esi = player_said_any("push", "pull", "gear", "open", "close");

	if (edi && player_said("LADDER")) {
		digi_play("710R09", 1, 255, -1, -1);
	} else if (edi && player_said("LADDER ")) {
		digi_play(_field1C ? "710R09" : "710R10", 1, 255, -1, -1);
	} else if (edi && player_said("ROPE")) {
		digi_play("710R08", 1, 255, -1, -1);
	} else if (ecx && player_said_any("LADDER", "LADDER ")) {
		player_set_commands_allowed(false);

		if (_field1C) {
			digi_play("710R21", 1, 255, -1, -1);
		} else if (_G(flags[V030] == 0)) {
			digi_play("710R02", 1, 255, -1, -1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				digi_play("710R02", 1, 255, 2, -1);
				break;

			case 2:
				digi_play("710R03", 1, 255, -1, -1);
				break;

			default:
				break;
			}
		}

		player_set_commands_allowed(true);
	} // ecx && player_said_any("LADDER", "LADDER ")

	else if (ecx && player_said("Rope")) {
		player_set_commands_allowed(false);
		digi_play("710R04", 1, 255, -1, -1);
		player_set_commands_allowed(true);
	} else if (ecx && player_said("Mooring") && _field1C) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			digi_play("710R05", 1, 255, 2, -1);

			break;

		case 2:
			digi_play("710R07", 1, 255, -1, -1);
			player_set_commands_allowed(true);

			break;

		default:
			break;
		}
	} // ecx && player_said("Mooring") && _field1C

	else if (ecx && player_said("MOORING")) {
		player_set_commands_allowed(false);
		digi_play("710R05", 1, 255, -1, -1);
		player_set_commands_allowed(true);
	} else if (ecx && player_said("MACHINERY")) {
		player_set_commands_allowed(false);
		digi_play("710R06", 1, 255, -1, -1);
		player_set_commands_allowed(true);
	} else if (ecx && player_said(" ")) {
		player_set_commands_allowed(false);
		digi_play(_field1C ? "710R20" : "710R01", 1, 255, -1, -1);
		player_set_commands_allowed(true);
	} else if (esi && player_said("Rope")) {
		if (_G(flags[V223])) {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				player_update_info(_G(my_walker), &_G(player_info));
				ws_hide_walker(_G(my_walker));
				_safariShadow1Mach = series_place_sprite("SAFARI SHADOW 1", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3840);
				_ripPullsKeyropeMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 3840, false, triggerMachineByHashCallback, "rip pulls keyrope machine");
				sendWSMessage_10000(1, _ripPullsKeyropeMach, _ripPullsRopeForLaderSeries, 1, 15, 1, _ripPullsRopeForLaderSeries, 15, 15, 0);

				break;

			case 1:
				sendWSMessage_10000(1, _ripPullsKeyropeMach, _ripPullsRopeForLaderSeries, 16, 21, 21, _ripPullsRopeForLaderSeries, 21, 21, 0);
				digi_play("710_s03a", 2, 255, -1, -1);
				kernel_timing_trigger(10, 2, nullptr);

				break;

			case 2:
				hotspot_set_active(_G(currentSceneDef).hotspots, "Ladder", false);
				_field1C = 0;

				terminateMachine(_710Rpld4Mach);
				terminateMachine(_710Rpld5Mach);

				_ripContractionMach02 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 1541, false, triggerMachineByHashCallback, "rip contraption machine");
				sendWSMessage_10000(1, _ripContractionMach02, _laderComesDownSeries, 47, 37, 31, _laderComesDownSeries, 37, 37, 0);
				digi_play("710_s01", 1, 255, -1, -1);

				break;

			case 3:
				terminateMachine(_ripContractionMach02);
				_710Rpld4Mach = series_place_sprite("710rpld3", 0, 0, -53, 100, 3840);
				_G(flags[V223]) = 0;
				player_set_commands_allowed(true);

				break;

			case 21:
				sendWSMessage_10000(1, _ripPullsKeyropeMach, _ripPullsRopeForLaderSeries, 22, 44, 22, _ripPullsRopeForLaderSeries, 44, 44, 0);
				break;

			case 22:
				terminateMachine(_ripPullsKeyropeMach);
				_710Rpld5Mach = series_place_sprite("710rpld5", 0, 0, -53, 100, 3840);
				terminateMachine(_safariShadow1Mach);
				ws_unhide_walker(_G(my_walker));
				ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, -1, 5, true);

				break;

			case 31:
				sendWSMessage_10000(1, _ripContractionMach02, _laderComesDownSeries, 36, 1, 3, _laderComesDownSeries, 1, 1, 0);
				break;

			default:
				break;
			}
		} else {
			_field1C = 1;

			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				player_update_info(_G(my_walker), &_G(player_info));
				_safariShadow1Mach = series_place_sprite("SAFARI SHADOW 1", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3840);
				ws_hide_walker(_G(my_walker));
				hotspot_set_active(_G(currentSceneDef).hotspots, "Ladder", true);
				terminateMachine(_710Rpld5Mach);
				_ripPullsKeyropeMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 3840, false, triggerMachineByHashCallback, "rip pulls keyrope machine");
				sendWSMessage_10000(1, _ripPullsKeyropeMach, _ripPullsRopeForLaderSeries, 1, 15, 1, _ripPullsRopeForLaderSeries, 15, 15, 0);

				break;

			case 1:
				digi_play("710_s03", 2, 255, -1, -1);
				sendWSMessage_10000(1, _ripPullsKeyropeMach, _ripPullsRopeForLaderSeries, 16, 21, 2, _ripPullsRopeForLaderSeries, 21, 21, 0);

				break;

			case 2:
				sendWSMessage_10000(1, _ripPullsKeyropeMach, _ripPullsRopeForLaderSeries, 22, 44, 21, _ripPullsRopeForLaderSeries, 44, 44, 0);
				kernel_timing_trigger(10, 22, nullptr);

				break;

			case 3:
				terminateMachine(_ripContractionMach02);
				_710Rpld4Mach = series_place_sprite("710rpld4", 0, 0, -53, 100, 1541);
				_G(flags[V223]) = 1;
				player_set_commands_allowed(true);
				digi_play("710r07", 1, 255, -1, -1);

				break;

			case 21:
				terminateMachine(_ripPullsKeyropeMach);
				_710Rpld5Mach = series_place_sprite("710rpld5", 0, 0, -53, 100, 3840);
				terminateMachine(_safariShadow1Mach);
				ws_unhide_walker(_G(my_walker));
				player_update_info(_G(my_walker), &_G(player_info));
				ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, -1, 5, true);

				break;

			case 22:
				terminateMachine(_710Rpld4Mach);
				_ripContractionMach02 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 1541, false, triggerMachineByHashCallback, "rip contraption machine");
				sendWSMessage_10000(1, _ripContractionMach02, _laderComesDownSeries, 1, 18, 31, _laderComesDownSeries, 18, 18, 0);

				break;

			case 31:
				sendWSMessage_10000(1, _ripContractionMach02, _laderComesDownSeries, 19, 47, 3, _laderComesDownSeries, 47, 47, 0);
				digi_play("710_s01", 1, 255, -1, -1);

				break;

			default:
				break;
			}
		}
	} // esi && player_said("Rope")

	else if (ecx && player_said_any("rope  ", "rope   ", "rope    ", "rope     "))
		digi_play("com110", 1, 255, -1, 997);
	else if (esi && player_said("rope   ")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			terminateMachine(_710Rprs3Mach);
			_ripPullMach02 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 3840, false, triggerMachineByHashCallback, "rip pull machine");
			player_update_info(_G(my_walker), &_G(player_info));
			_safariShadow1Mach = series_place_sprite("SAFARI SHADOW 1", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3840);
			ws_hide_walker(_G(my_walker));
			sendWSMessage_10000(1, _ripPullMach02, _710Rpro3Series, 1, 16, 1, _710Rpro3Series, 16, 16, 0);

			break;

		case 1:
			digi_play("950_s42", 2, 255, -1, -1);
			sendWSMessage_10000(1, _ripPullMach02, _710Rpro3Series, 17, 38, 2, _710Rpro3Series, 1, 1, 0);

			break;

		case 2:
			if (_field4C)
				kernel_timing_trigger(5, 3, nullptr);
			else {
				_ripPullMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 3845, false, triggerMachineByHashCallback, "rip pull machine");
				sendWSMessage_10000(1, _ripPullMach, _mazeCentreDoorLiteSeries, 1, 10, 3, _mazeCentreDoorLiteSeries, 10, 10, 0);
				_field4C = 1;
			}

			break;

		case 3:
			terminateMachine(_ripPullMach02);
			_710Rprs3Mach = series_place_sprite("710RPRS3", 0, 0, -53, 100, 2565);
			terminateMachine(_safariShadow1Mach);
			ws_unhide_walker(_G(my_walker));

			player_set_commands_allowed(true);

			break;

		default:
			break;
		}
	} // esi && player_said("rope   ")

	else if (esi && player_said("rope     ")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			terminateMachine(_710Rprs1Mach);
			_ripPullMach02 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 512, false, triggerMachineByHashCallback, "rip pull machine");
			player_update_info(_G(my_walker), &_G(player_info));
			_safariShadow1Mach = series_place_sprite("SAFARI SHADOW 1", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3840);
			ws_hide_walker(_G(my_walker));
			sendWSMessage_10000(1, _ripPullMach02, _710Rpro1Series, 1, 15, 1, _710Rpro1Series, 15, 15, 0);

			break;

		case 1:
			digi_play("950_s42", 2, 255, -1, -1);
			sendWSMessage_10000(1, _ripPullMach02, _710Rpro1Series, 15, 38, 3, _710Rpro1Series, 1, 1, 0);

			break;

		case 3:
			terminateMachine(_ripPullMach02);
			_710Rprs1Mach = series_place_sprite("710RPRS1", 0, 0, -53, 100, 517);
			terminateMachine(_safariShadow1Mach);
			ws_unhide_walker(_G(my_walker));

			player_set_commands_allowed(true);

			break;

		default:
			break;
		}
	} // esi && player_said("rope     ")


	// TODO Not implemented yet

	_G(player).command_ready = false;
}

void Room710::daemon() {
	switch (_G(kernel).trigger) {
	case 30:
		digi_play("710_s02", 2, 200, -1, -1);
		sendWSMessage_10000(1, _ripContraptionMach, _710Rpld2Series, 82, 63, 31, _710Rpld2Series, 63, 63, 0);

		break;

	case 31:
		digi_play("710_s02", 2, 200, -1, -1);
		sendWSMessage_10000(1, _ripContraptionMach, _710Rpld2Series, 62, 43, 32, _710Rpld2Series, 43, 43, 0);

		break;

	case 32:
		digi_play("710_s02", 2, 200, -1, -1);
		sendWSMessage_10000(1, _ripContraptionMach, _710Rpld2Series, 42, 28, 33, _710Rpld2Series, 28, 28, 0);

		break;

	case 33:
		sendWSMessage_10000(1, _ripContraptionMach, _710Rpld2Series, 27, 1, 34, _710Rpld2Series, 1, 1, 0);
		break;

	case 34:
		terminateMachine(_ripContraptionMach);
		ws_unhide_walker(_G(my_walker));
		player_set_commands_allowed(true);
		_ripContraptionMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 1024, false, triggerMachineByHashCallback, "rip contraption machine");

		break;

	default:
		break;
	}

}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
