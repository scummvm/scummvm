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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */
#include "gob/gob.h"
#include "gob/map.h"
#include "gob/video.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/inter.h"
#include "gob/goblin.h"
#include "gob/sound.h"
#include "gob/scenery.h"

namespace Gob {

int8 map_passMap[kMapHeight][kMapWidth];	// [y][x]
int16 map_itemsMap[kMapHeight][kMapWidth];	// [y][x]

Map_Point map_wayPoints[40];
int16 map_nearestWayPoint = 0;
int16 map_nearestDest = 0;

int16 map_curGoblinX;
int16 map_curGoblinY;
int16 map_destX;
int16 map_destY;

Map_ItemPos map_itemPoses[40];
int8 map_loadFromAvo;
char map_sourceFile[15];
static char *map_avoDataPtr;

void map_placeItem(int16 x, int16 y, int16 id) {
	if ((map_itemsMap[y][x] & 0xff00) != 0)
		map_itemsMap[y][x] = (map_itemsMap[y][x] & 0xff00) | id;
	else
		map_itemsMap[y][x] = (map_itemsMap[y][x] & 0x00ff) | (id << 8);
}

enum {
	kLeft  = (1 << 0),
	kUp    = (1 << 1),
	kRight = (1 << 2),
	kDown  = (1 << 3)
};

int16 map_getDirection(int16 x0, int16 y0, int16 x1, int16 y1) {
	int16 dir = 0;

	if (x0 == x1 && y0 == y1)
		return 0;

	if (!(x1 >= 0 && x1 < kMapWidth && y1 >= 0 && y1 < kMapHeight))
		return 0;

	if (y1 > y0)
		dir |= kDown;
	else if (y1 < y0)
		dir |= kUp;

	if (x1 > x0)
		dir |= kRight;
	else if (x1 < x0)
		dir |= kLeft;

	if (map_passMap[y0][x0] == 3 && (dir & kUp)) {
		if (map_passMap[y0 - 1][x0] != 0)
			return kDirN;
	}

	if (map_passMap[y0][x0] == 3 && (dir & kDown)) {
		if (map_passMap[y0 + 1][x0] != 0)
			return kDirS;
	}

	if (map_passMap[y0][x0] == 6 && (dir & kUp)) {
		if (map_passMap[y0 - 1][x0] != 0)
			return kDirN;
	}

	if (map_passMap[y0][x0] == 6 && (dir & kDown)) {
		if (map_passMap[y0 + 1][x0] != 0)
			return kDirS;
	}

	if (dir == kLeft) {
		if (x0 - 1 >= 0 && map_passMap[y0][x0 - 1] != 0)
			return kDirW;
		return 0;
	}

	if (dir == kRight) {
		if (x0 + 1 < kMapWidth && map_passMap[y0][x0 + 1] != 0)
			return kDirE;
		return 0;
	}

	if (dir == kUp) {
		if (y0 - 1 >= 0 && map_passMap[y0 - 1][x0] != 0)
			return kDirN;

		if (y0 - 1 >= 0 && x0 - 1 >= 0
		    && map_passMap[y0 - 1][x0 - 1] != 0)
			return kDirNW;

		if (y0 - 1 >= 0 && x0 + 1 < kMapWidth
		    && map_passMap[y0 - 1][x0 + 1] != 0)
			return kDirNE;

		return 0;
	}

	if (dir == kDown) {
		if (y0 + 1 < kMapHeight && map_passMap[y0 + 1][x0] != 0)
			return kDirS;

		if (y0 + 1 < kMapHeight && x0 - 1 >= 0
		    && map_passMap[y0 + 1][x0 - 1] != 0)
			return kDirSW;

		if (y0 + 1 < kMapHeight && x0 + 1 < kMapWidth
		    && map_passMap[y0 + 1][x0 + 1] != 0)
			return kDirSE;

		return 0;
	}

	if (dir == (kRight | kUp)) {
		if (y0 - 1 >= 0 && x0 + 1 < kMapWidth
		    && map_passMap[y0 - 1][x0 + 1] != 0)
			return kDirNE;

		if (y0 - 1 >= 0 && map_passMap[y0 - 1][x0] != 0)
			return kDirN;

		if (x0 + 1 < kMapWidth && map_passMap[y0][x0 + 1] != 0)
			return kDirE;

		return 0;
	}

	if (dir == (kRight | kDown)) {
		if (x0 + 1 < kMapWidth && y0 + 1 < kMapHeight
		    && map_passMap[y0 + 1][x0 + 1] != 0)
			return kDirSE;

		if (y0 + 1 < kMapHeight && map_passMap[y0 + 1][x0] != 0)
			return kDirS;

		if (x0 + 1 < kMapWidth && map_passMap[y0][x0 + 1] != 0)
			return kDirE;

		return 0;
	}

	if (dir == (kLeft | kUp)) {
		if (x0 - 1 >= 0 && y0 - 1 >= 0
		    && map_passMap[y0 - 1][x0 - 1] != 0)
			return kDirNW;

		if (y0 - 1 >= 0 && map_passMap[y0 - 1][x0] != 0)
			return kDirN;

		if (x0 - 1 >= 0 && map_passMap[y0][x0 - 1] != 0)
			return kDirW;

		return 0;
	}

	if (dir == (kLeft | kDown)) {
		if (x0 - 1 >= 0 && y0 + 1 < kMapHeight
		    && map_passMap[y0 + 1][x0 - 1] != 0)
			return kDirSW;

		if (y0 + 1 < kMapHeight && map_passMap[y0 + 1][x0] != 0)
			return kDirS;

		if (x0 - 1 >= 0 && map_passMap[y0][x0 - 1] != 0)
			return kDirW;

		return 0;
	}
	return -1;
}

int16 map_findNearestWayPoint(int16 x, int16 y) {
	int16 nearestWayPoint = -1;
	int16 length;
	int16 i;
	int16 tmp;

	length = 30000;

	for (i = 0; i < 40; i++) {
		if (map_wayPoints[i].x < 0 ||
		    map_wayPoints[i].x >= kMapWidth ||
		    map_wayPoints[i].y < 0 || map_wayPoints[i].y >= kMapHeight)
			return -1;

		tmp = ABS(x - map_wayPoints[i].x) + ABS(y - map_wayPoints[i].y);

		if (tmp <= length) {
			nearestWayPoint = i;
			length = tmp;
		}
	}

	return nearestWayPoint;
}

void map_findNearestToGob(void) {
	int16 wayPoint = map_findNearestWayPoint(map_curGoblinX, map_curGoblinY);

	if (wayPoint != -1)
		map_nearestWayPoint = wayPoint;
}

void map_findNearestToDest(void) {
	int16 wayPoint = map_findNearestWayPoint(map_destX, map_destY);

	if (wayPoint != -1)
		map_nearestDest = wayPoint;
}

int16 map_checkDirectPath(int16 x0, int16 y0, int16 x1, int16 y1) {
	uint16 dir;

	while (1) {
		dir = map_getDirection(x0, y0, x1, y1);

		if (x0 == x1 && y0 == y1)
			return 1;

		if (dir == 0)
			return 3;

		switch (dir) {
		case kDirNW:
			x0--;
			y0--;
			break;

		case kDirN:
			y0--;
			break;

		case kDirNE:
			x0++;
			y0--;
			break;

		case kDirW:
			x0--;
			break;

		case kDirE:
			x0++;
			break;

		case kDirSW:
			x0--;
			y0++;
			break;

		case kDirS:
			y0++;
			break;

		case kDirSE:
			x0++;
			y0++;
			break;
		}
	}
}

int16 map_checkLongPath(int16 x0, int16 y0, int16 x1, int16 y1, int16 i0, int16 i1) {
	uint16 dir;
	int16 curX;
	int16 curY;
	int16 nextLink;

	curX = x0;
	curY = y0;
	dir = 0;

	nextLink = 1;

	while (1) {
		if (x0 == curX && y0 == curY)
			nextLink = 1;

		if (nextLink != 0) {
			if (map_checkDirectPath(x0, y0, x1, y1) == 1)
				return 1;

			nextLink = 0;
			if (i0 > i1) {
				curX = map_wayPoints[i0].x;
				curY = map_wayPoints[i0].y;
				i0--;
			} else if (i0 < i1) {
				curX = map_wayPoints[i0].x;
				curY = map_wayPoints[i0].y;
				i0++;
			} else if (i0 == i1) {
				curX = map_wayPoints[i0].x;
				curY = map_wayPoints[i0].y;
			}
		}
		if (i0 == i1 && map_wayPoints[i0].x == x0
		    && map_wayPoints[i0].y == y0) {
			if (map_checkDirectPath(x0, y0, x1, y1) == 1)
				return 1;
			return 0;
		}
		dir = map_getDirection(x0, y0, curX, curY);
		switch (dir) {
		case 0:
			return 0;

		case kDirNW:
			x0--;
			y0--;
			break;

		case kDirN:
			y0--;
			break;

		case kDirNE:
			x0++;
			y0--;
			break;

		case kDirW:
			x0--;
			break;

		case kDirE:
			x0++;
			break;

		case kDirSW:
			x0--;
			y0++;
			break;

		case kDirS:
			y0++;
			break;

		case kDirSE:
			x0++;
			y0++;
			break;
		}
	}
}

void map_optimizePoints(void) {
	int16 i;

	if (map_nearestWayPoint < map_nearestDest) {
		for (i = map_nearestWayPoint; i <= map_nearestDest; i++) {
			if (map_checkDirectPath(map_curGoblinX, map_curGoblinY,
				map_wayPoints[i].x, map_wayPoints[i].y) == 1)
				map_nearestWayPoint = i;
		}
	} else if (map_nearestWayPoint > map_nearestDest) {
		for (i = map_nearestWayPoint; i >= map_nearestDest; i--) {
			if (map_checkDirectPath(map_curGoblinX, map_curGoblinY,
				map_wayPoints[i].x, map_wayPoints[i].y) == 1)
				map_nearestWayPoint = i;
		}
	}
}

void map_loadDataFromAvo(char *dest, int16 size) {
	memcpy(dest, map_avoDataPtr, size);
	map_avoDataPtr += size;
}

uint16 map_loadFromAvo_LE_UINT16() {
	uint16 tmp = READ_LE_UINT16(map_avoDataPtr);
	map_avoDataPtr += 2;
	return tmp;
}

void map_loadItemToObject(void) {
	int16 flag;
	int16 count;
	int16 i;

	flag = map_loadFromAvo_LE_UINT16();
	if (flag == 0)
		return;

	map_avoDataPtr += 1456;
	count = map_loadFromAvo_LE_UINT16();
	for (i = 0; i < count; i++) {
		map_avoDataPtr += 20;
		gob_itemToObject[i] = map_loadFromAvo_LE_UINT16();
		map_avoDataPtr += 5;
	}
}

void map_loadMapObjects(char *avjFile) {
	int16 i;
	char avoName[128];
	int16 handle;
	char item;
	int16 soundCount;
	int16 tmp;
	char *savedPtr;
	char *savedPtr2;
	char *savedPtr3;
	int16 state;
	int16 col;
	int32 flag;
	Gob_State *pState;
	char buf[128];
	char sndNames[20][14];
	char *dataBuf;
	int16 x;
	int16 y;
	int16 count2;
	int16 count3;

	strcpy(avoName, map_sourceFile);
	strcat(avoName, ".avo");

	handle = data_openData(avoName);
	if (handle >= 0) {
		map_loadFromAvo = 1;
		data_closeData(handle);
		map_avoDataPtr = data_getData(avoName);
		dataBuf = map_avoDataPtr;
		map_loadDataFromAvo((char *)map_passMap, kMapHeight * kMapWidth);

		for (y = 0; y < kMapHeight; y++) {
			for (x = 0; x < kMapWidth; x++) {
				map_loadDataFromAvo(&item, 1);
				map_itemsMap[y][x] = item;
			}
		}

		for (i = 0; i < 40; i++) {
			map_wayPoints[i].x = map_loadFromAvo_LE_UINT16();
			map_wayPoints[i].y = map_loadFromAvo_LE_UINT16();
		}
		map_loadDataFromAvo((char *)map_itemPoses, szMap_ItemPos * 20);
	} else {
		map_loadFromAvo = 0;
		map_avoDataPtr = data_getData(avjFile);
		dataBuf = map_avoDataPtr;
	}

	map_avoDataPtr += 32;
	map_avoDataPtr += 76;
	map_avoDataPtr += 4;
	map_avoDataPtr += 20;

	for (i = 0; i < 3; i++) {
		tmp = map_loadFromAvo_LE_UINT16();
		map_avoDataPtr += tmp * 14;
	}

	soundCount = map_loadFromAvo_LE_UINT16();
	savedPtr = map_avoDataPtr;

	map_avoDataPtr += 14 * soundCount;
	map_avoDataPtr += 4;
	map_avoDataPtr += 24;

	count2 = map_loadFromAvo_LE_UINT16();
	count3 = map_loadFromAvo_LE_UINT16();

	savedPtr2 = map_avoDataPtr;
	map_avoDataPtr += count2 * 8;

	savedPtr3 = map_avoDataPtr;
	map_avoDataPtr += count3 * 8;

	gob_gobsCount = map_loadFromAvo_LE_UINT16();
	for (i = 0; i < gob_gobsCount; i++) {
		gob_goblins[i] = (Gob_Object *)malloc(sizeof(Gob_Object));

		gob_goblins[i]->xPos = READ_LE_UINT16(savedPtr2);
		savedPtr2 += 2;

		gob_goblins[i]->yPos = READ_LE_UINT16(savedPtr2);
		savedPtr2 += 2;

		gob_goblins[i]->order = READ_LE_UINT16(savedPtr2);
		savedPtr2 += 2;

		gob_goblins[i]->state = READ_LE_UINT16(savedPtr2);
		savedPtr2 += 2;

		if (i == 3)
			gob_goblins[i]->stateMach = (Gob_StateLine *)malloc(szGob_StateLine * 70);
		else
			gob_goblins[i]->stateMach = (Gob_StateLine *)malloc(szGob_StateLine * 40);

		// FIXME: All is wrong further. We should unwind calls to map_loadDataFromAvo()
		map_loadDataFromAvo((char *)gob_goblins[i]->stateMach, 40 * szGob_StateLine);
		map_avoDataPtr += 160;
		gob_goblins[i]->multObjIndex = *map_avoDataPtr;
		map_avoDataPtr += 2;

		gob_goblins[i]->realStateMach = gob_goblins[i]->stateMach;
		for (state = 0; state < 40; state++) {
			for (col = 0; col < 6; col++) {
				if (gob_goblins[i]->stateMach[state][col] == 0)
					continue;

				Gob_State *tmpState = (Gob_State *)malloc(sizeof(Gob_State));
				gob_goblins[i]->stateMach[state][col] = tmpState;

				tmpState->animation = map_loadFromAvo_LE_UINT16();
				tmpState->layer = map_loadFromAvo_LE_UINT16();
				map_avoDataPtr += 8;
				tmpState->unk0 = map_loadFromAvo_LE_UINT16();
				tmpState->unk1 = map_loadFromAvo_LE_UINT16();

				map_avoDataPtr += 2;
				if (READ_LE_UINT32(map_avoDataPtr) != 0) {
					map_avoDataPtr += 4;
					tmpState->sndItem = map_loadFromAvo_LE_UINT16();
				} else {
					map_avoDataPtr += 6;
					tmpState->sndItem = -1;
				}
				tmpState->freq = map_loadFromAvo_LE_UINT16();
				tmpState->repCount = map_loadFromAvo_LE_UINT16();
				tmpState->sndFrame = map_loadFromAvo_LE_UINT16();
			}
		}
	}

	pState = (Gob_State *)malloc(sizeof(Gob_State));
	gob_goblins[0]->stateMach[39][0] = pState;
	pState->animation = 0;
	pState->layer = 98;
	pState->unk0 = 0;
	pState->unk1 = 0;
	pState->sndItem = -1;

	pState = (Gob_State *) malloc(sizeof(Gob_State));
	gob_goblins[1]->stateMach[39][0] = pState;
	pState->animation = 0;
	pState->layer = 99;
	pState->unk0 = 0;
	pState->unk1 = 0;
	pState->sndItem = -1;

	pState = (Gob_State *) malloc(sizeof(Gob_State));
	gob_goblins[2]->stateMach[39][0] = pState;
	pState->animation = 0;
	pState->layer = 100;
	pState->unk0 = 0;
	pState->unk1 = 0;
	pState->sndItem = -1;

	gob_goblins[2]->stateMach[10][0]->sndFrame = 13;
	gob_goblins[2]->stateMach[11][0]->sndFrame = 13;
	gob_goblins[2]->stateMach[28][0]->sndFrame = 13;
	gob_goblins[2]->stateMach[29][0]->sndFrame = 13;

	gob_goblins[1]->stateMach[10][0]->sndFrame = 13;
	gob_goblins[1]->stateMach[11][0]->sndFrame = 13;

	for (state = 40; state < 70; state++) {
		pState = (Gob_State *)malloc(sizeof(Gob_State));
		gob_goblins[3]->stateMach[state][0] = pState;
		gob_goblins[3]->stateMach[state][1] = 0;

		pState->animation = 9;
		pState->layer = state - 40;
		pState->sndItem = -1;
		pState->sndFrame = 0;
	}

	gob_objCount = map_loadFromAvo_LE_UINT16();
	for (i = 0; i < gob_objCount; i++) {
		gob_objects[i] =
		    (Gob_Object *) malloc(sizeof(Gob_Object));

		gob_objects[i]->xPos = READ_LE_UINT16(savedPtr3);
		savedPtr3 += 2;

		gob_objects[i]->yPos = READ_LE_UINT16(savedPtr3);
		savedPtr3 += 2;

		gob_objects[i]->order = READ_LE_UINT16(savedPtr3);
		savedPtr3 += 2;

		gob_objects[i]->state = READ_LE_UINT16(savedPtr3);
		savedPtr3 += 2;

		gob_objects[i]->stateMach = (Gob_StateLine *)malloc(szGob_StateLine * 40);

		map_loadDataFromAvo((char *)gob_objects[i]->stateMach, 40 * szGob_StateLine);
		map_avoDataPtr += 160;
		gob_objects[i]->multObjIndex = *map_avoDataPtr;
		map_avoDataPtr += 2;

		gob_objects[i]->realStateMach = gob_objects[i]->stateMach;
		for (state = 0; state < 40; state++) {
			for (col = 0; col < 6; col++) {
				if (gob_objects[i]->stateMach[state][col] == 0)
					continue;

				Gob_State *tmpState = (Gob_State *)malloc(sizeof(Gob_State));
				gob_objects[i]->stateMach[state][col] = tmpState;

				tmpState->animation = map_loadFromAvo_LE_UINT16();
				tmpState->layer = map_loadFromAvo_LE_UINT16();
				map_avoDataPtr += 8;
				tmpState->unk0 = map_loadFromAvo_LE_UINT16();
				tmpState->unk1 = map_loadFromAvo_LE_UINT16();

				map_avoDataPtr += 2;
				if (READ_LE_UINT32(map_avoDataPtr) != 0) {
					map_avoDataPtr += 4;
					tmpState->sndItem = map_loadFromAvo_LE_UINT16();
				} else {
					map_avoDataPtr += 6;
					tmpState->sndItem = -1;
				}
				tmpState->freq = map_loadFromAvo_LE_UINT16();
				tmpState->repCount = map_loadFromAvo_LE_UINT16();
				tmpState->sndFrame = map_loadFromAvo_LE_UINT16();
			}
		}
	}

	gob_objects[10] = (Gob_Object *)malloc(sizeof(Gob_Object));
	memset(gob_objects[10], 0, sizeof(Gob_Object));

	gob_objects[10]->stateMach = (Gob_StateLine *)malloc(szGob_StateLine * 40);
	memset(gob_objects[10]->stateMach, 0, szGob_StateLine * 40);

	pState = (Gob_State *)malloc(sizeof(Gob_State));
	gob_objects[10]->stateMach[0][0] = pState;

	memset(pState, 0, sizeof(Gob_State));
	pState->animation = 9;
	pState->layer = 27;
	pState->unk0 = 0;
	pState->unk1 = 0;
	pState->sndItem = -1;
	pState->sndFrame = 0;

	gob_placeObject(gob_objects[10], 1);

	gob_objects[10]->realStateMach = gob_objects[10]->stateMach;
	gob_objects[10]->type = 1;
	gob_objects[10]->unk14 = 1;

	state = map_loadFromAvo_LE_UINT16();
	for (i = 0; i < state; i++) {
		map_avoDataPtr += 30;

		map_loadDataFromAvo((char *)&flag, 4);
		map_avoDataPtr += 56;

		if (flag != 0)
			map_avoDataPtr += 30;
	}

	map_loadDataFromAvo((char *)&tmp, 2);
	map_avoDataPtr += 48;
	map_loadItemToObject();
	map_avoDataPtr = savedPtr;

	for (i = 0; i < soundCount; i++) {
		map_loadDataFromAvo(buf, 14);
		strcat(buf, ".SND");
		strcpy(sndNames[i], buf);
	}

	free(dataBuf);

	gob_soundData[14] = snd_loadSoundData("diamant1.snd");

	for (i = 0; i < soundCount; i++) {
		handle = data_openData(sndNames[i]);
		if (handle < 0)
			continue;

		data_closeData(handle);
		gob_soundData[i] = snd_loadSoundData(sndNames[i]);
	}
}

void map_loadMapsInitGobs(void) {
	int16 layer;
	int16 i;

	if (map_loadFromAvo == 0)
		error("map_load: Loading .pas/.pos files is not supported!");

	for (i = 0; i < 3; i++) {
		gob_nextLayer(gob_goblins[i]);
	}

	for (i = 0; i < 3; i++) {

		layer =
		    gob_goblins[i]->stateMach[gob_goblins[i]->state][0]->layer;

		scen_updateAnim(layer, 0, gob_goblins[i]->animation, 0,
		    gob_goblins[i]->xPos, gob_goblins[i]->yPos, 0);

		gob_goblins[i]->yPos = (gob_gobPositions[i].y + 1) * 6 -
		    (scen_toRedrawBottom - scen_animTop);

		gob_goblins[i]->xPos = gob_gobPositions[i].x * 12 -
		    (scen_toRedrawLeft - scen_animLeft);

		gob_goblins[i]->order = scen_toRedrawBottom / 24 + 3;
	}

	gob_currentGoblin = 0;
	gob_pressedMapX = gob_gobPositions[0].x;
	gob_pressedMapY = gob_gobPositions[0].y;
	gob_pathExistence = 0;

	gob_goblins[0]->doAnim = 0;
	gob_goblins[1]->doAnim = 1;
	gob_goblins[2]->doAnim = 1;
}

}				// End of namespace Gob
