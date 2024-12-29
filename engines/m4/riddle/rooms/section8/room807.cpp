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

#include "m4/riddle/rooms/section8/room807.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/riddle.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {


// TODO : Refactor - This array is also present in walker.cpp
static const char *SAFARI_SHADOWS[5] = {
	"safari shadow 1", "safari shadow 2", "safari shadow 3",
	"safari shadow 4", "safari shadow 5"
};

void Room807::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
}

void Room807::init() {
	if (inv_object_in_scene("wooden beam", 807)) {
		_807PostMach = series_show("807post", 4095, 0, -1, -1, 0, 100, 0, 0);
	} else {
		hotspot_set_active(_G(currentSceneDef).hotspots, "wooden beam", false);
	}

	if (inv_object_in_scene("wooden post", 807)) {
		_807BeamMach = series_show("807beam", 4095, 0, -1, -1, 0, 100, 0, 0);
	} else {
		hotspot_set_active(_G(currentSceneDef).hotspots, "wooden post", false);
	}

	if (_G(flags[V274])) {
		_807DoorMach = series_show("807door", 4095, 0, -1, -1, 0, 100, 0, 0);
		hotspot_set_active(_G(currentSceneDef).hotspots, "stone block", true);
		hotspot_set_active(_G(currentSceneDef).hotspots, "corridor", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "chariot ", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "north", false);
	} else {
		hotspot_set_active(_G(currentSceneDef).hotspots, "stone block", false);

		if (player_been_here(807)) {
			_807DoorMach = series_show("807kart", 4095, 0, -1, -1, 100, 0, 0);
		}
	}

	_ripLowReachPos1Series = series_load("rip low reach pos1", -1, nullptr);
	_ripTrekHiReach2HndSeries = series_load("rip trek hi reach 2hnd", -1, nullptr);
	_ripTalkerPos5Series = series_load("RIP TALKER POS 5", -1, nullptr);
	_mctd82aSeries = series_load("mctd82a", -1, nullptr);
	_ripPos3LookAroundSeries = series_load("RIP POS 3 LOOK AROUND", -1, nullptr);
	_ripLooksAroundInAweSeries = series_load("RIP LOOKS AROUND IN AWE", -1, nullptr);

	series_play("807fire1", 4095, 0, -1, 7, -1, 100, 0, 0, 0, -1);
	series_play("807fire2", 4095, 0, -1, 7, -1, 100, 0, 0, 0, -1);

	_field34 = 0;

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		player_set_commands_allowed(true);
		digi_preload("950_s29", -1);

		if (_field38 != 0) {
			ws_demand_location(_G(my_walker), 476, 318);
			ws_demand_facing(_G(my_walker), 11);
			ws_hide_walker(_G(my_walker));
			_807Crnk2Mach = series_show("807rp05", 256, 0, -1, -1, 12, 100, 0, 0);
			player_update_info(_G(my_walker), &_G(player_info));
			_safariShadowMach = series_place_sprite(*SAFARI_SHADOWS, 0, 476, 318, _G(player_info).scale, 257);
			_G(kernel).trigger_mode = KT_DAEMON;
			kernel_trigger_dispatchx((kernel_trigger_create(6)));
			_G(kernel).trigger_mode = KT_PREPARSE;
			hotspot_set_active(_G(currentSceneDef).hotspots, "slot", false);
		} else if (inv_object_in_scene("crank", 807)) {
			_807Crnk2Mach = series_show("807crnk2", 4095, 0, -1, -1, 9, 100, 0, 0);
			hotspot_set_active(_G(currentSceneDef).hotspots, "slot", false);
		} else {
			hotspot_set_active(_G(currentSceneDef).hotspots, "crank", false);
		}

		if (_G(flags[V276]) != 0) {
			hotspot_set_active(_G(currentSceneDef).hotspots, "mei chen", false);
		} else {
			ws_walk_load_shadow_series(S8_SHADOW_DIRS1, S8_SHADOW_NAMES1);
			ws_walk_load_walker_series(S8_SHADOW_DIRS2, S8_SHADOW_NAMES2, false);
			_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 560, 400, 11, Walker::player_walker_callback, "mc_trek");
		}

		break;

	case 808:
		player_set_commands_allowed(false);
		if (inv_object_in_scene("crank", 807)) {
			_807Crnk2Mach = series_show("807crnk2", 4095, 0, -1, -1, 9, 100, 0, 0);
			hotspot_set_active(_G(currentSceneDef).hotspots, "slot", false);
		} else {
			hotspot_set_active(_G(currentSceneDef).hotspots, "crank", false);
		}

		hotspot_set_active(_G(currentSceneDef).hotspots, "mei chen", false);
		_field38 = 0;
		ws_demand_location(_G(my_walker), 273, 270);
		ws_demand_facing(_G(my_walker), 5);

		if (_G(flags[V276]) != 0) {
			ws_walk(_G(my_walker), 250, 345, nullptr, 5, 2, true);
		} else {
			ws_walk_load_walker_series(S8_SHADOW_DIRS1, S8_SHADOW_NAMES1);
			_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 295, 250, 5, Walker::player_walker_callback, "mc_trek");
			ws_walk(_mcTrekMach, 560, 400, nullptr, 5, 11, true);
			ws_walk(_G(my_walker), 250, 345, nullptr, -1, 2, true);
		}

		break;

	default:
		player_set_commands_allowed(false);
		ws_walk_load_shadow_series(S8_SHADOW_DIRS1, S8_SHADOW_NAMES1);
		ws_walk_load_walker_series(S8_SHADOW_DIRS2, S8_SHADOW_NAMES2, false);
		if (inv_object_in_scene("crank", 807)) {
			_807Crnk2Mach = series_show("807crnk2", 4095, 0, -1, -1, 9, 100, 0, 0);
			hotspot_set_active(_G(currentSceneDef).hotspots, "slot", false);
		} else {
			hotspot_set_active(_G(currentSceneDef).hotspots, "crank", false);
		}

		hotspot_set_active(_G(currentSceneDef).hotspots, "mei chen", false);
		_field38 = 0;

		if (!player_been_here(807)) {
			_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 450, 60, 1, Walker::player_walker_callback, "mc_trek");
			ws_demand_location(_G(my_walker), 366, 345);
			ws_demand_facing(_G(my_walker), 11);
			ws_hide_walker(_G(my_walker));
			digi_preload("950_S33", -1);
			digi_preload("807_S01", -1);
			digi_play("950_S33", 2, 255, -1, -1);
			_807DoorMach = series_stream("807crush", 5, 0, 0);
			series_stream_break_on_frame(_807DoorMach, 60, 3);
		} else {
			ws_demand_location(_G(my_walker), 366, 500);
			ws_demand_facing(_G(my_walker), 1);

			if (_G(flags[V276]) != 0) {
				ws_walk(_G(my_walker), 366, 345, nullptr, 5, 2, true);
			} else {
				_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 450, 600, 1, Walker::player_walker_callback, "mc_trek");
				ws_walk(_G(my_walker), 366, 345, nullptr, -1, 2, true);
				ws_walk(_mcTrekMach, 560, 400, nullptr, 5, 11, true);
			}
		}

		break;
	}

	digi_play_loop("950_s29", 2, 127, -1, -1);
}

void Room807::pre_parser() {
	// TODO Not yet implemented
}

void Room807::parser() {
	// TODO Not yet implemented
}

void Room807::daemon() {
	// TODO Not yet implemented
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
