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

#include "inputval.h"

using namespace pyrodactyl::input;

InputVal::InputVal() {
	key = SDL_SCANCODE_UNKNOWN;
	alt = SDL_SCANCODE_UNKNOWN;
	c_bu = SDL_CONTROLLER_BUTTON_INVALID;
}

//------------------------------------------------------------------------
// Purpose: Load input values
//------------------------------------------------------------------------
void InputVal::LoadState(rapidxml::xml_node<char> *node) {
	LoadStr(name, "name", node);
	LoadEnum(key, "key", node);
	LoadEnum(alt, "alt", node);
	LoadEnum(c_bu, "bu", node);

	if (NodeValid("axis", node, false))
		c_ax.LoadState(node->first_node("axis"));
}

//------------------------------------------------------------------------
// Purpose: Save them
//------------------------------------------------------------------------
void InputVal::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root, const char *title) {
	rapidxml::xml_node<char> *child;
	child = doc.allocate_node(rapidxml::node_element, title);

	child->append_attribute(doc.allocate_attribute("name", name.c_str()));
	child->append_attribute(doc.allocate_attribute("key", gStrPool.Get(key)));
	child->append_attribute(doc.allocate_attribute("alt", gStrPool.Get(alt)));
	child->append_attribute(doc.allocate_attribute("bu", gStrPool.Get(c_bu)));

	if (c_ax.id != SDL_CONTROLLER_AXIS_INVALID)
		c_ax.SaveState(doc, child);

	root->append_node(child);
}

//------------------------------------------------------------------------
// Purpose: See if we have been using the analog sticks
//------------------------------------------------------------------------
const bool InputVal::Equals(const SDL_ControllerAxisEvent &Event) {
	if (c_ax.id == Event.axis) {
		if (!c_ax.toggle)
			c_ax.toggle = (c_ax.greater && Event.value > c_ax.val) || (!c_ax.greater && Event.value < c_ax.val);
		else {
			if (!((c_ax.greater && Event.value > c_ax.val) || (!c_ax.greater && Event.value < c_ax.val))) {
				c_ax.toggle = false;
				return true;
			};
		}
	}

	return false;
}

//------------------------------------------------------------------------
// Purpose: See if we have been using the controller buttons sticks
//------------------------------------------------------------------------
const bool InputVal::Equals(const SDL_ControllerButtonEvent &Event) {
	return (c_bu == Event.button);
}

//------------------------------------------------------------------------
// Purpose: See if we have been using the keyboard
//------------------------------------------------------------------------
const bool InputVal::Equals(const SDL_KeyboardEvent &Event) {
	return (key == Event.keysym.scancode || alt == Event.keysym.scancode);
}
