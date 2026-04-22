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

#ifndef MADS_PHANTOM_ROOM406_H
#define MADS_PHANTOM_ROOM406_H

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

#define fx_bend_down_9              0       /* rrd_9    */
#define fx_red_frame                1       /* rm401f0  */
#define fx_green_frame              2       /* rm401f1  */
#define fx_blue_frame               3       /* rm401f2  */
#define fx_yellow_frame             4       /* rm401f3  */

/* ========================= Triggers ======================== */

/* ========================= Other Macros ==================== */

#define WEST_X                   20
#define WEST_Y                   122

#define WALK_TO_WEST_X           66
#define WALK_TO_WEST_Y           122

#define EAST_X                   310
#define EAST_Y                   118

#define WALK_TO_EAST_X           271
#define WALK_TO_EAST_Y           118

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
