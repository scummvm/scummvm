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
				loadNum(speed, "speed", imgnode);

				for (auto n = imgnode->first_node("map"); n != NULL; n = n->next_sibling("map"))
					map.push_back(n);
			}

			if (nodeValid("fg", node))
				fg.load(node->first_node("fg"));

			if (nodeValid("dim", node)) {
				loadNum(camera.w, "x", node->first_node("dim"));
				loadNum(camera.h, "y", node->first_node("dim"));
			}

			if (nodeValid("pos", node))
				pos.load(node->first_node("pos"));

			if (nodeValid("scroll", node))
				scroll.load(node->first_node("scroll"));

			if (nodeValid("marker", node))
				marker.load(node->first_node("marker"));

			if (nodeValid("title", node))
				title.load(node->first_node("title"));

			if (nodeValid("locations", node))
				travel.load(node->first_node("locations"));

			if (nodeValid("overlay", node))
				bu_overlay.load(node->first_node("overlay"));
		}
	}

	SetImage(cur, true);
	Update(info);
	CalcBounds();
}

//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void Map::draw(pyrodactyl::event::Info &info) {

	// The map graphic is clipped to fit inside the UI
	img_bg.draw(pos.x, pos.y, &camera);

	if (overlay) {
		// The overlay needs to be clipped as well, so we must find the intersection of the camera and the clip itself
		for (auto &i : map[cur].reveal) {
			Rect r = i;
			int X = pos.x + i.x - camera.x, Y = pos.y + i.y - camera.y;

			// Do not draw any area of the clip that is outside the camera bounds

			// If we're outside the left edges, we need to cull the left point
			if (X < pos.x) {
				X += camera.x - i.x;
				r.x += camera.x - i.x;
				r.w -= camera.x - i.x;

				if (r.w < 0)
					r.w = 0;
			}

			if (Y < pos.y) {
				Y += camera.y - i.y;
				r.y += camera.y - i.y;
				r.h -= camera.y - i.y;

				if (r.h < 0)
					r.h = 0;
			}

			// If we're outside the right edge, we need to cull the width and height
			if (X + r.w > pos.x + camera.w)
				r.w = pos.x + camera.w - X;
			if (Y + r.h > pos.y + camera.h)
				r.h = pos.y + camera.h - Y;

			img_overlay.draw(X, Y, &r);
		}
	}

	travel.draw(camera.x - pos.x, camera.y - pos.y);

	fg.draw();
	bu_overlay.draw();

	title._text = info.curLocName();
	title.draw();

	marker.draw(pos, player_pos, camera);

	scroll.draw();
}

//------------------------------------------------------------------------
// Purpose: Center the world map on a spot
//------------------------------------------------------------------------
void Map::Center(const Vector2i &vec) {
	camera.x = vec.x - camera.w / 2;
	camera.y = vec.y - camera.h / 2;
	Validate();
}

//------------------------------------------------------------------------
// Purpose: Keep the camera in bounds and decide marker visibility
//------------------------------------------------------------------------
void Map::Validate() {
	// Make all scroll buttons visible first
	for (auto &i : scroll.element)
		i._visible = true;

	// Keep camera in bounds
	if (camera.x + camera.w > size.x)
		camera.x = size.x - camera.w;
	if (camera.y + camera.h > size.y)
		camera.y = size.y - camera.h;
	if (camera.x < 0)
		camera.x = 0;
	if (camera.y < 0)
		camera.y = 0;

	// decide visibility of scroll buttons
	scroll.element[DIRECTION_RIGHT]._visible = !(camera.x == size.x - camera.w);
	scroll.element[DIRECTION_DOWN]._visible = !(camera.y == size.y - camera.h);
	scroll.element[DIRECTION_LEFT]._visible = !(camera.x == 0);
	scroll.element[DIRECTION_UP]._visible = !(camera.y == 0);
}

//------------------------------------------------------------------------
// Purpose: Move
//------------------------------------------------------------------------
void Map::Move(const Common::Event &Event) {
	// Reset the velocity to avoid weirdness
	vel.x = 0;
	vel.y = 0;

	// We don't use the result, but this keeps the button states up to date
	scroll.handleEvents(Event);

	switch (Event.type) {
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_RBUTTONDOWN: {
		bool click = false;
		int count = 0;
		for (auto &i : scroll.element) {
			if (i.Contains(g_engine->_mouse->_button)) {
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

	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONUP:
		pan = false;
		break;

	case Common::EVENT_MOUSEMOVE:
		if (pan) {
			camera.x -= g_engine->_mouse->_rel.x;
			camera.y -= g_engine->_mouse->_rel.y;
			Validate();
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
	bu_overlay._state = overlay;

	camera.x += vel.x;
	camera.y += vel.y;
	Validate();

	for (auto &i : travel.element)
		i._visible = i.x >= camera.x && i.y >= camera.y;

	marker.internalEvents(pos, player_pos, camera, bounds);
}

//------------------------------------------------------------------------
// Purpose: Handle Events
//------------------------------------------------------------------------
bool Map::handleEvents(pyrodactyl::event::Info &info, const Common::Event &Event) {
	int choice = travel.handleEvents(Event, -1 * camera.x, -1 * camera.y);
	if (choice >= 0) {
		cur_loc = travel.element[choice].loc;
		pan = false;
		return true;
	}

	marker.handleEvents(pos, player_pos, camera, Event);

	Move(Event);
	if (bu_overlay.handleEvents(Event) == BUAC_LCLICK)
		overlay = bu_overlay._state;

	return false;
}

#if 0
//------------------------------------------------------------------------
// Purpose: Handle Events
//------------------------------------------------------------------------
bool Map::handleEvents(pyrodactyl::event::Info &info, const SDL_Event &Event) {
	int choice = travel.handleEvents(Event, -1 * camera.x, -1 * camera.y);
	if (choice >= 0) {
		cur_loc = travel.element[choice].loc;
		pan = false;
		return true;
	}

	marker.handleEvents(pos, player_pos, camera, Event);

	Move(Event);
	if (bu_overlay.handleEvents(Event) == BUAC_LCLICK)
		overlay = bu_overlay.state;

	return false;
}
#endif

void Map::SetImage(const unsigned int &val, const bool &force) {
	if (force || (cur != val && val < map.size())) {
		cur = val;

		img_bg.deleteImage();
		img_overlay.deleteImage();

		img_bg.load(map[cur].path_bg);
		img_overlay.load(map[cur].path_overlay);

		size.x = img_bg.w();
		size.y = img_bg.h();

		marker.Clear();
		for (auto &i : map[cur].dest)
			marker.AddButton(i.name, i.pos.x, i.pos.y);

		marker.AssignPaths();
	}
}

//------------------------------------------------------------------------
// Purpose: Select the marker corresponding to a quest title
//------------------------------------------------------------------------
void Map::SelectDest(const Common::String &name) {
	marker.SelectDest(name);
}

//------------------------------------------------------------------------
// Purpose: Update the status of the fast travel buttons
//------------------------------------------------------------------------
void Map::Update(pyrodactyl::event::Info &info) {
	for (auto &i : travel.element) {
		i.unlock.evaluate(info);
		i._visible = i.unlock.result();
	}
}

//------------------------------------------------------------------------
// Purpose: Add a rectangle to the revealed world map data
//------------------------------------------------------------------------
void Map::RevealAdd(const int &id, const Rect &area) {
	if ((unsigned int)id < map.size()) {
		for (auto i = map[id].reveal.begin(); i != map[id].reveal.end(); ++i)
			if (*i == area)
				return;

		map[id].reveal.push_back(area);
	}
}

//------------------------------------------------------------------------
// Purpose: Add or remove a destination marker from the world map
//------------------------------------------------------------------------
void Map::DestAdd(const Common::String &name, const int &x, const int &y) {
	if (cur < map.size()) {
		for (auto i = map[cur].dest.begin(); i != map[cur].dest.end(); ++i) {
			if (i->name == name) {
				i->pos.x = x;
				i->pos.y = y;
				return;
			}
		}

		map[cur].DestAdd(name, x, y);
		marker.AddButton(name, x, y);
		marker.AssignPaths();
	}
}

void Map::DestDel(const Common::String &name) {
	if (cur < map.size()) {
		for (auto i = map[cur].dest.begin(); i != map[cur].dest.end(); ++i) {
			if (i->name == name) {
				map[cur].dest.erase(i);
				break;
			}
		}

		marker.Erase(name);
	}
}

//------------------------------------------------------------------------
// Purpose: Save and load object state
//------------------------------------------------------------------------
void Map::saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "map");

	child->append_attribute(doc.allocate_attribute("cur", gStrPool->Get(cur)));
	saveBool(overlay, "overlay", doc, child);

	for (auto r = map.begin(); r != map.end(); ++r) {
		rapidxml::xml_node<char> *child_data = doc.allocate_node(rapidxml::node_element, "data");
		r->saveState(doc, child_data);
		child->append_node(child_data);
	}

	root->append_node(child);
}

void Map::loadState(rapidxml::xml_node<char> *node) {
	if (nodeValid("map", node)) {
		rapidxml::xml_node<char> *mapnode = node->first_node("map");
		loadBool(overlay, "overlay", mapnode);

		int val = cur;
		loadNum(val, "cur", mapnode);

		auto r = map.begin();
		for (rapidxml::xml_node<char> *n = mapnode->first_node("data"); n != NULL && r != map.end(); n = n->next_sibling("data"), ++r)
			r->loadState(n);

		SetImage(val, true);
	}
}

//------------------------------------------------------------------------
// Purpose: Reset the UI positions in response to change in resolution
//------------------------------------------------------------------------
void Map::setUI() {
	pos.setUI();
	fg.setUI();

	travel.setUI();
	marker.setUI();

	bu_overlay.setUI();
	scroll.setUI();
	title.setUI();

	CalcBounds();
}

} // End of namespace Crab
