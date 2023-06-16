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

//------------------------------------------------------------------------
// Purpose: We re-use the same level object each time
// this function cleans up everything to make it good as new (get it)
//------------------------------------------------------------------------
void Level::Reset() {
	if (player_index > 0 && objects.size() > player_index)
		objects[player_index].pathing.shutdown();

	player_index = 0;
	terrain.Reset();

	objects.clear();
	obj_seq.clear();
	background.clear();
	fly.clear();

	Camera(0, 0, g_engine->_screenSettings->cur.w, g_engine->_screenSettings->cur.h);

	showmap.Set(true);
	inside_exit = false;
	first_hit = true;
	music.id = -1;
	preview_path.clear();

	img.Delete();
}

//------------------------------------------------------------------------
// Purpose: Get index of a sprite in the object array
//------------------------------------------------------------------------
pyrodactyl::anim::Sprite *Level::GetSprite(const Common::String &id) {
	int count = 0;
	for (auto i = objects.begin(); i != objects.end(); ++i, ++count)
		if (i->ID() == id)
			return &objects[count];

	return NULL;
}

//------------------------------------------------------------------------
// Purpose: Figure visibility and let the AI see what moves are allowed
//------------------------------------------------------------------------
void Level::CalcProperties(Info &info) {
	for (auto i = objects.begin(); i != objects.end(); ++i)
		i->CalcProperties(info);
}

//------------------------------------------------------------------------
// Purpose: Handle the level movement
//------------------------------------------------------------------------
void Level::HandleEvents(Info &info, const Common::Event &Event) {
	if (!g_engine->_mouse->inside_hud) {
		if (Event.type == Common::EVENT_LBUTTONDOWN || Event.type == Common::EVENT_RBUTTONDOWN) {
			// Only go to path if our mouse is a normal walking cursor
			if (!g_engine->_mouse->hover) {
				// The destination coordinate is set by adding camera position to click position
				Vector2i dest = g_engine->_mouse->button;
				dest.x += camera.x;
				dest.y += camera.y;

				Rect b = objects[player_index].BoundRect();
				b.w /= 2;
				b.h /= 2;
				b.x = dest.x - b.w / 2;
				b.y = dest.y - b.h / 2;

				objects[player_index].SetDestPathfinding(dest, !terrain.InsideNoWalk(dest));
			}
		}
	}

	objects[player_index].HandleEvents(info, camera, sc_default, Event);
}

#if 0
//------------------------------------------------------------------------
// Purpose: Handle the level movement
//------------------------------------------------------------------------
void Level::HandleEvents(Info &info, const SDL_Event &Event) {
	if (!g_engine->_mouse->inside_hud) {
		if (Event.type == SDL_MOUSEBUTTONDOWN) {
			// Only go to path if our mouse is a normal walking cursor
			if (!g_engine->_mouse->hover) {
				// The destination coordinate is set by adding camera position to click position
				Vector2i dest = g_engine->_mouse->button;
				dest.x += camera.x;
				dest.y += camera.y;

				Rect b = objects[player_index].BoundRect();
				b.w /= 2;
				b.h /= 2;
				b.x = dest.x - b.w / 2;
				b.y = dest.y - b.h / 2;

				objects[player_index].SetDestPathfinding(dest, !terrain.InsideNoWalk(dest));
			}
		}
	}

	objects[player_index].HandleEvents(info, camera, sc_default, Event);
}
#endif

//------------------------------------------------------------------------
// Purpose: Swap the player sprites
//------------------------------------------------------------------------
void Level::PlayerID(const Common::String &ID, const int &X, const int &Y) {
	int index = 0;
	for (auto i = objects.begin(); i != objects.end(); ++i, ++index) {
		if (i->ID() == ID) {
			player_index = index;

			if (X != -1)
				i->X(X);
			if (Y != -1)
				i->Y(Y);

			break;
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Rearrange stuff after resolution change
//------------------------------------------------------------------------
void Level::SetUI() {
	camera.w = g_engine->_screenSettings->cur.w;
	camera.h = g_engine->_screenSettings->cur.h;
	SetCamera();
}

} // End of namespace Crab
