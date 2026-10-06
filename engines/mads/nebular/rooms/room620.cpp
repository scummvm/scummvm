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

static void room_620_init() {
	g_sprite_ids[1] = kernel_load_series(kernel_name('b', 0), 0);
	g_sequence_ids[1] = kernel_seq_stamp(g_sprite_ids[1], false, -1);
	player.commands_allowed = false;
	player.walker_visible = false;
	kernel_timing_trigger(30, 70);
	kernel_set_interface_mode(INTER_LIMITED_SENTENCES);
	section_6_music();
}

static void room_620_daemon() {
	switch (kernel.trigger) {
	case 70:
		kernel_seq_delete(g_sequence_ids[1]);
		kernel_run_animation(kernel_name('E', -1), 71);
		break;

	case 71:
		if (previous_room == 751) {
			global[kCityFlooded] = true;
			global[kTeleporterRoom + 5] = 0;
			new_room = 701;
		} else if (previous_room == 752) {
			global[kCityFlooded] = true;
			global[kTeleporterRoom + 5] = 0;
			new_room = 702;
		} else if (previous_room < 501 || previous_room > 752) {
			global[kCityFlooded] = true;
			global[kTeleporterRoom + 5] = 0;
			new_room = previous_room;
		} else if (previous_room <= 612) {
			global[kResurrectRoom] = global[kHoverCarLocation];
			inter_give_to_player(OBJ_TIMEBOMB);
			global[kTimebombStatus] = 0;
			global[kTimebombTimer] = 0;
			new_room = 605;
		}
		player.commands_allowed = true;
		break;

	default:
		break;
	}
}

void room_620_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_620_preload() {
	room_init_code_pointer = room_620_init;
	room_daemon_code_pointer = room_620_daemon;

	*player.series_name = '\0';
	section_6_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
