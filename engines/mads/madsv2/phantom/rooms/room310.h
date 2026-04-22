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

#ifndef MADS_PHANTOM_ROOM310_H
#define MADS_PHANTOM_ROOM310_H

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

	word message_1;       /* Raoul's message color */
	word message_2;       /* Christine's color     */

	int multiplane_base[4];       /* Multiplanar scrolling bases */

	int lake_frame;

} Scratch;


#define fx_multiplane_0         0
#define fx_multiplane_1         0
#define fx_multiplane_2         0
#define fx_multiplane_3         0


/* ========================= Triggers ======================== */

#define ROOM_310_CONV    60 
#define ROOM_310_DONE    80 


/* ========================= Other Macros ==================== */

#define NUM_MULTIPLANES         4

#define camera_ratio_1          1
#define camera_ratio_2          1 /* 5 */

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
