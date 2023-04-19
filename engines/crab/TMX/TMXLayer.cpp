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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "crab/TMX/TMXLayer.h"

namespace Crab {

using namespace TMX;

bool Layer::Load(rapidxml::xml_node<char> *node) {
	if (NodeValid(node))
		return LoadStr(name, "name", node) && LoadNum(w, "width", node) && LoadNum(h, "height", node);

	return false;
}

bool MapLayer::Load(const std::string &path, rapidxml::xml_node<char> *node) {
	if (Layer::Load(node)) {
		if (NodeValid("image", node, false)) {
			type = LAYER_IMAGE;
			rapidxml::xml_node<char> *imgnode = node->first_node("image");

			if (imgnode->first_attribute("source") != NULL)
				img.Load((path + imgnode->first_attribute("source")->value()).c_str());
		} else {
			type = LAYER_NORMAL;
			int i = 0;
			std::vector<TileInfo> t;

			//.tmx stores tiles row-first
			for (auto n = node->first_node("data")->first_node("tile"); n != NULL; n = n->next_sibling("tile")) {
				t.push_back(n);

				if (++i >= w) {
					tile.push_back(t);
					t.clear();
					i = 0;
				}
			}
		}

		// load properties associated with the layer, such as:
		//  Is it a prop layer or not? If yes, the prop dimensions
		//  The rate of scrolling of the layer, used for parallax scrolling
		if (NodeValid("properties", node, false)) {
			std::string n, v;
			for (auto p = node->first_node("properties")->first_node("property"); p != NULL; p = p->next_sibling("property")) {
				if (LoadStr(n, "name", p) && LoadStr(v, "value", p)) {
					if (n == "prop" && v == "true")
						type = LAYER_PROP;
					else if (n == "autohide" && v == "true")
						type = LAYER_AUTOHIDE;
					else if (n == "autoshow" && v == "true")
						type = LAYER_AUTOSHOW;
					else if (n == "x") {
						pos.x = StringToNumber<int>(v);
					} else if (n == "y") {
						pos.y = StringToNumber<int>(v);
					} else if (n == "w") {
						pos.w = StringToNumber<int>(v);
					} else if (n == "h") {
						pos.h = StringToNumber<int>(v);
					} else if (n == "scroll_rate_x") {
						rate.x = StringToNumber<float>(v);
						type = LAYER_PARALLAX;
					} else if (n == "scroll_rate_y") {
						rate.y = StringToNumber<float>(v);
						type = LAYER_PARALLAX;
					}
				}
			}
		}

		return true;
	}

	return false;
}

} // End of namespace Crab
