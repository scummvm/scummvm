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

#include "m4/riddle/rooms/section4/room405.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

static const int16 NORMAL_DIRS[] = { 220, 221, -1 };
static const char *NORMAL_NAMES[] = {
	"baron walker pos1",
	"baron walker pos7"
};

static const int16 SHADOW_DIRS[] = { 230, 231, -1 };
static const char *SHADOW_NAMES[] = {
	"baron walker shadow pos1",
	"baron walker shadow pos7"
};


void Room405::preload() {
	_G(player).walker_type = 1;
	_G(player).shadow_type = 1;
	LoadWSAssets("OTHER SCRIPT");
}

void Room405::init() {
	player_set_commands_allowed(false);
	_door = series_place_sprite("405doora", 0, 0, 0, 100, 0);

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_val1 = 0;
		_val2 = -1;
		_val3 = 0;
		_val4 = -1;
		_val5 = 0;
		_val6 = 0;
		_val7 = 0;
		_val8 = 0;
		_val9 = 0;
	}

	if (!_G(flags)[V338] || !inv_object_is_here("GERMAN BANKNOTE"))
		hotspot_set_active("GERMAN BANKNOTE", false);

	_safariShadow = series_load("SAFARI SHADOW 3");

	if (inv_object_is_here("GERMAN BANKNOTE"))
		_bankNote = series_place_sprite("405 BANK NOTE UNDER RUG", 0, 0, 0, 100, 0xf00);

	_candlesBurning = series_load("TWO CANDLES BURNING");
	_candles = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, 0,
		triggerMachineByHashCallbackNegative, "candles");
	sendWSMessage_10000(1, _candles, _candlesBurning, 1, 8, -1,
		_candlesBurning, 1, 8, 0);

	if (_G(game).previous_room == KERNEL_RESTORING_GAME) {
		player_set_commands_allowed(true);
		return;
	}

	_G(flags)[V110] = 1;
	if (!inv_player_has("TURTLE"))
		inv_move_object("TURTLE", 305);

	ws_demand_location(155, 370, 9);

	if (player_been_here(405) || _G(kittyScreaming)) {
		ws_walk(230, 345, nullptr, 50, 2);
	} else {
		ws_walk_load_shadow_series(SHADOW_DIRS, SHADOW_NAMES);
		ws_walk_load_walker_series(NORMAL_DIRS, NORMAL_NAMES);
		_baron = triggerMachineByHash_3000(8, 11, NORMAL_DIRS, SHADOW_DIRS, 185, 365, 1,
			triggerMachineByHashCallback3000, "BARON_walker");
		ws_walk(329, 320, nullptr, 20, 9);
	}
}

void Room405::daemon() {
}

void Room405::pre_parser() {

}

void Room405::parser() {

}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
