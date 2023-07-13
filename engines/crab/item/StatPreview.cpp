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


#include "crab/item/StatPreview.h"

namespace Crab {

using namespace pyrodactyl::item;

void StatPreview::load(rapidxml::xml_node<char> *node) {
	loadBool(enabled, "enabled", node);

	if (nodeValid("stat", node)) {
		rapidxml::xml_node<char> *snode = node->first_node("stat");
		stat.load(snode);
		loadNum(inc_s.x, "w", snode);
		loadNum(inc_s.y, "h", snode);
	}

	if (nodeValid("unit", node)) {
		rapidxml::xml_node<char> *snode = node->first_node("unit");
		unit.load(snode);
		loadNum(inc_u.x, "w", snode);
		loadNum(inc_u.y, "h", snode);
	}
}

void StatPreview::draw(Item &item, pyrodactyl::stat::StatDrawHelper &helper) {
	if (enabled) {
		int count = 0;
		for (auto i = item._bonus.begin(); i != item._bonus.end(); ++i, ++count) {
			stat.draw(helper.Name(i->type), inc_s.x * count, inc_s.y * count);
			unit.draw(gStrPool->Get(i->val), inc_u.x * count, inc_u.y * count);
		}
	}
}

} // End of namespace Crab
