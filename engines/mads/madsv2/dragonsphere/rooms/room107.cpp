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
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section1.h"
#include "mads/madsv2/dragonsphere/rooms/room107.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];       /* Sprite series handles */
	int16 sequence[15];     /* Sequence handles      */
	int16 animation[4];     /* Animation handles     */


	int16 temp;             /* for synching sprites */

	int16 qm_frame;         /* animation frame being held for Queen mom stuff */
	int16 qm_action;        /* Type of action to run for Queem mom animation */
	int16 anim_0_running;

	int16 q_frame;          /* animation frame being held for Queen stuff */
	int16 q_action;         /* Type of action to run for Queen animation */
	int16 q_talk_count;     /* counter for Queen talking */
	int16 anim_1_running;

	int16 prevent;
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

static Scratch scratch;

/* ========================= Sprites ========================= */

#define fx_shieldstone        1  /* rm107p0 */
#define fx_sword              2  /* rm107p1 */
#define fx_take_sword_stone   3  /* kgrm_3  */
#define fx_door               4  /* rm107x  */ 
#define fx_open_door          5  /* kgrd_9  */ 
#define fx_fire               6  /* rm107y0 */
#define fx_chandelier_candles 7  /* rm107y1 */
#define fx_fire_sconce        8  /* rm107y2 */


/* ======================== Triggers ========================= */

#define ROOM_107_DOOR_CLOSES  70
#define ROOM_107_TALK         78
#define ROOM_107_YOU_TALK     80
#define ROOM_107_ME_TALK      82


/* walk points */
#define START_X_ROOM_108      28
#define START_Y_ROOM_108      152
#define START_X_ROOM_106      68
#define START_Y_ROOM_106      83
#define START_X_ROOM_106_2    66
#define START_Y_ROOM_106_2    76
#define WALK_TO_X_FROM_106    70
#define WALK_TO_Y_FROM_106    102

/* cursor_points */
#define DOCUMENT_1_Y          129
#define DOCUMENT_3_X          136

#define TALK_X                59
#define TALK_Y                106

#define QM_SHUT_UP            0
#define QM_TALK               1
#define QM_LEAVE              2

#define Q_SHUT_UP             0
#define Q_TALK                1
#define Q_TALK_TO_QM          2
#define Q_LEAVE               3

#define CONV_4_QUEENS         4


static void room_107_init() {
	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
		local->anim_1_running = false;
		local->prevent = false;
	}

	conv_get(CONV_4_QUEENS);


	/* Load sprite series */

	ss[fx_fire] = kernel_load_series(kernel_name('y', 0), false);
	ss[fx_chandelier_candles] = kernel_load_series(kernel_name('y', 1), false);
	ss[fx_door] = kernel_load_series(kernel_name('x', -1), false);
	ss[fx_open_door] = kernel_load_series("*KGRD_9", false);
	ss[fx_fire_sconce] = kernel_load_series(kernel_name('y', 2), false);


	seq[fx_fire_sconce] = kernel_seq_forward(ss[fx_fire_sconce], false, 7, 0, 0, 0);
	seq[fx_fire] = kernel_seq_forward(ss[fx_fire], false, 7, 0, 0, 0);
	kernel_seq_depth(seq[fx_fire], 15);
	seq[fx_chandelier_candles] = kernel_seq_forward(ss[fx_chandelier_candles], false, 7, 0, 0, 0);

	if (object_is_here(sword) || object_is_here(shieldstone)) {
		ss[fx_take_sword_stone] = kernel_load_series("*KGRM_3", false);
	}

	/* Start continuous sequences */
	/* must put dynamics first before sprite sequencing */

	if (object_is_here(sword)) {
		ss[fx_sword] = kernel_load_series(kernel_name('p', 1), false);
		seq[fx_sword] = kernel_seq_stamp(ss[fx_sword], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_sword], 6);

	} else {
		kernel_flip_hotspot(words_sword, false);
	}

	if (object_is_here(shieldstone)) {
		ss[fx_shieldstone] = kernel_load_series(kernel_name('p', 0), false);
		seq[fx_shieldstone] = kernel_seq_stamp(ss[fx_shieldstone], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_shieldstone], 6);

	} else {
		kernel_flip_hotspot(words_shieldstone, false);
	}

	if (previous_room == 108) {        /* Player comes from the Guardroom */
		player.x = START_X_ROOM_108;
		player.y = START_Y_ROOM_108;
		player.facing = FACING_NORTHEAST;
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 15);

	} else if (previous_room != KERNEL_RESTORING_GAME) { /* Player comes from Throne room rm108 */
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_door], 13);

		if (player.been_here_before) {
			player_first_walk(START_X_ROOM_106, START_Y_ROOM_106, FACING_SOUTH,
				WALK_TO_X_FROM_106, WALK_TO_Y_FROM_106, FACING_SOUTH,
				false);
			player_walk_trigger(ROOM_107_DOOR_CLOSES);

		} else {
			player_first_walk(START_X_ROOM_106, START_Y_ROOM_106, FACING_SOUTH,
				TALK_X, TALK_Y, FACING_NORTHEAST, false);
			player_walk_trigger(ROOM_107_TALK);

			aa[0] = kernel_run_animation(kernel_name('q', 1), 0);
			local->anim_0_running = true;
			local->qm_action = QM_SHUT_UP;
			/* Queen Mother */

			aa[1] = kernel_run_animation(kernel_name('q', -1), 0);
			local->anim_1_running = true;
			local->q_action = Q_SHUT_UP;
			/* Queen */

			player.commands_allowed = false;
		}

	} else {

		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 15);
	}

	section_1_music();
}

static void handle_animation_qm() {
	int qm_reset_frame;

	if (kernel_anim[aa[0]].frame != local->qm_frame) {
		local->qm_frame = kernel_anim[aa[0]].frame;
		qm_reset_frame = -1;

		switch (local->qm_frame) {
		case 7:   /* end of talk           */
		case 8:   /* end of freeze         */
		case 22:  /* end of different talk */
			switch (local->qm_action) {
			case QM_TALK:
				local->qm_action = QM_SHUT_UP;
				if (imath_random(1, 2) == 1) {
					qm_reset_frame = 8;  /* talk */
				} else {
					qm_reset_frame = 0;  /* different talk */
				}
				break;

			case QM_SHUT_UP:
				qm_reset_frame = 7;    /* freeze */
				break;

			case QM_LEAVE:
				qm_reset_frame = 22;   /* leave */
				break;
			}
			break;

		case 62: /* qm is almost out of room */
			kernel_timing_trigger(1, ROOM_107_DOOR_CLOSES);
			break;

		case 68:
			if (local->anim_1_running) {
				qm_reset_frame = 67;

			} else {
				kernel_abort_animation(aa[0]);
				local->anim_0_running = false;
			}
			break;
		}

		if (qm_reset_frame >= 0) {
			kernel_reset_animation(aa[0], qm_reset_frame);
			local->qm_frame = qm_reset_frame;
		}
	}
}

static void handle_animation_q() {
	int q_reset_frame;

	if (kernel_anim[aa[1]].frame != local->q_frame) {
		local->q_frame = kernel_anim[aa[1]].frame;
		q_reset_frame = -1;

		switch (local->q_frame) {

		case 1:   /* end of freeze                       */
		case 10:  /* end of talk                         */
		case 27:  /* end of talk to QM (from other node) */
			switch (local->q_action) {
			case Q_TALK_TO_QM:
				q_reset_frame = 15;   /* talk to QM (new node) */
				break;

			case Q_TALK:
				q_reset_frame = 1;    /* talk */
				local->q_action = Q_SHUT_UP;
				break;

			case Q_LEAVE:
				q_reset_frame = 27;   /* leave */
				break;

			case Q_SHUT_UP:
				q_reset_frame = 0;    /* freeze */
				break;
			}
			break;

		case 21:   /* end of talk frame 1 */
		case 22:   /* end of talk frame 2 */
		case 23:   /* end of talk frame 3 */
			if (local->q_action == Q_TALK_TO_QM) {
				q_reset_frame = imath_random(20, 22);
				++local->q_talk_count;
				if (local->q_talk_count > 15) {
					if (local->q_action != Q_LEAVE) {
						local->q_action = Q_SHUT_UP;
					}
					local->q_talk_count = 0;
					q_reset_frame = 23; /* return to other node to freeze (new node) */
				}
			} else {
				q_reset_frame = 23; /* return to other node to freeze (new node) */
			}
			break;

		case 42:  /* just out the door */
			local->qm_action = QM_LEAVE;
			break;

		case 83:
			kernel_abort_animation(aa[1]);
			local->anim_1_running = false;
			break;
		}

		if (q_reset_frame >= 0) {
			kernel_reset_animation(aa[1], q_reset_frame);
			local->q_frame = q_reset_frame;
		}
	}
}

void room_107_daemon() {
	if (local->anim_0_running) {
		handle_animation_qm();
	}

	if (local->anim_1_running) {
		handle_animation_q();
	}

	if (kernel.trigger >= ROOM_107_DOOR_CLOSES) {
		switch (kernel.trigger) {
		case ROOM_107_DOOR_CLOSES:
			kernel_seq_delete(seq[fx_door]);
			sound_play(N_DoorCloses);
			seq[fx_door] = kernel_seq_backward(ss[fx_door], false,
				8, 0, 0, 1);
			kernel_seq_depth(seq[fx_door], 14);
			kernel_seq_range(seq[fx_door], 1, 5);
			kernel_seq_trigger(seq[fx_door],
				KERNEL_TRIGGER_EXPIRE, 0, ROOM_107_DOOR_CLOSES + 1);
			break;

		case ROOM_107_DOOR_CLOSES + 1:
			local->temp = seq[fx_door];
			seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_door], 14);
			kernel_synch(KERNEL_SERIES, seq[fx_door], KERNEL_SERIES, local->temp);
			if (conv_control.running != CONV_4_QUEENS) {
				player.commands_allowed = true;
			}
			break;
		}
	}

	if (kernel.trigger == ROOM_107_TALK) {
		conv_run(CONV_4_QUEENS);
		conv_export_value(false);
		conv_export_value(false);
	}
}

static void process_conversation_queens() {
	int you_trig_flag = false;
	int me_trig_flag = false;

	if (player_verb == conv004_exit_b_b) {
		local->q_action = Q_LEAVE;
		you_trig_flag = true;
		me_trig_flag = true;
	}

	if (local->q_action != Q_LEAVE) {
		switch (kernel.trigger) {
		case ROOM_107_YOU_TALK:
			switch (player_verb) {
			case conv004_seen_only:
				local->q_action = Q_TALK_TO_QM;
				break;

			case conv004_am_only:
			case conv004_wait_only:
			case conv004_queen_only:
			case conv004_last_only:
				local->q_action = Q_TALK;
				break;

			case conv004_resolved_only:
			case conv004_thanks_only:
			case conv004_nay_only:
			case conv004_fathers_only:
				local->qm_action = QM_TALK;
				local->q_action = Q_SHUT_UP;
				break;
			}
			break;

		case ROOM_107_ME_TALK:
			local->qm_action = QM_SHUT_UP;
			local->q_action = Q_SHUT_UP;
			break;
		}
	}

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_107_ME_TALK);
	} /* if me_trig_flag == true, then a me trigger is called from above, not here. */

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_107_YOU_TALK);
	} /* if you_trig_flag == true, then a you trigger is called from above, not here. */

	local->q_talk_count = 0;
}

static void room_107_pre_parser() {
	if (player_said_2(look, document)) {
		player.need_to_walk = true;
	}
}

static void room_107_parser() {
	if (conv_control.running == CONV_4_QUEENS) {
		process_conversation_queens();
		goto handled;
	}

	if (player.look_around) {
		text_show(10701);
		goto handled;
	}

	if (player_said_2(walk_through, door_to_guardroom) || player_said_2(open, door_to_guardroom) ||
		player_said_2(pull, door_to_guardroom)) {
		new_room = 108;
		goto handled;
	}

	if (player_said_2(walk_through, door_to_throne_room) || player_said_2(open, door_to_throne_room) ||
		player_said_2(pull, door_to_throne_room)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible = false;
			seq[fx_open_door] = kernel_seq_pingpong(ss[fx_open_door],
				true, 8, 0, 0, 2);
			kernel_seq_player(seq[fx_open_door], true);
			kernel_seq_trigger(seq[fx_open_door],
				KERNEL_TRIGGER_SPRITE, 2, 1);
			kernel_seq_trigger(seq[fx_open_door],
				KERNEL_TRIGGER_EXPIRE, 0, 3);
			break;

		case 1:
			kernel_seq_delete(seq[fx_door]);
			sound_play(N_DoorOpens);
			seq[fx_door] = kernel_seq_forward(ss[fx_door], false,
				6, 0, 0, 1);
			kernel_seq_depth(seq[fx_door], 15);
			kernel_seq_trigger(seq[fx_door],
				KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;

		case 2:
			local->temp = seq[fx_door];
			seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_door], 15);
			kernel_synch(KERNEL_SERIES, seq[fx_door], KERNEL_SERIES, local->temp);
			break;

		case 3:
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_open_door]);
			kernel_timing_trigger(1, 4);
			break;

		case 4:
			player_walk(START_X_ROOM_106, START_Y_ROOM_106, FACING_NORTH);
			player_walk_trigger(5);
			break;

		case 5:
			player_walk(START_X_ROOM_106_2, START_Y_ROOM_106_2, FACING_NORTH);
			kernel_seq_delete(seq[fx_door]);
			sound_play(N_DoorCloses);
			seq[fx_door] = kernel_seq_backward(ss[fx_door], false,
				6, 0, 0, 1);
			kernel_seq_depth(seq[fx_door], 1);
			kernel_seq_trigger(seq[fx_door],
				KERNEL_TRIGGER_EXPIRE, 0, 6);
			break;

		case 6:
			new_room = 106;
			break;
		}
		goto handled;
	}

	if (player_said_2(take, sword)) {
		switch (kernel.trigger) {
		case 0:
			if (!player_has(sword)) {
				player.commands_allowed = false;
				player.walker_visible = false;
				seq[fx_take_sword_stone] = kernel_seq_pingpong(ss[fx_take_sword_stone], true, 7, 0, 0, 2);
				kernel_seq_trigger(seq[fx_take_sword_stone], KERNEL_TRIGGER_SPRITE, 5, 2);
				kernel_seq_trigger(seq[fx_take_sword_stone], KERNEL_TRIGGER_EXPIRE, 0, 3);
				kernel_seq_depth(seq[fx_take_sword_stone], 1);
				kernel_seq_range(seq[fx_take_sword_stone], KERNEL_FIRST, 6);
				kernel_seq_player(seq[fx_take_sword_stone], true);
				goto handled;
			}
			break;

		case 2:
			if (local->prevent) {
				kernel_seq_delete(seq[fx_sword]);
				kernel_flip_hotspot(words_sword, false);
				global[player_score] += 1;
				sound_play(N_TakeObjectSnd);
				inter_give_to_player(sword);
				object_examine(sword, 10726, 0);
			}
			local->prevent = true;
			goto handled;
			break;

		case 3:
			local->prevent = false;
			player.walker_visible = true;
			player.commands_allowed = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_sword_stone]);
			goto handled;
			break;
		}
	}

	if (player_said_2(take, shieldstone)) {
		switch (kernel.trigger) {
		case 0:
			if (!player_has(shieldstone)) {
				player.commands_allowed = false;
				player.walker_visible = false;
				seq[fx_take_sword_stone] = kernel_seq_pingpong(ss[fx_take_sword_stone], true, 7, 0, 0, 2);
				kernel_seq_trigger(seq[fx_take_sword_stone], KERNEL_TRIGGER_SPRITE, 3, 2);
				kernel_seq_trigger(seq[fx_take_sword_stone], KERNEL_TRIGGER_EXPIRE, 0, 3);
				kernel_seq_depth(seq[fx_take_sword_stone], 1);
				kernel_seq_range(seq[fx_take_sword_stone], KERNEL_FIRST, 4);
				kernel_seq_player(seq[fx_take_sword_stone], true);
				goto handled;
			}
			break;

		case 2:
			if (local->prevent) {
				kernel_seq_delete(seq[fx_shieldstone]);
				kernel_flip_hotspot(words_shieldstone, false);
				global[player_score] += 1;
				sound_play(N_TakeObjectSnd);
				inter_give_to_player(shieldstone);
				object_examine(shieldstone, 10725, 0);
			}
			local->prevent = true;
			goto handled;
			break;

		case 3:
			local->prevent = false;
			player.walker_visible = true;
			player.commands_allowed = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_sword_stone]);
			goto handled;
			break;
		}
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(quill_pen)) {
			text_show(10702);
			goto handled;
		}

		if (player_said_1(chair)) {
			text_show(10704);
			goto handled;
		}

		if (player_said_1(council_table)) {
			text_show(10705);
			goto handled;
		}

		if (player_said_1(ink_bottle)) {
			text_show(10706);
			goto handled;
		}

		if (player_said_1(door_to_throne_room)) {
			text_show(10707);
			goto handled;
		}

		if (player_said_1(candlestick)) {
			text_show(10708);
			goto handled;
		}

		if (player_said_1(rug)) {
			text_show(10709);
			goto handled;
		}

		if (player_said_1(desk)) {
			text_show(10711);
			goto handled;
		}

		if (player_said_1(document)) {
			if (inter_point_y <= DOCUMENT_1_Y) {
				text_show(10712);
				text_show(10713);

			} else if (inter_point_x >= DOCUMENT_3_X) {
				text_show(10716);

			} else {
				text_show(10714);
				text_show(10715);
			}
			goto handled;
		}

		if (player_said_1(door_to_guardroom)) {
			text_show(10717);
			goto handled;
		}

		if (player_said_1(fireplace)) {
			text_show(10718);
			goto handled;
		}

		if (player_said_1(fireplace_screen)) {
			text_show(10719);
			goto handled;
		}

		if ((player_said_1(look) || player_said_1(look_at)) &&
			player_said_1(chandelier)) {
			text_show(10721);
			goto handled;
		}

		if (player_said_1(painting)) {
			text_show(10722);
			goto handled;
		}

		if (player_said_1(shieldstone) && object_is_here(shieldstone)) {
			text_show(10723);
			goto handled;
		}

		if (player_said_1(sword) && object_is_here(sword)) {
			text_show(10724);
			goto handled;
		}

		if (player_said_1(decoration)) {
			text_show(10727);
			goto handled;
		}

		if (player_said_1(sconce)) {
			text_show(10728);
			goto handled;
		}
	}

	if (player_said_1(take) &&
		(player_said_1(quill_pen) ||
			player_said_1(ink_bottle) ||
			player_said_1(document))) {
		text_show(10703);
		goto handled;
	}

	if ((player_said_1(push) || player_said_1(pull)) &&
		player_said_1(fireplace_screen)) {
		text_show(10720);
		goto handled;
	}

	if (player_said_2(pull, rug)) {
		text_show(10710);
		goto handled;
	}

	if (player_said_2(take, candlestick)) {
		text_show(10729);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_107_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.temp);
	s.syncAsSint16LE(scratch.qm_frame);
	s.syncAsSint16LE(scratch.qm_action);
	s.syncAsSint16LE(scratch.anim_0_running);
	s.syncAsSint16LE(scratch.q_frame);
	s.syncAsSint16LE(scratch.q_action);
	s.syncAsSint16LE(scratch.q_talk_count);
	s.syncAsSint16LE(scratch.anim_1_running);
	s.syncAsSint16LE(scratch.prevent);
}

void room_107_preload() {
	room_init_code_pointer = room_107_init;
	room_pre_parser_code_pointer = room_107_pre_parser;
	room_parser_code_pointer = room_107_parser;
	room_daemon_code_pointer = room_107_daemon;

	section_1_walker();
	section_1_interface();

	vocab_make_active(words_walk_to);
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
