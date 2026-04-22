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

#ifndef MADS_PHANTOM_ROOM108_H
#define MADS_PHANTOM_ROOM108_H

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

	int16 sprite[15];         /* Sprite series handles */
	int16 sequence[15];       /* Sequence handles      */
	int16 animation[4];       /* Animation handles     */
	int16 anim_0_running;

	int16 char_action;        /* action Charles is going to perform */
	int16 char_frame;
	int16 char_talk_count;
	int16 char_shut_up_count;
	int16 dynamic_char;       /* Dynamic HS for Charles */
	int16 prev_shut_up_frame;
	int16 max_talk_count;     /* max talking count for Charles */
	int16 did_raise_hand;     /* true, when if talking, did raise hand */
	int16 converse_counter;   /* counter for talking displacements */

} Scratch;


/* ========================= Sprite Series ========================= */

#define fx_1993   0   /* rm108z */


/* ========================= Conversation ========================== */

#define CONV_CHARLES    2


/* ========================= Charles (Conv 2) action states ======== */

#define CONV2_CHAR_SHUT_UP        0
#define CONV2_CHAR_TALK           1
#define CONV2_CHAR_WRITE          2
#define CONV2_CHAR_OH_MY          3
#define CONV2_CHAR_POINT_UP       4
#define CONV2_CHAR_BOTH_HANDS_UP  5
#define CONV2_CHAR_TURN_TO_RAOUL  6
#define CONV2_CHAR_POINT_RIGHT    7


/* ========================= Triggers ============================== */

#define ROOM_108_CHAR_TALK           60
#define ROOM_108_CHAR_SHUT_UP        62
#define ROOM_108_CHAR_WRITE          64
#define ROOM_108_CHAR_BOTH_HANDS_UP  66
#define ROOM_108_CHAR_POINT_UP       68
#define ROOM_108_CHAR_POINT_RIGHT    70
#define ROOM_108_CHAR_OH_MY          72


/* ========================= Player positions ====================== */

#define PLAYER_X_FROM_106    48
#define PLAYER_Y_FROM_106    81

#define WALK_TO_X_FROM_106   71
#define WALK_TO_Y_FROM_106   76

#define PLAYER_X_FROM_104_1  243
#define PLAYER_X_FROM_104_2  185
#define PLAYER_X_FROM_104_3  124
#define PLAYER_Y_FROM_104    143


/* ========================= Dynamic hotspot — stool (1993) ======== */

#define STOOL_X        250
#define STOOL_Y         68
#define STOOL_X_SIZE     8
#define STOOL_Y_SIZE    21
#define STOOL_WALK_X   253
#define STOOL_WALK_Y    75


/* ========================= Dynamic hotspot — wall (1881) ========= */

#define WALL_X        258
#define WALL_Y         58
#define WALL_X_SIZE     6
#define WALL_Y_SIZE    10
#define WALL_WALK_X   236
#define WALL_WALK_Y    69


/* ========================= Dynamic hotspot — Charles ============= */

#define DYN_CHAR_X          253
#define DYN_CHAR_Y           52
#define DYN_CHAR_XS          15
#define DYN_CHAR_YS          34
#define DYN_CHAR_WALK_TO_X  235
#define DYN_CHAR_WALK_TO_Y  102


extern void room_108_init(void);
extern void room_108_daemon(void);
extern void room_108_parser(void);
extern void room_108_preload(void);

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
