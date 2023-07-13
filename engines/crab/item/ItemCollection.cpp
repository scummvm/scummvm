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

#include "crab/item/ItemCollection.h"

namespace Crab {

using namespace pyrodactyl::people;
using namespace pyrodactyl::item;
using namespace pyrodactyl::ui;

//------------------------------------------------------------------------
// Purpose: Load the reference information
//------------------------------------------------------------------------
void ItemCollection::load(rapidxml::xml_node<char> *node) {
	if (nodeValid("info", node))
		_itemInfo.load(node->first_node("info"));

	if (nodeValid("ref", node))
		_ref.load(node->first_node("ref"));

	if (nodeValid("inc", node))
		_inc.load(node->first_node("inc"));

	if (nodeValid("dim", node)) {
		rapidxml::xml_node<char> *dimnode = node->first_node("dim");
		loadNum(_rows, "rows", dimnode);
		loadNum(_cols, "cols", dimnode);
	}

	loadBool(_useKeyboard, "keyboard", node);
}

//------------------------------------------------------------------------
// Purpose: Add a character's inventory if not added already
//------------------------------------------------------------------------
void ItemCollection::init(const Common::String &charId) {
	if (_item.contains(charId) == 0)
		_item[charId].Init(_ref, _inc, _rows, _cols, _useKeyboard);
}

//------------------------------------------------------------------------
// Purpose: Handle events
//------------------------------------------------------------------------
void ItemCollection::handleEvents(const Common::String &charId, const Common::Event &event) {
	if (_item.contains(charId) > 0)
		_item[charId].handleEvents(event);
}

#if 0
//------------------------------------------------------------------------
// Purpose: Handle events
//------------------------------------------------------------------------
void ItemCollection::handleEvents(const Common::String &char_id, const SDL_Event &Event) {
	if (item.contains(char_id) > 0)
		item[char_id].handleEvents(Event);
}
#endif

//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void ItemCollection::draw(const Common::String &charId) {
	if (_item.contains(charId) > 0)
		_item[charId].draw(_itemInfo);
}

//------------------------------------------------------------------------
// Purpose: Delete an item from a character's inventory
//------------------------------------------------------------------------
void ItemCollection::del(const Common::String &charId, const Common::String &itemId) {
	if (_item.contains(charId) > 0)
		_item[charId].Del(itemId);
}

//------------------------------------------------------------------------
// Purpose: Add an item to a character's inventory
//------------------------------------------------------------------------
void ItemCollection::add(const Common::String &charId, Item &itemData) {
	// We might want to give a player character not yet encountered an item before we ever meet them
	// Which is why we add a new inventory in case the character inventory does not exist yet
	init(charId);

	_item[charId].Equip(itemData);
}

//------------------------------------------------------------------------
// Purpose: Find if a character has an item
//------------------------------------------------------------------------
bool ItemCollection::has(const Common::String &charId, const Common::String &container, const Common::String &itemId) {
	if (_item.contains(charId) > 0)
		return _item[charId].Has(container, itemId);

	return false;
}

//------------------------------------------------------------------------
// Purpose: Load items from save file
//------------------------------------------------------------------------
void ItemCollection::loadState(rapidxml::xml_node<char> *node) {
	for (auto n = node->first_node(); n != NULL; n = n->next_sibling()) {
		// Add all characters in the save file, whether we have them in the inventory or not
		init(n->name());
		_item[n->name()].loadState(n);
	}
}

//------------------------------------------------------------------------
// Purpose: Write items to save file
//------------------------------------------------------------------------
void ItemCollection::saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	for (auto i = _item.begin(); i != _item.end(); ++i) {
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, i->_key.c_str());
		i->_value.saveState(doc, child);
		root->append_node(child);
	}
}

//------------------------------------------------------------------------
// Purpose: Reset UI elements when resolution changes
//------------------------------------------------------------------------
void ItemCollection::setUI() {
	_itemInfo.setUI();

	for (auto i = _item.begin(); i != _item.end(); ++i)
		i->_value.setUI();
}

} // End of namespace Crab
