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

#ifndef MADS_PHANTOM_ROOM309_H
#define MADS_PHANTOM_ROOM309_H

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

	int16 boat_action;      /* pulling up in boat */
	int16 boat_frame;
	int16 anim_0_running;

	int chris_talk_count;

} Scratch;


/* ========================= Sprite Series =================== */

#define fx_door                 0       /* rm309x0  */
#define fx_take_9               1       /* rdr_9    */
#define fx_boat                 2       /* rm309x1  */

/* ========================= Triggers ======================== */

#define ROOM_309_DOOR_OPENS     60
#define ROOM_309_DOOR_CLOSES    65
#define ROOM_309_OUT_OF_BOAT    70

/* ========================= Other Macros ==================== */

#define PLAYER_X_FROM_404       319
#define PLAYER_Y_FROM_404       136

#define WALK_TO_X_FROM_404      281
#define WALK_TO_Y_FROM_404      148

#define PLAYER_X_FROM_308       0
#define PLAYER_Y_FROM_308       121

#define WALK_TO_X_FROM_308      28
#define WALK_TO_Y_FROM_308      142

#define PLAYER_X_FROM_310       209
#define PLAYER_Y_FROM_310       144

#define DYNAMIC_CHR_WALK_TO_X   62
#define DYNAMIC_CHR_WALK_TO_Y   146

#define CONV_MISC               26

#define ACTION_TALK             0
#define ACTION_OTHER            1

#define WALK_TO_CAT_NO_X        285
#define WALK_TO_CAT_NO_Y        147

#define DOOR_X                  16
#define DOOR_Y                  139

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
