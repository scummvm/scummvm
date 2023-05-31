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
#include "crab/common_header.h"

namespace Crab {

namespace TMX {
struct TileSet {
	// The name of the tileset
	Common::String name;

	// The location of the tileset image on the disk
	Common::String loc;

	// The first gid of the tileset
	GidFormat first_gid;

	// Dimensions of tiles
	int tile_w, tile_h;

	// Number of rows and columns of tiles
	int total_rows, total_cols;

	// The image used by the tileset
	pyrodactyl::image::Image img;

	// Stuff used to store temporary data

	// The rectangle used to store clip info
	Rect clip;

	void Init() {
		first_gid = 1;
		tile_w = 1;
		tile_h = 1;
		total_rows = 1;
		total_cols = 1;
	}

	TileSet() { Init(); }
	TileSet(const Common::String &path, rapidxml::xml_node<char> *node) {
		Init();
		Load(path, node);
	}

	void Load(const Common::String &path, rapidxml::xml_node<char> *node);
	void Draw(const Vector2i &pos, const TileInfo &tile);
	void PreDraw(const Vector2i &pos, const TileInfo &tile, Graphics::ManagedSurface *surf);
};

class TileSetGroup {
	Common::Array<TileSet> tileset;

	// The latest tile position
	Vector2i v;

	// The area that we have to draw
	Vector2i start, finish;

public:
	TileSetGroup() {}

	void Reset();

	void Load(const Common::String &path, rapidxml::xml_node<char> *node);
	void Draw(MapLayer &layer, const Rect &camera, const Vector2i &tile_size, const Rect &player_pos, pyrodactyl::image::Image &img);
	void PreDraw(MapLayer &layer, const Vector2i &tile_size, Graphics::ManagedSurface *surf);
	void ForceDraw(MapLayer &layer, const Rect &camera, const Vector2i &tile_size, const Rect &player_pos);
};
} // End of namespace TMX

} // End of namespace Crab

#endif // CRAB_TMXTILESET_H
