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

#ifndef CRAB_ITEM_H
#define CRAB_ITEM_H

#include "ImageManager.h"
#include "bonus.h"
#include "common_header.h"
#include "person.h"

namespace pyrodactyl {
namespace item {
struct Item {
	// The id, name and description of the item
	std::string id, name, desc;

	// The image for the item
	ImageKey img;

	// The type of item
	std::string type;

	// The stat bonuses provided by the item
	std::vector<pyrodactyl::stat::Bonus> bonus;

	// The price
	unsigned int value;

	Item() { Clear(); }
	~Item() {}

	void Clear();
	void StatChange(pyrodactyl::people::Person &obj, bool increase);

	void Load(rapidxml::xml_node<char> *node);
	void Draw(const int &x, const int &y);

	void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
};
} // End of namespace item
} // End of namespace pyrodactyl

#endif // CRAB_ITEM_H
