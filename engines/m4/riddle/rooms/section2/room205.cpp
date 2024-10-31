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

#include "m4/riddle/rooms/section2/room205.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/riddle.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room205::preload() {
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
}

void Room205::init() {
	midi_play("vines", 0, -1, -1, 949);
	_ripTrekMedReachHandPos1Series = series_load("RIP TREK MED REACH HAND POS1", -1, nullptr);
	_ripTrekLowReacherPos5Series = series_load("RIP TREK LOW REACHER POS5", -1, nullptr);
	_ripTrekLowReachPos2Series = series_load("RIP TREK LOW REACH POS2", -1, nullptr);
	_ripGetsShotSeries = series_load("RIP GETS SHOT", -1, nullptr);
	_205GunFireSeries = series_load("205 gun fire", -1, nullptr);

	if (!_G(flags[V029])) {
		digi_preload("205R01", -1);
		digi_preload("205M02", -1);

		_205rp1Series = series_load("205RP01", -1, nullptr);
		_205mc01Series = series_load("205MC01", -1, nullptr);
		_205mc02Series = series_load("205MC02", -1, nullptr);

		digi_preload("205R11", -1);

		_205rp02Series = series_load("205RP02", -1, nullptr);
	}

	_field198 = 0;
	_field19C = 0;
	_field1A0 = 0;
	_fieldD8 = 0;
	_fieldE8 = 0;
	_fieldE4 = 0;
	series_show("205PRIES", 3840, 16, -1, -1, 0, 100, 0, 0);
	_205LeftEntranceTabletMach = series_show("205 LEFT ENTRANCE TABLET", 257, 16, -1, -1, 0, 100, 0, 0);

	if (!_G(flags[V028]))
		hotspot_set_active(_G(currentSceneDef).hotspots, "MASTER LU'S TABLET", false);

	if (_G(flags[V024])) {
		_fieldE0 = 1;
		series_show("205GONG", 1025, 16, -1, -1, 0, 100, 0, 0);
		hotspot_set_active(_G(currentSceneDef).hotspots, "GUN", false);
		digi_preload("205_s34", -1);
		digi_play_loop("205_s34", 3, 25, -1, -1);

		if (!_G(flags[V028]))
			kernel_timing_trigger(7200, 1055, nullptr);
	} else {
		_fieldE0 = 0;
		hotspot_set_active(_G(currentSceneDef).hotspots, "GONG ", false);
		_205FireInBrazierSeries = series_load("205 FIRE IN BRAZIER", -1, nullptr);
		_205FireInBrazierMach = series_play("205 FIRE IN BRAZIER", 1024, 0, -1, 7, -1, 100, 0, 0, 0, -1);
		digi_preload("205_s34", -1);
		digi_play_loop("205_s34", 3, 166, -1, -1);

		if (_G(flags[V029]))
			_205GunInBrazierMach = series_show("205 gun in brazier", 768, 16, -1, -1, 0, 100, 0, 0);
	}

	if (inv_player_has("CHARCOAL")) {
		if (_G(flags[V029]))
			hotspot_set_active(_G(currentSceneDef).hotspots, "CHARCOAL", false);
	} else if (_G(flags[V029]))
		_205CharcoalSpriteMach = series_show("205 CHARCOAL SPRITE", 3840, 16, -1, -1, 0, 100, 0, 0);

	if (_G(flags[V025])) {
		series_show("205 MALLET LAYED ON GONG", 1024, 16, -1, -1, 0, 100, 0, 0);
		hotspot_set_active(_G(currentSceneDef).hotspots, "GAP WITH JOURNAL", false);
	} else {
		hotspot_set_active(_G(currentSceneDef).hotspots, "MALLET ", false);
	}

	if (_G(flags[V029])) {
		series_show("205BITS", 3846, 16, -1, -1, 0, 100, 0, 0);
		kernel_timing_trigger(imath_ranged_rand(1200, 1800), 901, nullptr);
	}

	if (_G(flags[V028])) {
		_205MeiStanderMach = series_show("205 MEI STANDER", 3845, 16, -1, -1, 0, 100, 0, 0);
		series_show("205 TABLETS DOWN", 3840, 16, -1, -1, 0, 100, 0, 0);
		series_show("205GLASS", 3840, 16, -1, -1, 0, 100, 0, 0);

		hotspot_set_active(_G(currentSceneDef).hotspots, "TABLET ", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "RIGHT TABLET", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "SHEN GUO", false);
	} else {
		_205TabletsSeries = series_load("205 TABLETS", -1, nullptr);
		_205TabletsMach = series_show("205 TABLETS", 3584, 16, -1, -1, 0, 100, 0, 0);

		hotspot_set_active(_G(currentSceneDef).hotspots, "GLASSES", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "FALLEN TABLETS", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN ", false);
	}

	_G(flags[V027]) = 0;
	_fieldDC = 0;

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		player_set_commands_allowed(false);
		if (_G(flags[V029])) {
			ws_demand_facing(_G(my_walker), 11);
			ws_demand_location(_G(my_walker), 320, 480);
			ws_walk(_G(my_walker), 317, 356, nullptr, 500, 11, true);
		} else {
			hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN ", false);
			ws_walk_load_walker_series(S8_SHADOW_DIRS1, S8_SHADOW_NAMES1);
			ws_walk_load_walker_series(S8_SHADOW_DIRS2, S8_SHADOW_NAMES2);
			_mcEntranceTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 258, 490, 1, triggerMachineByHashCallback3000, "mc_entrance_trek");
			ws_demand_facing(_G(my_walker), 11);
			ws_demand_location(_G(my_walker), 320, 480);
			ws_walk(_G(my_walker), 317, 356, nullptr, 501, 11, true);
			sendWSMessage_10000(_mcEntranceTrekMach, 261, 359, 2, -1, true);

			return;
		}
	}

	if (!_G(flags[V028]))
		kernel_timing_trigger(1, 1000, nullptr);
}

void Room205::pre_parser() {
	if (player_said("GEAR", "RIGHT TABLET") && _G(flags[V022]) && !_G(flags[V024])) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}

	if (player_said("EXIT") && _G(flags[V025])) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}
}

void Room205::parser() {
}

void Room205::daemon() {
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
