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

#include "m4/riddle/rooms/section9/room996.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

Room996::Room996() : Room() {
	for (int i = 0; i < 13; ++i)
		_flagArray[i] = 0;
}

void Room996::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room996::init() {
	digi_preload("950_s56", -1);
	interface_hide();
	series_show("996mark", 0, 16, -1, -1, 0, 100, 0, 0);
	_flag = 1;
	if (_G(flags)[V290]) {
		_roomStates_loop0 = series_show("996 Charcoal Page", 256, 16, -1, -1, 0, 100, 0, 0);
		digi_preload("205r13a", -1);
		digi_play("205r13a", 1, 255, -1, 205);
		hotspot_set_active(_G(currentSceneDef).hotspots, "FORWARD", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "BACK", false);
	} else {
		if (player_been_here(205)) {
			_roomStates_loop0 = series_show("996 RIPPED OUT PAGE", 256, 16, -1, -1, 0, 100, 0, 0);
			hotspot_set_active(_G(currentSceneDef).hotspots, "FORWARD", false);
			hotspot_set_active(_G(currentSceneDef).hotspots, "BACK", false);
		} else {
			if (_G(flags)[V280]) {
				_roomStates_tt = series_show("996 Peru Pictograph Snake", 256, 16, -1, -1, 0, 100, 0, 0);

				if (_G(flags)[V148]) {
					_roomStates_field4 = series_show("996 Peru Pictograph Condor", 256, 16, -1, -1, 0, 100, 0, 0);
				} else {
					_roomStates_field4 = series_show("996 Peru Pictograph Spider", 256, 16, -1, -1, 0, 100, -13, -87);
				}

				if (_G(flags)[V156]) {
					_roomStates_pu = series_show("996 Peru Pictos Solved Spider", 256, 16, -1, -1, 0, 100, 0, 0);
				} else {
					_roomStates_field8 = series_show("996 Peru Pictograph Spider", 256, 16, -1, -1, 0, 100, 0, 0);
				}

				if (_G(flags)[V283]) {
					_roomStates_untie = series_show("996 Peru Pictograph Monkey", 256, 16, -1, -1, 0, 100, 0, 0);
				} else {
					_roomStates_untie = series_show("996 Peru Pictograph Spider", 256, 16, -1, -1, 0, 100, -21, 71);
				}
			}

			hotspot_set_active(_G(currentSceneDef).hotspots, "BACK", false);
		}
	}

	for (int i = 1; i < 12; ++i)
		_flagArray[i] = 0;

	if (_G(flags)[kEasterIslandCartoon])
		_flagArray[1] = 1;

	if (_G(flags)[kChinshiCartoon])
		_flagArray[1] = 2;

	if (_G(flags)[kTabletsCartoon])
		_flagArray[1] = 3;

	if (_G(flags)[kEpitaphCartoon])
		_flagArray[1] = 5;

	if (_G(flags)[kGraveyardCartoon])
		_flagArray[1] = 4;

	if (_G(flags)[kCastleCartoon])
		_flagArray[1] = 6;

	if (_G(flags)[kMocaMocheCartoon])
		_flagArray[1] = 7;

	if (_G(flags)[kTempleCartoon])
		_flagArray[1] = 8;

	if (_G(flags)[kEmeraldCartoon])
		_flagArray[1] = 9;

	_flagArray[12] = 1;
	_moveValue = 0;
}

void Room996::pre_parser() {
	if (player_said("forward", nullptr, nullptr) && (_flagArray[12] != 3)) {
		digi_play("950_s56", 1, 255, -1, 950);
		++_flagArray[12];
	}

	if (player_said("back", nullptr, nullptr) && (_flagArray[12] != 3)) {
		digi_play("950_s56", 1, 255, -1, 950);
		--_flagArray[12];
	}

	if (player_said("forward", nullptr, nullptr) && (_flagArray[12] == 3) && (_flagArray[1] == 0)) {
		++_flagArray[12];
	}

	if (player_said("forward", nullptr, nullptr) && (_flagArray[12] == 4) && (_G(flags)[V047] == 0)) {
		++_flagArray[12];
	}

	if (player_said("back", nullptr, nullptr) && (_flagArray[12] == 4) && (_G(flags)[V047] == 0)) {
		--_flagArray[12];
	}

	if (player_said("back", nullptr, nullptr) && (_flagArray[12] == 3) && (_flagArray[1] == 0)) {
		--_flagArray[12];
	}

	_flagArray[12] = CLIP(_flagArray[12], (int32)1, (int32)5);

	_G(player).need_to_walk = false;
	_G(player).ready_to_walk = true;
	_G(player).waiting_for_walk = false;
}

void Room996::parser() {
	if (player_said("--", nullptr, nullptr)) {
		_G(player).command_ready = false;
		return;
	}

	if (player_said("exit", nullptr, nullptr) && (_G(kernel).trigger >= -1)) {
		if (_G(kernel).trigger == -1) {
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(1);
			_G(player).command_ready = false;
			return;
		}

		if (_G(kernel).trigger == 1) {
			interface_show();
			restoreAutosave();
			_G(player).command_ready = false;
			return;
		}
	}

	switch (_flagArray[12]) {
	case 1:
		if (_G(kernel).trigger == -1) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "FORWARD", true);
			hotspot_set_active(_G(currentSceneDef).hotspots, "BACK", false);
			terminateMachine(_roomStates_ripTalker);
			terminateMachine(_roomStates_field18);
			terminateMachine(_roomStates_ripTalking);
			terminateMachine(_roomStates_ripTalk);
			terminateMachine(_roomStates_field24);
			kernel_timing_trigger(10, 1, nullptr);

		} else if (_G(kernel).trigger == 1 && _G(flags)[V280] != 0) {
			_roomStates_tt = series_show("996 Peru Pictograph Snake", 256, 16, -1, -1, 0, 100, 0, 0);

			if (_G(flags)[V148])
				_roomStates_field4 = series_show("996 Peru Pictograph Condor", 256, 16, -1, -1, 0, 100, 0, 0);
			else
				_roomStates_field4 = series_show("996 Peru Pictograph Spider", 256, 16, -1, -1, 0, 100, -13, -87);

			if (_G(flags)[V156])
				_roomStates_pu = series_show("996 Peru Pictos Solved Spider", 256, 16, -1, -1, 0, 100, 0, 0);
			else
				_roomStates_field8 = series_show("996 Peru Pictograph Spider", 256, 16, -1, -1, 0, 100, 0, 0);

			if (_G(flags)[V283])
				_roomStates_untie = series_show("996 Peru Pictograph Monkey", 256, 16, -1, -1, 0, 100, 0, 0);
			else
				_roomStates_untie = series_show("996 Peru Pictograph Spider", 256, 16, -1, -1, 0, 100, -21, 71);
		}
		break;
	case 2:
		if (_G(kernel).trigger == -1) {
			if (_flagArray[_moveValue] != 0 || _G(flags)[V047] || _G(flags)[V196] || _G(flags)[V201] || _G(flags)[V207] || _G(flags)[V208] || _G(flags)[V209])
				hotspot_set_active(_G(currentSceneDef).hotspots, "forward", true);
			else
				hotspot_set_active(_G(currentSceneDef).hotspots, "forward", false);

			hotspot_set_active(_G(currentSceneDef).hotspots, "back", true);
			terminateMachine(_roomStates_tt);
			terminateMachine(_roomStates_field4);
			terminateMachine(_roomStates_pu);
			terminateMachine(_roomStates_field8);
			terminateMachine(_roomStates_untie);
			terminateMachine(_roomStates_loop1);
			terminateMachine(_roomStates_loop2);
			terminateMachine(_roomStates_loop3);
			terminateMachine(_roomStates_loop4);
			terminateMachine(_roomStates_loop5);
			terminateMachine(_roomStates_loop6);
			terminateMachine(_roomStates_loop7);
			terminateMachine(_roomStates_ear2);
			terminateMachine(_roomStates_field4C);
			kernel_timing_trigger(10, 1, nullptr);

		} else if (_G(kernel).trigger == 1) {
			if (_G(flags)[V285])
				_roomStates_ripTalker = series_show("996 TITLE JOURNAL TYPE", 256, 16, -1, -1, 0, 100, 0, 0);

			if (_G(flags)[V286])
				_roomStates_field18 = series_show("996 sikkamese text alone", 256, 16, -1, -1, 0, 100, 0, 0);

			if (_G(flags)[V287])
				_roomStates_ripTalking = series_show("996 PERU SCRIPT", 256, 16, -1, -1, 0, 100, 0, 0);

			if (_G(flags)[V288]) {
				if (player_been_here(615))
					_roomStates_ripTalk = series_show("996 EASTER ISLAND SCRIPT", 256, 16, -1, -1, 0, 100, 0, 0);
				else
					_roomStates_ripTalk = series_show("996 East Isl. w/o #s", 256, 16, -1, -1, 0, 100, 0, 0);
			}

			if (_G(flags)[V289])
				_roomStates_field24 = series_show("996 KEY TO TOMB SCRIPT", 256, 16, -1, -1, 0, 100, 0, 0);

		}
		break;
	case 3:
		if (_G(kernel).trigger == -1) {
			if (player_said("forward", nullptr, nullptr)) {
				digi_play("950_s56", 1, 255, -1, 950);
				++_moveValue;
			} else if (player_said("back", nullptr, nullptr)) {
				digi_play("950_s56", 1, 255, -1, 950);
				--_moveValue;
			}

			if (_moveValue == 0) {
				_flagArray[12] = 2;
				digi_play("950_s56", 1, 255, -1, 950);
			} else if (_flagArray[_moveValue] == 0) {
				if (_G(flags)[V047])
					_flagArray[12] = 4;
				else if (_G(flags)[V196] || _G(flags)[V201] || _G(flags)[V207] || _G(flags)[V208] || _G(flags)[V209])
					_flagArray[12] = 5;
			}

			if (_flagArray[_moveValue] != 0 || _G(flags)[V047] || _G(flags)[V196] || _G(flags)[V201] || _G(flags)[V207] || _G(flags)[V208] || _G(flags)[V209])
				hotspot_set_active(_G(currentSceneDef).hotspots, "forward", true);
			else
				hotspot_set_active(_G(currentSceneDef).hotspots, "forward", false);

			if (_flagArray[12] == 4) {
				if (_G(flags)[V196] || _G(flags)[V201] || _G(flags)[V207] || _G(flags)[V208] || _G(flags)[V209])
					hotspot_set_active(_G(currentSceneDef).hotspots, "forward", true);
				else
					hotspot_set_active(_G(currentSceneDef).hotspots, "forward", false);
			}

			if (_flagArray[12] == 5)
				hotspot_set_active(_G(currentSceneDef).hotspots, "forward", false);

			terminateMachine(_roomStates_ripTalker);
			terminateMachine(_roomStates_field18);
			terminateMachine(_roomStates_ripTalking);
			terminateMachine(_roomStates_ripTalk);
			terminateMachine(_roomStates_field24);
			terminateMachine(_roomStates_loop1);
			terminateMachine(_roomStates_loop2);
			terminateMachine(_roomStates_loop3);
			terminateMachine(_roomStates_loop4);
			terminateMachine(_roomStates_loop5);
			terminateMachine(_roomStates_loop6);
			terminateMachine(_roomStates_loop7);
			terminateMachine(_roomStates_ear2);
			terminateMachine(_roomStates_field4C);
			kernel_timing_trigger(10, 1, nullptr);
		} else if (_G(kernel).trigger == 1) {
			switch (_flagArray[_moveValue]) {
			case 1:
				_roomStates_loop1 = series_show("996car", 256, 16, -1, -1, 0, 100, 0, 0);
				break;
			case 2:
				_roomStates_loop1 = series_show("996car", 256, 16, -1, -1, 1, 100, 0, 0);
				break;
			case 3:
				_roomStates_loop1 = series_show("996car", 256, 16, -1, -1, 2, 100, 0, 0);
				break;
			case 4:
				_roomStates_loop1 = series_show("996car", 256, 16, -1, -1, 3, 100, 0, 0);
				break;
			case 5:
				_roomStates_loop1 = series_show("996car", 256, 16, -1, -1, 4, 100, 0, 0);
				break;
			case 6:
				_roomStates_loop1 = series_show("996car", 256, 16, -1, -1, 5, 100, 0, 0);
				break;
			case 7:
				_roomStates_loop1 = series_show("996car", 256, 16, -1, -1, 6, 100, 0, 0);
				break;
			case 8:
				_roomStates_loop1 = series_show("996car", 256, 16, -1, -1, 7, 100, 0, 0);
				break;
			case 9:
				_roomStates_loop1 = series_show("996car", 256, 16, -1, -1, 8, 100, 0, 0);
				break;
			default:
				break;
			}
		}
		break;
	case 4:
		if (_G(kernel).trigger == -1) {
			if (_G(flags)[V196] || _G(flags)[V201] || _G(flags)[V207] || _G(flags)[V208] || _G(flags)[V209])
				hotspot_set_active(_G(currentSceneDef).hotspots, "forward", true);
			else
				hotspot_set_active(_G(currentSceneDef).hotspots, "forward", false);

			terminateMachine(_roomStates_ripTalker);
			terminateMachine(_roomStates_field18);
			terminateMachine(_roomStates_ripTalking);
			terminateMachine(_roomStates_ripTalk);
			terminateMachine(_roomStates_field24);
			terminateMachine(_roomStates_loop1);
			terminateMachine(_roomStates_loop3);
			terminateMachine(_roomStates_loop4);
			terminateMachine(_roomStates_loop5);
			terminateMachine(_roomStates_loop6);
			terminateMachine(_roomStates_loop7);
			terminateMachine(_roomStates_ear2);
			terminateMachine(_roomStates_field4C);
			kernel_timing_trigger(10, 1, nullptr);
		} else if (_G(kernel).trigger == 1) {
			_check201Fl = _G(flags)[V201] != 0;
			_check207Fl = _G(flags)[V207] != 0;
			_check208Fl = _G(flags)[V208] != 0;
			_check209Fl = _G(flags)[V209] != 0;

			if (!_check201Fl && !_check207Fl && !_check208Fl && !_check209Fl) {
				_roomStates_loop2 = series_show("PING OBJ136", 256, 16, -1, -1, 0, 100, 206, 67);
			} else if (_check201Fl && !_check207Fl && !_check208Fl && !_check209Fl) {
				_roomStates_loop2 = series_show("PING OBJ122", 256, 16, -1, -1, 0, 100, 206, 67);
			} else if (!_check201Fl && _check207Fl && !_check208Fl && !_check209Fl) {
				_roomStates_loop2 = series_show("PING OBJ123", 256, 16, -1, -1, 0, 100, 206, 67);
			} else if (!_check201Fl && !_check207Fl && !_check208Fl && _check209Fl) {
				_roomStates_loop2 = series_show("PING OBJ124", 256, 16, -1, -1, 0, 100, 206, 67);
			} else if (!_check201Fl && !_check207Fl && _check208Fl && !_check209Fl) {
				_roomStates_loop2 = series_show("PING OBJ125", 256, 16, -1, -1, 0, 100, 206, 67);
			} else if (_check201Fl && _check207Fl && !_check208Fl && !_check209Fl) {
				_roomStates_loop2 = series_show("PING OBJ126", 256, 16, -1, -1, 0, 100, 206, 67);
			} else if (_check201Fl && !_check207Fl && _check208Fl && !_check209Fl) {
				_roomStates_loop2 = series_show("PING OBJ127", 256, 16, -1, -1, 0, 100, 206, 67);
			} else if (_check201Fl && !_check207Fl && !_check208Fl && _check209Fl) {
				_roomStates_loop2 = series_show("PING OBJ128", 256, 16, -1, -1, 0, 100, 206, 67);
			} else if (!_check201Fl && _check207Fl && !_check208Fl && _check209Fl) {
				_roomStates_loop2 = series_show("PING OBJ129", 256, 16, -1, -1, 0, 100, 206, 67);
			} else if (!_check201Fl && _check207Fl && _check208Fl && !_check209Fl) {
				_roomStates_loop2 = series_show("PING OBJ130", 256, 16, -1, -1, 0, 100, 206, 67);
			} else if (!_check201Fl && !_check207Fl && _check208Fl && _check209Fl) {
				_roomStates_loop2 = series_show("PING OBJ131", 256, 16, -1, -1, 0, 100, 206, 67);
			} else if (_check201Fl && _check207Fl && _check208Fl && !_check209Fl) {
				_roomStates_loop2 = series_show("PING OBJ132", 256, 16, -1, -1, 0, 100, 206, 67);
			} else if (_check201Fl && !_check207Fl && _check208Fl && _check209Fl) {
				_roomStates_loop2 = series_show("PING OBJ133", 256, 16, -1, -1, 0, 100, 206, 67);
			} else if (_check201Fl && _check207Fl && !_check208Fl && _check209Fl) {
				_roomStates_loop2 = series_show("PING OBJ134", 256, 16, -1, -1, 0, 100, 206, 67);
			} else if (!_check201Fl && _check207Fl && _check208Fl && _check209Fl) {
				_roomStates_loop2 = series_show("PING OBJ135", 256, 16, -1, -1, 0, 100, 206, 67);
			} else if (_check201Fl && _check207Fl && _check208Fl && _check209Fl) {
				_roomStates_loop2 = series_show("PING TWELVETREES' MAP", 256, 16, -1, -1, 0, 100, 206, 67);
			}
		}

		break;
	case 5:
		if (_G(kernel).trigger == -1) {
			terminateMachine(_roomStates_ripTalker);
			terminateMachine(_roomStates_field18);
			terminateMachine(_roomStates_ripTalking);
			terminateMachine(_roomStates_ripTalk);
			terminateMachine(_roomStates_field24);
			terminateMachine(_roomStates_loop2);
			hotspot_set_active(_G(currentSceneDef).hotspots, "forward", false);
			kernel_timing_trigger(10, 1, nullptr);
		} else if (_G(kernel).trigger == 1) {
			_roomStates_loop3 = series_show("996 EA Glyph title", 256, 16, -1, -1, 0, 100, 0, 0);
			if (_G(flags)[V201] || _G(flags)[V207] || _G(flags)[V208] || _G(flags)[V209])
				_roomStates_loop5 = series_show("996 EA Glyph Quarry title", 256, 16, -1, -1, 0, 100, 0, 0);

			if (_G(flags)[V196])
				_roomStates_loop4 = series_show("996 EA Glyph Fallen head", 256, 16, -1, -1, 0, 100, 0, 0);

			if (_G(flags)[V201])
				_roomStates_loop7 = series_show("996 EA Glyph Sharks Tooth", 256, 16, -1, -1, 0, 100, 0, 0);

			if (_G(flags)[V207])
				_roomStates_loop6 = series_show("996 EA Glyph Scallop Shell", 256, 16, -1, -1, 0, 100, 0, 0);

			if (_G(flags)[V208])
				_roomStates_ear2 = series_show("996 EA Glyph Starfish", 256, 16, -1, -1, 0, 100, 0, 0);

			if (_G(flags)[V209])
				_roomStates_field4C = series_show("996 EA Glyph SeaHorse", 256, 16, -1, -1, 0, 100, 0, 0);
		}
		break;
	default:
		break;
	}

	_G(player).command_ready = false;
}

void Room996::daemon() {
	if (_G(kernel).trigger == 1)
		player_set_commands_allowed(true);
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
