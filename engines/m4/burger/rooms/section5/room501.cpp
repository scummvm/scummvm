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

#include "m4/burger/rooms/section5/room501.h"
#include "m4/burger/rooms/section5/section5.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

static const seriesStreamBreak SERIES1[] = {
	{   0, "501_013", 3,  60, -1, (uint32)-1, nullptr, 0 },
	{   5, "501_010", 2, 255, -1,  0, nullptr, 0 },
	{  15, "501_011", 2, 255, -1,  0, nullptr, 0 },
	{  38, "501z001", 1, 255, -1,  0, nullptr, 0 },
	{  61, "501z002", 1, 255, -1,  0, nullptr, 0 },
	{ 109, "501_001", 2, 255, -1,  0, nullptr, 0 },
	{ 127, "501_002", 2, 255, -1,  0, nullptr, 0 },
	{ 135, "501_003", 2, 255, -1,  0, nullptr, 0 },
	{ 142, "501_004", 2, 255, -1,  0, nullptr, 0 },
	{ 153, "501_005", 2, 255, -1,  0, nullptr, 0 },
	{ 175, "501z003", 1, 255, -1,  0, nullptr, 0 },
	{ 195, "501w001", 1, 255, -1,  0, nullptr, 0 },
	{ 229, "501z004", 1, 255, -1,  0, nullptr, 0 },
	{ 278, "501_006", 2, 255, -1,  0, nullptr, 0 },
	{ 286, "501_007", 2, 255, -1,  0, nullptr, 0 },
	{ 305, "501_008", 2, 255, -1,  0, nullptr, 0 },
	{ 336, "501p001", 1, 255, -1,  0, nullptr, 0 },
	{ 340, "501z005", 2, 255, -1,  0, nullptr, 0 },
	{ 387, "501_014", 2, 125, -1,  0, nullptr, 0 },
	{ 391, "501p002", 1, 255, -1,  0, nullptr, 0 },
	{ 409, "501z006", 1, 255, -1,  0, nullptr, 0 },
	{ 424, "501p003", 1, 255, -1,  0, nullptr, 0 },
	{ 452, "501_009", 2, 255, -1,  0, nullptr, 0 },
	{ 465, "501p004", 1, 255, -1,  0, nullptr, 0 },
	{ 506, "501w002", 1, 255, -1,  0, nullptr, 0 },
	{ 530, "501p005", 1, 255, -1,  0, nullptr, 0 },
	{ 541, "501p006", 1, 255, -1,  0, nullptr, 0 },
	{ 584, "501z007", 1, 255, -1,  0, nullptr, 0 },
	{ 656, "501p007", 1, 255, -1,  0, nullptr, 0 },
	STREAM_BREAK_END
};

void Room501::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room501::init() {
	g_vars->_flags.reset4();
	kernel_trigger_dispatch_now(1);
}

void Room501::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		digi_preload_stream_breaks(SERIES1);
		pal_mirror_colours(118, 122);
		pal_cycle_init(118, 127, 6, -1, -1);
		series_stream_with_breaks(SERIES1, "501intro", 6, 0x100, 5006);
		break;

	case 5006:
		_G(game).new_room = 502;
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
