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

#ifndef MADS_PHANTOM_ROOM106_H
#define MADS_PHANTOM_ROOM106_H

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

	int sprite[15];       /* Sprite series handles */
	int sequence[15];     /* Sequence handles      */
	int animation[4];     /* Animation handles     */
	int dynamic_sandbag;  /* Dynamic HS for fallen sandbag */

} Scratch;


/* ========================= Sprite Series ========================= */

#define fx_sandbag       0   /* rm106x0 */
#define fx_yikes         1   /* rm106a0 */
#define fx_door          2   /* rm106x1 */
#define fx_take_9        3   /* rdr_9   */
#define fx_climb_up      4   /* rm106a1 */
#define fx_bend_down_9   5   /* rrd_9   */
#define fx_climb_down    6   /* rm106a2 */
#define fx_1881          7   /* rm106z  */
#define fx_cable_hook    8   /* rm106p0 */


/* ========================= Triggers ============================== */

#define ROOM_106_DOOR_CLOSES    60
#define ROOM_106_DOOR_OPENS     65
#define ROOM_106_CLIMB_DOWN     75
#define ROOM_106_CLIMB_UP       80
#define ROOM_106_INIT_TEXT      85


/* ========================= Player positions ====================== */

#define PLAYER_X_FROM_105        227
#define PLAYER_Y_FROM_105        143

#define PLAYER_X_FROM_108         19
#define PLAYER_Y_FROM_108        147

#define PLAYER_X_FROM_107        143
#define PLAYER_Y_FROM_107         68

#define PLAYER_X_FROM_109        178
#define PLAYER_Y_FROM_109         68

#define WALK_TO_X_FROM_107       163
#define WALK_TO_Y_FROM_107        68

#define AFTER_LOOK_ANIM_X        225
#define AFTER_LOOK_ANIM_Y        143

#define WALK_TO_AFTER_ANIM_X     236
#define WALK_TO_AFTER_ANIM_Y     142

#define FRONT_OF_DOOR_X          179
#define FRONT_OF_DOOR_Y           63

#define WALK_TO_X_BEHIND_DOOR    180
#define WALK_TO_Y_BEHIND_DOOR     60

#define OFF_SCREEN_X_FROM_108    -20
#define OFF_SCREEN_Y_FROM_108    130


/* ========================= Fallen sandbag hotspot ================ */

#define SANDBAG_X         227
#define SANDBAG_Y         140
#define SANDBAG_XS         18
#define SANDBAG_YS         11

#define SANDBAG_WALK_TO_X  224
#define SANDBAG_WALK_TO_Y  152


/* ========================= Sandbag dynamics (1881) =============== */

#define DYN_SANDBAG_1_X       40
#define DYN_SANDBAG_1_Y       47
#define DYN_SANDBAG_1_X_SIZE  11
#define DYN_SANDBAG_1_Y_SIZE  17

#define DYN_SANDBAG_2_X       98
#define DYN_SANDBAG_2_Y       14
#define DYN_SANDBAG_2_X_SIZE   5
#define DYN_SANDBAG_2_Y_SIZE  10

#define DYN_SANDBAG_3_X      111
#define DYN_SANDBAG_3_Y       23
#define DYN_SANDBAG_3_X_SIZE   6
#define DYN_SANDBAG_3_Y_SIZE   9

#define DYN_SANDBAG_4_X      119
#define DYN_SANDBAG_4_Y       12
#define DYN_SANDBAG_4_X_SIZE   5
#define DYN_SANDBAG_4_Y_SIZE   8


/* ========================= Floor dynamics (1881) ================= */

#define DYN_FLOOR_1_X          80
#define DYN_FLOOR_1_Y         114
#define DYN_FLOOR_1_X_SIZE     24
#define DYN_FLOOR_1_Y_SIZE      4
#define DYN_FLOOR_1_WALK_TO_X  93
#define DYN_FLOOR_1_WALK_TO_Y 121

#define DYN_FLOOR_2_X         106
#define DYN_FLOOR_2_Y         102
#define DYN_FLOOR_2_X_SIZE      5
#define DYN_FLOOR_2_Y_SIZE     10
#define DYN_FLOOR_2_WALK_TO_X 108
#define DYN_FLOOR_2_WALK_TO_Y 109


extern void room_106_init(void);
extern void room_106_daemon(void);
extern void room_106_pre_parser(void);
extern void room_106_parser(void);
extern void room_106_preload(void);

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
