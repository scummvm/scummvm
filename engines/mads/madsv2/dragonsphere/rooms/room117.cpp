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
#include "mads/madsv2/dragonsphere/rooms/room117.h"

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

/* ========================= Sprites ========================= */

#define fx_swim_to_left      1 /* rm117c0 */
#define fx_swim_to_right     2 /* rm117c1 */


/* ======================== Triggers ========================= */

#define ROOM_117_SWIM_LEFT   70
#define ROOM_117_SWIM_RIGHT  80
#define ROOM_117_BUBBLES     90


static void room_117_init() {
	viewing_at_y = ((video_y - display_y) >> 1);
	kernel_init_dialog();  /* clear interface */
	kernel_set_interface_mode(INTER_CONVERSATION);

	if (kernel.teleported_in) {
		inter_give_to_player(crystal_ball);
		inter_give_to_player(shifter_ring);
		inter_give_to_player(magic_belt);
		global[player_persona] = PLAYER_IS_PID;
	}

	player.commands_allowed = false;
	player.walker_visible = false;

	if (previous_room == 109) {
		/* Seal comes from Dungeon room */
		ss[fx_swim_to_right] = kernel_load_series(kernel_name('c', 1), false);
		global[no_load_walker] = false;
		kernel_timing_trigger(TENTH_SECOND, ROOM_117_SWIM_RIGHT);

	} else if (previous_room != KERNEL_RESTORING_GAME) {
		/* Seal comes from below Way station rm 113 */
		ss[fx_swim_to_left] = kernel_load_series(kernel_name('c', 0), false);
		kernel_timing_trigger(TENTH_SECOND, ROOM_117_SWIM_LEFT);
	}

	section_1_music();
}

static void room_117_daemon() {
	switch (kernel.trigger) {
	case ROOM_117_SWIM_LEFT:
		seq[fx_swim_to_left] = kernel_seq_forward(ss[fx_swim_to_left], false, 7, 0, 0, 1);
		kernel_seq_trigger(seq[fx_swim_to_left],
			KERNEL_TRIGGER_EXPIRE, 0, ROOM_117_SWIM_LEFT + 1);
		kernel_seq_trigger(seq[fx_swim_to_left],
			KERNEL_TRIGGER_SPRITE, 21, ROOM_117_BUBBLES);
		sound_play(N_RushingWater);
		break;

	case ROOM_117_SWIM_LEFT + 1:
		global[no_load_walker] = true;
		new_room = 109;
		break;

	case ROOM_117_SWIM_RIGHT:
		seq[fx_swim_to_left] = kernel_seq_forward(ss[fx_swim_to_left], false, 7, 0, 0, 1);
		kernel_seq_trigger(seq[fx_swim_to_left],
			KERNEL_TRIGGER_EXPIRE, 0, ROOM_117_SWIM_RIGHT + 1);
		kernel_seq_trigger(seq[fx_swim_to_left],
			KERNEL_TRIGGER_SPRITE, 4, ROOM_117_BUBBLES);
		kernel_seq_trigger(seq[fx_swim_to_left],
			KERNEL_TRIGGER_SPRITE, 24, ROOM_117_BUBBLES);
		sound_play(N_RushingWater);
		break;

	case ROOM_117_BUBBLES:
		sound_play(N_WaterBubbles);
		break;

	case ROOM_117_SWIM_RIGHT + 1:
		new_room = 113;
		break;
	}
}

static void room_117_pre_parser() {
	// No implementation
}

static void room_117_parser() {
	// No implementation
}

void room_117_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
}

void room_117_preload() {
	room_init_code_pointer = room_117_init;
	room_pre_parser_code_pointer = room_117_pre_parser;
	room_parser_code_pointer = room_117_parser;
	room_daemon_code_pointer = room_117_daemon;

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
