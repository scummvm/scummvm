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

#include "crab/level/MusicArea.h"

namespace Crab {

using namespace pyrodactyl::level;

void MusicArea::load(rapidxml::xml_node<char> *node, const bool &echo) {
	Shape::load(node, echo);

	if (nodeValid("properties", node, echo)) {
		auto pnode = node->first_node("properties");
		for (auto n = pnode->first_node("property"); n != NULL; n = n->next_sibling("property")) {
			Common::String name;
			if (loadStr(name, "name", n, echo)) {
				if (name == "music")
					loadBool(track, "value", n, echo);
				else if (name == "id")
					loadNum(id, "value", n, echo);
				else if (name == "loops")
					loadNum(loops, "value", n, echo);
			}
		}
	}
}

} // End of namespace Crab
