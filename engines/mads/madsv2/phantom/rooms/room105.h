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

#ifndef MADS_PHANTOM_ROOM105_H
#define MADS_PHANTOM_ROOM105_H

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

	int16 sprite[15];       /* Sprite series handles */
	int16 sequence[15];     /* Sequence handles      */
	int16 animation[4];     /* Animation handles     */
	int16 column_prop;      /* Dynamic HS for column_prop */

} Scratch;


/* ========================= Sprite Series ========================= */

#define fx_lantern      0   /* rm105x0 */
#define fx_red_frame    1   /* rm105x1 */
#define fx_door         2   /* rm105x2 */
#define fx_1993         3   /* rm105z  */
#define fx_thunder      4   /* rm105x3 */
#define fx_bend_down_9  5   /* rrd_9   */
#define fx_take_9       6   /* rdr_9   */
#define fx_climb_up     7   /* rm105a0 */
#define fx_climb_down   8   /* rm105a1 */


/* ========================= Triggers ============================== */

#define ROOM_105_DOOR_CLOSES    60
#define ROOM_105_DOOR_OPENS     65
#define ROOM_105_DUN_THUNDER    70
#define ROOM_105_INIT_TEXT      75


/* ========================= Player positions ====================== */

#define PLAYER_X_FROM_103           3
#define PLAYER_Y_FROM_103         112

#define PLAYER_X_FROM_STAIRCASE   198
#define PLAYER_Y_FROM_STAIRCASE   132

#define WALK_TO_X_FROM_103         45
#define WALK_TO_Y_FROM_103        131

#define WALK_TO_X_OPEN_DOOR         0
#define WALK_TO_Y_OPEN_DOOR       111

#define FRONT_OF_DOOR_X            33
#define FRONT_OF_DOOR_Y           128

#define WALK_TO_STAIRS_X          207
#define WALK_TO_STAIRS_Y          131


/* ========================= Dynamic hotspot coords — props ======== */

#define PROP_1_X        0
#define PROP_1_Y      125
#define PROP_2_X       29
#define PROP_2_Y      136
#define PROP_3_X       79
#define PROP_3_Y      141

#define PROP_1_X_SIZE  29
#define PROP_1_Y_SIZE  31
#define PROP_2_X_SIZE  50
#define PROP_2_Y_SIZE  20
#define PROP_3_X_SIZE  53
#define PROP_3_Y_SIZE  15


/* ========================= Dynamic hotspot coords — bear ========= */

#define BEAR_1_X      278
#define BEAR_1_Y      132
#define BEAR_2_X      299
#define BEAR_2_Y      146
#define BEAR_3_X      269
#define BEAR_3_Y      142

#define BEAR_1_X_SIZE  21
#define BEAR_1_Y_SIZE  24
#define BEAR_2_X_SIZE   8
#define BEAR_2_Y_SIZE  10
#define BEAR_3_X_SIZE   8
#define BEAR_3_Y_SIZE   8


/* ========================= Dynamic hotspot coords — light ======== */

#define LIGHT_X       102
#define LIGHT_Y        14
#define LIGHT_X_SIZE   24
#define LIGHT_Y_SIZE   10


/* ========================= Dynamic hotspot coords — columns ====== */

#define COLUMN_1_X       132
#define COLUMN_1_Y        24
#define COLUMN_2_X        56
#define COLUMN_2_Y        45

#define COLUMN_1_X_SIZE   21
#define COLUMN_1_Y_SIZE  105
#define COLUMN_2_X_SIZE   11
#define COLUMN_2_Y_SIZE   77

#define WALK_TO_COLUMN_1_X  159
#define WALK_TO_COLUMN_1_Y  133
#define WALK_TO_COLUMN_2_X   72
#define WALK_TO_COLUMN_2_Y  126


extern void room_105_init(void);
extern void room_105_daemon(void);
extern void room_105_pre_parser(void);
extern void room_105_parser(void);
extern void room_105_preload(void);

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
