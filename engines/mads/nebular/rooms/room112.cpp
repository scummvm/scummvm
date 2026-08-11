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

#include "mads/core/config.h"
#include "mads/core/game.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section1.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

static void room_112_init() {
	section_1_music();

	g_sprite_ids[0] = kernel_load_series(kernel_name('X', 0), 0);
	g_sprite_ids[1] = kernel_load_series(kernel_name('X', 1), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('X', 2), 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('X', 5), 0);

	g_sequence_ids[0] = kernel_seq_forward(g_sprite_ids[0], false, 10, 20, 17, 0);
	g_sequence_ids[1] = kernel_seq_forward(g_sprite_ids[1], false, 4, 0, 0, 0);
	g_sequence_ids[2] = kernel_seq_forward(g_sprite_ids[2], false, 6, 0, 3, 0);

	player.commands_allowed = false;
	player.walker_visible = false;

	inter_reset_dialog();
	kernel_set_interface_mode(INTER_CONVERSATION);

	kernel_run_animation(kernel_full_name(112, 'X', -1, "", KERNEL_AA), 70);
}

static void room_112_daemon() {
	if ((kernel_anim[0].anim != nullptr) && (config_file.naughtiness == NICE)) {
		if (kernel_anim[0].frame >= 54) {
			kernel_abort_animation(0);
			kernel.trigger = 70;
		}
	}

	if (kernel.trigger == 70) {
		g_sequence_ids[3] = kernel_seq_forward(g_sprite_ids[3], false, 7, 11, 0, 3);
		kernel_seq_depth(g_sequence_ids[3], 1);
		kernel_seq_trigger(g_sequence_ids[3], KERNEL_TRIGGER_EXPIRE, 0, 71);
	}

	if (kernel.trigger == 71) {
		new_room = 101;
		player.commands_allowed = true;
		player.walker_visible = true;
	}
}

void room_112_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_112_preload() {
	room_init_code_pointer = room_112_init;
	room_daemon_code_pointer = room_112_daemon;

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
