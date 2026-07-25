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
#include "mads/nebular/rooms/section3.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

static void room_321_init() {
	player.walker_visible = false;
	player.commands_allowed = false;

	inter_reset_dialog();
	kernel_set_interface_mode(INTER_CONVERSATION);

	int suffixNum;
	if (global[kSexOfRex] == REX_FEMALE) {
		global[kSexOfRex] = REX_MALE;
		suffixNum = 1;
	} else {
		global[kSexOfRex] = REX_FEMALE;
		suffixNum = player.been_here_before ? 2 : 0;
	}

	kernel_run_animation(kernel_name('g', suffixNum), 60);
	section_3_music();
}

static void room_321_daemon() {
	if (kernel_anim[0].anim != nullptr) {
		if ((kernel_anim[0].frame >= 260) && (global[kSexOfRex] == REX_MALE) && (config_file.naughtiness >= NICE))
			new_room = 316;
	}

	if (kernel.trigger == 60)
		new_room = 316;
}

void room_321_synchronize(Common::Serializer &s) {
	// No implementation
}

void room_321_preload() {
	room_init_code_pointer = room_321_init;
	room_daemon_code_pointer = room_321_daemon;

	section_3_walker();
	section_3_interface();
}

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS
