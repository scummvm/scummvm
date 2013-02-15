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

LinesManager::LinesManager() {
	for (int i = 0; i < MAX_LINES; ++i) {
		Common::fill((byte *)&_zoneLine[i], (byte *)&_zoneLine[i] + sizeof(LigneZoneItem), 0);
		Common::fill((byte *)&Ligne[i], (byte *)&Ligne[i] + sizeof(LigneItem), 0);
	}

	for (int i = 0; i < 4000; ++i)
		Common::fill((byte *)&_smoothRoute[i], (byte *)&_smoothRoute[i] + sizeof(SmoothItem), 0);

	for (int i = 0; i < 32002; ++i)
		super_parcours[i] = 0;

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
	NV_LIGNEDEP = 0;
	NV_LIGNEOFS = 0;
	NV_POSI = 0;
	NVPX = 0;
	NVPY = 0;
	_smoothMoveDirection = 0;
	_lastLine = 0;
	_maxLineIdx = 0;
	_pathFindingMaxDepth = 0;
	essai0 = NULL;
	essai1 = NULL;
	essai2 = NULL;
	BufLig = (int16 *)g_PTRNUL;
	_route = (int16 *)g_PTRNUL;
	_currentSegmentId = 0;
	BUFFERTAPE = NULL;
}

LinesManager::~LinesManager() {
	_vm->_globals.freeMemory(BUFFERTAPE);
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
	for (int idx = 0; (int16)READ_LE_UINT16((uint16 *)ptr + (idx * 5)) != -1; idx++) {
		addLine(idx,
		    (int16)READ_LE_UINT16((uint16 *)ptr + (idx * 5)),
		    (int16)READ_LE_UINT16((uint16 *)ptr + (idx * 5) + 1),
		    (int16)READ_LE_UINT16((uint16 *)ptr + (idx * 5) + 2),
		    (int16)READ_LE_UINT16((uint16 *)ptr + (idx * 5) + 3),
		    (int16)READ_LE_UINT16((uint16 *)ptr + (idx * 5) + 4));
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
void LinesManager::addLine(int idx, int direction, int a3, int a4, int a5, int a6) {
	assert (idx <= MAX_LINES);

	if (_linesNumb < idx)
		_linesNumb = idx;

	Ligne[idx]._lineData = (int16 *)_vm->_globals.freeMemory((byte *)Ligne[idx]._lineData);
	int v8 = abs(a3 - a5) + 1;
	int v34 = abs(a4 - a6) + 1;
	int v33 = v34;
	if (v8 > v34)
		v34 = v8;

	byte *v10 = _vm->_globals.allocMemory(4 * v34 + 8);
	assert (v10 != g_PTRNUL);

	Common::fill(v10, v10 + 4 * v34 + 8, 0);
	Ligne[idx]._lineData = (int16 *)v10;

	int16 *v32 = Ligne[idx]._lineData;
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
			Ligne[idx].field6 = 1;
			Ligne[idx].field8 = 5;
		}
		if (v12 == 1) {
			Ligne[idx].field6 = 5;
			Ligne[idx].field8 = 1;
		}
	}
	if (v11 == 1) {
		if (v12 == -1) {
			Ligne[idx].field6 = 2;
			Ligne[idx].field8 = 6;
		}
		if (!v12) {
			Ligne[idx].field6 = 3;
			Ligne[idx].field8 = 7;
		}
		if (v12 == 1) {
			Ligne[idx].field6 = 4;
			Ligne[idx].field8 = 8;
		}
	}
	if (v11 == -1) {
		if (v12 == 1) {
			Ligne[idx].field6 = 6;
			Ligne[idx].field8 = 2;
		}
		if (!v12) {
			Ligne[idx].field6 = 7;
			Ligne[idx].field8 = 3;
		}
		if (v12 == -1) {
			Ligne[idx].field6 = 8;
			Ligne[idx].field8 = 4;
		}
	}
	if (v11 == 1 && v37 > 250 && v37 <= 999) {
		Ligne[idx].field6 = 4;
		Ligne[idx].field8 = 8;
	}
	if (v11 == -1 && v37 > 250 && v37 <= 999) {
		Ligne[idx].field6 = 6;
		Ligne[idx].field8 = 2;
	}
	if (v11 == 1 && v37 < -250 && v37 > -1000) {
		Ligne[idx].field6 = 2;
		Ligne[idx].field8 = 6;
	}
	// This condition is impossible to meet!
	// Code present in the Linux and BeOS executables
	// CHECKME: maybe it should be checking negative values?
	if (v11 == -1 && v37 <= 249 && v37 > 1000) {
		Ligne[idx].field6 = 8;
		Ligne[idx].field8 = 4;
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

	Ligne[idx]._lineDataEndIdx = v35 + 1;
	Ligne[idx]._direction = direction;

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
		lineData = Ligne[curLineIdx]._lineData;

		if (lineData == (int16 *)g_PTRNUL)
			continue;

		bool collisionFl = true;
		int lineStartX = lineData[0];
		int lineStartY = lineData[1];
		int lineDataIdx = 2 * Ligne[curLineIdx]._lineDataEndIdx;
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

		for (int idx = 0; idx < Ligne[curLineIdx]._lineDataEndIdx; idx++) {
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
	int lineX = Ligne[0]._lineData[0];
	int lineY = Ligne[0]._lineData[1];

	int lineIdx = 1;
	for (;;) {
		int curDataIdx = Ligne[lineIdx]._lineDataEndIdx;
		int16 *curLineData = Ligne[lineIdx]._lineData;

		int curLineX = curLineData[2 * curDataIdx - 2];
		int curLineY = curLineData[2 * curDataIdx - 1];
		if (_vm->_graphicsManager._maxX == curLineX || _vm->_graphicsManager._maxY == curLineY || 
			_vm->_graphicsManager._minX == curLineX || _vm->_graphicsManager._minY == curLineY ||
			(lineX == curLineX && lineY == curLineY))
			break;
		if (lineIdx == MAX_LINES)
			error("ERROR - LAST LINE NOT FOUND");

		int16 *nextLineData = Ligne[lineIdx + 1]._lineData;
		if (nextLineData[0] != curLineX && nextLineData[1] != curLineY)
			break;
		++lineIdx;
	}

	_lastLine = lineIdx;
	for (int idx = 1; idx < MAX_LINES; idx++) {
		if ((Ligne[idx]._lineDataEndIdx < _maxLineIdx) && (idx != _lastLine + 1)) {
			Ligne[idx].field6 = Ligne[idx - 1].field6;
			Ligne[idx].field8 = Ligne[idx - 1].field8;
		}
	}
}

// Avoid
int LinesManager::CONTOURNE(int a1, int a2, int a3, int a4, int a5, int16 *route, int a7) {
	int v36 = a1;
	int v7 = a2;
	int v8 = a3;
	if (a1 < a4) {
		for (int i = a2; Ligne[a1]._lineDataEndIdx > i; ++i) {
			route[v8] = Ligne[a1]._lineData[2 * i];
			route[v8 + 1] = Ligne[a1]._lineData[2 * i + 1];
			route[v8 + 2] = Ligne[a1].field6;
			v8 += a7;
		}

		for (int i = a1 + 1; i < a4; i++) {
			for (int j = 0; j < Ligne[i]._lineDataEndIdx; j++) {
				route[v8] = Ligne[i]._lineData[2 * j];
				route[v8 + 1] = Ligne[i]._lineData[2 * j + 1];
				route[v8 + 2] = Ligne[i].field6;
				v8 += a7;
			}
		}

		v7 = 0;
		v36 = a4;
	}
	if (v36 > a4) {
		for (int i = v7; i > 0; --i) {
			route[v8] = Ligne[v36]._lineData[2 * i];
			route[v8 + 1] = Ligne[v36]._lineData[2 * i + 1];
			route[v8 + 2] = Ligne[v36].field8;
			v8 += a7;
		}
		for (int i = v36 - 1; i > a4; i--) {
			for (int j = Ligne[i]._lineDataEndIdx - 1; j > 0; --j) {
				route[v8] = Ligne[i]._lineData[2 * j];
				route[v8 + 1] = Ligne[i]._lineData[2 * j + 1];
				route[v8 + 2] = Ligne[i].field8;
				v8 += a7;
			}
		}
		v7 = Ligne[a4]._lineDataEndIdx - 1;
		v36 = a4;
	}
	if (v36 == a4) {
		if (a5 >= v7) {
			for (int i = v7; i < a5; i++) {
				route[v8] = Ligne[a4]._lineData[2 * i];
				route[v8 + 1] = Ligne[a4]._lineData[2 * i + 1];
				route[v8 + 2] = Ligne[a4].field6;
				v8 += a7;
			}
		} else {
			for (int i = v7; i > a5; --i) {
				route[v8] = Ligne[a4]._lineData[2 * i];
				route[v8 + 1] = Ligne[a4]._lineData[2 * i+ 1];
				route[v8 + 2] = Ligne[a4].field8;
				v8 += a7;
			}
		}
	}
	return v8;
}

// Avoid 1
int LinesManager::CONTOURNE1(int a1, int a2, int a3, int a4, int a5, int16 *route, int a7, int a8, int a9) {
	int v9 = a1;
	int v10 = a2;
	int v40 = a3;
	if (a4 < a1) {
		for (int i = a2; i < Ligne[a1]._lineDataEndIdx; ++i) {
			route[v40] = Ligne[a1]._lineData[2 * i];
			route[v40 + 1] = Ligne[a1]._lineData[2 * i + 1];
			route[v40 + 2] = Ligne[a1].field6;
			v40 += a7;
		}
		int v15 = a1 + 1;
		if (v15 == a9 + 1)
			v15 = a8;
		while (a4 != v15) {
			for (int i = 0; i < Ligne[v15]._lineDataEndIdx; i++) {
				route[v40] = Ligne[v15]._lineData[2 * i];
				route[v40 + 1] = Ligne[v15]._lineData[2 * i + 1];
				route[v40 + 2] = Ligne[v15].field6;
				v40 += a7;

			}
			++v15;
			if (a9 + 1 == v15)
				v15 = a8;
		}
		v10 = 0;
		v9 = a4;
	}
	if (a4 > v9) {
		for (int i = v10; i > 0; --i) {
			route[v40] = Ligne[v9]._lineData[2 * i];
			route[v40 + 1] = Ligne[v9]._lineData[2 * i + 1];
			route[v40 + 2] = Ligne[v9].field8;
			v40 += a7;
		}
		int v24 = v9 - 1;
		if (v24 == a8 - 1)
			v24 = a9;
		while (a4 != v24) {
			for (int i = Ligne[v24]._lineDataEndIdx - 1; i > 0; --i) {
				route[v40] = Ligne[v24]._lineData[2 * i];
				route[v40 + 1] = Ligne[v24]._lineData[2 * i + 1];
				route[v40 + 2] = Ligne[v24].field8;
				v40 += a7;
			}
			--v24;
			if (a8 - 1 == v24)
				v24 = a9;
		}
		v10 = Ligne[a4]._lineDataEndIdx - 1;
		v9 = a4;
	}
	if (a4 == v9) {
		if (a5 >= v10) {
			for (int i = v10; i < a5; i++) {
				route[v40] = Ligne[a4]._lineData[2 * i];
				route[v40 + 1] = Ligne[a4]._lineData[2 * i + 1];
				route[v40 + 2] = Ligne[a4].field6;
				v40 += a7;
			}
		} else {
			for (int i = v10; i > a5; i--) {
				route[v40] = Ligne[a4]._lineData[2 * i];
				route[v40 + 1] = Ligne[a4]._lineData[2 * i + 1];
				route[v40 + 2] = Ligne[a4].field8;
				v40 += a7;
			}
		}
	}
	return v40;
}

bool LinesManager::MIRACLE(int a1, int a2, int a3, int a4, int a5) {
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

	int v5 = a1;
	int v6 = a2;
	int v50 = a3;
	int v7 = a5;
	int v51;
	if (checkCollisionLine(a1, a2, &v51, &v50, 0, _linesNumb)) {
		switch (Ligne[v50]._direction) {
		case 1:
			v6 = a2 - 2;
			break;
		case 2:
			v6 -= 2;
			v5 += 2;
			break;
		case 3:
			v5 += 2;
			break;
		case 4:
			v6 += 2;
			v5 += 2;
			break;
		case 5:
			v6 += 2;
			break;
		case 6:
			v6 += 2;
			v5 -= 2;
			break;
		case 7:
			v5 -= 2;
			break;
		case 8:
			v6 -= 2;
			v5 -= 2;
			break;
		}
	}
	int v41 = v5;
	int v40 = v6;
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
		int v21 = 0;
		if (a4 > v50) {
			if (v48 <= v46 && v44 <= v46 && v42 <= v46 && v46 > v50)
				v21 = 1;
			if (v48 <= v44 && v46 <= v44 && v42 <= v44 && v50 < v44)
				v21 = 3;
			if (v46 <= v48 && v44 <= v48 && v42 <= v48 && v50 < v48)
				v21 = 5;
			if (v48 <= v42 && v44 <= v42 && v46 <= v42 && v50 < v42)
				v21 = 7;
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
				v21 = 1;
			if (v44 != 1300 && v48 >= v44 && v46 >= v44 && v42 >= v44 && v50 > v44)
				v21 = 3;
			if (v48 != 1300 && v46 >= v48 && v44 >= v48 && v42 >= v48 && v50 > v48)
				v21 = 5;
			if (v42 != 1300 && v48 >= v42 && v44 >= v42 && v46 >= v42 && v50 > v42)
				v21 = 7;
		}

		switch(v21) {
		case 1:
			v36 = v46;
			v35 = v47;
			for (int v22 = 0; v22 < v39; v22++) {
				if (checkCollisionLine(v41, v40 - v22, &v47, &v46, _lastLine + 1, _linesNumb) && _lastLine < v46) {
					int v23 = GENIAL(v46, v47, v41, v40 - v22, v41, v40 - v39, v7, &super_parcours[0], 4);
					if (v23 == -1)
						return false;
					v7 = v23;
					if (NVPY != -1)
						v22 = NVPY - v40;
				}
				super_parcours[v7] = v41;
				super_parcours[v7 + 1] = v40 - v22;
				super_parcours[v7 + 2] = 1;
				super_parcours[v7 + 3] = 0;
				v7 += 4;
			}
			NV_LIGNEDEP = v36;
			NV_LIGNEOFS = v35;
			NV_POSI = v7;
			return true;
			break;
		case 3:
			v36 = v44;
			v35 = v45;
			for (int v31 = 0; v31 < v38; v31++) {
				if (checkCollisionLine(v31 + v41, v40, &v47, &v46, _lastLine + 1, _linesNumb) && _lastLine < v46) {
					int v32 = GENIAL(v46, v47, v31 + v41, v40, v38 + v41, v40, v7, &super_parcours[0], 4);
					if (v32 == -1)
						return false;
					v7 = v32;
					if (NVPX != -1)
						v31 = NVPX - v41;
				}
				super_parcours[v7] = v31 + v41;
				super_parcours[v7 + 1] = v40;
				super_parcours[v7 + 2] = 3;
				super_parcours[v7 + 3] = 0;
				v7 += 4;
			}
			NV_LIGNEDEP = v36;
			NV_LIGNEOFS = v35;
			NV_POSI = v7;
			return true;
			break;
		case 5:
			v36 = v48;
			v35 = v49;
			for (int v25 = 0; v25 < v37; v25++) {
				if (checkCollisionLine(v41, v25 + v40, &v47, &v46, _lastLine + 1, _linesNumb) && _lastLine < v46) {
					int v26 = GENIAL(v46, v47, v41, v25 + v40, v41, v37 + v40, v7, &super_parcours[0], 4);
					if (v26 == -1)
						return false;
					v7 = v26;
					if (NVPY != -1)
						v25 = v40 - NVPY;
				}
				super_parcours[v7] = v41;
				super_parcours[v7 + 1] = v25 + v40;
				super_parcours[v7 + 2] = 5;
				super_parcours[v7 + 3] = 0;
				v7 += 4;
			}
			NV_LIGNEDEP = v36;
			NV_LIGNEOFS = v35;
			NV_POSI = v7;
			return true;
			break;
		case 7:
			v36 = v42;
			v35 = v43;
			for (int v28 = 0; v28 < v18; v28++) {
				if (checkCollisionLine(v41 - v28, v40, &v47, &v46, _lastLine + 1, _linesNumb) && _lastLine < v46) {
					int v29 = GENIAL(v46, v47, v41 - v28, v40, v41 - v18, v40, v7, &super_parcours[0], 4);
					if (v29 == -1)
						return false;
					v7 = v29;
					if (NVPX != -1)
						v28 = v41 - NVPX;
				}
				super_parcours[v7] = v41 - v28;
				super_parcours[v7 + 1] = v40;
				super_parcours[v7 + 2] = 7;
				super_parcours[v7 + 3] = 0;
				v7 += 4;
			}
			NV_LIGNEDEP = v36;
			NV_LIGNEOFS = v35;
			NV_POSI = v7;
			return true;
			break;
		}
	}
	return false;
}

int LinesManager::GENIAL(int lineIdx, int dataIdx, int a3, int a4, int a5, int a6, int a7, int16 *route, int a9) {
	int result = a7;
	int v80 = -1;
	++_pathFindingMaxDepth;
	if (_pathFindingMaxDepth > 10) {
		warning("PathFinding - Max depth reached");
		route[a7] = -1;
		route[a7 + 1] = -1;
		route[a7 + 2] = -1;
		return -1;
	}
	int16 *v10 = Ligne[lineIdx]._lineData;
	int v98 = v10[0];
	int v97 = v10[1];
	int v92 = lineIdx;

	int v65;
	bool loopCond = false;
	for (;;) {
		int v86 = v92 - 1;
		int v11 = 2 * Ligne[v92 - 1]._lineDataEndIdx;

		int16 *v12 = Ligne[v92 - 1]._lineData;
		if (v12 == (int16 *)g_PTRNUL)
			break;
		while (v12[v11 - 2] != v98 || v97 != v12[v11 - 1]) {
			--v86;
			if (_lastLine - 1 != v86) {
				v11 = 2 * Ligne[v86]._lineDataEndIdx;
				v12 = Ligne[v86]._lineData;
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

	int16 *v13 = Ligne[lineIdx]._lineData;
	int v95 = v13[2 * Ligne[lineIdx]._lineDataEndIdx - 2];
	int v93 = v13[2 * Ligne[lineIdx]._lineDataEndIdx - 1];
	int v91 = lineIdx;
	int foundLineIdx, foundDataIdx;
	loopCond = false;
	for (;;) {
		int v87 = v91 + 1;
		int v15 = 2 * Ligne[v91 + 1]._lineDataEndIdx;
		int16 *v16 = Ligne[v91 + 1]._lineData;
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
				v15 = 2 * Ligne[v87]._lineDataEndIdx;
				v16 = Ligne[v87]._lineData;
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

	Common::fill(&BufLig[0], &BufLig[1000], 0);
	int bugLigIdx = 0;
	for (int v88 = 0; v88 < v85 + 1; v88++) {
		BufLig[bugLigIdx] = v82;
		BufLig[bugLigIdx + 1] = v81;
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
		if (checkCollisionLine(BufLig[bugLigIdx], BufLig[bugLigIdx + 1], &foundDataIdx, &foundLineIdx, v92, v91) && _lastLine < foundLineIdx) {
			v80 = foundLineIdx;
			v77 = foundDataIdx;
			v78 = BufLig[bugLigIdx];
			v79 = BufLig[bugLigIdx + 1];
			break;
		}
		bugLigIdx -= 2;
	}
	int v66 = 0;
	int v68 = 0;
	int v70 = 0;
	int v72 = 0;
	for (int i = v92; i <= v91; ++i) {
		int16 *lineData = Ligne[i]._lineData;
		if (lineData == (int16 *)g_PTRNUL)
			error("error in genial routine");
		if (i == v92) {
			v72 = lineData[2 * Ligne[i]._lineDataEndIdx - 1];
			if (lineData[1] <= lineData[2 * Ligne[i]._lineDataEndIdx - 1])
				v72 = lineData[1];
			v70 = lineData[2 * Ligne[i]._lineDataEndIdx - 1];
			if (lineData[1] >= lineData[2 * Ligne[i]._lineDataEndIdx - 1])
				v70 = lineData[1];
			v68 = lineData[2 * Ligne[i]._lineDataEndIdx - 2];
			if (lineData[0] <= lineData[2 * Ligne[i]._lineDataEndIdx - 2])
				v68 = lineData[0];
			v66 = lineData[2 * Ligne[i]._lineDataEndIdx - 2];
			if (lineData[0] >= lineData[2 * Ligne[i]._lineDataEndIdx - 2])
				v66 = lineData[0];
		} else {
			if (lineData[1] < lineData[2 * Ligne[i]._lineDataEndIdx - 1] && lineData[1] < v72)
				v72 = lineData[1];
			if (lineData[2 * Ligne[i]._lineDataEndIdx - 1] < lineData[1] && lineData[2 * Ligne[i]._lineDataEndIdx - 1] < v72)
				v72 = lineData[2 * Ligne[i]._lineDataEndIdx - 1];
			if (lineData[1] > lineData[2 * Ligne[i]._lineDataEndIdx - 1] && lineData[1] > v70)
				v70 = lineData[1];
			if (lineData[2 * Ligne[i]._lineDataEndIdx - 1] > lineData[1] && lineData[2 * Ligne[i]._lineDataEndIdx - 1] > v70)
				v70 = lineData[2 * Ligne[i]._lineDataEndIdx - 1];
			if (lineData[0] < lineData[2 * Ligne[i]._lineDataEndIdx - 2] && v68 > lineData[0])
				v68 = lineData[0];
			if (lineData[2 * Ligne[i]._lineDataEndIdx - 2] < lineData[0] && v68 > lineData[2 * Ligne[i]._lineDataEndIdx - 2])
				v68 = lineData[2 * Ligne[i]._lineDataEndIdx - 2];
			if (lineData[0] > lineData[2 * Ligne[i]._lineDataEndIdx - 2] && v66 < lineData[0])
				v66 = lineData[0];
			if (lineData[2 * Ligne[i]._lineDataEndIdx - 2] > lineData[0] && v66 < lineData[2 * Ligne[i]._lineDataEndIdx - 2])
				v66 = lineData[2 * Ligne[i]._lineDataEndIdx - 2];
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
			route[a7] = -1;
			route[a7 + 1] = -1;
			route[a7 + 2] = -1;
			return -1;
		}
	}
	if (v78 < a3 - 1 || v78 > a3 + 1 || v79 < a4 - 1 || v79 > a4 + 1) {
		NVPX = v78;
		NVPY = v79;
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
				if (dataIdx >  abs(Ligne[lineIdx]._lineDataEndIdx / 2)) {
					result = CONTOURNE(lineIdx, dataIdx, a7, v80, v77, route, a9);
				} else {
					result = CONTOURNE1(lineIdx, dataIdx, a7, v80, v77, route, a9, v92, v91);
				}
			}
			if (abs(v80 - lineIdx) < v43)
				result = CONTOURNE(lineIdx, dataIdx, result, v80, v77, route, a9);
			if (v43 < abs(v80 - lineIdx))
				result = CONTOURNE1(lineIdx, dataIdx, result, v80, v77, route, a9, v92, v91);
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
				if (dataIdx > abs(Ligne[lineIdx]._lineDataEndIdx / 2)) {
					result = CONTOURNE1(lineIdx, dataIdx, result, v80, v77, route, a9, v92, v91);
				} else {
					result = CONTOURNE(lineIdx, dataIdx, result, v80, v77, route, a9);
				}
			}
			if (v45 < v48)
				result = CONTOURNE(lineIdx, dataIdx, result, v80, v77, route, a9);
			if (v48 < v45)
				result = CONTOURNE1(lineIdx, dataIdx, result, v80, v77, route, a9, v92, v91);
		}
		if (lineIdx == v80)
			result = CONTOURNE(lineIdx, dataIdx, result, lineIdx, v77, route, a9);
		for(;;) {
			if (!checkCollisionLine(NVPX, NVPY, &foundDataIdx, &foundLineIdx, _lastLine + 1, _linesNumb))
				break;

			switch (Ligne[foundLineIdx]._direction) {
			case 1:
				--NVPY;
				break;
			case 2:
				--NVPY;
				++NVPX;
				break;
			case 3:
				++NVPX;
				break;
			case 4:
				++NVPY;
				++NVPX;
				break;
			case 5:
				++NVPY;
				break;
			case 6:
				++NVPY;
				--NVPX;
				break;
			case 7:
				--NVPX;
				break;
			case 8:
				--NVPY;
				--NVPX;
				break;
			}
		}
	} else {
		NVPX = -1;
		NVPY = -1;
	}
	return result;
}

// Avoid 2
int16 *LinesManager::PARCOURS2(int fromX, int fromY, int destX, int destY) {
	int foundLineIdx;
	int foundDataIdx;
	int v118 = 0;
	int v119 = 0;
	int v126[9];
	int v131[9];
	int v136[9];
	int v141[9];

	int clipDestX = destX;
	int clipDestY = destY;
	int v121 = 0;
	int v120 = 0;
	int v115 = 0;
	int v114 = 0;
	int v113 = 0;
	int v111 = 0;
	if (destY <= 24)
		clipDestY = 25;
	if (!_vm->_globals.NOT_VERIF) {
		if (abs(fromX - _vm->_globals._oldRouteFromX) <= 4 && abs(fromY - _vm->_globals._oldRouteFromY) <= 4 &&
		    abs(_vm->_globals._oldRouteDestX - destX) <= 4 && abs(_vm->_globals._oldRouteDestY - clipDestY) <= 4)
			return (int16 *)g_PTRNUL;

		if (abs(fromX - destX) <= 4 && abs(fromY - clipDestY) <= 4)
			return (int16 *)g_PTRNUL;

		if (_vm->_globals._oldZoneNum > 0 && _vm->_objectsManager._zoneNum > 0 && _vm->_globals._oldZoneNum == _vm->_objectsManager._zoneNum)
			return (int16 *)g_PTRNUL;
	}
	_vm->_globals.NOT_VERIF = false;
	_vm->_globals._oldZoneNum = _vm->_objectsManager._zoneNum;
	_vm->_globals._oldRouteFromX = fromX;
	_vm->_globals._oldRouteDestX = destX;
	_vm->_globals._oldRouteFromY = fromY;
	_vm->_globals._oldRouteDestY = clipDestY;
	_pathFindingMaxDepth = 0;
	int v112 = 0;
	if (destX <= 19)
		clipDestX = 20;
	if (clipDestY <= 19)
		clipDestY = 20;
	if (clipDestX > _vm->_graphicsManager._maxX - 10)
		clipDestX = _vm->_graphicsManager._maxX - 10;
	if (clipDestY > _vm->_globals._characterMaxPosY)
		clipDestY = _vm->_globals._characterMaxPosY;

	if (abs(fromX - clipDestX) <= 3 && abs(fromY - clipDestY) <= 3)
		return (int16 *)g_PTRNUL;

	for (int i = 0; i <= 8; ++i) {
		v141[i] = -1;
		v136[i] = 0;
		v131[i] = 1300;
		v126[i] = 1300;
	}

	if (PARC_PERS(fromX, fromY, clipDestX, clipDestY, -1, -1, 0) == 1)
		return &super_parcours[0];

	int v14 = 0;
	for (int tmpY = clipDestY; tmpY < _vm->_graphicsManager._maxY; tmpY++, v14++) { 
		if (checkCollisionLine(clipDestX, tmpY, &v136[5], &v141[5], 0, _lastLine) && v141[5] <= _lastLine)
			break;
		v136[5] = 0;
		v141[5] = -1;
	}
	v131[5] = v14;

	v14 = 0;
	for (int tmpY = clipDestY; tmpY > _vm->_graphicsManager._minY; tmpY--, v14++) {
		if (checkCollisionLine(clipDestX, tmpY, &v136[1], &v141[1], 0, _lastLine) && v141[1] <= _lastLine)
			break;
		v136[1] = 0;
		v141[1] = -1;
		if (v131[5] < v14 && v141[5] != -1)
			break;
	}
	v131[1] = v14;

	v14 = 0;
	for (int tmpX = clipDestX; tmpX < _vm->_graphicsManager._maxX; tmpX++) {
		if (checkCollisionLine(tmpX, clipDestY, &v136[3], &v141[3], 0, _lastLine) && v141[3] <= _lastLine)
			break;
		v136[3] = 0;
		v141[3] = -1;
		++v14;
		if (v131[1] < v14 && v141[1] != -1)
				break;
		if (v131[5] < v14 && v141[5] != -1)
			break;
	}
	v131[3] = v14;

	v14 = 0;
	for (int tmpX = clipDestX; tmpX > _vm->_graphicsManager._minX; tmpX--) {
		if (checkCollisionLine(tmpX, clipDestY, &v136[7], &v141[7], 0, _lastLine) && v141[7] <= _lastLine)
			break;
		v136[7] = 0;
		v141[7] = -1;
		++v14;
		if (v131[1] < v14 && v141[1] != -1)
			break;
		if (v131[5] < v14 && v141[5] != -1)
			break;
		if (v131[3] < v14 && v141[3] != -1)
			break;
	}
	v131[7] = v14;

	if (v141[1] < 0 || _lastLine < v141[1])
		v141[1] = -1;
	if (v141[3] < 0 || _lastLine < v141[3])
		v141[3] = -1;
	if (v141[5] < 0 || _lastLine < v141[5])
		v141[5] = -1;
	if (v141[7] < 0 || _lastLine < v141[7])
		v141[7] = -1;
	if (v141[1] < 0)
		v131[1] = 1300;
	if (v141[3] < 0)
		v131[3] = 1300;
	if (v141[5] < 0)
		v131[5] = 1300;
	if (v141[7] < 0)
		v131[7] = 1300;
	if (v141[1] == -1 && v141[3] == -1 && v141[5] == -1 && v141[7] == -1)
		return (int16 *)g_PTRNUL;

	if (v141[5] != -1 && v131[1] >= v131[5] && v131[3] >= v131[5] && v131[7] >= v131[5]) {
		v121 = v141[5];
		v120 = v136[5];
	} else if (v141[1] != -1 && v131[5] >= v131[1] && v131[3] >= v131[1] && v131[7] >= v131[1]) {
		v121 = v141[1];
		v120 = v136[1];
	} else if (v141[3] != -1 && v131[1] >= v131[3] && v131[5] >= v131[3] && v131[7] >= v131[3]) {
		v121 = v141[3];
		v120 = v136[3];
	} else if (v141[7] != -1 && v131[5] >= v131[7] && v131[3] >= v131[7] && v131[1] >= v131[7]) {
		v121 = v141[7];
		v120 = v136[7];
	}

	for (int i = 0; i <= 8; ++i) {
		v141[i] = -1;
		v136[i] = 0;
		v131[i] = 1300;
		v126[i] = 1300;
	}

	v14 = 0;
	for (int tmpY = fromY; tmpY < _vm->_graphicsManager._maxY; tmpY++, v14++) {
		if (checkCollisionLine(fromX, tmpY, &v136[5], &v141[5], 0, _lastLine) && v141[5] <= _lastLine)
			break;
		v136[5] = 0;
		v141[5] = -1;
	}
	v131[5] = v14 + 1;

	v14 = 0;
	for (int tmpY = fromY; tmpY > _vm->_graphicsManager._minY; tmpY--) {
		if (checkCollisionLine(fromX, tmpY, &v136[1], &v141[1], 0, _lastLine) && v141[1] <= _lastLine)
			break;
		v136[1] = 0;
		v141[1] = -1;
		++v14;
		if (v141[5] != -1 && v14 > 80)
			break;
	}
	v131[1] = v14 + 1;

	v14 = 0;
	for (int tmpX = fromX; tmpX < _vm->_graphicsManager._maxX; tmpX++) {
		if (checkCollisionLine(tmpX, fromY, &v136[3], &v141[3], 0, _lastLine) && v141[3] <= _lastLine)
			break;
		v136[3] = 0;
		v141[3] = -1;
		++v14;
		if ((v141[5] != -1 || v141[1] != -1) && (v14 > 100))
			break;
	}
	v131[3] = v14 + 1;

	v14 = 0;
	for (int tmpX = fromX; tmpX > _vm->_graphicsManager._minX; tmpX--) {
		if (checkCollisionLine(tmpX, fromY, &v136[7], &v141[7], 0, _lastLine) && v141[7] <= _lastLine)
			break;
		v136[7] = 0;
		v141[7] = -1;
		++v14;
		if ((v141[5] != -1 || v141[1] != -1 || v141[3] != -1) && (v14 > 100))
			break;
	}
	v131[7] = v14 + 1;

	if (v141[1] != -1)
		v126[1] = abs(v141[1] - v121);

	if (v141[3] != -1)
		v126[3] = abs(v141[3] - v121);

	if (v141[5] != -1)
		v126[5] = abs(v141[5] - v121);

	if (v141[7] != -1)
		v126[7] = abs(v141[7] - v121);

	if (v141[1] == -1 && v141[3] == -1 && v141[5] == -1 && v141[7] == -1)
		error("Nearest point not found");

	if (v141[1] != -1 && v126[3] >= v126[1] && v126[5] >= v126[1] && v126[7] >= v126[1]) {
		v115 = v141[1];
		v111 = v131[1];
		v113 = 1;
		v114 = v136[1];
	} else if (v141[5] != -1 && v126[3] >= v126[5] && v126[1] >= v126[5] && v126[7] >= v126[5]) {
		v115 = v141[5];
		v111 = v131[5];
		v113 = 5;
		v114 = v136[5];
	} else if (v141[3] != -1 && v126[1] >= v126[3] && v126[5] >= v126[3] && v126[7] >= v126[3]) {
		v115 = v141[3];
		v111 = v131[3];
		v113 = 3;
		v114 = v136[3];
	} else if (v141[7] != -1 && v126[1] >= v126[7] && v126[5] >= v126[7] && v126[3] >= v126[7]) {
		v115 = v141[7];
		v111 = v131[7];
		v113 = 7;
		v114 = v136[7];
	}

	int v55 = PARC_PERS(fromX, fromY, clipDestX, clipDestY, v115, v121, 0);
	
	if (v55 == 1)
		return &super_parcours[0];

	if (v55 == 2) {
		v115 = NV_LIGNEDEP;
		v114 = NV_LIGNEOFS;
		v112 = NV_POSI;
	} else {
		if (v113 == 1) {
			for (int deltaY = 0; deltaY < v111; deltaY++) {
				if (checkCollisionLine(fromX, fromY - deltaY, &foundDataIdx, &foundLineIdx, _lastLine + 1, _linesNumb) && _lastLine < foundLineIdx) {
					int v58 = GENIAL(foundLineIdx, foundDataIdx, fromX, fromY - deltaY, fromX, fromY - v111, v112, &super_parcours[0], 4);
					if (v58 == -1) {
						super_parcours[v112] = -1;
						super_parcours[v112 + 1] = -1;
						super_parcours[v112 + 2] = -1;
						super_parcours[v112 + 3] = -1;
						return &super_parcours[0];
					}
					v112 = v58;
					if (NVPY != -1)
						deltaY = fromY - NVPY;
				}
				super_parcours[v112] = fromX;
				super_parcours[v112 + 1] = fromY - deltaY;
				super_parcours[v112 + 2] = 1;
				super_parcours[v112 + 3] = 0;
				v112 += 4;
			}
		}
		if (v113 == 5) {
			for (int deltaY = 0; deltaY < v111; deltaY++) {
				if (checkCollisionLine(fromX, deltaY + fromY, &foundDataIdx, &foundLineIdx, _lastLine + 1, _linesNumb)
				        && _lastLine < foundLineIdx) {
					int v61 = GENIAL(foundLineIdx, foundDataIdx, fromX, deltaY + fromY, fromX, v111 + fromY, v112, &super_parcours[0], 4);
					if (v61 == -1) {
						super_parcours[v112] = -1;
						super_parcours[v112 + 1] = -1;
						super_parcours[v112 + 2] = -1;
						super_parcours[v112 + 3] = -1;
						return &super_parcours[0];
					}
					v112 = v61;
					if (NVPY != -1)
						deltaY = NVPY - fromY;
				}
				super_parcours[v112] = fromX;
				super_parcours[v112 + 1] = deltaY + fromY;
				super_parcours[v112 + 2] = 5;
				super_parcours[v112 + 3] = 0;
				v112 += 4;
			}
		}
		if (v113 == 7) {
			for (int deltaX = 0; deltaX < v111; deltaX++) {
				if (checkCollisionLine(fromX - deltaX, fromY, &foundDataIdx, &foundLineIdx, _lastLine + 1, _linesNumb) && _lastLine < foundLineIdx) {
					int v64 = GENIAL(foundLineIdx, foundDataIdx, fromX - deltaX, fromY, fromX - v111, fromY, v112, &super_parcours[0], 4);
					if (v64 == -1) {
						super_parcours[v112] = -1;
						super_parcours[v112 + 1] = -1;
						super_parcours[v112 + 2] = -1;
						super_parcours[v112 + 3] = -1;
						return &super_parcours[0];
					}
					v112 = v64;
					if (NVPX != -1)
						deltaX = fromX - NVPX;
				}
				super_parcours[v112] = fromX - deltaX;
				super_parcours[v112 + 1] = fromY;
				super_parcours[v112 + 2] = 7;
				super_parcours[v112 + 3] = 0;
				v112 += 4;
			}
		}
		if (v113 == 3) {
			for (int deltaX = 0; deltaX < v111; deltaX++) {
				if (checkCollisionLine(deltaX + fromX, fromY, &foundDataIdx, &foundLineIdx, _lastLine + 1, _linesNumb) && _lastLine < foundLineIdx) {
					int v67 = GENIAL(foundLineIdx, foundDataIdx, deltaX + fromX, fromY, v111 + fromX, fromY, v112, &super_parcours[0], 4);
					if (v67 == -1) {
						super_parcours[v112] = -1;
						super_parcours[v112 + 1] = -1;
						super_parcours[v112 + 2] = -1;
						super_parcours[v112 + 3] = -1;
						return &super_parcours[0];
					}
					v112 = v67;
					if (NVPX != -1)
						deltaX = NVPX - fromX;
				}
				super_parcours[v112] = deltaX + fromX;
				super_parcours[v112 + 1] = fromY;
				super_parcours[v112 + 2] = 3;
				super_parcours[v112 + 3] = 0;
				v112 += 4;
			}
		}
	}
	

	bool loopCond;
	do {
		loopCond = false;
		if (v115 < v121) {
			for (int i = v114; Ligne[v115]._lineDataEndIdx > i; ++i) {
				v119 = Ligne[v115]._lineData[2 * i];
				v118 = Ligne[v115]._lineData[2 * i + 1];
				super_parcours[v112] = Ligne[v115]._lineData[2 * i];
				super_parcours[v112 + 1] = Ligne[v115]._lineData[2 * i + 1];
				super_parcours[v112 + 2] = Ligne[v115].field6;
				super_parcours[v112 + 3] = 0;
				v112 += 4;
			}
			for (int v116 = v115 + 1; v116 < v121; v116++) {
				int v72 = 0;
				int v110 = v116;
				for (int j = v116; Ligne[j]._lineDataEndIdx > v72; j = v116) {
					v119 = Ligne[v110]._lineData[2 * v72];
					v118 = Ligne[v110]._lineData[2 * v72 + 1];
					super_parcours[v112] = Ligne[v110]._lineData[2 * v72];
					super_parcours[v112 + 1] = Ligne[v110]._lineData[2 * v72 + 1];
					super_parcours[v112 + 2] = Ligne[v110].field6;
					super_parcours[v112 + 3] = 0;
					v112 += 4;
					if (Ligne[v110]._lineDataEndIdx > 30 && v72 == Ligne[v110]._lineDataEndIdx / 2) {
						int v78 = PARC_PERS(Ligne[v110]._lineData[2 * v72], Ligne[v110]._lineData[2 * v72 + 1], clipDestX, clipDestY, v110, v121, v112);
						if (v78 == 1)
							return &super_parcours[0];
						if (v78 == 2 || MIRACLE(v119, v118, v110, v121, v112)) {
							v115 = NV_LIGNEDEP;
							v114 = NV_LIGNEOFS;
							v112 = NV_POSI;
							loopCond = true;
							break;
						}
					}
					++v72;
					v110 = v116;
				}

				if (loopCond)
					break;

				int v79 = PARC_PERS(v119, v118, clipDestX, clipDestY, v116, v121, v112);
				if (v79 == 1)
					return &super_parcours[0];
				if (v79 == 2 || MIRACLE(v119, v118, v116, v121, v112)) {
					v115 = NV_LIGNEDEP;
					v114 = NV_LIGNEOFS;
					v112 = NV_POSI;
					loopCond = true;
					break;
				}
			}
			if (loopCond)
				continue;

			v114 = 0;
			v115 = v121;
		}
		if (v115 > v121) {
			for (int dataIdx = v114; dataIdx > 0; dataIdx--) {
				v119 = Ligne[v115]._lineData[2 * dataIdx];
				v118 = Ligne[v115]._lineData[2 * dataIdx + 1];

				super_parcours[v112] = Ligne[v115]._lineData[2 * dataIdx];
				super_parcours[v112 + 1] = Ligne[v115]._lineData[2 * dataIdx + 1];
				super_parcours[v112 + 2] = Ligne[v115].field8;
				super_parcours[v112 + 3] = 0;
				v112 += 4;
			}
			for (int v117 = v115 - 1; v117 > v121; v117--) {
				for (int dataIdx = Ligne[v117]._lineDataEndIdx - 1; dataIdx > -1; dataIdx--) {
					v119 = Ligne[v117]._lineData[2 * dataIdx];
					v118 = Ligne[v117]._lineData[2 * dataIdx + 1];
					super_parcours[v112] = Ligne[v117]._lineData[2 * dataIdx];
					super_parcours[v112 + 1] = Ligne[v117]._lineData[2 * dataIdx + 1];
					super_parcours[v112 + 2] = Ligne[v117].field8;
					super_parcours[v112 + 3] = 0;
					v112 += 4;
					if (Ligne[v117]._lineDataEndIdx > 30 && dataIdx == Ligne[v117]._lineDataEndIdx / 2) {
						int v88 = PARC_PERS(v119, v118, clipDestX, clipDestY, v117, v121, v112);
						if (v88 == 1)
							return &super_parcours[0];
						if (v88 == 2 || MIRACLE(v119, v118, v117, v121, v112)) {
							v115 = NV_LIGNEDEP;
							v114 = NV_LIGNEOFS;
							v112 = NV_POSI;
							loopCond = true;
							break;
						}
					}
				}
				if (!loopCond) {
					int v89 = PARC_PERS(v119, v118, clipDestX, clipDestY, v117, v121, v112);
					if (v89 == 1)
						return &super_parcours[0];
					if (v89 == 2 || MIRACLE(v119, v118, v117, v121, v112)) {
						v115 = NV_LIGNEDEP;
						v114 = NV_LIGNEOFS;
						v112 = NV_POSI;
						loopCond = true;
						break;
					}
				}
			}

			if (!loopCond) {
				v114 = Ligne[v121]._lineDataEndIdx - 1;
				v115 = v121;
			}
		}
	} while (loopCond);

	if (v115 == v121) {
		if (v114 <= v120) {
			for (int dataIdx = v114; dataIdx < v120; dataIdx++) {
				super_parcours[v112] = Ligne[v121]._lineData[2 * dataIdx];
				super_parcours[v112 + 1] = Ligne[v121]._lineData[2 * dataIdx + 1];
				super_parcours[v112 + 2] = Ligne[v121].field6;
				super_parcours[v112 + 3] = 0;
				v112 += 4;
			}
		} else {
			for (int dataIdx = v114; dataIdx > v120; dataIdx--) {
				super_parcours[v112] = Ligne[v121]._lineData[2 * dataIdx];
				super_parcours[v112 + 1] = Ligne[v121]._lineData[2 * dataIdx + 1];
				super_parcours[v112 + 2] = Ligne[v121].field8;
				super_parcours[v112 + 3] = 0;
				v112 += 4;
			}
		}
	}
	if (PARC_PERS(super_parcours[v112 - 4], super_parcours[v112 - 3], clipDestX, clipDestY, -1, -1, v112) != 1) {
		super_parcours[v112] = -1;
		super_parcours[v112 + 1] = -1;
		super_parcours[v112 + 2] = -1;
		super_parcours[v112 + 3] = -1;
	}

	return &super_parcours[0];
}

int LinesManager::PARC_PERS(int fromX, int fromY, int destX, int destY, int a5, int a6, int a7) {
	int v18;
	int v19;
	int v20;
	int v21;
	int v22;
	int v23;
	int v24;
	int v33;
	int v36;
	int v39;
	int v40;
	bool v45;
	int v54;
	int v55;
	int v58;
	int v66;
	int newDirection;
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
	int v113;
	int v114;
	int v115;
	int v116;
	int v117;
	int v118;
	int v119;
	int v120;
	int v121;
	int v122;
	int v123;
	int v124;
	int v125;
	bool v126 = false;
	int collLineIdx;
	int collDataIdx;
	int v140;
	int v141;
	int v142;
	int v143;
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
		switch (Ligne[foundLineIdx]._direction) {
		case 1:
			curY -= 2;
			break;
		case 2:
			curY -= 2;
			curX += 2;
			break;
		case 3:
			curX += 2;
			break;
		case 4:
			curY += 2;
			curX += 2;
		case 5:
			curY += 2;
			break;
		case 6:
			curY += 2;
			curX -= 2;
			break;
		case 7:
			curX -= 2;
			break;
		case 8:
			curY -= 2;
			curX -= 2;
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
	for (;;) {
		v111 = curX;
		v109 = curY;
		if (destX >= curX - 2 && destX <= curX + 2 && destY >= curY - 2 && destY <= curY + 2) {
LABEL_149:
			essai0[v115] = -1;
			essai0[v115 + 1] = -1;
			essai0[v115 + 2] = -1;

LABEL_150:
			if (v115) {
				v116 = 0;
				for (;;) {
					super_parcours[v137] = essai0[v116];
					super_parcours[v137 + 1] = essai0[v116 + 1];
					super_parcours[v137 + 2] = essai0[v116 + 2];
					super_parcours[v137 + 3] = 0;
					v116 += 3;
					v137 += 4;

					if (essai0[v116] == -1 && essai0[v116 + 1] == -1)
						break;
				}
			}
			super_parcours[v137] = -1;
			super_parcours[v137 + 1] = -1;
			super_parcours[v137 + 2] = -1;
			super_parcours[v137 + 3] = -1;
			return 1;
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
		newDirection = -1;
		if (v94 == -1 && (v101 >= 0 && v101 <= 150))
			newDirection = 1;
		if (v13 == 1 && (v99 >= -1 && v99 <= 150))
			newDirection = 3;
		if (v94 == 1 && (v101 >= -150 && v101 <= 150))
			newDirection = 5;
		if (v13 == -1 && (v99 >= -150 && v99 <= 150))
			newDirection = 7;
		if (v94 == -1 && (v101 >= -150 && v101 <= 0))
			newDirection = 1;

		if (newDirection == -1 && !checkSmoothMove(curX, v109, destX, destY) && !makeSmoothMove(curX, v109, destX, destY))
			break;
LABEL_72:
		v19 = abs(v111 - destX);
		v20 = v19 + 1;
		v95 = abs(v109 - destY);
		v108 = v95 + 1;
		if (v20 > (v95 + 1))
			v108 = v20;
		if (v108 <= 10)
			goto LABEL_149;
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
			newDirection = 1;
		if (v22 == 1) {
			if (v96 == -1)
				newDirection = 2;
			if (!v96)
				newDirection = 3;
			if (v96 == 1)
				newDirection = 4;
		}
		if (!v22 && v96 == 1)
			newDirection = 5;
		if ((v22 != -1) && (v96 == -1)) {
			if (v102 >= 0 && v102 < 510)
				newDirection = 1;
			else if (v102 >= 510 && v102 <= 1000)
				newDirection = 2;
		} else {
			if (v96 == 1)
				newDirection = 6;
			else if (!v96)
				newDirection = 7;
			else if (v96 == -1) {
				if (v102 >= 0 && v102 < 510)
					newDirection = 1;
				else if (v102 >= 510 && v102 <= 1000)
					newDirection = 2;
				else 
					newDirection = 8;
			}
		}
		if (v22 == 1) {
			if (v100 >= -1 && v100 <= 510)
				newDirection = 2;
			if (v100 >= -510 && v100 <= 0)
				newDirection = 3;
			if (v100 >= 0 && v100 <= 510)
				newDirection = 3;
			if (v100 >= 510 && v100 <= 1000)
				newDirection = 4;
		}
		if (v96 == 1) {
			if (v102 >= 510 && v102 <= 1000)
				newDirection = 4;
			if (v102 >= 0 && v102 <= 510)
				newDirection = 5;
			// CHECKME: The two conditions on v102 are not compatible!
			if (v102 >= -1 && v102 <= -510)
				newDirection = 6;
			if (v102 >= -510 && v102 <= 0)
				newDirection = 5;
		}
		if (v22 == -1) {
			if (v100 >= 510 && v100 <= 1000)
				newDirection = 6;
			if (v100 >= 0 && v100 <= 510)
				newDirection = 7;
			if (v100 >= -1000 && v100 <= -510)
				newDirection = 8;
			if (v100 >= -510 && v100 <= 0)
				newDirection = 7;
		}
		if (v96 == -1) {
			if (v102 >= -1000 && v102 <= -510)
				newDirection = 8;
			if (v102 >= -510 && v102 <= 0)
				newDirection = 1;
		}
		v23 = 0;
		if (v108 + 1 <= 0)
			goto LABEL_149;
		while (!checkCollisionLine(v104, v103, &v143, &v142, 0, _linesNumb)) {
			essai0[v115] = v104;
			essai0[v115 + 1] = v103;
			essai0[v115 + 2] = newDirection;
			v106 += v102;
			v105 += v100;
			v104 = v106 / 1000;
			v103 = v105 / 1000;
			v115 += 3;
			++v23;
			if (v23 >= v108 + 1)
				goto LABEL_149;
		}
		if (_lastLine >= v142)
			goto LABEL_157;
		v24 = GENIAL(v142, v143, v104, v103, destX, destY, v115, essai0, 3);
		if (v24 == -1)
			goto LABEL_150;
		v115 = v24;
		if (NVPX != -1 || NVPY != -1) {
			v142 = -1;
			goto LABEL_157;
		}
		curX = -1;
		curY = -1;
	}
	newDirection = _smoothMoveDirection;
	v14 = 0;
	for (;;) {
		if (_smoothRoute[v14]._posX == -1 || _smoothRoute[v14]._posY == -1) {
			v126 = true;
			v18 = v14 - 1;
			v111 = _smoothRoute[v18]._posX;
			v109 = _smoothRoute[v18]._posY;
			goto LABEL_72;
		}
		if (checkCollisionLine(_smoothRoute[v14]._posX, _smoothRoute[v14]._posY, &v143, &v142, 0, _linesNumb))
			break;

		essai0[v115] = _smoothRoute[v14]._posX;
		essai0[v115 + 1] = _smoothRoute[v14]._posY;
		essai0[v115 + 2] = newDirection;
		v115 += 3;
		++v14;
		if (v126) {
			v18 = v14 - 1;
			v111 = _smoothRoute[v18]._posX;
			v109 = _smoothRoute[v18]._posY;
			goto LABEL_72;
		}
	}
	if (v142 > _lastLine)
		v142 = -1;

LABEL_157:
	essai0[v115] = -1;
	essai0[v115 + 1] = -1;
	essai0[v115 + 2] = -1;

	v117 = 0;
	v33 = v98;
	v92 = v97;
LABEL_158:
	v113 = v33;
	if (destX >= v33 - 2 && destX <= v33 + 2 && destY >= v92 - 2 && destY <= v92 + 2)
		goto LABEL_194;
	if (v33 >= destX) {
LABEL_165:
		if (v113 > destX) {
			v36 = v113;
			while (!checkCollisionLine(v36, v92, &v141, &v140, 0, _linesNumb)) {
				essai1[v117] = v36;
				essai1[v117 + 1] = v92;
				essai1[v117 + 2] = 7;
				v117 += 3;
				--v36;
				if (destX >= v36)
					goto LABEL_171;
			}
			goto LABEL_168;
		}
LABEL_171:
		if (v92 >= destY) {
LABEL_181:
			for (int v43 = v92; v43 > destY; v43--) {
				if (checkCollisionLine(destX, v43, &v141, &v140, 0, _linesNumb)) {
					if (_lastLine < v140) {
						int v44 = GENIAL(v140, v141, destX, v43, destX, destY, v117, essai1, 3);
						if (v44 == -1)
							goto LABEL_195;
						v117 = v44;
						if (NVPX != -1 && NVPY != -1) {
							v33 = NVPX;
							v92 = NVPY;
							v45 = checkCollisionLine(NVPX, NVPY, &v141, &v140, 0, _lastLine);
							if (v45 && v140 <= _lastLine)
								goto LABEL_202;
							goto LABEL_158;
						}
					}
					if (v140 <= _lastLine)
						goto LABEL_202;
				}
				essai1[v117] = destX;
				essai1[v117 + 1] = v43;
				essai1[v117 + 2] = 1;
				v117 += 3;
			}
LABEL_194:
			essai1[v117] = -1;
			essai1[v117 + 1] = -1;
			essai1[v117 + 2] = -1;
LABEL_195:
			if (v117) {
				v118 = 0;
				for (;;) {
					super_parcours[v137] = essai1[v118];
					super_parcours[v137 + 1] = essai1[v118 + 1];
					super_parcours[v137 + 2] = essai1[v118 + 2];
					super_parcours[v137 + 3] = 0;
					v118 += 3;
					v137 += 4;
					if (essai1[v118] == -1 && essai1[v118 + 1] == -1)
						break;
				}
			}
			super_parcours[v137] = -1;
			super_parcours[v137 + 1] = -1;
			super_parcours[v137 + 2] = -1;
			super_parcours[v137 + 3] = -1;
			return 1;
		}
		v39 = v92;
		for (;;) {
			if (checkCollisionLine(destX, v39, &v141, &v140, 0, _linesNumb)) {
				if (_lastLine < v140) {
					v40 = GENIAL(v140, v141, destX, v39, destX, destY, v117, essai1, 3);
					if (v40 == -1)
						goto LABEL_195;
					v117 = v40;
					if (NVPX != -1 && NVPY != -1) {
						v33 = NVPX;
						v92 = NVPY;
						v45 = checkCollisionLine(NVPX, NVPY, &v141, &v140, 0, _lastLine);
						if (v45 && v140 <= _lastLine)
							goto LABEL_202;
						goto LABEL_158;
					}
				}
				if (v140 <= _lastLine)
					goto LABEL_202;
			}

			essai1[v117] = destX;
			essai1[v117 + 1] = v39;
			essai1[v117 + 2] = 5;
			v117 += 3;
			++v39;
			if (destY <= v39)
				goto LABEL_181;
		}
	}
	while (!checkCollisionLine(v33, v92, &v141, &v140, 0, _linesNumb)) {
		essai1[v117] = v33;
		essai1[v117 + 1] = v92;
		essai1[v117 + 2] = 3;
		v117 += 3;
		++v33;
		if (destX <= v33)
			goto LABEL_165;
	}
LABEL_168:
	if (v140 > _lastLine)
		v140 = -1;
LABEL_202:
	essai1[v117] = -1;
	essai1[v117 + 1] = -1;
	essai1[v117 + 2] = -1;
	v117 = 0;
	v54 = v98;
	v93 = v97;
LABEL_203:
	v114 = v54;
	if (destX >= v54 - 2 && destX <= v54 + 2 && destY >= v93 - 2 && destY <= v93 + 2)
		goto LABEL_241;
	if (v93 < destY) {
		v55 = v93;
		while (!checkCollisionLine(v114, v55, &collDataIdx, &collLineIdx, 0, _linesNumb)) {
			essai2[v117] = v114;
			essai2[v117 + 1] = v55;
			essai2[v117 + 2] = 5;
			v117 += 3;
			++v55;
			if (destY <= v55)
				goto LABEL_211;
		}
		goto LABEL_214;
	}
LABEL_211:
	if (v93 > destY) {
		v58 = v93;
		while (!checkCollisionLine(v114, v58, &collDataIdx, &collLineIdx, 0, _linesNumb)) {
			essai2[v117] = v114;
			essai2[v117 + 1] = v58;
			essai2[v117 + 2] = 1;
			v117 += 3;
			--v58;
			if (destY >= v58)
				goto LABEL_217;
		}
LABEL_214:
		if (collLineIdx > _lastLine)
			collLineIdx = -1;
LABEL_249:
		essai2[v117] = -1;
		essai2[v117 + 1] = -1;
		essai2[v117 + 2] = -1;

		if (!v136) {
			if (a6 > foundLineIdx) {
				if (essai0[0] != -1 && v142 > foundLineIdx && v140 <= v142 && collLineIdx <= v142 && a6 >= v142) {
					NV_LIGNEDEP = v142;
					NV_LIGNEOFS = v143;
					v120 = 0;
					for (;;) {
						super_parcours[v137] = essai0[v120];
						super_parcours[v137 + 1] = essai0[v120 + 1];
						super_parcours[v137 + 2] = essai0[v120 + 2];
						super_parcours[v137 + 3] = 0;
						v120 += 3;
						v137 += 4;
						if (essai0[v120] == -1 && essai0[v120 + 1] == -1)
							break;
					}
					NV_POSI = v137;
					return 2;
				}
				if (essai1[0] != -1 && foundLineIdx < v140 && collLineIdx <= v140 && v142 <= v140 && a6 >= v140) {
					NV_LIGNEDEP = v140;
					NV_LIGNEOFS = v141;
					v121 = 0;
					for (;;) {
						assert(v137 <= 32000);
						super_parcours[v137] = essai1[v121];
						super_parcours[v137 + 1] = essai1[v121 + 1];
						super_parcours[v137 + 2] = essai1[v121 + 2];
						super_parcours[v137 + 3] = 0;
						v121 += 3;
						v137 += 4;
						if (essai1[v121] == -1 && essai1[v121 + 1] == -1)
							break;
					}
					NV_POSI = v137;
					return 2;
				}
				if (essai2[0] != -1) {
					if (foundLineIdx < collLineIdx && v140 < collLineIdx && v142 < collLineIdx && a6 >= collLineIdx) {
						NV_LIGNEDEP = collLineIdx;
						NV_LIGNEOFS = collDataIdx;
						v122 = 0;
						for (;;) {
							assert(v137 <= 32000);
							super_parcours[v137] = essai2[v122];
							super_parcours[v137 + 1] = essai2[v122 + 1];
							super_parcours[v137 + 2] = essai2[v122 + 2];
							super_parcours[v137 + 3] = 0;
							v122 += 3;
							v137 += 4;
							if (essai2[v122] == -1 && essai2[v122 + 1] == -1)
								break;
						};
						NV_POSI = v137;
						return 2;
					}
				}
			}
			if (a6 < foundLineIdx) {
				if (v142 == -1)
					v142 = 1300;
				if (v140 == -1)
					v142 = 1300;
				if (collLineIdx == -1)
					v142 = 1300;
				if (essai1[0] != -1 && v140 < foundLineIdx && collLineIdx >= v140 && v142 >= v140 && a6 <= v140) {
					NV_LIGNEDEP = v140;
					NV_LIGNEOFS = v141;
					v123 = 0;
					for (;;) {
						assert(137 <= 32000);
						super_parcours[v137] = essai1[v123];
						super_parcours[v137 + 1] = essai1[v123 + 1];
						super_parcours[v137 + 2] = essai1[v123 + 2];
						super_parcours[v137 + 3] = 0;
						v123 += 3;
						v137 += 4;
						if (essai1[v123] == -1 && essai1[v123 + 1] == -1)
							break;
					}
					NV_POSI = v137;
					return 2;
				}
				if (essai2[0] != -1 && foundLineIdx > collLineIdx && v140 >= collLineIdx && v142 >= collLineIdx && a6 <= collLineIdx) {
					NV_LIGNEDEP = collLineIdx;
					NV_LIGNEOFS = collDataIdx;
					v124 = 0;
					for (;;) {
						assert(v137 <= 32000);
						super_parcours[v137] = essai2[v124];
						super_parcours[v137 + 1] = essai2[v124 + 1];
						super_parcours[v137 + 2] = essai2[v124 + 2];
						super_parcours[v137 + 3] = 0;
						v124 += 3;
						v137 += 4;
						if (essai2[v124] == -1 && essai2[v124 + 1] == -1)
							break;
					}
					NV_POSI = v137;
					return 2;
				}
				if (essai1[0] != -1 && foundLineIdx > v142 && v140 >= v142 && collLineIdx >= v142 && a6 <= v142) {
					NV_LIGNEDEP = v142;
					NV_LIGNEOFS = v143;
					v125 = 0;
					for (;;) {
						assert(137 <= 32000);
						super_parcours[v137] = essai0[v125];
						super_parcours[v137 + 1] = essai0[v125 + 1];
						super_parcours[v137 + 2] = essai0[v125 + 2];
						super_parcours[v137 + 3] = 0;
						v125 += 3;
						v137 += 4;
						if (essai0[v125] == -1 && essai0[v125 + 1] == -1)
							break;
					}

					NV_POSI = v137;
					return 2;
				}
			}
		}
		return 0;
	}
LABEL_217:
	if (v114 < destX) {
		for (int v61 = v114; v61 < destX; v61++) {
			if (checkCollisionLine(v61, destY, &collDataIdx, &collLineIdx, 0, _linesNumb)) {
				if (_lastLine < collLineIdx) {
					int v62 = GENIAL(collLineIdx, collDataIdx, v61, destY, destX, destY, v117, essai2, 3);
					if (v62 == -1)
						goto LABEL_195;
					v117 = v62;
					if (NVPX != -1) {
						if (NVPY != -1) {
							v54 = NVPX;
							v93 = NVPY;
							colResult = checkCollisionLine(NVPX, NVPY, &collDataIdx, &collLineIdx, 0, _lastLine);
							if (colResult && collLineIdx <= _lastLine)
								goto LABEL_249;
							goto LABEL_203;
						}
					}
				}
				if (collLineIdx <= _lastLine)
					goto LABEL_249;
			}

			essai2[v117] = v61;
			essai2[v117 + 1] = destY;
			essai2[v117 + 2] = 3;
			v117 += 3;
		}
	}
	if (v114 > destX) {
		for (int v65 = v114; v65 > destX; v65--) {
			if (checkCollisionLine(v65, destY, &collDataIdx, &collLineIdx, 0, _linesNumb)) {
				if (_lastLine < collLineIdx) {
					v66 = GENIAL(collLineIdx, collDataIdx, v65, destY, destX, destY, v117, essai2, 3);
					if (v66 == -1)
						goto LABEL_242;
					v117 = v66;
					if (NVPX != -1 && NVPY != -1) {
						v54 = NVPX;
						v93 = NVPY;
						colResult = checkCollisionLine(NVPX, NVPY, &collDataIdx, &collLineIdx, 0, _lastLine);
						if (colResult && collLineIdx <= _lastLine)
							goto LABEL_249;
						goto LABEL_203;
					}
				}
				if (collLineIdx <= _lastLine)
					goto LABEL_249;
			}
			essai2[v117] = v65;
			essai2[v117 + 1] = destY;
			essai2[v117 + 2] = 7;
			v117 += 3;
		}
	}
	collLineIdx = -1;
LABEL_241:
	essai2[v117] = -1;
	essai2[v117 + 1] = -1;
	essai2[v117 + 2] = -1;
LABEL_242:
	if (v117) {
		v119 = 0;
		for (;;) {
			super_parcours[v137] = essai2[v119];
			super_parcours[v137 + 1] = essai2[v119 + 1];
			super_parcours[v137 + 2] = essai2[v119 + 2];
			super_parcours[v137 + 3] = 0;
			v119 += 3;
			v137 += 4;
			if (essai2[v119] == -1 && essai2[v119 + 1] == -1)
				break;
		}
	}
	super_parcours[v137] = -1;
	super_parcours[v137 + 1] = -1;
	super_parcours[v137 + 2] = -1;
	super_parcours[v137 + 3] = -1;
	return 1;
}

int16 *LinesManager::cityMapCarRoute(int x1, int y1, int x2, int y2) {
	int16 *result;
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
				v28 = essai2[v27];
				int v29 = essai2[v27 + 1];
				int v66 = essai2[v27 + 2];
				v27 += 4;

				if (checkCollisionLine(v28, v29, &arrDataIdx[1], &arrLineIdx[1], 0, _lastLine))
					break;

				super_parcours[superRouteIdx] = v28;
				super_parcours[superRouteIdx + 1] = v29;
				super_parcours[superRouteIdx + 2] = v66;
				super_parcours[superRouteIdx + 3] = 0;

				essai0[superRouteIdx] = v28;
				essai0[superRouteIdx + 1] = v29;
				essai0[superRouteIdx + 2] = v66;
				essai0[superRouteIdx + 3] = 0;
				superRouteIdx += 4;
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
				int v34 = v68;
				for (int i = Ligne[v69]._lineDataEndIdx; v34 < i - 2; i = Ligne[v69]._lineDataEndIdx) {
					super_parcours[superRouteIdx] = Ligne[v69]._lineData[2 * v34];
					super_parcours[superRouteIdx + 1] = Ligne[v69]._lineData[2 * v34 + 1];
					super_parcours[superRouteIdx + 2] = Ligne[v69].field6;
					super_parcours[superRouteIdx + 3] = 0;
					superRouteIdx += 4;
					++v34;
				}
				for (int j = v69 + 1; j < v73; ++j) {
					if (PLAN_TEST(Ligne[j]._lineData[0], Ligne[j]._lineData[1], superRouteIdx, j, v73)) {
						v69 = NV_LIGNEDEP;
						v68 = NV_LIGNEOFS;
						superRouteIdx = NV_POSI;
						loopFl = true;
						break;
					}
					if (Ligne[j]._lineDataEndIdx - 2 > 0) {
						for (int v40 = 0; v40 < Ligne[j]._lineDataEndIdx - 2; v40++) {
							super_parcours[superRouteIdx] = Ligne[j]._lineData[2 * v40];
							super_parcours[superRouteIdx + 1] = Ligne[j]._lineData[2 * v40 + 1];
							super_parcours[superRouteIdx + 2] = Ligne[j].field6;
							super_parcours[superRouteIdx + 3] = 0;
							superRouteIdx += 4;
						}
					}
				}
				if (loopFl)
					continue;
				v68 = 0;
				v69 = v73;
			}
			if (v69 > v73) {
				for (int k = v68; k > 0; --k) {
					super_parcours[superRouteIdx] = Ligne[v69]._lineData[2 * k];
					super_parcours[superRouteIdx + 1] = Ligne[v69]._lineData[2 * k + 1];
					super_parcours[superRouteIdx + 2] = Ligne[v69].field8;
					super_parcours[superRouteIdx + 3] = 0;
					superRouteIdx += 4;
				}
				for (int l = v69 - 1; l > v73; --l) {
					int v48 = l;
					if (PLAN_TEST(Ligne[l]._lineData[2 * Ligne[v48]._lineDataEndIdx - 2], Ligne[l]._lineData[2 * Ligne[v48]._lineDataEndIdx - 1], superRouteIdx, l, v73)) {
						v69 = NV_LIGNEDEP;
						v68 = NV_LIGNEOFS;
						superRouteIdx = NV_POSI;
						loopFl = true;
						break;
					}

					for (int v49 = Ligne[v48]._lineDataEndIdx - 2; v49 > 0; v49 --) {
						super_parcours[superRouteIdx] = Ligne[l]._lineData[2 * v49];
						super_parcours[superRouteIdx + 1] = Ligne[l]._lineData[2 * v49 + 1];
						super_parcours[superRouteIdx + 2] = Ligne[l].field8;
						super_parcours[superRouteIdx + 3] = 0;
						superRouteIdx += 4;
					}
				}
				if (loopFl)
					continue;

				v68 = Ligne[v73]._lineDataEndIdx - 1;
				v69 = v73;
			}
			if (v69 == v73) {
				if (v68 <= v72) {
					for (int v57 = v68; v57 < v72; v57++) {
						super_parcours[superRouteIdx] = Ligne[v73]._lineData[2 * v57];
						super_parcours[superRouteIdx + 1] = Ligne[v73]._lineData[2 * v57 + 1];
						super_parcours[superRouteIdx + 2] = Ligne[v73].field6;
						super_parcours[superRouteIdx + 3] = 0;
						superRouteIdx += 4;
					}
				} else {
					for (int v53 = v68; v53 > v72; v53--) {
						super_parcours[superRouteIdx] = Ligne[v73]._lineData[2 * v53];
						super_parcours[superRouteIdx + 1] = Ligne[v73]._lineData[2 * v53 + 1];
						super_parcours[superRouteIdx + 2] = Ligne[v73].field8;
						super_parcours[superRouteIdx + 3] = 0;
						superRouteIdx += 4;
					}
				}
			}
		}
		super_parcours[superRouteIdx] = -1;
		super_parcours[superRouteIdx + 1] = -1;
		super_parcours[superRouteIdx + 2] = -1;
		super_parcours[superRouteIdx + 3] = -1;
		result = &super_parcours[0];
	} else {
		result = (int16 *)g_PTRNUL;
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
			int v25 = _vm->_globals.Hopkins[hopkinsIdx].field0;
			int v40 = _vm->_globals.Hopkins[hopkinsIdx].field2;
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
			_smoothMoveDirection = 6;
			return false;
		}
	} else if (fromX < destX && destY > fromY) {
		int hopkinsIdx = 36;
		int smoothIdx = 0;
		int stepCount = 0;
		while (curX < destX && destY > curY) {
			int v14 = _vm->_globals.Hopkins[hopkinsIdx].field0;
			int v39 = _vm->_globals.Hopkins[hopkinsIdx].field2;
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
			_smoothMoveDirection = 4;
			return false;
		}
	} else if (fromX > destX && destY < fromY) {
		int hopkinsIdx = 12;
		int smoothIdx = 0;
		int stepCount = 0;
		while (curX > destX && destY < curY) {
			int v11 = _vm->_graphicsManager.zoomOut(_vm->_globals.Hopkins[hopkinsIdx].field0, 25);
			int v38 = _vm->_graphicsManager.zoomOut(_vm->_globals.Hopkins[hopkinsIdx].field2, 25);
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
			_smoothMoveDirection = 8;
			return false;
		}
	} else if (fromX < destX && destY < fromY) {
		int hopkinsIdx = 12;
		int smoothIdx = 0;
		int stepCount = 0;
		while (curX < destX && destY < curY) {
			int oldY = curY;
			int v7 = _vm->_graphicsManager.zoomOut(_vm->_globals.Hopkins[hopkinsIdx].field0, 25);
			int v37 = _vm->_graphicsManager.zoomOut(_vm->_globals.Hopkins[hopkinsIdx].field2, 25);
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
			_smoothMoveDirection = 2;
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

	int idxTestUp = TEST_LIGNE(paramX, paramY - 2, &v42, &lineIdxTestUp, &dataIdxTestUp);
	int idxTestDown = TEST_LIGNE(paramX, paramY + 2, &v43, &lineIdxTestDown, &dataIdxTestDown);
	int idxTestLeft = TEST_LIGNE(paramX - 2, paramY, &v44, &lineIdxTestLeft, &dataIdxTestLeft);
	int idxTestRight = TEST_LIGNE(paramX + 2, paramY, &v45, &lineIdxTestRight, &dataIdxTestRight);
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
		NV_LIGNEDEP = lineIdxTestUp;
		NV_LIGNEOFS = dataIdxTestUp;
	} else if (v8 == 2) {
		idxTest = idxTestDown;
		v33 = v43;
		NV_LIGNEDEP = lineIdxTestDown;
		NV_LIGNEOFS = dataIdxTestDown;
	} else if (v8 == 3) {
		idxTest = idxTestLeft;
		v33 = v44;
		NV_LIGNEDEP = lineIdxTestLeft;
		NV_LIGNEOFS = dataIdxTestLeft;
	} else if (v8 == 4) {
		idxTest = idxTestRight;
		v33 = v45;
		NV_LIGNEDEP = lineIdxTestRight;
		NV_LIGNEOFS = dataIdxTestRight;
	}

	int superRouteIdx = a3;
	if (v33 == 1) {
		for (int i = 0; i < Ligne[idxTest]._lineDataEndIdx; i++) {
			super_parcours[superRouteIdx] = Ligne[idxTest]._lineData[2 * i];
			super_parcours[superRouteIdx + 1] = Ligne[idxTest]._lineData[2 * i + 1];
			super_parcours[superRouteIdx + 2] = Ligne[idxTest].field6;
			super_parcours[superRouteIdx + 3] = 0;
			superRouteIdx += 4;
		}
	} else if (v33 == 2) {
		for (int v19 = Ligne[idxTest]._lineDataEndIdx - 1; v19 > -1; v19--) {
			super_parcours[superRouteIdx] = Ligne[idxTest]._lineData[2 * v19];
			super_parcours[superRouteIdx + 1] = Ligne[idxTest]._lineData[2 * v19 + 1];
			super_parcours[superRouteIdx + 2] = Ligne[idxTest].field8;
			super_parcours[superRouteIdx + 3] = 0;
			superRouteIdx += 4;
		}
	}
	NV_POSI = superRouteIdx;
	return true;
}

// Test line
int LinesManager::TEST_LIGNE(int paramX, int paramY, int *a3, int *foundLineIdx, int *foundDataIdx) {
	int16 *lineData;
	int lineDataEndIdx;
	int collLineIdx;
	int collDataIdx;

	for (int idx = _lastLine + 1; idx < _linesNumb + 1; idx++) {
		lineData = Ligne[idx]._lineData;
		lineDataEndIdx = Ligne[idx]._lineDataEndIdx;
		if (lineData[0] == paramX && lineData[1] == paramY) {
			*a3 = 1;
			int posX = lineData[2 * (lineDataEndIdx - 1)];
			int posY = lineData[2 * (lineDataEndIdx - 1) + 1];
			if (Ligne[idx].field6 == 5 || Ligne[idx].field6 == 1)
				posY += 2;
			if (Ligne[idx].field6 == 3 || Ligne[idx].field8 == 7)
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
			if (Ligne[idx].field6 == 5 || Ligne[idx].field6 == 1)
				posY -= 2;
			if (Ligne[idx].field6 == 3 || Ligne[idx].field8 == 7)
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
	int retVal = 25;
	int size = _vm->_globals._spriteSize[idx];
	if (_vm->_globals.PERSO_TYPE == 1) {
		if (size < 0)
			size = -size;
		size = 20 * (5 * size - 100) / -80;
	} else if (_vm->_globals.PERSO_TYPE == 2) {
		if (size < 0)
			size = -size;
		size = 20 * (5 * size - 165) / -67;
	}

	if (size < 0)
		retVal = _vm->_graphicsManager.zoomOut(25, -size);
	else if (size > 0)
		retVal = _vm->_graphicsManager.zoomIn(25, size);

	return retVal;
}

void LinesManager::PACOURS_PROPRE(int16 *route) {
	int v4;
	int v5;
	int v9;
	int v10;
	int v11;
	int v12;

	int v1 = 0;
	int v14 = -1;
	int v2 = route[1];
	int v15 = route[2];
	if (route[0] == -1 && v2 == -1)
		return;

	for (;;) {
		if (v14 != -1 && v15 != v14) {
			v11 = v1;
			v12 = 0;
			v10 = CALC_PROPRE(v2);
			v4 = route[v1];
			v9 = route[v1];
			v5 = route[v1 + 1];
			while (v4 != -1 || v5 != -1) {
				int idx = v1;
				v1 += 4;
				++v12;
				if (route[idx + 2] != v15)
					break;
				v4 = route[v1];
				v9 = route[v1];
				v5 = route[v1 + 1];
			}
			if (v12 < v10) {
				int v7 = v11;
				for (int v8 = 0; v8 < v12; v8++) {
					route[v7 + 2] = v14;
					v7 += 4;
				}
				v15 = v14;
			}
			v1 = v11;
			if (v9 == -1 && v5 == -1)
				break;
		}
		v1 += 4;
		v14 = v15;
		v2 = route[v1 + 1];
		v15 = route[v1 + 2];
		if (route[v1] == -1 && v2 == -1)
			break;
	}
}

int LinesManager::MZONE() {
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
			if (ZONEP[squareZoneId]._enabledFl && _squareZone[squareZoneId]._enabledFl == 1
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
			colRes1 = colision(xp, yCurrent);
			if (colRes1 != -1 && ZONEP[colRes1]._enabledFl)
				break;
		}

		if (colRes1 == -1)
			return -1;

		int colRes2 = 0;
		for (int j = yp; j < _vm->_graphicsManager._maxY; ++j) {
			colRes2 = colision(xp, j);
			if (colRes2 != -1 && ZONEP[colRes1]._enabledFl)
				break;
		}

		if (colRes2 == -1)
			return -1;

		int colRes3 = 0;
		for (int k = xp; k >= 0; --k) {
			colRes3 = colision(k, yp);
			if (colRes3 != -1 && ZONEP[colRes1]._enabledFl)
				break;
		}
		if (colRes3 == -1)
			return -1;

		int colRes4 = 0;
		for (int xCurrent = xp; _vm->_graphicsManager._maxX > xCurrent; ++xCurrent) {
			colRes4 = colision(xCurrent, yp);
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

int LinesManager::colision(int xp, int yp) {
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
			int16 *dataP = _vm->_linesManager._zoneLine[curZoneLineIdx]._zoneData;
			if (dataP != (int16 *)g_PTRNUL) {
				int count = _vm->_linesManager._zoneLine[curZoneLineIdx]._count;
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

				if (flag && _vm->_linesManager._zoneLine[curZoneLineIdx]._count > 0) {
					for (int i = 0; i < count; ++i) {
						int xCheck = *dataP++;
						int yCheck = *dataP++;

						if ((xp == xCheck || (xp + 1) == xCheck) && (yp == yCheck))
							return _vm->_linesManager._zoneLine[curZoneLineIdx]._bobZoneIdx;
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
		_squareZone[idx]._enabledFl = 0;
		_squareZone[idx]._squareZoneFl = false;
		_squareZone[idx]._left = 1280;
		_squareZone[idx]._right = 0;
		_squareZone[idx]._top = 460;
		_squareZone[idx]._bottom = 0;
		_squareZone[idx]._minZoneLineIdx = 401;
		_squareZone[idx]._maxZoneLineIdx = 0;
	}

	for (int idx = 0; idx < MAX_LINES; ++idx) {
		int16 *dataP = _vm->_linesManager._zoneLine[idx]._zoneData;
		if (dataP == (int16 *)g_PTRNUL)
			continue;

		int carreZoneId = _vm->_linesManager._zoneLine[idx]._bobZoneIdx;
		_squareZone[carreZoneId]._enabledFl = 1;
		if (_squareZone[carreZoneId]._maxZoneLineIdx < idx)
			_squareZone[carreZoneId]._maxZoneLineIdx = idx;
		if (_squareZone[carreZoneId]._minZoneLineIdx > idx)
			_squareZone[carreZoneId]._minZoneLineIdx = idx;

		for (int i = 0; i < _vm->_linesManager._zoneLine[idx]._count; i++) {
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
		_vm->_linesManager.ZONEP[idx]._destX = 0;
		_vm->_linesManager.ZONEP[idx]._destY = 0;
		_vm->_linesManager.ZONEP[idx]._spriteIndex = 0;
	}

	_vm->_linesManager.essai0 = (int16 *)g_PTRNUL;
	_vm->_linesManager.essai1 = (int16 *)g_PTRNUL;
	_vm->_linesManager.essai2 = (int16 *)g_PTRNUL;
	_vm->_linesManager.BufLig = (int16 *)g_PTRNUL;
	_vm->_linesManager._route = (int16 *)g_PTRNUL;

	for (int idx = 0; idx < MAX_LINES; ++idx) {
		_vm->_linesManager.Ligne[idx]._lineDataEndIdx = 0;
		_vm->_linesManager.Ligne[idx]._direction = 0;
		_vm->_linesManager.Ligne[idx].field6 = 0;
		_vm->_linesManager.Ligne[idx].field8 = 0;
		_vm->_linesManager.Ligne[idx]._lineData = (int16 *)g_PTRNUL;

		_vm->_linesManager._zoneLine[idx]._count = 0;
		_vm->_linesManager._zoneLine[idx]._bobZoneIdx = 0;
		_vm->_linesManager._zoneLine[idx]._zoneData = (int16 *)g_PTRNUL;
	}

	for (int idx = 0; idx < 100; ++idx) {
		_vm->_linesManager._squareZone[idx]._enabledFl = 0;
	}

	BUFFERTAPE = _vm->_globals.allocMemory(85000);

	_vm->_linesManager.essai0 = (int16 *)BUFFERTAPE;
	_vm->_linesManager.essai1 = (int16 *)(BUFFERTAPE + 25000);
	_vm->_linesManager.essai2 = (int16 *)(BUFFERTAPE + 50000);
	_vm->_linesManager.BufLig = (int16 *)(BUFFERTAPE + 75000);
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
		Ligne[idx]._lineDataEndIdx = 0;
		Ligne[idx]._lineData = (int16 *)g_PTRNUL;
	}
}

// Remove Line
void LinesManager::removeLine(int idx) {
	if (idx > MAX_LINES)
		error("Attempting to add a line obstacle > MAX_LIGNE.");
	Ligne[idx]._lineData = (int16 *)_vm->_globals.freeMemory((byte *)Ligne[idx]._lineData);
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

	if (_vm->_globals.NOMARCHE || (_vm->_linesManager._route == (int16 *)g_PTRNUL) || _vm->_globals.compteur_71 > 4) {
		_vm->_globals.compteur_71 = 0;
		int zoneId;
		if (_vm->_globals._oldMouseX != mouseX || _vm->_globals._oldMouseY != oldMouseY) {
			zoneId = _vm->_linesManager.MZONE();
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
			if (ZONEP[zoneId].field6 || ZONEP[zoneId].field7 ||
				ZONEP[zoneId].field8 || ZONEP[zoneId].field9 ||
				ZONEP[zoneId].fieldA || ZONEP[zoneId].fieldB ||
				ZONEP[zoneId].fieldC || ZONEP[zoneId].fieldD ||
				ZONEP[zoneId].fieldE || ZONEP[zoneId].fieldF) {
					if (_vm->_globals._oldMouseZoneId != zoneId) {
						_vm->_fontManager.initTextBuffers(5, ZONEP[zoneId].field12, _vm->_globals.FICH_ZONE, 0, 430, 0, 0, 252);
						_vm->_fontManager.showText(5);
						_vm->_globals._forceHideText = true;
					}
					_vm->_globals._hotspotTextColor += 25;
					if (_vm->_globals._hotspotTextColor > 100)
						_vm->_globals._hotspotTextColor = 0;
					_vm->_graphicsManager.SETCOLOR4(251, _vm->_globals._hotspotTextColor, _vm->_globals._hotspotTextColor,
						_vm->_globals._hotspotTextColor);
					if (_vm->_eventsManager._mouseCursorId == 4) {
						if (ZONEP[zoneId].field6 == 2) {
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
		if (_vm->_globals.NOMARCHE && (_vm->_eventsManager._mouseCursorId == 4)) {
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
