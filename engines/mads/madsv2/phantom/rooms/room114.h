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

#ifndef MADS_PHANTOM_ROOM114_H
#define MADS_PHANTOM_ROOM114_H

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
	int sprite[15];     /* Sprite series handles */
	int sequence[15];   /* Sequence handles      */
	int animation[4];   /* Animation handles     */

} Scratch;


/* ========================= Sprite Series ========================= */

#define fx_1993    0   /* rm114z  - 1993 overlay backdrop  */
#define fx_take_9  1   /* *RDR_9  - take-object animation  */
#define fx_rope    2   /* rm114f0 - rope prop sprite       */
#define fx_climb   3   /* rm114a0 - climb animation series */


/* ========================= Player positions ====================== */

#define PLAYER_X_FROM_105  218
#define PLAYER_Y_FROM_105  123


extern void room_114_init();
extern void room_114_parser();
extern void room_114_preload();

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
