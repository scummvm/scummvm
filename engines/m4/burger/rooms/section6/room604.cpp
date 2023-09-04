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

#include "m4/burger/rooms/section6/room604.h"
#include "m4/burger/rooms/section6/section6.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const char *SAID[][4] = {
	{ "WOOD SHAVINGS", nullptr,   "604w010", nullptr   },
	{ "ASHES",         "604w015", "604w016", "604w016" },
	{ "APPLE CORE",    "604w017", "604w018", "604w019" },
	{ "BOTTLE CAP",    "604w020", "604w021", "604w010" },
	{ "TUBE",          "604w022", "604w023", nullptr   },
	{ "TUBE ",         "604w024", "604w023", nullptr   },
	{ "BARS",          "604w025", "604w023", "604w023" },
	{ "FLOOR",         "604w026", "604w023", "604w023" },
	{ nullptr, nullptr, nullptr, nullptr }
};

static const seriesStreamBreak SERIES1[] = {
	{   4, "604I001",  1, 255, -1, 0, nullptr, 0 },
	{  29, "604Z001",  1, 255, -1, 0, nullptr, 0 },
	{  36, "604I002",  1, 255, -1, 0, nullptr, 0 },
	{  45, "604Z002",  1, 255, -1, 0, nullptr, 0 },
	{  70, "604Z003",  1, 255, -1, 0, nullptr, 0 },
	{ 106, "604I003",  1, 255, -1, 0, nullptr, 0 },
	{ 132, "604Z004A", 1, 255, -1, 0, nullptr, 0 },
	{ 208, "604Z004B", 1, 255, -1, 0, nullptr, 0 },
	{ 245, "604Z004C", 1, 255, -1, 0, nullptr, 0 },
	{ 324, "604I004",  1, 255, -1, 0, nullptr, 0 },
	{ 344, "604Z005",  1, 255, -1, 0, nullptr, 0 },
	{ 374, "604Z006",  1, 255, -1, 0, nullptr, 0 },
	{ 409, "604I005",  1, 255, -1, 0, nullptr, 0 },
	{ 459, "604Z007",  1, 255, -1, 0, nullptr, 0 },
	STREAM_BREAK_END
};

const seriesPlayBreak Room604::PLAY1[] = {
	{ 0, -1, "604_007", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room604::PLAY2[] = {
	{ 0,  2, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 3,  4, "600w016", 1, 100, -1,    0, 0, nullptr, 0 },
	{ 5, -1, nullptr,   1,   0, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room604::PLAY3[] = {
	{ 0,  2, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 3,  4, "600w015", 1, 100, -1,    0, 0, nullptr, 0 },
	{ 5, -1, nullptr,   1,   0, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room604::PLAY4[] = {
	{  0, 19, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 20, 21, "600_015", 2, 255, -1,    0, 0, nullptr, 0 },
	{ 22, -1, nullptr,   1,   0, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room604::PLAY5[] = {
	{  0, 22, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 23, -1, "600_015", 2, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room604::PLAY6[] = {
	{ 0,  5, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 6, -1, "604_001", 2, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room604::PLAY7[] = {
	{ 0,  5, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 6, -1, "604_001", 2, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room604::PLAY8[] = {
	{ 0,  6, nullptr,    1,   0, -1, 2048, 0, &_state1, 0 },
	{ 0,  6, nullptr,    1,   0, -1, 2048, 0, &_state1, 1 },
	{ 0,  6, nullptr,    1,   0, -1, 2048, 0, &_state1, 2 },
	{ 7, -1, "600w011a", 1, 255, -1,    0, 0, &_state1, 0 },
	{ 7, -1, "600w011b", 1, 255, -1,    0, 0, &_state1, 1 },
	{ 7, -1, "600w011c", 1, 255, -1,    0, 0, &_state1, 2 },
	{ 0, -1, "600w011d", 1, 255, -1,    0, 0, &_state1, 3 },
	{ 0, -1, "600w011e", 1, 255, -1,    0, 0, &_state1, 4 },
	{ 0, -1, "600w011f", 1, 255, -1,    0, 0, &_state1, 5 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room604::PLAY9[] = {
	{  0, 13, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 14, -1, "600_012", 2, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room604::PLAY10[] = {
	{  0,  3, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{  4, 14, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{ 15, 23, "604_008", 1, 255, -1,    0, 0, nullptr, 0 },
	{  4, 14, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{ 15, 23, "604_008", 1, 255, -1,    0, 0, nullptr, 0 },
	{  4, 14, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	{ 15, 23, "604_008", 1, 255, -1,    0, 0, nullptr, 0 },
	{ 24, -1, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room604::PLAY11[] = {
	{  0,  7, nullptr, 1, 0,    -1, 2048, 0, nullptr, 0 },
	{  8, 17, nullptr, 1, 0,    -1,    0, 0, nullptr, 0 },
	{ 18, -1, nullptr, 1, 0, 10016,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room604::PLAY12[] = {
	{ 0, 15,  nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 16, -1, "600_014", 2, 255, -1,    0, 0, nullptr, 0 },
	{ -1, -1, nullptr,   0,   0, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

long Room604::_state1;

Room604::Room604() : Section6Room() {
	_state1 = 0;
}

void Room604::init() {
	player_set_commands_allowed(false);
	_G(flags)[V246] = 0;

	if (_G(flags)[V274]) {
		hotspot_set_active("WOOD SHAVINGS", false);
		hotspot_set_active("ASHES", true);
		_val1 = 27;
		kernel_trigger_dispatch_now(6012);

	} else {
		hotspot_set_active("WOOD SHAVINGS", true);
		hotspot_set_active("ASHES", false);

		if (_G(game).previous_room != 601) {
			_val1 = 25;
			kernel_trigger_dispatch_now(6012);
		}
	}

	_G(flags)[V264] = 0;

	if (_G(flags)[V273] == 1) {
		series_show("602spill", 0x900);
		_G(kernel).call_daemon_every_loop = true;
	}

	if (_G(flags)[V245] == 10030) {
		Section6::_state1 = 6002;
		kernel_trigger_dispatch_now(6013);
	}

	if (_G(flags)[V243] == 6006) {
		_G(wilbur_should) = 18;
		Section6::_state3 = 6004;
		series_stream("604mg06", 4, 0xc00, 6011);
		series_play("604mg06s", 4, 0xc80);
		player_set_commands_allowed(false);
	}

	switch (_G(flags)[V243]) {
	case 6000:
		Section6::_state4 = 0;
		break;
	case 6006:
		Section6::_state4 = 5;
		break;
	case 6007:
		Section6::_state4 = 7;
		break;
	default:
		break;
	}

	kernel_trigger_dispatch_now(6014);

	switch (_G(game).previous_room) {
	case RESTORING_GAME:
		player_set_commands_allowed(true);
		break;

	case 601:
		_sectionSeries1 = series_load("604wi04");
		Section6::_series603 = series_load("604wi04s");
		_G(wilbur_should) = 1;

		ws_demand_location(328, 317, 2);
		ws_hide_walker();
		player_set_commands_allowed(false);

		if (_G(flags)[V242]) {
			_roomSeries1.show("604wi04", 1);
			kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		} else {
			kernel_trigger_dispatch_now(0);
		}
		break;

	case 602:
	case 603:
	case 612:
		if (Section6::_state2) {
			_G(wilbur_should) = 2;
			kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		} else {
			_G(wilbur_should) = 6;
			kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		}
		break;

	default:
		_sectionSeries1 = series_load("604wi04");
		Section6::_series603 = series_load("604wi04s");

		ws_demand_location(328, 317, 2);
		ws_hide_walker();
		player_set_commands_allowed(false);
		_roomSeries1.show("604wi04", 1);
		_G(wilbur_should) = 1;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		break;
	}
}

void Room604::daemon() {
}

void Room604::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(flags)[V243] == 6006 && player_said("TUBE ")) {
		intr_cancel_sentence();
		wilbur_speech("600w003");

	} else if (player_said("RAY GUN", "WOOD SHAVINGS")) {
		_G(player).command_ready = false;

		if (_G(flags)[V274] == 0) {
			_G(wilbur_should) = 17;
			player_hotspot_walk_override(353, 313, 10, gCHANGE_WILBUR_ANIMATION);
		}
	} else if (player_said("WOOD SHAVINGS", "KIBBLE")) {
		_G(wilbur_should) = 10;
		player_hotspot_walk_override(308, 301, 10, gCHANGE_WILBUR_ANIMATION);

	} else if (player_said("RAY GUN", "GERBILS")) {
		_G(wilbur_should) = 8;
		player_hotspot_walk_override(286, 297, 10, gCHANGE_WILBUR_ANIMATION);

	} else if (player_said("KIBBLE", "FLOOR")) {
		if (_G(flags)[V273]) {
			_G(player).need_to_walk = false;
			_G(player).need_to_walk = false;
			wilbur_speech("600w008z");
		} else {
			_G(wilbur_should) = 16;
			player_hotspot_walk_override(425, 303, 9, gCHANGE_WILBUR_ANIMATION);
			_G(player).command_ready = false;
		}

		return;
	}

	_G(player).command_ready = false;
}

void Room604::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (_G(walker).wilbur_said(SAID)) {
		// Already handled
	} else if (player_said("GEAR", "WOOD SHAVINGS")) {
		player_set_commands_allowed(false);

		if (_G(flags)[V243] == 6000) {
			_G(wilbur_should) = 9;
			wilbur_speech(_G(flags)[V248] ? "604w013" : "604w011");

		} else {
			_G(wilbur_should) = 13;
			wilbur_speech("604w005");
		}
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
