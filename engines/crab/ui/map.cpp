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

#include "crab/crab.h"
#include "crab/ui/map.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;
using namespace pyrodactyl::input;

//------------------------------------------------------------------------
// Purpose: Load stuff that can't be modified by the user
//------------------------------------------------------------------------
void Map::load(const Common::String &filename, pyrodactyl::event::Info &info) {
	XMLDoc conf(filename);
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.doc()->first_node("map");
		if (nodeValid(node)) {
			if (nodeValid("img", node)) {
				rapidxml::xml_node<char> *imgnode = node->first_node("img");
				loadNum(_speed, "speed", imgnode);

				for (auto n = imgnode->first_node("map"); n != nullptr; n = n->next_sibling("map"))
					_map.push_back(n);
			}

			if (nodeValid("fg", node))
				_fg.load(node->first_node("fg"));

			if (nodeValid("dim", node)) {
				loadNum(_camera.w, "x", node->first_node("dim"));
				loadNum(_camera.h, "y", node->first_node("dim"));
			}

			if (nodeValid("pos", node))
				_pos.load(node->first_node("pos"));

			if (nodeValid("scroll", node))
				_scroll.load(node->first_node("scroll"));

			if (nodeValid("marker", node))
				_marker.load(node->first_node("marker"));

			if (nodeValid("title", node))
				_title.load(node->first_node("title"));

			if (nodeValid("locations", node))
				_travel.load(node->first_node("locations"));

			if (nodeValid("overlay", node))
				_buOverlay.load(node->first_node("overlay"));
		}
	}

	setImage(_cur, true);
	update(info);
	calcBounds();
}

//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void Map::draw(pyrodactyl::event::Info &info) {

	// The map graphic is clipped to fit inside the UI
	_imgBg.draw(_pos.x, _pos.y, &_camera);

	if (_overlay) {
		// The overlay needs to be clipped as well, so we must find the intersection of the camera and the clip itself
		for (auto &i : _map[_cur]._reveal) {
			Rect r = i;
			int X = _pos.x + i.x - _camera.x, Y = _pos.y + i.y - _camera.y;

			// Do not draw any area of the clip that is outside the camera bounds

			// If we're outside the left edges, we need to cull the left point
			if (X < _pos.x) {
				X += _camera.x - i.x;
				r.x += _camera.x - i.x;
				r.w -= _camera.x - i.x;

				if (r.w < 0)
					r.w = 0;
			}

			if (Y < _pos.y) {
				Y += _camera.y - i.y;
				r.y += _camera.y - i.y;
				r.h -= _camera.y - i.y;

				if (r.h < 0)
					r.h = 0;
			}

			// If we're outside the right edge, we need to cull the width and height
			if (X + r.w > _pos.x + _camera.w)
				r.w = _pos.x + _camera.w - X;
			if (Y + r.h > _pos.y + _camera.h)
				r.h = _pos.y + _camera.h - Y;

			_imgOverlay.draw(X, Y, &r);
		}
	}

	_travel.draw(_camera.x - _pos.x, _camera.y - _pos.y);

	_fg.draw();
	_buOverlay.draw();

	_title._text = info.curLocName();
	_title.draw();

	_marker.draw(_pos, _playerPos, _camera);

	_scroll.draw();
}

//------------------------------------------------------------------------
// Purpose: Center the world map on a spot
//------------------------------------------------------------------------
void Map::center(const Vector2i &vec) {
	_camera.x = vec.x - _camera.w / 2;
	_camera.y = vec.y - _camera.h / 2;
	validate();
}

//------------------------------------------------------------------------
// Purpose: Keep the camera in bounds and decide marker visibility
//------------------------------------------------------------------------
void Map::validate() {
	// Make all scroll buttons visible first
	for (auto &i : _scroll._element)
		i._visible = true;

	// Keep camera in bounds
	if (_camera.x + _camera.w > _size.x)
		_camera.x = _size.x - _camera.w;
	if (_camera.y + _camera.h > _size.y)
		_camera.y = _size.y - _camera.h;
	if (_camera.x < 0)
		_camera.x = 0;
	if (_camera.y < 0)
		_camera.y = 0;

	// decide visibility of scroll buttons
	_scroll._element[DIRECTION_RIGHT]._visible = !(_camera.x == _size.x - _camera.w);
	_scroll._element[DIRECTION_DOWN]._visible = !(_camera.y == _size.y - _camera.h);
	_scroll._element[DIRECTION_LEFT]._visible = !(_camera.x == 0);
	_scroll._element[DIRECTION_UP]._visible = !(_camera.y == 0);
}

//------------------------------------------------------------------------
// Purpose: Move
//------------------------------------------------------------------------
void Map::move(const Common::Event &event) {
	// Reset the velocity to avoid weirdness
	_vel.x = 0;
	_vel.y = 0;

	// We don't use the result, but this keeps the button states up to date
	_scroll.handleEvents(event);

	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_RBUTTONDOWN: {
		bool click = false;
		int count = 0;
		for (auto &i : _scroll._element) {
			if (i.contains(g_engine->_mouse->_button)) {
				if (count == DIRECTION_UP)
					_vel.y = -1 * _speed;
				else if (count == DIRECTION_DOWN)
					_vel.y = _speed;
				else if (count == DIRECTION_RIGHT)
					_vel.x = _speed;
				else if (count == DIRECTION_LEFT)
					_vel.x = -1 * _speed;

				click = true;
			}
			count++;
		}

		if (!click) {
			_pan = true;
			_vel.x = 0;
			_vel.y = 0;
		} else
			_pan = false;
	} break;

	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONUP:
		_pan = false;
		break;

	case Common::EVENT_MOUSEMOVE:
		if (_pan) {
			_camera.x -= g_engine->_mouse->_rel.x;
			_camera.y -= g_engine->_mouse->_rel.y;
			validate();
		}
		break;
	default:
		warning("STUB: Map keyboard processing");
#if 0
		// Move the map camera if player presses the direction keys
		if (g_engine->_inputManager->Equals(IU_UP, Event) == SDL_PRESSED)
			vel.y = -1 * speed;
		else if (g_engine->_inputManager->Equals(IU_DOWN, Event) == SDL_PRESSED)
			vel.y = speed;
		else if (g_engine->_inputManager->Equals(IU_RIGHT, Event) == SDL_PRESSED)
			vel.x = speed;
		else if (g_engine->_inputManager->Equals(IU_LEFT, Event) == SDL_PRESSED)
			vel.x = -1 * speed;
		// Stop moving when we release a key (but only in that direction)
		else if (g_engine->_inputManager->Equals(IU_UP, Event) == SDL_RELEASED || g_engine->_inputManager->Equals(IU_DOWN, Event) == SDL_RELEASED)
			vel.y = 0;
		else if (g_engine->_inputManager->Equals(IU_LEFT, Event) == SDL_RELEASED || g_engine->_inputManager->Equals(IU_RIGHT, Event) == SDL_RELEASED)
			vel.x = 0;
		break;
	}
#endif
	}
}

#if 0
//------------------------------------------------------------------------
// Purpose: Move
//------------------------------------------------------------------------
void Map::Move(const SDL_Event &Event) {
	// Reset the velocity to avoid weirdness
	vel.x = 0;
	vel.y = 0;

	// We don't use the result, but this keeps the button states up to date
	scroll.handleEvents(Event);

	switch (Event.type) {
	case SDL_MOUSEBUTTONDOWN: {
		bool click = false;
		int count = 0;
		for (auto &i : scroll.element) {
			if (i.Contains(g_engine->_mouse->button)) {
				if (count == DIRECTION_UP)
					vel.y = -1 * speed;
				else if (count == DIRECTION_DOWN)
					vel.y = speed;
				else if (count == DIRECTION_RIGHT)
					vel.x = speed;
				else if (count == DIRECTION_LEFT)
					vel.x = -1 * speed;

				click = true;
			}
			count++;
		}

		if (!click) {
			pan = true;
			vel.x = 0;
			vel.y = 0;
		} else
			pan = false;
	} break;
	case SDL_MOUSEBUTTONUP:
		pan = false;
		break;
	case SDL_MOUSEMOTION:
		if (pan) {
			camera.x -= g_engine->_mouse->rel.x;
			camera.y -= g_engine->_mouse->rel.y;
			Validate();
		}
		break;
	default: {
		// Move the map camera if player presses the direction keys
		if (g_engine->_inputManager->Equals(IU_UP, Event) == SDL_PRESSED)
			vel.y = -1 * speed;
		else if (g_engine->_inputManager->Equals(IU_DOWN, Event) == SDL_PRESSED)
			vel.y = speed;
		else if (g_engine->_inputManager->Equals(IU_RIGHT, Event) == SDL_PRESSED)
			vel.x = speed;
		else if (g_engine->_inputManager->Equals(IU_LEFT, Event) == SDL_PRESSED)
			vel.x = -1 * speed;
		// Stop moving when we release a key (but only in that direction)
		else if (g_engine->_inputManager->Equals(IU_UP, Event) == SDL_RELEASED || g_engine->_inputManager->Equals(IU_DOWN, Event) == SDL_RELEASED)
			vel.y = 0;
		else if (g_engine->_inputManager->Equals(IU_LEFT, Event) == SDL_RELEASED || g_engine->_inputManager->Equals(IU_RIGHT, Event) == SDL_RELEASED)
			vel.x = 0;
		break;
	}
	}
}
#endif

//------------------------------------------------------------------------
// Purpose: Internal Events
//------------------------------------------------------------------------
void Map::internalEvents(pyrodactyl::event::Info &info) {
	// The map overlay and button state should be in sync
	_buOverlay._state = _overlay;

	_camera.x += _vel.x;
	_camera.y += _vel.y;
	validate();

	for (auto &i : _travel._element)
		i._visible = i.x >= _camera.x && i.y >= _camera.y;

	_marker.internalEvents(_pos, _playerPos, _camera, _bounds);
}

//------------------------------------------------------------------------
// Purpose: Handle Events
//------------------------------------------------------------------------
bool Map::handleEvents(pyrodactyl::event::Info &info, const Common::Event &event) {
	int choice = _travel.handleEvents(event, -1 * _camera.x, -1 * _camera.y);
	if (choice >= 0) {
		_curLoc = _travel._element[choice]._loc;
		_pan = false;
		return true;
	}

	_marker.handleEvents(_pos, _playerPos, _camera, event);

	move(event);
	if (_buOverlay.handleEvents(event) == BUAC_LCLICK)
		_overlay = _buOverlay._state;

	return false;
}

void Map::setImage(const unsigned int &val, const bool &force) {
	if (force || (_cur != val && val < _map.size())) {
		_cur = val;

		_imgBg.deleteImage();
		_imgOverlay.deleteImage();

		_imgBg.load(_map[_cur]._pathBg);
		_imgOverlay.load(_map[_cur]._pathOverlay);

		_size.x = _imgBg.w();
		_size.y = _imgBg.h();

		_marker.clear();
		for (auto &i : _map[_cur]._dest)
			_marker.addButton(i._name, i._pos.x, i._pos.y);

		_marker.assignPaths();
	}
}

//------------------------------------------------------------------------
// Purpose: Select the marker corresponding to a quest title
//------------------------------------------------------------------------
void Map::selectDest(const Common::String &name) {
	_marker.selectDest(name);
}

//------------------------------------------------------------------------
// Purpose: Update the status of the fast travel buttons
//------------------------------------------------------------------------
void Map::update(pyrodactyl::event::Info &info) {
	for (auto &i : _travel._element) {
		i._unlock.evaluate(info);
		i._visible = i._unlock.result();
	}
}

//------------------------------------------------------------------------
// Purpose: Add a rectangle to the revealed world map data
//------------------------------------------------------------------------
void Map::revealAdd(const int &id, const Rect &area) {
	if ((unsigned int)id < _map.size()) {
		for (auto i = _map[id]._reveal.begin(); i != _map[id]._reveal.end(); ++i)
			if (*i == area)
				return;

		_map[id]._reveal.push_back(area);
	}
}

//------------------------------------------------------------------------
// Purpose: Add or remove a destination marker from the world map
//------------------------------------------------------------------------
void Map::destAdd(const Common::String &name, const int &x, const int &y) {
	if (_cur < _map.size()) {
		for (auto i = _map[_cur]._dest.begin(); i != _map[_cur]._dest.end(); ++i) {
			if (i->_name == name) {
				i->_pos.x = x;
				i->_pos.y = y;
				return;
			}
		}

		_map[_cur].destAdd(name, x, y);
		_marker.addButton(name, x, y);
		_marker.assignPaths();
	}
}

void Map::destDel(const Common::String &name) {
	if (_cur < _map.size()) {
		for (auto i = _map[_cur]._dest.begin(); i != _map[_cur]._dest.end(); ++i) {
			if (i->_name == name) {
				_map[_cur]._dest.erase(i);
				break;
			}
		}

		_marker.erase(name);
	}
}

//------------------------------------------------------------------------
// Purpose: Save and load object state
//------------------------------------------------------------------------
void Map::saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "map");

	child->append_attribute(doc.allocate_attribute("cur", g_engine->_stringPool->Get(_cur)));
	saveBool(_overlay, "overlay", doc, child);

	for (auto r = _map.begin(); r != _map.end(); ++r) {
		rapidxml::xml_node<char> *child_data = doc.allocate_node(rapidxml::node_element, "data");
		r->saveState(doc, child_data);
		child->append_node(child_data);
	}

	root->append_node(child);
}

void Map::loadState(rapidxml::xml_node<char> *node) {
	if (nodeValid("map", node)) {
		rapidxml::xml_node<char> *mapnode = node->first_node("map");
		loadBool(_overlay, "overlay", mapnode);

		int val = _cur;
		loadNum(val, "cur", mapnode);

		auto r = _map.begin();
		for (rapidxml::xml_node<char> *n = mapnode->first_node("data"); n != nullptr && r != _map.end(); n = n->next_sibling("data"), ++r)
			r->loadState(n);

		setImage(val, true);
	}
}

//------------------------------------------------------------------------
// Purpose: Reset the UI positions in response to change in resolution
//------------------------------------------------------------------------
void Map::setUI() {
	_pos.setUI();
	_fg.setUI();

	_travel.setUI();
	_marker.setUI();

	_buOverlay.setUI();
	_scroll.setUI();
	_title.setUI();

	calcBounds();
}

} // End of namespace Crab
