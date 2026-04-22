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

#ifndef MADS_PHANTOM_ROOM307_H
#define MADS_PHANTOM_ROOM307_H

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

	int sprite[15];       /* Sprite series handles */
	int sequence[15];     /* Sequence handles      */
	int animation[4];     /* Animation handles     */

} Scratch;


/* ========================= Sprite Series =================== */

#define fx_test                 0       /* rm301x0  */
#define fx_bend_down_9          1       /* rrd_9    */
#define fx_green_frame          2       /* rm307p0  */
#define fx_door                 3       /* rm307x0  */
#define fx_take_9               4       /* rdr_9    */



/* ========================= Triggers ======================== */

#define ROOM_307_DOOR_CLOSES    60 
#define ROOM_307_DOOR_OPENS     70 


/* ========================= Other Macros ==================== */

#define OFF_SCREEN_X_FROM_303   340
#define OFF_SCREEN_Y_FROM_303   137
#define PLAYER_X_FROM_303       304 
#define PLAYER_Y_FROM_303       137

#define START_X_FROM_308        18
#define START_Y_FROM_308        134
#define END_X_FROM_308          41
#define END_Y_FROM_308          137

#define OPEN_DOOR_X             28
#define OPEN_DOOR_Y             137

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
