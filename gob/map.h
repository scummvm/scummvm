/*
** Gobliiins 1
** Original game by CoktelVision
**
** Reverse engineered by Ivan Dubrov <WFrag@yandex.ru>
**
*/
#ifndef __MAP_H
#define __MAP_H

namespace Gob {

#pragma START_PACK_STRUCTS
#define szMap_Point 4
typedef struct Map_Point {
	int16 x;
	int16 y;
} GCC_PACK Map_Point;

#define szMap_ItemPos 3
typedef struct Map_ItemPos {
	char x;
	char y;
	char orient;		// ??
} GCC_PACK Map_ItemPos;
#pragma END_PACK_STRUCTS

extern char map_passMap[28][26];	// [y][x]
extern int16 map_itemsMap[28][26];	// [y][x]
extern Map_Point map_wayPoints[40];
extern int16 map_nearestWayPoint;
extern int16 map_nearestDest;

extern int16 map_curGoblinX;
extern int16 map_curGoblinY;
extern int16 map_destX;
extern int16 map_destY;
extern char map_loadFromAvo;

extern Map_ItemPos map_itemPoses[40];
extern char map_sourceFile[15];
extern char *map_avoDataPtr;

int16 map_getDirection(int16 x0, int16 y0, int16 x1, int16 y1);
void map_findNearestToGob(void);
void map_findNearestToDest(void);
int16 map_checkDirectPath(int16 x0, int16 y0, int16 x1, int16 y1);
int16 map_checkLongPath(int16 x0, int16 y0, int16 x1, int16 y1, int16 i0, int16 i1);
int16 map_optimizePoints(int16 xPos, int16 yPos);
void map_loadItemToObject(void);
void map_loadMapObjects(char *avjFile);
void map_loadDataFromAvo(char *dest, int16 size);
void map_loadMapsInitGobs(void);

}				// End of namespace Gob

#endif	/* __MAP_H */
