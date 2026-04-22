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

#ifndef MADS_PHANTOM_ROOM303_H
#define MADS_PHANTOM_ROOM303_H

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
	int16 dynamic_hemp;     /* Dynamic HS for hemp   */
	int16 anim_0_running;
	int16 frame_guard;

} Scratch;


/* ========================= Sprite Series =================== */

#define fx_1993                   0       /* rm303z  */
#define fx_large_note             2       /* rm303p0  */


/* ========================= Triggers ======================== */

#define ROOM_303_DONE_CLIMBING_UP 60


/* ========================= Other Macros ==================== */

#define OFF_SCREEN_X_FROM_307   -20
#define OFF_SCREEN_Y_FROM_307   135
#define PLAYER_X_FROM_307       16 
#define PLAYER_Y_FROM_307       135

#define OFF_SCREEN_X_FROM_302   340
#define OFF_SCREEN_Y_FROM_302   136
#define PLAYER_X_FROM_302       303 
#define PLAYER_Y_FROM_302       136

#define PLAYER_X_FROM_305       117 
#define PLAYER_Y_FROM_305       92 

#define DYN_HEMP_X              74
#define DYN_HEMP_Y              92
#define DYN_HEMP_XS             7 
#define DYN_HEMP_YS             12 
#define DYN_HEMP_WALK_TO_X      95
#define DYN_HEMP_WALK_TO_Y      107 

#define CLIMB_X                 110
#define CLIMB_Y                 95

#define CONV_MISC               26

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
