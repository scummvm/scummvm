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
#include "mads/madsv2/dragonsphere/rooms/room111.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];       /* Sprite series handles */
	int16 sequence[15];     /* Sequence handles      */
	int16 animation[4];     /* Animation handles     */

	int16 dragon_frame;     /* frame animation is on */

	int16 done_with_conv;   /* T if done with conv   */
	int16 prev_room;
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

static Scratch scratch;


static void room_111_init() {
	local->done_with_conv = false;
	player.walker_visible = false;

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->prev_room = 0;
	}

	if ((previous_room == 106) || (local->prev_room == 106)) {   /* Player comes from throne room */
		local->prev_room = 106;
		sound_play(N_DragonInWater);
		aa[0] = kernel_run_animation(kernel_name('A', global[dragon_my_scene]), 0);
		kernel_reset_animation(aa[0], 29);

	} else if (previous_room != KERNEL_RESTORING_GAME) { /* Player comes from strategic map */
		local->prev_room = 0;
		kernel_init_dialog();
		kernel_set_interface_mode(INTER_LIMITED_SENTENCES);

		conv_get(global[dragon_my_scene] + 15);
		aa[0] = kernel_run_animation(kernel_name('A', global[dragon_my_scene]), 0);
		player.commands_allowed = false;
	}

	section_1_music();
}

static void room_111_daemon() {
	int reset_frame;

	if (kernel_anim[aa[0]].anim != NULL) {
		if (kernel_anim[aa[0]].frame != local->dragon_frame) {
			local->dragon_frame = kernel_anim[aa[0]].frame;
			reset_frame = -1;

			switch (local->dragon_frame) {
			case 38:
				if ((global[dragon_my_scene] > 0) && (local->prev_room != 106)) {
					conv_run(global[dragon_my_scene] + 15);
				}
				break;

			case 150:
				if (local->prev_room == 106) {
					reset_frame = 29;
				} else {
					if (local->done_with_conv) {
						/* reset_frame = 29; */
						/* global[pre_room] = 110; */
						new_room = 120;
					} else {
						switch (global[dragon_my_scene]) {
						case 1: reset_frame = 69; break;
						case 2: reset_frame = 83; break;
						case 3: reset_frame = 39; break;
						case 4: reset_frame = 74; break;
						case 5: reset_frame = 79; break;
						case 6: reset_frame = 89; break;
						case 7: reset_frame = 52; break;
						case 8: reset_frame = 92; break;
						}
					}
				}
				break;
			}

			if (reset_frame >= 0) {
				if (reset_frame != kernel_anim[aa[0]].frame) {
					kernel_reset_animation(aa[0], reset_frame);
					local->dragon_frame = reset_frame;
				}
			}
		}
	}
}

static void room_111_pre_parser() {
}

static void room_111_parser() {
	if (conv_control.running == 16) {
		if (player_verb == conv016_exit_b_b) {
			local->done_with_conv = true;
		}
		goto handled;
	}

	if (conv_control.running == 17) {
		if (player_verb == conv017_exit_b_b) {
			local->done_with_conv = true;
		}
		goto handled;
	}

	if (conv_control.running == 18) {
		if (player_verb == conv018_exit_b_b) {
			local->done_with_conv = true;
		}
		goto handled;
	}

	if (conv_control.running == 19) {
		if (player_verb == conv019_exit_b_b) {
			local->done_with_conv = true;
		}
		goto handled;
	}

	if (conv_control.running == 20) {
		if (player_verb == conv020_exit_b_b) {
			local->done_with_conv = true;
		}
		goto handled;
	}

	if (conv_control.running == 21) {
		if (player_verb == conv021_exit_b_b) {
			local->done_with_conv = true;
		}
		goto handled;
	}

	if (conv_control.running == 22) {
		if (player_verb == conv022_exit_b_b) {
			local->done_with_conv = true;
		}
		goto handled;
	}

	if (conv_control.running == 23) {
		if (player_verb == conv023_exit_b_b) {
			local->done_with_conv = true;
		}
		goto handled;
	}

	if (player_said_2(look_at, throne_room) ||
		player_said_2(look, throne_room)) {
		new_room = 106;
		goto handled;
	}

	if (player.look_around) {
		text_show(11106);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(Dragonsphere)) {
			text_show(11101);
			goto handled;
		}

		if (player_said_1(throne_room)) {
			new_room = 106;
			goto handled;
		}
	}

	if (player_said_2(touch, Dragonsphere)) {
		text_show(11102);
		goto handled;
	}

	if (player_said_2(take, Dragonsphere)) {
		text_show(11103);
		goto handled;
	}

	if (player_said_2(invoke, signet_ring)) {
		global[invoked_from_111] = true;
	}

	if (player_said_3(put, tentacle_parts, Dragonsphere)) {
		text_show(11107);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_111_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.dragon_frame);
	s.syncAsSint16LE(scratch.done_with_conv);
	s.syncAsSint16LE(scratch.prev_room);
}

void room_111_preload() {
	room_init_code_pointer = room_111_init;
	room_pre_parser_code_pointer = room_111_pre_parser;
	room_parser_code_pointer = room_111_parser;
	room_daemon_code_pointer = room_111_daemon;

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
