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
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section1.h"
#include "mads/madsv2/dragonsphere/rooms/room101.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

/**
 * Room local variables
 */
struct Scratch {
	int16 sprite[15];       /* Sprite series handles */
	int16 sequence[15];     /* Sequence handles      */
	int16 animation[4];     /* Animation handles     */

	int16 prev_door_seq;        /* outgoing door seq handle held during door-close synch */
	int16 fireplace_examined;   /* flag: -1 once fireplace has been looked at */
	int16 queen_prev_frame;     /* last animation-B frame seen (book mode change detection) */
	int16 queen_frame;          /* current animation-B frame */
	int16 king_frame;           /* current animation-C frame */
	int16 queen_action;         /* queen animation state (CONV0_WALK_IN … CONV0_LEAVE) */
	int16 king_action;          /* king animation state (CONV0_KING_SLEEP/TALK/GET_UP) */
	int16 pending_queen_action; /* queen action queued by conversation, consumed in daemon */
	int16 pending_king_action;  /* king action queued by conversation/pre-parser, consumed in daemon */
	int32 tick_accum;           /* accumulated game ticks for idle-animation timer */
	int32 elapsed_ticks;        /* last computed elapsed-tick delta */
	int32 last_clock;           /* clock snapshot used to compute elapsed_ticks */
	int16 timer_target;         /* random tick count at which next idle animation fires */
	int16 queen_anim_mode;      /* animation-B processing mode: 0=off, 1=book, 2=queen */
	int16 king_anim_mode;       /* animation-C processing mode: 0=done, 3=intro active */
	int16 resume_conv;          /* flag: -1 = call conv_run on next queen frame-69 pass */
	int16 suppress_command;     /* flag: -1 = silently consume next parser command (bed-exit interlock) */
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation


/* ========================= Sprite Series =================== */

#define fx_fire              1  /* rm101x0 */
#define fx_fire_shadow       2  /* rm101x1 */
#define fx_door              3  /* rm101x2 */
#define fx_sconce_fire       4  /* rm101x3 */
#define fx_draped_cape       5  /* rm101a5 */
#define fx_open_door         6  /* kgrd_6  */


/* ========================== Triggers ======================= */

#define ROOM_101_I_AM_SO_REFRESHED  70
#define ROOM_101_DOOR_CLOSES        80


#define CONVERSATION_WITH_QUEEN     0

/* Walk points */
#define START_X_ROOM_102        330
#define START_Y_ROOM_102        126
#define START_X_ROOM_103        114
#define START_Y_ROOM_103        152

#define WALK_TO_X_FROM_102      297
#define WALK_TO_Y_FROM_102      134

#define WALK_TO_DOOR_X          319
#define WALK_TO_DOOR_Y          129

#define START_BEHIND_SCREEN_X   76
#define START_BEHIND_SCREEN_Y   100

#define WALK_FROM_SCREEN_X      102
#define WALK_FROM_SCREEN_Y      100

#define MIDDLE_ROOM_X           109
#define MIDDLE_ROOM_Y           122

/* Queen states (rm101b.aa) */
#define CONV0_WALK_IN         1
#define CONV0_STIR            2
#define CONV0_STAND_1         3
#define CONV0_STAND_2         4
#define CONV0_SHAKE_FIST      5
#define CONV0_TALK_LEAVE      6
#define CONV0_LEAVE           7

/* King states (rm101c.aa) */
#define CONV0_KING_SLEEP      1
#define CONV0_KING_TALK       2
#define CONV0_KING_GET_UP     3

/* Animation running flags */
#define BOOK                  1
#define QUEEN                 2
#define KING                  3

/* Timer macros */
#define TIME_TO_MOVE_1        200
#define TIME_TO_MOVE_2        300
#define TIME_TO_MOVE_3        400
#define TIME_TO_MOVE_4        500
#define TIME_TO_MOVE_5        600
#define TIME_TO_MOVE_6        700
#define TIME_TO_MOVE_7        300

/* Random range */
#define RANDOM_LOWEST_NUMBER  1
#define RANDOM_HIGHEST_NUMBER 6


static Scratch scratch;

void room_101_init() {
	conv_get(0);

	ss[fx_fire]        = kernel_load_series(kernel_name('x', 0), 0);
	ss[fx_fire_shadow] = kernel_load_series(kernel_name('x', 1), 0);
	ss[fx_door]        = kernel_load_series(kernel_name('x', 2), 0);
	ss[fx_sconce_fire] = kernel_load_series(kernel_name('x', 3), 0);
	ss[fx_draped_cape] = kernel_load_series(kernel_name('a', 5), 0);
	ss[fx_open_door]   = kernel_load_series("*KGRD_6", 0);

	seq[fx_fire]        = kernel_seq_forward(ss[fx_fire],        false, 7, 0, 0, 0);
	seq[fx_fire_shadow] = kernel_seq_forward(ss[fx_fire_shadow], false, 7, 0, 0, 0);
	seq[fx_sconce_fire] = kernel_seq_forward(ss[fx_sconce_fire], false, 7, 0, 0, 0);

	if (global[crawled_out_of_bed_101] != 0) {
		seq[fx_draped_cape] = kernel_seq_stamp(ss[fx_draped_cape], 0, 8);
		kernel_seq_depth(seq[fx_draped_cape], 11);
	}

	scratch.fireplace_examined = 0;

	if (player.been_here_before == 0 || conv_restore_running == 0) {
		scratch.king_action = 1;
		player.walker_visible = 0;
		player.commands_allowed = 0;
		scratch.pending_king_action = 0;
		scratch.king_anim_mode = 3;

		aa[1] = kernel_run_animation(kernel_name('C', -1), 0);
		scratch.queen_action = 1;
		scratch.pending_queen_action = 0;
		scratch.tick_accum = 0;
		scratch.resume_conv = 0;
		scratch.queen_anim_mode = 2;
		scratch.suppress_command = 0;

		aa[0] = kernel_run_animation(kernel_name('B', -1), 0);

		if (conv_restore_running != 0) {
			goto done;
		}

		conv_run(0);
		scratch.queen_action = 3;
		scratch.king_action = 1;
		kernel_reset_animation(aa[0], 69);
		goto done;
	}

	// player.been_here_before != 0 && conv_restore_running != 0
	if (previous_room == 103) {
		player.x = 114;
		player.y = 152;
		player.facing = 9;
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], 0, -1);
		kernel_seq_depth(seq[fx_door], 12);
		goto done;
	}

	if (previous_room != -2) {
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], 0, -2);
		kernel_seq_depth(seq[fx_door], 12);
		player_first_walk(330, 126, 4, 4, 134, 297, 0);
		player_walk_trigger(80);
		goto done;
	}

	// previous_room == -2
	seq[fx_door] = kernel_seq_stamp(ss[fx_door], 0, -1);
	kernel_seq_depth(seq[fx_door], 12);

	if (scratch.king_action == 1) {
		player.walker_visible = 0;
		aa[1] = kernel_run_animation(kernel_name('C', -1), 0);
	}

done:
	section_1_music();
}

static void room_101_anim1() {
	int val = imath_random(1, 6);

	switch (val) {
	case 1: scratch.timer_target = 200; break;
	case 2: scratch.timer_target = 300; break;
	case 3: scratch.timer_target = 400; break;
	case 4: scratch.timer_target = 500; break;
	case 5: scratch.timer_target = 600; break;
	case 6: scratch.timer_target = 700; break;
	default:
		break;
	}
}

static void room_101_anim2(int16 *ptr) {
	int32 elapsed = kernel.clock - scratch.last_clock;
	scratch.elapsed_ticks = elapsed;

	if (elapsed < 0 || elapsed > 4) {
		scratch.tick_accum += 1;
	} else {
		scratch.tick_accum += elapsed;
	}

	scratch.last_clock = kernel.clock;

	int32 target = scratch.timer_target;

	if (target > scratch.tick_accum) {
		if (scratch.queen_action == 7) {
			player.commands_allowed = 0;
		} else if (scratch.queen_frame == 'E') {
			*ptr = 'B';
			return;
		}

		*ptr = 'E';
		return;
	}

	scratch.tick_accum = 0;
	scratch.timer_target = 0;

	if (scratch.queen_action == 7) {
		player.commands_allowed = 0;
		*ptr = 'a';
		return;
	}

	if (scratch.queen_frame == 'E') {
		scratch.queen_action = 4;
	} else {
		*ptr = 'B';
		scratch.queen_action = 3;
		return;
	}

	*ptr = 'E';
}

void room_101_daemon() {
	int16 var_4 = -1;  // target frame for anim B (aa[0])
	int16 var_2 = -1;  // target frame for anim C (aa[1])
	int frame_c = 0;
	int trig;

	// --- Block 1: Handle anim B (aa[0]) when queen_anim_mode == 1 (book) ---
	int bx = aa[0];
	if (kernel_anim[bx].anim != 0 && scratch.queen_anim_mode == 1) {
		if (kernel_anim[bx].frame != scratch.queen_prev_frame) {
			var_4 = -1;
			scratch.queen_prev_frame = kernel_anim[bx].frame;

			if (kernel_anim[bx].frame - 9 == 0) {
				text_show(10107);
			}

			if (var_4 >= 0 && kernel_anim[aa[0]].frame != var_4) {
				kernel_reset_animation(aa[0], var_4);
				scratch.queen_prev_frame = var_4;
			}
		}
	}

	// --- Block 2: Handle anim B (aa[0]) when queen_anim_mode == 2 (queen) ---
	if (scratch.queen_anim_mode == 2) {
		bx = aa[0];
		if (kernel_anim[bx].frame != scratch.queen_frame) {
			scratch.queen_frame = kernel_anim[bx].frame;
			var_4 = -1;

			if (scratch.pending_queen_action > 0) {
				scratch.queen_action = scratch.pending_queen_action;
				scratch.pending_queen_action = 0;
			}

			int frame = scratch.queen_frame;
			if (frame == 69) {
				goto block_frame69;
			} else if (frame > 69) {
				goto block_frame_gt69;
			} else {
				// frame < 69
				frame -= 48;
				if (frame == 0) {
					// original frame was 48
					if (scratch.queen_action == 1) {
						scratch.queen_action = 3;
						scratch.resume_conv = -1;
						var_4 = 'B';
					}
				} else {
					frame -= 18;
					if (frame == 0) {
						// original frame was 66
						if (scratch.queen_action == 2) {
							scratch.queen_action = 3;
						}
					}
					// else: default, fall through with var_4 == -1
				}
			}

			// Apply var_4 to anim B if valid
			if (var_4 >= 0 && kernel_anim[aa[0]].frame != var_4) {
				kernel_reset_animation(aa[0], var_4);
				scratch.queen_frame = var_4;
			}
		}
	}

	// --- Block 3: Handle anim C (aa[1]) when king_anim_mode == 3 ---
	bx = aa[1];
	if (kernel_anim[bx].anim == 0) goto check_trigger;
	if (scratch.king_anim_mode != 3) goto check_trigger;
	if (kernel_anim[bx].frame == scratch.king_frame) goto check_trigger;

	scratch.king_frame = kernel_anim[bx].frame;
	var_2 = -1;

	if (scratch.pending_king_action > 0) {
		scratch.king_action = scratch.pending_king_action;
		scratch.pending_king_action = 0;
	}

	frame_c = scratch.king_frame;
	switch (frame_c) {
	case 3:
		if (scratch.king_action == 1) var_2 = 0;
		if (scratch.king_action == 3) var_2 = 0x1D;
		break;
	case 29:
		conv_release();
		scratch.king_action = 1;
		var_2 = 0;
		break;
	case 64:
		global[player_score]++;
		sound_play(64);
		break;
	case 105:
		text_show(10116);
		break;
	case 148:
		seq[fx_draped_cape] = kernel_seq_stamp(ss[fx_draped_cape], 0, 8);
		kernel_seq_depth(seq[fx_draped_cape], 11);
		kernel_synch(1, seq[fx_draped_cape], 3, aa[1]);
		scratch.king_action = 0;
		player.x = 76;
		player.y = 100;
		player.walker_visible = -1;
		player_demand_facing(6);
		player_walk(109, 122, 3);
		player_walk_trigger(70);
		break;
	default:
		break;
	}

	// Apply var_2 to anim C if valid
	if (var_2 >= 0 && kernel_anim[aa[1]].frame != var_2) {
		kernel_reset_animation(aa[1], var_2);
		scratch.king_frame = var_2;
	}

check_trigger:
	if (kernel.trigger == 'F') {
		player.commands_allowed = -1;
		scratch.king_anim_mode = 0;
		text_show(10140);
	}

	if (kernel.trigger < 'P') return;

	// trigger >= 'P' (0x50 = 80)
	trig = kernel.trigger - 80;
	if (trig == 0) {
		// trigger == 80: loc_3915E
		kernel_seq_delete(seq[fx_door]);
		sound_play(25);
		seq[fx_door] = kernel_seq_backward(ss[fx_door], 0, 6, 1, 0, 0);
		kernel_seq_depth(seq[fx_door], 14);
		kernel_seq_range(seq[fx_door], 1, 4);
		kernel_seq_trigger(seq[fx_door], 0, 0, 81);
		return;
	}
	if (trig == 1) {
		// trigger == 81: loc_391AA
		scratch.prev_door_seq = seq[fx_door];
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], 0, -1);
		kernel_seq_depth(seq[fx_door], 14);
		kernel_synch(1, seq[fx_door], 1, scratch.prev_door_seq);
		player.commands_allowed = -1;
		return;
	}

	return;

	// --- frame == 69 block (called from block 2) ---
block_frame69:
	if (scratch.queen_action == 3 || scratch.queen_action == 4) {
		if (scratch.resume_conv != 0) {
			conv_run(0);
			scratch.resume_conv = 0;
		}
		switch (scratch.queen_action) {
		case 7:
			player.commands_allowed = 0;
			var_4 = 'a';
			break;
		case 2:
			var_4 = '0';
			break;
		case 5:
			var_4 = 'G';
			break;
		case 6:
			room_101_anim1();
			room_101_anim2(&var_4);
			break;
		default:
			break;
		}
	} else if (scratch.queen_action == 7) {
		player.commands_allowed = 0;
		var_4 = 'a';
	} else if (scratch.queen_action == 2) {
		var_4 = '0';
	} else if (scratch.queen_action == 5) {
		var_4 = 'G';
	} else if (scratch.queen_action == 6) {
		room_101_anim1();
		room_101_anim2(&var_4);
	}

	// Apply var_4 and return to block 2 exit
	if (var_4 >= 0 && kernel_anim[aa[0]].frame != var_4) {
		kernel_reset_animation(aa[0], var_4);
		scratch.queen_frame = var_4;
	}
	return;

	// --- frame > 69 block ---
block_frame_gt69:
	frame_c -= 0x47;  // 'G'
	if (frame_c == 0) {
		// original was 'G' (71): loop back to frame69 block
		goto block_frame69;
	}
	frame_c -= 26;
	if (frame_c == 0) {
		// original was 97 ('a'): loc_3900C
		scratch.queen_action = 3;
		var_4 = 'B';
		goto apply_var4_and_continue;
	}
	frame_c -= 54;
	if (frame_c == 0) {
		// original was 177: loc_39016
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], 0, -1);
		kernel_seq_depth(seq[fx_door], 14);
		kernel_synch(1, seq[fx_door], 3, aa[0]);
		player.commands_allowed = -1;
		scratch.queen_anim_mode = 0;
	}
	// fall through to apply var_4

apply_var4_and_continue:
	if (var_4 >= 0 && kernel_anim[aa[0]].frame != var_4) {
		kernel_reset_animation(aa[0], var_4);
		scratch.queen_frame = var_4;
	}
}

void room_101_pre_parser() {
	if (player_parse(3, 0) == 0 &&
			player_parse(30, 0) == 0 &&
			player_parse(9, 0) == 0 &&
			player_parse(8, 0) == 0 &&
			player_parse(7, 0) == 0 &&
			player_parse(12, 0) == 0 &&
			scratch.king_action == 1 &&
			player.need_to_walk != 0) {
		player.commands_allowed = 0;
		player.ready_to_walk = 0;
		scratch.pending_king_action = 3;
		global[crawled_out_of_bed_101] = -1;
		scratch.suppress_command = -1;
	}

	if (player_parse(47, 46, 0) != 0) {
		if (scratch.king_action == 1) {
			player.commands_allowed = 0;
			scratch.pending_king_action = 3;
			global[crawled_out_of_bed_101] = -1;
			scratch.suppress_command = -1;
			player_cancel_command();
		}
	}
}

static void process_conversation_queen() {
	if (player_verb == 0) {
		switch (kernel.trigger) {
		case 0:
			conv_you_trigger(1);
			break;
		case 1:
			scratch.pending_queen_action = 2;
			break;
		}
	}

	if (player_verb == 1) {
		switch (kernel.trigger) {
		case 0:
			conv_me_trigger(1);
			global[player_score]++;
			break;
		case 1:
			conv_hold();
			scratch.pending_king_action = 2;
			conv_you_trigger(2);
			break;
		case 2:
			scratch.pending_queen_action = 6;
			conv_me_trigger(3);
			break;
		case 3:
			scratch.pending_queen_action = 7;
			break;
		}
	}

	if (player_verb == 2) {
		switch (kernel.trigger) {
		case 0:
			conv_me_trigger(1);
			return;
		case 1:
			conv_hold();
			scratch.pending_king_action = 2;
			conv_you_trigger(2);
			return;
		case 2:
			scratch.pending_queen_action = 5;
			return;
		}
	}
}

void room_101_parser() {
	if (player.look_around) {
		text_show(10101);
		goto handled;
	}

	if (scratch.suppress_command) {
		scratch.suppress_command = 0;
		goto handled;
	}

	if (conv_control.running == CONVERSATION_WITH_QUEEN) {
		process_conversation_queen();
		goto handled;
	}

	// Door to queen's room: walk(37), open(6), use(10) + door(36)
	if (player_parse(37, 36, 0) || player_parse(6, 36, 0) || player_parse(10, 36, 0)) {
		if (kernel_anim[aa[1]].anim != 0) goto done;
		if (scratch.king_anim_mode == 3) goto done;
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = 0;
			player.walker_visible = 0;
			seq[fx_open_door] = kernel_seq_pingpong(ss[fx_open_door], false, 7, 0, 0, 2);
			kernel_seq_player(seq[fx_open_door], -1);
			kernel_seq_trigger(seq[fx_open_door], 2, 2, 1);
			kernel_seq_trigger(seq[fx_open_door], 0, 0, 3);
			goto handled;
		case 1:
			kernel_seq_delete(seq[fx_door]);
			sound_play(24);
			seq[fx_door] = kernel_seq_forward(ss[fx_door], false, 7, 0, 0, 1);
			kernel_seq_depth(seq[fx_door], 12);
			kernel_seq_trigger(seq[fx_door], 0, 0, 2);
			goto handled;
		case 2:
			scratch.prev_door_seq = seq[fx_door];
			seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, 5);
			kernel_seq_depth(seq[fx_door], 12);
			kernel_synch(1, seq[fx_door], 1, scratch.prev_door_seq);
			goto handled;
		case 3:
			player.walker_visible = -1;
			kernel_synch(2, 0, 1, seq[fx_open_door]);
			player_walk(319, 129, 6);
			player.walk_off_edge_to_room = 102;
			goto handled;
		}
		goto handled;
	}

	// Exit south to room 103: walk(39) + exit(38)
	if (player_parse(39, 38, 0)) {
		if (kernel_anim[aa[1]].anim != 0) goto done;
		if (scratch.king_anim_mode == 3) goto done;
		new_room = 103;
		goto handled;
	}

	// Open/read book: open(6) + book(33)
	if (player_parse(6, 33, 0)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = 0;
			player.walker_visible = 0;
			scratch.queen_anim_mode = 1;
			aa[0] = kernel_run_animation(kernel_name('A', -1), 1);
			goto handled;
		case 1:
			player.walker_visible = -1;
			kernel_synch(2, 0, 3, aa[0]);
			if (!(global[player_score_flags] & 2)) {
				global[player_score_flags] |= 2;
				global[player_score] += 3;
			}
			player.commands_allowed = -1;
			goto handled;
		}
		goto handled;
	}

	// Look verbs: look(3), examine(30)
	if (player_parse(3, 0) || player_parse(30, 0)) {
		if (player_parse(21, 0)) {
			if (kernel_anim[aa[1]].anim != 0 && scratch.king_anim_mode == 3)
				text_show(10139);
			else
				text_show(10102);
			goto handled;
		}
		if (player_parse(16, 0)) { text_show(10104); goto handled; }
		if (player_parse(33, 0)) { text_show(10105); goto handled; }
		if (player_parse(27, 0)) {
			if (kernel_anim[aa[1]].anim != 0 && scratch.king_anim_mode == 3)
				text_show(10108);
			else
				text_show(10109);
			goto handled;
		}
		if (player_parse(41, 0)) { text_show(10110); goto handled; }
		if (player_parse(40, 0)) { text_show(10111); goto handled; }
		if (player_parse(45, 0)) { text_show(10112); goto handled; }
		if (player_parse(329, 0)) { text_show(10113); goto handled; }
		if (player_parse(31, 0)) {
			if (kernel_anim[aa[1]].anim != 0 && scratch.king_anim_mode == 3)
				text_show(10114);
			else
				text_show(10115);
			goto handled;
		}
		if (player_parse(32, 0)) { text_show(10117); goto handled; }
		if (player_parse(34, 0)) { text_show(10118); goto handled; }
		if (player_parse(35, 0)) { text_show(10119); goto handled; }
		if (player_parse(38, 0)) { text_show(10121); goto handled; }
		if (player_parse(22, 0)) {
			if ((kernel_anim[aa[1]].anim != 0 && scratch.king_anim_mode == 3) || scratch.fireplace_examined == 0) {
				text_show(10123);
				scratch.fireplace_examined = -1;
			} else {
				text_show(10124);
			}
			goto handled;
		}
		if (player_parse(29, 0)) { text_show(10126); goto handled; }
		if (player_parse(44, 0)) { text_show(10127); goto handled; }
		if (player_parse(38, 0)) { text_show(10128); goto handled; }  // dead code: noun 38 already matched above
		if (player_parse(18, 0)) { text_show(10129); goto handled; }
		if (player_parse(42, 0)) { text_show(10131); goto handled; }
		if (player_parse(25, 0)) { text_show(10133); goto handled; }
		if (player_parse(23, 0)) { text_show(10134); goto handled; }
		if (player_parse(273, 0) || player_parse(569, 0)) { text_show(10141); goto handled; }
		if (player_parse(24, 0)) { text_show(10136); goto handled; }
		if (player_parse(36, 0)) { text_show(10138); goto handled; }
	}

	// Take/use/push/pull verbs — also reached by look fallthrough when no noun matched
	if ((player_parse(5, 0) || player_parse(10, 0)) && player_parse(21, 0)) {
		if (kernel_anim[aa[1]].anim != 0) goto handled;
		if (scratch.king_anim_mode == 3) goto handled;
		text_show(10103);
		goto handled;
	}
	if (player_parse(4, 33, 0)) {
		if (kernel_anim[aa[1]].anim != 0) goto handled;
		if (scratch.king_anim_mode == 3) goto handled;
		text_show(10106);
		goto handled;
	}
	if ((player_parse(5, 0) || player_parse(10, 0)) && player_parse(35, 0)) {
		if (kernel_anim[aa[1]].anim != 0) goto handled;
		if (scratch.king_anim_mode == 3) goto handled;
		text_show(10120);
		goto handled;
	}
	if ((player_parse(4, 0) || player_parse(10, 0)) && player_parse(42, 0)) {
		if (kernel_anim[aa[1]].anim != 0) goto handled;
		if (scratch.king_anim_mode == 3) goto handled;
		text_show(10132);
		goto handled;
	}
	if (player_parse(6, 24, 0)) {
		if (kernel_anim[aa[1]].anim != 0) goto handled;
		if (scratch.king_anim_mode == 3) goto handled;
		text_show(10137);
		goto handled;
	}
	if (player_parse(6, 23, 0)) {
		if (kernel_anim[aa[1]].anim != 0) goto handled;
		if (scratch.king_anim_mode == 3) goto handled;
		text_show(10135);
		goto handled;
	}
	if (player_parse(6, 22, 0)) {
		if (kernel_anim[aa[1]].anim != 0) goto handled;
		if (scratch.king_anim_mode == 3) goto handled;
		text_show(10125);
		goto handled;
	}
	if ((player_parse(5, 0) || player_parse(10, 0)) && player_parse(38, 0)) {
		if (kernel_anim[aa[1]].anim != 0) goto handled;
		if (scratch.king_anim_mode == 3) goto handled;
		text_show(10122);
		goto handled;
	}
	if (player_parse(10, 18, 0)) {
		// Unlike other use-verb blocks, blocking here falls through rather than consuming command
		if (kernel_anim[aa[1]].anim == 0 && scratch.king_anim_mode != 3) {
			text_show(10130);
			goto handled;
		}
	}
	if (player_parse(4, 569, 0) || player_parse(4, 273, 0)) {
		text_show(10142);
		goto handled;
	}
	if (player_parse(28, 27, 0)) {
		if (player.x == 76 && player.y == 100) {
			text_show(10143);
			goto handled;
		}
	}

	goto done;
handled:
	player.command_ready = false;
done:
	;
}

void room_101_error() {
}

void room_101_synchronize(Common::Serializer &s) {
	for (int i = 0; i < 15; i++) s.syncAsSint16LE(local->sprite[i]);
	for (int i = 0; i < 15; i++) s.syncAsSint16LE(local->sequence[i]);
	for (int i = 0; i < 4; i++)  s.syncAsSint16LE(local->animation[i]);
	s.syncAsSint16LE(scratch.prev_door_seq);
	s.syncAsSint16LE(scratch.fireplace_examined);
	s.syncAsSint16LE(scratch.queen_prev_frame);
	s.syncAsSint16LE(scratch.queen_frame);
	s.syncAsSint16LE(scratch.king_frame);
	s.syncAsSint16LE(scratch.queen_action);
	s.syncAsSint16LE(scratch.king_action);
	s.syncAsSint16LE(scratch.pending_queen_action);
	s.syncAsSint16LE(scratch.pending_king_action);
	s.syncAsSint32LE(scratch.tick_accum);
	s.syncAsSint32LE(scratch.elapsed_ticks);
	s.syncAsSint32LE(scratch.last_clock);
	s.syncAsSint16LE(scratch.timer_target);
	s.syncAsSint16LE(scratch.queen_anim_mode);
	s.syncAsSint16LE(scratch.king_anim_mode);
	s.syncAsSint16LE(scratch.resume_conv);
	s.syncAsSint16LE(scratch.suppress_command);
}

void room_101_preload() {
	room_init_code_pointer = room_101_init;
	room_pre_parser_code_pointer = room_101_pre_parser;
	room_parser_code_pointer = room_101_parser;
	room_daemon_code_pointer = room_101_daemon;

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
