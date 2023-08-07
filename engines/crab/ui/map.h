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

#ifndef CRAB_MAP_H
#define CRAB_MAP_H

#include "crab/event/GameEventInfo.h"
#include "crab/ui/ImageData.h"
#include "crab/image/ImageManager.h"
#include "crab/ui/MapData.h"
#include "crab/ui/MapMarkerMenu.h"
#include "crab/ui/StateButton.h"
#include "crab/ui/ToggleButton.h"
#include "crab/common_header.h"
#include "crab/ui/mapbutton.h"
#include "crab/event/triggerset.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class Map {
	// We have multiple world maps, each with their own data
	Common::Array<MapData> _map;

	// Index of the currently visible map
	uint _cur;

	// The currently loaded map background image
	pyrodactyl::image::Image _imgBg, _imgOverlay;

	// The position at which map image has to be drawn
	Element _pos;

	// Foreground image of the map
	ImageData _fg;

	// size = Dimensions of the map image
	// mouse = The current coordinates of the mouse
	// vel = The speed at which the map is moving
	Vector2i _size, _mouse, _vel;

	// The reference speed of the camera movement
	int _speed;

	// The pan toggle is used when the mouse is down and moving simultaneously
	// overlay = true if we are showing a more detailed world map
	bool _pan, _overlay;

	// The camera size and position for the map
	// Bounds is the area we draw the map elements for
	Rect _camera, _bounds;

	// The button to toggle between showing the overlay or not
	ToggleButton _buOverlay;

	// All data for drawing map markers
	MapMarkerMenu _marker;

	// The map name is drawn here
	HoverInfo _title;

	// The buttons for scrolling the map (only visible if there is area to scroll)
	ButtonMenu _scroll;

	// The menu for fast travel locations
	MapButtonMenu _travel;

	void calcBounds() {
		_bounds.x = _pos.x;
		_bounds.y = _pos.y;
		_bounds.w = _camera.w;
		_bounds.h = _camera.h;
	}

public:
	// The currently selected location
	Common::String _curLoc;

	// The coordinates of the player's current location
	Vector2i _playerPos;

	Map() {
		_speed = 1;
		_pan = false;
		_cur = 0;
		_overlay = true;
	}

	~Map() {
		_imgBg.deleteImage();
		_imgOverlay.deleteImage();
	}

	void load(const Common::String &filename, pyrodactyl::event::Info &info);

	void draw(pyrodactyl::event::Info &info);
	bool handleEvents(pyrodactyl::event::Info &info, const Common::Event &event);
	void internalEvents(pyrodactyl::event::Info &info);

	void center(const Vector2i &pos);
	void move(const Common::Event &event);
	void validate();

	void revealAdd(const int &id, const Rect &area);
	void destAdd(const Common::String &name, const int &x, const int &y);
	void destDel(const Common::String &name);
	void selectDest(const Common::String &name);

	void update(pyrodactyl::event::Info &info);
	void setImage(const uint &val, const bool &force = false);

	void saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
	void loadState(rapidxml::xml_node<char> *node);

	void setUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_MAP_H
