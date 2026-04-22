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

#ifndef MADS_PHANTOM_ROOM109_H
#define MADS_PHANTOM_ROOM109_H

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

	int sprite[15];         /* Sprite series handles */
	int sequence[15];       /* Sequence handles      */
	int animation[4];       /* Animation handles     */
	int on_floor;           /* Which floor are we on? 3=top, 1=bottom */
	int anim_0_running;
	int anim_1_running;
	int anim_2_running;
	int anim_3_running;

} Scratch;


/* ========================= Sprite Series ========================= */

#define fx_top_door       0   /* rm109x0 */
#define fx_middle_door    1   /* rm109x2 */
#define fx_top_flame      2   /* rm109x1 */
#define fx_middle_flame   3   /* rm109x3 */
#define fx_bottom_flame   4   /* rm109x4 */
#define fx_top_lester     5   /* rm109z0 */
#define fx_middle_lester  6   /* rm109z1 */
#define fx_bottom_lester  7   /* rm109z2 */
#define fx_take_6         8   /* rdr_6    */


/* ========================= Triggers ============================== */

#define ROOM_109_TOP_DOOR_CLOSES       60
#define ROOM_109_MIDDLE_DOOR_CLOSES    65
#define ROOM_109_CHANGE_LANTERN_FLAME  70
#define ROOM_109_DOOR_OPENS            75


/* ========================= Player positions ====================== */

#define PLAYER_X_FROM_106    31
#define PLAYER_Y_FROM_106    459

#define PLAYER_X_FROM_111    4
#define PLAYER_Y_FROM_111    136

#define PLAYER_X_FROM_110    3
#define PLAYER_Y_FROM_110    292

#define WALK_TO_X_FROM_111   32
#define WALK_TO_Y_FROM_111   138

#define WALK_TO_X_FROM_110   31
#define WALK_TO_Y_FROM_110   295


/* ========================= Camera floors ========================= */

#define TOP_FLOOR      0     /* for camera_jump_to */
#define MIDDLE_FLOOR   156   /* for camera_jump_to */
#define BOTTOM_FLOOR   312   /* for camera_jump_to */


/* ========================= Door approach positions =============== */

#define WALK_TO_X_OPEN_TOP_DOOR        2
#define WALK_TO_Y_OPEN_TOP_DOOR        131

#define WALK_TO_X_OPEN_MIDDLE_DOOR     2
#define WALK_TO_Y_OPEN_MIDDLE_DOOR     281


/* ========================= Climb / descend positions ============= */

#define END_CLIMB_BOTTOM_FLOOR_X       264
#define END_CLIMB_BOTTOM_FLOOR_Y       295

#define START_CLIMB_BOTTOM_X           58
#define START_CLIMB_BOTTOM_Y           452

#define END_CLIMB_MIDDLE_FLOOR_X       261
#define END_CLIMB_MIDDLE_FLOOR_Y       137

#define START_CLIMB_MIDDLE_X           58
#define START_CLIMB_MIDDLE_Y           295

#define START_DOWN_MIDDLE_X            269
#define START_DOWN_MIDDLE_Y            292

#define END_DOWN_MIDDLE_FLOOR_X        61
#define END_DOWN_MIDDLE_FLOOR_Y        450

#define START_DOWN_TOP_X               269
#define START_DOWN_TOP_Y               138

#define END_DOWN_TOP_FLOOR_X           59
#define END_DOWN_TOP_FLOOR_Y           296


extern void room_109_init();
extern void room_109_daemon();
extern void room_109_pre_parser();
extern void room_109_parser();
extern void room_109_preload();

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
