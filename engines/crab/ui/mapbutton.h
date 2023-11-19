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

#ifndef CRAB_MAPBUTTON_H
#define CRAB_MAPBUTTON_H

#include "crab/event/triggerset.h"
#include "crab/ui/menu.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class MapButton : public Button {
public:
	// The id of the location
	Common::String _loc;

	// Conditions needed for the location to be unlocked in world map
	pyrodactyl::event::TriggerSet _unlock;

	MapButton() {}
	~MapButton() {}

	void load(rapidxml::xml_node<char> *node) {
		Button::load(node);

		loadStr(_loc, "id", node);
		if (nodeValid("unlock", node, false))
			_unlock.load(node->first_node("unlock"));
	}
};

typedef Menu<MapButton> MapButtonMenu;
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_MAPBUTTON_H
