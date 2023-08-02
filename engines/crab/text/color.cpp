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

void ColorPool::load(const Common::String &filename) {
	_pool.clear();
	XMLDoc colList(filename);
	if (colList.ready()) {
		rapidxml::xml_node<char> *node = colList.doc()->first_node("colors");
		for (rapidxml::xml_node<char> *n = node->first_node("color"); n != nullptr; n = n->next_sibling("color")) {
			SDL_Color c;
			loadColor(c, n);
			_pool.push_back(c);
		}
	}
}

SDL_Color &ColorPool::get(const int &num) {
	if (_pool.size() <= static_cast<unsigned int>(num))
		return _invalid;

	return _pool[num];
}

} // End of namespace Crab
