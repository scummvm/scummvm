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
				_bg.load(node->first_node("bg"));

			_collection.load(node->first_node("items"));

			/*if (nodeValid("stats", node))
				helper.load(node->first_node("stats"));*/

			if (nodeValid("money", node))
				_money.load(node->first_node("money"));
		}
	}
}

void Inventory::loadItem(const Common::String &charId, const Common::String &id) {
	Item i;
	XMLDoc itemList(_itemfile);
	if (itemList.ready()) {
		rapidxml::xml_node<char> *node = itemList.doc()->first_node("items");
		for (auto n = node->first_node("item"); n != NULL; n = n->next_sibling("item")) {
			Common::String str = n->first_attribute("id")->value();
			if (id == str) {
				i.load(n);
				addItem(charId, i);
				break;
			}
		}
	}
}

void Inventory::delItem(const Common::String &charId, const Common::String &itemId) {
	_collection.del(charId, itemId);
}

void Inventory::addItem(const Common::String &charId, pyrodactyl::item::Item &item) {
	_collection.add(charId, item);
}

bool Inventory::hasItem(const Common::String &charId, const Common::String &container, const Common::String &itemId) {
	return _collection.has(charId, container, itemId);
}

//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void Inventory::draw(Person &obj, const int &moneyVal) {
	_bg.draw();
	// helper.DrawInfo(obj);
	_collection.draw(obj._id /*, helper*/);

	_money._caption._text = numberToString(moneyVal);
	_money.draw();
}

//------------------------------------------------------------------------
// Purpose: Handle events
//------------------------------------------------------------------------
void Inventory::handleEvents(const Common::String &string, const Common::Event &event) {
	_collection.handleEvents(string, event);
	_money.handleEvents(event);
}

//------------------------------------------------------------------------
// Purpose: Load and save items
//------------------------------------------------------------------------
void Inventory::loadState(rapidxml::xml_node<char> *node) {
	if (nodeValid("items", node))
		_collection.loadState(node->first_node("items"));
}

void Inventory::saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "items");
	_collection.saveState(doc, child);
	root->append_node(child);
}

//------------------------------------------------------------------------
// Purpose: Set UI positions after screen size change
//------------------------------------------------------------------------
void Inventory::setUI() {
	_bg.setUI();
	_collection.setUI();
	_money.setUI();
}

} // End of namespace Crab
