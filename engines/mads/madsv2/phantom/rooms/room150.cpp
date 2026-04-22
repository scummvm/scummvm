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

#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mcga.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/phantom/global.h"
#include "mads/madsv2/phantom/rooms/section1.h"
#include "mads/madsv2/phantom/rooms/room150.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_150_init() {
	viewing_at_y = ((video_y - display_y) >> 1);

	player.commands_allowed = false;
	player.walker_visible   = false;

	if (previous_room == 113) {
		aa[0] = kernel_run_animation(kernel_name('l', 1), 1);

	} else if (previous_room == 203) {
		aa[0] = kernel_run_animation(kernel_name('f', 1), 2);
		section_1_music();

	} else if (previous_room == 306) {
		aa[0] = kernel_run_animation(kernel_name('e', 1), 4);

	} else if (previous_room == 208) {
		aa[0] = kernel_run_animation(kernel_name('h', 1), 3);

	} else {
		aa[0] = kernel_run_animation(kernel_name('q', 1), 5);
	}
}

void room_150_daemon() {
	if (kernel.trigger == 1) {
		new_room = 203;
	}

	if (kernel.trigger == 2) {
		new_room = 111;
	}

	if (kernel.trigger == 4) {
		new_room = 204;
	}

	if (kernel.trigger == 3) {
		global[jacques_status] = JACQUES_IS_DEAD;
		new_room = 205;
	}

	if (kernel.trigger == 5) {
		game.going = false;
		win_status = 1;
	}

	if (new_room != room_id) {
		memset(&master_palette[4], 0, sizeof(RGBcolor) * 248);
		mcga_setpal(&master_palette);
	}
}

void room_150_preload() {
	room_init_code_pointer       = room_150_init;
	room_pre_parser_code_pointer = NULL;
	room_parser_code_pointer     = NULL;
	room_daemon_code_pointer     = room_150_daemon;

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
