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
void Trait::Load(rapidxml::xml_node<char> *node) {
	loadStr(id_str, "id", node);
	id = StringToNumber<int>(id_str);

	loadStr(name, "name", node);
	loadStr(desc, "desc", node);
	loadImgKey(img, "img", node);
	loadBool(unread, "unread", node);
}

void Trait::Clear() {
	id = -1;
	id_str = "";
	name = "";
	desc = "";
	img = 0;
	unread = false;
}

//------------------------------------------------------------------------
// Purpose: Save and load state
//------------------------------------------------------------------------
void Trait::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root, const char *rootname) {
	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, rootname);
	child->append_attribute(doc.allocate_attribute("id", gStrPool->Get(id)));
	child->append_attribute(doc.allocate_attribute("name", name.c_str()));
	child->append_attribute(doc.allocate_attribute("desc", desc.c_str()));
	child->append_attribute(doc.allocate_attribute("img", gStrPool->Get(img)));

	saveBool(unread, "unread", doc, child);

	root->append_node(child);
}

} // End of namespace Crab
