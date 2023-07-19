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

#include "crab/level/level.h"

namespace Crab {

using namespace TMX;
using namespace pyrodactyl::stat;
using namespace pyrodactyl::anim;
using namespace pyrodactyl::level;
using namespace pyrodactyl::image;
using namespace pyrodactyl::people;
using namespace pyrodactyl::input;
using namespace pyrodactyl::event;
using namespace pyrodactyl::music;

//------------------------------------------------------------------------
// Purpose: See if player clicked on a sprite they are colliding with
//------------------------------------------------------------------------
bool Level::containsClick(const Common::String &id, const Common::Event &event) {
	// If mouse is moved and is hovering on the specified sprite (id), set hover = true
	if (event.type == Common::EVENT_MOUSEMOVE) {
		for (auto &i : _objects) {
			Rect r = i.posRect();

			if (r.Contains(g_engine->_mouse->_motion.x + _camera.x, g_engine->_mouse->_motion.y + _camera.y)) {
				// This is to show the sprite's name on top of their head
				i._hover = true;

				// This is to update the mouse cursor only if we're in talking range of a sprite
				if (i.id() == id)
					g_engine->_mouse->_hover = true;
			} else
				i._hover = false;
		}

		return false;
	} else if (g_engine->_mouse->pressed()) {
		for (auto &i : _objects) {
			if (i.id() == id) {
				Rect r = i.posRect();
				if (r.Contains(g_engine->_mouse->_button.x + _camera.x, g_engine->_mouse->_button.y + _camera.y)) {
					g_engine->_mouse->_hover = true;
					return true;
				}
			}
		}
	}

	return false;
}

#if 0
//------------------------------------------------------------------------
// Purpose: See if player clicked on a sprite they are colliding with
//------------------------------------------------------------------------
bool Level::ContainsClick(const Common::String &id, const SDL_Event &Event) {
	// If mouse is moved and is hovering on the specified sprite (id), set hover = true
	if (Event.type == SDL_MOUSEMOTION) {
		for (auto &i : objects) {
			Rect r = i.posRect();

			if (r.Contains(g_engine->_mouse->motion.x + camera.x, g_engine->_mouse->motion.y + camera.y)) {
				// This is to show the sprite's name on top of their head
				i.hover = true;

				// This is to update the mouse cursor only if we're in talking range of a sprite
				if (i.ID() == id)
					g_engine->_mouse->hover = true;
			} else
				i.hover = false;
		}

		return false;
	} else if (g_engine->_mouse->Pressed()) {
		for (auto &i : objects) {
			if (i.ID() == id) {
				Rect r = i.posRect();
				if (r.Contains(g_engine->_mouse->button.x + camera.x, g_engine->_mouse->button.y + camera.y)) {
					g_engine->_mouse->hover = true;
					return true;
				}
			}
		}
	}

	return false;
}
#endif
//------------------------------------------------------------------------
// Purpose: Find if a layer is visible (used only for objects with layers associated with them)
//------------------------------------------------------------------------
bool Level::layerVisible(pyrodactyl::anim::Sprite *obj) {
	if (obj->_layer < 0)
		return true;

	if ((unsigned int)obj->_layer < _terrain._layer.size())
		return _terrain._layer[obj->_layer]._collide;

	return false;
}

//------------------------------------------------------------------------
// Purpose: Check if a sprite is colliding with the trigger areas
// Common::String &id is set to the id of colliding object
//------------------------------------------------------------------------
void Level::calcTrigCollide(pyrodactyl::event::Info &info) {
	for (auto i = _objects.begin(); i != _objects.end(); ++i)
		if (info.personValid(i->id()))
			_terrain.collideWithTrigger(i->boundRect(), info.personGet(i->id())._trig);
}

//------------------------------------------------------------------------
// Purpose: Check if a sprite is colliding with interactive objects
// Common::String &id is set to the id of colliding object
//------------------------------------------------------------------------
bool Level::collidingWithObject(pyrodactyl::event::Info &info, Common::String &id) {
	// Clip and Bounding rectangle of player
	Rect pPos = _objects[_playerIndex].posRect(), pBound = _objects[_playerIndex].boundRect();

	unsigned int index = 0;
	for (auto i = _objects.begin(); i != _objects.end(); ++i, ++index) {
		if (i->visible() && _playerIndex != index && info.state(i->id()) == PST_NORMAL) {
			// Clip and bounding rectangles for the NPC sprite
			Rect iPos = i->posRect(), iBound = i->boundRect();

			if (pPos.Collide(iPos) || iPos.Contains(pPos) || pBound.Collide(iBound) || iBound.Contains(pBound)) {
				id = i->id();
				return true;
			}
		}
	}

	return false;
}

//------------------------------------------------------------------------
// Purpose: Check if a sprite is colliding with non-interactive stuff in the level
// id is set to the id of colliding object
//------------------------------------------------------------------------
bool Level::collidingWithLevel(pyrodactyl::event::Info &info, pyrodactyl::anim::Sprite &s) {
	_terrain.collideWithNoWalk(s.boundRect(), s._collideData);

	_terrain.collideWithStairs(s.boundRect(), s._velMod);

	if (_terrain.collideWithMusic(s.boundRect(), _music)) {
		if (_music._track)
			g_engine->_musicManager->playMusic(_music._id);
		else
			g_engine->_musicManager->playEffect(_music._id, _music._loops);
	}

	// If we are colliding with something, return true
	return !s._collideData.empty();
}

} // End of namespace Crab
