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

#ifndef MADS_PHANTOM_ROOM506_H
#define MADS_PHANTOM_ROOM506_H

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
	int16 anim_0_running;
	int16 prevent;
	int16 ascending;

} Scratch;


/* ========================= Sprite Series =================== */

#define fx_left_flame           0       /* rm506x0  */
#define fx_middle_flame         1       /* rm506x1  */
#define fx_right_flame          2       /* rm506x2  */
#define fx_door                 3       /* rm506x3  */
#define fx_take_6               4       /* rdrr_6   */
#define fx_oar                  5       /* rm506p0  */
#define fx_christine            6       /* *chr_6   */
										/* *chr_3   */
										/* *chr_2   */
#define fx_raoul_ascend         7       /* rm506a1  */
#define fx_raoul_decend         8       /* rm506a0  */


/* ========================= Triggers ======================== */

#define ROOM_506_DOOR_CLOSES     60 
#define ROOM_506_DOOR_OPENS      65 
#define ROOM_506_DONE_GOING_UP   90
#define ROOM_506_DONE_GOING_DOWN 95

/* ========================= Other Macros ==================== */

#define PLAYER_X_FROM_501       0
#define PLAYER_Y_FROM_501       142

#define WALK_TO_X_FROM_501      23
#define WALK_TO_Y_FROM_501      145

#define PLAYER_X_FROM_504       186
#define PLAYER_Y_FROM_504       122

#define WALK_TO_X_FROM_504      301
#define WALK_TO_Y_FROM_504      98

#define DYNAMIC_CHR_WALK_TO_X   79
#define DYNAMIC_CHR_WALK_TO_Y   133

#define END_OF_DECEND_X         189
#define END_OF_DECEND_Y         123

#define START_OF_ASCEND_X       191
#define START_OF_ASCEND_Y       118

#define FRONT_OF_DOOR_X         33
#define FRONT_OF_DOOR_Y         142

#define CONV_MISC               26

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
