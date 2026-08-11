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

// Common/system includes basic things like Array
#include "common/system.h"

// Story is needed by both immortal.h and room.h
#include "immortal/story.h"

// Utilities.h contains many things used by all objects, not just immortal
#include "immortal/utilities.h"
#include "immortal/immortal.h"

#ifndef IMMORTAL_ROOM_H
#define IMMORTAL_ROOM_H

namespace Immortal {

enum RoomTile : uint8 {
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
	kTileCeilingTile = 2 // This duplicate is intentional
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
	FPattern _p = kFlameOff;
	uint8 _x = 0;
	uint8 _y = 0;
	int _c = 0;
};

struct Chest {
};

struct Bullet {
};

class Room {
private:
	Common::RandomSource _randomSource;

public:
	Room(uint8 x, uint8 y, RoomFlag f);
	~Room() {}

	/*
	 * --- Data ---
	 *
	 */

	// Constants
	const uint8 kLightTorchX  = 10;
	const uint8 kMaxFlameCycs = 16;

	Common::Array<SCycle>  _cycPtrs;
	Common::Array<Flame>   _fset;
	Common::Array<Monster> _monsters;
	Common::Array<Object>  _objects;

	RoomFlag _flags;

	uint8 _xPos      = 0;
	uint8 _yPos      = 0;
	uint8 _holeRoom  = 0;
	uint8 _holeCellX = 0;
	uint8 _holeCellY = 0;
	uint8 _candleTmp = 0;                           // Special case for candle in maze 0
	uint8 _numFlames = 0;
	uint8 _numInRoom = 0;

	/*
	 * --- Methods ---
	 *
	 */

	uint32 getRandomNumber(uint maxNum) {
		return _randomSource.getRandomNumber(maxNum);
	}

	/*
	 * [room.cpp] Functions from Room.GS
	 */

	//void init();
	//void inRoomNew();
	//void getTilePair(uint8 x, uint8 y);           // Modifies a struct of the tile number, aboveTile number, and the cell coordinates of the tile

	void setHole();
	void drawContents(uint16 vX, uint16 vY);
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
	 * [Cycle.cpp] Functions from Cyc
	 */

	// Init
	int cycleNew(CycID id);                        // Adds a cycle to the current list
	void cycleFree(int c);

	// Getters
	DataSprite *cycleGetDataSprite(int c);          // This takes the place of getFile + getNum
	int cycleGetIndex(int c);
	int cycleGetFrame(int c);
	int cycleGetNumFrames(int c);

	// Setters
	void cycleSetIndex(int c, int f);

	// Misc
	bool cycleAdvance(int c);
	CycID getCycList(int c);

	/* Unnecessary cycle functions
	void cycleInit();
	void cycleFree();
	void cycleGetNumFrames();
	void cycleGetList();*/

	/*
	 * [flameSet.cpp] Functions from flameSet.GS
	 */

	//void flameNew() does not need to exist, because we create the duplicate SFlame in Level, and the array in immortal.h is not accessible from here
	void flameInit();
	void flameDrawAll(uint16 vX, uint16 vY);
	bool roomLighted();
	void lightTorch(uint8 x, uint8 y);
	void flameFreeAll();
	void flameSetRoom(Common::Array<SFlame> &allFlames);
	int flameGetCyc(Flame *f, int first);

	/*
	 * [bullet.cpp] Functions from Bullet.GS
	 */



	/*
	 * [object.cpp] Functions from Object.GS
	 */


	/*
	 * [Univ.cpp] Functions from Univ.GS
	 */

	void univAddSprite(uint16 vX, uint16 vY, uint16 x, uint16 y, SpriteName s, int img, uint16 p);
};

} // namespace Immortal

#endif
