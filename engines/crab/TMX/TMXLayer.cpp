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

#include "crab/TMX/TMXLayer.h"

namespace Crab {

using namespace TMX;

bool Layer::load(rapidxml::xml_node<char> *node) {
	if (nodeValid(node))
		return loadStr(_name, "name", node) && loadNum(_w, "width", node) && loadNum(_h, "height", node);

	return false;
}

bool MapLayer::load(const Common::String &path, rapidxml::xml_node<char> *node) {
	if (Layer::load(node)) {
		if (nodeValid("image", node, false)) {
			_type = LAYER_IMAGE;
			rapidxml::xml_node<char> *imgnode = node->first_node("image");

			if (imgnode->first_attribute("source") != nullptr)
				_img.load((path + imgnode->first_attribute("source")->value()));
		} else {
			_type = LAYER_NORMAL;
			int i = 0;
			Common::Array<TileInfo> t;

			//.tmx stores tiles row-first
			for (auto n = node->first_node("data")->first_node("tile"); n != nullptr; n = n->next_sibling("tile")) {
				t.push_back(n);

				if (++i >= _w) {
					_tile.push_back(t);
					t.clear();
					i = 0;
				}
			}
		}

		// load properties associated with the layer, such as:
		//  Is it a prop layer or not? If yes, the prop dimensions
		//  The rate of scrolling of the layer, used for parallax scrolling
		if (nodeValid("properties", node, false)) {
			Common::String n, v;
			for (auto p = node->first_node("properties")->first_node("property"); p != nullptr; p = p->next_sibling("property")) {
				if (loadStr(n, "name", p) && loadStr(v, "value", p)) {
					if (n == "prop" && v == "true")
						_type = LAYER_PROP;
					else if (n == "autohide" && v == "true")
						_type = LAYER_AUTOHIDE;
					else if (n == "autoshow" && v == "true")
						_type = LAYER_AUTOSHOW;
					else if (n == "x") {
						_pos.x = stringToNumber<int>(v);
					} else if (n == "y") {
						_pos.y = stringToNumber<int>(v);
					} else if (n == "w") {
						_pos.w = stringToNumber<int>(v);
					} else if (n == "h") {
						_pos.h = stringToNumber<int>(v);
					} else if (n == "scroll_rate_x") {
						_rate.x = stringToNumber<float>(v);
						_type = LAYER_PARALLAX;
					} else if (n == "scroll_rate_y") {
						_rate.y = stringToNumber<float>(v);
						_type = LAYER_PARALLAX;
					}
				}
			}
		}

		return true;
	}

	return false;
}

} // End of namespace Crab
