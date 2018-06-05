/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef STARTREK_COMMON_H
#define STARTREK_COMMON_H

#include "common/scummsys.h"

namespace Common {
class Rect;
class Serializer;
}

namespace StarTrek {

template<class T>
T min(T a, T b) { return a < b ? a : b; }

template<class T>
T max(T a, T b) { return a > b ? a : b; }

Common::Rect getRectEncompassing(Common::Rect r1, Common::Rect r2);
void serializeRect(Common::Rect rect, Common::Serializer &ser);


// Fixed-point (16.16) number
typedef int32 Fixed32;

// Fixed-point (8.8) number
typedef int16 Fixed16;

}

#endif
