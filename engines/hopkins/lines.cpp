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

#include "common/system.h"
#include "common/textconsole.h"
#include "hopkins/lines.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"

namespace Hopkins {

LinesManager::LinesManager() {
	for (int i = 0; i < MAX_LINES; ++i) {
		Common::fill((byte *)&_zoneLine[i], (byte *)&_zoneLine[i] + sizeof(LigneZoneItem), 0);
		Common::fill((byte *)&Ligne[i], (byte *)&Ligne[i] + sizeof(LigneItem), 0);
	}
	for (int i = 0; i < 4000; ++i) {
		Common::fill((byte *)&_smoothRoute[i], (byte *)&_smoothRoute[i] + sizeof(SmoothItem), 0);
	}

	_linesNumb = 0;
	NV_LIGNEDEP = 0;
	NV_LIGNEOFS = 0;
	NV_POSI = 0;
	NVPX = 0;
	NVPY = 0;
	_smoothMoveDirection = 0;
}

void LinesManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

/**
 * Clear all zones and reset nextLine
 */
void LinesManager::clearAllZones() {
	for (int idx = 0; idx < MAX_LINES; ++idx)
		removeZoneLine(idx);
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
 * Remove Zone Line
 */
void LinesManager::removeZoneLine(int idx) {
	if (idx > MAX_LINES)
		error("Attempting to remove a line obstacle > MAX_LIGNE.");

	_zoneLine[idx]._zoneData = (int16 *)_vm->_globals.freeMemory((byte *)_zoneLine[idx]._zoneData);
}

/**
 * Add Zone Line
 */
void LinesManager::addZoneLine(int idx, int a2, int a3, int a4, int a5, int bobZoneIdx) {
	int16 *zoneData;

	if (a2 == a3 && a3 == a4 && a3 == a5) {
		_vm->_globals.BOBZONE_FLAG[bobZoneIdx] = true;
		_vm->_globals.BOBZONE[bobZoneIdx] = a3;
	} else {
		if (idx > MAX_LINES)
			error("Attempting to add a line obstacle > MAX_LIGNE.");

		_zoneLine[idx]._zoneData = (int16 *)_vm->_globals.freeMemory((byte *)_zoneLine[idx]._zoneData);

		int v8 = abs(a2 - a4);
		int v9 = abs(a3 - a5);
		int v20 = 1;
		if (v8 <= v9)
			v20 += v9;
		else
			v20 += v8;

		zoneData = (int16 *)_vm->_globals.allocMemory(2 * sizeof(int16) * v20 + (4 * sizeof(int16)));
		int v11 = idx;
		_zoneLine[v11]._zoneData = zoneData;
		if (zoneData == (int16 *)g_PTRNUL)
			error("AJOUTE LIGNE ZONE");

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
		_zoneLine[idx].field2 = bobZoneIdx;
	}
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

/**
 * Add Line
 */
void LinesManager::addLine(int idx, int a2, int a3, int a4, int a5, int a6, int a7) {
	int v7;
	int v8;
	int v9;
	byte *v10;
	int v11;
	int v12;
	int v13;
	int v24;
	int v25;
	int16 *v27;
	int v30;
	int v31;
	int16 *v32;
	int v33;
	int v34;
	int v35;
	int v36;
	int v37;
	int v38;
	int v39;
	int v40;

	if (idx > MAX_LINES)
		error("Attempting to add a line obstacle > MAX_LIGNE.");

	if (_linesNumb < idx)
		_linesNumb = idx;

	Ligne[idx]._lineData = (int16 *)_vm->_globals.freeMemory((byte *)Ligne[idx]._lineData);
	v7 = abs(a3 - a5);
	v8 = v7 + 1;
	v9 = abs(a4 - a6);
	v34 = v9 + 1;
	v33 = v9 + 1;
	if (v8 > v34)
		v34 = v8;

	v10 = _vm->_globals.allocMemory(4 * v34 + 8);
	if (v10 == g_PTRNUL)
		error("AJOUTE LIGNE OBSTACLE");

	Common::fill(v10, v10 + 4 * v34 + 8, 0);
	Ligne[idx]._lineData = (int16 *)v10;

	v32 = Ligne[idx]._lineData;
	v36 = 1000 * v8;
	v39 = 1000 * v8 / (v34 - 1);
	v37 = 1000 * v33 / (v34 - 1);
	if (a5 < a3)
		v39 = -v39;
	if (a6 < a4)
		v37 = -v37;
	v11 = (int)v39 / 1000;
	v12 = (int)v37 / 1000;
	v13 = (int)v37 / 1000;
	if (!((int)v39 / 1000)) {
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
		if (!v13) {
			Ligne[idx].field6 = 3;
			Ligne[idx].field8 = 7;
		}
		if (v13 == 1) {
			Ligne[idx].field6 = 4;
			Ligne[idx].field8 = 8;
		}
	}
	if (v11 == -1) {
		if (v13 == 1) {
			Ligne[idx].field6 = 6;
			Ligne[idx].field8 = 2;
		}
		if (!v13) {
			Ligne[idx].field6 = 7;
			Ligne[idx].field8 = 3;
		}
		if (v13 == -1) {
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
	v40 = v36 / v34;
	v38 = 1000 * v33 / v34;
	if (a5 < a3)
		v40 = -v40;
	if (a6 < a4)
		v38 = -v38;
	v24 = 1000 * a3;
	v25 = 1000 * a4;
	v31 = 1000 * a3 / 1000;
	v30 = 1000 * a4 / 1000;
	v35 = v34 - 1;
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

	v27 = v32 + 2;
	v27[0] = -1;
	v27[1] = -1;

	Ligne[idx]._lineDataEndIdx = v35 + 1;
	Ligne[idx].field2 = a7;
	Ligne[idx].field4 = a2;
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

	_vm->_objectsManager._lastLine = lineIdx;
	for (int idx = 1; idx < MAX_LINES; idx++) {
		if ((Ligne[idx]._lineDataEndIdx < _vm->_globals._maxLineLength) && (idx != _vm->_objectsManager._lastLine + 1)) {
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
	int v5;
	int v6;
	int v7;
	int v8;
	int v9;
	int v10;
	int i;
	int v12;
	int v13;
	int j;
	int v15;
	int v16;
	int k;
	int v18;
	int v19;
	int l;
	int v21;
	int v23;
	int v26;
	int v29;
	int v32;
	int v35 = 0;
	int v36 = 0;
	int v37;
	int v38;
	int v39;
	int v40;
	int v41;
	int v42 = 0;
	int v43 = 0;
	int v44 = 0;
	int v45 = 0;
	int v46 = 0;
	int v47 = 0;
	int v48 = 0;
	int v49 = 0;
	int v50;
	int v51;

	v5 = a1;
	v6 = a2;
	v50 = a3;
	v7 = a5;
	if (checkCollisionLine(a1, a2, &v51, &v50, 0, _linesNumb)) {
		v8 = Ligne[v50].field4;
		if (v8 == 1)
			v6 = a2 - 2;
		if (v8 == 2) {
			v6 -= 2;
			v5 = a1 + 2;
		}
		if (v8 == 3)
			v5 += 2;
		if (v8 == 4) {
			v6 += 2;
			v5 += 2;
		}
		if (v8 == 5)
			v6 += 2;
		if (v8 == 6) {
			v6 += 2;
			v5 -= 2;
		}
		if (v8 == 7)
			v5 -= 2;
		if (v8 == 8) {
			v6 -= 2;
			v5 -= 2;
		}
	}
	v41 = v5;
	v40 = v6;
	v9 = 0;
	v10 = v40;
	for (i = v40; v40 + 200 > v10; i = v10) {
		if (checkCollisionLine(v41, i, &v49, &v48, 0, _vm->_objectsManager._lastLine) == 1 && v48 <= _vm->_objectsManager._lastLine)
			break;
		v49 = 0;
		v48 = -1;
		++v9;
		++v10;
	}
	v37 = v9;
	v12 = 0;
	v13 = v40;
	for (j = v40; v40 - 200 < v13; j = v13) {
		if (checkCollisionLine(v41, j, &v47, &v46, 0, _vm->_objectsManager._lastLine) == 1 && v46 <= _vm->_objectsManager._lastLine)
			break;
		v47 = 0;
		v46 = -1;
		++v12;
		--v13;
	}
	v39 = v12;
	v15 = 0;
	v16 = v41;
	for (k = v41; v41 + 200 > v16; k = v16) {
		if (checkCollisionLine(k, v40, &v45, &v44, 0, _vm->_objectsManager._lastLine) == 1 && v44 <= _vm->_objectsManager._lastLine)
			break;
		v45 = 0;
		v44 = -1;
		++v15;
		++v16;
	}
	v38 = v15;
	v18 = 0;
	v19 = v41;
	for (l = v41; v41 - 200 < v19; l = v19) {
		if (checkCollisionLine(l, v40, &v43, &v42, 0, _vm->_objectsManager._lastLine) == 1 && v42 <= _vm->_objectsManager._lastLine)
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
	}
	if (a4 < v50) {
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
		v21 = 0;
		if (a4 > v50) {
			if (v48 <= v46 && v44 <= v46 && v42 <= v46 && v46 > v50)
				v21 = 1;
			if (v48 <= v44 && v46 <= v44 && v42 <= v44 && v50 < v44)
				v21 = 3;
			if (v46 <= v48 && v44 <= v48 && v42 <= v48 && v50 < v48)
				v21 = 5;
			if (v48 <= v42 && v44 <= v42 && v46 <= v42 && v50 < v42)
				v21 = 7;
		}
		if (a4 < v50) {
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
		if (v21) {
			if (v21 == 1) {
				v36 = v46;
				v35 = v47;
			}
			if (v21 == 3) {
				v36 = v44;
				v35 = v45;
			}
			if (v21 == 5) {
				v36 = v48;
				v35 = v49;
			}
			if (v21 == 7) {
				v36 = v42;
				v35 = v43;
			}
			if (v21 == 1) {
				for (int v22 = 0; v22 < v39; v22++) {
					if (checkCollisionLine(v41, v40 - v22, &v47, &v46, _vm->_objectsManager._lastLine + 1, _linesNumb)
					        && _vm->_objectsManager._lastLine < v46) {
								v23 = GENIAL(v46, v47, v41, v40 - v22, v41, v40 - v39, v7, &_vm->_globals.super_parcours[0], 4);
						if (v23 == -1)
							return false;
						v7 = v23;
						if (NVPY != -1)
							v22 = NVPY - v40;
					}
					_vm->_globals.super_parcours[v7] = v41;
					_vm->_globals.super_parcours[v7 + 1] = v40 - v22;
					_vm->_globals.super_parcours[v7 + 2] = 1;
					_vm->_globals.super_parcours[v7 + 3] = 0;
					v7 += 4;
				}
				NV_LIGNEDEP = v36;
				NV_LIGNEOFS = v35;
				NV_POSI = v7;
				return true;
			}
			if (v21 == 5) {
				for (int v25 = 0; v25 < v37; v25++) {
					if (checkCollisionLine(v41, v25 + v40, &v47, &v46, _vm->_objectsManager._lastLine + 1, _linesNumb)
					        && _vm->_objectsManager._lastLine < v46) {
						v26 = GENIAL(v46, v47, v41, v25 + v40, v41, v37 + v40, v7, &_vm->_globals.super_parcours[0], 4);
						if (v26 == -1)
							return false;
						v7 = v26;
						if (NVPY != -1)
							v25 = v40 - NVPY;
					}
					_vm->_globals.super_parcours[v7] = v41;
					_vm->_globals.super_parcours[v7 + 1] = v25 + v40;
					_vm->_globals.super_parcours[v7 + 2] = 5;
					_vm->_globals.super_parcours[v7 + 3] = 0;
					v7 += 4;
				}
				NV_LIGNEDEP = v36;
				NV_LIGNEOFS = v35;
				NV_POSI = v7;
				return true;
			}
			if (v21 == 7) {
				for (int v28 = 0; v28 < v18; v28++) {
					if (checkCollisionLine(v41 - v28, v40, &v47, &v46, _vm->_objectsManager._lastLine + 1, _linesNumb)
					        && _vm->_objectsManager._lastLine < v46) {
						v29 = GENIAL(v46, v47, v41 - v28, v40, v41 - v18, v40, v7, &_vm->_globals.super_parcours[0], 4);
						if (v29 == -1)
							return false;
						v7 = v29;
						if (NVPX != -1)
							v28 = v41 - NVPX;
					}
					_vm->_globals.super_parcours[v7] = v41 - v28;
					_vm->_globals.super_parcours[v7 + 1] = v40;
					_vm->_globals.super_parcours[v7 + 2] = 7;
					_vm->_globals.super_parcours[v7 + 3] = 0;
					v7 += 4;
				}
				NV_LIGNEDEP = v36;
				NV_LIGNEOFS = v35;
				NV_POSI = v7;
				return true;
			}
			if (v21 == 3) {
				for (int v31 = 0; v31 < v38; v31++) {
					if (checkCollisionLine(v31 + v41, v40, &v47, &v46, _vm->_objectsManager._lastLine + 1, _linesNumb)
					        && _vm->_objectsManager._lastLine < v46) {
						v32 = GENIAL(v46, v47, v31 + v41, v40, v38 + v41, v40, v7, &_vm->_globals.super_parcours[0], 4);
						if (v32 == -1)
							return false;
						v7 = v32;
						if (NVPX != -1)
							v31 = NVPX - v41;
					}
					_vm->_globals.super_parcours[v7] = v31 + v41;
					_vm->_globals.super_parcours[v7 + 1] = v40;
					_vm->_globals.super_parcours[v7 + 2] = 3;
					_vm->_globals.super_parcours[v7 + 3] = 0;
					v7 += 4;
				}
				NV_LIGNEDEP = v36;
				NV_LIGNEOFS = v35;
				NV_POSI = v7;
				return true;
			}
		}
	}
	return false;
}

int LinesManager::GENIAL(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int16 *route, int a9) {
	int result = a7;
	int v80 = -1;
	++_vm->_globals.pathFindingDepth;
	if (_vm->_globals.pathFindingDepth > 10) {
		warning("PathFinding - Max depth reached");
		route[a7] = -1;
		route[a7 + 1] = -1;
		route[a7 + 2] = -1;
		return -1;
	}
	int16 *v10 = Ligne[a1]._lineData;
	int v98 = v10[0];
	int v97 = v10[1];
	int v92 = a1;

	int v65;
	for (;;) {
		int v86 = v92 - 1;
		int v11 = 2 * Ligne[v92 - 1]._lineDataEndIdx;

		int16 *v12 = Ligne[v92 - 1]._lineData;
		if (v12 == (int16 *)g_PTRNUL)
			break;
		while (v12[v11 - 2] != v98 || v97 != v12[v11 - 1]) {
			--v86;
			if (_vm->_objectsManager._lastLine - 1 != v86) {
				v11 = 2 * Ligne[v86]._lineDataEndIdx;
				v12 = Ligne[v86]._lineData;
				if (v12 != (int16 *)g_PTRNUL)
					continue;
			}
			goto LABEL_11;
		}

		v92 = v86;
		v98 = v12[0];
		v97 = v12[1];
	}
LABEL_11:
	int16 *v13 = Ligne[a1]._lineData;
	int v95 = v13[2 * Ligne[a1]._lineDataEndIdx - 2];
	int v93 = v13[2 * Ligne[a1]._lineDataEndIdx - 1];
	int v91 = a1;
	int v100, v101;
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
			goto LABEL_17;
		}
		v91 = v87;
		v95 = v17;
		v93 = v16[v65 - 1];
	}
LABEL_17:
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

	Common::fill(&_vm->_globals.BufLig[0], &_vm->_globals.BufLig[1000], 0);
	int bugLigIdx = 0;
	for (int v88 = 0; v88 < v85 + 1; v88++) {
		_vm->_globals.BufLig[bugLigIdx] = v82;
		_vm->_globals.BufLig[bugLigIdx + 1] = v81;
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
		if (checkCollisionLine(_vm->_globals.BufLig[bugLigIdx], _vm->_globals.BufLig[bugLigIdx + 1], &v101, &v100, v92, v91) && _vm->_objectsManager._lastLine < v100) {
			v80 = v100;
			v77 = v101;
			v78 = _vm->_globals.BufLig[bugLigIdx];
			v79 = _vm->_globals.BufLig[bugLigIdx + 1];
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
			if (!checkCollisionLine(a5, v34, &v101, &v100, v92, v91))
				break;

			v76 = v100;
			if (!v34 || v73 > v34)
				break;
		}
		int v35 = a6;
		int v75 = -1;
		for (;;) {
			++v35;
			if (!checkCollisionLine(a5, v35, &v101, &v100, v92, v91))
				break;

			v75 = v100;
			if (_vm->_globals.Max_Perso_Y <= v35 || v71 <= v35)
				break;
		}
		int v36 = a5;
		int v74 = -1;
		for (;;) {
			++v36;
			if (!checkCollisionLine(v36, a6, &v101, &v100, v92, v91))
				break;

			v74 = v100;

			if (_vm->_graphicsManager._maxX <= v36 || v67 <= v36)
				break;
		}
		int v37 = a5;
		int v38 = -1;
		for(;;) {
			--v37;
			if (!checkCollisionLine(v37, a6, &v101, &v100, v92, v91))
				break;
			v38 = v100;
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
		if (a1 < v80) {
			int v43 = 0;
			int v42 = a1;
			do {
				if (v42 == v92 - 1)
					v42 = v91;
				++v43;
				--v42;
				if (v42 == v92 - 1)
					v42 = v91;
			} while (v80 != v42);
			if (abs(v80 - a1) == v43) {
				if (a2 >  abs(Ligne[a1]._lineDataEndIdx / 2)) {
					result = CONTOURNE(a1, a2, a7, v80, v77, route, a9);
				} else {
					result = CONTOURNE1(a1, a2, a7, v80, v77, route, a9, v92, v91);
				}
			}
			if (abs(v80 - a1) < v43)
				result = CONTOURNE(a1, a2, result, v80, v77, route, a9);
			if (v43 < abs(v80 - a1))
				result = CONTOURNE1(a1, a2, result, v80, v77, route, a9, v92, v91);
		}
		if (a1 > v80) {
			int v45 = abs(a1 - v80);
			int v47 = a1;
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
				if (a2 > abs(Ligne[a1]._lineDataEndIdx / 2)) {
					result = CONTOURNE1(a1, a2, result, v80, v77, route, a9, v92, v91);
				} else {
					result = CONTOURNE(a1, a2, result, v80, v77, route, a9);
				}
			}
			if (v45 < v48)
				result = CONTOURNE(a1, a2, result, v80, v77, route, a9);
			if (v48 < v45)
				result = CONTOURNE1(a1, a2, result, v80, v77, route, a9, v92, v91);
		}
		if (a1 == v80)
			result = CONTOURNE(a1, a2, result, a1, v77, route, a9);
		for(;;) {
			if (!checkCollisionLine(NVPX, NVPY, &v101, &v100, _vm->_objectsManager._lastLine + 1, _linesNumb))
				break;

			if (Ligne[v100].field4 == 1)
				--NVPY;
			if (Ligne[v100].field4 == 2) {
				--NVPY;
				++NVPX;
			}
			if (Ligne[v100].field4 == 3)
				++NVPX;
			if (Ligne[v100].field4 == 4) {
				++NVPY;
				++NVPX;
			}
			if (Ligne[v100].field4 == 5)
				++NVPY;
			if (Ligne[v100].field4 == 6) {
				++NVPY;
				--NVPX;
			}
			if (Ligne[v100].field4 == 7)
				--NVPX;
			if (Ligne[v100].field4 == 8) {
				--NVPY;
				--NVPX;
			}
		}
	} else {
		NVPX = -1;
		NVPY = -1;
	}
	return result;
}

// Avoid 2
int16 *LinesManager::PARCOURS2(int srcX, int srcY, int destX, int destY) {
	int v124;
	int v125;
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
		if (abs(srcX - _vm->_globals.old_x1_65) <= 4 && abs(srcY - _vm->_globals.old_y1_66) <= 4 &&
		    abs(_vm->_globals.old_x2_67 - destX) <= 4 && abs(_vm->_globals.old_y2_68 - clipDestY) <= 4)
			return (int16 *)g_PTRNUL;

		if (abs(srcX - destX) <= 4 && abs(srcY - clipDestY) <= 4)
			return (int16 *)g_PTRNUL;

		if (_vm->_globals._oldZoneNum > 0 && _vm->_objectsManager._zoneNum > 0 && _vm->_globals._oldZoneNum == _vm->_objectsManager._zoneNum)
			return (int16 *)g_PTRNUL;
	}
	_vm->_globals.NOT_VERIF = false;
	_vm->_globals._oldZoneNum = _vm->_objectsManager._zoneNum;
	_vm->_globals.old_x1_65 = srcX;
	_vm->_globals.old_x2_67 = destX;
	_vm->_globals.old_y1_66 = srcY;
	_vm->_globals.old_y2_68 = clipDestY;
	_vm->_globals.pathFindingDepth = 0;
	int v112 = 0;
	if (destX <= 19)
		clipDestX = 20;
	if (clipDestY <= 19)
		clipDestY = 20;
	if (clipDestX > _vm->_graphicsManager._maxX - 10)
		clipDestX = _vm->_graphicsManager._maxX - 10;
	if (clipDestY > _vm->_globals.Max_Perso_Y)
		clipDestY = _vm->_globals.Max_Perso_Y;

	if (abs(srcX - clipDestX) <= 3 && abs(srcY - clipDestY) <= 3)
		return (int16 *)g_PTRNUL;

	for (int i = 0; i <= 8; ++i) {
		v141[i] = -1;
		v136[i] = 0;
		v131[i] = 1300;
		v126[i] = 1300;
	}

	if (PARC_PERS(srcX, srcY, clipDestX, clipDestY, -1, -1, 0) == 1)
		return &_vm->_globals.super_parcours[0];

	int v14 = 0;
	for (int tmpY = clipDestY; tmpY < _vm->_graphicsManager._maxY; tmpY++, v14++) { 
		if (checkCollisionLine(clipDestX, tmpY, &v136[5], &v141[5], 0, _vm->_objectsManager._lastLine) && v141[5] <= _vm->_objectsManager._lastLine)
			break;
		v136[5] = 0;
		v141[5] = -1;
	}
	v131[5] = v14;

	v14 = 0;
	for (int tmpY = clipDestY; tmpY > _vm->_graphicsManager._minY; tmpY--, v14++) {
		if (checkCollisionLine(clipDestX, tmpY, &v136[1], &v141[1], 0, _vm->_objectsManager._lastLine) && v141[1] <= _vm->_objectsManager._lastLine)
			break;
		v136[1] = 0;
		v141[1] = -1;
		if (v131[5] < v14 && v141[5] != -1)
			break;
	}
	v131[1] = v14;

	v14 = 0;
	for (int tmpX = clipDestX; tmpX < _vm->_graphicsManager._maxX; tmpX++) {
		if (checkCollisionLine(tmpX, clipDestY, &v136[3], &v141[3], 0, _vm->_objectsManager._lastLine) && v141[3] <= _vm->_objectsManager._lastLine)
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
		if (checkCollisionLine(tmpX, clipDestY, &v136[7], &v141[7], 0, _vm->_objectsManager._lastLine) && v141[7] <= _vm->_objectsManager._lastLine)
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

	if (v141[1] < 0 || _vm->_objectsManager._lastLine < v141[1])
		v141[1] = -1;
	if (v141[3] < 0 || _vm->_objectsManager._lastLine < v141[3])
		v141[3] = -1;
	if (v141[5] < 0 || _vm->_objectsManager._lastLine < v141[5])
		v141[5] = -1;
	if (v141[7] < 0 || _vm->_objectsManager._lastLine < v141[7])
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
	for (int tmpY = srcY; tmpY < _vm->_graphicsManager._maxY; tmpY++, v14++) {
		if (checkCollisionLine(srcX, tmpY, &v136[5], &v141[5], 0, _vm->_objectsManager._lastLine) && v141[5] <= _vm->_objectsManager._lastLine)
			break;
		v136[5] = 0;
		v141[5] = -1;
	}
	v131[5] = v14 + 1;

	v14 = 0;
	for (int tmpY = srcY; tmpY > _vm->_graphicsManager._minY; tmpY--) {
		if (checkCollisionLine(srcX, tmpY, &v136[1], &v141[1], 0, _vm->_objectsManager._lastLine) && v141[1] <= _vm->_objectsManager._lastLine)
			break;
		v136[1] = 0;
		v141[1] = -1;
		++v14;
		if (v141[5] != -1 && v14 > 80)
			break;
	}
	v131[1] = v14 + 1;

	v14 = 0;
	for (int tmpX = srcX; tmpX < _vm->_graphicsManager._maxX; tmpX++) {
		if (checkCollisionLine(tmpX, srcY, &v136[3], &v141[3], 0, _vm->_objectsManager._lastLine) && v141[3] <= _vm->_objectsManager._lastLine)
			break;
		v136[3] = 0;
		v141[3] = -1;
		++v14;
		if ((v141[5] != -1 || v141[1] != -1) && (v14 > 100))
			break;
	}
	v131[3] = v14 + 1;

	v14 = 0;
	for (int tmpX = srcX; tmpX > _vm->_graphicsManager._minX; tmpX--) {
		if (checkCollisionLine(tmpX, srcY, &v136[7], &v141[7], 0, _vm->_objectsManager._lastLine) && v141[7] <= _vm->_objectsManager._lastLine)
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

	int v55 = PARC_PERS(srcX, srcY, clipDestX, clipDestY, v115, v121, 0);
	if (v55 != 1) {
		if (v55 == 2) {
LABEL_201:
			v115 = NV_LIGNEDEP;
			v114 = NV_LIGNEOFS;
			v112 = NV_POSI;
		} else {
			if (v113 == 1) {
				for (int deltaY = 0; deltaY < v111; deltaY++) {
					if (checkCollisionLine(srcX, srcY - deltaY, &v125, &v124, _vm->_objectsManager._lastLine + 1, _linesNumb)
					        && _vm->_objectsManager._lastLine < v124) {
						int v58 = GENIAL(v124, v125, srcX, srcY - deltaY, srcX, srcY - v111, v112, &_vm->_globals.super_parcours[0], 4);
						if (v58 == -1) {
							_vm->_globals.super_parcours[v112] = -1;
							_vm->_globals.super_parcours[v112 + 1] = -1;
							_vm->_globals.super_parcours[v112 + 2] = -1;
							_vm->_globals.super_parcours[v112 + 3] = -1;
							return &_vm->_globals.super_parcours[0];
						}
						v112 = v58;
						if (NVPY != -1)
							deltaY = srcY - NVPY;
					}
					_vm->_globals.super_parcours[v112] = srcX;
					_vm->_globals.super_parcours[v112 + 1] = srcY - deltaY;
					_vm->_globals.super_parcours[v112 + 2] = 1;
					_vm->_globals.super_parcours[v112 + 3] = 0;
					v112 += 4;
				}
			}
			if (v113 == 5) {
				for (int deltaY = 0; deltaY < v111; deltaY++) {
					if (checkCollisionLine(srcX, deltaY + srcY, &v125, &v124, _vm->_objectsManager._lastLine + 1, _linesNumb)
					        && _vm->_objectsManager._lastLine < v124) {
						int v61 = GENIAL(v124, v125, srcX, deltaY + srcY, srcX, v111 + srcY, v112, &_vm->_globals.super_parcours[0], 4);
						if (v61 == -1) {
							_vm->_globals.super_parcours[v112] = -1;
							_vm->_globals.super_parcours[v112 + 1] = -1;
							_vm->_globals.super_parcours[v112 + 2] = -1;
							_vm->_globals.super_parcours[v112 + 3] = -1;
							return &_vm->_globals.super_parcours[0];
						}
						v112 = v61;
						if (NVPY != -1)
							deltaY = NVPY - srcY;
					}
					_vm->_globals.super_parcours[v112] = srcX;
					_vm->_globals.super_parcours[v112 + 1] = deltaY + srcY;
					_vm->_globals.super_parcours[v112 + 2] = 5;
					_vm->_globals.super_parcours[v112 + 3] = 0;
					v112 += 4;
				}
			}
			if (v113 == 7) {
				for (int deltaX = 0; deltaX < v111; deltaX++) {
					if (checkCollisionLine(srcX - deltaX, srcY, &v125, &v124, _vm->_objectsManager._lastLine + 1, _linesNumb)
					        && _vm->_objectsManager._lastLine < v124) {
						int v64 = GENIAL(v124, v125, srcX - deltaX, srcY, srcX - v111, srcY, v112, &_vm->_globals.super_parcours[0], 4);
						if (v64 == -1) {
							_vm->_globals.super_parcours[v112] = -1;
							_vm->_globals.super_parcours[v112 + 1] = -1;
							_vm->_globals.super_parcours[v112 + 2] = -1;
							_vm->_globals.super_parcours[v112 + 3] = -1;
							return &_vm->_globals.super_parcours[0];
						}
						v112 = v64;
						if (NVPX != -1)
							deltaX = srcX - NVPX;
					}
					_vm->_globals.super_parcours[v112] = srcX - deltaX;
					_vm->_globals.super_parcours[v112 + 1] = srcY;
					_vm->_globals.super_parcours[v112 + 2] = 7;
					_vm->_globals.super_parcours[v112 + 3] = 0;
					v112 += 4;
				}
			}
			if (v113 == 3) {
				for (int deltaX = 0; deltaX < v111; deltaX++) {
					if (checkCollisionLine(deltaX + srcX, srcY, &v125, &v124, _vm->_objectsManager._lastLine + 1, _linesNumb)
					        && _vm->_objectsManager._lastLine < v124) {
						int v67 = GENIAL(v124, v125, deltaX + srcX, srcY, v111 + srcX, srcY, v112, &_vm->_globals.super_parcours[0], 4);
						if (v67 == -1) {
							_vm->_globals.super_parcours[v112] = -1;
							_vm->_globals.super_parcours[v112 + 1] = -1;
							_vm->_globals.super_parcours[v112 + 2] = -1;
							_vm->_globals.super_parcours[v112 + 3] = -1;
							return &_vm->_globals.super_parcours[0];
						}
						v112 = v67;
						if (NVPX != -1)
							deltaX = NVPX - srcX;
					}
					_vm->_globals.super_parcours[v112] = deltaX + srcX;
					_vm->_globals.super_parcours[v112 + 1] = srcY;
					_vm->_globals.super_parcours[v112 + 2] = 3;
					_vm->_globals.super_parcours[v112 + 3] = 0;
					v112 += 4;
				}
			}
		}
LABEL_234:
		if (v115 < v121) {
			for (int i = v114; Ligne[v115]._lineDataEndIdx > i; ++i) {
				v119 = Ligne[v115]._lineData[2 * i];
				v118 = Ligne[v115]._lineData[2 * i + 1];
				_vm->_globals.super_parcours[v112] = Ligne[v115]._lineData[2 * i];
				_vm->_globals.super_parcours[v112 + 1] = Ligne[v115]._lineData[2 * i + 1];
				_vm->_globals.super_parcours[v112 + 2] = Ligne[v115].field6;
				_vm->_globals.super_parcours[v112 + 3] = 0;
				v112 += 4;
			}
			for (int v116 = v115 + 1; v116 < v121; v116++) {
				int v72 = 0;
				int v110 = v116;
				for (int j = v116; Ligne[j]._lineDataEndIdx > v72; j = v116) {
					v119 = Ligne[v110]._lineData[2 * v72];
					v118 = Ligne[v110]._lineData[2 * v72 + 1];
					_vm->_globals.super_parcours[v112] = Ligne[v110]._lineData[2 * v72];
					_vm->_globals.super_parcours[v112 + 1] = Ligne[v110]._lineData[2 * v72 + 1];
					_vm->_globals.super_parcours[v112 + 2] = Ligne[v110].field6;
					_vm->_globals.super_parcours[v112 + 3] = 0;
					v112 += 4;
					if (Ligne[v110]._lineDataEndIdx > 30 && v72 == Ligne[v110]._lineDataEndIdx / 2) {
						int v78 = PARC_PERS(Ligne[v110]._lineData[2 * v72], Ligne[v110]._lineData[2 * v72 + 1], clipDestX, clipDestY, v110, v121, v112);
						if (v78 == 1)
							return &_vm->_globals.super_parcours[0];
						if (v78 == 2) {
							v115 = NV_LIGNEDEP;
							v114 = NV_LIGNEOFS;
							v112 = NV_POSI;
							goto LABEL_234;
						}
						if (MIRACLE(v119, v118, v110, v121, v112))
							goto LABEL_201;
					}
					++v72;
					v110 = v116;
				}
				int v79 = PARC_PERS(v119, v118, clipDestX, clipDestY, v116, v121, v112);
				if (v79 == 1)
					return &_vm->_globals.super_parcours[0];
				if (v79 == 2) {
					v115 = NV_LIGNEDEP;
					v114 = NV_LIGNEOFS;
					v112 = NV_POSI;
					goto LABEL_234;
				}
				if (MIRACLE(v119, v118, v116, v121, v112))
					goto LABEL_201;
			}
			v114 = 0;
			v115 = v121;
		}
		if (v115 > v121) {
			for (int dataIdx = v114; dataIdx > 0; dataIdx--) {
				v119 = Ligne[v115]._lineData[2 * dataIdx];
				v118 = Ligne[v115]._lineData[2 * dataIdx + 1];

				_vm->_globals.super_parcours[v112] = Ligne[v115]._lineData[2 * dataIdx];
				_vm->_globals.super_parcours[v112 + 1] = Ligne[v115]._lineData[2 * dataIdx + 1];
				_vm->_globals.super_parcours[v112 + 2] = Ligne[v115].field8;
				_vm->_globals.super_parcours[v112 + 3] = 0;
				v112 += 4;
			}
			for (int v117 = v115 - 1; v117 > v121; v117--) {
				for (int dataIdx = Ligne[v117]._lineDataEndIdx - 1; dataIdx > -1; dataIdx--) {
					v119 = Ligne[v117]._lineData[2 * dataIdx];
					v118 = Ligne[v117]._lineData[2 * dataIdx + 1];
					_vm->_globals.super_parcours[v112] = Ligne[v117]._lineData[2 * dataIdx];
					_vm->_globals.super_parcours[v112 + 1] = Ligne[v117]._lineData[2 * dataIdx + 1];
					_vm->_globals.super_parcours[v112 + 2] = Ligne[v117].field8;
					_vm->_globals.super_parcours[v112 + 3] = 0;
					v112 += 4;
					if (Ligne[v117]._lineDataEndIdx > 30 && dataIdx == Ligne[v117]._lineDataEndIdx / 2) {
						int v88 = PARC_PERS(v119, v118, clipDestX, clipDestY, v117, v121, v112);
						if (v88 == 1)
							return &_vm->_globals.super_parcours[0];
						if (v88 == 2) {
							v115 = NV_LIGNEDEP;
							v114 = NV_LIGNEOFS;
							v112 = NV_POSI;
							goto LABEL_234; 
						}
						if (MIRACLE(v119, v118, v117, v121, v112))
							goto LABEL_201;
					}
				}
				int v89 = PARC_PERS(v119, v118, clipDestX, clipDestY, v117, v121, v112);
				if (v89 == 1)
					return &_vm->_globals.super_parcours[0];
				if (v89 == 2) {
					v115 = NV_LIGNEDEP;
					v114 = NV_LIGNEOFS;
					v112 = NV_POSI;
					goto LABEL_234;
				}
				if (MIRACLE(v119, v118, v117, v121, v112))
					goto LABEL_201;
			}
			v114 = Ligne[v121]._lineDataEndIdx - 1;
			v115 = v121;
		}
		if (v115 == v121) {
			if (v114 <= v120) {
				for (int dataIdx = v114; dataIdx < v120; dataIdx++) {
					_vm->_globals.super_parcours[v112] = Ligne[v121]._lineData[2 * dataIdx];
					_vm->_globals.super_parcours[v112 + 1] = Ligne[v121]._lineData[2 * dataIdx + 1];
					_vm->_globals.super_parcours[v112 + 2] = Ligne[v121].field6;
					_vm->_globals.super_parcours[v112 + 3] = 0;
					v112 += 4;
				}
			} else {
				for (int dataIdx = v114; dataIdx > v120; dataIdx--) {
					_vm->_globals.super_parcours[v112] = Ligne[v121]._lineData[2 * dataIdx];
					_vm->_globals.super_parcours[v112 + 1] = Ligne[v121]._lineData[2 * dataIdx + 1];
					_vm->_globals.super_parcours[v112 + 2] = Ligne[v121].field8;
					_vm->_globals.super_parcours[v112 + 3] = 0;
					v112 += 4;
				}
			}
		}
		if (PARC_PERS(
		            _vm->_globals.super_parcours[v112 - 4],
		            _vm->_globals.super_parcours[v112 - 3],
		            clipDestX,
		            clipDestY,
		            -1,
		            -1,
		            v112) != 1) {
			_vm->_globals.super_parcours[v112] = -1;
			_vm->_globals.super_parcours[v112 + 1] = -1;
			_vm->_globals.super_parcours[v112 + 2] = -1;
			_vm->_globals.super_parcours[v112 + 3] = -1;
		}
		return &_vm->_globals.super_parcours[0];
	}

	return &_vm->_globals.super_parcours[0];
}

int LinesManager::PARC_PERS(int a1, int a2, int destX, int destY, int a5, int a6, int a7) {
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
	int v138;
	int v139;
	int v140;
	int v141;
	int v142;
	int v143;
	int v145;
	bool colResult = false;

	int v7 = a1;
	int v90 = a2;
	int v137 = a7;
	bool v136 = false;
	if (a5 == -1 && a6 == -1)
		v136 = true;
	int v144 = a5;
	if (checkCollisionLine(a1, a2, &v145, &v144, 0, _linesNumb)) {
		switch (Ligne[v144].field4) {
		case 1:
			v90 -= 2;
			break;
		case 2:
			v90 -= 2;
			v7 += 2;
			break;
		case 3:
			v7 += 2;
			break;
		case 4:
			v90 += 2;
			v7 += 2;
		case 5:
			v90 += 2;
			break;
		case 6:
			v90 += 2;
			v7 -= 2;
			break;
		case 7:
			v7 -= 2;
			break;
		case 8:
			v90 -= 2;
			v7 -= 2;
			break;
		}
	}
	v98 = v7;
	v97 = v90;
	v115 = 0;
	v142 = -1;
	v140 = -1;
	v138 = -1;

	int v9, v10, v11, v12, v13, v14;
	for (;;) {
		v111 = v7;
		v109 = v90;
		if (destX >= v7 - 2 && destX <= v7 + 2 && destY >= v90 - 2 && destY <= v90 + 2) {
LABEL_149:
			_vm->_globals.essai0[v115] = -1;
			_vm->_globals.essai0[v115 + 1] = -1;
			_vm->_globals.essai0[v115 + 2] = -1;

LABEL_150:
			if (v115) {
				v116 = 0;
				for (;;) {
					_vm->_globals.super_parcours[v137] = _vm->_globals.essai0[v116];
					_vm->_globals.super_parcours[v137 + 1] = _vm->_globals.essai0[v116 + 1];
					_vm->_globals.super_parcours[v137 + 2] = _vm->_globals.essai0[v116 + 2];
					_vm->_globals.super_parcours[v137 + 3] = 0;
					v116 += 3;
					v137 += 4;

					if (_vm->_globals.essai0[v116] == -1 && _vm->_globals.essai0[v116 + 1] == -1)
						break;
				}
			}
			_vm->_globals.super_parcours[v137] = -1;
			_vm->_globals.super_parcours[v137 + 1] = -1;
			_vm->_globals.super_parcours[v137 + 2] = -1;
			_vm->_globals.super_parcours[v137 + 3] = -1;
			return 1;
		}
		v9 = abs(v7 - destX);
		v10 = v9 + 1;
		v11 = abs(v90 - destY);
		v107 = v11 + 1;
		if (v10 > (int16)(v11 + 1))
			v107 = v10;
		v12 = v107 - 1;
		v101 = 1000 * v10 / v12;
		v99 = 1000 * (int16)(v11 + 1) / v12;
		if (destX < v7)
			v101 = -v101;
		if (destY < v90)
			v99 = -v99;
		v13 = (int16)v101 / 1000;
		v94 = (int16)v99 / 1000;
		newDirection = -1;
		if ((int16)v99 / 1000 == -1 && (unsigned int)v101 <= 150)
			newDirection = 1;
		if (v13 == 1) {
			if ((unsigned int)(v99 + 1) <= 151)
				newDirection = 3;
			if ((unsigned int)v99 <= 150)
				newDirection = 3;
		}
		if (v94 == 1) {
			if ((unsigned int)v101 <= 150)
				newDirection = 5;
			if ((unsigned int)(v101 + 150) <= 150)
				newDirection = 5;
		}
		if (v13 == -1) {
			if ((unsigned int)v99 <= 150)
				newDirection = 7;
			if ((unsigned int)(v99 + 150) <= 150)
				newDirection = 7;
		}
		if (v94 == -1 && (unsigned int)(v101 + 150) <= 150)
			newDirection = 1;
		if (newDirection == -1 && !checkSmoothMove(v7, v109, destX, destY) && !makeSmoothMove(v7, v109, destX, destY))
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
			if ((unsigned int)(v100 + 1) <= 511)
				newDirection = 2;
			if ((unsigned int)(v100 + 510) <= 510)
				newDirection = 3;
			if ((unsigned int)v100 <= 510)
				newDirection = 3;
			if ((unsigned int)(v100 - 510) <= 490)
				newDirection = 4;
		}
		if (v96 == 1) {
			if ((unsigned int)(v102 - 510) <= 490)
				newDirection = 4;
			if ((unsigned int)v102 <= 510)
				newDirection = 5;
			// CHECKME: The two conditions on v102 are not compatible!
			if (v102 >= -1 && v102 <= -510)
				newDirection = 6;
			if ((unsigned int)(v102 + 510) <= 510)
				newDirection = 5;
		}
		if (v22 == -1) {
			if ((unsigned int)(v100 - 510) <= 490)
				newDirection = 6;
			if ((unsigned int)v100 <= 510)
				newDirection = 7;
			if ((unsigned int)(v100 + 1000) <= 490)
				newDirection = 8;
			if ((unsigned int)(v100 + 510) <= 510)
				newDirection = 7;
		}
		if (v96 == -1) {
			if ((unsigned int)(v102 + 1000) <= 490)
				newDirection = 8;
			if ((unsigned int)(v102 + 510) <= 510)
				newDirection = 1;
		}
		v23 = 0;
		if (v108 + 1 <= 0)
			goto LABEL_149;
		while (!checkCollisionLine(v104, v103, &v143, &v142, 0, _linesNumb)) {
			_vm->_globals.essai0[v115] = v104;
			_vm->_globals.essai0[v115 + 1] = v103;
			_vm->_globals.essai0[v115 + 2] = newDirection;
			v106 += v102;
			v105 += v100;
			v104 = v106 / 1000;
			v103 = v105 / 1000;
			v115 += 3;
			++v23;
			if (v23 >= v108 + 1)
				goto LABEL_149;
		}
		if (_vm->_objectsManager._lastLine >= v142)
			goto LABEL_157;
		v24 = GENIAL(v142, v143, v104, v103, destX, destY, v115, _vm->_globals.essai0, 3);
		if (v24 == -1)
			goto LABEL_150;
		v115 = v24;
		if (NVPX != -1 || NVPY != -1) {
			v142 = -1;
			goto LABEL_157;
		}
		v7 = -1;
		v90 = -1;
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

		_vm->_globals.essai0[v115] = _smoothRoute[v14]._posX;
		_vm->_globals.essai0[v115 + 1] = _smoothRoute[v14]._posY;
		_vm->_globals.essai0[v115 + 2] = newDirection;
		v115 += 3;
		++v14;
		if (v126) {
			v18 = v14 - 1;
			v111 = _smoothRoute[v18]._posX;
			v109 = _smoothRoute[v18]._posY;
			goto LABEL_72;
		}
	}
	if (v142 > _vm->_objectsManager._lastLine)
		v142 = -1;

LABEL_157:
	_vm->_globals.essai0[v115] = -1;
	_vm->_globals.essai0[v115 + 1] = -1;
	_vm->_globals.essai0[v115 + 2] = -1;

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
				_vm->_globals.essai1[v117] = v36;
				_vm->_globals.essai1[v117 + 1] = v92;
				_vm->_globals.essai1[v117 + 2] = 7;
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
					if (_vm->_objectsManager._lastLine < v140) {
						int v44 = GENIAL(v140, v141, destX, v43, destX, destY, v117, _vm->_globals.essai1, 3);
						if (v44 == -1)
							goto LABEL_195;
						v117 = v44;
						if (NVPX != -1 && NVPY != -1) {
							v33 = NVPX;
							v92 = NVPY;
							v45 = checkCollisionLine(NVPX, NVPY, &v141, &v140, 0, _vm->_objectsManager._lastLine);
							if (v45 && v140 <= _vm->_objectsManager._lastLine)
								goto LABEL_202;
							goto LABEL_158;
						}
					}
					if (v140 <= _vm->_objectsManager._lastLine)
						goto LABEL_202;
				}
				_vm->_globals.essai1[v117] = destX;
				_vm->_globals.essai1[v117 + 1] = v43;
				_vm->_globals.essai1[v117 + 2] = 1;
				v117 += 3;
			}
LABEL_194:
			_vm->_globals.essai1[v117] = -1;
			_vm->_globals.essai1[v117 + 1] = -1;
			_vm->_globals.essai1[v117 + 2] = -1;
LABEL_195:
			if (v117) {
				v118 = 0;
				for (;;) {
					_vm->_globals.super_parcours[v137] = _vm->_globals.essai1[v118];
					_vm->_globals.super_parcours[v137 + 1] = _vm->_globals.essai1[v118 + 1];
					_vm->_globals.super_parcours[v137 + 2] = _vm->_globals.essai1[v118 + 2];
					_vm->_globals.super_parcours[v137 + 3] = 0;
					v118 += 3;
					v137 += 4;
					if (_vm->_globals.essai1[v118] == -1 && _vm->_globals.essai1[v118 + 1] == -1)
						break;
				}
			}
			_vm->_globals.super_parcours[v137] = -1;
			_vm->_globals.super_parcours[v137 + 1] = -1;
			_vm->_globals.super_parcours[v137 + 2] = -1;
			_vm->_globals.super_parcours[v137 + 3] = -1;
			return 1;
		}
		v39 = v92;
		for (;;) {
			if (checkCollisionLine(destX, v39, &v141, &v140, 0, _linesNumb)) {
				if (_vm->_objectsManager._lastLine < v140) {
					v40 = GENIAL(v140, v141, destX, v39, destX, destY, v117, _vm->_globals.essai1, 3);
					if (v40 == -1)
						goto LABEL_195;
					v117 = v40;
					if (NVPX != -1 && NVPY != -1) {
						v33 = NVPX;
						v92 = NVPY;
						v45 = checkCollisionLine(NVPX, NVPY, &v141, &v140, 0, _vm->_objectsManager._lastLine);
						if (v45 && v140 <= _vm->_objectsManager._lastLine)
							goto LABEL_202;
						goto LABEL_158;
					}
				}
				if (v140 <= _vm->_objectsManager._lastLine)
					goto LABEL_202;
			}

			_vm->_globals.essai1[v117] = destX;
			_vm->_globals.essai1[v117 + 1] = v39;
			_vm->_globals.essai1[v117 + 2] = 5;
			v117 += 3;
			++v39;
			if (destY <= v39)
				goto LABEL_181;
		}
	}
	while (!checkCollisionLine(v33, v92, &v141, &v140, 0, _linesNumb)) {
		_vm->_globals.essai1[v117] = v33;
		_vm->_globals.essai1[v117 + 1] = v92;
		_vm->_globals.essai1[v117 + 2] = 3;
		v117 += 3;
		++v33;
		if (destX <= v33)
			goto LABEL_165;
	}
LABEL_168:
	if (v140 > _vm->_objectsManager._lastLine)
		v140 = -1;
LABEL_202:
	_vm->_globals.essai1[v117] = -1;
	_vm->_globals.essai1[v117 + 1] = -1;
	_vm->_globals.essai1[v117 + 2] = -1;
	v117 = 0;
	v54 = v98;
	v93 = v97;
LABEL_203:
	v114 = v54;
	if (destX >= v54 - 2 && destX <= v54 + 2 && destY >= v93 - 2 && destY <= v93 + 2)
		goto LABEL_241;
	if (v93 < destY) {
		v55 = v93;
		while (!checkCollisionLine(v114, v55, &v139, &v138, 0, _linesNumb)) {
			_vm->_globals.essai2[v117] = v114;
			_vm->_globals.essai2[v117 + 1] = v55;
			_vm->_globals.essai2[v117 + 2] = 5;
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
		while (!checkCollisionLine(v114, v58, &v139, &v138, 0, _linesNumb)) {
			_vm->_globals.essai2[v117] = v114;
			_vm->_globals.essai2[v117 + 1] = v58;
			_vm->_globals.essai2[v117 + 2] = 1;
			v117 += 3;
			--v58;
			if (destY >= v58)
				goto LABEL_217;
		}
LABEL_214:
		if (v138 > _vm->_objectsManager._lastLine)
			v138 = -1;
LABEL_249:
		_vm->_globals.essai2[v117] = -1;
		_vm->_globals.essai2[v117 + 1] = -1;
		_vm->_globals.essai2[v117 + 2] = -1;

		if (!v136) {
			if (a6 > v144) {
				if (_vm->_globals.essai0[0] != -1 && v142 > v144 && v140 <= v142 && v138 <= v142 && a6 >= v142) {
					NV_LIGNEDEP = v142;
					NV_LIGNEOFS = v143;
					v120 = 0;
					for (;;) {
						_vm->_globals.super_parcours[v137] = _vm->_globals.essai0[v120];
						_vm->_globals.super_parcours[v137 + 1] = _vm->_globals.essai0[v120 + 1];
						_vm->_globals.super_parcours[v137 + 2] = _vm->_globals.essai0[v120 + 2];
						_vm->_globals.super_parcours[v137 + 3] = 0;
						v120 += 3;
						v137 += 4;
						if (_vm->_globals.essai0[v120] == -1 && _vm->_globals.essai0[v120 + 1] == -1)
							break;
					}
					NV_POSI = v137;
					return 2;
				}
				if (_vm->_globals.essai1[0] != -1 && v144 < v140 && v138 <= v140 && v142 <= v140 && a6 >= v140) {
					NV_LIGNEDEP = v140;
					NV_LIGNEOFS = v141;
					v121 = 0;
					for (;;) {
						assert(v137 <= 32000);
						_vm->_globals.super_parcours[v137] = _vm->_globals.essai1[v121];
						_vm->_globals.super_parcours[v137 + 1] = _vm->_globals.essai1[v121 + 1];
						_vm->_globals.super_parcours[v137 + 2] = _vm->_globals.essai1[v121 + 2];
						_vm->_globals.super_parcours[v137 + 3] = 0;
						v121 += 3;
						v137 += 4;
						if (_vm->_globals.essai1[v121] == -1 && _vm->_globals.essai1[v121 + 1] == -1)
							break;
					}
					NV_POSI = v137;
					return 2;
				}
				if (_vm->_globals.essai2[0] != -1) {
					if (v144 < v138 && v140 < v138 && v142 < v138 && a6 >= v138) {
						NV_LIGNEDEP = v138;
						NV_LIGNEOFS = v139;
						v122 = 0;
						for (;;) {
							assert(v137 <= 32000);
							_vm->_globals.super_parcours[v137] = _vm->_globals.essai2[v122];
							_vm->_globals.super_parcours[v137 + 1] = _vm->_globals.essai2[v122 + 1];
							_vm->_globals.super_parcours[v137 + 2] = _vm->_globals.essai2[v122 + 2];
							_vm->_globals.super_parcours[v137 + 3] = 0;
							v122 += 3;
							v137 += 4;
							if (_vm->_globals.essai2[v122] == -1 && _vm->_globals.essai2[v122 + 1] == -1)
								break;
						};
						NV_POSI = v137;
						return 2;
					}
				}
			}
			if (a6 < v144) {
				if (v142 == -1)
					v142 = 1300;
				if (v140 == -1)
					v142 = 1300;
				if (v138 == -1)
					v142 = 1300;
				if (_vm->_globals.essai1[0] != -1 && v140 < v144 && v138 >= v140 && v142 >= v140 && a6 <= v140) {
					NV_LIGNEDEP = v140;
					NV_LIGNEOFS = v141;
					v123 = 0;
					for (;;) {
						assert(137 <= 32000);
						_vm->_globals.super_parcours[v137] = _vm->_globals.essai1[v123];
						_vm->_globals.super_parcours[v137 + 1] = _vm->_globals.essai1[v123 + 1];
						_vm->_globals.super_parcours[v137 + 2] = _vm->_globals.essai1[v123 + 2];
						_vm->_globals.super_parcours[v137 + 3] = 0;
						v123 += 3;
						v137 += 4;
						if (_vm->_globals.essai1[v123] == -1 && _vm->_globals.essai1[v123 + 1] == -1)
							break;
					}
					NV_POSI = v137;
					return 2;
				}
				if (_vm->_globals.essai2[0] != -1 && v144 > v138 && v140 >= v138 && v142 >= v138 && a6 <= v138) {
					NV_LIGNEDEP = v138;
					NV_LIGNEOFS = v139;
					v124 = 0;
					for (;;) {
						assert(v137 <= 32000);
						_vm->_globals.super_parcours[v137] = _vm->_globals.essai2[v124];
						_vm->_globals.super_parcours[v137 + 1] = _vm->_globals.essai2[v124 + 1];
						_vm->_globals.super_parcours[v137 + 2] = _vm->_globals.essai2[v124 + 2];
						_vm->_globals.super_parcours[v137 + 3] = 0;
						v124 += 3;
						v137 += 4;
						if (_vm->_globals.essai2[v124] == -1 && _vm->_globals.essai2[v124 + 1] == -1)
							break;
					}
					NV_POSI = v137;
					return 2;
				}
				if (_vm->_globals.essai1[0] != -1 && v144 > v142 && v140 >= v142 && v138 >= v142 && a6 <= v142) {
					NV_LIGNEDEP = v142;
					NV_LIGNEOFS = v143;
					v125 = 0;
					for (;;) {
						assert(137 <= 32000);
						_vm->_globals.super_parcours[v137] = _vm->_globals.essai0[v125];
						_vm->_globals.super_parcours[v137 + 1] = _vm->_globals.essai0[v125 + 1];
						_vm->_globals.super_parcours[v137 + 2] = _vm->_globals.essai0[v125 + 2];
						_vm->_globals.super_parcours[v137 + 3] = 0;
						v125 += 3;
						v137 += 4;
						if (_vm->_globals.essai0[v125] == -1 && _vm->_globals.essai0[v125 + 1] == -1)
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
			if (checkCollisionLine(v61, destY, &v139, &v138, 0, _linesNumb)) {
				if (_vm->_objectsManager._lastLine < v138) {
					int v62 = GENIAL(v138, v139, v61, destY, destX, destY, v117, _vm->_globals.essai2, 3);
					if (v62 == -1)
						goto LABEL_195;
					v117 = v62;
					if (NVPX != -1) {
						if (NVPY != -1) {
							v54 = NVPX;
							v93 = NVPY;
							colResult = checkCollisionLine(NVPX, NVPY, &v139, &v138, 0, _vm->_objectsManager._lastLine);
							if (colResult && v138 <= _vm->_objectsManager._lastLine)
								goto LABEL_249;
							goto LABEL_203;
						}
					}
				}
				if (v138 <= _vm->_objectsManager._lastLine)
					goto LABEL_249;
			}

			_vm->_globals.essai2[v117] = v61;
			_vm->_globals.essai2[v117 + 1] = destY;
			_vm->_globals.essai2[v117 + 2] = 3;
			v117 += 3;
		}
	}
	if (v114 > destX) {
		for (int v65 = v114; v65 > destX; v65--) {
			if (checkCollisionLine(v65, destY, &v139, &v138, 0, _linesNumb)) {
				if (_vm->_objectsManager._lastLine < v138) {
					v66 = GENIAL(v138, v139, v65, destY, destX, destY, v117, _vm->_globals.essai2, 3);
					if (v66 == -1)
						goto LABEL_242;
					v117 = v66;
					if (NVPX != -1 && NVPY != -1) {
						v54 = NVPX;
						v93 = NVPY;
						colResult = checkCollisionLine(NVPX, NVPY, &v139, &v138, 0, _vm->_objectsManager._lastLine);
						if (colResult && v138 <= _vm->_objectsManager._lastLine)
							goto LABEL_249;
						goto LABEL_203;
					}
				}
				if (v138 <= _vm->_objectsManager._lastLine)
					goto LABEL_249;
			}
			_vm->_globals.essai2[v117] = v65;
			_vm->_globals.essai2[v117 + 1] = destY;
			_vm->_globals.essai2[v117 + 2] = 7;
			v117 += 3;
		}
	}
	v138 = -1;
LABEL_241:
	_vm->_globals.essai2[v117] = -1;
	_vm->_globals.essai2[v117 + 1] = -1;
	_vm->_globals.essai2[v117 + 2] = -1;
LABEL_242:
	if (v117) {
		v119 = 0;
		for (;;) {
			_vm->_globals.super_parcours[v137] = _vm->_globals.essai2[v119];
			_vm->_globals.super_parcours[v137 + 1] = _vm->_globals.essai2[v119 + 1];
			_vm->_globals.super_parcours[v137 + 2] = _vm->_globals.essai2[v119 + 2];
			_vm->_globals.super_parcours[v137 + 3] = 0;
			v119 += 3;
			v137 += 4;
			if (_vm->_globals.essai2[v119] == -1 && _vm->_globals.essai2[v119 + 1] == -1)
				break;
		}
	}
	_vm->_globals.super_parcours[v137] = -1;
	_vm->_globals.super_parcours[v137 + 1] = -1;
	_vm->_globals.super_parcours[v137 + 2] = -1;
	_vm->_globals.super_parcours[v137 + 3] = -1;
	return 1;
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
		while (!checkCollisionLine(newPosX, newPosY, &foundDataIdx, &foundLineIdx, 0, _linesNumb) || foundLineIdx > _vm->_objectsManager._lastLine) {
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
			_vm->_linesManager._smoothMoveDirection = 6;
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
			_vm->_linesManager._smoothMoveDirection = 4;
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
			_vm->_linesManager._smoothMoveDirection = 8;
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
			_vm->_linesManager._smoothMoveDirection = 2;
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
		_vm->_globals.essai0 = _vm->_globals.essai0;
		for (int i = 0; i < Ligne[idxTest]._lineDataEndIdx; i++) {
			_vm->_globals.super_parcours[superRouteIdx] = Ligne[idxTest]._lineData[2 * i];
			_vm->_globals.super_parcours[superRouteIdx + 1] = Ligne[idxTest]._lineData[2 * i + 1];
			_vm->_globals.super_parcours[superRouteIdx + 2] = Ligne[idxTest].field6;
			_vm->_globals.super_parcours[superRouteIdx + 3] = 0;
			superRouteIdx += 4;
		}
	} else if (v33 == 2) {
		for (int v19 = Ligne[idxTest]._lineDataEndIdx - 1; v19 > -1; v19--) {
			_vm->_globals.super_parcours[superRouteIdx] = Ligne[idxTest]._lineData[2 * v19];
			_vm->_globals.super_parcours[superRouteIdx + 1] = Ligne[idxTest]._lineData[2 * v19 + 1];
			_vm->_globals.super_parcours[superRouteIdx + 2] = Ligne[idxTest].field8;
			_vm->_globals.super_parcours[superRouteIdx + 3] = 0;
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

	for (int idx = _vm->_objectsManager._lastLine + 1; idx < _vm->_linesManager._linesNumb + 1; idx++) {
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
			if (!checkCollisionLine(posX, posY, &collDataIdx, &collLineIdx, 0, _vm->_objectsManager._lastLine))
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
			if (!checkCollisionLine(posX, posY, &collDataIdx, &collLineIdx, 0, _vm->_objectsManager._lastLine))
				error("Error in test line");
			*foundLineIdx = collLineIdx;
			*foundDataIdx = collDataIdx;
			return idx;
		}
	}
	return -1;
}

} // End of namespace Hopkins
