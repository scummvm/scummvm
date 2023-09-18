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

#include "m4/burger/rooms/section3/room304.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {


const char *Room304::SAID[][4] = {
	{ "DOOR",        "304w002", "300w002", nullptr   },
	{ "BED",         "304w007", "300w003", "304w008" },
	{ "PICTURE",     "304w009", "304w010", "300w005" },
	{ "MATCHES ",    "304w003", nullptr,   "304w004" },
	{ "MAP",         nullptr,   "304w013", "304w014" },
	{ "CRATE",       "304w015", "300w004", "300w004" },
	{ "STOVE",       "304w016", "304w017", "304w018" },
	{ "KETTLE",      "304w019", "304w018", "304w018" },
	{ "MUG",         "304w019", nullptr,   nullptr   },
	{ "RECIPE BOOK", nullptr,   "304w021", "304w022" },
	{ "WASHTUB",     "304w023", "304w024", "304w024" },
	{ "POT",         "304w025", "304w018", "304w018" },
	{ "FRYING PAN",  "304w026", "304w027", "304w018" },
	{ "WINDOW",      "304w028", "300w002", "304w029" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const seriesPlayBreak Room304::PLAY1[] = {
	{  0,  3, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{  4, 16, "304_001", 2, 255, -1, 2048, 0, nullptr, 0 },
	{ 17, -1, "304_002", 2, 255, -1, 2048, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room304::PLAY2[] = {
	{  0,  3, nullptr,   1,   0, -1, 2050,  0, nullptr, 0 },
	{  0,  0, "304w020", 1, 255, -1,    0, 10, nullptr, 0 },
	{  0,  6, nullptr,   0,   0, -1,    0,  0, nullptr, 0 },
	{  7,  7, nullptr,   0,   0, -1,    0,  5, nullptr, 0 },
	{  8, 12, nullptr,   0,   0, -1,    0,  0, nullptr, 0 },
	{ 13, -1, "304_004", 2, 255, -1,    0,  0, nullptr, 0 },
	{  7,  7, nullptr,   0,   0, -1,    0,  3, nullptr, 0 },
	{  8, 12, nullptr,   0,   0, -1,    0,  0, nullptr, 0 },
	{ 13, -1, "304_004", 2, 255, -1,    0,  0, nullptr, 0 },
	{  7,  7, nullptr,   0,   0, -1,    0,  5, nullptr, 0 },
	{  7,  3, nullptr,   0,   0, -1,    0,  0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room304::PLAY3[] = {
	{ 0,  3, nullptr, 1, 0, -1, 0, 0, nullptr, 0 },
	{ 4,  5, nullptr, 0, 0,  6, 0, 0, nullptr, 0 },
	{ 6, -1, nullptr, 0, 0, -1, 0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room304::PLAY4[] = {
	{  0, 24, nullptr,   1,   0, -1, 2048, 0, nullptr, 0 },
	{ 25, 36, "300_003", 2, 255, -1, 2048, 0, nullptr, 0 },
	{ 37, -1, nullptr,   0,   0,  3,    0, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const seriesPlayBreak Room304::PLAY5[] = {
	{ 17, 17, nullptr,   0,   0, -1,    0, 5, nullptr, 0 },
	{ 17, 21, "304_002", 2, 255, -1, 2048, 0, nullptr, 0 },
	PLAY_BREAK_END
};

const char *Room304::getDigi() {
	return _G(flags)[130] ? "304_003" : "300_005";
}

void Room304::init() {
	setupDigi();
	// TODO: set_palette_brightness(70);

	if (inv_player_has("MATCHES")) {
		hotspot_set_active("MATCHES ", false);
	} else {
		_matches = series_show("304match", 0xa00);
		hotspot_set_active("MATCHES ", true);
	}

	// TODO
}

void Room304::daemon() {
}

void Room304::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;
}

void Room304::parser() {

}

} // namespace Rooms
} // namespace Burger
} // namespace M4
