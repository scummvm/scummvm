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

#include "crab/ui/Inventory.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;
using namespace pyrodactyl::item;
using namespace pyrodactyl::people;

//------------------------------------------------------------------------
// Purpose: Load layout
//------------------------------------------------------------------------
void Inventory::load(const Common::String &filename) {
	XMLDoc conf(filename);
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.doc()->first_node("inventory");
		if (nodeValid(node)) {
			if (nodeValid("bg", node))
				bg.load(node->first_node("bg"));

			collection.load(node->first_node("items"));

			/*if (nodeValid("stats", node))
				helper.load(node->first_node("stats"));*/

			if (nodeValid("money", node))
				money.load(node->first_node("money"));
		}
	}
}

void Inventory::LoadItem(const Common::String &char_id, const Common::String &id) {
	Item i;
	XMLDoc item_list(itemfile);
	if (item_list.ready()) {
		rapidxml::xml_node<char> *node = item_list.doc()->first_node("items");
		for (auto n = node->first_node("item"); n != NULL; n = n->next_sibling("item")) {
			Common::String str = n->first_attribute("id")->value();
			if (id == str) {
				i.load(n);
				AddItem(char_id, i);
				break;
			}
		}
	}
}

void Inventory::DelItem(const Common::String &char_id, const Common::String &item_id) {
	collection.Del(char_id, item_id);
}

void Inventory::AddItem(const Common::String &char_id, Item &item) {
	collection.Add(char_id, item);
}

bool Inventory::HasItem(const Common::String &char_id, const Common::String &container, const Common::String &item_id) {
	return collection.Has(char_id, container, item_id);
}

//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void Inventory::draw(Person &obj, const int &money_val) {
	bg.draw();
	// helper.DrawInfo(obj);
	collection.draw(obj.id /*, helper*/);

	money.caption.text = NumberToString(money_val);
	money.draw();
}

//------------------------------------------------------------------------
// Purpose: Handle events
//------------------------------------------------------------------------
void Inventory::handleEvents(const Common::String &char_id, const Common::Event &Event) {
	collection.handleEvents(char_id, Event);
	money.handleEvents(Event);
}

#if 0
//------------------------------------------------------------------------
// Purpose: Handle events
//------------------------------------------------------------------------
void Inventory::handleEvents(const Common::String &char_id, const SDL_Event &Event) {
	collection.handleEvents(char_id, Event);
	money.handleEvents(Event);
}
#endif

//------------------------------------------------------------------------
// Purpose: Load and save items
//------------------------------------------------------------------------
void Inventory::loadState(rapidxml::xml_node<char> *node) {
	if (nodeValid("items", node))
		collection.loadState(node->first_node("items"));
}

void Inventory::saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "items");
	collection.saveState(doc, child);
	root->append_node(child);
}

//------------------------------------------------------------------------
// Purpose: Set UI positions after screen size change
//------------------------------------------------------------------------
void Inventory::setUI() {
	bg.setUI();
	collection.setUI();
	money.setUI();
}

} // End of namespace Crab
