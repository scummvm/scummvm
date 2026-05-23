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
#include "mads/madsv2/dragonsphere/rooms/section5.h"
#include "mads/madsv2/dragonsphere/rooms/room557.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];
	int16 sequence[15];
	int16 animation[4];
	int16 king_frame;
	int16 king_action;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define fx_rock_1            0
#define fx_rock_2            1

#define ROOM_557_ROCK_SOUND  60


static void room_557_init() {
	ss[fx_rock_1]  = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_rock_2]  = kernel_load_series(kernel_name('x', 1), false);

	viewing_at_y = ((video_y - display_y) >> 1);
	kernel_init_dialog();
	kernel_set_interface_mode(INTER_LIMITED_SENTENCES);

	player.commands_allowed = false;
	player.walker_visible   = false;

	aa[0] = kernel_run_animation(kernel_name('k', 1), 0);
}

static void room_557_daemon() {
	int king_reset_frame;

	if (kernel.trigger == ROOM_557_ROCK_SOUND) {
		sound_play(N_RockClatter);
	}

	if (kernel_anim[aa[0]].frame != local->king_frame) {
		local->king_frame = kernel_anim[aa[0]].frame;
		king_reset_frame = -1;

		switch (local->king_frame) {
		case 153:
			seq[fx_rock_1] = kernel_seq_forward(ss[fx_rock_1], false, 5, 0, 0, 1);
			kernel_seq_depth(seq[fx_rock_1], 1);
			kernel_seq_range(seq[fx_rock_1], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_rock_1], KERNEL_TRIGGER_SPRITE, 5, ROOM_557_ROCK_SOUND);
			kernel_seq_trigger(seq[fx_rock_1], KERNEL_TRIGGER_SPRITE, 15, ROOM_557_ROCK_SOUND);
			break;

		case 225:
			seq[fx_rock_2] = kernel_seq_forward(ss[fx_rock_2], false, 5, 0, 0, 1);
			kernel_seq_depth(seq[fx_rock_2], 1);
			kernel_seq_range(seq[fx_rock_2], KERNEL_FIRST, KERNEL_LAST);
			break;

		case 254:
			new_room = 508;
			break;
		}

		if (king_reset_frame >= 0) {
			kernel_reset_animation(aa[0], king_reset_frame);
			local->king_frame = king_reset_frame;
		}
	}
}

static void room_557_pre_parser() {
}

void room_557_parser() {
}

void room_557_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.king_frame);
	s.syncAsSint16LE(scratch.king_action);
}

void room_557_preload() {
	room_init_code_pointer       = room_557_init;
	room_pre_parser_code_pointer = room_557_pre_parser;
	room_parser_code_pointer     = room_557_parser;
	room_daemon_code_pointer     = room_557_daemon;

	section_5_walker();
	section_5_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
