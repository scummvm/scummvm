/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
#ifndef GOB_MAP_H
#define GOB_MAP_H

namespace Gob {

#pragma START_PACK_STRUCTS
#define szMap_Point 4
typedef struct Map_Point {
	int16 x;
	int16 y;
} GCC_PACK Map_Point;

#define szMap_ItemPos 3
typedef struct Map_ItemPos {
	int8 x;
	int8 y;
	int8 orient;		// ??
} GCC_PACK Map_ItemPos;
#pragma END_PACK_STRUCTS

extern int8 map_passMap[28][26];	// [y][x]
extern int16 map_itemsMap[28][26];	// [y][x]
extern Map_Point map_wayPoints[40];
extern int16 map_nearestWayPoint;
extern int16 map_nearestDest;

extern int16 map_curGoblinX;
extern int16 map_curGoblinY;
extern int16 map_destX;
extern int16 map_destY;
extern int8 map_loadFromAvo;

extern Map_ItemPos map_itemPoses[40];
extern char map_sourceFile[15];

void map_placeItem(int16 x, int16 y, int16 id);

int16 map_getDirection(int16 x0, int16 y0, int16 x1, int16 y1);
void map_findNearestToGob(void);
void map_findNearestToDest(void);
int16 map_checkDirectPath(int16 x0, int16 y0, int16 x1, int16 y1);
int16 map_checkLongPath(int16 x0, int16 y0, int16 x1, int16 y1, int16 i0, int16 i1);
int16 map_optimizePoints(int16 xPos, int16 yPos);
void map_loadItemToObject(void);
void map_loadMapObjects(char *avjFile);
void map_loadDataFromAvo(int8 *dest, int16 size);
void map_loadMapsInitGobs(void);

}				// End of namespace Gob

#endif	/* __MAP_H */
