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

#define INVALID_LINEIDX 1300

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
	bool _enabledFl;
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
	int _verbFl1;
	int _verbFl2;
	int _verbFl3;
	int _verbFl4;
	int _verbFl5;
	int _verbFl6;
	int _verbFl7;
	int _verbFl8;
	int _verbFl9;
	int _verbFl10;
	bool _enabledFl;
	int _messageId;
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
	Directions _smoothMoveDirection;
	LigneZoneItem _zoneLine[401];
	SegmentItem _segment[101];
	SquareZoneItem _squareZone[101];
	int _currentSegmentId;
	int _maxLineIdx;
	int _lastLine;
	int _linesNumb;
	int _newLineIdx;
	int _newLineDataIdx;
	int _newRouteIdx;
	int _newPosX;
	int _newPosY;

	byte *_largeBuf;
	RouteItem *_testRoute0;
	RouteItem *_testRoute1;
	int16 *_lineBuf;
	LigneItem _lineItem[400];
	RouteItem _bestRoute[8001];

	int checkInventoryHotspotsRow(int posX, int minZoneNum, bool lastRow);
	void removeZoneLine(int idx);
	void removeLine(int idx);
	int checkCollision(int xp, int yp);
	bool checkCollisionLine(int xp, int yp, int *foundDataIdx, int *foundLineIdx, int startLineIdx, int endLineIdx);
	bool checkSmoothMove(int fromX, int fromY, int destX, int destY);
	bool makeSmoothMove(int fromX, int fromY, int destX, int destY);
	int characterRoute(int fromX, int fromY, int destX, int destY, int startLineIdx, int endLineIdx, int routeIdx);
	int testLine(int paramX, int paramY, int *a3, int *foundLineIdx, int *foundDataIdx);
	void _useRoute0(int idx, int curRouteIdx);
	void useRoute1(int idx, int curRouteIdx);
	void useRoute2(int idx, int curRouteIdx);

	int CALC_PROPRE(int idx);
	int CONTOURNE(int lineIdx, int lineDataIdx, int routeIdx, int destLineIdx, int destLineDataIdx, RouteItem *route);
	int CONTOURNE1(int lineIdx, int lineDataIdx, int routeIdx, int destLineIdx, int destLineDataIdx, RouteItem *route, int a8, int a9);
	bool MIRACLE(int fromX, int fromY, int lineIdx, int destLineIdx, int routeIdx);
	int GENIAL(int lineIdx, int dataIdx, int fromX, int fromY, int destX, int destY, int routerIdx, RouteItem *route);
	bool PLAN_TEST(int paramX, int paramY, int superRouteIdx, int paramStartLineIdx, int paramEndLineIdx);

public:
	RouteItem *_route;
	RouteItem *_testRoute2;

	int BOBZONE[105];
	bool BOBZONE_FLAG[105];
	ZonePItem ZONEP[106];

	LinesManager();
	~LinesManager();
	void setParent(HopkinsEngine *vm);
	void clearAll();

	void setMaxLineIdx(int idx);
	int checkInventoryHotspots(int posX, int posY);
	void addZoneLine(int idx, int fromX, int fromY, int destX, int destY, int bobZoneIdx);
	void loadLines(const Common::String &file);
	void addLine(int lineIdx, Directions direction, int fromX, int fromY, int destX, int destY);
	void initRoute();
	RouteItem *cityMapCarRoute(int x1, int y1, int x2, int y2);
	void clearAllZones();
	void resetLines();
	void resetLinesNumb();
	void resetLastLine();
	void enableZone(int idx);
	void disableZone(int idx);
	void checkZone();
	int getMouseZone();

	void CARRE_ZONE();
	RouteItem *PARCOURS2(int fromX, int fromY, int destX, int destY);
	void PACOURS_PROPRE(RouteItem *route);
};

} // End of namespace Hopkins

#endif /* HOPKINS_FONT_H */
