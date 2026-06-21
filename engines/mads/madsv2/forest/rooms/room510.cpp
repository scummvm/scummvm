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

#include "mads/madsv2/forest/digi.h"
#include "mads/madsv2/forest/global.h"
#include "mads/madsv2/forest/midi.h"
#include "mads/madsv2/core/config.h"
#include "mads/madsv2/core/font.h"
#include "mads/madsv2/core/game.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/matte.h"
#include "mads/madsv2/core/mouse.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/quote.h"
#include "mads/madsv2/core/sprite.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {
namespace Rooms {

struct Scratch {
	int16 sprite[10];           /* 0x00 — sprite series handles */
	int16 sequence[10];         /* 0x14 — sequence handles      */
	int16 animation[10];        /* 0x28 — animation handles     */
	AnimationInfo animation_info[10]; /* 0x3C                   */
	int16 _8c;                  /* 0x8C                         */
	int16 _8e;                  /* 0x8E — digi-play phase check */
	int16 _90;                  /* 0x90 — volume/replay flag    */
	int16 _92;                  /* 0x92 — timing accumulator    */
	int16 _94;                  /* 0x94 — sequence table index  */
	int16 _96;                  /* 0x96 — active flag           */
	int16 _98;                  /* 0x98 — room mode (669/670)   */
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation
#define aainfo scratch.animation_info

// Credits-line table
struct Room510Entry {
	int16 quote_id;	// quote ID to display, or -1 (schedule anim4), or -2 (end).
	int16 x;		// x position override (0 = auto-centre on 320-wide screen).
	int16 y;		// y position override (0 = use 70).
};

static const Room510Entry room_510_array1[112] = {
	/* [0] unused */ {  0,  0,   0 },
	/* [1]  */ {  74,  0,  40 }, /* [2]  */ {  75,  0,  60 },
	/* [3]  */ {  76,  0,  80 }, /* [4]  */ {  77,  0, 100 },
	/* [5]  */ {  -1,  0,   0 }, /* [6]  */ {  78,  0,   0 },
	/* [7]  */ {  -1,  0,   0 }, /* [8]  */ {  79,  0,   0 },
	/* [9]  */ {  -1,  0,   0 }, /* [10] */ {  80,  0,   0 },
	/* [11] */ {  -1,  0,   0 }, /* [12] */ {  81,  0,   0 },
	/* [13] */ {  -1,  0,   0 }, /* [14] */ {  82,  0,  50 },
	/* [15] */ {  83,  0,  80 }, /* [16] */ {  -1,  0,   0 },
	/* [17] */ {  84,  0,  50 }, /* [18] */ {  85,  0,  80 },
	/* [19] */ {  -1,  0,   0 }, /* [20] */ {  86,  0,  50 },
	/* [21] */ {  87,  0,  80 }, /* [22] */ {  -1,  0,   0 },
	/* [23] */ {  88,  0,  50 }, /* [24] */ {  89,  0,  80 },
	/* [25] */ {  -1,  0,   0 }, /* [26] */ {  90,  0,  40 },
	/* [27] */ {  91,  0,  60 }, /* [28] */ {  92,  0,  80 },
	/* [29] */ {  93,  0, 100 }, /* [30] */ {  -1,  0,   0 },
	/* [31] */ {  94,  0,  50 }, /* [32] */ {  95,  0,  80 },
	/* [33] */ {  -1,  0,   0 }, /* [34] */ {  96,  0,  50 },
	/* [35] */ {  97,  0,  80 }, /* [36] */ {  -1,  0,   0 },
	/* [37] */ {  98,  0,  50 }, /* [38] */ {  99,  0,  80 },
	/* [39] */ { 100,  0, 100 }, /* [40] */ {  -1,  0,   0 },
	/* [41] */ { 101,  0,  50 }, /* [42] */ { 102,  0,  80 },
	/* [43] */ {  -1,  0,   0 }, /* [44] */ { 103,  0,  50 },
	/* [45] */ { 104,  0,  80 }, /* [46] */ {  -1,  0,   0 },
	/* [47] */ { 105,  0,  50 }, /* [48] */ { 106,  0,  80 },
	/* [49] */ { 107,  0, 100 }, /* [50] */ {  -1,  0,   0 },
	/* [51] */ { 108,  0,  50 }, /* [52] */ { 109,  0,  80 },
	/* [53] */ {  -1,  0,   0 }, /* [54] */ { 110,  0,  50 },
	/* [55] */ { 111,  0,  80 }, /* [56] */ { 112,  0, 100 },
	/* [57] */ {  -1,  0,   0 }, /* [58] */ { 113,  0,  50 },
	/* [59] */ { 114,  0,  80 }, /* [60] */ { 115,  0, 100 },
	/* [61] */ {  -1,  0,   0 }, /* [62] */ { 116,  0,  50 },
	/* [63] */ { 117,  0,  80 }, /* [64] */ {  -1,  0,   0 },
	/* [65] */ { 118,  0,  20 }, /* [66] */ { 119,  0,  40 },
	/* [67] */ { 120,  0,  60 }, /* [68] */ { 121,  0,  80 },
	/* [69] */ { 122,  0, 100 }, /* [70] */ { 123,  0, 120 },
	/* [71] */ {  -1,  0,   0 }, /* [72] */ { 124,  0,  20 },
	/* [73] */ { 125,  0,  40 }, /* [74] */ { 126,  0,  60 },
	/* [75] */ { 127,  0,  80 }, /* [76] */ { 128,  0, 100 },
	/* [77] */ {  -1,  0,   0 }, /* [78] */ { 129,  0,  40 },
	/* [79] */ { 130,  0,  80 }, /* [80] */ { 131,  0, 120 },
	/* [81] */ {  -1,  0,   0 }, /* [82] */ { 132,  0,  40 },
	/* [83] */ { 133,  0,  60 }, /* [84] */ { 134,  0,  80 },
	/* [85] */ {  -1,  0,   0 }, /* [86] */ { 135,  0,  40 },
	/* [87] */ { 136,  0,  80 }, /* [88] */ {  -1,  0,   0 },
	/* [89] */ { 137,  0,  40 }, /* [90] */ { 138,  0,  60 },
	/* [91] */ { 139,  0,  80 }, /* [92] */ {  -1,  0,   0 },
	/* [93] */ { 140,  0,  40 }, /* [94] */ { 141,  0,  80 },
	/* [95] */ {  -1,  0,   0 }, /* [96] */ { 142,  0,  20 },
	/* [97] */ { 143,  0,  40 }, /* [98] */ { 144,  0,  60 },
	/* [99] */ { 145,  0,  80 }, /* [100]*/ { 146,  0, 100 },
	/* [101]*/ { 147,  0, 120 }, /* [102]*/ {  -1,  0,   0 },
	/* [103]*/ { 148,  0,  40 }, /* [104]*/ { 149,  0,  60 },
	/* [105]*/ { 150,  0,  80 }, /* [106]*/ { 151,  0, 100 },
	/* [107]*/ {  -1,  0,   0 }, /* [108]*/ { 152,  0,  50 },
	/* [109]*/ { 153,  0,  80 }, /* [110]*/ {  -1,  0,   0 },
	/* [111]*/ {  -2,  0,   0 },
};

static void room_510_anim4() {
	for (int i = 0; i < 10; i++)
		kernel_message_delete(i);

	// The original called an anim5 function here, which did a shift within an unused array.
}

static void room_510_load_quotes() {
	scratch._94 = 1;
	kernel.quotes = quote_load(
		74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92,
		93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
		110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124,
		125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139,
		140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 0);
	scratch._92 = 100;
}

static void room_510_anim1() {
	const Room510Entry &entry = room_510_array1[scratch._94];
	char *text = quote_string(kernel.quotes, entry.quote_id);
	int x;
	if (entry.x) {
		x = entry.x;
	} else {
		x = 160 - font_string_width(font_conv, text, -1) / 2;
	}
	int y = entry.y ? entry.y : 70;
	kernel_message_add(text, x, y, 92, 999, 0, 0);
}

static void room_510_anim2() {
	int16 aa_frame = kernel_anim[aa[0]].frame;
	if (aa_frame != aainfo[0]._frame) {
		aainfo[0]._frame = aa_frame;
		if (aa_frame == 1) {
			digi_play_build(503, '_', 2, 3);
			digi_val2 = -1;
			scratch._90 = 1;
			kernel_seq_delete(seq[0]);
			kernel_seq_delete(seq[1]);
			kernel_seq_delete(seq[2]);
			kernel_seq_delete(seq[3]);
			kernel_seq_delete(seq[4]);
		} else if (aa_frame == 57) {
			digi_play_build(510, 'e', 1, 1);
			scratch._8e = 63;
		} else if (aa_frame == 131) {
			if (config_file.forest1)
				digi_stop(1);
			scratch._90 = 133;
			digi_play_build(510, '_', 3, 2);
		} else if (aa_frame == 140) {
			kernel_abort_animation(aa[0]);
			aainfo[0]._active = 0;
			aa[1] = kernel_run_animation(kernel_name('F', 2), 0);
			aainfo[1]._active = -1;
			scratch._8c = 40;
			kernel_synch(KERNEL_ANIM, aa[1], KERNEL_ANIM, aa[0]);
		}
	}

	if (kernel.trigger == 7 || kernel.trigger == 28) {
		if (scratch._8e == 63) {
			scratch._8e = 66;
			kernel_timing_trigger(2, 28);
		} else if (scratch._8e == 66) {
			scratch._8e = 90;
			digi_play_build(510, 'b', 1, 1);
		} else if (scratch._8e == 90) {
			scratch._8e = 91;
			kernel_timing_trigger(2, 28);
		} else if (scratch._8e == 91) {
			scratch._8e = 132;
			digi_play_build(510, 'r', 1, 1);
		} else if (scratch._8e == 133) {
			scratch._8e = -1;
		}
	}

	if (kernel.trigger == 8) {
		if (scratch._90 == 1) {
			scratch._90 = 1;
			digi_play_build(503, '_', 2, 3);
			digi_val2 = -1;
		} else if (scratch._90 == 133) {
			scratch._90 = -1;
		}
	}
}

static void room_510_anim3() {
	int16 aa_frame = kernel_anim[aa[1]].frame;
	if (aa_frame != aainfo[1]._frame) {
		aainfo[1]._frame = aa_frame;
		if (aa_frame == 1) {
			digi_play_build(510, '_', 1, 2);
			scratch._90 = 1;
			midi_stop();
			global_midi_play(4);
		} else if (aa_frame == 40) {
			midi_stop();
			new_room = 107;
		}
	}

	if (kernel.trigger == 8 && scratch._90 == 1)
		scratch._90 = -1;
}

static void room_510_init() {
	global[player_score] = 0;
	global[g009] = 0;
	scratch._96 = 0;
	scratch._98 = 670;
	viewing_at_y = 22;
	player.walker_visible = 0;
	player.commands_allowed = 0;
	mouse_hide();

	for (int count = 0; count < 10; count++) {
		aainfo[count]._active = 0;
		aainfo[count]._frame = -1;
	}

	if (previous_room == 104) {
		room_510_load_quotes();
		scratch._98 = 669;
		aa[2] = kernel_run_animation(kernel_name('c', 1), 106);
		aainfo[2]._active = -1;
		aainfo[2]._frame = 0;
		return;
	}

	if (global[g102] != 0 || flags[4] == 5) {
		room_510_load_quotes();
		kernel_timing_trigger(50, 106);
		kernel_timing_trigger(51, 108);
		return;
	}

	aa[0] = kernel_run_animation(kernel_name('F', 1), 0);
	aainfo[0]._active = -1;
	scratch._8c = 140;
}

static void room_510_daemon() {
	if (global[player_hyperwalked] == -1) {
		if (global[g102] == 0) {
			game_save_name(0);
			kernel_save_game(save_game_buf);
		}
		global[g102] = 0;
		new_room = 904;
	}

	switch (kernel.trigger) {
	case 8:
		if (scratch._8e == 668) {
			scratch._8e = -1;
			global[g009] = 0;
			global_midi_play(14);
			kernel_timing_trigger(2100, 104);
		}
		break;
	case 102: {
		int16 action = room_510_array1[scratch._94].quote_id;
		if (action == -2) {
			global[g102] = 0;
			new_room = 904;
		} else if (action == -1) {
			kernel_timing_trigger(scratch._92, 103);
			scratch._92 = 100;
		} else {
			room_510_anim1();
			kernel_timing_trigger(1, 102);
			scratch._92 += 20;
		}
		scratch._94++;
		break;
	}
	case 103:
		room_510_anim4();
		kernel_timing_trigger(20, 102);
		break;
	case 104:
		global[g009] = -1;
		global_midi_play(11);
		break;
	case 106:
		scratch._96 = -1;
		last_keypressed = -1;
		aainfo[2]._active = 0;
		kernel_timing_trigger(40, 107);
		kernel_timing_trigger(30, 102);
		if (scratch._98 == 669)
			kernel_timing_trigger(1400, 108);
		break;
	case 107:
		mouse_cursor_sprite(cursor, 1);
		break;
	case 108:
		midi_stop();
		digi_play_build(306, '_', 999, 2);
		scratch._8e = 668;
		break;
	default:
		break;
	}

	if (aainfo[0]._active) room_510_anim2();
	if (aainfo[1]._active) room_510_anim3();
}

static void room_510_parser() {
	player.command_ready = 0;
}

void room_510_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	for (AnimationInfo &ai : scratch.animation_info) ai.synchronize(s);
	s.syncAsSint16LE(scratch._8c);
	s.syncAsSint16LE(scratch._8e);
	s.syncAsSint16LE(scratch._90);
	s.syncAsSint16LE(scratch._92);
	s.syncAsSint16LE(scratch._94);
	s.syncAsSint16LE(scratch._96);
	s.syncAsSint16LE(scratch._98);
}

void room_510_preload() {
	room_init_code_pointer   = room_510_init;
	room_parser_code_pointer = room_510_parser;
	room_daemon_code_pointer = room_510_daemon;

	global_section_walker();
	global_section_interface();
	player.walker_must_reload = true;
}

} // namespace Rooms
} // namespace Forest
} // namespace MADSV2
} // namespace MADS
