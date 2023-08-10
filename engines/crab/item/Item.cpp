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
#include "crab/GameParam.h"
#include "crab/item/Item.h"

namespace Crab {

using namespace pyrodactyl::image;
using namespace pyrodactyl::item;
using namespace pyrodactyl::stat;
using namespace pyrodactyl::people;

//------------------------------------------------------------------------
// Purpose: Load
//------------------------------------------------------------------------
void Item::load(rapidxml::xml_node<char> *node) {
	if (nodeValid(node)) {
		loadStr(_id, "id", node);
		loadStr(_name, "name", node);
		loadStr(_type, "type", node);
		loadStr(_desc, "desc", node);
		loadImgKey(_img, "img", node);

		_bonus.clear();
		for (auto n = node->first_node("bonus"); n != nullptr; n = n->next_sibling("bonus")) {
			Bonus b;
			b.load(n);
			_bonus.push_back(b);
		}
	}
}

void Item::clear() {
	_id = "";
	_name = "";
	_type = "";
	_desc = "";
	_img = 0;
	_bonus.clear();
	_value = 0;
}

//------------------------------------------------------------------------
// Purpose: Save and load state
//------------------------------------------------------------------------
void Item::saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	root->append_attribute(doc.allocate_attribute("id", _id.c_str()));
	root->append_attribute(doc.allocate_attribute("name", _name.c_str()));
	root->append_attribute(doc.allocate_attribute("type", _type.c_str()));
	root->append_attribute(doc.allocate_attribute("img", g_engine->_stringPool->get(_img)));
	root->append_attribute(doc.allocate_attribute("desc", _desc.c_str()));

	for (const auto &i : _bonus) {
		auto n = doc.allocate_node(rapidxml::node_element, "bonus");
		switch (i._type) {
		case STAT_HEALTH:
			n->append_attribute(doc.allocate_attribute("type", STATNAME_HEALTH));
			break;
		case STAT_ATTACK:
			n->append_attribute(doc.allocate_attribute("type", STATNAME_ATTACK));
			break;
		case STAT_DEFENSE:
			n->append_attribute(doc.allocate_attribute("type", STATNAME_DEFENSE));
			break;
		case STAT_SPEED:
			n->append_attribute(doc.allocate_attribute("type", STATNAME_SPEED));
			break;
			/*case STAT_CHARISMA:n->append_attribute(doc.allocate_attribute("type", STATNAME_CHARISMA)); break;
			case STAT_INTELLIGENCE:n->append_attribute(doc.allocate_attribute("type", STATNAME_INTELLIGENCE)); break;*/
		default:
			break;
		}

		n->append_attribute(doc.allocate_attribute("val", g_engine->_stringPool->get(i._val)));
		root->append_node(n);
	}
}

//------------------------------------------------------------------------
// Purpose: Calculate effect of item on stats
//------------------------------------------------------------------------
void Item::statChange(pyrodactyl::people::Person &obj, bool increase) {
	for (const auto &i : _bonus)
		if (increase)
			obj._stat.change(i._type, i._val);
		else
			obj._stat.change(i._type, -i._val);
}

void Item::draw(const int &x, const int &y) {
	g_engine->_imageManager->draw(x, y, _img);
}

} // End of namespace Crab
