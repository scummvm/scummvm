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

#include "crab/XMLDoc.h"
#include "crab/loaders.h"

namespace Crab {

bool nodeValid(rapidxml::xml_node<char> *node, const bool &echo) {
	if (node == nullptr) {
		/*if (echo)
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "XML error", "node not found", NULL);*/

		return false;
	}

	return true;
}

bool nodeValid(const Common::String &name, rapidxml::xml_node<char> *parentNode, const bool &echo) {
	if (parentNode == nullptr) {
		/*if (echo)
		{
		Common::String error_msg = "parent node of " + name + " not found \n";
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "XML error", error_msg.c_str(), NULL);
		}*/
		return false;
	} else if (parentNode->first_node(name.c_str()) == nullptr) {
		/*if (echo)
		{
		Common::String error_msg = "child node " + name + " of parent node " + parent_node->name() + " not found \n";
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "XML error", error_msg.c_str(), NULL);
		}*/
		return false;
	}

	return true;
}

bool loadStr(Common::String &val, const Common::String &name, rapidxml::xml_node<char> *node, const bool &echo) {
	if (node->first_attribute(name.c_str()) != nullptr)
		val = node->first_attribute(name.c_str())->value();
	else {
		/*if (echo)
		{
		Common::String error_msg = "string " + name + " not found in " + node->name() + "\n";
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "XML error", error_msg.c_str(), NULL);
		}*/
		return false;
	}

	return true;
}

#if 0

bool loadRect(SDL_Rect &rect, rapidxml::xml_node<char> *node, const bool &echo,
	const Common::String &x_name, const Common::String &y_name, const Common::String &w_name, const Common::String &h_name)
{
	if (loadNum(rect.x, x_name, node, echo) && loadNum(rect.y, y_name, node, echo)
		&& loadNum(rect.w, w_name, node, echo) && loadNum(rect.h, h_name, node, echo))
		return true;
	return false;
}
#endif

bool loadColor(SDL_Color &col, rapidxml::xml_node<char> *node, const bool &echo,
	const Common::String &rName, const Common::String &gName, const Common::String &bName) {
	int r = 0, g = 0, b = 0;

	if (loadNum(r, rName, node, echo) && loadNum(g, gName, node, echo) && loadNum(b, bName, node, echo)) {
		col.r = r;
		col.g = g;
		col.b = b;
		return true;
	}

	return false;
}

bool loadColor(int &col, rapidxml::xml_node<char> *node, const bool &echo) {
	return loadNum(col, "color", node, echo);
}

bool loadBool(bool &var, const Common::String &name, rapidxml::xml_node<char> *node, const bool &echo) {
	Common::String str;
	if (loadStr(str, name, node, echo)) {
		if (str == "true")
			var = true;
		else
			var = false;

		return true;
	}

	return false;
}

void saveBool(const bool &var, const char *name, rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	if (var)
		root->append_attribute(doc.allocate_attribute(name, "true"));
	else
		root->append_attribute(doc.allocate_attribute(name, "false"));
}

bool loadStatType(pyrodactyl::stat::StatType &type, rapidxml::xml_node<char> *node, const bool &echo) {
	using namespace pyrodactyl::stat;
	Common::String str;
	if (loadStr(str, "type", node, echo)) {
		if (str == STATNAME_HEALTH)
			type = STAT_HEALTH;
		else if (str == STATNAME_ATTACK)
			type = STAT_ATTACK;
		else if (str == STATNAME_DEFENSE)
			type = STAT_DEFENSE;
		else if (str == STATNAME_SPEED)
			type = STAT_SPEED;
		/*else if (str == STATNAME_CHARISMA) type = STAT_CHARISMA;
		else if (str == STATNAME_INTELLIGENCE) type = STAT_INTELLIGENCE;*/
		else
			return false;

		return true;
	}
	return false;
}

bool loadAlign(Align &align, rapidxml::xml_node<char> *node, const bool &echo, const Common::String &name) {
	int num = 0;
	if (loadNum(num, name, node, echo)) {
		align = static_cast<Align>(num);
		return true;
	}

	return false;
}

bool loadDirection(Direction &dir, rapidxml::xml_node<char> *node, const bool &echo, const Common::String &name) {
	Common::String str;
	if (loadStr(str, name, node, echo)) {
		if (str == "left")
			dir = DIRECTION_LEFT;
		else if (str == "right")
			dir = DIRECTION_RIGHT;
		else if (str == "up")
			dir = DIRECTION_UP;
		else
			dir = DIRECTION_DOWN;

		return true;
	}

	return false;
}

bool loadTextureFlipType(TextureFlipType &flip, rapidxml::xml_node<char> *node, const bool &echo) {
	Common::String str;
	if (loadStr(str, "flip", node, echo)) {
		if (str == "x")
			flip = FLIP_X;
		else if (str == "y")
			flip = FLIP_Y;
		else if (str == "xy")
			flip = FLIP_XY;
		else
			flip = FLIP_NONE;

		return true;
	}

	return false;
}

uint version(const Common::String &filename) {
	uint version = 0;

	XMLDoc doc(filename);
	if (doc.ready()) {
		rapidxml::xml_node<char> *node = doc.doc()->first_node();
		if (nodeValid(node))
			loadNum(version, "version", node);
	}

	return version;
}

} // End of namespace Crab
