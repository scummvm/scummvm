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

#ifndef MADS_PHANTOM_ROOMS_102_H
#define MADS_PHANTOM_ROOMS_102_H

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
	int sprite[15];       /* Sprite series handles */
	int sequence[15];     /* Sequence handles      */
	int animation[4];     /* Animation handles     */
	int anim_0_running;
} Scratch;


/* ========================= Load Sprite Series =================== */

#define fx_chandelier           0      /* rm102z  */
#define fx_door                 2      /* rm102x0 */
#define fx_death                3      /* RAL86   */


/* ========================= Other Macros ========================= */

#define PLAYER_X_FROM_103       282
#define PLAYER_Y_FROM_103       145

#define PLAYER_X_FROM_101       97
#define PLAYER_Y_FROM_101       79

#define FRONT_OF_DOOR_X         282
#define FRONT_OF_DOOR_Y         145


/* ========================= Triggers ============================= */

#define ROOM_102_DOOR_CLOSES    60
#define ROOM_102_DEATH          65
#define ROOM_102_TRY_AGAIN      70

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
