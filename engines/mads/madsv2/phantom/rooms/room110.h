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

#ifndef MADS_PHANTOM_ROOM110_H
#define MADS_PHANTOM_ROOM110_H

#include "mads/madsv2/phantom/rooms/section1.h"
#include "mads/madsv2/phantom/global.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

#define local ((Scratch *)(&game.scratch[0]))
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

typedef struct {        /* Room local variables */

	int16 sprite[15];         /* Sprite series handles */
	int16 sequence[15];       /* Sequence handles      */
	int16 animation[4];       /* Animation handles     */

} Scratch;


/* ========================= Sprite Series ========================= */

#define fx_1993         0   /* rm110z  */
#define fx_take_9       1   /* rdr_9   */
#define fx_door         2   /* rm110x0 */
#define fx_door_closed  3   /* rm110x1 */


/* ========================= Player positions ====================== */

#define PLAYER_X_FROM_109      310
#define PLAYER_Y_FROM_109      150

#define PLAYER_X_FROM_112      261
#define PLAYER_Y_FROM_112      121

#define WALK_TO_X_FROM_112     221
#define WALK_TO_Y_FROM_112     131

#define OFF_SCREEN_X_FROM_109  335
#define OFF_SCREEN_Y_FROM_109  150


/* ========================= Door approach positions =============== */

#define FRONT_LEFT_DOOR_X    111
#define FRONT_LEFT_DOOR_Y    126

#define FRONT_RIGHT_DOOR_X   221
#define FRONT_RIGHT_DOOR_Y   131

#define INSIDE_RIGHT_DOOR_X  261
#define INSIDE_RIGHT_DOOR_Y  120


extern void room_110_init();
extern void room_110_pre_parser();
extern void room_110_parser();
extern void room_110_preload();

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
