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

#include "crab/image/ImageManager.h"
#include "crab/stat/bonus.h"
#include "crab/common_header.h"
#include "crab/people/person.h"

namespace Crab {

namespace pyrodactyl {
namespace item {
struct Item {
	// The id, name and description of the item
	Common::String _id, _name, _desc;

	// The image for the item
	ImageKey _img;

	// The type of item
	Common::String _type;

	// The stat bonuses provided by the item
	Common::Array<pyrodactyl::stat::Bonus> _bonus;

	// The price
	uint _value;

	Item() {
		clear();
	}

	~Item() {}

	void clear();
	void statChange(pyrodactyl::people::Person &obj, bool increase);

	void load(rapidxml::xml_node<char> *node);
	void draw(const int &x, const int &y);

	void saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
};
} // End of namespace item
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_ITEM_H
