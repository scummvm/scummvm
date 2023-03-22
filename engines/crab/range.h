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

#ifndef CRAB_RANGE_H
#define CRAB_RANGE_H

#include "crab/animframe.h"
#include "crab/common_header.h"

namespace pyrodactyl {
namespace anim {
struct Range {
	bool valid;
	Rect val[DIRECTION_TOTAL];

	Range() { valid = false; }

	void Load(rapidxml::xml_node<char> *node) {
		if (NodeValid(node, false)) {
			val[DIRECTION_DOWN].Load(node->first_node("down"));
			val[DIRECTION_UP].Load(node->first_node("up"));
			val[DIRECTION_LEFT].Load(node->first_node("left"));
			val[DIRECTION_RIGHT].Load(node->first_node("right"));

			valid = true;
		} else
			valid = false;
	}
};
} // End of namespace anim
} // End of namespace pyrodactyl

#endif // CRAB_RANGE_H
