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

#include "ImageData.h"
#include "ItemCollection.h"
#include "ItemSlot.h"
#include "Stat.h"
#include "button.h"
#include "common_header.h"

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
	std::string itemfile;

	// Used to draw the money value
	Button money;

public:
	Inventory() {}
	~Inventory() {}

	void Init(const std::string &char_id) { collection.Init(char_id); }

	void LoadItem(const std::string &char_id, const std::string &name);
	void AddItem(const std::string &char_id, pyrodactyl::item::Item &item);
	void DelItem(const std::string &char_id, const std::string &item_id);
	bool HasItem(const std::string &char_id, const std::string &container, const std::string &item_id);

	void Load(const std::string &filename);
	void Draw(pyrodactyl::people::Person &obj, const int &money_val);
	void HandleEvents(const std::string &char_id, const SDL_Event &Event);

	void LoadState(rapidxml::xml_node<char> *node);
	void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);

	void ItemFile(const std::string &filename) { itemfile = filename; }

	void SetUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

#endif // CRAB_INVENTORY_H
