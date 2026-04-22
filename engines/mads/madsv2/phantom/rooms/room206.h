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

#ifndef MADS_PHANTOM_ROOM206_H
#define MADS_PHANTOM_ROOM206_H

#include "mads/madsv2/phantom/phantom.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {
namespace Rooms {

/* ---- Scratch (room-local variables) ---- */

struct Room206Scratch {
	int16 sprite[15];       // Sprite series handles
	int16 sequence[15];     // Sequence handles
	int16 animation[4];     // Animation handles
	int16 anim_0_running;
	int16 prevent;
	int16 prevent_2;
};

#define local ((Room206Scratch *)(&game.scratch[0]))
#define ss    local->sprite
#define seq   local->sequence
#define aa    local->animation

/* ---- Sprite series indices ---- */

#define fx_trap_door            0   /* rm206x0 */
#define fx_panel_closes         1   /* rm206x1 */
#define fx_panel_appears        2   /* rm206x2 */
#define fx_panel_opens          3   /* rm206x3 */
#define fx_note                 4   /* rm206p0 */
#define fx_take_9               5   /* *RDR_9  */

/* ---- Conversation ---- */

#define CONV_MISC               26

/* ---- Trigger constants ---- */

#define ROOM_206_DOOR_CLOSES        60
#define ROOM_206_SCREW_WITH_PANEL   64
#define ROOM_206_ENTER_PANEL        70
#define ROOM_206_TAKE_NOTE          77
#define ROOM_206_FROM_308           82
#define ROOM_206_HIT_ON_HEAD        88
#define ROOM_206_KNOCK              95

/* ---- Player / world positions ---- */

#define PLAYER_X_FROM_205       153
#define PLAYER_Y_FROM_205       148

#define PANEL_X                 108
#define PANEL_Y                 137

#define BEHIND_PANEL_X          67
#define BEHIND_PANEL_Y          127

#define HIT_HEAD_X              168
#define HIT_HEAD_Y              138

/* ---- Entry points ---- */

void room_206_preload();
void room_206_init();
void room_206_daemon();
void room_206_pre_parser();
void room_206_parser();

} // namespace Rooms
} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
