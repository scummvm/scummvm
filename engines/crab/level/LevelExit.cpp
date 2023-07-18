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

#include "crab/level/LevelExit.h"

namespace Crab {

using namespace pyrodactyl::level;

void Exit::load(rapidxml::xml_node<char> *node) {
	_dim.load(node);
	loadStr(_name, "name", node);

	if (nodeValid("properties", node)) {
		rapidxml::xml_node<char> *propertynode = node->first_node("properties");
		for (auto n = propertynode->first_node("property"); n != NULL; n = n->next_sibling("property")) {
			Common::String nodeName;
			loadStr(nodeName, "name", n);
			if (nodeName == "entry_x") {
				if (!loadNum(_entry.x, "value", n))
					_entry.x = -1;
			} else if (nodeName == "entry_y") {
				if (!loadNum(_entry.y, "value", n))
					_entry.y = -1;
			}
		}
	}
}

} // End of namespace Crab
