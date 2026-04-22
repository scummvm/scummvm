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

#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/imath.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/phantom/global.h"
#include "mads/madsv2/phantom/mads/inventory.h"
#include "mads/madsv2/phantom/mads/speeches.h"
#include "mads/madsv2/phantom/mads/words.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/mads/text.h"
#include "mads/madsv2/phantom/rooms/section1.h"
#include "mads/madsv2/phantom/rooms/room114.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

void room_114_init() {
	/* =================== Load Sprite Series ======================= */

	if (global[current_year] == 1993) {
		ss[fx_1993] = kernel_load_series(kernel_name('z', -1), false);
	} else {
		if (object_is_here(rope) && !player_has(rope_with_hook)) {
			ss[fx_rope]   = kernel_load_series(kernel_name('f', 0), false);
			ss[fx_take_9] = kernel_load_series("*RDR_9", false);
		}
	}

	ss[fx_climb] = kernel_load_series(kernel_name('a', 0), false);

	/* ================ If in 1881, put rope ======================== */

	if ((object_is_here(rope)) && (global[current_year] == 1881) && !player_has(rope_with_hook)) {
		seq[fx_rope] = kernel_seq_stamp(ss[fx_rope], false, 1);
		kernel_seq_depth(seq[fx_rope], 2);
	} else {
		kernel_flip_hotspot(words_rope, false);
	}

	/* ======= Load variant if in 1993 & change hotspots =========== */

	if (global[current_year] == 1993) {
		kernel_flip_hotspot(words_cylinder,   false);
		kernel_flip_hotspot(words_mannequins, false);
		kernel_flip_hotspot(words_prop,       false);
		kernel_flip_hotspot(words_bust,       false);
		kernel_flip_hotspot(words_scaffolding, false);
		kernel_draw_to_background(ss[fx_1993], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
	} else {
		kernel_flip_hotspot(words_boxes,      false);
		kernel_flip_hotspot(words_dinette_set, false);
		kernel_flip_hotspot(words_crate,      false);
		kernel_flip_hotspot(words_cases,      false);
	}


	/* ========================= Previous Rooms ===================== */

	if ((previous_room == 105) || (previous_room != KERNEL_RESTORING_GAME)) {
		player.x      = PLAYER_X_FROM_105;
		player.y      = PLAYER_Y_FROM_105;
		player.facing = FACING_WEST;
	}

	section_1_music();
}

void room_114_parser() {
	if (player_said_2(climb_up, circular_staircase)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible   = false;
			aa[0] = kernel_run_animation(kernel_name('u', 1), 1);
			kernel_synch(KERNEL_ANIM, aa[0], KERNEL_PLAYER, 0);
			break;

		case 1:
			new_room = 105;
			break;
		}
		goto handled;
	}

	if (player_said_2(climb_up, circular_staircase)) {
		new_room = 105;
		goto handled;
	}

	if (player_said_2(take, rope) &&
	    (object_is_here(rope) || kernel.trigger)) {
		switch (kernel.trigger) {
		case 0:
			global[player_score]    += 5;
			player.commands_allowed  = false;
			player.walker_visible    = false;
			seq[fx_take_9] = kernel_seq_pingpong(ss[fx_take_9], false,
			                                     6, 0, 0, 2);
			kernel_seq_range(seq[fx_take_9], 1, 3);
			kernel_seq_player(seq[fx_take_9], true);
			kernel_seq_trigger(seq[fx_take_9],
			                   KERNEL_TRIGGER_SPRITE, 3, 1);
			kernel_seq_trigger(seq[fx_take_9],
			                   KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;

		case 1:
			kernel_seq_delete(seq[fx_rope]);
			kernel_flip_hotspot(words_rope, false);
			inter_give_to_player(rope);
			sound_play(N_TakeObjectSnd);
			break;

		case 2:
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_take_9]);
			player.walker_visible = true;
			kernel_timing_trigger(20, 3);
			break;

		case 3:
			object_examine(rope, text_008_07, 0);
			/* You pick up the rope */
			player.commands_allowed = true;
			break;
		}
		goto handled;
	}

	if (player.look_around) {
		text_show(text_114_10);
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(floor)) {
			text_show(text_114_11);
			goto handled;
		}

		if (player_said_1(circular_staircase)) {
			text_show(text_114_12);
			goto handled;
		}

		if (player_said_1(rope) && object_is_here(rope)) {
			text_show(text_114_13);
			goto handled;
		}

		if (player_said_1(wall)) {
			text_show(text_114_14);
			goto handled;
		}

		if (player_said_1(mannequins)) {
			text_show(text_114_15);
			goto handled;
		}

		if (player_said_1(flat)) {
			text_show(text_114_16);
			goto handled;
		}

		if (player_said_1(scaffolding)) {
			text_show(text_114_17);
			goto handled;
		}

		if (player_said_1(mummy_prop)) {
			if (global[current_year] == 1993) {
				text_show(text_114_18);
			} else {
				text_show(text_114_33);
			}
			goto handled;
		}

		if (player_said_1(crates)) {
			if (global[current_year] == 1993) {
				text_show(text_114_19);
			} else {
				text_show(text_114_34);
			}
			goto handled;
		}

		if (player_said_1(crate)) {
			if (global[current_year] == 1993) {
				text_show(text_114_20);
			} else {
				text_show(text_114_35);
			}
			goto handled;
		}

		if (player_said_1(cartons)) {
			if (global[current_year] == 1993) {
				text_show(text_114_21);
			} else {
				text_show(text_114_36);
			}
			goto handled;
		}

		if (player_said_1(carton)) {
			if (global[current_year] == 1993) {
				text_show(text_114_22);
			} else {
				text_show(text_114_37);
			}
			goto handled;
		}

		if (player_said_1(prop)) {
			text_show(text_114_23);
			goto handled;
		}

		if (player_said_1(bust)) {
			text_show(text_114_24);
			goto handled;
		}

		if (player_said_1(mug)) {
			text_show(text_114_25);
			goto handled;
		}

		if ((player_said_1(cases)) || (player_said_1(case))) {
			text_show(text_114_26);
			goto handled;
		}

		if (player_said_1(boxes) || player_said_1(box)) {
			if (global[current_year] == 1993) {
				text_show(text_114_27);
			} else {
				text_show(text_114_39);
			}
			goto handled;
		}

		// NOTE: duplicate box check — unreachable dead code from original
		if (player_said_1(box)) {
			if (global[current_year] == 1881) {
				text_show(text_114_39);
				goto handled;
			}
		}

		if (player_said_1(dinette_set)) {
			text_show(text_114_28);
			goto handled;
		}

		if (player_said_1(cylinder)) {
			text_show(text_114_29);
			goto handled;
		}
	}

	if (player_said_2(open, cartons)) {
		if (global[current_year] == 1993) {
			text_show(text_114_21);
		} else {
			text_show(text_114_36);
		}
		goto handled;
	}

	if (player_said_2(open, carton)) {
		if (global[current_year] == 1993) {
			text_show(text_114_22);
		} else {
			text_show(text_114_37);
		}
		goto handled;
	}

	if (player_said_2(open, crates)) {
		if (global[current_year] == 1993) {
			text_show(text_114_19);
		} else {
			text_show(text_114_34);
		}
		goto handled;
	}

	if (player_said_2(open, crate)) {
		if (global[current_year] == 1993) {
			text_show(text_114_19);
		} else {
			text_show(text_114_35);
		}
		goto handled;
	}

	if (player_said_2(open, cases)) {
		text_show(text_114_26);
		goto handled;
	}

	if (player_said_2(open, boxes) || player_said_2(open, box)) {
		if (global[current_year] == 1993) {
			text_show(text_114_27);
		} else {
			text_show(text_114_39);
		}
		goto handled;
	}

	if (player_said_2(take, mug)) {
		text_show(text_114_30);
		goto handled;
	}

	if (player_said_2(open, mummy_prop)) {
		text_show(text_114_31);
		goto handled;
	}

	if (player_said_2(talk_to, mummy_prop)) {
		text_show(text_114_32);
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_114_preload() {
	room_init_code_pointer       = room_114_init;
	room_pre_parser_code_pointer = NULL;
	room_parser_code_pointer     = room_114_parser;
	room_daemon_code_pointer     = NULL;

	if (global[current_year] == 1993) {
		kernel_initial_variant = 1;
	}

	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
