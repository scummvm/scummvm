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

#ifndef GOT_GAME_MOVE_PATTERNS_H
#define GOT_GAME_MOVE_PATTERNS_H

#include "got/data/defines.h"

namespace Got {

extern int (*const _movementFunc[]) (Actor *actor);
int checkMove2(int x, int y, Actor *actor);
int checkMove3(int x, int y, Actor *actor);
int checkMove4(int x, int y, Actor *actor);
void setThorVars();

} // namespace Got

#endif
