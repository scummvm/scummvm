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

#ifndef MADS_PHANTOM_ROOM501_H
#define MADS_PHANTOM_ROOM501_H

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
	int16 prevent_2;

} Scratch;


/* ========================= Sprite Series =================== */

#define fx_flame                0       /* rm501x0  */
#define fx_door                 1       /* rm501x1  */
#define fx_left_door            2       /* rm501x2  */
#define fx_take_6               3       /* rdrr_6   */
#define fx_boat                 4       /* rm501x3  */
#define fx_shove_off            5       /* rm501a1  */
#define fx_christine            6       /* *chr_6   */
										/* *chr_9   */
										/* *chr_8   */



/* ========================= Triggers ======================== */

#define ROOM_501_LEFT_DOOR_CLOSES 55 
#define ROOM_501_DOOR_CLOSES      60 
#define ROOM_501_DOOR_OPENS       65 
#define ROOM_501_FROM_506         80
#define ROOM_501_SHOVE_OFF        90
#define ROOM_501_START_ROWING     100

/* ========================= Other Macros ==================== */

#define PLAYER_X_FROM_506       305
#define PLAYER_Y_FROM_506       112

#define WALK_TO_X_FROM_506      260
#define WALK_TO_Y_FROM_506      112

#define PLAYER_X_FROM_502       319
#define PLAYER_Y_FROM_502       116

#define WALK_TO_X_FROM_502      286
#define WALK_TO_Y_FROM_502      119

#define WEST_X                  -20
#define WEST_Y                  109

#define WALK_TO_WEST_X          24
#define WALK_TO_WEST_Y          109

#define DOOR_LEFT_X             266
#define DOOR_LEFT_Y             112

#define DOOR_RIGHT_X            287
#define DOOR_RIGHT_Y            118

#define CHRIS_X_LEFT            113
#define CHRIS_Y_LEFT            93

#define CHRIS_X_RIGHT           125
#define CHRIS_Y_RIGHT           94

#define CONV_MISC               26

#define WALK_TO_CAT_NO_X        24
#define WALK_TO_CAT_NO_Y        110

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
