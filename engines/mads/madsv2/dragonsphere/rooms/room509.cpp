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

#include "mads/madsv2/core/config.h"
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
#include "mads/madsv2/dragonsphere/rooms/room509.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];
	int16 sequence[15];
	int16 animation[4];
	int16 shak_frame;
	int16 shak_action;
	int16 shak_talk_count;
	int16 anim_0_running;
	int16 anim_1_running;
	int16 prevent;
	int32 update_clock;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

#define fx_take                 0
#define fx_water                1

#define ROOM_509_DOOR_CLOSES    60
#define ROOM_509_YOU_TALK       65
#define ROOM_509_ME_TALK        67
#define ROOM_509_FLAP           90

#define PLAYER_X_FROM_508       280
#define PLAYER_Y_FROM_508       145

#define WALK_TO_X_FROM_508      250
#define WALK_TO_Y_FROM_508      138

#define PLAYER_X_FROM_510       -15
#define PLAYER_Y_FROM_510       89

#define WALK_TO_X_FROM_510      12
#define WALK_TO_Y_FROM_510      89

#define CONV_SHAK_29_IN_WAY     29
#define CONV_SHAK_30_ON_LEDGE   30

#define SHAK_SHUT_UP            0
#define SHAK_TALK               1
#define SHAK_FLAP               2

#define WALK_TO_FLAP_X          133
#define WALK_TO_FLAP_Y          112

#define WALK_TO_LEDGE_X         128
#define WALK_TO_LEDGE_Y         109

#define TAKE_MUD_X              260
#define TAKE_MUD_Y              123


static void handle_animation_shak_block() {
	int shak_reset_frame;

	if (kernel_anim[aa[0]].frame != local->shak_frame) {
		local->shak_frame = kernel_anim[aa[0]].frame;
		shak_reset_frame = -1;

		switch (local->shak_frame) {
		case 60:
			player.commands_allowed = true;
			local->prevent          = false;
			local->shak_action      = SHAK_SHUT_UP;
			break;

		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 62:
			switch (local->shak_action) {
			case SHAK_TALK:
				shak_reset_frame = imath_random(3, 5);
				++local->shak_talk_count;
				if (local->shak_talk_count > 25) {
					local->shak_action     = SHAK_SHUT_UP;
					local->shak_talk_count = 0;
					shak_reset_frame       = 0;
				}
				break;

			case SHAK_SHUT_UP:
				if (local->shak_frame >= 4) {
					local->shak_frame = 1;
				}
				++local->shak_talk_count;
				if (local->shak_talk_count > imath_random(20, 30)) {
					if (local->shak_frame == 1) {
						shak_reset_frame = imath_random(0, 1);
					} else if (local->shak_frame == 2) {
						shak_reset_frame = imath_random(0, 2);
					} else if (local->shak_frame == 3) {
						shak_reset_frame = imath_random(1, 2);
					}
					local->shak_talk_count = 0;

				} else {
					shak_reset_frame = local->shak_frame - 1;
				}
				break;

			case SHAK_FLAP:
				shak_reset_frame       = 6;
				local->shak_talk_count = 0;
				break;
			}
			break;
		}

		if (shak_reset_frame >= 0) {
			kernel_reset_animation(aa[0], shak_reset_frame);
			local->shak_frame = shak_reset_frame;
		}
	}
}

static void handle_animation_shak_ledge() {
	int shak_reset_frame;
	int random;

	if (kernel_anim[aa[1]].frame != local->shak_frame) {
		local->shak_frame = kernel_anim[aa[1]].frame;
		shak_reset_frame = -1;

		switch (local->shak_frame) {
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 11:
		case 18:
		case 29:
			switch (local->shak_action) {
			case SHAK_TALK:
				shak_reset_frame = imath_random(0, 3);
				++local->shak_talk_count;
				if (local->shak_talk_count > 30) {
					local->shak_action     = SHAK_SHUT_UP;
					local->shak_talk_count = 0;
					shak_reset_frame       = 4;
				}
				break;

			case SHAK_SHUT_UP:
				++local->shak_talk_count;
				if (local->shak_talk_count > imath_random(20, 30)) {
					random = imath_random(1, 4);
					switch (random) {
					case 1: shak_reset_frame = 4;  break;
					case 2: shak_reset_frame = 5;  break;
					case 3: shak_reset_frame = 18; break;
					case 4: shak_reset_frame = 11; break;
					}
					local->shak_talk_count = 0;

				} else {
					shak_reset_frame = 4;
				}
				break;
			}
			break;

		case 8:
			switch (local->shak_action) {
			case SHAK_TALK:
				shak_reset_frame = 8;
				break;

			case SHAK_SHUT_UP:
				++local->shak_talk_count;
				if (local->shak_talk_count > imath_random(20, 30)) {
					shak_reset_frame       = imath_random(7, 8);
					local->shak_talk_count = 0;

				} else {
					shak_reset_frame = 7;
				}
				break;
			}
			break;

		case 15:
			switch (local->shak_action) {
			case SHAK_TALK:
				shak_reset_frame = 15;
				break;

			case SHAK_SHUT_UP:
				++local->shak_talk_count;
				if (local->shak_talk_count > imath_random(20, 30)) {
					shak_reset_frame       = imath_random(14, 15);
					local->shak_talk_count = 0;

				} else {
					shak_reset_frame = 14;
				}
				break;
			}
			break;

		case 24:
			switch (local->shak_action) {
			case SHAK_TALK:
				shak_reset_frame = 24;
				break;

			case SHAK_SHUT_UP:
				++local->shak_talk_count;
				if (local->shak_talk_count > imath_random(10, 15)) {
					shak_reset_frame = imath_random(23, 24);
					local->shak_talk_count = 0;

				} else {
					shak_reset_frame = 23;
				}
				break;
			}
			break;
		}

		if (shak_reset_frame >= 0) {
			kernel_reset_animation(aa[1], shak_reset_frame);
			local->shak_frame = shak_reset_frame;
		}
	}
}

static void room_509_init() {
	int id;

	kernel.disable_fastwalk = true;

	ss[fx_water]  = kernel_load_series(kernel_name('x', 0), false);
	seq[fx_water] = kernel_seq_forward(ss[fx_water], false, 8, 0, 0, 0);
	kernel_seq_depth(seq[fx_water], 1);
	kernel_seq_range(seq[fx_water], KERNEL_FIRST, KERNEL_LAST);

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
		local->anim_1_running = false;
		local->prevent        = false;
	}

	local->shak_talk_count     = 0;
	global[move_direction_510] = true;

	if (global[player_persona] == PLAYER_IS_KING) {
		conv_get(CONV_SHAK_29_IN_WAY);
		kernel.disable_fastwalk = true;
		aa[0]                   = kernel_run_animation(kernel_name('s', 1), 0);
		local->anim_0_running   = true;
		local->shak_action      = SHAK_SHUT_UP;

		id = kernel_add_dynamic(words_shak, words_walk_to, SYNTAX_MASC_NOT_PROPER, KERNEL_NONE,
		                        0, 0, 0, 0);
		kernel_dynamic_hot[id].prep = PREP_ON;
		kernel_dynamic_walk(id, WALK_TO_FLAP_X, WALK_TO_FLAP_Y, FACING_NORTHWEST);
		kernel_dynamic_anim(id, aa[0], 0);

		if (conv_restore_running == CONV_SHAK_29_IN_WAY) {
			conv_run(CONV_SHAK_29_IN_WAY);
			conv_export_value(global[shak_506_angry]);
		}

	} else {
		conv_get(CONV_SHAK_30_ON_LEDGE);
		aa[1]                   = kernel_run_animation(kernel_name('s', 2), 0);
		local->anim_1_running   = true;
		local->shak_action      = SHAK_SHUT_UP;

		id = kernel_add_dynamic(words_shak, words_walk_to, SYNTAX_MASC_NOT_PROPER, KERNEL_NONE,
		                        0, 0, 0, 0);
		kernel_dynamic_hot[id].prep = PREP_ON;
		kernel_dynamic_walk(id, WALK_TO_LEDGE_X, WALK_TO_LEDGE_Y, FACING_NORTHEAST);
		kernel_dynamic_anim(id, aa[1], 0);
	}

	if (previous_room == 510) {
		player_first_walk(PLAYER_X_FROM_510, PLAYER_Y_FROM_510, FACING_EAST,
		                  WALK_TO_X_FROM_510, WALK_TO_Y_FROM_510, FACING_SOUTHEAST, true);

	} else if ((previous_room == 508) || (previous_room != KERNEL_RESTORING_GAME)) {
		player_first_walk(PLAYER_X_FROM_508, PLAYER_Y_FROM_508, FACING_WEST,
		                  WALK_TO_X_FROM_508, WALK_TO_Y_FROM_508, FACING_NORTHWEST, true);
	}

	local->update_clock = kernel.clock;

	section_5_music();
}

static void room_509_daemon() {
	int dist;

	if (local->anim_0_running) {
		handle_animation_shak_block();
	}

	if (local->anim_1_running) {
		handle_animation_shak_ledge();
	}

	if (global[player_persona] == PLAYER_IS_KING) {

		if ((player.x < 115) && (player.y < 115) && (!local->prevent)) {
			player.commands_allowed = false;
			local->shak_action      = SHAK_FLAP;
			local->prevent          = true;
			player_walk(WALK_TO_FLAP_X, WALK_TO_FLAP_Y, FACING_NORTHWEST);
		}
	}

	if (kernel.clock >= local->update_clock) {
		dist = 127 - ((imath_hypot(player.x - 284, player.y - 152) * 127) / 378);
		if (!sound_off) {
			if (player.x < 38) {
				sound_queue(N_005Waterfall, 42);
			} else {
				sound_queue(N_005Waterfall, dist);
			}
		}
		local->update_clock = kernel.clock + player.frame_delay;
	}
}

static void process_conv_shak_in_way() {
	int you_trig_flag = false;
	int me_trig_flag  = false;

	if (kernel.trigger == ROOM_509_YOU_TALK) {
		local->shak_action = SHAK_TALK;
	}

	if (kernel.trigger == ROOM_509_ME_TALK) {
		local->shak_action = SHAK_SHUT_UP;
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_509_YOU_TALK);
	}

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_509_ME_TALK);
	}

	local->shak_talk_count = 0;
}

static void process_conv_shak_ledge() {
	int you_trig_flag = false;
	int me_trig_flag  = false;

	if (kernel.trigger == ROOM_509_YOU_TALK) {
		local->shak_action = SHAK_TALK;
	}

	if (kernel.trigger == ROOM_509_ME_TALK) {
		local->shak_action = SHAK_SHUT_UP;
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_509_YOU_TALK);
	}

	if (!me_trig_flag) {
		conv_me_trigger(ROOM_509_ME_TALK);
	}

	local->shak_talk_count = 0;
}

static void room_509_pre_parser() {
	if (player_said_1(path_to_east) && player.need_to_walk) {
		if (!player_said_1(walk_down)) {
			player.need_to_walk = false;
		}
	}

	if (player_said_2(walk_down, path_to_west)) {
		player.walk_off_edge_to_room = 510;
	}

	if (player_said_2(take, mud) && !global[has_taken_mud]) {
		if (!player_has(mud)) {
			player_walk(TAKE_MUD_X, TAKE_MUD_Y, FACING_NORTH);
		}
	}
}

static void room_509_parser() {
	if (conv_control.running == CONV_SHAK_29_IN_WAY) {
		process_conv_shak_in_way();
		player.command_ready = false;
		return;
	}

	if (conv_control.running == CONV_SHAK_30_ON_LEDGE) {
		process_conv_shak_ledge();
		player.command_ready = false;
		return;
	}

	if (player_said_2(walk_down, path_to_west)) {
		if (global[player_persona] == PLAYER_IS_KING) {
			player.command_ready = false;
			return;
		}
	}

	if (player_said_2(take, mud) && global[has_taken_mud]) {
		text_show(50924);
		player.command_ready = false;
		return;

	} else if (player_said_2(take, mud) && !player_has(mud)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible   = false;
			aa[2]                   = kernel_run_animation(kernel_name('t', 1), 1);
			kernel_synch(KERNEL_ANIM, aa[2], KERNEL_PLAYER, 0);
			break;

		case 1:
			kernel_abort_animation(aa[2]);
			player.walker_visible   = true;
			player.commands_allowed = true;
			global[has_taken_mud]   = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_NOW, 0);
			++global[player_score];
			sound_play(N_TakeObjectSnd);
			inter_give_to_player(mud);
			object_examine(mud, 50921, 0);
			break;
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(talk_to, shak)) {
		if (global[player_persona] == PLAYER_IS_KING) {
			conv_run(CONV_SHAK_29_IN_WAY);
			conv_export_value(global[shak_506_angry]);
		} else {
			conv_run(CONV_SHAK_30_ON_LEDGE);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_2(walk_down, path_to_east)) {
		new_room = 508;
		player.command_ready = false;
		return;
	}

	if (player.look_around) {
		if (global[player_persona] == PLAYER_IS_KING) {
			text_show(50901);
		} else {
			text_show(50902);
		}
		player.command_ready = false;
		return;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(sky)) {
			text_show(50903);
			player.command_ready = false;
			return;
		}

		if (player_said_1(edge_of_cliff)) {
			text_show(50904);
			player.command_ready = false;
			return;
		}

		if (player_said_1(mountainside)) {
			text_show(50905);
			player.command_ready = false;
			return;
		}

		if (player_said_1(path_to_east)) {
			text_show(50906);
			player.command_ready = false;
			return;
		}

		if (player_said_1(waterfall)) {
			text_show(50907);
			player.command_ready = false;
			return;
		}

		if (player_said_1(ledge)) {
			text_show(50911);
			player.command_ready = false;
			return;
		}

		if (player_said_1(boulders)) {
			text_show(50912);
			player.command_ready = false;
			return;
		}

		if (player_said_1(landing)) {
			if (local->anim_0_running) {
				text_show(50914);
			} else {
				text_show(50913);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(path_to_west)) {
			if (local->anim_0_running) {
				text_show(50915);
			} else {
				text_show(50916);
			}
			player.command_ready = false;
			return;
		}

		if (player_said_1(shak)) {
			text_show(50917);
			player.command_ready = false;
			return;
		}

		if (player_said_1(mud)) {
			if (player.main_object_source == STROKE_INTERFACE) {
				text_show(50920);
				player.command_ready = false;
				return;
			}
		}
	}

	if (player_said_3(fill, goblet, waterfall) || player_said_3(put, goblet, waterfall)) {
		text_show(50908);
		player.command_ready = false;
		return;
	}

	if (player_said_3(throw, dates, shak) || player_said_3(give, dates, shak)) {
		text_show(50918);
		player.command_ready = false;
		return;
	}

	if (player_said_2(open, bottle_of_flies) || player_said_3(give, bottle_of_flies, shak)) {
		text_show(50919);
		player.command_ready = false;
		return;
	}

	if (player_said_3(put, torch, waterfall)) {
		text_show(50908);
		player.command_ready = false;
		return;
	}

	if (player_said_2(put, waterfall)) {
		text_show(50910);
		player.command_ready = false;
		return;
	}

	if (player_said_2(take, mud)) {
		text_show(50921);
		player.command_ready = false;
		return;
	}

	if (player_said_2(make_noise, birdcall)) {
		sound_play(N_BlowBirdCall);
		text_show(50923);
		player.command_ready = false;
		return;
	}

	if (player_said_3(sword, attack, shak) ||
	    player_said_3(sword, carve_up, shak) ||
	    player_said_3(sword, thrust, shak)) {
		if (global[said_use_sword_shak]) {
			text_show(50922);
		} else {
			conv_run(CONV_SHAK_29_IN_WAY);
			conv_export_value(global[shak_506_angry]);
			global[said_use_sword_shak] = true;
		}
		player.command_ready = false;
		return;
	}
}

void room_509_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.shak_frame);
	s.syncAsSint16LE(scratch.shak_action);
	s.syncAsSint16LE(scratch.shak_talk_count);
	s.syncAsSint16LE(scratch.anim_0_running);
	s.syncAsSint16LE(scratch.anim_1_running);
	s.syncAsSint16LE(scratch.prevent);
	s.syncAsSint32LE(scratch.update_clock);
}

void room_509_preload() {
	room_init_code_pointer       = room_509_init;
	room_pre_parser_code_pointer = room_509_pre_parser;
	room_parser_code_pointer     = room_509_parser;
	room_daemon_code_pointer     = room_509_daemon;

	section_5_walker();
	section_5_interface();

	vocab_make_active(words_shak);
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
