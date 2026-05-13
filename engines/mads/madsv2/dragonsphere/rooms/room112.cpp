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

#include "mads/madsv2/core/conv.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section1.h"
#include "mads/madsv2/dragonsphere/rooms/room112.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];       /* Sprite series handles */
	int16 sequence[15];     /* Sequence handles      */
	int16 animation[4];     /* Animation handles     */
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

static Scratch scratch;

/* ======================== Triggers ========================= */

#define ROOM_112_DONE_HEALING      60
#define ROOM_112_DONE_UNHEALING    62


static void room_112_init(void) {
	viewing_at_y = ((video_y - display_y) >> 1);
	kernel_init_dialog();  /* clear interface */
	kernel_set_interface_mode(INTER_LIMITED_SENTENCES);

	player.commands_allowed = false;
	player.walker_visible = false;

	if (player.x == 0) {
		aa[0] = kernel_run_animation(kernel_name('g', 1), ROOM_112_DONE_HEALING);
		global[player_score] += 5;
	} else {
		aa[0] = kernel_run_animation(kernel_name('g', 2), ROOM_112_DONE_UNHEALING);
	}

	section_1_music();
}

static void room_112_daemon() {
	if (kernel.trigger == ROOM_112_DONE_HEALING) {
		global[guard_pid_status] = GUARD_IS_HEALED;
		player.x = 0;
		text_show(11027);
		new_room = 110;
	}

	if (kernel.trigger == ROOM_112_DONE_UNHEALING) {
		global[guard_pid_status] = GUARD_IS_UNHEALED;
		player.x = 100;
		new_room = 110;
	}
}

static void room_112_pre_parser() {
	player.need_to_walk = false;
}

static void room_112_parser() {
	// No implementation
}


void room_112_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
}

void room_112_preload() {
	room_init_code_pointer = room_112_init;
	room_pre_parser_code_pointer = room_112_pre_parser;
	room_parser_code_pointer = room_112_parser;
	room_daemon_code_pointer = room_112_daemon;

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
