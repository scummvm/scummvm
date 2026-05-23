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
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section6.h"
#include "mads/madsv2/dragonsphere/rooms/room609.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];       /* Sprite series handles */
	int16 sequence[15];     /* Sequence handles      */
	int16 animation[4];     /* Animation handles     */

	int16 king_frame;       /* animation frame being held for king stuff */
	int16 anim_0_running;

	int16 down_frame;       /* animation frame being held for down stuff */
	int16 anim_1_running;

	int16 prevent;
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation


/* ========================= Sprite Series =================== */

#define fx_chunks               0       /* rm609x0  */
#define fx_torch                1       /* rm609x1  */
#define fx_stone_gone           2       /* rm609p0  */
#define fx_death                3       /* rm609a2  */
#define fx_rope                 4       /* rm609x3  */


/* ========================= Triggers ======================== */

#define ROOM_609_DONE_TORCH     60 
#define ROOM_609_DONE_WALK      70 


/* ========================= Other Macros ==================== */

#define PLAYER_X_FROM_606       241
#define PLAYER_Y_FROM_606       151

#define PLAYER_X_AFTER_TORCH    287
#define PLAYER_Y_AFTER_TORCH    154

#define DEATH_X                 164
#define DEATH_Y                 134

static Scratch scratch;


static void room_609_init() {
	kernel.disable_fastwalk = true;
	local->prevent = false;

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
	}

	if (previous_room != 607) {
		ss[fx_death] = kernel_load_series(kernel_name('a', 2), false);
	}

	if (previous_room == 607) {
		kernel_load_series(kernel_name('a', 3), false);
		/* to prevent palette fuck up */
	}

	if (global[rope_is_hanging_in_607] && previous_room != 607) {
		ss[fx_rope] = kernel_load_series(kernel_name('x', 3), false);
		seq[fx_rope] = kernel_seq_stamp(ss[fx_rope], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_rope], 1);

	} else {
		kernel_flip_hotspot(words_rope, false);
	}


	ss[fx_stone_gone] = kernel_load_series(kernel_name('p', 0), false);
	ss[fx_torch] = kernel_load_series(kernel_name('y', 9), false);

	if (object_is_here(blue_powerstone)) {
		seq[fx_stone_gone] = kernel_seq_stamp(ss[fx_stone_gone], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_stone_gone], 1);
	} else {
		kernel_flip_hotspot(words_powerstone, false);
	}

	if (global[torch_is_in_609]) {
		seq[fx_torch] = kernel_seq_forward(ss[fx_torch], false, 6, 0, 0, 0);
		kernel_seq_depth(seq[fx_torch], 2);
		kernel_seq_range(seq[fx_torch], KERNEL_FIRST, KERNEL_LAST);
	}


	/* ========================= Previous Rooms ================== */

	if (previous_room == 607) {
		viewing_at_y = ((video_y - display_y) >> 1);
		kernel_init_dialog();  /* clear interface */
		kernel_set_interface_mode(INTER_LIMITED_SENTENCES);

		player.x = 280;  /* so death does not come */
		player.walker_visible = false;
		player.commands_allowed = false;
		aa[1] = kernel_run_animation(kernel_name('d', 1), 1);
		local->anim_1_running = true;
		kernel_flip_hotspot(words_torch, false);

	} else if (previous_room == 606 || previous_room != KERNEL_RESTORING_GAME) {
		if (global[torch_is_in_609]) {
			player.x = PLAYER_X_FROM_606;
			player.y = PLAYER_Y_FROM_606;
			player.facing = FACING_NORTH;

		} else {
			global[torch_is_in_609] = true;
			player.walker_visible = false;
			player.commands_allowed = false;
			aa[0] = kernel_run_animation(kernel_name('t', 1), ROOM_609_DONE_TORCH);
			local->anim_0_running = true;
			player.x = PLAYER_X_AFTER_TORCH;
			player.y = PLAYER_Y_AFTER_TORCH;
			player.facing = FACING_NORTHWEST;
		}
	}
}

static void handle_animation_king() {
	int king_reset_frame;

	if (kernel_anim[aa[0]].frame != local->king_frame) {
		local->king_frame = kernel_anim[aa[0]].frame;
		king_reset_frame = -1;

		switch (local->king_frame) {
		case 21:
			seq[fx_torch] = kernel_seq_forward(ss[fx_torch], false, 6, 0, 0, 0);
			kernel_seq_depth(seq[fx_torch], 2);
			kernel_seq_range(seq[fx_torch], KERNEL_FIRST, KERNEL_LAST);
			inter_move_object(torch, 509);
			break;
		}

		if (king_reset_frame >= 0) {
			kernel_reset_animation(aa[0], king_reset_frame);
			local->king_frame = king_reset_frame;
		}
	}
}

static void handle_animation_down() {
	int down_reset_frame;

	if (kernel_anim[aa[1]].frame != local->down_frame) {
		local->down_frame = kernel_anim[aa[1]].frame;
		down_reset_frame = -1;

		switch (local->down_frame) {
		case 45:
			kernel_seq_delete(seq[fx_stone_gone]);
			kernel_flip_hotspot(words_powerstone, false);
			global[player_score] += 5;
			sound_play(N_TakeObjectSnd);
			inter_give_to_player(blue_powerstone);
			object_examine(blue_powerstone, 60922, 0);
			break;
		}

		if (down_reset_frame >= 0) {
			kernel_reset_animation(aa[1], down_reset_frame);
			local->down_frame = down_reset_frame;
		}
	}
}

static void room_609_daemon() {
	if (local->anim_0_running) {
		handle_animation_king();
	}

	if (local->anim_1_running) {
		handle_animation_down();
	}

	if (kernel.trigger == ROOM_609_DONE_TORCH) {
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
		player.walker_visible = true;
		player.commands_allowed = true;
		local->anim_0_running = false;
	}

	if (kernel.trigger == 1) {
		kernel_abort_animation(aa[1]);
		local->anim_1_running = false;
		aa[2] = kernel_run_animation(kernel_name('u', 1), 2);
		kernel_synch(KERNEL_ANIM, aa[2], KERNEL_NOW, 0);
	}

	if (kernel.trigger == 2) {
		new_room = 607;
	}

	if (player.special_code > 0) {
		if (!local->prevent) {
			player.commands_allowed = false;
			local->prevent = true;
			player_walk(DEATH_X, DEATH_Y, FACING_WEST);
			player_walk_trigger(ROOM_609_DONE_WALK);
			/* error_watch_point ("you're dead", kernel.trigger, kernel.trigger); */
		}
	}

	if (kernel.trigger == ROOM_609_DONE_WALK) {
		player.walker_visible = false;
		player.commands_allowed = false;
		seq[fx_death] = kernel_seq_forward(ss[fx_death], false, 7, 0, 0, 1);
		kernel_seq_depth(seq[fx_death], 1);
		kernel_seq_range(seq[fx_death], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_death], KERNEL_TRIGGER_EXPIRE, 0, ROOM_609_DONE_WALK + 1);
		kernel_seq_trigger(seq[fx_death], KERNEL_TRIGGER_SPRITE, 25, ROOM_609_DONE_WALK + 2);
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_death]);
		sound_play(N_MeltInGlowingFloor);
	}

	if (kernel.trigger == ROOM_609_DONE_WALK + 1) {
		if (game.difficulty == EASY_MODE) {
			text_show(60931);
		} else {
			text_show(45);
		}
		kernel.force_restart = true;
	}

	if (kernel.trigger == ROOM_609_DONE_WALK + 2) {
		sound_play(N_PlayerDies);
	}
}

static void room_609_pre_parser() {
	if (player_said_1(pedestal) || (player_said_1(powerstone) && object_is_here(blue_powerstone))) {
		if (game.difficulty == EASY_MODE) {
			if (player_said_1(take) || player_said_1(push) ||
				player_said_1(pull) || player_said_1(put) ||
				player_said_1(open) || player_said_1(close)) {
				text_show(60911);
				player_cancel_command();
			}

		} else if (player_said_1(push) ||
			player_said_1(pull) || player_said_1(put) ||
			player_said_1(open) || player_said_1(close)) {
			text_show(60911);
			player_cancel_command();
		}
	}

	if (player_said_1(take_magic_from)) {
		player.need_to_walk = false;
	}
}

static void room_609_parser() {
	if (local->prevent) {
		goto handled;
	}

	if (player_said_2(walk_through, doorway_to_south)) {
		new_room = 606;
		goto handled;
	}

	if (player.look_around) {
		if (global[rope_is_hanging_in_607]) {
			text_show(60903);
		} else if (global[hole_is_in_607]) {
			text_show(60902);
		} else {
			text_show(60901);
		}
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(floor)) {
			text_show(60905);
			goto handled;
		}

		if (player_said_1(glowing_floor)) {
			text_show(60906);
			goto handled;
		}

		if (player_said_1(pedestal)) {
			if (object_is_here(blue_powerstone)) {
				text_show(60909);
			} else {
				text_show(60910);
			}
			goto handled;
		}

		if (player_said_1(large_spider_web)) {
			text_show(60915);
			goto handled;
		}

		if (player_said_1(trail_of_green_slime)) {
			if (game.difficulty == HARD_MODE) {
				text_show(60918);
			} else {
				text_show(60917);
			}
			goto handled;
		}

		if (player_said_1(doorway_to_south)) {
			text_show(60919);
			goto handled;
		}

		if (player_said_1(wall)) {
			text_show(60920);
			goto handled;
		}

		if (player_said_1(powerstone)) {
			if (object_is_here(blue_powerstone)) {
				text_show(60921);
				goto handled;
			}
		}

		if (player_said_1(torch)) {
			text_show(60929);
			goto handled;
		}

		if (player_said_1(rope)) {
			text_show(60903);
			goto handled;
		}
	}

	if (player_said_1(glowing_floor)) {
		if (player_said_1(put) || player_said_1(throw)) {
			text_show(60907);
			goto handled;
		}
	}

	if (player_said_2(pour_contents_of, glowing_floor)) {
		text_show(60908);
		goto handled;
	}

	if (player_said_1(pedestal) || (player_said_1(blue_powerstone) && object_is_here(blue_powerstone))) {
		if (player_said_1(throw)) {
			if (player_said_1(shieldstone)) {
				text_show(60913);
			} else if (player_said_1(teleport_door)) {
				text_show(60914);
			} else {
				text_show(60912);
			}
			goto handled;
		}
	}

	if (player_said_1(large_spider_web)) {
		if (!player_said_1(walk_to)) {
			text_show(60916);
			goto handled;
		}
	}

	/*  if (player_said_1 (throw)) {
		if (player_said_1 (hole_in_ceiling) || player_said_1 (hole_in_trapdoor)) {
		  text_show (60926);
		  goto handled;
		}
	  }
	*/

	if (player_said_2(take, rope)) {
		if (global[rope_is_hanging_in_607]) {
			text_show(60928);
			goto handled;
		}
	}

	if (player_said_2(throw, rope)) {
		if (player_said_1(pedestal) || (player_said_1(blue_powerstone) && object_is_here(blue_powerstone))) {
			text_show(60927);
			goto handled;
		}
	}

	if (player_said_2(take, torch)) {
		text_show(60930);
		goto handled;
	}

	if (player_said_2(take_magic_from, glowing_floor)) {
		text_show(963);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_609_synchronize(Common::Serializer &s) {
	for (int i = 0; i < 15; i++) s.syncAsSint16LE(local->sprite[i]);
	for (int i = 0; i < 15; i++) s.syncAsSint16LE(local->sequence[i]);
	for (int i = 0; i < 4; i++)  s.syncAsSint16LE(local->animation[i]);
	s.syncAsSint16LE(local->king_frame);
	s.syncAsSint16LE(local->anim_0_running);
	s.syncAsSint16LE(local->down_frame);
	s.syncAsSint16LE(local->anim_1_running);
	s.syncAsSint16LE(local->prevent);
}

void room_609_preload() {
	room_init_code_pointer = room_609_init;
	room_pre_parser_code_pointer = room_609_pre_parser;
	room_parser_code_pointer = room_609_parser;
	room_daemon_code_pointer = room_609_daemon;

	if (room_id == 607) {
		global[no_load_walker] = true;
	}

	section_6_walker();
	section_6_interface();

	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
