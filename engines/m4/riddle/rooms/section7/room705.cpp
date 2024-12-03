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

#include "m4/riddle/rooms/section7/room705.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_vmng_screen.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room705::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
}

void Room705::init() {
	digi_preload("950_S40", -1);
	digi_preload("950_S40A", -1);
	_ripTrekMedReachPos1Series = series_load("RIP TREK MED REACH HAND POS1", -1, nullptr);

	if (_G(flags[V224])) {
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #9", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #10", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #11", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #12", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "EMPTY NICHE", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "EMPTY NICHE ", false);
	}

	hotspot_set_active(_G(currentSceneDef).hotspots, _G(flags[V220]) ? "EMPTY NICHE" : "PRAYER WHEEL #9", false);

	switch (_G(flags[V220])) {
	case 1:
		_leftWheelMach = series_place_sprite("705 LEFT PEACE WHEEL", 0, 0, 0, 100, 1280);
		break;

	case 2:
		_leftWheelMach = series_place_sprite("705 LEFT INSIGHT WHEEL", 0, 0, 0, 100, 1280);
		break;

	case 3:
		_leftWheelMach = series_place_sprite("705 LEFT SERENITY WHEEL", 0, 0, 0, 100, 1280);
		break;

	case 4:
		_leftWheelMach = series_place_sprite("705 LEFT TRUTH WHEEL", 0, 0, 0, 100, 1280);
		break;

	case 5:
		_leftWheelMach = series_place_sprite("705 LEFT WISDOM WHEEL", 0, 0, 0, 100, 1280);
		break;

	default:
		break;
	}

	hotspot_set_active(_G(currentSceneDef).hotspots, _G(flags[V221]) ? "EMPTY NICHE " : "PRAYER WHEEL #12", false);

	switch (_G(flags[V221])) {
	case 1:
		_leftWheelMach = series_place_sprite("705 RT PEACE WHEEL", 0, 640, 0, 100, 1280);
		break;

	case 2:
		_leftWheelMach = series_place_sprite("705 RT INSIGHT WHEEL", 0, 640, 0, 100, 1280);
		break;

	case 3:
		_leftWheelMach = series_place_sprite("705 RT SERENITY WHEEL", 0, 640, 0, 100, 1280);
		break;

	case 4:
		_leftWheelMach = series_place_sprite("705 RT TRUTH WHEEL", 0, 0, 640, 100, 1280);
		break;

	case 5:
		_leftWheelMach = series_place_sprite("705 RT WISDOM WHEEL", 0, 0, 640, 100, 1280);
		break;

	default:
		break;
	}

	switch (_G(game).previous_room) {
	case 704:
		player_first_walk(0, 300, 3, 130, 318, 3, true);
		break;

	case 706:
		MoveScreenDelta(_G(game_buff_ptr), -640, 0);
		player_first_walk(1299, 300, 9, 1170, 318, 9, true);
		break;

	case 707:
		MoveScreenDelta(_G(game_buff_ptr), -320, 0);
		ws_demand_location(_G(my_walker), 643, 357);
		ws_demand_facing(_G(my_walker), 5);
		player_set_commands_allowed(false);
		ws_hide_walker(_G(my_walker));
		_ripGoesDownStairsSeries = series_load("705 RIP GOES DOWN STAIRS", -1, nullptr);
		_ripStairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 320, 0, 100, 0, 0, triggerMachineByHashCallback, "rip stairs machine");
		sendWSMessage_10000(1, _ripStairsMach, _ripGoesDownStairsSeries, 4, 39, 160, _ripGoesDownStairsSeries, 39, 39, 0);

		break;

	default: // includes case KERNEL_RESTORE (-2)
		digi_preload("950_s39", -1);
		break;
	}

	digi_play_loop("950_s39", 3, 255, -1, -1);
}

void Room705::pre_parser() {
	// Empty
}

void Room705::parser() {
}

void Room705::daemon() {
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
