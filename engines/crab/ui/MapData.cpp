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

#include "crab/crab.h"
#include "crab/GameParam.h"
#include "crab/ui/MapData.h"

namespace Crab {

using namespace pyrodactyl::ui;

void MapData::load(rapidxml::xml_node<char> *node) {
	loadPath(_pathBg, "bg", node);
	loadPath(_pathOverlay, "overlay", node);
}

void MapData::destAdd(const Common::String &name, const int &x, const int &y) {
	MarkerData md;
	md._name = name;
	md._pos.x = x;
	md._pos.y = y;
	_dest.push_back(md);
}

void MapData::saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	rapidxml::xml_node<char> *child_clip = doc.allocate_node(rapidxml::node_element, "clip");
	for (auto &c : _reveal)
		c.saveState(doc, child_clip, "rect");
	root->append_node(child_clip);

	rapidxml::xml_node<char> *child_dest = doc.allocate_node(rapidxml::node_element, "dest");
	for (auto &d : _dest) {
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "pos");
		child->append_attribute(doc.allocate_attribute("name", d._name.c_str()));
		child->append_attribute(doc.allocate_attribute("x", g_engine->_stringPool->get(d._pos.x)));
		child->append_attribute(doc.allocate_attribute("y", g_engine->_stringPool->get(d._pos.y)));
		child_dest->append_node(child);
	}
	root->append_node(child_dest);
}

void MapData::loadState(rapidxml::xml_node<char> *node) {
	_reveal.clear();
	if (nodeValid("clip", node)) {
		rapidxml::xml_node<char> *clipnode = node->first_node("clip");
		for (rapidxml::xml_node<char> *n = clipnode->first_node("rect"); n != nullptr; n = n->next_sibling("rect")) {
			Rect r;
			r.load(n);
			_reveal.push_back(r);
		}
	}

	_dest.clear();
	if (nodeValid("dest", node)) {
		rapidxml::xml_node<char> *destnode = node->first_node("dest");
		for (rapidxml::xml_node<char> *n = destnode->first_node("pos"); n != nullptr; n = n->next_sibling("pos")) {
			MarkerData md;
			loadStr(md._name, "name", n);
			md._pos.load(n);
			_dest.push_back(md);
		}
	}
}

} // End of namespace Crab
