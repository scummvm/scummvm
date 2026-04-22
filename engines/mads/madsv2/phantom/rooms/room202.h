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

#ifndef MADS_PHANTOM_ROOM202_H
#define MADS_PHANTOM_ROOM202_H

#include "mads/madsv2/phantom/rooms/section2.h"
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

	int16 usher_action;
	int16 usher_frame;
	int16 usher_talk_count;

	int16 degas_action;
	int16 degas_frame;

	int16 rich_frame;
	int16 rich_action;
	int16 rich_talk_count;

	int16 anim_0_running;
	int16 anim_1_running;
	int16 converse_counter;
	int16 prevent;

	int16 chandelier_base[5];
	int16 dyn_chandeliers[5];

	int16 gave;

} Scratch;


/* ========================= Sprite Series ========================= */

#define fx_left_door     0   /* rm202x0  */
#define fx_take_9        1   /* rdr_9    */
#define fx_chandelier_0  2   /* rm202f0  */
#define fx_chandelier_1  3
#define fx_chandelier_2  4
#define fx_chandelier_3  5
#define fx_chandelier_4  6


/* ========================= Conversations ========================= */

#define CONV_USHER_17  17
#define CONV_DEGAS_9   9

/* Conv 17 — Usher action states */
#define CONV17_USHER_TALK        0
#define CONV17_USHER_POINT       1
#define CONV17_USHER_SHUT_UP     2
#define CONV17_USHER_HAVE_TICKET 3
#define CONV17_USHER_LEFT_UP     4

/* Conv 9 — Degas action states */
#define CONV9_DEGAS_TALK    0
#define CONV9_DEGAS_BOW     1
#define CONV9_DEGAS_GIRL    2
#define CONV9_DEGAS_LEAVE   3
#define CONV9_DEGAS_SHUT_UP 4


/* ========================= Triggers ============================== */

#define ROOM_202_DOOR_CLOSES    60
#define ROOM_202_USHER_TALK     70
#define ROOM_202_USHER_POINT    72
#define ROOM_202_ME_TALK        74
#define ROOM_202_END            76
#define ROOM_202_DOOR_OPENS     80
#define ROOM_202_TALK_TO_DEGAS  90
#define ROOM_202_DEGAS_TALK     93
#define ROOM_202_DEGAS_WALK     96
#define ROOM_202_DEGAS_DONE     100


/* ========================= Player positions ====================== */

#define PLAYER_X_FROM_201  3
#define PLAYER_Y_FROM_201  141

#define PLAYER_X_FROM_204  253
#define PLAYER_Y_FROM_204  117

#define PLAYER_X_FROM_205  510
#define PLAYER_Y_FROM_205  117

#define PLAYER_X_FROM_101  636
#define PLAYER_Y_FROM_101  143

#define WALK_TO_X_FROM_201  40
#define WALK_TO_Y_FROM_201  141

#define WALK_TO_X_FROM_203  132
#define WALK_TO_Y_FROM_203  134

#define WALK_TO_X_FROM_204  255
#define WALK_TO_Y_FROM_204  133

#define WALK_TO_X_FROM_205  512
#define WALK_TO_Y_FROM_205  133

#define WALK_TO_X_FROM_101  598
#define WALK_TO_Y_FROM_101  143

#define WALK_TO_X_BEHIND_LEFT_DOOR  134
#define WALK_TO_Y_BEHIND_LEFT_DOOR  112


/* ========================= Misc positions ======================== */

#define FRONT_OF_LEFT_DOOR_X  126
#define FRONT_OF_LEFT_DOOR_Y  123

#define USHER_X  569
#define USHER_Y  147

#define DEGAS_X  400
#define DEGAS_Y  141

#define DEGAS_AFTER_CONV_X  596
#define DEGAS_AFTER_CONV_Y  144

#define RIGHT_HALF  320


/* ========================= Chandelier ============================ */

#define NUM_CHANDELIERS  5
#define camera_ratio_1   1
#define camera_ratio_2   5


extern void set_chandelier_positions();

extern void room_202_init();
extern void room_202_daemon();
extern void room_202_pre_parser();
extern void room_202_parser();
extern void room_202_preload();

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
