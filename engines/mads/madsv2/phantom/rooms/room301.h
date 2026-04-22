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

#ifndef MADS_PHANTOM_ROOM301_H
#define MADS_PHANTOM_ROOM301_H

#include "mads/madsv2/phantom/phantom.h"

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
	int16 dynamic_light;    /* Dynamic HS for lights */
	int16 dynamic_sand;     /* Dynamic HS for sandbags */
	int16 anim_0_running;
	int16 guard;
	int16 prevent;

} Scratch;


/* ========================= Sprite Series =================== */

#define fx_1993_0                 0       /* rm301z0 */
#define fx_1993_1                 1       /* rm301z1 */
#define fx_1993_2                 2       /* rm301z2 */
#define fx_1993_3                 3       /* rm301z3 */
#define fx_1993_4                 4       /* rm301z4 */
#define fx_1993_5                 5       /* rm301z5 */
#define fx_1993_6                 6       /* rm301z6 */
#define fx_down_stairs            7       /* rm301a0 */
#define fx_push                   8       /* rm301a1 */
#define fx_shadow                 9       /* rm301x0 */


/* ========================= Triggers ======================== */

#define ROOM_301_WALK_TO_PUSH_LOC 60 
#define ROOM_301_SHOW_TEXT        64
#define ROOM_301_DONE_PUSHING     70 

/* ========================= Other Macros ==================== */

#define LEFT_STAGE              0
#define RIGHT_STAGE             320

#define OFF_SCREEN_X_FROM_302   -20
#define OFF_SCREEN_Y_FROM_302   132
#define PLAYER_X_FROM_302       19
#define PLAYER_Y_FROM_302       132


#define PLAYER_X_FROM_106       568
#define PLAYER_Y_FROM_106       133

#define WALK_TO_X_FROM_106      568
#define WALK_TO_Y_FROM_106      133

#define DYN_LIGHT_1_X           50
#define DYN_LIGHT_1_Y           116
#define DYN_LIGHT_1_XS          19
#define DYN_LIGHT_1_YS          10
#define DYN_LIGHT_1_WALK_TO_X   67
#define DYN_LIGHT_1_WALK_TO_Y   129

#define DYN_LIGHT_2_X           126
#define DYN_LIGHT_2_Y           118
#define DYN_LIGHT_2_XS          29
#define DYN_LIGHT_2_YS          8 
#define DYN_LIGHT_2_WALK_TO_X   152
#define DYN_LIGHT_2_WALK_TO_Y   129

#define DYN_SAND_1_X            464
#define DYN_SAND_1_Y            114
#define DYN_SAND_1_XS           11 
#define DYN_SAND_1_YS           9 
#define DYN_SAND_1_WALK_TO_X    475
#define DYN_SAND_1_WALK_TO_Y    125

#define DYN_SAND_2_X            468
#define DYN_SAND_2_Y            42 
#define DYN_SAND_2_XS           6 
#define DYN_SAND_2_YS           13
#define DYN_SAND_2_WALK_TO_X    466
#define DYN_SAND_2_WALK_TO_Y    58 


#define PUSH_X                  256
#define PUSH_Y                  130

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
