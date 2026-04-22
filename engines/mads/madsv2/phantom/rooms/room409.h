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

#ifndef MADS_PHANTOM_ROOM409_H
#define MADS_PHANTOM_ROOM409_H

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

#define fx_bend_down_9          0       /* rrd_9    */
#define fx_sword                1       /* rm409p0  */
#define fx_red_frame            2       /* rm409f0  */
#define fx_green_frame          3       /* rm409f1  */
#define fx_blue_frame           4       /* rm409f2  */
#define fx_yellow_frame         5       /* rm409f3  */
#define fx_door                 6       /* rm409x0  */
#define fx_gas                  7       /* rm409a0  */
#define fx_take_9               8       /* rdr_9    */




/* ======================== Triggers ========================= */

#define ROOM_409_DIE            60
#define ROOM_409_DOOR_OPENS     65


/* ========================= Other Macros ==================== */

#define NORTH_X                  195
#define NORTH_Y                  92

#define WALK_TO_NORTH_X          195
#define WALK_TO_NORTH_Y          107 

#define SOUTH_X                  184
#define SOUTH_Y                  145

#define PANEL_X                  229
#define PANEL_Y                  106

#define DOOR_X                   191
#define DOOR_Y                   104

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
