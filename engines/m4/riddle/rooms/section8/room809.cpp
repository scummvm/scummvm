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
		_byte1A1988 = "MEI CHEN     ";
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
	// TODO Not implemented yet
}

void Room809::daemon() {
	// TODO Not implemented yet
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
