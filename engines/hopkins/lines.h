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

#include "hopkins/globals.h"

#include "common/scummsys.h"
#include "common/str.h"

namespace Hopkins {

class HopkinsEngine;

struct LigneZoneItem {
	int _count;
	int _bobZoneIdx;
	int16 *_zoneData;
};

struct RouteItem;

struct LigneItem {
	int _lineDataEndIdx;
	Directions _direction;
	Directions _directionRouteInc;
	Directions _directionRouteDec;
	int16 *_lineData;

	int appendToRouteInc(int from, int to, RouteItem *route, int index);
	int appendToRouteDec(int from, int to, RouteItem *route, int index);
};

struct SmoothItem {
	int _posX;
	int _posY;
};

struct SegmentItem {
	int _minZoneLineIdx;
	int _maxZoneLineIdx;
};

struct SquareZoneItem {
	int _enabledFl;
	int _left;
	int _right;
	int _top;
	int _bottom;
	int _minZoneLineIdx;
	int _maxZoneLineIdx;
	bool _squareZoneFl;
};

struct ZonePItem {
	int _destX;
	int _destY;
	int _spriteIndex;
	int field6;
	int field7;
	int field8;
	int field9;
	int fieldA;
	int fieldB;
	int fieldC;
	int fieldD;
	int fieldE;
	int fieldF;
	bool _enabledFl;
	int field12;
};

struct RouteItem {
	int16 _x;
	int16 _y;
	Directions _dir;
	bool isValid() const { return _x != -1 || _y != -1; }
	void invalidate() { _x = _y = -1; _dir = DIR_NONE; }
	void set(int16 X, int16 Y, Directions dir) { _x = X; _y = Y; _dir = dir; }
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
	Directions _smoothMoveDirection;
	RouteItem super_parcours[8001];
	byte *BUFFERTAPE;
	RouteItem *essai0;
	RouteItem *essai1;
	int16 *BufLig;
	LigneZoneItem _zoneLine[401];
	LigneItem Ligne[400];
	SegmentItem _segment[101];
	SquareZoneItem _squareZone[101];
	int _currentSegmentId;
	int _maxLineIdx;
	int _lastLine;
	int _linesNumb;

	int checkInventoryHotspotsRow(int posX, int minZoneNum, bool lastRow);
	void removeZoneLine(int idx);
	void removeLine(int idx);
	bool checkCollisionLine(int xp, int yp, int *foundDataIdx, int *foundLineIdx, int startLineIdx, int endLineIdx);
	bool checkSmoothMove(int fromX, int fromY, int destX, int destY);
	bool makeSmoothMove(int fromX, int fromY, int destX, int destY);

	int CALC_PROPRE(int idx);
	int CONTOURNE1(int a1, int a2, int a3, int a4, int a5, RouteItem *route, int a8, int a9);
	int CONTOURNE(int a1, int a2, int a3, int a4, int a5, RouteItem *route);
	bool MIRACLE(int fromX, int fromY, int a3, int a4, int a5);
	int GENIAL(int lineIdx, int dataIdx, int a3, int a4, int a5, int a6, int a7, RouteItem *route);
	int PARC_PERS(int fromX, int fromY, int destX, int destY, int a5, int a6, int a7);
	bool PLAN_TEST(int paramX, int paramY, int a3, int a4, int a5);
	int TEST_LIGNE(int paramX, int paramY, int *a3, int *foundLineIdx, int *foundDataIdx);
	int colision(int xp, int yp);

public:
	RouteItem *_route;
	RouteItem *essai2;

	int BOBZONE[105];
	bool BOBZONE_FLAG[105];
	ZonePItem ZONEP[106];

	LinesManager();
	~LinesManager();
	void setParent(HopkinsEngine *vm);
	void clearAll();

	void setMaxLineIdx(int idx);
	int checkInventoryHotspots(int posX, int posY);
	void addZoneLine(int idx, int a2, int a3, int a4, int a5, int bobZoneIdx);
	void loadLines(const Common::String &file);
	void addLine(int idx, Directions direction, int a3, int a4, int a5, int a6);
	void initRoute();
	RouteItem *cityMapCarRoute(int x1, int y1, int x2, int y2);
	void clearAllZones();
	void resetLines();
	void resetLinesNumb();
	void resetLastLine();
	void enableZone(int idx);
	void disableZone(int idx);
	void checkZone();

	int MZONE();
	void CARRE_ZONE();
	RouteItem *PARCOURS2(int fromX, int fromY, int destX, int destY);
	void PACOURS_PROPRE(RouteItem *route);
};

} // End of namespace Hopkins

#endif /* HOPKINS_FONT_H */
