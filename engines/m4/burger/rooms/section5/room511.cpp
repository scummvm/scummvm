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

#include "m4/burger/rooms/section5/room511.h"
#include "m4/burger/rooms/section5/section5.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

const seriesStreamBreak Room511::SERIES1[] = {
	{ 0, "511b001a", 2, 255, -1, 0, &_state1,  1 },
	{ 0, "511b001b", 2, 255, -1, 0, &_state1,  2 },
	{ 0, "511b001c", 2, 255, -1, 0, &_state1,  3 },
	{ 0, "511b001d", 2, 255, -1, 0, &_state1,  4 },
	{ 0, "511b001e", 2, 255, -1, 0, &_state1,  5 },
	{ 0, "511b001f", 2, 255, -1, 0, &_state1,  6 },
	{ 0, "511b001g", 2, 255, -1, 0, &_state1,  7 },
	{ 0, "511b001h", 2, 255, -1, 0, &_state1,  8 },
	{ 0, "511b001e", 2, 255, -1, 0, &_state1,  9 },
	{ 0, "511b001j", 2, 255, -1, 0, &_state1, 10 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room511::SERIES2[] = {
	{  0, "608_003",  3, 100, -1, 0, nullptr, 0 },
	{  1, "512f003a", 1, 255, -1, 0, &_state2, 11 },
	{  1, "512f003b", 1, 255, -1, 0, &_state2, 12 },
	{  1, "512f001a", 1, 255, -1, 0, &_state2, 21 },
	{  1, "512f001b", 1, 255, -1, 0, &_state2, 22 },
	{  1, "512f001c", 1, 255, -1, 0, &_state2, 23 },
	{  1, "512f002a", 1, 255, -1, 0, &_state2, 31 },
	{ 33, "608_001",  2, 200, -1, 0, nullptr,   0 },
	{ 48, "608_002",  2, 200, -1, 0, nullptr,   0 },
	STREAM_BREAK_END
};

const seriesStreamBreak Room511::SERIES3[] = {
	{   0, "513_001", 3,  40, -1, (uint)-1, nullptr, 0 },
	{   0, "513w001", 1, 255, -1,  0, nullptr, 0 },
	{  33, "513w002", 1, 255, -1,  0, nullptr, 0 },
	{  44, "513z001", 1, 255, -1,  0, nullptr, 0 },
	{  51, "513w003", 1, 255, -1,  0, nullptr, 0 },
	{  67, "513z002", 1, 255, -1,  0, nullptr, 0 },
	{ 117, "513z003", 1, 255, -1,  0, nullptr, 0 },
	{ 196, "513w005", 1, 255, -1,  0, nullptr, 0 },
	{ 209, "513z004", 1, 255, -1,  0, nullptr, 0 },
	{ 244, "513w006", 1, 255, -1,  0, nullptr, 0 },
	{ 292, "513z005", 1, 255, -1,  0, nullptr, 0 },
	{ 310, "513z006", 1, 255, -1,  0, nullptr, 0 },
	STREAM_BREAK_END
};

long Room511::_state1;
long Room511::_state2;


Room511::Room511() : Section5Room() {
	_state1 = 0;
	_state2 = 0;
}

void Room511::init() {
}

void Room511::daemon() {
}

void Room511::pre_parser() {

}

void Room511::parser() {

}

} // namespace Rooms
} // namespace Burger
} // namespace M4
