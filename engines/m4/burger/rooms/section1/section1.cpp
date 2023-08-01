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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"
#include "m4/m4.h"

namespace M4 {
namespace Burger {
namespace Rooms {

Section1::Section1() : Rooms::Section() {
	add(&_room101);
}

void Section1::updateWalker(int x, int y, int dir, int trigger, bool mode) {
	Section1 *s = dynamic_cast<Section1 *>(g_engine->_activeSection);
	assert(s);
	s->updateWalker_(x, y, dir, trigger, mode);
}

void Section1::updateWalker_(int x, int y, int dir, int trigger, bool mode) {
	_trigger = trigger;
	player_set_commands_allowed(false);
	ws_demand_location(_G(my_walker), x, y);
	ws_demand_facing(_G(my_walker), dir);
	ws_hide_walker(_G(my_walker));
	_G(roomVal3) = 0;

	gr_backup_palette();
	pal_fade_set_start(_G(master_palette), 0);
	_series = series_load("110", -1, _G(master_palette));
	_play = series_play_("110", 0, 0, -1, 600, -1, 100, 0, 0, 0, 0);

	kernel_trigger_dispatch_now(mode ? 1032 : 1027);
	kernel_timing_trigger(1, 1026);
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
