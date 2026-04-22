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

#ifndef MADS_PHANTOM_ROOM205_H
#define MADS_PHANTOM_ROOM205_H

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

	int16 rich_frame;
	int16 rich_action;
	int16 rich_talk_count;

	int16 giry_frame;
	int16 giry_action;
	int16 giry_talk_count;

	int16 just_did_option;
	int16 anim_0_running;
	int16 anim_1_running;
	int16 converse_counter;
	int16 no_hold;       /* if true, will not conv_hold in Parser */
	int16 give_ticket;   /* if true, will give ticket */

} Scratch;


/* ========================= Sprite Series ========================= */

#define fx_left_door  0   /* rm203f0 */
#define fx_right_door 1   /* rm203f1 */
#define fx_take_9     2   /* *RDR_9  */


/* ========================= Conversations ========================= */

#define CONV_RICHARD_18  18
#define CONV_GIRY_10     10
#define CONV_GIRY_11     11

/* Conv 18 — Richard action states */
#define CONV18_RICH_TALK         0
#define CONV18_RICH_POINT        1
#define CONV18_RICH_BOTH_GO      2
#define CONV18_RICH_SHUT_UP      3
#define CONV18_RICH_BOTH_ARMS_UP 4

/* Conv 10/11 — Giry action states */
#define CONV10_GIRY_TALK       0
#define CONV10_GIRY_POINT_TALK 1
#define CONV10_GIRY_SHUT_UP    2
#define CONV10_GIRY_NOD        3
#define CONV10_GIRY_TRANCE     4
#define CONV10_GIRY_WELCOME    5
#define CONV10_GIRY_LEFT_DOOR  6
#define CONV10_GIRY_RIGHT_DOOR 7
#define CONV10_GIRY_HANDS_UP   8
#define CONV11_GIRY_TAKE       9


/* ========================= Triggers ============================== */

#define ROOM_205_ME_TALK            60
#define ROOM_205_RICHARD_TALK       62
#define ROOM_205_END                64
#define ROOM_205_GIRY_TALK          66
#define ROOM_205_WELCOME            68
#define ROOM_205_POINT_TALK         70
#define ROOM_205_NOD                72
#define ROOM_205_BOTH_HANDS_UP      74
#define ROOM_205_DOOR_OPENS         80
#define ROOM_205_LEFT_DOOR_CLOSES   90
#define ROOM_205_RIGHT_DOOR_CLOSES  95
#define ROOM_205_TAKE_TICKET        100
#define ROOM_205_WALK               110


/* ========================= Player positions ====================== */

#define WALK_TO_X_FROM_202       19
#define WALK_TO_Y_FROM_202       144
#define OFF_SCREEN_X_FROM_202   -20
#define OFF_SCREEN_Y_FROM_202    144

#define PLAYER_X_FROM_206  36
#define PLAYER_Y_FROM_206  68

#define PLAYER_X_FROM_207  263
#define PLAYER_Y_FROM_207  65

#define BRIE_X  132
#define BRIE_Y  112


/* ========================= Box door walk positions =============== */

#define FRONT_5_X  37
#define FRONT_5_Y  67

#define FRONT_6_X  80
#define FRONT_6_Y  68

#define FRONT_7_X  167
#define FRONT_7_Y  65

#define FRONT_8_X  212
#define FRONT_8_Y  64

#define FRONT_9_X  258
#define FRONT_9_Y  63

#define BEHIND_LEFT_X   37
#define BEHIND_LEFT_Y   64

#define BEHIND_RIGHT_X  263
#define BEHIND_RIGHT_Y  59


/* ========================= Dynamic Giry positions ================ */

#define DYN_GIRY_X_R  257
#define DYN_GIRY_Y_R  79
#define DYN_GIRY_X_L  75
#define DYN_GIRY_Y_L  84


/* ========================= Madame Giry hotspot locations ========= */

/* Left position (by box 5) */
#define HS_MADAME_X_L_1  62
#define HS_MADAME_Y_L_1  54
#define HS_MADAME_X_L_2  62
#define HS_MADAME_Y_L_2  66

/* Middle position */
#define HS_MADAME_X_M_1  113
#define HS_MADAME_Y_M_1  44
#define HS_MADAME_X_M_2  107
#define HS_MADAME_Y_M_2  66

/* Right position (by box 9) */
#define HS_MADAME_X_R_1  283
#define HS_MADAME_Y_R_1  51
#define HS_MADAME_X_R_2  289
#define HS_MADAME_Y_R_2  62


extern void room_205_init();
extern void room_205_daemon();
extern void room_205_pre_parser();
extern void room_205_parser();
extern void room_205_preload();

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
