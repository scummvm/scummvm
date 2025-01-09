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

#include "m4/riddle/rooms/section8/section8_room.h"

#include "m4/riddle/riddle.h"
#include "m4/riddle/rooms/section8/section8.h"
#include "m4/riddle/vars.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_vmng.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

int32 Section8Room::subCE498(int32 val1) {
	if (val1 == _var2)
		return 0;

	if (val1 == _var3)
		return 1;

	if (val1 == _var4)
		return 2;

	if (val1 == _var5)
		return 3;

	return -1;
}

void Section8Room::sendWSMessage_3840000(machine *machine, int32 trigger) {
	int32 num = trigger;
	if (trigger == 0)
		num = -1;

	_G(globals)[GLB_TEMP_4] = kernel_trigger_create(num);
	sendWSMessage(ACTION_900 << 16, 0, machine, 0, nullptr, 1);
}

int32 Section8Room::subCE52E(int32 val1) {
	if (_currentRoom == 844 && val1 == 1 && _G(flags)[V271] != 0)
		return 3;

	if (_currentRoom == 834 && val1 == 5)
		return 2;

	if (subCE498(val1) < 0)
		return 0;

	return 1;
}

void Section8Room::moveScreen(int32 dx, int32 dy) {
	int32 status;
	ScreenContext *game_buff_ptr = vmng_screen_find(_G(gameDrawBuff), &status);
	if (game_buff_ptr == nullptr)
		error("M4: no gameDrawBuff");

	MoveScreenAbs(game_buff_ptr, _G(kernel).letter_box_x - dx, _G(kernel).letter_box_y - dy);
}

void Section8Room::getSeriesName(int32 val1, bool true_or_False) {
	if (subCE498(val1) < 0)
		_currentSeriesName = Common::String::format("%dsldf%d", _currentRoom, val1);
	else if (true_or_False)
		_currentSeriesName = Common::String::format("%dsldr%d", _currentRoom, val1);
	else
		_currentSeriesName = Common::String::format("%dslds%d", _currentRoom, val1);
}

int32 Section8Room::daemonSub1(int32 dx, bool ascendingFl) {
	int32 fullArr[5] = {100, 560, 960, 1360, 1815};
	int32* ascArr = &fullArr[1];
	int32 descArr[5] = {1815, 1360, 960, 560, 100};
	int i;

	if (ascendingFl) {
		for (i = 0; i < 4; ++i) {
			if (dx <= ascArr[i])
				break;
		}
	} else {
		for (i = 4; i > 0; --i) {
			if (dx >= descArr[i])
				break;
		}
	}

	hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN", false);
	hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN ", false);
	hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN  ", false);
	hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN   ", false);
	hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN    ", false);
	_guessHotspotName = Common::String("MEI CHEN      ");
	_unkArrayIndex = i;
	_unkArray[_unkArrayIndex] = 0;
	_guessX = fullArr[i];

	if (ascendingFl)
		_guessIndex = 11;
	else
		_guessIndex = 1;

	return fullArr[i];
}

void Section8Room::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
}


void Section8Room::pre_parser() {
	int16 caty = -1;

	if (player_said("look") || player_said("look at"))
		caty = 1;
	else if (player_said("gear") || player_said("use"))
		caty = 0;
	else if (player_said("take"))
		caty = 2;
	else if (player_said("talk to"))
		caty = 3;
	else if (player_said("go"))
		caty = 5;

	if (caty == 0
		&& (player_said("chariot") || player_said("chariot ") || scumm_strnicmp(_G(player).noun, "1st", 3) == 0
			|| scumm_strnicmp(_G(player).noun, "2nd", 3) == 0 || scumm_strnicmp(_G(player).noun, "3rd", 3) == 0
			|| scumm_strnicmp(_G(player).noun, "4th", 3) == 0 || scumm_strnicmp(_G(player).noun, "5th", 3) == 0
			|| scumm_strnicmp(_G(player).noun, "6th", 3) == 0 || scumm_strnicmp(_G(player).noun, "7th", 3) == 0)) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if ((caty == 1 && scumm_strnicmp(_G(player).noun, "mei chen", 8) == 0) || caty == 5) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}
}

void Section8Room::parser() {
	_G(player).command_ready = false;
	if (scumm_strnicmp(_G(player).noun, "mei chen", 8) && inv_player_has(_G(player).verb)) {
		digi_play("com017", 1, 255, -1, 997);
		return;
	}

	if (player_said("journal") && !player_said("look at")) {
		digi_play("com042", 1, 255, -1, 997);
		return;
	}

	int32 caty = -1;
	if (player_said("look") || player_said("look at")) {
		caty = 1;
	} else if (player_said("gear") || player_said("use")) {
		caty = 0;
	} else if (player_said("take")) {
		caty = 2;
	} else if (player_said("talk to")) {
		caty = 3;
	} else if (player_said("walk to") || player_said("spleen")) {
		caty = 4;
	} else if (player_said("go")) {
		caty = 5;
	}

	switch (caty) {
	case -1:
		_G(player).command_ready = true;
		break;

	case 0:
		if (player_said("chariot") || player_said("chariot ")) {
			switch (_G(kernel).trigger) {
			case -1:
				switch (_currentRoom) {
				case 804:
					ws_walk(_G(my_walker), 696, 330, nullptr, 10, 9, true);
					break;
				case 814:
					ws_walk(_G(my_walker), 851, 330, nullptr, 10, 9, true);
					break;
				case 824:
					ws_walk(_G(my_walker), 1627, 318, nullptr, 50, 3, true);
					break;
				case 834:
					digi_play("834r20", 1, 255, -1, -1);
					break;
				case 844:
					ws_walk(_G(my_walker), 1216, 330, nullptr, 10, 9, true);
					break;
				default:
					break;
				}

				break;

			case 10:
				player_set_commands_allowed(false);
				ws_hide_walker(_G(my_walker));
				switch (_currentRoom) {
				case 804:
					_ripAttemptsPush = series_load("rip attempts char push", -1, nullptr);
					_ripPushMach = series_plain_play("RIP ATTEMPTS CHAR PUSH", 1, 0, 100, 0, 5, 20, true);
					break;

				case 814:
					_ripAttemptsPush = series_load("rip attempts char push", -1, nullptr);
					_ripPushMach = series_plain_play("rip tries to push cart", 1, 0, 100, 0, 5, 20, true);
					break;

				case 844:
					_ripAttemptsPush = series_load("rip attempts char push", -1, nullptr);
					_ripPushMach = series_plain_play("844push", 1, 0, 100, 0, 5, 20, true);
					break;

				default:
					break;
				}
				break;

			case 20:
				kernel_timing_trigger(120, 30, nullptr);
				digi_play("950_s32", 2, 255, -1, -1);
				break;

			case 30:
				switch (_currentRoom) {
				case 804:
					_ripPushMach = series_plain_play("RIP ATTEMPTS CHAR PUSH", 1, 2, 100, 0, 5, 40, true);
					break;

				case 814:
					_ripPushMach = series_plain_play("rip tries to push cart", 1, 2, 100, 0, 5, 40, true);
					break;

				case 844:
					_ripPushMach = series_plain_play("844push", 1, 2, 100, 0, 5, 40, true);
					break;

				default:
					break;
				}

				break;

			case 40:
				player_set_commands_allowed(true);
				series_unload(_ripAttemptsPush);
				digi_play("814R26", 1, 255, -1, -1);
				terminateMachine(_ripPushMach);
				ws_unhide_walker(_G(my_walker));
				ws_demand_facing(_G(my_walker), 9);

				break;

			case 50:
				player_set_commands_allowed(false);
				terminateMachine(_824fire1Mach);
				terminateMachine(_824fire2Mach);
				series_unload(_ripTakerPos5);
				series_unload(_824fire2Serie);
				digi_preload("950_s33", -1);
				ws_hide_walker(_G(my_walker));
				terminateMachine(_ripPushMach);
				_ripPushMach = series_stream("824rp01", 5, 256, -1);
				series_stream_break_on_frame(_ripPushMach, 80, 60);
				digi_play_loop("950_s33", 2, 255, -1, -1);

				break;

			case 60:
				disable_player_commands_and_fade_init(70);
				break;

			case 70:
				_G(flags)[V270] = 805;
				_G(game).new_room = 805;
				adv_kill_digi_between_rooms(false);
				digi_preload("950_s29", -1);
				digi_play_loop("950_s29", 3, 96, -1, -1);

				break;

			default:
				break;
			}
		} // if (player_said("chariot") || player_said("chariot "))

		else if (scumm_strnicmp(_G(player).noun, "1st", 3) == 0 || scumm_strnicmp(_G(player).noun, "2nd", 3) == 0
			|| scumm_strnicmp(_G(player).noun, "3rd", 3) == 0 || scumm_strnicmp(_G(player).noun, "4th", 3) == 0
			|| scumm_strnicmp(_G(player).noun, "5th", 3) == 0 || scumm_strnicmp(_G(player).noun, "6th", 3) == 0
			|| scumm_strnicmp(_G(player).noun, "7th", 3) == 0) {
			_coordArrayId = _G(player).noun[0] - 0x30; // Thus a number between 1 and 7
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_trigger_dispatchx(kernel_trigger_create(15));
		}

		else if (player_said("horse") || player_said("fallen horse")) {
			// The original has 2 distinct tests but the same content -> merged
			digi_play("com069", 1, 255, -1, 997);
		}

		else if (player_said("soldier"))
			digi_play("com070", 1, 255, -1, 997);

		else if (player_said("soldier ")) {
			switch (_G(kernel).trigger) {
			case -1:
				digi_play("com070", 1, 255, -1, 997);
				break;

			case 10:
				digi_play("com071", 1, 255, -1, 997);
				break;

			default:
				break;
			}
		}

		else if (player_said("lit urn"))
			digi_play("com072", 1, 255, -1, 997);

		else if (player_said("unlit urn"))
			digi_play("com073", 1, 255, -1, 997);

		else
			_G(player).command_ready = true;

		break; // caty case 0

	case 1:
		if (player_said(" "))
			digi_play("com059", 1, 255, -1, 997);

		else if (player_said("wooden beam") && inv_object_in_scene("wooden beam", _currentRoom))
			digi_play("844r12", 1, 255, -1, -1);

		else if (scumm_strnicmp(_G(player).noun, "1st", 3) == 0 || scumm_strnicmp(_G(player).noun, "2nd", 3) == 0 || scumm_strnicmp(_G(player).noun, "3rd", 3) == 0 || scumm_strnicmp(_G(player).noun, "4th", 3) == 0 || scumm_strnicmp(_G(player).noun, "5th", 3) == 0 || scumm_strnicmp(_G(player).noun, "6th", 3) == 0 || scumm_strnicmp(_G(player).noun, "7th", 3) == 0) {
			switch (_currentRoom) {
			case 804:
			case 814:
				_currentSeriesName = Common::String::format("%dr%02d", _currentRoom, _G(player).noun[0] - 0x2B);
				break;
			case 824:
			case 834:
				_currentSeriesName = Common::String::format("%dr%02d", _currentRoom, _G(player).noun[0] - 0x2F);
				break;
			case 844:
				_currentSeriesName = Common::String::format("%dr%02d", _currentRoom, _G(player).noun[0] - 0x2E);
				break;
			default:
				break;
			}

			digi_play(_currentSeriesName.c_str(), 1, 255, -1, -1);
		}

		else if (player_said("lit urn"))
			digi_play("com060", 1, 255, -1, 997);

		else if (player_said("unlit urn"))
			digi_play("com061", 1, 255, -1, 997);

		else if (player_said("broken beam"))
			digi_play("804r15", 1, 255, -1, -1);

		else if (player_said("tipped soldier"))
			digi_play("com062", 1, 255, -1, 997);

		else if (player_said("fallen soldier") || player_said("fallen horse")) {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				player_update_info(_G(my_walker), &_G(player_info));
				_savedPlayerInfoFacing = _G(player_info).facing;
				_rptldSerie = series_load("rptld1a", -1, nullptr);
				setGlobals3(_rptldSerie, 1, 6);
				sendWSMessage_3840000(_G(my_walker), 10);

				break;

			case 10:
				if (player_said("fallen soldier"))
					digi_play("com062", 1, 255, 20, 997);
				else
					digi_play("814r18", 1, 255, 20, -1);

				break;

			case 20:
				setGlobals3(_rptldSerie, 6, 1);
				sendWSMessage_3840000(_G(my_walker), 30);
				break;

			case 30:
				player_set_commands_allowed(true);
				series_unload(_rptldSerie);
				ws_demand_facing(_G(my_walker), _savedPlayerInfoFacing);
				break;

			default:
				break;
			}
		} // player_said("fallen soldier") || player_said("fallen horse")

		else if (player_said("mural"))
			digi_play("com064", 1, 255, -1, 997);

		else if (player_said("chariot") || player_said("chariot "))
			digi_play("com063", 1, 255, -1, 997);

		else if (player_said("horse"))
			digi_play("com101", 1, 255, -1, 997);

		else if (scumm_strnicmp(_G(player).noun, "mei chen", 8) == 0) {
			if (_G(kernel).trigger == -1) {
				player_update_info(_G(my_walker), &_G(player_info));
				_savedPlayerInfoFacing = _G(player_info).facing;
				_savedPlayerInfoX = _G(player_info).x;
				_savedPlayerInfoY = _G(player_info).y;
				player_update_info(_mcTrekMach, &_G(player_info));
				if (_G(player_info).x >= _savedPlayerInfoX) {
					if (_G(player_info).x - _savedPlayerInfoX <= 30)
						ws_walk(_G(my_walker), _savedPlayerInfoX, _savedPlayerInfoY, nullptr, 10, 5, true);
					else
						ws_walk(_G(my_walker), _savedPlayerInfoX, _savedPlayerInfoY, nullptr, 10, 4, true);
				} else {
					if (_savedPlayerInfoX - _G(player_info).x <= 30)
						ws_walk(_G(my_walker), _savedPlayerInfoX, _savedPlayerInfoY, nullptr, 10, 7, true);
					else
						ws_walk(_G(my_walker), _savedPlayerInfoX, _savedPlayerInfoY, nullptr, 10, 8, true);
				}

			} else if (_G(kernel).trigger == 10) {
				digi_play("COM043", 1, 255, -1, 997);
			}

		} // if (player_said("horse") && scumm_strnicmp(_G(player).noun, "mei chen", 8) == 0)

		else if (player_said("soldier") || player_said("soldier "))
		// The original has two separate checks with the same underlying code => merged
			digi_play("com074", 1, 255, -1, 997);

		else
			_G(player).command_ready = true;

		break; // caty case 1

	case 2:
		if (player_said("chariot")) {
			switch (_currentRoom) {
			case 834:
				digi_play("834r15", 1, 255, -1, -1);
				break;

			case 844:
				digi_play("814r21", 1, 255, -1, -1);
				break;

			default:
				digi_play("804r20", 1, 255, -1, -1);
				break;
			}
		} // if (player_said("chariot"))

		else if (player_said("chariot "))
			digi_play("814r21", 1, 255, -1, -1);

		else if (player_said("horse") || player_said("fallen horse"))
			// The original has two separate checks with the same underlying code => merged
			digi_play("com065", 1, 255, -1, 997);

		else if (player_said("soldier") || player_said("soldier "))
			// The original has two separate checks with the same underlying code => merged
			digi_play("com066", 1, 255, -1, 997);

		else if (player_said("lit urn"))
			digi_play("com067", 1, 255, -1, 997);

		else if (player_said("unlit urn"))
			digi_play("com068", 1, 255, -1, 997);

		else if (player_said("wooden beam")) {
			switch (_G(kernel).trigger) {
			case -1:
				if (inv_object_in_scene("wooden beam", 844)) {
					player_set_commands_allowed(false);
					_ripTrekLowReach = series_load("RIP TREK LOW REACH POS2", -1, nullptr);
					setGlobals3(_ripTrekLowReach, 1, 16);
					sendWSMessage_3840000(_G(my_walker), 10);
				} else
					_G(player).command_ready = true;

				break;

			case 10:
				inv_give_to_player("WOODEN BEAM");
				kernel_examine_inventory_object("PING WOODEN BEAM", _G(master_palette), 5, 1, 410, 250, 20, nullptr, -1);

				break;

			case 20:
				terminateMachine(_844postMach);
				setGlobals3(_ripTrekLowReach, 16, 1);
				sendWSMessage_3840000(_G(my_walker), 30);
				digi_play("844r22", 1, 255, -1, -1);

				break;

			case 30:
				player_set_commands_allowed(true);
				series_unload(_ripTrekLowReach);
				hotspot_set_active(_G(currentSceneDef).hotspots, "wooden beam", false);
				ws_demand_facing(_G(my_walker), 2);

				break;

			default:
				break;
			}
		} // if (player_said("wooden beam"))

		break; // caty case 2

	case 3:
		if (scumm_strnicmp(_G(player).noun, "mei chen", 8) == 0) {
			switch (_G(kernel).trigger) {
			case -1:
				player_set_commands_allowed(false);
				_ripTakerPos5 = series_load("RIP TALKER POS 5", -1, nullptr);
				setGlobals1(_ripTakerPos5, 1, 4, 1, 4, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
				sendWSMessage_110000(_G(my_walker), -1);
				_savedRandom = imath_ranged_rand(1, 4);
				switch (_savedRandom) {
				case 1:
					digi_play("com044", 1, 255, 10, 997);
					break;

				case 2:
					digi_play("com045", 1, 255, 10, 997);
					break;

				case 3:
					digi_play("com046", 1, 255, 10, 997);
					break;

				default:
					digi_play("com047", 1, 255, 10, 997);
					break;
				}

				break;

			case 10:
				player_set_commands_allowed(true);
				sendWSMessage_150000(_G(my_walker), -1);
				series_unload(_ripTakerPos5);
				_savedRandom = imath_ranged_rand(1, 4);
				switch (_savedRandom) {
				case 1:
					digi_play("com048", 1, 255, -1, 997);
					break;

				case 2:
					digi_play("com049", 1, 255, -1, 997);
					break;

				case 3:
					digi_play("com050", 1, 255, -1, 997);
					break;

				default:
					digi_play("com051", 1, 255, -1, 997);
					break;
				}

				break;

			default:
				break;
			}
		} // if (scumm_strnicmp(_G(player).noun, "mei chen", 8) == 0)

		else
			_G(player).command_ready = true;

		break; // caty case 3

	case 5:
		switch (_G(kernel).trigger) {
		case -1:
			if (player_said("west")) {
				_savedNextRoom = 803;
				if (_currentRoom == 804 || _currentRoom == 814 || _currentRoom == 844)
					ws_walk(_G(my_walker), -10, 325, nullptr, 10, 3, true);
				else if (_currentRoom == 824)
					ws_walk(_G(my_walker), -10, 328, nullptr, 10, 3, true);
				else if (_currentRoom == 834)
					ws_walk(_G(my_walker), -10, 330, nullptr, 10, 3, true);
			} else {
				_savedNextRoom = 805;
				if (_currentRoom == 804 || _currentRoom == 814 || _currentRoom == 844)
					ws_walk(_G(my_walker), 1925, 325, nullptr, 10, 3, true);
				else if (_currentRoom == 824)
					ws_walk(_G(my_walker), 1925, 328, nullptr, 10, 3, true);
				else if (_currentRoom == 834)
					ws_walk(_G(my_walker), 1925, 330, nullptr, 10, 3, true);
			}

			break;

		case 10:
			player_set_commands_allowed(false);
			if (_G(flags)[V276] == 0) {
				setGlobals3(_meiHandsBehindBack, 17, 1);
				sendWSMessage_3840000(_mcTrekMach, -1);
			}
			disable_player_commands_and_fade_init(20);

			break;

		case 20:
			_G(game).new_room = _savedNextRoom;
			adv_kill_digi_between_rooms(false);
			digi_preload("950_s29", -1);
			digi_play_loop("950_s29", 3, 255, -1, -1);

			break;
		default:
			break;
		}

		break; // caty case 5

	default:
		break;
	}

}

void Section8Room::daemon() {
	int32 _dword1948BC[] = {1815, 1080, 1157, 1241, 1319, 1402, 1478};
	int32 _dword1948D8[] = {1561, 874, 949, 1034, 1117, 1198, 1276};
	int32 _dword1948F4[] = {1355, 624, 702, 787, 865, 949, 1027};
	int32 _dword194910[] = {1110, 278, 351, 439, 519, 601, 679};
	int32 _dword19492C[] = {756, 1219, 1301, 1383, 1466, 1548, 1626, 1701};

	switch (_G(kernel).trigger) {
	case 1:
		_byte1A19BC = 1; // Seems to be set once and never accessed otherwise...
		_field68 = 0;
		player_set_commands_allowed(true);
		if (_G(flags)[V276] == 0) {
			series_unload(S8_SHADOW_DIRS2[0]);
			series_unload(S8_SHADOW_DIRS2[1]);
			series_unload(S8_SHADOW_DIRS2[2]);
			series_unload(S8_SHADOW_DIRS2[3]);
			series_unload(S8_SHADOW_DIRS2[4]);
			series_unload(S8_SHADOW_DIRS1[3]);
			series_unload(S8_SHADOW_DIRS1[4]);
			_meiHandsBehindBack = series_load("MEI CHIEN HANDS BEHIND BACK", -1, nullptr);
			setGlobals3(_meiHandsBehindBack, 1, 17);
			sendWSMessage_3840000(_mcTrekMach, 4);
			_guessHotspotName = Common::String("MEI CHEN      ");
			_unkArray[_unkArrayIndex] = 0;
			kernel_timing_trigger(imath_ranged_rand(1200, 1800), 5, nullptr);
			// CHECKME: CouldMakeMem();
		}
		break;

	case 2:
		player_update_info(_mcTrekMach, &_G(player_info));
		if (-_G(game_buff_ptr)->x1 < _G(player_info).x) {
			if (-_G(game_buff_ptr)->x1 + 639 <= _G(player_info).x) {
				_field68 = 1;
				series_unload(_meiHandsBehindBack);
				series_load(S8_SHADOW_NAMES2[0], S8_SHADOW_DIRS2[0], nullptr);
				series_load(S8_SHADOW_NAMES2[1], S8_SHADOW_DIRS2[1], nullptr);
				series_load(S8_SHADOW_NAMES2[2], S8_SHADOW_DIRS2[2], nullptr);
				int32 varX = daemonSub1(_G(player_info).x, true);
				ws_walk(_mcTrekMach, varX, 400, nullptr, 3, 11, true);
			} else {
				kernel_timing_trigger(60, 2, nullptr);
			}
		} else {
			_field68 = 1;
			series_unload(_meiHandsBehindBack);
			series_load(S8_SHADOW_NAMES2[0], S8_SHADOW_DIRS2[0]);
			series_load(S8_SHADOW_NAMES2[1], S8_SHADOW_DIRS2[1]);
			series_load(S8_SHADOW_NAMES2[2], S8_SHADOW_DIRS2[2]);
			int32 varX = daemonSub1(_G(player_info).x, false);
			ws_walk(_mcTrekMach, varX, 400, nullptr, 3, 1, true);
		}

		break;

	case 3:
		series_unload(S8_SHADOW_DIRS2[0]);
		series_unload(S8_SHADOW_DIRS2[1]);
		series_unload(S8_SHADOW_DIRS2[2]);
		_meiHandsBehindBack = series_load("MEI CHIEN HANDS BEHIND BACK", -1, nullptr);
		setGlobals3(_meiHandsBehindBack, 1, 17);
		sendWSMessage_3840000(_mcTrekMach, 4);
		break;

	case 4:
		_field68 = 0;
		hotspot_set_active(_G(currentSceneDef).hotspots, _guessHotspotName.c_str(), true);
		kernel_trigger_dispatchx(kernel_trigger_create(2));
		break;

	case 5:
		if (g_engine->game_camera_panning() || _field68 != 0 || !player_commands_allowed() || !checkStrings()) {
			kernel_timing_trigger(60, 5, nullptr);
		} else {
			player_set_commands_allowed(false);
			intr_cancel_sentence();
			switch (imath_ranged_rand(1, 4)) {
			case 1:
				digi_play("950_s15", 2, 255, 6, -1);
				break;
			case 2:
				digi_play("950_s16", 2, 255, 6, -1);
				break;
			case 3:
				digi_play("950_s17", 2, 255, 6, -1);
				break;
			default:
				digi_play("950_s18", 2, 255, 6, -1);
				break;
			}
		}
		break;

	case 6:
		player_update_info(_G(my_walker), &_G(player_info));
		switch (_G(player_info).facing) {
		case 1:
		case 2:
		case 3:
		case 4:
			ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, 7, 3, true);
			_guessFacing = 3;
			break;
		case 5:
			kernel_timing_trigger(30, 7, "phantom reaction");
			_guessFacing = 5;
			break;
		case 7:
			kernel_timing_trigger(30, 7, "phantom reaction");
			_guessFacing = 7;
			break;
		case 8:
		case 9:
		case 10:
		case 11:
			ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, 7, 9, true);
			_guessFacing = 9;
			break;

		default:
			break;
		}
		break;

	case 7:
		_counter1 = 0;
		series_unload(_meiHandsBehindBack);
		_mctdSerie = series_load("mctd82a", -1, nullptr);
		setGlobals3(_mctdSerie, 1, 22);
		sendWSMessage_3840000(_mcTrekMach, 11);
		if (_guessFacing == 3 || _guessFacing == 9) {
			_ripLooksAround = series_load("RIP POS 3 LOOK AROUND", -1, nullptr);
			setGlobals3(_ripLooksAround, 1, 20);
		} else {
			_ripLooksAround = series_load("RIP LOOKS AROUND IN AWE", -1, nullptr);
			setGlobals3(_ripLooksAround, 1, 14);
		}

		sendWSMessage_3840000(_G(my_walker), 10);
		_case7RandVal = imath_ranged_rand(1, 4);
		switch (_case7RandVal) {
		case 1:
			digi_play("COM052", 1, 255, 8, 997);
			break;

		case 2:
			digi_play("COM054", 1, 255, 8, 997);
			break;

		case 3:
			digi_play("COM056", 1, 255, 8, 997);
			break;

		case 4:
			digi_play("COM057", 1, 255, 8, 997);
			break;

		default:
			break;
		}
		break;

	case 8:
		switch (_case7RandVal) {
		case 1:
			digi_play("COM053", 1, 255, 1, 997);
			break;

		case 3:
			digi_play("COM055", 1, 255, -1, 997);
			break;

		case 4:
			digi_play("COM058", 1, 255, -1, 997);
			break;

		default:
			break;
		}
		break;

	case 10:
		kernel_timing_trigger(imath_ranged_rand(90, 120), 12, nullptr);
		break;

	case 11:
		kernel_timing_trigger(imath_ranged_rand(90, 120), 13, nullptr);
		break;

	case 12:
		if (_guessFacing == 3 || _guessFacing == 9) {
			setGlobals3(_ripLooksAround, 19, 1);
		} else {
			setGlobals3(_ripLooksAround, 13, 1);
		}

		sendWSMessage_3840000(_G(my_walker), 14);
		break;

	case 13:
		setGlobals3(_mctdSerie, 22, 1);
		sendWSMessage_3840000(_mcTrekMach, 14);
		break;

	case 14:
		++_counter1;
		if (_counter1 == 2) {
			player_set_commands_allowed(true);
			ws_demand_facing(_G(my_walker), _guessFacing);
			series_unload(_ripLooksAround);
			series_unload(_mctdSerie);
			_meiHandsBehindBack = series_load("MEI CHIEN HANDS BEHIND BACK", -1, nullptr);
			setGlobals3(_mctdSerie, 17, 17);
			sendWSMessage_3840000(_mcTrekMach, -1);
			kernel_timing_trigger(imath_ranged_rand(7200, 14400), 5, nullptr);
		}
		break;

	case 15:
		switch (_currentRoom) {
		case 804:
			ws_walk(_G(my_walker), _dword1948BC[_coordArrayId], 321, nullptr, 16, 3, true);
			break;

		case 814:
			ws_walk(_G(my_walker), _dword1948D8[_coordArrayId], 321, nullptr, 16, 3, true);
			break;

		case 824:
			ws_walk(_G(my_walker), _dword1948F4[_coordArrayId], 321, nullptr, 16, 3, true);
			break;

		case 834:
			ws_walk(_G(my_walker), _dword194910[_coordArrayId], 321, nullptr, 16, 3, true);
			break;

		case 844:
			ws_walk(_G(my_walker), _dword19492C[_coordArrayId], 321, nullptr, 16, 3, true);
			break;

		default:
			break;
		}
		break;

	case 16: {
		player_set_commands_allowed(false);
		ws_hide_walker(_G(my_walker));
		int32 retVal = subCE498(_coordArrayId);
		if (retVal > 0) {
			terminateMachine(_machArr[retVal]);
			_machArr[retVal] = nullptr;
		}

		getSeriesName(_coordArrayId, true);
		_dynSerie1 = series_load(_currentSeriesName.c_str(), -1, nullptr);
		retVal = subCE52E(_coordArrayId);
		switch (retVal) {
		case 1:
			_dynSerie1Mach = series_play(_currentSeriesName.c_str(), 766, 0, 18, 5, 0, 100, 0, 0, 0, 9);
			break;

		case 2:
		case 3:
			_dynSerie1Mach = series_play(_currentSeriesName.c_str(), 766, 0, 17, 5, 0, 100, 0, 0, 0, 9);
			break;

		default:
			_dynSerie1Mach = series_play(_currentSeriesName.c_str(), 766, 16, 19, 5, 0, 100, 0, 0, 0, -1);
			break;
		}
		}
		break;

	case 17:
		digi_play("950_s31", 2, 255, -1, -1);
		_dynSerie1Mach = series_play(_currentSeriesName.c_str(), 766, 16, 19, 5, 0, 100, 0, 0, 10, -1);
		break;

	case 18:
		digi_play("950_s30", 2, 255, -1, -1);
		_dynSerie1Mach = series_play(_currentSeriesName.c_str(), 766, 16, 19, 5, 0, 100, 0, 0, 10, -1);
		break;

	case 19:
		if (subCE498(_coordArrayId) < 0) {
			kernel_timing_trigger(120, 20, nullptr);
			digi_play("950_s32", 2, 255, -1, -1);
		} else {
			kernel_timing_trigger(30, 20, nullptr);
		}
		break;

	case 20:
		terminateMachine(_dynSerie1Mach);
		series_play(_currentSeriesName.c_str(), 766, 2, 21, 5, 0, 100, 0, 0, 0, -1);
		switch (subCE52E(_coordArrayId)) {
		case 1:
			_G(flags)[V271] = 0;
			_G(flags)[V272] = 0;
			break;

		case 2:
			_G(flags)[V271] = 1;
			_G(flags)[V272] = 0;
			break;

		case 3:
			_G(flags)[V271] = 0;
			_G(flags)[V272] = 1;
			break;

		default:
			break;
		}
		break;

	case 21: {
		player_set_commands_allowed(true);
		terminateMachine(_dynSerie1Mach);
		series_unload(_dynSerie1);
		ws_unhide_walker(_G(my_walker));
		ws_demand_facing(_G(my_walker), 3);
		int32 retVal = subCE498(_coordArrayId);
		if (retVal < 0) {
			digi_play("com066", 1, 255, -1, 997);
		} else {
			getSeriesName(_coordArrayId, false);
			_machArr[retVal] = series_play(_currentSeriesName.c_str(), 768, 16, -1, 0, 0, 100, 0, 0, 0, -1);
		}
		}
		break;

	default:
		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
