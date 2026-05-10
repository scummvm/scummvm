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
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/core/player.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/core/vocab.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/rooms/section1.h"
#include "mads/madsv2/dragonsphere/rooms/room104.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[16];            // ss[]  — series handles
	int16 sequence[16];          // seq[] — sequence handles
	int16 animation[6];          // aa[]  — animation handles
	int16 doorway_id;            // 0x4C — dynamic hotspot handle for open doorway
	int16 wall_panel_id;         // 0x4E — dynamic hotspot handle for wall panel
	int16 secret_door_id;        // 0x50 — dynamic hotspot handle for secret door
	int16 tapestry_frame;        // 0x52 — last anim frame seen (tapestry sequence)
	int16 animation_running;     // 0x54 — which tapestry animation is running
	int16 king_frame;            // 0x56 — last anim frame seen (King)
	int16 king_action;           // 0x58 — current King animation state
	int16 king_talk_count;       // 0x5A — King talking counter
	int16 anim_0_running;        // 0x5C — King animation active flag
	int16 mac_frame;             // 0x5E — last anim frame seen (MacMorn)
	int16 mac_action;            // 0x60 — current MacMorn animation state
	int16 mac_talk_count;        // 0x62 — MacMorn talking counter
	int16 anim_1_running;        // 0x64 — MacMorn animation active flag
	int16 queen_frame;           // 0x66 — last anim frame seen (Queen)
	int16 queen_action;          // 0x68 — current Queen animation state
	int16 queen_talk_count;      // 0x6A — Queen talking counter
	int16 anim_2_running;        // 0x6C — Queen animation active flag
	int16 twinkles_frame;        // 0x6E — last anim frame seen (Twinkles)
	int16 twinkles_action;       // 0x70 — current Twinkles animation state
	int16 twinkles_talk_count;   // 0x72 — Twinkles freeze counter
	int16 anim_3_running;        // 0x74 — Twinkles animation active flag
	int16 pid_frame;             // 0x76 — last anim frame seen (Pid)
	int16 pid_action;            // 0x78 — current Pid animation state
	int16 pid_talk_count;        // 0x7A — Pid freeze counter
	int16 anim_4_running;        // 0x7C — Pid animation active flag
	int32 clock;                 // 0x7E — general timer (long in original)
	int32 death_timer;           // 0x82 — counts to 8 sec to kill Pid (long in original)
	int16 activate_timer;        // 0x86 — T = start counting death_timer
	int16 has_been_bear;         // 0x88 — Pid has completed bear transformation
	int16 mac_2_frame;           // 0x8A — last anim frame seen (MacMorn death)
	int16 anim_5_running;        // 0x8C — MacMorn death animation active flag
	int16 amulet_works;          // 0x8E — amulet is functional
	int16 pid_drawn_sword;       // 0x90 — Pid has drawn his sword
	int16 death_frame;           // 0x92 — last anim frame seen (death sequence)
	int16 anim_6_running;        // 0x94 — death animation active flag
};

static Scratch scratch;

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

// Sprite series slot indices (from ROOM104.MAC)
#define fx_fire_left           1   // rm104x0
#define fx_fire_right          2   // rm104x1
#define fx_fire                3   // rm104x2
#define fx_fire_shadow         4   // rm104x3
#define fx_table               5   // rm104x5
#define fx_door                6   // rm104x4
#define fx_open_door           7   // kgrd_8  (king only)
#define fx_pull_push_books     8   // kgrm1_8 (king only)
#define fx_open_doorway        9   // rm104y3 (pid only)
#define fx_tapestry_opened     10  // rm104y4
#define fx_tapestry_closed     11  // rm104y5 (king only)
#define fx_wall_open_close     12  // rm104y1 (king only)
#define fx_sword               13  // rm104b3 (pid only)
#define fx_e5                  14  // rm104e5 (pid only)
#define fx_e3                  15

// Trigger constants
#define ROOM_104_DOOR_CLOSES   70
#define ROOM_104_KING_ENTER    77
#define ROOM_104_RUN_P2        79
#define ROOM_104_YOU_TALK      81
#define ROOM_104_ME_TALK       83
#define ROOM_104_TABLE         85
#define ROOM_104_NEW_DEATH     95

// Walk/position constants (from ROOM104.MAC)
#define START_X_ROOM_103       77
#define START_Y_ROOM_103       93
#define START_X_ROOM_106       201
#define START_Y_ROOM_106       152
#define WALK_TO_X_FROM_103     74
#define WALK_TO_Y_FROM_103     107
#define WALK_TO_DOORWAY_X      295
#define WALK_TO_DOORWAY_Y      145
#define TAP_HS_CLOSED_X        310
#define TAP_HS_CLOSED_Y        70
#define TAP_HS_OPEN_X          290
#define TAP_HS_OPEN_Y          20
#define TABLE_UNFLIPPED_X      140
#define TABLE_UNFLIPPED_Y      107
#define TABLE_FLIPPED_X        139
#define TABLE_FLIPPED_Y        132

// Tapestry interaction zones (x ranges for look responses)
#define ARCHER_TAPESTRY_BEGIN_X  209
#define ARCHER_TAPESTRY_END_X    278
#define CASTLE_TAPESTRY_BEGIN_X  107
#define CASTLE_TAPESTRY_END_X    190
#define CORRECT_TAPESTRY_X       279  // minimum x for interacting with the correct tapestry

// Conversation
#define CONV_FINALE            1

// King animation states
#define KING_SHUT_UP           0
#define KING_TALK              1
#define KING_INVISIBLE         2

// MacMorn animation states
#define MAC_SHUT_UP            0
#define MAC_TALK               1
#define MAC_TABLE              2
#define MAC_UNBEAR             3
#define MAC_DIE                4
#define MAC_LAUGH              5
#define MAC_KILL_NOW           6

// Queen animation states
#define QUEEN_SHUT_UP          0
#define QUEEN_TALK             1
#define QUEEN_LOOK_LEFT        2
#define QUEEN_LOOK_CENTER      3

// Twinkles animation states
#define TWINKLES_SHUT_UP       0
#define TWINKLES_TALK          1
#define TWINKLES_INVISIBLE     2

// Pid animation states
#define PID_SHUT_UP            0
#define PID_TALK               1
#define PID_POINT_TALK         2
#define PID_BEAR               3
#define PID_UNBEAR             4
#define PID_KILL_MACMORN       5
#define PID_DRAW_SWORD         6
#define PID_FALL_NOW           7
#define PID_DIE_NO_SWORD       8

// Tapestry animation identifiers
#define OPEN_TAPESTRY          1
#define CLOSE_TAPESTRY         2

#define LENGTH_OF_LIFE            1300 

// Vocabulary word IDs (raw integers from disassembly; VOCABH.DB/VOCAB.DB)
//   words_MacMorn=350, words_wall_panel=276, words_secret_door=275
//   words_table=198, words_books=274, words_tapestry=26 (✓ verified)
//   words_Queen_Mother=347, words_king=291, words_doorway=277
//   words_walk_through=37 (✓), words_walk_to=13 (✓)

// ---------------------------------------------------------------------------

void room_104_init() {
	conv_get(CONV_FINALE);

	// Palette-only loads for Pid persona (prevent palette fragmentation)
	if (global[player_persona] == PLAYER_IS_PID) {
		kernel_load_series(kernel_name('e', 8), false);
		kernel_load_series(kernel_name('b', 5), false);
	}

	// Flip MacMorn hotspot off to prevent palette fragmentation
	kernel_flip_hotspot(350, false);  // words_MacMorn

	// Zero runtime state on fresh room entry (not when restoring a saved game)
	if (previous_room != KERNEL_RESTORING_GAME) {
		local->animation_running   = 0;
		local->anim_0_running      = 0;
		local->anim_1_running      = 0;
		local->anim_2_running      = 0;
		local->anim_3_running      = 0;
		local->anim_4_running      = 0;
		local->anim_5_running      = 0;
		local->king_talk_count     = 0;
		local->mac_talk_count      = 0;
		local->queen_talk_count    = 0;
		local->twinkles_talk_count = 0;
		local->death_timer         = 0;
		local->clock               = 0;
		local->activate_timer      = 0;
		local->has_been_bear       = 0;
		local->amulet_works        = 0;
		local->pid_drawn_sword     = 0;
	}

	// Always clear death animation flag regardless of save state
	local->anim_6_running = 0;

	// Load the shared fire/door sprite series
	ss[fx_fire_left]   = kernel_load_series(kernel_name('x', 0), false);
	ss[fx_fire_right]  = kernel_load_series(kernel_name('x', 1), false);
	ss[fx_fire]        = kernel_load_series(kernel_name('x', 2), false);
	ss[fx_fire_shadow] = kernel_load_series(kernel_name('x', 3), false);
	ss[fx_table]       = kernel_load_series(kernel_name('x', 5), false);
	ss[fx_door]        = kernel_load_series(kernel_name('x', 4), false);

	// When arriving from the stairway (room 119), bake the door into the
	// background rather than keeping it as an active sprite
	if (global[no_load_walker]) {
		kernel_draw_to_background(ss[fx_door], 5, KERNEL_HOME, KERNEL_HOME, 0, 100);
		matte_deallocate_series(ss[fx_door], true);
	}

	// ---------------------------------------------------------------------------
	// Persona-specific series and hotspot setup
	// ---------------------------------------------------------------------------
	if (global[player_persona] == PLAYER_IS_KING) {

		kernel_flip_hotspot_loc(198, false, TABLE_FLIPPED_X, TABLE_FLIPPED_Y);  // words_table

		ss[fx_open_door]       = kernel_load_series("*KGRD_8", false);
		ss[fx_pull_push_books] = kernel_load_series("*KGRM1_8", false);
		ss[fx_tapestry_closed] = kernel_load_series(kernel_name('y', 5), false);
		ss[fx_tapestry_opened] = kernel_load_series(kernel_name('y', 4), false);
		ss[fx_wall_open_close] = kernel_load_series(kernel_name('y', 1), false);

		// Stamp the correct tapestry sprite based on current state
		if (global[tapestry_status] == TAPESTRY_CLOSED ||
		    global[tapestry_status] == TAPESTRY_CLOSED2) {
			kernel_flip_hotspot(276, false);  // words_wall_panel
			kernel_flip_hotspot(275, false);  // words_secret_door
			seq[fx_tapestry_closed] = kernel_seq_stamp(ss[fx_tapestry_closed], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_tapestry_closed], 6);
		} else {
			// Tapestry is open
			seq[fx_tapestry_opened] = kernel_seq_stamp(ss[fx_tapestry_opened], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_tapestry_opened], 6);
			kernel_flip_hotspot_loc(26, false, TAP_HS_CLOSED_X, TAP_HS_CLOSED_Y);  // words_tapestry
		}

		// Adjust secret door / wall panel hotspots based on books state
		if (global[books_status] == BOOKS_PRESENT ||
		    global[books_status] == BOOKS_PRESENT2) {
			if (global[tapestry_status] == TAPESTRY_OPENED ||
			    global[tapestry_status] == TAPESTRY_OPENED2) {
				kernel_flip_hotspot(275, false);  // words_secret_door
			}
		} else if (global[books_status] == BOOKS_PULLED ||
		           global[books_status] == BOOKS_PULLED2) {
			seq[fx_wall_open_close] = kernel_seq_stamp(ss[fx_wall_open_close], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_wall_open_close], 8);
			if (global[tapestry_status] == TAPESTRY_OPENED ||
			    global[tapestry_status] == TAPESTRY_OPENED2) {
				kernel_flip_hotspot(276, false);   // words_wall_panel
				kernel_flip_hotspot(275, true);    // words_secret_door (true in release binary)
			}
		}

	} else {
		// Pid persona: doorway is the exit, bookcase/tapestry irrelevant
		ss[fx_e5]   = kernel_load_series(kernel_name('e', 5), false);
		kernel_load_series(kernel_name('b', 0), false);  // palette only — not saved
		ss[fx_sword] = kernel_load_series(kernel_name('b', 3), false);

		kernel_flip_hotspot_loc(198, false, TABLE_UNFLIPPED_X, TABLE_UNFLIPPED_Y);  // unflipped
		kernel_flip_hotspot_loc(198, true,  TABLE_FLIPPED_X,   TABLE_FLIPPED_Y);    // mirrored

		ss[fx_tapestry_opened] = kernel_load_series(kernel_name('y', 4), false);
		if (global[no_load_walker]) {
			kernel_draw_to_background(ss[fx_tapestry_opened], 1, KERNEL_HOME, KERNEL_HOME, 0, 100);
			matte_deallocate_series(ss[fx_tapestry_opened], true);
		}

		// The open doorway is the route to the throne room
		ss[fx_open_doorway]  = kernel_load_series(kernel_name('y', 3), false);
		seq[fx_open_doorway] = kernel_seq_stamp(ss[fx_open_doorway], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_open_doorway], 8);
		local->doorway_id = kernel_add_dynamic(277, 37, SYNTAX_SINGULAR,
		                        seq[fx_open_doorway], 0, 0, 0, 0);
		kernel_dynamic_walk(local->doorway_id, WALK_TO_DOORWAY_X, WALK_TO_DOORWAY_Y,
		                    FACING_NORTHEAST);

		global[books_status] = BOOKS_NOT_PRESENT;
		kernel_flip_hotspot_loc(26, false, TAP_HS_CLOSED_X, TAP_HS_CLOSED_Y);  // words_tapestry
	}

	// Hide the books hotspot if books are gone
	if (global[books_status] == BOOKS_NOT_PRESENT) {
		kernel_flip_hotspot(274, false);  // words_books
	}

	// ---------------------------------------------------------------------------
	// Start continuous fire sequences
	// ---------------------------------------------------------------------------
	seq[fx_fire_left]   = kernel_seq_forward(ss[fx_fire_left],   false, 7, 5, 0, 0);
	seq[fx_fire_right]  = kernel_seq_forward(ss[fx_fire_right],  false, 7, 0, 0, 0);
	seq[fx_fire]        = kernel_seq_forward(ss[fx_fire],        false, 7, 0, 0, 0);
	kernel_seq_depth(seq[fx_fire], 3);
	seq[fx_fire_shadow] = kernel_seq_forward(ss[fx_fire_shadow], false, 7, 0, 0, 0);

	// King persona: stamp table at initial position
	if (global[player_persona] == PLAYER_IS_KING) {
		seq[fx_table] = kernel_seq_stamp(ss[fx_table], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_table], 4);
	}

	// ---------------------------------------------------------------------------
	// Previous-room entry branches
	// ---------------------------------------------------------------------------
	if (previous_room == 106) {
		// Arriving from the Throne Room
		player.x      = START_X_ROOM_106;
		player.y      = START_Y_ROOM_106;
		player.facing = FACING_NORTHWEST;
		seq[fx_door]  = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 14);

	} else if (previous_room == 119 || previous_room == 104) {
		// Arriving from the Stairway (finale entry): King, Queen, and Pid
		// are already in the room; trigger their entrance animations
		player.walker_visible   = false;
		player.commands_allowed = false;

		seq[fx_table] = kernel_seq_stamp(ss[fx_table], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_table], 4);

		kernel_timing_trigger(ONE_SECOND, ROOM_104_KING_ENTER);

		aa[0]                 = kernel_run_animation(kernel_name('k', 1), 0);
		local->anim_0_running = true;
		local->king_action    = KING_INVISIBLE;
		kernel_reset_animation(aa[0], 22);

		aa[2]                 = kernel_run_animation(kernel_name('q', 1), 0);
		local->anim_2_running = true;
		local->queen_action   = QUEEN_SHUT_UP;

		seq[fx_e5] = kernel_seq_stamp(ss[fx_e5], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_e5], 6);

		aa[4] = kernel_run_animation(kernel_name('p', 1), ROOM_104_RUN_P2);

		{
			int16 id = kernel_add_dynamic(347, 13, SYNTAX_FEM_NOT_PROPER,
			                              KERNEL_NONE, 0, 0, 0, 0);  // words_Queen_Mother
			kernel_dynamic_hot[id].prep = PREP_ON;
			kernel_dynamic_anim(id, aa[2], 0);
		}

		kernel_flip_hotspot(350, true);   // words_MacMorn — visible now

		{
			int16 id = kernel_add_dynamic(291, 13, SYNTAX_MASC_NOT_PROPER,
			                              KERNEL_NONE, 0, 0, 0, 0);  // words_king
			kernel_dynamic_hot[id].prep = PREP_ON;
			kernel_dynamic_anim(id, aa[0], 0);
		}

	} else if (previous_room != KERNEL_RESTORING_GAME) {
		// Normal arrival from the East/West Hall (room 103)
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_door], 14);
		player_first_walk(START_X_ROOM_103, START_Y_ROOM_103, FACING_SOUTH,
		                  WALK_TO_X_FROM_103, WALK_TO_Y_FROM_103, FACING_SOUTH,
		                  false);
		player_walk_trigger(ROOM_104_DOOR_CLOSES);

	} else {
		// Restoring a saved game
		if (global[no_load_walker]) {
			// Restoring into the finale scene (Pid+King+Queen+MacMorn all present)
			player.walker_visible   = false;
			player.commands_allowed = false;

			seq[fx_table] = kernel_seq_stamp(ss[fx_table], false, KERNEL_LAST);
			kernel_seq_depth(seq[fx_table], 1);

			aa[0]                 = kernel_run_animation(kernel_name('k', 1), 0);
			local->anim_0_running = true;
			local->king_action    = KING_SHUT_UP;
			kernel_reset_animation(aa[0], 14);

			aa[2]                 = kernel_run_animation(kernel_name('q', 1), 0);
			local->anim_2_running = true;
			local->queen_action   = QUEEN_SHUT_UP;

			aa[4]                 = kernel_run_animation(kernel_name('p', 2), 0);
			local->anim_4_running = true;
			local->pid_action     = PID_SHUT_UP;
			if (local->amulet_works) {
				kernel_reset_animation(aa[4], 89);
			}

			aa[1]                 = kernel_run_animation(kernel_name('m', 1), 0);
			local->anim_1_running = true;
			local->mac_action     = MAC_SHUT_UP;
			kernel_reset_animation(aa[1], 25);

			{
				int16 id = kernel_add_dynamic(347, 13, SYNTAX_FEM_NOT_PROPER,
				                              KERNEL_NONE, 0, 0, 0, 0);  // words_Queen_Mother
				kernel_dynamic_hot[id].prep = PREP_ON;
				kernel_dynamic_anim(id, aa[2], 0);
			}

			kernel_flip_hotspot(350, true);   // words_MacMorn — visible

			{
				int16 id = kernel_add_dynamic(291, 13, SYNTAX_MASC_NOT_PROPER,
				                              KERNEL_NONE, 0, 0, 0, 0);  // words_king
				kernel_dynamic_hot[id].prep = PREP_ON;
				kernel_dynamic_anim(id, aa[0], 0);
			}

			if (conv_restore_running == CONV_FINALE) {
				player.commands_allowed = false;
				conv_run(CONV_FINALE);
				conv_export_value(0);            // placeholder
				conv_export_value(0);            // drew sword
				conv_export_value(0);            // placeholder
				if (global[llanie_status] != IS_SAVED) {
					conv_export_value(1);
				} else {
					conv_export_value(0);
				}
			} else {
				local->activate_timer   = true;
				local->death_timer      = 0;
				local->clock            = 0;
				player.commands_allowed = true;
			}

		} else {
			// Restoring a normal King session
			seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_door], 12);
		}
	}

	section_1_music();
}

// Dispatch table for ROOM_104_YOU_TALK: player_verb → character action block
//   mac-talks  (BA30): 0,2,4,5,7,8,11,17
//   king-talks (BA46): 1,3,6,20,22,24,26,28,32,34,39,41,43
//   queen-talks(BA72): 21,23,25,27,35,40,42
//   twinkles   (BA88): 31,33
//   (no-op)   (BB2B): all other entries ≤ 43
static void room_104_process_conversation_finale() {
	int16 you_trig_flag = 0;
	int16 me_trig_flag  = 0;

	// Player-choice (b_b) node dispatch
	switch (player_verb) {
	case 38:  // conv001_end_b_b
		global[end_of_game] = true;
		new_room = 106;
		break;

	case 30:  // conv001_pre_25_b_b
		*conv_my_next_start = 31;  // conv001_twentyfive
		conv_abort();
		if (global[llanie_status] == IS_SAVED) {
			aa[3]                  = kernel_run_animation(kernel_name('l', 1), 0);
			local->anim_3_running  = true;
			local->twinkles_action = TWINKLES_TALK;
		}
		break;

	case 10:  // conv001_exit_b_b
		if (!kernel.trigger)
			local->mac_action = MAC_TABLE;
		me_trig_flag  = true;
		you_trig_flag = true;
		break;

	case 13:  // conv001_bear_b_b
		local->pid_action = PID_BEAR;
		conv_hold();
		me_trig_flag  = true;
		you_trig_flag = true;
		break;

	case 16:  // conv001_unbear_b_b
		if (!kernel.trigger)
			local->mac_action = MAC_UNBEAR;
		me_trig_flag  = true;
		you_trig_flag = true;
		break;

	case 19:  // conv001_sixteen_b_b
		local->death_timer    = 0;
		local->clock          = 0;
		local->activate_timer = true;
		me_trig_flag  = true;
		you_trig_flag = true;
		break;
	}

	// Trigger dispatch
	switch (kernel.trigger) {
	case ROOM_104_YOU_TALK:  // 81
		// Per-node character action: which character speaks this NPC line?
		switch (player_verb) {
		// Mac talks (if idle, set to talking)
		case 0: case 2: case 4: case 5: case 7: case 8: case 11: case 17:
			if (local->mac_action == MAC_SHUT_UP)
				local->mac_action = MAC_TALK;
			local->pid_action = PID_SHUT_UP;
			if (local->queen_action != QUEEN_LOOK_LEFT)
				local->queen_action = QUEEN_SHUT_UP;
			local->king_action = KING_SHUT_UP;
			break;

		// King talks
		case 1: case 3: case 6: case 20: case 22: case 24: case 26: case 28:
		case 32: case 34: case 39: case 41: case 43:
			local->king_action = KING_TALK;
			local->pid_action  = PID_SHUT_UP;
			if (local->queen_action != QUEEN_LOOK_LEFT)
				local->queen_action = QUEEN_SHUT_UP;
			if (local->mac_action == MAC_TALK)
				local->mac_action = MAC_SHUT_UP;
			break;

		// Queen talks
		case 21: case 23: case 25: case 27: case 35: case 40: case 42:
			if (local->queen_action != QUEEN_LOOK_LEFT)
				local->queen_action = QUEEN_TALK;
			local->pid_action = PID_SHUT_UP;
			if (local->mac_action == MAC_TALK)
				local->mac_action = MAC_SHUT_UP;
			local->king_action = KING_SHUT_UP;
			break;

		// Twinkles talks
		case 31: case 33:
			local->twinkles_action = TWINKLES_TALK;
			local->pid_action      = PID_SHUT_UP;
			if (local->queen_action != QUEEN_LOOK_LEFT)
				local->queen_action = QUEEN_SHUT_UP;
			if (local->mac_action == MAC_TALK)
				local->mac_action = MAC_SHUT_UP;
			local->king_action = KING_SHUT_UP;
			break;
		}
		break;

	case ROOM_104_ME_TALK:  // 83
		// Nodes 8 and 11 (conv001_thirteen_only / conv001_fourteen_only) use point gesture
		if (player_verb == 8 || player_verb == 11)
			local->pid_action = PID_POINT_TALK;
		else
			local->pid_action = PID_TALK;
		if (local->mac_action == MAC_TALK)
			local->mac_action = MAC_SHUT_UP;
		if (local->queen_action != QUEEN_LOOK_LEFT)
			local->queen_action = QUEEN_SHUT_UP;
		local->king_action = KING_SHUT_UP;
		break;
	}

	if (!me_trig_flag)  conv_me_trigger(ROOM_104_ME_TALK);
	if (!you_trig_flag) conv_you_trigger(ROOM_104_YOU_TALK);

	local->king_talk_count     = 0;
	local->queen_talk_count    = 0;
	local->mac_talk_count      = 0;
	local->twinkles_talk_count = 0;
	local->pid_talk_count      = 0;
}

void room_104_pre_parser() {
	// verb(pull/open/close/push) AND tapestry: redirect walk to tapestry area when
	// the interaction point is in the right side of the room (inter_point_x > 280)
	if ((player_parse(10, 0) || player_parse(6, 0) || player_parse(11, 0) || player_parse(5, 0)) &&
	     player_parse(26, 0)) {  // tapestry
		if (inter_point_x > 280) {
			player_walk(WALK_TO_DOORWAY_X, WALK_TO_DOORWAY_Y, FACING_NORTHEAST);
		}
	}

	// For Pid persona, cancel any walk when the player tries actions that require
	// cutscene handling rather than walking
	if (player_parse(37, 176, 0) ||    // walk_through + door_to_throne_room
	    player_parse(37, 197, 0) ||    // walk_through + door_to_hallway
	    player_parse(37, 277, 0) ||    // walk_through + doorway
	    ((player_parse(10, 0) || player_parse(6, 0) || player_parse(5, 0)) && player_parse(26, 0)) ||   // pull/open/push + tapestry
	    ((player_parse(10, 0) || player_parse(11, 0) || player_parse(5, 0)) && player_parse(26, 0)) ||  // pull/close/push + tapestry
	    ((player_parse(5, 0) || player_parse(10, 0)) && player_parse(18, 0)) ||  // push/pull + rug
	    player_parse(35, 0) ||         // fireplace_screen
	    player_parse(200, 0) ||        // trophy
	    player_parse(203, 0) ||        // loveseat
	    player_parse(6, 201, 0)) {     // open + reading_bench
		if (global[player_persona] == PLAYER_IS_PID) {
			player_cancel_walk();
		}
	}

	// In the finale scene (no_load_walker), suppress the walk entirely
	if (global[no_load_walker]) {
		player.need_to_walk = false;
	}
}

static void handle_animation_king() {
	int king_reset_frame;

	if (kernel_anim[aa[0]].frame != local->king_frame) {
		local->king_frame = kernel_anim[aa[0]].frame;
		king_reset_frame = -1;

		switch (local->king_frame) {

		case 22:
			if (local->king_action == KING_INVISIBLE) {
				king_reset_frame = 21;
			} else {
				king_reset_frame = 0;
			}
			break;

		case 23:
			break;

		case 14:  /* end of enter room and freeze */
		case 21:  /* end of talk                  */
			switch (local->king_action) {
			case KING_TALK:
				king_reset_frame = 14;  /* go to talk (new node) */
				break;

			case KING_SHUT_UP:
				if (local->king_talk_count > imath_random(40, 50)) {
					local->king_talk_count = 0;
					if (imath_random(1, 2) == 1) {
						king_reset_frame = 13;    /* freeze */
					} else {
						king_reset_frame = 14;    /* freeze (new node) */
					}
				} else {
					++local->king_talk_count;
					king_reset_frame = 13;      /* remain freezing in this node */
				}
				break;
			}
			break;

		case 16: /* end of come to talk and freeze */
		case 17: /* end of talk frame 1            */
		case 18: /* end of talk frame 2            */
		case 19: /* end of talk frame 3            */
			switch (local->king_action) {
			case KING_TALK:
				king_reset_frame = imath_random(16, 18);
				++local->king_talk_count;
				if (local->king_talk_count > 15) {
					local->king_action = KING_SHUT_UP;
					local->king_talk_count = 0;
					king_reset_frame = 15; /* freeze */
				}
				break;

			case KING_SHUT_UP:
				if (local->king_talk_count > imath_random(40, 50)) {
					local->king_talk_count = 0;
					if (imath_random(1, 2) == 1) {
						king_reset_frame = 15;    /* freeze */
					} else {
						king_reset_frame = 19;    /* freeze (new node) */
					}

				} else {
					++local->king_talk_count;
					king_reset_frame = 15;      /* remain freezing in this node */
				}
				break;
			}
			break;
		}

		if (king_reset_frame >= 0) {
			kernel_reset_animation(aa[0], king_reset_frame);
			local->king_frame = king_reset_frame;
		}
	}
}


static void handle_animation_mac() {
	int mac_reset_frame;

	if (kernel_anim[aa[1]].frame != local->mac_frame) {
		local->mac_frame = kernel_anim[aa[1]].frame;
		mac_reset_frame = -1;

		switch (local->mac_frame) {

		case 25:  /* almost end of Macmorn drawing sword */
			player.commands_allowed = true;
			break;

		case 162:  /* end of laugh before killing pid */
			conv_release();  /* Mac will say 'pid looks like a fool lifting his sword' */
			break;

		case 26:  /* end of draw sword and freeze */
		case 90:  /* end of de-bear               */
		case 174: /* end of laugh before fight    */
			switch (local->mac_action) {
			case MAC_KILL_NOW:
				local->pid_frame = 105;
				mac_reset_frame = 26;
				kernel_reset_animation(aa[4], 105);     /* make pid invisible */
				kernel_synch(KERNEL_ANIM, aa[4], KERNEL_ANIM, aa[1]);
				break;

			case MAC_DIE:
				break;

			case MAC_LAUGH:
				mac_reset_frame = 147;    /* MacMorn laughs at pid */
				local->mac_action = MAC_SHUT_UP;
				break;

			case MAC_UNBEAR:
				mac_reset_frame = 76;    /* turn pid into human again */
				local->mac_action = MAC_SHUT_UP;
				player.commands_allowed = false;
				break;

			default:
				mac_reset_frame = 25;  /* freeze macmorn with sword in hand */
				break;
			}
			break;

		case 36:
			seq[fx_sword] = kernel_seq_stamp(ss[fx_sword], false, KERNEL_FIRST);
			kernel_seq_depth(seq[fx_sword], 13);
			break;

		case 37:
			kernel_reset_animation(aa[4], 89);
			local->pid_frame = 89;
			local->pid_action = PID_FALL_NOW;  /* Pid falls to ground */
			kernel_synch(KERNEL_ANIM, aa[4], KERNEL_ANIM, aa[1]);
			break;

		case 112: /* end of throw table */
			mac_reset_frame = 8;    /* draw sword */
			local->mac_action = MAC_SHUT_UP;
			break;

		case 1:   /* end of talk and freeze      */
		case 2:   /* end of talk                 */
		case 7:   /* end of talk from other node */
			switch (local->mac_action) {
			case MAC_TALK:
				if (local->mac_talk_count == 0) {
					if (imath_random(1, 2) == 1) {
						mac_reset_frame = 2;  /* go to talk (new node) */
					} else {
						mac_reset_frame = 1;  /* talk */
					}
					++local->mac_talk_count;

				} else {
					mac_reset_frame = imath_random(0, 1);
					++local->mac_talk_count;
					if (local->mac_talk_count > 15) {
						local->mac_action = MAC_SHUT_UP;
						local->mac_talk_count = 0;
						mac_reset_frame = 0; /* freeze */
					}
				}
				break;

			case MAC_SHUT_UP:
				mac_reset_frame = 0;    /* freeze */
				break;

			case MAC_TABLE:
				mac_reset_frame = 90;    /* throw table (he's pissed) */
				break;
			}
			break;

		case 3: /* end of come to talk */
		case 4: /* end of talk frame 1 */
		case 5: /* end of talk frame 2 */
			switch (local->mac_action) {
			case MAC_TALK:
				if (local->mac_talk_count == 0) {
					if (imath_random(1, 2) == 1) {
						mac_reset_frame = 5;  /* go to talk (new node) */
					} else {
						mac_reset_frame = 3;  /* talk */
					}
					++local->mac_talk_count;

				} else {
					mac_reset_frame = imath_random(3, 4);
					++local->mac_talk_count;
					if (local->mac_talk_count > 15) {
						local->mac_action = MAC_SHUT_UP;
						local->mac_talk_count = 0;
						mac_reset_frame = 5; /* freeze */
					}
				}
				break;

			case MAC_SHUT_UP:
			case MAC_TABLE:
			case MAC_UNBEAR:
			case MAC_DIE:
				mac_reset_frame = 5;    /* return to other node */
				break;
			}
			break;

		case 76:   /* end of MacMorn laughing over Pid's death */
			conv_reset(CONV_FINALE);
			text_show(10467);
			global[no_load_walker] = false;
			new_room = 119;
			break;

		case 82:  /* part way into un-bearing pid */
			local->pid_action = PID_UNBEAR;
			break;

		case 97:  /* somewhere into flipping table */
			local->activate_timer = true;
			kernel_seq_delete(seq[fx_table]);
			seq[fx_table] = kernel_seq_forward(ss[fx_table], false, 10, 0, 0, 1);
			kernel_seq_depth(seq[fx_table], 1);
			kernel_seq_range(seq[fx_table], KERNEL_FIRST, KERNEL_LAST);
			kernel_seq_trigger(seq[fx_table],
				KERNEL_TRIGGER_EXPIRE, 0, ROOM_104_TABLE);
			break;
		}

		if (mac_reset_frame >= 0) {
			kernel_reset_animation(aa[1], mac_reset_frame);
			local->mac_frame = mac_reset_frame;
		}
	}
}


static void handle_animation_mac_2() {
	int mac_reset_frame;

	if (kernel_anim[aa[1]].frame != local->mac_frame) {
		local->mac_frame = kernel_anim[aa[1]].frame;
		mac_reset_frame = -1;

		switch (local->mac_frame) {
		case 1:  /* almost end of Macmorn drawing sword */
			if (local->mac_action != MAC_DIE) {
				mac_reset_frame = 0;
			}
			break;

		case 2:
			kernel_seq_delete(seq[fx_e5]);
			kernel_synch(KERNEL_SERIES, seq[fx_e5], KERNEL_ANIM, aa[1]);
			break;

		case 37:
			kernel_abort_animation(aa[1]);
			matte_deallocate_series(ss[fx_e3], true);
			local->anim_5_running = false;
			mac_reset_frame = -1;

			text_show(10426);
			sound_play(N_QueenMother);
			conv_run(CONV_FINALE);
			conv_export_value(0);
			conv_export_value(0);
			conv_export_value(1);  /* used amulet */
			if (global[llanie_status] != IS_SAVED) {
				conv_export_value(1);
			} else {
				conv_export_value(0);
			}
			break;
		}

		if (mac_reset_frame >= 0) {
			kernel_reset_animation(aa[1], mac_reset_frame);
			local->mac_frame = mac_reset_frame;
		}
	}
}

static void handle_animation_queen() {
	int queen_reset_frame;

	if (kernel_anim[aa[2]].frame != local->queen_frame) {
		local->queen_frame = kernel_anim[aa[2]].frame;
		queen_reset_frame = -1;

		switch (local->queen_frame) {
		case 1:   /* end of freeze       */
		case 8:   /* end of talk 1       */
		case 15:  /* end of put hands up */
		case 24:  /* end of talk 2       */
			switch (local->queen_action) {
			case QUEEN_TALK:
				if (imath_random(1, 2) == 1) {
					queen_reset_frame = 1;    /* talk 1 */
				} else {
					queen_reset_frame = 15;   /* talk 2 */
				}
				local->queen_action = QUEEN_SHUT_UP;
				break;

			case QUEEN_SHUT_UP:
				if (local->queen_talk_count > imath_random(40, 50)) {
					local->queen_talk_count = 0;
					if (imath_random(1, 2) == 1) {
						queen_reset_frame = 0;    /* freeze */
					} else {
						queen_reset_frame = 8;    /* freeze (new node) */
					}

				} else {
					++local->queen_talk_count;
					queen_reset_frame = 0;      /* remain freezing in this node */
				}
				break;

			case QUEEN_LOOK_LEFT:
				queen_reset_frame = 8;        /* put hands down to look left */
				break;
			}
			break;

		case 12:  /* end of put hands down */
		case 29:  /* end of look center    */
			switch (local->queen_action) {
			case QUEEN_TALK:
				queen_reset_frame = 12;    /* talk (new node) */
				break;

			case QUEEN_SHUT_UP:
				if (local->queen_talk_count > imath_random(40, 50)) {
					local->queen_talk_count = 0;
					if (imath_random(1, 2) == 1) {
						queen_reset_frame = 11;    /* freeze */
					} else {
						queen_reset_frame = 12;    /* freeze (new node) */
					}

				} else {
					++local->queen_talk_count;
					queen_reset_frame = 11;      /* remain freezing in this node */
				}
				break;

			case QUEEN_LOOK_LEFT:
				queen_reset_frame = 24;        /* put hands down to look left */
				break;
			}
			break;

		case 27:  /* end of look left */
			switch (local->queen_action) {
			case QUEEN_TALK:
			case QUEEN_SHUT_UP:
				queen_reset_frame = 27;    /* go to new node */
				break;

			case QUEEN_LOOK_LEFT:
				queen_reset_frame = 26;    /* keep looking left */
				break;

			case QUEEN_LOOK_CENTER:
				queen_reset_frame = 27;    /* look center (new node) */
				local->queen_action = QUEEN_SHUT_UP;
				break;
			}
			break;
		}

		if (queen_reset_frame >= 0) {
			kernel_reset_animation(aa[2], queen_reset_frame);
			local->queen_frame = queen_reset_frame;
		}
	}
}

static void handle_animation_twinkles() {
	int twinkles_reset_frame;

	if (kernel_anim[aa[3]].frame != local->twinkles_frame) {
		local->twinkles_frame = kernel_anim[aa[3]].frame;
		twinkles_reset_frame = -1;

		switch (local->twinkles_frame) {

		case 1:   /* keep her invisible behind wall */
			if (local->twinkles_action == TWINKLES_INVISIBLE) {
				twinkles_reset_frame = 0;
			}
			break;

		case 10:
			local->queen_action = QUEEN_LOOK_LEFT;
			break;

		case 28:   /* Just as she comes into room, have her speak */
			conv_run(CONV_FINALE);
			conv_export_value(0);
			conv_export_value(0);
			conv_export_value(1);  /* used amulet */
			if (global[llanie_status] != IS_SAVED) {
				conv_export_value(1);
			} else {
				conv_export_value(0);
			}
			break;

		case 36:
			local->queen_action = QUEEN_LOOK_CENTER;
			break;

		case 37:  /* end of coming into room and freeze */
		case 38:  /* end of freeze 2                    */
		case 47:  /* end of talk                        */

			switch (local->twinkles_action) {

			case TWINKLES_TALK:
				twinkles_reset_frame = 38;  /* talk */
				local->twinkles_action = TWINKLES_SHUT_UP;
				break;

			case TWINKLES_SHUT_UP:
				if (local->twinkles_frame != 37 && local->twinkles_frame != 38) {
					local->twinkles_frame = 38;
				}

				if (local->twinkles_talk_count > imath_random(40, 50)) {
					local->twinkles_talk_count = 0;
					if (imath_random(1, 2) == 1) {
						twinkles_reset_frame = 36;    /* freeze */
					} else {
						twinkles_reset_frame = 37;    /* freeze */
					}

				} else {
					++local->twinkles_talk_count;
					twinkles_reset_frame = local->twinkles_frame - 1; /* remain frozen */
				}
				break;
			}
			break;
		}

		if (twinkles_reset_frame >= 0) {
			kernel_reset_animation(aa[3], twinkles_reset_frame);
			local->twinkles_frame = twinkles_reset_frame;
		}
	}
}


static void handle_animation_death() {
	int death_reset_frame;

	if (kernel_anim[aa[4]].frame != local->death_frame) {
		local->death_frame = kernel_anim[aa[4]].frame;
		death_reset_frame = -1;

		switch (local->death_frame) {

		case 11:
			global_speech_go(7); /* hurl */
			break;

		case 17:
			global_speech_go(6); /* stab */
			break;
		}

		if (death_reset_frame >= 0) {
			kernel_reset_animation(aa[4], death_reset_frame);
			local->death_frame = death_reset_frame;
		}
	}
}

static void handle_animation_pid() {
	int pid_reset_frame;

	if (kernel_anim[aa[4]].frame != local->pid_frame) {
		local->pid_frame = kernel_anim[aa[4]].frame;
		pid_reset_frame = -1;

		switch (local->pid_frame) {
		case 60: /* send to 108 to make amulet glow */
			pid_reset_frame = 107;
			break;

		case 111: /* send to 61 to make amulet attack */
			pid_reset_frame = 60;
			break;

		case 62:  /* now have macmorn die */
			local->mac_action = MAC_DIE;
			break;

		case 46:   /* almost at end of un-bear */
			local->activate_timer = true;
			local->clock = 0;
			local->death_timer = 0;
			player.commands_allowed = true;
			break;

		case 1:   /* end of freeze and talk */
		case 2:   /* end of talk            */
		case 3:   /* end of talk            */
		case 81:  /* end of killing macmorn */
		case 9:   /* end of talk            */
		case 47:  /* end of debear          */
			switch (local->pid_action) {
			case PID_TALK:
				pid_reset_frame = imath_random(0, 2);
				++local->pid_talk_count;
				if (local->pid_talk_count > 20) {
					local->pid_action = PID_SHUT_UP;
					local->pid_talk_count = 0;
					pid_reset_frame = 0; /* freeze */
				}
				break;

			case PID_POINT_TALK:
				pid_reset_frame = 3;    /* point and talk (new node) */
				break;

			case PID_DIE_NO_SWORD:
				kernel_abort_animation(aa[1]);
				kernel_abort_animation(aa[4]);
				local->anim_1_running = false;
				local->anim_4_running = false;
				aa[4] = kernel_run_animation(kernel_name('d', 2), ROOM_104_NEW_DEATH);
				local->anim_6_running = true;
				kernel_synch(KERNEL_ANIM, aa[4], KERNEL_NOW, 0);
				break;

			case PID_BEAR:
				local->activate_timer = false;
				pid_reset_frame = 10;   /* turn into bear */
				break;

			case PID_SHUT_UP:
				pid_reset_frame = 0; /* freeze */
				break;

			case PID_KILL_MACMORN:
				player.commands_allowed = false;
				local->activate_timer = false;
				pid_reset_frame = 47;   /* hold out amulet to kill MacMorn */
				local->pid_action = PID_SHUT_UP;
				break;

			case PID_DRAW_SWORD:
				pid_reset_frame = 81;   /* draw sword (pid will die) */
				break;
			}
			break;

		case 4:  /* end of coming to talk */
		case 5:  /* end of talk 1         */
		case 6:  /* end of talk 2         */
		case 7:  /* end of talk 3         */
			if (local->pid_action == PID_POINT_TALK) {
				pid_reset_frame = imath_random(4, 6);
				++local->pid_talk_count;
				if (local->pid_talk_count > 20) {
					local->pid_action = PID_SHUT_UP;
					local->pid_talk_count = 0;
					pid_reset_frame = 7; /* freeze */
				}

			} else {
				pid_reset_frame = 7; /* go to other node */
			}
			break;

		case 24:  /* macmorn will say 'we can deal with shapechangers' */
			conv_release();
			break;

		case 25:  /* end of turning into bear */
			if (local->pid_action == PID_UNBEAR) {
				pid_reset_frame = 25;
				local->pid_action = PID_SHUT_UP;
			} else {
				pid_reset_frame = 24;  /* stay a bear */
			}
			break;

		case 88:
			if (local->death_timer < LENGTH_OF_LIFE) {
				local->mac_action = MAC_LAUGH;
				/* if we did not time out, then player said 'attack macmorn' */
			}
			break;

		case 89:   /* pid holding sword up */
			if (local->pid_action == PID_KILL_MACMORN) {
				pid_reset_frame = 55; /* whip out amulet and blast macmorn */
				local->pid_action = PID_SHUT_UP;

			} else if (local->death_timer >= LENGTH_OF_LIFE) {
				local->mac_action = MAC_KILL_NOW;

			} else {
				local->pid_drawn_sword = true;
				pid_reset_frame = 88;  /* keep pid holding sword up */
			}
			break;

		case 91:
			global_speech_go(7); /* hurl */
			break;

		case 95:
			global_speech_go(6); /* stab */
			break;

		case 105:  /* end of pid laying on ground */
			pid_reset_frame = 104;  /* keep pid laying on ground */
			break;

		case 106: /* end of invisible */
			if (local->pid_action == PID_FALL_NOW) {
				pid_reset_frame = 89;   /* Pid fall now   */
			} else {
				pid_reset_frame = 105;  /* stay invisible */
			}
			break;
		}

		if (pid_reset_frame >= 0) {
			kernel_reset_animation(aa[4], pid_reset_frame);
			local->pid_frame = pid_reset_frame;
		}
	}
}

void room_104_daemon(void) {
	int reset_frame;
	int temp;  /* for synching purposes */
	long dif;  /* for timer stuff       */

	if (local->anim_0_running) {
		handle_animation_king();
	}

	if (local->anim_1_running) {
		handle_animation_mac();
	}

	if (local->anim_2_running) {
		handle_animation_queen();
	}

	if (local->anim_3_running) {
		handle_animation_twinkles();
	}

	if (local->anim_4_running) {
		handle_animation_pid();
	}

	if (local->anim_5_running) {
		handle_animation_mac_2();
	}

	if (local->anim_6_running) {
		handle_animation_death();
	}

	if ((local->animation_running == OPEN_TAPESTRY) && (kernel_anim[aa[0]].anim != NULL)) {
		if (kernel_anim[aa[0]].frame != local->tapestry_frame) {
			local->tapestry_frame = kernel_anim[aa[0]].frame;
			reset_frame = -1;

			switch (local->tapestry_frame) {
				/* make player appear */
			case 13:
				player.walker_visible = true;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
				break;
			}

			if (reset_frame >= 0) {
				if (reset_frame != kernel_anim[aa[0]].frame) {
					kernel_reset_animation(aa[0], reset_frame);
					local->tapestry_frame = reset_frame;
				}
			}
		}
	}

	/* Close the door when player enters */

	switch (kernel.trigger) {
	case ROOM_104_DOOR_CLOSES:
		kernel_seq_delete(seq[fx_door]);
		sound_play(N_DoorCloses);
		seq[fx_door] = kernel_seq_backward(ss[fx_door], false, 6, 0, 0, 1);
		kernel_seq_depth(seq[fx_door], 14);
		kernel_seq_range(seq[fx_door], 1, 4);
		kernel_seq_trigger(seq[fx_door],
			KERNEL_TRIGGER_EXPIRE, 0, ROOM_104_DOOR_CLOSES + 1);
		break;

	case ROOM_104_DOOR_CLOSES + 1:
		temp = seq[fx_door];
		seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
		kernel_seq_depth(seq[fx_door], 14);
		kernel_synch(KERNEL_SERIES, seq[fx_door], KERNEL_SERIES, temp);
		player.commands_allowed = true;
		break;
	}

	if (kernel.trigger == ROOM_104_KING_ENTER) {
		local->king_action = KING_SHUT_UP;
	}

	if (kernel.trigger == ROOM_104_RUN_P2) {

		kernel_abort_animation(aa[4]);

		aa[4] = kernel_run_animation(kernel_name('p', 2), 0);
		local->anim_4_running = true;
		local->pid_action = PID_SHUT_UP;
		kernel_synch(KERNEL_ANIM, aa[4], KERNEL_NOW, 0);

		kernel_seq_delete(seq[fx_e5]);

		aa[1] = kernel_run_animation(kernel_name('m', 1), 0);
		local->anim_1_running = true;
		local->mac_action = MAC_SHUT_UP;

		player.commands_allowed = false;
		conv_run(CONV_FINALE);
		conv_export_value(0);
		conv_export_value(0);
		conv_export_value(0);
		if (global[llanie_status] != IS_SAVED) {
			conv_export_value(1);
		} else {
			conv_export_value(0);
		}
	}

	if (kernel.trigger == ROOM_104_TABLE) {
		sound_play(N_McMornTipsTable);
		seq[fx_table] = kernel_seq_stamp(ss[fx_table], false, KERNEL_LAST);
		kernel_seq_depth(seq[fx_table], 1);
	}

	if (local->activate_timer) {
		dif = kernel.clock - local->clock;
		if ((dif >= 0) && (dif <= 4)) {
			local->death_timer += dif;
		} else {
			local->death_timer += 1;
		}
		local->clock = kernel.clock;

		if (local->death_timer >= LENGTH_OF_LIFE) {
			local->activate_timer = false;
			if (local->pid_drawn_sword) {
				local->pid_action = PID_DRAW_SWORD;
				conv_run(CONV_FINALE);
				conv_export_value(0);
				conv_export_value(1);  /* drew sword */
				conv_export_value(0);
				if (global[llanie_status] != IS_SAVED) {
					conv_export_value(1);
				} else {
					conv_export_value(0);
				}
				conv_hold();

			} else {
				local->pid_action = PID_DIE_NO_SWORD;
			}
			player.commands_allowed = false;
		}
	}

	if ((global[tapestry_status] == TAPESTRY_OPENED ||
		global[tapestry_status] == TAPESTRY_OPENED2) &&
		(global[books_status] == BOOKS_PULLED ||
			global[books_status] == BOOKS_PULLED2)) {
		global[can_view_crown_hole] = true;
	}

	if (kernel.trigger == ROOM_104_NEW_DEATH) {
		conv_reset(CONV_FINALE);
		text_show(10466);
		global[no_load_walker] = false;
		new_room = 119;
	}
}

void room_104_parser() {
	int16 temp;   // used for kernel_synch old-sequence argument

	// Active conversation: dispatch to the finale processor
	if (conv_control.running == CONV_FINALE) {
		room_104_process_conversation_finale();
		player.command_ready = false;
		return;
	}

	// Room description on look_around
	if (player.look_around) {
		if (global[player_persona] == PLAYER_IS_KING) {
			text_show(10401);  // text_104_01
		} else {
			text_show(10437);  // text_104_37
		}
		player.command_ready = false;
		return;
	}

	// walk_through / open / pull + door_to_throne_room (176)
	if (player_parse(37, 176, 0) || player_parse(6, 176, 0) || player_parse(10, 176, 0)) {
		if (global[player_persona] == PLAYER_IS_KING) {
			new_room = 106;
		} else {
			text_show(10434);  // text_104_34
		}
		player.command_ready = false;
		return;
	}

	// walk_through / open / pull + door_to_hallway (197)
	if (player_parse(37, 197, 0) || player_parse(6, 197, 0) || player_parse(10, 197, 0)) {
		if (global[player_persona] == PLAYER_IS_KING) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible   = false;
				seq[fx_open_door] = kernel_seq_pingpong(ss[fx_open_door], false, 8, 0, 0, 2);
				kernel_seq_player(seq[fx_open_door], true);
				kernel_seq_trigger(seq[fx_open_door], KERNEL_TRIGGER_SPRITE, 2, 1);
				kernel_seq_trigger(seq[fx_open_door], KERNEL_TRIGGER_EXPIRE, 0, 3);
				break;
			case 1:
				kernel_seq_delete(seq[fx_door]);
				sound_play(N_DoorOpens);
				seq[fx_door] = kernel_seq_forward(ss[fx_door], false, 6, 0, 0, 1);
				kernel_seq_depth(seq[fx_door], 14);
				kernel_seq_trigger(seq[fx_door], KERNEL_TRIGGER_EXPIRE, 0, 2);
				break;
			case 2:
				temp = seq[fx_door];
				seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_LAST);
				kernel_seq_depth(seq[fx_door], 14);
				kernel_synch(KERNEL_SERIES, seq[fx_door], KERNEL_SERIES, temp);
				break;
			case 3:
				player.walker_visible = true;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_open_door]);
				kernel_timing_trigger(1, 4);
				break;
			case 4:
				player_walk(START_X_ROOM_103, START_Y_ROOM_103, FACING_NORTH);
				player_walk_trigger(5);
				break;
			case 5:
				kernel_seq_delete(seq[fx_door]);
				sound_play(N_DoorCloses);
				seq[fx_door] = kernel_seq_backward(ss[fx_door], false, 7, 0, 0, 1);
				kernel_seq_depth(seq[fx_door], 1);
				kernel_seq_trigger(seq[fx_door], KERNEL_TRIGGER_EXPIRE, 0, 6);
				break;
			case 6:
				temp = seq[fx_door];
				seq[fx_door] = kernel_seq_stamp(ss[fx_door], false, KERNEL_FIRST);
				kernel_seq_depth(seq[fx_door], 1);
				kernel_synch(KERNEL_SERIES, seq[fx_door], KERNEL_SERIES, temp);
				kernel_timing_trigger(6, 7);
				break;
			case 7:
				new_room = 103;
				break;
			}
		} else {
			text_show(10434);  // text_104_34
		}
		player.command_ready = false;
		return;
	}

	// pull / take / open + books (274) — open the bookcase
	if (player_parse(10, 274, 0) || player_parse(4, 274, 0) || player_parse(6, 274, 0)) {
		if (global[books_status] == BOOKS_PRESENT ||
		    global[books_status] == BOOKS_PRESENT2 || kernel.trigger) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible   = false;
				seq[fx_pull_push_books] = kernel_seq_pingpong(ss[fx_pull_push_books], false, 8, 0, 0, 2);
				kernel_seq_range(seq[fx_pull_push_books], KERNEL_FIRST, 3);
				kernel_seq_player(seq[fx_pull_push_books], true);
				kernel_seq_trigger(seq[fx_pull_push_books], KERNEL_TRIGGER_SPRITE, 3, 1);
				kernel_seq_trigger(seq[fx_pull_push_books], KERNEL_TRIGGER_EXPIRE, 0, 3);
				break;
			case 1:
				seq[fx_wall_open_close] = kernel_seq_forward(ss[fx_wall_open_close], false, 6, 0, 0, 1);
				kernel_seq_depth(seq[fx_wall_open_close], 8);
				kernel_seq_trigger(seq[fx_wall_open_close], KERNEL_TRIGGER_EXPIRE, 0, 2);
				break;
			case 2:
				temp = seq[fx_wall_open_close];
				seq[fx_wall_open_close] = kernel_seq_stamp(ss[fx_wall_open_close], false, KERNEL_LAST);
				kernel_synch(KERNEL_SERIES, seq[fx_wall_open_close], KERNEL_SERIES, temp);
				kernel_seq_depth(seq[fx_wall_open_close], 8);
				if (global[tapestry_status] == TAPESTRY_OPENED ||
				    global[tapestry_status] == TAPESTRY_OPENED2) {
					kernel_flip_hotspot(276, false);  // words_wall_panel
					kernel_flip_hotspot(275, true);   // words_secret_door
				}
				break;
			case 3:
				player.walker_visible = true;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_pull_push_books]);
				kernel_timing_trigger(5, 4);
				break;
			case 4:
				if (global[books_status] == BOOKS_PRESENT) {
					global[player_score] += 2;
					global[books_status] = BOOKS_PULLED;
					if (global[tapestry_status] == TAPESTRY_OPENED ||
					    global[tapestry_status] == TAPESTRY_OPENED2) {
						text_show(10428);  // text_104_28
					} else {
						sound_play(N_BooksRumble);
						sound_play(N_WallGrinds);
						text_show(10427);  // text_104_27
					}
				} else {
					sound_play(94);        // N_BooksRumble
					sound_play(67);        // N_WallGrinds
					global[books_status] = BOOKS_PULLED2;
				}
				player.commands_allowed = true;
				break;
			}
			player.command_ready = false;
			return;
		}
	}

	// push + books (274) — close the bookcase
	if (player_parse(5, 274, 0)) {
		if (global[books_status] == BOOKS_PULLED || global[books_status] == BOOKS_PULLED2) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible   = false;
				seq[fx_pull_push_books] = kernel_seq_pingpong(ss[fx_pull_push_books], false, 8, 0, 0, 2);
				kernel_seq_range(seq[fx_pull_push_books], KERNEL_FIRST, 3);
				kernel_seq_player(seq[fx_pull_push_books], true);
				kernel_seq_trigger(seq[fx_pull_push_books], KERNEL_TRIGGER_SPRITE, 3, 1);
				kernel_seq_trigger(seq[fx_pull_push_books], KERNEL_TRIGGER_EXPIRE, 0, 3);
				break;
			case 1:
				sound_play(N_BooksRumble);
				sound_play(N_WallGrinds);
				kernel_seq_delete(seq[fx_wall_open_close]);
				seq[fx_wall_open_close] = kernel_seq_backward(ss[fx_wall_open_close], false, 6, 0, 0, 1);
				kernel_seq_depth(seq[fx_wall_open_close], 8);
				kernel_timing_trigger(TENTH_SECOND, 2);
				break;
			case 2:
				if (global[tapestry_status] == TAPESTRY_OPENED ||
				    global[tapestry_status] == TAPESTRY_OPENED2) {
					kernel_flip_hotspot(276, true);   // words_wall_panel
					kernel_flip_hotspot(275, false);  // words_secret_door
				}
				break;
			case 3:
				player.walker_visible = true;
				kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_pull_push_books]);
				kernel_timing_trigger(5, 4);
				break;
			case 4:
				if (global[books_status] == BOOKS_PULLED) {
					text_show(10429);  // text_104_29
				}
				global[books_status] = BOOKS_PRESENT2;
				player.commands_allowed = true;
				break;
			}
			player.command_ready = false;
			return;
		}
	}

	// pull / open / push + tapestry (26) — open the tapestry
	if (player_parse(10, 26, 0) || player_parse(6, 26, 0) || player_parse(5, 26, 0)) {
		if (global[player_persona] == PLAYER_IS_KING) {
			if (inter_point_x >= CORRECT_TAPESTRY_X) {
				if (global[tapestry_status] == TAPESTRY_CLOSED ||
				    global[tapestry_status] == TAPESTRY_CLOSED2) {
					switch (kernel.trigger) {
					case 0:
						kernel_seq_delete(seq[fx_tapestry_closed]);
						player.commands_allowed  = false;
						player.walker_visible    = false;
						local->animation_running = OPEN_TAPESTRY;
						aa[0] = kernel_run_animation(kernel_name('a', -1), 1);
						break;
					case 1:
						player.walker_visible = true;
						kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
						seq[fx_tapestry_opened] = kernel_seq_stamp(ss[fx_tapestry_opened], false, KERNEL_FIRST);
						kernel_seq_depth(seq[fx_tapestry_opened], 6);
						kernel_flip_hotspot_loc(26, true,  TAP_HS_OPEN_X,   TAP_HS_OPEN_Y);
						kernel_flip_hotspot_loc(26, false, TAP_HS_CLOSED_X, TAP_HS_CLOSED_Y);
						if (global[books_status] == BOOKS_PULLED ||
						    global[books_status] == BOOKS_PULLED2) {
							kernel_flip_hotspot(276, false);  // wall_panel
							kernel_flip_hotspot(275, true);   // secret_door
						} else {
							kernel_flip_hotspot(276, true);   // wall_panel
							kernel_flip_hotspot(275, false);  // secret_door
						}
						kernel_timing_trigger(TENTH_SECOND, 2);
						break;
					case 2:
						if (global[tapestry_status] == TAPESTRY_CLOSED) {
							global[tapestry_status] = TAPESTRY_OPENED;
							global[player_score] += 2;
							if (global[books_status] == BOOKS_NOT_PRESENT ||
							    global[books_status] == BOOKS_PRESENT ||
							    global[books_status] == BOOKS_PRESENT2) {
								text_show(10424);  // text_104_24
							} else if (global[books_status] == BOOKS_PULLED ||
							           global[books_status] == BOOKS_PULLED2) {
								text_show(10425);  // text_104_25
							}
						} else {
							global[tapestry_status] = TAPESTRY_OPENED2;
						}
						player.commands_allowed = true;
						break;
					}
					player.command_ready = false;
					return;
				}
				// tapestry already open: fall through to close-tapestry check below
			} else {
				text_show(10404);  // text_104_04
				player.command_ready = false;
				return;
			}
		} else {
			text_show(10445);  // text_104_45
			player.command_ready = false;
			return;
		}
	}

	// pull / close / push + tapestry (26) — close the tapestry
	if (player_parse(10, 26, 0) || player_parse(11, 26, 0) || player_parse(5, 26, 0)) {
		if (global[player_persona] == PLAYER_IS_KING) {
			if (inter_point_x >= CORRECT_TAPESTRY_X) {
				if (global[tapestry_status] == TAPESTRY_OPENED ||
				    global[tapestry_status] == TAPESTRY_OPENED2) {
					switch (kernel.trigger) {
					case 0:
						player.commands_allowed  = false;
						player.walker_visible    = false;
						kernel_seq_delete(seq[fx_tapestry_opened]);
						local->animation_running = CLOSE_TAPESTRY;
						aa[0] = kernel_run_animation(kernel_name('d', -1), 1);
						break;
					case 1:
						player.walker_visible = true;
						kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[0]);
						seq[fx_tapestry_closed] = kernel_seq_stamp(ss[fx_tapestry_closed], false, KERNEL_FIRST);
						kernel_seq_depth(seq[fx_tapestry_closed], 6);
						kernel_flip_hotspot_loc(26, false, TAP_HS_OPEN_X,   TAP_HS_OPEN_Y);
						kernel_flip_hotspot_loc(26, true,  TAP_HS_CLOSED_X, TAP_HS_CLOSED_Y);
						kernel_flip_hotspot(275, false);  // secret_door
						kernel_flip_hotspot(276, false);  // wall_panel
						kernel_timing_trigger(TENTH_SECOND, 2);
						break;
					case 2:
						global[tapestry_status] = TAPESTRY_CLOSED2;
						player.commands_allowed = true;
						break;
					}
					player.command_ready = false;
					return;
				}
			}
		} else {
			text_show(10445);  // text_104_45
			player.command_ready = false;
			return;
		}
	}

	// ---------------------------------------------------------------------------
	// look / look_at block
	// ---------------------------------------------------------------------------
	if (player_parse(3, 0) || player_parse(17, 0)) {  // look(3) or look_at(17) — TODO: verify look_at ID

		if (player_parse(3, 274, 0) || player_parse(17, 274, 0) ||   // books
		    player_parse(3, 199, 0) || player_parse(17, 199, 0)) {    // bookshelf
			if (global[player_persona] == PLAYER_IS_KING) {
				if (global[books_status] == BOOKS_NOT_PRESENT) {
					kernel_flip_hotspot(274, true);
					global[books_status] = BOOKS_PRESENT;
					text_show(10418);  // text_104_18
					player.command_ready = false;
					return;
				} else if (global[books_status] == BOOKS_PRESENT) {
					text_show(10418);
					player.command_ready = false;
					return;
				} else if (global[books_status] == BOOKS_PULLED ||
				           global[books_status] == BOOKS_PULLED2) {
					text_show(10419);  // text_104_19
					player.command_ready = false;
					return;
				} else if (global[books_status] == BOOKS_PRESENT2) {
					text_show(10420);  // text_104_20
					player.command_ready = false;
					return;
				}
			} else {
				text_show(10439);  // text_104_39
				player.command_ready = false;
				return;
			}
		}

		if (player_parse(3, 34, 0) || player_parse(17, 34, 0)) {   // fireplace (34)
			if (global[player_persona] == PLAYER_IS_KING) {
				text_show(10402);  // text_104_02
			} else {
				text_show(10438);  // text_104_38
			}
			player.command_ready = false;
			return;
		}

		if (player_parse(3, 26, 0) || player_parse(17, 26, 0)) {   // tapestry (26)
			if (global[player_persona] == PLAYER_IS_KING) {
				if (inter_point_x >= ARCHER_TAPESTRY_BEGIN_X && inter_point_x <= ARCHER_TAPESTRY_END_X) {
					text_show(10403);  // text_104_03
					player.command_ready = false;
					return;
				} else if (inter_point_x >= CASTLE_TAPESTRY_BEGIN_X && inter_point_x <= CASTLE_TAPESTRY_END_X) {
					text_show(10422);  // text_104_22
					player.command_ready = false;
					return;
				} else {
					if (global[tapestry_status] == TAPESTRY_OPENED ||
					    global[tapestry_status] == TAPESTRY_OPENED2) {
						text_show(10460);  // text_104_60
					} else {
						text_show(10423);  // text_104_23
					}
					player.command_ready = false;
					return;
				}
			} else {
				text_show(10439);  // text_104_39
				player.command_ready = false;
				return;
			}
		}

		if (player_parse(3, 18, 0) || player_parse(17, 18, 0)) {   // rug (18)
			if (global[player_persona] == PLAYER_IS_KING) {
				text_show(10405);  // text_104_05
			} else {
				text_show(10439);
			}
			player.command_ready = false;
			return;
		}

		if (player_parse(3, 35, 0) || player_parse(17, 35, 0)) {   // fireplace_screen (35)
			if (global[player_persona] == PLAYER_IS_KING) {
				text_show(10407);  // text_104_07
			} else {
				text_show(10439);
			}
			player.command_ready = false;
			return;
		}

		if (player_parse(3, 176, 0) || player_parse(17, 176, 0)) {  // door_to_throne_room (176)
			if (global[player_persona] == PLAYER_IS_KING) {
				text_show(10409);  // text_104_09
			} else {
				text_show(10434);  // text_104_34
			}
			player.command_ready = false;
			return;
		}

		if (player_parse(3, 204, 0) || player_parse(17, 204, 0)) {  // sconce — TODO: verify ID=204
			if (global[player_persona] == PLAYER_IS_KING) {
				text_show(10410);  // text_104_10
			} else {
				text_show(10440);  // text_104_40
			}
			player.command_ready = false;
			return;
		}

		// wood_basket — TODO: unknown vocab ID; needs disassembly verification
		// if (player_parse(3, ???, 0) || player_parse(17, ???, 0)) {
		//     if (global[player_persona] == PLAYER_IS_KING) {
		//         text_show(10411);  // text_104_11
		//     } else {
		//         text_show(10439);
		//     }
		//     player.command_ready = false;
		//     return;
		// }

		if (player_parse(3, 200, 0) || player_parse(17, 200, 0)) {  // trophy (200)
			if (global[player_persona] == PLAYER_IS_KING) {
				text_show(10412);  // text_104_12
			} else {
				text_show(10441);  // text_104_41
			}
			player.command_ready = false;
			return;
		}

		if (player_parse(3, 201, 0) || player_parse(17, 201, 0)) {  // reading_bench (201)
			if (global[player_persona] == PLAYER_IS_KING) {
				text_show(10414);  // text_104_14
			} else {
				text_show(10439);
			}
			player.command_ready = false;
			return;
		}

		if (player_parse(3, 203, 0) || player_parse(17, 203, 0)) {  // loveseat (203)
			if (global[player_persona] == PLAYER_IS_KING) {
				text_show(10416);  // text_104_16
			} else {
				text_show(10439);
			}
			player.command_ready = false;
			return;
		}

		if (player_parse(3, 197, 0) || player_parse(17, 197, 0)) {  // door_to_hallway (197)
			if (global[player_persona] == PLAYER_IS_KING) {
				text_show(10421);  // text_104_21
			} else {
				text_show(10434);
			}
			player.command_ready = false;
			return;
		}

		if (player_parse(3, 275, 0) || player_parse(17, 275, 0)) {  // secret_door (275)
			text_show(10430);  // text_104_30
			player.command_ready = false;
			return;
		}

		if (player_parse(3, 277, 0) || player_parse(17, 277, 0) ||
		    player_parse(37, 277, 0) || player_parse(6, 277, 0)) {  // doorway variants
			if (global[player_persona] == PLAYER_IS_PID) {
				text_show(10432);  // text_104_32
				player.command_ready = false;
				return;
			}
		}

		if (player_parse(3, 276, 0)) {  // look + wall_panel (276) — look_at not in original
			if (global[books_status] == BOOKS_NOT_PRESENT ||
			    global[books_status] == BOOKS_PRESENT) {
				text_show(10435);  // text_104_35
			} else {
				text_show(10436);  // text_104_36
			}
			player.command_ready = false;
			return;
		}

		if (player_parse(3, 291, 0) || player_parse(17, 291, 0)) {  // king (291)
			if (global[player_persona] == PLAYER_IS_PID) {
				text_show(10443);  // text_104_43
				player.command_ready = false;
				return;
			}
		}

		if (player_parse(3, 414, 0) || player_parse(17, 414, 0)) {  // music_box (414)
			if (global[no_load_walker]) {
				// TODO: object_examine(magic_music_box, 843, 0);
				//   magic_music_box = inventory object ID — needs verification
				//   843 = text_008_43 message ID
				player.command_ready = false;
				return;
			}
		}

		if (player_parse(3, 350, 0) || player_parse(17, 350, 0)) {  // MacMorn (350) — PID only here
			if (global[player_persona] == PLAYER_IS_PID) {
				text_show(10444);  // text_104_44
				player.command_ready = false;
				return;
			}
			// King looking at MacMorn: falls through to second MacMorn check at end of look block
		}

		if (player_parse(3, 198, 0) || player_parse(17, 198, 0)) {  // table (198)
			if (global[player_persona] == PLAYER_IS_PID) {
				text_show(10455);  // text_104_55
			} else {
				if (inter_point_x < 174) {
					text_show(10451);  // text_104_51
				} else {
					text_show(10448);  // text_104_48
				}
			}
			player.command_ready = false;
			return;
		}

		if (player_parse(3, 237, 0) || player_parse(17, 237, 0)) {  // decoration (237)
			if (global[player_persona] == PLAYER_IS_PID) {
				text_show(10439);
			} else {
				text_show(10449);  // text_104_49
			}
			player.command_ready = false;
			return;
		}

		if (player_parse(3, 56, 0) || player_parse(17, 56, 0)) {  // sword (56)
			if (global[player_persona] == PLAYER_IS_PID) {
				text_show(10439);
			} else {
				text_show(10450);  // text_104_50
			}
			player.command_ready = false;
			return;
		}

		// floor — TODO: unknown vocab ID; needs disassembly verification
		// if (player_parse(3, ???, 0) || player_parse(17, ???, 0)) {
		//     if (global[player_persona] == PLAYER_IS_PID) {
		//         text_show(10439);
		//         player.command_ready = false;
		//         return;
		//     }
		// }

		// wall — TODO: unknown vocab ID; needs disassembly verification
		// if (player_parse(3, ???, 0) || player_parse(17, ???, 0)) {
		//     if (global[player_persona] == PLAYER_IS_PID) {
		//         text_show(10439);
		//         player.command_ready = false;
		//         return;
		//     }
		// }

		// candlestick — TODO: unknown vocab ID; needs disassembly verification
		// if (player_parse(3, ???, 0) || player_parse(17, ???, 0)) {
		//     if (global[player_persona] == PLAYER_IS_PID) {
		//         text_show(10439);
		//     } else {
		//         text_show(10461);  // text_104_61
		//     }
		//     player.command_ready = false;
		//     return;
		// }

		if (player_parse(3, 347, 0) || player_parse(17, 347, 0)) {  // Queen_Mother (347)
			text_show(10456);  // text_104_56
			player.command_ready = false;
			return;
		}

		if (player_parse(3, 350, 0) || player_parse(17, 350, 0)) {  // MacMorn (350) — King reach
			text_show(10444);  // text_104_44
			player.command_ready = false;
			return;
		}
	}  // end look/look_at block

	// push / pull + rug (18)
	if (player_parse(5, 18, 0) || player_parse(10, 18, 0)) {
		if (global[player_persona] == PLAYER_IS_KING) {
			text_show(10406);  // text_104_06
		} else {
			text_show(10445);
		}
		player.command_ready = false;
		return;
	}

	// push / pull + fireplace_screen (35)
	if (player_parse(5, 35, 0) || player_parse(10, 35, 0)) {
		if (global[player_persona] == PLAYER_IS_KING) {
			text_show(10408);  // text_104_08
		} else {
			text_show(10445);
		}
		player.command_ready = false;
		return;
	}

	// push / pull + trophy (200)
	if (player_parse(5, 200, 0) || player_parse(10, 200, 0)) {
		if (global[player_persona] == PLAYER_IS_KING) {
			text_show(10413);  // text_104_13
		} else {
			text_show(10445);
		}
		player.command_ready = false;
		return;
	}

	// open + reading_bench (201)
	if (player_parse(6, 201, 0)) {
		if (global[player_persona] == PLAYER_IS_KING) {
			text_show(10415);  // text_104_15
		} else {
			text_show(10445);
		}
		player.command_ready = false;
		return;
	}

	// push / pull + loveseat (203)
	if (player_parse(5, 203, 0) || player_parse(10, 203, 0)) {
		if (global[player_persona] == PLAYER_IS_KING) {
			text_show(10417);  // text_104_17
		} else {
			text_show(10445);
		}
		player.command_ready = false;
		return;
	}

	// open / push / pull + secret_door (275) or wall_panel (276)
	if (player_parse(6, 275, 0) || player_parse(6, 276, 0) ||
	    player_parse(5, 275, 0) || player_parse(5, 276, 0) ||
	    player_parse(10, 275, 0) || player_parse(10, 276, 0)) {
		text_show(10431);  // text_104_31
		player.command_ready = false;
		return;
	}

	// invoke (47) + signet_ring — TODO: verify signet_ring vocab ID
	// if (player_parse(47, ???, 0)) {
	//     if (global[player_persona] == PLAYER_IS_PID) {
	//         text_show(10433);  // text_104_33
	//         player.command_ready = false;
	//         return;
	//     }
	// }

	// put (7) + tentacle_parts (125) + wall_panel (276)
	if (player_parse(7, 125, 276, 0)) {
		if (global[player_persona] == PLAYER_IS_KING) {
			text_show(10446);  // text_104_46
			player.command_ready = false;
			return;
		}
	}

	// shift_into_bear (116) — bear transformation in finale
	if (player_parse(116, 0)) {
		if (local->anim_0_running) {
			if (local->has_been_bear) {
				text_show(10457);  // text_104_57
			} else if (local->amulet_works) {
				text_show(10459);  // text_104_59
			} else {
				local->has_been_bear = true;
				global[player_score] += 2;
				conv_run(CONV_FINALE);
				conv_export_value(1);  // turned into bear
				conv_export_value(0);
				conv_export_value(0);
				if (global[llanie_status] != IS_SAVED) {
					conv_export_value(1);
				} else {
					conv_export_value(0);
				}
			}
			player.command_ready = false;
			return;
		}
	}

	// sword(56)+attack(57)/carve_up(58)/thrust(87) on MacMorn(350), or take(4)+sword(56)
	if (player_parse(56, 57, 350, 0) || player_parse(56, 58, 350, 0) ||
	    player_parse(56, 87, 350, 0) || player_parse(4, 56, 0)) {
		if (local->anim_0_running) {
			local->activate_timer = false;
			local->pid_action     = PID_DRAW_SWORD;
			if (!local->amulet_works) {
				global[player_score] += 5;
				conv_run(CONV_FINALE);
				conv_export_value(0);
				conv_export_value(1);  // drew sword
				conv_export_value(0);
				if (global[llanie_status] != IS_SAVED) {
					conv_export_value(1);
				} else {
					conv_export_value(0);
				}
				conv_hold();
			} else {
				local->mac_action       = MAC_KILL_NOW;
				player.commands_allowed = false;
			}
			local->amulet_works = true;
			player.command_ready = false;
			return;
		}
	}

	// invoke (47) + amulet (46)
	if (player_parse(47, 46, 0)) {
		if (local->anim_0_running) {
			if (local->amulet_works) {
				local->activate_timer   = false;
				local->pid_action       = PID_KILL_MACMORN;
				global[player_score]   += 15;
				global[amulet_status]   = AMULET_USED;
				player.commands_allowed = false;

				kernel_abort_animation(aa[1]);

				seq[fx_e5] = kernel_seq_stamp(ss[fx_e5], false, KERNEL_LAST);
				kernel_seq_depth(seq[fx_e5], 6);
				kernel_synch(KERNEL_SERIES, seq[fx_e5], KERNEL_NOW, 0);

				ss[fx_e3] = kernel_load_series(kernel_name('e', 3),
				                               PAL_MAP_ALL_TO_CLOSEST | PAL_MAP_ANY_TO_CLOSEST);
				aa[1]                 = kernel_run_animation(kernel_name('m', 2), 0);
				local->anim_1_running = false;
				local->anim_5_running = true;
			} else {
				// TODO: object_examine(amulet_obj_id, 945, 0);
				//   amulet_obj_id = inventory object ID for amulet — needs verification
				//   945 = text_009_45 message ID
			}
			player.command_ready = false;
			return;
		}
	}

	// sword+attack/carve_up/thrust on Queen_Mother (347)
	if (player_parse(56, 57, 347, 0) || player_parse(56, 58, 347, 0) ||
	    player_parse(56, 87, 347, 0)) {
		text_show(10458);  // text_104_58
		player.command_ready = false;
		return;
	}

	// talk_to (8) + MacMorn (350)
	if (player_parse(8, 350, 0)) {
		text_show(10464);  // text_104_64
		player.command_ready = false;
		return;
	}

	// talk_to (8) + Queen_Mother (347)
	if (player_parse(8, 347, 0)) {
		text_show(10463);  // text_104_63
		player.command_ready = false;
		return;
	}

	// talk_to (8) + king (291)
	if (player_parse(8, 291, 0)) {
		text_show(10465);  // text_104_65
		player.command_ready = false;
		return;
	}

	// pour_contents_of + MacMorn — TODO: verify pour_contents_of vocab ID
	// if (player_parse(???, 350, 0)) {
	//     text_show(10462);  // text_104_62
	//     player.command_ready = false;
	//     return;
	// }

	// walk_across (41) / walk_to (13) when Queen is in room
	if (local->anim_2_running) {
		if (player_parse(41, 0) || player_parse(13, 0)) {
			text_show(10445);  // text_104_45
			player.command_ready = false;
			return;
		}
	}

	// take (4) + candlestick — TODO: verify candlestick vocab ID
	// if (player_parse(4, ???, 0)) {
	//     text_show(10468);  // text_104_68
	//     player.command_ready = false;
	//     return;
	// }

	// open (6) + music_box (414) when in finale scene
	if (player_parse(6, 414, 0)) {
		if (global[no_load_walker]) {
			text_show(10470);  // text_104_70
			player.command_ready = false;
			return;
		}
	}
}

void room_104_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)    s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation) s.syncAsSint16LE(v);
	s.syncAsSint16LE(local->doorway_id);
	s.syncAsSint16LE(local->wall_panel_id);
	s.syncAsSint16LE(local->secret_door_id);
	s.syncAsSint16LE(local->tapestry_frame);
	s.syncAsSint16LE(local->animation_running);
	s.syncAsSint16LE(local->king_frame);
	s.syncAsSint16LE(local->king_action);
	s.syncAsSint16LE(local->king_talk_count);
	s.syncAsSint16LE(local->anim_0_running);
	s.syncAsSint16LE(local->mac_frame);
	s.syncAsSint16LE(local->mac_action);
	s.syncAsSint16LE(local->mac_talk_count);
	s.syncAsSint16LE(local->anim_1_running);
	s.syncAsSint16LE(local->queen_frame);
	s.syncAsSint16LE(local->queen_action);
	s.syncAsSint16LE(local->queen_talk_count);
	s.syncAsSint16LE(local->anim_2_running);
	s.syncAsSint16LE(local->twinkles_frame);
	s.syncAsSint16LE(local->twinkles_action);
	s.syncAsSint16LE(local->twinkles_talk_count);
	s.syncAsSint16LE(local->anim_3_running);
	s.syncAsSint16LE(local->pid_frame);
	s.syncAsSint16LE(local->pid_action);
	s.syncAsSint16LE(local->pid_talk_count);
	s.syncAsSint16LE(local->anim_4_running);
	s.syncAsSint32LE(local->clock);
	s.syncAsSint32LE(local->death_timer);
	s.syncAsSint16LE(local->activate_timer);
	s.syncAsSint16LE(local->has_been_bear);
	s.syncAsSint16LE(local->mac_2_frame);
	s.syncAsSint16LE(local->anim_5_running);
	s.syncAsSint16LE(local->amulet_works);
	s.syncAsSint16LE(local->pid_drawn_sword);
	s.syncAsSint16LE(local->death_frame);
	s.syncAsSint16LE(local->anim_6_running);
}

void room_104_error() {
}

void room_104_preload() {
	room_init_code_pointer       = room_104_init;
	room_pre_parser_code_pointer = room_104_pre_parser;
	room_parser_code_pointer     = room_104_parser;
	room_daemon_code_pointer     = room_104_daemon;

	if (room_id == 119) {
		global[no_load_walker] = true;
	}

	section_1_walker();
	section_1_interface();

	vocab_make_active(words_doorway);
	vocab_make_active(words_Queen_Mother);
	vocab_make_active(words_king);
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
