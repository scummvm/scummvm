/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "hopkins/lines.h"

#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"

#include "common/system.h"
#include "common/textconsole.h"

namespace Hopkins {


int LigneItem::appendToRouteInc(int from, int to, RouteItem *route, int index) {
	if (to == -1)
		to = _lineDataEndIdx;

	for (int i = from; i < to; ++i)
		route[index++].set(_lineData[2*i], _lineData[2*i+1], _directionRouteInc);
	return index;
}
int LigneItem::appendToRouteDec(int from, int to, RouteItem *route, int index) {
	if (from == -1)
		from = _lineDataEndIdx - 1;

	for (int i = from; i > to; --i)
		route[index++].set(_lineData[2*i], _lineData[2*i+1], _directionRouteDec);
	return index;
}


LinesManager::LinesManager() {
	for (int i = 0; i < MAX_LINES; ++i) {
		Common::fill((byte *)&_zoneLine[i], (byte *)&_zoneLine[i] + sizeof(LigneZoneItem), 0);
		Common::fill((byte *)&_lineItem[i], (byte *)&_lineItem[i] + sizeof(LigneItem), 0);
	}

	for (int i = 0; i < 4000; ++i)
		Common::fill((byte *)&_smoothRoute[i], (byte *)&_smoothRoute[i] + sizeof(SmoothItem), 0);

	for (int i = 0; i < 8001; ++i)
		_bestRoute[i].set(0, 0, DIR_NONE);

	for (int i = 0; i < 101; ++i) {
		Common::fill((byte *)&_segment[i], (byte *)&_segment[i] + sizeof(SegmentItem), 0);
		Common::fill((byte *)&_squareZone[i], (byte *)&_squareZone[i] + sizeof(SquareZoneItem), 0);
	}

	for (int i = 0; i < 105; ++i) {
		BOBZONE[i] = 0;
		BOBZONE_FLAG[i] = false;
	}

	for (int i = 0; i < 106; ++i)
		Common::fill((byte *)&ZONEP[i], (byte *)&ZONEP[i] + sizeof(ZonePItem), 0);

	_linesNumb = 0;
	_newLineIdx = 0;
	_newLineDataIdx = 0;
	_newRouteIdx = 0;
	_newPosX = 0;
	_newPosY = 0;
	_smoothMoveDirection = DIR_NONE;
	_lastLine = 0;
	_maxLineIdx = 0;
	_pathFindingMaxDepth = 0;
	essai0 = NULL;
	essai1 = NULL;
	essai2 = NULL;
	_lineBuf = (int16 *)g_PTRNUL;
	_route = (RouteItem *)g_PTRNUL;
	_currentSegmentId = 0;
	_largeBuf = g_PTRNUL;
}

LinesManager::~LinesManager() {
	_vm->_globals.freeMemory(_largeBuf);
}

void LinesManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

/**
 * Load lines
 */
void LinesManager::loadLines(const Common::String &file) {
	resetLines();
	_linesNumb = 0;
	_lastLine = 0;
	byte *ptr = _vm->_fileManager.loadFile(file);
	for (int idx = 0; READ_LE_INT16((uint16 *)ptr + (idx * 5)) != -1; idx++) {
		addLine(idx,
		    (Directions)READ_LE_INT16((uint16 *)ptr + (idx * 5)),
		    READ_LE_INT16((uint16 *)ptr + (idx * 5) + 1),
		    READ_LE_INT16((uint16 *)ptr + (idx * 5) + 2),
		    READ_LE_INT16((uint16 *)ptr + (idx * 5) + 3),
		    READ_LE_INT16((uint16 *)ptr + (idx * 5) + 4));
	}
	initRoute();
	_vm->_globals.freeMemory(ptr);
}

/** 
 * Check Hotspots in Inventory screen
 * Returns the ID of the hotspot under mouse
 */
int LinesManager::checkInventoryHotspots(int posX, int posY) {
	int hotspotId = 0;
	if (posY >= 120 && posY <= 153)
		hotspotId = checkInventoryHotspotsRow(posX, 1, false);
	if (posY >= 154 && posY <= 191)
		hotspotId = checkInventoryHotspotsRow(posX, 7, false);
	if (posY >= 192 && posY <= 229)
		hotspotId = checkInventoryHotspotsRow(posX, 13, false);
	if (posY >= 230 && posY <= 267)
		hotspotId = checkInventoryHotspotsRow(posX, 19, false);
	if (posY >= 268 && posY <= 306)
		hotspotId = checkInventoryHotspotsRow(posX, 25, true);
	if (posY >= 268 && posY <= 288 && posX >= _vm->_graphicsManager._scrollOffset + 424 && posX <= _vm->_graphicsManager._scrollOffset + 478)
		hotspotId = 30;
	if (posY >= 290 && posY <= 306 && posX >= _vm->_graphicsManager._scrollOffset + 424 && posX <= _vm->_graphicsManager._scrollOffset + 478)
		hotspotId = 31;
	if (posY < 114 || posY > 306 || posX < _vm->_graphicsManager._scrollOffset + 152 || posX > _vm->_graphicsManager._scrollOffset + 484)
		hotspotId = 32;

	return hotspotId;
}

/**
 * Check the hotspots in an inventory line
 * Returns the hotspot Id under the mouse, if any.
 */
int LinesManager::checkInventoryHotspotsRow(int posX, int minZoneNum, bool lastRow) {
	int result = minZoneNum;

	if (posX >= _vm->_graphicsManager._scrollOffset + 158 && posX < _vm->_graphicsManager._scrollOffset + 208)
		return result;

	if (posX >= _vm->_graphicsManager._scrollOffset + 208 && posX < _vm->_graphicsManager._scrollOffset + 266) {
		result += 1;
		return result;
	}

	if (posX >= _vm->_graphicsManager._scrollOffset + 266 && posX < _vm->_graphicsManager._scrollOffset + 320) {
		result += 2;
		return result;
	}

	if (posX >= _vm->_graphicsManager._scrollOffset + 320 && posX < _vm->_graphicsManager._scrollOffset + 370) {
		result += 3;
		return result;
	}

	if (posX >= _vm->_graphicsManager._scrollOffset + 370 && posX < _vm->_graphicsManager._scrollOffset + 424) {
		result += 4;
		return result;
	}

	if (!lastRow && posX >= _vm->_graphicsManager._scrollOffset + 424 && posX <= _vm->_graphicsManager._scrollOffset + 478) {
		result += 5;
		return result;
	}

	return 0;
}

/**
 * Add Zone Line
 */
void LinesManager::addZoneLine(int idx, int a2, int a3, int a4, int a5, int bobZoneIdx) {
	int16 *zoneData;

	if (a2 == a3 && a3 == a4 && a3 == a5) {
		BOBZONE_FLAG[bobZoneIdx] = true;
		BOBZONE[bobZoneIdx] = a3;
	} else {
		assert (idx <= MAX_LINES);
		_zoneLine[idx]._zoneData = (int16 *)_vm->_globals.freeMemory((byte *)_zoneLine[idx]._zoneData);

		int v8 = abs(a2 - a4);
		int v9 = abs(a3 - a5);
		int v20 = 1;
		if (v8 <= v9)
			v20 += v9;
		else
			v20 += v8;

		zoneData = (int16 *)_vm->_globals.allocMemory(2 * sizeof(int16) * v20 + (4 * sizeof(int16)));
		assert(zoneData != (int16 *)g_PTRNUL);

		_zoneLine[idx]._zoneData = zoneData;

		int16 *dataP = zoneData;
		int v23 = 1000 * v8 / v20;
		int v22 = 1000 * v9 / v20;
		if (a4 < a2)
			v23 = -v23;
		if (a5 < a3)
			v22 = -v22;
		int v13 = 1000 * a2;
		int v16 = 1000 * a3;
		for (int i = 0; i < v20; i++) {
			*dataP++ = v13 / 1000;
			*dataP++ = v16 / 1000;

			v13 += v23;
			v16 += v22;
		}
		*dataP++ = -1;
		*dataP++ = -1;

		_zoneLine[idx]._count = v20;
		_zoneLine[idx]._bobZoneIdx = bobZoneIdx;
	}
}

/**
 * Add Line
 */
void LinesManager::addLine(int idx, Directions direction, int a3, int a4, int a5, int a6) {
	assert (idx <= MAX_LINES);

	if (_linesNumb < idx)
		_linesNumb = idx;

	_lineItem[idx]._lineData = (int16 *)_vm->_globals.freeMemory((byte *)_lineItem[idx]._lineData);
	int v8 = abs(a3 - a5) + 1;
	int v34 = abs(a4 - a6) + 1;
	int v33 = v34;
	if (v8 > v34)
		v34 = v8;

	byte *v10 = _vm->_globals.allocMemory(4 * v34 + 8);
	assert (v10 != g_PTRNUL);

	Common::fill(v10, v10 + 4 * v34 + 8, 0);
	_lineItem[idx]._lineData = (int16 *)v10;

	int16 *v32 = _lineItem[idx]._lineData;
	int v36 = 1000 * v8;
	int v39 = 1000 * v8 / (v34 - 1);
	int v37 = 1000 * v33 / (v34 - 1);
	if (a5 < a3)
		v39 = -v39;
	if (a6 < a4)
		v37 = -v37;
	int v11 = (int)v39 / 1000;
	int v12 = (int)v37 / 1000;
	if (!v11) {
		if (v12 == -1) {
			_lineItem[idx]._directionRouteInc = DIR_UP;
			_lineItem[idx]._directionRouteDec = DIR_DOWN;
		}
		if (v12 == 1) {
			_lineItem[idx]._directionRouteInc = DIR_DOWN;
			_lineItem[idx]._directionRouteDec = DIR_UP;
		}
	}
	if (v11 == 1) {
		if (v12 == -1) {
			_lineItem[idx]._directionRouteInc = DIR_UP_RIGHT;
			_lineItem[idx]._directionRouteDec = DIR_DOWN_LEFT;
		}
		if (!v12) {
			_lineItem[idx]._directionRouteInc = DIR_RIGHT;
			_lineItem[idx]._directionRouteDec = DIR_LEFT;
		}
		if (v12 == 1) {
			_lineItem[idx]._directionRouteInc = DIR_DOWN_RIGHT;
			_lineItem[idx]._directionRouteDec = DIR_UP_LEFT;
		}
	}
	if (v11 == -1) {
		if (v12 == 1) {
			_lineItem[idx]._directionRouteInc = DIR_DOWN_LEFT;
			_lineItem[idx]._directionRouteDec = DIR_UP_RIGHT;
		}
		if (!v12) {
			_lineItem[idx]._directionRouteInc = DIR_LEFT;
			_lineItem[idx]._directionRouteDec = DIR_RIGHT;
		}
		if (v12 == -1) {
			_lineItem[idx]._directionRouteInc = DIR_UP_LEFT;
			_lineItem[idx]._directionRouteDec = DIR_DOWN_RIGHT;
		}
	}
	if (v11 == 1 && v37 > 250 && v37 <= 999) {
		_lineItem[idx]._directionRouteInc = DIR_DOWN_RIGHT;
		_lineItem[idx]._directionRouteDec = DIR_UP_LEFT;
	}
	if (v11 == -1 && v37 > 250 && v37 <= 999) {
		_lineItem[idx]._directionRouteInc = DIR_DOWN_LEFT;
		_lineItem[idx]._directionRouteDec = DIR_UP_RIGHT;
	}
	if (v11 == 1 && v37 < -250 && v37 > -1000) {
		_lineItem[idx]._directionRouteInc = DIR_UP_RIGHT;
		_lineItem[idx]._directionRouteDec = DIR_DOWN_LEFT;
	}
	// This condition is impossible to meet!
	// Code present in the Linux and BeOS executables
	// CHECKME: maybe it should be checking negative values?
	if (v11 == -1 && v37 <= 249 && v37 > 1000) {
		_lineItem[idx]._directionRouteInc = DIR_UP_LEFT;
		_lineItem[idx]._directionRouteDec = DIR_DOWN_RIGHT;
	}
	int v40 = v36 / v34;
	int v38 = 1000 * v33 / v34;
	if (a5 < a3)
		v40 = -v40;
	if (a6 < a4)
		v38 = -v38;
	int v24 = 1000 * a3;
	int v25 = 1000 * a4;
	int v31 = 1000 * a3 / 1000;
	int v30 = 1000 * a4 / 1000;
	int v35 = v34 - 1;
	for (int v26 = 0; v26 < v35; v26++) {
		v32[0] = v31;
		v32[1] = v30;
		v32 += 2;

		v24 += v40;
		v25 += v38;
		v31 = v24 / 1000;
		v30 = v25 / 1000;
	}
	v32[0] = a5;
	v32[1] = a6;

	v32 += 2;
	v32[0] = -1;
	v32[1] = -1;

	_lineItem[idx]._lineDataEndIdx = v35 + 1;
	_lineItem[idx]._direction = direction;

	++_linesNumb;
}

/**
 * Check collision line
 */
bool LinesManager::checkCollisionLine(int xp, int yp, int *foundDataIdx, int *foundLineIdx, int startLineIdx, int endLineIdx) {
	int16 *lineData;

	int left = xp + 4;
	int right = xp - 4;
	int top = yp + 4;
	int bottom = yp - 4;

	*foundDataIdx = -1;
	*foundLineIdx = -1;

	for (int curLineIdx = startLineIdx; curLineIdx <= endLineIdx; curLineIdx++) {
		lineData = _lineItem[curLineIdx]._lineData;

		if (lineData == (int16 *)g_PTRNUL)
			continue;

		bool collisionFl = true;
		int lineStartX = lineData[0];
		int lineStartY = lineData[1];
		int lineDataIdx = 2 * _lineItem[curLineIdx]._lineDataEndIdx;
		int lineEndX = lineData[lineDataIdx - 2];
		int lineEndY = lineData[lineDataIdx - 1];
		if (lineStartX >= lineEndX) {
			if (right > lineStartX || left < lineEndX)
				collisionFl = false;
		} else { // lineStartX < lineEndX
			if (left < lineStartX || right > lineEndX)
				collisionFl = false;
		}
		if (lineStartY >= lineEndY) {
			if (bottom > lineStartY || top < lineEndY)
				collisionFl = false;
		} else { // lineStartY < lineEndY
			if (top < lineStartY || bottom > lineEndY)
				collisionFl = false;
		}

		if (!collisionFl)
			continue;

		for (int idx = 0; idx < _lineItem[curLineIdx]._lineDataEndIdx; idx++) {
			int lineX = lineData[0];
			int lineY = lineData[1];
			lineData += 2;

			if ((xp == lineX || xp + 1 == lineX) && (yp == lineY || yp + 1 == lineY)) {
				*foundDataIdx = idx;
				*foundLineIdx = curLineIdx;
				return true;
			}
		}
	}
	return false;
}

/**
 * Init route
 */
void LinesManager::initRoute() {
	int lineX = _lineItem[0]._lineData[0];
	int lineY = _lineItem[0]._lineData[1];

	int lineIdx = 1;
	for (;;) {
		int curDataIdx = _lineItem[lineIdx]._lineDataEndIdx;
		int16 *curLineData = _lineItem[lineIdx]._lineData;

		int curLineX = curLineData[2 * curDataIdx - 2];
		int curLineY = curLineData[2 * curDataIdx - 1];
		if (_vm->_graphicsManager._maxX == curLineX || _vm->_graphicsManager._maxY == curLineY || 
			_vm->_graphicsManager._minX == curLineX || _vm->_graphicsManager._minY == curLineY ||
			(lineX == curLineX && lineY == curLineY))
			break;
		if (lineIdx == MAX_LINES)
			error("ERROR - LAST LINE NOT FOUND");

		int16 *nextLineData = _lineItem[lineIdx + 1]._lineData;
		if (nextLineData[0] != curLineX && nextLineData[1] != curLineY)
			break;
		++lineIdx;
	}

	_lastLine = lineIdx;
	for (int idx = 1; idx < MAX_LINES; idx++) {
		if ((_lineItem[idx]._lineDataEndIdx < _maxLineIdx) && (idx != _lastLine + 1)) {
			_lineItem[idx]._directionRouteInc = _lineItem[idx - 1]._directionRouteInc;
			_lineItem[idx]._directionRouteDec = _lineItem[idx - 1]._directionRouteDec;
		}
	}
}

// Avoid
int LinesManager::CONTOURNE(int a1, int a2, int a3, int a4, int a5, RouteItem *route) {
	int v36 = a1;
	int v7 = a2;
	int v8 = a3;
	if (a1 < a4) {
		v8 = _lineItem[a1].appendToRouteInc(a2, -1, route, v8);

		for (int i = a1 + 1; i < a4; i++)
			v8 = _lineItem[i].appendToRouteInc(0, -1, route, v8);

		v7 = 0;
		v36 = a4;
	}
	if (v36 > a4) {
		v8 = _lineItem[v36].appendToRouteDec(v7, 0, route, v8);
		for (int i = v36 - 1; i > a4; i--)
			v8 = _lineItem[i].appendToRouteDec(-1, 0, route, v8);
		v7 = _lineItem[a4]._lineDataEndIdx - 1;
		v36 = a4;
	}
	if (v36 == a4) {
		if (a5 >= v7) {
			v8 = _lineItem[a4].appendToRouteInc(v7, a5, route, v8);
		} else {
			v8 = _lineItem[a4].appendToRouteDec(v7, a5, route, v8);
		}
	}
	return v8;
}

// Avoid 1
int LinesManager::CONTOURNE1(int a1, int a2, int a3, int a4, int a5, RouteItem *route, int a8, int a9) {
	int v9 = a1;
	int v10 = a2;
	int v40 = a3;
	if (a4 < a1) {
		v40 = _lineItem[a1].appendToRouteInc(a2, -1, route, v40);
		int v15 = a1 + 1;
		if (v15 == a9 + 1)
			v15 = a8;
		while (a4 != v15) {
			v40 = _lineItem[v15].appendToRouteInc(0, -1, route, v40);
			++v15;
			if (a9 + 1 == v15)
				v15 = a8;
		}
		v10 = 0;
		v9 = a4;
	}
	if (a4 > v9) {
		v40 = _lineItem[v9].appendToRouteDec(v10, 0, route, v40);
		int v24 = v9 - 1;
		if (v24 == a8 - 1)
			v24 = a9;
		while (a4 != v24) {
			v40 = _lineItem[v24].appendToRouteDec(-1, 0, route, v40);
			--v24;
			if (a8 - 1 == v24)
				v24 = a9;
		}
		v10 = _lineItem[a4]._lineDataEndIdx - 1;
		v9 = a4;
	}
	if (a4 == v9) {
		if (a5 >= v10) {
			v40 = _lineItem[a4].appendToRouteInc(v10, a5, route, v40);
		} else {
			v40 = _lineItem[a4].appendToRouteDec(v10, a5, route, v40);
		}
	}
	return v40;
}

bool LinesManager::MIRACLE(int fromX, int fromY, int a3, int a4, int a5) {
	int v35 = 0;
	int v36 = 0;
	int v42 = 0;
	int v43 = 0;
	int v44 = 0;
	int v45 = 0;
	int v46 = 0;
	int v47 = 0;
	int v48 = 0;
	int v49 = 0;

	int curX = fromX;
	int curY = fromY;
	int v50 = a3;
	int v7 = a5;
	int v51;
	if (checkCollisionLine(fromX, fromY, &v51, &v50, 0, _linesNumb)) {
		switch (_lineItem[v50]._direction) {
		case DIR_UP:
			curY -= 2;
			break;
		case DIR_UP_RIGHT:
			curY -= 2;
			curX += 2;
			break;
		case DIR_RIGHT:
			curX += 2;
			break;
		case DIR_DOWN_RIGHT:
			curY += 2;
			curX += 2;
			break;
		case DIR_DOWN:
			curY += 2;
			break;
		case DIR_DOWN_LEFT:
			curY += 2;
			curX -= 2;
			break;
		case DIR_LEFT:
			curX -= 2;
			break;
		case DIR_UP_LEFT:
			curY -= 2;
			curX -= 2;
			break;
		default:
			break;
		}
	}
	int v41 = curX;
	int v40 = curY;
	int v9 = 0;
	int v10 = v40;
	for (int i = v40; v40 + 200 > v10; i = v10) {
		if (checkCollisionLine(v41, i, &v49, &v48, 0, _lastLine) == 1 && v48 <= _lastLine)
			break;
		v49 = 0;
		v48 = -1;
		++v9;
		++v10;
	}
	int v37 = v9;
	int v12 = 0;
	int v13 = v40;
	for (int j = v40; v40 - 200 < v13; j = v13) {
		if (checkCollisionLine(v41, j, &v47, &v46, 0, _lastLine) == 1 && v46 <= _lastLine)
			break;
		v47 = 0;
		v46 = -1;
		++v12;
		--v13;
	}
	int v39 = v12;
	int v15 = 0;
	int v16 = v41;
	for (int k = v41; v41 + 200 > v16; k = v16) {
		if (checkCollisionLine(k, v40, &v45, &v44, 0, _lastLine) == 1 && v44 <= _lastLine)
			break;
		v45 = 0;
		v44 = -1;
		++v15;
		++v16;
	}
	int v38 = v15;
	int v18 = 0;
	int v19 = v41;
	for (int l = v41; v41 - 200 < v19; l = v19) {
		if (checkCollisionLine(l, v40, &v43, &v42, 0, _lastLine) == 1 && v42 <= _lastLine)
			break;
		v43 = 0;
		v42 = -1;
		++v18;
		--v19;
	}
	if (a4 > v50) {
		if (v46 != -1 && v46 <= v50)
			v46 = -1;
		if (v44 != -1 && v50 >= v44)
			v44 = -1;
		if (v48 != -1 && v50 >= v48)
			v48 = -1;
		if (v42 != -1 && v50 >= v42)
			v42 = -1;
		if (v46 != -1 && a4 < v46)
			v46 = -1;
		if (v44 != -1 && a4 < v44)
			v44 = -1;
		if (v48 != -1 && a4 < v48)
			v48 = -1;
		if (v42 != -1 && a4 < v42)
			v42 = -1;
	} else if (a4 < v50) {
		if (v46 != -1 && v46 >= v50)
			v46 = -1;
		if (v44 != -1 && v50 <= v44)
			v44 = -1;
		if (v48 != -1 && v50 <= v48)
			v48 = -1;
		if (v42 != -1 && v50 <= v42)
			v42 = -1;
		if (v46 != -1 && a4 > v46)
			v46 = -1;
		if (v44 != -1 && a4 > v44)
			v44 = -1;
		if (v48 != -1 && a4 > v48)
			v48 = -1;
		if (v42 != -1 && a4 > v42)
			v42 = -1;
	}
	if (v46 != -1 || v44 != -1 || v48 != -1 || v42 != -1) {
		Directions newDir = DIR_NONE;
		if (a4 > v50) {
			if (v48 <= v46 && v44 <= v46 && v42 <= v46 && v46 > v50)
				newDir = DIR_UP;
			if (v48 <= v44 && v46 <= v44 && v42 <= v44 && v50 < v44)
				newDir = DIR_RIGHT;
			if (v46 <= v48 && v44 <= v48 && v42 <= v48 && v50 < v48)
				newDir = DIR_DOWN;
			if (v48 <= v42 && v44 <= v42 && v46 <= v42 && v50 < v42)
				newDir = DIR_LEFT;
		} else if (a4 < v50) {
			if (v46 == -1)
				v46 = 1300;
			if (v44 == -1)
				v44 = 1300;
			if (v48 == -1)
				v48 = 1300;
			if (v42 == -1)
				v42 = 1300;
			if (v46 != 1300 && v48 >= v46 && v44 >= v46 && v42 >= v46 && v46 < v50)
				newDir = DIR_UP;
			if (v44 != 1300 && v48 >= v44 && v46 >= v44 && v42 >= v44 && v50 > v44)
				newDir = DIR_RIGHT;
			if (v48 != 1300 && v46 >= v48 && v44 >= v48 && v42 >= v48 && v50 > v48)
				newDir = DIR_DOWN;
			if (v42 != 1300 && v48 >= v42 && v44 >= v42 && v46 >= v42 && v50 > v42)
				newDir = DIR_LEFT;
		}

		switch(newDir) {
		case DIR_UP:
			v36 = v46;
			v35 = v47;
			for (int v22 = 0; v22 < v39; v22++) {
				if (checkCollisionLine(v41, v40 - v22, &v47, &v46, _lastLine + 1, _linesNumb) && _lastLine < v46) {
					int v23 = GENIAL(v46, v47, v41, v40 - v22, v41, v40 - v39, v7, &_bestRoute[0]);
					if (v23 == -1)
						return false;
					v7 = v23;
					if (_newPosY != -1)
						v22 = _newPosY - v40;
				}
				_bestRoute[v7].set(v41, v40 - v22, DIR_UP);
				v7++;
			}
			_newLineIdx = v36;
			_newLineDataIdx = v35;
			_newRouteIdx = v7;
			return true;
			break;
		case DIR_RIGHT:
			v36 = v44;
			v35 = v45;
			for (int v31 = 0; v31 < v38; v31++) {
				if (checkCollisionLine(v31 + v41, v40, &v47, &v46, _lastLine + 1, _linesNumb) && _lastLine < v46) {
					int v32 = GENIAL(v46, v47, v31 + v41, v40, v38 + v41, v40, v7, &_bestRoute[0]);
					if (v32 == -1)
						return false;
					v7 = v32;
					if (_newPosX != -1)
						v31 = _newPosX - v41;
				}
				_bestRoute[v7].set(v31 + v41, v40, DIR_RIGHT);
				v7++;
			}
			_newLineIdx = v36;
			_newLineDataIdx = v35;
			_newRouteIdx = v7;
			return true;
			break;
		case DIR_DOWN:
			v36 = v48;
			v35 = v49;
			for (int v25 = 0; v25 < v37; v25++) {
				if (checkCollisionLine(v41, v25 + v40, &v47, &v46, _lastLine + 1, _linesNumb) && _lastLine < v46) {
					int v26 = GENIAL(v46, v47, v41, v25 + v40, v41, v37 + v40, v7, &_bestRoute[0]);
					if (v26 == -1)
						return false;
					v7 = v26;
					if (_newPosY != -1)
						v25 = v40 - _newPosY;
				}
				_bestRoute[v7].set(v41, v25 + v40, DIR_DOWN);
				v7++;
			}
			_newLineIdx = v36;
			_newLineDataIdx = v35;
			_newRouteIdx = v7;
			return true;
			break;
		case DIR_LEFT:
			v36 = v42;
			v35 = v43;
			for (int v28 = 0; v28 < v18; v28++) {
				if (checkCollisionLine(v41 - v28, v40, &v47, &v46, _lastLine + 1, _linesNumb) && _lastLine < v46) {
					int v29 = GENIAL(v46, v47, v41 - v28, v40, v41 - v18, v40, v7, &_bestRoute[0]);
					if (v29 == -1)
						return false;
					v7 = v29;
					if (_newPosX != -1)
						v28 = v41 - _newPosX;
				}
				_bestRoute[v7].set(v41 - v28, v40, DIR_LEFT);
				v7++;
			}
			_newLineIdx = v36;
			_newLineDataIdx = v35;
			_newRouteIdx = v7;
			return true;
			break;
		default:
			break;
		}
	}
	return false;
}

int LinesManager::GENIAL(int lineIdx, int dataIdx, int a3, int a4, int a5, int a6, int a7, RouteItem *route) {
	int result = a7;
	int v80 = -1;
	++_pathFindingMaxDepth;
	if (_pathFindingMaxDepth > 10) {
		warning("PathFinding - Max depth reached");
		route[a7].invalidate();
		return -1;
	}
	int16 *v10 = _lineItem[lineIdx]._lineData;
	int v98 = v10[0];
	int v97 = v10[1];
	int v92 = lineIdx;

	int v65;
	bool loopCond = false;
	for (;;) {
		int v86 = v92 - 1;
		int v11 = 2 * _lineItem[v92 - 1]._lineDataEndIdx;

		int16 *v12 = _lineItem[v92 - 1]._lineData;
		if (v12 == (int16 *)g_PTRNUL)
			break;
		while (v12[v11 - 2] != v98 || v97 != v12[v11 - 1]) {
			--v86;
			if (_lastLine - 1 != v86) {
				v11 = 2 * _lineItem[v86]._lineDataEndIdx;
				v12 = _lineItem[v86]._lineData;
				if (v12 != (int16 *)g_PTRNUL)
					continue;
			}
			loopCond = true;
			break;
		}
		if (loopCond)
			break;

		v92 = v86;
		v98 = v12[0];
		v97 = v12[1];
	}

	int16 *v13 = _lineItem[lineIdx]._lineData;
	int v95 = v13[2 * _lineItem[lineIdx]._lineDataEndIdx - 2];
	int v93 = v13[2 * _lineItem[lineIdx]._lineDataEndIdx - 1];
	int v91 = lineIdx;
	int foundLineIdx, foundDataIdx;
	loopCond = false;
	for (;;) {
		int v87 = v91 + 1;
		int v15 = 2 * _lineItem[v91 + 1]._lineDataEndIdx;
		int16 *v16 = _lineItem[v91 + 1]._lineData;
		if (v16 == (int16 *)g_PTRNUL)
			break;
		int v17;
		for (;;) {
			v65 = v15;
			v17 = v16[v15 - 2];
			if (v16[0] == v95 && v93 == v16[1])
				break;

			++v87;
			if (v87 != _linesNumb + 1) {
				v15 = 2 * _lineItem[v87]._lineDataEndIdx;
				v16 = _lineItem[v87]._lineData;
				if (v16 != (int16 *)g_PTRNUL)
					continue;
			}
			loopCond = true;
			break;
		}
		if (loopCond)
			break;

		v91 = v87;
		v95 = v17;
		v93 = v16[v65 - 1];
	}

	int v58 = abs(a3 - a5) + 1;
	int v85 = abs(a4 - a6) + 1;
	int v20 = v85;
	if (v58 > v20)
		v85 = v58;
	int v84 = 1000 * v58 / v85;
	int v83 = 1000 * v20 / v85;
	int v21 = 1000 * a3;
	int v22 = 1000 * a4;
	int v82 = a3;
	int v81 = a4;
	if (a5 < a3)
		v84 = -v84;
	if (a6 < a4)
		v83 = -v83;
	if (v85 > 800)
		v85 = 800;

	Common::fill(&_lineBuf[0], &_lineBuf[1000], 0);
	int bugLigIdx = 0;
	for (int v88 = 0; v88 < v85 + 1; v88++) {
		_lineBuf[bugLigIdx] = v82;
		_lineBuf[bugLigIdx + 1] = v81;
		v21 += v84;
		v22 += v83;
		v82 = v21 / 1000;
		v81 = v22 / 1000;
		bugLigIdx += 2;
	}
	bugLigIdx -= 2;
	int v77 = 0;
	int v78 = 0;
	int v79 = 0;
	for (int v89 = v85 + 1; v89 > 0; v89--) {
		if (checkCollisionLine(_lineBuf[bugLigIdx], _lineBuf[bugLigIdx + 1], &foundDataIdx, &foundLineIdx, v92, v91) && _lastLine < foundLineIdx) {
			v80 = foundLineIdx;
			v77 = foundDataIdx;
			v78 = _lineBuf[bugLigIdx];
			v79 = _lineBuf[bugLigIdx + 1];
			break;
		}
		bugLigIdx -= 2;
	}
	int v66 = 0;
	int v68 = 0;
	int v70 = 0;
	int v72 = 0;
	for (int i = v92; i <= v91; ++i) {
		int16 *lineData = _lineItem[i]._lineData;
		if (lineData == (int16 *)g_PTRNUL)
			error("error in genial routine");
		if (i == v92) {
			v72 = lineData[2 * _lineItem[i]._lineDataEndIdx - 1];
			if (lineData[1] <= lineData[2 * _lineItem[i]._lineDataEndIdx - 1])
				v72 = lineData[1];
			v70 = lineData[2 * _lineItem[i]._lineDataEndIdx - 1];
			if (lineData[1] >= lineData[2 * _lineItem[i]._lineDataEndIdx - 1])
				v70 = lineData[1];
			v68 = lineData[2 * _lineItem[i]._lineDataEndIdx - 2];
			if (lineData[0] <= lineData[2 * _lineItem[i]._lineDataEndIdx - 2])
				v68 = lineData[0];
			v66 = lineData[2 * _lineItem[i]._lineDataEndIdx - 2];
			if (lineData[0] >= lineData[2 * _lineItem[i]._lineDataEndIdx - 2])
				v66 = lineData[0];
		} else {
			if (lineData[1] < lineData[2 * _lineItem[i]._lineDataEndIdx - 1] && lineData[1] < v72)
				v72 = lineData[1];
			if (lineData[2 * _lineItem[i]._lineDataEndIdx - 1] < lineData[1] && lineData[2 * _lineItem[i]._lineDataEndIdx - 1] < v72)
				v72 = lineData[2 * _lineItem[i]._lineDataEndIdx - 1];
			if (lineData[1] > lineData[2 * _lineItem[i]._lineDataEndIdx - 1] && lineData[1] > v70)
				v70 = lineData[1];
			if (lineData[2 * _lineItem[i]._lineDataEndIdx - 1] > lineData[1] && lineData[2 * _lineItem[i]._lineDataEndIdx - 1] > v70)
				v70 = lineData[2 * _lineItem[i]._lineDataEndIdx - 1];
			if (lineData[0] < lineData[2 * _lineItem[i]._lineDataEndIdx - 2] && v68 > lineData[0])
				v68 = lineData[0];
			if (lineData[2 * _lineItem[i]._lineDataEndIdx - 2] < lineData[0] && v68 > lineData[2 * _lineItem[i]._lineDataEndIdx - 2])
				v68 = lineData[2 * _lineItem[i]._lineDataEndIdx - 2];
			if (lineData[0] > lineData[2 * _lineItem[i]._lineDataEndIdx - 2] && v66 < lineData[0])
				v66 = lineData[0];
			if (lineData[2 * _lineItem[i]._lineDataEndIdx - 2] > lineData[0] && v66 < lineData[2 * _lineItem[i]._lineDataEndIdx - 2])
				v66 = lineData[2 * _lineItem[i]._lineDataEndIdx - 2];
		}
	}
	int v69 = v68 - 2;
	int v73 = v72 - 2;
	int v67 = v66 + 2;
	int v71 = v70 + 2;
	if (a5 >= v69 && a5 <= v67 && a6 >= v73 && a6 <= v71) {
		int v34 = a6;
		int v76 = -1;
		for (;;) {
			--v34;
			if (!checkCollisionLine(a5, v34, &foundDataIdx, &foundLineIdx, v92, v91))
				break;

			v76 = foundLineIdx;
			if (!v34 || v73 > v34)
				break;
		}
		int v35 = a6;
		int v75 = -1;
		for (;;) {
			++v35;
			if (!checkCollisionLine(a5, v35, &foundDataIdx, &foundLineIdx, v92, v91))
				break;

			v75 = foundLineIdx;
			if (_vm->_globals._characterMaxPosY <= v35 || v71 <= v35)
				break;
		}
		int v36 = a5;
		int v74 = -1;
		for (;;) {
			++v36;
			if (!checkCollisionLine(v36, a6, &foundDataIdx, &foundLineIdx, v92, v91))
				break;

			v74 = foundLineIdx;

			if (_vm->_graphicsManager._maxX <= v36 || v67 <= v36)
				break;
		}
		int v37 = a5;
		int v38 = -1;
		for(;;) {
			--v37;
			if (!checkCollisionLine(v37, a6, &foundDataIdx, &foundLineIdx, v92, v91))
				break;
			v38 = foundLineIdx;
			if (v37 <= 0 || v69 >= v37)
				break;;
		}
		if (v74 != -1 && v38 != -1 && v76 != -1 && v75 != -1) {
			route[a7].invalidate();
			return -1;
		}
	}
	if (v78 < a3 - 1 || v78 > a3 + 1 || v79 < a4 - 1 || v79 > a4 + 1) {
		_newPosX = v78;
		_newPosY = v79;
		if (lineIdx < v80) {
			int v43 = 0;
			int v42 = lineIdx;
			do {
				if (v42 == v92 - 1)
					v42 = v91;
				++v43;
				--v42;
				if (v42 == v92 - 1)
					v42 = v91;
			} while (v80 != v42);
			if (abs(v80 - lineIdx) == v43) {
				if (dataIdx >  abs(_lineItem[lineIdx]._lineDataEndIdx / 2)) {
					result = CONTOURNE(lineIdx, dataIdx, a7, v80, v77, route);
				} else {
					result = CONTOURNE1(lineIdx, dataIdx, a7, v80, v77, route, v92, v91);
				}
			}
			if (abs(v80 - lineIdx) < v43)
				result = CONTOURNE(lineIdx, dataIdx, result, v80, v77, route);
			if (v43 < abs(v80 - lineIdx))
				result = CONTOURNE1(lineIdx, dataIdx, result, v80, v77, route, v92, v91);
		}
		if (lineIdx > v80) {
			int v45 = abs(lineIdx - v80);
			int v47 = lineIdx;
			int v48 = 0;
			do {
				if (v47 == v91 + 1)
					v47 = v92;
				++v48;
				++v47;
				if (v47 == v91 + 1)
					v47 = v92;
			} while (v80 != v47);
			if (v45 == v48) {
				if (dataIdx > abs(_lineItem[lineIdx]._lineDataEndIdx / 2)) {
					result = CONTOURNE1(lineIdx, dataIdx, result, v80, v77, route, v92, v91);
				} else {
					result = CONTOURNE(lineIdx, dataIdx, result, v80, v77, route);
				}
			}
			if (v45 < v48)
				result = CONTOURNE(lineIdx, dataIdx, result, v80, v77, route);
			if (v48 < v45)
				result = CONTOURNE1(lineIdx, dataIdx, result, v80, v77, route, v92, v91);
		}
		if (lineIdx == v80)
			result = CONTOURNE(lineIdx, dataIdx, result, lineIdx, v77, route);
		for(;;) {
			if (!checkCollisionLine(_newPosX, _newPosY, &foundDataIdx, &foundLineIdx, _lastLine + 1, _linesNumb))
				break;

			switch (_lineItem[foundLineIdx]._direction) {
			case DIR_UP:
				--_newPosY;
				break;
			case DIR_UP_RIGHT:
				--_newPosY;
				++_newPosX;
				break;
			case DIR_RIGHT:
				++_newPosX;
				break;
			case DIR_DOWN_RIGHT:
				++_newPosY;
				++_newPosX;
				break;
			case DIR_DOWN:
				++_newPosY;
				break;
			case DIR_DOWN_LEFT:
				++_newPosY;
				--_newPosX;
				break;
			case DIR_LEFT:
				--_newPosX;
				break;
			case DIR_UP_LEFT:
				--_newPosY;
				--_newPosX;
				break;
			default:
				break;
			}
		}
	} else {
		_newPosX = -1;
		_newPosY = -1;
	}
	return result;
}

// Avoid 2
RouteItem *LinesManager::PARCOURS2(int fromX, int fromY, int destX, int destY) {
	int foundLineIdx;
	int foundDataIdx;
	int curLineY = 0;
	int curLineX = 0;
	int v126[9];
	int v131[9];
	int collLineDataIdxArr[9];
	int collLineIdxArr[9];

	int clipDestX = destX;
	int clipDestY = destY;
	int curLineIdx = 0;
	int curLineDataIdx = 0;
	int lineIdx = 0;
	int lineDataIdx = 0;
	Directions newDir = DIR_NONE;
	int v111 = 0;
	if (destY <= 24)
		clipDestY = 25;
	if (!_vm->_globals._checkDistanceFl) {
		if (abs(fromX - _vm->_globals._oldRouteFromX) <= 4 && abs(fromY - _vm->_globals._oldRouteFromY) <= 4 &&
		    abs(_vm->_globals._oldRouteDestX - destX) <= 4 && abs(_vm->_globals._oldRouteDestY - clipDestY) <= 4)
			return (RouteItem *)g_PTRNUL;

		if (abs(fromX - destX) <= 4 && abs(fromY - clipDestY) <= 4)
			return (RouteItem *)g_PTRNUL;

		if (_vm->_globals._oldZoneNum > 0 && _vm->_objectsManager._zoneNum > 0 && _vm->_globals._oldZoneNum == _vm->_objectsManager._zoneNum)
			return (RouteItem *)g_PTRNUL;
	}
	_vm->_globals._checkDistanceFl = false;
	_vm->_globals._oldZoneNum = _vm->_objectsManager._zoneNum;
	_vm->_globals._oldRouteFromX = fromX;
	_vm->_globals._oldRouteDestX = destX;
	_vm->_globals._oldRouteFromY = fromY;
	_vm->_globals._oldRouteDestY = clipDestY;
	_pathFindingMaxDepth = 0;
	int routeIdx = 0;
	if (destX <= 19)
		clipDestX = 20;
	if (clipDestY <= 19)
		clipDestY = 20;
	if (clipDestX > _vm->_graphicsManager._maxX - 10)
		clipDestX = _vm->_graphicsManager._maxX - 10;
	if (clipDestY > _vm->_globals._characterMaxPosY)
		clipDestY = _vm->_globals._characterMaxPosY;

	if (abs(fromX - clipDestX) <= 3 && abs(fromY - clipDestY) <= 3)
		return (RouteItem *)g_PTRNUL;

	for (int i = 0; i <= 8; ++i) {
		collLineIdxArr[i] = -1;
		collLineDataIdxArr[i] = 0;
		v131[i] = 1300;
		v126[i] = 1300;
	}

	if (characterRoute(fromX, fromY, clipDestX, clipDestY, -1, -1, 0) == 1)
		return _bestRoute;

	int v14 = 0;
	for (int tmpY = clipDestY; tmpY < _vm->_graphicsManager._maxY; tmpY++, v14++) { 
		if (checkCollisionLine(clipDestX, tmpY, &collLineDataIdxArr[5], &collLineIdxArr[5], 0, _lastLine) && collLineIdxArr[5] <= _lastLine)
			break;
		collLineDataIdxArr[5] = 0;
		collLineIdxArr[5] = -1;
	}
	v131[5] = v14;

	v14 = 0;
	for (int tmpY = clipDestY; tmpY > _vm->_graphicsManager._minY; tmpY--, v14++) {
		if (checkCollisionLine(clipDestX, tmpY, &collLineDataIdxArr[1], &collLineIdxArr[1], 0, _lastLine) && collLineIdxArr[1] <= _lastLine)
			break;
		collLineDataIdxArr[1] = 0;
		collLineIdxArr[1] = -1;
		if (v131[5] < v14 && collLineIdxArr[5] != -1)
			break;
	}
	v131[1] = v14;

	v14 = 0;
	for (int tmpX = clipDestX; tmpX < _vm->_graphicsManager._maxX; tmpX++) {
		if (checkCollisionLine(tmpX, clipDestY, &collLineDataIdxArr[3], &collLineIdxArr[3], 0, _lastLine) && collLineIdxArr[3] <= _lastLine)
			break;
		collLineDataIdxArr[3] = 0;
		collLineIdxArr[3] = -1;
		++v14;
		if (v131[1] < v14 && collLineIdxArr[1] != -1)
				break;
		if (v131[5] < v14 && collLineIdxArr[5] != -1)
			break;
	}
	v131[3] = v14;

	v14 = 0;
	for (int tmpX = clipDestX; tmpX > _vm->_graphicsManager._minX; tmpX--) {
		if (checkCollisionLine(tmpX, clipDestY, &collLineDataIdxArr[7], &collLineIdxArr[7], 0, _lastLine) && collLineIdxArr[7] <= _lastLine)
			break;
		collLineDataIdxArr[7] = 0;
		collLineIdxArr[7] = -1;
		++v14;
		if (v131[1] < v14 && collLineIdxArr[1] != -1)
			break;
		if (v131[5] < v14 && collLineIdxArr[5] != -1)
			break;
		if (v131[3] < v14 && collLineIdxArr[3] != -1)
			break;
	}
	v131[7] = v14;

	if (collLineIdxArr[1] < 0 || _lastLine < collLineIdxArr[1])
		collLineIdxArr[1] = -1;
	if (collLineIdxArr[3] < 0 || _lastLine < collLineIdxArr[3])
		collLineIdxArr[3] = -1;
	if (collLineIdxArr[5] < 0 || _lastLine < collLineIdxArr[5])
		collLineIdxArr[5] = -1;
	if (collLineIdxArr[7] < 0 || _lastLine < collLineIdxArr[7])
		collLineIdxArr[7] = -1;
	if (collLineIdxArr[1] < 0)
		v131[1] = 1300;
	if (collLineIdxArr[3] < 0)
		v131[3] = 1300;
	if (collLineIdxArr[5] < 0)
		v131[5] = 1300;
	if (collLineIdxArr[7] < 0)
		v131[7] = 1300;
	if (collLineIdxArr[1] == -1 && collLineIdxArr[3] == -1 && collLineIdxArr[5] == -1 && collLineIdxArr[7] == -1)
		return (RouteItem *)g_PTRNUL;

	if (collLineIdxArr[5] != -1 && v131[1] >= v131[5] && v131[3] >= v131[5] && v131[7] >= v131[5]) {
		curLineIdx = collLineIdxArr[5];
		curLineDataIdx = collLineDataIdxArr[5];
	} else if (collLineIdxArr[1] != -1 && v131[5] >= v131[1] && v131[3] >= v131[1] && v131[7] >= v131[1]) {
		curLineIdx = collLineIdxArr[1];
		curLineDataIdx = collLineDataIdxArr[1];
	} else if (collLineIdxArr[3] != -1 && v131[1] >= v131[3] && v131[5] >= v131[3] && v131[7] >= v131[3]) {
		curLineIdx = collLineIdxArr[3];
		curLineDataIdx = collLineDataIdxArr[3];
	} else if (collLineIdxArr[7] != -1 && v131[5] >= v131[7] && v131[3] >= v131[7] && v131[1] >= v131[7]) {
		curLineIdx = collLineIdxArr[7];
		curLineDataIdx = collLineDataIdxArr[7];
	}

	for (int i = 0; i <= 8; ++i) {
		collLineIdxArr[i] = -1;
		collLineDataIdxArr[i] = 0;
		v131[i] = 1300;
		v126[i] = 1300;
	}

	v14 = 0;
	for (int tmpY = fromY; tmpY < _vm->_graphicsManager._maxY; tmpY++, v14++) {
		if (checkCollisionLine(fromX, tmpY, &collLineDataIdxArr[5], &collLineIdxArr[5], 0, _lastLine) && collLineIdxArr[5] <= _lastLine)
			break;
		collLineDataIdxArr[5] = 0;
		collLineIdxArr[5] = -1;
	}
	v131[5] = v14 + 1;

	v14 = 0;
	for (int tmpY = fromY; tmpY > _vm->_graphicsManager._minY; tmpY--) {
		if (checkCollisionLine(fromX, tmpY, &collLineDataIdxArr[1], &collLineIdxArr[1], 0, _lastLine) && collLineIdxArr[1] <= _lastLine)
			break;
		collLineDataIdxArr[1] = 0;
		collLineIdxArr[1] = -1;
		++v14;
		if (collLineIdxArr[5] != -1 && v14 > 80)
			break;
	}
	v131[1] = v14 + 1;

	v14 = 0;
	for (int tmpX = fromX; tmpX < _vm->_graphicsManager._maxX; tmpX++) {
		if (checkCollisionLine(tmpX, fromY, &collLineDataIdxArr[3], &collLineIdxArr[3], 0, _lastLine) && collLineIdxArr[3] <= _lastLine)
			break;
		collLineDataIdxArr[3] = 0;
		collLineIdxArr[3] = -1;
		++v14;
		if ((collLineIdxArr[5] != -1 || collLineIdxArr[1] != -1) && (v14 > 100))
			break;
	}
	v131[3] = v14 + 1;

	v14 = 0;
	for (int tmpX = fromX; tmpX > _vm->_graphicsManager._minX; tmpX--) {
		if (checkCollisionLine(tmpX, fromY, &collLineDataIdxArr[7], &collLineIdxArr[7], 0, _lastLine) && collLineIdxArr[7] <= _lastLine)
			break;
		collLineDataIdxArr[7] = 0;
		collLineIdxArr[7] = -1;
		++v14;
		if ((collLineIdxArr[5] != -1 || collLineIdxArr[1] != -1 || collLineIdxArr[3] != -1) && (v14 > 100))
			break;
	}
	v131[7] = v14 + 1;

	if (collLineIdxArr[1] != -1)
		v126[1] = abs(collLineIdxArr[1] - curLineIdx);

	if (collLineIdxArr[3] != -1)
		v126[3] = abs(collLineIdxArr[3] - curLineIdx);

	if (collLineIdxArr[5] != -1)
		v126[5] = abs(collLineIdxArr[5] - curLineIdx);

	if (collLineIdxArr[7] != -1)
		v126[7] = abs(collLineIdxArr[7] - curLineIdx);

	if (collLineIdxArr[1] == -1 && collLineIdxArr[3] == -1 && collLineIdxArr[5] == -1 && collLineIdxArr[7] == -1)
		error("Nearest point not found");

	if (collLineIdxArr[1] != -1 && v126[3] >= v126[1] && v126[5] >= v126[1] && v126[7] >= v126[1]) {
		lineIdx = collLineIdxArr[1];
		v111 = v131[1];
		newDir = DIR_UP;
		lineDataIdx = collLineDataIdxArr[1];
	} else if (collLineIdxArr[5] != -1 && v126[3] >= v126[5] && v126[1] >= v126[5] && v126[7] >= v126[5]) {
		lineIdx = collLineIdxArr[5];
		v111 = v131[5];
		newDir = DIR_DOWN;
		lineDataIdx = collLineDataIdxArr[5];
	} else if (collLineIdxArr[3] != -1 && v126[1] >= v126[3] && v126[5] >= v126[3] && v126[7] >= v126[3]) {
		lineIdx = collLineIdxArr[3];
		v111 = v131[3];
		newDir = DIR_RIGHT;
		lineDataIdx = collLineDataIdxArr[3];
	} else if (collLineIdxArr[7] != -1 && v126[1] >= v126[7] && v126[5] >= v126[7] && v126[3] >= v126[7]) {
		lineIdx = collLineIdxArr[7];
		v111 = v131[7];
		newDir = DIR_LEFT;
		lineDataIdx = collLineDataIdxArr[7];
	}

	int v55 = characterRoute(fromX, fromY, clipDestX, clipDestY, lineIdx, curLineIdx, 0);
	
	if (v55 == 1)
		return _bestRoute;

	if (v55 == 2) {
		lineIdx = _newLineIdx;
		lineDataIdx = _newLineDataIdx;
		routeIdx = _newRouteIdx;
	} else {
		if (newDir == DIR_UP) {
			for (int deltaY = 0; deltaY < v111; deltaY++) {
				if (checkCollisionLine(fromX, fromY - deltaY, &foundDataIdx, &foundLineIdx, _lastLine + 1, _linesNumb) && _lastLine < foundLineIdx) {
					int tmpRouteIdx = GENIAL(foundLineIdx, foundDataIdx, fromX, fromY - deltaY, fromX, fromY - v111, routeIdx, _bestRoute);
					if (tmpRouteIdx == -1) {
						_bestRoute[routeIdx].invalidate();
						return &_bestRoute[0];
					}
					routeIdx = tmpRouteIdx;
					if (_newPosY != -1)
						deltaY = fromY - _newPosY;
				}
				_bestRoute[routeIdx].set(fromX, fromY - deltaY, DIR_UP);
				routeIdx++;
			}
		}
		if (newDir == DIR_DOWN) {
			for (int deltaY = 0; deltaY < v111; deltaY++) {
				if (checkCollisionLine(fromX, deltaY + fromY, &foundDataIdx, &foundLineIdx, _lastLine + 1, _linesNumb)
				        && _lastLine < foundLineIdx) {
					int tmpRouteIdx = GENIAL(foundLineIdx, foundDataIdx, fromX, deltaY + fromY, fromX, v111 + fromY, routeIdx, &_bestRoute[0]);
					if (tmpRouteIdx == -1) {
						_bestRoute[routeIdx].invalidate();
						return &_bestRoute[0];
					}
					routeIdx = tmpRouteIdx;
					if (_newPosY != -1)
						deltaY = _newPosY - fromY;
				}
				_bestRoute[routeIdx].set(fromX, fromY + deltaY, DIR_DOWN);
				routeIdx++;
			}
		}
		if (newDir == DIR_LEFT) {
			for (int deltaX = 0; deltaX < v111; deltaX++) {
				if (checkCollisionLine(fromX - deltaX, fromY, &foundDataIdx, &foundLineIdx, _lastLine + 1, _linesNumb) && _lastLine < foundLineIdx) {
					int tmpRouteIdx = GENIAL(foundLineIdx, foundDataIdx, fromX - deltaX, fromY, fromX - v111, fromY, routeIdx, &_bestRoute[0]);
					if (tmpRouteIdx == -1) {
						_bestRoute[routeIdx].invalidate();
						return &_bestRoute[0];
					}
					routeIdx = tmpRouteIdx;
					if (_newPosX != -1)
						deltaX = fromX - _newPosX;
				}
				_bestRoute[routeIdx].set(fromX - deltaX, fromY, DIR_LEFT);
				routeIdx++;
			}
		}
		if (newDir == DIR_RIGHT) {
			for (int deltaX = 0; deltaX < v111; deltaX++) {
				if (checkCollisionLine(deltaX + fromX, fromY, &foundDataIdx, &foundLineIdx, _lastLine + 1, _linesNumb) && _lastLine < foundLineIdx) {
					int tmpRouteIdx = GENIAL(foundLineIdx, foundDataIdx, deltaX + fromX, fromY, v111 + fromX, fromY, routeIdx, &_bestRoute[0]);
					if (tmpRouteIdx == -1) {
						_bestRoute[routeIdx].invalidate();
						return &_bestRoute[0];
					}
					routeIdx = tmpRouteIdx;
					if (_newPosX != -1)
						deltaX = _newPosX - fromX;
				}
				_bestRoute[routeIdx].set(fromX + deltaX, fromY, DIR_RIGHT);
				routeIdx++;
			}
		}
	}
	

	bool loopCond;
	do {
		loopCond = false;
		if (lineIdx < curLineIdx) {
			for (int i = lineDataIdx; _lineItem[lineIdx]._lineDataEndIdx > i; ++i) {
				curLineX = _lineItem[lineIdx]._lineData[2 * i];
				curLineY = _lineItem[lineIdx]._lineData[2 * i + 1];
				_bestRoute[routeIdx].set(_lineItem[lineIdx]._lineData[2 * i], _lineItem[lineIdx]._lineData[2 * i + 1], _lineItem[lineIdx]._directionRouteInc);
				routeIdx++;
			}
			for (int idx = lineIdx + 1; idx < curLineIdx; idx++) {
				for (int dataIdx = 0; _lineItem[idx]._lineDataEndIdx > dataIdx; dataIdx++) {
					curLineX = _lineItem[idx]._lineData[2 * dataIdx];
					curLineY = _lineItem[idx]._lineData[2 * dataIdx + 1];
					_bestRoute[routeIdx].set(_lineItem[idx]._lineData[2 * dataIdx], _lineItem[idx]._lineData[2 * dataIdx + 1], _lineItem[idx]._directionRouteInc);
					routeIdx++;
					if (_lineItem[idx]._lineDataEndIdx > 30 && dataIdx == _lineItem[idx]._lineDataEndIdx / 2) {
						int v78 = characterRoute(_lineItem[idx]._lineData[2 * dataIdx], _lineItem[idx]._lineData[2 * dataIdx + 1], clipDestX, clipDestY, idx, curLineIdx, routeIdx);
						if (v78 == 1)
							return &_bestRoute[0];
						if (v78 == 2 || MIRACLE(curLineX, curLineY, idx, curLineIdx, routeIdx)) {
							lineIdx = _newLineIdx;
							lineDataIdx = _newLineDataIdx;
							routeIdx = _newRouteIdx;
							loopCond = true;
							break;
						}
					}
				}

				if (loopCond)
					break;

				int v79 = characterRoute(curLineX, curLineY, clipDestX, clipDestY, idx, curLineIdx, routeIdx);
				if (v79 == 1)
					return &_bestRoute[0];
				if (v79 == 2 || MIRACLE(curLineX, curLineY, idx, curLineIdx, routeIdx)) {
					lineIdx = _newLineIdx;
					lineDataIdx = _newLineDataIdx;
					routeIdx = _newRouteIdx;
					loopCond = true;
					break;
				}
			}
			if (loopCond)
				continue;

			lineDataIdx = 0;
			lineIdx = curLineIdx;
		}
		if (lineIdx > curLineIdx) {
			for (int dataIdx = lineDataIdx; dataIdx > 0; dataIdx--) {
				curLineX = _lineItem[lineIdx]._lineData[2 * dataIdx];
				curLineY = _lineItem[lineIdx]._lineData[2 * dataIdx + 1];

				_bestRoute[routeIdx].set(_lineItem[lineIdx]._lineData[2 * dataIdx], _lineItem[lineIdx]._lineData[2 * dataIdx + 1], _lineItem[lineIdx]._directionRouteDec);
				routeIdx++;
			}
			for (int v117 = lineIdx - 1; v117 > curLineIdx; v117--) {
				for (int dataIdx = _lineItem[v117]._lineDataEndIdx - 1; dataIdx > -1; dataIdx--) {
					curLineX = _lineItem[v117]._lineData[2 * dataIdx];
					curLineY = _lineItem[v117]._lineData[2 * dataIdx + 1];
					_bestRoute[routeIdx].set(_lineItem[v117]._lineData[2 * dataIdx], _lineItem[v117]._lineData[2 * dataIdx + 1], _lineItem[v117]._directionRouteDec);
					routeIdx++;
					if (_lineItem[v117]._lineDataEndIdx > 30 && dataIdx == _lineItem[v117]._lineDataEndIdx / 2) {
						int v88 = characterRoute(curLineX, curLineY, clipDestX, clipDestY, v117, curLineIdx, routeIdx);
						if (v88 == 1)
							return &_bestRoute[0];
						if (v88 == 2 || MIRACLE(curLineX, curLineY, v117, curLineIdx, routeIdx)) {
							lineIdx = _newLineIdx;
							lineDataIdx = _newLineDataIdx;
							routeIdx = _newRouteIdx;
							loopCond = true;
							break;
						}
					}
				}

				if (loopCond)
					break;

				int v89 = characterRoute(curLineX, curLineY, clipDestX, clipDestY, v117, curLineIdx, routeIdx);
				if (v89 == 1)
					return &_bestRoute[0];
				if (v89 == 2 || MIRACLE(curLineX, curLineY, v117, curLineIdx, routeIdx)) {
					lineIdx = _newLineIdx;
					lineDataIdx = _newLineDataIdx;
					routeIdx = _newRouteIdx;
					loopCond = true;
					break;
				}
			}

			if (!loopCond) {
				lineDataIdx = _lineItem[curLineIdx]._lineDataEndIdx - 1;
				lineIdx = curLineIdx;
			}
		}
	} while (loopCond);

	if (lineIdx == curLineIdx) {
		if (lineDataIdx <= curLineDataIdx) {
			routeIdx = _lineItem[curLineIdx].appendToRouteInc(lineDataIdx, curLineDataIdx, _bestRoute, routeIdx);
		} else {
			routeIdx = _lineItem[curLineIdx].appendToRouteDec(lineDataIdx, curLineDataIdx, _bestRoute, routeIdx);
		}
	}
	if (characterRoute(_bestRoute[routeIdx - 1]._x, _bestRoute[routeIdx - 1]._y, clipDestX, clipDestY, -1, -1, routeIdx) != 1) {
		_bestRoute[routeIdx].invalidate();
	}

	return &_bestRoute[0];
}

int LinesManager::characterRoute(int fromX, int fromY, int destX, int destY, int a5, int a6, int a7) {
	int v18;
	int v19;
	int v20;
	int v21;
	int v22;
	int v23;
	int v24;
	int v33;
	bool v45;
	int v54;
	int v55;
	Directions newDirection;
	int v92;
	int v93;
	int v94;
	int v95;
	int v96;
	int v97;
	int v98;
	int v99;
	int v100;
	int v101;
	int v102;
	int v103;
	int v104;
	int v105;
	int v106;
	int v107;
	int v108;
	int v109;
	int v111;
	int v114;
	int v115;
	int v117;
	int collLineIdx;
	int collDataIdx = 0;
	int v140;
	int v142;
	bool colResult = false;

	int curX = fromX;
	int curY = fromY;
	int v137 = a7;
	bool v136 = false;
	if (a5 == -1 && a6 == -1)
		v136 = true;
	int foundDataIdx;
	int foundLineIdx = a5;
	if (checkCollisionLine(fromX, fromY, &foundDataIdx, &foundLineIdx, 0, _linesNumb)) {
		switch (_lineItem[foundLineIdx]._direction) {
		case DIR_UP:
			curY -= 2;
			break;
		case DIR_UP_RIGHT:
			curY -= 2;
			curX += 2;
			break;
		case DIR_RIGHT:
			curX += 2;
			break;
		case DIR_DOWN_RIGHT:
			curY += 2;
			curX += 2;
		case DIR_DOWN:
			curY += 2;
			break;
		case DIR_DOWN_LEFT:
			curY += 2;
			curX -= 2;
			break;
		case DIR_LEFT:
			curX -= 2;
			break;
		case DIR_UP_LEFT:
			curY -= 2;
			curX -= 2;
			break;
		default:
			break;
		}
	}
	v98 = curX;
	v97 = curY;
	v115 = 0;
	v142 = -1;
	v140 = -1;
	collLineIdx = -1;

	int distX, v10, distY, v12, v13, v14;
	int repeatFlag = 0;
	int v143 = 0;
	int v141 = 0;
	for (;;) {
		v111 = curX;
		v109 = curY;
		if (destX >= curX - 2 && destX <= curX + 2 && destY >= curY - 2 && destY <= curY + 2) {
			essai0[v115].invalidate();
			goto retLABEL_essai0;
		}
		distX = abs(curX - destX);
		v10 = distX + 1;
		distY = abs(curY - destY);
		v107 = distY + 1;
		if (v10 > v107)
			v107 = v10;
		v12 = v107 - 1;
		assert(v12 != 0);
		v101 = 1000 * v10 / v12;
		v99 = 1000 * (distY + 1) / v12;
		if (destX < curX)
			v101 = -v101;
		if (destY < curY)
			v99 = -v99;
		v13 = (int16)v101 / 1000;
		v94 = (int16)v99 / 1000;
		newDirection = DIR_NONE;
		if (v94 == -1 && (v101 >= 0 && v101 <= 150))
			newDirection = DIR_UP;
		if (v13 == 1 && (v99 >= -1 && v99 <= 150))
			newDirection = DIR_RIGHT;
		if (v94 == 1 && (v101 >= -150 && v101 <= 150))
			newDirection = DIR_DOWN;
		if (v13 == -1 && (v99 >= -150 && v99 <= 150))
			newDirection = DIR_LEFT;
		if (v94 == -1 && (v101 >= -150 && v101 <= 0))
			newDirection = DIR_UP;

		if (newDirection == DIR_NONE && !checkSmoothMove(curX, v109, destX, destY) && !makeSmoothMove(curX, v109, destX, destY)) {
			newDirection = _smoothMoveDirection;
			v14 = 0;
			for (v14 = 0; _smoothRoute[v14]._posX != -1 && _smoothRoute[v14]._posY != -1; ++v14) {
				if (checkCollisionLine(_smoothRoute[v14]._posX, _smoothRoute[v14]._posY, &v143, &v142, 0, _linesNumb)) {
					if (v142 > _lastLine)
						v142 = -1;
					break;
				}

				essai0[v115].set(_smoothRoute[v14]._posX, _smoothRoute[v14]._posY, newDirection);
				v115++;

				if (repeatFlag == 1) {
					repeatFlag = 2;
					break;
				}
			}

			if (repeatFlag != 2 && _smoothRoute[v14]._posX != -1 && _smoothRoute[v14]._posY != -1)
				break;

			repeatFlag = 1;
			v18 = v14 - 1;
			v111 = _smoothRoute[v18]._posX;
			v109 = _smoothRoute[v18]._posY;
		}
		v19 = abs(v111 - destX);
		v20 = v19 + 1;
		v95 = abs(v109 - destY);
		v108 = v95 + 1;
		if (v20 > (v95 + 1))
			v108 = v20;
		if (v108 <= 10) {
			essai0[v115].invalidate();
			goto retLABEL_essai0;
		}
		v21 = v108 - 1;
		v102 = 1000 * v20 / v21;
		v100 = 1000 * (v95 + 1) / v21;
		if (destX < v111)
			v102 = -v102;
		if (destY < v109)
			v100 = -v100;
		v22 = v102 / 1000;
		v96 = v100 / 1000;
		v106 = 1000 * v111;
		v105 = 1000 * v109;
		v104 = 1000 * v111 / 1000;
		v103 = v105 / 1000;
		if (!(v102 / 1000) && v96 == -1)
			newDirection = DIR_UP;
		if (v22 == 1) {
			if (v96 == -1)
				newDirection = DIR_UP_RIGHT;
			if (!v96)
				newDirection = DIR_RIGHT;
			if (v96 == 1)
				newDirection = DIR_DOWN_RIGHT;
		}
		if (!v22 && v96 == 1)
			newDirection = DIR_DOWN;
		if ((v22 != -1) && (v96 == -1)) {
			if (v102 >= 0 && v102 < 510)
				newDirection = DIR_UP;
			else if (v102 >= 510 && v102 <= 1000)
				newDirection = DIR_UP_RIGHT;
		} else {
			if (v96 == 1)
				newDirection = DIR_DOWN_LEFT;
			else if (!v96)
				newDirection = DIR_LEFT;
			else if (v96 == -1) {
				if (v102 >= 0 && v102 < 510)
					newDirection = DIR_UP;
				else if (v102 >= 510 && v102 <= 1000)
					newDirection = DIR_UP_RIGHT;
				else 
					newDirection = DIR_UP_LEFT;
			}
		}
		if (v22 == 1) {
			if (v100 >= -1000 && v100 <= -510)
				newDirection = DIR_UP_RIGHT;
			if (v100 >= -510 && v100 <= 510)
				newDirection = DIR_RIGHT;
			if (v100 >= 510 && v100 <= 1000)
				newDirection = DIR_DOWN_RIGHT;
		}
		if (v96 == 1) {
			if (v102 >= 510 && v102 <= 1000)
				newDirection = DIR_DOWN_RIGHT;
			if (v102 >= -510 && v102 <= 510)
				newDirection = DIR_DOWN;
			if (v102 >= -1000 && v102 <= -510)
				newDirection = DIR_DOWN_LEFT;
		}
		if (v22 == -1) {
			if (v100 >= 510 && v100 <= 1000)
				newDirection = DIR_DOWN_LEFT;
			if (v100 >= -510 && v100 <= 510)
				newDirection = DIR_LEFT;
			if (v100 >= -1000 && v100 <= -510)
				newDirection = DIR_UP_LEFT;
		}
		if (v96 == -1) {
			if (v102 >= -1000 && v102 <= -510)
				newDirection = DIR_UP_LEFT;
			if (v102 >= -510 && v102 <= 0)
				newDirection = DIR_UP;
		}
		v23 = 0;
		if (v108 + 1 <= 0) {
			essai0[v115].invalidate();
			goto retLABEL_essai0;
		}
		while (!checkCollisionLine(v104, v103, &v143, &v142, 0, _linesNumb)) {
			essai0[v115].set(v104, v103, newDirection);
			v106 += v102;
			v105 += v100;
			v104 = v106 / 1000;
			v103 = v105 / 1000;
			v115++;
			++v23;
			if (v23 >= v108 + 1) {
				essai0[v115].invalidate();
				goto retLABEL_essai0;
			}
		}
		if (_lastLine >= v142)
			break;
		v24 = GENIAL(v142, v143, v104, v103, destX, destY, v115, essai0);
		if (v24 == -1)
			goto retLABEL_essai0;
		v115 = v24;
		if (_newPosX != -1 || _newPosY != -1) {
			v142 = -1;
			break;
		}
		curX = -1;
		curY = -1;
	}

	essai0[v115].invalidate();

	v117 = 0;
	v33 = v98;
	v92 = v97;

	while (true) {

		if (destX >= v33 - 2 && destX <= v33 + 2 && destY >= v92 - 2 && destY <= v92 + 2) {
			essai1[v117].invalidate();
			goto retLABEL_essai1;
		}
		while (v33 != destX) {
			if (checkCollisionLine(v33, v92, &v141, &v140, 0, _linesNumb)) {
				if (v140 > _lastLine)
					v140 = -1;
				break;
			}

			if (v33 < destX)
				essai1[v117++].set(v33++, v92, DIR_RIGHT);
			else
				essai1[v117++].set(v33--, v92, DIR_LEFT);
		}
		if (v33 != destX)
			break;

		int v43 = v92;
		while (v43 != destY) {
			if (checkCollisionLine(destX, v43, &v141, &v140, 0, _linesNumb)) {
				if (v140 <= _lastLine)
					break;

				int v44 = GENIAL(v140, v141, destX, v43, destX, destY, v117, essai1);
				if (v44 == -1)
					goto retLABEL_essai1;
				v117 = v44;
				if (_newPosX != -1 && _newPosY != -1)
					break;
			}

			if (v43 < destY)
				essai1[v117++].set(destX, v43++, DIR_DOWN);
			else
				essai1[v117++].set(destX, v43--, DIR_UP);
		}
		if (v43 == destY) {
			essai1[v117].invalidate();
			goto retLABEL_essai1;
		}
		if (v140 <= _lastLine)
			break;
		v33 = _newPosX;
		v92 = _newPosY;
		v45 = checkCollisionLine(_newPosX, _newPosY, &v141, &v140, 0, _lastLine);
		if (v45 && v140 <= _lastLine)
			break;
	}

	essai1[v117].invalidate();
	v117 = 0;
	v54 = v98;
	v93 = v97;
	while (true) {
		int v61;
		v114 = v54;
		if (destX >= v54 - 2 && destX <= v54 + 2 && destY >= v93 - 2 && destY <= v93 + 2) {
			essai2[v117].invalidate();
			goto retLABEL_essai2;
		}

		v55 = v93;
		while (v55 != destY) {
			if (checkCollisionLine(v114, v55, &collDataIdx, &collLineIdx, 0, _linesNumb)) {
				if (collLineIdx > _lastLine)
					collLineIdx = -1;
				break;
			}

			if (v55 < destY)
				essai2[v117++].set(v114, v55++, DIR_DOWN);
			else
				essai2[v117++].set(v114, v55--, DIR_UP);
		}
		if (v55 != destY)
			break;

		v61 = v114;
		while (v61 != destX) {
			if (checkCollisionLine(v61, destY, &collDataIdx, &collLineIdx, 0, _linesNumb)) {
				if (collLineIdx <= _lastLine)
					break;

				int v62 = GENIAL(collLineIdx, collDataIdx, v61, destY, destX, destY, v117, essai2);
				if (v62 == -1) {
					// CHECKME: This goto was to retLABEL_essai1... 
					goto retLABEL_essai2;
				}
				v117 = v62;
				if (_newPosX != -1 && _newPosY != -1)
					break;
			}

			if (v61 < destX)
				essai2[v117++].set(v61++, destY, DIR_RIGHT);
			else
				essai2[v117++].set(v61--, destY, DIR_LEFT);
		}
		if (v61 == destX) {
			collLineIdx = -1;
			essai2[v117].invalidate();
			goto retLABEL_essai2;
		}
		if (collLineIdx <= _lastLine)
			break;

		v54 = _newPosX;
		v93 = _newPosY;
		colResult = checkCollisionLine(_newPosX, _newPosY, &collDataIdx, &collLineIdx, 0, _lastLine);
		if (colResult && collLineIdx <= _lastLine)
			break;
	}

	essai2[v117].invalidate();

	if (!v136) {
		if (a6 > foundLineIdx) {
			if (essai0[0]._x != -1 && v142 > foundLineIdx && v140 <= v142 && collLineIdx <= v142 && a6 >= v142) {
				_newLineIdx = v142;
				_newLineDataIdx = v143;
				int i = 0;
				do {
					assert(v137 <= 8000);
					_bestRoute[v137++] = essai0[i++];
				} while (essai0[i].isValid());
				_newRouteIdx = v137;
				return 2;
			}
			if (essai1[0]._x != -1 && foundLineIdx < v140 && collLineIdx <= v140 && v142 <= v140 && a6 >= v140) {
				_newLineIdx = v140;
				_newLineDataIdx = v141;
				int i = 0;
				do {
					assert(v137 <= 8000);
					_bestRoute[v137++] = essai1[i++];
				} while (essai1[i].isValid());
				_newRouteIdx = v137;
				return 2;
			}
			if (essai2[0]._x != -1 && foundLineIdx < collLineIdx && v140 < collLineIdx && v142 < collLineIdx && a6 >= collLineIdx) {
				_newLineIdx = collLineIdx;
				_newLineDataIdx = collDataIdx;
				int i = 0;
				do {
					assert(v137 <= 8000);
					_bestRoute[v137++] = essai2[i++];
				} while (essai2[i].isValid());
				_newRouteIdx = v137;
				return 2;
			}
		}
		if (a6 < foundLineIdx) {
			if (v142 == -1)
				v142 = 1300;
			if (v140 == -1)
				v142 = 1300;
			if (collLineIdx == -1)
				v142 = 1300;
			if (essai1[0]._x != -1 && v140 < foundLineIdx && collLineIdx >= v140 && v142 >= v140 && a6 <= v140) {
				_newLineIdx = v140;
				_newLineDataIdx = v141;
				int i = 0;
				do {
					assert(v137 <= 8000);
					_bestRoute[v137++] = essai1[i++];
				} while (essai1[i].isValid());
				_newRouteIdx = v137;
				return 2;
			}
			if (essai2[0]._x != -1 && foundLineIdx > collLineIdx && v140 >= collLineIdx && v142 >= collLineIdx && a6 <= collLineIdx) {
				_newLineIdx = collLineIdx;
				_newLineDataIdx = collDataIdx;
				int i = 0;
				do {
					assert(v137 <= 8000);
					_bestRoute[v137++] = essai2[i++];
				} while (essai2[i].isValid());
				_newRouteIdx = v137;
				return 2;
			}
			// CHECKME: Checking essai0[0]._X might make more sense here?
			if (essai1[0]._x != -1 && foundLineIdx > v142 && v140 >= v142 && collLineIdx >= v142 && a6 <= v142) {
				_newLineIdx = v142;
				_newLineDataIdx = v143;
				int i = 0;
				do {
					assert(v137 <= 8000);
					_bestRoute[v137++] = essai0[i++];
				} while (essai0[i].isValid());
				_newRouteIdx = v137;
				return 2;
			}
		}
	}
	return 0;

retLABEL_essai0:
	if (v115) {
		int i = 0;
		do {
			assert(v137 <= 8000);
			_bestRoute[v137++] = essai0[i++];
		} while (essai0[i].isValid());
	}
	_bestRoute[v137].invalidate();
	return 1;

retLABEL_essai1:
	if (v117) {
		int i = 0;
		do {
			assert(v137 <= 8000);
			_bestRoute[v137++] = essai1[i++];
		} while (essai1[i].isValid());
	}
	_bestRoute[v137].invalidate();
	return 1;

retLABEL_essai2:
	if (v117) {
		int i = 0;
		do {
			assert(v137 <= 8000);
			_bestRoute[v137++] = essai2[i++];
		} while (essai2[i].isValid());
	}
	_bestRoute[v137].invalidate();
	return 1;
}

RouteItem *LinesManager::cityMapCarRoute(int x1, int y1, int x2, int y2) {
	RouteItem *result;
	int arrDelta[10];
	int arrDataIdx[10];
	int arrLineIdx[10];

	int clipX2 = x2;
	int clipY2 = y2;
	int superRouteIdx = 0;
	if (x2 <= 14)
		clipX2 = 15;
	if (y2 <= 14)
		clipY2 = 15;
	if (clipX2 > _vm->_graphicsManager._maxX - 10)
		clipX2 = _vm->_graphicsManager._maxX - 10;
	if (clipY2 > 445)
		clipY2 = 440;

	int delta = 0;
	for (delta = 0; clipY2 + delta < _vm->_graphicsManager._maxY; delta++) {
		if (checkCollisionLine(clipX2, clipY2 + delta, &arrDataIdx[5], &arrLineIdx[5], 0, _lastLine) && arrLineIdx[5] <= _lastLine)
			break;
		arrDataIdx[5] = 0;
		arrLineIdx[5] = -1;
	}
	arrDelta[5] = delta;

	for (delta = 0; clipY2 - delta > _vm->_graphicsManager._minY; delta++) {
		if (checkCollisionLine(clipX2, clipY2 - delta , &arrDataIdx[1], &arrLineIdx[1], 0, _lastLine) && arrLineIdx[1] <= _lastLine)
			break;
		arrDataIdx[1] = 0;
		arrLineIdx[1] = -1;
		if (arrDelta[5] < delta && arrLineIdx[5] != -1)
			break;
	}
	arrDelta[1] = delta;

	for (delta = 0; clipX2 + delta < _vm->_graphicsManager._maxX; delta++) {
		if (checkCollisionLine(clipX2 + delta, clipY2, &arrDataIdx[3], &arrLineIdx[3], 0, _lastLine) && arrLineIdx[3] <= _lastLine)
			break;
		arrDataIdx[3] = 0;
		arrLineIdx[3] = -1;
		if (arrDelta[1] <= delta && arrLineIdx[1] != -1)
			break;
		if (arrDelta[5] <= delta && arrLineIdx[5] != -1)
			break;
	}
	arrDelta[3] = delta;

	for (delta = 0; clipX2 - delta > _vm->_graphicsManager._minX; delta++) {
		if (checkCollisionLine(clipX2 - delta, clipY2, &arrDataIdx[7], &arrLineIdx[7], 0, _lastLine) && arrLineIdx[7] <= _lastLine)
			break;
		arrDataIdx[7] = 0;
		arrLineIdx[7] = -1;
		if ((arrDelta[1] <= delta && arrLineIdx[1] != -1) || (arrDelta[3] <= delta && arrLineIdx[3] != -1) || (arrDelta[5] <= delta && arrLineIdx[5] != -1))
			break;
	}
	arrDelta[7] = delta;

	int v68 = 0;
	int v69 = 0;
	int v72 = 0;
	int v73 = 0;

	if (arrLineIdx[1] == -1)
		arrDelta[1] = 1300;
	if (arrLineIdx[3] == -1)
		arrDelta[3] = 1300;
	if (arrLineIdx[5] == -1)
		arrDelta[5] = 1300;
	if (arrLineIdx[7] == -1)
		arrDelta[7] = 1300;
	if (arrLineIdx[1] != -1 || arrLineIdx[3] != -1 || arrLineIdx[5] != -1 || arrLineIdx[7] != -1) {
		bool v23 = false;
		if (arrLineIdx[5] != -1 && arrDelta[1] >= arrDelta[5] && arrDelta[3] >= arrDelta[5] && arrDelta[7] >= arrDelta[5]) {
			v73 = arrLineIdx[5];
			v72 = arrDataIdx[5];
			v23 = true;
		}
		if (arrLineIdx[1] != -1 && !v23 && arrDelta[5] >= arrDelta[1] && arrDelta[3] >= arrDelta[1] && arrDelta[7] >= arrDelta[1]) {
			v73 = arrLineIdx[1];
			v72 = arrDataIdx[1];
			v23 = true;
		}
		if (arrLineIdx[3] != -1 && !v23 && arrDelta[1] >= arrDelta[3] && arrDelta[5] >= arrDelta[3] && arrDelta[7] >= arrDelta[3]) {
			v73 = arrLineIdx[3];
			v72 = arrDataIdx[3];
			v23 = true;
		}
		if (arrLineIdx[7] != -1 && !v23 && arrDelta[5] >= arrDelta[7] && arrDelta[3] >= arrDelta[7] && arrDelta[1] >= arrDelta[7]) {
			v73 = arrLineIdx[7];
			v72 = arrDataIdx[7];
		}
		for (int v24 = 0; v24 <= 8; v24++) {
			arrLineIdx[v24] = -1;
			arrDataIdx[v24] = 0;
			arrDelta[v24] = 1300;
		}
		if (checkCollisionLine(x1, y1, &arrDataIdx[1], &arrLineIdx[1], 0, _lastLine)) {
			v69 = arrLineIdx[1];
			v68 = arrDataIdx[1];
		} else if (checkCollisionLine(x1, y1, &arrDataIdx[1], &arrLineIdx[1], 0, _linesNumb)) {
			int v27 = 0;
			int v28;
			for (;;) {
				v28 = essai2[v27]._x;
				int v29 = essai2[v27]._y;
				Directions v66 = essai2[v27]._dir;
				v27++;

				if (checkCollisionLine(v28, v29, &arrDataIdx[1], &arrLineIdx[1], 0, _lastLine))
					break;

				_bestRoute[superRouteIdx].set(v28, v29, v66);

				essai0[superRouteIdx].set(v28, v29, v66);
				superRouteIdx++;
				if (v28 == -1)
					break;;
			}
			if (v28 != -1) {
				v69 = arrLineIdx[1];
				v68 = arrDataIdx[1];
			}
		} else {
			v69 = 1;
			v68 = 1;
			superRouteIdx = 0;
		}
		bool loopFl = true;
		while (loopFl) {
			loopFl = false;
			if (v69 < v73) {
				superRouteIdx = _lineItem[v69].appendToRouteInc(v68, _lineItem[v69]._lineDataEndIdx - 2, _bestRoute, superRouteIdx);
				for (int j = v69 + 1; j < v73; ++j) {
					if (PLAN_TEST(_lineItem[j]._lineData[0], _lineItem[j]._lineData[1], superRouteIdx, j, v73)) {
						v69 = _newLineIdx;
						v68 = _newLineDataIdx;
						superRouteIdx = _newRouteIdx;
						loopFl = true;
						break;
					}
					if (_lineItem[j]._lineDataEndIdx - 2 > 0) {
						superRouteIdx = _lineItem[j].appendToRouteInc(0, _lineItem[j]._lineDataEndIdx - 2, _bestRoute, superRouteIdx);
					}
				}
				if (loopFl)
					continue;
				v68 = 0;
				v69 = v73;
			}
			if (v69 > v73) {
				superRouteIdx = _lineItem[v69].appendToRouteDec(v68, 0, _bestRoute, superRouteIdx);
				for (int l = v69 - 1; l > v73; --l) {
					if (PLAN_TEST(_lineItem[l]._lineData[2 * _lineItem[l]._lineDataEndIdx - 2], _lineItem[l]._lineData[2 * _lineItem[l]._lineDataEndIdx - 1], superRouteIdx, l, v73)) {
						v69 = _newLineIdx;
						v68 = _newLineDataIdx;
						superRouteIdx = _newRouteIdx; 
						loopFl = true;
						break;
					}

					superRouteIdx = _lineItem[l].appendToRouteDec(_lineItem[l]._lineDataEndIdx - 2, 0, _bestRoute, superRouteIdx);
				}
				if (loopFl)
					continue;

				v68 = _lineItem[v73]._lineDataEndIdx - 1;
				v69 = v73;
			}
			if (v69 == v73) {
				if (v68 <= v72) {
					superRouteIdx = _lineItem[v73].appendToRouteInc(v68, v72, _bestRoute, superRouteIdx);
				} else {
					superRouteIdx = _lineItem[v73].appendToRouteDec(v68, v72, _bestRoute, superRouteIdx);
				}
			}
		}
		_bestRoute[superRouteIdx].invalidate();
		result = &_bestRoute[0];
	} else {
		result = (RouteItem *)g_PTRNUL;
	}
	return result;
}

bool LinesManager::checkSmoothMove(int fromX, int fromY, int destX, int destY) {
	int foundLineIdx;
	int foundDataIdx;

	int distX = abs(fromX - destX) + 1;
	int distY = abs(fromY - destY) + 1;
	if (distX > distY)
		distY = distX;
	if (distY <= 10)
		return true;

	int stepX = 1000 * distX / (distY - 1);
	int stepY = 1000 * distY / (distY - 1);
	if (destX < fromX)
		stepX = -stepX;
	if (destY < fromY)
		stepY = -stepY;

	int smoothPosX = 1000 * fromX;
	int smoothPosY = 1000 * fromY;
	int newPosX = fromX;
	int newPosY = fromY;

	if (distY + 1 > 0) {
		int stepCount = 0;
		while (!checkCollisionLine(newPosX, newPosY, &foundDataIdx, &foundLineIdx, 0, _linesNumb) || foundLineIdx > _lastLine) {
			smoothPosX += stepX;
			smoothPosY += stepY;
			newPosX = smoothPosX / 1000;
			newPosY = smoothPosY / 1000;
			++stepCount;
			if (stepCount >= distY + 1)
				return false;
		}
		return true;
	}
	return false;
}

bool LinesManager::makeSmoothMove(int fromX, int fromY, int destX, int destY) {
	int curX = fromX;
	int curY = fromY;
	if (fromX > destX && destY > fromY) {
		int hopkinsIdx = 36;
		int smoothIdx = 0;
		int stepCount = 0;
		while (curX > destX && destY > curY) {
			int v25 = _vm->_globals._hopkinsItem[hopkinsIdx]._speedX;
			int v40 = _vm->_globals._hopkinsItem[hopkinsIdx]._speedY;
			int spriteSize = _vm->_globals._spriteSize[curY];
			if (spriteSize < 0) {
				v25 = _vm->_graphicsManager.zoomOut(v25, -spriteSize);
				v40 = _vm->_graphicsManager.zoomOut(v40, -spriteSize);
			} else if (spriteSize > 0) {
				v25 = _vm->_graphicsManager.zoomIn(v25, spriteSize);
				v40 = _vm->_graphicsManager.zoomIn(v40, spriteSize);
			}
			for (int i = 0; i < v25; i++) {
				--curX;
				_smoothRoute[smoothIdx]._posX = curX;
				if (curY != curY + v40)
					curY++;
				_smoothRoute[smoothIdx]._posY = curY;
				smoothIdx++;
			}
			++hopkinsIdx;
			if (hopkinsIdx == 48)
				hopkinsIdx = 36;
			++stepCount;
		}
		if (stepCount > 5) {
			_smoothRoute[smoothIdx]._posX = -1;
			_smoothRoute[smoothIdx]._posY = -1;
			_smoothMoveDirection = DIR_DOWN_LEFT;
			return false;
		}
	} else if (fromX < destX && destY > fromY) {
		int hopkinsIdx = 36;
		int smoothIdx = 0;
		int stepCount = 0;
		while (curX < destX && destY > curY) {
			int v14 = _vm->_globals._hopkinsItem[hopkinsIdx]._speedX;
			int v39 = _vm->_globals._hopkinsItem[hopkinsIdx]._speedY;
			int spriteSize = _vm->_globals._spriteSize[curY];
			if (spriteSize < 0) {
				v14 = _vm->_graphicsManager.zoomOut(v14, -spriteSize);
				v39 = _vm->_graphicsManager.zoomOut(v39, -spriteSize);
			} else if (spriteSize > 0) {
				v14 = _vm->_graphicsManager.zoomIn(v14, spriteSize);
				v39 = _vm->_graphicsManager.zoomIn(v39, spriteSize);
			}
			for (int i = 0; i < v14; i++) {
				++curX;
				_smoothRoute[smoothIdx]._posX = curX;
				if (curY != curY + v39)
					curY++;
				_smoothRoute[smoothIdx]._posY = curY;
				smoothIdx++;
			}
			++hopkinsIdx;
			if (hopkinsIdx == 48)
				hopkinsIdx = 36;
			++stepCount;
		}
		if (stepCount > 5) {
			_smoothRoute[smoothIdx]._posX = -1;
			_smoothRoute[smoothIdx]._posY = -1;
			_smoothMoveDirection = DIR_DOWN_RIGHT;
			return false;
		}
	} else if (fromX > destX && destY < fromY) {
		int hopkinsIdx = 12;
		int smoothIdx = 0;
		int stepCount = 0;
		while (curX > destX && destY < curY) {
			int v11 = _vm->_graphicsManager.zoomOut(_vm->_globals._hopkinsItem[hopkinsIdx]._speedX, 25);
			int v38 = _vm->_graphicsManager.zoomOut(_vm->_globals._hopkinsItem[hopkinsIdx]._speedY, 25);
			int oldY = curY;
			for (int v12 = 0; v12 < v11; v12++) {
				--curX;
				_smoothRoute[smoothIdx]._posX = curX;
				if ((uint16)curY != (uint16)oldY + v38)
					curY--;
				_smoothRoute[smoothIdx]._posY = curY;
				smoothIdx++;
			}
			++hopkinsIdx;
			if (hopkinsIdx == 24)
				hopkinsIdx = 12;
			++stepCount;
		}
		if (stepCount > 5) {
			_smoothRoute[smoothIdx]._posX = -1;
			_smoothRoute[smoothIdx]._posY = -1;
			_smoothMoveDirection = DIR_UP_LEFT;
			return false;
		}
	} else if (fromX < destX && destY < fromY) {
		int hopkinsIdx = 12;
		int smoothIdx = 0;
		int stepCount = 0;
		while (curX < destX && destY < curY) {
			int oldY = curY;
			int v7 = _vm->_graphicsManager.zoomOut(_vm->_globals._hopkinsItem[hopkinsIdx]._speedX, 25);
			int v37 = _vm->_graphicsManager.zoomOut(_vm->_globals._hopkinsItem[hopkinsIdx]._speedY, 25);
			for (int i = 0; i < v7; i++) {
				++curX;
				_smoothRoute[smoothIdx]._posX = curX;
				if ((uint16)curY != (uint16)oldY + v37)
					curY--;
				_smoothRoute[smoothIdx]._posY = curY;
				smoothIdx++;
			}
			++hopkinsIdx;
			if (hopkinsIdx == 24)
				hopkinsIdx = 12;
			++stepCount;
		}

		if (stepCount > 5) {
			_smoothRoute[smoothIdx]._posX = -1;
			_smoothRoute[smoothIdx]._posY = -1;
			_smoothMoveDirection = DIR_UP_RIGHT;
			return false;
		}
	}
	return true;
}

bool LinesManager::PLAN_TEST(int paramX, int paramY, int a3, int a4, int a5) {
	int v42;
	int v43;
	int v44;
	int v45;
	int dataIdxTestUp;
	int dataIdxTestDown;
	int dataIdxTestLeft;
	int dataIdxTestRight;
	int lineIdxTestUp;
	int lineIdxTestDown;
	int lineIdxTestLeft;
	int lineIdxTestRight;

	int idxTestUp = testLine(paramX, paramY - 2, &v42, &lineIdxTestUp, &dataIdxTestUp);
	int idxTestDown = testLine(paramX, paramY + 2, &v43, &lineIdxTestDown, &dataIdxTestDown);
	int idxTestLeft = testLine(paramX - 2, paramY, &v44, &lineIdxTestLeft, &dataIdxTestLeft);
	int idxTestRight = testLine(paramX + 2, paramY, &v45, &lineIdxTestRight, &dataIdxTestRight);
	if (idxTestUp == -1 && idxTestDown == -1 && idxTestLeft == -1 && idxTestRight == -1)
		return false;

	int v8;
	if (a4 == -1 || a5 == -1) {
		if (idxTestUp != -1)
			v8 = 1;
		else if (idxTestDown != -1)
			v8 = 2;
		else if (idxTestLeft != -1)
			v8 = 3;
		else if (idxTestRight != -1)
			v8 = 4;
		else 
			return false;
	} else {
		int v28 = 100;
		int v7 = 100;
		int v35 = 100;
		int v27 = 100;
		int v36 = abs(a4 - a5);
		if (idxTestUp != -1) {
			v28 = abs(lineIdxTestUp - a5);
		}
		if (idxTestDown != -1) {
			v7 = abs(lineIdxTestDown - a5);
		}
		if (idxTestLeft != -1) {
			v35 = abs(lineIdxTestLeft - a5);
		}
		if (idxTestRight != -1) {
			v27 = abs(lineIdxTestRight - a5);
		}

		if (v28 < v36 && v28 <= v7 && v28 <= v35 && v28 <= v27)
			v8 = 1;
		else if (v36 > v7 && v28 >= v7 && v35 >= v7 && v27 >= v7)
			v8 = 2;
		else if (v35 < v36 && v35 <= v28 && v35 <= v7 && v35 <= v27)
			v8 = 3;
		else if (v27 < v36 && v27 <= v28 && v27 <= v7 && v27 <= v35)
			v8 = 4;
		else
			return false;
	}

	int v33 = 0;
	int idxTest = 0;
	if (v8 == 1) {
		idxTest = idxTestUp;
		v33 = v42;
		_newLineIdx = lineIdxTestUp;
		_newLineDataIdx = dataIdxTestUp;
	} else if (v8 == 2) {
		idxTest = idxTestDown;
		v33 = v43;
		_newLineIdx = lineIdxTestDown;
		_newLineDataIdx = dataIdxTestDown;
	} else if (v8 == 3) {
		idxTest = idxTestLeft;
		v33 = v44;
		_newLineIdx = lineIdxTestLeft;
		_newLineDataIdx = dataIdxTestLeft;
	} else if (v8 == 4) {
		idxTest = idxTestRight;
		v33 = v45;
		_newLineIdx = lineIdxTestRight;
		_newLineDataIdx = dataIdxTestRight;
	}

	int superRouteIdx = a3;
	if (v33 == 1) {
		superRouteIdx = _lineItem[idxTest].appendToRouteInc(0, -1, _bestRoute, superRouteIdx);
	} else if (v33 == 2) {
		superRouteIdx = _lineItem[idxTest].appendToRouteDec(-1, -1, _bestRoute, superRouteIdx);
	}
	_newRouteIdx = superRouteIdx;
	return true;
}

// Test line
int LinesManager::testLine(int paramX, int paramY, int *a3, int *foundLineIdx, int *foundDataIdx) {
	int16 *lineData;
	int lineDataEndIdx;
	int collLineIdx;
	int collDataIdx;

	for (int idx = _lastLine + 1; idx < _linesNumb + 1; idx++) {
		lineData = _lineItem[idx]._lineData;
		lineDataEndIdx = _lineItem[idx]._lineDataEndIdx;
		if (lineData[0] == paramX && lineData[1] == paramY) {
			*a3 = 1;
			int posX = lineData[2 * (lineDataEndIdx - 1)];
			int posY = lineData[2 * (lineDataEndIdx - 1) + 1];
			if (_lineItem[idx]._directionRouteInc == DIR_DOWN || _lineItem[idx]._directionRouteInc == DIR_UP)
				posY += 2;
			if (_lineItem[idx]._directionRouteInc == DIR_RIGHT || _lineItem[idx]._directionRouteDec == DIR_LEFT)
				posX += 2;
			if (!checkCollisionLine(posX, posY, &collDataIdx, &collLineIdx, 0, _lastLine))
				error("Error in test line");
			*foundLineIdx = collLineIdx;
			*foundDataIdx = collDataIdx;
			return idx;
		}
		if (lineData[2 * (lineDataEndIdx - 1)] == paramX && lineData[2 * (lineDataEndIdx - 1) + 1] == paramY) {
			*a3 = 2;
			int posX = lineData[0];
			int posY = lineData[1];
			if (_lineItem[idx]._directionRouteInc == DIR_DOWN || _lineItem[idx]._directionRouteInc == DIR_UP)
				posY -= 2;
			if (_lineItem[idx]._directionRouteInc == DIR_RIGHT || _lineItem[idx]._directionRouteDec == DIR_LEFT)
				posX -= 2;
			if (!checkCollisionLine(posX, posY, &collDataIdx, &collLineIdx, 0, _lastLine))
				error("Error in test line");
			*foundLineIdx = collLineIdx;
			*foundDataIdx = collDataIdx;
			return idx;
		}
	}
	return -1;
}

int LinesManager::CALC_PROPRE(int idx) {
	int size = _vm->_globals._spriteSize[idx];
	if (_vm->_globals._characterType == 1) {
		if (size < 0)
			size = -size;
		size = 20 * (5 * size - 100) / -80;
	} else if (_vm->_globals._characterType == 2) {
		if (size < 0)
			size = -size;
		size = 20 * (5 * size - 165) / -67;
	}

	int retVal = 25;
	if (size < 0)
		retVal = _vm->_graphicsManager.zoomOut(25, -size);
	else if (size > 0)
		retVal = _vm->_graphicsManager.zoomIn(25, size);

	return retVal;
}

void LinesManager::PACOURS_PROPRE(RouteItem *route) {
	int routeIdx = 0;
	Directions oldDir = DIR_NONE;
	int route0Y = route[0]._y;
	Directions curDir = route[0]._dir;
	if (route[0]._x == -1 && route0Y == -1)
		return;

	for (;;) {
		if (oldDir != DIR_NONE && curDir != oldDir) {
			int oldRouteIdx = routeIdx;
			int routeCount = 0;
			int v10 = CALC_PROPRE(route0Y);
			int curRouteX = route[routeIdx]._x;
			int curRouteY = route[routeIdx]._y;
			while (curRouteX != -1 || curRouteY != -1) {
				int idx = routeIdx;
				++routeIdx;
				++routeCount;
				if (route[idx]._dir != curDir)
					break;
				curRouteX = route[routeIdx]._x;
				curRouteY = route[routeIdx]._y;
			}
			if (routeCount < v10) {
				int idx = oldRouteIdx;
				for (int i = 0; i < routeCount; i++) {
					route[idx]._dir = oldDir;
					idx++;
				}
				curDir = oldDir;
			}
			routeIdx = oldRouteIdx;
			if (curRouteX == -1 && curRouteY == -1)
				break;
		}
		routeIdx++;
		oldDir = curDir;
		route0Y = route[routeIdx]._y;
		curDir = route[routeIdx]._dir;
		if (route[routeIdx]._x == -1 && route0Y == -1)
			break;
	}
}

int LinesManager::getMouseZone() {
	int result;

	int xp = _vm->_eventsManager._mousePos.x + _vm->_eventsManager._mouseOffset.x;
	int yp = _vm->_eventsManager._mousePos.y + _vm->_eventsManager._mouseOffset.y;
	if ((_vm->_eventsManager._mousePos.y + _vm->_eventsManager._mouseOffset.y) > 19) {
		for (int bobZoneId = 0; bobZoneId <= 48; bobZoneId++) {
			int bobId = BOBZONE[bobZoneId];
			if (bobId && BOBZONE_FLAG[bobZoneId] && _vm->_objectsManager._bob[bobId].field0 && _vm->_objectsManager._bob[bobId]._frameIndex != 250 &&
				!_vm->_objectsManager._bob[bobId]._disabledAnimationFl && xp > _vm->_objectsManager._bob[bobId]._oldX && 
				xp < _vm->_objectsManager._bob[bobId]._oldWidth + _vm->_objectsManager._bob[bobId]._oldX && yp > _vm->_objectsManager._bob[bobId]._oldY) {
					if (yp < _vm->_objectsManager._bob[bobId]._oldHeight + _vm->_objectsManager._bob[bobId]._oldY) {
						if (ZONEP[bobZoneId]._spriteIndex == -1) {
							ZONEP[bobZoneId]._destX = 0;
							ZONEP[bobZoneId]._destY = 0;
						}
						if (!ZONEP[bobZoneId]._destX && !ZONEP[bobZoneId]._destY) {
							ZONEP[bobZoneId]._destX = _vm->_objectsManager._bob[bobId]._oldWidth + _vm->_objectsManager._bob[bobId]._oldX;
							ZONEP[bobZoneId]._destY = _vm->_objectsManager._bob[bobId]._oldHeight + _vm->_objectsManager._bob[bobId]._oldY + 6;
							ZONEP[bobZoneId]._spriteIndex = -1;
						}
						return bobZoneId;
					}
			}
		}
		_currentSegmentId = 0;
		for (int squareZoneId = 0; squareZoneId <= 99; squareZoneId++) {
			if (ZONEP[squareZoneId]._enabledFl && _squareZone[squareZoneId]._enabledFl
				&& _squareZone[squareZoneId]._left <= xp && _squareZone[squareZoneId]._right >= xp
				&& _squareZone[squareZoneId]._top <= yp && _squareZone[squareZoneId]._bottom >= yp) {
					if (_squareZone[squareZoneId]._squareZoneFl)
						return _zoneLine[_squareZone[squareZoneId]._minZoneLineIdx]._bobZoneIdx;

					_segment[_currentSegmentId]._minZoneLineIdx = _squareZone[squareZoneId]._minZoneLineIdx;
					_segment[_currentSegmentId]._maxZoneLineIdx = _squareZone[squareZoneId]._maxZoneLineIdx;
					++_currentSegmentId;
			}
		}
		if (!_currentSegmentId)
			return -1;


		int colRes1 = 0;
		for (int yCurrent = yp; yCurrent >= 0; --yCurrent) {
			colRes1 = checkCollision(xp, yCurrent);
			if (colRes1 != -1 && ZONEP[colRes1]._enabledFl)
				break;
		}

		if (colRes1 == -1)
			return -1;

		int colRes2 = 0;
		for (int j = yp; j < _vm->_graphicsManager._maxY; ++j) {
			colRes2 = checkCollision(xp, j);
			if (colRes2 != -1 && ZONEP[colRes1]._enabledFl)
				break;
		}

		if (colRes2 == -1)
			return -1;

		int colRes3 = 0;
		for (int k = xp; k >= 0; --k) {
			colRes3 = checkCollision(k, yp);
			if (colRes3 != -1 && ZONEP[colRes1]._enabledFl)
				break;
		}
		if (colRes3 == -1)
			return -1;

		int colRes4 = 0;
		for (int xCurrent = xp; _vm->_graphicsManager._maxX > xCurrent; ++xCurrent) {
			colRes4 = checkCollision(xCurrent, yp);
			if (colRes4 != -1 && ZONEP[colRes1]._enabledFl)
				break;
		}
		if (colRes1 == colRes2 && colRes1 == colRes3 && colRes1 == colRes4)
			result = colRes1;
		else
			result = -1;

	} else {
		result = 0;
	}
	return result;
}

int LinesManager::checkCollision(int xp, int yp) {
	if (_currentSegmentId <= 0)
		return -1;

	int xMax = xp + 4;
	int xMin = xp - 4;

	for (int idx = 0; idx <= _currentSegmentId; ++idx) {
		int curZoneLineIdx = _segment[idx]._minZoneLineIdx;
		if (_segment[idx]._maxZoneLineIdx < curZoneLineIdx)
			continue;

		int yMax = yp + 4;
		int yMin = yp - 4;

		do {
			int16 *dataP = _zoneLine[curZoneLineIdx]._zoneData;
			if (dataP != (int16 *)g_PTRNUL) {
				int count = _zoneLine[curZoneLineIdx]._count;
				int v1 = dataP[0];
				int v2 = dataP[1];
				int v3 = dataP[count * 2 - 2];
				int v4 = dataP[count * 2 - 1];

				bool flag = true;
				if (v1 < v3 && (xMax < v1 || xMin > v3))
					flag = false;
				if (v1 >= v3 && (xMin > v1 || xMax < v3))
					flag = false;
				if (v2 < v4 && (yMax < v2 || yMin > v4))
					flag = false;
				if (v2 >= v4 && (yMin > v2 || yMax < v4))
					flag = false;

				if (flag && _zoneLine[curZoneLineIdx]._count > 0) {
					for (int i = 0; i < count; ++i) {
						int xCheck = *dataP++;
						int yCheck = *dataP++;

						if ((xp == xCheck || (xp + 1) == xCheck) && (yp == yCheck))
							return _zoneLine[curZoneLineIdx]._bobZoneIdx;
					}
				}
			}
		} while (++curZoneLineIdx <= _segment[idx]._maxZoneLineIdx);
	}

	return -1;
}

// Square Zone
void LinesManager::CARRE_ZONE() {
	for (int idx = 0; idx < 100; ++idx) {
		_squareZone[idx]._enabledFl = false;
		_squareZone[idx]._squareZoneFl = false;
		_squareZone[idx]._left = 1280;
		_squareZone[idx]._right = 0;
		_squareZone[idx]._top = 460;
		_squareZone[idx]._bottom = 0;
		_squareZone[idx]._minZoneLineIdx = 401;
		_squareZone[idx]._maxZoneLineIdx = 0;
	}

	for (int idx = 0; idx < MAX_LINES; ++idx) {
		int16 *dataP = _zoneLine[idx]._zoneData;
		if (dataP == (int16 *)g_PTRNUL)
			continue;

		int carreZoneId = _zoneLine[idx]._bobZoneIdx;
		_squareZone[carreZoneId]._enabledFl = true;
		if (_squareZone[carreZoneId]._maxZoneLineIdx < idx)
			_squareZone[carreZoneId]._maxZoneLineIdx = idx;
		if (_squareZone[carreZoneId]._minZoneLineIdx > idx)
			_squareZone[carreZoneId]._minZoneLineIdx = idx;

		for (int i = 0; i < _zoneLine[idx]._count; i++) {
			int zoneX = *dataP++;
			int zoneY = *dataP++;

			if (_squareZone[carreZoneId]._left >= zoneX)
				_squareZone[carreZoneId]._left = zoneX;
			if (_squareZone[carreZoneId]._right <= zoneX)
				_squareZone[carreZoneId]._right = zoneX;
			if (_squareZone[carreZoneId]._top >= zoneY)
				_squareZone[carreZoneId]._top = zoneY;
			if (_squareZone[carreZoneId]._bottom <= zoneY)
				_squareZone[carreZoneId]._bottom = zoneY;
		}
	}

	for (int idx = 0; idx < 100; idx++) {
		int zoneWidth = abs(_squareZone[idx]._left - _squareZone[idx]._right);
		int zoneHeight = abs(_squareZone[idx]._top - _squareZone[idx]._bottom);
		if (zoneWidth == zoneHeight)
			_squareZone[idx]._squareZoneFl = true;
	}
}

void LinesManager::clearAll() {
	for (int idx = 0; idx < 105; ++idx) {
		ZONEP[idx]._destX = 0;
		ZONEP[idx]._destY = 0;
		ZONEP[idx]._spriteIndex = 0;
	}

	essai0 = (RouteItem *)g_PTRNUL;
	essai1 = (RouteItem *)g_PTRNUL;
	essai2 = (RouteItem *)g_PTRNUL;
	_lineBuf = (int16 *)g_PTRNUL;
	_route = (RouteItem *)g_PTRNUL;

	for (int idx = 0; idx < MAX_LINES; ++idx) {
		_lineItem[idx]._lineDataEndIdx = 0;
		_lineItem[idx]._direction = DIR_NONE;
		_lineItem[idx]._directionRouteInc = DIR_NONE;
		_lineItem[idx]._directionRouteDec = DIR_NONE;
		_lineItem[idx]._lineData = (int16 *)g_PTRNUL;

		_zoneLine[idx]._count = 0;
		_zoneLine[idx]._bobZoneIdx = 0;
		_zoneLine[idx]._zoneData = (int16 *)g_PTRNUL;
	}

	for (int idx = 0; idx < 100; ++idx)
		_squareZone[idx]._enabledFl = false;

	// FIXME: Delete these somewhere
	_vm->_linesManager.essai0 = new RouteItem[8334];
	_vm->_linesManager.essai1 = new RouteItem[8334];
	_vm->_linesManager.essai2 = new RouteItem[8334];
	if (!_vm->_linesManager.essai0)
		_vm->_linesManager.essai0 = (RouteItem*)g_PTRNUL;
	if (!_vm->_linesManager.essai1)
		_vm->_linesManager.essai1 = (RouteItem*)g_PTRNUL;
	if (!_vm->_linesManager.essai2)
		_vm->_linesManager.essai2 = (RouteItem*)g_PTRNUL;
	
	_largeBuf = _vm->_globals.allocMemory(10000);
	_vm->_linesManager._lineBuf = (int16 *)(_largeBuf);
}

/**
 * Clear all zones and reset nextLine
 */
void LinesManager::clearAllZones() {
	for (int idx = 0; idx < MAX_LINES; ++idx)
		removeZoneLine(idx);
}

/**
 * Remove Zone Line
 */
void LinesManager::removeZoneLine(int idx) {
	assert (idx <= MAX_LINES);
	_zoneLine[idx]._zoneData = (int16 *)_vm->_globals.freeMemory((byte *)_zoneLine[idx]._zoneData);
}

void LinesManager::resetLines() {
	for (int idx = 0; idx < MAX_LINES; ++idx) {
		removeLine(idx);
		_lineItem[idx]._lineDataEndIdx = 0;
		_lineItem[idx]._lineData = (int16 *)g_PTRNUL;
	}
}

// Remove Line
void LinesManager::removeLine(int idx) {
	if (idx > MAX_LINES)
		error("Attempting to add a line obstacle > MAX_LIGNE.");
	_lineItem[idx]._lineData = (int16 *)_vm->_globals.freeMemory((byte *)_lineItem[idx]._lineData);
}

void LinesManager::setMaxLineIdx(int idx) {
	_maxLineIdx = idx;
}

void LinesManager::resetLastLine() {
	_lastLine = 0;
}

void LinesManager::resetLinesNumb() {
	_linesNumb = 0;
}

void LinesManager::enableZone(int idx) {
	if (BOBZONE[idx]) {
		BOBZONE_FLAG[idx] = true;
	} else {
		ZONEP[idx]._enabledFl = true;
	}
}

void LinesManager::disableZone(int idx) {
	if (BOBZONE[idx]) {
		BOBZONE_FLAG[idx] = false;
	} else {
		ZONEP[idx]._enabledFl = false;
	}
}

void LinesManager::checkZone() {
	int mouseX = _vm->_eventsManager.getMouseX();
	int mouseY = _vm->_eventsManager.getMouseY();
	int oldMouseY = mouseY;
	if (_vm->_globals._cityMapEnabledFl
		|| _vm->_eventsManager._startPos.x >= mouseX
		|| (mouseY = _vm->_graphicsManager._scrollOffset + 54, mouseX >= mouseY)
		|| (mouseY = oldMouseY - 1, mouseY < 0 || mouseY > 59)) {
			if (_vm->_objectsManager._visibleFl)
				_vm->_objectsManager._eraseVisibleCounter = 4;
			_vm->_objectsManager._visibleFl = false;
	} else {
		_vm->_objectsManager._visibleFl = true;
	}
	if (_vm->_objectsManager._forceZoneFl) {
		_vm->_globals.compteur_71 = 100;
		_vm->_globals._oldMouseZoneId = -1;
		_vm->_globals._oldMouseX = -200;
		_vm->_globals._oldMouseY = -220;
		_vm->_objectsManager._forceZoneFl = false;
	}

	_vm->_globals.compteur_71++;
	if (_vm->_globals.compteur_71 <= 1)
		return;

	if (_vm->_globals._freezeCharacterFl || (_route == (RouteItem *)g_PTRNUL) || _vm->_globals.compteur_71 > 4) {
		_vm->_globals.compteur_71 = 0;
		int zoneId;
		if (_vm->_globals._oldMouseX != mouseX || _vm->_globals._oldMouseY != oldMouseY) {
			zoneId = getMouseZone();
		} else {
			zoneId = _vm->_globals._oldMouseZoneId;
		}
		if (_vm->_globals._oldMouseZoneId != zoneId) {
			_vm->_graphicsManager.SETCOLOR4(251, 100, 100, 100);
			_vm->_eventsManager._mouseCursorId = 4;
			_vm->_eventsManager.changeMouseCursor(4);
			if (_vm->_globals._forceHideText) {
				_vm->_fontManager.hideText(5);
				_vm->_globals._forceHideText = false;
				return;
			}
		}
		if (zoneId != -1) {
			if (ZONEP[zoneId]._verbFl1 || ZONEP[zoneId]._verbFl2 ||
				ZONEP[zoneId]._verbFl3 || ZONEP[zoneId]._verbFl4 ||
				ZONEP[zoneId]._verbFl5 || ZONEP[zoneId]._verbFl6 ||
				ZONEP[zoneId]._verbFl7 || ZONEP[zoneId]._verbFl8 ||
				ZONEP[zoneId]._verbFl9 || ZONEP[zoneId]._verbFl10) {
					if (_vm->_globals._oldMouseZoneId != zoneId) {
						_vm->_fontManager.initTextBuffers(5, ZONEP[zoneId]._messageId, _vm->_globals._zoneFilename, 0, 430, 0, 0, 252);
						_vm->_fontManager.showText(5);
						_vm->_globals._forceHideText = true;
					}
					_vm->_globals._hotspotTextColor += 25;
					if (_vm->_globals._hotspotTextColor > 100)
						_vm->_globals._hotspotTextColor = 0;
					_vm->_graphicsManager.SETCOLOR4(251, _vm->_globals._hotspotTextColor, _vm->_globals._hotspotTextColor,
						_vm->_globals._hotspotTextColor);
					if (_vm->_eventsManager._mouseCursorId == 4) {
						if (ZONEP[zoneId]._verbFl1 == 2) {
							_vm->_eventsManager.changeMouseCursor(16);
							_vm->_eventsManager._mouseCursorId = 16;
							_vm->_objectsManager.setVerb(16);
						}
					}
			} else {
				_vm->_graphicsManager.SETCOLOR4(251, 100, 100, 100);
				_vm->_eventsManager._mouseCursorId = 4;
				_vm->_eventsManager.changeMouseCursor(4);
			}
		}
		_vm->_objectsManager._zoneNum = zoneId;
		_vm->_globals._oldMouseX = mouseX;
		_vm->_globals._oldMouseY = oldMouseY;
		_vm->_globals._oldMouseZoneId = zoneId;
		if (_vm->_globals._freezeCharacterFl && (_vm->_eventsManager._mouseCursorId == 4)) {
			if (zoneId != -1 && zoneId != 0)
				_vm->_objectsManager.handleRightButton();
		}
		if ((_vm->_globals._cityMapEnabledFl && zoneId == -1) || !zoneId) {
			_vm->_objectsManager.setVerb(0);
			_vm->_eventsManager._mouseCursorId = 0;
			_vm->_eventsManager.changeMouseCursor(0);
		}
	}
}

} // End of namespace Hopkins
