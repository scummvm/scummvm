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
#include "mads/madsv2/core/camera.h"
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/quote.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/phantom/mads/inventory.h"
#include "mads/madsv2/phantom/mads/sounds.h"
#include "mads/madsv2/phantom/mads/speeches.h"
#include "mads/madsv2/phantom/rooms/section4.h"
#include "mads/madsv2/phantom/rooms/room410.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

static void get_lever_coordinate_and_type(int *x, int *y, int *type, int lever_number, int *noun) {
	/* x will return the x location for the stamping of skull */
	/* y will return the y location for the stamping of skull */
	/* type will return which type the skull is (1-3) */
	/* lever_number (if known - only in init code) will help us find the x,y,type */
	/* if lever_number is not known, noun will be passed a true value.  When this happens
	   we will find out which lever was pushed, and that value will be passed back
	   thru noun */

	if (noun != NULL) {
		switch (player_main_noun) {
		case words_skull_switch_1:  lever_number = 1; break;
		case words_skull_switch_2:  lever_number = 2; break;
		case words_skull_switch_3:  lever_number = 3; break;
		case words_skull_switch_4:  lever_number = 4; break;
		case words_skull_switch_5:  lever_number = 5; break;
		case words_skull_switch_6:  lever_number = 6; break;
		case words_skull_switch_7:  lever_number = 7; break;
		case words_skull_switch_8:  lever_number = 8; break;
		case words_skull_switch_9:  lever_number = 9; break;
		case words_skull_switch_10: lever_number = 10; break;
		case words_skull_switch_11: lever_number = 11; break;
		case words_skull_switch_12: lever_number = 12; break;
		case words_skull_switch_13: lever_number = 13; break;
		case words_skull_switch_14: lever_number = 14; break;
		case words_skull_switch_15: lever_number = 15; break;
		case words_skull_switch_16: lever_number = 16; break;
		case words_skull_switch_17: lever_number = 17; break;
		case words_skull_switch_18: lever_number = 18; break;
		case words_skull_switch_19: lever_number = 19; break;
		case words_skull_switch_20: lever_number = 20; break;
		case words_skull_switch_21: lever_number = 21; break;
		case words_skull_switch_22: lever_number = 22; break;
		case words_skull_switch_23: lever_number = 23; break;
		case words_skull_switch_24: lever_number = 24; break;
		case words_skull_switch_25: lever_number = 25; break;
		case words_skull_switch_26: lever_number = 26; break;
		}
		*noun = lever_number;
	}

	switch (lever_number) {  /* 1 - 26 */
	case 1:  *x = 124; *y = 46;  *type = 3; break;
	case 2:  *x = 143; *y = 46;  *type = 2; break;
	case 3:  *x = 162; *y = 46;  *type = 1; break;
	case 4:  *x = 181; *y = 46;  *type = 3; break;
	case 5:  *x = 200; *y = 46;  *type = 1; break;
	case 6:  *x = 219; *y = 46;  *type = 2; break;
	case 7:  *x = 238; *y = 46;  *type = 1; break;
	case 8:  *x = 133; *y = 71;  *type = 3; break;
	case 9:  *x = 152; *y = 71;  *type = 2; break;
	case 10: *x = 171; *y = 71;  *type = 1; break;
	case 11: *x = 190; *y = 71;  *type = 3; break;
	case 12: *x = 209; *y = 71;  *type = 2; break;
	case 13: *x = 228; *y = 71;  *type = 1; break;
	case 14: *x = 124; *y = 98;  *type = 1; break;
	case 15: *x = 143; *y = 98;  *type = 3; break;
	case 16: *x = 162; *y = 98;  *type = 2; break;
	case 17: *x = 181; *y = 98;  *type = 1; break;
	case 18: *x = 200; *y = 98;  *type = 1; break;
	case 19: *x = 219; *y = 98;  *type = 2; break;
	case 20: *x = 238; *y = 98;  *type = 1; break;
	case 21: *x = 133; *y = 125; *type = 3; break;
	case 22: *x = 152; *y = 125; *type = 1; break;
	case 23: *x = 171; *y = 125; *type = 3; break;
	case 24: *x = 190; *y = 125; *type = 2; break;
	case 25: *x = 209; *y = 125; *type = 1; break;
	case 26: *x = 228; *y = 125; *type = 2; break;
	}
}

void room_410_init() {
	int x;
	int y;
	int type;
	int count;
	int ss_type = 0; /* which skull are we going to stamp */

	player.walker_visible = false;

	/* ===================== Load Sprite Series ================== */

	ss[fx_skull_0] = kernel_load_series(kernel_name('l', 0), false);
	ss[fx_skull_1] = kernel_load_series(kernel_name('l', 1), false);
	ss[fx_skull_2] = kernel_load_series(kernel_name('l', 2), false);
	ss[fx_hand] = kernel_load_series(kernel_name('a', 0), false);
	ss[fx_alphabet] = kernel_load_series(kernel_name('f', 0), false);


	/* ========================= Previous Rooms ================== */


	for (count = 1; count < 27; count++) {
		get_lever_coordinate_and_type(&x, &y, &type, count, NULL);

		switch (type) {
		case 1: ss_type = ss[fx_skull_0]; break;
		case 2: ss_type = ss[fx_skull_1]; break;
		case 3: ss_type = ss[fx_skull_2]; break;
		}

		seq[fx_skull_0] = kernel_seq_stamp(ss_type, false, 1);
		kernel_seq_loc(seq[fx_skull_0], x, y);
		kernel_seq_depth(seq[fx_skull_0], 2);
		skull_seq.num[count - 1] = seq[fx_skull_0];
		/* save the handle for later deletion */
	}

	/* if (previous_room == KERNEL_RESTORING_GAME) { */
	if (global[flicked_1]) {
		get_lever_coordinate_and_type(&x, &y, &type, global[flicked_1], NULL);
		switch (type) {
		case 1: ss_type = ss[fx_skull_0]; break;
		case 2: ss_type = ss[fx_skull_1]; break;
		case 3: ss_type = ss[fx_skull_2]; break;
		}
		kernel_seq_delete(skull_seq.num[global[flicked_1] - 1]);
		seq[fx_skull_0] = kernel_seq_stamp
		(ss_type, false, KERNEL_LAST);
		kernel_seq_loc(seq[fx_skull_0], x, y);
		kernel_seq_depth(seq[fx_skull_0], 2);
	}

	if (global[flicked_2]) {
		get_lever_coordinate_and_type(&x, &y, &type, global[flicked_2], NULL);
		switch (type) {
		case 1: ss_type = ss[fx_skull_0]; break;
		case 2: ss_type = ss[fx_skull_1]; break;
		case 3: ss_type = ss[fx_skull_2]; break;
		}
		kernel_seq_delete(skull_seq.num[global[flicked_2] - 1]);
		seq[fx_skull_0] = kernel_seq_stamp
		(ss_type, false, KERNEL_LAST);
		kernel_seq_loc(seq[fx_skull_0], x, y);
		kernel_seq_depth(seq[fx_skull_0], 2);
	}

	if (global[flicked_3]) {
		get_lever_coordinate_and_type(&x, &y, &type, global[flicked_3], NULL);
		switch (type) {
		case 1: ss_type = ss[fx_skull_0]; break;
		case 2: ss_type = ss[fx_skull_1]; break;
		case 3: ss_type = ss[fx_skull_2]; break;
		}
		kernel_seq_delete(skull_seq.num[global[flicked_3] - 1]);
		seq[fx_skull_0] = kernel_seq_stamp
		(ss_type, false, KERNEL_LAST);
		kernel_seq_loc(seq[fx_skull_0], x, y);
		kernel_seq_depth(seq[fx_skull_0], 2);
	}

	if (global[flicked_4]) {
		get_lever_coordinate_and_type(&x, &y, &type, global[flicked_4], NULL);
		switch (type) {
		case 1: ss_type = ss[fx_skull_0]; break;
		case 2: ss_type = ss[fx_skull_1]; break;
		case 3: ss_type = ss[fx_skull_2]; break;
		}
		kernel_seq_delete(skull_seq.num[global[flicked_4] - 1]);
		seq[fx_skull_0] = kernel_seq_stamp
		(ss_type, false, KERNEL_LAST);
		kernel_seq_loc(seq[fx_skull_0], x, y);
		kernel_seq_depth(seq[fx_skull_0], 2);
	}

	section_4_music();
}

void room_410_parser() {
	int x;
	int y;
	int type;
	int number;

	if (player.look_around) {
		text_show(text_410_13);
		goto handled;
	}

	if (player_said_1(push) || player_said_1(pull)) {
		if (global[door_in_409_is_open]) {
			text_show(text_410_14);
			goto handled;

		} else {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				get_lever_coordinate_and_type(&x, &y, &type, 0, &number);
				seq[fx_hand] = kernel_seq_pingpong(ss[fx_hand], false, 4, 0, 0, 2);
				kernel_seq_depth(seq[fx_hand], 1);
				kernel_seq_range(seq[fx_hand], KERNEL_FIRST, KERNEL_LAST);
				if (y == 46) {
					y = 48; /* If y == 46 (push a top row button), then you will see thru
							   the hand just above the interface (the sprite is not tall enough) */
				}
				kernel_seq_loc(seq[fx_hand], x + 4, y + 107);
				if (!global[door_in_409_is_open]) {
					kernel_seq_trigger(seq[fx_hand],
						KERNEL_TRIGGER_SPRITE, 16, 1);
				}
				kernel_seq_trigger(seq[fx_hand],
					KERNEL_TRIGGER_EXPIRE, 0, 2);
				break;

			case 1:
				get_lever_coordinate_and_type(&x, &y, &type, 0, &number);

				if ((global[flicked_1] != number) && (global[flicked_2] != number) &&
					(global[flicked_3] != number) && (global[flicked_4] != number)) {

					sound_play(N_LeverSnap);

					if (!global[flicked_1]) {
						global[flicked_1] = number;
						if (global[flicked_1] == 5) {
							sound_play(N_LeverBing);
						}

					} else if (!global[flicked_2]) {
						global[flicked_2] = number;
						if ((global[flicked_1] == 5) && (global[flicked_2] == 18)) {
							sound_play(N_LeverBing);
						}

					} else if (!global[flicked_3]) {
						global[flicked_3] = number;
						if ((global[flicked_1] == 5) && (global[flicked_2] == 18) &&
							(global[flicked_3] == 9)) {
							sound_play(N_LeverBing);
						}

					} else if (!global[flicked_4]) {
						global[flicked_4] = number;
						if ((global[flicked_1] == 5) && (global[flicked_2] == 18) &&
							(global[flicked_3] == 9) && (global[flicked_4] == 11)) {
							sound_play(N_LeverBing);
						}
					}

					if (game.difficulty == EASY_MODE) {
						kernel_draw_to_background(ss[fx_alphabet], number, KERNEL_HOME, KERNEL_HOME, 0, 100);
					}

					switch (type) {
					case 1:
						kernel_seq_delete(skull_seq.num[number - 1]);
						seq[fx_skull_0] = kernel_seq_forward(ss[fx_skull_0], false, 4, 0, 0, 1);
						kernel_seq_depth(seq[fx_skull_0], 2);
						kernel_seq_range(seq[fx_skull_0], KERNEL_FIRST, KERNEL_LAST);
						kernel_seq_trigger(seq[fx_skull_0], KERNEL_TRIGGER_EXPIRE, 0, 4);
						kernel_seq_loc(seq[fx_skull_0], x, y);
						break;

					case 2:
						kernel_seq_delete(skull_seq.num[number - 1]);
						seq[fx_skull_1] = kernel_seq_forward(ss[fx_skull_1], false, 4, 0, 0, 1);
						kernel_seq_depth(seq[fx_skull_1], 2);
						kernel_seq_range(seq[fx_skull_1], KERNEL_FIRST, KERNEL_LAST);
						kernel_seq_trigger(seq[fx_skull_1], KERNEL_TRIGGER_EXPIRE, 0, 4);
						kernel_seq_loc(seq[fx_skull_1], x, y);
						break;

					case 3:
						kernel_seq_delete(skull_seq.num[number - 1]);
						seq[fx_skull_2] = kernel_seq_forward(ss[fx_skull_2], false, 4, 0, 0, 1);
						kernel_seq_depth(seq[fx_skull_2], 2);
						kernel_seq_range(seq[fx_skull_2], KERNEL_FIRST, KERNEL_LAST);
						kernel_seq_trigger(seq[fx_skull_2], KERNEL_TRIGGER_EXPIRE, 0, 4);
						kernel_seq_loc(seq[fx_skull_2], x, y);
						break;
					}
				}
				break;

			case 2:
				player.commands_allowed = true;

				if ((global[flicked_1] == 5) && (global[flicked_2] == 18) &&
					(global[flicked_3] == 9) && (global[flicked_4] == 11)) {
					if (!global[door_in_409_is_open]) {
						sound_play(N_DoorGong);
					}
				} /* a 'gong' sound if player was successfull */

				if (global[flicked_1] && global[flicked_2] &&
					global[flicked_3] && global[flicked_4]) {
					new_room = 409;
				}  /* no matter which 4 levers are pulled, goto 409 */
				break;

			case 4:
				get_lever_coordinate_and_type(&x, &y, &type, 0, &number);
				seq[fx_skull_0] = kernel_seq_stamp
				(ss[fx_skull_0], false, KERNEL_LAST);
				kernel_seq_depth(seq[fx_skull_0], 2);
				kernel_seq_loc(seq[fx_skull_0], x, y);
				break;

			case 5:
				get_lever_coordinate_and_type(&x, &y, &type, 0, &number);
				seq[fx_skull_1] = kernel_seq_stamp
				(ss[fx_skull_1], false, KERNEL_LAST);
				kernel_seq_depth(seq[fx_skull_1], 2);
				kernel_seq_loc(seq[fx_skull_1], x, y);
				break;

			case 6:
				get_lever_coordinate_and_type(&x, &y, &type, 0, &number);
				seq[fx_skull_2] = kernel_seq_stamp
				(ss[fx_skull_2], false, KERNEL_LAST);
				kernel_seq_depth(seq[fx_skull_2], 2);
				kernel_seq_loc(seq[fx_skull_2], x, y);
				break;
			}
		}
		goto handled;
	}

	if (player_said_1(look) || player_said_1(look_at)) {

		if (player_said_1(wall)) {
			text_show(text_410_11);
			goto handled;
		}

		if (player_said_1(switch_panel)) {
			text_show(text_410_11);
			goto handled;
		}

		if (player_said_1(catacomb_room)) {
			text_show(text_410_15);
			goto handled;
		}

		switch (player_main_noun) {
		case words_skull_switch_1:
		case words_skull_switch_2:
		case words_skull_switch_3:
		case words_skull_switch_4:
		case words_skull_switch_5:
		case words_skull_switch_6:
		case words_skull_switch_7:
		case words_skull_switch_8:
		case words_skull_switch_9:
		case words_skull_switch_10:
		case words_skull_switch_11:
		case words_skull_switch_12:
		case words_skull_switch_13:
		case words_skull_switch_14:
		case words_skull_switch_15:
		case words_skull_switch_16:
		case words_skull_switch_17:
		case words_skull_switch_18:
		case words_skull_switch_19:
		case words_skull_switch_20:
		case words_skull_switch_21:
		case words_skull_switch_22:
		case words_skull_switch_23:
		case words_skull_switch_24:
		case words_skull_switch_25:
		case words_skull_switch_26:
			text_show(text_410_12);
			goto handled;
			break;
		}
	}

	if (player_said_2(exit_to, catacomb_room)) {
		new_room = 409;
		goto handled;
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_410_preload() {
	room_init_code_pointer = room_410_init;
	room_pre_parser_code_pointer = NULL;
	room_parser_code_pointer = room_410_parser;
	room_daemon_code_pointer = NULL;

	section_4_walker();
	section_4_interface();
}

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS
