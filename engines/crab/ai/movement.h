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

#ifndef CRAB_MOVEMENT_H
#define CRAB_MOVEMENT_H

#include "crab/Rectangle.h"
#include "crab/common_header.h"
#include "crab/timer.h"
#include "crab/vectors.h"

namespace Crab {

namespace pyrodactyl {
namespace ai {
struct MovementSet {
	struct Movement {
		// The position this sprite has to move to
		Rect target;

		// The time the sprite waits before it starts moving to pos
		uint32 delay;

		Movement(rapidxml::xml_node<char> *node) {
			target.Load(node);
			LoadNum(delay, "delay", node);
		}
	};

	// The path followed by the sprite
	std::vector<Movement> path;

	// If true, sprite repeats the path pattern after reaching the last co-ordinate
	bool repeat;

	// The current path node we are traveling to
	unsigned int cur;

	// The time the sprite has spent waiting is calculated here
	Timer timer;

	// Is this set enabled?
	bool enabled;

	MovementSet() {
		cur = 0;
		repeat = false;
		enabled = false;
	}
	MovementSet(rapidxml::xml_node<char> *node) {
		cur = 0;
		Load(node);
	}

	void Load(rapidxml::xml_node<char> *node);

	bool InternalEvents(const Rect rect);
	Rect Target() { return path[cur].target; }
};
} // End of namespace ai
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_MOVEMENT_H
