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

#ifndef MADS_PHANTOM_ROOM207_H
#define MADS_PHANTOM_ROOM207_H

#include "mads/madsv2/phantom/phantom.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

/* ---- Scratch (room-local variables) ---- */

struct Room207Scratch {
	int16 sprite[15];       // Sprite series handles
	int16 sequence[15];     // Sequence handles
	int16 animation[4];     // Animation handles
	int16 prevent;
	int16 anim_0_running;
};

#define local ((Room207Scratch *)(&game.scratch[0]))
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

/* ---- Sprite series indices ---- */

#define fx_sit          0   /* rm207a0 */
#define fx_curtain      1   /* rm207c0 */
#define fx_lights       2   /* rm207x0 */

/* ---- Trigger constants ---- */

#define ROOM_207_DOOR_CLOSES    60  /* defined but not used */

/* ---- Player / world positions ---- */

#define PLAYER_X_FROM_205   159
#define PLAYER_Y_FROM_205   147

#define SEAT_X              139
#define SEAT_Y              124

/* ---- Entry points ---- */

void room_207_preload();
void room_207_init();
void room_207_daemon();
void room_207_pre_parser();
void room_207_parser();

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
