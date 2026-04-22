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

#ifndef MADS_PHANTOM_ROOM203_H
#define MADS_PHANTOM_ROOM203_H

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

	int16 brie_action;
	int16 brie_frame;
	int16 brie_talk_count;

	int16 raoul_action;
	int16 raoul_frame;
	int16 raoul_talk_count;

	int16 rich_action;
	int16 rich_frame;
	int16 rich_talk_count;

	int16 daae_action;
	int16 daae_frame;
	int16 daae_talk_count;

	int16 anim_0_running;
	int16 anim_1_running;
	int16 anim_2_running;
	int16 anim_3_running;
	int16 show_note;
	int16 converse_counter;

} Scratch;


/* ========================= Sprite Series ========================= */

#define fx_1993        0   /* rm203z  */
#define fx_parchment   1   /* rm203p1 */
#define fx_parchment_2 2   /* rm203p1 (second stamp slot) */
#define fx_letter      3   /* rm203p0 */
#define fx_take_6      4   /* rdr_6   */
#define fx_door        5   /* rm203x0 */


/* ========================= Conversations ========================= */

#define CONV_BRIE_5  5
#define CONV_RICH_8  8
#define CONV_RD_15   15

/* Conv 5 — Brie action states */
#define CONV5_BRIE_TALK          0
#define CONV5_BRIE_TALK_HANDS_UP 1
#define CONV5_BRIE_POINT         2
#define CONV5_BRIE_NOD           3
#define CONV5_BRIE_SHUT_UP       4

/* Conv 8 — Rich action states (also shared by Conv 15) */
#define CONV8_RICH_TALK              0
#define CONV8_RICH_LEFT_UP_DONT_KNOW 1
#define CONV8_RICH_POINT             2
#define CONV8_RICH_BOTH_UP           3
#define CONV8_RICH_SHUT_UP           4
#define CONV15_RICH_STAND_TALK       5
#define CONV15_RICH_SIT_DOWN         6
#define CONV15_RICH_STAND_SHUT_UP    7

/* Conv 5 — Raoul action states */
#define CONV5_RAOUL_SHUT_UP   0
#define CONV5_RAOUL_TALK      1
#define CONV5_RAOUL_GET_UP    2
#define CONV5_RAOUL_TAKE      3
#define CONV5_RAOUL_TAKE_TRAY 4
#define CONV5_RAOUL_SHOW      5

/* Conv 15 — Daae action states */
#define CONV15_DAAE_SHUT_UP    0
#define CONV15_DAAE_TALK_RICH  1
#define CONV15_DAAE_TALK_RAOUL 2
#define CONV15_DAAE_LEAVE      3


/* ========================= Triggers ============================== */

#define ROOM_203_BRIE_START_TALKING     60
#define ROOM_203_RAOUL_START_TALKING    65
#define ROOM_203_GET_UP                 70
#define ROOM_203_TAKE_NOTE              74
#define ROOM_203_BRIE_TALK_HANDS_UP     76
#define ROOM_203_BRIE_POINT             78
#define ROOM_203_RICH_POINT             81
#define ROOM_203_RICH_LEFT_UP_DONT_KNOW 83
#define ROOM_203_RICH_BOTH_UP           85
#define ROOM_203_DOOR_OPENS             90
#define ROOM_203_DOOR_CLOSES            95
#define ROOM_203_RICH_START_TALKING     100
#define ROOM_203_DAAE_START_TALKING     105
#define ROOM_203_AFTER_SCREAM           110
#define ROOM_203_END                    115


/* ========================= Player positions ====================== */

#define PLAYER_X_FROM_202  195
#define PLAYER_Y_FROM_202  147

#define PLAYER_X_FROM_113  98
#define PLAYER_Y_FROM_113  137

#define PLAYER_X_FROM_204  292
#define PLAYER_Y_FROM_204  119

#define WALK_TO_X_FROM_204  276
#define WALK_TO_Y_FROM_204  123

#define WALK_TO_X_BEHIND_DOOR  319
#define WALK_TO_Y_BEHIND_DOOR  123


/* ========================= Misc positions ======================== */

#define FRONT_CHAIR_X  154
#define FRONT_CHAIR_Y  131

#define DOOR_X  276
#define DOOR_Y  123


extern void room_203_init();
extern void room_203_daemon();
extern void room_203_pre_parser();
extern void room_203_parser();
extern void room_203_preload();

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
