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
		item_info.load(node->first_node("info"));

	if (nodeValid("ref", node))
		ref.load(node->first_node("ref"));

	if (nodeValid("inc", node))
		inc.load(node->first_node("inc"));

	if (nodeValid("dim", node)) {
		rapidxml::xml_node<char> *dimnode = node->first_node("dim");
		loadNum(rows, "rows", dimnode);
		loadNum(cols, "cols", dimnode);
	}

	loadBool(usekeyboard, "keyboard", node);
}

//------------------------------------------------------------------------
// Purpose: Add a character's inventory if not added already
//------------------------------------------------------------------------
void ItemCollection::Init(const Common::String &char_id) {
	if (item.contains(char_id) == 0)
		item[char_id].Init(ref, inc, rows, cols, usekeyboard);
}

//------------------------------------------------------------------------
// Purpose: Handle events
//------------------------------------------------------------------------
void ItemCollection::HandleEvents(const Common::String &char_id, const Common::Event &Event) {
	if (item.contains(char_id) > 0)
		item[char_id].HandleEvents(Event);
}

#if 0
//------------------------------------------------------------------------
// Purpose: Handle events
//------------------------------------------------------------------------
void ItemCollection::HandleEvents(const Common::String &char_id, const SDL_Event &Event) {
	if (item.contains(char_id) > 0)
		item[char_id].HandleEvents(Event);
}
#endif

//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void ItemCollection::draw(const Common::String &char_id) {
	if (item.contains(char_id) > 0)
		item[char_id].draw(item_info);
}

//------------------------------------------------------------------------
// Purpose: Delete an item from a character's inventory
//------------------------------------------------------------------------
void ItemCollection::Del(const Common::String &char_id, const Common::String &item_id) {
	if (item.contains(char_id) > 0)
		item[char_id].Del(item_id);
}

//------------------------------------------------------------------------
// Purpose: Add an item to a character's inventory
//------------------------------------------------------------------------
void ItemCollection::Add(const Common::String &char_id, Item &item_data) {
	// We might want to give a player character not yet encountered an item before we ever meet them
	// Which is why we add a new inventory in case the character inventory does not exist yet
	Init(char_id);

	item[char_id].Equip(item_data);
}

//------------------------------------------------------------------------
// Purpose: Find if a character has an item
//------------------------------------------------------------------------
bool ItemCollection::Has(const Common::String &char_id, const Common::String &container, const Common::String &item_id) {
	if (item.contains(char_id) > 0)
		return item[char_id].Has(container, item_id);

	return false;
}

//------------------------------------------------------------------------
// Purpose: Load items from save file
//------------------------------------------------------------------------
void ItemCollection::loadState(rapidxml::xml_node<char> *node) {
	for (auto n = node->first_node(); n != NULL; n = n->next_sibling()) {
		// Add all characters in the save file, whether we have them in the inventory or not
		Init(n->name());
		item[n->name()].loadState(n);
	}
}

//------------------------------------------------------------------------
// Purpose: Write items to save file
//------------------------------------------------------------------------
void ItemCollection::saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	for (auto i = item.begin(); i != item.end(); ++i) {
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, i->_key.c_str());
		i->_value.saveState(doc, child);
		root->append_node(child);
	}
}

//------------------------------------------------------------------------
// Purpose: Reset UI elements when resolution changes
//------------------------------------------------------------------------
void ItemCollection::setUI() {
	item_info.setUI();

	for (auto i = item.begin(); i != item.end(); ++i)
		i->_value.setUI();
}

} // End of namespace Crab
