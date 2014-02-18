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

#define INVALID_LINE_VALUE 1300

#define MAX_LINES 400

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

struct ZoneItem {
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

	bool _forceHideText;
	int _hotspotTextColor;
	int _pathFindingMaxDepth;
	SmoothItem _smoothRoute[4000];
	Directions _smoothMoveDirection;
	LigneZoneItem _zoneLine[MAX_LINES+1];
	SegmentItem _segment[101];
	int _currentSegmentId;
	int _maxLineIdx;
	int _lastLine;
	int _newLineIdx;
	int _newLineDataIdx;
	int _newRouteIdx;
	int _newPosX;
	int _newPosY;
	int _oldMouseX, _oldMouseY;
	int _oldRouteFromX;
	int _oldRouteFromY;
	int _oldRouteDestX;
	int _oldRouteDestY;
	int _oldZoneNum;

	byte *_largeBuf;
	RouteItem *_testRoute0;
	RouteItem *_testRoute1;
	int16 *_lineBuf;
	RouteItem _bestRoute[8001];
	int _zoneSkipCount;
	int _oldMouseZoneId;

	int avoidObstacle(int lineIdx, int lineDataIdx, int routeIdx, int destLineIdx, int destLineDataIdx, RouteItem *route);
	int avoidObstacleOnSegment(int lineIdx, int lineDataIdx, int routeIdx, int destLineIdx, int destLineDataIdx, RouteItem *route, int startLineIdx, int endLineIdx);
	int checkInventoryHotspotsRow(int posX, int minZoneNum, bool lastRow);
	void removeZoneLine(int idx);
	void removeLine(int idx);
	int checkCollision(int xp, int yp);
	bool checkCollisionLine(int xp, int yp, int *foundDataIdx, int *foundLineIdx, int startLineIdx, int endLineIdx);
	bool checkSmoothMove(int fromX, int fromY, int destX, int destY);
	bool makeSmoothMove(int fromX, int fromY, int destX, int destY);
	int characterRoute(int fromX, int fromY, int destX, int destY, int startLineIdx, int endLineIdx, int routeIdx);
	int testLine(int paramX, int paramY, int *testValue, int *foundLineIdx, int *foundDataIdx);
	void useRoute0(int idx, int curRouteIdx);
	void useRoute1(int idx, int curRouteIdx);
	void useRoute2(int idx, int curRouteIdx);
	int computeYSteps(int idx);
	int computeRouteIdx(int lineIdx, int dataIdx, int fromX, int fromY, int destX, int destY, int routerIdx, RouteItem *route);

	bool MIRACLE(int fromX, int fromY, int lineIdx, int destLineIdx, int routeIdx);
	bool PLAN_TEST(int paramX, int paramY, int superRouteIdx, int paramStartLineIdx, int paramEndLineIdx);

public:
	RouteItem *_route;
	RouteItem *_testRoute2;

	int _bobZone[105];
	bool _bobZoneFl[105];
	ZoneItem _zone[106];
	SquareZoneItem _squareZone[101];
	LigneItem _lineItem[MAX_LINES];
	int _linesNumb;

	LinesManager(HopkinsEngine *vm);
	~LinesManager();
	void clearAll();

	void setMaxLineIdx(int idx);
	int checkInventoryHotspots(int posX, int posY);
	void addZoneLine(int idx, int fromX, int fromY, int destX, int destY, int bobZoneIdx);
	void loadLines(const Common::String &file);
	void addLine(int lineIdx, Directions direction, int fromX, int fromY, int destX, int destY);
	void initRoute();
	RouteItem *findRoute(int fromX, int fromY, int destX, int destY);
	RouteItem *cityMapCarRoute(int x1, int y1, int x2, int y2);
	void clearAllZones();
	void initSquareZones();
	void resetLines();
	void resetLinesNumb();
	void resetLastLine();
	void enableZone(int idx);
	void disableZone(int idx);
	void checkZone();
	int getMouseZone();
	void optimizeRoute(RouteItem *route);
};

} // End of namespace Hopkins

#endif /* HOPKINS_FONT_H */
