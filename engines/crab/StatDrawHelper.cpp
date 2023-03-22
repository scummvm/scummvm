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

#include "stdafx.h"

#include "StatDrawHelper.h"

using namespace pyrodactyl::text;
using namespace pyrodactyl::stat;
using namespace pyrodactyl::people;

void StatInfo::Load(rapidxml::xml_node<char> *node) {
	if (NodeValid(node)) {
		LoadBool(active, "active", node);
		desc.Load(node->first_node("info"));
		LoadStr(text, "text", node->first_node("info"));

		if (NodeValid("value", node)) {
			rapidxml::xml_node<char> *valuenode = node->first_node("value");
			dim.Load(valuenode);
			LoadImgKey(full, "full", valuenode);
			LoadImgKey(empty, "empty", valuenode);
		}
	}
}

void StatInfo::Draw(const int &val, const int &max) {
	if (active) {
		using namespace pyrodactyl::image;
		desc.Draw(text);

		int i = 0;
		for (; i < val; ++i)
			gImageManager.Draw(dim.x + i * dim.w, dim.y + i * dim.h, full);
		for (; i < max; ++i)
			gImageManager.Draw(dim.x + i * dim.w, dim.y + i * dim.h, empty);
	}
}

void StatDrawHelper::Load(rapidxml::xml_node<char> *node) {
	if (NodeValid(node)) {
		info[STAT_HEALTH].Load(node->first_node(STATNAME_HEALTH));
		info[STAT_ATTACK].Load(node->first_node(STATNAME_ATTACK));
		info[STAT_DEFENSE].Load(node->first_node(STATNAME_DEFENSE));
		info[STAT_SPEED].Load(node->first_node(STATNAME_SPEED));
		/*info[STAT_CHARISMA].Load(node->first_node(STATNAME_CHARISMA));
		info[STAT_INTELLIGENCE].Load(node->first_node(STATNAME_INTELLIGENCE));*/
	}
}

void StatDrawHelper::DrawInfo(const Person &obj) {
	for (int i = 0; i < STAT_TOTAL; i++)
		info[i].Draw(obj.stat.val[i].cur, obj.stat.val[i].max);
}
