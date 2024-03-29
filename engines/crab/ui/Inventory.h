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

#ifndef CRAB_INVENTORY_H
#define CRAB_INVENTORY_H

#include "crab/item/ItemCollection.h"
#include "crab/item/ItemSlot.h"
#include "crab/ui/button.h"
#include "crab/ui/ImageData.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class Inventory {
	// The background image and location
	ImageData _bg;

	// The equipment and storage space, stored according to the player character id
	pyrodactyl::item::ItemCollection _collection;

	// To draw the player stats
	// pyrodactyl::stat::StatDrawHelper helper;

	// The file where all the item information is stored
	Common::Path _itemfile;

	// Used to draw the money value
	Button _money;

public:
	Inventory() {}
	~Inventory() {}

	void init(const Common::String &charId) {
		_collection.init(charId);
	}

	void loadItem(const Common::String &charId, const Common::String &name);
	void addItem(const Common::String &charId, pyrodactyl::item::Item &item);
	void delItem(const Common::String &charId, const Common::String &itemId);
	bool hasItem(const Common::String &charId, const Common::String &container, const Common::String &itemId);

	void load(const Common::Path &filename);
	void draw(pyrodactyl::people::Person &obj, const int &moneyVal);

	void handleEvents(const Common::String &string, const Common::Event &Event);

	void loadState(rapidxml::xml_node<char> *node);
	void saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);

	void itemFile(const Common::String &filename) {
		_itemfile = filename;
	}

	void setUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_INVENTORY_H
