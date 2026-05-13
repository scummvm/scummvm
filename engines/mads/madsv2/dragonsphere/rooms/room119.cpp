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
#include "mads/madsv2/dragonsphere/rooms/room119.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];       /* Sprite series handles */
	int16 sequence[15];     /* Sequence handles      */
	int16 animation[4];     /* Animation handles     */

	int16 pid_frame;       /* animation frame being held for pid stuff */
	int16 pid_action;      /* Type of action to run for pid animation */
	int16 pid_talk_count;  /* counter for pid talking */
	int16 anim_0_running;

	int16 king_frame;       /* animation frame being held for king stuff */
	int16 king_action;      /* Type of action to run for king animation */
	int16 king_talk_count;  /* counter for king talking */
	int16 anim_1_running;

	int16 invoked_ring;
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

static Scratch scratch;


/* ========================= Sprites ========================= */

#define fx_torch_fire          0       /* rm109x1  */
#define fx_door_room           1       /* rm109y0  */
#define fx_door_door           2       /* rm109y1  */
#define fx_button              3       /* rm109x0  */


/* ======================== Triggers ========================= */

#define ROOM_119_BUTTON_ACTION 60
#define ROOM_119_STONE_DOOR    65
#define ROOM_119_WOOD_DOOR     70
#define ROOM_119_TEXT          75
#define ROOM_119_LEAVE_ROOM    79


#define PID_NOTHING            0
#define PID_UP_EXIT            1
#define PID_DOWN_EXIT          2
#define PID_BUTTON             3
#define PID_CROWN              4
#define PID_CONV               5

#define KING_FACE_UP_STAIRS    0
#define KING_OTHER             1

#define CONVERSATION_WITH_KING 35


void room_119_init() {
	local->invoked_ring = false;

	if (kernel.teleported_in) {
		inter_give_to_player(key_crown);
		inter_give_to_player(crystal_ball);
		inter_give_to_player(shifter_ring);
		inter_give_to_player(magic_belt);
		inter_give_to_player(sword);
		inter_give_to_player(amulet);
		global[player_persona] = PLAYER_IS_PID;
	}

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
		local->anim_1_running = false;
	}

	conv_get(CONVERSATION_WITH_KING);

	/* Load sprite series */
	ss[fx_torch_fire] = kernel_load_series(kernel_name('x', 1), false);
	ss[fx_button] = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_door_room] = kernel_load_series(kernel_name('y', 0), false);
	ss[fx_door_door] = kernel_load_series(kernel_name('y', 1), false);

	/* Start continuous sequences */
	seq[fx_torch_fire] = kernel_seq_forward(ss[fx_torch_fire], false, 7, 0, 0, 0);

	aa[0] = kernel_run_animation(kernel_name('p', 1), 0);
	local->anim_0_running = true;
	local->pid_action = PID_NOTHING;

	if (previous_room == KERNEL_RESTORING_GAME || previous_room == 104) {
		kernel_reset_animation(aa[0], 47);
	} else {
		player.commands_allowed = false;
	}

	if (global[king_status] == KING_WITH_SOUL) {
		global[king_is_in_stairwell] = true;
		aa[1] = kernel_run_animation(kernel_name('k', 1), 0);
		local->anim_1_running = true;
		local->king_action = KING_FACE_UP_STAIRS;
		if (previous_room == KERNEL_RESTORING_GAME || previous_room == 104) {
			kernel_reset_animation(aa[1], 8);
		} else {
			player.commands_allowed = false;
		}
	} else {
		kernel_flip_hotspot(words_king, false);
	}

	if (global[books_status] == BOOKS_PULLED ||
		global[books_status] == BOOKS_PULLED2) {
		seq[fx_button] = kernel_seq_stamp(ss[fx_button], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_button], 15);
	}

	if (global[wooden_door_open]) {
		seq[fx_door_door] = kernel_seq_stamp(ss[fx_door_door], false, 6);
		kernel_seq_depth(seq[fx_door_door], 14);
	}

	section_1_music();
}


static void handle_animation_pid() {
	int pid_reset_frame;

	if (kernel_anim[aa[0]].frame != local->pid_frame) {
		local->pid_frame = kernel_anim[aa[0]].frame;
		pid_reset_frame = -1;

		switch (local->pid_frame) {
		case 109:
			/* just pushed button */
			kernel_timing_trigger(1, ROOM_119_BUTTON_ACTION);
			break;

		case 131:
			/* just put on crown */
			if (global[books_status] == BOOKS_PULLED ||
					global[books_status] == BOOKS_PULLED2) {
				sound_play(N_MagicDoorOpens);
				sound_play(N_Angels);
				kernel_timing_trigger(1, ROOM_119_WOOD_DOOR);
			}
			break;

		case 103:  /* end of climb down from top stairs */
		case 170:  /* end of climb down from bottom     */
			new_room = 116;
			break;

		case 181:
			/* end of climb up into 104 */
			global[no_load_walker] = true;
			new_room = 104;
			break;

		case 86:
			/* end of climb down stairs and leave room */
			local->king_action = KING_OTHER;
			break;

		case 97:
			/* almost end of climb down stairs and leave room */
			if (global[king_status] == KING_WITH_SOUL) {
				local->pid_action = PID_CONV;
				conv_run(CONVERSATION_WITH_KING);
				if (global[king_status] == KING_WITH_SOUL) {
					conv_export_value(1);
				} else {
					conv_export_value(0);
				}
				conv_export_value(1);
			}
			break;

		case 98:
			/* almost end of climb down stairs and leave room */
			if (global[king_status] == KING_WITH_SOUL) {
				if (local->pid_action == PID_CONV) {
					pid_reset_frame = 97;
				}
			}
			break;

		case 113: /* almost end of push button */
		case 150: /* almost end of wear crown  */
		case 46:  /* almost end of freeze      */
			player.commands_allowed = true;
			break;

		case 114: /* end of push button */
		case 151: /* end of wear crown  */
		case 47:  /* end of freeze      */
			if (local->pid_frame == 151) {
				if (global[books_status] == BOOKS_PULLED ||
						global[books_status] == BOOKS_PULLED2) {
					local->pid_action = PID_UP_EXIT;
					/* climb up stairs to 104 */
				} else {
					kernel_timing_trigger(2, ROOM_119_TEXT);
				}

			} else if (local->pid_frame == 114) {
				kernel_timing_trigger(2, ROOM_119_TEXT + 1);
			}

			switch (local->pid_action) {

			case PID_NOTHING:
				pid_reset_frame = 46; /* freeze */
				break;

			case PID_UP_EXIT:
				player.commands_allowed = false;
				pid_reset_frame = 172;  /* climb up stairs to 104 */
				break;

			case PID_DOWN_EXIT:
				player.commands_allowed = false;
				pid_reset_frame = 47;  /* climb down stairs to 116 */
				break;

			case PID_BUTTON:
				player.commands_allowed = false;
				pid_reset_frame = 103;  /* push button */
				local->pid_action = PID_NOTHING;
				break;

			case PID_CROWN:
				player.commands_allowed = false;
				pid_reset_frame = 114;  /* wear crown */
				local->pid_action = PID_NOTHING;
				break;
			}
			break;
		}

		if (pid_reset_frame >= 0) {
			kernel_reset_animation(aa[0], pid_reset_frame);
			local->pid_frame = pid_reset_frame;
		}
	}
}

static void handle_animation_king() {
	int king_reset_frame;

	if (kernel_anim[aa[1]].frame != local->king_frame) {
		local->king_frame = kernel_anim[aa[1]].frame;
		king_reset_frame = -1;

		switch (local->king_frame) {

		case 8:
			/* king looking up staircase at Pid */
			if (local->king_action == KING_FACE_UP_STAIRS) {
				king_reset_frame = 7; /* freeze looking up stairs */
			}
			break;

		case 15:
			/* repeat end of king anim */
			king_reset_frame = 14;
			break;
		}

		if (king_reset_frame >= 0) {
			kernel_reset_animation(aa[1], king_reset_frame);
			local->king_frame = king_reset_frame;
		}
	}
}

static void room_119_daemon() {
	int temp;

	if (local->anim_0_running) {
		handle_animation_pid();
	}

	if (local->anim_1_running) {
		handle_animation_king();
	}

	if ((global[books_status] == BOOKS_PULLED) ||
		(global[books_status] == BOOKS_PULLED2)) {

		switch (kernel.trigger) {

		case ROOM_119_WOOD_DOOR:
			player.commands_allowed = false;
			seq[fx_door_room] = kernel_seq_forward(ss[fx_door_room], false, 6, 0, 0, 1);
			kernel_seq_depth(seq[fx_door_room], 14);
			kernel_seq_range(seq[fx_door_room], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_door_room],
				KERNEL_TRIGGER_EXPIRE, 0, ROOM_119_WOOD_DOOR + 1);
			break;

		case ROOM_119_WOOD_DOOR + 1:
			seq[fx_door_room] = kernel_seq_stamp(ss[fx_door_room], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_door_room], 14);
			player.commands_allowed = false;
			break;
		}

	} else if (global[wooden_door_open]) {
		switch (kernel.trigger) {
		case ROOM_119_WOOD_DOOR:
			player.commands_allowed = false;
			kernel_seq_delete(seq[fx_door_door]);
			seq[fx_door_door] = kernel_seq_backward(ss[fx_door_door], false, 6, 0, 0, 1);
			kernel_seq_depth(seq[fx_door_door], 14);
			kernel_seq_range(seq[fx_door_door], 1, 6);
			kernel_seq_trigger(seq[fx_door_door],
				KERNEL_TRIGGER_EXPIRE, 0, ROOM_119_WOOD_DOOR + 1);
			break;

		case ROOM_119_WOOD_DOOR + 1:
			global[wooden_door_open] = false;
			player.commands_allowed = true;
			break;
		}

	} else {
		switch (kernel.trigger) {
		case ROOM_119_WOOD_DOOR:
			player.commands_allowed = false;
			seq[fx_door_door] = kernel_seq_forward(ss[fx_door_door], false, 6, 0, 0, 1);
			kernel_seq_depth(seq[fx_door_door], 14);
			kernel_seq_range(seq[fx_door_door], 1, 6);
			kernel_seq_trigger(seq[fx_door_door],
				KERNEL_TRIGGER_EXPIRE, 0, ROOM_119_WOOD_DOOR + 1);
			break;

		case ROOM_119_WOOD_DOOR + 1:
			seq[fx_door_door] = kernel_seq_stamp(ss[fx_door_door], false, 6);
			kernel_seq_depth(seq[fx_door_door], 14);
			global[wooden_door_open] = true;
			player.commands_allowed = true;
			break;
		}
	}

	switch (kernel.trigger) {
	case ROOM_119_STONE_DOOR:
		sound_play(N_MagicDoorOpens);
		temp = seq[fx_door_door];
		kernel_seq_delete(seq[fx_door_door]);
		player.commands_allowed = false;
		seq[fx_door_door] = kernel_seq_forward(ss[fx_door_door], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_door_door], 14);
		kernel_seq_range(seq[fx_door_door], 7, KERNEL_LAST);
		kernel_synch(KERNEL_SERIES, seq[fx_door_door], KERNEL_SERIES, temp);
		kernel_seq_trigger(seq[fx_door_door],
			KERNEL_TRIGGER_EXPIRE, 0, ROOM_119_STONE_DOOR + 1);
		break;

	case ROOM_119_STONE_DOOR + 1:
		seq[fx_door_door] = kernel_seq_stamp(ss[fx_door_door], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_door_door], 14);
		player.commands_allowed = false;
		kernel_timing_trigger(6, ROOM_119_STONE_DOOR + 2);
		break;

	case ROOM_119_STONE_DOOR + 2:
		global[no_load_walker] = true;
		new_room = 104;
		break;
	}

	if (global[books_status] == BOOKS_PRESENT ||
			global[books_status] == BOOKS_PRESENT2 ||
			global[books_status] == BOOKS_NOT_PRESENT) {
		switch (kernel.trigger) {
		case ROOM_119_BUTTON_ACTION:
			sound_play(N_MagicDoorUnlocked);
			sound_play(N_WallGrinds);
			seq[fx_button] = kernel_seq_forward(ss[fx_button], false, 6, 0, 0, 1);
			kernel_seq_depth(seq[fx_button], 14);
			kernel_seq_range(seq[fx_button], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_button],
				KERNEL_TRIGGER_EXPIRE, 0, ROOM_119_BUTTON_ACTION + 1);
			break;

		case ROOM_119_BUTTON_ACTION + 1:
			seq[fx_button] = kernel_seq_stamp(ss[fx_button], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_button], 15);
			global[books_status] = BOOKS_PULLED2;
			if (global[wooden_door_open]) {
				kernel_timing_trigger(1, ROOM_119_STONE_DOOR);
			}
			break;
		}

	} else {
		switch (kernel.trigger) {
		case ROOM_119_BUTTON_ACTION:
			sound_play(N_MagicDoorUnlocked);
			sound_play(N_WallGrinds);
			kernel_seq_delete(seq[fx_button]);
			seq[fx_button] = kernel_seq_backward(ss[fx_button], false, 6, 0, 0, 1);
			kernel_seq_depth(seq[fx_button], 14);
			kernel_seq_range(seq[fx_button], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_button],
				KERNEL_TRIGGER_EXPIRE, 0, ROOM_119_BUTTON_ACTION + 1);
			break;

		case ROOM_119_BUTTON_ACTION + 1:
			seq[fx_button] = kernel_seq_stamp(ss[fx_button], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_button], 15);
			global[books_status] = BOOKS_PRESENT2;
			break;
		}
	}

	if (kernel.trigger == ROOM_119_TEXT) {
		text_show(11907);
	}

	if (kernel.trigger == ROOM_119_TEXT + 1) {
		text_show(11903);
	}
}

static void process_conversation_king() {
	int you_trig_flag = false;
	int me_trig_flag = false;

	if (player_verb == conv035_exit_b_b) {
		me_trig_flag = true;
		you_trig_flag = true;
		if (!kernel.trigger) {
			conv_me_trigger(ROOM_119_LEAVE_ROOM);
		}
	}

	if (kernel.trigger == ROOM_119_LEAVE_ROOM) {
		if (local->invoked_ring) {
			new_room = 110;
		} else {
			local->pid_action = PID_DOWN_EXIT;
		}
	}
}

static void room_119_pre_parser() {

}

static void room_119_parser() {
	if (conv_control.running == CONVERSATION_WITH_KING) {
		process_conversation_king();
		goto handled;
	}

	if (player_said_2(invoke_power_of, crystal_ball)) {

		if (!global[king_is_in_stairwell]) {
			text_show(11913);

		} else switch (kernel.trigger) {

		case 0:
			sound_play(N_InvokeCrystalBall);
			player.commands_allowed = false;
			seq[fx_door_room] = kernel_seq_forward(ss[fx_door_room], false, 6, 0, 0, 1);
			kernel_seq_depth(seq[fx_door_room], 14);
			kernel_seq_range(seq[fx_door_room], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_door_room],
				KERNEL_TRIGGER_EXPIRE, 0, 1);
			break;

		case 1:
			seq[fx_door_room] = kernel_seq_stamp(ss[fx_door_room], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_door_room], 14);
			player.commands_allowed = false;
			text_show(11911);
			inter_move_object(crystal_ball, NOWHERE);
			text_show(970);
			local->pid_action = PID_UP_EXIT;
			break;
		}
		goto handled;
	}

	if (player_said_2(invoke, signet_ring)) {
		if (global[king_status] == KING_WITH_SOUL) {
			conv_run(CONVERSATION_WITH_KING);
			if (global[king_status] == KING_WITH_SOUL) {
				conv_export_value(1);
			} else {
				conv_export_value(0);
			}
			conv_export_value(1);
			local->invoked_ring = true;
			goto handled;

		} else {
			new_room = 110;
			goto handled;
		}
	}

	if (player.look_around) {
		text_show(11901);
		goto handled;
	}

	if (player_said_2(walk_down, stairs)) {
		local->pid_action = PID_DOWN_EXIT;
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(wall_switch)) {
			text_show(11902);
			goto handled;
		}

		if (player_said_1(stairs)) {
			text_show(11906);
			goto handled;
		}

		if (player_said_1(door)) {
			text_show(11904);
			goto handled;
		}

		if (player_said_1(king)) {
			text_show(11912);
			goto handled;
		}
	}

	if (player_said_1(wall_switch) && (player_said_1(push) || player_said_1(activate))) {
		local->pid_action = PID_BUTTON;

		if (!(global[player_score_flags] & SCORE_PUSH_BUTTON_119)) {
			global[player_score_flags] = global[player_score_flags] | SCORE_PUSH_BUTTON_119;
			global[player_score] += 1;
		}

		goto handled;
	}

	if ((player_said_1(push) || player_said_1(pull) ||
		player_said_1(open) || player_said_1(walk_through)) &&
		player_said_1(door)) {
		text_show(11905);
		goto handled;
	}

	if (player_said_2(wear, key_crown)) {
		if (global[king_status] == KING_WITH_SOUL) {
			local->pid_action = PID_CROWN;

			if (!(global[player_score_flags] & SCORE_WEAR_CROWN_119)) {
				global[player_score_flags] = global[player_score_flags] | SCORE_WEAR_CROWN_119;
				global[player_score] += 3;
			}

		} else {
			text_show(11908);
		}
		goto handled;
	}

	if (player_said_2(talk_to, king)) {
		conv_run(CONVERSATION_WITH_KING);
		if (global[king_status] == KING_WITH_SOUL) {
			conv_export_value(1);
		} else {
			conv_export_value(0);
		}
		conv_export_value(0);
		goto handled;
	}

	if (player_said_2(close, door_to_north)) {
		text_show(42);
		goto handled;
	}

	if (player_said_2(gaze_into, crystal_ball)) {
		text_show(11910);
		goto handled;
	}

	if (player_said_2(pour_contents_of, door)) {
		text_show(11914);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_119_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.pid_frame);
	s.syncAsSint16LE(scratch.pid_action);
	s.syncAsSint16LE(scratch.pid_talk_count);
	s.syncAsSint16LE(scratch.anim_0_running);
	s.syncAsSint16LE(scratch.king_frame);
	s.syncAsSint16LE(scratch.king_action);
	s.syncAsSint16LE(scratch.king_talk_count);
	s.syncAsSint16LE(scratch.anim_1_running);
	s.syncAsSint16LE(scratch.invoked_ring);
}

void room_119_preload() {
	room_init_code_pointer = room_119_init;
	room_pre_parser_code_pointer = room_119_pre_parser;
	room_parser_code_pointer = room_119_parser;
	room_daemon_code_pointer = room_119_daemon;

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
