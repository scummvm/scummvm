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

#ifndef CRAB_LOADERS_H
#define CRAB_LOADERS_H

#include "common/debug.h"
#include "common/str.h"
#include "crab/color.h"
#include "crab/gametype.h"
#include "crab/numstr.h"
#include "crab/rapidxml/rapidxml.hpp"

namespace Crab {

// Function to check if node is valid
// return true for valid, false otherwise
bool nodeValid(rapidxml::xml_node<char> *node, const bool &echo = true);
bool nodeValid(const Common::String &name, rapidxml::xml_node<char> *parentNode, const bool &echo = true);

// Functions to load attributes from xml files
// return true on success, false on failure
bool loadStr(Common::String &val, const Common::String &name, rapidxml::xml_node<char> *node, const bool &echo = true);

// Used for loading numerical types
template<typename T>
bool loadNum(T &val, const Common::String &name, rapidxml::xml_node<char> *node, const bool &echo = true) {
	if (node->first_attribute(name.c_str()) != nullptr)
		val = stringToNumber<T>(node->first_attribute(name.c_str())->value());
	else {
		if (echo)
			warning("XML: attribute %s not found in node %s -> %s", name.c_str(), node->parent()->name(), node->name());
		return false;
	}

	return true;
}

// Used for loading enumerator types that are integers
template<typename T>
bool loadEnum(T &val, const Common::String &name, rapidxml::xml_node<char> *node, const bool &echo = true) {
	if (node->first_attribute(name.c_str()) != nullptr)
		val = static_cast<T>(stringToNumber<int>(node->first_attribute(name.c_str())->value()));
	else {
		if (echo)
			warning("XML: attribute %s not found in node %s -> %s", name.c_str(), node->parent()->name(), node->name());
		return false;
	}

	return true;
}

//Load Color
bool loadColor(Color &col, rapidxml::xml_node<char> *node, const bool &echo = true,
	const Common::String &r_name = "r", const Common::String &g_name = "g", const Common::String &b_name = "b");

//Shortcut to load integer color index to a number
bool loadColor(int &col, rapidxml::xml_node<char> *node, const bool &echo = true);

// Load two dimensional coordinates
template<typename T>
bool loadXY(T &x, T &y, rapidxml::xml_node<char> *node, const bool &echo = true) {
	bool result = loadNum(x, "x", node, echo);
	result = loadNum(y, "y", node, echo) && result;

	return result;
}

// Load three dimensional coordinates
template<typename T>
bool loadXYZ(T &x, T &y, T &z, rapidxml::xml_node<char> *node, const bool &echo = true) {
	bool result = loadNum(x, "x", node, echo);
	result = loadNum(y, "y", node, echo) && result;
	result = loadNum(z, "z", node, echo) && result;

	return result;
}

// Load Boolean variable
bool loadBool(bool &var, const Common::String &name, rapidxml::xml_node<char> *node, const bool &echo = true);

// Write Boolean variable to file
void saveBool(const bool &var, const char *name, rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);

// Functions to load various type of objects
bool loadStatType(pyrodactyl::stat::StatType &type, rapidxml::xml_node<char> *node, const bool &echo = true);

bool loadAlign(Align &align, rapidxml::xml_node<char> *node, const bool &echo = true, const Common::String &name = "align");
bool loadDirection(Direction &dir, rapidxml::xml_node<char> *node, const bool &echo = true, const Common::String &name = "dir");

bool loadTextureFlipType(TextureFlipType &flip, rapidxml::xml_node<char> *node, const bool &echo = true);

// Check the version of a file
uint version(const Common::String &filename);

} // End of namespace Crab

#endif // CRAB_LOADERS_H
