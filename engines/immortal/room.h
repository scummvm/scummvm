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

/* --- What is a Room ---
 *
 */

#include "common/file.h"
#include "common/memstream.h"
#include "common/debug.h"
#include "common/error.h"
#include "immortal/story.h"

#ifndef IMMORTAL_ROOM_H
#define IMMORTAL_ROOM_H

namespace Immortal {

enum Tile : uint8 {
	kTileFloor,
	kTileUpper5,
	kTileUpper3,
	kTileCeiling,
	kTileTop1,
	kTileTop7,
	kTileWallFace,
	kTileTopLower13,
	kTileTopLower75,
	kTileLower3,
	kTileLower5,
	kTileCeilingTile = 2
};

struct Flame {
};

// Temp
struct Object {
};

// Temp
struct Monster {
};

struct Spark {
};

struct Chest {
};

struct Bullet {
};

class Room {
private:

public:
	Room(uint8 x, uint8 y, RoomFlag f);
	~Room() {}

Common::Array<Flame>   _fset;
Common::Array<Monster> _monsters;
Common::Array<Object>  _objects;

   RoomFlag  _flags;
	  uint8  _xPos;
	  uint8  _yPos;
	  uint8  _holeRoom;
	  uint8  _holeCellX;
	  uint8  _holeCellY;

	//void init();
	//void inRoomNew();
	//void getTilePair(uint8 x, uint8 y);			// Modifies a struct of the tile number, aboveTile number, and the cell coordinates of the tile

	void setHole();
	void drawContents();
	bool getTilePair(uint8 x, uint8 y, int id);
	bool getWideWallNormal(uint8 x, uint8 y, uint8 xPrev, uint8 yPrev, int id, int spacing);
	bool getWallNormal(uint8 x, uint8 y, uint8 xPrev, uint8 yPrev, int id);
	void addMonster();
	void addObject();
	void removeObject();
	void removeMonster();

Common::Array<Monster> getMonsterList();
Common::Array<Object> getObjectList();

	void getXY(uint16 &x, uint16 &y);
	void getCell(uint16 &x, uint16 &y);
};



} // namespace immortal

#endif















