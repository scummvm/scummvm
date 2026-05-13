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
#include "mads/madsv2/core/inter.h"
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/core/vocab.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section1.h"
#include "mads/madsv2/dragonsphere/rooms/room102.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

// ---------------------------------------------------------------------------
// Scratch layout (matches ROOM102.MAC)
//
//   game.scratch offsets:
//     sprite[0..14]       = 0x00..0x1C  (int16 each)
//     sequence[0..14]     = 0x1E..0x3A
//     animation[0..3]     = 0x3C..0x42
//     diary1_id           = 0x44
//     diary2_id           = 0x46
//     diary_frame         = 0x48
//     animation_running   = 0x4A
//     temp                = 0x4C
// ---------------------------------------------------------------------------
struct Scratch {
	int16 sprite[15];           // ss[]  — series handles
	int16 sequence[15];         // seq[] — sequence handles
	int16 animation[4];         // aa[]  — animation handles
	int16 diary1_id;            // 0x44 — dynamic hotspot handle for diary 1
	int16 diary2_id;            // 0x46 — dynamic hotspot handle for diary 2
	int16 diary_frame;          // 0x48 — last observed animation frame (reading)
	int16 animation_running;    // 0x4A — which diary is being read: DIARY1 or DIARY2
	int16 temp;                 // 0x4C — temp sprite handle for synch during door close
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

// Sprite series slot indices (from ROOM102.MAC)
#define fx_diary1        1   // rm102p0
#define fx_diary2        2   // rm102p1
// fx_reach_diaries  3      // kgrh_9 — defined but never loaded
#define fx_fire          4   // rm102y0
#define fx_fire_shadow   5   // rm102y1
#define fx_door          6   // rm102x
#define fx_open_door     7   // kgrd_6
#define fx_fire_sconce   8   // rm102y2

// Trigger constants
#define ROOM_102_DOOR_CLOSES  70

// Walk/position constants
#define START_X_ROOM_101     -10
#define START_Y_ROOM_101     130
#define WALK_TO_X_FROM_101   35
#define WALK_TO_Y_FROM_101   144
#define START_X_ROOM_103     170
#define START_Y_ROOM_103     152
#define WALK_TO_DIARIES_X    47
#define WALK_TO_DIARIES_Y    123
#define WALK_TO_DOOR_X       0
#define WALK_TO_DOOR_Y       130

// Animation identifiers (which diary reading sequence is active)
#define DIARY1  1
#define DIARY2  2

// Vocabulary word IDs (from VOCAB.DB; formula: 1-based-line + 13)
//   Hardcoded verbs (VOCABH.DB):
//     look=3, take=4, push=5, open=6, pull=10, close=11, walk_to=13
//   VOCAB.DB-derived nouns (verified against disassembly where noted):
//     walk_through=37 (✓), look_at=30 (✓)
//     floor=16, rug=18, wall=20, bed=21, chest=23, window=24, nightstand=25
//     tapestry=26 (✓), fireplace=34, fireplace_screen=35
//     decoration=41 (✓), ceiling=196, door_to_hallway=197
//     wood_basket=204, door_to_king_s_room=245 (✓)
//     flowers=251, shutters=252, bookcase=253, diaries=264, sconce=329

// ---------------------------------------------------------------------------

void room_102_init() {
	// Load sprite series (fx_reach_diaries / kgrh_9 is intentionally skipped)
	ss[fx_diary1]      = kernel_load_series(kernel_name('p', 0), false);
	ss[fx_diary2]      = kernel_load_series(kernel_name('p', 1), false);
	ss[fx_fire]        = kernel_load_series(kernel_name('y', 0), false);
	ss[fx_fire_shadow] = kernel_load_series(kernel_name('y', 1), false);
	ss[fx_door]        = kernel_load_series(kernel_name('x', -1), false);
	ss[fx_open_door]   = kernel_load_series("*KGRD_6", false);
	ss[fx_fire_sconce] = kernel_load_series(kernel_name('y', 2), false);

	// Start continuous background sequences
	seq[fx_fire]        = kernel_seq_forward(ss[fx_fire],        false, 7, 0, 0, 0);
	seq[fx_fire_shadow] = kernel_seq_forward(ss[fx_fire_shadow], false, 7, 0, 0, 0);
	seq[fx_fire_sconce] = kernel_seq_forward(ss[fx_fire_sconce], false, 7, 0, 0, 0);

	// Stamp diary sprites and register their dynamic hotspots
	seq[fx_diary1]      = kernel_seq_stamp(ss[fx_diary1], false, KERNEL_FIRST);
	kernel_seq_depth(seq[fx_diary1], 12);
	local->diary1_id    = kernel_add_dynamic(264, 13, SYNTAX_PLURAL,
	                          seq[fx_diary1], 0, 0, 0, 0);
	kernel_dynamic_walk(local->diary1_id, WALK_TO_DIARIES_X, WALK_TO_DIARIES_Y, FACING_NORTHWEST);

	seq[fx_diary2]      = kernel_seq_stamp(ss[fx_diary2], false, KERNEL_FIRST);
	kernel_seq_depth(seq[fx_diary2], 12);
	local->diary2_id    = kernel_add_dynamic(264, 13, SYNTAX_PLURAL,
	                          seq[fx_diary2], 0, 0, 0, 0);
	kernel_dynamic_walk(local->diary2_id, WALK_TO_DIARIES_X, WALK_TO_DIARIES_Y, FACING_NORTHWEST);

	if (previous_room == 103) {
		// Player comes from the East/West Hall (room 103)
		player.x      = START_X_ROOM_103;
		player.y      = START_Y_ROOM_103;
		player.facing = FACING_NORTHWEST;
		seq[fx_door]  = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 5);

	} else if (previous_room != KERNEL_RESTORING_GAME) {
		// Player comes from the King's bedroom (room 101)
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_door], 5);
		player_first_walk(START_X_ROOM_101, START_Y_ROOM_101, FACING_EAST,
		                  WALK_TO_X_FROM_101, WALK_TO_Y_FROM_101, FACING_EAST,
		                  false);
		player_walk_trigger(ROOM_102_DOOR_CLOSES);

	} else {
		// Restoring a saved game
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 5);
	}

	section_1_music();
}

void room_102_pre_parser() {
}

void room_102_daemon() {
	int16 reset_frame;

	// --- Diary 1 reading sequence ---
	if (local->animation_running == DIARY1 && kernel_anim[aa[0]].anim != 0) {
		if (kernel_anim[aa[0]].frame != local->diary_frame) {
			local->diary_frame = kernel_anim[aa[0]].frame;
			reset_frame = -1;

			switch (local->diary_frame) {
			case 6:
				kernel_seq_delete(seq[fx_diary1]);
				kernel_synch(KERNEL_SERIES, seq[fx_diary1], KERNEL_ANIM, aa[0]);
				break;
			case 10:
				sound_play(N_TurnDiaryPage);
				break;
			case 26:
				text_show(10210);
				text_show(10211);
				text_show(10212);
				break;
			}

			if (reset_frame >= 0 && kernel_anim[aa[0]].frame != reset_frame) {
				kernel_reset_animation(aa[0], reset_frame);
				local->diary_frame = reset_frame;
			}
		}
	}

	// --- Diary 2 reading sequence ---
	if (local->animation_running == DIARY2 && kernel_anim[aa[0]].anim != 0) {
		if (kernel_anim[aa[0]].frame != local->diary_frame) {
			local->diary_frame = kernel_anim[aa[0]].frame;
			reset_frame = -1;

			switch (local->diary_frame) {
			case 6:
				kernel_seq_delete(seq[fx_diary2]);
				kernel_synch(KERNEL_SERIES, seq[fx_diary2], KERNEL_ANIM, aa[0]);
				break;
			case 26:
				text_show(10213);
				text_show(10214);
				break;
			}

			if (reset_frame >= 0 && kernel_anim[aa[0]].frame != reset_frame) {
				kernel_reset_animation(aa[0], reset_frame);
				local->diary_frame = reset_frame;
			}
		}
	}

	// --- Door close trigger (fired when player walks in from room 101) ---
	if (kernel.trigger >= ROOM_102_DOOR_CLOSES) {
		switch (kernel.trigger) {
		case ROOM_102_DOOR_CLOSES:
			kernel_seq_delete(seq[fx_door]);
			sound_play(N_DoorCloses);
			seq[fx_door] = kernel_seq_backward(ss[fx_door], false, 9, 0, 0, 1);
			kernel_seq_depth(seq[fx_door], 5);
			kernel_seq_range(seq[fx_door], 1, 4);
			kernel_seq_trigger(seq[fx_door], KERNEL_TRIGGER_EXPIRE, 0,
			                   ROOM_102_DOOR_CLOSES + 1);
			break;

		case ROOM_102_DOOR_CLOSES + 1:
			local->temp  = seq[fx_door];
			seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_door], KERNEL_LAST);
			kernel_synch(KERNEL_SERIES, seq[fx_door], KERNEL_SERIES, local->temp);
			player.commands_allowed = true;
			break;
		}
	}
}

void room_102_parser() {
	if (player.look_around) {
		text_show(10201);
		player.command_ready = false;
		return;
	}

	// --- Walk through / open / pull: door to King's room (room 101) ---
	if (player_parse(37, 245, 0) || player_parse(6, 245, 0) || player_parse(10, 245, 0)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed = false;
			player.walker_visible   = false;
			seq[fx_open_door] = kernel_seq_pingpong(ss[fx_open_door], true, 8, 0, 0, 2);
			kernel_seq_player(seq[fx_open_door], true);
			kernel_seq_trigger(seq[fx_open_door], KERNEL_TRIGGER_SPRITE, 2, 1);
			kernel_seq_trigger(seq[fx_open_door], KERNEL_TRIGGER_EXPIRE, 0, 3);
			break;
		case 1:
			kernel_seq_delete(seq[fx_door]);
			sound_play(N_DoorOpens);
			seq[fx_door] = kernel_seq_forward(ss[fx_door], false, 9, 0, 0, 1);
			kernel_seq_depth(seq[fx_door], 5);
			kernel_seq_trigger(seq[fx_door], KERNEL_TRIGGER_EXPIRE, 0, 2);
			break;
		case 2:
			local->temp  = seq[fx_door];
			seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, 5);
			kernel_seq_depth(seq[fx_door], 5);
			kernel_synch(KERNEL_SERIES, seq[fx_door], KERNEL_SERIES, local->temp);
			break;
		case 3:
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_open_door]);
			player_walk(WALK_TO_DOOR_X, WALK_TO_DOOR_Y, FACING_WEST);
			player.walk_off_edge_to_room = 101;
			break;
		}
		player.command_ready = false;
		return;
	}

	// --- Walk through / open / pull: door to hallway (room 103) ---
	if (player_parse(37, 197, 0) || player_parse(6, 197, 0) || player_parse(10, 197, 0)) {
		new_room = 103;
		player.command_ready = false;
		return;
	}

	// --- Take / open: diaries ---
	if (player_parse(4, 264, 0) || player_parse(6, 264, 0)) {
		switch (kernel.trigger) {
		case 0:
			player.commands_allowed  = false;
			player.walker_visible    = false;
			local->animation_running = DIARY1;
			aa[0] = kernel_run_animation(kernel_name('B', -1), 1);
			kernel_synch(KERNEL_ANIM, aa[0], KERNEL_PLAYER, 0);
			break;
		case 1:
			seq[fx_diary1]   = kernel_seq_stamp(ss[fx_diary1], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_diary1], 12);
			local->diary1_id = kernel_add_dynamic(264, 13, SYNTAX_PLURAL,
			                       seq[fx_diary1], 0, 0, 0, 0);
			kernel_dynamic_walk(local->diary1_id, WALK_TO_DIARIES_X, WALK_TO_DIARIES_Y,
			                    FACING_NORTHWEST);
			player.walker_visible = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
			kernel_timing_trigger(TENTH_SECOND, 2);
			break;
		case 2:
			player_walk(51, 121, FACING_NORTHWEST);
			player_walk_trigger(3);
			break;
		case 3:
			player.walker_visible    = false;
			local->animation_running = DIARY2;
			aa[0] = kernel_run_animation(kernel_name('A', -1), 4);
			kernel_synch(KERNEL_ANIM, aa[0], KERNEL_PLAYER, 0);
			break;
		case 4:
			seq[fx_diary2]   = kernel_seq_stamp(ss[fx_diary2], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_diary2], 12);
			local->diary2_id = kernel_add_dynamic(264, 13, SYNTAX_PLURAL,
			                       seq[fx_diary2], 0, 0, 0, 0);
			kernel_dynamic_walk(local->diary2_id, WALK_TO_DIARIES_X, WALK_TO_DIARIES_Y,
			                    FACING_NORTHWEST);
			player.walker_visible   = true;
			player.commands_allowed = true;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
			break;
		}
		player.command_ready = false;
		return;
	}

	// --- Look / look at ---
	if (player_parse(3, 0) || player_parse(30, 0)) {
		if (player_parse(34, 0)) {
			text_show(10202);
			player.command_ready = false;
			return;
		}
		if (player_parse(21, 0)) {
			text_show(10203);
			player.command_ready = false;
			return;
		}
		if (player_parse(252, 0)) {
			text_show(10204);
			player.command_ready = false;
			return;
		}
		if (player_parse(18, 0)) {
			text_show(10206);
			player.command_ready = false;
			return;
		}
		if (player_parse(253, 0)) {
			text_show(10208);
			player.command_ready = false;
			return;
		}
		if (player_parse(264, 0)) {
			text_show(10209);
			player.command_ready = false;
			return;
		}
		if (player_parse(245, 0)) {
			text_show(10215);
			player.command_ready = false;
			return;
		}
		if (player_parse(251, 0)) {
			text_show(10216);
			player.command_ready = false;
			return;
		}
		if (player_parse(24, 0)) {
			text_show(10217);
			player.command_ready = false;
			return;
		}
		if (player_parse(204, 0)) {
			text_show(10219);
			player.command_ready = false;
			return;
		}
		if (player_parse(35, 0)) {
			text_show(10220);
			player.command_ready = false;
			return;
		}
		if (player_parse(25, 0)) {
			text_show(10222);
			player.command_ready = false;
			return;
		}
		if (player_parse(197, 0)) {
			text_show(10223);
			player.command_ready = false;
			return;
		}
		if (player_parse(23, 0)) {
			text_show(10224);
			player.command_ready = false;
			return;
		}
		if (player_parse(26, 0)) {
			text_show(10226);
			player.command_ready = false;
			return;
		}
		if (player_parse(329, 0)) {
			text_show(10227);
			player.command_ready = false;
			return;
		}
		if (player_parse(16, 0)) {
			text_show(10228);
			player.command_ready = false;
			return;
		}
		if (player_parse(20, 0)) {
			text_show(10229);
			player.command_ready = false;
			return;
		}
		if (player_parse(41, 0)) {
			text_show(10230);
			player.command_ready = false;
			return;
		}
		if (player_parse(196, 0)) {
			text_show(10231);
			player.command_ready = false;
			return;
		}
		return;  // no noun matched inside look — do not clear command_ready
	}

	// --- Open chest ---
	if (player_parse(6, 23, 0)) {
		text_show(10224);
		player.command_ready = false;
		return;
	}

	// --- Take flowers ---
	if (player_parse(4, 251, 0)) {
		text_show(10225);
		player.command_ready = false;
		return;
	}

	// --- Open window ---
	if (player_parse(6, 24, 0)) {
		text_show(10218);
		player.command_ready = false;
		return;
	}

	// --- Push / pull fireplace screen ---
	if ((player_parse(5, 0) || player_parse(10, 0)) && player_parse(35, 0)) {
		text_show(10221);
		player.command_ready = false;
		return;
	}

	// --- Close shutters ---
	if (player_parse(11, 252, 0)) {
		text_show(10205);
		player.command_ready = false;
		return;
	}

	// --- Pull rug ---
	if (player_parse(10, 18, 0)) {
		text_show(10207);
		player.command_ready = false;
		return;
	}
}

void room_102_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(local->diary1_id);
	s.syncAsSint16LE(local->diary2_id);
	s.syncAsSint16LE(local->diary_frame);
	s.syncAsSint16LE(local->animation_running);
	s.syncAsSint16LE(local->temp);
}

void room_102_error() {
}

void room_102_preload() {
	room_init_code_pointer       = room_102_init;
	room_pre_parser_code_pointer = room_102_pre_parser;
	room_parser_code_pointer     = room_102_parser;
	room_daemon_code_pointer     = room_102_daemon;

	section_1_walker();
	section_1_interface();

	vocab_make_active(words_diaries);
	vocab_make_active(words_walk_to);
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
