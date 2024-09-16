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

#include "m4/riddle/rooms/section5/room509.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room509::init() {
	_roomNum = -1;
	_ripDigsWall = series_load("RIP DIGS AT WALL");
	_ripFrom506 = series_load("RIP CLIMBS STEPS FROM 506");
	_ripFrom510 = series_load("RIP DOWN FROM 510");
	_ripClimbStairs1 = series_load("RIP CLIMBS STAIRS 1");
	_ripClimbStairs3 = series_load("RIP CLIMBS STAIRS 3");
	_ripMedHighReacher = series_load("RIP MED HIGH REACHER POS2");
	digi_play_loop("509_s01", 3, 140);

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		_flag1 = false;
		break;

	case 510:
		player_set_commands_allowed(false);
		ws_demand_location(289, 261, 6);
		kernel_timing_trigger(1, 3);
		break;

	default:
		player_set_commands_allowed(false);
		ws_demand_location(342, 320, 2);
		kernel_timing_trigger(1, 2);
		_flag1 = !player_been_here(509);
		break;
	}
}

void Room509::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		player_set_commands_allowed(true);
		break;

	case 2:
		ws_hide_walker();
		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
			triggerMachineByHashCallbackNegative, "Rip climbs steps from 506");
		sendWSMessage_10000(1, _ripley, _ripFrom506, 1, 39, 6,
			_ripFrom506, 40, 40, 0);
		break;

	case 3:
		ws_hide_walker();
		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
			triggerMachineByHashCallbackNegative, "Rip climbs steps from 506");
		sendWSMessage_10000(1, _ripley, _ripFrom510, 1, 15, 7,
			_ripFrom510, 16, 16, 0);
		break;

	case 5:
		digi_play("509_s02", 1);
		break;

	case 6:
		terminateMachineAndNull(_ripley);
		ws_unhide_walker();

		if (_flag1)
			digi_play("509R01", 1, 255, 1);
		else
			player_set_commands_allowed(true);
		break;

	case 7:
		terminateMachineAndNull(_ripley);
		ws_unhide_walker();
		player_set_commands_allowed(true);
		break;

	case 8:
		sendWSMessage_10000(1, _ripley, _ripDigsWall, 34, 2, 7,
			_ripDigsWall, 1, 1, 0);
		break;

	case 9:
		adv_kill_digi_between_rooms(false);
		digi_play_loop("509_s01", 3, 170);

		switch (_roomNum) {
		case 506:
		case 510:
			_G(game).setRoom(_roomNum);
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}
}

void Room509::pre_parser() {
	if (player_said("gear", "hutch"))
		_G(player).resetWalk();
}

void Room509::parser() {
	bool lookFlag = player_said_any("look", "look at");
	bool takeFlag = player_said("take");
	bool useFlag = player_said("gear");

	if (lookFlag && player_said("wall")) {
		digi_play("509R03", 1);
	} else if (lookFlag && player_said("hutch")) {
		digi_play(inv_player_has("CRYSTAL SKULL") ? "509R04A" : "509R04", 1);
	} else if (lookFlag && player_said("stairs up")) {
		digi_play("509R05", 1);
	} else if (lookFlag && player_said("stairs down")) {
		digi_play("509R06", 1);
	} else if (lookFlag && player_said(" ")) {
		digi_play("509R02", 1);
	} else if (lookFlag && player_said("  ")) {
		digi_play(_G(player).been_here_before ? "506r01" : "506r01a", 1, 255, -1, 506);
	} else if (useFlag && player_said("hutch")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			ws_walk(313, 271, nullptr, 1, 2);
			break;
		case 1:
			setGlobals1(_ripMedHighReacher, 1, 9, 9, 9, 0, 9, 10, 10, 10, 0,
				10, 9, 9, 9, 0, 9, 1, 1, 1);
			sendWSMessage_110000(2);
			break;
		case 2:
			sendWSMessage_120000(3);
			break;
		case 3:
			sendWSMessage_130000(4);
			break;
		case 4:
			sendWSMessage_120000(5);
			break;
		case 5:
			sendWSMessage_130000(6);
			break;
		case 6:
			sendWSMessage_180000(7);
			digi_play("509R07", 1, 255, 5);
			break;
		case 7:
			sendWSMessage_150000(-1);
			player_set_commands_allowed(true);
			break;
		default:
			break;
		}
	} else if (player_said("SHOVEL", "hutch")) {
		player_set_commands_allowed(false);
		ws_hide_walker();
		digi_preload("509_s02");
		_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
			triggerMachineByHashCallbackNegative, "RIP DIGS AT WALL");
		_G(kernel).trigger_mode = KT_DAEMON;
		sendWSMessage_10000(1, _ripley, _ripDigsWall, 1, 33, 8,
			_ripDigsWall, 34, 34, 0);
		kernel_timing_trigger(120, 5);
	} else if (!lookFlag && !takeFlag && !useFlag &&
			!player_said("SHOVEL", "hutch") && player_said("hutch")) {
		digi_play("509R08", 1);
	} else if (player_said("walk to", "stairs up")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			ws_hide_walker();
			_roomNum = 510;
			_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
				triggerMachineByHashCallbackNegative, "RIP CLIMBS UP TO 510");
			sendWSMessage_10000(1, _ripley, _ripClimbStairs3, 1, 8, 1,
				_ripClimbStairs3, 9, 16, 0);
			break;

		case 1:
			_G(kernel).trigger_mode = KT_DAEMON;
			disable_player_commands_and_fade_init(9);
			break;

		default:
			break;
		}
	} else if (player_said("walk to", "stairs down")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			ws_hide_walker();
			_roomNum = 506;
			_ripley = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x400, 0,
				triggerMachineByHashCallbackNegative, "RIP CLIMBS DOWN TO 506");
			sendWSMessage_10000(1, _ripley, _ripClimbStairs1, 1, 14, 1,
				_ripClimbStairs1, 15, 28, 0);
			break;

		case 1:
			_G(kernel).trigger_mode = KT_DAEMON;
			disable_player_commands_and_fade_init(9);
			break;

		default:
			break;
		}
	} else {
		return;
	}

	_G(player).command_ready = false;
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
