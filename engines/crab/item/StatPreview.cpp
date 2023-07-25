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

#include "crab/crab.h"
#include "crab/item/StatPreview.h"

namespace Crab {

using namespace pyrodactyl::item;

void StatPreview::load(rapidxml::xml_node<char> *node) {
	loadBool(_enabled, "enabled", node);

	if (nodeValid("stat", node)) {
		rapidxml::xml_node<char> *snode = node->first_node("stat");
		stat.load(snode);
		loadNum(_incS.x, "w", snode);
		loadNum(_incS.y, "h", snode);
	}

	if (nodeValid("unit", node)) {
		rapidxml::xml_node<char> *snode = node->first_node("unit");
		unit.load(snode);
		loadNum(_incU.x, "w", snode);
		loadNum(_incU.y, "h", snode);
	}
}

void StatPreview::draw(Item &item, pyrodactyl::stat::StatDrawHelper &helper) {
	if (_enabled) {
		int count = 0;
		for (auto i = item._bonus.begin(); i != item._bonus.end(); ++i, ++count) {
			stat.draw(helper.name(i->_type), _incS.x * count, _incS.y * count);
			unit.draw(g_engine->_stringPool->Get(i->_val), _incU.x * count, _incU.y * count);
		}
	}
}

} // End of namespace Crab
