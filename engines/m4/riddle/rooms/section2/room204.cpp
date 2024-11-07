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

#include "m4/riddle/rooms/section2/room204.h"
#include "m4/riddle/rooms/section2/section2.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_vmng.h"
#include "m4/gui/gui_vmng_screen.h"
#include "m4/riddle/riddle.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room204::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
}

void Room204::init() {
	digi_preload("950_s04", -1);
	digi_play_loop("950_s04", 3, 70, -1, -1);
	if (!_G(flags[V070]))
		_G(flags[V078]) = 0;

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_field4 = 0;
		_field180 = 0;
		_field184 = 0;
		_field188 = 0;

		if (!player_been_here(205) && (!_G(flags[V056]) || _G(flags[V049]) == 1)) {
			_field4 = 1;
			initWalkerSeries();
			_G(flags[V049]) = 0;
		}
	}

	_courtyardGongSeries = series_load("COURTYARD GONG", -1, nullptr);
	_malletSpriteSeries = series_load("MALLET SPRITE", -1, nullptr);
	_field68 = 0;
	_field44 = -1;
	_field48 = -1;
	_fieldC4 = -1;
	_fieldBC = -1;
	_fieldEC = -1;
	_fieldF0 = -1;
	_field104 = 0;
	_field108 = 0;

	if (inv_object_is_here("SILVER BUTTERFLY")) {
		_silverButterflyCoinMach = series_place_sprite("SILVER BUTTERFLY COIN", 0, 1280, 0, 100, 3840);
	} else {
		hotspot_set_active(_G(currentSceneDef).hotspots, "silver butterfly", false);
	}

	if (inv_object_is_here("GONG")) {
		_courtyardGongMach = series_place_sprite("COURTYARD GONG", 0, 0, 0, 100, 2457);
	} else {
		hotspot_set_active(_G(currentSceneDef).hotspots, "gong", false);
	}

	if (inv_object_is_here("MALLET")) {
		_malletSpriteMach = series_place_sprite("MALLET SPRITE", 0, 0, 0, 100, 2304);
	} else {
		hotspot_set_active(_G(currentSceneDef).hotspots, "mallet", false);
	}

	if (_G(game).previous_room == KERNEL_RESTORING_GAME) {
		if (player_been_here(205)) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "acolyte", false);
			_G(camera_reacts_to_player) = false;
		} else {
			kernel_timing_trigger(1, 578, nullptr);
			if (_field4 == 1) {
				initWalkerSeries();

				_mcMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, _fieldE0, 323, (_fieldDC == 1) ? 10 : 2, Walker::player_walker_callback, "mc walker room 204");
				sub216B2();
				if (_fieldE0 == 472) {
					sub1F6AF();
					sub1F641();
				}

				kernel_timing_trigger(1, 630, nullptr);
			}
		}
	} else if (_G(game).previous_room == 205) {
		player_set_commands_allowed(false);
		_G(camera_reacts_to_player) = false;
		hotspot_set_active(_G(currentSceneDef).hotspots, "acolyte", false);
		ws_demand_location(_G(my_walker), 421, 330);
		ws_demand_facing(_G(my_walker), 3);
		ws_hide_walker(_G(my_walker));
		series_play("204 leap down", 3840, 0, 15, 5, 0, 100, 0, 0, 0, -1);
	} else if (keyCheck() && !player_been_here(205)) {
		_G(camera_reacts_to_player) = false;
		player_set_commands_allowed(false);
		ws_demand_location(_G(my_walker), 600, 334);
		ws_demand_facing(_G(my_walker), 9);
		sendWSMessage_10000(_G(my_walker), 424, 331, 9, 9, true);
		_mcMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 620, 340, 9, Walker::player_walker_callback, "mc");
		sendWSMessage_10000(_mcMach, 450, 340, 9, -1, true);
		kernel_timing_trigger(60, 5, nullptr);
	} else {
		_fieldDC = 0;
		_field40 = 0;
		player_set_commands_allowed(false);
		if (!_G(flags[V070])) {
			digi_preload("204_S02", -1);
			_mcMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 1864, 334, 9, Walker::player_walker_callback, "mc walker room 204");
			DisposePath(_mcMach->walkPath);
			_ripDropsSeries = series_load("RIP DROPS", -1, nullptr);
			player_set_commands_allowed(false);
			ws_demand_location(_G(my_walker), 1864, 334);
			ws_demand_facing(_G(my_walker), 3);

			_G(flags[V070]) = 1;
			_G(flags[V068]) = 1;
			_G(flags[V078]) = 1;

			int32 status;
			ScreenContext *game_buff_ptr = vmng_screen_find(_G(gameDrawBuff), &status);
			MoveScreenDelta(game_buff_ptr, -1280, 0);
			kernel_timing_trigger(1, 500, nullptr);
		} else {
			_G(flags[V068]) = 1;
			if (!_field4) {
				ws_demand_location(_G(my_walker), 1864, 334);
				ws_demand_facing(_G(my_walker), 9);
				int32 status;
				ScreenContext *game_buff_ptr = vmng_screen_find(_G(gameDrawBuff), &status);
				MoveScreenDelta(game_buff_ptr, -1280, 0);
				kernel_timing_trigger(1, 708, nullptr);
			} else if (_G(flags[V049]) == 1) {
				_G(flags[V049]) = 0;
				_G(flags[V078]) = 2;
				ws_demand_location(_G(my_walker), 1864, 334);
				ws_demand_facing(_G(my_walker), 9);
				_mcMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 1864, 334, 4, Walker::player_walker_callback, "mc walker room 204");
				kernel_timing_trigger(1, 536, nullptr);
				int32 status;
				ScreenContext *game_buff_ptr = vmng_screen_find(_G(gameDrawBuff), &status);
				MoveScreenDelta(game_buff_ptr, -1280, 0);
			} else {
				ws_demand_location(_G(my_walker), 1864, 334);
				ws_demand_facing(_G(my_walker), 9);
				int32 status;
				ScreenContext *game_buff_ptr = vmng_screen_find(_G(gameDrawBuff), &status);
				MoveScreenDelta(game_buff_ptr, -1280, 0);
				kernel_timing_trigger(1, 708, nullptr);
				switch (imath_ranged_rand(1, 4)) {
				case 1:
					_fieldE4 = 1663;
					break;

				case 2:
					_fieldE4 = 1576;
					break;

				case 3:
					_fieldE4 = 1494;
					break;

				case 4:
					_fieldE4 = 1412;
					break;

				default:
					break;
				}

				_mcMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, _fieldE4, 323, 2, Walker::player_walker_callback, "mc walker room 204");
				sub216B2();
				kernel_timing_trigger(1, 630, nullptr);
				kernel_timing_trigger(1, 578, nullptr);
			}
		}
	}
}

void Room204::pre_parser() {
}

void Room204::parser() {
}

void Room204::daemon() {
}

void Room204::initWalkerSeries() {
	ws_walk_load_walker_series(S8_SHADOW_DIRS2, S8_SHADOW_NAMES2, false);
	ws_walk_load_walker_series(S8_SHADOW_DIRS1, S8_SHADOW_NAMES1, false);
}

void Room204::sub216B2() {
}

void Room204::sub1F6AF() {
}

void Room204::sub1F641() {
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
