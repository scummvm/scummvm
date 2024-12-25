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

#ifndef GOT_GAME_MOVE_H
#define GOT_GAME_MOVE_H

#include "got/data/defines.h"

namespace Got {

extern void next_frame(ACTOR *actr);
extern bool point_within(int x, int y, int x1, int y1, int x2, int y2);
extern bool overlap(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);
extern int  reverse_direction(ACTOR *actr);
extern void thor_shoots();
extern void thor_damaged(ACTOR *actr);
extern void actor_destroyed(ACTOR *actr);
extern int  actor_shoots(ACTOR *actr, int dir);
extern void actor_always_shoots(ACTOR *actr, int dir);
extern void actor_damaged(ACTOR *actr, int damage);
extern void move_actor(ACTOR *actr);

} // namespace Got

#endif
