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

#pragma once

#include "Image.h"
#include "TMXLayer.h"
#include "common_header.h"

namespace TMX {
struct TileSet {
	// The name of the tileset
	std::string name;

	// The location of the tileset image on the disk
	std::string loc;

	// The first gid of the tileset
	int first_gid;

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
	TileSet(const std::string &path, rapidxml::xml_node<char> *node) {
		Init();
		Load(path, node);
	}

	void Load(const std::string &path, rapidxml::xml_node<char> *node);
	void Draw(const Vector2i &pos, const TileInfo &tile);
};

class TileSetGroup {
	std::vector<TileSet> tileset;

	// The latest tile position
	Vector2i v;

	// The area that we have to draw
	Vector2i start, finish;

public:
	TileSetGroup() {}

	void Reset();

	void Load(const std::string &path, rapidxml::xml_node<char> *node);
	void Draw(MapLayer &layer, const Rect &camera, const Vector2i &tile_size, const Rect &player_pos);
};
}; // End of namespace TMX
