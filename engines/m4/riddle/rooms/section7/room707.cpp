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

#include "m4/riddle/rooms/section7/room707.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_vmng_screen.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room707::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
}

void Room707::init() {
	ws_demand_location(_G(my_walker), 299, 200);
	ws_demand_facing(_G(my_walker), 4);
	if (!player_been_here(707)) {
		_field44 = 0;
	}

	_peerIntoCupolaPos2Series = series_load("PEER INTO CUPOLA POS2", -1, nullptr);
	_peerIntoCupolaPos1Series = series_load("PEER INTO CUPOLA POS1", -1, nullptr);

	switch (_G(game).previous_room) {
	case 703:
		ws_demand_location(_G(my_walker), 421, 150);
		ws_demand_facing(_G(my_walker), 4);
		player_set_commands_allowed(false);
		ws_hide_walker(_G(my_walker));
		_ripWalksUpLeftStairsSeries = series_load("RIP WALKS UP LEFT STAIRS", -1, nullptr);
		_ripStairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, false, triggerMachineByHashCallback, "rip stairs machine");
		sendWSMessage_10000(1, _ripStairsMach, _ripWalksUpLeftStairsSeries, 1, 22, 11, _ripWalksUpLeftStairsSeries, 22, 22, 0);

		break;

	case 704:
		ws_demand_location(_G(my_walker), 374, 252);
		ws_demand_facing(_G(my_walker), 2);
		_ripUpStairsPos2Series = series_load("RIP UP STAIRS POSITION 2", -1, nullptr);
		player_set_commands_allowed(false);
		ws_hide_walker(_G(my_walker));
		_ripStairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, false, triggerMachineByHashCallback, "rip stairs machine");
		sendWSMessage_10000(1, _ripStairsMach, _ripUpStairsPos2Series, 1, 37, 11, _ripUpStairsPos2Series, 37, 37, 0);

		break;

	case 705:
		MoveScreenDelta(_G(game_buff_ptr), -640, 0);
		ws_demand_location(_G(my_walker), 930, 248);
		ws_demand_facing(_G(my_walker), 10);
		player_set_commands_allowed(false);
		ws_hide_walker(_G(my_walker));
		_ripUpStairsPos10Series = series_load("RIP UP STAIRS POSITION 10", -1, nullptr);
		_ripStairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, false, triggerMachineByHashCallback, "rip stairs machine");
		sendWSMessage_10000(1, _ripStairsMach, _ripUpStairsPos10Series, 1, 37, 11, _ripUpStairsPos10Series, 37, 37, 0);

		break;

	case 706:
		MoveScreenDelta(_G(game_buff_ptr), -640, 0);
		ws_demand_location(_G(my_walker), 860, 149);
		ws_demand_facing(_G(my_walker), 8);
		player_set_commands_allowed(false);
		ws_hide_walker(_G(my_walker));
		_ripWalksUpFarStairsSeries = series_load("RIP WALKS UP FAR STAIRS", -1, nullptr);
		_ripStairsMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, false, triggerMachineByHashCallback, "rip stairs machine");
		sendWSMessage_10000(1, _ripStairsMach, _ripWalksUpFarStairsSeries, 1, 22, 11, _ripWalksUpFarStairsSeries, 22, 22, 0);

		break;

	default:
		digi_preload("950_s39", -1);
		break;
	}

	digi_play_loop("950_s39", 3, 255, -1, -1);
}

void Room707::pre_parser() {
	// No implementation
}

void Room707::parser() {
	// TODO : Not implemented yet
}

void Room707::daemon() {
	if (_G(kernel).trigger == 11) {
		player_set_commands_allowed(true);
		ws_unhide_walker(_G(my_walker));
		terminateMachine(_ripStairsMach);
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
