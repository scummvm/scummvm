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

extern void nextFrame(Actor *actor);
extern bool pointWithin(int x, int y, int x1, int y1, int x2, int y2);
extern bool overlap(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);
extern int  reverseDirection(Actor *actor);
extern void thorShoots();
extern void thorDamaged(Actor *actor);
extern void actorDestroyed(Actor *actor);
extern int  actorShoots(Actor *actor, int dir);
extern void actorAlwaysShoots(Actor *actor, int dir);
extern void actorDamaged(Actor *actor, int damage);
extern void moveActor(Actor *actor);

} // namespace Got

#endif
