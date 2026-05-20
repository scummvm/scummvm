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
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section2.h"
#include "mads/madsv2/dragonsphere/rooms/room204.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];
	int16 sequence[15];
	int16 animation[4];

	int16 king_frame;
	int16 anim_0_running;

	int16 pop_0_running;
	int16 pop_1_running;
	int16 pop_2_running;
	int16 pop_3_running;
	int16 pop_4_running;
	int16 pop_5_running;
	int16 bubble_1_running;
	int16 bubble_2_running;
	int16 smoke_running;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define fx_pop_0                0
#define fx_pop_1                1
#define fx_pop_2                2
#define fx_pop_3                3
#define fx_pop_4                4
#define fx_pop_5                5
#define fx_bubble_1             6
#define fx_bubble_2             7
#define fx_smoke                8
#define fx_take                 9

#define TRIG                    60
#define ROOM_204_POP            70

#define PLAYER_X_FROM_203_K     -15
#define PLAYER_Y_FROM_203_K     127
#define WALK_TO_X_FROM_203_K    173
#define WALK_TO_Y_FROM_203_K    118


static void handle_anim_king_dream() {
	int king_reset_frame;

	if (kernel_anim[aa[0]].frame != local->king_frame) {
		local->king_frame = kernel_anim[aa[0]].frame;
		king_reset_frame = -1;

		switch (local->king_frame) {

			case 64: /* just before doll sequence */
				if (global[doll_given]) {
					king_reset_frame      = 86;
					player.walker_visible = false;
					kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
				}
				break;

			case 83: /* just touched pid doll */
				++ global[dragon_high_scene];
				sound_play(N_TakeObjectSnd);
				global[doll_given] = true;
				inter_give_to_player(pid_doll);
				break;

			case 76: /* take doll */
				player.walker_visible = false;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
				break;
		}

		if (king_reset_frame >= 0) {
			kernel_reset_animation(aa[0], king_reset_frame);
			local->king_frame = king_reset_frame;
		}
	}
}

static void room_204_init() {
	kernel_set_interface_mode(INTER_LIMITED_SENTENCES);
	viewing_at_y = ((video_y - display_y) >> 1);
	player.commands_allowed = false;

	local->anim_0_running   = false;
	local->pop_0_running    = false;
	local->pop_1_running    = false;
	local->pop_2_running    = false;
	local->pop_3_running    = false;
	local->pop_4_running    = false;
	local->pop_5_running    = false;
	local->bubble_1_running = false;
	local->bubble_2_running = false;
	local->smoke_running    = false;

	ss[fx_pop_0]    = kernel_load_series(kernel_name('y', 0), false);
	ss[fx_pop_1]    = kernel_load_series(kernel_name('y', 1), false);
	ss[fx_pop_2]    = kernel_load_series(kernel_name('y', 2), false);
	ss[fx_pop_3]    = kernel_load_series(kernel_name('y', 3), false);
	ss[fx_pop_4]    = kernel_load_series(kernel_name('y', 4), false);
	ss[fx_pop_5]    = kernel_load_series(kernel_name('y', 5), false);
	ss[fx_bubble_1] = kernel_load_series(kernel_name('z', 0), false);
	ss[fx_bubble_2] = kernel_load_series(kernel_name('z', 1), false);
	ss[fx_smoke]    = kernel_load_series(kernel_name('h', 0), false);

	player_first_walk(PLAYER_X_FROM_203_K, PLAYER_Y_FROM_203_K, FACING_EAST,
	                  WALK_TO_X_FROM_203_K, WALK_TO_Y_FROM_203_K, FACING_NORTHEAST, false);

	if (global[player_persona] == PLAYER_IS_KING) {
		ss[fx_take] = kernel_load_series("*KGDRD_7", false);
		player_walk_trigger(1);
	} else {
		player_walk_trigger(5);
	}

	section_2_music();
}

static void room_204_daemon() {
	if (local->anim_0_running) {
		handle_anim_king_dream();
	}

	if (!local->pop_0_running && imath_random(1, 150) == 1) {
		seq[fx_pop_0] = kernel_seq_forward(ss[fx_pop_0], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_pop_0], 10);
		kernel_seq_range(seq[fx_pop_0], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_pop_0], KERNEL_TRIGGER_SPRITE, 8, ROOM_204_POP);
		kernel_seq_trigger(seq[fx_pop_0], KERNEL_TRIGGER_EXPIRE, 0, TRIG);
		local->pop_0_running = true;
	}

	if (!local->pop_1_running && imath_random(1, 150) == 1) {
		seq[fx_pop_1] = kernel_seq_forward(ss[fx_pop_1], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_pop_1], 10);
		kernel_seq_range(seq[fx_pop_1], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_pop_1], KERNEL_TRIGGER_SPRITE, 8, ROOM_204_POP);
		kernel_seq_trigger(seq[fx_pop_1], KERNEL_TRIGGER_EXPIRE, 0, TRIG + 1);
		local->pop_1_running = true;
	}

	if (!local->pop_2_running && imath_random(1, 150) == 1) {
		seq[fx_pop_2] = kernel_seq_forward(ss[fx_pop_2], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_pop_2], 10);
		kernel_seq_range(seq[fx_pop_2], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_pop_2], KERNEL_TRIGGER_SPRITE, 7, ROOM_204_POP);
		kernel_seq_trigger(seq[fx_pop_2], KERNEL_TRIGGER_EXPIRE, 0, TRIG + 2);
		local->pop_2_running = true;
	}

	if (!local->pop_3_running && imath_random(1, 150) == 1) {
		seq[fx_pop_3] = kernel_seq_forward(ss[fx_pop_3], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_pop_3], 10);
		kernel_seq_range(seq[fx_pop_3], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_pop_3], KERNEL_TRIGGER_SPRITE, 7, ROOM_204_POP);
		kernel_seq_trigger(seq[fx_pop_3], KERNEL_TRIGGER_EXPIRE, 0, TRIG + 3);
		local->pop_3_running = true;
	}

	if (!local->pop_4_running && imath_random(1, 150) == 1) {
		seq[fx_pop_4] = kernel_seq_forward(ss[fx_pop_4], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_pop_4], 10);
		kernel_seq_range(seq[fx_pop_4], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_pop_4], KERNEL_TRIGGER_SPRITE, 6, ROOM_204_POP);
		kernel_seq_trigger(seq[fx_pop_4], KERNEL_TRIGGER_EXPIRE, 0, TRIG + 4);
		local->pop_4_running = true;
	}

	if (!local->pop_5_running && imath_random(1, 150) == 1) {
		seq[fx_pop_5] = kernel_seq_forward(ss[fx_pop_5], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_pop_5], 10);
		kernel_seq_range(seq[fx_pop_5], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_pop_5], KERNEL_TRIGGER_SPRITE, 7, ROOM_204_POP);
		kernel_seq_trigger(seq[fx_pop_5], KERNEL_TRIGGER_EXPIRE, 0, TRIG + 5);
		local->pop_5_running = true;
	}

	switch (kernel.trigger) {
		case TRIG:     local->pop_0_running = false; break;
		case TRIG + 1: local->pop_1_running = false; break;
		case TRIG + 2: local->pop_2_running = false; break;
		case TRIG + 3: local->pop_3_running = false; break;
		case TRIG + 4: local->pop_4_running = false; break;
		case TRIG + 5: local->pop_5_running = false; break;
	}

	if (!local->bubble_1_running && imath_random(1, 150) == 1) {
		seq[fx_bubble_1] = kernel_seq_forward(ss[fx_bubble_1], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_bubble_1], 10);
		kernel_seq_range(seq[fx_bubble_1], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_bubble_1], KERNEL_TRIGGER_EXPIRE, 0, TRIG + 6);
		local->bubble_1_running = true;
	}

	if (!local->bubble_2_running && imath_random(1, 150) == 1) {
		seq[fx_bubble_2] = kernel_seq_forward(ss[fx_bubble_2], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_bubble_2], 10);
		kernel_seq_range(seq[fx_bubble_2], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_bubble_2], KERNEL_TRIGGER_EXPIRE, 0, TRIG + 7);
		local->bubble_2_running = true;
	}

	if (!local->smoke_running && imath_random(1, 150) == 1) {
		seq[fx_smoke] = kernel_seq_forward(ss[fx_smoke], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_smoke], 10);
		kernel_seq_range(seq[fx_smoke], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_smoke], KERNEL_TRIGGER_EXPIRE, 0, TRIG + 8);
		local->smoke_running = true;
	}

	switch (kernel.trigger) {
		case TRIG + 6: local->bubble_1_running = false; break;
		case TRIG + 7: local->bubble_2_running = false; break;
		case TRIG + 8: local->smoke_running    = false; break;
	}

	if (kernel.trigger == 1) {
		aa[0] = kernel_run_animation(kernel_name('k', 1), 2);
	}

	if (kernel.trigger == 2) {
		kernel_timing_trigger(10, 3);
	}

	if (kernel.trigger == 3) {
		aa[0]                 = kernel_run_animation(kernel_name('k', 2), 4);
		local->anim_0_running = true;
	}

	if (kernel.trigger == 4) {
		player.walker_visible = true;
		local->anim_0_running = false;
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
		player_walk(PLAYER_X_FROM_203_K, PLAYER_Y_FROM_203_K, FACING_WEST);
		player.walk_off_edge_to_room = 203;
	}

	if (kernel.trigger == 5) {
		aa[0] = kernel_run_animation(kernel_name('p', 1), 6);
	}

	if (kernel.trigger == 6) {
		kernel_timing_trigger(10, 7);
	}

	if (kernel.trigger == 7) {
		player_walk(PLAYER_X_FROM_203_K, PLAYER_Y_FROM_203_K, FACING_WEST);
		player.walk_off_edge_to_room = 203;
	}

	if (kernel.trigger == ROOM_204_POP) {
		sound_play(N_LavaPlops);
	}
}

static void room_204_pre_parser() {
}

static void room_204_parser() {
}

void room_204_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.king_frame);
	s.syncAsSint16LE(scratch.anim_0_running);
	s.syncAsSint16LE(scratch.pop_0_running);
	s.syncAsSint16LE(scratch.pop_1_running);
	s.syncAsSint16LE(scratch.pop_2_running);
	s.syncAsSint16LE(scratch.pop_3_running);
	s.syncAsSint16LE(scratch.pop_4_running);
	s.syncAsSint16LE(scratch.pop_5_running);
	s.syncAsSint16LE(scratch.bubble_1_running);
	s.syncAsSint16LE(scratch.bubble_2_running);
	s.syncAsSint16LE(scratch.smoke_running);
}

void room_204_preload() {
	room_init_code_pointer       = room_204_init;
	room_pre_parser_code_pointer = room_204_pre_parser;
	room_parser_code_pointer     = room_204_parser;
	room_daemon_code_pointer     = room_204_daemon;

	section_2_walker();
	section_2_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
