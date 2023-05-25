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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "crab/crab.h"
#include "crab/item/Item.h"

namespace Crab {

using namespace pyrodactyl::image;
using namespace pyrodactyl::item;
using namespace pyrodactyl::stat;
using namespace pyrodactyl::people;

//------------------------------------------------------------------------
// Purpose: Load
//------------------------------------------------------------------------
void Item::Load(rapidxml::xml_node<char> *node) {
	if (NodeValid(node)) {
		LoadStr(id, "id", node);
		LoadStr(name, "name", node);
		LoadStr(type, "type", node);
		LoadStr(desc, "desc", node);
		LoadImgKey(img, "img", node);

		bonus.clear();
		for (auto n = node->first_node("bonus"); n != NULL; n = n->next_sibling("bonus")) {
			Bonus b;
			b.Load(n);
			bonus.push_back(b);
		}
	}
}

void Item::Clear() {
	id = "";
	name = "";
	type = "";
	desc = "";
	img = 0;
	bonus.clear();
	value = 0;
}

//------------------------------------------------------------------------
// Purpose: Save and load state
//------------------------------------------------------------------------
void Item::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	root->append_attribute(doc.allocate_attribute("id", id.c_str()));
	root->append_attribute(doc.allocate_attribute("name", name.c_str()));
	root->append_attribute(doc.allocate_attribute("type", type.c_str()));
	root->append_attribute(doc.allocate_attribute("img", gStrPool->Get(img)));
	root->append_attribute(doc.allocate_attribute("desc", desc.c_str()));

	for (auto i = bonus.begin(); i != bonus.end(); ++i) {
		auto n = doc.allocate_node(rapidxml::node_element, "bonus");
		switch (i->type) {
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

		n->append_attribute(doc.allocate_attribute("val", gStrPool->Get(i->val)));
		root->append_node(n);
	}
}

//------------------------------------------------------------------------
// Purpose: Calculate effect of item on stats
//------------------------------------------------------------------------
void Item::StatChange(Person &obj, bool increase) {
	for (auto i = bonus.begin(); i != bonus.end(); ++i)
		if (increase)
			obj.stat.Change(i->type, i->val);
		else
			obj.stat.Change(i->type, -i->val);
}

void Item::Draw(const int &x, const int &y) {
	g_engine->_imageManager->Draw(x, y, img);
}

} // End of namespace Crab
