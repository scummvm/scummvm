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

#ifndef MADS_DRAGONSPHERE_GLOBAL_H
#define MADS_DRAGONSPHERE_GLOBAL_H

#include "common/serializer.h"
#include "mads/madsv2/core/vocabh.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {

enum {
// ----------------- Global to all sections ------------------

	player_persona                  =  10,  // Player is King or Pid
	player_score                    =  11,  // player's score of the game
	dragon_high_scene               =  12,  // Keeps track of which animation
	dragon_my_scene                 =  13,  // to show in rm111 of cracking dragonsphere
	no_load_walker                  =  14,  // if T, will not load walker in section code
	player_score_flags              =  15,  // 16 flags for checking if player increased score
	sorceror_defeated               =  16,  // T if you defeat the Sorceror
	pre_room                        =  17,  // used in 120
	perform_displacements           =  18,  // if T, will do displacements

// ------------------- Section 1  20 - 39 --------------------

	king_status                     =  20,  // If king captive or not
	ward_status                     =  21,  // If ward is blocking emerald 
	talked_to_soptus                =  22,  // T if talked to soptus in 110
	talked_to_shifter               =  23,  // T if talked to shifter in 110
	amulet_status                   =  24,  // status of amulet  
	books_status                    =  25,  // status of books in rm104 part 1
	tapestry_status                 =  26,  // status of tapestry in rm104 part 1  
	talked_to_merchant              =  27,  // T if talked to merchant in 110
	can_view_crown_hole             =  28,  // T if can view crown hole in secret door in 104
	talked_to_status                =  29,  // point keeper for rm110 how many talked to
	guard_pid_status                =  30,  // from rm112 actions, indicates whether or not to kill pid
											// how many times she talks to him*/
	crawled_out_of_bed_101          =  31,  // T if King got up from bed in opening scene
	statue_is_on_stairway_114       =  32,  // T if status has been placed on stairway in 114
	dog_is_asleep                   =  33,  // T if dog is asleep in 115
	crystal_ball_dead               =  34,  // T if crystal ball has been used already
	threw_bone                      =  35,  // T if Pid throw bone to dog
	wooden_door_open                =  36,  // T if wooden door is open in 119
	king_is_in_stairwell            =  37,  // T if king is in stairwell (119)
	llanie_status                   =  38,  // Either BEFORE_FALL, SHE_FELL, or IS_SAVED
	no_talk_to_guard                =  39,  // Temporary

// ------------------- Section 2  40 - 59 --------------------

	object_given_201                =  40,  // object king gave guards in 201 
	king_got_stabbed                =  41,  // T if king got stabbed in 201
	given_object_before             =  42,  // T if king gave object in 201 before
	guards_are_asleep               =  43,  // T if guards in 201 are asleep from soporific
	dome_up                         =  44,  // T if shield is over pool monster
	talked_to_wise                  =  45,  // T if talked to wise shifter in 203
	doll_given                      =  46,  // T if doll was given in cave of shifting dreams
	reset_conv                      =  47,  // if T, will reset conv in 201 init
	talked_to_greta                 =  48,  // T if talked to Greta in 206
	slime_healed                    =  49,  // T if slime is healed in 205
	dance_music_on                  =  50,  // T if belly dance music is on in 406
	player_is_seal                  =  51,  // T if player is seal in 113
	vines_have_player               =  52,  // T if vines have player in custody
	end_of_game                     =  53,  // T if end of game
	reset_conv_2                    =  54,  // if T, will reset conv in 201 init
	invoked_from_111                =  55,  // T if invoked signet ring when room == 111

// ------------------- Section 3  60 - 79 --------------------

	save_wins_in_desert             =  60,   // save these variables incase pid dies in 406
	bubbles_up_in_301               =  61,   // T if bubbles block maze entrance in 301
	bubble_wont_attack              =  62,   // if T, bubble in 302 won't attack player
	pid_looked_at_doll              =  63,   // T if pid looked at doll

// ------------------- Section 4  80 - 99 --------------------

	oasis                           =  80,   // The location of the oasis (1-36)
	fire_holes                      =  81,   // The location of the fire holes (1-36)
	desert_room                     =  82,   // The location (1-36) of player
	from_direction                  =  83,   // Holds direction that he will come into room from
	desert_counter                  =  84,   // number of moves across desert
	move_direction_409              =  85,   // true if counting up, false if counting down
	heal_verbs_visible              =  86,   // T if verbs for pid doll should be displayed
	grapes_have_grown               =  87,   // holds status of grapes in 411
	pid_talk_shamon                 =  88,   // T if pid talked to shamon
	pid_just_died                   =  89,   // T if pid just got impaled by roc || drank shit in 406
	grapes_are_dead                 =  90,   // T if grapes will not grow anymore
	roc_is_chewing_dates            =  91,   // T if roc is chewing dates in 411
	wins_in_desert                  =  92,   // # of wins in game
	wins_till_prize                 =  93,   // # wins needed for prize
	pid_has_been_healed_sop         =  94,   // T if pid drank soporific to be healed in 406
	game_points                     =  95,   // used in co 40, 41
	dance_points                    =  96,   // used in co 40, 41
	clue_points                     =  97,   // used in co 40, 41
	prizes_owed_to_player           =  98,   // ++'d in co 50 when player wins prize
	object_flags                    =  99,   // bit flags for what objects pid has when enter tent 1st time

// ------------------ Section 5  100 - 119 -------------------

	waterfall_diverted              = 100,  // T if the waterfall has been diverted
	shak_status                     = 101,  // Either SHAK_NEVER_MET or SHAK_MET
	max_grid_value                  = 102,  // holds the position in the array of the last
											// tower that Pid steps on in 510
	move_direction_510				= 103,  // true if counting up, false if counting down
	shak_506_angry					= 104,  // T if conv in 506 ended on angry note
	monster_is_dead					= 105,  // T if cliff monster is dead (and now in 503)
	done_talking_lani_502			= 106,  // T if talked to Llanie in 502
	found_lani_504					= 107,  // T if we saw llanie in cave in 504
	said_use_sword_shak				= 108,  // T if player tried to attack shak w/sword in 509
	goblet_filled_soporific			= 109,  // T if goblet has been filled with soporific
	been_in_504_as_pid				= 110,  // T if player has been in 504 as pid
	seen_lani_dead_1st_time			= 111,  // T if player (king) has been in 504 after lani fell
	said_poem_in_504				= 112,  // T if player said poem correctly in front of llanie in 504
	tried_to_heal_llanie_504		= 113,  // T if player tried to heal llanie in 504
	put_bundle_on_llanie_504		= 114,  // T if player put spirit or new bundle on dead llanie in 504
	make_504_empty					= 115,  // T if nobody will be in room 504. Set to T in 120, 110, & 508
	has_taken_mud					= 116,  // T if player has taken mud ever
	platform_clicked_606			= 117,  // T if platform clicked in 606
	had_spirit_bundle				= 118,  // T if player ever had spirit bundle (from 614)

// ------------------ Section 6  120 - 139 -------------------

	mud_is_in_eye_603               = 120,	// T if mud has been thrown in eye in 603
	rope_is_alive                   = 121,	// T if rope is in snake form
	rat_cage_is_open                = 122,	// T if rat has been let out in 605
	flask_on_plate                  = 123,	// either NONE, YES_EMPTY, or YES_ACID
	fluid_is_dripping               = 124,	// T if fluid is dripping onto hot plate in 605
	hole_is_in_607                  = 125,	// T if a hole has been created on floor in 607  
	rope_is_hanging_in_607          = 126,	// T if rope is hanging from ceiling in 607  
	object_is_in_freezer_605        = 127,	// T if any object is in freezer
	object_imitated                 = 128,	// holds number of object that polystone has imitated (a powerstone)
	has_red                         = 129,	// T if player came into 614 with red powerstone
	has_yellow                      = 130,	// T if player came into 614 with yellow powerstone
	has_blue                        = 131,	// T if player came into 614 with blue powerstone
	wizard_dead                     = 132,	// T if wizard is dead in 614
	vine_will_grab                  = 133,	// if T, vine will grab king in 601
	floor_is_cool                   = 134,	// T if floor in 613 has been cooled
	rat_melted                      = 135,	// T if rat melted in 613
	door_is_cool                    = 136,	// T if teleportal door in 606 is cool
	used_elevator                   = 137,	// T if player used elevator in 606
	been_on_top_floor               = 138,	// T if player has been on top floor in 606
	torch_is_in_609                 = 139,	// T if torch is in 609

// --------------------- Grid variables ----------------------

	grid_position                       = 140
};

/* Global macros */

#define PLAYER_IS_KING                  0   /* Who the player is */
#define PLAYER_IS_PID                   1

#define SCORE_LOOK_SPHERE_106           1
#define SCORE_READ_BOOK_101             2
#define SCORE_SHIFT_INTO_SEAL_113       4   
#define SCORE_PUSH_BUTTON_119           8   
#define SCORE_WEAR_CROWN_119            16  
#define SCORE_MAKE_ACID_DRIP_605        32  
#define SCORE_CROSS_PILLARS             64  
#define SCORE_ENTER_609                 128  
#define SCORE_CROSS_SKY                 256 
#define SCORE_JUMP_INTO_WELL            512 
#define SCORE_MAKE_NOISE_BIRDCALL       1024
#define SCORE_TAKE_BONE                 2048

/* section 1 macros */
#define KING_CAPTIVE                    0   /* various states of king */
#define KING_WITHOUT_SOUL               1
#define KING_WITH_SOUL                  2

#define WARD_PRESENT                    0   /* status of ward in room 113*/
#define WARD_GONE                       1     

#define AMULET_NOT_CORRECT_TIME         0   /* can't invoke amulet yet */
#define AMULET_CORRECT_TIME             1   /* ready to invoke         */
#define AMULET_USED                     2   /* have invoked amulet     */

#define BOOKS_NOT_PRESENT               0   /* rm104 part 1 books not there */
#define BOOKS_PRESENT                   1   /* books hotspot there */
#define BOOKS_PULLED                    2   /* books pulled, secret door there */
#define BOOKS_PRESENT2                  3   /* books hotspot there, already pulled once */ 
#define BOOKS_PULLED2                   4   /* books pulled for 2nd time */

#define TAPESTRY_CLOSED                 0   /* tapestry has never been pulled */
#define TAPESTRY_OPENED                 1   /* tapestry opened for 1st time */
#define TAPESTRY_CLOSED2                2   /* tapestry closed 2nd time */
#define TAPESTRY_OPENED2                3   /* tapestry opened for 2nd time */

#define TALKED_TO_NONE                  0   /* how many talked to in rm110 */
#define TALKED_FAERIE                   1
#define TALKED_SOPTUS                   2
#define TALKED_SLATHAN                  3
#define TALKED_MERCHANT                 4
#define TALKED_FAERIE_SOPTUS            5
#define TALKED_FAERIE_SLATHAN           6
#define TALKED_FAERIE_MERCHANT          7
#define TALKED_SOPTUS_SLATHAN           8
#define TALKED_SOPTUS_MERCHANT          9
#define TALKED_SLATHAN_MERCHANT        10
#define TALKED_FAERIE_SOPTUS_SLATHAN   11 
#define TALKED_FAERIE_SOPTUS_MERCHANT  12
#define TALKED_FAERIE_SLATHAN_MERCHANT 13
#define TALKED_SOPTUS_SLATHAN_MERCHANT 14
#define TALKED_TO_ALL                  15

#define STAIRWAY_STATUE                 1   /* used with global[statue_is_on_stairway_104] */
#define STAIRWAY_CRYSTAL_BALL           2

#define GUARD_NEVER_HEALED              0
#define GUARD_IS_HEALED                 1
#define GUARD_IS_UNHEALED               2

#define BEFORE_FALL                     0
#define SHE_FELL                        1
#define IS_SAVED                        2

#define SHAK_NEVER_MET                  0
#define SHAK_MET                        1

#define NONE                            0
#define YES_EMPTY                       1
#define YES_ACID                        2

#define FREEZER                         70

#define NEVER_USED_FREEZER              0
#define HAS_USED_FREEZER                1
#define HAS_USED_AND_EMPTY              2

#define FROM_SOUTH                      0    /* used for global[from_direction] */
#define FROM_EAST                       1
#define FROM_NORTH                      2
#define FROM_WEST                       3

#define GRAPES_NOT_THERE                0
#define GRAPES_GROWING                  1  /* is for global[grapes_have_grown] in 411 */
#define GRAPES_GROWN                    2

extern void global_section_constructor();
extern void sync_room(Common::Serializer &s);

} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS

#endif
