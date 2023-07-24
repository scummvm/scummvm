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
void Level::reset() {
	if (_playerIndex > 0 && _objects.size() > _playerIndex)
		_objects[_playerIndex]._pathing.shutdown();

	_playerIndex = 0;
	_terrain.reset();

	_objects.clear();
	_objSeq.clear();
	_background.clear();
	_fly.clear();

	Camera(0, 0, g_engine->_screenSettings->_cur.w, g_engine->_screenSettings->_cur.h);

	_showmap.set(true);
	_insideExit = false;
	_firstHit = true;
	_music._id = -1;
	_previewPath.clear();

	_img.deleteImage();
}

//------------------------------------------------------------------------
// Purpose: Get index of a sprite in the object array
//------------------------------------------------------------------------
pyrodactyl::anim::Sprite *Level::getSprite(const Common::String &id) {
	int count = 0;
	for (auto i = _objects.begin(); i != _objects.end(); ++i, ++count)
		if (i->id() == id)
			return &_objects[count];

	return NULL;
}

//------------------------------------------------------------------------
// Purpose: Figure visibility and let the AI see what moves are allowed
//------------------------------------------------------------------------
void Level::calcProperties(pyrodactyl::event::Info &info) {
	for (auto i = _objects.begin(); i != _objects.end(); ++i)
		i->calcProperties(info);
}

//------------------------------------------------------------------------
// Purpose: Handle the level movement
//------------------------------------------------------------------------
void Level::handleEvents(Info &info, const Common::Event &event) {
	if (!g_engine->_mouse->_insideHud) {
		if (event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_RBUTTONDOWN) {
			// Only go to path if our mouse is a normal walking cursor
			if (!g_engine->_mouse->_hover) {
				// The destination coordinate is set by adding camera position to click position
				Vector2i dest = g_engine->_mouse->_button;
				dest.x += _camera.x;
				dest.y += _camera.y;

				Rect b = _objects[_playerIndex].boundRect();
				b.w /= 2;
				b.h /= 2;
				b.x = dest.x - b.w / 2;
				b.y = dest.y - b.h / 2;

				_objects[_playerIndex].setDestPathfinding(dest, !_terrain.insideNoWalk(dest));
			}
		}
	}

	_objects[_playerIndex].handleEvents(info, _camera, _scDefault, event);
}

//------------------------------------------------------------------------
// Purpose: Swap the player sprites
//------------------------------------------------------------------------
void Level::playerId(const Common::String &ID, const int &x, const int &y) {
	int index = 0;
	for (auto i = _objects.begin(); i != _objects.end(); ++i, ++index) {
		if (i->id() == ID) {
			_playerIndex = index;

			if (x != -1)
				i->x(x);
			if (y != -1)
				i->y(y);

			break;
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Rearrange stuff after resolution change
//------------------------------------------------------------------------
void Level::setUI() {
	_camera.w = g_engine->_screenSettings->_cur.w;
	_camera.h = g_engine->_screenSettings->_cur.h;
	setCamera();
}

} // End of namespace Crab
