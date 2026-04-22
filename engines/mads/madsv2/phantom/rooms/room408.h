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

#ifndef MADS_PHANTOM_ROOM408_H
#define MADS_PHANTOM_ROOM408_H

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

/* ========================= Sprite Series =================== */

#define fx_north                 0       /* rm408c0  */
#define fx_bend_down_9           1       /* rrd_9    */
#define fx_red_frame             2       /* rm408f0  */
#define fx_green_frame           3       /* rm408f1  */
#define fx_blue_frame            4       /* rm408f2  */
#define fx_yellow_frame          5       /* rm408f3  */
#define fx_web                   6       /* rm408c1  */
#define fx_brick                 7       /* rm408c2  */
#define fx_un_web                8       /* rm408x   */


/* ======================== Triggers ========================= */

#define ROOM_408_SHOW_TEXT       60
#define ROOM_408_DONE            70


/* ========================= Other Macros ==================== */

#define NORTH_X                  174
#define NORTH_Y                  100

#define WALK_TO_NORTH_X          174
#define WALK_TO_NORTH_Y          106 

#define SOUTH_X                  175
#define SOUTH_Y                  145

#define FLOOR_1_X                147
#define FLOOR_1_Y                121

#define FLOOR_2_X                153
#define FLOOR_2_Y                121

#define FLOOR_3_X                154
#define FLOOR_3_Y                115

#define FLOOR_4_X                161
#define FLOOR_4_Y                101

#define FLOOR_5_X                162
#define FLOOR_5_Y                106

#define FLOOR_6_X                187
#define FLOOR_6_Y                107

#define FLOOR_7_X                185
#define FLOOR_7_Y                101

#define FLOOR_8_X                192
#define FLOOR_8_Y                119

#define WALL_1_X                 147
#define WALL_1_Y                 76

#define WALL_2_X                 159
#define WALL_2_Y                 108

#define WALL_3_X                 185
#define WALL_3_Y                 93

#define WALL_4_X                 199
#define WALL_4_Y                 91

#define DYN_WEB_RIGHT_X          199
#define DYN_WEB_RIGHT_Y          112 
#define DYN_WEB_RIGHT_XS         6  
#define DYN_WEB_RIGHT_YS         12 
#define WEB_WALK_TO_RIGHT_X      194
#define WEB_WALK_TO_RIGHT_Y      125

#define DYN_WEB_RIGHT_X_2        202
#define DYN_WEB_RIGHT_Y_2        81 
#define DYN_WEB_RIGHT_XS_2       5
#define DYN_WEB_RIGHT_YS_2       31
#define WEB_WALK_TO_RIGHT_X_2    194
#define WEB_WALK_TO_RIGHT_Y_2    125

#define DYN_WEB_LEFT_X           138
#define DYN_WEB_LEFT_Y           74
#define DYN_WEB_LEFT_XS          7  
#define DYN_WEB_LEFT_YS          33
#define WEB_WALK_TO_LEFT_X       154
#define WEB_WALK_TO_LEFT_Y       124

#define DYN_WEB_LEFT_X_2         143
#define DYN_WEB_LEFT_Y_2         107
#define DYN_WEB_LEFT_XS_2        6 
#define DYN_WEB_LEFT_YS_2        15
#define WEB_WALK_TO_LEFT_X_2     154
#define WEB_WALK_TO_LEFT_Y_2     124

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
