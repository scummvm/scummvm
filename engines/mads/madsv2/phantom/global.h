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

#ifndef MADS_PHANTOM_GLOBAL_H
#define MADS_PHANTOM_GLOBAL_H

#include "mads/madsv2/core/vocabh.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {

enum {
	/* --- System / all-sections (0–19) --- */
	kWalkerTiming             = 0,
	kWalkerTiming2            = 1,
	stop_walker_disabled      = 2,
	temp_interface            = 3,
	walker_converse           = 4,
	walker_converse_state     = 5,
	walker_converse_now       = 6,
	_007                      = 7,
	_008                      = 8,
	_009                      = 9,
	current_year              = 10,
	music_selected            = 11,
	player_score              = 12,
	player_score_flags        = 13,
	done_brie_conv_203        = 14,
	lantern_status            = 15,
	_016                      = 16,
	_017                      = 17,
	_018                      = 18,
	leave_angel_music_on      = 19,

	/* --- Section 1 (20–59) --- */
	trap_door_status          = 20,
	christine_door_status     = 21,
	sandbag_status            = 22,
	chris_f_status            = 23,
	brie_talk_status          = 24,
	julies_door               = 25,
	prompter_stand_status     = 26,
	chris_d_status            = 27,
	julie_name_is_known       = 28,
	chris_kicked_raoul_out    = 29,
	jacques_name_is_known     = 30,
	jacques_status            = 31,
	florent_name_is_known     = 32,
	charles_name_is_known     = 33,
	room_103_104_transition   = 34,
	observed_phan_104         = 35,
	death_location            = 36,
	make_brie_leave_203       = 37,
	hint_that_daae_is_home_1  = 38,
	hint_that_daae_is_home_2  = 39,

	/* --- Section 2 (40–59) --- */
	christine_told_envelope   = 40,
	read_book                 = 41,
	scanned_bookcase          = 42,
	ran_conv_in_205           = 43,
	doors_in_205              = 44,
	panel_in_206              = 45,
	madame_name_is_known      = 46,
	madame_giry_loc           = 47,
	looked_at_case            = 48,
	madame_giry_shows_up      = 49,
	done_rich_conv_203        = 50,
	came_from_fade            = 51,
	ticket_people_here        = 52,
	degas_name_is_known       = 53,
	temp_var                  = 54,
	flicked_1                 = 55,
	flicked_2                 = 56,
	flicked_3                 = 57,
	flicked_4                 = 58,
	_059                      = 59,

	/* --- Section 3 (60–79) --- */
	top_floor_locked          = 60,
	field_7A                  = 61,
	field_7C                  = 62,
	field_7E                  = 63,
	field_80                  = 64,
	field_82                  = 65,
	field_84                  = 66,
	field_86                  = 67,
	field_88                  = 68,
	field_8A                  = 69,
	_070                      = 70,
	field_8E                  = 71,
	field_90                  = 72,
	field_92                  = 73,
	field_94                  = 74,
	field_96                  = 75,
	field_98                  = 76,
	field_9A                  = 77,
	field_9C                  = 78,
	field_9E                  = 79,

	/* --- Section 4 / Catacombs (80–99) --- */
	catacombs_room            = 80,
	catacombs_misc            = 81,
	catacombs_flag            = 82,
	catacombs_from            = 83,
	catacombs_309             = 84,
	catacombs_409a            = 85,
	catacombs_409b            = 86,
	catacombs_501             = 87,
	catacombs_309_from        = 88,
	catacombs_409a_from       = 89,
	catacombs_409b_from       = 90,
	catacombs_501_from        = 91,
	catacombs_next_room       = 92,
	door_in_409_is_open       = 93,
	priest_piston_puke        = 94,
	cob_web_is_cut            = 95,
	field_C0                  = 96,
	field_C2                  = 97,
	field_C4                  = 98,
	_099                      = 99,

	/* --- Section 5 (100–112) --- */
	christine_is_in_boat      = 100,
	chris_will_take_seat      = 101,
	right_door_is_open_504    = 102,
	coffin_status             = 103,
	chris_left_505            = 104,
	knocked_over_head         = 105,
	fight_status              = 106,
	he_listened               = 107,
	can_find_book_library     = 108,
	ring_is_on_finger         = 109,
	looked_at_skull_face      = 110,
	cable_hook_was_seperate   = 111,
	make_rich_leave_203       = 112
};


// brie_talk_status values
#define BEFORE_BRIE_MOTIONS			0
#define BEFORE_CHANDELIER_CONV		1
#define AFTER_CONVS_0_AND_1			2

// jacques_status values
#define JACQUES_IS_ALIVE				0
#define JACQUES_IS_DEAD					1
#define JACQUES_IS_DEAD_RICH_GONE		2
#define JAC_DEAD_RICH_GONE_SEEN_BODY	3

// walker_converse values
#define CONVERSE_NONE           0
#define CONVERSE_LEAN           1
#define CONVERSE_HAND_WAVE      2
#define CONVERSE_HAND_WAVE_2    3
#define CONVERSE_HAND_CHIN      4

// death_location values
#define FAR_PIT     0
#define MIDDLE_PIT  1
#define NEAR_PIT    2

// jacques_name_is_known
#define YES_AND_END_CONV 2

// trap_door_status
#define TRAP_DOOR_IS_OPEN   0
#define TRAP_DOOR_IS_CLOSED 1

// prompter_stand_status
#define PROMPT_LEFT			0
#define PROMPT_RIGHT		1

// room_103_104_transition
#define PEEK_THROUGH		0
#define NEW_ROOM			1

// player_score_flags
#define SCORE_TRAP_DOOR		0x01
#define SCORE_DEAD_FLORENT	0x04
#define SCORE_HOLLOW_COLUMN	0x08
#define SCORE_CHASE_CATWALK 0x10

// ticket_people_here
#define NEITHER				0
#define SELLER				1
#define USHER_AND_SELLER	2

// sandbag_status
#define SANDBAG_SECURE		0
#define SANDBAG_DROPPED		1

// julies_door
#define FULLY_OPEN			0
#define CRACKED_OPEN		1

// chris_f_status
#define CHRIS_F_IS_DEAD		0
#define CHRIS_F_IS_ALIVE	1

// christine_door_status
#define CHRIS_IS_IN			0
#define CHRIS_DOOR_CLOSED	1
#define CHRIS_DOOR_CHOPPED	2

// julie_name_is_known
#define JULIE_NO			0
#define NO_AND_QUIT_CONV	1
#define JULIE_YES			2

// done_brie_conv_203
#define YES_AND_CHASE		3

// chris_kicked_raoul_out
#define CHRIS_KICKED_YES		1
#define CAME_INTO_EMPTY_113		3

// madame_giry_loc
#define LEFT				0
#define MIDDLE				1
#define RIGHT				2

// doors_in_205
#define BOTH_LOCKED			0
#define LEFT_OPEN			1
#define RIGHT_OPEN			2
#define BOTH_OPEN			3

// panel_in_206
#define PANEL_UNDISCOVERED	0
#define PANEL_DISCOVERED	1
#define PANEL_LOCKED		2
#define PANEL_UNLOCKED		3

// lantern_status
#define LANTERN_IS_OFF		0
#define LANTERN_IS_ON		1

// fight_status
#define FIGHT_NOT_HAPPENED	0
#define FIGHT_TALKING		1
#define FIGHT_PARTED		2

// coffin_status
#define COFFIN_CLOSED		0
#define COFFIN_UNLOCKED		1
#define COFFIN_OPEN			2

// chris_d_status
#define BEFORE_LOVE			0

/* Random music generated at start of each game */
#define TOCCATA_FUGUE_D_MINOR     1
#define LITTLE_FUGUE_G_MINOR      2
#define FUGUE_B_MINOR             3
#define FUGUE_C_MINOR             4


extern void global_section_constructor();

} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
