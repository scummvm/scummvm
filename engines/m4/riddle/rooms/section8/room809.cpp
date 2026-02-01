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

#include "m4/riddle/rooms/section8/room809.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/riddle.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room809::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
}

void Room809::init() {
	if (_G(game).previous_room == KERNEL_RESTORING_GAME || _G(flags)[V263])
		digi_preload("950_s29", -1);

	_field20Fl = false;
	ws_walk_load_shadow_series(S8_SHADOW_DIRS1, S8_SHADOW_NAMES1);
	ws_walk_load_walker_series(S8_SHADOW_DIRS2, S8_SHADOW_NAMES2, false);
	_mcHandsBehindBackSeries = series_load("MEI CHIEN HANDS BEHIND BACK", -1, nullptr);
	_ripTalkerPos5Series = series_load("RIP TALKER POS 5", -1, nullptr);

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_ripTrekHeadTurnPos5Series = series_load("RIP TREK HEAD TURN POS5", -1, nullptr);
	}

	series_play("809fir1a", 4095, 0, -1, 7, -1, 100, 0, 0, 0, -1);
	series_play("809fir1b", 4095, 0, -1, 7, -1, 100, 0, 0, 0, -1);
	series_play("809fir1c", 4095, 0, -1, 7, -1, 100, 0, 0, 0, -1);
	series_play("809fir1d", 4095, 0, -1, 7, -1, 100, 0, 0, 0, -1);
	series_play("809fir1e", 4095, 0, -1, 7, -1, 100, 0, 0, 0, -1);
	series_play("809fir1f", 4095, 0, -1, 7, -1, 100, 0, 0, 0, -1);
	series_play("809fir1g", 0, 0, -1, 7, -1, 100, 0, 0, 0, -1);
	series_play("809fir2a", 0, 0, -1, 7, -1, 100, 0, 0, 0, -1);
	series_play("809fir2b", 4095, 0, -1, 7, -1, 100, 0, 0, 0, -1);
	series_play("809fir2c", 4095, 0, -1, 7, -1, 100, 0, 0, 0, -1);
	series_play("809fir2d", 4095, 0, -1, 7, -1, 100, 0, 0, 0, -1);
	series_play("809fir2e", 4095, 0, -1, 7, -1, 100, 0, 0, 0, -1);
	series_play("809fir2f", 4095, 0, -1, 7, -1, 100, 0, 0, 0, -1);
	series_play("809fir2g", 4095, 0, -1, 7, -1, 100, 0, 0, 0, -1);
	series_play("809fir3a", 4095, 0, -1, 7, -1, 100, 0, 0, 0, -1);
	series_play("809fir3b", 4095, 0, -1, 7, -1, 100, 0, 0, 0, -1);
	series_play("809fir3c", 4095, 0, -1, 7, -1, 100, 0, 0, 0, -1);

	if (_G(game).previous_room == KERNEL_RESTORING_GAME) {
		player_set_commands_allowed(true);
		series_unload(S8_SHADOW_DIRS2[3]);
		series_unload(S8_SHADOW_DIRS2[4]);
		series_unload(S8_SHADOW_DIRS1[3]);
		series_unload(S8_SHADOW_DIRS1[4]);

		hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN ", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN  ", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN   ", false);

		_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1,
			_mcPosX, 317, _mcFacing, Walker::player_walker_callback, "mc_trek");
		setGlobals3(_mcHandsBehindBackSeries, 1, 17);
		sendWSMessage_3840000(_mcTrekMach, 38);
		_enableHotspotName = "MEI CHEN     ";
		_byte1A1990[_field24_index] = 0;

		kernel_timing_trigger(60, 36, "verify mc's position");
		if (inv_object_in_scene("two soldiers' shields", 809)) {
			ws_hide_walker(_G(my_walker));
			_809rp01Mach = series_show("809rp01", 256, 0, -1, -1, 179, 100, 0, 0);
			_G(kernel).trigger_mode = KT_PARSE;
			kernel_timing_trigger(400, 51, nullptr);
			_G(kernel).trigger_mode = KT_PREPARSE;
		}

		digi_play_loop("950_s29", 3, 48, -1, -1);

	} else {
		player_set_commands_allowed(false);
		ws_demand_facing(_G(my_walker), 3);
		ws_demand_location(_G(my_walker), 90, 317);
		_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 70, 317, 3, Walker::player_walker_callback, "mc_trek");

		_field24_index = 0;
		_mcPosX = 160;

		if (_G(flags)[V263] == 0) {
			_G(flags)[V263] = 1;

			moveScreen(1280, 0);
			_G(camera_reacts_to_player) = false;
			kernel_timing_trigger(600, 3, "cutscene pan");
			series_unload(S8_SHADOW_DIRS2[0]);
			series_unload(S8_SHADOW_DIRS2[1]);
			series_unload(S8_SHADOW_DIRS2[3]);
			series_unload(S8_SHADOW_DIRS2[4]);
			series_unload(S8_SHADOW_DIRS1[0]);
			series_unload(S8_SHADOW_DIRS1[1]);
			series_unload(S8_SHADOW_DIRS1[3]);
			series_unload(S8_SHADOW_DIRS1[4]);

			digi_preload("809m01");
			digi_preload("809r01");
			digi_preload("809_s01");

			_809hallSeries = series_load("809hall", -1, nullptr);
			digi_play("lostcity", 3, 255, -1, 809);

		} else {
			ws_walk(_G(my_walker), 150, 315, nullptr, 0, 3, true);
			DisposePath(_mcTrekMach->walkPath);
			_mcTrekMach->walkPath = CreateCustomPath(106, 318, 160, 323, -1);
			ws_custom_walk(_mcTrekMach, 3, 2, true);
			digi_play_loop("950_s29", 3, 48, -1, -1);
		}
	}

}

void Room809::pre_parser() {
	if (player_said("look at", "gate")) {
		_playerFacing = 9;
	} else if (player_said("look at", "mausoleum") || player_said("go", "west")) {
		_playerFacing = 3;
	} else if (player_said("look at", "urn")) {
		_playerFacing = -1;
	} else {
		player_update_info(_G(my_walker), &_G(player_info));
		_playerFacing = _G(player_info).x >= _G(player).click_x ? 9 : 3;
	}

	if (!player_said("spleen") || inv_object_in_scene("two soldiers' shields", 809)) {
		_G(player).resetWalk();
	}
}

void Room809::parser() {
	_G(player).command_ready = false;
	if (inv_player_has(_G(player).noun)) {
		_G(player).command_ready = true;
		return;
	}

	switch (_G(kernel).trigger) {
	case -1:
		if (checkSaid()) {
			int32 destX;
			int32 destY;
			player_update_info(_G(my_walker), &_G(player_info));
			if (_G(player_info).x >= _G(player).click_x) {
				destX = imath_min(_G(player_info).x, _G(player).click_x + 20);
				destX = imath_max(destX, 145);
				if (_G(player).click_y < 315) {
					if (_playerFacing < 0)
						_playerFacing = 11;
					destY = 315;
				} else if (_G(player).click_y <= 321) {
					destY = _G(player).click_y;
				} else {
					if (_playerFacing < 0)
						_playerFacing = 7;

					destY = 321;
				}
			} else {
				if (_G(player).click_x >= 1340)
					destX = 1349;
				else
					destX = imath_max(_G(player_info).x, _G(player).click_x - 20);

				if (_G(player).click_y < 315) {
					if (_playerFacing < 0)
						_playerFacing = 1;
					destY = 315;
				} else if (_G(player).click_y <= 321)
					destY = _G(player).click_y;
				else {
					if (_playerFacing < 0)
						_playerFacing = 5;

					destY = 321;
				}
			}
			ws_walk(_G(my_walker), destX, destY, nullptr, 1, _playerFacing, true);
		} else {
			kernel_trigger_dispatchx(kernel_trigger_create(1));
		}

		break;

	case 1: {
		player_update_info();
		if (_G(player_info).x >= 1340 && -_G(game_buff_ptr)->x1 < 1259) {
			g_engine->camera_shift_xy(1259, 0);
		}

		int32 opcode = -1;

		if (player_said_any("look", "look at"))
			opcode = 1;
		else if (player_said_any("gear", "use"))
			opcode = 0;
		else if (player_said("take"))
			opcode = 2;
		else if (player_said("talk to"))
			opcode = 3;
		else if (player_said_any("walk to", "walk", "spleen"))
			opcode = 5;
		else if (player_said("journal"))
			opcode = 4;
		else if (player_said("go"))
			opcode = 6;

		switch (opcode) {
		case 1:
			if (player_said(" ")) {
				digi_play("809r02", 1, 255, -1, -1);
			} else if (player_said("mountains")) {
				digi_play("809r34", 1, 255, -1, -1);
			} else if (player_said("diorama")) {
				digi_play("809r02", 1, 255, -1, -1);
			} else if (player_said("lake")) {
				digi_play("809r03", 1, 255, -1, -1);
			} else if (player_said("mausoleum")) {
				digi_play("809r04", 1, 255, -1, -1);
			} else if (player_said("river")) {
				digi_play("809r05", 1, 255, -1, -1);
			} else if (player_said("buildings")) {
				digi_play("809r06", 1, 255, -1, -1);
			} else if (player_said("dragon head")) {
				digi_play("809r07", 1, 255, _G(flags[V101]) ? 39 : -1, -1);
			} else if (player_said("soldier ")) {
				digi_play("809r08", 1, 255, -1, -1);
			} else if (player_said("lit urn")) {
				digi_play("com060", 1, 255, -1, 997);
			} else if (player_said("unlit urn")) {
				digi_play("com061", 1, 255, -1, 997);
			} else if (player_said("urn")) {
				digi_play("809r33", 1, 255, -1, -1);
			} else if (player_said("weir")) {
				digi_play("809r10", 1, 255, -1, -1);
			} else if (player_said("gate")) {
				digi_play("809r09", 1, 255, -1, -1);
			} else if (player_said_any("mei chen", "mei chen ", "mei chen  ") || (player_said("mei chen   ") && !inv_object_in_scene("two soldiers' shields", 809))) {
				kernel_trigger_dispatchx(kernel_trigger_create(40));
			} else if (player_said("mei chen   ")) {
				digi_play("809r11", 1, 255, -1, -1);
			} else {
				_G(player).command_ready = true;
			}
			break;

		case 2:
			_G(player).command_ready = true;
			break;

		case 3:
			if (inv_object_in_scene("two soldiers' shields", 809)) {
				player_set_commands_allowed(false);
				inv_give_to_player("two soldiers' shields");
				terminateMachine(_809rp01Mach);
				_809rp01Mach = series_play("809rp01", 256, 18, 52, 5, 0, 100, 0, 0, 0, -1);
			} else {
				player_update_info(_G(my_walker), &_G(player_info));
				_playerDestX = _G(player_info).x;
				_playerDestY = _G(player_info).y;

				player_update_info(_mcTrekMach, &_G(player_info));

				if (_playerDestX <= _G(player_info).x) {
					if (_G(player_info).x - 15 <= _playerDestX) {
						ws_walk(_G(my_walker), _playerDestX, _playerDestY, nullptr, 42, 5, true);
					} else {
						ws_walk(_G(my_walker), _G(player_info).x - 15, 315, nullptr, 42, 5, true);
					}
				} else if (_G(player_info).x + 15 >= _playerDestX) {
					ws_walk(_G(my_walker), _playerDestX, _playerDestY, nullptr, 42, 7, true);
				} else {
					ws_walk(_G(my_walker), _G(player_info).x + 15, 315, nullptr, 42, 7, true);
				}
			}
			break;

		case 4:
			digi_play("809r32", 1, 255, -1, -1);
			break;

		case 5:
			if (inv_object_in_scene("two soldiers' shields", 809)) {
				player_set_commands_allowed(false);
				inv_give_to_player("two soldiers' shields");
				terminateMachine(_809rp01Mach);
				_809rp01Mach = series_plain_play("809rp01", 1, 2, 100, 0, 5, 52, true);
			}

			break;
		case 6:
			if (player_said("east")) {
				kernel_trigger_dispatchx(kernel_trigger_create(53));
			} else if (player_said("west")) {
				kernel_trigger_dispatchx(kernel_trigger_create(65));
			}

			break;

		case 0:
		default:
			if (player_said("lake") && inv_player_has(_G(player).verb)) {
				if (player_said("soldier's shield")) {
					digi_play("809r18", 1, 255, -1, -1);
				} else if (player_said("two soldiers' shields")) {
					ws_walk(_G(my_walker), 1346, 318, nullptr, 48, 3, true);
				} else if (player_said("farmer's shovel")) {
					ws_walk(_G(my_walker), 1346, 318, nullptr, 44, 3, true);
				} else {
					digi_play("809r16", 1, 255, -1, -1);
				}
			} else if (player_said_any("mei chen", "mei chen ", "mei chen  ", "mei chen   ") && inv_player_has(_G(player).verb)) {
				digi_play("com017", 1, 255, -1, 997);
			} else {
				_G(player).command_ready = true;
			}
			break;
		}
		
		}
		break;

	case 39:
		_G(flags[V101]) = 1;
		digi_play("809r07a", 1, 255, -1, -1);
		break;

	case 40:
		player_update_info(_G(my_walker), &_G(player_info));
		_playerFacing = _G(player_info).facing;
		_playerDestX = _G(player_info).x;
		_playerDestY = _G(player_info).y;

		player_update_info(_mcTrekMach, &_G(player_info));

		if (_G(player_info).x >= _playerDestX) {
			if (_G(player_info).x - _playerDestX <= 30) {
				ws_walk(_G(my_walker), _playerDestX, _playerDestY, nullptr, 41, 5, true);
			} else {
				ws_walk(_G(my_walker), _playerDestX, _playerDestY, nullptr, 41, 4, true);
			}
		} else if (_playerDestX - _G(player_info).x <= 30) {
			ws_walk(_G(my_walker), _playerDestX, _playerDestY, nullptr, 41, 7, true);
		} else {
			ws_walk(_G(my_walker), _playerDestX, _playerDestY, nullptr, 41, 8, true);
		}

		break;

	case 41:
		digi_play("809r11", 1, 255, -1, -1);
		break;

	case 42:
		player_set_commands_allowed(false);
		setGlobals1(_ripTalkerPos5Series, 1, 4, 1, 4, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0);
		sendWSMessage_110000(_G(my_walker), -1);
		switch (imath_ranged_rand(1, 4)) {
		case 1:
			digi_play("com034", 1, 255, 43, 997);
			break;

		case 2:
			digi_play("com035", 1, 255, 43, 997);
			break;

		case 3:
			digi_play("com036", 1, 255, 43, 997);
			break;

		case 4:
		default:
			digi_play("com037", 1, 255, 43, 997);
			break;
		}

		break;

	case 43:
		player_set_commands_allowed(true);
		sendWSMessage_150000(_G(my_walker), -1);
		switch (imath_ranged_rand(1, 4)) {
		case 1:
			digi_play("com038", 1, 255, -1, 997);
			break;

		case 2:
			digi_play("com039", 1, 255, -1, 997);
			break;

		case 3:
			digi_play("com040", 1, 255, -1, 997);
			break;

		case 4:
		default:
			digi_play("com041", 1, 255, -1, 997);
			break;
		}

		break;

	case 44:
		player_set_commands_allowed(false);
		_G(flags[V103]) = 1;
		if (-_G(game_buff_ptr)->x1 < 1259) {
			g_engine->camera_shift_xy(1259, 0);
		}

		ws_hide_walker(_G(my_walker));
		_809hallSeries = series_load("809rp03", -1, nullptr);
		series_play("809rp03", 0, 0, 45, 5, 0, 100, 0, 0, 0, 104);

		break;

	case 45:
		series_play("809rp03", 0, 0, 46, 5, 0, 100, 0, 0, 105, -1);
		digi_play("809_s02", 2, 255, -1, -1);

		break;

	case 46:
		series_play("809rp03", 0, 2, 47, 5, 0, 100, 0, 0, 0, -1);
		digi_play("809R17", 1, 255, -1, -1);

		break;

	case 47:
		player_set_commands_allowed(true);
		series_unload(_809hallSeries);
		ws_unhide_walker(_G(my_walker));
		ws_demand_facing(_G(my_walker), 3);

		break;

	case 48:
		player_set_commands_allowed(false);
		if (-_G(game_buff_ptr)->x1 < 1259) {
			g_engine->camera_shift_xy(1259, 0);
		}

		ws_hide_walker(_G(my_walker));
		series_play("809rp01", 256, 0, 49, 5, 0, 100, 0, 0, 0, 169);

		break;


	case 49:
		_809rp01Mach = series_play("809rp01", 256, 16, 50, 5, 0, 100, 0, 0, 170, -1);
		digi_play("809_s06", 2, 255, -1, -1);

		break;

	case 50:
		player_set_commands_allowed(true);
		inv_move_object("two soldiers' shields", 809);
		kernel_timing_trigger(imath_ranged_rand(360, 540), 51, nullptr);

		break;

	case 51:
		if (!_field20Fl && inv_object_in_scene("two soldiers' shields", 809)) {
			terminateMachine(_809rp01Mach);
			_809rp01Mach = series_play("809shufl", 0, 17, -1, 7, 1, 100, 0, 0, 0, -1);
			digi_play("809_s03", 2, 255, -1, -1);
			kernel_timing_trigger(imath_ranged_rand(360, 540), 51, nullptr);
		}

		break;

	case 52:
		player_set_commands_allowed(true);
		terminateMachine(_809rp01Mach);
		ws_unhide_walker(_G(my_walker));
		ws_demand_facing(_G(my_walker), 3);
		ws_demand_location(_G(my_walker), 1346, 318);
		// The load is just used to get the SeriesId if already loaded, to unload it. So it's normal there's a load and unload one after the other, so not remove.
		_809hallSeries = series_load("809rp01", -1, nullptr);
		series_unload(_809hallSeries);

		kernel_trigger_dispatchx(kernel_trigger_create(1));

		break;

	case 53:
		player_set_commands_allowed(false);
		player_update_info(_mcTrekMach, &_G(player_info));
		if (_G(player_info).x < 1265) {
			kernel_timing_trigger(30, 53, nullptr);
		} else if (inv_object_in_scene("two soldiers' shields", 809)) {
			_field20Fl = true;
			series_unload(0);
			series_unload(1);
			series_unload(3);
			series_unload(4);

			digi_preload("809_s04", -1);
			digi_preload("809m05", -1);
			digi_preload("809r19", -1);
			digi_preload("809_s05", -1);

			// The load is just used to get the SeriesId if already loaded, to unload it. So it's normal there's a load and unload one after the other, so not remove.
			_809hallSeries = series_load("809rp01", -1, nullptr);
			series_unload(_809hallSeries);
			_809hallSeries = series_load("809shufl", -1, nullptr);
			series_unload(_809hallSeries);

			terminateMachine(_809rp01Mach);
			_809crossMach = series_stream("809cross", 5, 0, 56);
			series_stream_break_on_frame(_809crossMach, 119, 55);
			digi_play("809_s04", 2, 255, -1, -1);
			digi_play("809M05", 1, 255, 54, -1);
		} else if (_G(flags[V102])) {
			digi_play("809M02", 1, 255, 64, -1);
		} else {
			_G(flags[V102]) = 1;
			if (_G(flags[V103])) {
				digi_play("809M02", 1, 255, 61, -1);
			} else {
				digi_play("809M02", 1, 255, 59, -1);
			}
		}
		break;

	case 54:
		digi_play("809r19", 1, 255, -1, -1);
		break;

	case 55:
		digi_play("809_s05", 2, 255, -1, -1);
		break;

	case 56:
		_809crossMach = series_stream("809exit", 5, 0, -1);
		series_stream_break_on_frame(_809crossMach, 49, 57);

		break;

	case 57:
		disable_player_commands_and_fade_init(58);
		break;

	case 58:
		_G(game).setRoom(810);
		break;

	case 59:
		digi_play("809m04", 1, 255, 60, -1);
		break;

	case 60:
		player_set_commands_allowed(true);
		digi_play("809r14", 1, 255, 63, -1);

		break;

	case 61:
		digi_play("809m03", 1, 255, 62, -1);
		break;

	case 62:
		player_set_commands_allowed(true);
		digi_play("809r12", 1, 255, 63, -1);

		break;

	case 63:
		player_set_commands_allowed(true);
		digi_play("809r13", 1, 255, -1, -1);

		break;

	case 64:
		player_set_commands_allowed(true);
		digi_play("809r15", 1, 255, -1, -1);

		break;

	case 65:
		ws_walk(_G(my_walker), 120, 317, nullptr, 66, -1, false);
		break;

	case 66:
		ws_walk(_G(my_walker), 90, 317, nullptr, -1, 9, true);
		disable_player_commands_and_fade_init(67);

		break;

	case 67:
		_G(game).setRoom(808);
		break;

	default:
		break;
	}
}

void Room809::daemon() {
	switch (_G(kernel).trigger) {
	case 2:
		player_set_commands_allowed(true);
		series_unload(S8_SHADOW_DIRS2[3]);
		series_unload(S8_SHADOW_DIRS2[4]);
		series_unload(S8_SHADOW_DIRS1[3]);
		series_unload(S8_SHADOW_DIRS1[4]);

		setGlobals3(_mcHandsBehindBackSeries, 1, 17);
		sendWSMessage_3840000(_mcTrekMach, 38);
		kernel_timing_trigger(60, 36, "verify mc's position");
		_enableHotspotName = "MEI CHEN";

		hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN ", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN  ", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN   ", false);

		break;

	case 3:
		g_engine->adv_camera_pan_step(2);
		g_engine->camera_shift_xy(0, 0);
		kernel_timing_trigger(60, 4, "finished pan?");

		break;

	case 4:
		if (g_engine->game_camera_panning())
			kernel_timing_trigger(60, 4, "finished pan?");
		else
			kernel_trigger_dispatchx(kernel_trigger_create(5));

		break;

	case 5:
		ws_walk(_G(my_walker), 150, 315, nullptr, -1, 3, true);
		DisposePath(_mcTrekMach->walkPath);
		_mcTrekMach->walkPath = CreateCustomPath(106, 318, 160, 323, -1);
		ws_custom_walk(_mcTrekMach, 3, 6, true);
		_G(camera_reacts_to_player) = true;
		g_engine->adv_camera_pan_step(10);
		_809MusicFadingVol = 255;

		break;

	case 6:
		if (_809MusicFadingVol <= 100) {
			kernel_trigger_dispatchx(kernel_trigger_create(7));
		} else {
			_809MusicFadingVol = imath_max(100, _809MusicFadingVol - 10);
			digi_change_panning(3, _809MusicFadingVol);
			kernel_timing_trigger(10, 6, "fade music");
		}

		break;

	case 7:
		kernel_timing_trigger(120, 8, nullptr);
		break;

	case 8:
		digi_play("809_s01", 1, 120, -1, -1);
		kernel_timing_trigger(45, 9, nullptr);

		break;

	case 9:
		player_update_info(_G(my_walker), &_G(player_info));
		ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, 10, 4, true);

		break;

	case 10:
		kernel_timing_trigger(7, 11, nullptr);
		break;

	case 11:
		player_update_info(_G(my_walker), &_G(player_info));
		ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, 12, 5, true);

		break;

	case 12:
		kernel_timing_trigger(7, 13, nullptr);
		break;

	case 13:
		player_update_info(_G(my_walker), &_G(player_info));
		ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, 14, 6, true);

		break;

	case 14:
		kernel_timing_trigger(7, 15, nullptr);
		break;

	case 15:
		player_update_info(_G(my_walker), &_G(player_info));
		ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, 16, 7, true);

		break;

	case 16:
		kernel_timing_trigger(7, 17, nullptr);
		break;

	case 17:
		player_update_info(_G(my_walker), &_G(player_info));
		ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, 18, 8, true);

		break;

	case 18:
		kernel_timing_trigger(7, 19, nullptr);
		break;

	case 19:
		player_update_info(_G(my_walker), &_G(player_info));
		ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, 20, 9, true);

		break;

	case 20:
		kernel_timing_trigger(60, 21, nullptr);
		break;

	case 21:
		_809hallMach = series_play("809hall", 0, 16, -1, 5, 0, 100, 0, 0, 0, -1);
		kernel_timing_trigger(300, 22, nullptr);

		break;

	case 22:
		terminateMachine(_809hallMach);
		_809hallMach = series_play("809hall", 0, 2, 23, 5, 0, 100, 0, 0, 0, -1);

		break;

	case 23:
		series_unload(_809hallSeries);
		kernel_timing_trigger(30, 24, nullptr);

		break;

	case 24:
		digi_play("809m01", 1, 255, 29, -1);
		kernel_timing_trigger(90, 25, nullptr);

		break;

	case 25:
		player_update_info(_G(my_walker), &_G(player_info));
		ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, -1, 8, true);
		kernel_timing_trigger(7, 26, nullptr);

		break;

	case 26:
		player_update_info(_G(my_walker), &_G(player_info));
		ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, -1, 7, true);
		kernel_timing_trigger(7, 27, nullptr);

		break;

	case 27:
		player_update_info(_G(my_walker), &_G(player_info));
		ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, -1, 6, true);
		kernel_timing_trigger(7, 28, nullptr);

		break;

	case 28:
		player_update_info(_G(my_walker), &_G(player_info));
		ws_walk(_G(my_walker), _G(player_info).x, _G(player_info).y, nullptr, -1, 5, true);

		break;

	case 29:
		setGlobals1(_ripTalkerPos5Series, 1, 4, 1, 4, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		sendWSMessage_110000(_G(my_walker), -1);
		digi_play("809r01", 1, 255, 30, -1);

		break;

	case 30:
		sendWSMessage_150000(_G(my_walker), -1);
		setGlobals3(_ripTrekHeadTurnPos5Series, 8, 12);
		sendWSMessage_3840000(_G(my_walker), -1);
		kernel_timing_trigger(120, 31, nullptr);

		break;

	case 31:
		setGlobals3(_ripTrekHeadTurnPos5Series, 12, 8);
		sendWSMessage_3840000(_G(my_walker), 32);

		break;

	case 32:
		player_update_info(_G(my_walker), &_G(player_info));
		ws_walk(_G(my_walker), _G(player_info).x + 30, _G(player_info).y, nullptr, 34, 3, true);
		series_unload(_ripTrekHeadTurnPos5Series);

		break;

	case 34:
		setGlobals3(_mcHandsBehindBackSeries, 1, 17);
		sendWSMessage_3840000(_mcTrekMach, 38);
		kernel_timing_trigger(60, 36, "verify mc's position");
		_enableHotspotName = "MEI CHEN";
		hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN ", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN  ", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN   ", false);

		kernel_timing_trigger(1, 35, nullptr);

		break;

	case 35:
		if (_809MusicFadingVol <= 0) {
			digi_stop(3);
			digi_unload("809m01");
			digi_unload("809r01");
			digi_unload("809_s01");

			digi_play_loop("950_s29", 3, 48, -1, -1);
			series_load(S8_SHADOW_NAMES2[0], S8_SHADOW_DIRS2[0] , nullptr);
			series_load(S8_SHADOW_NAMES2[1], S8_SHADOW_DIRS2[1], nullptr);
			series_load(S8_SHADOW_NAMES1[0], S8_SHADOW_DIRS1[0], nullptr);
			series_load(S8_SHADOW_NAMES1[1], S8_SHADOW_DIRS1[1], nullptr);
			player_set_commands_allowed(true);
		} else {
			--_809MusicFadingVol;
			digi_change_panning(3, _809MusicFadingVol);
			kernel_timing_trigger(1, 35, "fade music");
		}

		break;

	case 36:
		player_update_info(_mcTrekMach, &_G(player_info));

		if (-_G(game_buff_ptr)->x1 < _G(player_info).x) {
			if (639 - _G(game_buff_ptr)->x1 <= _G(player_info).x) {
				_mcTrekDestX = getMcDestX(_G(player_info).x, true);
				if (669 - _G(game_buff_ptr)->x1 < _G(player_info).x) {
					ws_demand_facing(_mcTrekMach, 11);
					ws_demand_location(_mcTrekMach, 669 - _G(game_buff_ptr)->x1, 323);
				}

				ws_walk(_mcTrekMach, _mcTrekDestX, 323, nullptr, 37, 11, true);
			}
		} else {
			_mcTrekDestX = getMcDestX(_G(player_info).x, false);
			if (-30 - _G(game_buff_ptr)->x1 > _G(player_info).x) {
				ws_demand_facing(_mcTrekMach, 1);
				ws_demand_location(_mcTrekMach, -30 - _G(game_buff_ptr)->x1, 323);
			}

			ws_walk(_mcTrekMach, _mcTrekDestX, 323, nullptr, 37, 1, true);
		}

		kernel_timing_trigger(60, 36, "verify mc's position");

		break;

	case 37:
		setGlobals3(_mcHandsBehindBackSeries, 1, 17);
		sendWSMessage_3840000(_mcTrekMach, 38);

		break;

	case 38:
		hotspot_set_active(_G(currentSceneDef).hotspots, _enableHotspotName, true);
		break;

	default:
		break;
	}
}

void Room809::syncGame(Common::Serializer &s) {
	s.syncAsSint32LE(_mcPosX);
	s.syncAsSint32LE(_mcFacing);
	s.syncAsSint32LE(_mcTrekDestX);
	s.syncAsSint32LE(_playerDestX);
	s.syncAsSint32LE(_playerDestY);
	s.syncAsSint32LE(_playerFacing);
}

int32 Room809::getMcDestX(int32 xPos, bool facing) {
	static const uint16 X_THRESHOLDS1[3] = { 540, 960, 1282 };
	static const uint16 X_THRESHOLDS2[4] = { 0x7fff, 160, 540, 960 };
	static const uint16 X_DESTS[5] = { 160, 540, 960, 1282 };
	int32 index;

	if (facing) {
		index = 0;
		for (; index < 3; ++index) {
			if (xPos <= X_THRESHOLDS1[index])
				break;
		}
	} else {
		index = 3;
		for (; index > 0; --index) {
			if (xPos > X_THRESHOLDS2[index])
				break;
		}
	}

	hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN", false);
	hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN ", false);
	hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN  ", false);
	hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN   ", false);

	_enableHotspotName = "MEI CHEN     ";
	_byte1A1990[index] = 0;
	_field24_index = index;
	_mcPosX = X_DESTS[index];
	_mcFacing = facing ? 11 : 1;

	return _mcPosX;
}

bool Room809::checkSaid() {
	if (player_said_any("spleen", "west", "mei chen", "mei chen ",
			"mei chen  ", "mei chen   ", "farmer's shovel",
			"two soldiers' shields")
			|| inv_object_in_scene("two soldiers' shields", 809))
		return false;

	return true;
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
