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
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section1.h"
#include "mads/madsv2/dragonsphere/rooms/room115.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];       /* Sprite series handles */
	int16 sequence[15];     /* Sequence handles      */
	int16 animation[4];     /* Animation handles     */

	int16 beast_frame;      /* animation frame being held for beast stuff */
	int16 beast_action;       /* Type of action to run for beast animation */
	int16 prevent;
	int16 prevent_2;
	int16 say_text;
	int16 crunch_timer;

	char message[20];     /* String to say to dog */
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

static Scratch scratch;

/* ========================= Sprites ========================= */

#define fx_fire               1  /* rm115x0 */
#define fx_bone               2  /* rm115x1 */
#define fx_throw_bone         3  /* rm115b0 */


/* ======================== Triggers ========================= */

#define ROOM_115_CAN_KILL     70          


/* walk points */
#define START_X_ROOM_116      23
#define START_Y_ROOM_116      91
#define START_X_ROOM_114      305
#define START_Y_ROOM_114      104
#define WALK_TO_X_FROM_116    75
#define WALK_TO_Y_FROM_116    110

#define THROW_BONE_X          149
#define THROW_BONE_Y          132

#define BEAST_EAT_X           102
#define BEAST_EAT_Y           121

#define SAY_WORD_X            192
#define SAY_WORD_Y            137

/* animation controls */
#define LOOK_AROUND           0
#define GET_UP                1
#define ATTACK                2
#define REALLY_ATTACK         3
#define SLEEP                 4

#define BEAST_LAY_X           138
#define BEAST_LAY_Y           85


static void room_115_init() {
	kernel.disable_fastwalk = true;

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->say_text = false;
	}

	local->crunch_timer = 0;

	/* Load sprite series */

	ss[fx_fire] = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_bone] = kernel_load_series(kernel_name('x', 1), false);
	ss[fx_throw_bone] = kernel_load_series(kernel_name('b', 0), false);

	if (global[threw_bone]) {
		seq[fx_bone] = kernel_seq_stamp(ss[fx_bone], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_bone], 11);
	} else {
		kernel_flip_hotspot(words_bone, false);
	}

	kernel_flip_hotspot(words_darkness_beast, false);
	kernel_flip_hotspot_loc(words_darkness_beast, true, BEAST_LAY_X, BEAST_LAY_Y);

	local->prevent = false;
	local->prevent_2 = false;

	/* Start continuous sequences */

	seq[fx_fire] = kernel_seq_forward(ss[fx_fire], false, 7, 0, 0, 0);

	if (previous_room == 116) {  /* Player comes from the Ice\King room */
		player.x = START_X_ROOM_116;
		player.y = START_Y_ROOM_116;
		player.facing = FACING_SOUTHWEST;
		player_walk(WALK_TO_X_FROM_116, WALK_TO_Y_FROM_116, FACING_SOUTHEAST);

	} else if (previous_room != KERNEL_RESTORING_GAME) { /* Player comes from Stair trap rm114 */
		player.x = START_X_ROOM_114;
		player.y = START_Y_ROOM_114;
		player.facing = FACING_WEST;
	}

	aa[0] = kernel_run_animation(kernel_name('d', 1), 0);
	if (global[dog_is_asleep]) {
		local->beast_action = SLEEP;
	} else {
		local->beast_action = LOOK_AROUND;
	}

	if (kernel.teleported_in) {
		inter_give_to_player(bone);
		inter_give_to_player(piece_of_paper);
	}

	section_1_music();
}

static void handle_animation_beast() {
	int beast_reset_frame;
	int random;

	if (kernel_anim[aa[0]].frame != local->beast_frame) {
		local->beast_frame = kernel_anim[aa[0]].frame;
		beast_reset_frame = -1;

		switch (local->beast_frame) {
		case 1:   /* end of head down */
		case 60:  /* end of head down */
		case 80:  /* end of head down */

			if (local->say_text) {
				if (local->say_text == 1) {         /* crystal ball used */
					text_show(11513);
					inter_move_object(crystal_ball, NOWHERE);
					text_show(970);
				} else if (local->say_text == 2) {  /* parchment used */
					text_show(11508);
				}
				player.commands_allowed = true;
				local->say_text = false;
			}

			switch (local->beast_action) {
			case LOOK_AROUND:
				if (imath_random(1, 30) == 1) {
					beast_reset_frame = 1;  /* put head up (new node) */
				} else {
					beast_reset_frame = 0;  /* keep head down */
				}
				break;

			case GET_UP:
			case ATTACK:
				beast_reset_frame = 1;  /* put head up (new node) */
				break;

			case SLEEP:
				beast_reset_frame = 0;  /* keep head down */
				break;
			}
			break;

		case 3:  /* end of head up */
			switch (local->beast_action) {
			case LOOK_AROUND:
				random = imath_random(1, 30);
				if (random == 1) {
					beast_reset_frame = 3;   /* mode head (new node) */
				} else if (random == 2) {
					beast_reset_frame = 58;  /* put head down (new node) */
				} else {
					beast_reset_frame = 2;   /* keep in this position */
				}
				break;

			case GET_UP:
			case ATTACK:
				beast_reset_frame = 3;   /* mode head (new node) */
				break;

			case SLEEP:
				beast_reset_frame = 58;  /* keep head down */
				break;
			}
			break;

		case 4:  /* end of keeping head up */
			switch (local->beast_action) {
			case LOOK_AROUND:
				random = imath_random(1, 30);
				if (random == 1) {
					beast_reset_frame = 4;   /* mode head (new node) */
				} else if (random == 2) {
					beast_reset_frame = 2;   /* move head (new node) */
				} else {
					beast_reset_frame = 3;   /* keep head where it is */
				}
				break;

			case GET_UP:
			case ATTACK:
				beast_reset_frame = 5;   /* get up (new node) */
				kernel_flip_hotspot(words_darkness_beast, true);
				kernel_flip_hotspot_loc(words_darkness_beast, false, BEAST_LAY_X, BEAST_LAY_Y);
				break;

			case SLEEP:
				beast_reset_frame = 2;  /* keep head down */
				break;
			}
			break;

		case 5:  /* end of keeping head up */
			switch (local->beast_action) {
			case LOOK_AROUND:
				random = imath_random(1, 30);
				if (random == 1) {
					beast_reset_frame = 3;   /* mode head (new node) */
				} else {
					beast_reset_frame = 4;   /* keep head where it is */
				}
				break;

			case GET_UP:
			case ATTACK:
			case SLEEP:
				beast_reset_frame = 3;  /* move head (new node) */
				break;
			}
			break;

		case 22:
			if (!global[dog_is_asleep]) {
				local->beast_action = LOOK_AROUND;
			}
			break;

		case 23:  /* end of getting up */
		case 25:  /* end of moving head */
			switch (local->beast_action) {
			case LOOK_AROUND:
				random = imath_random(1, 30);
				if (random == 1) {
					beast_reset_frame = 23;   /* move head (new node) */
				} else if (random == 2) {
					beast_reset_frame = 25;   /* move head (new node) */
				} else if (random == 3) {
					beast_reset_frame = 24;   /* keep head where it is */
				} else if (random == 4) {
					beast_reset_frame = 22;   /* keep head where it is */
				} else {
					beast_reset_frame = local->beast_frame - 1;
				}
				break;

			case ATTACK:
				beast_reset_frame = 25;  /* move head (new node) */
				break;

			case SLEEP:
				kernel_flip_hotspot(words_darkness_beast, false);
				kernel_flip_hotspot_loc(words_darkness_beast, true, BEAST_LAY_X, BEAST_LAY_Y);
				beast_reset_frame = 60;  /* sleep (new node) */
				break;
			}
			break;

		case 24:  /* end of moving head */
			switch (local->beast_action) {
			case LOOK_AROUND:
				random = imath_random(1, 30);
				if (random == 1) {
					beast_reset_frame = 22;   /* move head (new node) */
				} else {
					beast_reset_frame = 23;   /* keep head where it is */
				}
				break;

			case ATTACK:
			case SLEEP:
				beast_reset_frame = 22;  /* move head (new node) */
				break;
			}
			break;

		case 26:  /* end of moving head */
			switch (local->beast_action) {
			case LOOK_AROUND:
				random = imath_random(1, 30);
				if (random == 1) {
					beast_reset_frame = 24;   /* move head (new node) */
				} else if (random == 2) {
					beast_reset_frame = 26;   /* move head (new node) */
				} else {
					beast_reset_frame = 25;   /* keep head where it is */
				}
				break;

			case ATTACK:
				beast_reset_frame = 26;  /* move head (new node) */
				break;

			case SLEEP:
				beast_reset_frame = 24;  /* move head (new node) */
				break;
			}
			break;

		case 27:  /* end of moving head */
			switch (local->beast_action) {
			case LOOK_AROUND:
				random = imath_random(1, 30);
				if (random == 1) {
					beast_reset_frame = 27;   /* move head (new node) */
				} else if (random == 2) {
					beast_reset_frame = 25;   /* move head (new node) */
				} else {
					beast_reset_frame = 26;   /* keep head where it is */
				}
				break;

			case ATTACK:
				beast_reset_frame = 26;  /* attack */
				break;

			case REALLY_ATTACK:
				beast_reset_frame = 29;  /* attack */
				break;

			case SLEEP:
				beast_reset_frame = 25;  /* move head (new node) */
				break;
			}
			break;

		case 28:  /* end of moving head */
			switch (local->beast_action) {
			case LOOK_AROUND:
				random = imath_random(1, 30);
				if (random == 1) {
					beast_reset_frame = 26;   /* move head (new node) */
				} else if (random == 2) {
					beast_reset_frame = 28;   /* move head (new node) */
				} else {
					beast_reset_frame = 27;   /* keep head where it is */
				}
				break;

			case ATTACK:
			case SLEEP:
				beast_reset_frame = 26;  /* attack */
				break;
			}
			break;

		case 29:  /* end of moving head */
			switch (local->beast_action) {
			case LOOK_AROUND:
				random = imath_random(1, 30);
				if (random == 1) {
					beast_reset_frame = 27;   /* move head (new node) */
				} else {
					beast_reset_frame = 28;   /* keep head where it is */
				}
				break;

			case ATTACK:
			case SLEEP:
				beast_reset_frame = 27;  /* move head (new node) */
				break;
			}
			break;

		case 35:
			player.walker_visible = false;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
			break;

		case 36:
			global_speech_go(3);
			break;

		case 57:
			global_speech_go(4);
			break;

		case 58:
			++local->crunch_timer;
			if (local->crunch_timer > 20) {
				kernel.force_restart = true;
				sound_play(N_PlayerDies);
				if (game.difficulty == EASY_MODE) {
					text_show(11521);
				} else {
					text_show(45);
				}
			} else {
				beast_reset_frame = 57;
			}
			break;
		}

		if (beast_reset_frame >= 0) {
			kernel_reset_animation(aa[0], beast_reset_frame);
			local->beast_frame = beast_reset_frame;
		}
	}
}

static void room_115_daemon() {
	handle_animation_beast();

	if (player.x <= 200 && !local->prevent && !global[dog_is_asleep]) {
		local->beast_action = GET_UP;
		local->prevent = true;
	}

	if (player.x <= 140 && !local->prevent_2 && !global[dog_is_asleep]) {
		local->prevent_2 = true;
		player.commands_allowed = false;
		local->beast_action = ATTACK;
		player_walk(BEAST_EAT_X, BEAST_EAT_Y, FACING_NORTHWEST);
		player_walk_trigger(ROOM_115_CAN_KILL);
	}

	if (kernel.trigger == ROOM_115_CAN_KILL) {
		local->beast_action = REALLY_ATTACK;
	}
}

static void room_115_pre_parser() {
	if (player_said_2(walk_down, passageway_to_east)) {
		player.walk_off_edge_to_room = 114;
	}

	if (player_said_2(take, bone)) {
		if (global[threw_bone] && !global[dog_is_asleep]) {
			player.need_to_walk = false;
		}
	}

	if (player_said_2(speak_words_on, parchment)) {
		if (!global[dog_is_asleep]) {
			player_walk(SAY_WORD_X, SAY_WORD_Y, FACING_NORTHWEST);
		}
	}
}

static void room_115_parser() {
	int id;

	if (player.look_around) {
		if (global[dog_is_asleep]) {
			text_show(11519);
		} else {
			text_show(11501);
		}
		goto handled;
	}

	if (player_said_2(walk_down, passageway_to_west)) {
		if (global[dog_is_asleep]) {
			new_room = 116;
		}
		goto handled;
	}


	if (player_said_2(speak_words_on, parchment)) {
		if (!global[dog_is_asleep] || kernel.trigger) {
			switch (kernel.trigger) {
			case 0:
			case 1:
				player.commands_allowed = false;
				if ((local->beast_frame < 23) || (local->beast_frame > 35)) {
					kernel_timing_trigger(10, 1);
				} else {
					sound_play(N_BeastSnd);
					local->message[0] = '"';
					local->message[1] = 1;
					local->message[2] = 2;
					local->message[3] = 3;
					local->message[4] = 4;
					local->message[5] = 5;
					local->message[6] = '!';
					local->message[7] = '"';
					local->message[8] = 0;

					id = kernel_message_add(local->message, 0, 0,
						MESSAGE_COLOR, FOUR_SECONDS,
						0, KERNEL_MESSAGE_PLAYER);
					kernel_message_teletype(id, 8, true);

					kernel_timing_trigger(TWO_SECONDS, 2);

					player.commands_allowed = false;
				}
				break;

			case 2:
				global[dog_is_asleep] = true;
				local->beast_action = SLEEP;
				global[player_score] += 2;
				local->say_text = 2;
				sound_play(N_DogWhimper);
				break;
			}
			goto handled;
		}
	}

	if (player_said_2(invoke_power_of, crystal_ball) && !global[crystal_ball_dead]) {
		sound_play(N_InvokeCrystalBall);
		global[dog_is_asleep] = true;
		global[crystal_ball_dead] = true;
		local->beast_action = SLEEP;
		local->say_text = 1;
		sound_play(N_DogWhimper);
		goto handled;
	}

	if (player_said_3(throw, bone, darkness_beast) ||
		player_said_3(give, bone, darkness_beast) ||
		player_said_3(throw, bone, grotto)) {
		switch (kernel.trigger) {
		case 0:
			if (player_has(bone)) {
				player_walk(THROW_BONE_X, THROW_BONE_Y, FACING_NORTHWEST);
				player_walk_trigger(1);
			}
			break;

		case 1:
			player.commands_allowed = false;
			player.walker_visible = false;
			global[threw_bone] = true;
			seq[fx_throw_bone] = kernel_seq_forward(ss[fx_throw_bone], false, 7, 0, 0, 1);
			kernel_seq_depth(seq[fx_throw_bone], 7);
			kernel_seq_range(seq[fx_throw_bone], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_throw_bone], KERNEL_TRIGGER_EXPIRE, 0, 3);
			kernel_seq_trigger(seq[fx_throw_bone], KERNEL_TRIGGER_SPRITE, 9, 2);
			kernel_synch(KERNEL_SERIES, seq[fx_throw_bone], KERNEL_PLAYER, 0);
			break;

		case 2:
			player.commands_allowed = true;
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_throw_bone]);
			break;

		case 3:
			inter_move_object(bone, NOWHERE);
			seq[fx_bone] = kernel_seq_stamp(ss[fx_bone], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_bone], 14);
			kernel_flip_hotspot(words_bone, true);
			if (!global[dog_is_asleep]) {
				text_show(11507);
			} else {
				text_show(11518);
			}
			break;
		}
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(torch)) {
			text_show(11502);
			goto handled;
		}

		if (player_said_1(edge_of_abyss)) {
			text_show(11503);
			goto handled;
		}

		if (player_said_1(passageway_to_east)) {
			text_show(11504);
			goto handled;
		}

		if (player_said_1(passageway_to_west)) {
			text_show(11505);
			goto handled;
		}

		if (player_said_1(grotto)) {
			text_show(11520);
			goto handled;
		}

		if (player_said_1(darkness_beast)) {
			if (global[dog_is_asleep]) {
				text_show(11508);
			} else {
				text_show(11506);
			}
			goto handled;
		}

		if (player_said_1(bone) && !player_has(bone)) {
			text_show(11509);
			goto handled;
		}
	}

	if (player_said_3(throw, dates, darkness_beast)) {
		text_show(11514);
		goto handled;
	}

	if (player_said_2(gaze_into, crystal_ball)) {
		if (!global[dog_is_asleep]) {
			text_show(11512);
			goto handled;
		}
	}

	if (player_said_2(take, bone) && !player_has(bone)) {
		if (global[dog_is_asleep]) {
			text_show(11511);
		} else {
			text_show(11510);
		}
		goto handled;
	}

	if (player_said_2(open, bottle_of_flies)) {
		text_show(11516);
		goto handled;
	}

	if (player_said_3(give, soptus_soporific, darkness_beast)) {
		text_show(11517);
		goto handled;
	}

	if (player_said_3(sword, attack, darkness_beast) ||
		player_said_3(sword, carve_up, darkness_beast) ||
		player_said_3(sword, thrust, darkness_beast)) {
		text_show(11515);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_115_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.beast_frame);
	s.syncAsSint16LE(scratch.beast_action);
	s.syncAsSint16LE(scratch.prevent);
	s.syncAsSint16LE(scratch.prevent_2);
	s.syncAsSint16LE(scratch.say_text);
	s.syncAsSint16LE(scratch.crunch_timer);
	s.syncBytes((byte *)scratch.message, sizeof(scratch.message));
}

void room_115_preload() {
	room_init_code_pointer = room_115_init;
	room_pre_parser_code_pointer = room_115_pre_parser;
	room_parser_code_pointer = room_115_parser;
	room_daemon_code_pointer = room_115_daemon;

	section_1_walker();
	section_1_interface();

	vocab_make_active(words_darkness_beast);
	vocab_make_active(words_walk_to);
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
