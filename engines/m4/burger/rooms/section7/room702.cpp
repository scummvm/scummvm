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

#include "m4/burger/rooms/section7/room702.h"
#include "m4/burger/rooms/section7/section7.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const char *SAID[][4] = {
	{ "DOOR",          nullptr,    "702W002", nullptr   },
	{ "FLUMIX",        "702w001z", "702W002", "702W002" },
	{ "ABDUCT-O-TRON", "702W006",  nullptr,   "702W007" },
	{ "ABDUCT-O CONTROLS",    "702W008", nullptr,   "702W009" },
	{ "NEUTRINO PULSE GUN",   "702W010", nullptr,   "702W011" },
	{ "NEUTRINO GUN CONTROL", "702W012", nullptr,   "702W009" },
	{ "VENT",          "702W013",  nullptr,   "702W013" },
	{ "BRIG",          "702W014",  "702W002", "702W015" },
	{ "MIRROR",        "999W010", nullptr,    "999W010" },
	{ nullptr, nullptr, nullptr, nullptr }
};


void Room702::init() {
}

void Room702::daemon() {
}

void Room702::pre_parser() {
}

void Room702::parser() {
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
