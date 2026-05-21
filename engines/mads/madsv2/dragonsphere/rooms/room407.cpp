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
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section4.h"
#include "mads/madsv2/dragonsphere/rooms/room407.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];       /* Sprite series handles */
	int16 sequence[15];     /* Sequence handles      */
	int16 animation[5];     /* Animation handles     */

	char his_score[3];
	char my_score[3];

	int8 his_score_int;
	int8 my_score_int;

	int16 his_score_id;
	int16 my_score_id;

	int16 highlighting;
	int16 prevent;
	int16 final_choice;

	int16 number_of_spins;
	int16 last_spin;
	int16 dealt;

	int16 num_of_red;
	int16 num_of_yellow;
	int16 num_of_green;
	int16 num_of_purple;

	int16 whos_turn;
	int16 var_to_conv;

	byte stuff_to_give[5];
	int16 will_give;
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation


/* ========================= Sprite Series =================== */

#define fx_half_bottle          0       /* rm407p0  */
#define fx_red_abs              1       /* rm411x0  */
#define fx_yellow_abs           2       /* rm411x1  */
#define fx_green_abs            3       /* rm411x2  */
#define fx_purple_abs           4       /* rm411x3  */

#define fx_red_sel              5       /* rm411g0  */
#define fx_yellow_sel           6       /* rm411g1  */
#define fx_green_sel            7       /* rm411g2  */
#define fx_purple_sel           8       /* rm411g3  */

#define fx_face                 9       /* facepid  */
#define fx_face2                10      /* facepid  */


/* ========================= Triggers ======================== */

#define ROOM_407_RUN_CONV       60
#define ROOM_407_SPIN           70
#define ROOM_407_ME_TALK        75
#define ROOM_407_YOU_TALK       77
#define ROOM_407_CAL_PICK       79

/* ========================== Other ========================== */

#define CONV_50_GAME            50

#define HIS_TURN                0
#define MY_TURN                 1

#define NOT_THERE               0
#define CAN_GIVE                1
#define HOSIED                  2

static Scratch scratch;


static void update_scores() {
	Common::strcpy_s(local->his_score, "\0");

	kernel_message_purge();

	switch (local->his_score_int) {
	case 0: Common::strcpy_s(local->his_score, "0\0"); break;
	case 1: Common::strcpy_s(local->his_score, "1\0"); break;
	case 2: Common::strcpy_s(local->his_score, "2\0"); break;
	case 3: Common::strcpy_s(local->his_score, "3\0"); break;
	case 4: Common::strcpy_s(local->his_score, "4\0"); break;
	case 5: Common::strcpy_s(local->his_score, "5\0"); break;
	case 6: Common::strcpy_s(local->his_score, "6\0"); break;
	case 7: Common::strcpy_s(local->his_score, "7\0"); break;
	case 8: Common::strcpy_s(local->his_score, "8\0"); break;
	case 9: Common::strcpy_s(local->his_score, "9\0"); break;
	case 10: Common::strcpy_s(local->his_score, "10\0"); break;
	case 11: Common::strcpy_s(local->his_score, "11\0"); break;
	case 12: Common::strcpy_s(local->his_score, "12\0"); break;
	case 13: Common::strcpy_s(local->his_score, "13\0"); break;
	case 14: Common::strcpy_s(local->his_score, "14\0"); break;
	case 15: Common::strcpy_s(local->his_score, "15\0"); break;
	case 16: Common::strcpy_s(local->his_score, "16\0"); break;
	case 17: Common::strcpy_s(local->his_score, "17\0"); break;
	case 18: Common::strcpy_s(local->his_score, "18\0"); break;
	case 19: Common::strcpy_s(local->his_score, "19\0"); break;
	case 20: Common::strcpy_s(local->his_score, "20\0"); break;
	case 21: Common::strcpy_s(local->his_score, "21\0"); break;
	case 22: Common::strcpy_s(local->his_score, "22\0"); break;
	case 23: Common::strcpy_s(local->his_score, "23\0"); break;
	case 24: Common::strcpy_s(local->his_score, "24\0"); break;
	case 25: Common::strcpy_s(local->his_score, "25\0"); break;
	case 26: Common::strcpy_s(local->his_score, "26\0"); break;
	case 27: Common::strcpy_s(local->his_score, "27\0"); break;
	}

	switch (local->my_score_int) {
	case 0: Common::strcpy_s(local->my_score, "0\0"); break;
	case 1: Common::strcpy_s(local->my_score, "1\0"); break;
	case 2: Common::strcpy_s(local->my_score, "2\0"); break;
	case 3: Common::strcpy_s(local->my_score, "3\0"); break;
	case 4: Common::strcpy_s(local->my_score, "4\0"); break;
	case 5: Common::strcpy_s(local->my_score, "5\0"); break;
	case 6: Common::strcpy_s(local->my_score, "6\0"); break;
	case 7: Common::strcpy_s(local->my_score, "7\0"); break;
	case 8: Common::strcpy_s(local->my_score, "8\0"); break;
	case 9: Common::strcpy_s(local->my_score, "9\0"); break;
	case 10: Common::strcpy_s(local->my_score, "10\0"); break;
	case 11: Common::strcpy_s(local->my_score, "11\0"); break;
	case 12: Common::strcpy_s(local->my_score, "12\0"); break;
	case 13: Common::strcpy_s(local->my_score, "13\0"); break;
	case 14: Common::strcpy_s(local->my_score, "14\0"); break;
	case 15: Common::strcpy_s(local->my_score, "15\0"); break;
	case 16: Common::strcpy_s(local->my_score, "16\0"); break;
	case 17: Common::strcpy_s(local->my_score, "17\0"); break;
	case 18: Common::strcpy_s(local->my_score, "18\0"); break;
	case 19: Common::strcpy_s(local->my_score, "19\0"); break;
	case 20: Common::strcpy_s(local->my_score, "20\0"); break;
	case 21: Common::strcpy_s(local->my_score, "21\0"); break;
	case 22: Common::strcpy_s(local->my_score, "22\0"); break;
	case 23: Common::strcpy_s(local->my_score, "23\0"); break;
	case 24: Common::strcpy_s(local->my_score, "24\0"); break;
	case 25: Common::strcpy_s(local->my_score, "25\0"); break;
	case 26: Common::strcpy_s(local->my_score, "26\0"); break;
	case 27: Common::strcpy_s(local->my_score, "27\0"); break;
	}

	local->his_score_id = kernel_message_add(local->his_score,
		59, 51, MESSAGE_COLOR, 999999, 0, KERNEL_MESSAGE_CENTER);

	local->my_score_id = kernel_message_add(local->my_score,
		260, 85, MESSAGE_COLOR, 999999, 0, KERNEL_MESSAGE_CENTER);
}

static void room_407_init() {
	kernel_init_dialog();
	kernel_set_interface_mode(INTER_LIMITED_SENTENCES);

	if (previous_room != KERNEL_RESTORING_GAME) {
		if (object[dates].location == 406)            local->stuff_to_give[0] = CAN_GIVE; else local->stuff_to_give[0] = NOT_THERE;
		if (object[statue].location == 406)           local->stuff_to_give[1] = CAN_GIVE; else local->stuff_to_give[1] = NOT_THERE;
		if (object[ruby_ring].location == 406)        local->stuff_to_give[2] = CAN_GIVE; else local->stuff_to_give[2] = NOT_THERE;
		if (object[bottle_of_flies].location == 406)  local->stuff_to_give[3] = CAN_GIVE; else local->stuff_to_give[3] = NOT_THERE;
		if (object[soptus_soporific].location == 406) local->stuff_to_give[4] = CAN_GIVE; else local->stuff_to_give[4] = NOT_THERE;

		local->var_to_conv = 0;
		local->his_score_int = 0;
		local->my_score_int = 0;
		local->number_of_spins = 0;
		local->whos_turn = MY_TURN;
		local->num_of_red = 12;
		local->num_of_yellow = 12;
		local->num_of_green = 12;
		local->num_of_purple = 12;
		local->highlighting = -1;
		local->final_choice = -1;
		local->last_spin = -1;
		local->his_score_int = 0;
		local->my_score_int = 0;
		local->will_give = true;
	}

	if (global[player_persona] == PLAYER_IS_PID) {
		ss[fx_face] = kernel_load_series("*FACEPID", false);
		seq[fx_face] = kernel_seq_stamp(ss[fx_face], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_face], 1);
		kernel_seq_loc(seq[fx_face], 260, 79);

	} else {
		ss[fx_face] = kernel_load_series("*FACEKING", false);
		seq[fx_face] = kernel_seq_stamp(ss[fx_face], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_face], 1);
		kernel_seq_loc(seq[fx_face], 260, 79);
	}

	ss[fx_face2] = kernel_load_series("*FACECAL", false);
	seq[fx_face2] = kernel_seq_stamp(ss[fx_face2], false, KERNEL_FIRST);
	kernel_seq_depth(seq[fx_face2], 1);
	kernel_seq_loc(seq[fx_face2], 59, 45);

	player.walker_visible = false;

	local->prevent = false;

	conv_get(CONV_50_GAME);

	ss[fx_red_abs] = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_yellow_abs] = kernel_load_series(kernel_name('x', 1), false);
	ss[fx_green_abs] = kernel_load_series(kernel_name('x', 2), false);
	ss[fx_purple_abs] = kernel_load_series(kernel_name('x', 3), false);
	ss[fx_red_sel] = kernel_load_series(kernel_name('g', 0), false);
	ss[fx_yellow_sel] = kernel_load_series(kernel_name('g', 1), false);
	ss[fx_green_sel] = kernel_load_series(kernel_name('g', 2), false);
	ss[fx_purple_sel] = kernel_load_series(kernel_name('g', 3), false);


	update_scores();

	if (previous_room == KERNEL_RESTORING_GAME) {
		switch (local->last_spin) {
		case 1:
			seq[fx_red_abs] = kernel_seq_stamp(ss[fx_red_abs], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_red_abs], 1);
			kernel_seq_loc(seq[fx_red_abs], 159, 134);
			break;

		case 2:
			seq[fx_yellow_abs] = kernel_seq_stamp(ss[fx_yellow_abs], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_yellow_abs], 1);
			kernel_seq_loc(seq[fx_yellow_abs], 159, 134);
			break;

		case 3:
			seq[fx_green_abs] = kernel_seq_stamp(ss[fx_green_abs], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_green_abs], 1);
			kernel_seq_loc(seq[fx_green_abs], 159, 134);
			break;

		case 4:
			seq[fx_purple_abs] = kernel_seq_stamp(ss[fx_purple_abs], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_purple_abs], 1);
			kernel_seq_loc(seq[fx_purple_abs], 159, 134);
			break;
		}

		switch (local->final_choice) {
		case words_red_stone:
			seq[fx_red_sel] = kernel_seq_stamp(ss[fx_red_sel], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_red_sel], 1);
			local->highlighting = words_red_stone;
			break;

		case words_yellow_stone:
			seq[fx_yellow_sel] = kernel_seq_stamp(ss[fx_yellow_sel], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_yellow_sel], 1);
			local->highlighting = words_yellow_stone;
			break;

		case words_green_stone:
			seq[fx_green_sel] = kernel_seq_stamp(ss[fx_green_sel], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_green_sel], 1);
			local->highlighting = words_green_stone;
			break;

		case words_purple_stone:
			seq[fx_purple_sel] = kernel_seq_stamp(ss[fx_purple_sel], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_purple_sel], 1);
			local->highlighting = words_purple_stone;
			break;
		}
	}

	if (previous_room != KERNEL_RESTORING_GAME ||
		conv_restore_running == CONV_50_GAME) {
		conv_run(CONV_50_GAME);
		conv_export_pointer(&global[wins_in_desert]);
		conv_export_value(global[wins_till_prize]);
		conv_export_value(local->will_give);
		conv_export_value(local->var_to_conv);
	}


	section_4_music();
}

static void do_a_round() {
	int new_spin;
	int sum;

	++local->number_of_spins;

	if (local->number_of_spins > 1) switch (local->last_spin) {
	case 1: kernel_seq_delete(seq[fx_red_abs]);    break;
	case 2: kernel_seq_delete(seq[fx_yellow_abs]); break;
	case 3: kernel_seq_delete(seq[fx_green_abs]);  break;
	case 4: kernel_seq_delete(seq[fx_purple_abs]); break;
	}

	do
		new_spin = imath_random(1, 4);
	while (local->last_spin == new_spin);

	local->last_spin = new_spin;

	if (local->number_of_spins == 10) {
		sum = local->num_of_red + local->num_of_yellow + local->num_of_green + local->num_of_purple;
		local->last_spin = imath_random(1, sum);


		if (local->last_spin <= (0 + local->num_of_red)) {
			local->last_spin = 1;
			--local->num_of_red;

		} else if (local->last_spin <= (local->num_of_red + local->num_of_yellow)) {
			local->last_spin = 2;
			--local->num_of_yellow;

		} else if (local->last_spin <= (local->num_of_red + local->num_of_yellow + local->num_of_green)) {
			local->last_spin = 3;
			--local->num_of_green;

		} else {
			local->last_spin = 4;
			--local->num_of_purple;
		}

		kernel_timing_trigger(6, ROOM_407_RUN_CONV);

	} else {
		kernel_timing_trigger(6, ROOM_407_SPIN);
	}

	sound_play(N_GamePieceSnd);

	switch (local->last_spin) {
	case 1:
		seq[fx_red_abs] = kernel_seq_stamp(ss[fx_red_abs], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_red_abs], 1);
		kernel_seq_loc(seq[fx_red_abs], 159, 134);
		break;

	case 2:
		seq[fx_yellow_abs] = kernel_seq_stamp(ss[fx_yellow_abs], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_yellow_abs], 1);
		kernel_seq_loc(seq[fx_yellow_abs], 159, 134);
		break;

	case 3:
		seq[fx_green_abs] = kernel_seq_stamp(ss[fx_green_abs], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_green_abs], 1);
		kernel_seq_loc(seq[fx_green_abs], 159, 134);
		break;

	case 4:
		seq[fx_purple_abs] = kernel_seq_stamp(ss[fx_purple_abs], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_purple_abs], 1);
		kernel_seq_loc(seq[fx_purple_abs], 159, 134);
		break;
	}
}

static void after_round_me() {
	switch (local->final_choice) {

	case words_red_stone:
		switch (local->last_spin) {
		case 4:
			local->whos_turn = HIS_TURN;
			local->var_to_conv = 3;
			break;

		default:
			++local->my_score_int;
			local->var_to_conv = 2;
			break;
		}
		break;

	case words_yellow_stone:
		switch (local->last_spin) {
		case 2:
		case 3:
			local->my_score_int += 2;
			local->var_to_conv = 2;
			break;

		default:
			local->whos_turn = HIS_TURN;
			local->var_to_conv = 3;
			break;
		}
		break;

	case words_green_stone:
		switch (local->last_spin) {
		case 3:
			local->my_score_int += 5;
			local->var_to_conv = 2;
			break;

		default:
			local->whos_turn = HIS_TURN;
			local->var_to_conv = 3;
			break;
		}
		break;

	case words_purple_stone:
		switch (local->last_spin) {
		case 4:
			local->my_score_int += 12;
			local->var_to_conv = 2;
			break;

		default:
			local->his_score_int += 2;
			if (local->his_score_int > 15) local->his_score_int = 15;
			local->whos_turn = HIS_TURN;
			local->var_to_conv = 3;
			break;
		}
		break;
	}
}

static void after_round_him() {
	switch (local->final_choice) {

	case words_red_stone:
		switch (local->last_spin) {
		case 4:
			local->whos_turn = MY_TURN;
			local->var_to_conv = 5;
			break;

		default:
			++local->his_score_int;
			local->var_to_conv = 4;
			break;
		}
		break;

	case words_yellow_stone:
		switch (local->last_spin) {
		case 2:
		case 3:
			local->his_score_int += 2;
			local->var_to_conv = 4;
			break;

		default:
			local->whos_turn = MY_TURN;
			local->var_to_conv = 5;
			break;
		}
		break;

	case words_green_stone:
		switch (local->last_spin) {
		case 3:
			local->his_score_int += 5;
			local->var_to_conv = 4;
			break;

		default:
			local->whos_turn = MY_TURN;
			local->var_to_conv = 5;
			break;
		}
		break;

	case words_purple_stone:
		switch (local->last_spin) {
		case 4:
			local->his_score_int += 12;
			local->var_to_conv = 4;
			break;

		default:
			local->my_score_int += 2;
			if (local->my_score_int > 15) local->my_score_int = 15;
			local->whos_turn = MY_TURN;
			local->var_to_conv = 5;
			break;
		}
		break;
	}
}

static void room_407_daemon() {
	int count;
	int currently_on = -1;
	int button_down = false;

	if (((mouse_status & 1) || (mouse_status & 2)) &&
		conv_control.running != CONV_50_GAME && player.commands_allowed) {
		button_down = true;
	}

	for (count = 0; count < room_num_spots; count++) {
		if (room_spots[count].active) {
			if (mouse_x >= room_spots[count].ul_x &&
				mouse_x <= room_spots[count].lr_x &&
				mouse_y >= room_spots[count].ul_y &&
				mouse_y <= room_spots[count].lr_y) {

				switch (room_spots[count].vocab) {
				case words_red_stone:
					currently_on = words_red_stone;
					break;

				case words_yellow_stone:
					currently_on = words_yellow_stone;
					break;

				case words_green_stone:
					currently_on = words_green_stone;
					break;

				case words_purple_stone:
					currently_on = words_purple_stone;
					break;
				}
			}
		}
	}

	if (button_down && local->highlighting == -1) {
		if (currently_on == words_red_stone) {
			seq[fx_red_sel] = kernel_seq_stamp(ss[fx_red_sel], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_red_sel], 1);
			local->highlighting = words_red_stone;

		} else if (currently_on == words_yellow_stone) {
			seq[fx_yellow_sel] = kernel_seq_stamp(ss[fx_yellow_sel], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_yellow_sel], 1);
			local->highlighting = words_yellow_stone;

		} else if (currently_on == words_green_stone) {
			seq[fx_green_sel] = kernel_seq_stamp(ss[fx_green_sel], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_green_sel], 1);
			local->highlighting = words_green_stone;

		} else if (currently_on == words_purple_stone) {
			seq[fx_purple_sel] = kernel_seq_stamp(ss[fx_purple_sel], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_purple_sel], 1);
			local->highlighting = words_purple_stone;
		}
	}

	if ((local->highlighting != -1 && (currently_on != local->highlighting)) ||
		(!button_down && local->highlighting != -1)) {

		if (!button_down && local->highlighting != -1) {
			local->final_choice = local->highlighting;
			player.commands_allowed = false;
		}

		if (local->final_choice == -1) switch (local->highlighting) {
		case words_red_stone:
			kernel_seq_delete(seq[fx_red_sel]);
			break;

		case words_yellow_stone:
			kernel_seq_delete(seq[fx_yellow_sel]);
			break;

		case words_green_stone:
			kernel_seq_delete(seq[fx_green_sel]);
			break;

		case words_purple_stone:
			kernel_seq_delete(seq[fx_purple_sel]);
			break;
		}
		local->highlighting = -1;
	}


	if ((local->final_choice != -1 && local->number_of_spins == 0) ||
		kernel.trigger == ROOM_407_SPIN) {
		do_a_round();
	}

	if (kernel.trigger == ROOM_407_RUN_CONV) {
		if (local->whos_turn == HIS_TURN) {
			after_round_him();
		} else {
			after_round_me();
		}

		if (local->whos_turn == MY_TURN) {
			player.commands_allowed = true;
		} else {
			player.commands_allowed = false;
		}

		if (local->his_score_int >= 16) {
			local->var_to_conv = 7;
			player.commands_allowed = true;
		}

		if (local->my_score_int >= 16) {
			local->var_to_conv = 6;
			++global[wins_in_desert];
		}

		update_scores();


		if (global[wins_in_desert] == global[wins_till_prize]) {

			if (global[player_persona] == PLAYER_IS_KING) {
				if (local->stuff_to_give[0] == CAN_GIVE) {
					++global[prizes_owed_to_player];

				} else if (local->stuff_to_give[1] == CAN_GIVE) {
					++global[prizes_owed_to_player];

				} else if (local->stuff_to_give[2] == CAN_GIVE) {
					++global[prizes_owed_to_player];
				}

			} else {
				if (local->stuff_to_give[0] == CAN_GIVE) {
					++global[prizes_owed_to_player];

				} else if (local->stuff_to_give[1] == CAN_GIVE) {
					++global[prizes_owed_to_player];

				} else if (local->stuff_to_give[2] == CAN_GIVE) {
					++global[prizes_owed_to_player];

				} else if (local->stuff_to_give[3] == CAN_GIVE) {
					++global[prizes_owed_to_player];

				} else if (local->stuff_to_give[4] == CAN_GIVE) {
					++global[prizes_owed_to_player];
				}
			}

			if (global[player_persona] == PLAYER_IS_KING) {
				if (local->stuff_to_give[0] != CAN_GIVE &&
					local->stuff_to_give[1] != CAN_GIVE &&
					local->stuff_to_give[2] != CAN_GIVE) {
					local->will_give = false;
				}

				if (local->stuff_to_give[0] == CAN_GIVE) {
					local->stuff_to_give[0] = HOSIED;

				} else if (local->stuff_to_give[1] == CAN_GIVE) {
					local->stuff_to_give[1] = HOSIED;

				} else if (local->stuff_to_give[2] == CAN_GIVE) {
					local->stuff_to_give[2] = HOSIED;
				}

			} else {
				if (local->stuff_to_give[0] != CAN_GIVE &&
					local->stuff_to_give[1] != CAN_GIVE &&
					local->stuff_to_give[2] != CAN_GIVE &&
					local->stuff_to_give[3] != CAN_GIVE &&
					local->stuff_to_give[4] != CAN_GIVE) {
					local->will_give = false;
				}

				if (local->stuff_to_give[0] == CAN_GIVE) {
					local->stuff_to_give[0] = HOSIED;

				} else if (local->stuff_to_give[1] == CAN_GIVE) {
					local->stuff_to_give[1] = HOSIED;

				} else if (local->stuff_to_give[2] == CAN_GIVE) {
					local->stuff_to_give[2] = HOSIED;

				} else if (local->stuff_to_give[3] == CAN_GIVE) {
					local->stuff_to_give[3] = HOSIED;

				} else if (local->stuff_to_give[4] == CAN_GIVE) {
					local->stuff_to_give[4] = HOSIED;
				}
			}
		}

		conv_run(CONV_50_GAME);
		conv_export_pointer(&global[wins_in_desert]);
		conv_export_value(global[wins_till_prize]);
		conv_export_value(local->will_give);
		conv_export_value(local->var_to_conv);
	}
}

static void process_conv_game() {
	if (player_verb == conv050_answers_yes ||
		player_verb == conv050_answers_rulesask) {
		local->his_score_int = 0;
		local->my_score_int = 0;
		local->num_of_red = 12;
		local->num_of_yellow = 12;
		local->num_of_green = 12;
		local->num_of_purple = 12;
		local->whos_turn = MY_TURN;
	}

	if (player_verb == conv050_exit_b_b) {
		*conv_my_next_start = conv050_branch;
		conv_abort();
		update_scores();

		switch (local->final_choice) {
		case words_red_stone:    kernel_seq_delete(seq[fx_red_sel]);    break;
		case words_yellow_stone: kernel_seq_delete(seq[fx_yellow_sel]); break;
		case words_green_stone:  kernel_seq_delete(seq[fx_green_sel]);  break;
		case words_purple_stone: kernel_seq_delete(seq[fx_purple_sel]); break;
		}

		switch (local->last_spin) {
		case 1: kernel_seq_delete(seq[fx_red_abs]);    break;
		case 2: kernel_seq_delete(seq[fx_yellow_abs]); break;
		case 3: kernel_seq_delete(seq[fx_green_abs]);  break;
		case 4: kernel_seq_delete(seq[fx_purple_abs]); break;
		}

		if (local->whos_turn == MY_TURN) {
			local->final_choice = -1;
			local->highlighting = -1;
			local->last_spin = -1;
			local->number_of_spins = 0;

		} else {
			kernel_timing_trigger(ONE_SECOND, ROOM_407_CAL_PICK);
		}
	}

	if (player_verb == conv050_exit_d_d) {
		*conv_my_next_start = conv050_branch;
		conv_abort();
		new_room = 406;
	}
}

static void room_407_pre_parser() {
	player.need_to_walk = false;

	if (player_said_1(select)) {
		player_cancel_command();
	}
}

static void room_407_parser() {
	if (conv_control.running == CONV_50_GAME) {
		process_conv_game();
		goto handled;
	}

	if (kernel.trigger == ROOM_407_CAL_PICK) {
		local->highlighting = imath_random(1, 4);

		kernel_timing_trigger(ONE_SECOND, ROOM_407_CAL_PICK + 1);

		switch (local->highlighting) {
		case 1:
			seq[fx_red_sel] = kernel_seq_stamp(ss[fx_red_sel], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_red_sel], 1);
			local->highlighting = words_red_stone;
			local->final_choice = words_red_stone;
			break;

		case 2:
			seq[fx_yellow_sel] = kernel_seq_stamp(ss[fx_yellow_sel], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_yellow_sel], 1);
			local->highlighting = words_yellow_stone;
			local->final_choice = words_yellow_stone;
			break;

		case 3:
			seq[fx_green_sel] = kernel_seq_stamp(ss[fx_green_sel], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_green_sel], 1);
			local->highlighting = words_green_stone;
			local->final_choice = words_green_stone;
			break;

		case 4:
			seq[fx_purple_sel] = kernel_seq_stamp(ss[fx_purple_sel], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_purple_sel], 1);
			local->highlighting = words_purple_stone;
			local->final_choice = words_purple_stone;
			break;
		}
		goto handled;
	}


	if (kernel.trigger == ROOM_407_CAL_PICK + 1) {
		local->number_of_spins = 0;
		local->last_spin = 0;
		goto handled;
	} /* this will start spin in daemon */


	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_407_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncBytes((byte *)scratch.his_score, sizeof(scratch.his_score));
	s.syncBytes((byte *)scratch.my_score,  sizeof(scratch.my_score));
	s.syncAsByte(scratch.his_score_int);
	s.syncAsByte(scratch.my_score_int);
	s.syncAsSint16LE(scratch.his_score_id);
	s.syncAsSint16LE(scratch.my_score_id);
	s.syncAsSint16LE(scratch.highlighting);
	s.syncAsSint16LE(scratch.prevent);
	s.syncAsSint16LE(scratch.final_choice);
	s.syncAsSint16LE(scratch.number_of_spins);
	s.syncAsSint16LE(scratch.last_spin);
	s.syncAsSint16LE(scratch.dealt);
	s.syncAsSint16LE(scratch.num_of_red);
	s.syncAsSint16LE(scratch.num_of_yellow);
	s.syncAsSint16LE(scratch.num_of_green);
	s.syncAsSint16LE(scratch.num_of_purple);
	s.syncAsSint16LE(scratch.whos_turn);
	s.syncAsSint16LE(scratch.var_to_conv);
	for (byte &v : scratch.stuff_to_give) s.syncAsByte(v);
	s.syncAsSint16LE(scratch.will_give);
}

void room_407_preload() {
	room_init_code_pointer = room_407_init;
	room_pre_parser_code_pointer = room_407_pre_parser;
	room_parser_code_pointer = room_407_parser;
	room_daemon_code_pointer = room_407_daemon;

	global[no_load_walker] = true;

	section_4_walker();
	section_4_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
