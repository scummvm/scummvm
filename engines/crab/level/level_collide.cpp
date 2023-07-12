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
bool Level::ContainsClick(const Common::String &id, const Common::Event &Event) {
	// If mouse is moved and is hovering on the specified sprite (id), set hover = true
	if (Event.type == Common::EVENT_MOUSEMOVE) {
		for (auto &i : objects) {
			Rect r = i.posRect();

			if (r.Contains(g_engine->_mouse->motion.x + camera.x, g_engine->_mouse->motion.y + camera.y)) {
				// This is to show the sprite's name on top of their head
				i._hover = true;

				// This is to update the mouse cursor only if we're in talking range of a sprite
				if (i.id() == id)
					g_engine->_mouse->hover = true;
			} else
				i._hover = false;
		}

		return false;
	} else if (g_engine->_mouse->Pressed()) {
		for (auto &i : objects) {
			if (i.id() == id) {
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
bool Level::LayerVisible(Sprite *obj) {
	if (obj->_layer < 0)
		return true;

	if ((unsigned int)obj->_layer < terrain.layer.size())
		return terrain.layer[obj->_layer].collide;

	return false;
}

//------------------------------------------------------------------------
// Purpose: Check if a sprite is colliding with the trigger areas
// Common::String &id is set to the id of colliding object
//------------------------------------------------------------------------
void Level::CalcTrigCollide(Info &info) {
	for (auto i = objects.begin(); i != objects.end(); ++i)
		if (info.personValid(i->id()))
			terrain.CollideWithTrigger(i->boundRect(), info.personGet(i->id()).trig);
}

//------------------------------------------------------------------------
// Purpose: Check if a sprite is colliding with interactive objects
// Common::String &id is set to the id of colliding object
//------------------------------------------------------------------------
bool Level::CollidingWithObject(Info &info, Common::String &id) {
	// Clip and Bounding rectangle of player
	Rect p_pos = objects[player_index].posRect(), p_bound = objects[player_index].boundRect();

	unsigned int index = 0;
	for (auto i = objects.begin(); i != objects.end(); ++i, ++index) {
		if (i->visible() && player_index != index && info.state(i->id()) == PST_NORMAL) {
			// Clip and bounding rectangles for the NPC sprite
			Rect i_pos = i->posRect(), i_bound = i->boundRect();

			if (p_pos.Collide(i_pos) || i_pos.Contains(p_pos) || p_bound.Collide(i_bound) || i_bound.Contains(p_bound)) {
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
bool Level::CollidingWithLevel(Info &info, Sprite &s) {
	terrain.CollideWithNoWalk(s.boundRect(), s._collideData);

	terrain.CollideWithStairs(s.boundRect(), s._velMod);

	if (terrain.CollideWithMusic(s.boundRect(), music)) {
		if (music.track)
			g_engine->_musicManager->PlayMusic(music.id);
		else
			g_engine->_musicManager->PlayEffect(music.id, music.loops);
	}

	// If we are colliding with something, return true
	return !s._collideData.empty();
}

} // End of namespace Crab
