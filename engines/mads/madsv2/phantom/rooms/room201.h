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

#ifndef MADS_PHANTOM_ROOM201_H
#define MADS_PHANTOM_ROOM201_H

#include "mads/madsv2/phantom/rooms/section2.h"
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

	int16 raoul_frame;
	int16 raoul_action;
	int16 anim_0_running;

	int16 seller_frame;
	int16 seller_action;
	int16 seller_talk_count;
	int16 anim_1_running;
	int16 want_to_hold;

} Scratch;


/* ========================= Sprite Series ========================= */

#define fx_curtain   0   /* rm201x0 */
#define fx_envelope  1   /* rm201f0 */


/* ========================= Conversation ========================== */

#define CONV_TICKET_16  16

/* Conv 16 — Raoul action states */
#define CONV16_RAOUL_TALK      0
#define CONV16_RAOUL_SHUT_UP   1
#define CONV16_RAOUL_TAKE_IT   2
#define CONV16_RAOUL_QUIT      3

/* Conv 16 — Seller action states */
#define CONV16_SELLER_SHUT_UP    0
#define CONV16_SELLER_TALK       1
#define CONV16_SELLER_LOOK_DOWN  2
#define CONV16_SELLER_GIVE       3


/* ========================= Triggers ============================== */

#define ROOM_201_DOOR_CLOSES      60
#define ROOM_201_SELLER_TALK      65
#define ROOM_201_ME_TALK          70
#define ROOM_201_DONE_RAOUL_ANIM  80
#define ROOM_201_END              90


/* ========================= Player positions ====================== */

#define PLAYER_X_FROM_202   314
#define PLAYER_Y_FROM_202    86

#define WALK_TO_X_FROM_202  266
#define WALK_TO_Y_FROM_202   98


/* ========================= Misc positions ======================== */

#define WINDOW_X  72
#define WINDOW_Y  101

#define SIGN_X    147
#define SIGN_Y    104


extern void room_201_init();
extern void room_201_daemon();
extern void room_201_pre_parser();
extern void room_201_parser();
extern void room_201_preload();

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
