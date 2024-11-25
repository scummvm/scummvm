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

#include "m4/riddle/rooms/section7/room703.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_vmng_screen.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room703::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
}

void Room703::init() {
	_ripTrekMedReachHandPos1Series = series_load("RIP TREK MED REACH HAND POS1", -1, nullptr);
	digi_preload("950_S40", -1);
	digi_preload("950_S40a", -1);

	if (_G(flags[V224])) {
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #1", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #2", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #3", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #4", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "EMPTY NICHE", false);
	}

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		digi_preload("950_s39", -1);
		break;

	case 702:
		if (!player_been_here(703)) {
			_G(flags[V217]) = 3;
			_G(flags[V218]) = 1;
			_G(flags[V219]) = 5;
			_G(flags[V220]) = 2;
			_G(flags[V221]) = 4;
		}

		MoveScreenDelta(_G(game_buff_ptr), -320, 0);
		ws_demand_location(_G(my_walker), 630, 450);
		ws_demand_facing(_G(my_walker), 1);
		ws_walk(_G(my_walker), 625, 357, nullptr, -1, 1, true);

		break;

	case 704:
		MoveScreenDelta(_G(game_buff_ptr), -640, 0);
		player_first_walk(1299, 300, 9, 1170, 318, 9, true);

		break;

	case 706:
		player_first_walk(0, 300, 9, 130, 318, 8, true);
		break;

	case 707:
		MoveScreenDelta(_G(game_buff_ptr), -320, 0);
		ws_demand_location(_G(my_walker), 648, 358);
		ws_demand_facing(_G(my_walker), 5);
		player_set_commands_allowed(false);
		ws_hide_walker(_G(my_walker));
		_703RipGoesDownStairsSeries = series_load("703 RIP GOES DOWN STAIRS", -1, nullptr);
		_ripStairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 320, 0, 100, 0, false, callback, "rip stairs machine");
		sendWSMessage_10000(1, _ripStairsMach, _703RipGoesDownStairsSeries, 4, 39, 140, _703RipGoesDownStairsSeries, 39, 39, 0);

		break;

	default:
		digi_preload("950_s39");
		player_first_walk(1299, 400, 10, 970, 318, 10, true);
		break;
	}

	if (_G(flags[V217])) {
		hotspot_set_active(_G(currentSceneDef).hotspots, "EMPTY NICHE", false);
	} else {
		hotspot_set_active(_G(currentSceneDef).hotspots, "PRAYER WHEEL #4", false);
	}

	switch (_G(flags[V217])) {
	case 1:
		_prayerWheelMach = series_place_sprite("703 PEACE PRAYER WHEEL", 0, 640, 0, 100, 1280);
		break;

	case 2:
		_prayerWheelMach = series_place_sprite("703 INSIGHT PRAYER WHEEL", 0, 640, 0, 100, 1280);
		break;

	case 3:
		_prayerWheelMach = series_place_sprite("703 SERENITY PRAYER WHEEL", 0, 640, 0, 100, 1280);
		break;

	case 4:
		_prayerWheelMach = series_place_sprite("703 TRUTH PRAYER WHEEL", 0, 640, 0, 100, 1280);
		break;

	case 5:
		_prayerWheelMach = series_place_sprite("703 WISDOM PRAYER WHEEL", 0, 640, 0, 100, 1280);
		break;

	default:
		break;
	}

	digi_play_loop("950_s39", 3, 255, -1, -1);
}

void Room703::pre_parser() {
	if (player_said("rm702") && (inv_player_has("WISDOM WHEEL") || inv_player_has("SERENITY WHEEL") || inv_player_has("PEACE WHEEL") || inv_player_has("INSIGHT WHEEL") || inv_player_has("TRUTH WHEEL"))) {
		_G(player).need_to_walk = false;
		_G(player).ready_to_walk = true;
		_G(player).waiting_for_walk = false;
	}
}

void Room703::parser() {
}

void Room703::daemon() {
}

void Room703::callback(frac16 myMessage, machine *sender) {
	kernel_trigger_dispatchx(myMessage);
}
} // namespace Rooms
} // namespace Riddle
} // namespace M4
