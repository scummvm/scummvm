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

#ifndef MADS_PHANTOM_ROOM113_H
#define MADS_PHANTOM_ROOM113_H

#include "mads/madsv2/phantom/rooms/section1.h"
#include "mads/madsv2/phantom/global.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

#define local ((Scratch *)(&game.scratch[0]))
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

typedef struct {        /* Room local variables */

	int16 sprite[15];         /* Sprite series handles                          */
	int16 sequence[15];       /* Sequence handles                               */
	int16 animation[5];       /* Animation handles [0=Daae, 1=Raoul13/Raoul4's  */
	                          /*   proxy, 2=Florent, 3=Raoul4, 4=Julie]         */

	int16 chris_daae_dynamic; /* Dynamic HS for Christine (love scene)          */
	int16 chris_flor_dynamic; /* Dynamic HS for Christine/Florent (1993)        */

	int16 standing_talking;   /* T once Daae & Raoul are standing face-to-face  */
	int16 day_wants_to_talk;  /* T if Daae should start talking at next idle pt  */
	int16 music_is_playing;   /* T once angel music has been triggered           */
	int16 right_after_kiss;   /* T once kiss frame (165) has been reached        */
	int16 anim_0_running;     /* T if Daae animation is active                  */
	int16 anim_1_running;     /* T if Raoul-13 animation is active              */
	int16 anim_2_running;     /* T if Florent animation is active               */
	int16 anim_3_running;     /* T if Raoul-4 animation is active               */
	int16 anim_4_running;     /* T if Julie animation is active                 */
	int16 prevent_1;          /* Prevent first-time code from repeating         */
	int16 prevent_2;          /* Prevent first-time code from repeating         */
	int16 raoul_is_up;        /* T once Raoul has stood up from couch           */
	int16 arms_are_out;       /* T once Daae has arms outstretched (frame 95)   */

	int16 day_frame;
	int16 day_action;
	int16 day_talk_count;
	int16 stand_talk_count;
	int16 florent_frame;
	int16 florent_action;
	int16 florent_talk_count;
	int16 raoul_frame;
	int16 raoul_action;
	int16 raoul_talk_count;
	int16 julie_frame;
	int16 julie_action;
	int16 julie_talk_count;
	int16 just_did_option;

} Scratch;


/* ========================= Sprite Series ========================= */

#define fx_1993        0   /* rm113z (1993 overlay backdrop)   */
#define fx_dress       1   /* rm113f0 (Florent/Christine dress) */
#define fx_small_note  2   /* rm113f1 (note sprite)            */
#define fx_corpse      3   /* rm113c3 (dead Florent)           */
#define fx_face_1      4   /* *faceral  (1881 only)            */
#define fx_face_2      5   /* *facecrsd (1881 only)            */


/* ========================= Conversations ========================= */

#define CONV_LOVE_SCENE     13
#define CONV_FLORENT         4
#define CONV_FLORENT_DEAD    6

/* Conv 4 (Florent) — Florent action states */
#define CONV4_FLORENT_TALK		0
#define CONV4_FLORENT_SHUT_UP	1
#define CONV4_FLORENT_POINT		2
#define CONV4_FLORENT_HOLD_HAND	3

/* Conv 4 (Florent) — Raoul action states */
#define CONV4_RAOUL_TALK		0
#define CONV4_RAOUL_SHUT_UP		1
#define CONV4_RAOUL_INVISIBLE	3
#define CONV4_RAOUL_GET_UP		4
#define CONV4_RAOUL_TAKE_NOTE	5
#define CONV4_RAOUL_CHIN		2

/* Conv 6 (Julie/dead Florent) — Julie action states */
#define CONV6_JULIE_TALK  0
#define CONV6_JULIE_CRY   1

/* Conv 13 (love scene) — Daae action states */
#define CONV13_DAY_MIRROR			2	//0
#define CONV13_DAY_TALK				0	//1
#define CONV13_DAY_GET_READY_TALK	1	//2
#define CONV13_DAY_WAVE				3	//3
#define CONV13_DAY_STAND_TALK		5	//4
#define CONV13_DAY_STAND_SHUT_UP	6	//5
#define CONV13_BOTH_SHUT_UP			7	//6
#define CONV13_DAY_GET_UP			4	//7

/* Conv 13 (love scene) — Raoul action states */
#define CONV13_RAOUL_INVISIBLE		3	//0
#define CONV13_RAOUL_SHUT_UP		1	//1
#define CONV13_RAOUL_TALK			0	//2
#define CONV13_RAOUL_CHIN			2	//3


/* ========================= Triggers ============================== */

/* Daemon triggers */
#define ROOM_113_BY_MIRROR              57
#define ROOM_113_AT_CORPSE              59
#define ROOM_113_AT_DRESSER             61
#define ROOM_113_WAIT_FOR_FADE_IN       110

/* Process_conversation_dead triggers */
#define ROOM_113_JULIE_START_TALKING    63
#define ROOM_113_JULIE_STOP_TALKING     65

/* Process_conversation_florent triggers */
#define ROOM_113_RAOUL_START_TALKING	 62
#define ROOM_113_FLORENT_START_TALKING2	 64
#define ROOM_113_FLORENT_START_TALKING	 66
#define ROOM_113_MADE_IT_TO_FLORENT		 78
#define ROOM_113_AT_COUCH				 80
#define ROOM_113_BEFORE_COUCH			 82
#define ROOM_113_BETWEEN_COUCH			 84
#define ROOM_113_TAKE_NOTE				 86
#define ROOM_113_DELAY_BEFORE_WALK		 92
#define ROOM_113_PUT_ON_HOLD			 96
#define ROOM_113_HOLD_HAND				 98
#define ROOM_113_END_HOLD_HAND			100

/* Process_conversation_love triggers */
#define ROOM_113_CHECK_FOR_ARMS_OUT		70		//89
#define ROOM_113_DAY_START_TALKING		66		//91
#define ROOM_113_WAIT_TO_SIT			78		//93
#define ROOM_113_CHECK_FOR_AFTER_KISS	72		//95
#define ROOM_113_HOLD_OPTIONS			76		//97


/* ========================= Player positions ====================== */

#define PLAYER_X_FROM_111          190
#define PLAYER_Y_FROM_111          148

#define PLAYER_X_AFTER_CONV13      175
#define PLAYER_Y_AFTER_CONV13      148

#define COUCH_X                    201
#define COUCH_Y                    120

#define MIRROR_X                   272
#define MIRROR_Y                   138

#define SCREEN_X                   106
#define SCREEN_Y                   127

#define CORPSE_X                   175
#define CORPSE_Y                   128

#define FLORENT_TOUCH_HAND_X       142
#define FLORENT_TOUCH_HAND_Y        68


/* ========================= Florent dynamic hotspot =============== */

#define DYN_FLORENT_X               43
#define DYN_FLORENT_Y              118
#define DYN_FLORENT_X_SIZE          15
#define DYN_FLORENT_Y_SIZE          29


/* ========================= Misc coordinates ====================== */

#define CHRIS_DEAD_X               220
#define CHRIS_DEAD_Y               130

#define LIGHT_1993_X               155
#define LIGHT_1993_Y                17

#define LIGHT_1881_X               150
#define LIGHT_1881_Y                46

#define SMALL_NUMBER_OF_TICKS      1


extern void room_113_init();
extern void room_113_daemon();
extern void room_113_pre_parser();
extern void room_113_parser();
extern void room_113_preload();

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
