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

#include "m4/burger/rooms/section3/room303.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const char *Room303::SAID[][4] = {
	{ "JUG ",        "303w022", nullptr,   "303w022z" },
	{ "JUG  ",       "303w022", nullptr,   "303w022z" },
	{ "FUEL TANK",   "303w003", "303w004", nullptr    },
	{ "BURNER",      nullptr,   "303w010", nullptr    },
	{ "BOILER",      "303w014", "303w015", nullptr    },
	{ "STOOL",       "303w019", "300w004", "303w020"  },
	{ "DISTILLED CARROT JUICE ",  "303w021", nullptr, nullptr },
	{ "DISTILLED CARROT JUICE  ", "303w021", nullptr, nullptr },
	{ "CONDENSER",   "303w023", "303w024", "303w025"  },
	{ "KEG",         nullptr,   "303w024", "303w025"  },
	{ "CABIN",       "303w028", "300w002", "300w002"  },
	{ "WINDOW",      "303w029", "300w002", "303w030"  },
	{ "WOOD",        "303w031", "303w032", "303w033"  },
	{ "ROCK",        "303w034", "300w002", "300w002"  },
	{ "TREES",       "303w035", "300w002", "300w002"  },
	{ "FORCE FIELD", "303w036", "300w002", "300w002"  },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesStreamBreak Room303::SERIES1[] = {
	{ 0, "303_001", 2, 255, -1, 0, 0, 0 },
	STREAM_BREAK_END
};

const seriesPlayBreak Room303::PLAY1[] = {
	{ 0, 7, 0, 1, 0, -1, 2048, 0, 0, 0 },
	{ 8, -1, 0, 1, 0, 9, 2048, 0, 0, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room303::PLAY2[] = {
	{  0,  5, nullptr,   1,   0,   -1, 2048, 0, nullptr, 0 },
	{  6,  7, "300w048", 1, 255,   -1,    0, 0, nullptr, 0 },
	{ 10, -1, "303_002", 2, 255,   -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room303::PLAY3[] = {
	{  9, 14, nullptr,   1,   0,   -1, 2050, 0, nullptr, 0 },
	{  0,  8, nullptr,   1,   0,    9, 2050, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room303::PLAY4[] = {
	{  0, -1, nullptr,   1,   0,   -1, 2050, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room303::PLAY5[] = {
	{  0, -1, nullptr,   1,   0,   -1, 2048, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room303::PLAY6[] = {
	{  0, -1, nullptr,   1,   0,   -1, 2048, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room303::PLAY7[] = {
	{  0, -1, nullptr,   1,   0,   -1, 2048, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room303::PLAY8[] = {
	{  0, 31, nullptr,   1,   0,   -1, 2048, 0, nullptr, 0 },
	{ 32, -1, "303_003", 1, 255,   -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room303::PLAY9[] = {
	{  0, 28, nullptr,   1,   0,   -1, 2048, 0, nullptr, 0 },
	{ 29, 36, "300_003", 2, 255,   -1,    0, 0, nullptr, 0 },
	{ 37, -1, nullptr,   0,   0,    5,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room303::PLAY10[] = {
	{  0,  6, nullptr,   1,   0,   -1, 2048, 0, nullptr, 0 },
	{  7,  6, "303_007", 1, 100,   -1,    0, 0, nullptr, 0 },
	{  6,  6, nullptr,   0,   0,   -1,    0, 2, nullptr, 0 },
	{  6,  7, nullptr,   0,   0,   -1,    1, 1, nullptr, 0 },
	{  8, -1, nullptr,   0,   0,   -1,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const char *Room303::getDigi() {
	if (_G(flags)[V118] == 3002) {
		_digiVolume = 125;
		return "303_005";
	} else if (_G(flags)[V117]) {
		return "303_006";
	} else {
		return "300_005";
	}
}

void Room303::init() {
}

void Room303::daemon() {
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
