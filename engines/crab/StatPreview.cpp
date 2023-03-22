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


#include "StatPreview.h"

using namespace pyrodactyl::item;

void StatPreview::Load(rapidxml::xml_node<char> *node) {
	LoadBool(enabled, "enabled", node);

	if (NodeValid("stat", node)) {
		rapidxml::xml_node<char> *snode = node->first_node("stat");
		stat.Load(snode);
		LoadNum(inc_s.x, "w", snode);
		LoadNum(inc_s.y, "h", snode);
	}

	if (NodeValid("unit", node)) {
		rapidxml::xml_node<char> *snode = node->first_node("unit");
		unit.Load(snode);
		LoadNum(inc_u.x, "w", snode);
		LoadNum(inc_u.y, "h", snode);
	}
}

void StatPreview::Draw(Item &item, pyrodactyl::stat::StatDrawHelper &helper) {
	if (enabled) {
		int count = 0;
		for (auto i = item.bonus.begin(); i != item.bonus.end(); ++i, ++count) {
			stat.Draw(helper.Name(i->type), inc_s.x * count, inc_s.y * count);
			unit.Draw(gStrPool.Get(i->val), inc_u.x * count, inc_u.y * count);
		}
	}
}
