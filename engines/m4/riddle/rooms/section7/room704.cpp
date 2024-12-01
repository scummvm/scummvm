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

#include "m4/riddle/rooms/section7/room704.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_vmng_screen.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room704::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
}

void Room704::init() {
	digi_preload("950_S40", -1);
	digi_preload("950_S40A", -1);
	_ripTrekMedReachHandPos1Series = series_load("RIP TREK MED REACH HAND POS1", -1, nullptr);

	if (_G(flags[V224])) {
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #5", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #6", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #7", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "MONK #8", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "EMPTY NICHE", false);
	}

	hotspot_set_active(_G(currentSceneDef).hotspots, _G(flags[V218]) ? "EMPTY NICHE" : "PRAYER WHEEL #6", false);

	switch (_G(flags[V218])) {
	case 1:
		_prayerWheelMach = series_place_sprite("704 PEACE PRAYER WHEEL", 0, 0, 0, 100, 1280);
		break;

	case 2:
		_prayerWheelMach = series_place_sprite("704 INSIGHT PRAYER WHEEL", 0, 0, 0, 100, 1280);
		break;

	case 3:
		_prayerWheelMach = series_place_sprite("704 SERENITY PRAYER WHEEL", 0, 0, 0, 100, 1280);
		break;

	case 4:
		_prayerWheelMach = series_place_sprite("704 TRUTH PRAYER WHEEL", 0, 0, 0, 100, 1280);
		break;

	case 5:
		_prayerWheelMach = series_place_sprite("704 WISDOM PRAYER WHEEL", 0, 0, 0, 100, 1280);
		break;

	default:
		break;
	}

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		// Same as default?
		digi_preload("950_s39", -1);
		break;

	case 703:
		player_first_walk(0, 300, 3, 130, 318, 3, true);
		break;

	case 705:
		MoveScreenDelta(_G(game_buff_ptr), -640, 0);
		ws_demand_location(_G(my_walker), 1299, 300);
		ws_demand_facing(_G(my_walker), 9);
		ws_walk(_G(my_walker), 1170, 318, nullptr, -1, 0, true);

		break;

	case 707:
		MoveScreenDelta(_G(game_buff_ptr), -320, 0);
		ws_demand_location(_G(my_walker), 645, 356);
		ws_demand_facing(_G(my_walker), 5);
		_ripGoesDownStairsSeries = series_load("704 RIP GOES DOWN STAIRS", -1, nullptr);
		player_set_commands_allowed(false);
		ws_hide_walker(_G(my_walker));
		_ripStairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 320, 0, 100, 0, 0, callback, "rip stairs machine");
		sendWSMessage_10000(1, _ripStairsMach, _ripGoesDownStairsSeries, 4, 39, 140, _ripGoesDownStairsSeries, 39, 39, 0);

		break;

	default:
		digi_preload("950_s39", -1);
		break;
	}

	digi_play_loop("950_s39", 3, 255, -1, -1);
}

void Room704::pre_parser() {
	// No implementation
}

void Room704::parser() {
	// TODO
}

void Room704::daemon() {
	// TODO
}


void Room704::callback(frac16 myMessage, machine *sender) {
	kernel_trigger_dispatchx(myMessage);
}



} // namespace Rooms
} // namespace Riddle
} // namespace M4
