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

#include "m4/riddle/rooms/section8/room804.h"

#include "m4/riddle/riddle.h"
#include "m4/riddle/rooms/section8/section8.h"
#include "m4/riddle/vars.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_vmng.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

int32 Room804::subCE498(int32 val1) {
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

void Room804::subD7916(machine *machine, int32 val1) {
	int32 num = val1;
	if (val1 == 0)
		num = -1;

	_G(globals)[GLB_TEMP_4] = kernel_trigger_create(num);
	sendWSMessage(0x3840000, 0, machine, 0, nullptr, 1);
}

int32 Room804::subCE52E(int32 val1) {
	if (_currentRoom == 844 && val1 == 1 && _G(flags)[V271] != 0)
		return 3;

	if (_currentRoom == 834 && val1 == 5)
		return 2;

	if (subCE498(val1) < 0)
		return 0;

	return 1;
}

void Room804::moveScreen(int32 dx, int32 dy) {
	int32 status;
	ScreenContext *game_buff_ptr = vmng_screen_find(_G(gameDrawBuff), &status);
	if (game_buff_ptr == nullptr)
		error("M4: no gameDrawBuff");

	MoveScreenAbs(game_buff_ptr, _G(kernel).letter_box_x - dx, _G(kernel).letter_box_y - dy);
}

void Room804::getSeriesName(int32 val1, bool true_or_False) {
	if (subCE498(val1) < 0)
		_currentSeriesName = Common::String::format("%ldsldf%ld", _currentRoom, val1);
	else if (true_or_False)
		_currentSeriesName = Common::String::format("%ldsldr%ld", _currentRoom, val1);
	else
		_currentSeriesName = Common::String::format("%ldslds%ld", _currentRoom, val1);
}

int32 Room804::daemonSub1(int32 dx, bool ascendingFl) {
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
	_unkArray[i] = 0;
	_field70 = i;
	_guessX = fullArr[i];

	if (ascendingFl)
		_guessIndex = 11;
	else
		_guessIndex = 1;

	return fullArr[i];
}

void Room804::init() {
	_currentRoom = 804;
	if (_G(flags)[V276]) {
		ws_walk_load_walker_series(S8_SHADOW_DIRS1, S8_SHADOW_NAMES1, false);
		ws_walk_load_walker_series(S8_SHADOW_DIRS2, S8_SHADOW_NAMES2, false);
	}

	series_plain_play("804 lit urn 1", -1, 0, 100, 32767, 7, -1, false);
	series_plain_play("804 lit urn 2", -1, 0, 100, 32767, 7, -1, false);
	_var1 = 3;
	_var2 = 1;
	_var3 = 4;
	_var4 = 6;
	_var5 = -1;

	getSeriesName(1, false);
	_machArr[0] = series_play(_currentSeriesName.c_str(), 767, 16, -1, 0, 0, 100, 0, 0, 0, -1);
	getSeriesName(4, false);
	_machArr[1] = series_play(_currentSeriesName.c_str(), 767, 16, -1, 0, 0, 100, 0, 0, 0, -1);
	getSeriesName(6, false);
	_machArr[2] = series_play(_currentSeriesName.c_str(), 767, 16, -1, 0, 0, 100, 0, 0, 0, -1);
	_machArr[3] = nullptr;
	hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN", false);
	hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN ", false);
	hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN  ", false);
	hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN   ", false);
	hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN    ", false);

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		player_set_commands_allowed(true);
		digi_preload("950_s29", -1);
		if (_G(flags)[V276] == 0) {
			_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, _guessX, 400, _guessIndex, Walker::player_walker_callback, "mc_trek");
			kernel_timing_trigger(1, 1, nullptr);
		}
		break;
	case 803:
		player_set_commands_allowed(false);
		ws_demand_facing(_G(my_walker), 3);
		ws_demand_location(_G(my_walker), -10, 325);
		if (_G(flags)[V276] == 0) {
			_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, -75, 325, 3, Walker::player_walker_callback, "mc_trek");
			_field70 = 0;
			_guessX = 100;
			_guessIndex = 1;
			ws_walk(_G(my_walker), 150, 340, nullptr, -1, 3, true);
			ws_walk(_mcTrekMach, 80, 400, nullptr, 1, 1, true);
		} else {
			ws_walk(_G(my_walker), 150, 340, nullptr, 1, 3, true);
		}
		break;
	default:
		player_set_commands_allowed(false);
		moveScreen(1280, 0);
		ws_demand_facing(_G(my_walker), 9);
		ws_demand_location(_G(my_walker), 1925, 325);
		if (_G(flags)[V276] == 0) {
			_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 2000, 325, 9, Walker::player_walker_callback, "mc_trek");
			_field70 = 4;
			_guessX = 1815;
			_guessIndex = 11;
			ws_walk(_G(my_walker), 1765, 345, nullptr, 0, 9, true);
			DisposePath(_mcTrekMach->walkPath);
			_mcTrekMach->walkPath = CreateCustomPath(1835, 325, 1815, 400, -1);
			ws_custom_walk(_mcTrekMach, 11, 1, true);
		} else {
			ws_walk(_G(my_walker), 1765, 345, nullptr, 1, 9, true);
		}

		break;
	}

	digi_play_loop("950_s29", 3, 96, -1, -1);
}

void Room804::pre_parser() {
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

void Room804::parser() {
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
				terminateMachine(_unkMach1);
				terminateMachine(_unkMach2);
				series_unload(_unkSerie1);
				series_unload(_unkSerie2);
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
			_field64 = _G(player).noun[0] - 0x30; // Thus a number between 1 and 7
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
				_currentSeriesName = Common::String::format("%ldr%02ld", _currentRoom, _G(player).noun[0] - 0x2B);
				break;
			case 824:
			case 834:
				_currentSeriesName = Common::String::format("%ldr%02ld", _currentRoom, _G(player).noun[0] - 0x2F);
				break;
			case 844:
				_currentSeriesName = Common::String::format("%ldr%02ld", _currentRoom, _G(player).noun[0] - 0x2E);
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
			case 10:
			case 20:
			case 30:
				break;

			default:
				break;

			}
		} // player_said("fallen soldier") || player_said("fallen horse")

		break; // caty case 1

	case 2:
	case 3:
	case 4:
	case 5:
	default:
		break;
	}

}

void Room804::daemon() {
	int32 _dword1948BC[] = {1815, 1080, 1157, 1241, 1319, 1402, 1478};
	int32 _dword1948D8[] = {1561, 874, 949, 1034, 1117, 1198, 1276};
	int32 _dword1948F4[] = {1355, 624, 702, 787, 865, 949, 1027};
	int32 _dword194910[] = {1110, 278, 351, 439, 519, 601, 679};
	int32 _dword19492C[] = {756, 1219, 1301, 1383, 1466, 1548, 1626, 1701};

	switch (_G(kernel).trigger) {
	case 1:
		_byte1A19BC = 1;
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
			subD7916(_mcTrekMach, 4);
			_guessHotspotName = Common::String("MEI CHEN      ");
			_unkArray[_field70] = 0;
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
		subD7916(_mcTrekMach, 4);
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
		subD7916(_mcTrekMach, 11);
		if (_guessFacing == 3 || _guessFacing == 9) {
			_ripLooksAround = series_load("RIP POS 3 LOOK AROUND", -1, nullptr);
			setGlobals3(_ripLooksAround, 1, 20);
		} else {
			_ripLooksAround = series_load("RIP LOOKS AROUND IN AWE", -1, nullptr);
			setGlobals3(_ripLooksAround, 1, 14);
		}

		subD7916(_G(my_walker), 10);
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

		subD7916(_G(my_walker), 14);
		break;

	case 13:
		setGlobals3(_mctdSerie, 22, 1);
		subD7916(_mcTrekMach, 14);
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
			subD7916(_mcTrekMach, -1);
			kernel_timing_trigger(imath_ranged_rand(7200, 14400), 5, nullptr);
		}
		break;

	case 15:
		switch (_currentRoom) {
		case 804:
			ws_walk(_G(my_walker), _dword1948BC[_field64], 321, nullptr, 16, 3, true);
			break;

		case 814:
			ws_walk(_G(my_walker), _dword1948D8[_field64], 321, nullptr, 16, 3, true);
			break;

		case 824:
			ws_walk(_G(my_walker), _dword1948F4[_field64], 321, nullptr, 16, 3, true);
			break;

		case 834:
			ws_walk(_G(my_walker), _dword194910[_field64], 321, nullptr, 16, 3, true);
			break;

		case 844:
			ws_walk(_G(my_walker), _dword19492C[_field64], 321, nullptr, 16, 3, true);
			break;

		default:
			break;
		}
		break;

	case 16: {
		player_set_commands_allowed(false);
		ws_hide_walker(_G(my_walker));
		int32 retVal = subCE498(_field64);
		if (retVal > 0) {
			terminateMachine(_machArr[retVal]);
			_machArr[retVal] = nullptr;
		}

		getSeriesName(_field64, true);
		_dynSerie1 = series_load(_currentSeriesName.c_str(), -1, nullptr);
		retVal = subCE52E(_field64);
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
		if (subCE498(_field64) < 0) {
			kernel_timing_trigger(120, 20, nullptr);
			digi_play("950_s32", 2, 255, -1, -1);
		} else {
			kernel_timing_trigger(30, 20, nullptr);
		}
		break;

	case 20:
		terminateMachine(_dynSerie1Mach);
		series_play(_currentSeriesName.c_str(), 766, 2, 21, 5, 0, 100, 0, 0, 0, -1);
		switch (subCE52E(_field64)) {
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
		int32 retVal = subCE498(_field64);
		if (retVal < 0) {
			digi_play("com066", 1, 255, -1, 997);
		} else {
			getSeriesName(_field64, false);
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
