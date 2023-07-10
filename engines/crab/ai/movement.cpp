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

#include "crab/ai/movement.h"

namespace Crab {

using namespace pyrodactyl::ai;

//------------------------------------------------------------------------
// Purpose: Walk in preset paths
//------------------------------------------------------------------------
void MovementSet::load(rapidxml::xml_node<char> *node) {
	_enabled = true;
	loadBool(_repeat, "repeat", node);
	for (auto n = node->first_node("walk"); n != NULL; n = n->next_sibling("walk"))
		_path.push_back(n);
}

//------------------------------------------------------------------------
// Purpose: To make the AI patrol/wait along certain points
//------------------------------------------------------------------------
bool MovementSet::internalEvents(const Rect rect) {
	if (_enabled) {
		// If we are at the current waypoint, get to the next waypoint
		if (_path[_cur]._target.Collide(rect)) {
			_cur = (_cur + 1) % _path.size();
			_timer.Start();
		}

		// Wait according to the delay value in the node
		if (_timer.Ticks() >= _path[_cur]._delay)
			return true;
	}

	return false;
}

} // End of namespace Crab
