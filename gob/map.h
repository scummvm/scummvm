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
 * $Header$
 *
 */
#ifndef GOB_MAP_H
#define GOB_MAP_H

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
	enum {
		kMapWidth  = 26,
		kMapHeight = 28
	};

#pragma START_PACK_STRUCTS

	typedef struct Point {
		int16 x;
		int16 y;
	} GCC_PACK Point;

#define szMap_ItemPos 3

	typedef struct ItemPos {
		int8 x;
		int8 y;
		int8 orient;		// ??
	} GCC_PACK ItemPos;

#pragma END_PACK_STRUCTS

	int8 _passMap[kMapHeight][kMapWidth];	// [y][x]
	int16 _itemsMap[kMapHeight][kMapWidth];	// [y][x]
	Point _wayPoints[40];
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
	void findNearestToGob(void);
	void findNearestToDest(void);
	int16 checkDirectPath(int16 x0, int16 y0, int16 x1, int16 y1);
	int16 checkLongPath(int16 x0, int16 y0, int16 x1, int16 y1, int16 i0, int16 i1);
	void optimizePoints(void);
	void loadItemToObject(void);
	void loadMapObjects(char *avjFile);
	void loadDataFromAvo(char *dest, int16 size);
	void loadMapsInitGobs(void);

	Map(GobEngine *vm);

protected:
	char *_avoDataPtr;
	GobEngine *_vm;

	int16 findNearestWayPoint(int16 x, int16 y);
	uint16 loadFromAvo_LE_UINT16();
};

}				// End of namespace Gob

#endif	/* __MAP_H */
