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

#ifndef MADS_PHANTOM_ROOM112_H
#define MADS_PHANTOM_ROOM112_H

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

	int16 sprite[15];         /* Sprite series handles                   */
	int16 sequence[15];       /* Sequence handles                        */
	int16 animation[4];       /* Animation handles                       */
	int16 dynamic_julie;      /* For dynamic HS of Julie                 */
	int16 anim_0_running;     /* T if Julie's practicing anim is running */
	int16 anim_1_running;     /* T if Raoul's chair movement anim is running */
	int16 just_did_option;
	int16 julie_frame;
	int16 julie_action;
	int16 julie_talk_count;
	int16 raoul_frame;
	int16 raoul_action;
	int16 raoul_talk_count;
	int16 display_wait;       /* for getting up out of the chair */

} Scratch;


/* ========================= Conversation ========================== */

#define CONV_JULIE_PRACTICE    3

#define CONV3_JULIE_TALK       0
#define CONV3_JULIE_I_DUNNO    1
#define CONV3_JULIE_PRACTICE   2

#define CONV3_RAOUL_TALK       0
#define CONV3_RAOUL_SHUT_UP    1
#define CONV3_RAOUL_INVISIBLE  2
#define CONV3_RAOUL_GET_UP     3


/* ========================= Triggers ============================== */

#define ROOM_112_JULIE_TALK      60
#define ROOM_112_RAOUL_TALK      62
#define ROOM_112_BEFORE_CHAIR    68
#define ROOM_112_BETWEEN_CHAIR   70
#define ROOM_112_AT_CHAIR        72
#define ROOM_112_GET_UP          74


/* ========================= Player positions ====================== */

#define PLAYER_X_FROM_110        128
#define PLAYER_Y_FROM_110        145

#define CHAIR_X                  53
#define CHAIR_Y                  128


/* ========================= Julie dynamic hotspot ================= */

#define DYNAMIC_JULIE_X          255
#define DYNAMIC_JULIE_Y          82
#define DYNAMIC_JULIE_X_SIZE     30
#define DYNAMIC_JULIE_Y_SIZE     44
#define DYNAMIC_JULIE_WALK_TO_X  216
#define DYNAMIC_JULIE_WALK_TO_Y  137


extern void room_112_init();
extern void room_112_daemon();
extern void room_112_pre_parser();
extern void room_112_parser();
extern void room_112_preload();

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
