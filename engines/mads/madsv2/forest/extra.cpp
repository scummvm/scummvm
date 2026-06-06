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

#include "common/textconsole.h"
#include "mads/madsv2/core/global.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/forest/extra.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/journal.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {

void display_interface() {
	error("TODO: display_interface");
}

void fly_on_screen(int flying_object) {
	error("TODO: fly_on_screen");
}

void fly_off_screen(int flying_object) {
	error("TODO: fly_off_screen");
}

void display_inventory(void) {
	error("TODO: display_inventory");
}

void solve_me_selected(void) {
	error("TODO: solve_me_selected");
}

void door_selected(void) {
	error("TODO: door_selected");
}

void extra_spinning_object(void) {
	error("TODO: extra_spinning_object");
}

void stamp_sprite_to_interface(int x, int y, int sprite, int series) {
	error("TODO: stamp_sprite_to_interface");
}

void delete_sprite_in_interface(int series) {
	matte_deallocate_series(series, false);
}

void extra_change_animation(int handle, int x, int y, byte scale, byte depth) {
	error("TODO: extra_change_animation");
}

void extra_shift_animation(int handle, int x, int y, byte scale) {
	error("TODO: extra_shift_animation");
}

void extra_blank_knothole(void) {
	error("TODO: extra_blank_knothole");
}

void do_interface_for_ouaf() {
	if (mouse_y > 156 &&
		mouse_stop_stroke &&
		player.commands_allowed &&
		/* global[4] == -1 && */
		!kernel.trigger &&
		/* player.command_ready && */
		inter_input_mode == INTER_LIMITED_SENTENCES &&
		!global[inventory_is_displayed]
		/* pl conv_control.running < 0 */) {
		if (room_id == 199) {
			// Taranjeet's Journal
			leave_journal();

		} else if (mouse_x < 64) {
			display_journal();
		} else if (mouse_x < 139) {
			display_inventory();
		} else if (mouse_x < 195) {
		} else if (mouse_x < 250) {
			solve_me_selected();
		} else {
			door_selected();
		}
	}

	if (kernel.trigger == 40) {
		display_inventory();
	}
}

} // namespace Forest
} // namespace MADSV2
} // namespace MADS
