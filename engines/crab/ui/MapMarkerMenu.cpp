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

#include "crab/ui/MapMarkerMenu.h"

namespace Crab {

using namespace pyrodactyl::ui;

//------------------------------------------------------------------------
// Purpose: Load
//------------------------------------------------------------------------
void MapMarkerMenu::load(rapidxml::xml_node<char> *node) {
	if (nodeValid("ref", node))
		_ref.load(node->first_node("ref"));

	if (nodeValid("player", node))
		_player.load(node->first_node("player"));

	if (nodeValid("offset", node)) {
		rapidxml::xml_node<char> *offnode = node->first_node("offset");

		if (nodeValid("marker", offnode))
			_offset._marker.load(offnode->first_node("marker"));

		if (nodeValid("player", offnode))
			_offset._player.load(offnode->first_node("player"));
	}

	_menu.useKeyboard(true);
}

//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void MapMarkerMenu::draw(const Element &pos, const Vector2i &player_pos, const Rect &camera) {
	// Calculate all offsets
	Vector2i offsetP(pos.x + player_pos.x + _offset._player.x - camera.x, pos.y + player_pos.y + _offset._player.y - camera.y);
	Vector2i offsetM(pos.x - camera.x + _offset._marker.x, pos.y - camera.y + _offset._marker.y);

	// Only draw the image - captions drawn later to prevent drawing another button over caption
	_player.imageCaptionOnlyDraw(offsetP.x, offsetP.y);

	for (auto &i : _menu._element)
		i.imageCaptionOnlyDraw(offsetM.x, offsetM.y);

	// Now draw the tool-tips for everything combined
	_player.hoverInfoOnlyDraw(offsetP.x, offsetP.y);

	for (auto &i : _menu._element)
		i.hoverInfoOnlyDraw(offsetM.x, offsetM.y);
}

//------------------------------------------------------------------------
// Purpose: Handle Events
//------------------------------------------------------------------------
void MapMarkerMenu::handleEvents(const Element &pos, const Vector2i &playerPos, const Rect &camera, const Common::Event &event) {
	if (playerPos.x >= camera.x && playerPos.y >= camera.y)
		_player.handleEvents(event, pos.x + playerPos.x - camera.x + _offset._player.x, pos.y + playerPos.y - camera.y + _offset._player.y);

	int choice = _menu.handleEvents(event, pos.x - camera.x + _offset._marker.x, pos.y - camera.y + _offset._marker.y);
	if (choice != -1) {
		int c = 0;
		for (auto &i : _menu._element) {
			if (c == choice) // For an already selected marker, clicking it toggles the selection state
				i.state(!i.state());
			else
				i.state(false);

			++c;
		}
	}
}

#if 0
//------------------------------------------------------------------------
// Purpose: Handle Events
//------------------------------------------------------------------------
void MapMarkerMenu::handleEvents(const Element &pos, const Vector2i &player_pos, const Rect &camera, const SDL_Event &Event) {
	if (player_pos.x >= camera.x && player_pos.y >= camera.y)
		player.handleEvents(Event, pos.x + player_pos.x - camera.x + offset.player.x, pos.y + player_pos.y - camera.y + offset.player.y);

	int choice = menu.handleEvents(Event, pos.x - camera.x + offset.marker.x, pos.y - camera.y + offset.marker.y);
	if (choice != -1) {
		int c = 0;
		for (auto &i : menu.element) {
			if (c == choice) // For an already selected marker, clicking it toggles the selection state
				i.State(!i.State());
			else
				i.State(false);

			++c;
		}
	}
}
#endif

//------------------------------------------------------------------------
// Purpose: Internal Events
//------------------------------------------------------------------------
void MapMarkerMenu::internalEvents(const Element &pos, const Vector2i &playerPos, const Rect &camera, Rect bounds) {
	// Find if the player marker is visible or not
	{
		Rect r(pos.x + playerPos.x - _offset._marker.x - camera.x,
			   pos.y + playerPos.y - _offset._marker.y - camera.y,
			   _player.w + _offset._marker.x,
			   _player.h + _offset._marker.y);

		_player._visible = bounds.Contains(r);
	}

	// Redefine p for marker buttons
	Vector2i p(pos.x - camera.x + _offset._marker.x, pos.y - camera.y + _offset._marker.y);

	// Calculate visibility for each marker
	for (auto &i : _menu._element) {
		Rect r(i.x + p.x - _offset._marker.x, i.y + p.y - _offset._marker.y,
			   i.w + _offset._marker.x, i.h + _offset._marker.y);

		i._visible = bounds.Contains(r);
	}
}

//------------------------------------------------------------------------
// Purpose: Reposition UI
//------------------------------------------------------------------------
void MapMarkerMenu::setUI() {
	_player.setUI();
	_menu.setUI();
}

} // End of namespace Crab
