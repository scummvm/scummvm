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
#include "crab/stat/StatDrawHelper.h"

namespace Crab {

using namespace pyrodactyl::text;
using namespace pyrodactyl::stat;
using namespace pyrodactyl::people;

void StatInfo::load(rapidxml::xml_node<char> *node) {
	if (nodeValid(node)) {
		loadBool(_active, "active", node);
		_desc.load(node->first_node("info"));
		loadStr(_text, "text", node->first_node("info"));

		if (nodeValid("value", node)) {
			rapidxml::xml_node<char> *valuenode = node->first_node("value");
			_dim.load(valuenode);
			loadImgKey(_full, "full", valuenode);
			loadImgKey(_empty, "empty", valuenode);
		}
	}
}

void StatInfo::draw(const int &val, const int &max) {
	if (_active) {
		using namespace pyrodactyl::image;
		_desc.draw(_text);

		int i = 0;
		for (; i < val; ++i)
			g_engine->_imageManager->draw(_dim.x + i * _dim.w, _dim.y + i * _dim.h, _full);
		for (; i < max; ++i)
			g_engine->_imageManager->draw(_dim.x + i * _dim.w, _dim.y + i * _dim.h, _empty);
	}
}

void StatDrawHelper::load(rapidxml::xml_node<char> *node) {
	if (nodeValid(node)) {
		_info[STAT_HEALTH].load(node->first_node(STATNAME_HEALTH));
		_info[STAT_ATTACK].load(node->first_node(STATNAME_ATTACK));
		_info[STAT_DEFENSE].load(node->first_node(STATNAME_DEFENSE));
		_info[STAT_SPEED].load(node->first_node(STATNAME_SPEED));
		/*info[STAT_CHARISMA].load(node->first_node(STATNAME_CHARISMA));
		info[STAT_INTELLIGENCE].load(node->first_node(STATNAME_INTELLIGENCE));*/
	}
}

void StatDrawHelper::drawInfo(const pyrodactyl::people::Person &obj) {
	for (int i = 0; i < STAT_TOTAL; i++)
		_info[i].draw(obj._stat._val[i]._cur, obj._stat._val[i]._max);
}

} // End of namespace Crab
