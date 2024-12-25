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

#include "m4/riddle/rooms/section7/room710.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room710::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
}

void Room710::init() {
	_710Rpld2Series = series_load("710RPLD2", -1, nullptr);
	_710Rpld3Series = series_load("710RPLD3", -1, nullptr);
	_ripPullsRopeForLaderSeries = series_load("RIP PULLS ROPE FOR LADER", -1, nullptr);
	_laderComesDownSeries = series_load("LADER COMES DOWN", -1, nullptr);
	_ripTrekMedReachHandPos1Series = series_load("RIP TREK MED REACH HAND POS1", -1, nullptr);
	_710Rpro1Series = series_load("710RPRO1", -1, nullptr);
	_710Rpro2Series = series_load("710RPRO2", -1, nullptr);
	_710Rpro3Series = series_load("710RPRO3", -1, nullptr);
	_710Rpro4Series = series_load("710RPRO4", -1, nullptr);
	_mazeCentreDoorLiteSeries = series_load("MAZE CENTRE DOOR LITE", -1, nullptr);

	digi_preload("710_s02", -1);
	digi_preload("710_s01", -1);

	_field4C = 0;

	_ripContraptionMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 256, false, triggerMachineByHashCallback, "rip contraption machine");
	_ripReacherMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 1792, false, triggerMachineByHashCallback, "rip reacher machine");
	_710Rprs3Mach = series_place_sprite("710RPRS3", 0, 0, -53, 100, 2565);
	_710Rprs2Mach = series_place_sprite("710RPRS2", 0, 0, -53, 100, 2053);
	_710Rprs1Mach = series_place_sprite("710RPRS1", 0, 0, -53, 100, 517);
	_710Rprs4Mach = series_place_sprite("710RPRS4", 0, 0, -53, 100, 1029);

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		digi_preload("950_s41", -1);
		if (_G(flags[V223])) {
			_field1C = 1;
			_710Rpld4Mach = series_place_sprite("710rpld4", 0, 0, -53, 100, 1541);
			_710Rpld5Mach = series_place_sprite("710rpld5", 0, 0, -53, 100, 3840);
		} else {
			_field1C = 0;
			_710Rpld4Mach = series_place_sprite("710rpld3", 0, 0, -53, 100, 3840);
			_710Rpld5Mach = series_place_sprite("710rpld5", 0, 0, -53, 100, 3840);
			hotspot_set_active(_G(currentSceneDef).hotspots, "Ladder", false);
		}
		break;

	case 709:
		ws_demand_location(_G(my_walker), 320, 282);
		ws_demand_facing(_G(my_walker), 5);

		if (_G(flags[V223])) {
			_field1C = 1;
			_710Rpld4Mach = series_place_sprite("710rpld4", 0, 0, -53, 100, 1541);
			_710Rpld5Mach = series_place_sprite("710rpld5", 0, 0, -53, 100, 3840);
		} else {
			_field1C = 0;
			hotspot_set_active(_G(currentSceneDef).hotspots, "Ladder", false);
			_710Rpld4Mach = series_place_sprite("710rpld3", 0, 0, -53, 100, 3840);
			_710Rpld5Mach = series_place_sprite("710rpld5", 0, 0, -53, 100, 3840);
		}

		break;

	case 711:
		digi_preload("950_s41", -1);
		ws_demand_location(_G(my_walker), 320, 308);
		ws_demand_facing(_G(my_walker), 1);
		ws_hide_walker(_G(my_walker));
		player_set_commands_allowed(false);

		_710Rpld4Mach = series_place_sprite("710rpld4", 0, 0, -53, 100, 1541);
		_710Rpld5Mach = series_place_sprite("710rpld5", 0, 0, -53, 100, 4095);

		_field1C = 1;
		hotspot_set_active(_G(currentSceneDef).hotspots, "Ladder", true);
		sendWSMessage_10000(1, _ripContraptionMach, _710Rpld2Series, 86, 83, 30, _710Rpld2Series, 83, 83, 0);

		break;

	default:
		digi_preload("950_s41", -1);
		ws_demand_location(_G(my_walker), 282, 282);
		ws_demand_facing(_G(my_walker), 4);

		if (_G(flags[V223])) {
			_field1C = 1;
			_710Rpld4Mach = series_place_sprite("710rpld4", 0, 0, -53, 100, 1541);
			_710Rpld5Mach = series_place_sprite("710rpld5", 0, 0, -53, 100, 4095);
		} else {
			_field1C = 0;
			_710Rpld4Mach = series_place_sprite("710rpld3", 0, 0, -53, 100, 3840);
			_710Rpld5Mach = series_place_sprite("710rpld5", 0, 0, -53, 100, 4095);
			hotspot_set_active(_G(currentSceneDef).hotspots, "Ladder", false);
		}

		break;
	}

	if (player_been_here(710))
		midi_play("drama1", 127, 0, -1, 949);

	digi_play_loop("950_s41", 3, 255, -1, -1);
}

void Room710::pre_parser() {
	// No implementation
}

void Room710::parser() {
	// TODO Not implemented yet
}

void Room710::daemon() {
	// TODO Not Implemented yet
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
