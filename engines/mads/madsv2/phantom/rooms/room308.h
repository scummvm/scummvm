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

#ifndef MADS_PHANTOM_ROOM308_H
#define MADS_PHANTOM_ROOM308_H

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
	int16 anim_3_running;
	int16 on_floor;
	int16 prevent;
	int16 prevent_2;

} Scratch;


/* ========================= Sprite Series =================== */

#define fx_raoul                0       /* rm308a0  */
#define fx_her                  1       /* rm308b0  */

/* ========================= Triggers ======================== */

#define ROOM_308_SHOW_TEXT      60


/* ========================= Other Macros ==================== */


#define RAOUL_BOTTOM_X          160
#define RAOUL_BOTTOM_Y          127

#define RAOUL_MIDDLE_X          160
#define RAOUL_MIDDLE_Y          76

#define HIM_BOTH_BOTTOM_X       160
#define HIM_BOTH_BOTTOM_Y       104

#define HER_BOTH_BOTTOM_X       161
#define HER_BOTH_BOTTOM_Y       124

#define CONV_MISC               26

#define HIM_BOTH_TOP_X          160
#define HIM_BOTH_TOP_Y          27

#define HER_BOTH_TOP_X          161
#define HER_BOTH_TOP_Y          48

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
