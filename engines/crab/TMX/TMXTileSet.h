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

#ifndef CRAB_TMXTILESET_H
#define CRAB_TMXTILESET_H

#include "crab/image/Image.h"
#include "crab/TMX/TMXLayer.h"

namespace Crab {

namespace TMX {
struct TileSet {
	// The name of the tileset
	Common::String _name;

	// The location of the tileset image on the disk
	Common::String _loc;

	// The first gid of the tileset
	GidFormat _firstGid;

	// Dimensions of tiles
	int _tileW, _tileH;

	// Number of rows and columns of tiles
	int _totalRows, _totalCols;

	// The image used by the tileset
	pyrodactyl::image::Image _img;

	// Stuff used to store temporary data

	// The rectangle used to store clip info
	Rect _clip;

	void init() {
		_firstGid = 1;
		_tileW = 1;
		_tileH = 1;
		_totalRows = 1;
		_totalCols = 1;
	}

	TileSet() {
		init();
	}

	TileSet(const Common::String &path, rapidxml::xml_node<char> *node) {
		init();
		load(path, node);
	}

	void load(const Common::String &path, rapidxml::xml_node<char> *node);
	void draw(const Vector2i &pos, const TileInfo &tile);
	void preDraw(const Vector2i &pos, const TileInfo &tile, Graphics::ManagedSurface *surf);
};

class TileSetGroup {
	Common::Array<TileSet> _tileset;

	// The latest tile position
	Vector2i _v;

	// The area that we have to draw
	Vector2i _start, _finish;

public:
	TileSetGroup() {}

	void reset();

	void load(const Common::String &path, rapidxml::xml_node<char> *node);
	void draw(MapLayer &layer, const Rect &camera, const Vector2i &tileSize, const Rect &playerPos, pyrodactyl::image::Image &img);
	void preDraw(MapLayer &layer, const Vector2i &tileSize, Graphics::ManagedSurface *surf);
	void forceDraw(MapLayer &layer, const Rect &camera, const Vector2i &tileSize, const Rect &playerPos);
};
} // End of namespace TMX

} // End of namespace Crab

#endif // CRAB_TMXTILESET_H
