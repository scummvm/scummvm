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
		Common::fill((byte *)&SMOOTH[i], (byte *)&SMOOTH[i] + sizeof(SmoothItem), 0);
	}

	_linesNumb = 0;
	NV_LIGNEDEP = 0;
	NV_LIGNEOFS = 0;
	NV_POSI = 0;
	NVPX = 0;
	NVPY = 0;
	SMOOTH_SENS = 0;
	SMOOTH_X = SMOOTH_Y = 0;
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

	_vm->_linesManager._zoneLine[idx].zoneData = (int16 *)_vm->_globals.freeMemory((byte *)_vm->_linesManager._zoneLine[idx].zoneData);
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

		_zoneLine[idx].zoneData = (int16 *)_vm->_globals.freeMemory((byte *)_zoneLine[idx].zoneData);

		int v8 = abs(a2 - a4);
		int v9 = abs(a3 - a5);
		int v20 = 1;
		if (v8 <= v9)
			v20 += v9;
		else
			v20 += v8;

		zoneData = (int16 *)_vm->_globals.allocMemory(2 * sizeof(int16) * v20 + (4 * sizeof(int16)));
		int v11 = idx;
		_zoneLine[v11].zoneData = zoneData;
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

		_zoneLine[idx].count = v20;
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
		if (_vm->_graphicsManager.max_x == curLineX || _vm->_graphicsManager.max_y == curLineY || 
			_vm->_graphicsManager.min_x == curLineX || _vm->_graphicsManager.min_y == curLineY ||
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
int LinesManager::CONTOURNE(int a1, int a2, int a3, int a4, int a5, int16 *a6, int a7) {
	int v36 = a1;
	int v7 = a2;
	int v8 = a3;
	int v50;
	if (a1 < a4) {
		for (int i = a2; Ligne[a1]._lineDataEndIdx > i; ++i) {
			int16 *v10 = Ligne[a1]._lineData;
			int v11 = v10[2 * i];
			v50 = v10[2 * i + 1];

			int v12 = v8;
			a6[v12] = v11;
			a6[v12 + 1] = v50;
			a6[v12 + 2] = Ligne[a1].field6;
			v8 += a7;
		}

		for (int v34 = a1 + 1; v34 < a4; v34++) {
			for (int i = 0; i < Ligne[v34]._lineDataEndIdx; i++) {
				int16 *v14 = Ligne[v34]._lineData;
				int v15 = v14[2 * i];
				v50 = v14[2 * i + 1];
				int v16 = v8;

				a6[v16] = v15;
				a6[v16 + 1] = v50;
				a6[v16 + 2] = Ligne[v34].field6;
				v8 += a7;
			}
		}

		v7 = 0;
		v36 = a4;
	}
	if (v36 > a4) {
		for (int j = v7; j > 0; --j) {
			int16 *v18 = Ligne[v36]._lineData;
			int v19 = v18[2 * j];
			v50 = v18[2 * j + 1];

			int v20 = v8;
			a6[v20] = v19;
			a6[v20 + 1] = v50;
			a6[v20 + 2] = Ligne[v36].field8;
			v8 += a7;
		}
		if ((int)(v36 - 1) > a4) {
			for (int v35 = v36 - 1; v35 > a4; v35--) {
				for (int k = Ligne[v35]._lineDataEndIdx - 1; k > 0; --k) {
					int16 *v22 = Ligne[v35]._lineData;
					int v23 = v22[2 * k];
					v50 = v22[2 * k + 1];

					int v24 = v8;
					a6[v24] = v23;
					a6[v24 + 1] = v50;
					a6[v24 + 2] = Ligne[v35].field8;
					v8 += a7;
				}

			}
		}
		v7 = Ligne[a4]._lineDataEndIdx - 1;
		v36 = a4;
	}
	if (v36 == a4) {
		if (a5 >= v7) {
			for (int i = v7; i > a5; i++) {
				int16 *v30 = Ligne[a4]._lineData;
				int v31 = v30[2 * i];
				v50 = v30[2 * i + 1];

				int v32 = v8;
				a6[v32] = v31;
				a6[v32 + 1] = v50;
				a6[v32 + 2] = Ligne[a4].field6;
				v8 += a7;
			}
		} else {
			for (int i = v7; i > a5; --i) {
				int16 *v26 = Ligne[a4]._lineData;
				int v27 = v26[2 * i];
				v50 = v26[2 * i+ 1];

				int v28 = v8;
				a6[v28] = v27;
				a6[v28 + 1] = v50;
				a6[v28 + 2] = Ligne[a4].field8;
				v8 += a7;
			}
		}
	}
	return v8;
}

// Avoid 1
int LinesManager::CONTOURNE1(int a1, int a2, int a3, int a4, int a5, int16 *a6, int a7, int a8, int a9) {
	int v9 = a1;
	int v10 = a2;
	int v40 = a3;
	int v50;
	if (a4 < a1) {
		for (int i = a2; Ligne[a1]._lineDataEndIdx > i; ++i) {
			int16 *v12 = Ligne[a1]._lineData;
			int v13 = v12[2 * i];
			v50 = v12[2 * i + 1];

			int v14 = v40;
			a6[v14] = v13;
			a6[v14 + 1] = v50;
			a6[v14 + 2] = Ligne[a1].field6;
			v40 += a7;
		}
		int v15 = a1 + 1;
		if (a1 + 1 == a9 + 1)
			v15 = a8;
		while (a4 != v15) {
			for (int v16 = 0; v16 < Ligne[v15]._lineDataEndIdx; v16++) {
				int16 *v17 = Ligne[v15]._lineData;
				int v18 = v17[2 * v16];
				v50 = v17[2 * v16 + 1];

				int v19 = v40;
				a6[v19] = v18;
				a6[v19 + 1] = v50;
				a6[v19 + 2] = Ligne[v15].field6;
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
		for (int j = v10; j > 0; --j) {
			int16 *v21 = Ligne[v9]._lineData;
			int v22 = v21[2 * j];
			v50 = v21[2 * j + 1];

			int v23 = v40;
			a6[v23] = v22;
			a6[v23 + 1] = v50;
			a6[v23 + 2] = Ligne[v9].field8;
			v40 += a7;
		}
		int v24 = v9 - 1;
		if (v24 == a8 - 1)
			v24 = a9;
		while (a4 != v24) {
			for (int k = Ligne[v24]._lineDataEndIdx - 1; k > 0; --k) {
				int16 *v26 = Ligne[v24]._lineData;
				int v27 = v26[2 * k];
				v50 = v26[2 * k + 1];

				int v28 = v40;
				a6[v28] = v27;
				a6[v28 + 1] = v50;
				a6[v28 + 2] = Ligne[v24].field8;
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
			if (a5 > v10) {
				int v39 = a4;
				for (int v33 = v10; v33 < a5; v33++) {
					int16 *v34 = Ligne[v39]._lineData;
					int v35 = v34[2 * v33];
					v50 = v34[2 * v33 + 1];

					int v36 = v40;
					a6[v36] = v35;
					a6[v36 + 1] = v50;
					a6[v36 + 2] = Ligne[v39].field6;
					v40 += a7;
				}
			}
		} else {
			int v38 = a4;
			for (int v29 = v10; v29 > a5; v29--) {
				int16 *v30 = Ligne[v38]._lineData;
				int v31 = v30[2 * v29];
				v50 = v30[2 * v29 + 1];

				int v32 = v40;
				a6[v32] = v31;
				a6[v32 + 1] = v50;
				a6[v32 + 2] = Ligne[v38].field8;
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
	int v24;
	int v26;
	int v27;
	int v29;
	int v30;
	int v32;
	int v33;
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
					v24 = v7;
					_vm->_globals.super_parcours[v24] = v41;
					_vm->_globals.super_parcours[v24 + 1] = v40 - v22;
					_vm->_globals.super_parcours[v24 + 2] = 1;
					_vm->_globals.super_parcours[v24 + 3] = 0;
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
					v27 = v7;
					_vm->_globals.super_parcours[v27] = v41;
					_vm->_globals.super_parcours[v27 + 1] = v25 + v40;
					_vm->_globals.super_parcours[v27 + 2] = 5;
					_vm->_globals.super_parcours[v27 + 3] = 0;
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
					v30 = v7;
					_vm->_globals.super_parcours[v30] = v41 - v28;
					_vm->_globals.super_parcours[v30 + 1] = v40;
					_vm->_globals.super_parcours[v30 + 2] = 7;
					_vm->_globals.super_parcours[v30 + 3] = 0;
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
					v33 = v7;
					_vm->_globals.super_parcours[v33] = v31 + v41;
					_vm->_globals.super_parcours[v33 + 1] = v40;
					_vm->_globals.super_parcours[v33 + 2] = 3;
					_vm->_globals.super_parcours[v33 + 3] = 0;
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

int LinesManager::GENIAL(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int16 *a8, int a9) {
	int v99 = a7;
	int v80 = -1;
	++_vm->_globals.pathFindingDepth;
	if (_vm->_globals.pathFindingDepth > 10) {
		warning("PathFinding - Max depth reached");
		a7 = a7;
		a8[a7] = -1;
		a8[a7 + 1] = -1;
		a8[a7 + 2] = -1;
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
			if (v16[0] == v95) {
				if (v93 == v16[1])
					break;
			}
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

			if (_vm->_graphicsManager.max_x <= v36 || v67 <= v36)
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
			a7 = a7;
			a8[a7] = -1;
			a8[a7 + 1] = -1;
			a8[a7 + 2] = -1;
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
					v99 = CONTOURNE(a1, a2, a7, v80, v77, a8, a9);
				} else {
					v99 = CONTOURNE1(a1, a2, a7, v80, v77, a8, a9, v92, v91);
				}
			}
			if (abs(v80 - a1) < v43)
				v99 = CONTOURNE(a1, a2, v99, v80, v77, a8, a9);
			if (v43 < abs(v80 - a1))
				v99 = CONTOURNE1(a1, a2, v99, v80, v77, a8, a9, v92, v91);
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
					v99 = CONTOURNE1(a1, a2, v99, v80, v77, a8, a9, v92, v91);
				} else {
					v99 = CONTOURNE(a1, a2, v99, v80, v77, a8, a9);
				}
			}
			if (v45 < v48)
				v99 = CONTOURNE(a1, a2, v99, v80, v77, a8, a9);
			if (v48 < v45)
				v99 = CONTOURNE1(a1, a2, v99, v80, v77, a8, a9, v92, v91);
		}
		if (a1 == v80)
			v99 = CONTOURNE(a1, a2, v99, a1, v77, a8, a9);
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
	return v99;
}

// Avoid 2
int16 *LinesManager::PARCOURS2(int srcX, int srcY, int destX, int destY) {
	int v12;
	int v14;
	int v15;
	int v16;
	int v18;
	int v19;
	int v20;
	int v22;
	int v23;
	int v24;
	int v26;
	int v27;
	int v28;
	int v31;
	int v34;
	int v35;
	int v36;
	int v38;
	int v39;
	int v40;
	int v42;
	int v43;
	int v44;
	int v46;
	int v47;
	int v48;
	int v50;
	int v51;
	int v52;
	int v53;
	int v54;
	int v55;
	int v56;
	int v58;
	int v59;
	int v60;
	int v61;
	int v62;
	int v63;
	int v64;
	int v65;
	int v66;
	int v67;
	int v68;
	int i;
	int16 *v70;
	int v71;
	int v72;
	int j;
	int16 *v74;
	int v75;
	int v76;
	int v77;
	int v78;
	int v79;
	int v80;
	int16 *v81;
	int v82;
	int v83;
	int16 *v84;
	int v85;
	int v86;
	int v87;
	int v88;
	int v89;
	int v90;
	int16 *v91;
	int v92;
	int v93;
	int v94;
	int16 *v95;
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
	int v110;
	int v111;
	int v112;
	int v113;
	int v114;
	int v115;
	int v116;
	int v117;
	int v118 = 0;
	int v119 = 0;
	int v120;
	int v121;
	int clipDestY;
	int clipDestX;
	int v124;
	int v125;
	int v126[9];
	int v131[9];
	int v136[9];
	int v141[9];

	clipDestX = destX;
	clipDestY = destY;
	v121 = 0;
	v120 = 0;
	v115 = 0;
	v114 = 0;
	v113 = 0;
	v111 = 0;
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
	v112 = 0;
	if (destX <= 19)
		clipDestX = 20;
	if (clipDestY <= 19)
		clipDestY = 20;
	if (clipDestX > _vm->_graphicsManager.max_x - 10)
		clipDestX = _vm->_graphicsManager.max_x - 10;
	if (clipDestY > _vm->_globals.Max_Perso_Y)
		clipDestY = _vm->_globals.Max_Perso_Y;

	if (abs(srcX - clipDestX) <= 3 && abs(srcY - clipDestY) <= 3)
		return (int16 *)g_PTRNUL;

	for (v12 = 0; v12 <= 8; ++v12) {
		v141[v12] = -1;
		v136[v12] = 0;
		v131[v12] = 1300;
		v126[v12] = 1300;
		++v12;
	}

	if (PARC_PERS(srcX, srcY, clipDestX, clipDestY, -1, -1, 0) != 1) {
		v14 = 0;
		v15 = clipDestY;
		if (_vm->_graphicsManager.max_y > clipDestY) {
			v16 = 5;
			do {
				v101 = v16;
				v16 = v101;
				if (checkCollisionLine(clipDestX, v15, &v136[5], &v141[5], 0, _vm->_objectsManager._lastLine) && v141[v101] <= _vm->_objectsManager._lastLine)
					break;
				v136[v101] = 0;
				v141[v101] = -1;
				++v14;
				++v15;
			} while (_vm->_graphicsManager.max_y > v15);
		}
		v131[5] = v14;
		v18 = 0;
		v19 = clipDestY;
		if (_vm->_graphicsManager.min_y < clipDestY) {
			v20 = 1;
			do {
				v102 = v20;
				v20 = v102;
				if (checkCollisionLine(clipDestX, v19, &v136[1], &v141[1], 0, _vm->_objectsManager._lastLine) && v141[v102] <= _vm->_objectsManager._lastLine)
					break;
				v136[v102] = 0;
				v141[v102] = -1;
				if (v131[5] < v18) {
					if (v141[5] != -1)
						break;
				}
				++v18;
				--v19;
			} while (_vm->_graphicsManager.min_y < v19);
		}
		v131[1] = v18;
		v22 = 0;
		v23 = clipDestX;
		if (_vm->_graphicsManager.max_x > clipDestX) {
			v24 = 3;
			do {
				v103 = v24;
				v24 = v103;
				if (checkCollisionLine(v23, clipDestY, &v136[3], &v141[3], 0, _vm->_objectsManager._lastLine) && v141[v103] <= _vm->_objectsManager._lastLine)
					break;
				v136[v103] = 0;
				v141[v103] = -1;
				++v22;
				if (v131[1] < v22) {
					if (v141[1] != -1)
						break;
				}
				if (v131[5] < v22 && v141[5] != -1)
					break;
				++v23;
			} while (_vm->_graphicsManager.max_x > v23);
		}
		v131[3] = v22;
		v26 = 0;
		v27 = clipDestX;
		if (_vm->_graphicsManager.min_x < clipDestX) {
			v28 = 7;
			do {
				v104 = v28;
				v28 = v104;
				if (checkCollisionLine(v27, clipDestY, &v136[7], &v141[7], 0, _vm->_objectsManager._lastLine) && v141[v104] <= _vm->_objectsManager._lastLine)
					break;
				v136[v104] = 0;
				v141[v104] = -1;
				++v26;
				if (v131[1] < v26) {
					if (v141[1] != -1)
						break;
				}
				if (v131[5] < v26 && v141[5] != -1)
					break;
				if (v131[3] < v26 && v141[3] != -1)
					break;
				--v27;
			} while (_vm->_graphicsManager.min_x < v27);
		}
		v131[7] = v26;
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
		v31 = 0;
		if (v141[5] != -1 && v131[1] >= v131[5] && v131[3] >= v131[5] && v131[7] >= v131[5]) {
			v121 = v141[5];
			v120 = v136[5];
			v31 = 1;
		}
		if (v141[1] != -1 && !v31 && v131[5] >= v131[1] && v131[3] >= v131[1] && v131[7] >= v131[1]) {
			v121 = v141[1];
			v120 = v136[1];
			v31 = 1;
		}
		if (v141[3] != -1 && !v31 && v131[1] >= v131[3] && v131[5] >= v131[3] && v131[7] >= v131[3]) {
			v121 = v141[3];
			v120 = v136[3];
			v31 = 1;
		}
		if (v141[7] != -1 && !v31 && v131[5] >= v131[7] && v131[3] >= v131[7] && v131[1] >= v131[7]) {
			v121 = v141[7];
			v120 = v136[7];
		}

		for (v12 = 0; v12 <= 8; ++v12) {
			v141[v12] = -1;
			v136[v12] = 0;
			v131[v12] = 1300;
			v126[v12] = 1300;
		}

		v34 = 0;
		v35 = srcY;
		if (_vm->_graphicsManager.max_y > srcY) {
			v36 = 5;
			do {
				v105 = v36;
				v36 = v105;
				if (checkCollisionLine(srcX, v35, &v136[5], &v141[5], 0, _vm->_objectsManager._lastLine) && v141[v105] <= _vm->_objectsManager._lastLine)
					break;
				v136[v105] = 0;
				v141[v105] = -1;
				++v34;
				++v35;
			} while (_vm->_graphicsManager.max_y > v35);
		}
		v131[5] = v34 + 1;
		v38 = 0;
		v39 = srcY;
		if (_vm->_graphicsManager.min_y < srcY) {
			v40 = 1;
			do {
				v106 = v40;
				v40 = v106;
				if (checkCollisionLine(srcX, v39, &v136[1], &v141[1], 0, _vm->_objectsManager._lastLine) && v141[v106] <= _vm->_objectsManager._lastLine)
					break;
				v136[v106] = 0;
				v141[v106] = -1;
				++v38;
				if (v141[5] != -1) {
					if (v38 > 80)
						break;
				}
				--v39;
			} while (_vm->_graphicsManager.min_y < v39);
		}
		v131[1] = v38 + 1;
		v42 = 0;
		v43 = srcX;
		if (_vm->_graphicsManager.max_x > srcX) {
			v44 = 3;
			do {
				v107 = v44;
				v44 = v107;
				if (checkCollisionLine(v43, srcY, &v136[3], &v141[3], 0, _vm->_objectsManager._lastLine) && v141[v107] <= _vm->_objectsManager._lastLine)
					break;
				v136[v107] = 0;
				v141[v107] = -1;
				++v42;
				if (v141[5] != -1 || v141[1] != -1) {
					if (v42 > 100)
						break;
				}
				++v43;
			} while (_vm->_graphicsManager.max_x > v43);
		}
		v131[3] = v42 + 1;
		v46 = 0;
		v47 = srcX;
		if (_vm->_graphicsManager.min_x < srcX) {
			v48 = 7;
			do {
				v108 = v48;
				v48 = v108;
				if (checkCollisionLine(v47, srcY, &v136[7], &v141[7], 0, _vm->_objectsManager._lastLine) && v141[v108] <= _vm->_objectsManager._lastLine)
					break;
				v136[v108] = 0;
				v141[v108] = -1;
				++v46;
				if (v141[5] != -1 || v141[1] != -1 || v141[3] != -1) {
					if (v46 > 100)
						break;
				}
				--v47;
			} while (_vm->_graphicsManager.min_x < v47);
		}
		v131[7] = v46 + 1;
		if (v141[1] != -1) {
			v50 = abs(v141[1] - v121);
			v126[1] = v50;
		}
		if (v141[3] != -1) {
			v51 = abs(v141[3] - v121);
			v126[3] = v51;
		}
		if (v141[5] != -1) {
			v52 = abs(v141[5] - v121);
			v126[5] = v52;
		}
		if (v141[7] != -1) {
			v53 = abs(v141[7] - v121);
			v126[7] = v53;
		}
		if (v141[1] == -1 && v141[3] == -1 && v141[5] == -1 && v141[7] == -1)
			error("Nearest point not found error");
		v54 = 0;
		if (v141[1] != -1 && v126[3] >= v126[1] && v126[5] >= v126[1] && v126[7] >= v126[1]) {
			v54 = 1;
			v115 = v141[1];
			v111 = v131[1];
			v113 = 1;
			v114 = v136[1];
		}
		if (!v54) {
			if (v141[5] != -1 && v126[3] >= v126[5] && v126[1] >= v126[5] && v126[7] >= v126[5]) {
				v54 = 1;
				v115 = v141[5];
				v111 = v131[5];
				v113 = 5;
				v114 = v136[5];
			}
			if (!v54) {
				if (v141[3] != -1 && v126[1] >= v126[3] && v126[5] >= v126[3] && v126[7] >= v126[3]) {
					v54 = 1;
					v115 = v141[3];
					v111 = v131[3];
					v113 = 3;
					v114 = v136[3];
				}
				if (!v54 && v141[7] != -1 && v126[1] >= v126[7] && v126[5] >= v126[7] && v126[3] >= v126[7]) {
					v115 = v141[7];
					v111 = v131[7];
					v113 = 7;
					v114 = v136[7];
				}
			}
		}
		v55 = PARC_PERS(srcX, srcY, clipDestX, clipDestY, v115, v121, 0);
		if (v55 != 1) {
			if (v55 == 2) {
LABEL_201:
				v115 = NV_LIGNEDEP;
				v114 = NV_LIGNEOFS;
				v112 = NV_POSI;
			} else {
				if (v113 == 1) {
					v56 = 0;
					if (v111 > 0) {
						do {
							if (checkCollisionLine(srcX, srcY - v56, &v125, &v124, _vm->_objectsManager._lastLine + 1, _linesNumb)
							        && _vm->_objectsManager._lastLine < v124) {
								v112 = v112;
								v58 = GENIAL(v124, v125, srcX, srcY - v56, srcX, srcY - v111, v112, &_vm->_globals.super_parcours[0], 4);
								if (v58 == -1) {
									_vm->_globals.super_parcours[v112] = -1;
									_vm->_globals.super_parcours[v112 + 1] = -1;
									_vm->_globals.super_parcours[v112 + 2] = -1;
									_vm->_globals.super_parcours[v112 + 3] = -1;
									return &_vm->_globals.super_parcours[0];
								}
								v112 = v58;
								if (NVPY != -1)
									v56 = srcY - NVPY;
							}
							v59 = v112;
							_vm->_globals.super_parcours[v59] = srcX;
							_vm->_globals.super_parcours[v59 + 1] = srcY - v56;
							_vm->_globals.super_parcours[v59 + 2] = 1;
							_vm->_globals.super_parcours[v59 + 3] = 0;
							v112 += 4;
							++v56;
						} while (v111 > v56);
					}
				}
				if (v113 == 5) {
					v60 = 0;
					if (v111 > 0) {
						do {
							if (checkCollisionLine(srcX, v60 + srcY, &v125, &v124, _vm->_objectsManager._lastLine + 1, _linesNumb)
							        && _vm->_objectsManager._lastLine < v124) {
								v112 = v112;
								v61 = GENIAL(v124, v125, srcX, v60 + srcY, srcX, v111 + srcY, v112, &_vm->_globals.super_parcours[0], 4);
								if (v61 == -1) {
									_vm->_globals.super_parcours[v112] = -1;
									_vm->_globals.super_parcours[v112 + 1] = -1;
									_vm->_globals.super_parcours[v112 + 2] = -1;
									_vm->_globals.super_parcours[v112 + 3] = -1;
									return &_vm->_globals.super_parcours[0];
								}
								v112 = v61;
								if (NVPY != -1)
									v60 = NVPY - srcY;
							}
							v62 = v112;
							_vm->_globals.super_parcours[v62] = srcX;
							_vm->_globals.super_parcours[v62 + 1] = v60 + srcY;
							_vm->_globals.super_parcours[v62 + 2] = 5;
							_vm->_globals.super_parcours[v62 + 3] = 0;
							v112 += 4;
							++v60;
						} while (v111 > v60);
					}
				}
				if (v113 == 7) {
					v63 = 0;
					if (v111 > 0) {
						do {
							if (checkCollisionLine(srcX - v63, srcY, &v125, &v124, _vm->_objectsManager._lastLine + 1, _linesNumb)
							        && _vm->_objectsManager._lastLine < v124) {
								v112 = v112;
								v64 = GENIAL(v124, v125, srcX - v63, srcY, srcX - v111, srcY, v112, &_vm->_globals.super_parcours[0], 4);
								if (v64 == -1) {
									_vm->_globals.super_parcours[v112] = -1;
									_vm->_globals.super_parcours[v112 + 1] = -1;
									_vm->_globals.super_parcours[v112 + 2] = -1;
									_vm->_globals.super_parcours[v112 + 3] = -1;
									return &_vm->_globals.super_parcours[0];
								}
								v112 = v64;
								if (NVPX != -1)
									v63 = srcX - NVPX;
							}
							v65 = v112;
							_vm->_globals.super_parcours[v65] = srcX - v63;
							_vm->_globals.super_parcours[v65 + 1] = srcY;
							_vm->_globals.super_parcours[v65 + 2] = 7;
							_vm->_globals.super_parcours[v65 + 3] = 0;
							v112 += 4;
							++v63;
						} while (v111 > v63);
					}
				}
				if (v113 == 3) {
					v66 = 0;
					if (v111 > 0) {
						do {
							if (checkCollisionLine(v66 + srcX, srcY, &v125, &v124, _vm->_objectsManager._lastLine + 1, _linesNumb)
							        && _vm->_objectsManager._lastLine < v124) {
								v112 = v112;
								v67 = GENIAL(v124, v125, v66 + srcX, srcY, v111 + srcX, srcY, v112, &_vm->_globals.super_parcours[0], 4);
								if (v67 == -1) {
									_vm->_globals.super_parcours[v112] = -1;
									_vm->_globals.super_parcours[v112 + 1] = -1;
									_vm->_globals.super_parcours[v112 + 2] = -1;
									_vm->_globals.super_parcours[v112 + 3] = -1;
									return &_vm->_globals.super_parcours[0];
								}
								v112 = v67;
								if (NVPX != -1)
									v66 = NVPX - srcX;
							}
							v68 = v112;
							_vm->_globals.super_parcours[v68] = v66 + srcX;
							_vm->_globals.super_parcours[v68 + 1] = srcY;
							_vm->_globals.super_parcours[v68 + 2] = 3;
							_vm->_globals.super_parcours[v68 + 3] = 0;
							v112 += 4;
							++v66;
						} while (v111 > v66);
					}
				}
			}
LABEL_234:
			if (v115 < v121) {
				for (i = v114; Ligne[v115]._lineDataEndIdx > i; ++i) {
					v70 = Ligne[v115]._lineData;
					v119 = v70[2 * i];
					v118 = v70[2 * i + 1];
					v71 = v112;
					_vm->_globals.super_parcours[v71] = v119;
					_vm->_globals.super_parcours[v71 + 1] = v118;
					_vm->_globals.super_parcours[v71 + 2] = Ligne[v115].field6;
					_vm->_globals.super_parcours[v71 + 3] = 0;
					v112 += 4;
				}
				v116 = v115 + 1;
				if ((v115 + 1) < v121) {
					do {
						v72 = 0;
						v110 = v116;
						for (j = v116; Ligne[j]._lineDataEndIdx > v72; j = v116) {
							v74 = Ligne[v110]._lineData;
							v119 = v74[2 * v72];
							v118 = v74[2 * v72 + 1];
							v75 = v112;
							_vm->_globals.super_parcours[v75] = v119;
							_vm->_globals.super_parcours[v75 + 1] = v118;
							_vm->_globals.super_parcours[v75 + 2] = Ligne[v110].field6;
							_vm->_globals.super_parcours[v75 + 3] = 0;
							v112 += 4;
							v76 = Ligne[v110]._lineDataEndIdx;
							if (v76 > 30) {
								v77 = abs(v76 / 2);
								if (v72 == v77) {
									v78 = PARC_PERS(v119, v118, clipDestX, clipDestY, v110, v121, v112);
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
							}
							++v72;
							v110 = v116;
						}
						v79 = PARC_PERS(v119, v118, clipDestX, clipDestY, v116, v121, v112);
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
						++v116;
					} while (v116 < v121);
				}
				v114 = 0;
				v115 = v121;
			}
			if (v115 > v121) {
				v80 = v114;
				if (v114 > 0) {
					v98 = v115;
					do {
						v81 = Ligne[v98]._lineData;
						v119 = v81[2 * v80];
						v118 = v81[2 * v80 + 1];

						v82 = v112;
						_vm->_globals.super_parcours[v82] = v119;
						_vm->_globals.super_parcours[v82 + 1] = v118;
						_vm->_globals.super_parcours[v82 + 2] = Ligne[v98].field8;
						_vm->_globals.super_parcours[v82 + 3] = 0;
						v112 += 4;
						--v80;
					} while (v80 > 0);
				}
				v117 = v115 - 1;
				if ((v115 - 1) > v121) {
					do {
						v83 = Ligne[v117]._lineDataEndIdx - 1;
						if (v83 > -1) {
							v109 = v117;
							do {
								v84 = Ligne[v109]._lineData;
								v119 = v84[2 * v83];
								v118 = v84[2 * v83 + 1];
								v85 = v112;
								_vm->_globals.super_parcours[v85] = v119;
								_vm->_globals.super_parcours[v85 + 1] = v118;
								_vm->_globals.super_parcours[v85 + 2] = Ligne[v109].field8;
								_vm->_globals.super_parcours[v85 + 3] = 0;
								v112 += 4;
								v86 = Ligne[v109]._lineDataEndIdx;
								if (v86 > 30) {
									v87 = abs(v86 / 2);
									if (v83 == v87) {
										v88 = PARC_PERS(v119, v118, clipDestX, clipDestY, v117, v121, v112);
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
								--v83;
							} while (v83 > -1);
						}
						v89 = PARC_PERS(v119, v118, clipDestX, clipDestY, v117, v121, v112);
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
						--v117;
					} while (v117 > v121);
				}
				v114 = Ligne[v121]._lineDataEndIdx - 1;
				v115 = v121;
			}
			if (v115 == v121) {
				if (v114 <= v120) {
					if (v114 < v120) {
						v94 = v114;
						v100 = v121;
						do {
							v95 = Ligne[v100]._lineData;
							v96 = v95[2 * v94 + 1];
							v97 = v112;
							_vm->_globals.super_parcours[v97] = v95[2 * v94];
							_vm->_globals.super_parcours[v97 + 1] = v96;
							_vm->_globals.super_parcours[v97 + 2] = Ligne[v100].field6;
							_vm->_globals.super_parcours[v97 + 3] = 0;
							v112 += 4;
							++v94;
						} while (v120 > v94);
					}
				} else {
					v90 = v114;
					v99 = v121;
					do {
						v91 = Ligne[v99]._lineData;
						v92 = v91[2 * v90 + 1];
						v93 = v112;
						_vm->_globals.super_parcours[v93] = v91[2 * v90];
						_vm->_globals.super_parcours[v93 + 1] = v92;
						_vm->_globals.super_parcours[v93 + 2] = Ligne[v99].field8;
						_vm->_globals.super_parcours[v93 + 3] = 0;
						v112 += 4;
						--v90;
					} while (v120 < v90);
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
	}
	return &_vm->_globals.super_parcours[0];
}

int LinesManager::PARC_PERS(int a1, int a2, int a3, int a4, int a5, int a6, int a7) {
	int16 *v17;
	int v18;
	int v19;
	int v20;
	int v21;
	int v22;
	int v23;
	int v24;
	int v25;
	int16 *v26;
	int v27;
	int16 *v28;
	int16 *v29;
	int v30;
	int v31;
	int16 *v32;
	int v33;
	int v34;
	int16 *v35;
	int v36;
	int v37;
	int16 *v38;
	int v39;
	int v40;
	int v41;
	int16 *v42;
	int v43;
	int v44;
	bool v45;
	int v46;
	int16 *v47;
	int v48;
	int16 *v49;
	int16 *v50;
	int v51;
	int v52;
	int16 *v53;
	int v54;
	int v55;
	int v56;
	int16 *v57;
	int v58;
	int v59;
	int16 *v60;
	int v61;
	int v62;
	int v63;
	int16 *v64;
	int v65;
	int v66;
	int16 *v72;
	int v73;
	int v74;
	int v76;
	int16 *v77;
	int16 *v78;
	int v79;
	int16 *v80;
	int v81;
	int16 *v82;
	int v83;
	int16 *v84;
	int v85;
	int16 *v86;
	int v87;
	int16 *v88;
	int v89;
	int v91;
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
	int v110;
	int v111;
	int v112;
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
	int v126 = 0;
	int v127;
	int v128;
	int v129;
	int v130;
	int v131;
	int v132;
	int v133;
	int v134;
	int v135;
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
	int v136 = 0;
	if (a5 == -1 && a6 == -1)
		v136 = 1;
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
		if (a3 >= v7 - 2 && a3 <= v7 + 2 && a4 >= v90 - 2 && a4 <= v90 + 2) {
LABEL_149:
			v27 = v115;
			v28 = _vm->_globals.essai0;
			v28[v27] = -1;
			v28[v27 + 1] = -1;
			v28[v27 + 2] = -1;

LABEL_150:
			if (v115) {
				v127 = 0;
				v116 = 0;
				v29 = _vm->_globals.essai0;
				do {
					v30 = v137;
					_vm->_globals.super_parcours[v30] = v29[v116];
					_vm->_globals.super_parcours[v30 + 1] = v29[v116 + 1];
					_vm->_globals.super_parcours[v30 + 2] = v29[v116 + 2];
					_vm->_globals.super_parcours[v30 + 3] = 0;
					v116 += 3;
					v137 += 4;

					if (v29[v116] == -1) {
						if (v29[v116 + 1] == -1)
							v127 = 1;
					}
				} while (v127 != 1);
			}
			v74 = v137;
			_vm->_globals.super_parcours[v74] = -1;
			_vm->_globals.super_parcours[v74 + 1] = -1;
			_vm->_globals.super_parcours[v74 + 2] = -1;
			_vm->_globals.super_parcours[v74 + 3] = -1;
			return 1;
		}
		v9 = abs(v7 - a3);
		v10 = v9 + 1;
		v11 = abs(v90 - a4);
		v107 = v11 + 1;
		if (v10 > (int16)(v11 + 1))
			v107 = v10;
		v12 = v107 - 1;
		v101 = 1000 * v10 / v12;
		v99 = 1000 * (int16)(v11 + 1) / v12;
		if (a3 < v7)
			v101 = -v101;
		if (a4 < v90)
			v99 = -v99;
		v13 = (int16)v101 / 1000;
		v94 = (int16)v99 / 1000;
		v91 = -1;
		if ((int16)v99 / 1000 == -1 && (unsigned int)v101 <= 150)
			v91 = 1;
		if (v13 == 1) {
			if ((unsigned int)(v99 + 1) <= 151)
				v91 = 3;
			if ((unsigned int)v99 <= 150)
				v91 = 3;
		}
		if (v94 == 1) {
			if ((unsigned int)v101 <= 150)
				v91 = 5;
			if ((unsigned int)(v101 + 150) <= 150)
				v91 = 5;
		}
		if (v13 == -1) {
			if ((unsigned int)v99 <= 150)
				v91 = 7;
			if ((unsigned int)(v99 + 150) <= 150)
				v91 = 7;
		}
		if (v94 == -1 && (unsigned int)(v101 + 150) <= 150)
			v91 = 1;
		if (v91 == -1 && !VERIF_SMOOTH(v7, v109, a3, a4) && SMOOTH_MOVE(v7, v109, a3, a4) != -1)
			break;
LABEL_72:
		v19 = abs(v111 - a3);
		v20 = v19 + 1;
		v95 = abs(v109 - a4);
		v108 = v95 + 1;
		if (v20 > (v95 + 1))
			v108 = v20;
		if (v108 <= 10)
			goto LABEL_149;
		v21 = v108 - 1;
		v102 = 1000 * v20 / v21;
		v100 = 1000 * (v95 + 1) / v21;
		if (a3 < v111)
			v102 = -v102;
		if (a4 < v109)
			v100 = -v100;
		v22 = v102 / 1000;
		v96 = v100 / 1000;
		v106 = 1000 * v111;
		v105 = 1000 * v109;
		v104 = 1000 * v111 / 1000;
		v103 = v105 / 1000;
		if (!(v102 / 1000) && v96 == -1)
			v91 = 1;
		if (v22 == 1) {
			if (v96 == -1)
				v91 = 2;
			if (!v96)
				v91 = 3;
			if (v96 == 1)
				v91 = 4;
		}
		if (!v22 && v96 == 1)
			v91 = 5;
		if ((v22 != -1) && (v96 == -1)) {
			if (v102 >= 0 && v102 < 510)
				v91 = 1;
			else if (v102 >= 510 && v102 <= 1000)
				v91 = 2;
		} else {
			if (v96 == 1)
				v91 = 6;
			else if (!v96)
				v91 = 7;
			else if (v96 == -1) {
				if (v102 >= 0 && v102 < 510)
					v91 = 1;
				else if (v102 >= 510 && v102 <= 1000)
					v91 = 2;
				else 
					v91 = 8;
			}
		}
		if (v22 == 1) {
			if ((unsigned int)(v100 + 1) <= 511)
				v91 = 2;
			if ((unsigned int)(v100 + 510) <= 510)
				v91 = 3;
			if ((unsigned int)v100 <= 510)
				v91 = 3;
			if ((unsigned int)(v100 - 510) <= 490)
				v91 = 4;
		}
		if (v96 == 1) {
			if ((unsigned int)(v102 - 510) <= 490)
				v91 = 4;
			if ((unsigned int)v102 <= 510)
				v91 = 5;
			// CHECKME: The two conditions on v102 are not compatible!
			if (v102 >= -1 && v102 <= -510)
				v91 = 6;
			if ((unsigned int)(v102 + 510) <= 510)
				v91 = 5;
		}
		if (v22 == -1) {
			if ((unsigned int)(v100 - 510) <= 490)
				v91 = 6;
			if ((unsigned int)v100 <= 510)
				v91 = 7;
			if ((unsigned int)(v100 + 1000) <= 490)
				v91 = 8;
			if ((unsigned int)(v100 + 510) <= 510)
				v91 = 7;
		}
		if (v96 == -1) {
			if ((unsigned int)(v102 + 1000) <= 490)
				v91 = 8;
			if ((unsigned int)(v102 + 510) <= 510)
				v91 = 1;
		}
		v23 = 0;
		if (v108 + 1 <= 0)
			goto LABEL_149;
		while (!checkCollisionLine(v104, v103, &v143, &v142, 0, _linesNumb)) {
			v25 = v115;
			v26 = _vm->_globals.essai0;
			v26[v25] = v104;
			v26[v25 + 1] = v103;
			v26[v25 + 2] = v91;
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
		v24 = GENIAL(v142, v143, v104, v103, a3, a4, v115, _vm->_globals.essai0, 3);
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
	v91 = SMOOTH_SENS;
	v14 = 0;
	int v16;
	for (;;) {
		int v15 = SMOOTH[v14].field0;
		v112 = v15;
		v110 = SMOOTH[v14].field2;
		if (v15 == -1 || SMOOTH[v14].field2 == -1) {
			v126 = 1;
			if (v126 == 1) {
				v18 = v14 - 1;
				v111 = SMOOTH[v18].field0;
				v109 = SMOOTH[v18].field2;
				goto LABEL_72;
			}
		}
		if (checkCollisionLine(v15, v110, &v143, &v142, 0, _linesNumb))
			break;
		v16 = v115;

		v17 = _vm->_globals.essai0;
		v17[v16] = v112;
		v17[v16 + 1] = v110;
		v17[v16 + 2] = v91;
		v115 += 3;
		++v14;
		if (v126 == 1) {
			v18 = v14 - 1;
			v111 = SMOOTH[v18].field0;
			v109 = SMOOTH[v18].field2;
			goto LABEL_72;
		}
	}
	if (v142 > _vm->_objectsManager._lastLine)
		v142 = -1;

LABEL_157:
	v31 = v115;
	v32 = _vm->_globals.essai0;
	v32[v31] = -1;
	v32[v31 + 1] = -1;
	v32[v31 + 2] = -1;

	v117 = 0;
	v33 = v98;
	v92 = v97;
LABEL_158:
	v113 = v33;
	if (a3 >= v33 - 2 && a3 <= v33 + 2 && a4 >= v92 - 2 && a4 <= v92 + 2)
		goto LABEL_194;
	if (v33 >= a3) {
LABEL_165:
		if (v113 > a3) {
			v36 = v113;
			while (!checkCollisionLine(v36, v92, &v141, &v140, 0, _linesNumb)) {
				v37 = v117;
				v38 = _vm->_globals.essai1;
				v38[v37] = v36;
				v38[v37 + 1] = v92;
				v38[v37 + 2] = 7;
				v117 += 3;
				--v36;
				if (a3 >= v36)
					goto LABEL_171;
			}
			goto LABEL_168;
		}
LABEL_171:
		if (v92 >= a4) {
LABEL_181:
			if (v92 > a4) {
				v43 = v92;
				do {
					if (checkCollisionLine(a3, v43, &v141, &v140, 0, _linesNumb)) {
						if (_vm->_objectsManager._lastLine < v140) {
							v44 = GENIAL(v140, v141, a3, v43, a3, a4, v117, _vm->_globals.essai1, 3);
							if (v44 == -1)
								goto LABEL_195;
							v117 = v44;
							if (NVPX != -1) {
								if (NVPY != -1) {
									v33 = NVPX;
									v92 = NVPY;
									v45 = checkCollisionLine(NVPX, NVPY, &v141, &v140, 0, _vm->_objectsManager._lastLine);
									if (v45 && v140 <= _vm->_objectsManager._lastLine)
										goto LABEL_202;
									goto LABEL_158;
								}
							}
						}
						if (v140 <= _vm->_objectsManager._lastLine)
							goto LABEL_202;
					}
					v46 = v117;
					v47 = _vm->_globals.essai1;
					v47[v46] = a3;
					v47[v46 + 1] = v43;
					v47[v46 + 2] = 1;
					v117 += 3;
					--v43;
				} while (a4 < v43);
			}
LABEL_194:
			v48 = v117;
			v49 = _vm->_globals.essai1;
			v49[v48] = -1;
			v49[v48 + 1] = -1;
			v49[v48 + 2] = -1;
LABEL_195:
			if (v117) {
				v128 = 0;
				v118 = 0;
				v50 = _vm->_globals.essai1;
				do {
					v51 = v137;
					_vm->_globals.super_parcours[v51] = v50[v118];
					_vm->_globals.super_parcours[v51 + 1] = v50[v118 + 1];
					_vm->_globals.super_parcours[v51 + 2] = v50[v118 + 2];
					_vm->_globals.super_parcours[v51 + 3] = 0;
					v118 += 3;
					v137 += 4;
					if (v50[v118] == -1 && v50[v118 + 1] == -1)
						v128 = 1;
				} while (v128 != 1);
			}
			v74 = v137;
			_vm->_globals.super_parcours[v74] = -1;
			_vm->_globals.super_parcours[v74 + 1] = -1;
			_vm->_globals.super_parcours[v74 + 2] = -1;
			_vm->_globals.super_parcours[v74 + 3] = -1;
			return 1;
		}
		v39 = v92;
		for (;;) {
			if (checkCollisionLine(a3, v39, &v141, &v140, 0, _linesNumb)) {
				if (_vm->_objectsManager._lastLine < v140) {
					v40 = GENIAL(v140, v141, a3, v39, a3, a4, v117, _vm->_globals.essai1, 3);
					if (v40 == -1)
						goto LABEL_195;
					v117 = v40;
					if (NVPX != -1) {
						if (NVPY != -1) {
							v33 = NVPX;
							v92 = NVPY;
							v45 = checkCollisionLine(NVPX, NVPY, &v141, &v140, 0, _vm->_objectsManager._lastLine);
							if (v45 && v140 <= _vm->_objectsManager._lastLine)
								goto LABEL_202;
							goto LABEL_158;
						}
					}
				}
				if (v140 <= _vm->_objectsManager._lastLine)
					goto LABEL_202;
			}
			v41 = v117;

			v42 = _vm->_globals.essai1;
			v42[v41] = a3;
			v42[v41 + 1] = v39;
			v42[v41 + 2] = 5;
			v117 += 3;
			++v39;
			if (a4 <= v39)
				goto LABEL_181;
		}
	}
	while (!checkCollisionLine(v33, v92, &v141, &v140, 0, _linesNumb)) {
		v34 = v117;
		v35 = _vm->_globals.essai1;
		v35[v34] = v33;
		v35[v34 + 1] = v92;
		v35[v34 + 2] = 3;
		v117 += 3;
		++v33;
		if (a3 <= v33)
			goto LABEL_165;
	}
LABEL_168:
	if (v140 > _vm->_objectsManager._lastLine)
		v140 = -1;
LABEL_202:
	v52 = v117;
	v53 = _vm->_globals.essai1;
	v53[v52] = -1;
	v53[v52 + 1] = -1;
	v53[v52 + 2] = -1;
	v117 = 0;
	v54 = v98;
	v93 = v97;
LABEL_203:
	v114 = v54;
	if (a3 >= v54 - 2 && a3 <= v54 + 2 && a4 >= v93 - 2 && a4 <= v93 + 2)
		goto LABEL_241;
	if (v93 < a4) {
		v55 = v93;
		while (!checkCollisionLine(v114, v55, &v139, &v138, 0, _linesNumb)) {
			v56 = v117;
			v57 = _vm->_globals.essai2;
			v57[v56] = v114;
			v57[v56 + 1] = v55;
			v57[v56 + 2] = 5;
			v117 += 3;
			++v55;
			if (a4 <= v55)
				goto LABEL_211;
		}
		goto LABEL_214;
	}
LABEL_211:
	if (v93 > a4) {
		v58 = v93;
		while (!checkCollisionLine(v114, v58, &v139, &v138, 0, _linesNumb)) {
			v59 = v117;
			v60 = _vm->_globals.essai2;
			v60[v59] = v114;
			v60[v59 + 1] = v58;
			v60[v59 + 2] = 1;
			v117 += 3;
			--v58;
			if (a4 >= v58)
				goto LABEL_217;
		}
LABEL_214:
		if (v138 > _vm->_objectsManager._lastLine)
			v138 = -1;
LABEL_249:
		v76 = v117;
		v77 = _vm->_globals.essai2;
		v77[v76] = -1;
		v77[v76 + 1] = -1;
		v77[v76 + 2] = -1;

		if (v136 != 1) {
			if (a6 > v144) {
				if (_vm->_globals.essai0[0] != -1 && v142 > v144 && v140 <= v142 && v138 <= v142 && a6 >= v142) {
					NV_LIGNEDEP = v142;
					NV_LIGNEOFS = v143;
					v130 = 0;
					v120 = 0;
					v78 = _vm->_globals.essai0;
					do {
						v79 = v137;
						_vm->_globals.super_parcours[v79] = v78[v120];
						_vm->_globals.super_parcours[v79 + 1] = v78[v120 + 1];
						_vm->_globals.super_parcours[v79 + 2] = v78[v120 + 2];
						_vm->_globals.super_parcours[v79 + 3] = 0;
						v120 += 3;
						v137 += 4;
						if (v78[v120] == -1 && v78[v120 + 1] == -1)
							v130 = 1;
					} while (v130 != 1);
					NV_POSI = v137;
					return 2;
				}
				v80 = _vm->_globals.essai1;
				if (_vm->_globals.essai1[0] != -1 && v144 < v140 && v138 <= v140 && v142 <= v140 && a6 >= v140) {
					NV_LIGNEDEP = v140;
					NV_LIGNEOFS = v141;
					v131 = 0;
					v121 = 0;
					do {
						assert(137 <= 32000);
						v81 = v137;
						_vm->_globals.super_parcours[v81] = v80[v121];
						_vm->_globals.super_parcours[v81 + 1] = v80[v121 + 1];
						_vm->_globals.super_parcours[v81 + 2] = v80[v121 + 2];
						_vm->_globals.super_parcours[v81 + 3] = 0;
						v121 += 3;
						v137 += 4;
						if (v80[v121] == -1 && v80[v121 + 1] == -1)
							v131 = 1;
					} while (v131 != 1);
					NV_POSI = v137;
					return 2;
				}
				v82 = _vm->_globals.essai2;
				if (_vm->_globals.essai2[0] != -1) {
					if (v144 < v138 && v140 < v138 && v142 < v138 && a6 >= v138) {
						NV_LIGNEDEP = v138;
						NV_LIGNEOFS = v139;
						v132 = 0;
						v122 = 0;
						do {
							assert(v137 <= 32000);
							v83 = v137;
							_vm->_globals.super_parcours[v83] = v82[v122];
							_vm->_globals.super_parcours[v83 + 1] = v82[v122 + 1];
							_vm->_globals.super_parcours[v83 + 2] = v82[v122 + 2];
							_vm->_globals.super_parcours[v83 + 3] = 0;
							v122 += 3;
							v137 += 4;
							if (v82[v122] == -1 && v82[v122 + 1] == -1)
								v132 = 1;
						} while (v132 != 1);
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
					v133 = 0;
					v123 = 0;
					v84 = _vm->_globals.essai1;
					do {
						assert(137 <= 32000);
						v85 = v137;
						_vm->_globals.super_parcours[v85] = v84[v123];
						_vm->_globals.super_parcours[v85 + 1] = v84[v123 + 1];
						_vm->_globals.super_parcours[v85 + 2] = v84[v123 + 2];
						_vm->_globals.super_parcours[v85 + 3] = 0;
						v123 += 3;
						v137 += 4;
						if (v84[v123] == -1 && v84[v123 + 1] == -1)
							v133 = 1;
					} while (v133 != 1);
					NV_POSI = v137;
					return 2;
				}
				v86 = _vm->_globals.essai2;
				if (_vm->_globals.essai2[0] != -1 && v144 > v138 && v140 >= v138 && v142 >= v138 && a6 <= v138) {
					NV_LIGNEDEP = v138;
					NV_LIGNEOFS = v139;
					v134 = 0;
					v124 = 0;
					do {
						assert(137 <= 32000);
						v87 = v137;
						_vm->_globals.super_parcours[v87] = v86[v124];
						_vm->_globals.super_parcours[v87 + 1] = v86[v124 + 1];
						_vm->_globals.super_parcours[v87 + 2] = v86[v124 + 2];
						_vm->_globals.super_parcours[v87 + 3] = 0;
						v124 += 3;
						v137 += 4;
						if (v86[v124] == -1 && v86[v124 + 1] == -1)
							v134 = 1;
					} while (v134 != 1);
					NV_POSI = v137;
					return 2;
				}
				if (_vm->_globals.essai1[0] != -1 && v144 > v142 && v140 >= v142 && v138 >= v142 && a6 <= v142) {
					NV_LIGNEDEP = v142;
					NV_LIGNEOFS = v143;
					v135 = 0;
					v125 = 0;
					v88 = _vm->_globals.essai0;
					do {
						assert(137 <= 32000);
						v89 = v137;
						_vm->_globals.super_parcours[v89] = v88[v125];
						_vm->_globals.super_parcours[v89 + 1] = v88[v125 + 1];
						_vm->_globals.super_parcours[v89 + 2] = v88[v125 + 2];
						_vm->_globals.super_parcours[v89 + 3] = 0;
						v125 += 3;
						v137 += 4;
						if (v88[v125] == -1 && v88[v125 + 1] == -1)
							v135 = 1;
					} while (v135 != 1);

					NV_POSI = v137;
					return 2;
				}
			}
		}
		return 0;
	}
LABEL_217:
	if (v114 < a3) {
		v61 = v114;
		do {
			if (checkCollisionLine(v61, a4, &v139, &v138, 0, _linesNumb)) {
				if (_vm->_objectsManager._lastLine < v138) {
					v62 = GENIAL(v138, v139, v61, a4, a3, a4, v117, _vm->_globals.essai2, 3);
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
			v63 = v117;

			v64 = _vm->_globals.essai2;
			v64[v63] = v61;
			v64[v63 + 1] = a4;
			v64[v63 + 2] = 3;
			v117 += 3;
			++v61;
		} while (a3 > v61);
	}
	if (v114 > a3) {
		v65 = v114;
		do {
			if (checkCollisionLine(v65, a4, &v139, &v138, 0, _linesNumb)) {
				if (_vm->_objectsManager._lastLine < v138) {
					v66 = GENIAL(v138, v139, v65, a4, a3, a4, v117, _vm->_globals.essai2, 3);
					if (v66 == -1)
						goto LABEL_242;
					v117 = v66;
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
			_vm->_globals.essai2[v117] = v65;
			_vm->_globals.essai2[v117 + 1] = a4;
			_vm->_globals.essai2[v117 + 2] = 7;
			v117 += 3;
			--v65;
		} while (a3 < v65);
	}
	v138 = -1;
LABEL_241:
	_vm->_globals.essai2[v117] = -1;
	_vm->_globals.essai2[v117 + 1] = -1;
	_vm->_globals.essai2[v117 + 2] = -1;
LABEL_242:
	if (v117) {
		v129 = 0;
		v119 = 0;
		v72 = _vm->_globals.essai2;
		do {
			v73 = v137;
			_vm->_globals.super_parcours[v73] = v72[v119];
			_vm->_globals.super_parcours[v73 + 1] = v72[v119 + 1];
			_vm->_globals.super_parcours[v73 + 2] = v72[v119 + 2];
			_vm->_globals.super_parcours[v73 + 3] = 0;
			v119 += 3;
			v137 += 4;
			if (v72[v119] == -1 && v72[v119 + 1] == -1)
				v129 = 1;
		} while (v129 != 1);
	}
	v74 = v137;
	_vm->_globals.super_parcours[v74] = -1;
	_vm->_globals.super_parcours[v74 + 1] = -1;
	_vm->_globals.super_parcours[v74 + 2] = -1;
	_vm->_globals.super_parcours[v74 + 3] = -1;
	return 1;
}

int LinesManager::VERIF_SMOOTH(int a1, int a2, int a3, int a4) {
	int v6;
	int v7;
	int v8;
	int v9;
	int v12;
	int v14;
	int v15;
	int v16;
	int v17;
	int v18;

	int v5 = abs(a1 - a3) + 1;
	int v13 = abs(a2 - a4) + 1;
	if (v5 > v13)
		v13 = v5;
	if (v13 <= 10)
		return -1;
	v6 = v13 - 1;
	v16 = 1000 * v5 / v6;
	v15 = 1000 * (abs(a2 - a4) + 1) / v6;
	if (a3 < a1)
		v16 = -v16;
	if (a4 < a2)
		v15 = -v15;
	v7 = 1000 * a1;
	v8 = 1000 * a2;
	v9 = 1000 * a1 / 1000;
	v12 = 1000 * a2 / 1000;
	v14 = 0;
	if (v13 + 1 > 0) {
		while (!checkCollisionLine(v9, v12, &v18, &v17, 0, _linesNumb) || v17 > _vm->_objectsManager._lastLine) {
			v7 += v16;
			v8 += v15;
			v9 = v7 / 1000;
			v12 = v8 / 1000;
			++v14;
			if (v14 >= v13 + 1)
				return 0;
		}
		return -1;
	}
	return 0;
}

int LinesManager::SMOOTH_MOVE(int a3, int a4, int a5, int a6) {
	int v6;
	int v7;
	int v10;
	int v11;
	int v14;
	int v22;
	int v25;
	int v33;
	int v37;
	int v38;
	int v39;
	int v40;
	int v41;
	int v42;
	int v47;
	int v50;
	int v51;
	int v52;
	int hopkinsIdx;
	int smoothIdx;

	int v62 = a3;
	int v63 = a4;
	if (a3 > a5 && a6 > a4) {
		hopkinsIdx = 36;
		smoothIdx = 0;
		int loopCount = 0;
		while (v62 > a5 && a6 > v63) {
			v25 = _vm->_globals.Hopkins[hopkinsIdx].field0;
			v40 = _vm->_globals.Hopkins[hopkinsIdx].field2;
			int spriteSize = _vm->_globals.STAILLE[v63];
			if (spriteSize < 0) {
				v25 = _vm->_graphicsManager.zoomOut(v25, -spriteSize);
				v40 = _vm->_graphicsManager.zoomOut(v40, -spriteSize);
			} else if (spriteSize > 0) {
				v25 = _vm->_graphicsManager.zoomIn(v25, spriteSize);
				v40 = _vm->_graphicsManager.zoomIn(v40, spriteSize);
			}
			v33 = v63 + v40;
			for (int v34 = 0; v34 < v25; v34++) {
				--v62;
				SMOOTH[smoothIdx].field0 = v62;
				if (v63 != v33)
					v63++;
				SMOOTH[smoothIdx].field2 = v63;
				smoothIdx++;
			}
			++hopkinsIdx;
			if (hopkinsIdx == 48)
				hopkinsIdx = 36;
			++loopCount;
		}
		if (loopCount > 5) {
			SMOOTH[smoothIdx].field0 = -1;
			SMOOTH[smoothIdx].field2 = -1;
			_vm->_linesManager.SMOOTH_SENS = 6;
			SMOOTH_X = v62;
			SMOOTH_Y = v63;
			return 0;
		}
	} else if (a3 < a5 && a6 > a4) {
		v52 = 36;
		smoothIdx = 0;
		int loopCount = 0;
		while (v62 < a5 && a6 > v63) {
			v14 = _vm->_globals.Hopkins[v52].field0;
			v39 = _vm->_globals.Hopkins[v52].field2;
			int spriteSize = _vm->_globals.STAILLE[v63];
			if (spriteSize < 0) {
				v47 = _vm->_globals.STAILLE[v63];
				v14 = _vm->_graphicsManager.zoomOut(v14, -spriteSize);
				v39 = _vm->_graphicsManager.zoomOut(v39, -spriteSize);
				spriteSize = v47;
			}
			if (spriteSize > 0) {
				v14 = _vm->_graphicsManager.zoomIn(v14, spriteSize);
				v39 = _vm->_graphicsManager.zoomIn(v39, spriteSize);
			}
			v22 = v63 + v39;
			for (int i = 0; i < v14; i++) {
				++v62;
				SMOOTH[smoothIdx].field0 = v62;
				if (v63 != v22)
					v63++;
				SMOOTH[smoothIdx].field2 = v63;
				smoothIdx++;
			}
			++v52;
			if (v52 == 48)
				v52 = 36;
			++loopCount;
		}
		if (loopCount > 5) {
			SMOOTH[smoothIdx].field0 = -1;
			SMOOTH[smoothIdx].field2 = -1;
			_vm->_linesManager.SMOOTH_SENS = 4;
			SMOOTH_X = v62;
			SMOOTH_Y = v63;
			return 0;
		}
	} else if (a3 > a5 && a6 < a4) {
		v51 = 12;
		smoothIdx = 0;
		int loopCount = 0;
		while (v62 > a5 && a6 < v63) {
			v10 = _vm->_globals.Hopkins[v51].field2;
			v42 = v63;
			v11 = _vm->_graphicsManager.zoomOut(_vm->_globals.Hopkins[v51].field0, 25);
			v38 = _vm->_graphicsManager.zoomOut(v10, 25);
			v63 = v42;
			for (int v12 = 0; v12 < v11; v12++) {
				--v62;
				SMOOTH[smoothIdx].field0 = v62;
				if ((uint16)v63 != (uint16)v42 + v38)
					v63--;
				SMOOTH[smoothIdx].field2 = v63;
				smoothIdx++;
			}
			++v51;
			if (v51 == 24)
				v51 = 12;
			++loopCount;
		}
		if (loopCount > 5) {
			SMOOTH[smoothIdx].field0 = -1;
			SMOOTH[smoothIdx].field2 = -1;
			_vm->_linesManager.SMOOTH_SENS = 8;
			SMOOTH_X = v62;
			SMOOTH_Y = v63;
			return 0;
		}
	} else if (a3 < a5 && a6 < a4) {
		v50 = 12;
		int smoothIdx = 0;
		int loopCount = 0;
		while (v62 < a5 && a6 < v63) {
			v6 = _vm->_globals.Hopkins[v50].field2;
			v41 = v63;
			v7 = _vm->_graphicsManager.zoomOut(_vm->_globals.Hopkins[v50].field0, 25);
			v37 = _vm->_graphicsManager.zoomOut(v6, 25);
			v63 = v41;
			for (int v8 = 0; v8 < v7; v8++) {
				++v62;
				SMOOTH[smoothIdx].field0 = v62;
				if ((uint16)v63 != (uint16)v41 + v37)
					v63--;
				SMOOTH[smoothIdx].field2 = v63;
				smoothIdx++;
			}
			++v50;
			if (v50 == 24)
				v50 = 12;
			++loopCount;
		}

		if (loopCount > 5) {
			SMOOTH[smoothIdx].field0 = -1;
			SMOOTH[smoothIdx].field2 = -1;
			_vm->_linesManager.SMOOTH_SENS = 2;
			SMOOTH_X = v62;
			SMOOTH_Y = v63;
			return 0;
		}
	}
	return -1;
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
