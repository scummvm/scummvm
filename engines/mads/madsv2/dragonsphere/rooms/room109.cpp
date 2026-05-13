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
#include "mads/madsv2/dragonsphere/rooms/room109.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];       /* Sprite series handles */
	int16 sequence[15];     /* Sequence handles      */
	int16 animation[4];     /* Animation handles     */

	int16 seal_frame;       /* animation frame being held for seal stuff */
	int16 seal_action;      /* Type of action to run for seal animation */
	int16 seal_talk_count;  /* counter for seal talking */
	int16 anim_0_running;

	int16 queen_frame;      /* animation frame being held for queen stuff */
	int16 queen_action;     /* Type of action to run for queen animation */
	int16 queen_talk_count; /* counter for queen talking */
	int16 anim_1_running;
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

static Scratch scratch;

/* ========================= Sprites ========================= */

#define fx_door             0   /* rm109x0 */


/* ======================== Triggers ========================= */

#define ROOM_109_YOU_TALK   60
#define ROOM_109_ME_TALK    65

/* ====================== Other macros ======================= */

#define START_X_ROOM_108    79
#define START_Y_ROOM_108    108

#define SEAL_SHUT_UP        0
#define SEAL_TALK           1
#define SEAL_LEAVE          2


#define QUEEN_INIT          0
#define QUEEN_SHUT_UP       1
#define QUEEN_TALK          2


#define CONV_QUEEN          5


static void room_109_init() {
	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
		local->anim_1_running = false;
	}

	local->seal_talk_count = 0;
	local->queen_talk_count = 0;

	conv_get(CONV_QUEEN);

	if (previous_room == 117 || local->anim_0_running) {
		ss[fx_door] = kernel_load_series(kernel_name('x', 0), false);
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 14);
		kernel_init_dialog();  /* clear interface */
		kernel_set_interface_mode(INTER_CONVERSATION);
		player.commands_allowed = false;
		player.walker_visible = false;
		aa[1] = kernel_run_animation(kernel_name('q', 1), 0);
		local->anim_1_running = true;
		local->queen_action = QUEEN_INIT;
		if (conv_restore_running == CONV_QUEEN) {
			aa[0] = kernel_run_animation(kernel_name('s', 1), 0);
			kernel_reset_animation(aa[0], 13);
			kernel_reset_animation(aa[1], 17);
			local->anim_0_running = true;
			local->seal_action = SEAL_SHUT_UP;
			local->queen_action = QUEEN_SHUT_UP;
			conv_run(CONV_QUEEN);
		}

	} else if (previous_room != KERNEL_RESTORING_GAME) { /* Player comes from Guardroom rm108 */
		player.x = START_X_ROOM_108;
		player.y = START_Y_ROOM_108;
		player.facing = FACING_SOUTHEAST;
	}

	section_1_music();
}

static void handle_animation_seal() {
	int seal_reset_frame;

	if (kernel_anim[aa[0]].frame != local->seal_frame) {
		local->seal_frame = kernel_anim[aa[0]].frame;
		seal_reset_frame = -1;

		switch (local->seal_frame) {
		case 8:
			conv_run(CONV_QUEEN);
			local->queen_action = QUEEN_SHUT_UP;
			break;

		case 9:   /* end of coming into room */
		case 10:  /* end talk 1              */
		case 11:  /* end talk 2              */
		case 12:  /* end talk 3              */
		case 13:  /* end freeze 1            */
		case 14:  /* end freeze 2            */
			switch (local->seal_action) {
			case SEAL_TALK:
				seal_reset_frame = imath_random(9, 11);
				++local->seal_talk_count;
				if (local->seal_talk_count > 10) {
					local->seal_action = SEAL_SHUT_UP;
					local->seal_talk_count = 0;
					seal_reset_frame = 12;
				}
				break;

			case SEAL_SHUT_UP:
				++local->seal_talk_count;
				if (local->seal_talk_count > imath_random(3, 7)) {
					local->seal_talk_count = 0;
					seal_reset_frame = imath_random(12, 13);
				} else {
					seal_reset_frame = local->seal_frame - 1;
				}
				break;

			case SEAL_LEAVE:
				seal_reset_frame = 14;
				break;
			}
			break;

		case 21:  /* end of leaving */
			new_room = 117;
			break;
		}

		if (seal_reset_frame >= 0) {
			kernel_reset_animation(aa[0], seal_reset_frame);
			local->seal_frame = seal_reset_frame;
		}
	}
}

static void handle_animation_queen() {
	int queen_reset_frame;
	int random;

	if (kernel_anim[aa[1]].frame != local->queen_frame) {
		local->queen_frame = kernel_anim[aa[1]].frame;
		queen_reset_frame = -1;

		switch (local->queen_frame) {
		case 12:
			aa[0] = kernel_run_animation(kernel_name('s', 1), 0);
			local->anim_0_running = true;
			local->seal_action = SEAL_SHUT_UP;
			break;

		case 13:  /* end of queen signing */
			if (local->queen_action == QUEEN_INIT) {
				queen_reset_frame = 12; /* keep signing */
			} /* else turn to see seal emerging from grate (new node) */
			break;

		case 17:  /* end of freeze                     */
		case 18:  /* end of freeze                     */
		case 19:  /* end of freeze and looking at seal */
		case 26:  /* end talk 1                        */
		case 37:  /* end talk 2                        */
		case 51:  /* end talk 3                        */
			switch (local->queen_action) {
			case QUEEN_TALK:
				random = imath_random(1, 3);
				if (random == 1) {
					queen_reset_frame = 19;  /* do talk 1 */
					local->queen_action = QUEEN_SHUT_UP;
				} else if (random == 2) {
					queen_reset_frame = 26;  /* do talk 2 (new node) */
				} else if (random == 3) {
					queen_reset_frame = 37;  /* do talk 3 (new node) */
				}
				break;

			case QUEEN_SHUT_UP:
				++local->queen_talk_count;
				if (local->queen_talk_count > imath_random(3, 7)) {
					local->queen_talk_count = 0;
					queen_reset_frame = imath_random(16, 18);
				} else {
					queen_reset_frame = local->queen_frame - 1; /* repeat this frame */
				}
				break;
			}
			break;

		case 30:  /* end of coming to this node */
		case 31:  /* end of talk frame          */
		case 32:  /* end of talk frame          */
		case 33:  /* end of talk frame          */
			switch (local->queen_action) {
			case QUEEN_TALK:
				queen_reset_frame = imath_random(30, 32);  /* random talk frames */
				++local->queen_talk_count;
				if (local->queen_talk_count > 17) {
					local->queen_action = QUEEN_SHUT_UP;
					local->queen_talk_count = 0;
					queen_reset_frame = 33;
				}
				break;

			case QUEEN_SHUT_UP:
				queen_reset_frame = 33;  /* end talking (new node) */
				break;
			}
			break;

		case 42:  /* end of coming to this node */
		case 43:  /* end of talk frame          */
		case 44:  /* end of talk frame          */
		case 45:  /* end of talk frame          */

			switch (local->queen_action) {

			case QUEEN_TALK:
				queen_reset_frame = imath_random(42, 44);   /* random talk frames */
				++local->queen_talk_count;
				if (local->queen_talk_count > 17) {
					local->queen_action = QUEEN_SHUT_UP;
					local->queen_talk_count = 0;
					queen_reset_frame = 45;
				}
				break;

			case QUEEN_SHUT_UP:
				queen_reset_frame = 45;  /* end talking (new node) */
				break;
			}
			break;
		}

		if (queen_reset_frame >= 0) {
			kernel_reset_animation(aa[1], queen_reset_frame);
			local->queen_frame = queen_reset_frame;
		}
	}
}

static void room_109_daemon() {
	if (local->anim_0_running) {
		handle_animation_seal();
	}

	if (local->anim_1_running) {
		handle_animation_queen();
	}
}

static void process_conversation_queen() {
	int you_trig_flag = false;
	int me_trig_flag = false;

	if (player_verb == conv005_exit_b_b) {
		local->seal_action = SEAL_LEAVE;
		you_trig_flag = true;
		me_trig_flag = true;
	}

	if (local->seal_action != SEAL_LEAVE) {
		switch (kernel.trigger) {
		case ROOM_109_ME_TALK:
			local->seal_action = SEAL_TALK;
			local->queen_action = QUEEN_SHUT_UP;
			break;

		case ROOM_109_YOU_TALK:
			local->seal_action = SEAL_SHUT_UP;
			local->queen_action = QUEEN_TALK;
			break;
		}
	}

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_109_ME_TALK);
	} /* if me_trig_flag == true, then a me trigger is called from above, not here. */

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_109_YOU_TALK);
	} /* if you_trig_flag == true, then a you trigger is called from above, not here. */

	local->seal_talk_count = 0;
	local->queen_talk_count = 0;
}


static void room_109_pre_parser() {
	if (player_said_1(door_to_guardroom)) {
		if (!player_said_1(walk_through)) {
			player.need_to_walk = false;
		}
	}
}

static void room_109_parser() {
	if (conv_control.running == CONV_QUEEN) {
		process_conversation_queen();
		goto handled;
	}

	if (player.look_around) {
		if (global[player_persona] == PLAYER_IS_KING) {
			text_show(10901);
		} else {
			text_show(10912);
		}
		goto handled;
	}

	if (player_said_2(close, door_to_guardroom)) {
		text_show(10917);
		goto handled;
	}

	if (player_said_2(walk_through, door_to_guardroom)) {
		new_room = 108;
		goto handled;
	}

	/* take out later */
	if (player_said_2(talk_to, floor_grate)) {
		new_room = 117;
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(dungeon_floor)) {
			text_show(10902);
			goto handled;
		}

		if (player_said_1(dungeon_wall)) {
			text_show(10903);
			goto handled;
		}

		if (player_said_1(floor_grate)) {
			if (global[player_persona] == PLAYER_IS_KING) {
				text_show(10904);
			} else {
				text_show(10913);
			}
			goto handled;
		}

		if (player_said_1(calendar)) {
			text_show(10906);
			goto handled;
		}

		if (player_said_1(bedding)) {
			text_show(10907);
			goto handled;
		}

		if (player_said_1(manacles)) {
			text_show(10909);
			goto handled;
		}

		if (player_said_1(door_to_guardroom)) {
			text_show(10911);
			goto handled;
		}
	}

	if ((player_said_1(open) || player_said_1(pull) ||
		player_said_1(push) || player_said_1(take)) &&
		player_said_1(bedding)) {
		/* if (player_not_seal) */
		text_show(10908);
		goto handled;
	}

	if (player_said_2(pull, manacles)) {
		/* if (player_not_seal) */
		text_show(10910);
		goto handled;
	}

	if ((player_said_1(open) || player_said_1(pull) ||
		player_said_1(push)) &&
		player_said_1(floor_grate)) {
		/* if (player_not_seal) */
		text_show(10905);
		/*  } else {
		  text_show(10914);
		  }*/
		goto handled;
	}

	if (player_said_2(open, door_to_guardroom)) {
		text_show(41);
		goto handled;
	}

	if (player_said_2(close, door_to_guardroom)) {
		text_show(44);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_109_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.seal_frame);
	s.syncAsSint16LE(scratch.seal_action);
	s.syncAsSint16LE(scratch.seal_talk_count);
	s.syncAsSint16LE(scratch.anim_0_running);
	s.syncAsSint16LE(scratch.queen_frame);
	s.syncAsSint16LE(scratch.queen_action);
	s.syncAsSint16LE(scratch.queen_talk_count);
	s.syncAsSint16LE(scratch.anim_1_running);
}

void room_109_preload() {
	room_init_code_pointer = room_109_init;
	room_pre_parser_code_pointer = room_109_pre_parser;
	room_parser_code_pointer = room_109_parser;
	room_daemon_code_pointer = room_109_daemon;

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
