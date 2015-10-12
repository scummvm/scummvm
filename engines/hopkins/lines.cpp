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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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

LinesManager::LinesManager(HopkinsEngine *vm) {
	_vm = vm;

	for (int i = 0; i < MAX_LINES + 1; ++i)
		Common::fill((byte *)&_zoneLine[i], (byte *)&_zoneLine[i] + sizeof(LigneZoneItem), 0);

	for (int i = 0; i < MAX_LINES; ++i)
		Common::fill((byte *)&_lineItem[i], (byte *)&_lineItem[i] + sizeof(LigneItem), 0);

	for (int i = 0; i < 4000; ++i)
		Common::fill((byte *)&_smoothRoute[i], (byte *)&_smoothRoute[i] + sizeof(SmoothItem), 0);

	for (int i = 0; i < 8001; ++i)
		_bestRoute[i].set(0, 0, DIR_NONE);

	for (int i = 0; i < 101; ++i) {
		Common::fill((byte *)&_segment[i], (byte *)&_segment[i] + sizeof(SegmentItem), 0);
		Common::fill((byte *)&_squareZone[i], (byte *)&_squareZone[i] + sizeof(SquareZoneItem), 0);
	}

	for (int i = 0; i < 105; ++i) {
		_bobZone[i] = 0;
		_bobZoneFl[i] = false;
	}

	for (int i = 0; i < 106; ++i)
		Common::fill((byte *)&_zone[i], (byte *)&_zone[i] + sizeof(ZoneItem), 0);

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
	_testRoute0 = NULL;
	_testRoute1 = NULL;
	_testRoute2 = NULL;
	_lineBuf = NULL;
	_route = NULL;
	_currentSegmentId = 0;
	_largeBuf = NULL;
	_zoneSkipCount = 0;
	_hotspotTextColor = 0;
	_forceHideText = false;
	_oldMouseZoneId = 0;
	_oldMouseX = 0;
	_oldMouseY = 0;
	_oldRouteFromX = 0;
	_oldRouteFromY = 0;
	_oldRouteDestX = 0;
	_oldRouteDestY = 0;
	_oldZoneNum = 0;
}

LinesManager::~LinesManager() {
	_vm->_globals->freeMemory(_largeBuf);
	if (_testRoute0)
		delete[] _testRoute0;
	if (_testRoute1)
		delete[] _testRoute1;
	if (_testRoute2)
		delete[] _testRoute2;
}

int LigneItem::appendToRouteInc(int from, int to, RouteItem *route, int index) {
	debugC(5, kDebugPath, "appendToRouteInc(%d, %d, route, %d)", from, to, index);
	if (to == -1)
		to = _lineDataEndIdx;

	for (int i = from; i < to; ++i)
		route[index++].set(_lineData[2*i], _lineData[2*i+1], _directionRouteInc);
	return index;
}

int LigneItem::appendToRouteDec(int from, int to, RouteItem *route, int index) {
	debugC(5, kDebugPath, "appendToRouteDecc(%d, %d, route, %d)", from, to, index);
	if (from == -1)
		from = _lineDataEndIdx - 1;

	for (int i = from; i > to; --i)
		route[index++].set(_lineData[2*i], _lineData[2*i+1], _directionRouteDec);
	return index;
}

/**
 * Load lines
 */
void LinesManager::loadLines(const Common::String &file) {
	debugC(5, kDebugPath, "loadLines(%s)", file.c_str());
	resetLines();
	_linesNumb = 0;
	_lastLine = 0;
	byte *ptr = _vm->_fileIO->loadFile(file);
	for (int idx = 0; READ_LE_INT16((uint16 *)ptr + (idx * 5)) != -1; idx++) {
		addLine(idx,
		    (Directions)READ_LE_INT16((uint16 *)ptr + (idx * 5)),
		    READ_LE_INT16((uint16 *)ptr + (idx * 5) + 1),
		    READ_LE_INT16((uint16 *)ptr + (idx * 5) + 2),
		    READ_LE_INT16((uint16 *)ptr + (idx * 5) + 3),
		    READ_LE_INT16((uint16 *)ptr + (idx * 5) + 4));
	}
	initRoute();
	_vm->_globals->freeMemory(ptr);
}

/**
 * Check Hotspots in Inventory screen
 * Returns the ID of the hotspot under mouse
 */
int LinesManager::checkInventoryHotspots(int posX, int posY) {
	debugC(5, kDebugPath, "checkInventoryHotspots(%d, %d)", posX, posY);
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
	if (posY >= 268 && posY <= 288 && posX >= _vm->_graphicsMan->_scrollOffset + 424 && posX <= _vm->_graphicsMan->_scrollOffset + 478)
		hotspotId = 30;
	if (posY >= 290 && posY <= 306 && posX >= _vm->_graphicsMan->_scrollOffset + 424 && posX <= _vm->_graphicsMan->_scrollOffset + 478)
		hotspotId = 31;
	if (posY < 114 || posY > 306 || posX < _vm->_graphicsMan->_scrollOffset + 152 || posX > _vm->_graphicsMan->_scrollOffset + 484)
		hotspotId = 32;

	return hotspotId;
}

/**
 * Check the hotspots in an inventory line
 * Returns the hotspot Id under the mouse, if any.
 */
int LinesManager::checkInventoryHotspotsRow(int posX, int minZoneNum, bool lastRow) {
	debugC(5, kDebugPath, "checkInventoryHotspotsRow(%d, %d, %d)", posX, minZoneNum, lastRow ? 1 : 0);
	int result = minZoneNum;

	if (posX >= _vm->_graphicsMan->_scrollOffset + 158 && posX < _vm->_graphicsMan->_scrollOffset + 208)
		return result;

	if (posX >= _vm->_graphicsMan->_scrollOffset + 208 && posX < _vm->_graphicsMan->_scrollOffset + 266) {
		result += 1;
		return result;
	}

	if (posX >= _vm->_graphicsMan->_scrollOffset + 266 && posX < _vm->_graphicsMan->_scrollOffset + 320) {
		result += 2;
		return result;
	}

	if (posX >= _vm->_graphicsMan->_scrollOffset + 320 && posX < _vm->_graphicsMan->_scrollOffset + 370) {
		result += 3;
		return result;
	}

	if (posX >= _vm->_graphicsMan->_scrollOffset + 370 && posX < _vm->_graphicsMan->_scrollOffset + 424) {
		result += 4;
		return result;
	}

	if (!lastRow && posX >= _vm->_graphicsMan->_scrollOffset + 424 && posX <= _vm->_graphicsMan->_scrollOffset + 478) {
		result += 5;
		return result;
	}

	return 0;
}

/**
 * Add Zone Line
 */
void LinesManager::addZoneLine(int idx, int fromX, int fromY, int destX, int destY, int bobZoneIdx) {
	debugC(5, kDebugPath, "addZoneLine(%d, %d, %d, %d, %d, %d)", idx, fromX, fromY, destX, destY, bobZoneIdx);
	int16 *zoneData;

	if (fromX == fromY && fromY == destX && fromY == destY) {
		_bobZoneFl[bobZoneIdx] = true;
		_bobZone[bobZoneIdx] = fromY;
	} else {
		assert(idx < MAX_LINES + 1);
		_zoneLine[idx]._zoneData = (int16 *)_vm->_globals->freeMemory((byte *)_zoneLine[idx]._zoneData);

		int distX = abs(fromX - destX);
		int distY = abs(fromY - destY);
		int maxDist = 1;
		if (distX <= distY)
			maxDist += distY;
		else
			maxDist += distX;

		zoneData = (int16 *)_vm->_globals->allocMemory(2 * sizeof(int16) * maxDist + (4 * sizeof(int16)));
		assert(zoneData);

		_zoneLine[idx]._zoneData = zoneData;

		int16 *dataP = zoneData;
		int stepX = 1000 * distX / maxDist;
		int stepY = 1000 * distY / maxDist;
		if (destX < fromX)
			stepX = -stepX;
		if (destY < fromY)
			stepY = -stepY;
		int smoothPosX = 1000 * fromX;
		int smoothPosY = 1000 * fromY;
		for (int i = 0; i < maxDist; i++) {
			*dataP++ = smoothPosX / 1000;
			*dataP++ = smoothPosY / 1000;

			smoothPosX += stepX;
			smoothPosY += stepY;
		}
		*dataP++ = -1;
		*dataP++ = -1;

		_zoneLine[idx]._count = maxDist;
		_zoneLine[idx]._bobZoneIdx = bobZoneIdx;
	}
}

/**
 * Add Line
 */
void LinesManager::addLine(int lineIdx, Directions direction, int fromX, int fromY, int destX, int destY) {
	debugC(5, kDebugPath, "addLine(%d, %d, %d, %d, %d, %d)", lineIdx, direction, fromX, fromY, destX, destY);
	assert(lineIdx < MAX_LINES);

	if (_linesNumb < lineIdx)
		_linesNumb = lineIdx;

	_lineItem[lineIdx]._lineData = (int16 *)_vm->_globals->freeMemory((byte *)_lineItem[lineIdx]._lineData);
	int distX = abs(fromX - destX) + 1;
	int distY = abs(fromY - destY) + 1;
	int maxDist = distY;
	if (distX > maxDist)
		maxDist = distX;

	byte *zoneData = _vm->_globals->allocMemory(4 * maxDist + 8);
	assert(zoneData);

	Common::fill(zoneData, zoneData + 4 * maxDist + 8, 0);
	_lineItem[lineIdx]._lineData = (int16 *)zoneData;

	int16 *curLineData = _lineItem[lineIdx]._lineData;
	int stepX = 1000 * distX / (maxDist - 1);
	int stepY = 1000 * distY / (maxDist - 1);
	if (destX < fromX)
		stepX = -stepX;
	if (destY < fromY)
		stepY = -stepY;
	int dirX = (int)stepX / 1000; // -1: Left, 0: None, 1: Right
	int dirY = (int)stepY / 1000; // -1: Up, 0: None, 1: Right
	if (!dirX) {
		if (dirY == -1) {
			_lineItem[lineIdx]._directionRouteInc = DIR_UP;
			_lineItem[lineIdx]._directionRouteDec = DIR_DOWN;
		} else if (dirY == 1) {
			_lineItem[lineIdx]._directionRouteInc = DIR_DOWN;
			_lineItem[lineIdx]._directionRouteDec = DIR_UP;
		}
		// If dirY == 0, no move
	} else if (dirX == 1) {
		if (dirY == -1) {
			_lineItem[lineIdx]._directionRouteInc = DIR_UP_RIGHT;
			_lineItem[lineIdx]._directionRouteDec = DIR_DOWN_LEFT;
		} else if (!dirY) {
			_lineItem[lineIdx]._directionRouteInc = DIR_RIGHT;
			_lineItem[lineIdx]._directionRouteDec = DIR_LEFT;
		} else if (dirY == 1) {
			_lineItem[lineIdx]._directionRouteInc = DIR_DOWN_RIGHT;
			_lineItem[lineIdx]._directionRouteDec = DIR_UP_LEFT;
		}
	} else if (dirX == -1) {
		if (dirY == 1) {
			_lineItem[lineIdx]._directionRouteInc = DIR_DOWN_LEFT;
			_lineItem[lineIdx]._directionRouteDec = DIR_UP_RIGHT;
		} else if (!dirY) {
			_lineItem[lineIdx]._directionRouteInc = DIR_LEFT;
			_lineItem[lineIdx]._directionRouteDec = DIR_RIGHT;
		} else if (dirY == -1) {
			_lineItem[lineIdx]._directionRouteInc = DIR_UP_LEFT;
			_lineItem[lineIdx]._directionRouteDec = DIR_DOWN_RIGHT;
		}
	}

	// Second pass to soften cases where dirY == 0
	if (dirX == 1) {
		if (stepY > 250 && stepY <= 999) {
			_lineItem[lineIdx]._directionRouteInc = DIR_DOWN_RIGHT;
			_lineItem[lineIdx]._directionRouteDec = DIR_UP_LEFT;
		} else if (stepY < -250 && stepY > -1000) {
			_lineItem[lineIdx]._directionRouteInc = DIR_UP_RIGHT;
			_lineItem[lineIdx]._directionRouteDec = DIR_DOWN_LEFT;
		}
	} else if (dirX == -1) {
		if (stepY > 250 && stepY <= 999) {
			_lineItem[lineIdx]._directionRouteInc = DIR_DOWN_LEFT;
			_lineItem[lineIdx]._directionRouteDec = DIR_UP_RIGHT;
		} else if (stepY < -250 && stepY > -1000) {
			// In the original code, the test was on positive values and
			// was impossible to meet.
			_lineItem[lineIdx]._directionRouteInc = DIR_UP_LEFT;
			_lineItem[lineIdx]._directionRouteDec = DIR_DOWN_RIGHT;
		}
	}

	stepX = 1000 * distX / maxDist;
	stepY = 1000 * distY / maxDist;
	if (destX < fromX)
		stepX = -stepX;
	if (destY < fromY)
		stepY = -stepY;
	int smoothPosX = 1000 * fromX;
	int smoothPosY = 1000 * fromY;
	for (int i = 0; i < maxDist - 1; i++) {
		curLineData[0] = smoothPosX / 1000;
		curLineData[1] = smoothPosY / 1000;
		curLineData += 2;

		smoothPosX += stepX;
		smoothPosY += stepY;
	}
	curLineData[0] = destX;
	curLineData[1] = destY;

	curLineData += 2;
	curLineData[0] = -1;
	curLineData[1] = -1;

	_lineItem[lineIdx]._lineDataEndIdx = maxDist;
	_lineItem[lineIdx]._direction = direction;

	++_linesNumb;
}

/**
 * Check collision line
 */
bool LinesManager::checkCollisionLine(int xp, int yp, int *foundDataIdx, int *foundLineIdx, int startLineIdx, int endLineIdx) {
	debugC(5, kDebugPath, "checkCollisionLine(%d, %d, foundDataIdx, foundLineIdx, %d, %d)", xp, yp, startLineIdx ,endLineIdx);
	int16 *lineData;

	int left = xp + 4;
	int right = xp - 4;
	int top = yp + 4;
	int bottom = yp - 4;

	*foundDataIdx = -1;
	*foundLineIdx = -1;

	for (int curLineIdx = startLineIdx; curLineIdx <= endLineIdx; curLineIdx++) {
		lineData = _lineItem[curLineIdx]._lineData;

		if (lineData == NULL)
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
	debugC(5, kDebugPath, "initRoute()");
	int lineX = _lineItem[0]._lineData[0];
	int lineY = _lineItem[0]._lineData[1];

	int lineIdx = 1;
	for (;;) {
		int curDataIdx = _lineItem[lineIdx]._lineDataEndIdx;
		int16 *curLineData = _lineItem[lineIdx]._lineData;

		int curLineX = curLineData[2 * curDataIdx - 2];
		int curLineY = curLineData[2 * curDataIdx - 1];
		if (_vm->_graphicsMan->_maxX == curLineX || _vm->_graphicsMan->_maxY == curLineY ||
			_vm->_graphicsMan->_minX == curLineX || _vm->_graphicsMan->_minY == curLineY ||
			(lineX == curLineX && lineY == curLineY))
			break;
		if (lineIdx == MAX_LINES)
			error("ERROR - LAST LINE NOT FOUND");

		int16 *nextLineData = _lineItem[lineIdx + 1]._lineData;
		if (!nextLineData)
			break;
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

// Avoid obstacle
int LinesManager::avoidObstacle(int lineIdx, int lineDataIdx, int routeIdx, int destLineIdx, int destLineDataIdx, RouteItem *route) {
	debugC(5, kDebugPath, "avoidObstacle(%d, %d, %d, %d, %d, route)", lineIdx, lineDataIdx, routeIdx, destLineIdx, destLineDataIdx);
	int curLineIdx = lineIdx;
	int curLineDataIdx = lineDataIdx;
	int curRouteIdx = routeIdx;
	if (lineIdx < destLineIdx) {
		curRouteIdx = _lineItem[lineIdx].appendToRouteInc(lineDataIdx, -1, route, curRouteIdx);

		for (int i = lineIdx + 1; i < destLineIdx; i++)
			curRouteIdx = _lineItem[i].appendToRouteInc(0, -1, route, curRouteIdx);

		curLineDataIdx = 0;
		curLineIdx = destLineIdx;
	}
	if (curLineIdx > destLineIdx) {
		curRouteIdx = _lineItem[curLineIdx].appendToRouteDec(curLineDataIdx, 0, route, curRouteIdx);
		for (int i = curLineIdx - 1; i > destLineIdx; i--)
			curRouteIdx = _lineItem[i].appendToRouteDec(-1, 0, route, curRouteIdx);
		curLineDataIdx = _lineItem[destLineIdx]._lineDataEndIdx - 1;
		curLineIdx = destLineIdx;
	}
	if (curLineIdx == destLineIdx) {
		if (destLineDataIdx >= curLineDataIdx) {
			curRouteIdx = _lineItem[destLineIdx].appendToRouteInc(curLineDataIdx, destLineDataIdx, route, curRouteIdx);
		} else {
			curRouteIdx = _lineItem[destLineIdx].appendToRouteDec(curLineDataIdx, destLineDataIdx, route, curRouteIdx);
		}
	}
	return curRouteIdx;
}

// Avoid Obstacle, taking into account start/End lind Idx
int LinesManager::avoidObstacleOnSegment(int lineIdx, int lineDataIdx, int routeIdx, int destLineIdx, int destLineDataIdx, RouteItem *route, int startLineIdx, int endLineIdx) {
	debugC(5, kDebugPath, "avoidObstacleOnSegment(%d, %d, %d, %d, %d, route, %d, %d)", lineIdx, lineDataIdx, routeIdx, destLineIdx, destLineDataIdx, startLineIdx, endLineIdx);
	int curLineIdx = lineIdx;
	int curLineDataIdx = lineDataIdx;
	int curRouteIdx = routeIdx;
	if (destLineIdx < lineIdx) {
		curRouteIdx = _lineItem[lineIdx].appendToRouteInc(lineDataIdx, -1, route, curRouteIdx);
		int wrkLineIdx = lineIdx + 1;
		if (wrkLineIdx == endLineIdx + 1)
			wrkLineIdx = startLineIdx;
		while (destLineIdx != wrkLineIdx) {
			curRouteIdx = _lineItem[wrkLineIdx].appendToRouteInc(0, -1, route, curRouteIdx);
			++wrkLineIdx;
			if (endLineIdx + 1 == wrkLineIdx)
				wrkLineIdx = startLineIdx;
		}
		curLineDataIdx = 0;
		curLineIdx = destLineIdx;
	}
	if (destLineIdx > curLineIdx) {
		curRouteIdx = _lineItem[curLineIdx].appendToRouteDec(curLineDataIdx, 0, route, curRouteIdx);
		int wrkLineIdx = curLineIdx - 1;
		if (wrkLineIdx == startLineIdx - 1)
			wrkLineIdx = endLineIdx;
		while (destLineIdx != wrkLineIdx) {
			curRouteIdx = _lineItem[wrkLineIdx].appendToRouteDec(-1, 0, route, curRouteIdx);
			--wrkLineIdx;
			if (startLineIdx - 1 == wrkLineIdx)
				wrkLineIdx = endLineIdx;
		}
		curLineDataIdx = _lineItem[destLineIdx]._lineDataEndIdx - 1;
		curLineIdx = destLineIdx;
	}
	if (destLineIdx == curLineIdx) {
		if (destLineDataIdx >= curLineDataIdx) {
			curRouteIdx = _lineItem[destLineIdx].appendToRouteInc(curLineDataIdx, destLineDataIdx, route, curRouteIdx);
		} else {
			curRouteIdx = _lineItem[destLineIdx].appendToRouteDec(curLineDataIdx, destLineDataIdx, route, curRouteIdx);
		}
	}
	return curRouteIdx;
}

bool LinesManager::MIRACLE(int fromX, int fromY, int lineIdx, int destLineIdx, int routeIdx) {
	debugC(5, kDebugPath, "MIRACLE(%d, %d, %d, %d, %d)", fromX, fromY, lineIdx, destLineIdx, routeIdx);
	int lineIdxLeft = 0;
	int lineDataIdxLeft = 0;
	int lineIdxRight = 0;
	int lineDataIdxRight = 0;
	int linesIdxUp = 0;
	int linesDataIdxUp = 0;
	int lineIdxDown = 0;
	int lineDataIdxDown = 0;

	int curX = fromX;
	int curY = fromY;
	int curLineIdx = lineIdx;
	int tmpRouteIdx = routeIdx;
	int dummyDataIdx;
	if (checkCollisionLine(fromX, fromY, &dummyDataIdx, &curLineIdx, 0, _linesNumb)) {
		switch (_lineItem[curLineIdx]._direction) {
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

	int stepVertIncCount = 0;
	for (int i = curY; curY + 200 > i; i++) {
		if (checkCollisionLine(curX, i, &lineDataIdxDown, &lineIdxDown, 0, _lastLine) == 1 && lineIdxDown <= _lastLine)
			break;
		lineDataIdxDown = 0;
		lineIdxDown = -1;
		++stepVertIncCount;
	}

	int stepVertDecCount = 0;
	for (int i = curY; curY - 200 < i; i--) {
		if (checkCollisionLine(curX, i, &linesDataIdxUp, &linesIdxUp, 0, _lastLine) == 1 && linesIdxUp <= _lastLine)
			break;
		linesDataIdxUp = 0;
		linesIdxUp = -1;
		++stepVertDecCount;
	}

	int stepHoriIncCount = 0;
	for (int i = curX; curX + 200 > i; i++) {
		if (checkCollisionLine(i, curY, &lineDataIdxRight, &lineIdxRight, 0, _lastLine) == 1 && lineIdxRight <= _lastLine)
			break;
		lineDataIdxRight = 0;
		lineIdxRight = -1;
		++stepHoriIncCount;
	}

	int stepHoriDecCount = 0;
	for (int i = curX; curX - 200 < i; i--) {
		if (checkCollisionLine(i, curY, &lineDataIdxLeft, &lineIdxLeft, 0, _lastLine) == 1 && lineIdxLeft <= _lastLine)
			break;
		lineDataIdxLeft = 0;
		lineIdxLeft = -1;
		++stepHoriDecCount;
	}

	if (destLineIdx > curLineIdx) {
		if (linesIdxUp != -1 && linesIdxUp <= curLineIdx)
			linesIdxUp = -1;
		if (lineIdxRight != -1 && curLineIdx >= lineIdxRight)
			lineIdxRight = -1;
		if (lineIdxDown != -1 && curLineIdx >= lineIdxDown)
			lineIdxDown = -1;
		if (lineIdxLeft != -1 && curLineIdx >= lineIdxLeft)
			lineIdxLeft = -1;
		if (linesIdxUp != -1 && destLineIdx < linesIdxUp)
			linesIdxUp = -1;
		if (lineIdxRight != -1 && destLineIdx < lineIdxRight)
			lineIdxRight = -1;
		if (lineIdxDown != -1 && destLineIdx < lineIdxDown)
			lineIdxDown = -1;
		if (lineIdxLeft != -1 && destLineIdx < lineIdxLeft)
			lineIdxLeft = -1;
	} else if (destLineIdx < curLineIdx) {
		if (linesIdxUp != -1 && linesIdxUp >= curLineIdx)
			linesIdxUp = -1;
		if (lineIdxRight != -1 && curLineIdx <= lineIdxRight)
			lineIdxRight = -1;
		if (lineIdxDown != -1 && curLineIdx <= lineIdxDown)
			lineIdxDown = -1;
		if (lineIdxLeft != -1 && curLineIdx <= lineIdxLeft)
			lineIdxLeft = -1;
		if (linesIdxUp != -1 && destLineIdx > linesIdxUp)
			linesIdxUp = -1;
		if (lineIdxRight != -1 && destLineIdx > lineIdxRight)
			lineIdxRight = -1;
		if (lineIdxDown != -1 && destLineIdx > lineIdxDown)
			lineIdxDown = -1;
		if (lineIdxLeft != -1 && destLineIdx > lineIdxLeft)
			lineIdxLeft = -1;
	}
	if (linesIdxUp != -1 || lineIdxRight != -1 || lineIdxDown != -1 || lineIdxLeft != -1) {
		Directions newDir = DIR_NONE;
		if (destLineIdx > curLineIdx) {
			if (lineIdxDown <= linesIdxUp && lineIdxRight <= linesIdxUp && lineIdxLeft <= linesIdxUp && linesIdxUp > curLineIdx)
				newDir = DIR_UP;
			if (lineIdxDown <= lineIdxRight && linesIdxUp <= lineIdxRight && lineIdxLeft <= lineIdxRight && curLineIdx < lineIdxRight)
				newDir = DIR_RIGHT;
			if (linesIdxUp <= lineIdxDown && lineIdxRight <= lineIdxDown && lineIdxLeft <= lineIdxDown && curLineIdx < lineIdxDown)
				newDir = DIR_DOWN;
			if (lineIdxDown <= lineIdxLeft && lineIdxRight <= lineIdxLeft && linesIdxUp <= lineIdxLeft && curLineIdx < lineIdxLeft)
				newDir = DIR_LEFT;
		} else if (destLineIdx < curLineIdx) {
			if (linesIdxUp == -1)
				linesIdxUp = INVALID_LINE_VALUE;
			if (lineIdxRight == -1)
				lineIdxRight = INVALID_LINE_VALUE;
			if (lineIdxDown == -1)
				lineIdxDown = INVALID_LINE_VALUE;
			if (lineIdxLeft == -1)
				lineIdxLeft = INVALID_LINE_VALUE;
			if (linesIdxUp != INVALID_LINE_VALUE && lineIdxDown >= linesIdxUp && lineIdxRight >= linesIdxUp && lineIdxLeft >= linesIdxUp && linesIdxUp < curLineIdx)
				newDir = DIR_UP;
			if (lineIdxRight != INVALID_LINE_VALUE && lineIdxDown >= lineIdxRight && linesIdxUp >= lineIdxRight && lineIdxLeft >= lineIdxRight && curLineIdx > lineIdxRight)
				newDir = DIR_RIGHT;
			if (lineIdxDown != INVALID_LINE_VALUE && linesIdxUp >= lineIdxDown && lineIdxRight >= lineIdxDown && lineIdxLeft >= lineIdxDown && curLineIdx > lineIdxDown)
				newDir = DIR_DOWN;
			if (lineIdxLeft != INVALID_LINE_VALUE && lineIdxDown >= lineIdxLeft && lineIdxRight >= lineIdxLeft && linesIdxUp >= lineIdxLeft && curLineIdx > lineIdxLeft)
				newDir = DIR_LEFT;
		}

		int newLinesDataIdx = 0;
		int newLinesIdx = 0;
		switch(newDir) {
		case DIR_UP:
			newLinesIdx = linesIdxUp;
			newLinesDataIdx = linesDataIdxUp;
			for (int i = 0; i < stepVertDecCount; i++) {
				if (checkCollisionLine(curX, curY - i, &linesDataIdxUp, &linesIdxUp, _lastLine + 1, _linesNumb) && _lastLine < linesIdxUp) {
					int tmpRouteIdxUp = computeRouteIdx(linesIdxUp, linesDataIdxUp, curX, curY - i, curX, curY - stepVertDecCount, tmpRouteIdx, &_bestRoute[0]);
					if (tmpRouteIdxUp == -1)
						return false;
					tmpRouteIdx = tmpRouteIdxUp;
					if (_newPosY != -1)
						i = _newPosY - curY;
				}
				_bestRoute[tmpRouteIdx].set(curX, curY - i, DIR_UP);
				tmpRouteIdx++;
			}
			_newLineIdx = newLinesIdx;
			_newLineDataIdx = newLinesDataIdx;
			_newRouteIdx = tmpRouteIdx;
			return true;
			break;
		case DIR_RIGHT:
			newLinesIdx = lineIdxRight;
			newLinesDataIdx = lineDataIdxRight;
			for (int i = 0; i < stepHoriIncCount; i++) {
				if (checkCollisionLine(i + curX, curY, &linesDataIdxUp, &linesIdxUp, _lastLine + 1, _linesNumb) && _lastLine < linesIdxUp) {
					int tmpRouteIdxRight = computeRouteIdx(linesIdxUp, linesDataIdxUp, i + curX, curY, stepHoriIncCount + curX, curY, tmpRouteIdx, &_bestRoute[0]);
					if (tmpRouteIdxRight == -1)
						return false;
					tmpRouteIdx = tmpRouteIdxRight;
					if (_newPosX != -1)
						i = _newPosX - curX;
				}
				_bestRoute[tmpRouteIdx].set(i + curX, curY, DIR_RIGHT);
				tmpRouteIdx++;
			}
			_newLineIdx = newLinesIdx;
			_newLineDataIdx = newLinesDataIdx;
			_newRouteIdx = tmpRouteIdx;
			return true;
			break;
		case DIR_DOWN:
			newLinesIdx = lineIdxDown;
			newLinesDataIdx = lineDataIdxDown;
			for (int i = 0; i < stepVertIncCount; i++) {
				if (checkCollisionLine(curX, i + curY, &linesDataIdxUp, &linesIdxUp, _lastLine + 1, _linesNumb) && _lastLine < linesIdxUp) {
					int tmpRouteIdxDown = computeRouteIdx(linesIdxUp, linesDataIdxUp, curX, i + curY, curX, stepVertIncCount + curY, tmpRouteIdx, &_bestRoute[0]);
					if (tmpRouteIdxDown == -1)
						return false;
					tmpRouteIdx = tmpRouteIdxDown;
					if (_newPosY != -1)
						i = curY - _newPosY;
				}
				_bestRoute[tmpRouteIdx].set(curX, i + curY, DIR_DOWN);
				tmpRouteIdx++;
			}
			_newLineIdx = newLinesIdx;
			_newLineDataIdx = newLinesDataIdx;
			_newRouteIdx = tmpRouteIdx;
			return true;
			break;
		case DIR_LEFT:
			newLinesIdx = lineIdxLeft;
			newLinesDataIdx = lineDataIdxLeft;
			for (int i = 0; i < stepHoriDecCount; i++) {
				if (checkCollisionLine(curX - i, curY, &linesDataIdxUp, &linesIdxUp, _lastLine + 1, _linesNumb) && _lastLine < linesIdxUp) {
					int tmpRouteIdxLeft = computeRouteIdx(linesIdxUp, linesDataIdxUp, curX - i, curY, curX - stepHoriDecCount, curY, tmpRouteIdx, &_bestRoute[0]);
					if (tmpRouteIdxLeft == -1)
						return false;
					tmpRouteIdx = tmpRouteIdxLeft;
					if (_newPosX != -1)
						i = curX - _newPosX;
				}
				_bestRoute[tmpRouteIdx].set(curX - i, curY, DIR_LEFT);
				tmpRouteIdx++;
			}
			_newLineIdx = newLinesIdx;
			_newLineDataIdx = newLinesDataIdx;
			_newRouteIdx = tmpRouteIdx;
			return true;
			break;
		default:
			break;
		}
	}
	return false;
}

int LinesManager::computeRouteIdx(int lineIdx, int dataIdx, int fromX, int fromY, int destX, int destY, int routerIdx, RouteItem *route) {
	debugC(5, kDebugPath, "computeRouteIdx(%d, %d, %d, %d, %d, %d, %d)", lineIdx, dataIdx, fromX, fromY, destX, destY, routerIdx);
	int result = routerIdx;
	++_pathFindingMaxDepth;
	if (_pathFindingMaxDepth > 10) {
		warning("PathFinding - Max depth reached");
		route[routerIdx].invalidate();
		return -1;
	}
	int lineX = _lineItem[lineIdx]._lineData[0];
	int lineY = _lineItem[lineIdx]._lineData[1];
	int startLineIdx = lineIdx;

	int curLineDataEndIdx;
	bool loopCond = false;
	for (;;) {
		int curLineIdx = startLineIdx - 1;
		int endLineIdx = 2 * _lineItem[startLineIdx - 1]._lineDataEndIdx;

		int16 *lineData = _lineItem[startLineIdx - 1]._lineData;
		if (lineData == NULL)
			break;
		while (lineData[endLineIdx - 2] != lineX || lineY != lineData[endLineIdx - 1]) {
			--curLineIdx;
			if (_lastLine - 1 != curLineIdx) {
				endLineIdx = 2 * _lineItem[curLineIdx]._lineDataEndIdx;
				lineData = _lineItem[curLineIdx]._lineData;
				if (lineData)
					continue;
			}
			loopCond = true;
			break;
		}
		if (loopCond)
			break;

		startLineIdx = curLineIdx;
		lineX = lineData[0];
		lineY = lineData[1];
	}

	int lastIdx = _lineItem[lineIdx]._lineDataEndIdx - 1;
	int lastPosX = _lineItem[lineIdx]._lineData[(2 * lastIdx)];
	int lastPosY = _lineItem[lineIdx]._lineData[(2 * lastIdx) + 1];
	int endLineIdx = lineIdx;
	int foundLineIdx, foundDataIdx;
	loopCond = false;
	for (;;) {
		int curLineIdx = endLineIdx + 1;
		int nextLineDataEndIdx = 2 * _lineItem[curLineIdx]._lineDataEndIdx;
		int16 *lineData = _lineItem[curLineIdx]._lineData;
		if (lineData == NULL)
			break;
		for (;;) {
			curLineDataEndIdx = nextLineDataEndIdx;
			if (lineData[0] == lastPosX && lastPosY == lineData[1])
				break;

			++curLineIdx;
			if (curLineIdx != _linesNumb + 1) {
				nextLineDataEndIdx = 2 * _lineItem[curLineIdx]._lineDataEndIdx;
				lineData = _lineItem[curLineIdx]._lineData;
				if (lineData)
					continue;
			}
			loopCond = true;
			break;
		}
		if (loopCond)
			break;

		endLineIdx = curLineIdx;
		lastPosX = lineData[curLineDataEndIdx - 2];
		lastPosY = lineData[curLineDataEndIdx - 1];
	}

	int distX = abs(fromX - destX) + 1;
	int distY = abs(fromY - destY) + 1;
	int maxDist = distY;
	if (distX > distY)
		maxDist = distX;
	int stepX = 1000 * distX / maxDist;
	int stepY = 1000 * distY / maxDist;
	int smoothPosX = 1000 * fromX;
	int smoothPosY = 1000 * fromY;
	if (destX < fromX)
		stepX = -stepX;
	if (destY < fromY)
		stepY = -stepY;
	if (maxDist > 800)
		maxDist = 800;

	Common::fill(&_lineBuf[0], &_lineBuf[1000], 0);
	int bugLigIdx = 0;
	for (int i = 0; i < maxDist + 1; i++) {
		_lineBuf[bugLigIdx] = smoothPosX / 1000;
		_lineBuf[bugLigIdx + 1] = smoothPosY / 1000;
		smoothPosX += stepX;
		smoothPosY += stepY;
		bugLigIdx += 2;
	}
	bugLigIdx -= 2;
	int destDataIdx = 0;
	int destLineIdx = -1;
	int bufX = 0;
	int bufY = 0;
	for (int i = maxDist + 1; i > 0; i--) {
		if (checkCollisionLine(_lineBuf[bugLigIdx], _lineBuf[bugLigIdx + 1], &foundDataIdx, &foundLineIdx, startLineIdx, endLineIdx) && _lastLine < foundLineIdx) {
			destLineIdx = foundLineIdx;
			destDataIdx = foundDataIdx;
			bufX = _lineBuf[bugLigIdx];
			bufY = _lineBuf[bugLigIdx + 1];
			break;
		}
		bugLigIdx -= 2;
	}
	int maxLineX = 0;
	int minLineX = 0;
	int maxLineY = 0;
	int minLineY = 0;
	for (int i = startLineIdx; i <= endLineIdx; ++i) {
		int16 *lineData = _lineItem[i]._lineData;
		if (lineData == NULL)
			error("error in genial routine");
		if (i == startLineIdx) {
			minLineY = MIN(lineData[1], lineData[2 * _lineItem[i]._lineDataEndIdx - 1]);
			maxLineY = MAX(lineData[1], lineData[2 * _lineItem[i]._lineDataEndIdx - 1]);

			minLineX = MIN(lineData[0], lineData[2 * _lineItem[i]._lineDataEndIdx - 2]);
			maxLineX = MAX(lineData[0], lineData[2 * _lineItem[i]._lineDataEndIdx - 2]);
		} else {
			if (lineData[1] < lineData[2 * _lineItem[i]._lineDataEndIdx - 1] && lineData[1] < minLineY)
				minLineY = lineData[1];
			if (lineData[2 * _lineItem[i]._lineDataEndIdx - 1] < lineData[1] && lineData[2 * _lineItem[i]._lineDataEndIdx - 1] < minLineY)
				minLineY = lineData[2 * _lineItem[i]._lineDataEndIdx - 1];
			if (lineData[1] > lineData[2 * _lineItem[i]._lineDataEndIdx - 1] && lineData[1] > maxLineY)
				maxLineY = lineData[1];
			if (lineData[2 * _lineItem[i]._lineDataEndIdx - 1] > lineData[1] && lineData[2 * _lineItem[i]._lineDataEndIdx - 1] > maxLineY)
				maxLineY = lineData[2 * _lineItem[i]._lineDataEndIdx - 1];
			if (lineData[0] < lineData[2 * _lineItem[i]._lineDataEndIdx - 2] && minLineX > lineData[0])
				minLineX = lineData[0];
			if (lineData[2 * _lineItem[i]._lineDataEndIdx - 2] < lineData[0] && minLineX > lineData[2 * _lineItem[i]._lineDataEndIdx - 2])
				minLineX = lineData[2 * _lineItem[i]._lineDataEndIdx - 2];
			if (lineData[0] > lineData[2 * _lineItem[i]._lineDataEndIdx - 2] && maxLineX < lineData[0])
				maxLineX = lineData[0];
			if (lineData[2 * _lineItem[i]._lineDataEndIdx - 2] > lineData[0] && maxLineX < lineData[2 * _lineItem[i]._lineDataEndIdx - 2])
				maxLineX = lineData[2 * _lineItem[i]._lineDataEndIdx - 2];
		}
	}

	minLineX -= 2;
	minLineY -= 2;
	maxLineX += 2;
	maxLineY += 2;
	if (destX >= minLineX && destX <= maxLineX && destY >= minLineY && destY <= maxLineY) {
		int curY = destY;
		int linesIdxUp = -1;
		do {
			--curY;
			if (checkCollisionLine(destX, curY, &foundDataIdx, &foundLineIdx, startLineIdx, endLineIdx)) {
				linesIdxUp = foundLineIdx;
				break;
			}
		} while (curY && curY >= minLineY);

		curY = destY;
		int lineIdxDown = -1;
		do {
			++curY;
			if (checkCollisionLine(destX, curY, &foundDataIdx, &foundLineIdx, startLineIdx, endLineIdx)) {
				lineIdxDown = foundLineIdx;
				break;
			}
		} while (curY < _vm->_globals->_characterMaxPosY && curY < maxLineY);

		int curX = destX;
		int lineIdxRight = -1;
		do {
			++curX;
			if (checkCollisionLine(curX, destY, &foundDataIdx, &foundLineIdx, startLineIdx, endLineIdx)) {
				lineIdxRight = foundLineIdx;
				break;
			}
		} while (curX < _vm->_graphicsMan->_maxX && curX < maxLineX);

		curX = destX;
		int lineIdxLeft = -1;
		do {
			--curX;
			if (checkCollisionLine(curX, destY, &foundDataIdx, &foundLineIdx, startLineIdx, endLineIdx)) {
				lineIdxLeft = foundLineIdx;
				break;
			}
		} while (curX > 0 && curX > minLineX);

		if (lineIdxRight != -1 && lineIdxLeft != -1 && linesIdxUp != -1 && lineIdxDown != -1) {
			route[routerIdx].invalidate();
			return -1;
		}
	}
	if (bufX < fromX - 1 || bufX > fromX + 1 || bufY < fromY - 1 || bufY > fromY + 1) {
		_newPosX = bufX;
		_newPosY = bufY;
		if (lineIdx < destLineIdx) {
			int stepCount = 0;
			int curLineIdx = lineIdx;
			do {
				if (curLineIdx == startLineIdx - 1)
					curLineIdx = endLineIdx;
				++stepCount;
				--curLineIdx;
				if (curLineIdx == startLineIdx - 1)
					curLineIdx = endLineIdx;
			} while (destLineIdx != curLineIdx);
			if (abs(destLineIdx - lineIdx) == stepCount) {
				if (dataIdx >  abs(_lineItem[lineIdx]._lineDataEndIdx / 2)) {
					result = avoidObstacle(lineIdx, dataIdx, routerIdx, destLineIdx, destDataIdx, route);
				} else {
					result = avoidObstacleOnSegment(lineIdx, dataIdx, routerIdx, destLineIdx, destDataIdx, route, startLineIdx, endLineIdx);
				}
			}
			if (abs(destLineIdx - lineIdx) < stepCount)
				result = avoidObstacle(lineIdx, dataIdx, result, destLineIdx, destDataIdx, route);
			if (stepCount < abs(destLineIdx - lineIdx))
				result = avoidObstacleOnSegment(lineIdx, dataIdx, result, destLineIdx, destDataIdx, route, startLineIdx, endLineIdx);
		}
		if (lineIdx > destLineIdx) {
			int destStepCount = abs(lineIdx - destLineIdx);
			int curLineIdx = lineIdx;
			int curStepCount = 0;
			do {
				if (curLineIdx == endLineIdx + 1)
					curLineIdx = startLineIdx;
				++curStepCount;
				++curLineIdx;
				if (curLineIdx == endLineIdx + 1)
					curLineIdx = startLineIdx;
			} while (destLineIdx != curLineIdx);
			if (destStepCount == curStepCount) {
				if (dataIdx > abs(_lineItem[lineIdx]._lineDataEndIdx / 2)) {
					result = avoidObstacleOnSegment(lineIdx, dataIdx, result, destLineIdx, destDataIdx, route, startLineIdx, endLineIdx);
				} else {
					result = avoidObstacle(lineIdx, dataIdx, result, destLineIdx, destDataIdx, route);
				}
			}
			if (destStepCount < curStepCount)
				result = avoidObstacle(lineIdx, dataIdx, result, destLineIdx, destDataIdx, route);
			if (curStepCount < destStepCount)
				result = avoidObstacleOnSegment(lineIdx, dataIdx, result, destLineIdx, destDataIdx, route, startLineIdx, endLineIdx);
		}
		if (lineIdx == destLineIdx)
			result = avoidObstacle(lineIdx, dataIdx, result, lineIdx, destDataIdx, route);
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

// Find Route from a point to the other
RouteItem *LinesManager::findRoute(int fromX, int fromY, int destX, int destY) {
	debugC(5, kDebugPath, "findRoute(%d, %d, %d, %d)", fromX, fromY, destX, destY);
	int foundLineIdx;
	int foundDataIdx;
	int curLineY = 0;
	int curLineX = 0;
	int stepArr[9];
	int deltaArr[9];
	int collLineDataIdxArr[9];
	int collLineIdxArr[9];

	int clipDestX = destX;
	int clipDestY = destY;
	int curLineIdx = 0;
	int curLineDataIdx = 0;
	int lineIdx = 0;
	int lineDataIdx = 0;
	Directions newDir = DIR_NONE;
	if (destY <= 24)
		clipDestY = 25;
	if (!_vm->_globals->_checkDistanceFl) {
		if (abs(fromX - _oldRouteFromX) <= 4 && abs(fromY - _oldRouteFromY) <= 4 &&
		    abs(_oldRouteDestX - destX) <= 4 && abs(_oldRouteDestY - clipDestY) <= 4)
			return NULL;

		if (abs(fromX - destX) <= 4 && abs(fromY - clipDestY) <= 4)
			return NULL;

		if (_oldZoneNum > 0 && _vm->_objectsMan->_zoneNum > 0 && _oldZoneNum == _vm->_objectsMan->_zoneNum)
			return NULL;
	}
	_vm->_globals->_checkDistanceFl = false;
	_oldZoneNum = _vm->_objectsMan->_zoneNum;
	_oldRouteFromX = fromX;
	_oldRouteDestX = destX;
	_oldRouteFromY = fromY;
	_oldRouteDestY = clipDestY;
	_pathFindingMaxDepth = 0;
	int routeIdx = 0;
	if (destX <= 19)
		clipDestX = 20;
	if (clipDestY <= 19)
		clipDestY = 20;
	if (clipDestX > _vm->_graphicsMan->_maxX - 10)
		clipDestX = _vm->_graphicsMan->_maxX - 10;
	if (clipDestY > _vm->_globals->_characterMaxPosY)
		clipDestY = _vm->_globals->_characterMaxPosY;

	if (abs(fromX - clipDestX) <= 3 && abs(fromY - clipDestY) <= 3)
		return NULL;

	for (int i = 0; i <= 8; ++i) {
		collLineIdxArr[i] = -1;
		collLineDataIdxArr[i] = 0;
		deltaArr[i] = INVALID_LINE_VALUE;
		stepArr[i] = INVALID_LINE_VALUE;
	}

	if (characterRoute(fromX, fromY, clipDestX, clipDestY, -1, -1, 0) == 1)
		return _bestRoute;

	int tmpDelta = 0;
	for (int tmpY = clipDestY; tmpY < _vm->_graphicsMan->_maxY; tmpY++, tmpDelta++) {
		if (checkCollisionLine(clipDestX, tmpY, &collLineDataIdxArr[DIR_DOWN], &collLineIdxArr[DIR_DOWN], 0, _lastLine) && collLineIdxArr[DIR_DOWN] <= _lastLine)
			break;
		collLineDataIdxArr[DIR_DOWN] = 0;
		collLineIdxArr[DIR_DOWN] = -1;
	}
	deltaArr[DIR_DOWN] = tmpDelta;

	tmpDelta = 0;
	for (int tmpY = clipDestY; tmpY > _vm->_graphicsMan->_minY; tmpY--, tmpDelta++) {
		if (checkCollisionLine(clipDestX, tmpY, &collLineDataIdxArr[DIR_UP], &collLineIdxArr[DIR_UP], 0, _lastLine) && collLineIdxArr[DIR_UP] <= _lastLine)
			break;
		collLineDataIdxArr[DIR_UP] = 0;
		collLineIdxArr[DIR_UP] = -1;
		if (deltaArr[DIR_DOWN] < tmpDelta && collLineIdxArr[DIR_DOWN] != -1)
			break;
	}
	deltaArr[DIR_UP] = tmpDelta;

	tmpDelta = 0;
	for (int tmpX = clipDestX; tmpX < _vm->_graphicsMan->_maxX; tmpX++) {
		if (checkCollisionLine(tmpX, clipDestY, &collLineDataIdxArr[DIR_RIGHT], &collLineIdxArr[DIR_RIGHT], 0, _lastLine) && collLineIdxArr[DIR_RIGHT] <= _lastLine)
			break;
		collLineDataIdxArr[DIR_RIGHT] = 0;
		collLineIdxArr[DIR_RIGHT] = -1;
		++tmpDelta;
		if (deltaArr[DIR_UP] < tmpDelta && collLineIdxArr[DIR_UP] != -1)
				break;
		if (deltaArr[DIR_DOWN] < tmpDelta && collLineIdxArr[DIR_DOWN] != -1)
			break;
	}
	deltaArr[DIR_RIGHT] = tmpDelta;

	tmpDelta = 0;
	for (int tmpX = clipDestX; tmpX > _vm->_graphicsMan->_minX; tmpX--) {
		if (checkCollisionLine(tmpX, clipDestY, &collLineDataIdxArr[DIR_LEFT], &collLineIdxArr[DIR_LEFT], 0, _lastLine) && collLineIdxArr[DIR_LEFT] <= _lastLine)
			break;
		collLineDataIdxArr[DIR_LEFT] = 0;
		collLineIdxArr[DIR_LEFT] = -1;
		++tmpDelta;
		if (deltaArr[DIR_UP] < tmpDelta && collLineIdxArr[DIR_UP] != -1)
			break;
		if (deltaArr[DIR_DOWN] < tmpDelta && collLineIdxArr[DIR_DOWN] != -1)
			break;
		if (deltaArr[DIR_RIGHT] < tmpDelta && collLineIdxArr[DIR_RIGHT] != -1)
			break;
	}
	deltaArr[DIR_LEFT] = tmpDelta;

	if (collLineIdxArr[DIR_UP] < 0 || _lastLine < collLineIdxArr[DIR_UP])
		collLineIdxArr[DIR_UP] = -1;
	if (collLineIdxArr[DIR_RIGHT] < 0 || _lastLine < collLineIdxArr[DIR_RIGHT])
		collLineIdxArr[DIR_RIGHT] = -1;
	if (collLineIdxArr[DIR_DOWN] < 0 || _lastLine < collLineIdxArr[DIR_DOWN])
		collLineIdxArr[DIR_DOWN] = -1;
	if (collLineIdxArr[DIR_LEFT] < 0 || _lastLine < collLineIdxArr[DIR_LEFT])
		collLineIdxArr[DIR_LEFT] = -1;
	if (collLineIdxArr[DIR_UP] < 0)
		deltaArr[DIR_UP] = INVALID_LINE_VALUE;
	if (collLineIdxArr[DIR_RIGHT] < 0)
		deltaArr[DIR_RIGHT] = INVALID_LINE_VALUE;
	if (collLineIdxArr[DIR_DOWN] < 0)
		deltaArr[DIR_DOWN] = INVALID_LINE_VALUE;
	if (collLineIdxArr[DIR_LEFT] < 0)
		deltaArr[DIR_LEFT] = INVALID_LINE_VALUE;
	if (collLineIdxArr[DIR_UP] == -1 && collLineIdxArr[DIR_RIGHT] == -1 && collLineIdxArr[DIR_DOWN] == -1 && collLineIdxArr[DIR_LEFT] == -1)
		return NULL;

	if (collLineIdxArr[DIR_DOWN] != -1 && deltaArr[DIR_UP] >= deltaArr[DIR_DOWN] && deltaArr[DIR_RIGHT] >= deltaArr[DIR_DOWN] && deltaArr[DIR_LEFT] >= deltaArr[DIR_DOWN]) {
		curLineIdx = collLineIdxArr[DIR_DOWN];
		curLineDataIdx = collLineDataIdxArr[DIR_DOWN];
	} else if (collLineIdxArr[DIR_UP] != -1 && deltaArr[DIR_DOWN] >= deltaArr[DIR_UP] && deltaArr[DIR_RIGHT] >= deltaArr[DIR_UP] && deltaArr[DIR_LEFT] >= deltaArr[DIR_UP]) {
		curLineIdx = collLineIdxArr[DIR_UP];
		curLineDataIdx = collLineDataIdxArr[DIR_UP];
	} else if (collLineIdxArr[DIR_RIGHT] != -1 && deltaArr[DIR_UP] >= deltaArr[DIR_RIGHT] && deltaArr[DIR_DOWN] >= deltaArr[DIR_RIGHT] && deltaArr[DIR_LEFT] >= deltaArr[DIR_RIGHT]) {
		curLineIdx = collLineIdxArr[DIR_RIGHT];
		curLineDataIdx = collLineDataIdxArr[DIR_RIGHT];
	} else if (collLineIdxArr[DIR_LEFT] != -1 && deltaArr[DIR_DOWN] >= deltaArr[DIR_LEFT] && deltaArr[DIR_RIGHT] >= deltaArr[DIR_LEFT] && deltaArr[DIR_UP] >= deltaArr[DIR_LEFT]) {
		curLineIdx = collLineIdxArr[DIR_LEFT];
		curLineDataIdx = collLineDataIdxArr[DIR_LEFT];
	}

	for (int i = 0; i <= 8; ++i) {
		collLineIdxArr[i] = -1;
		collLineDataIdxArr[i] = 0;
		deltaArr[i] = INVALID_LINE_VALUE;
		stepArr[i] = INVALID_LINE_VALUE;
	}

	tmpDelta = 0;
	for (int tmpY = fromY; tmpY < _vm->_graphicsMan->_maxY; tmpY++, tmpDelta++) {
		if (checkCollisionLine(fromX, tmpY, &collLineDataIdxArr[DIR_DOWN], &collLineIdxArr[DIR_DOWN], 0, _lastLine) && collLineIdxArr[DIR_DOWN] <= _lastLine)
			break;
		collLineDataIdxArr[DIR_DOWN] = 0;
		collLineIdxArr[DIR_DOWN] = -1;
	}
	deltaArr[DIR_DOWN] = tmpDelta + 1;

	tmpDelta = 0;
	for (int tmpY = fromY; tmpY > _vm->_graphicsMan->_minY; tmpY--) {
		if (checkCollisionLine(fromX, tmpY, &collLineDataIdxArr[DIR_UP], &collLineIdxArr[DIR_UP], 0, _lastLine) && collLineIdxArr[DIR_UP] <= _lastLine)
			break;
		collLineDataIdxArr[DIR_UP] = 0;
		collLineIdxArr[DIR_UP] = -1;
		++tmpDelta;
		if (collLineIdxArr[DIR_DOWN] != -1 && tmpDelta > 80)
			break;
	}
	deltaArr[DIR_UP] = tmpDelta + 1;

	tmpDelta = 0;
	for (int tmpX = fromX; tmpX < _vm->_graphicsMan->_maxX; tmpX++) {
		if (checkCollisionLine(tmpX, fromY, &collLineDataIdxArr[DIR_RIGHT], &collLineIdxArr[DIR_RIGHT], 0, _lastLine) && collLineIdxArr[DIR_RIGHT] <= _lastLine)
			break;
		collLineDataIdxArr[DIR_RIGHT] = 0;
		collLineIdxArr[DIR_RIGHT] = -1;
		++tmpDelta;
		if ((collLineIdxArr[DIR_DOWN] != -1 || collLineIdxArr[DIR_UP] != -1) && (tmpDelta > 100))
			break;
	}
	deltaArr[DIR_RIGHT] = tmpDelta + 1;

	tmpDelta = 0;
	for (int tmpX = fromX; tmpX > _vm->_graphicsMan->_minX; tmpX--) {
		if (checkCollisionLine(tmpX, fromY, &collLineDataIdxArr[DIR_LEFT], &collLineIdxArr[DIR_LEFT], 0, _lastLine) && collLineIdxArr[DIR_LEFT] <= _lastLine)
			break;
		collLineDataIdxArr[DIR_LEFT] = 0;
		collLineIdxArr[DIR_LEFT] = -1;
		++tmpDelta;
		if ((collLineIdxArr[DIR_DOWN] != -1 || collLineIdxArr[DIR_UP] != -1 || collLineIdxArr[DIR_RIGHT] != -1) && (tmpDelta > 100))
			break;
	}
	deltaArr[DIR_LEFT] = tmpDelta + 1;

	if (collLineIdxArr[DIR_UP] != -1)
		stepArr[DIR_UP] = abs(collLineIdxArr[DIR_UP] - curLineIdx);

	if (collLineIdxArr[DIR_RIGHT] != -1)
		stepArr[DIR_RIGHT] = abs(collLineIdxArr[DIR_RIGHT] - curLineIdx);

	if (collLineIdxArr[DIR_DOWN] != -1)
		stepArr[DIR_DOWN] = abs(collLineIdxArr[DIR_DOWN] - curLineIdx);

	if (collLineIdxArr[DIR_LEFT] != -1)
		stepArr[DIR_LEFT] = abs(collLineIdxArr[DIR_LEFT] - curLineIdx);

	if (collLineIdxArr[DIR_UP] == -1 && collLineIdxArr[DIR_RIGHT] == -1 && collLineIdxArr[DIR_DOWN] == -1 && collLineIdxArr[DIR_LEFT] == -1)
		error("Nearest point not found");

	int delta = 0;
	if (collLineIdxArr[DIR_UP] != -1 && stepArr[DIR_RIGHT] >= stepArr[DIR_UP] && stepArr[DIR_DOWN] >= stepArr[DIR_UP] && stepArr[DIR_LEFT] >= stepArr[DIR_UP]) {
		lineIdx = collLineIdxArr[DIR_UP];
		delta = deltaArr[DIR_UP];
		newDir = DIR_UP;
		lineDataIdx = collLineDataIdxArr[DIR_UP];
	} else if (collLineIdxArr[DIR_DOWN] != -1 && stepArr[DIR_RIGHT] >= stepArr[DIR_DOWN] && stepArr[DIR_UP] >= stepArr[DIR_DOWN] && stepArr[DIR_LEFT] >= stepArr[DIR_DOWN]) {
		lineIdx = collLineIdxArr[DIR_DOWN];
		delta = deltaArr[DIR_DOWN];
		newDir = DIR_DOWN;
		lineDataIdx = collLineDataIdxArr[DIR_DOWN];
	} else if (collLineIdxArr[DIR_RIGHT] != -1 && stepArr[DIR_UP] >= stepArr[DIR_RIGHT] && stepArr[DIR_DOWN] >= stepArr[DIR_RIGHT] && stepArr[DIR_LEFT] >= stepArr[DIR_RIGHT]) {
		lineIdx = collLineIdxArr[DIR_RIGHT];
		delta = deltaArr[DIR_RIGHT];
		newDir = DIR_RIGHT;
		lineDataIdx = collLineDataIdxArr[DIR_RIGHT];
	} else if (collLineIdxArr[DIR_LEFT] != -1 && stepArr[DIR_UP] >= stepArr[DIR_LEFT] && stepArr[DIR_DOWN] >= stepArr[DIR_LEFT] && stepArr[DIR_RIGHT] >= stepArr[DIR_LEFT]) {
		lineIdx = collLineIdxArr[DIR_LEFT];
		delta = deltaArr[DIR_LEFT];
		newDir = DIR_LEFT;
		lineDataIdx = collLineDataIdxArr[DIR_LEFT];
	}

	int bestRouteNum = characterRoute(fromX, fromY, clipDestX, clipDestY, lineIdx, curLineIdx, 0);

	if (bestRouteNum == 1)
		return _bestRoute;

	if (bestRouteNum == 2) {
		lineIdx = _newLineIdx;
		lineDataIdx = _newLineDataIdx;
		routeIdx = _newRouteIdx;
	} else {
		switch (newDir) {
		case DIR_UP:
			for (int deltaY = 0; deltaY < delta; deltaY++) {
				if (checkCollisionLine(fromX, fromY - deltaY, &foundDataIdx, &foundLineIdx, _lastLine + 1, _linesNumb) && _lastLine < foundLineIdx) {
					int tmpRouteIdx = computeRouteIdx(foundLineIdx, foundDataIdx, fromX, fromY - deltaY, fromX, fromY - delta, routeIdx, _bestRoute);
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
			break;
		case DIR_DOWN:
			for (int deltaY = 0; deltaY < delta; deltaY++) {
				if (checkCollisionLine(fromX, deltaY + fromY, &foundDataIdx, &foundLineIdx, _lastLine + 1, _linesNumb)
				        && _lastLine < foundLineIdx) {
					int tmpRouteIdx = computeRouteIdx(foundLineIdx, foundDataIdx, fromX, deltaY + fromY, fromX, delta + fromY, routeIdx, &_bestRoute[0]);
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
			break;
		case DIR_LEFT:
			for (int deltaX = 0; deltaX < delta; deltaX++) {
				if (checkCollisionLine(fromX - deltaX, fromY, &foundDataIdx, &foundLineIdx, _lastLine + 1, _linesNumb) && _lastLine < foundLineIdx) {
					int tmpRouteIdx = computeRouteIdx(foundLineIdx, foundDataIdx, fromX - deltaX, fromY, fromX - delta, fromY, routeIdx, &_bestRoute[0]);
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
			break;
		case DIR_RIGHT:
			for (int deltaX = 0; deltaX < delta; deltaX++) {
				if (checkCollisionLine(deltaX + fromX, fromY, &foundDataIdx, &foundLineIdx, _lastLine + 1, _linesNumb) && _lastLine < foundLineIdx) {
					int tmpRouteIdx = computeRouteIdx(foundLineIdx, foundDataIdx, deltaX + fromX, fromY, delta + fromX, fromY, routeIdx, &_bestRoute[0]);
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
			break;
		default:
			break;
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
						bestRouteNum = characterRoute(_lineItem[idx]._lineData[2 * dataIdx], _lineItem[idx]._lineData[2 * dataIdx + 1], clipDestX, clipDestY, idx, curLineIdx, routeIdx);
						if (bestRouteNum == 1)
							return &_bestRoute[0];
						if (bestRouteNum == 2 || MIRACLE(curLineX, curLineY, idx, curLineIdx, routeIdx)) {
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

				bestRouteNum = characterRoute(curLineX, curLineY, clipDestX, clipDestY, idx, curLineIdx, routeIdx);
				if (bestRouteNum == 1)
					return &_bestRoute[0];
				if (bestRouteNum == 2 || MIRACLE(curLineX, curLineY, idx, curLineIdx, routeIdx)) {
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
			for (int i = lineIdx - 1; i > curLineIdx; i--) {
				for (int dataIdx = _lineItem[i]._lineDataEndIdx - 1; dataIdx > -1; dataIdx--) {
					curLineX = _lineItem[i]._lineData[2 * dataIdx];
					curLineY = _lineItem[i]._lineData[2 * dataIdx + 1];
					_bestRoute[routeIdx].set(_lineItem[i]._lineData[2 * dataIdx], _lineItem[i]._lineData[2 * dataIdx + 1], _lineItem[i]._directionRouteDec);
					routeIdx++;
					if (_lineItem[i]._lineDataEndIdx > 30 && dataIdx == _lineItem[i]._lineDataEndIdx / 2) {
						bestRouteNum = characterRoute(curLineX, curLineY, clipDestX, clipDestY, i, curLineIdx, routeIdx);
						if (bestRouteNum == 1)
							return &_bestRoute[0];
						if (bestRouteNum == 2 || MIRACLE(curLineX, curLineY, i, curLineIdx, routeIdx)) {
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

				bestRouteNum = characterRoute(curLineX, curLineY, clipDestX, clipDestY, i, curLineIdx, routeIdx);
				if (bestRouteNum == 1)
					return &_bestRoute[0];
				if (bestRouteNum == 2 || MIRACLE(curLineX, curLineY, i, curLineIdx, routeIdx)) {
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
		if (lineDataIdx <= curLineDataIdx)
			routeIdx = _lineItem[curLineIdx].appendToRouteInc(lineDataIdx, curLineDataIdx, _bestRoute, routeIdx);
		else
			routeIdx = _lineItem[curLineIdx].appendToRouteDec(lineDataIdx, curLineDataIdx, _bestRoute, routeIdx);
	}
	if (characterRoute(_bestRoute[routeIdx - 1]._x, _bestRoute[routeIdx - 1]._y, clipDestX, clipDestY, -1, -1, routeIdx) != 1) {
		_bestRoute[routeIdx].invalidate();
	}

	return &_bestRoute[0];
}

void LinesManager::useRoute0(int idx, int curRouteIdx) {
	debugC(5, kDebugPath, "useRoute0(%d, %d)", idx, curRouteIdx);
	if (idx) {
		int i = 0;
		do {
			assert(curRouteIdx <= 8000);
			_bestRoute[curRouteIdx++] = _testRoute0[i++];
		} while (_testRoute0[i].isValid());
	}
	_bestRoute[curRouteIdx].invalidate();
}

void LinesManager::useRoute1(int idx, int curRouteIdx) {
	debugC(5, kDebugPath, "useRoute1(%d, %d)", idx, curRouteIdx);
	if (idx) {
		int i = 0;
		do {
			assert(curRouteIdx <= 8000);
			_bestRoute[curRouteIdx++] = _testRoute1[i++];
		} while (_testRoute1[i].isValid());
	}
	_bestRoute[curRouteIdx].invalidate();
}

void LinesManager::useRoute2(int idx, int curRouteIdx) {
	debugC(5, kDebugPath, "useRoute2(%d, %d)", idx, curRouteIdx);
	if (idx) {
		int i = 0;
		do {
			assert(curRouteIdx <= 8000);
			_bestRoute[curRouteIdx++] = _testRoute2[i++];
		} while (_testRoute2[i].isValid());
	}
	_bestRoute[curRouteIdx].invalidate();
}

int LinesManager::characterRoute(int fromX, int fromY, int destX, int destY, int startLineIdx, int endLineIdx, int routeIdx) {
	debugC(5, kDebugPath, "characterRoute(%d, %d, %d, %d, %d, %d, %d)", fromX, fromY, destX, destY, startLineIdx, endLineIdx, routeIdx);
	int collDataIdxRoute2 = 0;

	int curX = fromX;
	int curY = fromY;
	int curRouteIdx = routeIdx;
	bool dummyLineFl = false;
	if (startLineIdx == -1 && endLineIdx == -1)
		dummyLineFl = true;
	int foundDataIdx;
	int foundLineIdx = startLineIdx;
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
	int oldX = curX;
	int oldY = curY;
	int idxRoute0 = 0;
	int collLineIdxRoute0 = -1;
	int collLineIdxRoute1 = -1;
	int collLineIdxRoute2 = -1;

	int repeatFlag = 0;
	int collDataIdxRoute0 = 0;
	int collDataIdxRoute1 = 0;
	for (;;) {
		int newX = curX;
		int newY = curY;
		if (destX >= curX - 2 && destX <= curX + 2 && destY >= curY - 2 && destY <= curY + 2) {
			_testRoute0[idxRoute0].invalidate();
			useRoute0(idxRoute0, curRouteIdx);
			return 1;
		}
		int distX = abs(curX - destX) + 1;
		int distY = abs(curY - destY) + 1;
		int maxDist;
		if (distX > distY)
			maxDist = distX;
		else
			maxDist = distY;
		maxDist--;
		assert(maxDist != 0);
		int stepX = 1000 * distX / maxDist;
		int stepY = 1000 * distY / maxDist;
		if (destX < curX)
			stepX = -stepX;
		if (destY < curY)
			stepY = -stepY;
		int vertDirection = (int16)stepX / 1000;
		int horzDirection = (int16)stepY / 1000;
		Directions newDirection = DIR_NONE;
		if (horzDirection == -1 && (stepX >= 0 && stepX <= 150))
			newDirection = DIR_UP;
		if (vertDirection == 1 && (stepY >= -1 && stepY <= 150))
			newDirection = DIR_RIGHT;
		if (horzDirection == 1 && (stepX >= -150 && stepX <= 150))
			newDirection = DIR_DOWN;
		if (vertDirection == -1 && (stepY >= -150 && stepY <= 150))
			newDirection = DIR_LEFT;
		if (horzDirection == -1 && (stepX >= -150 && stepX <= 0))
			newDirection = DIR_UP;

		if (newDirection == DIR_NONE && !checkSmoothMove(curX, newY, destX, destY) && !makeSmoothMove(curX, newY, destX, destY)) {
			newDirection = _smoothMoveDirection;
			int smoothRouteIdx = 0;
			for (smoothRouteIdx = 0; _smoothRoute[smoothRouteIdx]._posX != -1 && _smoothRoute[smoothRouteIdx]._posY != -1; ++smoothRouteIdx) {
				if (checkCollisionLine(_smoothRoute[smoothRouteIdx]._posX, _smoothRoute[smoothRouteIdx]._posY, &collDataIdxRoute0, &collLineIdxRoute0, 0, _linesNumb)) {
					if (collLineIdxRoute0 > _lastLine)
						collLineIdxRoute0 = -1;
					break;
				}

				_testRoute0[idxRoute0].set(_smoothRoute[smoothRouteIdx]._posX, _smoothRoute[smoothRouteIdx]._posY, newDirection);
				idxRoute0++;

				if (repeatFlag == 1) {
					repeatFlag = 2;
					break;
				}
			}

			if (repeatFlag != 2 && _smoothRoute[smoothRouteIdx]._posX != -1 && _smoothRoute[smoothRouteIdx]._posY != -1)
				break;

			repeatFlag = 1;
			newX = _smoothRoute[smoothRouteIdx - 1]._posX;
			newY = _smoothRoute[smoothRouteIdx - 1]._posY;
		}
		int newDistX = abs(newX - destX) + 1;
		int newDistY = abs(newY - destY) + 1;
		int newMaxDist = newDistY;
		if (newDistX > newDistY)
			newMaxDist = newDistX;
		if (newMaxDist <= 10) {
			_testRoute0[idxRoute0].invalidate();
			useRoute0(idxRoute0, curRouteIdx);
			return 1;
		}
		int newStepX = 1000 * newDistX / (newMaxDist - 1);
		int newStepY = 1000 * newDistY / (newMaxDist - 1);
		if (destX < newX)
			newStepX = -newStepX;
		if (destY < newY)
			newStepY = -newStepY;
		int newVertDirection = newStepX / 1000;
		int newHorzDirection = newStepY / 1000;
		int newSmoothX = 1000 * newX;
		int newSmoothY = 1000 * newY;
		int curPosX = newSmoothX / 1000;
		int curPosY = newSmoothY / 1000;
		if (!(newStepX / 1000) && newHorzDirection == -1)
			newDirection = DIR_UP;
		if (newVertDirection == 1) {
			if (newHorzDirection == -1)
				newDirection = DIR_UP_RIGHT;
			if (!newHorzDirection)
				newDirection = DIR_RIGHT;
			if (newHorzDirection == 1)
				newDirection = DIR_DOWN_RIGHT;
		}
		if (!newVertDirection && newHorzDirection == 1)
			newDirection = DIR_DOWN;
		if ((newVertDirection != -1) && (newHorzDirection == -1)) {
			if (newStepX >= 0 && newStepX < 510)
				newDirection = DIR_UP;
			else if (newStepX >= 510 && newStepX <= 1000)
				newDirection = DIR_UP_RIGHT;
		} else {
			if (newHorzDirection == 1)
				newDirection = DIR_DOWN_LEFT;
			else if (!newHorzDirection)
				newDirection = DIR_LEFT;
			else if (newHorzDirection == -1) {
				if (newStepX >= 0 && newStepX < 510)
					newDirection = DIR_UP;
				else if (newStepX >= 510 && newStepX <= 1000)
					newDirection = DIR_UP_RIGHT;
				else
					newDirection = DIR_UP_LEFT;
			}
		}
		if (newVertDirection == 1) {
			if (newStepY >= -1000 && newStepY <= -510)
				newDirection = DIR_UP_RIGHT;
			if (newStepY >= -510 && newStepY <= 510)
				newDirection = DIR_RIGHT;
			if (newStepY >= 510 && newStepY <= 1000)
				newDirection = DIR_DOWN_RIGHT;
		}
		if (newHorzDirection == 1) {
			if (newStepX >= 510 && newStepX <= 1000)
				newDirection = DIR_DOWN_RIGHT;
			if (newStepX >= -510 && newStepX <= 510)
				newDirection = DIR_DOWN;
			if (newStepX >= -1000 && newStepX <= -510)
				newDirection = DIR_DOWN_LEFT;
		}
		if (newVertDirection == -1) {
			if (newStepY >= 510 && newStepY <= 1000)
				newDirection = DIR_DOWN_LEFT;
			if (newStepY >= -510 && newStepY <= 510)
				newDirection = DIR_LEFT;
			if (newStepY >= -1000 && newStepY <= -510)
				newDirection = DIR_UP_LEFT;
		}
		if (newHorzDirection == -1) {
			if (newStepX >= -1000 && newStepX <= -510)
				newDirection = DIR_UP_LEFT;
			if (newStepX >= -510 && newStepX <= 0)
				newDirection = DIR_UP;
		}
		if (newMaxDist + 1 <= 0) {
			_testRoute0[idxRoute0].invalidate();
			useRoute0(idxRoute0, curRouteIdx);
			return 1;
		}
		int curDist = 0;
		while (!checkCollisionLine(curPosX, curPosY, &collDataIdxRoute0, &collLineIdxRoute0, 0, _linesNumb)) {
			_testRoute0[idxRoute0].set(curPosX, curPosY, newDirection);
			newSmoothX += newStepX;
			newSmoothY += newStepY;
			curPosX = newSmoothX / 1000;
			curPosY = newSmoothY / 1000;
			idxRoute0++;
			++curDist;
			if (curDist >= newMaxDist + 1) {
				_testRoute0[idxRoute0].invalidate();
				useRoute0(idxRoute0, curRouteIdx);
				return 1;
			}
		}
		if (_lastLine >= collLineIdxRoute0)
			break;
		int tmpRouteIdx = computeRouteIdx(collLineIdxRoute0, collDataIdxRoute0, curPosX, curPosY, destX, destY, idxRoute0, _testRoute0);
		if (tmpRouteIdx == -1) {
			useRoute0(idxRoute0, curRouteIdx);
			return 1;
		}
		idxRoute0 = tmpRouteIdx;
		if (_newPosX != -1 || _newPosY != -1) {
			collLineIdxRoute0 = -1;
			break;
		}
		curX = -1;
		curY = -1;
	}

	_testRoute0[idxRoute0].invalidate();

	int idxRoute1 = 0;
	int posXRoute1 = oldX;
	int posYRoute1 = oldY;

	while (true) {

		if (destX >= posXRoute1 - 2 && destX <= posXRoute1 + 2 && destY >= posYRoute1 - 2 && destY <= posYRoute1 + 2) {
			_testRoute1[idxRoute1].invalidate();
			useRoute1(idxRoute1, curRouteIdx);
			return 1;
		}
		while (posXRoute1 != destX) {
			if (checkCollisionLine(posXRoute1, posYRoute1, &collDataIdxRoute1, &collLineIdxRoute1, 0, _linesNumb)) {
				if (collLineIdxRoute1 > _lastLine)
					collLineIdxRoute1 = -1;
				break;
			}

			if (posXRoute1 < destX)
				_testRoute1[idxRoute1++].set(posXRoute1++, posYRoute1, DIR_RIGHT);
			else
				_testRoute1[idxRoute1++].set(posXRoute1--, posYRoute1, DIR_LEFT);
		}
		if (posXRoute1 != destX)
			break;

		int curPosY = posYRoute1;
		while (curPosY != destY) {
			if (checkCollisionLine(destX, curPosY, &collDataIdxRoute1, &collLineIdxRoute1, 0, _linesNumb)) {
				if (collLineIdxRoute1 <= _lastLine)
					break;

				int tmpRouteIdx = computeRouteIdx(collLineIdxRoute1, collDataIdxRoute1, destX, curPosY, destX, destY, idxRoute1, _testRoute1);
				if (tmpRouteIdx == -1) {
					useRoute1(idxRoute1, curRouteIdx);
					return 1;
				}
				idxRoute1 = tmpRouteIdx;
				if (_newPosX != -1 && _newPosY != -1)
					break;
			}

			if (curPosY < destY)
				_testRoute1[idxRoute1++].set(destX, curPosY++, DIR_DOWN);
			else
				_testRoute1[idxRoute1++].set(destX, curPosY--, DIR_UP);
		}
		if (curPosY == destY) {
			_testRoute1[idxRoute1].invalidate();
			useRoute1(idxRoute1, curRouteIdx);
			return 1;
		}
		if (collLineIdxRoute1 <= _lastLine)
			break;
		posXRoute1 = _newPosX;
		posYRoute1 = _newPosY;
		bool colRes = checkCollisionLine(_newPosX, _newPosY, &collDataIdxRoute1, &collLineIdxRoute1, 0, _lastLine);
		if (colRes && collLineIdxRoute1 <= _lastLine)
			break;
	}

	_testRoute1[idxRoute1].invalidate();
	idxRoute1 = 0;
	int posXRoute2 = oldX;
	int posYRoute2 = oldY;
	while (true) {
		int curPosX;
		if (destX >= posXRoute2 - 2 && destX <= posXRoute2 + 2 && destY >= posYRoute2 - 2 && destY <= posYRoute2 + 2) {
			_testRoute2[idxRoute1].invalidate();
			useRoute2(idxRoute1, curRouteIdx);
			return 1;
		}

		int curPosYRoute2 = posYRoute2;
		while (curPosYRoute2 != destY) {
			if (checkCollisionLine(posXRoute2, curPosYRoute2, &collDataIdxRoute2, &collLineIdxRoute2, 0, _linesNumb)) {
				if (collLineIdxRoute2 > _lastLine)
					collLineIdxRoute2 = -1;
				break;
			}

			if (curPosYRoute2 < destY)
				_testRoute2[idxRoute1++].set(posXRoute2, curPosYRoute2++, DIR_DOWN);
			else
				_testRoute2[idxRoute1++].set(posXRoute2, curPosYRoute2--, DIR_UP);
		}
		if (curPosYRoute2 != destY)
			break;

		curPosX = posXRoute2;
		while (curPosX != destX) {
			if (checkCollisionLine(curPosX, destY, &collDataIdxRoute2, &collLineIdxRoute2, 0, _linesNumb)) {
				if (collLineIdxRoute2 <= _lastLine)
					break;

				int tmpRouteIdx = computeRouteIdx(collLineIdxRoute2, collDataIdxRoute2, curPosX, destY, destX, destY, idxRoute1, _testRoute2);
				if (tmpRouteIdx == -1) {
					useRoute2(idxRoute1, curRouteIdx);
					return 1;
				}
				idxRoute1 = tmpRouteIdx;
				if (_newPosX != -1 && _newPosY != -1)
					break;
			}

			if (curPosX < destX)
				_testRoute2[idxRoute1++].set(curPosX++, destY, DIR_RIGHT);
			else
				_testRoute2[idxRoute1++].set(curPosX--, destY, DIR_LEFT);
		}
		if (curPosX == destX) {
			collLineIdxRoute2 = -1;
			_testRoute2[idxRoute1].invalidate();
			useRoute2(idxRoute1, curRouteIdx);
			return 1;
		}
		if (collLineIdxRoute2 <= _lastLine)
			break;

		posXRoute2 = _newPosX;
		posYRoute2 = _newPosY;
		bool colResult = checkCollisionLine(_newPosX, _newPosY, &collDataIdxRoute2, &collLineIdxRoute2, 0, _lastLine);
		if (colResult && collLineIdxRoute2 <= _lastLine)
			break;
	}

	_testRoute2[idxRoute1].invalidate();

	if (!dummyLineFl) {
		if (endLineIdx > foundLineIdx) {
			if (_testRoute0[0]._x != -1 && collLineIdxRoute0 > foundLineIdx && collLineIdxRoute1 <= collLineIdxRoute0 && collLineIdxRoute2 <= collLineIdxRoute0 && endLineIdx >= collLineIdxRoute0) {
				_newLineIdx = collLineIdxRoute0;
				_newLineDataIdx = collDataIdxRoute0;
				int i = 0;
				do {
					assert(curRouteIdx <= 8000);
					_bestRoute[curRouteIdx++] = _testRoute0[i++];
				} while (_testRoute0[i].isValid());
				_newRouteIdx = curRouteIdx;
				return 2;
			}
			if (_testRoute1[0]._x != -1 && foundLineIdx < collLineIdxRoute1 && collLineIdxRoute2 <= collLineIdxRoute1 && collLineIdxRoute0 <= collLineIdxRoute1 && endLineIdx >= collLineIdxRoute1) {
				_newLineIdx = collLineIdxRoute1;
				_newLineDataIdx = collDataIdxRoute1;
				int i = 0;
				do {
					assert(curRouteIdx <= 8000);
					_bestRoute[curRouteIdx++] = _testRoute1[i++];
				} while (_testRoute1[i].isValid());
				_newRouteIdx = curRouteIdx;
				return 2;
			}
			if (_testRoute2[0]._x != -1 && foundLineIdx < collLineIdxRoute2 && collLineIdxRoute1 < collLineIdxRoute2 && collLineIdxRoute0 < collLineIdxRoute2 && endLineIdx >= collLineIdxRoute2) {
				_newLineIdx = collLineIdxRoute2;
				_newLineDataIdx = collDataIdxRoute2;
				int i = 0;
				do {
					assert(curRouteIdx <= 8000);
					_bestRoute[curRouteIdx++] = _testRoute2[i++];
				} while (_testRoute2[i].isValid());
				_newRouteIdx = curRouteIdx;
				return 2;
			}
		}
		if (endLineIdx < foundLineIdx) {
			if (collLineIdxRoute0 == -1)
				collLineIdxRoute0 = INVALID_LINE_VALUE;
			if (collLineIdxRoute1 == -1)
				collLineIdxRoute0 = INVALID_LINE_VALUE;
			if (collLineIdxRoute2 == -1)
				collLineIdxRoute0 = INVALID_LINE_VALUE;
			if (_testRoute1[0]._x != -1 && collLineIdxRoute1 < foundLineIdx && collLineIdxRoute2 >= collLineIdxRoute1 && collLineIdxRoute0 >= collLineIdxRoute1 && endLineIdx <= collLineIdxRoute1) {
				_newLineIdx = collLineIdxRoute1;
				_newLineDataIdx = collDataIdxRoute1;
				int i = 0;
				do {
					assert(curRouteIdx <= 8000);
					_bestRoute[curRouteIdx++] = _testRoute1[i++];
				} while (_testRoute1[i].isValid());
				_newRouteIdx = curRouteIdx;
				return 2;
			}
			if (_testRoute2[0]._x != -1 && foundLineIdx > collLineIdxRoute2 && collLineIdxRoute1 >= collLineIdxRoute2 && collLineIdxRoute0 >= collLineIdxRoute2 && endLineIdx <= collLineIdxRoute2) {
				_newLineIdx = collLineIdxRoute2;
				_newLineDataIdx = collDataIdxRoute2;
				int i = 0;
				do {
					assert(curRouteIdx <= 8000);
					_bestRoute[curRouteIdx++] = _testRoute2[i++];
				} while (_testRoute2[i].isValid());
				_newRouteIdx = curRouteIdx;
				return 2;
			}

			if (_testRoute0[0]._x != -1 && foundLineIdx > collLineIdxRoute0 && collLineIdxRoute1 >= collLineIdxRoute0 && collLineIdxRoute2 >= collLineIdxRoute0 && endLineIdx <= collLineIdxRoute0) {
				_newLineIdx = collLineIdxRoute0;
				_newLineDataIdx = collDataIdxRoute0;
				int i = 0;
				do {
					assert(curRouteIdx <= 8000);
					_bestRoute[curRouteIdx++] = _testRoute0[i++];
				} while (_testRoute0[i].isValid());
				_newRouteIdx = curRouteIdx;
				return 2;
			}
		}
	}
	return 0;
}

RouteItem *LinesManager::cityMapCarRoute(int x1, int y1, int x2, int y2) {
	debugC(5, kDebugPath, "cityMapCarRoute(%d, %d, %d, %d)", x1, y1, x2, y2);
	RouteItem *result;
	int arrDelta[10];
	int arrDataIdx[10];
	int arrLineIdx[10];

	int clipX2 = x2;
	int clipY2 = y2;
	if (x2 <= 14)
		clipX2 = 15;
	if (y2 <= 14)
		clipY2 = 15;
	if (clipX2 > _vm->_graphicsMan->_maxX - 10)
		clipX2 = _vm->_graphicsMan->_maxX - 10;
	if (clipY2 > 445)
		clipY2 = 440;

	int delta = 0;
	for (delta = 0; clipY2 + delta < _vm->_graphicsMan->_maxY; delta++) {
		if (checkCollisionLine(clipX2, clipY2 + delta, &arrDataIdx[DIR_DOWN], &arrLineIdx[DIR_DOWN], 0, _lastLine) && arrLineIdx[DIR_DOWN] <= _lastLine)
			break;
		arrDataIdx[DIR_DOWN] = 0;
		arrLineIdx[DIR_DOWN] = -1;
	}
	arrDelta[DIR_DOWN] = delta;

	for (delta = 0; clipY2 - delta > _vm->_graphicsMan->_minY; delta++) {
		if (checkCollisionLine(clipX2, clipY2 - delta , &arrDataIdx[DIR_UP], &arrLineIdx[DIR_UP], 0, _lastLine) && arrLineIdx[DIR_UP] <= _lastLine)
			break;
		arrDataIdx[DIR_UP] = 0;
		arrLineIdx[DIR_UP] = -1;
		if (arrDelta[DIR_DOWN] < delta && arrLineIdx[DIR_DOWN] != -1)
			break;
	}
	arrDelta[DIR_UP] = delta;

	for (delta = 0; clipX2 + delta < _vm->_graphicsMan->_maxX; delta++) {
		if (checkCollisionLine(clipX2 + delta, clipY2, &arrDataIdx[DIR_RIGHT], &arrLineIdx[DIR_RIGHT], 0, _lastLine) && arrLineIdx[DIR_RIGHT] <= _lastLine)
			break;
		arrDataIdx[DIR_RIGHT] = 0;
		arrLineIdx[DIR_RIGHT] = -1;
		if ((arrDelta[DIR_UP] <= delta && arrLineIdx[DIR_UP] != -1) || (arrDelta[DIR_DOWN] <= delta && arrLineIdx[DIR_DOWN] != -1))
			break;
	}
	arrDelta[DIR_RIGHT] = delta;

	for (delta = 0; clipX2 - delta > _vm->_graphicsMan->_minX; delta++) {
		if (checkCollisionLine(clipX2 - delta, clipY2, &arrDataIdx[DIR_LEFT], &arrLineIdx[DIR_LEFT], 0, _lastLine) && arrLineIdx[DIR_LEFT] <= _lastLine)
			break;
		arrDataIdx[DIR_LEFT] = 0;
		arrLineIdx[DIR_LEFT] = -1;
		if ((arrDelta[DIR_UP] <= delta && arrLineIdx[DIR_UP] != -1) || (arrDelta[DIR_RIGHT] <= delta && arrLineIdx[DIR_RIGHT] != -1) || (arrDelta[DIR_DOWN] <= delta && arrLineIdx[DIR_DOWN] != -1))
			break;
	}
	arrDelta[DIR_LEFT] = delta;

	if (arrLineIdx[DIR_UP] == -1)
		arrDelta[DIR_UP] = INVALID_LINE_VALUE;
	if (arrLineIdx[DIR_RIGHT] == -1)
		arrDelta[DIR_RIGHT] = INVALID_LINE_VALUE;
	if (arrLineIdx[DIR_DOWN] == -1)
		arrDelta[DIR_DOWN] = INVALID_LINE_VALUE;
	if (arrLineIdx[DIR_LEFT] == -1)
		arrDelta[DIR_LEFT] = INVALID_LINE_VALUE;
	if (arrLineIdx[DIR_UP] != -1 || arrLineIdx[DIR_RIGHT] != -1 || arrLineIdx[DIR_DOWN] != -1 || arrLineIdx[DIR_LEFT] != -1) {
		int curLineDataIdx = 0;
		int curLineIdx = 0;
		if (arrLineIdx[DIR_DOWN] != -1 && arrDelta[DIR_UP] >= arrDelta[DIR_DOWN] && arrDelta[DIR_RIGHT] >= arrDelta[DIR_DOWN] && arrDelta[DIR_LEFT] >= arrDelta[DIR_DOWN]) {
			curLineIdx = arrLineIdx[DIR_DOWN];
			curLineDataIdx = arrDataIdx[DIR_DOWN];
		} else if (arrLineIdx[DIR_UP] != -1 && arrDelta[DIR_DOWN] >= arrDelta[DIR_UP] && arrDelta[DIR_RIGHT] >= arrDelta[DIR_UP] && arrDelta[DIR_LEFT] >= arrDelta[DIR_UP]) {
			curLineIdx = arrLineIdx[DIR_UP];
			curLineDataIdx = arrDataIdx[DIR_UP];
		} else if (arrLineIdx[DIR_RIGHT] != -1 && arrDelta[DIR_UP] >= arrDelta[DIR_RIGHT] && arrDelta[DIR_DOWN] >= arrDelta[DIR_RIGHT] && arrDelta[DIR_LEFT] >= arrDelta[DIR_RIGHT]) {
			curLineIdx = arrLineIdx[DIR_RIGHT];
			curLineDataIdx = arrDataIdx[DIR_RIGHT];
		} else if (arrLineIdx[DIR_LEFT] != -1 && arrDelta[DIR_DOWN] >= arrDelta[DIR_LEFT] && arrDelta[DIR_RIGHT] >= arrDelta[DIR_LEFT] && arrDelta[DIR_UP] >= arrDelta[DIR_LEFT]) {
			curLineIdx = arrLineIdx[DIR_LEFT];
			curLineDataIdx = arrDataIdx[DIR_LEFT];
		}

		for (int i = 0; i <= 8; i++) {
			arrLineIdx[i] = -1;
			arrDataIdx[i] = 0;
			arrDelta[i] = INVALID_LINE_VALUE;
		}

		int superRouteIdx = 0;
		int curRouteDataIdx = 0;
		int curRouteLineIdx = 0;
		if (checkCollisionLine(x1, y1, &arrDataIdx[DIR_UP], &arrLineIdx[DIR_UP], 0, _lastLine)) {
			curRouteLineIdx = arrLineIdx[DIR_UP];
			curRouteDataIdx = arrDataIdx[DIR_UP];
		} else if (checkCollisionLine(x1, y1, &arrDataIdx[DIR_UP], &arrLineIdx[DIR_UP], 0, _linesNumb)) {
			int curRouteIdx = 0;
			int curRouteX;
			for (;;) {
				curRouteX = _testRoute2[curRouteIdx]._x;
				int curRouteY = _testRoute2[curRouteIdx]._y;
				Directions curRouteDir = _testRoute2[curRouteIdx]._dir;
				curRouteIdx++;

				if (checkCollisionLine(curRouteX, curRouteY, &arrDataIdx[DIR_UP], &arrLineIdx[DIR_UP], 0, _lastLine))
					break;

				_bestRoute[superRouteIdx].set(curRouteX, curRouteY, curRouteDir);

				_testRoute0[superRouteIdx].set(curRouteX, curRouteY, curRouteDir);
				superRouteIdx++;
				if (curRouteX == -1)
					break;
			}
			if (curRouteX != -1) {
				curRouteLineIdx = arrLineIdx[DIR_UP];
				curRouteDataIdx = arrDataIdx[DIR_UP];
			}
		} else {
			curRouteLineIdx = 1;
			curRouteDataIdx = 1;
			superRouteIdx = 0;
		}
		bool loopFl = true;
		while (loopFl) {
			loopFl = false;
			if (curRouteLineIdx < curLineIdx) {
				superRouteIdx = _lineItem[curRouteLineIdx].appendToRouteInc(curRouteDataIdx, _lineItem[curRouteLineIdx]._lineDataEndIdx - 2, _bestRoute, superRouteIdx);
				for (int j = curRouteLineIdx + 1; j < curLineIdx; ++j) {
					if (PLAN_TEST(_lineItem[j]._lineData[0], _lineItem[j]._lineData[1], superRouteIdx, j, curLineIdx)) {
						curRouteLineIdx = _newLineIdx;
						curRouteDataIdx = _newLineDataIdx;
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
				curRouteDataIdx = 0;
				curRouteLineIdx = curLineIdx;
			}
			if (curRouteLineIdx > curLineIdx) {
				superRouteIdx = _lineItem[curRouteLineIdx].appendToRouteDec(curRouteDataIdx, 0, _bestRoute, superRouteIdx);
				for (int l = curRouteLineIdx - 1; l > curLineIdx; --l) {
					if (PLAN_TEST(_lineItem[l]._lineData[2 * _lineItem[l]._lineDataEndIdx - 2], _lineItem[l]._lineData[2 * _lineItem[l]._lineDataEndIdx - 1], superRouteIdx, l, curLineIdx)) {
						curRouteLineIdx = _newLineIdx;
						curRouteDataIdx = _newLineDataIdx;
						superRouteIdx = _newRouteIdx;
						loopFl = true;
						break;
					}

					superRouteIdx = _lineItem[l].appendToRouteDec(_lineItem[l]._lineDataEndIdx - 2, 0, _bestRoute, superRouteIdx);
				}
				if (loopFl)
					continue;

				curRouteDataIdx = _lineItem[curLineIdx]._lineDataEndIdx - 1;
				curRouteLineIdx = curLineIdx;
			}
			if (curRouteLineIdx == curLineIdx) {
				if (curRouteDataIdx <= curLineDataIdx) {
					superRouteIdx = _lineItem[curLineIdx].appendToRouteInc(curRouteDataIdx, curLineDataIdx, _bestRoute, superRouteIdx);
				} else {
					superRouteIdx = _lineItem[curLineIdx].appendToRouteDec(curRouteDataIdx, curLineDataIdx, _bestRoute, superRouteIdx);
				}
			}
		}
		_bestRoute[superRouteIdx].invalidate();
		result = &_bestRoute[0];
	} else {
		result = NULL;
	}
	return result;
}

bool LinesManager::checkSmoothMove(int fromX, int fromY, int destX, int destY) {
	debugC(5, kDebugPath, "checkSmoothMove(%d, %d, %d, %d)", fromX, fromY, destX, destY);
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
		int foundLineIdx;
		int foundDataIdx;
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
	debugC(5, kDebugPath, "makeSmoothMove(%d, %d, %d, %d)", fromX, fromY, destX, destY);
	int curX = fromX;
	int curY = fromY;
	if (fromX > destX && destY > fromY) {
		int hopkinsIdx = 36;
		int smoothIdx = 0;
		int stepCount = 0;
		while (curX > destX && destY > curY) {
			int realSpeedX = _vm->_globals->_hopkinsItem[hopkinsIdx]._speedX;
			int realSpeedY = _vm->_globals->_hopkinsItem[hopkinsIdx]._speedY;
			int spriteSize = _vm->_globals->_spriteSize[curY];
			if (spriteSize < 0) {
				realSpeedX = _vm->_graphicsMan->zoomOut(realSpeedX, -spriteSize);
				realSpeedY = _vm->_graphicsMan->zoomOut(realSpeedY, -spriteSize);
			} else if (spriteSize > 0) {
				realSpeedX = _vm->_graphicsMan->zoomIn(realSpeedX, spriteSize);
				realSpeedY = _vm->_graphicsMan->zoomIn(realSpeedY, spriteSize);
			}
			int oldY = curY;
			for (int i = 0; i < realSpeedX; i++) {
				--curX;
				_smoothRoute[smoothIdx]._posX = curX;
				if (curY != oldY + realSpeedY)
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
			int realSpeedX = _vm->_globals->_hopkinsItem[hopkinsIdx]._speedX;
			int realSpeedY = _vm->_globals->_hopkinsItem[hopkinsIdx]._speedY;
			int spriteSize = _vm->_globals->_spriteSize[curY];
			if (spriteSize < 0) {
				realSpeedX = _vm->_graphicsMan->zoomOut(realSpeedX, -spriteSize);
				realSpeedY = _vm->_graphicsMan->zoomOut(realSpeedY, -spriteSize);
			} else if (spriteSize > 0) {
				realSpeedX = _vm->_graphicsMan->zoomIn(realSpeedX, spriteSize);
				realSpeedY = _vm->_graphicsMan->zoomIn(realSpeedY, spriteSize);
			}
			int oldY = curY;
			for (int i = 0; i < realSpeedX; i++) {
				++curX;
				_smoothRoute[smoothIdx]._posX = curX;
				if (curY != oldY + realSpeedY)
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
			int realSpeedX = _vm->_graphicsMan->zoomOut(_vm->_globals->_hopkinsItem[hopkinsIdx]._speedX, 25);
			int realSpeedY = _vm->_graphicsMan->zoomOut(_vm->_globals->_hopkinsItem[hopkinsIdx]._speedY, 25);
			int oldY = curY;
			for (int i = 0; i < realSpeedX; i++) {
				--curX;
				_smoothRoute[smoothIdx]._posX = curX;
				if ((uint16)curY != (uint16)oldY + realSpeedY)
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
			int realSpeedX = _vm->_graphicsMan->zoomOut(_vm->_globals->_hopkinsItem[hopkinsIdx]._speedX, 25);
			int realSpeedY = _vm->_graphicsMan->zoomOut(_vm->_globals->_hopkinsItem[hopkinsIdx]._speedY, 25);
			for (int i = 0; i < realSpeedX; i++) {
				++curX;
				_smoothRoute[smoothIdx]._posX = curX;
				if ((uint16)curY != (uint16)oldY + realSpeedY)
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

bool LinesManager::PLAN_TEST(int paramX, int paramY, int superRouteIdx, int paramStartLineIdx, int paramEndLineIdx) {
	debugC(5, kDebugPath, "PLAN_TEST(%d, %d, %d, %d, %d)", paramX, paramY, superRouteIdx, paramStartLineIdx, paramEndLineIdx);
	int sideTestUp;
	int sideTestDown;
	int sideTestLeft;
	int sideTestRight;
	int lineIdxTestUp;
	int lineIdxTestDown;
	int lineIdxTestLeft;
	int lineIdxTestRight;
	int dataIdxTestUp;
	int dataIdxTestDown;
	int dataIdxTestLeft;
	int dataIdxTestRight;

	int idxTestUp = testLine(paramX, paramY - 2, &sideTestUp, &lineIdxTestUp, &dataIdxTestUp);
	int idxTestDown = testLine(paramX, paramY + 2, &sideTestDown, &lineIdxTestDown, &dataIdxTestDown);
	int idxTestLeft = testLine(paramX - 2, paramY, &sideTestLeft, &lineIdxTestLeft, &dataIdxTestLeft);
	int idxTestRight = testLine(paramX + 2, paramY, &sideTestRight, &lineIdxTestRight, &dataIdxTestRight);
	if (idxTestUp == -1 && idxTestDown == -1 && idxTestLeft == -1 && idxTestRight == -1)
		return false;

	// Direction: 1 = Up, 2 = Down, 3 = Left, 4 = Right
	int direction;
	if (paramStartLineIdx == -1 || paramEndLineIdx == -1) {
		if (idxTestUp != -1)
			direction = 1;
		else if (idxTestDown != -1)
			direction = 2;
		else if (idxTestLeft != -1)
			direction = 3;
		else if (idxTestRight != -1)
			direction = 4;
		else
			return false;
	} else {
		int stepCountUp = 100;
		int stepCountDown = 100;
		int stepCountLeft = 100;
		int stepCountRight = 100;
		int paramStepCount = abs(paramStartLineIdx - paramEndLineIdx);
		if (idxTestUp != -1) {
			stepCountUp = abs(lineIdxTestUp - paramEndLineIdx);
		}
		if (idxTestDown != -1) {
			stepCountDown = abs(lineIdxTestDown - paramEndLineIdx);
		}
		if (idxTestLeft != -1) {
			stepCountLeft = abs(lineIdxTestLeft - paramEndLineIdx);
		}
		if (idxTestRight != -1) {
			stepCountRight = abs(lineIdxTestRight - paramEndLineIdx);
		}

		if (stepCountUp < paramStepCount && stepCountUp <= stepCountDown && stepCountUp <= stepCountLeft && stepCountUp <= stepCountRight)
			direction = 1;
		else if (paramStepCount > stepCountDown && stepCountUp >= stepCountDown && stepCountLeft >= stepCountDown && stepCountRight >= stepCountDown)
			direction = 2;
		else if (stepCountLeft < paramStepCount && stepCountLeft <= stepCountUp && stepCountLeft <= stepCountDown && stepCountLeft <= stepCountRight)
			direction = 3;
		else if (stepCountRight < paramStepCount && stepCountRight <= stepCountUp && stepCountRight <= stepCountDown && stepCountRight <= stepCountLeft)
			direction = 4;
		else
			return false;
	}

	int sideTest = 0;
	int idxTest = 0;
	if (direction == 1) {
		idxTest = idxTestUp;
		sideTest = sideTestUp;
		_newLineIdx = lineIdxTestUp;
		_newLineDataIdx = dataIdxTestUp;
	} else if (direction == 2) {
		idxTest = idxTestDown;
		sideTest = sideTestDown;
		_newLineIdx = lineIdxTestDown;
		_newLineDataIdx = dataIdxTestDown;
	} else if (direction == 3) {
		idxTest = idxTestLeft;
		sideTest = sideTestLeft;
		_newLineIdx = lineIdxTestLeft;
		_newLineDataIdx = dataIdxTestLeft;
	} else if (direction == 4) {
		idxTest = idxTestRight;
		sideTest = sideTestRight;
		_newLineIdx = lineIdxTestRight;
		_newLineDataIdx = dataIdxTestRight;
	}

	int routeIdx = superRouteIdx;
	if (sideTest == 1) {
		routeIdx = _lineItem[idxTest].appendToRouteInc(0, -1, _bestRoute, routeIdx);
	} else if (sideTest == 2) {
		routeIdx = _lineItem[idxTest].appendToRouteDec(-1, -1, _bestRoute, routeIdx);
	}
	_newRouteIdx = routeIdx;
	return true;
}

// Test line
int LinesManager::testLine(int paramX, int paramY, int *testValue, int *foundLineIdx, int *foundDataIdx) {
	debugC(5, kDebugPath, "testLine(%d, %d, testValue, foundLineIdx, foundDataIdx)", paramX, paramY);
	int16 *lineData;
	int collLineIdx;
	int collDataIdx;

	for (int idx = _lastLine + 1; idx < _linesNumb + 1; idx++) {
		lineData = _lineItem[idx]._lineData;
		int lineDataEndIdx = _lineItem[idx]._lineDataEndIdx;
		if (!lineData)
			continue;

		if (lineData[0] == paramX && lineData[1] == paramY) {
			*testValue = 1;
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

		if (lineDataEndIdx > 0) {
			if (lineData[2 * (lineDataEndIdx - 1)] == paramX && lineData[2 * (lineDataEndIdx - 1) + 1] == paramY) {
				*testValue = 2;
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
	}
	return -1;
}

int LinesManager::computeYSteps(int idx) {
	debugC(5, kDebugPath, "computeYSteps(%d)", idx);
	int zoomPct = _vm->_globals->_spriteSize[idx];
	if (_vm->_globals->_characterType == CHARACTER_HOPKINS_CLONE) {
		if (zoomPct < 0)
			zoomPct = -zoomPct;
		zoomPct = 20 * (5 * zoomPct - 100) / -80;
	} else if (_vm->_globals->_characterType == CHARACTER_SAMANTHA) {
		if (zoomPct < 0)
			zoomPct = -zoomPct;
		zoomPct = 20 * (5 * zoomPct - 165) / -67;
	}

	int retVal = 25;
	if (zoomPct < 0)
		retVal = _vm->_graphicsMan->zoomOut(25, -zoomPct);
	else if (zoomPct > 0)
		retVal = _vm->_graphicsMan->zoomIn(25, zoomPct);

	return retVal;
}

void LinesManager::optimizeRoute(RouteItem *route) {
	debugC(5, kDebugPath, "optimizeRoute(route)");
	if (route[0]._x == -1 && route[0]._y == -1)
		return;

	int routeIdx = 0;
	Directions oldDir = DIR_NONE;
	int route0Y = route[0]._y;
	Directions curDir = route[0]._dir;

	for (;;) {
		if (oldDir != DIR_NONE && curDir != oldDir) {
			int oldRouteIdx = routeIdx;
			int routeCount = 0;
			int yStep = computeYSteps(route0Y);
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
			if (routeCount < yStep) {
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
	debugC(9, kDebugPath, "getMouseZone()");
	int result;

	int xp = _vm->_events->_mousePos.x + _vm->_events->_mouseOffset.x;
	int yp = _vm->_events->_mousePos.y + _vm->_events->_mouseOffset.y;
	if ((_vm->_events->_mousePos.y + _vm->_events->_mouseOffset.y) > 19) {
		for (int bobZoneId = 0; bobZoneId <= 48; bobZoneId++) {
			int bobId = _bobZone[bobZoneId];
			if (bobId && _bobZoneFl[bobZoneId] && _vm->_objectsMan->_bob[bobId]._bobMode && _vm->_objectsMan->_bob[bobId]._frameIndex != 250 &&
				!_vm->_objectsMan->_bob[bobId]._disabledAnimationFl && xp > _vm->_objectsMan->_bob[bobId]._oldX &&
				xp < _vm->_objectsMan->_bob[bobId]._oldWidth + _vm->_objectsMan->_bob[bobId]._oldX && yp > _vm->_objectsMan->_bob[bobId]._oldY) {
					if (yp < _vm->_objectsMan->_bob[bobId]._oldHeight + _vm->_objectsMan->_bob[bobId]._oldY) {
						if (_zone[bobZoneId]._spriteIndex == -1) {
							_zone[bobZoneId]._destX = 0;
							_zone[bobZoneId]._destY = 0;
						}
						if (!_zone[bobZoneId]._destX && !_zone[bobZoneId]._destY) {
							_zone[bobZoneId]._destX = _vm->_objectsMan->_bob[bobId]._oldWidth + _vm->_objectsMan->_bob[bobId]._oldX;
							_zone[bobZoneId]._destY = _vm->_objectsMan->_bob[bobId]._oldHeight + _vm->_objectsMan->_bob[bobId]._oldY + 6;
							_zone[bobZoneId]._spriteIndex = -1;
						}

						// WORKAROUND: Avoid allowing hotspots that should remain non-interactive
						if (bobZoneId == 24 && _vm->_globals->_curRoomNum == 14)
							continue;

						return bobZoneId;
					}
			}
		}
		_currentSegmentId = 0;
		for (int squareZoneId = 0; squareZoneId <= 99; squareZoneId++) {
			if (_zone[squareZoneId]._enabledFl && _squareZone[squareZoneId]._enabledFl
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
			if (colRes1 != -1 && _zone[colRes1]._enabledFl)
				break;
		}

		if (colRes1 == -1)
			return -1;

		int colRes2 = 0;
		for (int j = yp; j < _vm->_graphicsMan->_maxY; ++j) {
			colRes2 = checkCollision(xp, j);
			if (colRes2 != -1 && _zone[colRes1]._enabledFl)
				break;
		}

		if (colRes2 == -1)
			return -1;

		int colRes3 = 0;
		for (int k = xp; k >= 0; --k) {
			colRes3 = checkCollision(k, yp);
			if (colRes3 != -1 && _zone[colRes1]._enabledFl)
				break;
		}
		if (colRes3 == -1)
			return -1;

		int colRes4 = 0;
		for (int xCurrent = xp; _vm->_graphicsMan->_maxX > xCurrent; ++xCurrent) {
			colRes4 = checkCollision(xCurrent, yp);
			if (colRes4 != -1 && _zone[colRes1]._enabledFl)
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
	debugC(7, kDebugPath, "checkCollision(%d, %d)", xp, yp);
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
			LigneZoneItem *curZoneLine = &_zoneLine[curZoneLineIdx];
			int16 *dataP = curZoneLine->_zoneData;
			if (dataP) {
				int count = curZoneLine->_count;
				int startX = dataP[0];
				int startY = dataP[1];
				int destX = dataP[count * 2 - 2];
				int destY = dataP[count * 2 - 1];

				bool flag = true;
				if ((startX < destX && (xMax < startX || xMin > destX))  ||
				    (startX >= destX && (xMin > startX || xMax < destX)) ||
				    (startY < destY && (yMax < startY || yMin > destY))  ||
				    (startY >= destY && (yMin > startY || yMax < destY)))
					flag = false;

				if (flag && curZoneLine->_count > 0) {
					for (int i = 0; i < count; ++i) {
						int xCheck = *dataP++;
						int yCheck = *dataP++;

						if ((xp == xCheck || (xp + 1) == xCheck) && (yp == yCheck))
							return curZoneLine->_bobZoneIdx;
					}
				}
			}
		} while (++curZoneLineIdx <= _segment[idx]._maxZoneLineIdx);
	}

	return -1;
}

// Square Zone
void LinesManager::initSquareZones() {
	debugC(5, kDebugPath, "initSquareZones()");
	for (int idx = 0; idx < 100; ++idx) {
		SquareZoneItem *curZone = &_squareZone[idx];
		curZone->_enabledFl = false;
		curZone->_squareZoneFl = false;
		curZone->_left = 1280;
		curZone->_right = 0;
		curZone->_top = 460;
		curZone->_bottom = 0;
		curZone->_minZoneLineIdx = 401;
		curZone->_maxZoneLineIdx = 0;
	}

	for (int idx = 0; idx < MAX_LINES + 1; ++idx) {
		int16 *dataP = _zoneLine[idx]._zoneData;
		if (dataP == NULL)
			continue;

		SquareZoneItem *curZone = &_squareZone[_zoneLine[idx]._bobZoneIdx];
		curZone->_enabledFl = true;
		curZone->_maxZoneLineIdx = MAX(curZone->_maxZoneLineIdx, idx);
		curZone->_minZoneLineIdx = MIN(curZone->_minZoneLineIdx, idx);

		for (int i = 0; i < _zoneLine[idx]._count; i++) {
			int zoneX = *dataP++;
			int zoneY = *dataP++;

			curZone->_left = MIN(curZone->_left, zoneX);
			curZone->_right = MAX(curZone->_right, zoneX);
			curZone->_top = MIN(curZone->_top, zoneY);
			curZone->_bottom = MAX(curZone->_bottom, zoneY);
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
	debugC(5, kDebugPath, "clearAll()");
	for (int idx = 0; idx < 105; ++idx) {
		_zone[idx]._destX = 0;
		_zone[idx]._destY = 0;
		_zone[idx]._spriteIndex = 0;
	}

	_testRoute0 = NULL;
	_testRoute1 = NULL;
	_testRoute2 = NULL;
	_lineBuf = NULL;
	_route = NULL;

	for (int idx = 0; idx < MAX_LINES; ++idx) {
		_lineItem[idx]._lineDataEndIdx = 0;
		_lineItem[idx]._direction = DIR_NONE;
		_lineItem[idx]._directionRouteInc = DIR_NONE;
		_lineItem[idx]._directionRouteDec = DIR_NONE;
		_lineItem[idx]._lineData = NULL;

		_zoneLine[idx]._count = 0;
		_zoneLine[idx]._bobZoneIdx = 0;
		_zoneLine[idx]._zoneData = NULL;
	}

	for (int idx = 0; idx < 100; ++idx)
		_squareZone[idx]._enabledFl = false;

	_testRoute0 = new RouteItem[8334];
	_testRoute1 = new RouteItem[8334];
	_testRoute2 = new RouteItem[8334];
	if (!_testRoute0)
		_testRoute0 = NULL;
	if (!_testRoute1)
		_testRoute1 = NULL;
	if (!_testRoute2)
		_testRoute2 = NULL;

	_largeBuf = _vm->_globals->allocMemory(10000);
	_lineBuf = (int16 *)(_largeBuf);
}

/**
 * Clear all zones and reset nextLine
 */
void LinesManager::clearAllZones() {
	debugC(5, kDebugPath, "clearAllZones()");
	for (int idx = 0; idx < MAX_LINES; ++idx)
		removeZoneLine(idx);
}

/**
 * Remove Zone Line
 */
void LinesManager::removeZoneLine(int idx) {
	debugC(5, kDebugPath, "removeZoneLine(%d)", idx);
	assert(idx < MAX_LINES + 1);
	_zoneLine[idx]._zoneData = (int16 *)_vm->_globals->freeMemory((byte *)_zoneLine[idx]._zoneData);
}

void LinesManager::resetLines() {
	debugC(5, kDebugPath, "resetLines()");
	for (int idx = 0; idx < MAX_LINES; ++idx) {
		_lineItem[idx]._lineData = (int16 *)_vm->_globals->freeMemory((byte *)_lineItem[idx]._lineData);
		_lineItem[idx]._lineDataEndIdx = 0;
		_lineItem[idx]._lineData = NULL;
	}
}

void LinesManager::setMaxLineIdx(int idx) {
	debugC(5, kDebugPath, "setMaxLineIdx(%d)", idx);
	_maxLineIdx = idx;
}

void LinesManager::resetLastLine() {
	debugC(5, kDebugPath, "resetLastLine()");
	_lastLine = 0;
}

void LinesManager::resetLinesNumb() {
	debugC(5, kDebugPath, "resetLinesNumb()");
	_linesNumb = 0;
}

void LinesManager::enableZone(int idx) {
	debugC(5, kDebugPath, "enableZone(%d)", idx);
	if (_bobZone[idx]) {
		_bobZoneFl[idx] = true;
	} else {
		_zone[idx]._enabledFl = true;
	}
}

void LinesManager::disableZone(int idx) {
	debugC(5, kDebugPath, "disableZone(%d)", idx);
	if (_bobZone[idx]) {
		_bobZoneFl[idx] = false;
	} else {
		_zone[idx]._enabledFl = false;
	}
}

void LinesManager::checkZone() {
	debugC(9, kDebugPath, "checkZone()");
	int mouseX = _vm->_events->getMouseX();
	int mouseY = _vm->_events->getMouseY();
	int oldMouseY = mouseY;
	if (_vm->_globals->_cityMapEnabledFl
		|| _vm->_events->_startPos.x >= mouseX
		|| (mouseY = _vm->_graphicsMan->_scrollOffset + 54, mouseX >= mouseY)
		|| (mouseY = oldMouseY - 1, mouseY < 0 || mouseY > 59)) {
			if (_vm->_objectsMan->_visibleFl)
				_vm->_objectsMan->_eraseVisibleCounter = 4;
			_vm->_objectsMan->_visibleFl = false;
	} else {
		_vm->_objectsMan->_visibleFl = true;
	}
	if (_vm->_objectsMan->_forceZoneFl) {
		_zoneSkipCount = 100;
		_oldMouseZoneId = -1;
		_oldMouseX = -200;
		_oldMouseY = -220;
		_vm->_objectsMan->_forceZoneFl = false;
	}

	_zoneSkipCount++;
	if (_zoneSkipCount <= 1)
		return;

	if (_vm->_globals->_freezeCharacterFl || (_route == NULL) || _zoneSkipCount > 4) {
		_zoneSkipCount = 0;
		int zoneId;
		if (_oldMouseX != mouseX || _oldMouseY != oldMouseY) {
			zoneId = getMouseZone();

			// WORKAROUND: Incorrect hotspot zones in the guard's control room
			if (_vm->_globals->_curRoomNum == 71 && (zoneId == 14 || zoneId == 12 || zoneId == 17))
				zoneId = _oldMouseZoneId;
		} else {
			zoneId = _oldMouseZoneId;
		}
		if (_oldMouseZoneId != zoneId) {
			_vm->_graphicsMan->setColorPercentage2(251, 100, 100, 100);
			_vm->_events->_mouseCursorId = 4;
			_vm->_events->changeMouseCursor(4);
			if (_forceHideText) {
				_vm->_fontMan->hideText(5);
				_forceHideText = false;
				return;
			}
		}
		if (zoneId != -1) {
			if (_zone[zoneId]._verbFl1 || _zone[zoneId]._verbFl2 ||
				_zone[zoneId]._verbFl3 || _zone[zoneId]._verbFl4 ||
				_zone[zoneId]._verbFl5 || _zone[zoneId]._verbFl6 ||
				_zone[zoneId]._verbFl7 || _zone[zoneId]._verbFl8 ||
				_zone[zoneId]._verbFl9 || _zone[zoneId]._verbFl10) {
					if (_oldMouseZoneId != zoneId) {
						_vm->_fontMan->initTextBuffers(5, _zone[zoneId]._messageId, _vm->_globals->_zoneFilename, 0, 430, 0, 0, 252);
						_vm->_fontMan->showText(5);
						_forceHideText = true;
					}
					_hotspotTextColor += 25;
					if (_hotspotTextColor > 100)
						_hotspotTextColor = 0;
					_vm->_graphicsMan->setColorPercentage2(251, _hotspotTextColor, _hotspotTextColor, _hotspotTextColor);
					if (_vm->_events->_mouseCursorId == 4) {
						if (_zone[zoneId]._verbFl1 == 2) {
							_vm->_events->changeMouseCursor(16);
							_vm->_events->_mouseCursorId = 16;
							_vm->_objectsMan->setVerb(16);
						}
					}
			} else {
				_vm->_graphicsMan->setColorPercentage2(251, 100, 100, 100);
				_vm->_events->_mouseCursorId = 4;
				_vm->_events->changeMouseCursor(4);
			}
		}
		_vm->_objectsMan->_zoneNum = zoneId;
		_oldMouseX = mouseX;
		_oldMouseY = oldMouseY;
		_oldMouseZoneId = zoneId;
		if (_vm->_globals->_freezeCharacterFl && (_vm->_events->_mouseCursorId == 4)) {
			if (zoneId != -1 && zoneId != 0)
				_vm->_objectsMan->handleRightButton();
		}
		if ((_vm->_globals->_cityMapEnabledFl && zoneId == -1) || !zoneId) {
			_vm->_objectsMan->setVerb(0);
			_vm->_events->_mouseCursorId = 0;
			_vm->_events->changeMouseCursor(0);
		}
	}
}

} // End of namespace Hopkins
