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

#ifndef CRAB_TMXMAP_H
#define CRAB_TMXMAP_H

#include "ImageManager.h"
#include "LevelExit.h"
#include "LevelResult.h"
#include "MusicArea.h"
#include "PathfindingGrid.h"
#include "Shape.h"
#include "Stairs.h"
#include "TMXTileSet.h"
#include "common_header.h"
#include "spriteai.h"

namespace TMX {
// For TMX version 1.0, orthogonal maps only
class TMXMap {
protected:
	// The actual dimensions of the level
	int w, h;

	// The area you can move in
	Rect area_walk;

	// The non-walk able areas in the level
	std::vector<Shape> area_nowalk;

	// The trigger rectangles in the level
	std::vector<Shape> area_trig;

	// Stairs modify the player walking speed
	std::vector<pyrodactyl::level::Stairs> area_stairs;

	// Music areas change the music if player collides with them
	std::vector<pyrodactyl::level::MusicArea> area_music;

	// Archived methods for loading poly lines in tiled
	// void LoadPath(rapidxml::xml_node<char> *node);
	// const Vector2i GetPoint(const Vector2i &ref, std::string &x, std::string &y);

public:
	// The exits to different levels
	std::vector<pyrodactyl::level::Exit> area_exit;

	// The layer on top of which objects walk
	unsigned int sprite_layer;

	// Dimensions of the level in terms of tiles
	int tile_rows, tile_cols;

	// Dimensions of the level in terms of pathfinding grid cells (SZ)
	int path_rows, path_cols;

	// The width and height of tiles
	Vector2i tile_size;

	// The width and height of pathfinding grid cells (SZ)
	Vector2i path_size;

	// The layers of tiles in the level
	std::vector<MapLayer> layer;

	// The props in the level
	std::vector<MapLayer> prop;

	PathfindingGrid *grid; // The grid of graph nodes used for navigating.

	// Movement costs
	struct {
		int open, no_walk, stairs;
	} movementCosts;

	TMXMap();
	~TMXMap() {}

	void Reset();
	void Load(const std::string &path, std::string filename);

	void DrawDebug(const Rect &camera);

	bool InsideWalk(const Rect bounding_box);
	bool InsideWalk(const Vector2i &pos);

	bool InsideNoWalk(const Vector2i &pos);
	void CollideWithNoWalk(const Rect bounding_box, std::list<CollisionData> &colliders);

	bool CollideWithExit(const Rect rect, LevelResult &res);
	bool CollideWithStairs(const Rect rect, Vector2f &vel_mod);
	bool CollideWithMusic(const Rect rect, pyrodactyl::level::MusicInfo &music);

	bool CollideWithTrigger(const Rect rect, int index);
	void CollideWithTrigger(const Rect rect, std::vector<int> &collision_table);

	int W() { return w; }
	int H() { return h; }
	Rect AreaWalk() { return area_walk; }
	std::vector<Shape> AreaNoWalk() { return area_nowalk; }
	std::vector<pyrodactyl::level::Stairs> AreaStairs() { return area_stairs; }
};
} // End of namespace TMX

#endif // CRAB_TMXMAP_H
