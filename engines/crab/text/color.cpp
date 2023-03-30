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

#include "crab/text/color.h"

namespace Crab {

using namespace pyrodactyl::text;

void ColorPool::Load(const std::string &filename) {
	warning("STUB: ColorPool::Load()");
#if 0
	pool.clear();
	XMLDoc col_list(filename);
	if (col_list.ready()) {
		rapidxml::xml_node<char> *node = col_list.Doc()->first_node("colors");
		for (rapidxml::xml_node<char> *n = node->first_node("color"); n != NULL; n = n->next_sibling("color")) {
			SDL_Color c;
			LoadColor(c, n);
			pool.push_back(c);
		}
	}
#endif
}

SDL_Color &ColorPool::Get(const int &num) {
	if (pool.size() <= static_cast<unsigned int>(num))
		return invalid;

	return pool.at(num);
}

} // End of namespace Crab
