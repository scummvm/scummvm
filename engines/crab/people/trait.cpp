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

#include "crab/people/trait.h"

namespace Crab {

using namespace pyrodactyl::people;

//------------------------------------------------------------------------
// Purpose: Load
//------------------------------------------------------------------------
void Trait::load(rapidxml::xml_node<char> *node) {
	loadStr(_idStr, "id", node);
	_id = stringToNumber<int>(_idStr);

	loadStr(_name, "name", node);
	loadStr(_desc, "desc", node);
	loadImgKey(_img, "img", node);
	loadBool(_unread, "unread", node);
}

void Trait::clear() {
	_id = -1;
	_idStr = "";
	_name = "";
	_desc = "";
	_img = 0;
	_unread = false;
}

//------------------------------------------------------------------------
// Purpose: Save and load state
//------------------------------------------------------------------------
void Trait::saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root, const char *rootname) {
	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, rootname);
	child->append_attribute(doc.allocate_attribute("id", gStrPool->Get(_id)));
	child->append_attribute(doc.allocate_attribute("name", _name.c_str()));
	child->append_attribute(doc.allocate_attribute("desc", _desc.c_str()));
	child->append_attribute(doc.allocate_attribute("img", gStrPool->Get(_img)));

	saveBool(_unread, "unread", doc, child);

	root->append_node(child);
}

} // End of namespace Crab
