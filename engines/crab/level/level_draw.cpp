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

#include "graphics/managed_surface.h"
#include "crab/level/level.h"

namespace Crab {

using namespace TMX;
using namespace pyrodactyl::stat;
using namespace pyrodactyl::anim;
using namespace pyrodactyl::level;
using namespace pyrodactyl::image;
using namespace pyrodactyl::people;
using namespace pyrodactyl::input;

// Purpose: Pre render the terrain layer
void Level::preDraw() {
	Graphics::ManagedSurface *surf = new Graphics::ManagedSurface(_terrain.w(), _terrain.h(), *g_engine->_format);
	uint layerCount = 0u;
	for (auto l = _terrain._layer.begin(); l != _terrain._layer.end(); ++l, ++layerCount) {
		g_engine->_imageManager->_tileset.preDraw(*l, _terrain._tileSize, surf);

		// Draw the normal sprites if this is the layer for it
		if (layerCount == _terrain._spriteLayer)
			preDrawObjects(surf);
	}

	// This is to ensure we don't miss out on drawing sprites
	if (layerCount <= _terrain._spriteLayer)
		preDrawObjects(surf);

	_img.load(surf);
	delete surf;
}

//------------------------------------------------------------------------
// Purpose: Draw the level
//------------------------------------------------------------------------
void Level::draw(pyrodactyl::event::Info &info) {
	setCamera();
	sortObjectsToDraw();

	// Background sprites are assumed to be sorted by their layer count
	auto b = _background.begin();

	uint layerCount = 0u;

	// Draw the terrain layer
	g_engine->_imageManager->_tileset.draw(_terrain._layer[0], _camera, _terrain._tileSize, _objects[_playerIndex].posRect(), _img);

	Vector2i pos = _objects[_playerIndex]._aiData._dest;
	Rect newpos(pos.x - (_destMarker._size.x / 2), pos.y - (_destMarker._size.y / 2), _destMarker._size.x, _destMarker._size.y);

	for (auto l = _terrain._layer.begin(); l != _terrain._layer.end(); ++l, ++layerCount) {

		// Draw the background sprites
		for (; b != _background.end(); ++b) {
			if ((uint)b->_layer > layerCount) // We don't have any sprites to draw at this layer
				break;
			else if ((uint)b->_layer == layerCount && b->visible())
				b->draw(info, _camera);
		}

		// Draw the normal sprites if this is the layer for it
		if (layerCount == _terrain._spriteLayer)
			drawObjects(info);

		// Draw the terrain layer
		g_engine->_imageManager->_tileset.forceDraw(*l, _camera, _terrain._tileSize, _objects[_playerIndex].posRect());
	}

	// Draw the background sprites
	for (; b != _background.end(); ++b) {
		if ((uint)b->_layer > layerCount) // We don't have any sprites to draw at this layer
			break;
		else if ((uint)b->_layer >= layerCount && b->visible())
			b->draw(info, _camera);
	}

	// This is to ensure we don't miss out on drawing sprites
	if (layerCount <= _terrain._spriteLayer)
		drawObjects(info);

	// Fliers are drawn above every sprite but below popup text
	for (auto &i : _fly) {
		// Only draw if it is supposed to be flying
		if (i._aiData._walk._enabled)
			i.draw(info, _camera);
	}

	// Draw popup text over all level layers
	for (auto &i : _objects) {
		// Only draw popups for visible sprites
		if (i.visible()) {
			if (i.popupShow())
				i.drawPopup(_pop, _camera);
			else {
				// Only draw their name if they are
				//(a) hovered over by the mouse, OR
				//(b) are in talk range and don't have popup text over their head
				if (i._hover || (info.lastPerson() == i.id() && !i.popupShow()))
					_talkNotify.draw(info, i, _camera);
			}
		}
	}

	if (g_engine->_debugDraw & DRAW_TMX)
		_terrain.drawDebug(_camera);
}

//------------------------------------------------------------------------
// Purpose: Draw sprites and prop layers in sequences according to depth
//------------------------------------------------------------------------
void Level::drawObjects(pyrodactyl::event::Info &info) {
	// Draw player destination marker
	if (_objects[_playerIndex]._aiData._dest._active)
		_destMarker.draw(_objects[_playerIndex]._aiData._dest, _camera);

	Vector2i pos = _objects[_playerIndex]._aiData._dest;
	Rect newpos(pos.x - (_destMarker._size.x / 2), pos.y - (_destMarker._size.y / 2), _destMarker._size.x, _destMarker._size.y);

	if (_terrain._prop.empty()) {
		for (auto &entry : _objSeq) {
			if (entry.second->visible() && layerVisible(entry.second))
				entry.second->draw(info, _camera);
		}
	} else {
		auto a = _terrain._prop.begin();
		auto b = _objSeq.begin();

		while (a != _terrain._prop.end() && b != _objSeq.end()) {
			auto obj = b->second;
			if (a->_pos.y + a->_pos.h < obj->y() + obj->h()) {
				for (auto &i : a->_boundRect) {
					// draw prop bounds if requested
					if (g_engine->_debugDraw & DRAW_PROP_BOUNDS)
						i.draw(-_camera.x, -_camera.y, 128, 128, 0, 255);

					if (i.collide(_objects[_playerIndex].posRect())) {
						g_engine->_imageManager->_tileset.forceDraw(*a, _camera, _terrain._tileSize, _objects[_playerIndex].posRect());
					}

					if (i.contains(_objects[_playerIndex]._aiData._dest)) {
						g_engine->_imageManager->_tileset.forceDraw(*a, _camera, _terrain._tileSize, newpos);
					}
				}
				++a;
			} else {
				if (obj->visible() && layerVisible(obj))
					obj->draw(info, _camera);
				++b;
			}
		}

		if (a == _terrain._prop.end()) {
			for (; b != _objSeq.end(); ++b) {
				auto obj = b->second;
				if (obj->visible() && layerVisible(obj))
					obj->draw(info, _camera);
			}
		} else if (b == _objSeq.end()) {
			for (; a != _terrain._prop.end(); ++a) {
				for (auto &i : a->_boundRect) {
					// draw prop bounds if requested
					if (g_engine->_debugDraw & DRAW_PROP_BOUNDS)
						i.draw(-_camera.x, -_camera.y, 128, 128, 0, 255);

					if (i.collide(_objects[_playerIndex].posRect())) {
						g_engine->_imageManager->_tileset.forceDraw(*a, _camera, _terrain._tileSize, _objects[_playerIndex].posRect());
					}

					if (i.contains(_objects[_playerIndex]._aiData._dest)) {
						g_engine->_imageManager->_tileset.forceDraw(*a, _camera, _terrain._tileSize, newpos);
					}
				}
			}
		}
	}
}

void Level::preDrawObjects(Graphics::ManagedSurface *surf) {
	if (_terrain._prop.empty()) {
		return;
	} else {
		auto a = _terrain._prop.begin();
		auto b = _objSeq.begin();

		while (a != _terrain._prop.end() && b != _objSeq.end()) {
			auto obj = b->second;
			if (a->_pos.y + a->_pos.h < obj->y() + obj->h()) {
				g_engine->_imageManager->_tileset.preDraw(*a, _terrain._tileSize, surf);
				++a;
			} else {
				++b;
			}
		}

		if (a == _terrain._prop.end()) {
		} else if (b == _objSeq.end()) {
			for (; a != _terrain._prop.end(); ++a)
				g_engine->_imageManager->_tileset.preDraw(*a, _terrain._tileSize, surf);
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Sort objects to draw them according to their Y coordinates
//------------------------------------------------------------------------
bool Level::operator()(int i, int j) {
	return _objects[i].y() + _objects[i].h() < _objects[j].y() + _objects[j].h();
}

void Level::sortObjectsToDraw() {
	// add each object to the map to sort it
	_objSeq.clear();
	for (auto &object : _objects)
		_objSeq.insert(Common::Pair<int, Sprite *>(object.y() + object.h(), &object));
}

//------------------------------------------------------------------------
// Purpose: Center the camera on the player for scrolling levels
//------------------------------------------------------------------------
void Level::setCamera() {
	// Use the focus points of sprites
	Vector2i focus = _objects[_playerIndex].camFocus();

	// Center the camera over the player
	_camera.x = focus.x - (g_engine->_screenSettings->_cur.w / 2);
	_camera.y = focus.y - (g_engine->_screenSettings->_cur.h / 2);

	// Keep the camera in bounds
	if (_camera.x > _terrain.w() - _camera.w)
		_camera.x = _terrain.w() - _camera.w;
	if (_camera.y > _terrain.h() - _camera.h)
		_camera.y = _terrain.h() - _camera.h;
	if (_camera.x < 0)
		_camera.x = 0;
	if (_camera.y < 0)
		_camera.y = 0;
}

} // End of namespace Crab
