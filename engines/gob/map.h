/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */
#ifndef GOB_MAP_H
#define GOB_MAP_H

#include "gob/util.h"
#include "gob/mult.h"

namespace Gob {

// The same numeric values are also used for the arrow keys.

class Map {
public:
	enum {
		kDirNW = 0x4700,
		kDirN  = 0x4800,
		kDirNE = 0x4900,
		kDirW  = 0x4b00,
		kDirE  = 0x4d00,
		kDirSW = 0x4f00,
		kDirS  = 0x5000,
		kDirSE = 0x5100
	};

#include "common/pack-start.h"	// START STRUCT PACKING

	struct Point {
		int16 x;
		int16 y;
		int16 field_2; // Gob2
	};

#define szMap_ItemPos 3

	struct ItemPos {
		int8 x;
		int8 y;
		int8 orient;		// ??
	};

#include "common/pack-end.h"	// END STRUCT PACKING

	int16 _mapWidth;
	int16 _mapHeight;
	int16 _screenWidth;
	int16 _tilesWidth;
	int16 _tilesHeight;
	int16 _passWidth;
	bool _bigTiles;

	int8 *_passMap; // [y * _mapWidth + x], getPass(x, y);
	int16 **_itemsMap;	// [y][x]
	int16 _wayPointsCount;
	Point *_wayPoints;
	int16 _nearestWayPoint;
	int16 _nearestDest;

	int16 _curGoblinX;
	int16 _curGoblinY;
	int16 _destX;
	int16 _destY;
	int8 _loadFromAvo;

	ItemPos _itemPoses[40];
	char _sourceFile[15];

	void placeItem(int16 x, int16 y, int16 id);

	int16 getDirection(int16 x0, int16 y0, int16 x1, int16 y1);
	int16 checkDirectPath(Mult::Mult_Object *obj, int16 x0, int16 y0, int16 x1, int16 y1);
	int16 checkLongPath(int16 x0, int16 y0, int16 x1, int16 y1, int16 i0, int16 i1);
	void loadItemToObject(void);
	void loadDataFromAvo(char *dest, int16 size);
	void loadMapsInitGobs(void);

	virtual int8 getPass(int x, int y, int heightOff = -1) = 0;
	virtual void setPass(int x, int y, int8 pass, int heightOff = -1) = 0;

	virtual void loadMapObjects(char *avjFile) = 0;
	virtual void findNearestToGob(Mult::Mult_Object *obj) = 0;
	virtual void findNearestToDest(Mult::Mult_Object *obj) = 0;
	virtual void optimizePoints(Mult::Mult_Object *obj, int16 x, int16 y) = 0;

	virtual void init(void) = 0;
	Map(GobEngine *vm);
	virtual ~Map();

protected:
	char *_avoDataPtr;
	GobEngine *_vm;

	int16 findNearestWayPoint(int16 x, int16 y);
	uint16 loadFromAvo_LE_UINT16();
};

class Map_v1 : public Map {
public:
	virtual void loadMapObjects(char *avjFile);
	virtual void findNearestToGob(Mult::Mult_Object *obj);
	virtual void findNearestToDest(Mult::Mult_Object *obj);
	virtual void optimizePoints(Mult::Mult_Object *obj, int16 x, int16 y);

	virtual inline int8 getPass(int x, int y, int heightOff = -1) {
		return _passMap[y * _mapWidth + x];
	}
	
	virtual inline void setPass(int x, int y, int8 pass, int heightOff = -1) {
		_passMap[y * _mapWidth + x] = pass;
	}

	virtual void init(void);
	Map_v1(GobEngine *vm);
	virtual ~Map_v1();
};

class Map_v2 : public Map_v1 {
public:
	virtual void loadMapObjects(char *avjFile);
	virtual void findNearestToGob(Mult::Mult_Object *obj);
	virtual void findNearestToDest(Mult::Mult_Object *obj);
	virtual void optimizePoints(Mult::Mult_Object *obj, int16 x, int16 y);

	virtual inline int8 getPass(int x, int y, int heightOff = -1) {
		if (heightOff == -1)
			heightOff = _passWidth;
		return _passMap[y * heightOff + x];
	}
	
	virtual inline void setPass(int x, int y, int8 pass, int heightOff = -1) {
		if (heightOff == -1)
			heightOff = _passWidth;
		_passMap[y * heightOff + x] = pass;
	}

	virtual void init(void);
	Map_v2(GobEngine *vm);
	virtual ~Map_v2();
};

}				// End of namespace Gob

#endif	/* __MAP_H */
