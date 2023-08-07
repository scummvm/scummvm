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

#include "crab/event/quest.h"

namespace Crab {

using namespace pyrodactyl::event;

Quest::Quest(const Common::String &title, const Common::String &text, const bool &unread, const bool &marker) : _title(title) {
	_text.insert_at(0, text);
	_unread = unread;
	_marker = marker;
}

void Quest::loadState(rapidxml::xml_node<char> *node) {
	loadStr(_title, "title", node);
	loadBool(_unread, "unread", node);
	loadBool(_marker, "marker", node);

	for (rapidxml::xml_node<char> *n = node->first_node("info"); n != nullptr; n = n->next_sibling("info"))
		_text.push_back(n->value());
}

void Quest::saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "quest");
	child->append_attribute(doc.allocate_attribute("title", _title.c_str()));

	saveBool(_unread, "unread", doc, child);
	saveBool(_marker, "marker", doc, child);

	for (const auto &i : _text) {
		rapidxml::xml_node<char> *grandchild = doc.allocate_node(rapidxml::node_element, "info");
		grandchild->value(i.c_str());
		child->append_node(grandchild);
	}

	root->append_node(child);
}

} // End of namespace Crab
