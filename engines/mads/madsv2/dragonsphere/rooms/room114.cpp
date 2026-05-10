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
#include "mads/madsv2/dragonsphere/rooms/section1.h"
#include "mads/madsv2/dragonsphere/rooms/room114.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];       /* Sprite series handles */
	int16 sequence[15];     /* Sequence handles      */
	int16 animation[4];     /* Animation handles     */

	int16 stairway_frame;
	int16 stairway_action;
	int16 on_stairway;        /* T if came from under well and have choices on stairway */

	int16 anim_0_running;
	int16 anim_1_running;
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

static Scratch scratch;


/* ========================= Sprites ========================= */

#define fx_sconce_fire       1 /* rm114y */

#define ROOM_114_RUN_AA_SUB_0 60

#define START_X_ROOM_113     109
#define START_Y_ROOM_113     118
#define START_X_ROOM_115     16
#define START_Y_ROOM_115     105

#define LADDER_FREEZE        0
#define LADDER_DIE           1
#define LADDER_GO_UP         2
#define LADDER_GO_DOWN       3 
#define LADDER_PLACE_OBJECT  4
#define LADDER_OTHER         5

#define UP_LADDER_X          230
#define UP_LADDER_Y          30


static void room_114_init(void) {
	/* Load sprite series */

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->on_stairway = false;
		local->anim_0_running = false;
		local->anim_1_running = false;
	}

	kernel_flip_hotspot(words_statue, false);

	ss[fx_sconce_fire] = kernel_load_series(kernel_name('y', -1), false);

	/* Start continuous sequences */
	seq[fx_sconce_fire] = kernel_seq_forward(ss[fx_sconce_fire], false, 7, 0, 0, 0);
	kernel_seq_depth(seq[fx_sconce_fire], 15);


	if (previous_room == KERNEL_RESTORING_GAME) {
		aa[0] = kernel_run_animation(kernel_name('d', 1), 0);
		local->anim_0_running = true;
		if (global[statue_is_on_stairway_114] == STAIRWAY_STATUE) {
			kernel_flip_hotspot(words_statue, true);
		}
		if (local->on_stairway) {
			player.walker_visible = false;
			local->stairway_action = LADDER_FREEZE;
			kernel_reset_animation(aa[0], 49);

		} else {
			kernel_reset_animation(aa[0], 104);
		}

	} else if (previous_room == 113) { /* Player comes from under well */
		player.x = START_X_ROOM_113;
		player.y = START_Y_ROOM_113;
		player.facing = FACING_WEST;
		player.walker_visible = false;
		player.commands_allowed = false;

		if (!global[statue_is_on_stairway_114]) {
			local->stairway_action = LADDER_FREEZE;
			local->on_stairway = true;
			local->anim_0_running = false;
			aa[2] = kernel_run_animation(kernel_name('d', 3), ROOM_114_RUN_AA_SUB_0);

		} else {
			aa[0] = kernel_run_animation(kernel_name('d', 1), 0);
			local->anim_0_running = true;
			kernel_reset_animation(aa[0], 49);
			kernel_reset_animation(aa[0], 210);
			local->stairway_action = LADDER_OTHER;
			if (global[statue_is_on_stairway_114] == STAIRWAY_STATUE) {
				kernel_flip_hotspot(words_statue, true);
			}
			/* Have Pid come down into room - statue is already on stairway - or crystal ball - he is safe */
		}

	} else if (previous_room != KERNEL_RESTORING_GAME) { /* Player comes from Darkened Beast rm115 */
		aa[0] = kernel_run_animation(kernel_name('d', 1), 0);
		local->anim_0_running = true;
		player.x = START_X_ROOM_115;
		player.y = START_Y_ROOM_115;
		player.facing = FACING_EAST;
		local->stairway_action = LADDER_OTHER;
		if (global[statue_is_on_stairway_114] == STAIRWAY_STATUE) {
			kernel_flip_hotspot(words_statue, true);
		}
		kernel_reset_animation(aa[0], 104);
	}

	section_1_music();
}

static void handle_animation_stairway_1() {
	int stairway_reset_frame;

	if (kernel_anim[aa[1]].frame != local->stairway_frame) {
		local->stairway_frame = kernel_anim[aa[1]].frame;
		stairway_reset_frame = -1;

		switch (local->stairway_frame) {
		case 103:
			if (game.difficulty == EASY_MODE) {
				text_show(11429);
			} else {
				text_show(45);
			}
			new_room = 113;
			break;
		}

		if (stairway_reset_frame >= 0) {
			kernel_reset_animation(aa[1], stairway_reset_frame);
			local->stairway_frame = stairway_reset_frame;
		}
	}
}

static void handle_animation_stairway() {
	int stairway_reset_frame;

	if (kernel_anim[aa[0]].frame != local->stairway_frame) {
		local->stairway_frame = kernel_anim[aa[0]].frame;
		stairway_reset_frame = -1;

		switch (local->stairway_frame) {
		case 144:
		case 103:
		case 209:
		case 327:
			new_room = 113;
			break;

		case 272:
			if (global[statue_is_on_stairway_114]) {
				stairway_reset_frame = 157;
			}
			break;

		case 54:
			inter_move_object(statue, NOWHERE);
			break;

		case 49: /* end of frozen on bottom of stairway - waiting for input */
		case 59: /* end of place object */

			if (local->stairway_frame == 59) {
				text_show(11428);
			}

			if (local->stairway_action == LADDER_FREEZE) {
				stairway_reset_frame = 48;
				player.commands_allowed = true;
				/* keep him on bottom of stairway waiting for user input */

			} else if (local->stairway_action == LADDER_GO_UP) {
				stairway_reset_frame = 265;
				/* make him go up and leave room */

			} else if (local->stairway_action == LADDER_PLACE_OBJECT) {
				stairway_reset_frame = 49;
				local->stairway_action = LADDER_FREEZE;
				/* put object on end of stairway */

			} else if (local->stairway_action == LADDER_GO_DOWN) {
				if (!global[statue_is_on_stairway_114]) {
					kernel_abort_animation(aa[0]);
					aa[1] = kernel_run_animation(kernel_name('d', 2), 0);
					stairway_reset_frame = -1;
					local->anim_0_running = false;
					local->anim_1_running = true;
					kernel_reset_animation(aa[1], 60);
					kernel_synch(KERNEL_ANIM, aa[1], KERNEL_NOW, 0);

				} else {
					stairway_reset_frame = 59;
					/* make player walk off stairway */
				}
			}
			break;

		case 66:
			if (!global[statue_is_on_stairway_114]) {
				stairway_reset_frame = 66;
				/* make player die */

			} else {
				player.walker_visible = true;
				player.commands_allowed = true;
				stairway_reset_frame = 103;
				local->on_stairway = false;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
				player.ready_to_walk = true;
			}
			break;

		case 104: /* end of stairway stamped down in room */
			if (local->stairway_action == LADDER_GO_UP) {
				stairway_reset_frame = 150;
				player.commands_allowed = false;
				player.walker_visible = false;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
				/* make Pid climb stairway */
			} else {
				stairway_reset_frame = 103;
				/* keep stairway stamped down */
			}
			break;

		case 265: /* end of coming into room - stairs already in place */
			player.x = START_X_ROOM_113;
			player.y = START_Y_ROOM_113;
			player.walker_visible = true;
			player.commands_allowed = true;
			stairway_reset_frame = 103;
			local->on_stairway = false;
			player_demand_facing(FACING_WEST);
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
			break;
		}

		if (stairway_reset_frame >= 0) {
			kernel_reset_animation(aa[0], stairway_reset_frame);
			local->stairway_frame = stairway_reset_frame;
		}
	}
}

static void room_114_daemon() {
	if (kernel.trigger == ROOM_114_RUN_AA_SUB_0) {
		kernel_abort_animation(aa[2]);
		aa[0] = kernel_run_animation(kernel_name('d', 1), 0);
		local->anim_0_running = true;
		kernel_reset_animation(aa[0], 49);
		kernel_synch(KERNEL_ANIM, aa[0], KERNEL_NOW, 0);
	}

	if (local->anim_1_running) {
		handle_animation_stairway_1();
	}

	if (local->anim_0_running) {
		handle_animation_stairway();
	}
}

static void room_114_pre_parser(void) {
	if (player_said_2(walk_down, passageway_to_west)) {
		player.walk_off_edge_to_room = 115;
	}

	if (local->on_stairway) {
		if (player_said_2(climb_up, stairway) || player_said_2(climb_through, trap_door)) {
			player.need_to_walk = false;
		}

		if (player_said_1(walk_across) || player_said_1(walk_to) || player_said_1(walk_down)) {
			local->stairway_action = LADDER_GO_DOWN;
			player.commands_allowed = false;
			player.ready_to_walk = false;
		}

		if (player_said_1(push) || player_said_1(pull) || player_said_1(open) ||
			player_said_1(close) || player_said_1(give) || player_said_1(take) ||
			player_said_1(throw) || player_said_1(put)) {
			player.need_to_walk = false;
		}
	}
}


static void room_114_parser() {
	if (player_said_3(put, statue, stairway) && global[statue_is_on_stairway_114] == STAIRWAY_STATUE) {
		text_show(11414);
		goto handled;
	}

	if (player_said_2(climb_up, stairway) || player_said_2(climb_through, trap_door)) {
		if (!local->on_stairway) {
			local->stairway_action = LADDER_GO_UP;
			player.commands_allowed = false;
			goto handled;
		}
	}

	if (local->on_stairway) {
		if (player_said_2(climb_up, stairway) || player_said_2(climb_through, trap_door)) {
			local->stairway_action = LADDER_GO_UP;
			player.commands_allowed = false;
			goto handled;

		} else if (player_said_3(put, statue, stairway) ||
			player_said_2(invoke_power_of, crystal_ball)) {

			if (!global[statue_is_on_stairway_114]) {
				if (player_said_1(statue)) {
					global[player_score] += 3;
					kernel_flip_hotspot(words_statue, true);
					local->stairway_action = LADDER_PLACE_OBJECT;
					player.commands_allowed = false;
					global[statue_is_on_stairway_114] = STAIRWAY_STATUE;

				} else {
					global[statue_is_on_stairway_114] = STAIRWAY_CRYSTAL_BALL;
					sound_play(N_InvokeCrystalBall);
					text_show(11411);
					inter_move_object(crystal_ball, NOWHERE);
					text_show(970);
				}
				goto handled;
			}

		} else if (player_said_2(put, stairway)) {
			if (!global[statue_is_on_stairway_114]) {
				if (game.difficulty == EASY_MODE) {
					text_show(11412);
				} else {
					text_show(11413);
				}
				goto handled;
			}

		} else if (player_said_1(walk_across) || player_said_1(walk_to) || player_said_1(walk_down)) {
			goto handled;
		}
	}

	if (player.look_around) {
		if (global[statue_is_on_stairway_114]) {
			if (local->on_stairway) {
				text_show(11401);
			} else {
				text_show(11402);
			}
		} else {
			text_show(11401);
		}
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(statue)) {
			if (local->on_stairway && !player_has(statue)) {
				text_show(11409);
				goto handled;
			} else if (!player_has(statue)) {
				text_show(11408);
				goto handled;
			}
		}

		if (player_said_1(stairway)) {

			if (global[statue_is_on_stairway_114] == STAIRWAY_CRYSTAL_BALL) {

				if (!local->on_stairway) {
					text_show(11422);
					/* crystal ball did the trick */
				} else if (global[statue_is_on_stairway_114]) {
					text_show(11421);
					/* I hope the crystal ball did the trick */
				}

			} else if (global[statue_is_on_stairway_114] == STAIRWAY_STATUE) {

				if (!local->on_stairway) {
					text_show(11408);
					/* The statue did the trick! */
				} else if (global[statue_is_on_stairway_114]) {
					text_show(11407);
					/* I hope the statue will do the trick */
				}

			} else {
				text_show(11405);
				/* plain old look at stairway */
			}
			goto handled;
		}

		if (player_said_1(passageway_to_west)) {
			text_show(11403);
			goto handled;
		}

		if (player_said_1(mechanism)) {
			text_show(11404);
			goto handled;
		}

		if (player_said_1(cave_wall)) {
			text_show(11423);
			goto handled;
		}

		if (player_said_1(trap_door)) {
			text_show(11427);
			goto handled;
		}

		if (player_said_1(cave_ceiling)) {
			text_show(11424);
			goto handled;
		}

		if (player_said_1(floor)) {
			text_show(11425);
			goto handled;
		}

		if (player_said_1(sconce)) {
			text_show(11426);
			goto handled;
		}

		if (player_said_1(spearheads)) {
			text_show(11406);
			goto handled;
		}
	}

	if (player_said_2(take, statue)) {
		if (!player_has(statue)) {
			text_show(11416);
			goto handled;
		}
	}

	if (player_said_2(open, stairway)) {
		text_show(11418);
		goto handled;
	}

	if (player_said_2(gaze_into, crystal_ball)) {
		text_show(11410);
		goto handled;
	}

	if (player_said_2(close, stairway)) {
		text_show(11417);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_114_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.stairway_frame);
	s.syncAsSint16LE(scratch.stairway_action);
	s.syncAsSint16LE(scratch.on_stairway);
	s.syncAsSint16LE(scratch.anim_0_running);
	s.syncAsSint16LE(scratch.anim_1_running);
}

void room_114_preload() {
	room_init_code_pointer = room_114_init;
	room_pre_parser_code_pointer = room_114_pre_parser;
	room_parser_code_pointer = room_114_parser;
	room_daemon_code_pointer = room_114_daemon;

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
