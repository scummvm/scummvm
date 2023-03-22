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

#ifndef CRAB_INPUTVAL_H
#define CRAB_INPUTVAL_H

#include "GameParam.h"
#include "XMLDoc.h"
#include "common_header.h"
#include "loaders.h"

namespace pyrodactyl {
namespace input {
class InputVal {
public:
	// Name of the key (such as "punch", "up")
	std::string name;

	// Keyboard keys
	SDL_Scancode key, alt;

	// Controller button
	SDL_GameControllerButton c_bu;

	// The controller axis
	struct AxisData {
		// The axis
		SDL_GameControllerAxis id;

		// Value of controller axis - because controller axes have a range of values
		int val;

		// Do we want to do a "less than" or "greater than" comparison?
		bool greater;

		// For menus, we want to consider a "flick" of the analog stick - i.e. a back and forth movement
		// This means we toggle this flag once the stick hits maximum value, then check if the stick has
		// gone back to its previous value
		bool toggle;

		AxisData() {
			id = SDL_CONTROLLER_AXIS_INVALID;
			val = 0;
			toggle = false;
			greater = false;
		}

		void LoadState(rapidxml::xml_node<char> *node) {
			LoadEnum(id, "id", node);
			LoadNum(val, "val", node);
			LoadBool(greater, "greater", node);
		}

		void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
			rapidxml::xml_node<char> *child;
			child = doc.allocate_node(rapidxml::node_element, "axis");

			child->append_attribute(doc.allocate_attribute("id", gStrPool.Get(id)));
			child->append_attribute(doc.allocate_attribute("val", gStrPool.Get(val)));

			SaveBool(greater, "greater", doc, child);

			root->append_node(child);
		}
	} c_ax;

	InputVal();

	const bool Equals(const SDL_KeyboardEvent &val);
	const bool Equals(const SDL_ControllerButtonEvent &Event);
	const bool Equals(const SDL_ControllerAxisEvent &Event);

	void LoadState(rapidxml::xml_node<char> *node);
	void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root, const char *title);
};
} // End of namespace input
} // End of namespace pyrodactyl

#endif // CRAB_INPUTVAL_H
