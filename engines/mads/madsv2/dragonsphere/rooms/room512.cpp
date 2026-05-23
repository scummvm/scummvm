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
#include "mads/madsv2/dragonsphere/rooms/section5.h"
#include "mads/madsv2/dragonsphere/rooms/room512.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];
	int16 sequence[15];
	int16 animation[4];
	int16 pid_frame;
	int16 pid_action;
	int16 anim_0_running;
	int16 shak_frame;
	int16 shak_action;
	int16 shak_talk_count;
	int16 anim_1_running;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define ROOM_512_YOU_TALK      60
#define ROOM_512_ME_TALK       62

#define fx_belt                0

#define PLAYER_X_FROM_510      182
#define PLAYER_Y_FROM_510      93

#define PID_SHUT_UP            0
#define PID_TALK               1
#define PID_LOOK_BELT          2
#define PID_JUMP               3

#define CONV_33_SHAK           33

#define SHAK_SHUT_UP           0
#define SHAK_TALK              1
#define SHAK_FLY               2
#define SHAK_INVISIBLE         3


static void handle_animation_pid() {
	int pid_reset_frame;

	if (kernel_anim[aa[0]].frame != local->pid_frame) {
		local->pid_frame = kernel_anim[aa[0]].frame;
		pid_reset_frame = -1;

		switch (local->pid_frame) {

		case 15:
			sound_play(N_TakeObjectSnd);
			inter_give_to_player(magic_belt);
			object_examine(magic_belt, 51208, 0);
			kernel_seq_delete(seq[fx_belt]);
			++global[player_score];
			break;

		case 101:
			new_room = 505;
			break;

		case 30:
			local->shak_action = SHAK_SHUT_UP;
			break;

		case 56:
		case 49:
		case 38:

			switch (local->pid_action) {

			case PID_TALK:
				pid_reset_frame = 38;
				local->pid_action = PID_SHUT_UP;
				break;

			case PID_LOOK_BELT:
				pid_reset_frame = 49;
				local->pid_action = PID_SHUT_UP;
				break;

			case PID_SHUT_UP:
				pid_reset_frame = 37;
				break;

			case PID_JUMP:
				pid_reset_frame = 56;
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

static void handle_animation_shak() {
	int shak_reset_frame;
	int random;

	if (kernel_anim[aa[1]].frame != local->shak_frame) {
		local->shak_frame = kernel_anim[aa[1]].frame;
		shak_reset_frame = -1;

		switch (local->shak_frame) {

		case 21:
			conv_run(CONV_33_SHAK);
			break;

		case 22:
		case 26:

			switch (local->shak_action) {
			case SHAK_SHUT_UP:
				++local->shak_talk_count;
				if (local->shak_talk_count > imath_random(20, 30)) {
					random = imath_random(1, 5);
					if (random == 1) {
						shak_reset_frame = 22;
					} else if (random == 2) {
						shak_reset_frame = 26;
					} else {
						shak_reset_frame = 21;
					}
					local->shak_talk_count = 0;

				} else {
					shak_reset_frame = 21;
				}
				break;

			case SHAK_TALK:
				shak_reset_frame = 22;
				break;

			case SHAK_FLY:
				shak_reset_frame   = 30;
				local->shak_action = SHAK_INVISIBLE;
				break;
			}
			break;

		case 24:
		case 28:
		case 29:
		case 30:

			switch (local->shak_action) {

			case SHAK_TALK:
				shak_reset_frame = imath_random(27, 29);
				++local->shak_talk_count;
				if (local->shak_talk_count > 30) {
					local->shak_action     = SHAK_SHUT_UP;
					local->shak_talk_count = 0;
					shak_reset_frame       = 23;
				}
				break;

			case SHAK_SHUT_UP:
				++local->shak_talk_count;
				if (local->shak_talk_count > imath_random(20, 30)) {
					random = imath_random(1, 3);
					if (random == 1) {
						shak_reset_frame = 24;
					} else {
						shak_reset_frame = 23;
					}
					local->shak_talk_count = 0;

				} else {
					shak_reset_frame = 23;
				}
				break;

			case SHAK_FLY:
				shak_reset_frame = 24;
				break;
			}
			break;

		case 54:
			local->pid_action = PID_JUMP;
			break;

		case 57:
			if (local->shak_action == SHAK_INVISIBLE) {
				shak_reset_frame = 56;
			} else {
				shak_reset_frame = 0;
			}
			break;
		}

		if (shak_reset_frame >= 0) {
			kernel_reset_animation(aa[1], shak_reset_frame);
			local->shak_frame = shak_reset_frame;
		}
	}
}

static void room_512_init() {
	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
		local->anim_1_running = false;
	}

	if (object_is_here(magic_belt)) {
		conv_get(CONV_33_SHAK);
		ss[fx_belt]  = kernel_load_series(kernel_name('p', 0), false);
		seq[fx_belt] = kernel_seq_stamp(ss[fx_belt], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_belt], 14);

		aa[1]                   = kernel_run_animation(kernel_name('s', 1), 0);
		local->anim_1_running   = true;
		local->shak_action      = SHAK_INVISIBLE;
		kernel_reset_animation(aa[1], 57);

	} else {
		kernel_flip_hotspot(words_belt, false);
	}

	global[move_direction_510] = false;
	player.x                   = PLAYER_X_FROM_510;
	player.y                   = PLAYER_Y_FROM_510;
	player.facing              = FACING_NORTHWEST;

	section_5_music();
}

static void room_512_daemon() {
	if (local->anim_0_running) {
		handle_animation_pid();
	}

	if (local->anim_1_running) {
		handle_animation_shak();
	}
}

static void process_conv_shak() {
	int you_trig_flag = false;
	int me_trig_flag  = false;

	if (player_verb == conv033_exit_b_b) {
		conv_hold();
		local->shak_action = SHAK_FLY;
		you_trig_flag      = true;
		me_trig_flag       = true;
	}

	if (kernel.trigger == ROOM_512_YOU_TALK) {
		local->shak_action = SHAK_TALK;
		local->pid_action  = PID_SHUT_UP;
	}

	if (kernel.trigger == ROOM_512_ME_TALK) {
		if (player_verb == conv033_six_only) {
			local->pid_action  = PID_LOOK_BELT;
		} else {
			local->pid_action  = PID_TALK;
		}
		local->shak_action = SHAK_SHUT_UP;
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_512_YOU_TALK);
	}

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_512_ME_TALK);
	}

	local->shak_talk_count = 0;
}

static void room_512_pre_parser() {
}

static void room_512_parser() {
	if (conv_control.running == CONV_33_SHAK) {
		process_conv_shak();
		player.command_ready = false;
		return;
	}

	if (player_said_2(take, belt)) {
		player.walker_visible   = false;
		player.commands_allowed = false;
		aa[0]                   = kernel_run_animation(kernel_name('p', 1), 0);
		local->anim_0_running   = true;
		local->pid_action       = PID_SHUT_UP;
		kernel_reset_animation(aa[0], 1);
		kernel_synch(KERNEL_ANIM, aa[0], KERNEL_PLAYER, 0);
		player.command_ready = false;
		return;
	}

	if (player_said_2(jump_to, pillar)) {
		if (inter_point_x < 183) {
			player.x = 0;
		} else {
			player.x = 1;
		}
		new_room = 510;
		player.command_ready = false;
		return;
	}

	if (player.look_around) {
		text_show(51201);
		player.command_ready = false;
		return;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(mountainside)) {
			text_show(51202);
			player.command_ready = false;
			return;
		}

		if (player_said_1(nest)) {
			if (object_is_here(magic_belt)) {
				text_show(51204);
				player.command_ready = false;
				return;
			}
		}

		if (player_said_1(pillar)) {
			text_show(51205);
			player.command_ready = false;
			return;
		}

		if (player_said_1(ledge)) {
			text_show(51206);
			player.command_ready = false;
			return;
		}

		if (player_said_1(belt)) {
			if (object_is_here(magic_belt)) {
				text_show(51207);
				player.command_ready = false;
				return;
			}
		}

		if (player_said_1(abyss)) {
			text_show(51203);
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(look_into, abyss)) {
		text_show(51203);
		player.command_ready = false;
		return;
	}

	if (player_said_2(put, nest)) {
		text_show(51209);
		player.command_ready = false;
		return;
	}
}

void room_512_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.pid_frame);
	s.syncAsSint16LE(scratch.pid_action);
	s.syncAsSint16LE(scratch.anim_0_running);
	s.syncAsSint16LE(scratch.shak_frame);
	s.syncAsSint16LE(scratch.shak_action);
	s.syncAsSint16LE(scratch.shak_talk_count);
	s.syncAsSint16LE(scratch.anim_1_running);
}

void room_512_preload() {
	room_init_code_pointer       = room_512_init;
	room_pre_parser_code_pointer = room_512_pre_parser;
	room_parser_code_pointer     = room_512_parser;
	room_daemon_code_pointer     = room_512_daemon;

	section_5_walker();
	section_5_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
