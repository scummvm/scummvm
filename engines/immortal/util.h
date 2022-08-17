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

#ifndef IMMORTAL_UTIL_H
#define IMMORTAL_UTIL_H

#include "common/system.h"

namespace Immortal {

namespace Util {

void delay(int j);                                  // Delay engine by j jiffies (from driver originally, but makes more sense grouped with misc)
void delay4(int j);                                 // || /4
void delay8(int j);                                 // || /8
bool inside(int x1, int y1, int a, int x2, int y2);
bool insideRect(int x, int y, int r);

}; // namespace Util

}; // namespace Immortal

#endif