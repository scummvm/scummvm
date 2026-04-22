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

#ifndef MADS_PHANTOM_ROOM204_H
#define MADS_PHANTOM_ROOM204_H

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

	int16 sprite[15];        /* Sprite series handles */
	int16 sequence[15];      /* Sequence handles      */
	int16 animation[4];      /* Animation handles     */
	int16 anim_0_running;
	int16 anim_1_running;
	int16 anim_2_running;
	int16 anim_3_running;
	int16 keep_raoul_down;
	int16 florent_is_gone;

	int16 prevent;
	int16 prevent_2;
	int16 prevent_3;
	int16 end_of_game;

	int16 brie_action;
	int16 brie_frame;
	int16 flor_action;
	int16 flor_frame;
	int16 raoul_action;
	int16 raoul_frame;

	int16 raoul_talk_count;

} Scratch;


/* ========================= Sprite Series ========================= */

#define fx_1993       0   /* rm204z  */
#define fx_broken_case 1  /* rm204f1 */
#define fx_book       2   /* rm204p0 */
#define fx_door       3   /* rm204x6 */
#define fx_reach_up_7 4   /* *RALRH_9 */
#define fx_take_6     5   /* *RDRR_6  */
#define fx_case       6   /* rm204f0  */


/* ========================= Conversations ========================= */

#define CONV_END_22  22

/* Conv 22 — Brie action states */
#define CONV22_BRIE_TALK    0
#define CONV22_BRIE_POINT   1
#define CONV22_BRIE_SHUT_UP 2
#define CONV22_BRIE_LEAVE   3
#define CONV22_BRIE_RIGHT_UP 4

/* Conv 22 — Florent action states */
#define CONV22_FLOR_TALK_RAOUL 0
#define CONV22_FLOR_TALK_BRIE  1
#define CONV22_FLOR_SHUT_UP    2
#define CONV22_FLOR_LEAVE      3
#define CONV22_FLOR_RAOUL_TALK 4

/* Conv 22 — Raoul action states */
#define CONV22_RAOUL_TALK      0
#define CONV22_RAOUL_SHUT_UP   1
#define CONV22_RAOUL_SIT       2
#define CONV22_RAOUL_GET_BOOK  3
#define CONV22_RAOUL_INVISIBLE 4
#define CONV22_RAOUL_GLANCE    5


/* ========================= Triggers ============================== */

#define ROOM_204_DOOR_OPENS   60
#define ROOM_204_DOOR_CLOSES  70
#define ROOM_204_ME_TALK      75
#define ROOM_204_YOU_TALK     80
#define ROOM_204_BYE          85


/* ========================= Player positions ====================== */

#define PLAYER_X_FROM_202_1881  115
#define PLAYER_Y_FROM_202_1881  147

#define PLAYER_X_FROM_202_1993  175
#define PLAYER_Y_FROM_202_1993  145

#define PLAYER_X_FROM_203  2
#define PLAYER_Y_FROM_203  140

#define WALK_TO_X_FROM_203  30
#define WALK_TO_Y_FROM_203  140

#define WALK_TO_X_BEHIND_DOOR  0
#define WALK_TO_Y_BEHIND_DOOR  136


/* ========================= Dynamic hotspot bounds ================ */

/* Coffee table (1993) */
#define COFFEE_X       83
#define COFFEE_Y       140
#define COFFEE_X_SIZE  45
#define COFFEE_Y_SIZE  12
#define WALK_TO_COFFEE_X  84
#define WALK_TO_COFFEE_Y  150

/* Comfy chairs (1881) */
#define CHAIR_1_X       220
#define CHAIR_1_Y       147
#define CHAIR_1_X_SIZE  6
#define CHAIR_1_Y_SIZE  8
#define WALK_TO_CHAIR_X  220
#define WALK_TO_CHAIR_Y  150

#define CHAIR_2_X       226
#define CHAIR_2_Y       134
#define CHAIR_2_X_SIZE  12
#define CHAIR_2_Y_SIZE  21

#define CHAIR_3_X       238
#define CHAIR_3_Y       128
#define CHAIR_3_X_SIZE  13
#define CHAIR_3_Y_SIZE  27

/* Grand foyer exits (1993) */
#define EXIT_1_X       199
#define EXIT_1_Y       147
#define EXIT_1_X_SIZE  52
#define EXIT_1_Y_SIZE  8
#define WALK_TO_EXIT_1_X  224
#define WALK_TO_EXIT_1_Y  152

#define EXIT_2_X       145
#define EXIT_2_Y       147
#define EXIT_2_X_SIZE  54
#define EXIT_2_Y_SIZE  8
#define WALK_TO_EXIT_2_X  175
#define WALK_TO_EXIT_2_Y  152

/* Grand foyer exits (1881) */
#define EXIT_3_X       199
#define EXIT_3_Y       147
#define EXIT_3_X_SIZE  19
#define EXIT_3_Y_SIZE  8
#define WALK_TO_EXIT_3_X  209
#define WALK_TO_EXIT_3_Y  152

#define EXIT_4_X       84
#define EXIT_4_Y       147
#define EXIT_4_X_SIZE  61
#define EXIT_4_Y_SIZE  8
#define WALK_TO_EXIT_4_X  115
#define WALK_TO_EXIT_4_Y  152


/* ========================= Misc ================================== */

#define DOOR_X  27
#define DOOR_Y  139


extern void room_204_init();
extern void room_204_daemon();
extern void room_204_pre_parser();
extern void room_204_parser();
extern void room_204_preload();

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
