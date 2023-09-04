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

#include "m4/burger/rooms/section6/room603.h"
#include "m4/burger/rooms/section6/section6.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const Section6Room::GerbilPoint Room603::GERBIL[] = {
	{ 0, 0 }, { 0, 0 }, { 0, 0 }, { 429, 183 },
	{ 429, 185 }, { 429, 185 }, { 436, 200 }, { 436, 200 },
	{ 436, 200 }, { 422, 188 }, { 422, 188 }, { 422, 188 },
	{ 417, 192 }, { 417, 192 }, { 417, 192 }, { 411, 190 },
	{ 411, 190 }, { 411, 190 }, { 411, 190 }, { 411, 190 },
	{ 411, 190 }, { 403, 182 }, { 403, 182 }, { 403, 182 },
	{ 339, 177 }, { 339, 177 }, { 339, 177 }, { 301, 195 },
	{ 301, 195 }, { 301, 195 }, { 295, 199 }, { 439, 189 },
	{ 439, 189 }, { 295, 199 }, { 431, 190 }, { 431, 190 },
	{ 295, 199 }, { 413, 193 }, { 413, 193 }, { 290, 196 },
	{ 403, 202 }, { 403, 202 }, { 293, 195 }, { 392, 203 },
	{ 392, 203 }, { 293, 195 }, { 389, 222 }, { 389, 222 },
	{ 295, 193 }, { 415, 230 }, { 423, 185 }, { 293, 196 },
	{ 428, 235 }, { 415, 185 }, { 293, 196 }, { 430, 233 },
	{ 420, 204 }, { 293, 196 }, { 430, 233 }, { 420, 204 },
	{ 293, 196 }, { 430, 232 }, { 420, 204 }, { 293, 196 },
	{ 438, 233 }, { 411, 199 }, { 293, 196 }, { 441, 235 },
	{ 408, 198 }, { 293, 196 }, { 444, 235 }, { 403, 197 },
	{ 293, 196 }, { 447, 237 }, { 402, 195 }, { 293, 196 },
	{ 450, 238 }, { 399, 194 }, { 293, 196 }, { 452, 237 },
	{ 397, 194 }, { 293, 196 }, { 455, 235 }, { 395, 194 },
	{ 293, 196 }, { 458, 233 }, { 393, 195 }, { 293, 196 },
	{ 461, 233 }, { 392, 195 }, { 293, 196 }, { 461, 233 },
	{ 392, 195 }, { 293, 196 }, { 461, 233 }, { 392, 195 }
};

static const char *SAID[][4] = {
	{ "WATER DISH", nullptr,   "603w004", "603w005" },
	{ "WATER",      "603w006", nullptr,   nullptr   },
	{ "CARROT",     "603w008", "603w009", "603w010" },
	{ "TUBE",       "603w011", "603w005", nullptr   },
	{ "TUBE ",      "603w011", "603w005", nullptr   },
	{ "TUBE  ",     "603w012", "603w005", nullptr   },
	{ "TUBE   ",    "603w012", "603w005", nullptr   },
	{ "BARS",       "603w013", "603w005", "603w005" },
	{ "FLOOR",      "603w014", "603w005", "603w005" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesPlayBreak Room603::PLAY1[] = {
	{ 0,  0, "603_001", 2, 255, -1, 0, 0, nullptr, 0 },
	{ 1, -1, "603w001", 1, 255, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room603::PLAY2[] = {
	{  0, 22, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 23, 24, "600_015", 2, 255, -1,    0, 0, nullptr, 0 },
	{ 25, -1, nullptr,   1,   0, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room603::PLAY3[] = {
	{ 0,  3, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 4,  5, "600w016", 2, 100, -1,    0, 0, nullptr, 0 },
	{ 6, -1, nullptr,   1,   0, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room603::PLAY4[] = {
	{ 0, -1, nullptr, 1, 0, -1, 2048, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room603::PLAY5[] = {
	{ 0,  2, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 3,  4, "600w015", 2, 100, -1,    0, 0, nullptr, 0 },
	{ 5, -1, nullptr,   1,   0, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room603::PLAY6[] = {
	{ 0,  2, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 3,  4, "600w015", 2, 100, -1,    0, 0, nullptr, 0 },
	{ 5, -1, nullptr,   1,   0, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room603::PLAY7[] = {
	{  0, 17, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 18, -1, "603_004", 1, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room603::PLAY8[] = {
	{  0, 17, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 18, -1, "603_006", 2, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room603::PLAY9[] = {
	{  0, 17, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 18, -1, "600_014", 2, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};


const seriesPlayBreak Room603::PLAY10[] = {
	{ 0,  6, nullptr, 1, 0, -1, 2048, 0, nullptr, 0 },
	{ 7, -1, nullptr, 1, 0,  0,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room603::PLAY11[] = {
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

const seriesPlayBreak Room603::PLAY12[] = {
	{  0, 13, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 14, -1, "600_012", 2, 255, -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room603::PLAY13[] = {
	{ 0,  5, nullptr,   1,   0, -1, 2048,  0, nullptr, 0 },
	{ 6,  6, "603w007", 1, 255, -1,    0, 10, nullptr, 0 },
	{ 5, -1, nullptr,   1, 255, -1,    0,  0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room603::PLAY14[] = {
	{  0, 24, nullptr, 1,   0, -1, 2048, 0, nullptr, 0 },
	{ 25, -1, nullptr, 1, 255,  0,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room603::PLAY15[] = {
	{ 0, -1, nullptr, 1, 0, -1, 2048, 0, nullptr, 0 },
	PLAY_BREAK_END
};

long Room603::_state1;

Room603::Room603() : Section6Room() {
	_gerbilTable = GERBIL;
	_state1 = 0;
}

void Room603::init() {
	player_set_commands_allowed(false);
	_G(flags)[V246] = 0;
	_G(flags)[V264] = 0;

	if (_G(flags)[V269] == 1)
		series_show("602spill", 0x900, 0, -1, -1, 0, 100, 80, 0);
	_G(kernel).continue_handling_trigger = _G(flags)[V269] == 1 ? 1 : 0;

	if (_G(flags)[V270] == 6000) {
		hotspot_set_active("WATER", true);
		_val1 = 22;
		_series1 = series_show("603hole", 0xfff);
	} else {
		hotspot_set_active("WATER", false);
		_val1 = 27;
		kernel_trigger_dispatch_now(0);
	}

	if (_G(flags)[V245] == 10029) {
		Section6::_state1 = 6002;
		kernel_trigger_dispatch_now(6013);
	}

	if (_G(flags)[V243] == 6000) {
		Section6::_state4 = 1;
		kernel_trigger_dispatch_now(6014);
	}
	if (_G(flags)[V243] == 6006) {
		_series2 = series_load("603mg01");
		Section6::_state3 = 6002;
		Section6::_state3 = 6001;
		kernel_timing_trigger(60, 6011);
	}

	switch (_G(game).previous_room) {
	case RESTORING_GAME:
		player_set_commands_allowed(true);
		break;

	case 602:
	case 612:
		_G(wilbur_should) = 3;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		break;

	case 604:
		_G(wilbur_should) = 2;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		break;

	default:
		player_set_commands_allowed(true);
		ws_demand_location(203, 333);
		break;
	}

	if (_G(flags)[V243] == 6006)
		Section6::_state4 = 5;

	if (_G(flags)[V243] == 6007)
		Section6::_state4 = 8;
	else if (_G(flags)[V243] == 6000)
		Section6::_state4 = 1;

	kernel_trigger_dispatch_now(6014);
}

void Room603::daemon() {
}

void Room603::pre_parser() {

}

void Room603::parser() {

}

} // namespace Rooms
} // namespace Burger
} // namespace M4
