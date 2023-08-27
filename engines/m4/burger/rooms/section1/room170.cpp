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

#include "m4/burger/rooms/section1/room170.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const char *SAID[][4] = {
	{ "HOUSE",        "170w001", "170w002", "170w002" },
	{ "FRONT DOOR",   nullptr,   nullptr,   nullptr   },
	{ "VERA'S DINER", nullptr,   "170w002", nullptr   },
	{ "TOWN HALL",    nullptr,   "170w002", nullptr   },
	{ "HIGHWAY 2",    "170w009", "170w002", "170w009" },
	{ nullptr, nullptr, nullptr, nullptr }
};

static const seriesPlayBreak PLAY1[] = {
	{  0,  9, "100_004", 1, 255, -1, 0, 0, nullptr, 0 },
	{ 10, -1, "100_003", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY2[] = {
	{  0, 11, "170_001", 1,  50, -1, 0,  0, nullptr, 0 },
	{ 12, 19, "170_002", 1, 255, -1, 0,  0, nullptr, 0 },
	{ 20, 20, "170w005", 1, 255, -1, 0, -1, nullptr, 0 },
	{ 21, 39, "170_002", 1, 255, -1, 0,  0, nullptr, 0 },
	{ 40, 40, "170w006", 1, 125, -1, 0, -1, nullptr, 0 },
	{ 41, 52, nullptr,   0,   0, -1, 0,  0, nullptr, 0 },
	{  5,  0, "170_001", 1,  50, -1, 0,  0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY3[] = {
	{  0, 12, "170_001", 1, 50, -1, 0, 0, nullptr, 0 },
	{ 23, 52, nullptr,   0,  0, -1, 0, 0, nullptr, 0 },
	{  5,  0, "170_001", 1, 50, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};


void Room170::init() {
	digi_preload("100_001");
	digi_play_loop("100_001", 3, 255);

	_firstTime = player_been_here(170);
	series_show("170box", 0xa00);

	switch (_G(game).previous_room) {
	case RESTORING_GAME:
		kernel_trigger_dispatch_now(1);
		break;

	case 104:
		ws_demand_location(494, 400, 10);
		ws_walk(355, 353, nullptr, 1);
		break;

	case 142:
		ws_demand_location(-40, 375, 2);
		ws_walk(25, 350, 0, 1);
		break;

	case 171:
		ws_demand_location(319, 192, 8);
		kernel_trigger_dispatch_now(1);
		break;

	default:
		ws_demand_location(134, 307, 5);
		kernel_trigger_dispatch_now(1);
		break;
	}
}

void Room170::daemon() {
	// TODO
}

void Room170::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said_any("GEAR", "LOOK AT")) {
		if (player_said("TOWN HALL")) {
			setupTownHall();
			_G(kernel).call_daemon_every_loop = true;
		}
	} else {
		if (player_said("VERA'S DINER"))
			player_set_facing_at(-40, 375);

		if (player_said("TOWN HALL"))
			player_set_facing_at(494, 400);
	}

	if (player_said("YARD"))
		player_set_facing_hotspot();
}

void Room170::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(walker).wilbur_said(SAID)) {
		// Handled
	} else if (player_said_any("GEAR", "LOOK AT") && player_said("VERA'S DINER")) {
		disable_player_commands_and_fade_init(1014);
	} else if (player_said_any("GEAR", "LOOK AT") && player_said("FRONT DOOR")) {
		_G(wilbur_should) = 1;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
	} else if (player_said_any("GEAR", "LOOK AT") && player_said("MAILBOX")) {
		if (_G(flags)[V079]) {
			player_set_commands_allowed(false);

			if (_G(flags)[V080]) {
				_G(wilbur_should) = 5;
				kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
			} else {
				_G(wilbur_should) = 3;
				wilbur_speech("170w004", gCHANGE_WILBUR_ANIMATION);
			}
		} else {
			wilbur_speech("170w008");
		}
	} else {
		return;
	}

	_G(player).command_ready = false;
}

void Room170::setupTownHall() {
	if (player_said_any("GEAR", "LOOK AT") && player_said("TOWN HALL"))
		_lookTownHall = true;

	if (_lookTownHall) {
		player_update_info();

		if (_G(player_info).y > 375 && player_commands_allowed())
			disable_player_commands_and_fade_init(1004);
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
