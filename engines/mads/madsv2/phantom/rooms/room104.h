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

#ifndef MADS_PHANTOM_ROOM104_H
#define MADS_PHANTOM_ROOM104_H

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

	int sprite[15];       /* Sprite series handles */
	int sequence[15];     /* Sequence handles      */
	int animation[6];     /* Animation handles     */
	int anim_0_running;
	int anim_1_running;
	int anim_2_running;

	int get_ready_she_leave;    /* if T, Daae will, after talking, leave scene */
	int get_ready_he_leave;     /* if T, Daae will, after talking, leave scene */

	int wants_to_talk;          /* if T, Raoul will talk */
	int wants_to_get_up;        /* if T, Raoul will sit up */

	int sitting_up;             /* T if Raoul is sitting up from fall */

	int rich_action;            /* action Richard is going to perform */
	int rich_frame;

	int raoul_action;           /* action Raoul is going to perform */
	int raoul_frame;

	int daae_walk_action;       /* action Daae is going to perform */
	int daae_walk_frame;

	int couple_action;          /* action couple is going to perform */
	int couple_frame;

	int rich_talk_count;        /* counters for talking */
	int couple_he_talk_count;   /* counters for he talking */
	int couple_she_talk_count;  /* counters for she talking */
	int couple_look_rich_count; /* counter for couple looking at Richard */

	int wants_to_stand_up;      /* if T, will not talk, etc. */

	int last_player_step;       /* Frame marker for player walk */

} Scratch;


/* ========================= Sprite Series ========================= */

#define fx_trap_door    0   /* rm104x0 */
#define fx_climb_thru   1   /* rm104a0 */
#define fx_chandelier   2   /* rm104z0 */
#define fx_fall         3   /* rm104a6 */


/* ========================= Triggers ============================== */

#define ROOM_104_CLIMB_THRU           60
#define ROOM_104_TRAP_DOOR_MOVEMENT   65
#define ROOM_104_SIT_UP               67
#define ROOM_104_SHE_TALK             69
#define ROOM_104_ME_TALK              71
#define ROOM_104_KISS_HIM             73
#define ROOM_104_RICHARD_TALK         75
#define ROOM_104_SHE_TALK_TO_RICH     77
#define ROOM_104_HE_TALK_TO_RICH      79
#define ROOM_104_GET_TO_SHE_WALK      81
#define ROOM_104_COMMANDS_ALLOWED_NOT 83
#define ROOM_104_POINT                87
#define ROOM_104_ARMS_OUT             89
#define ROOM_104_INIT_TEXT            91
#define ROOM_104_END_PHANTOM          93


/* ========================= Camera positions ====================== */

#define RIGHT_STAGE     320     /* for camera_jump_to */
#define LEFT_STAGE        0     /* for camera_jump_to */
#define MIDDLE_STAGE    158     /* for camera_jump_to */


/* ========================= Player positions from room 107 ======== */

#define PLAYER_X_FROM_107        627
#define PLAYER_Y_FROM_107_1       95
#define PLAYER_Y_FROM_107_2      120
#define PLAYER_Y_FROM_107_3      142
#define OFF_SCREEN_X_FROM_107    655


/* ========================= Player positions from room 108 ======== */

#define PLAYER_X_FROM_108         12
#define PLAYER_Y_FROM_108_1      148
#define PLAYER_Y_FROM_108_2      128
#define PLAYER_Y_FROM_108_3       97
#define OFF_SCREEN_X_FROM_108    -20


/* ========================= Conversation ========================== */

#define CONV_AFTER_FALL_FROM_301    7


/* ========================= Richard (Conv 7) action states ======== */

#define CONV7_RICH_SHUT_UP    0
#define CONV7_RICH_TALK       1
#define CONV7_RICH_ARMS_OUT   2
#define CONV7_RICH_POINT      3
#define CONV7_RICH_WALK       4
#define CONV7_RICH_LOOK_UP    5


/* ========================= Couple (Conv 7) action states ========= */
/* the '_R' means Richard */

#define CONV7_COUPLE_SHUT_UP          1
#define CONV7_COUPLE_SHUT_UP_R        2
#define CONV7_COUPLE_HE_TALK          3
#define CONV7_COUPLE_SHE_TALK         4
#define CONV7_COUPLE_KISS             5
#define CONV7_COUPLE_TOUCH_HEAD       6
#define CONV7_COUPLE_HE_TALK_R        7
#define CONV7_COUPLE_SHE_TALK_R       8
#define CONV7_COUPLE_LOOK_AT_R_SHORT  9
#define CONV7_COUPLE_LOOK_AT_R_LONG   10
#define CONV7_COUPLE_STAY_DOWN        11
#define CONV7_COUPLE_SHE_TALK_LAY     12
#define CONV7_COUPLE_RAOUL_ALONE      13
#define CONV7_COUPLE_PINCH            14
#define CONV7_COUPLE_SHE_WALK         15
#define CONV7_COUPLE_HE_GET_UP        16
#define CONV7_COUPLE_INVISIBLE        17


/* ========================= Daae walk action states =============== */

#define CONV7_DAAE_WALK_INVISIBLE     0
#define CONV7_DAAE_WALK_LEAVE         1


/* ========================= Post-conversation positions =========== */

#define AFTER_CONV_X    166
#define AFTER_CONV_Y    126


/* ========================= Restore positions (from death) ======== */

#define RESTORE_X_FAR     496
#define RESTORE_Y_FAR      79
#define RESTORE_X_MIDDLE  346
#define RESTORE_Y_MIDDLE   71
#define RESTORE_X_NEAR    172
#define RESTORE_Y_NEAR     73


/* ========================= Entry from room 103 =================== */

#define NEW_ROOM_FROM_103_X  319
#define NEW_ROOM_FROM_103_Y   96


extern void room_104_init(void);
extern void room_104_daemon(void);
extern void room_104_pre_parser(void);
extern void room_104_parser(void);
extern void room_104_preload(void);

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
