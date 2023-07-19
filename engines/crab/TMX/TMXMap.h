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

#include "crab/image/ImageManager.h"
#include "crab/level/LevelExit.h"
#include "crab/LevelResult.h"
#include "crab/level/MusicArea.h"
#include "crab/PathfindingGrid.h"
#include "crab/Shape.h"
#include "crab/level/Stairs.h"
#include "crab/TMX/TMXTileSet.h"
#include "crab/common_header.h"
#include "crab/ai/spriteai.h"

namespace Crab {

namespace TMX {
// For TMX version 1.0, orthogonal maps only
class TMXMap {
protected:
	// The actual dimensions of the level
	int _w, _h;

	// The area you can move in
	Rect _areaWalk;

	// The non-walk able areas in the level
	Common::Array<Shape> _areaNowalk;

	// The trigger rectangles in the level
	Common::Array<Shape> _areaTrig;

	// Stairs modify the player walking speed
	Common::Array<pyrodactyl::level::Stairs> _areaStairs;

	// Music areas change the music if player collides with them
	Common::Array<pyrodactyl::level::MusicArea> _areaMusic;

	// Archived methods for loading poly lines in tiled
	// void LoadPath(rapidxml::xml_node<char> *node);
	// const Vector2i GetPoint(const Vector2i &ref, Common::String &x, Common::String &y);

public:
	// The exits to different levels
	Common::Array<pyrodactyl::level::Exit> _areaExit;

	// The layer on top of which objects walk
	unsigned int _spriteLayer;

	// Dimensions of the level in terms of tiles
	int _tileRows, _tileCols;

	// Dimensions of the level in terms of pathfinding grid cells (SZ)
	int _pathRows, _pathCols;

	// The width and height of tiles
	Vector2i _tileSize;

	// The width and height of pathfinding grid cells (SZ)
	Vector2i _pathSize;

	// The layers of tiles in the level
	Common::Array<MapLayer> _layer;

	// The props in the level
	Common::Array<MapLayer> _prop;

	PathfindingGrid *_grid; // The grid of graph nodes used for navigating.

	// Movement costs
	struct {
		int _open, _noWalk, _stairs;
	} _movementCosts;

	TMXMap();
	~TMXMap() {}

	void reset();
	void load(const Common::String &path, Common::String filename);

	void drawDebug(const Rect &camera);

	bool insideWalk(const Rect boundingBox);
	bool insideWalk(const Vector2i &pos);

	bool insideNoWalk(const Vector2i &pos);
	void collideWithNoWalk(const Rect boundingBox, Common::List<CollisionData> &colliders);

	bool collideWithExit(const Rect rect, LevelResult &res);
	bool collideWithStairs(const Rect rect, Vector2f &velMod);
	bool collideWithMusic(const Rect rect, pyrodactyl::level::MusicInfo &music);

	bool collideWithTrigger(const Rect rect, int index);
	void collideWithTrigger(const Rect rect, Common::Array<int> &collisionTable);

	int w() {
		return _w;
	}

	int h() {
		return _h;
	}

	Rect areaWalk() {
		return _areaWalk;
	}

	Common::Array<Shape> areaNoWalk() {
		return _areaNowalk;
	}

	Common::Array<pyrodactyl::level::Stairs> areaStairs() {
		return _areaStairs;
	}
};
} // End of namespace TMX

} // End of namespace Crab

#endif // CRAB_TMXMAP_H
