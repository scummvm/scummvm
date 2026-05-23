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
#include "mads/madsv2/dragonsphere/rooms/room607.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];       /* Sprite series handles */
	int16 sequence[15];     /* Sequence handles      */
	int16 animation[4];     /* Animation handles     */

	int16 pour_frame;       /* animation frame being held for pour stuff */
	int16 anim_0_running;
	int16 prevent;
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation


/* ========================= Sprite Series =================== */

#define fx_hole                 0       /* rm607h0  */
#define fx_rope                 1       /* rm607x0  */
#define fx_look                 2       /* rm607a1  */
#define fx_tie                  3       /* rm607a2  */
#define fx_down                 4       /* rm607a3  */
#define fx_fire_1               5       /* rm607x1  */
#define fx_take                 6       /* trchgt   */
#define fx_trap                 7       /* rm607t0  */


/* ========================= Triggers ======================== */

#define ROOM_607_DONE_UP        60 


/* ========================= Other Macros ==================== */

#define PLAYER_X_FROM_606       171
#define PLAYER_Y_FROM_606       149

#define POUR_X                  92
#define POUR_Y                  139

#define LOOK_X                  70
#define LOOK_Y                  130

#define TIE_X                   44
#define TIE_Y                   144

#define DOWN_X                  46
#define DOWN_Y                  141

#define TORCH_HS_X              60
#define TORCH_HS_Y              80

#define TORCH_X                 70
#define TORCH_Y                 128

static Scratch scratch;


static void room_607_init() {
	int count;

	local->anim_0_running = false;
	local->prevent = false;

	ss[fx_look] = kernel_load_series(kernel_name('a', 1), false);
	ss[fx_rope] = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_hole] = kernel_load_series(kernel_name('h', 0), false);
	ss[fx_tie] = kernel_load_series(kernel_name('a', 2), false);
	ss[fx_down] = kernel_load_series(kernel_name('a', 3), false);
	ss[fx_trap] = kernel_load_series(kernel_name('t', 0), false);

	if (object_is_here(torch)) {
		ss[fx_take] = kernel_load_series("*TRCHGT", false);
		ss[fx_fire_1] = kernel_load_series(kernel_name('x', 1), false);
		seq[fx_fire_1] = kernel_seq_forward(ss[fx_fire_1], false, 6, 0, 0, 0);
		kernel_seq_depth(seq[fx_fire_1], 10);
		kernel_seq_range(seq[fx_fire_1], KERNEL_FIRST, KERNEL_LAST);
	} else {
		kernel_flip_hotspot_loc(words_torch, false, TORCH_HS_X, TORCH_HS_Y);
	}

	if (!global[hole_is_in_607]) {
		if (game.difficulty == HARD_MODE) {
			seq[fx_hole] = kernel_seq_stamp(ss[fx_hole], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_hole], 14);
			kernel_flip_hotspot(words_hole, false);
			kernel_flip_hotspot(words_trap_door, false);
		} else {
			seq[fx_trap] = kernel_seq_stamp(ss[fx_trap], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_trap], 8);
			kernel_flip_hotspot(words_hole, false);
		}

	} else {
		kernel_flip_hotspot(words_trap_door, false);
	}

	if (global[rope_is_hanging_in_607]) {

		if (previous_room != 609) {
			seq[fx_rope] = kernel_seq_stamp(ss[fx_rope], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_rope], 5);
		}

		for (count = 0; count < room_num_spots; count++) {
			if (room_spots[count].vocab == words_hole) {
				room_spots[count].cursor_number = 3;
				room_spots[count].verb = words_climb_through;
			}
		}

	} else {
		kernel_flip_hotspot(words_rope, false);
	}

	/* ========================= Previous Rooms ================== */

	if (previous_room == 609) {
		player.x = DOWN_X;
		player.y = DOWN_Y;
		player.facing = FACING_NORTH;
		player.walker_visible = false;
		player.commands_allowed = false;
		seq[fx_down] = kernel_seq_backward(ss[fx_down], false, 7, 0, 0, 1);
		kernel_seq_depth(seq[fx_down], 2);
		kernel_seq_range(seq[fx_down], KERNEL_FIRST, KERNEL_LAST);
		kernel_seq_trigger(seq[fx_down], KERNEL_TRIGGER_EXPIRE, 0, ROOM_607_DONE_UP);

	} else if (previous_room == 604 || previous_room != KERNEL_RESTORING_GAME) {
		player.x = PLAYER_X_FROM_606;
		player.y = PLAYER_Y_FROM_606;
		player.facing = FACING_NORTH;
	}

	section_6_music();
}

static void handle_animation_pour() {
	int pour_reset_frame;

	if (kernel_anim[aa[0]].frame != local->pour_frame) {
		local->pour_frame = kernel_anim[aa[0]].frame;
		pour_reset_frame = -1;

		switch (local->pour_frame) {
		case 10:
			sound_play(N_AcidBurnsFloor);
			break;

		case 16:  /* well into acid pouring onto floor/trapdoor */
			if (game.difficulty == HARD_MODE) {
				kernel_seq_delete(seq[fx_hole]);

			} else if (game.difficulty != HARD_MODE) {
				kernel_seq_delete(seq[fx_trap]);
			}
			break;

		case 20:
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
			break;
		}

		if (pour_reset_frame >= 0) {
			kernel_reset_animation(aa[0], pour_reset_frame);
			local->pour_frame = pour_reset_frame;
		}
	}
}

static void room_607_daemon() {
	if (local->anim_0_running) {
		handle_animation_pour();
	}

	if (kernel.trigger == ROOM_607_DONE_UP) {
		seq[fx_rope] = kernel_seq_stamp(ss[fx_rope], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_rope], 6);

		kernel_synch(KERNEL_SERIES, seq[fx_rope], KERNEL_SERIES, seq[fx_down]);
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_down]);

		player.walker_visible = true;
		player.commands_allowed = true;
	}
}

static void room_607_pre_parser() {
	if (player_said_2(pour_contents_of, floor) ||
		player_said_2(pour_contents_of, trap_door)) {
		player_walk(POUR_X, POUR_Y, FACING_SOUTHWEST);
	}

	if ((player_said_3(tie, rope, manacles) ||
		player_said_3(put, rope, manacles)) &&
		player_has(rope) && global[hole_is_in_607] &&
		inter_point_x < 50) {
		player_walk(TIE_X, TIE_Y, FACING_NORTHWEST);
	}

	if (player_said_2(look, hole)) {
		player_walk(LOOK_X, LOOK_Y, FACING_SOUTH);
	}

	if (player_said_2(take, torch) && object_is_here(torch)) {
		player_walk(TORCH_X, TORCH_Y, FACING_NORTHWEST);
	}

	if (player_said_2(climb_through, hole) && global[rope_is_hanging_in_607]) {
		player_walk(DOWN_X, DOWN_Y, FACING_NORTH);
	}
}

static void room_607_parser() {
	int count;

	if (player_said_2(take, torch)) {
		switch (kernel.trigger) {
		case 0:
			if (!player_has(torch)) {
				player.commands_allowed = false;
				player.walker_visible = false;
				seq[fx_take] = kernel_seq_pingpong(ss[fx_take], false, 7, 0, 0, 2);
				kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_SPRITE, 4, 1);
				kernel_seq_trigger(seq[fx_take], KERNEL_TRIGGER_EXPIRE, 0, 2);
				kernel_seq_depth(seq[fx_take], 2);
				kernel_seq_range(seq[fx_take], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_player(seq[fx_take], true);
				goto handled;
			}
			break;

		case 1:
			if (local->prevent) {
				kernel_seq_delete(seq[fx_fire_1]);
				kernel_flip_hotspot_loc(words_torch, false, TORCH_HS_X, TORCH_HS_Y);
				++global[player_score];
				sound_play(N_TakeObjectSnd);
				inter_give_to_player(torch);
				object_examine(torch, 60728, 0);
			}
			local->prevent = true;
			goto handled;
			break;

		case 2:
			player.walker_visible = true;
			player.commands_allowed = true;
			local->prevent = false;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take]);
			goto handled;
			break;
		}
	}

	if (player_said_2(pour_contents_of, floor) && global[hole_is_in_607]) {
		text_show(60743);
		goto handled;

	} else if ((player_said_2(pour_contents_of, floor) ||
		player_said_2(pour_contents_of, trap_door)) && !global[hole_is_in_607]) {
		switch (kernel.trigger) {
		case 0:
			player.walker_visible = false;
			player.commands_allowed = false;
			local->anim_0_running = true;
			aa[0] = kernel_run_animation(kernel_name('p', 1), 1);
			goto handled;
			break;

		case 1:
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
			kernel_flip_hotspot(words_hole, true);
			kernel_flip_hotspot(words_trap_door, false);
			kernel_load_variant(1);
			inter_move_object(flask_full_of_acid, NOWHERE);
			inter_give_to_player(flask);
			local->anim_0_running = false;
			player.commands_allowed = true;
			global[hole_is_in_607] = true;
			global[player_score] += 5;

			if (game.difficulty == HARD_MODE) {
				text_show(60722);
			} else {
				if (player_has_been_in_room(609)) {
					text_show(60724);
				} else {
					text_show(60723);
				}
			}
			goto handled;
			break;
		}
	}

	if (player_said_2(look, hole)) {
		switch (kernel.trigger) {
		case 0:
			player.walker_visible = false;
			player.commands_allowed = false;
			seq[fx_look] = kernel_seq_pingpong(ss[fx_look], false, 7, 0, 0, 2);
			kernel_seq_depth(seq[fx_look], 2);
			kernel_seq_range(seq[fx_look], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_look], KERNEL_TRIGGER_EXPIRE, 0, 1);
			kernel_seq_player(seq[fx_look], true);
			kernel_seq_loc(seq[fx_look], LOOK_X, LOOK_Y + 3);
			goto handled;
			break;

		case 1:
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_look]);
			player.walker_visible = true;
			player.commands_allowed = true;

			if (game.difficulty == HARD_MODE) {
				if (player_has(blue_powerstone)) {
					text_show(60748);

				} else {
					text_show(60702);
				}

			} else {
				if (player_has(blue_powerstone)) {
					text_show(60748);

				} else if (player_has_been_in_room(609)) {
					text_show(60725);

				} else {
					text_show(60703);
				}
			}
			goto handled;
			break;
		}
	}

	if (player_said_2(climb_through, hole) && global[rope_is_hanging_in_607]) {
		if (global[torch_is_in_609]) {
			if (!player_has(blue_powerstone)) {
				switch (kernel.trigger) {
				case 0:
					player.walker_visible = false;
					player.commands_allowed = false;
					kernel_seq_delete(seq[fx_rope]);
					seq[fx_down] = kernel_seq_forward(ss[fx_down], false, 7, 0, 0, 1);
					kernel_seq_depth(seq[fx_down], 2);
					kernel_seq_range(seq[fx_down], KERNEL_FIRST, KERNEL_LAST);
					kernel_seq_trigger(seq[fx_down], KERNEL_TRIGGER_EXPIRE, 0, 1);
					goto handled;
					break;

				case 1:
					new_room = 609;
					goto handled;
					break;
				}

			} else {
				text_show(60745);
				goto handled;
			}

		} else {
			text_show(60747);
			goto handled;
		}
	}

	if ((player_said_3(tie, rope, manacles) ||
		player_said_3(put, rope, manacles)) && player_has(rope) && !global[hole_is_in_607]) {
		text_show(60752);
		goto handled;
	}

	if ((player_said_3(tie, rope, manacles) ||
		player_said_3(put, rope, manacles)) && player_has(rope) && global[hole_is_in_607]) {
		if (inter_point_x < 50) {
			switch (kernel.trigger) {
			case 0:
				player.walker_visible = false;
				player.commands_allowed = false;
				seq[fx_tie] = kernel_seq_forward(ss[fx_tie], false, 8, 0, 0, 1);
				kernel_seq_depth(seq[fx_tie], 2);
				kernel_seq_range(seq[fx_tie], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_trigger(seq[fx_tie], KERNEL_TRIGGER_SPRITE, 21, 1);
				kernel_seq_trigger(seq[fx_tie], KERNEL_TRIGGER_EXPIRE, 0, 2);
				goto handled;
				break;

			case 1:
				seq[fx_rope] = kernel_seq_stamp(ss[fx_rope], false, KERNEL_FIRST);
				kernel_seq_depth(seq[fx_rope], 5);
				kernel_flip_hotspot(words_rope, true);
				goto handled;
				break;

			case 2:
				inter_move_object(rope, NOWHERE);
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_tie]);
				kernel_load_variant(2);
				player.walker_visible = true;
				player.commands_allowed = true;
				global[rope_is_hanging_in_607] = true;
				global[player_score] += 3;
				text_show(60726);

				for (count = 0; count < room_num_spots; count++) {
					if (room_spots[count].vocab == words_hole) {
						room_spots[count].cursor_number = 3;
						room_spots[count].verb = words_climb_through;
					}
				}
				goto handled;
				break;
			}

		} else {
			text_show(60737);
			goto handled;
			/* you dont have enough rope to tie it there */
		}
	}

	if (player_said_2(take, rope) && !player_has(rope)) {
		text_show(60739);
		goto handled;
		/* there is no need to take the rope */
	}

	if (player_said_2(walk_through, door_to_south)) {
		new_room = 606;
		goto handled;
	}

	if (player.look_around) {
		text_show(60701);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(torch) && object_is_here(torch)) {
			text_show(60704);
			goto handled;
		}

		if (player_said_1(wall)) {
			text_show(60705);
			goto handled;
		}

		if (player_said_1(manacles)) {
			text_show(60706);
			goto handled;
		}

		if (player_said_1(leg_lock)) {
			text_show(60707);
			goto handled;
		}

		if (player_said_1(neck_lock)) {
			text_show(60708);
			goto handled;
		}

		if (player_said_1(bench)) {
			text_show(60709);
			goto handled;
		}

		if (player_said_1(skeleton)) {
			text_show(60710);
			goto handled;
		}

		if (player_said_1(shaft_of_light)) {
			text_show(60711);
			goto handled;
		}

		if (player_said_1(waist_lock)) {
			text_show(60712);
			goto handled;
		}

		if (player_said_1(window)) {
			text_show(60713);
			goto handled;
		}

		if (player_said_1(door_to_south)) {
			text_show(60714);
			goto handled;
		}

		if (player_said_1(trail_of_green_slime)) {
			if (game.difficulty == HARD_MODE) {
				text_show(60715);
			} else {
				text_show(60716);
			}
			goto handled;
		}

		if (player_said_1(ceiling)) {
			text_show(60717);
			goto handled;
		}

		if (player_said_1(floor)) {
			if (global[hole_is_in_607]) {
				text_show(60751);

			} else if (game.difficulty == HARD_MODE) {
				text_show(60718);

			} else {
				text_show(60719);
			}
			goto handled;
		}

		if (player_said_1(trap_door)) {
			text_show(60720);
			goto handled;
		}

		if (player_said_1(rope)) {
			if (global[rope_is_hanging_in_607]) {
				text_show(60744);
				goto handled;
			}
		}
	}

	if (player_said_2(pull, manacles)) {
		text_show(60721);
		goto handled;
	}

	if (player_said_1(open) || player_said_1(close)) {
		if (player_said_1(leg_lock) || player_said_1(neck_lock) || player_said_1(waist_lock) ||
			player_said_1(manacles)) {
			text_show(60729);
			goto handled;
		}
	}

	if (player_said_2(push, bench)) {
		text_show(60730);
		goto handled;
	}

	if (player_said_2(talk_to, skeleton)) {
		text_show(60731);
		goto handled;
	}

	if (player_said_2(give, skeleton)) {
		if (!player_said_1(bone)) {
			text_show(60732);
			goto handled;
		}
	}

	if (player_said_3(sword, attack, floor) ||
		player_said_3(sword, carve_up, floor) ||
		player_said_3(sword, thrust, floor)) {
		text_show(60733);
		goto handled;
	}

	if (player_said_3(give, bone, skeleton)) {
		text_show(60734);
		goto handled;
	}

	if (player_said_1(hole)) {
		if (player_said_1(throw) || player_said_1(put)) {
			text_show(60735);
			goto handled;
		}
	}

	if (player_said_3(put, torch, wall)) {
		if (player_has(torch)) {
			text_show(60736);
			goto handled;
		}
	}

	if (player_said_3(sword, attack, rope) ||
		player_said_3(sword, carve_up, rope) ||
		player_said_3(sword, thrust, rope)) {
		if (!player_has(rope)) {
			text_show(60741);
			goto handled;
		}
	}

	if (player_said_2(open, trap_door)) {
		text_show(60749);
		goto handled;
	}

	if (player_said_2(take, trail_of_green_slime) ||
		player_said_3(put, trail_of_green_slime, goblet) ||
		player_said_3(fill, trail_of_green_slime, goblet)) {
		if (player_has(goblet)) {
			text_show(60750);
			goto handled;
		}
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_607_synchronize(Common::Serializer &s) {
	for (int i = 0; i < 15; i++) s.syncAsSint16LE(local->sprite[i]);
	for (int i = 0; i < 15; i++) s.syncAsSint16LE(local->sequence[i]);
	for (int i = 0; i < 4; i++)  s.syncAsSint16LE(local->animation[i]);
	s.syncAsSint16LE(local->pour_frame);
	s.syncAsSint16LE(local->anim_0_running);
	s.syncAsSint16LE(local->prevent);
}

void room_607_preload(void) {
	room_init_code_pointer = room_607_init;
	room_pre_parser_code_pointer = room_607_pre_parser;
	room_parser_code_pointer = room_607_parser;
	room_daemon_code_pointer = room_607_daemon;

	if (room_id == 609) {
		global[no_load_walker] = false;
	}

	if (global[rope_is_hanging_in_607]) {
		kernel_initial_variant = 2;
	} else if (global[hole_is_in_607]) {
		kernel_initial_variant = 1;
	}

	section_6_walker();
	section_6_interface();

	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
