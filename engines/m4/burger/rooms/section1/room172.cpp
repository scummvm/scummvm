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

#include "m4/burger/rooms/section1/room172.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const char *SAID[][4] = {
	{ "AUNT POLLY",    "172w001", "172w002", "172w002" },
	{ "PARLOUR",       "172w002", nullptr,   nullptr   },
	{ "BASEMENT",      nullptr,   "172w002", nullptr   },
	{ "MICROWAVE",     "172w003", nullptr,   nullptr   },
	{ "REFRIGERATOR",  "172w005", "172w002", nullptr   },
	{ "OVEN",          nullptr,   "172w002", nullptr   },
	{ "SINK",          "172w008", "172w009", nullptr   },
	{ "HUTCH",         "172w011", "172w012", "172w002" },
	{ "BROOM",         "172w013", "172w014", "172w015" },
	{ "WINDOW",        "172w016", "172w002", "172w017" },
	{ "CALENDAR",      "172w018", "172w019", "172w018" },
	{ "CUPBOARD ",     "172w021", "172w002", nullptr   },
	{ "CUPBOARD",      "172w022", "172w002", nullptr   },
	{ "POTS AND PANS", "172w023", "172w024", "172w024" },
	{ "COOKIES",       "172w025", "172w026", "172w027" },
	{ "PRESERVES",     "172w028", "172w029", "172w029" },
	{ "CARROT JUICE ", "172w030", nullptr,   "172w031" },
	{ "JUICER",        "172w032", nullptr,   "172w033" },
	{ "VEGGIES",       "172w034", "172w035", "172w035" },
	{ nullptr, nullptr, nullptr, nullptr }
};

static const seriesPlayBreak PLAY1[] = {
	{  0,  9, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 10, -1, nullptr, 0, 0,  2, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY2[] = {
	{ 8, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY3[] = {
	{  0, 27, "172_001", 2, 255, -1, 0,  0, nullptr, 0 },
	{ 28, 30, "172_005", 3, 125, -1, 0, -1, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY4[] = {
	{ 44, 45, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 45, 45, nullptr, 0, 0, -1, 0, 3, nullptr, 0 },
	{ 45, 44, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY5[] = {
	{ 36, 43, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	{ 36, 36, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY6[] = {
	{ 46, 48, "172_004", 3, 125, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

static const seriesPlayBreak PLAY7[] = {
	{ 0, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

void Room172::init() {
	switch (_G(game).previous_room) {
	case RESTORING_GAME:
		break;

	case 171:
		if (!_G(flags)[V092] && !_G(flags)[V091]) {
			player_set_commands_allowed(false);
			series_load("172aplt");
			series_load("172aplts");
			series_load("172ap02t");
			digi_preload("172p906");

			_val1 = 4;
			kernel_trigger_dispatch_now(1);
			ws_demand_location(300, 325, 3);
			ws_walk(368, 359, 0, -1, 2);

		} else {
			ws_demand_location(11, 296, 3);
			ws_walk(156, 297, 0, -1, -1);
		}
		break;

	case 173:
		ws_demand_location(171, 254, 3);
		ws_walk(250, 270, 0, -1, -1);
		break;

	default:
		ws_demand_location(312, 292, 5);
		break;
	}

	hotspot_set_active("AUNT POLLY", false);

	if (_G(flags)[V092]) {
		_val1 = 15;
		kernel_trigger_dispatch_now(1);
		_walk1 = intr_add_no_walk_rect(357, 311, 639, 353, 356, 354);
		hotspot_set_active("AUNT POLLY", true);
	} else {
		_series1 = series_show(_G(flags)[V091] ? "172fud2" : "172fud", 0x700);
		series_show("172fuds", 0x701);
	}

	if (inv_player_has("CARROT JUICE") || inv_player_has("BOTTLE") ||
			inv_player_has("SOAPY WATER")) {
		hotspot_set_active("CARROT JUICE ", false);
	} else {
		_series3.show("172jug", 0x700);
	}
}

void Room172::daemon() {
	// TODO
}

void Room172::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(flags)[V092] && _G(player).walk_x >= 357 && _G(player).walk_x <= 639 &&
			_G(player).walk_y >= 311 && _G(player).walk_y <= 353)
		player_hotspot_walk_override(356, 332, 3);

	if (player_said_any("GEAR", "LOOK AT")) {
		if (player_said("PARLOUR"))
			player_set_facing_at(84, 300);

		if (player_said("BASEMENT"))
			player_set_facing_at(171, 254);
	}
}

void Room172::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	// TODO
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
