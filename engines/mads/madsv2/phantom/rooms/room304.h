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

#ifndef MADS_PHANTOM_ROOM304_H
#define MADS_PHANTOM_ROOM304_H

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

	int16 anim_0_running;
	int16 anim_1_running;
	int16 anim_2_running;

	int16 raoul_frame;
	int16 raoul_action;
	int16 raoul_look_count;

	int16 raoul_fight_frame;
	int16 raoul_fight_action;
	int16 raoul_fight_talk_count;

	int16 phantom_frame;
	int16 phantom_action;

} Scratch;


/* ========================= Sprite Series =================== */

#define fx_chan_tilt            0       /* rm304f0  */
#define fx_chan                 1       /* rm304f1  */

/* ========================= Triggers ======================== */

#define ROOM_304_CLIMB_DOWN     60
#define ROOM_304_CLIMB_UP       65

/* ========================= Other Macros ==================== */


#define RAOUL_CLIMB_UP          0
#define RAOUL_OTHER             1

#define CONV23_RAOUL_OTHER      0
#define CONV23_RAOUL_SHUT_UP    1        
#define CONV23_RAOUL_TALK       2        

#define CONV_FIGHT_23           23

#define CONV23_PHANTOM_DAAE     0
#define CONV23_PHANTOM_LET_GO   1
#define CONV23_PHANTOM_FIGHT    2

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
