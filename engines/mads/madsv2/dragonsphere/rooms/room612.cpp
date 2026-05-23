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
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section6.h"
#include "mads/madsv2/dragonsphere/rooms/room612.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];       /* Sprite series handles */
	int16 sequence[15];     /* Sequence handles      */
	int16 animation[4];     /* Animation handles     */
	int16 last_arc;

	int16 tele_frame;       /* animation frame being held for tele stuff */
	int16 anim_0_running;

	int16 count;
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation


/* ========================= Sprite Series =================== */

#define fx_gear_1               0       /* rm612x4  */
#define fx_gear_2               1       /* rm612x5  */
#define fx_ghost_1              2       /* rm612x1  */
#define fx_ghost_2              3       /* rm612x2  */
#define fx_ghost_3              4       /* rm612x3  */
#define fx_ladder               5       /* rm612x0  */
#define fx_arc                  6       /* rm612x6  */
#define fx_sparkle              7       /* rm612x7  */
#define fx_fire_1               8       /* rm612y0  */
#define fx_fire_2               9       /* rm612y1  */
#define fx_fire_3               10      /* rm612y2  */
#define fx_fire_4               11      /* rm612y3  */

/* ========================= Triggers ======================== */

#define CLANK                   60

/* ========================= Other Macros ==================== */

#define PLAYER_X_FROM_606       137
#define PLAYER_Y_FROM_606       151

#define PLAYER_BEGIN_X          65
#define PLAYER_BEGIN_Y          149

#define PLAYER_END_X            73
#define PLAYER_END_Y            153

static Scratch scratch;


static void room_612_init() {
	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
	}

	local->count = 0;

	/* ===================== Load Sprite Series ================== */

	ss[fx_fire_1] = kernel_load_series(kernel_name('y', 0), false);
	ss[fx_fire_2] = kernel_load_series(kernel_name('y', 1), false);
	ss[fx_fire_3] = kernel_load_series(kernel_name('y', 2), false);
	ss[fx_fire_4] = kernel_load_series(kernel_name('y', 3), false);

	seq[fx_fire_1] = kernel_seq_forward(ss[fx_fire_1], false, 5, 0, 0, 0);
	kernel_seq_depth(seq[fx_fire_1], 12);
	kernel_seq_range(seq[fx_fire_1], KERNEL_FIRST, KERNEL_LAST);

	seq[fx_fire_2] = kernel_seq_forward(ss[fx_fire_2], false, 5, 0, 0, 0);
	kernel_seq_depth(seq[fx_fire_2], 12);
	kernel_seq_range(seq[fx_fire_2], KERNEL_FIRST, KERNEL_LAST);

	seq[fx_fire_3] = kernel_seq_forward(ss[fx_fire_3], false, 5, 0, 0, 0);
	kernel_seq_depth(seq[fx_fire_3], 12);
	kernel_seq_range(seq[fx_fire_3], KERNEL_FIRST, KERNEL_LAST);

	seq[fx_fire_4] = kernel_seq_forward(ss[fx_fire_4], false, 5, 0, 0, 0);
	kernel_seq_depth(seq[fx_fire_4], 12);
	kernel_seq_range(seq[fx_fire_4], KERNEL_FIRST, KERNEL_LAST);


	ss[fx_gear_1] = kernel_load_series(kernel_name('x', 4), false);
	ss[fx_gear_2] = kernel_load_series(kernel_name('x', 5), false);
	ss[fx_ghost_1] = kernel_load_series(kernel_name('x', 1), false);
	ss[fx_ghost_2] = kernel_load_series(kernel_name('x', 2), false);
	ss[fx_ghost_3] = kernel_load_series(kernel_name('x', 3), false);
	ss[fx_ladder] = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_arc] = kernel_load_series(kernel_name('x', 6), false);
	ss[fx_sparkle] = kernel_load_series(kernel_name('x', 7), false);

	seq[fx_arc] = kernel_seq_stamp(ss[fx_arc], false, 1);
	kernel_seq_depth(seq[fx_arc], 10);
	kernel_timing_trigger(7, 10); /* activate arcs */

	kernel_timing_trigger(imath_random(100, 500), 1); /* activate ghosts */
	kernel_timing_trigger(imath_random(100, 500), 3);
	kernel_timing_trigger(imath_random(100, 500), 5);

	kernel_timing_trigger(1, 7); /* activate jacob's ladder */

	seq[fx_sparkle] = kernel_seq_forward(ss[fx_sparkle], false, 7, 0, 0, 0);
	kernel_seq_depth(seq[fx_sparkle], 10);
	kernel_seq_range(seq[fx_sparkle], KERNEL_FIRST, KERNEL_LAST);

	seq[fx_gear_1] = kernel_seq_forward(ss[fx_gear_1], false, 8, 0, 0, 0);
	kernel_seq_depth(seq[fx_gear_1], 10);
	kernel_seq_range(seq[fx_gear_1], KERNEL_FIRST, KERNEL_LAST);

	seq[fx_gear_2] = kernel_seq_forward(ss[fx_gear_2], false, 7, 0, 0, 0);
	kernel_seq_depth(seq[fx_gear_2], 10);
	kernel_seq_range(seq[fx_gear_2], KERNEL_FIRST, KERNEL_LAST);
	kernel_seq_trigger(seq[fx_gear_2], KERNEL_TRIGGER_SPRITE, 2, CLANK);


	/* ========================= Previous Rooms ================== */

	if (previous_room == 613) {  /* return from flood of 613 */
		player.x = PLAYER_END_X;
		player.y = PLAYER_END_Y;
		player.facing = FACING_EAST;
		local->anim_0_running = true;
		player.walker_visible = false;
		player.commands_allowed = false;
		aa[0] = kernel_run_animation(kernel_name('w', 1), 0);
		kernel_reset_animation(aa[0], 37);

	} else if ((previous_room == 606) || (previous_room != KERNEL_RESTORING_GAME)) {
		player.x = PLAYER_X_FROM_606;
		player.y = PLAYER_Y_FROM_606;
		player.facing = FACING_NORTH;
	}

	section_6_music();
}

static void handle_animation_tele() {
	int tele_reset_frame;

	if (kernel_anim[aa[0]].frame != local->tele_frame) {
		local->tele_frame = kernel_anim[aa[0]].frame;
		tele_reset_frame = -1;

		switch (local->tele_frame) {
		case 21:  /* start of putting up teleporter door */
			inter_move_object(teleport_door, NOWHERE);
			global[player_score] += 5;
			break;

		case 36:  /* end of putting up teleporter door in window */
			new_room = 613;
			break;

		case 37:
			++local->count;
			if (local->count < 10) {
				tele_reset_frame = 36;
			}
			break;

		case 56:  /* just after teleporter falls out window */
			text_show(61206);
			break;

		case 86:  /* end of walking out of water */
			kernel_abort_animation(aa[0]);
			tele_reset_frame = -1;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			player.walker_visible = true;
			player.commands_allowed = true;
			break;
		}

		if (tele_reset_frame >= 0) {
			kernel_reset_animation(aa[0], tele_reset_frame);
			local->tele_frame = tele_reset_frame;
		}
	}
}

static void room_612_daemon() {
	int tmp;

	if (local->anim_0_running) {
		handle_animation_tele();
	}

	switch (kernel.trigger) {
	case 1:
		seq[fx_ghost_1] = kernel_seq_forward(ss[fx_ghost_1], false, 7, 0, 0, 1);
		kernel_seq_depth(seq[fx_ghost_1], 10);
		kernel_seq_range(seq[fx_ghost_1], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_ghost_1], KERNEL_TRIGGER_EXPIRE, 0, 2);
		sound_play(N_MachineMist);
		break;

	case 2:
		kernel_timing_trigger(imath_random(250, 500), 1);
		break;
	}

	switch (kernel.trigger) {
	case 3:
		seq[fx_ghost_2] = kernel_seq_forward(ss[fx_ghost_2], false, 7, 0, 0, 1);
		kernel_seq_depth(seq[fx_ghost_2], 10);
		kernel_seq_range(seq[fx_ghost_2], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_ghost_2], KERNEL_TRIGGER_EXPIRE, 0, 4);
		sound_play(N_MachineMist);
		break;

	case 4:
		kernel_timing_trigger(imath_random(250, 500), 3);
		break;
	}

	switch (kernel.trigger) {
	case 5:
		seq[fx_ghost_3] = kernel_seq_forward(ss[fx_ghost_3], false, 7, 0, 0, 1);
		kernel_seq_depth(seq[fx_ghost_3], 10);
		kernel_seq_range(seq[fx_ghost_3], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_ghost_3], KERNEL_TRIGGER_EXPIRE, 0, 6);
		sound_play(N_MachineMist);
		break;

	case 6:
		kernel_timing_trigger(imath_random(250, 500), 5);
		break;
	}

	switch (kernel.trigger) {
	case 7:
		seq[fx_ladder] = kernel_seq_forward(ss[fx_ladder], false, 8, 0, 0, 1);
		kernel_seq_depth(seq[fx_ladder], 10);
		kernel_seq_range(seq[fx_ladder], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_ladder], KERNEL_TRIGGER_EXPIRE, 0, 8);
		sound_play(N_MachineZap);
		break;

	case 8:
		kernel_timing_trigger(imath_random(10, 25), 7);
		break;
	}

	switch (kernel.trigger) {
	case 10:
		kernel_seq_delete(seq[fx_arc]);
		tmp = imath_random(1, 3);
		while (tmp == local->last_arc) {
			tmp = imath_random(1, 3);
		}
		local->last_arc = tmp;
		seq[fx_arc] = kernel_seq_stamp(ss[fx_arc], false, local->last_arc);
		kernel_seq_depth(seq[fx_arc], 10);
		kernel_timing_trigger(7, 10);
		break;
	}

	if (kernel.trigger == CLANK) {
		sound_play(N_MachineClank);
	}
}

static void room_612_pre_parser() {
	if (player_said_3(put, teleport_door, window) ||
		player_said_3(put, teleport_door, water_source) ||
		player_said_3(put, teleport_door, water_flow)) {
		player_walk(PLAYER_BEGIN_X, PLAYER_BEGIN_Y, FACING_NORTHWEST);
	}
}

static void room_612_parser() {
	if (player_said_3(put, teleport_door, window) ||
		player_said_3(put, teleport_door, water_source) ||
		player_said_3(put, teleport_door, water_flow)) {
		local->anim_0_running = true;
		player.walker_visible = false;
		player.commands_allowed = false;
		aa[0] = kernel_run_animation(kernel_name('w', 1), 0);
		kernel_synch(KERNEL_ANIM, aa[0], KERNEL_PLAYER, 0);
		player_demand_location(PLAYER_END_X, PLAYER_END_Y);
		player_demand_facing(FACING_EAST);
		goto handled;
	}

	if (player_said_2(walk_through, doorway_to_south)) {
		new_room = 606;
		goto handled;
	}

	if (player.look_around) {
		text_show(61201);
		goto handled;
	}

	if (player_said_2(take_magic_from, infernal_machine)) {
		text_show(963);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(floor)) {
			text_show(61202);
			goto handled;
		}

		if (player_said_1(window)) {
			text_show(61203);
			goto handled;
		}

		if (player_said_1(water_flow) || player_said_1(water_source)) {
			text_show(61209);
			goto handled;
		}

		if (player_said_1(doorway_to_south)) {
			text_show(61212);
			goto handled;
		}

		if (player_said_1(infernal_machine)) {
			text_show(61213);
			goto handled;
		}
	}

	if (player_said_1(window)) {
		if (player_said_1(open) || player_said_1(close)) {
			text_show(61204);
			goto handled;

		} else if (player_said_1(put)) {
			text_show(61207);
			goto handled;

		} else if (player_said_1(throw)) {
			text_show(61208);
			goto handled;
		}
	}

	if (player_said_3(fill, goblet, water_source) ||
		player_said_3(fill, goblet, water_flow) ||
		player_said_3(put, goblet, water_source) ||
		player_said_3(put, goblet, water_flow)) {
		text_show(61218);
		goto handled;
	}

	if (player_said_2(put, water_flow) ||
		player_said_2(put, water_source)) {
		text_show(61211);
		goto handled;
	}

	if (player_said_1(water_source)) {
		if (player_said_1(push) || player_said_1(pull)) {
			text_show(61210);
			goto handled;
		}
	}

	if (player_said_1(infernal_machine)) {
		if (player_said_1(open) || player_said_1(close)) {
			text_show(61214);
			goto handled;

		} else if (player_said_1(push) || player_said_1(pull)) {
			text_show(61215);
			goto handled;

		} else if (player_said_1(throw) || player_said_1(put)) {
			text_show(61217);
			goto handled;

		} else if (player_said_1(talk_to)) {
			text_show(61216);
			goto handled;
		}
	}


	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_612_synchronize(Common::Serializer &s) {
	for (int i = 0; i < 15; i++) s.syncAsSint16LE(local->sprite[i]);
	for (int i = 0; i < 15; i++) s.syncAsSint16LE(local->sequence[i]);
	for (int i = 0; i < 4; i++)  s.syncAsSint16LE(local->animation[i]);
	s.syncAsSint16LE(local->last_arc);
	s.syncAsSint16LE(local->tele_frame);
	s.syncAsSint16LE(local->anim_0_running);
	s.syncAsSint16LE(local->count);
}

void room_612_preload() {
	room_init_code_pointer = room_612_init;
	room_pre_parser_code_pointer = room_612_pre_parser;
	room_parser_code_pointer = room_612_parser;
	room_daemon_code_pointer = room_612_daemon;

	section_6_walker();
	section_6_interface();

	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
