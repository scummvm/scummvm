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

#ifndef MADS_PHANTOM_ROOM111_H
#define MADS_PHANTOM_ROOM111_H

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
	int16 delete_axe;         /* true if axe was deleted */
	int16 anim_0_running;
	int16 anim_1_running;
	int16 listen_frame;
	int16 listen_action;
	int16 it_is_closed;

} Scratch;


/* ========================= Sprite Series ========================= */

#define fx_take_9     0   /* rdr_9   */
#define fx_1993       1   /* rm111z  */
#define fx_axe        2   /* rm111a1 */
#define fx_broken_axe 3   /* rm111a2 */
#define fx_open_door  4   /* rm111a3 */
#define fx_door       5   /* rm111x0 */


/* ========================= Conversation ========================== */

#define CONV_LISTEN         14
#define CONV14_LISTEN        0
#define CONV14_UN_LISTEN     1


/* ========================= Triggers ============================== */

#define ROOM_111_MADE_IT_OUT_DOOR  60
#define ROOM_111_CLOSED_DOOR       62
#define ROOM_111_AT_LISTEN_POINT   64
#define ROOM_111_LISTEN            66
#define ROOM_111_UN_LISTEN         68
#define ROOM_111_DONE_LISTENING    70
#define ROOM_111_NOTHING           72


/* ========================= Player positions ====================== */

#define PLAYER_X_FROM_113          112
#define PLAYER_Y_FROM_113          126

#define PLAYER_X_FROM_113_SAWDUST  146
#define PLAYER_Y_FROM_113_SAWDUST  108

#define PLAYER_X_FROM_109          311
#define PLAYER_Y_FROM_109          150

#define OFF_SCREEN_X_FROM_109      335
#define OFF_SCREEN_Y_FROM_109      150

#define LISTEN_X                   119
#define LISTEN_Y                   124


/* ========================= Door approach positions =============== */

#define FRONT_RIGHT_DOOR_X   219
#define FRONT_RIGHT_DOOR_Y   131

#define FRONT_LEFT_DOOR_X    109
#define FRONT_LEFT_DOOR_Y    124

#define WALK_TO_X_INSIDE_112 145
#define WALK_TO_Y_INSIDE_112 108

#define AFTER_AXE_ANIM_X     126
#define AFTER_AXE_ANIM_Y     126


/* ========================= Wall dynamic hotspot ================== */

#define WALL_X        35
#define WALL_Y        82
#define WALL_X_SIZE   13
#define WALL_Y_SIZE   11

#define WALL_WALK_X   78
#define WALL_WALK_Y   122


extern void room_111_init();
extern void room_111_daemon();
extern void room_111_pre_parser();
extern void room_111_parser();
extern void room_111_preload();

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
