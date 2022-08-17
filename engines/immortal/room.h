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

// Common is needed by immortal.h, room.h, and monster.h
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/util.h"
#include "common/platform.h"

// There is a lot of bit masking that needs to happen, so this header includes several enums for immortal.h, room.h, and monster.h
#include "immortal/bitmask.h"

#include "immortal/util.h"

// Story is needed by both immortal.h and room.h
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

/* Quick note:
 * This looks entirely redundant and silly, I agree. However
 * this is because the source does more or less the same thing.
 * At compile time, it creates and stores in memory what are the
 * equivalent of structs (or maybe tuples), and then at run time
 * when creating a room, it makes room specific versions that can
 * be changed. So essentially it creates two RAM structs and then
 * treats the first as ROM. As such, that's what I'm doing here.
 * The 'Story' structs are ROM, the 'Room' structs are RAM. There
 * are also slight differences, like how the room Flame has a reference
 * to the Cyc it is using. Although again the Story ones are ram
 * and could do this too.
 */

// Temp
struct Object {
};

// Temp
struct Monster {
};

struct Flame {
FPattern _p;
	uint8 _x;
	uint8 _y;
	Cyc   _c;
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

	/*
	 * --- Data ---
	 *
	 */

	// Constants
	const uint8 kLightTorchX = 10;

Common::Array<Flame>   _fset;
Common::Array<Monster> _monsters;
Common::Array<Object>  _objects;

   RoomFlag _flags;
	   uint8 _xPos;
	   uint8 _yPos;
	   uint8 _holeRoom;
	   uint8 _holeCellX;
	   uint8 _holeCellY;
	   uint8 _candleTmp;									// Special case for candle in maze 0


	/*
	 * --- Methods ---
	 *
	 */

	/*
	 * [room.cpp] Functions from Room.GS
	 */

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


	/*
	 * [flameSet.cpp] Functions from flameSet.GS
	 */

	void flameSetRoom(Common::Array<SFlame>);
	void flameDrawAll();
	bool roomLighted();
	void lightTorch(int x, int y);
	 Cyc flameGetCyc(int first);
	//void flameFreeAll();
	//void flameSetRoom();

	/*
	 * [bullet.cpp] Functions from Bullet.GS
	 */



	/*
	 * [object.cpp] Functions from Object.GS
	 */


};



} // namespace immortal

#endif















