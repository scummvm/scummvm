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
	Common::Array<MapData> map;

	// Index of the currently visible map
	unsigned int cur;

	// The currently loaded map background image
	pyrodactyl::image::Image img_bg, img_overlay;

	// The position at which map image has to be drawn
	Element pos;

	// Foreground image of the map
	ImageData fg;

	// size = Dimensions of the map image
	// mouse = The current coordinates of the mouse
	// vel = The speed at which the map is moving
	Vector2i size, mouse, vel;

	// The reference speed of the camera movement
	int speed;

	// The pan toggle is used when the mouse is down and moving simultaneously
	// overlay = true if we are showing a more detailed world map
	bool pan, overlay;

	// The camera size and position for the map
	// Bounds is the area we draw the map elements for
	Rect camera, bounds;

	// The button to toggle between showing the overlay or not
	ToggleButton bu_overlay;

	// All data for drawing map markers
	MapMarkerMenu marker;

	// The map name is drawn here
	HoverInfo title;

	// The buttons for scrolling the map (only visible if there is area to scroll)
	ButtonMenu scroll;

	// The menu for fast travel locations
	MapButtonMenu travel;

	void CalcBounds() {
		bounds.x = pos.x;
		bounds.y = pos.y;
		bounds.w = camera.w;
		bounds.h = camera.h;
	}

public:
	// The currently selected location
	Common::String cur_loc;

	// The coordinates of the player's current location
	Vector2i player_pos;

	Map() {
		speed = 1;
		pan = false;
		cur = 0;
		overlay = true;
	}
	~Map() {
		img_bg.Delete();
		img_overlay.Delete();
	}

	void load(const Common::String &filename, pyrodactyl::event::Info &info);

	void Draw(pyrodactyl::event::Info &info);
	bool HandleEvents(pyrodactyl::event::Info &info, const Common::Event &Event);
#if 0
	bool HandleEvents(pyrodactyl::event::Info &info, const SDL_Event &Event);
#endif
	void InternalEvents(pyrodactyl::event::Info &info);

	void Center(const Vector2i &pos);
	void Move(const Common::Event &Event);
#if 0
	void Move(const SDL_Event &Event);
#endif
	void Validate();

	void RevealAdd(const int &id, const Rect &area);
	void DestAdd(const Common::String &name, const int &x, const int &y);
	void DestDel(const Common::String &name);
	void SelectDest(const Common::String &name);

	void Update(pyrodactyl::event::Info &info);
	void SetImage(const unsigned int &val, const bool &force = false);

	void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
	void LoadState(rapidxml::xml_node<char> *node);

	void SetUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_MAP_H
