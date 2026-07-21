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

#include "mads/core/conv.h"
#include "mads/core/game.h"
#include "mads/core/imath.h"
#include "mads/core/inter.h"
#include "mads/core/kernel.h"
#include "mads/core/mouse.h"
#include "mads/core/sound.h"
#include "mads/core/text.h"
#include "mads/dragonsphere/mads/conv.h"
#include "mads/dragonsphere/mads/inventory.h"
#include "mads/dragonsphere/mads/sounds.h"
#include "mads/dragonsphere/mads/words.h"
#include "mads/dragonsphere/global.h"
#include "mads/dragonsphere/rooms/section1.h"
#include "mads/dragonsphere/rooms/room120.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];       /* Sprite series handles */
	int16 sequence[15];     /* Sequence handles      */
	int16 animation[4];     /* Animation handles     */

	char path_0[3];
	char path_1[3];

	int16 highlighting;
	int16 prevent;
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation


/* ========================= Sprite Series =================== */

#define fx_icon_wizard          0       /* rm120a0   */
#define fx_icon_desert          1       /* rm120b0   */
#define fx_icon_tree_left       2       /* rm120c0   */
#define fx_icon_tree_right      3       /* rm120f0   */

#define fx_wizard               4       /* rm120a1   */
#define fx_desert               5       /* rm120b1   */
#define fx_tree_left            6       /* rm120c1   */
#define fx_tree_right           7       /* rm120f1   */

#define fx_path_0               8       /* anything  */
#define fx_path_1               9       /* anything  */
#define fx_test                 10      /* interface */


/* ========================= Triggers ======================== */

#define ROOM_120_DOOR_CLOSES    60 
#define MUSIC                   79

/* these are coordinates within the big boxes */

#define WIZARD_X                108
#define WIZARD_Y                140

#define DESERT_X                9
#define DESERT_Y                90

#define TREE_LEFT_X             70
#define TREE_LEFT_Y             65

#define TREE_RIGHT_X            213
#define TREE_RIGHT_Y            54


static void room_120_init() {
	kernel_init_dialog();
	kernel_set_interface_mode(INTER_LIMITED_SENTENCES);

	player.walker_visible = false;
	local->highlighting = -1;
	local->prevent = false;

	if (global[llanie_status] == IS_SAVED) {
		global[make_504_empty] = true;
	}

	ss[fx_icon_desert] = kernel_load_series(kernel_name('b', 0), false);
	ss[fx_icon_tree_left] = kernel_load_series(kernel_name('c', 0), false);
	ss[fx_icon_tree_right] = kernel_load_series(kernel_name('f', 0), false);
	ss[fx_icon_wizard] = kernel_load_series(kernel_name('a', 0), false);

	ss[fx_tree_left] = kernel_load_series(kernel_name('c', 1), false);
	ss[fx_tree_right] = kernel_load_series(kernel_name('f', 1), false);
	ss[fx_wizard] = kernel_load_series(kernel_name('a', 1), false);
	ss[fx_desert] = kernel_load_series(kernel_name('b', 1), false);

	if (player_has_been_in_room(301)) {
		seq[fx_tree_left] = kernel_seq_stamp(ss[fx_tree_left], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_tree_left], 2);
	} else {
		ss[fx_icon_tree_left] = kernel_load_series(kernel_name('c', 0), false);
		seq[fx_icon_tree_left] = kernel_seq_stamp(ss[fx_icon_tree_left], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_icon_tree_left], 2);
		kernel_flip_hotspot_loc(words_Brynn_Fann, false, TREE_LEFT_X, TREE_LEFT_Y);
	}

	if (player_has_been_in_room(201)) {
		seq[fx_tree_right] = kernel_seq_stamp(ss[fx_tree_right], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_tree_right], 2);
	} else {
		ss[fx_icon_tree_right] = kernel_load_series(kernel_name('f', 0), false);
		seq[fx_icon_tree_right] = kernel_seq_stamp(ss[fx_icon_tree_right], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_icon_tree_right], 2);
		kernel_flip_hotspot_loc(words_Slathan_ni_Patan, false, TREE_RIGHT_X, TREE_RIGHT_Y);
	}

	if (player_has_been_in_room(401)) {
		seq[fx_desert] = kernel_seq_stamp(ss[fx_desert], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_desert], 2);
	} else {
		ss[fx_icon_desert] = kernel_load_series(kernel_name('b', 0), false);
		seq[fx_icon_desert] = kernel_seq_stamp(ss[fx_icon_desert], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_icon_desert], 2);
		kernel_flip_hotspot_loc(words_Soptus_Ecliptus, false, DESERT_X, DESERT_Y);
	}

	if (player_has_been_in_room(501)) {
		seq[fx_wizard] = kernel_seq_stamp(ss[fx_wizard], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_wizard], 2);
	} else {
		ss[fx_icon_wizard] = kernel_load_series(kernel_name('a', 0), false);
		seq[fx_icon_wizard] = kernel_seq_stamp(ss[fx_icon_wizard], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_icon_wizard], 2);
		kernel_flip_hotspot_loc(words_Hightower, false, WIZARD_X, WIZARD_Y);
	}

	section_1_music();
}

static void generate_path() {
	if (local->highlighting == words_Gran_Callahach) {
		if (global[pre_room] == 110) { /* Gran Callahach */
			local->path_0[0] = ' ';
			local->path_0[1] = ' ';
			local->path_1[0] = ' ';
			local->path_1[1] = ' ';

		} else if (global[pre_room] == 301) { /* Brynn-Fann */
			local->path_0[0] = 'c';
			local->path_0[1] = '5';
			local->path_1[0] = ' ';
			local->path_1[1] = ' ';

		} else if (global[pre_room] == 201) { /* Slathan ni Patan */
			local->path_0[0] = 'f';
			local->path_0[1] = '5';
			local->path_1[0] = ' ';
			local->path_1[1] = ' ';

		} else if (global[pre_room] == 401) { /* Soptus Ecliptus */
			local->path_0[0] = 'b';
			local->path_0[1] = '3';
			local->path_1[0] = 'a';
			local->path_1[1] = '7';

		} else { /* Hightower */
			local->path_0[0] = 'a';
			local->path_0[1] = '3';
			local->path_1[0] = 'a';
			local->path_1[1] = '7';
		}

	} else if (local->highlighting == words_Slathan_ni_Patan) {
		if (global[pre_room] == 110) { /* Gran Callahach */
			local->path_0[0] = 'e';
			local->path_0[1] = '2';
			local->path_1[0] = ' ';
			local->path_1[1] = ' ';

		} else if (global[pre_room] == 301) { /* Brynn-Fann */
			local->path_0[0] = 'c';
			local->path_0[1] = '4';
			local->path_1[0] = ' ';
			local->path_1[1] = ' ';

		} else if (global[pre_room] == 201) { /* Slathan ni Patan */
			local->path_0[0] = ' ';
			local->path_0[1] = ' ';
			local->path_1[0] = ' ';
			local->path_1[1] = ' ';

		} else if (global[pre_room] == 401) { /* Soptus Ecliptus */
			local->path_0[0] = 'b';
			local->path_0[1] = '3';
			local->path_1[0] = 'a';
			local->path_1[1] = '8';

		} else { /* Hightower */
			local->path_0[0] = 'a';
			local->path_0[1] = '3';
			local->path_1[0] = 'a';
			local->path_1[1] = '8';
		}

	} else if (local->highlighting == words_Hightower) {
		if (global[pre_room] == 110) { /* Gran Callahach */
			local->path_0[0] = 'e';
			local->path_0[1] = '1';
			local->path_1[0] = 'a';
			local->path_1[1] = '4';

		} else if (global[pre_room] == 301) { /* Brynn-Fann */
			local->path_0[0] = 'c';
			local->path_0[1] = '3';
			local->path_1[0] = 'a';
			local->path_1[1] = '4';

		} else if (global[pre_room] == 201) { /* Slathan ni Patan */
			local->path_0[0] = 'f';
			local->path_0[1] = '3';
			local->path_1[0] = 'a';
			local->path_1[1] = '4';

		} else if (global[pre_room] == 401) { /* Soptus Ecliptus */
			local->path_0[0] = 'b';
			local->path_0[1] = '5';
			local->path_1[0] = ' ';
			local->path_1[1] = ' ';

		} else { /* Hightower */
			local->path_0[0] = ' ';
			local->path_0[1] = ' ';
			local->path_1[0] = ' ';
			local->path_1[1] = ' ';
		}

	} else if (local->highlighting == words_Soptus_Ecliptus) {
		if (global[pre_room] == 110) { /* Gran Callahach */
			local->path_0[0] = 'e';
			local->path_0[1] = '1';
			local->path_1[0] = 'b';
			local->path_1[1] = '4';

		} else if (global[pre_room] == 301) { /* Brynn-Fann */
			local->path_0[0] = 'c';
			local->path_0[1] = '3';
			local->path_1[0] = 'b';
			local->path_1[1] = '4';

		} else if (global[pre_room] == 201) { /* Slathan ni Patan */
			local->path_0[0] = 'f';
			local->path_0[1] = '3';
			local->path_1[0] = 'b';
			local->path_1[1] = '4';

		} else if (global[pre_room] == 401) { /* Soptus Ecliptus */
			local->path_0[0] = ' ';
			local->path_0[1] = ' ';
			local->path_1[0] = ' ';
			local->path_1[1] = ' ';

		} else { /* Hightower */
			local->path_0[0] = 'a';
			local->path_0[1] = '5';
			local->path_1[0] = ' ';
			local->path_1[1] = ' ';
		}

	} else if (local->highlighting == words_Brynn_Fann) {
		if (global[pre_room] == 110) { /* Gran Callahach */
			local->path_0[0] = 'e';
			local->path_0[1] = '0';
			local->path_1[0] = ' ';
			local->path_1[1] = ' ';

		} else if (global[pre_room] == 301) { /* Brynn-Fann */
			local->path_0[0] = ' ';
			local->path_0[1] = ' ';
			local->path_1[0] = ' ';
			local->path_1[1] = ' ';

		} else if (global[pre_room] == 201) { /* Slathan ni Patan */
			local->path_0[0] = 'f';
			local->path_0[1] = '4';
			local->path_1[0] = ' ';
			local->path_1[1] = ' ';

		} else if (global[pre_room] == 401) { /* Soptus Ecliptus */
			local->path_0[0] = 'b';
			local->path_0[1] = '3';
			local->path_1[0] = 'a';
			local->path_1[1] = '6';

		} else { /* Hightower */
			local->path_0[0] = 'a';
			local->path_0[1] = '3';
			local->path_1[0] = 'a';
			local->path_1[1] = '6';
		}
	}

	local->path_0[2] = '\0';
	local->path_1[2] = '\0';
}

void room_120_daemon() {
	int temp; /* for synching purposes */
	char name[9] = "";
	int count;
	int good;

	if (kernel.trigger == 10) {
		generate_path();
		player.commands_allowed = false;

		if (local->highlighting == words_Brynn_Fann) {
			if (!player_has_been_in_room(301)) {
				kernel_seq_delete(seq[fx_icon_tree_left]);
				seq[fx_tree_left] = kernel_seq_forward(ss[fx_tree_left], false, 7, 0, 0, 1);
				kernel_seq_depth(seq[fx_tree_left], 2);
				kernel_seq_range(seq[fx_tree_left], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_trigger(seq[fx_tree_left], KERNEL_TRIGGER_EXPIRE, 0, 1);
				sound_play(N_WorldEmerges);
			} else {
				kernel_timing_trigger(1, 2);  /* start path 0 of 1 */
			}

		} else if (local->highlighting == words_Slathan_ni_Patan) {
			if (!player_has_been_in_room(201)) {
				kernel_seq_delete(seq[fx_icon_tree_right]);
				seq[fx_tree_right] = kernel_seq_forward(ss[fx_tree_right], false, 7, 0, 0, 1);
				kernel_seq_depth(seq[fx_tree_right], 2);
				kernel_seq_range(seq[fx_tree_right], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_trigger(seq[fx_tree_right], KERNEL_TRIGGER_EXPIRE, 0, 1);
				sound_play(N_WorldEmerges);
			} else {
				kernel_timing_trigger(1, 2);  /* start path 0 of 1 */
			}

		} else if (local->highlighting == words_Soptus_Ecliptus) {
			if (!player_has_been_in_room(401)) {
				kernel_seq_delete(seq[fx_icon_desert]);
				seq[fx_desert] = kernel_seq_forward(ss[fx_desert], false, 7, 0, 0, 1);
				kernel_seq_depth(seq[fx_desert], 2);
				kernel_seq_range(seq[fx_desert], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_trigger(seq[fx_desert], KERNEL_TRIGGER_EXPIRE, 0, 1);
				sound_play(N_WorldEmerges);
			} else {
				kernel_timing_trigger(1, 2);  /* start path 0 of 1 */
			}

		} else if (local->highlighting == words_Hightower) {
			if (!player_has_been_in_room(501)) {
				kernel_seq_delete(seq[fx_icon_wizard]);
				seq[fx_wizard] = kernel_seq_forward(ss[fx_wizard], false, 7, 0, 0, 1);
				kernel_seq_depth(seq[fx_wizard], 2);
				kernel_seq_range(seq[fx_wizard], KERNEL_FIRST, KERNEL_LAST);
				kernel_seq_trigger(seq[fx_wizard], KERNEL_TRIGGER_EXPIRE, 0, 1);
				sound_play(N_WorldEmerges);
			} else {
				kernel_timing_trigger(1, 2);  /* start path 0 of 1 */
			}

		} else {
			/* gran callahach */
			kernel_timing_trigger(1, 2);  /* start path 0 of 1 */
		}
	}

	if (kernel.trigger == 1) {
		if (local->highlighting == words_Brynn_Fann) {
			temp = seq[fx_tree_left];
			seq[fx_tree_left] = kernel_seq_stamp(ss[fx_tree_left], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_tree_left], 2);
			kernel_synch(KERNEL_SERIES, seq[fx_tree_left], KERNEL_SERIES, temp);
			kernel_timing_trigger(1, 2);

		} else if (local->highlighting == words_Slathan_ni_Patan) {
			temp = seq[fx_tree_right];
			seq[fx_tree_right] = kernel_seq_stamp(ss[fx_tree_right], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_tree_right], 2);
			kernel_synch(KERNEL_SERIES, seq[fx_tree_right], KERNEL_SERIES, temp);
			kernel_timing_trigger(1, 2);

		} else if (local->highlighting == words_Soptus_Ecliptus) {
			temp = seq[fx_desert];
			seq[fx_desert] = kernel_seq_stamp(ss[fx_desert], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_desert], 2);
			kernel_synch(KERNEL_SERIES, seq[fx_desert], KERNEL_SERIES, temp);
			kernel_timing_trigger(1, 2);

		} else if (local->highlighting == words_Hightower) {
			temp = seq[fx_wizard];
			seq[fx_wizard] = kernel_seq_stamp(ss[fx_wizard], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_wizard], 2);
			kernel_synch(KERNEL_SERIES, seq[fx_wizard], KERNEL_SERIES, temp);
			kernel_timing_trigger(1, 2);
		}
	}

	if (kernel.trigger == 2) {
		Common::strcpy_s(name, "*RM120");
		Common::strcat_s(name, local->path_0);

		if ((local->highlighting == words_Brynn_Fann) && (global[pre_room] == 301)) {
			new_room = 301;

		} else if ((local->highlighting == words_Gran_Callahach) && (global[pre_room] == 110)) {
			new_room = 110;

		} else if ((local->highlighting == words_Slathan_ni_Patan) && (global[pre_room] == 201)) {
			new_room = 201;

		} else if ((local->highlighting == words_Hightower) && (global[pre_room] == 501)) {
			new_room = 501;

		} else if ((local->highlighting == words_Soptus_Ecliptus) && (global[pre_room] == 401)) {
			if (!player_has_been_in_room(404)) {
				new_room = 404;
			} else {
				global[desert_room] = 42;
				global[from_direction] = FROM_EAST;
				new_room = 401;
			}

		} else {
			ss[fx_path_0] = kernel_load_series(name, false);
			seq[fx_path_0] = kernel_seq_forward(ss[fx_path_0], false, 5, 0, 0, 1);
			kernel_seq_depth(seq[fx_path_0], 1);
			kernel_seq_range(seq[fx_path_0], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_path_0], KERNEL_TRIGGER_EXPIRE, 0, 3);
		}
	}

	if (kernel.trigger == 3) {
		temp = seq[fx_path_0];
		seq[fx_path_0] = kernel_seq_stamp(ss[fx_path_0], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_path_0], 1);
		kernel_synch(KERNEL_SERIES, seq[fx_path_0], KERNEL_SERIES, temp);
		kernel_timing_trigger(6, 4);
	}

	if (kernel.trigger == 4) {
		if (local->path_1[0] == ' ') {
			kernel_timing_trigger(6, 6);

		} else {
			Common::strcpy_s(name, "*RM120");
			Common::strcat_s(name, local->path_1);
			ss[fx_path_1] = kernel_load_series(name, false);
			seq[fx_path_1] = kernel_seq_forward(ss[fx_path_1], false, 5, 0, 0, 1);
			kernel_seq_depth(seq[fx_path_1], 1);
			kernel_seq_range(seq[fx_path_1], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_path_1], KERNEL_TRIGGER_EXPIRE, 0, 5);
		}
	}

	if (kernel.trigger == 5) {
		temp = seq[fx_path_1];
		seq[fx_path_1] = kernel_seq_stamp(ss[fx_path_1], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_path_1], 1);
		kernel_synch(KERNEL_SERIES, seq[fx_path_1], KERNEL_SERIES, temp);
		kernel_timing_trigger(6, 6);
	}

	if (kernel.trigger == 6) {
		if (local->highlighting == words_Brynn_Fann) {
			new_room = 301;

		} else if (local->highlighting == words_Gran_Callahach) {
			new_room = 110;

		} else if (local->highlighting == words_Slathan_ni_Patan) {
			new_room = 201;

		} else if (local->highlighting == words_Soptus_Ecliptus) {
			if (!player_has_been_in_room(404)) {
				new_room = 404;
			} else {
				global[desert_room] = 42;
				global[from_direction] = FROM_EAST;
				new_room = 401;
			}

		} else if (local->highlighting == words_Hightower) {
			if (global[player_persona] == PLAYER_IS_KING) {
				new_room = 501;
			} else {
				new_room = 502;
			}
		}
	}

	if (((mouse_status & 1) || (mouse_status & 2)) && local->highlighting == -1) {
		good = -1;

		for (count = 0; count < room_num_spots; count++) {
			if (room_spots[count].active) {
				if (mouse_x >= room_spots[count].ul_x &&
					mouse_x <= room_spots[count].lr_x &&
					mouse_y >= room_spots[count].ul_y &&
					mouse_y <= room_spots[count].lr_y) {
					good = count;
				}
			}
		}

		if (room_spots[good].vocab == words_Soptus_Ecliptus) {
			if (!player_has_been_in_room(401)) {
				kernel_seq_delete(seq[fx_icon_desert]);
				seq[fx_icon_desert] = kernel_seq_stamp(ss[fx_icon_desert], false, KERNEL_LAST);
				kernel_seq_depth(seq[fx_icon_desert], 2);
			}
			local->highlighting = words_Soptus_Ecliptus;

		} else if (room_spots[good].vocab == words_Brynn_Fann) {
			if (!player_has_been_in_room(301)) {
				kernel_seq_delete(seq[fx_icon_tree_left]);
				seq[fx_icon_tree_left] = kernel_seq_stamp(ss[fx_icon_tree_left], false, KERNEL_LAST);
				kernel_seq_depth(seq[fx_icon_tree_left], 2);
			}
			local->highlighting = words_Brynn_Fann;

		} else if (room_spots[good].vocab == words_Slathan_ni_Patan) {
			if (!player_has_been_in_room(201)) {
				kernel_seq_delete(seq[fx_icon_tree_right]);
				seq[fx_icon_tree_right] = kernel_seq_stamp(ss[fx_icon_tree_right], false, KERNEL_LAST);
				kernel_seq_depth(seq[fx_icon_tree_right], 2);
			}
			local->highlighting = words_Slathan_ni_Patan;

		} else if (room_spots[good].vocab == words_Hightower) {
			if (!player_has_been_in_room(501)) {
				kernel_seq_delete(seq[fx_icon_wizard]);
				seq[fx_icon_wizard] = kernel_seq_stamp(ss[fx_icon_wizard], false, KERNEL_LAST);
				kernel_seq_depth(seq[fx_icon_wizard], 2);
			}
			local->highlighting = words_Hightower;

		} else if (room_spots[good].vocab == words_Gran_Callahach) {
			local->highlighting = words_Gran_Callahach;
		}
	}

	good = 0;

	if (((mouse_status & 1) || (mouse_status & 2)) && local->highlighting != -1 && player.commands_allowed) {

		for (count = 0; count < room_num_spots; count++) {
			if (room_spots[count].active) {
				if (mouse_x >= room_spots[count].ul_x &&
					mouse_x <= room_spots[count].lr_x &&
					mouse_y >= room_spots[count].ul_y &&
					mouse_y <= room_spots[count].lr_y) {
					good = 1;
				}
			}
		}
	}

	if (!good && (local->highlighting != -1) && !local->prevent) {
		switch (local->highlighting) {
		case words_Soptus_Ecliptus:
			if (!player_has_been_in_room(401)) {
				kernel_seq_delete(seq[fx_icon_desert]);
				seq[fx_icon_desert] = kernel_seq_stamp(ss[fx_icon_desert], false, KERNEL_FIRST);
				kernel_seq_depth(seq[fx_icon_desert], 2);
				kernel_flip_hotspot_loc(words_Soptus_Ecliptus, false, DESERT_X, DESERT_Y);
			}
			break;

		case words_Brynn_Fann:
			if (!player_has_been_in_room(301)) {
				kernel_seq_delete(seq[fx_icon_tree_left]);
				seq[fx_icon_tree_left] = kernel_seq_stamp(ss[fx_icon_tree_left], false, KERNEL_FIRST);
				kernel_seq_depth(seq[fx_icon_tree_left], 2);
				kernel_flip_hotspot_loc(words_Brynn_Fann, false, TREE_LEFT_X, TREE_LEFT_Y);
			}
			break;

		case words_Slathan_ni_Patan:
			if (!player_has_been_in_room(201)) {
				kernel_seq_delete(seq[fx_icon_tree_right]);
				seq[fx_icon_tree_right] = kernel_seq_stamp(ss[fx_icon_tree_right], false, KERNEL_FIRST);
				kernel_seq_depth(seq[fx_icon_tree_right], 2);
				kernel_flip_hotspot_loc(words_Slathan_ni_Patan, false, TREE_RIGHT_X, TREE_RIGHT_Y);
			}
			break;

		case words_Hightower:
			if (!player_has_been_in_room(501)) {
				kernel_seq_delete(seq[fx_icon_wizard]);
				seq[fx_icon_wizard] = kernel_seq_stamp(ss[fx_icon_wizard], false, KERNEL_FIRST);
				kernel_seq_depth(seq[fx_icon_wizard], 2);
				kernel_flip_hotspot_loc(words_Hightower, false, WIZARD_X, WIZARD_Y);
			}
			break;
		}

		if (local->highlighting != -1 && local->highlighting != 1000 && !local->prevent) {
			good = -1;

			for (count = 0; count < room_num_spots; count++) {
				if (room_spots[count].active) {
					if (mouse_x >= room_spots[count].ul_x &&
						mouse_x <= room_spots[count].lr_x &&
						mouse_y >= room_spots[count].ul_y &&
						mouse_y <= room_spots[count].lr_y) {
						good = count;
					}
				}
			}

			if (good != -1) {
				kernel_timing_trigger(1, 10);
				local->prevent = true;

			} else {
				local->highlighting = -1;
			}

		} else {
			local->highlighting = -1;
		}
	}

	if (kernel.trigger == MUSIC) {
		sound_play(N_WalkingMus);
	}
}

static void room_120_pre_parser() {
	player_cancel_command();
	player.need_to_walk = false;
}


static void room_120_parser() {
}

void room_120_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncBytes((byte *)scratch.path_0, 3);
	s.syncBytes((byte *)scratch.path_1, 3);
	s.syncAsSint16LE(scratch.highlighting);
	s.syncAsSint16LE(scratch.prevent);
}

void room_120_preload() {
	room_init_code_pointer = room_120_init;
	room_pre_parser_code_pointer = room_120_pre_parser;
	room_parser_code_pointer = room_120_parser;
	room_daemon_code_pointer = room_120_daemon;

	/* inter_init_sentence(); */
	section_1_walker();
	section_1_interface();
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
