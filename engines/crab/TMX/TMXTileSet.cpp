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

#include "graphics/screen.h"
#include "crab/crab.h"
#include "crab/text/TextManager.h"
#include "crab/TMX/TMXTileSet.h"

namespace Crab {

using namespace TMX;

void TileSet::load(const Common::String &path, rapidxml::xml_node<char> *node) {
	if (nodeValid(node)) {
		loadNum(_firstGid, "firstgid", node);
		loadStr(_name, "name", node);
		loadNum(_tileW, "tilewidth", node);
		loadNum(_tileH, "tileheight", node);

		_clip.w = _tileW;
		_clip.h = _tileH;

		if (nodeValid("image", node)) {
			rapidxml::xml_node<char> *imgnode = node->first_node("image");
			Common::String filename;
			loadStr(filename, "source", imgnode);
			_loc = path + filename;

			_img.load(_loc);
			_totalRows = _img.h() / _tileH;
			_totalCols = _img.w() / _tileW;
			debugC(kDebugGraphics, "Total rows : %d Total cols: %d gid: %d", _totalRows, _totalCols, _firstGid);

		}
	}

	// Prevent divide by zero errors later
	if (_totalCols == 0)
		_totalCols = 1;
}

void TileSetGroup::reset() {
	for (auto &i : _tileset)
		i._img.deleteImage();

	_tileset.clear();
}

void TileSetGroup::load(const Common::String &path, rapidxml::xml_node<char> *node) {
	reset();
	for (auto n = node->first_node("tileset"); n != nullptr; n = n->next_sibling("tileset")) {
		TileSet t;
		t.load(path, n);
		_tileset.push_back(t);
	}
}

void TileSet::draw(const Vector2i &pos, const TileInfo &tile) {
	if (tile._gid != 0) {
		_clip.x = ((tile._gid - _firstGid) % _totalCols) * _tileW;
		_clip.y = ((tile._gid - _firstGid) / _totalCols) * _tileH;

		_img.draw(pos.x, pos.y, &_clip, tile._flip);
	}
}

void TileSet::preDraw(const Vector2i &pos, const TileInfo &tile, Graphics::ManagedSurface *surf) {
	if (tile._gid != 0) {
		_clip.x = ((tile._gid - _firstGid) % _totalCols) * _tileW;
		_clip.y = ((tile._gid - _firstGid) / _totalCols) * _tileH;

		_img.draw(pos.x, pos.y, &_clip, tile._flip, surf);
	}
}

void TileSetGroup::preDraw(MapLayer &layer, const Vector2i &tileSize, Graphics::ManagedSurface *surf) {
	if (layer._type == LAYER_IMAGE || layer._type == LAYER_AUTOSHOW)
		return;

	_start.x = 0;
	_start.y = 0;

	_finish.x = layer._tile.size();
	_finish.y = layer._tile[0].size();

	_v.x = _start.y * tileSize.x;
	_v.y = _start.x * tileSize.y;

	for (int x = _start.x; x < _finish.x; ++x) {
			for (int y = _start.y; y < _finish.y; ++y) {
				for (int i = _tileset.size() - 1; i >= 0; --i)
					if (layer._tile[x][y]._gid >= _tileset[i]._firstGid) {
						_tileset[i].preDraw(_v, layer._tile[x][y], surf);
						layer._boundRect.push_back(Rect(_v.x, _v.y, tileSize.x, tileSize.y));
						break;
					}

				_v.x += tileSize.x;
			}

			_v.x = _start.y * tileSize.x;
			_v.y += tileSize.y;
	}

	Common::List<Rect>::iterator rOuter, rInner;

	// Process the bound rect list to find any rects to merge
	for (rOuter = layer._boundRect.begin(); rOuter != layer._boundRect.end(); ++rOuter) {
		rInner = rOuter;
		while (++rInner != layer._boundRect.end()) {
			if ((*rOuter).collide(*rInner)) {
					rOuter->extend(*rInner);
				layer._boundRect.erase(rInner);
				rInner = rOuter;
			}
		}
	}
}

void TileSetGroup::forceDraw(MapLayer &layer, const Rect &camera, const Vector2i &tileSize, const Rect &playerPos) {

	if (layer._type == LAYER_IMAGE)
		return;

	layer._collide = layer._pos.collide(playerPos);

	// Normal and prop layers are drawn this way
	// The row and column we start drawing at
	_start.x = playerPos.y / tileSize.y;
	_start.y = playerPos.x / tileSize.x;

	if (_start.x < 0 || _start.y < 0)
		return;

	// The row and column we end drawing at
	_finish.x = (playerPos.y + playerPos.h) / tileSize.y + 1;
	_finish.y = (playerPos.x + playerPos.w) / tileSize.x + 1;

	if (layer._type == LAYER_AUTOSHOW) {
		if (layer._collide)
			return;

		_start.x = camera.y / tileSize.y;
		_start.y = camera.x / tileSize.x;

		//The row and column we end drawing at
		_finish.x = (camera.y + camera.h) / tileSize.y + 1;
		_finish.y = (camera.x + camera.w) / tileSize.x + 1;
	}

	if (_finish.x > (int)layer._tile.size())
		_finish.x = layer._tile.size();
	if (_finish.y > (int)layer._tile[0].size())
		_finish.y = layer._tile[0].size();

	_v.x = _start.y * tileSize.x - camera.x;
	_v.y = _start.x * tileSize.y - camera.y;

	for (int x = _start.x; x < _finish.x; ++x) {
		for (int y = _start.y; y < _finish.y; ++y) {
			for (int i = _tileset.size() - 1; i >= 0; --i)
				if (layer._tile[x][y]._gid >= _tileset[i]._firstGid) {
						_tileset[i].draw(_v, layer._tile[x][y]);
					break;
				}

			_v.x += tileSize.x;
		}

		_v.x = _start.y * tileSize.x - camera.x;
		_v.y += tileSize.y;
	}
}

void TileSetGroup::draw(MapLayer &layer, const Rect &camera, const Vector2i &tileSize, const Rect &playerPos, pyrodactyl::image::Image &img) {
	if (layer._type == LAYER_IMAGE)
		layer._img.draw(-1.0f * camera.x * layer._rate.x, -1.0f * camera.y * layer._rate.y);
	else if (layer._type == LAYER_PARALLAX) {
		// The row and column we start drawing at
		_start.x = 0;
		_start.y = 0;

		// The row and column we end drawing at
		_finish.x = layer._tile.size() - 1;
		_finish.y = layer._tile[0].size() - 1;

		_v.x = (_start.y * tileSize.x - camera.x) * layer._rate.x;
		_v.y = (_start.x * tileSize.y - camera.y) * layer._rate.y;

		for (int x = _start.x; x < _finish.x; ++x) {
			for (int y = _start.y; y < _finish.y; ++y) {
				for (int i = _tileset.size() - 1; i >= 0; --i)
					if (layer._tile[x][y]._gid >= _tileset[i]._firstGid) {
						_tileset[i].draw(_v, layer._tile[x][y]);
						break;
					}

				_v.x += tileSize.x;
			}

			_v.x = (_start.y * tileSize.x - camera.x) * layer._rate.x;
			_v.y += tileSize.y;
		}
	} else {
		layer._collide = layer._pos.collide(playerPos);

		// If player is inside the layer bounds, draw normally - else skip drawing
		if (layer._type == LAYER_AUTOHIDE && !layer._collide)
			return;

		// If the player is outside the layer bounds, draw normally - else skip drawing
		if (layer._type == LAYER_AUTOSHOW && layer._collide)
			return;

		// Normal and prop layers are drawn this way

		// The row and column we start drawing at
		_start.x = camera.y / tileSize.y;
		_start.y = camera.x / tileSize.x;

		// The row and column we end drawing at
		_finish.x = (camera.y + camera.h) / tileSize.y + 1;
		_finish.y = (camera.x + camera.w) / tileSize.x + 1;

		if (_finish.x > (int)layer._tile.size())
			_finish.x = layer._tile.size();
		if (_finish.y > (int)layer._tile[0].size())
			_finish.y = layer._tile[0].size();

		_v.x = camera.x;
		_v.y = camera.y;

		Vector2i end;

		end.x = camera.x + g_engine->_screen->w;
		end.y = camera.y + g_engine->_screen->h;

		Rect clip(_v.x, _v.y, end.x - _v.x, end.y - _v.y);
		img.fastDraw(0, 0, &clip);
	}
}

} // End of namespace Crab
