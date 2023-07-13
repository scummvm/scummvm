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

#ifndef CRAB_ITEMSLOT_H
#define CRAB_ITEMSLOT_H

#include "crab/image/ImageManager.h"
#include "crab/item/Item.h"
#include "crab/ui/StateButton.h"
#include "crab/common_header.h"

namespace Crab {

namespace pyrodactyl {
namespace item {
enum SlotType { SLOT_EQUIP,
				SLOT_STORAGE };

class ItemSlot : public pyrodactyl::ui::StateButton {
	// Ignore the type of item check
	bool no_type;

public:
	// The type of item allowed in this slot (can be overridden by item_type)
	Common::String item_type;

	// Is the slot empty?
	bool empty;

	// Is the slot enabled? (used for stat calculation)
	bool enabled;

	// Is this a new item? Draw the unread notification icon if so
	bool unread;

	// The type of the item slot
	SlotType category;

	// The item contained in the slot
	Item item;

	ItemSlot() {
		empty = true;
		enabled = true;
		category = SLOT_STORAGE;
		no_type = false;
		unread = false;
	}
	~ItemSlot() {}

	void Init(const ItemSlot &ref, const int &XOffset = 0, const int &YOffset = 0);
	void load(rapidxml::xml_node<char> *node);

	void draw();

	pyrodactyl::ui::ButtonAction handleEvents(const Common::Event &Event, const int &XOffset = 0, const int &YOffset = 0);
#if 0
	pyrodactyl::ui::ButtonAction handleEvents(const SDL_Event &Event, const int &XOffset = 0, const int &YOffset = 0);
#endif

	bool CanSwap(ItemSlot &target) { return target.no_type || item._type == target.item_type; }
	bool Swap(ItemSlot &target);
	bool Equip(Item &i);

	void StatChange(pyrodactyl::people::Person &obj, bool increase);

	void saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
	void loadState(rapidxml::xml_node<char> *node);
};
} // End of namespace item
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_ITEMSLOT_H
