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

#ifndef MADS_PHANTOM_ROOM505_H
#define MADS_PHANTOM_ROOM505_H

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

	int16 raoul_action;       /* action Raoul is going to perform */
	int16 raoul_frame;
	int16 raoul_talk_count;

	int16 both_action;        /* action They are going to perform */
	int16 both_frame;
	int16 both_talk_count;

	int16 part_action;        /* action They are going to perform (when parted) */
	int16 part_frame;
	int16 part_talk_count;

	int16 anim_0_running;
	int16 anim_1_running;
	int16 anim_2_running;

	int16 made_it_past_106;   /* If T, will not put on conv_hold (during embrace) */
	int16 leave_room;         /* if T, She will follow him out door */
	int16 they_parted;

} Scratch;


/* ========================= Sprite Series =================== */

#define fx_pusher               0       /* rm505a1  */
#define fx_skull_0              1       /* rm505x0  */
#define fx_skull_1              2       /* rm505x1  */
#define fx_skull_2              3       /* rm505x2  */
#define fx_skull_3              4       /* rm505x3  */
#define fx_skull_4              5       /* rm505x4  */
#define fx_skull_5              6       /* rm505x5  */
#define fx_lid                  7       /* rm505x6  */
#define fx_take_9               8       /* rm505a4  */


/* ========================= Triggers ======================== */

#define ROOM_505_OPEN_IT        60 
#define ROOM_505_AFTER_RESCUE   65 
#define ROOM_505_START_CONV     70 
#define ROOM_505_END_CONV       75 
#define ROOM_505_HOLD_HANDS     80
#define ROOM_505_ME_TALK        85
#define ROOM_505_YOU_TALK       90
#define ROOM_505_UNLOCK         95


/* ========================= Other Macros ==================== */

#define CONV_COFFIN             20

#define CONV20_RAOUL_TALK       0
#define CONV20_RAOUL_SHUT_UP    1
#define CONV20_RAOUL_END        2

#define PLAYER_X_FROM_504       5
#define PLAYER_Y_FROM_504       87

#define WALK_TO_X_FROM_504      58
#define WALK_TO_Y_FROM_504      104

#define HIT_GLASS               0
#define GET_OUT_A               1
#define GET_OUT_B               2
#define BOTH_SHUT_UP            3
#define SHE_TALK                4
#define HE_TALK                 5
#define PART                    6
#define GIVE_RING               7
#define SHE_WAIT                8
#define C_YA                    9
#define SHE_WAIT_TALK           10

#define COFFIN_X                244
#define COFFIN_Y                130

#define HOLD_HANDS_X            136
#define HOLD_HANDS_Y            126

#define RAOUL_END_X             93
#define RAOUL_END_Y             133

#define DYNAMIC_CHR_WALK_TO_X   91
#define DYNAMIC_CHR_WALK_TO_Y   108

#define COVER_AIR_X             216
#define COVER_AIR_Y             44

#define LOCK_X                  279
#define LOCK_Y                  150

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
