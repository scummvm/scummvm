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

#include "crab/ui/menu.h"
#include "crab/ui/StateButton.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class MapMarkerMenu {
	// The reference map marker
	StateButton _ref;

	// The menu containing all the map markers
	Menu<StateButton> _menu;

	// The offset at which every map marker is drawn (used to compensate for icon design width)
	struct {
		Vector2i _marker, _player;
	} _offset;

	// The button for the player's current position
	Button _player;

public:
	MapMarkerMenu() {}
	~MapMarkerMenu() {}

	void addButton(const Common::String &name, const int &x, const int &y) {
		StateButton b;
		b.init(_ref, x, y);
		b._tooltip._text = name;
		_menu._element.push_back(b);
	}

	void clear() {
		_menu._element.clear();
	}

	void assignPaths() {
		_menu.assignPaths();
	}

	void selectDest(const Common::String &name) {
		for (auto &i : _menu._element)
			i.state(i._tooltip._text == name);
	}

	void erase(const Common::String &name) {
		for (auto i = _menu._element.begin(); i != _menu._element.end(); ++i) {
			if (i->_tooltip._text == name) {
				_menu._element.erase(i);
				assignPaths();
				break;
			}
		}
	}

	void load(rapidxml::xml_node<char> *node);

	void draw(const Element &pos, const Vector2i &player_pos, const Rect &camera);

	void handleEvents(const Element &pos, const Vector2i &playerPos, const Rect &camera, const Common::Event &event);

	void internalEvents(const Element &pos, const Vector2i &playerPos, const Rect &camera, Rect bounds);

	void setUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_MAPMARKERMENU_H
