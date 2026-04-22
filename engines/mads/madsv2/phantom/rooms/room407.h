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

#ifndef MADS_PHANTOM_ROOM407_H
#define MADS_PHANTOM_ROOM407_H

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
	int16 frame_is_here;    /* T if any frame is here */
	int16 dyn_red;
	int16 dyn_green;
	int16 dyn_blue;
	int16 dyn_yellow;
	int16 frame_here_for_taking; /* T if a specific frame is here */

} Scratch;


/* ===================== Sprite seriers ====================== */

#define fx_bend_down_9              0       /* rrd_9    */
#define fx_red_frame                1       /* rm401f0  */
#define fx_green_frame              2       /* rm401f1  */
#define fx_blue_frame               3       /* rm401f2  */
#define fx_yellow_frame             4       /* rm401f3  */

/* ========================= Triggers ======================== */



/* ========================= Other Macros ==================== */

#define NORTH_X                  197
#define NORTH_Y                  14

#define WALK_TO_NORTH_X          181
#define WALK_TO_NORTH_Y          14  

#define WEST_X                   -20
#define WEST_Y                   143

#define WALK_TO_WEST_X           20
#define WALK_TO_WEST_Y           143

#define EAST_X                   330
#define EAST_Y                   146

#define WALK_TO_EAST_X           298
#define WALK_TO_EAST_Y           146

#define SOUTH_X                  147
#define SOUTH_Y                  14 

#define WALK_TO_SOUTH_X          165
#define WALK_TO_SOUTH_Y          14 



#define CAT_RIGHT_1_X            310
#define CAT_RIGHT_1_Y            107

#define CAT_RIGHT_2_X            308 
#define CAT_RIGHT_2_Y            175

#define CAT_RIGHT_3_X            308
#define CAT_RIGHT_3_Y            146

#define CAT_RIGHT_4_X            309
#define CAT_RIGHT_4_Y            152

#define CAT_LEFT_1_X             9  
#define CAT_LEFT_1_Y             46 

#define CAT_LEFT_2_X             8   
#define CAT_LEFT_2_Y             138

#define CAT_LEFT_3_X             12 
#define CAT_LEFT_3_Y             149

#define CAT_LEFT_4_X             0   
#define CAT_LEFT_4_Y             151

#define LAKE_X                   172
#define LAKE_Y                   18

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
