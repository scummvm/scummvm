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

#ifndef MADS_PHANTOM_ROOM208_H
#define MADS_PHANTOM_ROOM208_H

#include "mads/madsv2/phantom/phantom.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

/* ---- Scratch (room-local variables) ---- */

struct Room208Scratch {
	int16 sprite[15];           // Sprite series handles
	int16 sequence[15];         // Sequence handles
	int16 animation[4];         // Animation handles
	int16 middle_direction;     // 1 = increasing (toward whisper), 0 = decreasing (returning)
	int16 top_left_frame;       // Current frame for top left person
	int16 top_right_frame;      // Current frame for top right person
	int16 middle_left_frame;    // Current frame for middle left person
	int16 middle_middle_frame;  // Current frame for middle middle person
	int16 middle_right_frame;   // Current frame for middle right person
	int16 bottom_left_frame;    // Current frame for bottom left person
	int16 bottom_middle_frame;  // Current frame for bottom middle person
	int16 bottom_right_frame;   // Current frame for bottom right person
	int16 prevent;
	int16 prevent_2;
};

#define local ((Room208Scratch *)(&game.scratch[0]))
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

/* ---- Sprite series indices ---- */

#define fx_top_left         0   /* rm208x0 */
#define fx_top_right        1   /* rm208x1 */
#define fx_middle_left      2   /* rm208x2 */
#define fx_middle_middle    3   /* rm208x3 */
#define fx_middle_right     4   /* rm208x4 */
#define fx_bottom_left      5   /* rm208x5 */
#define fx_bottom_middle    6   /* rm208x6 */
#define fx_bottom_right     7   /* rm208x7 */

/* ---- Trigger constants ---- */

#define ROOM_208_MOVE_TOP_LEFT      60
#define ROOM_208_MOVE_TOP_RIGHT     62
#define ROOM_208_MOVE_MIDDLE_LEFT   64
#define ROOM_208_MOVE_MIDDLE_MIDDLE 66
#define ROOM_208_MOVE_MIDDLE_RIGHT  68
#define ROOM_208_MOVE_BOTTOM_LEFT   70
#define ROOM_208_MOVE_BOTTOM_MIDDLE 72
#define ROOM_208_MOVE_BOTTOM_RIGHT  74

#define ROOM_208_DONE_PHANTOM       80

/* ---- Delay constant ---- */

#define QUICKLY                     10  /* very short delay in game ticks */

/* ---- Entry points ---- */

void room_208_preload();
void room_208_init();
void room_208_daemon();
/* no pre_parser or parser — both NULL in preload */

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
