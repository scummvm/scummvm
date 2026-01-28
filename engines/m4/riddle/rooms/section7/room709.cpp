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

#include "m4/riddle/rooms/section7/room709.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_sys.h"
#include "m4/riddle/vars.h"
#include "m4/platform/keys.h"
#include "m4/platform/timer.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

const Maze709Room Room709::_rooms[100] = {
	{  0,  0,  0,  0 },
	{  0,  2,  0, 12 },
	{  1,  0,  0, 13 },
	{  0,  4,  0, 14 },
	{  3,  5,  0, 15 },
	{  4,  6,  0,  0 },
	{  5,  7,  0, 17 },
	{  6,  8,  0,  0 },
	{  7,  9,  0,  0 },
	{  8,  0,  0, 20 },

	{  0, 11,  0, 21 },
	{ 10,  0,  0, 22 },
	{  0,  0,  1,  0 },
	{  0,  0,  2, 24 },
	{  0,  0,  3, 25 },
	{  0, 16,  4,  0 },
	{ 15,  0,  0,  0 },
	{  0,  0,  6,  0 },
	{  0, 19,  0, 29 },
	{ 18, 20,  0,  0 },

	{ 19, 21,  9,  0 },
	{ 20,  0, 10,  0 },
	{  0,  0, 11,  0 },
	{  0, 24,  0, 34 },
	{ 23,  0, 13,  0 },
	{  0,  0, 14, 36 },
	{ 25,  0,  0, 37 },
	{  0, 28,  0, 38 },
	{ 27,  0,  0,  0 },
	{  0,  0, 18, 40 },

	{  0, 31,  0, 41 },
	{ 30, 32,  0, 42 },
	{ 31, 33,  0,  0 },
	{ 32,  0,  0, 44 },
	{  0,  0, 23, 45 },
	{  0, 36,  0,  0 },
	{ 35,  0, 25, 47 },
	{  0, 38, 26,  0 },
	{ 37,  0, 27,  0 },
	{  0, 40,  0, 50 },

	{ 39,  0, 29,  0 },
	{  0,  0, 30, 52 },
	{  0, 43, 31, 53 },
	{ 42, 44,  0,  0 },
	{ 43,  0, 33,  0 },
	{  0, 46, 34, 56 },
	{ 45, 47,  0,  0 },
	{ 46,  0, 36,  0 },
	{  0, 49,  0, 59 },
	{ 48,  0,  0,  0 },

	{  0,  0, 39,  0 },
	{  0, 52,  0, 62 },
	{ 51,  0, 41,  0 },
	{  0, 54, 42,  0 },
	{ 53, 55,  0,  0 },
	{ 54,  0,  0, 66 },
	{  0,  0, 45, 67 },
	{ 56,  0,  0,  0 },
	{  0, 59,  0, 69 },
	{ 58,  0, 48, 70 },

	{  0, 61,  0, 71 },
	{ 60, 62,  0,  0 },
	{ 61,  0, 51,  0 },
	{  0, 64,  0,  0 },
	{ 63, 65,  0, 75 },
	{ 64, 66,  0,  0 },
	{ 65,  0, 55,  0 },
	{  0,  0, 56, 78 },
	{  0, 69,  0, 79 },
	{ 68,  0, 58,  0 },

	{  0, 71, 59,  0 },
	{ 70,  0, 60,  0 },
	{  0, 73,  0, 83 },
	{ 72, 74,  0,  0 },
	{ 73,  0,  0, 85 },
	{  0, 76, 64,  0 },
	{ 75, 77,  0,  0 },
	{ 76,  0,  0, 88 },
	{  0,  0, 67, 89 },
	{  0, 80, 68,  0 },

	{ 79, 81,  0, 91 },
	{ 80,  0,  0, 92 },
	{  0, 83,  0,  0 },
	{ 82,  0, 72, 94 },
	{  0, 85,  0, 95 },
	{ 84, 86, 74,  0 },
	{ 85, 87,  0,  0 },
	{ 86,  0,  0, 98 },
	{  0,  0, 77, 99 },
	{  0, 90, 78,  0 },

	{ 89, 91,  0,  0 },
	{ 90,  0, 80,  0 },
	{  0, 93, 81,  0 },
	{ 92,  0,  0,  0 },
	{  0,  0, 83,  0 },
	{  0, 96, 84,  0 },
	{ 95, 97,  0,  0 },
	{ 96,  0,  0,  0 },
	{  0, 99, 87,  0 },
	{ 98,  0, 88,  0 }
};

Room709::Room709() : Room() {
}

void Room709::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
}

void Room709::syncGame(Common::Serializer &s) {
	s.syncAsSint32LE(_mazeCurrentIndex);
	s.syncAsByte(_chiselActiveFl);
	s.syncAsByte(_incenseBurnerActiveFl);
	s.syncAsByte(_pullCenterFl);
	s.syncAsByte(_pullLeftFl);
	s.syncAsByte(_pullNearFl);
	s.syncAsByte(_pullRightFl);
}

void Room709::init() {
	AddSystemHotkey(KEY_CLEAR, clearPressed);
	AddSystemHotkey(5, clearPressed);
	digi_preload("950_s42");

	_pullLeftFl = false;
	_pullRightFl = false;
	_pullCenterFl = false;
	_pullNearFl = false;

	_ripTrekLowReachPos2Series = series_load("RIP TREK LOW REACH POS2", -1, nullptr);
	_709rpro1Series = series_load("709RPRO1", -1, nullptr);
	_709rpro2Series = series_load("709RPRO2", -1, nullptr);
	_709rpro3Series = series_load("709RPRO3", -1, nullptr);
	_709rpro4Series = series_load("709RPRO4", -1, nullptr);
	_mazeLeftDoorLiteSeries = series_load("MAZE LEFT DOOR LITE", -1, nullptr);
	_mazeCenterDoorLiteSeries = series_load("MAZE CENTRE DOOR LITE", -1, nullptr);
	_mazeRightDoorLiteSeries = series_load("MAZE RIGHT DOOR LITE", -1, nullptr);
	_709NearDoorLiteSeries = series_load("709 NEAR DOOR LIGHT", -1, nullptr);

	_709rpro8Mach = series_place_sprite("709RPRO8", 0, 0, -53, 100, 2048);
	_709rpro7Mach = series_place_sprite("709RPRO7", 0, 0, -53, 100, 2304);
	_709rpro6Mach = series_place_sprite("709RPRO6", 0, 0, -53, 100, 1792);
	_709rpro5Mach = series_place_sprite("709RPRO5", 0, 0, -53, 100, 512);

	_chiselActiveFl = false;
	_incenseBurnerActiveFl = false;

	hotspot_set_active(_G(currentSceneDef).hotspots, "Incense Burner", false);
	hotspot_set_active(_G(currentSceneDef).hotspots, "Chisel", false);
	
	_G(room303_btnFlag) = true;
	_G(global301) = 88;
	_G(flag1) = 1;
	_G(204_dword1A1898) = 98;
	
	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		digi_preload("950_s41", -1);
		if (_mazeCurrentIndex == 97 && inv_object_is_here("CHISEL")) {
			_chiselActiveFl = true;
			_709ChiselMach = series_place_sprite("709 CHISEL", 0, 0, 0, 100, 3840);
			hotspot_set_active(_G(currentSceneDef).hotspots, "Chisel", true);
		}

		if (_mazeCurrentIndex == 22 && inv_object_is_here("INCENSE BURNER")) {
			_incenseBurnerActiveFl = true;
			_709IncenseHolderMach = series_place_sprite("709 INCENSE HOLDER", 0, 0, 0, 100, 3840);
			hotspot_set_active(_G(currentSceneDef).hotspots, "Incense Burner", true);
		}

		if (_mazeCurrentIndex == 82) {
			_ripPullMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 3840, false, triggerMachineByHashCallback, "rip pull machine");
			sendWSMessage_10000(1, _ripPullMach, _mazeLeftDoorLiteSeries, 10, 10, -1, _mazeLeftDoorLiteSeries, 10, 10, 0);
			_pullLeftFl = true;
		}

		break;

	case 706:
		digi_preload("950_s41", -1);
		_mazeCurrentIndex = 82;
		debugRoomChanged();

		_ripPullMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 3840, false, triggerMachineByHashCallback, "rip pull machine");
		sendWSMessage_10000(1, _ripPullMach, _mazeLeftDoorLiteSeries, 10, 10, -1, _mazeLeftDoorLiteSeries, 10, 10, 0);
		_pullLeftFl = true;
		ws_demand_location(_G(my_walker), 186, 290);
		ws_demand_facing(_G(my_walker), 2);
		ws_walk(_G(my_walker), 276, 300, nullptr, -1, 0, true);
		break;

	case 710:
		_mazeCurrentIndex = 39;
		debugRoomChanged();

		ws_demand_location(_G(my_walker), 320, 400);
		ws_demand_facing(_G(my_walker), 1);
		break;

	default:
		_mazeCurrentIndex = 82;
		debugRoomChanged();

		digi_preload("950_s41", -1);
		break;
	}

	debugRoomChanged();
	digi_play_loop("950_s41", 3, 255, -1, -1);
}

void Room709::pre_parser() {
	const bool lookFl = player_said_any("look", "look at");
	const bool talkFl = player_said_any("talk", "talk to", "take");
	const bool gearFl = player_said_any("push", "pull", "gear", "open", "close", "RopeS", "RopeL", "RopeR", "RopeB");

	if (player_said("Right")) {
		if (!_rooms[_mazeCurrentIndex]._right || !_pullRightFl || lookFl || talkFl || gearFl) {
			_G(player).need_to_walk = false;
			_G(player).ready_to_walk = true;
			_G(player).waiting_for_walk = false;
		}
	}

	if (player_said("Left")) {
		if (!_rooms[_mazeCurrentIndex]._left || !_pullLeftFl || lookFl || talkFl || gearFl) {
			_G(player).need_to_walk = false;
			_G(player).ready_to_walk = true;
			_G(player).waiting_for_walk = false;
		}

		if (_mazeCurrentIndex == 82) {
			_G(player).need_to_walk = false;
			_G(player).ready_to_walk = true;
			_G(player).waiting_for_walk = false;
		}
	}

	if (player_said("Straight")) {
		if (!_rooms[_mazeCurrentIndex]._up || !_pullCenterFl || lookFl || talkFl || gearFl) {
			_G(player).need_to_walk = false;
			_G(player).ready_to_walk = true;
			_G(player).waiting_for_walk = false;
		}
	}

	if (player_said("Back")) {
		if (!_rooms[_mazeCurrentIndex]._down || !_pullNearFl || lookFl || talkFl || gearFl) {
			_G(player).need_to_walk = false;
			_G(player).ready_to_walk = true;
			_G(player).waiting_for_walk = false;
		}
	}

	if ((lookFl || talkFl || gearFl) && player_said_any(" ", "  ")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (talkFl && player_said("Incense Burner") && inv_object_is_here("Incense Burner")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (talkFl && player_said("Chisel") && inv_object_is_here("Chisel")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (!lookFl && !talkFl && player_said_any("LIGHTER", "LIT LIGHTER")) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}
}

void Room709::parser() {
	const bool lookFl = player_said_any("look", "look at");
	const bool talkFl = player_said_any("talk", "talk to", "take");
	const bool gearFl = player_said_any("push", "pull", "gear", "open", "close", "RopeS", "RopeL", "RopeR", "RopeB");

	if (gearFl && player_said("rope   ")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			terminateMachine(_709rpro8Mach);
			_ripPullMach02 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 2048, false, triggerMachineByHashCallback, "rip pull machine");
			player_update_info(_G(my_walker), &_G(player_info));
			_safariShadow1Mach = series_place_sprite("SAFARI SHADOW 1", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3840);
			ws_hide_walker(_G(my_walker));
			sendWSMessage_10000(1, _ripPullMach02, _709rpro4Series, 1, 16, 1, _709rpro4Series, 16, 16, 0);
			sendWSMessage_190000(_ripPullMach02, 4);

			break;

		case 1:
			digi_play("950_s42", 2, 255, -1, -1);
			sendWSMessage_10000(1, _ripPullMach02, _709rpro4Series, 17, 38, 2, _709rpro4Series, 1, 1, 0);
			sendWSMessage_190000(_ripPullMach02, 4);

			break;

		case 2:
			if (_rooms[_mazeCurrentIndex]._right && !_pullRightFl) {
				_ripPullMach03 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 3840, false, triggerMachineByHashCallback, "rip pull machine");
				sendWSMessage_10000(1, _ripPullMach03, _mazeRightDoorLiteSeries, 1, 10, 3, _mazeRightDoorLiteSeries, 10, 10, 0);
				_pullRightFl = true;
			} else {
				kernel_timing_trigger(5, 3, nullptr);
			}

			break;

		case 3:
			terminateMachine(_ripPullMach02);
			_709rpro8Mach = series_place_sprite("709RPRO8", 0, 0, -53, 100, 2048);
			terminateMachine(_safariShadow1Mach);
			ws_unhide_walker(_G(my_walker));
			player_set_commands_allowed(true);

			break;

		default:
			break;
		}
	} // esi && player_said("rope   ")

	else if (gearFl && player_said("rope     ")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			terminateMachine(_709rpro6Mach);
			_ripPullMach02 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 1792, false, triggerMachineByHashCallback, "rip pull machine");
			player_update_info(_G(my_walker), &_G(player_info));
			_safariShadow1Mach = series_place_sprite("SAFARI SHADOW 1", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3840);
			ws_hide_walker(_G(my_walker));
			sendWSMessage_10000(1, _ripPullMach02, _709rpro2Series, 1, 15, 1, _709rpro2Series, 15, 15, 0);
			sendWSMessage_190000(_ripPullMach02, 4);

			break;

		case 1:
			digi_play("950_s42", 2, 255, -1, -1);
			sendWSMessage_10000(1, _ripPullMach02, _709rpro2Series, 15, 38, 2, _709rpro2Series, 1, 1, 0);
			sendWSMessage_190000(_ripPullMach02, 4);

			break;

		case 2:
			if (_rooms[_mazeCurrentIndex]._left && !_pullLeftFl) {
				_ripPullMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 3840, false, triggerMachineByHashCallback, "rip pull machine");
				sendWSMessage_10000(1, _ripPullMach, _mazeLeftDoorLiteSeries, 1, 10, 3, _mazeLeftDoorLiteSeries, 10, 10, 0);
				_pullLeftFl = true;
			} else {
				kernel_timing_trigger(5, 3, nullptr);
			}

			break;

		case 3:
			terminateMachine(_ripPullMach02);
			_709rpro6Mach = series_place_sprite("709RPRO6", 0, 0, -53, 100, 1792);
			terminateMachine(_safariShadow1Mach);
			ws_unhide_walker(_G(my_walker));
			player_set_commands_allowed(true);

			break;

		default:
			break;
		}
	} // esi && player_said("rope     ")

	else if (gearFl && player_said("rope  ")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			terminateMachine(_709rpro7Mach);
			_ripPullMach02 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 2304, false, triggerMachineByHashCallback, "rip pull machine");
			player_update_info(_G(my_walker), &_G(player_info));
			_safariShadow1Mach = series_place_sprite("SAFARI SHADOW 1", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3840);
			ws_hide_walker(_G(my_walker));
			sendWSMessage_10000(1, _ripPullMach02, _709rpro3Series, 1, 16, 1, _709rpro3Series, 16, 16, 0);
			sendWSMessage_190000(_ripPullMach02, 4);

			break;

		case 1:
			digi_play("950_s42", 2, 255, -1, -1);
			sendWSMessage_10000(1, _ripPullMach02, _709rpro3Series, 16, 38, 2, _709rpro3Series, 1, 1, 0);
			sendWSMessage_190000(_ripPullMach02, 4);

			break;

		case 2:
			if (_rooms[_mazeCurrentIndex]._up && !_pullCenterFl) {
				_ripPullMach04 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 3840, false, triggerMachineByHashCallback, "rip pull machine");
				sendWSMessage_10000(1, _ripPullMach04, _mazeCenterDoorLiteSeries, 1, 10, 3, _mazeCenterDoorLiteSeries, 10, 10, 0);
				_pullCenterFl = true;
			} else {
				kernel_timing_trigger(5, 3, nullptr);
			}

			break;

		case 3:
			terminateMachine(_ripPullMach02);
			_709rpro7Mach = series_place_sprite("709RPRO7", 0, 0, -53, 100, 2304);
			terminateMachine(_safariShadow1Mach);
			ws_unhide_walker(_G(my_walker));
			player_set_commands_allowed(true);

			break;

		default:
			break;

		}
	} // esi && player_said("rope  ")

	else if (gearFl && player_said("rope    ")) {
		switch (_G(kernel).trigger) {
		case -1:
			player_set_commands_allowed(false);
			terminateMachine(_709rpro5Mach);
			_ripPullMach02 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 512, false, triggerMachineByHashCallback, "rip pull machine");
			player_update_info(_G(my_walker), &_G(player_info));
			_safariShadow1Mach = series_place_sprite("SAFARI SHADOW 1", 0, _G(player_info).x, _G(player_info).y, _G(player_info).scale, 3840);
			ws_hide_walker(_G(my_walker));
			sendWSMessage_10000(1, _ripPullMach02, _709rpro1Series, 1, 17, 1, _709rpro1Series, 17, 17, 0);
			sendWSMessage_190000(_ripPullMach02, 4);

			break;

		case 1:
			digi_play("950_s42", 2, 255, -1, -1);
			sendWSMessage_10000(1, _ripPullMach02, _709rpro1Series, 17, 38, 2, _709rpro1Series, 1, 1, 0);
			sendWSMessage_190000(_ripPullMach02, 4);

			break;

		case 2:
			if (_rooms[_mazeCurrentIndex]._down && !_pullNearFl) {
				_ripPullMach05 = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 3840, false, triggerMachineByHashCallback, "rip pull machine");
				sendWSMessage_10000(1, _ripPullMach05, _709NearDoorLiteSeries, 1, 10, 3, _709NearDoorLiteSeries, 10, 10, 0);
				_pullNearFl = true;
			} else {
				kernel_timing_trigger(5, 3, nullptr);
			}

			break;

		case 3:
			terminateMachine(_ripPullMach02);
			_709rpro5Mach = series_place_sprite("709RPRO5", 0, 0, -53, 100, 512);
			terminateMachine(_safariShadow1Mach);
			ws_unhide_walker(_G(my_walker));
			player_set_commands_allowed(true);

			break;

		default:
			break;
		}
	} // esi && player_said("rope    ")

	else if (player_said("Right")) {
		if (!_rooms[_mazeCurrentIndex]._right || !_pullRightFl) {
			digi_play("708R01", 1, 255, -1, -1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				disable_player_commands_and_fade_init(2);
				kernel_timing_trigger(30, 2, nullptr);

				break;

			case 2:
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_trigger_dispatchx(kernel_trigger_create(30));
				_G(kernel).trigger_mode = KT_PARSE;

				break;

			default:
				break;
			}
		}
	} // player_said("Right")

	else if (player_said("Left")) {
		if (!_rooms[_mazeCurrentIndex]._left || !_pullLeftFl) {
			if (_mazeCurrentIndex == 82) {
				switch (_G(kernel).trigger) {
				case -1:
					ws_walk(_G(my_walker), 182, 292, nullptr, 2, 9, true);
					break;

				case 2:
					disable_player_commands_and_fade_init(3);
					break;

				case 3:
					RemoveSystemHotkey(KEY_CLEAR);
					RemoveSystemHotkey(5);
					_G(game).setRoom(706);
					break;

				default:
					break;
				}
			} else {
				digi_play("708R01", 1, 255, -1, -1);
			}
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				disable_player_commands_and_fade_init(2);
				break;

			case 2:
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_trigger_dispatchx(kernel_trigger_create(20));
				_G(kernel).trigger_mode = KT_PARSE;

				break;

			default:
				break;
			}
		}

	} // player_said("Left")

	else if (player_said("Straight")) {
		if (!_rooms[_mazeCurrentIndex]._up || !_pullCenterFl) {
			digi_play("708R01", 1, 255, -1, -1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				disable_player_commands_and_fade_init(2);

				break;

			case 2:
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_trigger_dispatchx(kernel_trigger_create(40));
				_G(kernel).trigger_mode = KT_PARSE;

				break;

			default:
				break;
			}
		}
	} // player_said("Straight")

	else if (player_said("Back")) {
		if (!_rooms[_mazeCurrentIndex]._down || !_pullNearFl) {
			digi_play("708R01", 1, 255, -1, -1);
		} else {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				disable_player_commands_and_fade_init(2);

				break;

			case 2:
				_G(kernel).trigger_mode = KT_DAEMON;
				kernel_trigger_dispatchx(kernel_trigger_create(50));
				_G(kernel).trigger_mode = KT_PARSE;

				break;

			default:
				break;
			}
		}
	} // player_said("Back")

	else if (lookFl && player_said("Incense Burner") && inv_object_is_here("Incense Burner")) {
		digi_play("709R12", 1, 255, -1, -1);
	} else if (lookFl && player_said("Chisel") && inv_object_is_here("Chisel")) {
		digi_play("709R02", 1, 255, -1, -1);
	} else if (lookFl && player_said_any("rope  ", "rope   ", "rope    ", "rope     ")) {
		digi_play("com110", 1, 255, -1, 997);
	} else if (talkFl && player_said("Incense Burner") && inv_object_is_here("Incense Burner")) {
		switch (_G(kernel).trigger) {
		case -1:
			ws_walk(_G(my_walker), 373, 279, nullptr, 2, 2, true);
			break;

		case 2:
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_trigger_dispatchx(kernel_trigger_create(10));
			_G(kernel).trigger_mode = KT_PARSE;

			break;

		default:
			break;
		}
	} else if (talkFl && player_said("Chisel") && inv_object_is_here("chisel")) {
		switch (_G(kernel).trigger) {
		case -1:
			ws_walk(_G(my_walker), 267, 278, nullptr, 2, 10, true);
			break;

		case 2:
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_trigger_dispatchx(kernel_trigger_create(10));
			_G(kernel).trigger_mode = KT_PARSE;

			break;

		default:
			break;
		}
	} else if (!lookFl && player_said("journal") && !inv_player_has(_G(player).noun)) {
		digi_play("709R11", 1, 255, -1, -1);
	} else if (!lookFl && !talkFl && player_said_any("LIGHTER", "LIT LIGHTER")) {
		digi_play("com141", 1, 255, -1, 997);
	} else if (lookFl && player_said(" ")) {
		digi_play("709R01", 1, 255, -1, -1);
	} else if (lookFl && player_said("  ")) {
		digi_play("708R02", 1, 255, -1, 708);
	} else if (gearFl || talkFl) {
		switch (imath_ranged_rand(1, 5)) {
		case 1:
			digi_play("com006", 1, 255, -1, 997);
			break;

		case 2:
			digi_play("com008", 1, 255, -1, 997);
			break;

		case 3:
			digi_play("com013", 1, 255, -1, 997);
			break;

		case 4:
			digi_play("com010", 1, 255, -1, 997);
			break;

		case 5:
			digi_play("com011", 1, 255, -1, 997);
			break;

		default:
			break;
		}
	} else
		return;

	_G(player).command_ready = false;
}

void Room709::daemon() {
	int32 delay;

	switch (_G(kernel).trigger) {
	case 10:
		player_set_commands_allowed(false);
		setGlobals1(_ripTrekLowReachPos2Series, 1, 16, 16, 16, 0, 16, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 11);

		break;

	case 11:
		if (_mazeCurrentIndex == 97) {
			inv_give_to_player("CHISEL");
			terminateMachine(_709ChiselMach);
			_chiselActiveFl = false;
			hotspot_set_active(_G(currentSceneDef).hotspots, "Chisel", false);
			kernel_examine_inventory_object("PING CHISEL", _G(master_palette), 5, 1, 163, 203, -1, nullptr, -1);
			digi_play("709R03", 1, 255, 12, -1);
		}

		if (_mazeCurrentIndex == 22) {
			inv_give_to_player("INCENSE BURNER");
			terminateMachine(_709IncenseHolderMach);
			hotspot_set_active(_G(currentSceneDef).hotspots, "Incense Burner", false);
			_incenseBurnerActiveFl = false;
			kernel_examine_inventory_object("PING INCENSE BURNER", _G(master_palette), 5, 1, 329, 189, -1, nullptr, -1);
			digi_play("709R13", 1, 255, 12, -1);
		}

		break;

	case 12:
		sendWSMessage_120000(_G(my_walker), 13);
		break;

	case 13:
		kernel_timing_trigger(5, 14, nullptr);
		break;

	case 14:
		sendWSMessage_150000(_G(my_walker), 15);
		break;

	case 15:
		player_set_commands_allowed(true);
		break;

	case 20: // related to player_said("left")
		ws_demand_location(_G(my_walker), 410, 300);
		ws_demand_facing(_G(my_walker), 9);
		_mazeCurrentIndex = _rooms[_mazeCurrentIndex]._left;
		debugRoomChanged();

		if (_chiselActiveFl) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "Chisel", false);
			terminateMachine(_709ChiselMach);
		}

		if (_mazeCurrentIndex == 97 && inv_object_is_here("CHISEL")) {
			_chiselActiveFl = true;
			_709ChiselMach = series_place_sprite("709 CHISEL", 0, 0, 0, 100, 3840);
			hotspot_set_active(_G(currentSceneDef).hotspots, "Chisel", true);
		}

		if (_incenseBurnerActiveFl) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "Incense Burner", false);
			terminateMachine(_709IncenseHolderMach);
		}

		if (_mazeCurrentIndex == 22 && inv_object_is_here("INCENSE BURNER")) {
			_incenseBurnerActiveFl = true;
			_709IncenseHolderMach = series_place_sprite("709 INCENSE HOLDER", 0, 0, 0, 100, 3840);
			hotspot_set_active(_G(currentSceneDef).hotspots, "Incense Burner", true);
		}

		if (_pullRightFl) {
			terminateMachine(_ripPullMach03);
			_pullRightFl = false;
		}

		if (_pullLeftFl) {
			terminateMachine(_ripPullMach);
			_pullLeftFl = false;
		}

		if (_pullCenterFl) {
			terminateMachine(_ripPullMach04);
			_pullCenterFl = false;
		}

		if (_pullNearFl) {
			terminateMachine(_ripPullMach05);
			_pullNearFl = false;
		}

		if (_mazeCurrentIndex == 82) {
			_ripPullMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 3840, false, triggerMachineByHashCallback, "rip pull machine");
			sendWSMessage_10000(1, _ripPullMach, _mazeLeftDoorLiteSeries, 1, 10, -1, _mazeLeftDoorLiteSeries, 10, 10, 0);
			_pullLeftFl = true;
		}

		pal_fade_init(_G(master_palette), 0, 255, 100, 30, -1);
		player_set_commands_allowed(true);
		delay = imath_ranged_rand(1, 10);
		if (delay == 10) {
			kernel_timing_trigger(delay, 1000, nullptr);
		}
		break;

	case 30:
		ws_demand_location(_G(my_walker), 230, 300);
		ws_demand_facing(_G(my_walker), 3);
		_mazeCurrentIndex = _rooms[_mazeCurrentIndex]._right;
		debugRoomChanged();

		if (_chiselActiveFl) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "Chisel", false);
			terminateMachine(_709ChiselMach);
		}

		if (_mazeCurrentIndex == 97 && inv_object_is_here("Chisel")) {
			_chiselActiveFl = true;
			_709ChiselMach = series_place_sprite("709 CHISEL", 0,0,0,100,3840);
			hotspot_set_active(_G(currentSceneDef).hotspots, "Chisel", true);
		}

		if (_incenseBurnerActiveFl) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "Incense Burner", false);
			terminateMachine(_709IncenseHolderMach);
		}

		if (_mazeCurrentIndex == 22 && inv_object_is_here("INCENSE BURNER")) {
			_incenseBurnerActiveFl = true;
			_709IncenseHolderMach = series_place_sprite("709 INCENSE HOLDER", 0, 0, 0, 100, 3840);
			hotspot_set_active(_G(currentSceneDef).hotspots, "Incense Burner", true);
		}

		if (_pullRightFl) {
			terminateMachine(_ripPullMach03);
			_pullRightFl = false;
		}

		if (_pullLeftFl) {
			terminateMachine(_ripPullMach);
			_pullLeftFl = false;
		}

		if (_pullCenterFl) {
			terminateMachine(_ripPullMach04);
			_pullCenterFl = false;
		}

		if (_pullNearFl) {
			terminateMachine(_ripPullMach05);
			_pullNearFl = false;
		}

		pal_fade_init(_G(master_palette), 0, 255, 100, 30, -1);
		player_set_commands_allowed(true);
		delay = imath_ranged_rand(1, 10);
		if (delay == 10) {
			kernel_timing_trigger(delay, 1000, nullptr);
		}

		break;

	case 40:
		ws_demand_location(_G(my_walker), 320, 400);
		ws_demand_facing(_G(my_walker), 1);
		_mazeCurrentIndex = _rooms[_mazeCurrentIndex]._up;
		debugRoomChanged();

		if (_chiselActiveFl) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "Chisel", false);
			terminateMachine(_709ChiselMach);
		}

		if (_mazeCurrentIndex == 97 && inv_object_is_here("Chisel")) {
			_chiselActiveFl = true;
			_709ChiselMach = series_place_sprite("709 CHISEL", 0, 0, 0, 100, 3840);
			hotspot_set_active(_G(currentSceneDef).hotspots, "Chisel", true);
		}

		if (_incenseBurnerActiveFl) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "Incense Burner", false);
			terminateMachine(_709IncenseHolderMach);
		}

		if (_mazeCurrentIndex == 22 && inv_object_is_here("INCENSE BURNER")) {
			_incenseBurnerActiveFl = true;
			_709IncenseHolderMach = series_place_sprite("709 INCENSE HOLDER", 0, 0, 0, 100, 3840);
			hotspot_set_active(_G(currentSceneDef).hotspots, "Incense Burner", true);
		}

		if (_pullRightFl) {
			terminateMachine(_ripPullMach03);
			_pullRightFl = false;
		}

		if (_pullLeftFl) {
			terminateMachine(_ripPullMach);
			_pullLeftFl = false;
		}

		if (_pullCenterFl) {
			terminateMachine(_ripPullMach04);
			_pullCenterFl = false;
		}

		if (_pullNearFl) {
			terminateMachine(_ripPullMach05);
			_pullNearFl = false;
		}

		pal_fade_init(_G(master_palette), 0, 255, 100, 30, -1);
		player_set_commands_allowed(true);
		delay = imath_ranged_rand(1, 10);
		if (delay == 10) {
			kernel_timing_trigger(delay, 1000, nullptr);
		}

		break;

	case 50:
		ws_demand_location(_G(my_walker), 320, 280);
		ws_demand_facing(_G(my_walker), 5);
		_mazeCurrentIndex = _rooms[_mazeCurrentIndex]._down;
		debugRoomChanged();

		if (_mazeCurrentIndex == 50) {
			adv_kill_digi_between_rooms(false);
			digi_play_loop("950_s41", 3, 255, -1, -1);
			RemoveSystemHotkey(KEY_CLEAR);
			RemoveSystemHotkey(5);
			_G(game).setRoom(710);
		}

		if (_chiselActiveFl) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "Chisel", false);
			terminateMachine(_709ChiselMach);
		}

		if (_mazeCurrentIndex == 97 && inv_object_is_here("Chisel")) {
			_chiselActiveFl = true;
			_709ChiselMach = series_place_sprite("709 CHISEL", 0, 0, 0, 100, 3840);
			hotspot_set_active(_G(currentSceneDef).hotspots, "Chisel", true);
		}

		if (_incenseBurnerActiveFl) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "Incense Burner", false);
			terminateMachine(_709IncenseHolderMach);
		}

		if (_mazeCurrentIndex == 22 && inv_object_is_here("INCENSE BURNER")) {
			_incenseBurnerActiveFl = true;
			_709IncenseHolderMach = series_place_sprite("709 INCENSE HOLDER", 0, 0, 0, 100, 3840);
			hotspot_set_active(_G(currentSceneDef).hotspots, "Incense Burner", true);
		}

		if (_pullRightFl) {
			terminateMachine(_ripPullMach03);
			_pullRightFl = false;
		}

		if (_pullLeftFl) {
			terminateMachine(_ripPullMach);
			_pullLeftFl = false;
		}

		if (_pullCenterFl) {
			terminateMachine(_ripPullMach04);
			_pullCenterFl = false;
		}

		if (_pullNearFl) {
			terminateMachine(_ripPullMach05);
			_pullNearFl = false;
		}

		pal_fade_init(_G(master_palette), 0, 255, 100, 30, -1);
		player_set_commands_allowed(true);
		delay = imath_ranged_rand(1, 10);
		if (delay == 10) {
			kernel_timing_trigger(delay, 1000, nullptr);
		}

		break;

	case 100:
		digi_play("950_s41", 3, 255, 100, -1);
		break;

	case 500:
		digi_stop(1);
		digi_play_loop("304_s05", 2, 255, -1, -1);
		kernel_timing_trigger(20, 502, nullptr);

		break;

	case 502:
		digi_play("709_s99", 3, 255, 504, -1);
		break;

	case 504:
		adv_kill_digi_between_rooms(false);
		digi_play_loop("950_s41", 3, 255, -1, -1);
		RemoveSystemHotkey(KEY_CLEAR);
		RemoveSystemHotkey(5);
		_G(game).setRoom(710);

		break;

	case 1000:
		player_set_commands_allowed(false);
		kernel_timing_trigger(10, 1001, nullptr);

		break;

	case 1001:
		player_update_info(_G(my_walker), &_G(player_info));

		switch (_G(player_info).facing) {
		case 1:
			kernel_timing_trigger(10, 1002);
			break;

		case 3:
		case 9:
			kernel_timing_trigger(10, 1047);
			break;

		case 5:
			kernel_timing_trigger(10, 1022);
			break;

		default:
			break;
		}
		break;

	case 1002:
		kernel_timing_trigger(10, imath_ranged_rand(1, 2) == 1 ? 1007 : 1012, nullptr);
		break;

	case 1007:
		_ripTrekHeadTurnPos1Series = series_load("RIP TREK HEAD TURN POS1", -1, nullptr);
		setGlobals1(_ripTrekHeadTurnPos1Series, 1, 3, 3, 3, 0, 3, 1, 1, 1, 0, 4, 8, 8, 8, 0, 8, 4, 4, 4, 0);
		sendWSMessage_110000(_G(my_walker), 1008);

		break;

	case 1008:
		sendWSMessage_190000(_G(my_walker), 5);
		sendWSMessage_120000(_G(my_walker), 1009);

		break;

	case 1009:
		sendWSMessage_130000(_G(my_walker), 1010);
		break;

	case 1010:
		sendWSMessage_180000(_G(my_walker), 1011);
		break;

	case 1011:
		sendWSMessage_150000(_G(my_walker), 1073);
		break;

	case 1012:
		_ripTrekHeadTurnPos1Series = series_load("RIP TREK HANDS HIPS POS1", -1, nullptr);
		setGlobals1(_ripTrekHeadTurnPos1Series, 1, 1, 1, 1, 0, 1, 12, 12, 12, 0, 12, 1, 1, 1, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 1013);

		break;

	case 1013:
		sendWSMessage_190000(_G(my_walker), 5);
		sendWSMessage_120000(_G(my_walker), 1014);
		break;

	case 1014:
		sendWSMessage_130000(_G(my_walker), 1015);
		break;

	case 1015:
	case 1031:
	case 1034:
	case 1041:
	case 1061:
	case 1070:
		sendWSMessage_150000(_G(my_walker), 1073);
		break;

	case 1022:
		switch (imath_ranged_rand(1, 3)) {
		case 1:
			kernel_timing_trigger(10, 1027, nullptr);
			break;

		case 2:
			kernel_timing_trigger(10, 1032, nullptr);
			break;

		case 3:
			kernel_timing_trigger(10, 1037, nullptr);
			break;


		default:
			break;
		}
		break;

	case 1027:
		_ripTrekHeadTurnPos1Series = series_load("RIP TREK HEAD TURN POS5", -1, nullptr);
		setGlobals1(_ripTrekHeadTurnPos1Series, 1, 7, 7, 7, 0, 7, 1, 1, 1, 0, 8, 12, 12, 12, 0, 12, 8, 8, 8, 0);
		sendWSMessage_110000(_G(my_walker), 1028);

		break;

	case 1028:
		sendWSMessage_120000(_G(my_walker), 1029);
		break;

	case 1029:
		sendWSMessage_130000(_G(my_walker), 1030);
		break;

	case 1030:
		sendWSMessage_180000(_G(my_walker), 1031);
		break;

	case 1032:
		_ripTrekHeadTurnPos1Series = series_load("RIP TREK HANDS HIP POS5", -1, nullptr);
		setGlobals1(_ripTrekHeadTurnPos1Series, 1, 10, 10, 10, 0, 10, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 1033);

		break;

	case 1033:
		sendWSMessage_120000(_G(my_walker), 1034);
		break;

	case 1037:
		_ripTrekHeadTurnPos1Series = series_load("RIP LOOKS L R DOWN", -1, nullptr);
		setGlobals1(_ripTrekHeadTurnPos1Series, 1, 4, 4, 4, 0, 4, 1, 1, 1, 0, 5, 9, 9, 9, 0, 9, 5, 5, 5, 0);
		sendWSMessage_110000(_G(my_walker), 1038);

		break;

	case 1038:
		sendWSMessage_120000(_G(my_walker), 1039);
		break;

	case 1039:
		sendWSMessage_130000(_G(my_walker), 1040);
		break;

	case 1040:
		sendWSMessage_180000(_G(my_walker), 1041);
		break;

	case 1047:
		kernel_timing_trigger(10, imath_ranged_rand(1, 2) == 1 ? 1057 : 1067, nullptr);
		break;

	case 1057:
		_ripTrekHeadTurnPos1Series = series_load("RIP TREK HEAD TURN POS3", -1, nullptr);
		setGlobals1(_ripTrekHeadTurnPos1Series, 1, 5, 5, 5, 0, 5, 1, 1, 1, 0, 6, 9, 9, 9, 0, 9, 6, 6, 6, 0);
		sendWSMessage_110000(_G(my_walker), 1058);

		break;

	case 1058:
		sendWSMessage_120000(_G(my_walker), 1059);
		break;

	case 1059:
		sendWSMessage_130000(_G(my_walker), 1060);
		break;

	case 1060:
		sendWSMessage_180000(_G(my_walker), 1061);
		break;

	case 1067:
		_ripTrekHeadTurnPos1Series = series_load("RIP TREK HAND CHIN POS3", -1, nullptr);
		setGlobals1(_ripTrekHeadTurnPos1Series, 1, 14, 14, 14, 0, 14, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), 1068);

		break;

	case 1068:
		kernel_timing_trigger(60, 1069, nullptr);
		break;

	case 1069:
		sendWSMessage_120000(_G(my_walker), 1070);
		break;

	case 1073:
		series_unload(_ripTrekHeadTurnPos1Series);
		player_set_commands_allowed(true);
		ws_unhide_walker(_G(my_walker));

		break;

	default:
		break;
	}
}

void Room709::clearPressed(void *, void *) {
	static int32 _field84 = 0;
	static int32 _field88 = 0;
	static uint32 _field8C = 0;

	_field8C = timer_read_60();

	if (_field84 == 0) {
		_field88 = timer_read_60();
		++_field84;
	} else if (_field8C - _field88 < 60) {
		_field88 = _field8C;
		++_field84;
	} else {
		_field88 = _field8C;
		_field84 = 1;
	}

	if (_field84 == 3) {
		digi_preload("304_s05", -1);
		digi_preload("709_s99", -1);
		_G(kernel).trigger_mode = KT_DAEMON;
		disable_player_commands_and_fade_init(500);
	}
}

void Room709::debugRoomChanged() {
	if (gDebugLevel > 0) {
		// Show room exits
		const Maze709Room &room = _rooms[_mazeCurrentIndex];
		debug(1, "Room %.2d, DIRS: %c %c %c %c",
			_mazeCurrentIndex,
			room._left ? 'L' : ' ',
			room._right ? 'R' : ' ',
			room._up ? 'U' : ' ',
			room._down ? 'D' : ' '
		);

		// Assert that room exits match up to the corresponding exits
		// in the destination room
		if (room._left)
			assert(_rooms[room._left]._right == _mazeCurrentIndex);
		if (room._right)
			assert(_rooms[room._right]._left == _mazeCurrentIndex);
		if (room._up)
			assert(_rooms[room._up]._down == _mazeCurrentIndex);
		if (room._down)
			assert(_rooms[room._down]._up == _mazeCurrentIndex);
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
