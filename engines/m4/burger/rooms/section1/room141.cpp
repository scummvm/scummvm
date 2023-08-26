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

#include "m4/burger/rooms/section1/room141.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const char *SAID[][4] = {
	{ "TRUFFLES", "141W002", "141W003", "141W003" },
	{ "ISLAND",   "141W004", "141W003", "141W003" },
	{ "CABIN",    "141W005", "141W003", "141W006" },
	{ "GARDEN",   "141W007", "141W003", "141W006" },
	{ "TROUGH",   "141W008", "141W006", "141W006" },
	{ "SIGN",     "141W009", "141W006", "141W006" },
	{ "TREES",    "141W010", "141W003", "141W003" },
	{ "WATER",    "141W011", "141W003", "141W012" },
	{ "ROCKS",    "141W013", "141W003", nullptr   },
	{ "DOCK",     "141W002", nullptr,   nullptr   },
	{ "HANLON'S POINT", nullptr, "141W003", "141W003" },
	{ nullptr, nullptr, nullptr, nullptr }
};


void Room141::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room141::init() {
	digi_preload("144_001");

	if (_G(game).previous_room != RESTORING_GAME)
		player_set_commands_allowed(false);

	if (!_G(flags)[V112]) {
		digi_preload("140t001a", 140);
		digi_preload("140t001b", 140);
		digi_preload("140t001c", 140);
		digi_preload("140t001d", 140);
		digi_preload("140t001e", 140);
		digi_preload("140t001f", 140);

		series_load("140tr01");
		series_load("140tr01s");
		series_load("140tr02");
		series_load("140tr02s");
		series_load("140tr03");
		series_load("140tr03s");
	}

	series_load("140tr04");
	series_load("140tr04s");
	series_load("140tr05");
	series_load("140tr05s");
	series_load("140tr06");
	series_load("140tr06s");
	series_load("140tr07");
	series_load("140tr07s");

	if (!_G(flags)[V112]) {
		_series1 = series_play(_G(flags)[V000] == 1002 ? "141pboat" : "140pboat",
			0xf00, 0, -1, 10, -1, 100, 0, 0, 0, 3);

		series_load("140pe01");
		series_load("140pe01s");
		digi_preload("20n01011", 140);
		digi_preload("20n01012", 140);
		digi_preload("20n01013", 140);
		digi_preload("20n01014", 140);

		_val1 = 14;
		_val2 = 14;
		kernel_trigger_dispatch_now(7);
	}

	if (_G(game).previous_room == RESTORING_GAME) {
		_series2 = series_play("141wave", 0xf00, 0, -1, 10, -1, 100, 0, 0, 0, 3);
		_val3 = 9;
		_val4 = 9;
		_val5 = 52;
		_val6 = 52;

	} else {
		_val5 = 52;
		_val6 = 50;

		if (_G(flags)[V112]) {
			_val3 = 1;
			_val4 = 8;
		} else {
			_val3 = 1;
			_val4 = 1;
			_val7 = 0;
		}
	}

	kernel_trigger_dispatch_now(8);
	kernel_trigger_dispatch_now(6);
	digi_play_loop("144_001", 3, 180);
}

void Room141::daemon() {
}

void Room141::parser() {
	bool lookFlag = player_said("look") || player_said("look at");

	if (player_said("conv20")) {
		conv20();

	} else if (player_said("whistle") && player_said_any("GEAR", "WILBUR")) {
		player_set_commands_allowed(false);
		_val6 = 62;

	} else if (player_said("gear", "dock") || player_said("try to dock")) {
		_val4 = 12;

	} else if (inv_player_has(_G(player).verb) &&
			player_said_any("cabin", "garden", "trough", "sign")) {
		wilbur_speech("141w006");

	} else if (inv_player_has(_G(player).verb) && player_said_any("trees", "water")) {
		wilbur_speech("141w003");

	} else if (player_said("take") && !inv_player_has(_G(player).noun)) {
		if (player_said("trough") || player_said("sign")) {
			_val4 = 12;

		} else if (!_G(walker).wilbur_said(SAID)) {
			goto check_exit;
		}

	} else if (player_said("gear") && !inv_player_has(_G(player).noun)) {
		if (player_said("cabin") || player_said("garden") ||
				player_said("trough") || player_said("sign")) {
			_val4 = 12;

		} else if (!_G(walker).wilbur_said(SAID)) {
			goto check_exit;
		}

	} else if (!_G(walker).wilbur_said(SAID)) {
		goto check_exit;
	}
	goto done;

check_exit:
	if (player_said("exit") || (player_said("hanlon's point") && lookFlag)) {
		player_set_commands_allowed(false);
		_val6 = 55;

	} else {
		return;
	}

done:
	_G(player).command_ready = false;
}

void Room141::conv20() {
	error("TODO: Room141::conv20");
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
