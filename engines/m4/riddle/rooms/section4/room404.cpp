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

#include "m4/riddle/rooms/section4/room404.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

static const int16 NORMAL_DIRS[] = { 200, 201, -1 };
static const char *NORMAL_NAMES[] = {
	"butler walker pos1",
	"butler walker pos3"
};

static const int16 SHADOW_DIRS[] = { 200, 201, -1 };
static const char *SHADOW_NAMES[] = {
	"butler walker shadow pos1",
	"wolf walker shadow pos3"
};

void Room404::preload() {
	_G(player).walker_type = 1;
	_G(player).shadow_type = 1;
	LoadWSAssets("OTHER SCRIPT");
}

void Room404::init() {
	_door = series_place_sprite("404DOORA", 0, 0, 0, 100, 0xf00);

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		player_set_commands_allowed(false);
		_val1 = 0;
		_val2 = -1;
		_val3 = 0;
		_val4 = -1;
		_val5 = 0;
		_val6 = 0;
		_val7 = 0;
		_val8 = 0;
	}

	_safariShadow = series_load("SAFARI SHADOW 3");
	_butlerTurns7 = series_load("Butler turns to pos7");
	_butlerTurns9 = series_load("Butler turns to pos9");
	_butlerTalkLoop = series_load("Butler talk loop");
	_val7 = 2000;
	_val8 = 2100;

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		_butlerTalks = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x900, 0,
			triggerMachineByHashCallbackNegative, "BUTLER talks rip");
		sendWSMessage_10000(1, _butlerTalks, _butlerTalkLoop, 1, 1, -1,
			_butlerTalkLoop, 1, 1, 0);
		player_set_commands_allowed(true);
		break;

	case 405:
		_butlerTalks = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x900, 0,
			triggerMachineByHashCallbackNegative, "BUTLER talks rip");
		sendWSMessage_10000(1, _butlerTalks, _butlerTalkLoop, 1, 1, -1,
			_butlerTalkLoop, 1, 1, 0);
		ws_demand_location(58, 347, 3);
		ws_walk(90, 347, nullptr, 50, 3);
		break;

	case 406:
		_butlerTalks = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0x900, 0,
			triggerMachineByHashCallbackNegative, "BUTLER talks rip");
		sendWSMessage_10000(1, _butlerTalks, _butlerTalkLoop, 1, 1, -1,
			_butlerTalkLoop, 1, 1, 0);
		ws_demand_location(174, 268, 3);
		ws_walk(250, 285, nullptr,
			inv_player_has("BILLIARD BALL") ? 70 : 60,
			3, 1);
		break;

	default:
		ws_walk_load_shadow_series(SHADOW_DIRS, SHADOW_NAMES);
		ws_walk_load_walker_series(NORMAL_DIRS, NORMAL_NAMES);
		ws_demand_location(340, 480, 2);

#ifdef KITTY_SCREAMING
		const char *KITTY = "SCREAMING";
#else
		const char *KITTY = nullptr;
#endif
		_machine1 = triggerMachineByHash_3000(8, 10, NORMAL_DIRS, SHADOW_DIRS,
			380, 421, 1, triggerMachineByHashCallback3000, "BUTLER_walker");

		if (strcmp(KITTY, "SCREAMING") || !player_been_here(404)) {
			sendWSMessage_10000(_machine1, 410, 332, 1, 21, 1);
			kernel_timing_trigger(120, 20);
			digi_play("404_s01", 2);
		}
		break;
	}
}

void Room404::daemon() {
	// TODO
}

void Room404::pre_parser() {
	// TODO
}

void Room404::parser() {
	// TODO
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
