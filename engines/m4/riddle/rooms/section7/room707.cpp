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

#include "m4/riddle/rooms/section7/room707.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_vmng_screen.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room707::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
}

void Room707::init() {
	ws_demand_location(_G(my_walker), 299, 200);
	ws_demand_facing(_G(my_walker), 4);
	if (!player_been_here(707)) {
		_field44 = 0;
	}

	_peerIntoCupolaPos2Series = series_load("PEER INTO CUPOLA POS2", -1, nullptr);
	_peerIntoCupolaPos1Series = series_load("PEER INTO CUPOLA POS1", -1, nullptr);

	switch (_G(game).previous_room) {
	case 703:
		ws_demand_location(_G(my_walker), 421, 150);
		ws_demand_facing(_G(my_walker), 4);
		player_set_commands_allowed(false);
		ws_hide_walker(_G(my_walker));
		_ripWalksUpLeftStairsSeries = series_load("RIP WALKS UP LEFT STAIRS", -1, nullptr);
		_ripStairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, false, triggerMachineByHashCallback, "rip stairs machine");
		sendWSMessage_10000(1, _ripStairsMach, _ripWalksUpLeftStairsSeries, 1, 22, 11, _ripWalksUpLeftStairsSeries, 22, 22, 0);

		break;

	case 704:
		ws_demand_location(_G(my_walker), 374, 252);
		ws_demand_facing(_G(my_walker), 2);
		_ripUpStairsPos2Series = series_load("RIP UP STAIRS POSITION 2", -1, nullptr);
		player_set_commands_allowed(false);
		ws_hide_walker(_G(my_walker));
		_ripStairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, false, triggerMachineByHashCallback, "rip stairs machine");
		sendWSMessage_10000(1, _ripStairsMach, _ripUpStairsPos2Series, 1, 37, 11, _ripUpStairsPos2Series, 37, 37, 0);

		break;

	case 705:
		MoveScreenDelta(_G(game_buff_ptr), -640, 0);
		ws_demand_location(_G(my_walker), 930, 248);
		ws_demand_facing(_G(my_walker), 10);
		player_set_commands_allowed(false);
		ws_hide_walker(_G(my_walker));
		_ripUpStairsPos10Series = series_load("RIP UP STAIRS POSITION 10", -1, nullptr);
		_ripStairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, false, triggerMachineByHashCallback, "rip stairs machine");
		sendWSMessage_10000(1, _ripStairsMach, _ripUpStairsPos10Series, 1, 37, 11, _ripUpStairsPos10Series, 37, 37, 0);

		break;

	case 706:
		MoveScreenDelta(_G(game_buff_ptr), -640, 0);
		ws_demand_location(_G(my_walker), 860, 149);
		ws_demand_facing(_G(my_walker), 8);
		player_set_commands_allowed(false);
		ws_hide_walker(_G(my_walker));
		_ripWalksUpFarStairsSeries = series_load("RIP WALKS UP FAR STAIRS", -1, nullptr);
		_ripStairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, false, triggerMachineByHashCallback, "rip stairs machine");
		sendWSMessage_10000(1, _ripStairsMach, _ripWalksUpFarStairsSeries, 1, 22, 11, _ripWalksUpFarStairsSeries, 22, 22, 0);

		break;

	default:
		digi_preload("950_s39", -1);
		break;
	}

	digi_play_loop("950_s39", 3, 255, -1, -1);
}

void Room707::pre_parser() {
	// No implementation
}

void Room707::parser() {
	const bool lookFl = player_said_any("look", "look at");

	if (lookFl && player_said("VOTIVE STUPA"))
		digi_play("707r03", 1, 255, -1, -1);
	else if (lookFl && player_said("WINDOW")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			setGlobals1(_peerIntoCupolaPos2Series, 1, 13, 13, 13, 0, 13, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), 2);

			break;

		case 2:
			_popUpMach = series_place_sprite("707 LEFT SLIT POPUP", 0, 0, 0, 100, 256);
			kernel_timing_trigger(100, 3, nullptr);

			break;

		case 3:
			digi_play("707r05", 1, 255, 4, -1);
			break;

		case 4:
			terminateMachine(_popUpMach);
			sendWSMessage_120000(_G(my_walker), 5);
			break;

		case 5:
			sendWSMessage_150000(_G(my_walker), 6);
			break;

		case 6:
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
	} // lookFl && player_said("WINDOW")

	else if (lookFl && player_said("WINDOW ")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			setGlobals1(_peerIntoCupolaPos2Series, 1, 13, 13, 13, 0, 13, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
			sendWSMessage_110000(_G(my_walker), 2);

			break;

		case 2:
			_popUpMach = series_place_sprite("707 RIGHT SLIT POPUP", 0, 0, 0, 100, 256);
			kernel_timing_trigger(100, 3, nullptr);

			break;

		case 3:
			digi_play("707r05", 1, 255, 4, -1);
			break;

		case 4:
			terminateMachine(_popUpMach);
			sendWSMessage_120000(_G(my_walker), 5);
			break;

		case 5:
			sendWSMessage_150000(_G(my_walker), 6);
			break;

		case 6:
			player_set_commands_allowed(true);
			break;

		default:
			break;
		}
	} // lookFl && player_said("WINDOW ")

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
				break;
			}

			break;

		case 2:
			digi_play(_field44 ? "707r04" : "707r04a", 1, 255, -1, -1);
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

	else if (player_said("RM704")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			ws_hide_walker(_G(my_walker));
			_ripDownStairsPos8Series = series_load("RIP DOWN STAIRS POSITION 8", -1, nullptr);
			_ripStairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, false, triggerMachineByHashCallback, "rip stairs machine");
			sendWSMessage_10000(1, _ripStairsMach, _ripDownStairsPos8Series, 4, 27, 2, _ripDownStairsPos8Series, 27, 27, 0);

			break;

		case 2:
			sendWSMessage_10000(1, _ripStairsMach, _ripDownStairsPos8Series, 28, 37, -1, _ripDownStairsPos8Series, 37, 37, 0);
			disable_player_commands_and_fade_init(3);

			break;

		case 3:
			terminateMachine(_ripStairsMach);
			player_set_commands_allowed(true);
			adv_kill_digi_between_rooms(false);
			digi_play_loop("950_s39", 3, 255, -1, -1);
			_G(game).new_room = 704;

			break;

		default:
			break;
		}
	} // player_said("RM704")

	else if (player_said("RM705")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			ws_hide_walker(_G(my_walker));
			_ripDownStairsPos4Series = series_load("RIP DOWN STAIRS POSITION 4", -1, nullptr);
			_ripStairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, false, triggerMachineByHashCallback, "rip stairs machine going to rm704");
			sendWSMessage_10000(1, _ripStairsMach, _ripDownStairsPos4Series, 4, 27, 2, _ripDownStairsPos4Series, 27, 27, 0);

			break;

		case 2:
			sendWSMessage_10000(1, _ripStairsMach, _ripDownStairsPos4Series, 28, 37, -1, _ripDownStairsPos4Series, 37, 37, 0);
			disable_player_commands_and_fade_init(3);

			break;

		case 3:
			terminateMachine(_ripStairsMach);
			player_set_commands_allowed(true);
			adv_kill_digi_between_rooms(false);
			digi_play_loop("950_s39", 3, 255, -1, -1);
			_G(game).new_room = 705;

			break;

		default:
			break;
		}
	} // player_said("RM705")

	else if (player_said("RM703")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			ws_hide_walker(_G(my_walker));
			_ripWalksDownLeftStairsSeries = series_load("RIP WALKS DOWN LEFT STAIRS", -1, nullptr);
			_ripStairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, false, triggerMachineByHashCallback, "rip stairs machine going to rm704");
			sendWSMessage_10000(1, _ripStairsMach, _ripWalksDownLeftStairsSeries, 1, 14, 2, _ripWalksDownLeftStairsSeries, 14, 14, 0);

			break;

		case 2:
			sendWSMessage_10000(1, _ripStairsMach, _ripWalksDownLeftStairsSeries, 15, 19, -1, _ripWalksDownLeftStairsSeries, 19, 19, 0);
			disable_player_commands_and_fade_init(3);

			break;

		case 3:
			terminateMachine(_ripStairsMach);
			player_set_commands_allowed(true);
			adv_kill_digi_between_rooms(false);
			digi_play_loop("950_s39", 3, 255, -1, -1);
			_G(game).new_room = 703;

			break;

		default:
			break;
		}
	} // player_said("RM703")

	else if (player_said("RM706")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			ws_hide_walker(_G(my_walker));
			_ripWalksDownFarStairsSeries = series_load("RIP WALKS DOWN FAR STAIRS", -1, nullptr);
			_ripStairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, false, triggerMachineByHashCallback, "rip stairs machine going to rm704");
			sendWSMessage_10000(1, _ripStairsMach, _ripWalksDownFarStairsSeries, 1, 14, 2, _ripWalksDownFarStairsSeries, 14, 14, 0);

			break;

		case 2:
			sendWSMessage_10000(1, _ripStairsMach, _ripWalksDownFarStairsSeries, 15, 19, -1, _ripWalksDownFarStairsSeries, 19, 19, 0);
			disable_player_commands_and_fade_init(3);

			break;

		case 3:
			terminateMachine(_ripStairsMach);
			player_set_commands_allowed(true);
			adv_kill_digi_between_rooms(false);
			digi_play_loop("950_s39", 3, 255, -1, -1);
			_G(game).new_room = 706;

			break;

		default:
			break;
		}
	} // player_said("RM706")

	else if (lookFl && player_said(" "))
		digi_play("707r01", 1, 255, -1, -1);
	else if (player_said("GRATE"))
		digi_play("707r02", 1, 255, -1, -1);
	else
		return;

	_G(player).command_ready = false;
}

void Room707::daemon() {
	if (_G(kernel).trigger == 11) {
		player_set_commands_allowed(true);
		ws_unhide_walker(_G(my_walker));
		terminateMachine(_ripStairsMach);
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
