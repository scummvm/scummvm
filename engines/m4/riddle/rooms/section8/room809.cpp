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
	if (_G(game).previous_room == KERNEL_RESTORING_GAME || _G(flags[V263]))
		digi_preload("950_s29", -1);

	_field20 = 0;
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

		_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, _field28, 317, _field2C, Walker::player_walker_callback, "mc_trek");
		setGlobals3(_mcHandsBehindBackSeries, 1, 17);
		sendWSMessage_3840000(_mcTrekMach, 38);
		_enableHotspotName = "MEI CHEN     ";
		_byte1A1990[_field24] = 0;

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

		_field24 = 0;
		_field28 = 160;

		if (_G(flags[V263]) == 0) {
			_G(flags[V263]) = 1;

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
		_dword1A1998 = 9;
	} else if (player_said("look at", "mausoleum") || player_said("go", "west")) {
		_dword1A1998 = 3;
	} else if (player_said("look at", "urn")) {
		_dword1A1998 = -1;
	} else {
		player_update_info(_G(my_walker), &_G(player_info));
		_dword1A1998 = _G(player_info).x >= _G(player).click_x ? 9 : 3;
	}

	if (!player_said("spleen") || inv_object_in_scene("two soldiers' shields", 809)) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
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
	case 1:
	case 39:
	case 40:
	case 41:
	case 42:
	case 43:
	case 44:
	case 45:
	case 46:
	case 47:
	case 48:
	case 49:
	case 50:
	case 51:
	case 52:
	case 53:
	case 54:
	case 55:
	case 56:
	case 57:
	case 58:
	case 59:
	case 60:
	case 61:
	case 62:
	case 63:
	case 64:
	case 65:
	case 66:
	case 67:
	default:
		break;
	}

	
	// TODO Not implemented yet
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
				_dword1A1980_x = room809_sub1(_G(player_info).x, 1);
				if (669 - _G(game_buff_ptr)->x1 < _G(player_info).x) {
					ws_demand_facing(_mcTrekMach, 11);
					ws_demand_location(669 - _G(game_buff_ptr)->x1, 323);
				}

				ws_walk(_mcTrekMach, _dword1A1980_x, 323, nullptr, 37, 11, true);
			}
		} else {
			_dword1A1980_x = room809_sub1(_G(player_info).x, 0);
			if (-30 - _G(game_buff_ptr)->x1 > _G(player_info).x) {
				ws_demand_facing(_mcTrekMach, 1);
				ws_demand_location(-30 - _G(game_buff_ptr)->x1, 323);
			}

			ws_walk(_mcTrekMach, _dword1A1980_x, 323, nullptr, 37, 1, true);
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

int32 Room809::room809_sub1(int32 val1, int32 val2) {
	//TODO Not implemented yet

	return 0;
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
