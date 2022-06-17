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

/*
 * Based on ScottFree interpreter version 1.14 developed by Swansea
 * University Computer Society without disassembly of any other game
 * drivers, only of game databases as permitted by EEC law (for purposes
 * of compatibility).
 *
 * Licensed under GPLv2
 *
 * https://github.com/angstsmurf/spatterlight/tree/master/terps/scott
 */

#ifndef GLK_SCOTT_ROBIN_OF_SHERWOOD_H
#define GLK_SCOTT_ROBIN_OF_SHERWOOD_H

#include "glk/scott/definitions.h"

namespace Glk {
namespace Scott {

void updateRobinOfSherwoodAnimations();
void robinOfSherwoodLook();
GameIDType loadExtraSherwoodData();
GameIDType loadExtraSherwoodData64();
int isForestLocation();
void sherwoodAction(int p);

} // End of namespace Scott
} // End of namespace Glk

#endif
