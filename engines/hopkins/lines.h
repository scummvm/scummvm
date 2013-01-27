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

#ifndef HOPKINS_LINES_H
#define HOPKINS_LINES_H

#include "common/scummsys.h"
#include "common/str.h"

namespace Hopkins {

class HopkinsEngine;

struct LigneZoneItem {
	int _count;
	int field2;
	int16 *_zoneData;
};

struct LigneItem {
	int _lineDataEndIdx;
	int field2; // Useless variable
	int _direction;
	int field6;
	int field8;
	int16 *_lineData;
};

struct SmoothItem {
	int _posX;
	int _posY;
};

struct SegmentItem {
	int field0; // Useless variable
	int field2;
	int field4;
};

struct CarreZoneItem {
	int _enabledFl;
	int _left;
	int _right;
	int _top;
	int _bottom;
	int _minZoneLineIdx;
	int _maxZoneLineIdx;
	bool _squareZoneFl;
};

class LinesManager {
private:
	HopkinsEngine *_vm;

	int _pathFindingMaxDepth;
	SmoothItem _smoothRoute[4000];
	int NV_LIGNEDEP;
	int NV_LIGNEOFS;
	int NV_POSI;
	int NVPX;
	int NVPY;
	int _smoothMoveDirection;
	int16 super_parcours[32002];

	int CALC_PROPRE(int idx);
	int checkInventoryHotspotsRow(int posX, int minZoneNum, bool lastRow);
	void removeZoneLine(int idx);
	void removeLine(int idx);
	bool checkCollisionLine(int xp, int yp, int *foundDataIdx, int *foundLineIdx, int startLineIdx, int endLineIdx);
	int CONTOURNE1(int a1, int a2, int a3, int a4, int a5, int16 *route, int a7, int a8, int a9);
	int CONTOURNE(int a1, int a2, int a3, int a4, int a5, int16 *route, int a7);
	bool MIRACLE(int a1, int a2, int a3, int a4, int a5);
	int GENIAL(int lineIdx, int dataIdx, int a3, int a4, int a5, int a6, int a7, int16 *route, int a9);
	int PARC_PERS(int fromX, int fromY, int destX, int destY, int a5, int a6, int a7);
	bool checkSmoothMove(int fromX, int fromY, int destX, int destY);
	bool makeSmoothMove(int fromX, int fromY, int destX, int destY);
	bool PLAN_TEST(int paramX, int paramY, int a3, int a4, int a5);
	int TEST_LIGNE(int paramX, int paramY, int *a3, int *foundLineIdx, int *foundDataIdx);

public:
	int _lastLine;
	int _linesNumb;
	LigneZoneItem _zoneLine[401];
	LigneItem Ligne[400];
	int _maxLineIdx;
	SegmentItem Segment[101];
	CarreZoneItem CarreZone[101];
	int SegmentEnCours;

public:
	int16 *essai0;
	int16 *essai1;
	int16 *essai2;
	int16 *BufLig;
	int16 *_route;

	LinesManager();
	void setParent(HopkinsEngine *vm);

	int checkInventoryHotspots(int posX, int posY);
	void loadLines(const Common::String &file);
	void addZoneLine(int idx, int a2, int a3, int a4, int a5, int bobZoneIdx);
	int16 *PARCOURS2(int fromX, int fromY, int destX, int destY);
	void PACOURS_PROPRE(int16 *route);
	int16 *cityMapCarRoute(int x1, int y1, int x2, int y2);
	void clearAllZones();
	void resetLines();
	void addLine(int idx, int a2, int a3, int a4, int a5, int a6, int a7);
	void initRoute();
	int colision(int xp, int yp);
	int MZONE();
	void CARRE_ZONE();
};

} // End of namespace Hopkins

#endif /* HOPKINS_FONT_H */
