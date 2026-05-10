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
#include "mads/madsv2/core/kernel.h"
#include "mads/madsv2/core/object.h"
#include "mads/madsv2/core/sound.h"
#include "mads/madsv2/core/speech.h"
#include "mads/madsv2/core/text.h"
#include "mads/madsv2/dragonsphere/mads/conv.h"
#include "mads/madsv2/dragonsphere/mads/inventory.h"
#include "mads/madsv2/dragonsphere/mads/sounds.h"
#include "mads/madsv2/dragonsphere/mads/words.h"
#include "mads/madsv2/dragonsphere/global.h"
#include "mads/madsv2/dragonsphere/rooms/section1.h"
#include "mads/madsv2/dragonsphere/rooms/room110.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {
namespace Rooms {

struct Scratch {
	int16 sprite[15];				/* Sprite series handles */
	int16 sequence[15];				/* Sequence handles      */
	int16 animation[6];				/* Animation handles     */

	int16 character_frame[6];		/* hold frame of character to whom the king speaks */
	int16 faerie_id[2];				/* hotspot handles for faerie */
	int16 slathan_id[2];			/* hotspot handles for slathan shapechanger */
	int16 soptus_id;				/* hotspot handle for soptus */
	int16 merchant_id;				/* hotspot handle for merchant */
	int16 guard_id;					/* hotspot for guard */

	int16 new_action[6];			/* Type of action to run for character animation */
	int16 last_action[6];			/* Last action in character animation to stave */
									/* running it again. */
	int16 good_number[6];			/* so as not to repeat two looks */
	int16 change_action[6];			/* change action to a different state during conversation */
	int16 talking_to;				/* who talking to in conversations */
	int16 how_long_to_talk;			/* how long to talk in a talk sequence */
	int16 slathan_talk_count;		/* counter to help determine when to stop talking */
	int16 soptus_talk_count;		/* same as above */

	int16 anim_0_running;
	int16 anim_1_running;
	int16 anim_2_running;
	int16 anim_3_running;
	int16 anim_4_running;
	int16 anim_5_running;

	int16 guard_king_frame;			/* animation frame being held for guard while king stuff */
	int16 guard_king_action;		/* Type of action to run for guard while king animation */
	int16 guard_king_talk_count;	/* counter for guard while king talking */

	int16 guard_pid_frame;			/* animation frame being held for guard while pid stuff */
	int16 guard_pid_action;			/* Type of action to run for guard while pid animation */
	int16 guard_pid_talk_count;		/* counter for guard while pid talking */

	int32 slathan_timer;			/* Counters for slathan turning head*/
	int32 diff[5];
	int32 last_clock[6];
	int16  switch_direction[6];		/* Time to move in other directions*/
	int32 soptus_timer;				/* Counters for soptus */
	int16  merchant_talking;		/* true, if merchant talking, used in soptus animation */
	int16  soptus_talking;			/* true, if soptus talking, used in merchant animation */
	int16 time;

	int16 random_grunt_only;		/* saves random number choice for guard's actions */

	int32 clock;
	int32 death_timer;				/* when this counter reaches 15 sec, kill pid */
	int16 activate_timer;			/* If T, will start counting death_timer */
	byte cut_scene;
	int16 speech_playing;
};

#define local (&scratch)
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

static Scratch scratch;


/* ========================= Sprites ========================= */

#define fx_king_in_well                  1    /* rm110a  */
#define fx_pid_in_well                   2    /* rm110b0 */
#define fx_reach_fruit                   3    /* kgrh_9  */


/* ======================== Triggers ========================= */

#define ROOM_110_GOT_FRUIT               70
#define ROOM_110_SHOW_WALKER             80
#define ROOM_110_PID_SHISHKABOB          90
#define ROOM_110_YOU_TALK                100
#define ROOM_110_GO_TO_120               104
#define ROOM_110_START_WALKING           106
#define MUSIC                            120


/* walk points */
#define START_X_ROOM_118                 21
#define START_Y_ROOM_118                 134
#define START_X_ROOM_113                 177
#define START_Y_ROOM_113                 146
#define START_X_ROOM_120                 307
#define START_Y_ROOM_120                 141

#define WALK_TO_FAERIE_X                 285
#define WALK_TO_FAERIE_Y                 130

#define WALK_TO_SLATHAN_X                131
#define WALK_TO_SLATHAN_Y                122

#define WALK_TO_SOPTUS_X                 284
#define WALK_TO_SOPTUS_Y                 121

#define WALK_TO_MERCHANT_X               284
#define WALK_TO_MERCHANT_Y               121

#define WALK_TO_GUARD_X                  51
#define WALK_TO_GUARD_Y                  126

#define WALK_TO_GUARD_2_X                260
#define WALK_TO_GUARD_2_Y                137

/* cursor points */
#define RIGHT_STALL_X                    240

/* animations */
/* rm110a.aa - king - faerie */
/* FRAMES are as follows:    */
/* sit:      0 - 1           */
/*                           */
/* look:     2 - 7           */
/*                           */
/* point:    8 - 17          */
/*                           */
/* shrug:   18 - 30          */
/*                           */
/* scratch: 31 - 50          */
/*                           */
/* rm110b.aa - king - slathan*/
/* FRAMES are as follows:       */
/* rock toss    0 - 9 if ending */
/*  goto 10 else                */
/*                              */
/* toss_again   1 - 9           */
/*                              */
/* talk:       11 - 13          */
/*                              */
/* point:      14 - 27          */
/*                              */
/* nod:        28 - 32          */
/*                              */
/* look_around 33 - 42          */
/*                              */
/* sit         43 - 44          */
/*                              */
/* rm110c.aa - king - soptus    */
/* FRAMES are as follows:       */
/* stand face in       0 - 1    */
/*                              */
/* reach:              2 - 13   */
/*                              */
/* stand show face:   14 - 15   */
/*                              */
/* just say no:       16 - 32   */
/*                              */
/* peer:              33 - 36   */
/*                              */
/* turn to king:      37 - 41   */
/*                              */
/* bow:               42 - 52   */
/*                              */
/* talk:              53 - 58   */
/*                              */
/* point:             59 - 67   */
/*                              */
/* turn to merchant:  68 - 73   */
/* rm110d.aa - king - merchant  */
/* FRAMES are as follows:       */
/* stand:              0 - 2    */
/*                              */
/* bend over:          3 - 13   */
/*                              */
/* clean counter 1:   14 - 19   */
/*                              */
/* reach:             20 - 28   */
/*                              */
/* clean counter 2:   29 - 31   */
/*                              */
/* shrug:             32 - 46   */
/*                              */
/* just say no:       47 - 59   */
/*                              */
/* fold arms:         60 - 68   */
/*                              */
/* turn away:         69 - 73   */
/*                              */
/* hand on chin:      74 - 90   */
/*                              */

/* conversations */
/* conv008.con   */
/* conv014.con   */
/* conv009.con   */
/* conv006.con   */
/* conv007.con   */
/* conv010.con   */

#define CONV_FAERIE         8
#define CONV_SLATHAN        14
#define CONV_SOPTUS         9
#define CONV_MERCHANT       6
#define CONV_GUARD          7
#define CONV_GUARD_PID      10
#define CONV_GUARD_HEAL     12
#define CONV_GUARD_CASTLE   11

/* faerie conversation states */
#define CONV8_SIT                        1
#define CONV8_LOOK                       2
#define CONV8_POINT                      3
#define CONV8_SHRUG                      4
#define CONV8_SCRATCH                    5

/* slathan conversation states */
#define CONV14_SIT                       1
#define CONV14_ROCK_TOSS                 2
#define CONV14_TOSS_AGAIN                3
#define CONV14_TALK                      4
#define CONV14_POINT                     5
#define CONV14_NOD                       6
#define CONV14_NOD_TALK                  7
#define CONV14_LOOK_AROUND               8

/* soptus conversation states    */
#define CONV9_STAND_FACE_IN              1
#define CONV9_REACH                      2
#define CONV9_STAND_SHOW_FACE            3
#define CONV9_JUST_SAY_NO                4
#define CONV9_PEER                       5
#define CONV9_BOW_TURN_TO_MERCHANT       6
#define CONV9_TALK                       7
#define CONV9_TURN_TO_MERCHANT           8
#define CONV9_TURN_BOW                   9
#define CONV9_POINT_TALK                 10
#define CONV9_TALK_TURN_TO_MERCHANT      11
#define CONV9_TURN_BOW_TALK              12
#define CONV9_LOOK_AT_KING               13

/* merchant conversation states    */
#define CONV6_STAND                      1
#define CONV6_BEND_OVER                  2
#define CONV6_CLEAN_COUNTER_1            3
#define CONV6_REACH                      4
#define CONV6_CLEAN_COUNTER_2            5
#define CONV6_SHRUG                      6
#define CONV6_JUST_SAY_NO                7
#define CONV6_FOLD_ARMS                  8
#define CONV6_TURN_AWAY                  9
#define CONV6_HAND_ON_CHIN               10

/* guard conversation states */
#define CONV7_TALK                       0
#define CONV7_SHUT_UP                    1

/* guard conversation states */
#define CONV10_TALK                       0
#define CONV10_SHUT_UP                    1
#define CONV10_KILL                       2
#define CONV10_POINT                      3


/* dynamic hotspots */
#define FAERIE_0_X                       299
#define FAERIE_0_Y                       90
#define FAERIE_0_X_SIZE                  17
#define FAERIE_0_Y_SIZE                  21

#define FAERIE_1_X                       295
#define FAERIE_1_Y                       111
#define FAERIE_1_X_SIZE                  8
#define FAERIE_1_Y_SIZE                  13

#define SLATHAN_0_X                      96
#define SLATHAN_0_Y                      92
#define SLATHAN_0_X_SIZE                 16
#define SLATHAN_0_Y_SIZE                 17

#define SLATHAN_1_X                      102
#define SLATHAN_1_Y                      109
#define SLATHAN_1_X_SIZE                 18
#define SLATHAN_1_Y_SIZE                 15

#define GUARD_X                          16
#define GUARD_Y                          86
#define GUARD_X_SIZE                     14
#define GUARD_Y_SIZE                     38

/* random numbers for faerie */
/* for faerie */
#define RANDOM_LOW_NUMBER                1
#define RANDOM_HIGH_NUMBER               25
#define RANDOM_SIT_ALLOWED               25

/* random numbers for slathan */
#define RANDOM_SLATHAN_LOW_NUMBER        1
#define RANDOM_SLATHAN_HIGH_NUMBER       30
#define RANDOM_SLATHAN_SIT_ALLOWED       17
#define RANDOM_SLATHAN_TOSS_ALLOWED      27

#define RANDOM_END_NUMBER                3
#define RANDOM_CHOICE                    2

/* random numbers for soptus */
#define RANDOM_SOPTUS_LOW                1
#define RANDOM_SOPTUS_HIGH               50
#define RANDOM_SOPTUS_STAND_FACE_IN      44
#define RANDOM_SOPTUS_SHOW_FACE_HIGH     47
#define RANDOM_SOPTUS_REACH_HIGH         48
#define RANDOM_SOPTUS_PEER_HIGH          49

/* random numbers for merchant */
#define RANDOM_MERCHANT_LOW              1
#define RANDOM_MERCHANT_HIGH             40
#define RANDOM_MERCHANT_STAND            26
#define RANDOM_MERCHANT_BEND_OVER        27
#define RANDOM_MERCHANT_CLEAN_COUNTER_1  31
#define RANDOM_MERCHANT_REACH            32
#define RANDOM_MERCHANT_CLEAN_COUNTER_2  35
#define RANDOM_MERCHANT_SHRUG            36
#define RANDOM_MERCHANT_JUST_SAY_NO      37
#define RANDOM_MERCHANT_FOLD_ARMS        38
#define RANDOM_MERCHANT_TURN_AWAY        39

#define RANDOM_MERCHANT_HIGH_NUMBER      3

/* random numbers for guard */
#define RANDOM_GUARD_CHOICE_LOW          1
#define RANDOM_GUARD_CHOICE_HIGH         3

/* for who talking to in conversations */
#define NOONE                            0
#define FAERIE                           1
#define SOPTUS                           2
#define SLATHAN                          3
#define MERCHANT                         4

/* random numbers for everyone */
#define RANDOM_LOWEST_NUMBER             1
#define RANDOM_HIGHEST_NUMBER            6

/* timer macros */
#define TIME_TO_MOVE_1                   200
#define TIME_TO_MOVE_2                   300
#define TIME_TO_MOVE_3                   400
#define TIME_TO_MOVE_4                   500
#define TIME_TO_MOVE_5                   600
#define TIME_TO_MOVE_6                   700
#define TIME_TO_MOVE_7                   300

/* how long for talking */
#define TALK_SLATHAN_MEDIUM              20
#define TALK_SLATHAN_LONG                40
#define TALK_SOPTUS_SHORT                10
#define TALK_SOPTUS_MEDIUM               25
#define TALK_GUARD_SHORT                 10
#define TALK_GUARD_MEDIUM                25

#define LENGTH_OF_LIFE                   900


static void room_110_init() {
	if (global[llanie_status] == IS_SAVED) {
		global[make_504_empty] = true;
	}

	if (previous_room != KERNEL_RESTORING_GAME) {
		local->anim_0_running = false;
		local->anim_1_running = false;
		local->anim_2_running = false;
		local->anim_3_running = false;
		local->anim_4_running = false;
		local->anim_5_running = false;
		local->activate_timer = false;
	}

	local->death_timer = 0;
	local->clock = 0;
	local->cut_scene = false;
	local->speech_playing = false;

	/* If player is king, run conversations and animations with people */

	if (global[player_persona] == PLAYER_IS_KING) {
		if (!player_has(fruit)) {
			ss[fx_reach_fruit] = kernel_load_series("*KGRH_9", false);
		}
		ss[fx_king_in_well] = kernel_load_series(kernel_name('a', -1), false);

		if (object[bird_figurine].location == 303) {
			local->faerie_id[0] = kernel_add_dynamic(words_faerie, words_walk_to, SYNTAX_MASC_NOT_PROPER,
				KERNEL_NONE, FAERIE_0_X,
				FAERIE_0_Y, FAERIE_0_X_SIZE,
				FAERIE_0_Y_SIZE);
			kernel_dynamic_walk(local->faerie_id[0], WALK_TO_FAERIE_X, WALK_TO_FAERIE_Y, FACING_NORTHEAST);
			kernel_dynamic_hot[local->faerie_id[0]].prep = PREP_ON;

			local->faerie_id[1] = kernel_add_dynamic(words_faerie, words_walk_to, SYNTAX_MASC_NOT_PROPER,
				KERNEL_NONE, FAERIE_1_X,
				FAERIE_1_Y, FAERIE_1_X_SIZE,
				FAERIE_1_Y_SIZE);
			kernel_dynamic_walk(local->faerie_id[1], WALK_TO_FAERIE_X, WALK_TO_FAERIE_Y, FACING_NORTHEAST);
			kernel_dynamic_hot[local->faerie_id[1]].prep = PREP_ON;
		}

		if (object[pid_doll].location == 204) {
			local->slathan_id[0] = kernel_add_dynamic(words_shapechanger, words_walk_to, SYNTAX_MASC_NOT_PROPER,
				KERNEL_NONE, SLATHAN_0_X,
				SLATHAN_0_Y, SLATHAN_0_X_SIZE,
				SLATHAN_0_Y_SIZE);
			kernel_dynamic_walk(local->slathan_id[0], WALK_TO_SLATHAN_X, WALK_TO_SLATHAN_Y, FACING_NORTHWEST);
			kernel_dynamic_hot[local->slathan_id[0]].prep = PREP_ON;

			local->slathan_id[1] = kernel_add_dynamic(words_shapechanger, words_walk_to, SYNTAX_MASC_NOT_PROPER,
				KERNEL_NONE, SLATHAN_1_X,
				SLATHAN_1_Y, SLATHAN_1_X_SIZE,
				SLATHAN_1_Y_SIZE);
			kernel_dynamic_walk(local->slathan_id[1], WALK_TO_SLATHAN_X, WALK_TO_SLATHAN_Y, FACING_NORTHWEST);
			kernel_dynamic_hot[local->slathan_id[1]].prep = PREP_ON;
		}

		local->soptus_id = kernel_add_dynamic(words_Soptus_Ecliptus, words_walk_to, SYNTAX_MASC_NOT_PROPER,
			KERNEL_NONE, 0, 0, 0, 0);
		kernel_dynamic_walk(local->soptus_id, WALK_TO_SOPTUS_X, WALK_TO_SOPTUS_Y, FACING_NORTHWEST);
		kernel_dynamic_hot[local->soptus_id].prep = PREP_ON;

		local->merchant_id = kernel_add_dynamic(words_merchant, words_walk_to, SYNTAX_MASC_NOT_PROPER,
			KERNEL_NONE, 0, 0, 0, 0);
		kernel_dynamic_walk(local->merchant_id, WALK_TO_MERCHANT_X, WALK_TO_MERCHANT_Y, FACING_NORTHWEST);
		kernel_dynamic_hot[local->merchant_id].prep = PREP_ON;

		local->guard_id = kernel_add_dynamic(words_guard_captain, words_walk_to, SYNTAX_FEM_NOT_PROPER,
			KERNEL_NONE, GUARD_X,
			GUARD_Y, GUARD_X_SIZE,
			GUARD_Y_SIZE);
		kernel_dynamic_walk(local->guard_id, WALK_TO_GUARD_X, WALK_TO_GUARD_Y, FACING_NORTHWEST);
		kernel_dynamic_hot[local->guard_id].prep = PREP_ON;

		if (object[bird_figurine].location == 303) {
			/* Run faerie animation */
			aa[0] = kernel_run_animation(kernel_name('A', -1), 0);
			local->new_action[0] = CONV8_SIT;
			local->last_action[0] = CONV8_SIT;
			local->good_number[0] = false;
			local->change_action[0] = 0;
			local->anim_0_running = true;
		}

		if (object[pid_doll].location == 204) {
			/* Run slathan animation */
			aa[1] = kernel_run_animation(kernel_name('B', -1), 0);
			kernel_reset_animation(aa[1], 43);
			local->new_action[1] = CONV14_SIT;
			local->last_action[1] = CONV14_SIT;
			local->good_number[1] = false;
			local->change_action[1] = 0;
			local->slathan_talk_count = 0;
			local->slathan_timer = 0;
			local->anim_1_running = true;
		}

		/* Run soptus animation */
		aa[2] = kernel_run_animation(kernel_name('C', -1), 0);
		kernel_dynamic_anim(local->soptus_id, aa[2], 0);
		local->new_action[2] = CONV9_STAND_FACE_IN;
		local->last_action[2] = CONV9_STAND_FACE_IN;
		local->good_number[2] = false;
		local->change_action[2] = 0;
		local->soptus_talk_count = 0;
		local->soptus_timer = 0;
		local->anim_2_running = true;

		/* Run merchant animation */
		aa[3] = kernel_run_animation(kernel_name('D', -1), 0);
		kernel_dynamic_anim(local->merchant_id, aa[3], 0);
		local->new_action[3] = CONV6_STAND;
		local->last_action[3] = CONV6_STAND;
		local->good_number[3] = false;
		local->change_action[3] = 0;
		local->anim_3_running = true;

		/* Run guard animation */
		aa[4] = kernel_run_animation(kernel_name('E', -1), 0);
		local->guard_king_action = CONV7_SHUT_UP;
		local->anim_4_running = true;

		/* load conversations */
		local->talking_to = NOONE;
		conv_get(CONV_FAERIE);
		conv_get(CONV_SLATHAN);
		conv_get(CONV_SOPTUS);
		conv_get(CONV_MERCHANT);
		conv_get(CONV_GUARD);

	} else {
		kernel_flip_hotspot(words_guard_captain, false);
		ss[fx_pid_in_well] = kernel_load_series(kernel_name('b', 0), false);
		local->guard_id = kernel_add_dynamic(words_guard_captain, words_walk_to, SYNTAX_FEM_NOT_PROPER,
			KERNEL_NONE, 0, 0, 0, 0);
		kernel_dynamic_walk(local->guard_id, WALK_TO_GUARD_2_X, WALK_TO_GUARD_2_Y, FACING_WEST);
		kernel_dynamic_hot[local->guard_id].prep = PREP_ON;

		aa[5] = kernel_run_animation(kernel_name('f', -1), 0);
		local->guard_pid_action = CONV10_SHUT_UP;
		local->anim_5_running = true;

		kernel_dynamic_anim(local->guard_id, aa[5], 0);

		if (global[reset_conv] >= 0) {
			conv_reset(global[reset_conv]);
			global[reset_conv] = -1;
		}

		if (global[reset_conv_2] >= 0) {
			conv_reset(global[reset_conv_2]);
			global[reset_conv_2] = -1;
		}

		conv_get(CONV_GUARD_HEAL);   /* 12 */
		conv_get(CONV_GUARD_PID);    /* 10 */
		conv_get(CONV_GUARD_CASTLE); /* 11 */
	}

	/* for testing only - king walks off screen becomes Pid */
	if (previous_room == 120 || (previous_room == 111 && !global[invoked_from_111]) ||
		previous_room == 112 || previous_room == 110) {

		if (previous_room == 112) {
			player.x = WALK_TO_GUARD_2_X;
			player.y = WALK_TO_GUARD_2_Y;
			player.facing = FACING_WEST;

		} else {
			if (global[guard_pid_status] == GUARD_IS_UNHEALED || global[player_persona] == PLAYER_IS_KING) {
				player_first_walk(START_X_ROOM_120 + 30, START_Y_ROOM_120, FACING_WEST,
					START_X_ROOM_120, START_Y_ROOM_120, FACING_WEST, true);
			} else {
				player_first_walk(START_X_ROOM_120 + 30, START_Y_ROOM_120, FACING_WEST,
					START_X_ROOM_120, START_Y_ROOM_120, FACING_WEST, false);
			}
		}

		/* Player comes from strategic map or dragonsphere or guard captain cut scenes */
		if (global[player_persona] == PLAYER_IS_PID) {

			if (global[guard_pid_status] == GUARD_NEVER_HEALED) {
				/* global[no_talk_to_guard] = true; */
				player.commands_allowed = false;
				conv_run(CONV_GUARD_PID);
				conv_export_value(player_has(shifter_ring));
				if (global[llanie_status] == IS_SAVED) {
					conv_export_value(1);
				} else {
					conv_export_value(0);
				}

			} else if (global[guard_pid_status] == GUARD_IS_HEALED) {
				/* global[no_talk_to_guard] = true; */
				player.commands_allowed = false;
				conv_run(CONV_GUARD_HEAL);
			}
		}

	} else if (previous_room == KERNEL_RESTORING_GAME) {

		if (conv_restore_running == CONV_FAERIE) {
			conv_run(CONV_FAERIE);

		} else if (conv_restore_running == CONV_SOPTUS) {
			conv_run(CONV_SOPTUS);
			local->new_action[2] = CONV9_LOOK_AT_KING;
			local->last_action[2] = CONV9_LOOK_AT_KING;
			kernel_reset_animation(aa[2], 52);

		} else if (conv_restore_running == CONV_SLATHAN) {
			conv_run(CONV_SLATHAN);

		} else if (conv_restore_running == CONV_MERCHANT) {
			conv_run(CONV_MERCHANT);

		} else if (conv_restore_running == CONV_GUARD) {
			conv_run(CONV_GUARD);
			if (object[bird_figurine].location == 303) conv_export_value(true);
			else conv_export_value(false);
			if (object[pid_doll].location == 204) conv_export_value(true);
			else conv_export_value(false);
			local->random_grunt_only = imath_random(RANDOM_GUARD_CHOICE_LOW, RANDOM_GUARD_CHOICE_HIGH);
			conv_export_value(local->random_grunt_only);

		} else if (conv_restore_running == CONV_GUARD_PID) {
			player.commands_allowed = false;
			conv_run(CONV_GUARD_PID);
			conv_export_value(player_has(shifter_ring));
			if (global[llanie_status] == IS_SAVED) {
				conv_export_value(1);
			} else {
				conv_export_value(0);
			}

		} else if (conv_restore_running == CONV_GUARD_HEAL) {
			player.commands_allowed = false;
			conv_run(CONV_GUARD_HEAL);
		}


	} else if (previous_room == 113) {          /* Player comes from Well */
		player.x = START_X_ROOM_113;
		player.y = START_Y_ROOM_113;
		player.facing = FACING_SOUTH;

	} else if (previous_room == 118) {          /* Player comes from castle */
		player_first_walk(START_X_ROOM_118 - 30, START_Y_ROOM_118, FACING_EAST,
			START_X_ROOM_118, START_Y_ROOM_118, FACING_EAST, true);

	} else if (previous_room != KERNEL_RESTORING_GAME) {   /* Player could have used signet ring */

		if (global[player_persona] == PLAYER_IS_PID) {

			if (global[guard_pid_status] == GUARD_IS_UNHEALED) {
				player.x = START_X_ROOM_118;
				player.y = START_Y_ROOM_118;
				player.facing = FACING_EAST;

			} else {

				player.x = START_X_ROOM_120;
				player.y = START_Y_ROOM_120;
				player.facing = FACING_WEST;

				if (global[guard_pid_status] == GUARD_NEVER_HEALED) {
					player.commands_allowed = false;
					conv_run(CONV_GUARD_PID);
					conv_export_value(player_has(shifter_ring));
					if (global[llanie_status] == IS_SAVED) {
						conv_export_value(1);
					} else {
						conv_export_value(0);
					}

				} else if (global[guard_pid_status] == GUARD_IS_HEALED) {
					player.commands_allowed = false;
					conv_run(CONV_GUARD_HEAL);
				}
			}

		} else {
			player.x = START_X_ROOM_118;
			player.y = START_Y_ROOM_118;
			player.facing = FACING_EAST;
		}
	}

	global[invoked_from_111] = false;

	/* kernel_timing_trigger (10, MUSIC); */

	section_1_music();
}

static void handle_animation_guard_king() {
	int guard_king_reset_frame;

	if (kernel_anim[aa[4]].frame != local->guard_king_frame) {
		local->guard_king_frame = kernel_anim[aa[4]].frame;
		guard_king_reset_frame = -1;

		switch (local->guard_king_frame) {

		case 1:   /* end of freeze */
		case 11:  /* end of talk   */
		case 17:  /* end of talk   */

			switch (local->guard_king_action) {

			case CONV7_SHUT_UP:
				guard_king_reset_frame = 0;
				break;

			case CONV7_TALK:
				guard_king_reset_frame = 11;   /* talk (new node) */
				break;

			}
			break;

		case 12: /* end of coming to talk */
		case 13: /* end of talk frame 1   */
		case 14: /* end of talk frame 2   */
		case 15: /* end of talk frame 3   */

			guard_king_reset_frame = imath_random(12, 14);
			++local->guard_king_talk_count;
			if (local->guard_king_talk_count > 13) {
				local->guard_king_action = CONV7_SHUT_UP;
				local->guard_king_talk_count = 0;
				guard_king_reset_frame = 15;
			}
			break;
		}

		if (guard_king_reset_frame >= 0) {
			kernel_reset_animation(aa[4], guard_king_reset_frame);
			local->guard_king_frame = guard_king_reset_frame;
		}
	}
}

static void handle_animation_guard_pid() {
	int guard_pid_reset_frame;

	if (kernel_anim[aa[5]].frame != local->guard_pid_frame) {
		local->guard_pid_frame = kernel_anim[aa[5]].frame;
		guard_pid_reset_frame = -1;

		switch (local->guard_pid_frame) {

		case 29:
			global_speech_go(7); /* hurl */
			break;

		case 36:
			global_speech_go(6); /* stab */
			break;

		case 1:   /* end of freeze              */
		case 15:  /* end of point               */
		case 53:  /* end of talk (move hand up) */
			switch (local->guard_pid_action) {
			case CONV10_SHUT_UP:
				guard_pid_reset_frame = 0;  /* shut up */
				break;

			case CONV10_KILL:
				player.commands_allowed = false;
				guard_pid_reset_frame = 18; /* run Pid through */
				break;

			case CONV10_TALK:
				if (imath_random(1, 2) == 1) {
					guard_pid_reset_frame = 45;   /* talk (move hand up) */
				} else {
					guard_pid_reset_frame = 0;   /* shut up */
				}

				if (local->guard_pid_action != CONV10_KILL) {
					local->guard_pid_action = CONV10_SHUT_UP;
				}
				break;

			case CONV10_POINT:
				guard_pid_reset_frame = 3;   /* talk (actually point) */
				if (local->guard_pid_action != CONV10_KILL) {
					local->guard_pid_action = CONV10_SHUT_UP;
				}
				break;
			}
			break;

		case 47:  /* end of coming to talk */
		case 48:  /* end of talk 1         */
		case 49:  /* end of talk 2         */
			guard_pid_reset_frame = imath_random(47, 48);
			++local->guard_pid_talk_count;
			if (local->guard_pid_talk_count > 8) {
				if (local->guard_pid_action != CONV10_KILL) {
					local->guard_pid_action = CONV10_SHUT_UP;
				}
				local->guard_pid_talk_count = 0;
				guard_pid_reset_frame = 50;
			}
			break;

		case 28: /* thing getting shoved thru body */
			player.walker_visible = false;
			kernel_synch(KERNEL_PLAYER, 0, KERNEL_ANIM, aa[5]);
			break;

		case 45:  /* end of running Pid through */
			global[reset_conv] = 10;
			/* conv_reset (CONV_GUARD_PID); */
			if (game.difficulty == EASY_MODE) {
				text_show(11045);
			} else {
				text_show(45);
			}
			global[no_talk_to_guard] = false;
			kernel.force_restart = true;
			break;
		}

		if (guard_pid_reset_frame >= 0) {
			kernel_reset_animation(aa[5], guard_pid_reset_frame);
			local->guard_pid_frame = guard_pid_reset_frame;
		}
	}
}

static void room_110_get_random_faerie() {
	int random;

	local->last_action[0] = local->new_action[0];
	random = imath_random(RANDOM_LOW_NUMBER, RANDOM_HIGH_NUMBER);
	if (random < RANDOM_SIT_ALLOWED) {
		local->new_action[0] = CONV8_SIT;
		local->good_number[0] = true;
	} else {
		if (local->last_action[0] != CONV8_LOOK) {
			local->new_action[0] = CONV8_LOOK;
			local->good_number[0] = true;
		}
	}
}

static void room_110_get_random_slathan() {
	int random;

	local->last_action[1] = local->new_action[1];
	random = imath_random(RANDOM_SLATHAN_LOW_NUMBER, RANDOM_SLATHAN_HIGH_NUMBER);
	if (random <= RANDOM_SLATHAN_SIT_ALLOWED) {
		local->new_action[1] = CONV14_SIT;
		local->good_number[1] = true;
	} else {
		if (random > RANDOM_SLATHAN_SIT_ALLOWED && random <= RANDOM_SLATHAN_TOSS_ALLOWED) {
			if (local->last_action[1] != CONV14_ROCK_TOSS && local->last_action[1] != CONV14_TOSS_AGAIN) {
				local->new_action[1] = CONV14_ROCK_TOSS;
			} else {
				local->new_action[1] = CONV14_TOSS_AGAIN;
			}
			local->good_number[1] = true;
		} else {
			if (local->last_action[1] != CONV14_LOOK_AROUND) {
				local->new_action[1] = CONV14_LOOK_AROUND;
				local->good_number[1] = true;
			}
		}
	}
}

/**
 * Get random times for anyone and everyone who needs to time a move
 */
static void room_110_random_times(int index) {
	int random = 0;

	random = imath_random(RANDOM_LOWEST_NUMBER, RANDOM_HIGHEST_NUMBER);
	switch (random) {

	case RANDOM_LOWEST_NUMBER:
		local->switch_direction[index] = TIME_TO_MOVE_1;
		break;

	case 2:
		local->switch_direction[index] = TIME_TO_MOVE_2;
		break;

	case 3:
		local->switch_direction[index] = TIME_TO_MOVE_3;
		break;

	case 4:
		local->switch_direction[index] = TIME_TO_MOVE_4;
		break;

	case 5:
		local->switch_direction[index] = TIME_TO_MOVE_5;
		break;

	case RANDOM_HIGHEST_NUMBER:
		local->switch_direction[index] = TIME_TO_MOVE_6;
		break;
	}
}

/**
 * Hold head turn for slathan
 */
static void room_110_hold_head_turn(int16 reset_frame[6]) {
	local->diff[1] = kernel.clock - local->last_clock[1];
	if ((local->diff[1] >= 0) && (local->diff[1] <= local->switch_direction[1])) {
		local->slathan_timer += local->diff[1];
	} else {
		local->slathan_timer++;
	}
	local->last_clock[1] = kernel.clock;
	if (local->slathan_timer >= local->switch_direction[1]) {
		local->slathan_timer = 0;
		local->switch_direction[1] = 0;
		if (local->character_frame[1] == 37) {
			reset_frame[1] = 37;
		} else {
			reset_frame[1] = 40;
		}
	} else {
		if (local->character_frame[1] == 37) {
			reset_frame[1] = 36;
		} else {
			reset_frame[1] = 39;
		}
	}
}

/**
 * Make soptus switch directions standing while merchant talks to king
 */
static void room_110_soptus_stand(int16 reset_frame[6]) {
	local->diff[2] = kernel.clock - local->last_clock[2];
	if ((local->diff[2] >= 0) && (local->diff[2] <= local->switch_direction[2])) {
		local->soptus_timer += local->diff[2];
	} else {
		local->soptus_timer++;
	}
	local->last_clock[2] = kernel.clock;
	if (local->soptus_timer >= local->switch_direction[2]) {
		local->switch_direction[2] = 0;
		local->soptus_timer = 0;
		if (local->character_frame[2] == 2) {
			reset_frame[2] = 14;
		} else {
			reset_frame[2] = 0;
		}
	} else {
		if (local->character_frame[2] == 2) {
			reset_frame[2] = 0;
		} else {
			reset_frame[2] = 14;
		}
	}
}

static void room_110_soptus_noone_talking() {
	int random;

	local->last_action[2] = local->new_action[2];
	random = imath_random(RANDOM_SOPTUS_LOW, RANDOM_SOPTUS_HIGH);
	if (random <= RANDOM_SOPTUS_STAND_FACE_IN) {
		local->new_action[2] = CONV9_STAND_FACE_IN;
		local->good_number[2] = true;
	} else {
		if (random > RANDOM_SOPTUS_STAND_FACE_IN && random <= RANDOM_SOPTUS_SHOW_FACE_HIGH) {
			local->new_action[2] = CONV9_STAND_SHOW_FACE;
			local->good_number[2] = true;
		} else {
			if (random > RANDOM_SOPTUS_SHOW_FACE_HIGH && random <= RANDOM_SOPTUS_REACH_HIGH) {
				if (local->last_action[2] != CONV9_REACH) {
					local->new_action[2] = CONV9_REACH;
					local->good_number[2] = true;
				}
			} else {
				if (random > RANDOM_SOPTUS_REACH_HIGH && random <= RANDOM_SOPTUS_PEER_HIGH) {
					if (local->last_action[2] != CONV9_PEER) {
						local->new_action[2] = CONV9_PEER;
						local->good_number[2] = true;
					}
				} else {
					if (local->last_action[2] != CONV9_JUST_SAY_NO) {
						local->new_action[2] = CONV9_JUST_SAY_NO;
						local->good_number[2] = true;
					}
				}
			}
		}
	}
}

static void room_110_soptus_anyone_talking(int16 reset_frame[6]) {
	if (local->merchant_talking) {
		room_110_random_times(2);
		room_110_soptus_stand(reset_frame);
	} else {
		if (local->soptus_talking) {
			if ((local->new_action[2] == CONV9_TURN_BOW_TALK) ||
				(local->new_action[2] == CONV9_TURN_BOW)) {
				reset_frame[2] = 37;
			} else {
				local->good_number[2] = false;
				while (!local->good_number[2]) {
					room_110_soptus_noone_talking();
				}
				if (local->new_action[2] == CONV9_STAND_FACE_IN) {
					reset_frame[2] = 0;
				}
				if (local->new_action[2] == CONV9_STAND_SHOW_FACE) {
					reset_frame[2] = 14;
				}
				if (local->new_action[2] == CONV9_REACH) {
					reset_frame[2] = 2;
				}
				if (local->new_action[2] == CONV9_PEER) {
					reset_frame[2] = 33;
				}
				if (local->new_action[2] == CONV9_JUST_SAY_NO) {
					reset_frame[2] = 16;
				}
			}
		}
	}
}

/**
 * Random moves for merchant while soptus talking to king
 */
static void room_110_merchant_random_moves() {
	int random;

	random = imath_random(RANDOM_MERCHANT_LOW, RANDOM_MERCHANT_HIGH);
	if (random <= RANDOM_MERCHANT_STAND) {
		local->new_action[3] = CONV6_STAND;
	} else {
		if (random > RANDOM_MERCHANT_STAND && random <= RANDOM_MERCHANT_BEND_OVER) {
			if (local->last_action[3] != CONV6_BEND_OVER) {
				local->new_action[3] = CONV6_BEND_OVER;
			}
		} else {
			if (random > RANDOM_MERCHANT_BEND_OVER && random <= RANDOM_MERCHANT_CLEAN_COUNTER_1) {
				local->new_action[3] = CONV6_CLEAN_COUNTER_1;
			} else {
				if (random > RANDOM_MERCHANT_REACH && random <= RANDOM_MERCHANT_CLEAN_COUNTER_2) {
					local->new_action[3] = CONV6_CLEAN_COUNTER_2;
				} else {
					if (random > RANDOM_MERCHANT_FOLD_ARMS && random <= RANDOM_MERCHANT_TURN_AWAY) {
						if (local->last_action[3] != CONV6_TURN_AWAY) {
							local->new_action[3] = CONV6_TURN_AWAY;
						}
					} else {
						if (random > RANDOM_MERCHANT_TURN_AWAY && random <= RANDOM_MERCHANT_HIGH) {
							if (local->last_action[3] != CONV6_HAND_ON_CHIN) {
								local->new_action[3] = CONV6_HAND_ON_CHIN;
							}
						} else {
							local->new_action[3] = CONV6_STAND;
						}
					}
				}
			}
		}
	}                    /* gotta love them braces! */
}

static void room_110_merchant_noone_talking() {
	int random;

	random = imath_random(RANDOM_MERCHANT_LOW, RANDOM_MERCHANT_HIGH);
	if (random <= RANDOM_MERCHANT_STAND) {
		local->new_action[3] = CONV6_STAND;
		local->good_number[3] = true;
	}
	if (random > RANDOM_MERCHANT_STAND && random <= RANDOM_MERCHANT_BEND_OVER) {
		if (local->last_action[3] != CONV6_BEND_OVER) {
			local->new_action[3] = CONV6_BEND_OVER;
			local->good_number[3] = true;
		}
	}
	if (random > RANDOM_MERCHANT_BEND_OVER && random <= RANDOM_MERCHANT_CLEAN_COUNTER_1) {
		local->new_action[3] = CONV6_CLEAN_COUNTER_1;
		local->good_number[3] = true;
	}
	if (random > RANDOM_MERCHANT_CLEAN_COUNTER_1 && random <= RANDOM_MERCHANT_REACH) {
		if (local->last_action[3] != CONV6_REACH) {
			local->new_action[3] = CONV6_REACH;
			local->good_number[3] = true;
		}
	}
	if (random > RANDOM_MERCHANT_REACH && random <= RANDOM_MERCHANT_CLEAN_COUNTER_2) {
		local->new_action[3] = CONV6_CLEAN_COUNTER_2;
		local->good_number[3] = true;
	}
	if (random > RANDOM_MERCHANT_CLEAN_COUNTER_2 && random <= RANDOM_MERCHANT_SHRUG) {
		if (local->last_action[3] != CONV6_SHRUG) {
			local->new_action[3] = CONV6_SHRUG;
			local->good_number[3] = true;
		}
	}
	if (random > RANDOM_MERCHANT_SHRUG && random <= RANDOM_MERCHANT_JUST_SAY_NO) {
		if (local->last_action[3] != CONV6_JUST_SAY_NO) {
			local->new_action[3] = CONV6_JUST_SAY_NO;
			local->good_number[3] = true;
		}
	}
	if (random > RANDOM_MERCHANT_JUST_SAY_NO && random <= RANDOM_MERCHANT_FOLD_ARMS) {
		if (local->last_action[3] != CONV6_FOLD_ARMS) {
			local->new_action[3] = CONV6_FOLD_ARMS;
			local->good_number[3] = true;
		}
	}
	if (random > RANDOM_MERCHANT_FOLD_ARMS && random <= RANDOM_MERCHANT_TURN_AWAY) {
		if (local->last_action[3] != CONV6_TURN_AWAY) {
			local->new_action[3] = CONV6_TURN_AWAY;
			local->good_number[3] = true;
		}
	}
	if (random > RANDOM_MERCHANT_TURN_AWAY && random <= RANDOM_MERCHANT_HIGH) {
		if (local->last_action[3] != CONV6_HAND_ON_CHIN) {
			local->new_action[3] = CONV6_HAND_ON_CHIN;
			local->good_number[3] = true;
		}
	}
}

static void room_110_daemon() {
	int16 reset_frame[6];
	int random;
	long dif;

	if (local->anim_4_running) {
		handle_animation_guard_king();
	}

	if (local->anim_5_running) {
		handle_animation_guard_pid();
	}

	local->merchant_talking = (conv_control.running == CONV_MERCHANT);
	local->soptus_talking = (conv_control.running == CONV_SOPTUS);

	/* Control faerie conversation and animation */

	if (local->anim_0_running) {
		if (kernel_anim[aa[0]].frame != local->character_frame[0]) {
			local->character_frame[0] = kernel_anim[aa[0]].frame;
			reset_frame[0] = -1;

			if (local->change_action[0] > 0) {
				local->new_action[0] = local->change_action[0];
			}

			switch (local->character_frame[0]) {
			case 2:
				/* Has finished sitting */
				if (local->new_action[0] == CONV8_SIT) {
					local->good_number[0] = false;
					while (!local->good_number[0]) {
						room_110_get_random_faerie();
					}
				}
				if (local->new_action[0] == CONV8_SIT) {
					reset_frame[0] = 0;
				}
				if (local->new_action[0] == CONV8_POINT) {
					reset_frame[0] = 8;
				}
				if (local->new_action[0] == CONV8_SHRUG) {
					reset_frame[0] = 18;
				}
				if (local->new_action[0] == CONV8_SCRATCH) {
					reset_frame[0] = 31;
				}
				break;

			case 8:
				/* Has finished looking */
				if (local->new_action[0] == CONV8_LOOK) {
					local->good_number[0] = false;
					while (!local->good_number[0]) {
						room_110_get_random_faerie();
					}
				}
				if (local->new_action[0] == CONV8_SIT) {
					reset_frame[0] = 0;
				}
				if (local->new_action[0] == CONV8_LOOK) {
					reset_frame[0] = 2;
				}
				if (local->new_action[0] == CONV8_SHRUG) {
					reset_frame[0] = 18;
				}
				if (local->new_action[0] == CONV8_SCRATCH) {
					reset_frame[0] = 31;
				}
				break;

			case 18:	/* Has finished pointing */
			case 31:	/* Has finished shrugging */
			case 50:	/* Has finished scratching */
				local->change_action[0] = 0;
				local->good_number[0] = false;
				while (!local->good_number[0]) {
					room_110_get_random_faerie();
				}
				if (local->new_action[0] == CONV8_SIT) {
					reset_frame[0] = 0;
				}
				if (local->new_action[0] == CONV8_LOOK) {
					reset_frame[0] = 2;
				}
				break;
			}
			if (reset_frame[0] >= 0) {
				if (reset_frame[0] != kernel_anim[aa[0]].frame) {
					kernel_reset_animation(aa[0], reset_frame[0]);
					local->character_frame[0] = reset_frame[0];
				}
			}
		}
	}

	/* Control slathan (shapechanger) conversation and animation */

	if (local->anim_1_running) {
		if (kernel_anim[aa[1]].frame != local->character_frame[1]) {
			local->character_frame[1] = kernel_anim[aa[1]].frame;
			reset_frame[1] = -1;

			if (local->change_action[1] > 0) {
				local->new_action[1] = local->change_action[1];
				local->change_action[1] = 0;
			}

			switch (local->character_frame[1]) {
			case 10:
				/* Has almost finished tossing determine whether to toss again */
				/* or finish tossing.                                          */
				if ((local->new_action[1] == CONV14_ROCK_TOSS) ||
					(local->new_action[1] == CONV14_TOSS_AGAIN)) {
					random = imath_random(RANDOM_LOW_NUMBER, RANDOM_END_NUMBER);
					if (random == RANDOM_CHOICE) {
						local->good_number[1] = false;
						while (!local->good_number[1]) {
							room_110_get_random_slathan();
						}
						if (local->new_action[1] == CONV14_TOSS_AGAIN) {
							reset_frame[1] = 1;
						}
					} else {
						local->new_action[1] = CONV14_TOSS_AGAIN;
						reset_frame[1] = 1;
					}
				}
				break;

			case 11:
				/* Has finished tossing */
				if (local->new_action[1] == CONV14_SIT) {
					reset_frame[1] = 43;
				}
				if (local->new_action[1] == CONV14_LOOK_AROUND) {
					reset_frame[1] = 33;
				}
				if (local->new_action[1] == CONV14_TALK) {
					reset_frame[1] = 11;
				}
				if (local->new_action[1] == CONV14_POINT) {
					reset_frame[1] = 14;
				}
				if ((local->new_action[1] == CONV14_NOD) ||
					(local->new_action[1] == CONV14_NOD_TALK)) {
					reset_frame[1] = 28;
				}
				break;

			case 12:
			case 13:
			case 14:
				/* End of three talking frames */
				reset_frame[1] = imath_random(11, 13);  /* between 3 talking frames */
				++local->slathan_talk_count;                    /* run through so many talk cycles */

				if (local->slathan_talk_count > local->how_long_to_talk) {
					local->slathan_talk_count = 0;
					local->good_number[1] = false;
					while (!local->good_number[1]) {
						room_110_get_random_slathan();
					}
					if (local->new_action[1] == CONV14_SIT) {
						reset_frame[1] = 43;
					}
					if (local->new_action[1] == CONV14_LOOK_AROUND) {
						reset_frame[1] = 33;
					}
					if (local->new_action[1] == CONV14_ROCK_TOSS) {
						reset_frame[1] = 0;
					}
				}
				break;

			case 28:
				/* Has finished pointing */
				local->new_action[1] = CONV14_SIT;
				reset_frame[1] = 43;
				break;

			case 33:
				/* Has finished nodding */
				if (local->new_action[1] == CONV14_NOD) {
					local->good_number[1] = false;
					while (!local->good_number[1]) {
						room_110_get_random_slathan();
					}
				}
				if (local->new_action[1] == CONV14_ROCK_TOSS) {
					reset_frame[1] = 0;
				}
				if (local->new_action[1] == CONV14_LOOK_AROUND) {
					reset_frame[1] = 33;
				}
				if (local->new_action[1] == CONV14_SIT) {
					reset_frame[1] = 43;
				}
				if (local->new_action[1] == CONV14_NOD_TALK) {
					reset_frame[1] = 11;
				}
				if (local->new_action[1] == CONV14_TALK) {
					reset_frame[1] = 11;
				}
				break;

			case 37:
			case 40:
				/* Hold head turns for a random amount of time */
				if (local->new_action[1] == CONV14_POINT) {
					reset_frame[1] = 14;
				}
				if (local->new_action[1] == CONV14_SIT) {
					reset_frame[1] = 43;
				}
				if ((local->new_action[1] == CONV14_NOD) ||
					(local->new_action[1] == CONV14_NOD_TALK)) {
					reset_frame[1] = 28;
				}
				if (local->new_action[1] == CONV14_TALK) {
					reset_frame[1] = 11;
				}
				if (local->new_action[1] == CONV14_LOOK_AROUND) {
					room_110_random_times(1);
					room_110_hold_head_turn(reset_frame);
				}
				break;

			case 43:
				/* Has finished look around */
				if (local->new_action[1] != CONV14_SIT) {
					if (local->new_action[1] == CONV14_LOOK_AROUND) {
						local->good_number[1] = false;
						while (!local->good_number[1]) {
							room_110_get_random_slathan();
						}
					}
					if (local->new_action[1] == CONV14_ROCK_TOSS) {
						reset_frame[1] = 0;
					}
					if (local->new_action[1] == CONV14_POINT) {
						reset_frame[1] = 14;
					}
					if (local->new_action[1] == CONV14_SIT) {
						reset_frame[1] = 43;
					}
					if ((local->new_action[1] == CONV14_NOD) ||
						(local->new_action[1] == CONV14_NOD_TALK)) {
						reset_frame[1] = 28;
					}
					if (local->new_action[1] == CONV14_TALK) {
						reset_frame[1] = 11;
					}
				}
				break;

			case 45:
				/* Has finished sit */
				if (local->new_action[1] == CONV14_SIT) {
					local->good_number[1] = false;
					while (!local->good_number[1]) {
						room_110_get_random_slathan();
					}
				}

				if (local->new_action[1] == CONV14_ROCK_TOSS) {
					reset_frame[1] = 0;
				}
				if (local->new_action[1] == CONV14_POINT) {
					reset_frame[1] = 14;
				}
				if (local->new_action[1] == CONV14_SIT) {
					reset_frame[1] = 43;
				}
				if (local->new_action[1] == CONV14_TALK) {
					reset_frame[1] = 11;
				}
				if ((local->new_action[1] == CONV14_NOD) ||
					(local->new_action[1] == CONV14_NOD_TALK)) {
					reset_frame[1] = 28;
				}
				if (local->new_action[1] == CONV14_LOOK_AROUND) {
					reset_frame[1] = 33;
				}
				break;
			}
			if (reset_frame[1] >= 0) {
				if (reset_frame[1] != kernel_anim[aa[1]].frame) {
					kernel_reset_animation(aa[1], reset_frame[1]);
					local->character_frame[1] = reset_frame[1];
				}
			}
		}
	}

	/* Control soptus conversation and animation */

	if (local->anim_2_running) {
		if (kernel_anim[aa[2]].frame != local->character_frame[2]) {
			local->character_frame[2] = kernel_anim[aa[2]].frame;
			reset_frame[2] = -1;

			if (local->change_action[2] > 0) {
				local->new_action[2] = local->change_action[2];
				local->change_action[2] = 0;
			}

			switch (local->character_frame[2]) {
			case 2:
			case 16:
				/* end standing facing in */
				if (local->merchant_talking || local->soptus_talking) {
					room_110_soptus_anyone_talking(reset_frame);
				} else {
					local->good_number[2] = false;
					while (!local->good_number[2]) {
						room_110_soptus_noone_talking();
					}
					if ((local->new_action[2] == CONV9_STAND_FACE_IN) ||
						(local->new_action[2] == CONV9_STAND_SHOW_FACE)) {
						room_110_random_times(2);
						room_110_soptus_stand(reset_frame);
					}
					if (local->new_action[2] == CONV9_REACH) {
						reset_frame[2] = 2;
					}
					if (local->new_action[2] == CONV9_JUST_SAY_NO) {
						reset_frame[2] = 16;
					}
					if (local->new_action[2] == CONV9_PEER) {
						reset_frame[2] = 33;
					}
				}
				break;

			case 14:
				/* end of reaching in */
				if (local->merchant_talking || local->soptus_talking) {
					room_110_soptus_anyone_talking(reset_frame);
				} else {
					local->good_number[2] = false;
					while (!local->good_number[2]) {
						room_110_soptus_noone_talking();
					}
					if (local->new_action[2] == CONV9_STAND_FACE_IN) {
						reset_frame[2] = 0;
					}
					if (local->new_action[2] == CONV9_STAND_SHOW_FACE) {
						reset_frame[2] = 14;
					}
					if (local->new_action[2] == CONV9_JUST_SAY_NO) {
						reset_frame[2] = 16;
					}
					if (local->new_action[2] == CONV9_PEER) {
						reset_frame[2] = 33;
					}
				}
				break;

			case 33:
				/* end of just say no */
				if (local->merchant_talking || local->soptus_talking) {
					room_110_soptus_anyone_talking(reset_frame);
				} else {
					local->good_number[2] = false;
					while (!local->good_number[2]) {
						room_110_soptus_noone_talking();
					}
					if (local->new_action[2] == CONV9_STAND_FACE_IN) {
						reset_frame[2] = 0;
					}
					if (local->new_action[2] == CONV9_STAND_SHOW_FACE) {
						reset_frame[2] = 14;
					}
					if (local->new_action[2] == CONV9_REACH) {
						reset_frame[2] = 2;
					}
					if (local->new_action[2] == CONV9_PEER) {
						reset_frame[2] = 33;
					}
				}
				break;

			case 37:
				/* end of peering */
				if (local->merchant_talking || local->soptus_talking) {
					room_110_soptus_anyone_talking(reset_frame);
				} else {
					local->good_number[2] = false;
					while (!local->good_number[2]) {
						room_110_soptus_noone_talking();
					}
					if (local->new_action[2] == CONV9_STAND_FACE_IN) {
						reset_frame[2] = 0;
					}
					if (local->new_action[2] == CONV9_STAND_SHOW_FACE) {
						reset_frame[2] = 14;
					}
					if (local->new_action[2] == CONV9_JUST_SAY_NO) {
						reset_frame[2] = 16;
					}
					if (local->new_action[2] == CONV9_REACH) {
						reset_frame[2] = 2;
					}
				}
				break;

			case 52:
				/* after bowing make him stay in stand position by switching */
				/* to the turn_bow mode.                                     */
				if (local->new_action[2] == CONV9_BOW_TURN_TO_MERCHANT) {
					reset_frame[2] = 68;
				}
				break;

			case 53:
				/* turning to king to talk, keep 52 as main stand position */
				/* to go into other actions */
				if (local->new_action[2] == CONV9_LOOK_AT_KING) {
					reset_frame[2] = 52;
				}
				if (local->new_action[2] == CONV9_TURN_BOW) {
					local->new_action[2] = CONV9_LOOK_AT_KING;
					reset_frame[2] = 52;
				}
				if (local->new_action[2] == CONV9_POINT_TALK) {
					reset_frame[2] = 59;
				}
				if (local->new_action[2] == CONV9_TALK) {
					reset_frame[2] = 53;
				}
				if (local->new_action[2] == CONV9_TALK_TURN_TO_MERCHANT) {
					reset_frame[2] = 53;
				}
				if (local->new_action[2] == CONV9_BOW_TURN_TO_MERCHANT) {
					reset_frame[2] = 42;
				}
				if (local->new_action[2] == CONV9_TURN_BOW_TALK) {
					reset_frame[2] = 52;
					local->new_action[2] = CONV9_LOOK_AT_KING;
					conv_release();
				}

				break;

			case 54:
			case 55:
			case 56:
			case 57:
			case 58:
			case 59:
				/* randomly pick talking sequences for specified amounts of time.*/
				reset_frame[2] = imath_random(53, 58);   /* between talking frames */
				++local->soptus_talk_count;               /* run through so many talk cycles */

				if (local->soptus_talk_count > local->how_long_to_talk) {
					local->soptus_talk_count = 0;
					if (local->new_action[2] == CONV9_TALK_TURN_TO_MERCHANT) {
						reset_frame[2] = 68;
					} else {
						if (local->new_action[2] == CONV9_BOW_TURN_TO_MERCHANT) {
							reset_frame[2] = 42;
						} else {
							local->new_action[2] = CONV9_LOOK_AT_KING;
							reset_frame[2] = 52;
						}
					}
				} else {
					if (local->new_action[2] == CONV9_TALK_TURN_TO_MERCHANT) {
						reset_frame[2] = 68;
					} else {
						if (local->new_action[2] == CONV9_BOW_TURN_TO_MERCHANT) {
							reset_frame[2] = 42;
						}
					}
				}
				break;

			case 68:
				/* end of pointing, must talk */
				if (local->new_action[2] == CONV9_POINT_TALK) {
					local->new_action[2] = CONV9_TALK;
					reset_frame[2] = 53;
				}
				break;

			case 74:
				/* end of turn to face merchant, must stand facing in */

				/* if ((local->new_action[2] == CONV9_BOW_TURN_TO_MERCHANT) || */
					/* (local->new_action[2] == CONV9_TALK_TURN_TO_MERCHANT)) { */
				local->new_action[2] = CONV9_STAND_FACE_IN;
				reset_frame[2] = 0;
				/* } */
				break;

			}

			if (reset_frame[2] >= 0) {
				if (reset_frame[2] != kernel_anim[aa[2]].frame) {
					kernel_reset_animation(aa[2], reset_frame[2]);
					local->character_frame[2] = reset_frame[2];
				}
			}
		}
	}

	/* Control merchant conversation and animation */

	if (local->anim_3_running) {
		if (kernel_anim[aa[3]].frame != local->character_frame[3]) {
			local->character_frame[3] = kernel_anim[aa[3]].frame;
			reset_frame[3] = -1;

			if (local->change_action[3] > 0) {
				local->new_action[3] = local->change_action[3];
				local->change_action[3] = 0;
			}

			switch (local->character_frame[3]) {
			case  3:
			case 20:
			case 32:
				if (local->soptus_talking) {
					if (local->new_action[3] != CONV6_REACH) {
						room_110_merchant_random_moves();
					}
				} else {
					if (!local->merchant_talking) {
						local->good_number[3] = false;
						while (!local->good_number[3]) {
							room_110_merchant_noone_talking();
						}
					}
				}
				if (local->new_action[3] == CONV6_STAND) {
					reset_frame[3] = 0;
				}

				if (local->new_action[3] == CONV6_CLEAN_COUNTER_1) {
					random = imath_random(RANDOM_MERCHANT_LOW, RANDOM_MERCHANT_HIGH_NUMBER);
					local->time += 1;
					if (local->time <= RANDOM_MERCHANT_HIGH_NUMBER) {
						reset_frame[3] = 14;
					} else {
						reset_frame[3] = 0;
						local->new_action[3] = CONV6_STAND;
						local->time = 0;
					}
				}

				if (local->new_action[3] == CONV6_CLEAN_COUNTER_2) {
					random = imath_random(RANDOM_MERCHANT_LOW, RANDOM_MERCHANT_HIGH_NUMBER);
					local->time += 1;
					if (local->time <= RANDOM_MERCHANT_HIGH_NUMBER) {
						reset_frame[3] = 29;
					} else {
						reset_frame[3] = 0;
						local->time = 0;
						local->new_action[3] = CONV6_STAND;
					}
				}
				if (local->new_action[3] == CONV6_REACH) {
					reset_frame[3] = 20;
				}
				if (local->new_action[3] == CONV6_SHRUG) {
					reset_frame[3] = 32;
				}
				if (local->new_action[3] == CONV6_JUST_SAY_NO) {
					reset_frame[3] = 47;
				}
				if (local->new_action[3] == CONV6_FOLD_ARMS) {
					reset_frame[3] = 60;
				}
				if (local->new_action[3] == CONV6_TURN_AWAY) {
					reset_frame[3] = 69;
				}
				if (local->new_action[3] == CONV6_HAND_ON_CHIN) {
					reset_frame[3] = 74;
				}
				break;

			case 26:
				if (local->merchant_talking && local->new_action[3] == CONV6_REACH) {
					if (!player_has(fruit)) {
						local->new_action[3] = CONV6_STAND;
						player.walker_visible = false;
						seq[fx_reach_fruit] = kernel_seq_pingpong(ss[fx_reach_fruit],
							true, 8, 0, 0, 2);
						kernel_seq_range(seq[fx_reach_fruit], 1, 4);
						kernel_seq_player(seq[fx_reach_fruit], true);
						kernel_seq_trigger(seq[fx_reach_fruit],
							KERNEL_TRIGGER_SPRITE, 4, ROOM_110_GOT_FRUIT);
						kernel_seq_trigger(seq[fx_reach_fruit],
							KERNEL_TRIGGER_EXPIRE, 0, ROOM_110_SHOW_WALKER);
					}
				}
				break;

			case 14:
			case 29:
			case 47:
			case 60:
			case 69:
			case 74:
			case 91:
				local->last_action[3] = local->new_action[3];
				local->new_action[3] = CONV6_STAND;
				reset_frame[3] = 2;
				break;

			}


			if (reset_frame[3] >= 0) {
				if (reset_frame[3] != kernel_anim[aa[3]].frame) {
					kernel_reset_animation(aa[3], reset_frame[3]);
					local->character_frame[3] = reset_frame[3];
				}
			}
		}
	}

	/* obtaining fruit from the merchant */
	if (kernel.trigger == ROOM_110_GOT_FRUIT) {
		/* sound_queue (N_PickUpObject006);*/
		sound_play(N_TakeObjectSnd);
		inter_give_to_player(fruit);
		object_examine(fruit, 807, 0);
	}

	if (kernel.trigger == ROOM_110_SHOW_WALKER) {
		kernel_synch(KERNEL_PLAYER, 0, KERNEL_SERIES, seq[fx_reach_fruit]);
		player.walker_visible = true;
		conv_release();
	}

	if (local->activate_timer) {
		player.commands_allowed = true;

		dif = kernel.clock - local->clock;
		if ((dif >= 0) && (dif <= 4)) {
			local->death_timer += dif;
		} else {
			local->death_timer += 1;
		}
		local->clock = kernel.clock;

		if (local->death_timer >= LENGTH_OF_LIFE) {
			global[reset_conv_2] = 12;
			local->guard_pid_action = CONV10_KILL;
			local->activate_timer = false;
			player.commands_allowed = false;
		}
	}

	if (kernel.trigger == MUSIC) {
		sound_play(N_WayStationMus);
	}

	if (local->cut_scene && !player.walking) {
		local->cut_scene = false;
		global[dragon_my_scene]--;
	}
}

static void handle_conv_guard_king() {
	int you_trig_flag = false;

	if (kernel.trigger == ROOM_110_YOU_TALK) {
		local->guard_king_action = CONV7_TALK;
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_110_YOU_TALK);
	} /* if you_trig_flag == true, then a you trigger is called from above, not here. */

	local->guard_king_talk_count = 0;
}

static void handle_conv_guard_pid() {
	int you_trig_flag = false;

	if (player_verb == conv010_die_b_b) {
		local->guard_pid_action = CONV10_KILL;
		you_trig_flag = true;
	}

	if (player_verb == conv010_leave_b_b) {
		conv_me_trigger(ROOM_110_START_WALKING);
		you_trig_flag = true;
	}

	if (player_verb == conv010_hold_only) {
		local->guard_pid_action = CONV10_POINT;
		you_trig_flag = true;
	}

	if (player_verb == conv010_fix_b_b) {
		local->activate_timer = true;
	}

	if (kernel.trigger == ROOM_110_START_WALKING) {  /* to 120 */
		player.commands_allowed = false;
		player_walk(START_X_ROOM_120 + 30, START_Y_ROOM_120, FACING_EAST);
		player_walk_trigger(ROOM_110_GO_TO_120);
	}

	if (kernel.trigger == ROOM_110_YOU_TALK) {
		if (local->guard_pid_action != CONV10_KILL) {
			local->guard_pid_action = CONV10_TALK;
		}
	}

	if (!you_trig_flag) {
		conv_you_trigger(ROOM_110_YOU_TALK);
	} /* if you_trig_flag == true, then a you trigger is called from above, not here. */

	local->guard_pid_talk_count = 0;
}


void handle_conv_guard_heal() {
	if (player_verb == conv012_gash_b_b) {
		player.x = 100;
		new_room = 112;
	}
}


void room_110_check_score() {
	switch (global[talked_to_status]) {

	case TALKED_TO_NONE:
		if (local->talking_to == FAERIE) {
			global[talked_to_status] = TALKED_FAERIE;
		}
		if (local->talking_to == SOPTUS) {
			global[talked_to_status] = TALKED_SOPTUS;
		}
		if (local->talking_to == SLATHAN) {
			global[talked_to_status] = TALKED_SLATHAN;
		}
		if (local->talking_to == MERCHANT) {
			global[talked_to_status] = TALKED_MERCHANT;
		}
		global[player_score] += 1;
		break;

	case TALKED_FAERIE:
		if (local->talking_to == SOPTUS) {
			global[player_score] += 1;
			global[talked_to_status] = TALKED_FAERIE_SOPTUS;
		}
		if (local->talking_to == SLATHAN) {
			global[player_score] += 1;
			global[talked_to_status] = TALKED_FAERIE_SLATHAN;
		}
		if (local->talking_to == MERCHANT) {
			global[player_score] += 1;
			global[talked_to_status] = TALKED_FAERIE_MERCHANT;
		}
		break;

	case TALKED_SOPTUS:
		if (local->talking_to == FAERIE) {
			global[player_score] += 1;
			global[talked_to_status] = TALKED_FAERIE_SOPTUS;
		}
		if (local->talking_to == SLATHAN) {
			global[player_score] += 1;
			global[talked_to_status] = TALKED_SOPTUS_SLATHAN;
		}
		if (local->talking_to == MERCHANT) {
			global[player_score] += 1;
			global[talked_to_status] = TALKED_SOPTUS_MERCHANT;
		}
		break;

	case TALKED_SLATHAN:
		if (local->talking_to == FAERIE) {
			global[player_score] += 1;
			global[talked_to_status] = TALKED_FAERIE_SLATHAN;
		}
		if (local->talking_to == SOPTUS) {
			global[player_score] += 1;
			global[talked_to_status] = TALKED_SOPTUS_SLATHAN;
		}
		if (local->talking_to == MERCHANT) {
			global[player_score] += 1;
			global[talked_to_status] = TALKED_SLATHAN_MERCHANT;
		}
		break;

	case TALKED_MERCHANT:
		if (local->talking_to == FAERIE) {
			global[player_score] += 1;
			global[talked_to_status] = TALKED_FAERIE_MERCHANT;
		}
		if (local->talking_to == SOPTUS) {
			global[player_score] += 1;
			global[talked_to_status] = TALKED_SOPTUS_MERCHANT;
		}
		if (local->talking_to == SLATHAN) {
			global[player_score] += 1;
			global[talked_to_status] = TALKED_SLATHAN_MERCHANT;
		}
		break;

	case TALKED_FAERIE_SOPTUS:
		if (local->talking_to == SLATHAN) {
			global[player_score] += 1;
			global[talked_to_status] = TALKED_FAERIE_SOPTUS_SLATHAN;
		}
		if (local->talking_to == MERCHANT) {
			global[player_score] += 1;
			global[talked_to_status] = TALKED_FAERIE_SOPTUS_MERCHANT;
		}
		break;

	case TALKED_FAERIE_SLATHAN:
		if (local->talking_to == SOPTUS) {
			global[player_score] += 1;
			global[talked_to_status] = TALKED_FAERIE_SOPTUS_SLATHAN;
		}
		if (local->talking_to == MERCHANT) {
			global[player_score] += 1;
			global[talked_to_status] = TALKED_FAERIE_SLATHAN_MERCHANT;
		}
		break;

	case TALKED_FAERIE_MERCHANT:
		if (local->talking_to == SOPTUS) {
			global[player_score] += 1;
			global[talked_to_status] = TALKED_FAERIE_SOPTUS_MERCHANT;
		}
		if (local->talking_to == SLATHAN) {
			global[player_score] += 1;
			global[talked_to_status] = TALKED_FAERIE_SLATHAN_MERCHANT;
		}
		break;

	case TALKED_SOPTUS_SLATHAN:
		if (local->talking_to == FAERIE) {
			global[player_score] += 1;
			global[talked_to_status] = TALKED_FAERIE_SOPTUS_SLATHAN;
		}
		if (local->talking_to == MERCHANT) {
			global[player_score] += 1;
			global[talked_to_status] = TALKED_SOPTUS_SLATHAN_MERCHANT;
		}
		break;

	case TALKED_SLATHAN_MERCHANT:
		if (local->talking_to == FAERIE) {
			global[player_score] += 1;
			global[talked_to_status] = TALKED_FAERIE_SLATHAN_MERCHANT;
		}
		if (local->talking_to == SOPTUS) {
			global[player_score] += 1;
			global[talked_to_status] = TALKED_SOPTUS_SLATHAN_MERCHANT;
		}
		break;

	case TALKED_FAERIE_SOPTUS_SLATHAN:
		if (local->talking_to == MERCHANT) {
			global[player_score] += 1;
			global[talked_to_status] = TALKED_TO_ALL;
		}
		break;

	case TALKED_FAERIE_SOPTUS_MERCHANT:
		if (local->talking_to == SLATHAN) {
			global[player_score] += 1;
			global[talked_to_status] = TALKED_TO_ALL;
		}
		break;

	case TALKED_FAERIE_SLATHAN_MERCHANT:
		if (local->talking_to == SOPTUS) {
			global[player_score] += 1;
			global[talked_to_status] = TALKED_TO_ALL;
		}
		break;

	case TALKED_SOPTUS_SLATHAN_MERCHANT:
		if (local->talking_to == FAERIE) {
			global[player_score] += 1;
			global[talked_to_status] = TALKED_TO_ALL;
		}
		break;
	}
}

void handle_conv_faerie() {
	if ((player_verb == conv008_query_wish) ||
		(player_verb == conv008_query_humans) ||
		(player_verb == conv008_query_where) ||
		(player_verb == conv008_banter_only)) {

		switch (kernel.trigger) {
		case 0:
			conv_you_trigger(1);
			break;

		case 1:
			local->change_action[0] = CONV8_POINT;
			break;
		}
	}
	if ((player_verb == conv008_query_respect) ||
		(player_verb == conv008_query_norespect)) {

		switch (kernel.trigger) {
		case 0:
			conv_you_trigger(1);
			break;

		case 1:
			local->change_action[0] = CONV8_SHRUG;
			break;
		}
	}
	if ((player_verb == conv008_query_why_annoy) ||
		(player_verb == conv008_query_king)) {
		switch (kernel.trigger) {
		case 0:
			conv_you_trigger(1);
			break;

		case 1:
			local->change_action[0] = CONV8_SCRATCH;
			break;
		}
	}
}

void handle_conv_slathan() {
	if (player_verb == conv014_heehee_only) {
		switch (kernel.trigger) {
		case 0:
			conv_you_trigger(1);
			break;

		case 1:
			local->change_action[1] = CONV14_POINT;
			break;
		}
	}

	if (player_verb == conv014_query_respect) {
		switch (kernel.trigger) {
		case 0:
			conv_you_trigger(1);
			break;

		case 1:
			local->how_long_to_talk = TALK_SLATHAN_LONG;
			local->change_action[1] = CONV14_NOD_TALK;
			break;
		}
	}

	if ((player_verb == conv014_query_wish) ||
		(player_verb == conv014_query_humans) ||
		(player_verb == conv014_query_why_annoy)) {
		switch (kernel.trigger) {
		case 0:
			conv_you_trigger(1);
			break;

		case 1:
			if ((player_verb == conv014_query_wish) ||
				(player_verb == conv014_query_why_annoy)) {
				local->how_long_to_talk = TALK_SLATHAN_MEDIUM;
			} else {
				local->how_long_to_talk = TALK_SLATHAN_LONG;
			}
			local->change_action[1] = CONV14_TALK;
			break;
		}
	}

	if (player_verb == conv014_banter_only) {
		switch (kernel.trigger) {
		case 0:
			conv_you_trigger(1);
			break;

		case 1:
			local->change_action[1] = CONV14_NOD;
			break;
		}
	}
}


void handle_conv_soptus() {
	/* trigger 1 will not go into effect until conv is released in daemon */
	if (player_verb == conv009_hello1a_only) {
		switch (kernel.trigger) {
		case 0:
			local->change_action[2] = CONV9_TURN_BOW_TALK;
			conv_hold();
			conv_you_trigger(1);
			break;

		case 1:
			local->how_long_to_talk = TALK_SOPTUS_MEDIUM;
			local->change_action[2] = CONV9_TALK;
			break;
		}
	}

	if (player_verb == conv009_hello2a_only) {
		switch (kernel.trigger) {
		case 0:
			conv_you_trigger(1);
			break;

		case 1:
			local->change_action[2] = CONV9_TURN_BOW;
			break;
		}
	}

	if ((player_verb == conv009_choices_trip) ||
		(player_verb == conv009_choices_never)) {
		switch (kernel.trigger) {
		case 0:
			conv_you_trigger(1);
			break;

		case 1:
			local->how_long_to_talk = TALK_SOPTUS_MEDIUM;
			local->change_action[2] = CONV9_TALK;
			break;
		}
	}

	if (player_verb == conv009_choices_someday) {
		switch (kernel.trigger) {
		case 0:
			conv_you_trigger(1);
			break;

		case 1:
			local->how_long_to_talk = TALK_SOPTUS_MEDIUM;
			local->change_action[2] = CONV9_POINT_TALK;
			break;
		}
	}

	if (player_verb == conv009_exiter_leave) {
		switch (kernel.trigger) {
		case 0:
			conv_you_trigger(1);
			break;

		case 1:
			local->how_long_to_talk = TALK_SOPTUS_SHORT;
			local->change_action[2] = CONV9_TALK_TURN_TO_MERCHANT;
			break;
		}
	}
}

void handle_conv_merchant() {
	if (player_verb == conv006_wares_greeting) {
		switch (kernel.trigger) {
		case 0:
			conv_you_trigger(1);
			break;

		case 1:
			local->change_action[3] = CONV6_CLEAN_COUNTER_1;
			break;
		}
	}

	if (player_verb == conv006_how_only) {
		global[talked_to_merchant] = true;
	}

	if (player_verb == conv006_exit_b_b) {
		conv_hold();
		local->change_action[3] = CONV6_REACH;
	}

	if ((player_verb == conv006_choices_chat) ||
		(player_verb == conv006_how_only)) {
		switch (kernel.trigger) {
		case 0:
			conv_you_trigger(1);
			break;

		case 1:
			local->change_action[3] = CONV6_SHRUG;
			break;
		}
	}

	if ((player_verb == conv006_choices_sorry) ||
		(player_verb == conv006_wise_only)) {
		switch (kernel.trigger) {
		case 0:
			conv_you_trigger(1);
			break;

		case 1:
			local->change_action[3] = CONV6_FOLD_ARMS;
			break;
		}
	}

	if (player_verb == conv006_thanks_only) {
		switch (kernel.trigger) {
		case 0:
			conv_you_trigger(1);
			break;

		case 1:
			local->change_action[3] = CONV6_JUST_SAY_NO;
			break;
		}
	}
}


void room_110_pre_parser() {
	if (kernel.trigger == ROOM_110_GO_TO_120) {
		player_cancel_command();
		local->activate_timer = false;
		global[pre_room] = 110;
		if (global[dragon_my_scene] < global[dragon_high_scene]) {
			global[dragon_my_scene]++;
			new_room = 111;
			local->cut_scene = true;
		} else {
			new_room = 120;
		}
	}

	if (!local->activate_timer) {
		if (player_said_2(walk_through, gate_to_courtyard)) {
			if (global[player_persona] == PLAYER_IS_PID) {
				player.need_to_walk = false;
				conv_run(CONV_GUARD_CASTLE);
				player_cancel_command();
			} else {
				player.walk_off_edge_to_room = 118;
			}
		}

		if (!player_said_2(walk_down, road_to_east) && local->cut_scene) {
			local->cut_scene = false;
			global[dragon_my_scene]--;
		}

		if (player_said_2(walk_down, road_to_east)) {
			global[pre_room] = 110;
			if (global[dragon_my_scene] < global[dragon_high_scene]) {
				if (!local->cut_scene) {
					global[dragon_my_scene]++;
				}
				player.walk_off_edge_to_room = 111;
				local->cut_scene = true;

			} else if (local->cut_scene) {
				player.walk_off_edge_to_room = 111;

			} else {
				player.walk_off_edge_to_room = 120;
			}
		}
	}

	if (player_said_3(heal, doll, guard_captain)) {
		if (global[player_persona] == PLAYER_IS_PID &&
			global[guard_pid_status] != GUARD_IS_UNHEALED) {
			player.commands_allowed = false;
		}
	}

	if (local->activate_timer) {
		if (player_said_2(give, guard_captain)) {
			text_show(11043);
			player_cancel_command();

		} else if (player_said_1(take) ||
			player_said_1(push) ||
			player_said_1(open) ||
			player_said_1(put) ||
			player_said_1(give) ||
			player_said_1(pull) ||
			player_said_1(close) ||
			player_said_1(talk_to) ||
			player_said_2(invoke, signet_ring) ||
			player_said_1(throw)) {
			text_show(11042);
			player_cancel_command();

		} else if (player.need_to_walk) {

			if (player_said_1(walk_down)) {
				conv_reset(CONV_GUARD_PID);
				player.commands_allowed = false;
				player_walk(START_X_ROOM_120 + 30, START_Y_ROOM_120, FACING_EAST);
				player_walk_trigger(ROOM_110_GO_TO_120);

			} else if (player_said_3(heal, doll, guard_captain)) {
				local->activate_timer = false;
				player.commands_allowed = false;

			} else {
				local->guard_pid_action = CONV10_KILL;
				local->activate_timer = false;
				player.commands_allowed = false;
				player.need_to_walk = false;
				player_cancel_command();
			}

		} else if (player_said_2(look, guard_captain)) {
			text_show(11038);
			player_cancel_command();
		}
	}
}

static void room_110_parser() {
	if (kernel.trigger == ROOM_110_GO_TO_120) {
		global[pre_room] = 110;
		if (global[dragon_my_scene] < global[dragon_high_scene]) {
			global[dragon_my_scene]++;
			new_room = 111;
			local->cut_scene = true;
		} else {
			new_room = 120;
		}
		goto handled;
	}

	if (player.look_around) {
		if (global[player_persona] == PLAYER_IS_PID) {
			text_show(11019);
		} else {
			text_show(11001);
		}
		goto handled;
	}

	if (conv_control.running == CONV_FAERIE) {
		handle_conv_faerie();
		goto handled;
	}

	if (conv_control.running == CONV_SLATHAN) {
		handle_conv_slathan();
		goto handled;
	}

	if (conv_control.running == CONV_SOPTUS) {
		handle_conv_soptus();
		goto handled;
	}

	if (conv_control.running == CONV_MERCHANT) {
		handle_conv_merchant();
		goto handled;
	}

	if (conv_control.running == CONV_GUARD) {
		handle_conv_guard_king();
		goto handled;
	}

	if (conv_control.running == CONV_GUARD_PID) {
		handle_conv_guard_pid();
		goto handled;
	}

	if (conv_control.running == CONV_GUARD_HEAL) {
		handle_conv_guard_heal();
		goto handled;
	}

	if (player_said_2(jump_down, well)) {
		if (global[player_persona] == PLAYER_IS_KING) {
			switch (kernel.trigger) {
			case 0:
				player.commands_allowed = false;
				player.walker_visible = false;
				seq[fx_king_in_well] = kernel_seq_forward(ss[fx_king_in_well],
					false, 7, 0, 0, 1);
				kernel_seq_depth(seq[fx_king_in_well], 3);
				kernel_synch(KERNEL_SERIES, seq[fx_king_in_well], KERNEL_PLAYER, 0);

				if (speech_system_active && speech_on) {
					kernel_seq_trigger(seq[fx_king_in_well], KERNEL_TRIGGER_SPRITE, 16, 3);
				} else {
					kernel_seq_trigger(seq[fx_king_in_well], KERNEL_TRIGGER_SPRITE, 19, 1);
				}
				break;

			case 1:
				sound_play(N_JumpDownWell);
				kernel_timing_trigger(FOUR_SECONDS, 2);
				break;

			case 2:
				new_room = 113;
				break;

			case 3:
				global_speech_go(5);  /* falling */
				local->speech_playing = 5;
				kernel_timing_trigger(2, 4);
				break;

			case 4:
				if (local->speech_playing == 5) {
					if (speech_system_active && speech_on) {
						if (speech_status()) {
							kernel_timing_trigger(2, 4);

						} else {
							global_speech_go(2); /* bones */
							local->speech_playing = 2;
							kernel_timing_trigger(2, 4);
						}
					}

				} else if (local->speech_playing == 2) {
					if (speech_system_active && speech_on) {
						if (speech_status()) {
							kernel_timing_trigger(2, 4);

						} else {
							new_room = 113;
						}
					}

				} else {
					new_room = 113;
				}
				break;
			}

		} else {
			switch (kernel.trigger) {
			case 0:
				if (!(global[player_score_flags] & SCORE_JUMP_INTO_WELL)) {
					global[player_score_flags] = global[player_score_flags] | SCORE_JUMP_INTO_WELL;
					global[player_score] += 5;
				}
				player.commands_allowed = false;
				player.walker_visible = false;
				seq[fx_pid_in_well] = kernel_seq_forward(ss[fx_pid_in_well],
					false, 8, 0, 0, 1);
				kernel_seq_depth(seq[fx_pid_in_well], 3);
				kernel_synch(KERNEL_SERIES, seq[fx_pid_in_well], KERNEL_PLAYER, 0);

				if (speech_system_active && speech_on && !player_has(magic_belt)) {
					kernel_seq_trigger(seq[fx_pid_in_well], KERNEL_TRIGGER_SPRITE, 13, 3);
				} else {
					kernel_seq_trigger(seq[fx_pid_in_well], KERNEL_TRIGGER_SPRITE, 17, 1);
				}
				break;

			case 1:
				sound_play(N_JumpDownWell);
				kernel_timing_trigger(FOUR_SECONDS, 2);
				break;

			case 2:
				new_room = 113;
				break;

			case 3:
				global_speech_go(5);  /* falling */
				local->speech_playing = 5;
				kernel_timing_trigger(2, 4);
				break;

			case 4:
				if (local->speech_playing == 5) {
					if (speech_system_active && speech_on) {
						if (speech_status()) {
							kernel_timing_trigger(2, 4);

						} else {
							global_speech_go(2); /* bones */
							local->speech_playing = 2;
							kernel_timing_trigger(2, 4);
						}
					}

				} else if (local->speech_playing == 2) {
					if (speech_system_active && speech_on) {
						if (speech_status()) {
							kernel_timing_trigger(2, 4);

						} else {
							new_room = 113;
						}
					}

				} else {
					new_room = 113;
				}
				break;
			}
		}
		goto handled;
	}

	if (player_said_2(talk_to, faerie)) {
		local->talking_to = FAERIE;
		room_110_check_score();
		conv_run(CONV_FAERIE);
		goto handled;
	}

	if (player_said_2(talk_to, shapechanger)) {
		local->talking_to = SLATHAN;
		global[talked_to_shifter] = true;
		room_110_check_score();
		conv_run(CONV_SLATHAN);
		goto handled;
	}

	if (player_said_2(talk_to, Soptus_Ecliptus)) {
		local->talking_to = SOPTUS;
		global[talked_to_soptus] = true;
		room_110_check_score();
		conv_run(CONV_SOPTUS);
		goto handled;
	}

	if (player_said_2(talk_to, merchant)) {
		local->talking_to = MERCHANT;
		room_110_check_score();
		conv_run(CONV_MERCHANT);
		goto handled;
	}


	/* Pick random number betweeen 1 and 3 to maneuver the guard's actions */
	/* all within one entry for conversation with king.                    */
	/* In part 2, pid talks and can get shishkabobbed by the guard captain.*/

	if (player_said_2(talk_to, guard_captain)) {
		if (global[player_persona] == PLAYER_IS_KING) {
			local->talking_to = NOONE;
			conv_run(CONV_GUARD);
			if (object[bird_figurine].location == 303) conv_export_value(true);
			else conv_export_value(false);
			if (object[pid_doll].location == 204) conv_export_value(true);
			else conv_export_value(false);
			local->random_grunt_only = imath_random(RANDOM_GUARD_CHOICE_LOW, RANDOM_GUARD_CHOICE_HIGH);
			conv_export_value(local->random_grunt_only);
			goto handled;
		} else {
			conv_run(CONV_GUARD_PID);
			conv_export_value(player_has(shifter_ring));
			if (global[llanie_status] == IS_SAVED) {
				conv_export_value(1);
			} else {
				conv_export_value(0);
			}
			goto handled;
		}
	}

	if (player_said_1(look) || player_said_1(look_at)) {
		if (player_said_1(crank)) {
			text_show(11002);
			goto handled;
		}

		if (player_said_1(fields)) {
			if (global[player_persona] == PLAYER_IS_PID) {
				text_show(11020);
			} else {
				text_show(11004);
			}
			goto handled;
		}

		if (player_said_1(merchant_s_stall)) {
			if (global[player_persona] == PLAYER_IS_PID) {
				text_show(11021);
			} else if (inter_point_x >= RIGHT_STALL_X) {
				text_show(11007);
			} else {
				text_show(11006);
			}
			goto handled;
		}

		if (player_said_1(trees)) {
			text_show(11008);
			goto handled;
		}

		if (player_said_1(bucket)) {
			text_show(11009);
			goto handled;
		}

		if (player_said_1(well)) {
			text_show(11011);
			goto handled;
		}

		if (player_said_1(clouds)) {
			text_show(11013);
			goto handled;
		}

		if (player_said_1(hedge)) {
			if (global[player_persona] == PLAYER_IS_PID) {
				text_show(11022);
			} else {
				text_show(11014);
			}
			goto handled;
		}

		if (player_said_1(gate_to_courtyard)) {
			if (global[player_persona] == PLAYER_IS_PID) {
				text_show(11023);
			} else {
				text_show(11015);
			}
			goto handled;
		}

		if (player_said_1(road_to_east)) {
			if (global[player_persona] == PLAYER_IS_PID) {
				text_show(11024);
			} else {
				text_show(11016);
			}
			goto handled;
		}

		if (player_said_1(castle)) {
			if (global[player_persona] == PLAYER_IS_PID) {
				text_show(11025);
			} else {
				text_show(11017);
			}
			goto handled;
		}

		if (player_said_1(sky)) {
			if (global[player_persona] == PLAYER_IS_PID) {
				text_show(11026);
			} else {
				text_show(11018);
			}
			goto handled;
		}

		if (player_said_1(guard_captain)) {
			if (global[player_persona] == PLAYER_IS_KING) {
				text_show(11028);
			} else if (local->activate_timer) {
				if (game.difficulty == HARD_MODE) {
					text_show(11038);
				} else {
					text_show(11046);
				}
			} else if (global[guard_pid_status] == GUARD_IS_UNHEALED) {
				text_show(11039);
			}
			goto handled;
		}

		if (player_said_1(merchant)) {
			text_show(11029);
			goto handled;
		}

		if (player_said_1(shapechanger)) {
			text_show(11030);
			goto handled;
		}

		if (player_said_1(rope)) {
			text_show(11040);
			goto handled;
		}

		if (player_said_1(Soptus_Ecliptus)) {
			text_show(11031);
			goto handled;
		}

		if (player_said_1(faerie)) {
			text_show(11032);
			goto handled;
		}
	}

	if (player_said_2(gaze_into, crystal_ball)) {
		if (local->activate_timer) {
			text_show(11047);
		} else {
			text_show(11033);
		}
		goto handled;
	}

	if (player_said_3(tie, rope, well)) {
		text_show(11036);
		goto handled;
	}

	if (player_said_3(put, tentacle_parts, well) ||
		player_said_3(put, tentacle_parts, castle)) {
		text_show(11037);
		goto handled;
	}

	if ((player_said_1(push) || player_said_1(pull)) &&
		player_said_1(crank)) {
		text_show(11003);
		goto handled;
	}

	if (player_said_3(throw, rare_coin, well) ||
		player_said_3(put, rare_coin, well) ||
		player_said_3(give, rare_coin, well)) {
		text_show(11041);
		goto handled;
	}

	if ((player_said_1(throw) || player_said_1(put) || player_said_1(give)) &&
		player_said_1(well) &&
		player_has(object_named(player_main_noun))) {
		text_show(11012);
		goto handled;
	}

	if (player_said_2(walk_to, fields)) {
		text_show(11005);
		goto handled;
	}

	if (player_said_2(take, guard_captain)) {
		text_show(30);
		goto handled;
	}

	if (player_said_2(take, merchant)) {
		text_show(30);
		goto handled;
	}

	if (player_said_2(take, shapechanger)) {
		text_show(30);
		goto handled;
	}

	if (player_said_2(take, Soptus_Ecliptus)) {
		text_show(30);
		goto handled;
	}

	if (player_said_3(throw, rare_coin, well)) {
		text_show(11041);
		goto handled;
	}

	if (player_said_2(take, faerie)) {
		text_show(30);
		goto handled;
	}

	if (player_said_3(heal, doll, guard_captain)) {
		if (global[player_persona] == PLAYER_IS_PID) {

			if (global[guard_pid_status] == GUARD_IS_UNHEALED) {
				text_show(11044);

			} else {
				player.x = 0;
				new_room = 112;
			}
			goto handled;
		}
	}

	goto done;

handled:
	player.command_ready = false;

done:
	;
}

void room_110_synchronize(Common::Serializer &s) {
	for (int16 &v : scratch.sprite)           s.syncAsSint16LE(v);
	for (int16 &v : scratch.sequence)         s.syncAsSint16LE(v);
	for (int16 &v : scratch.animation)        s.syncAsSint16LE(v);
	for (int16 &v : scratch.character_frame)  s.syncAsSint16LE(v);
	for (int16 &v : scratch.faerie_id)        s.syncAsSint16LE(v);
	for (int16 &v : scratch.slathan_id)       s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.soptus_id);
	s.syncAsSint16LE(scratch.merchant_id);
	s.syncAsSint16LE(scratch.guard_id);
	for (int16 &v : scratch.new_action)       s.syncAsSint16LE(v);
	for (int16 &v : scratch.last_action)      s.syncAsSint16LE(v);
	for (int16 &v : scratch.good_number)      s.syncAsSint16LE(v);
	for (int16 &v : scratch.change_action)    s.syncAsSint16LE(v);
	s.syncAsSint16LE(scratch.talking_to);
	s.syncAsSint16LE(scratch.how_long_to_talk);
	s.syncAsSint16LE(scratch.slathan_talk_count);
	s.syncAsSint16LE(scratch.soptus_talk_count);
	s.syncAsSint16LE(scratch.anim_0_running);
	s.syncAsSint16LE(scratch.anim_1_running);
	s.syncAsSint16LE(scratch.anim_2_running);
	s.syncAsSint16LE(scratch.anim_3_running);
	s.syncAsSint16LE(scratch.anim_4_running);
	s.syncAsSint16LE(scratch.anim_5_running);
	s.syncAsSint16LE(scratch.guard_king_frame);
	s.syncAsSint16LE(scratch.guard_king_action);
	s.syncAsSint16LE(scratch.guard_king_talk_count);
	s.syncAsSint16LE(scratch.guard_pid_frame);
	s.syncAsSint16LE(scratch.guard_pid_action);
	s.syncAsSint16LE(scratch.guard_pid_talk_count);
	s.syncAsSint32LE(scratch.slathan_timer);
	for (int32 &v : scratch.diff)             s.syncAsSint32LE(v);
	for (int32 &v : scratch.last_clock)       s.syncAsSint32LE(v);
	for (int16 &v : scratch.switch_direction) s.syncAsSint16LE(v);
	s.syncAsSint32LE(scratch.soptus_timer);
	s.syncAsSint16LE(scratch.merchant_talking);
	s.syncAsSint16LE(scratch.soptus_talking);
	s.syncAsSint16LE(scratch.time);
	s.syncAsSint16LE(scratch.random_grunt_only);
	s.syncAsSint32LE(scratch.clock);
	s.syncAsSint32LE(scratch.death_timer);
	s.syncAsSint16LE(scratch.activate_timer);
	s.syncAsByte(scratch.cut_scene);
	s.syncAsSint16LE(scratch.speech_playing);
}

void room_110_preload() {
	room_init_code_pointer = room_110_init;
	room_pre_parser_code_pointer = room_110_pre_parser;
	room_parser_code_pointer = room_110_parser;
	room_daemon_code_pointer = room_110_daemon;

	section_1_walker();
	section_1_interface();

	if (global[player_persona] == PLAYER_IS_KING) {
		kernel_initial_variant = 1;
	}

	vocab_make_active(words_faerie);
	vocab_make_active(words_shapechanger);
	vocab_make_active(words_Soptus_Ecliptus);
	vocab_make_active(words_merchant);
	vocab_make_active(words_guard_captain);
	vocab_make_active(words_walk_to);
}

} // namespace Rooms
} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS
