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

#include "mads/core/game.h"
#include "mads/nebular/global.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/mads/inventory.h"
#include "mads/nebular/mads/words.h"
#include "mads/nebular/rooms/section6.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

static void room_605_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('r', -1), 0);
	g_sprite_ids[2] = kernel_load_series(kernel_name('b', -1), 0);
	g_sprite_ids[3] = kernel_load_series(kernel_name('l', -1), 0);
	g_sprite_ids[4] = kernel_load_series(kernel_name('p', -1), 0);
	g_sprite_ids[5] = kernel_load_series(kernel_name('n', -1), 0);
	g_sprite_ids[6] = kernel_load_series(kernel_name('f', -1), 0);

	g_sequence_ids[1] = kernel_seq_pingpong(g_sprite_ids[1], false, 15, 0, 0, 0);
	g_sequence_ids[2] = kernel_seq_pingpong(g_sprite_ids[2], false, 17, 0, 0, 0);
	g_sequence_ids[3] = kernel_seq_pingpong(g_sprite_ids[3], false, 14, 0, 0, 0);
	g_sequence_ids[4] = kernel_seq_pingpong(g_sprite_ids[4], false, 13, 0, 0, 0);
	g_sequence_ids[5] = kernel_seq_pingpong(g_sprite_ids[5], false, 17, 0, 0, 0);
	g_sequence_ids[6] = kernel_seq_pingpong(g_sprite_ids[6], false, 18, 0, 0, 0);

	player.walker_visible = false;
	player.commands_allowed = false;
	kernel_timing_trigger(600, 70);
	kernel_set_interface_mode(INTER_LIMITED_SENTENCES);
	section_6_music();
	g_engine->_soundManager->command(22, 0);
}

static void room_605_daemon() {
	if (kernel.trigger == 70) {
		g_engine->_soundManager->command(23, 0);
		if (global[kResurrectRoom] >= 700)
			text_show(60598);
		else
			text_show(60599);

		new_room = global[kResurrectRoom];
	}
}

void room_605_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_605_preload() {
	room_init_code_pointer = room_605_init;
	room_daemon_code_pointer = room_605_daemon;

	*player.series_name = '\0';
	section_6_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
