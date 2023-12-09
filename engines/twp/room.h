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

#ifndef TWP_ROOM_H
#define TWP_ROOM_H

#include "common/array.h"
#include "common/rect.h"
#include "common/stream.h"
#include "math/vector2d.h"

namespace Twp {

class Layer {
public:
 	Common::Array<Common::String> names;
    Math::Vector2d parallax;
    int zsort;
};

class Room {
public:
	void load(Common::SeekableReadStream &s);

public:
	Common::String name;			// Name of the room
	Common::String sheet;			// Name of the spritesheet to use
	Math::Vector2d roomSize;		// Size of the room
	int fullscreen;				// Indicates if a room is a closeup room (fullscreen=1) or not (fullscreen=2), just a guess
	int height;					// Height of the room (what else ?)
	Common::Array<Layer> layers;	// Parallax layers of a room
};

} // namespace Twp

#endif
