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

#include "crab/ui/ImageData.h"
#include "crab/item/ItemCollection.h"
#include "crab/item/ItemSlot.h"
#include "crab/stat/Stat.h"
#include "crab/ui/button.h"
#include "crab/common_header.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class Inventory {
	// The background image and location
	ImageData bg;

	// The equipment and storage space, stored according to the player character id
	pyrodactyl::item::ItemCollection collection;

	// To draw the player stats
	// pyrodactyl::stat::StatDrawHelper helper;

	// The file where all the item information is stored
	Common::String itemfile;

	// Used to draw the money value
	Button money;

public:
	Inventory() {}
	~Inventory() {}

	void Init(const Common::String &char_id) { collection.Init(char_id); }

	void LoadItem(const Common::String &char_id, const Common::String &name);
	void AddItem(const Common::String &char_id, pyrodactyl::item::Item &item);
	void DelItem(const Common::String &char_id, const Common::String &item_id);
	bool HasItem(const Common::String &char_id, const Common::String &container, const Common::String &item_id);

	void load(const Common::String &filename);
	void Draw(pyrodactyl::people::Person &obj, const int &money_val);

	void HandleEvents(const Common::String &char_id, const Common::Event &Event);
#if 0
	void HandleEvents(const Common::String &char_id, const SDL_Event &Event);
#endif

	void LoadState(rapidxml::xml_node<char> *node);
	void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);

	void ItemFile(const Common::String &filename) { itemfile = filename; }

	void SetUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_INVENTORY_H
