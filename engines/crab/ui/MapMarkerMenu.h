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

#ifndef CRAB_MAPMARKERMENU_H
#define CRAB_MAPMARKERMENU_H

#include "crab/ui/StateButton.h"
#include "crab/common_header.h"
#include "crab/ui/menu.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class MapMarkerMenu {
	// The reference map marker
	StateButton ref;

	// The menu containing all the map markers
	Menu<StateButton> menu;

	// The offset at which every map marker is drawn (used to compensate for icon design width)
	struct {
		Vector2i marker, player;
	} offset;

	// The button for the player's current position
	Button player;

public:
	MapMarkerMenu() {}
	~MapMarkerMenu() {}

	void AddButton(const Common::String &name, const int &x, const int &y) {
		StateButton b;
		b.Init(ref, x, y);
		b.tooltip.text = name;
		menu.element.push_back(b);
	}

	void Clear() { menu.element.clear(); }
	void AssignPaths() { menu.AssignPaths(); }

	void SelectDest(const Common::String &name) {
		for (auto &i : menu.element)
			i.State(i.tooltip.text == name);
	}

	void Erase(const Common::String &name) {
		for (auto i = menu.element.begin(); i != menu.element.end(); ++i) {
			if (i->tooltip.text == name) {
				menu.element.erase(i);
				AssignPaths();
				break;
			}
		}
	}

	void load(rapidxml::xml_node<char> *node);

	void draw(const Element &pos, const Vector2i &player_pos, const Rect &camera);

	void HandleEvents(const Element &pos, const Vector2i &player_pos, const Rect &camera, const Common::Event &Event);
#if 0
	void HandleEvents(const Element &pos, const Vector2i &player_pos, const Rect &camera, const SDL_Event &Event);
#endif
	void InternalEvents(const Element &pos, const Vector2i &player_pos, const Rect &camera, Rect bounds);

	void SetUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_MAPMARKERMENU_H
