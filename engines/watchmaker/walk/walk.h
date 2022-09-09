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

#ifndef WATCHMAKER_WALK_H
#define WATCHMAKER_WALK_H

#include "watchmaker/types.h"
#include "watchmaker/t3d.h"

namespace Watchmaker {

#define EPSILON         0.007f

#define NOOLDINTERS     1
#define NOCURINTERS     2
#define OLDANGLESKIP    4
#define CURANGLESKIP    8
#define CLICKINTO       16
#define POINTOUT1       32
#define POINTOUT2       64
#define LONGPATH        128
#define NOBOUNDCHECK    256
#define NOTSKIPPABLE    512

extern int32 ActionLen[];
extern int32 ActionStart[];

void FindPath(int32 oc, t3dCAMERA *Camera);
void ForceAnimInBounds(int32 oc);

} // End of namespace Watchmaker

#endif // WATCHMAKER_WALK_H
