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

#ifndef MADS_PHANTOM_ROOMS_101_H
#define MADS_PHANTOM_ROOMS_101_H

#include "mads/madsv2/core/general.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

#define local ((Scratch *)(&game.scratch[0]))
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

typedef struct {        /* Room local variables */
	int16 sprite[15];       /* Sprite series handles */
	int16 sequence[15];     /* Sequence handles      */
	int16 animation[4];     /* Animation handles     */
	int16 brie_calling_position;    /* Conv000 tells position of Brie */
	int16 brie_chandelier_position; /* Conv000 tells position of Brie */
	int16 brie_calling_frame;       /*	*/
	int16 brie_chandelier_frame;    /*	*/
	int16 talk_count;               /* how many times Brie will talk in CONV000 */
	int16 start_sitting_down;       /* true if picture_view_x > 208             */
	int16 dynamic_brie;             /* for dynamic Brie */
	int16 dynamic_brie_2;           /* for dynamic Brie */
	int16 execute_chan;             /* if talk_count == this var, then motion to chandelier */
	int16 execute_wipe;             /* if talk_count == this var, then wipe forehead */
	int16 start_walking;            /* if true, start walking away (conv001) */
	int16 start_walking_0;          /* if true, start walking away (conv000) */

	int16 anim_0_running;           /* T if aa[0] is running */
	int16 anim_1_running;           /* T if aa[1] is running */

	int16 converse_counter; /* counter for talking displacements */

} Scratch;


/* ========================= Sprite Series =================== */

#define fx_chandelier                  0       /* rm101z  */ 


/* ========================= Other Macros ==================== */

#define PLAYER_X_FROM_102	       625	
#define PLAYER_Y_FROM_102	       127

#define PLAYER_X_FROM_202	       18	
#define PLAYER_Y_FROM_202	       79

#define RIGHT_HALF                     320     /* for camera_jump_to */
#define LEFT_HALF                      0       /* for camera_jump_to */

#define CONV0_BRIE_ARMS_AT_SIDE        0       
#define CONV0_BRIE_RAISING_ARMS        1       
#define CONV0_BRIE_SHOULD_TALK         2       /* For conv000, Brie can do 2 different */
#define CONV0_BRIE_SHOULD_WALK         3       /* talk sequences, or turn an walk away */

#define CONV1_BRIE_SHOULD_MOTION_CHAN  0
#define CONV1_BRIE_SHOULD_WALK         1
#define CONV1_BRIE_SHOULD_TALK         2
#define CONV1_BRIE_SHOULD_FREEZE_STAND 3
#define CONV1_BRIE_SHOULD_FREEZE_SIT   4
#define CONV1_BRIE_SHOULD_RAISE_HANDS  5
#define CONV1_BRIE_SHOULD_WIPE         6

#define CONV_BRIE_MOTIONS_TO_RAOUL     0       /* CONV000 */                      
#define CONV_BRIE_BY_CHANDELIER        1       /* CONV001 */                      

#define OFF_SCREEN_X_FROM_202          -20
#define OFF_SCREEN_Y_FROM_202          75

#define OFF_SCREEN_X_FROM_102          655
#define OFF_SCREEN_Y_FROM_102          130

#define DYNAMIC_BRIE_X                 509 
#define DYNAMIC_BRIE_Y                 73
#define DYNAMIC_BRIE_X_SIZE            16
#define DYNAMIC_BRIE_Y_SIZE            39
#define DYNAMIC_BRIE_WALK_TO_X         490
#define DYNAMIC_BRIE_WALK_TO_Y         119
#define RESET                          -1

#define DYN_BRIE_1_WALK_TO_X           25
#define DYN_BRIE_1_WALK_TO_Y           80

  /* ======================== Triggers ========================= */

#define ROOM_101_START_CONV_MOTIONS    50      /* When this happens, CONV000 will start */
#define ROOM_101_START_CONV_CHAN       55      /* When this happens, CONV000 will start */
#define ROOM_101_BRIE_START_TALKING    60
#define ROOM_101_BRIE_STOP_TALKING     70
#define ROOM_101_BRIE_START_WALKING    80
#define ROOM_101_CONV0_MAKE_BRIE_WALK  90

#define ROOM_101_NO_TALK_DISP          100
#define ROOM_101_STOP_CONVERSING       105

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
