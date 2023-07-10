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
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#ifndef CRAB_LEVEL_OBJECTS_H
#define CRAB_LEVEL_OBJECTS_H

#include "crab/image/ImageManager.h"
#include "crab/common_header.h"
#include "crab/Rectangle.h"

namespace Crab {

namespace pyrodactyl {
namespace level {
// Is the world map accessible from this level?
struct MapVis {
	bool normal, current;

	MapVis() { Set(true); }
	void Set(bool val) {
		normal = val;
		current = val;
	}
};

// Used for drawing the destination marker for point and click movement
struct PlayerDestMarker {
	ImageKey img;
	Vector2i size;

	PlayerDestMarker() { img = 0; }

	void load(rapidxml::xml_node<char> *node);
	void draw(const Vector2i &pos, const Rect &camera);
};
} // End of namespace level
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_LEVEL_OBJECTS_H
