/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "colony/colony.h"
#include "colony/gfx.h"
#include "common/system.h"
#include "common/util.h"
#include "common/debug.h"
#include "graphics/palette.h"
#include <math.h>

namespace Colony {

void ColonyEngine::updateViewportLayout() {
	auto makeSafeRect = [](int left, int top, int right, int bottom) {
		if (right < left)
			right = left;
		if (bottom < top)
			bottom = top;
		return Common::Rect(left, top, right, bottom);
	};

	int dashWidth = 0;
	if (_showDashBoard) {
		dashWidth = CLIP<int>(_width / 6, 72, 140);
		if (_width - dashWidth < 160)
			dashWidth = 0;
	}

	_screenR = makeSafeRect(dashWidth, 0, _width, _height);
	_clip = _screenR;
	_centerX = (_screenR.left + _screenR.right) >> 1;
	_centerY = (_screenR.top + _screenR.bottom) >> 1;

	_dashBoardRect = makeSafeRect(0, 0, dashWidth, _height);
	if (dashWidth == 0) {
		_compassRect = Common::Rect(0, 0, 0, 0);
		_headsUpRect = Common::Rect(0, 0, 0, 0);
		_powerRect = Common::Rect(0, 0, 0, 0);
		return;
	}

	const int pad = 2;
	const int unit = MAX(8, (dashWidth - (pad * 2)) / 4);
	const int blockLeft = pad;
	const int blockRight = MIN(dashWidth - pad, blockLeft + unit * 4);

	const int compassBottom = _height - MAX(2, unit / 4);
	const int compassTop = MAX(pad, compassBottom - unit * 4);
	_compassRect = makeSafeRect(blockLeft, compassTop, blockRight, compassBottom);

	const int headsUpBottom = _compassRect.top - MAX(2, unit / 4) - 4;
	const int headsUpTop = headsUpBottom - unit * 4;
	_headsUpRect = makeSafeRect(blockLeft, MAX(pad, headsUpTop), blockRight, MAX(pad, headsUpBottom));

	_powerRect = makeSafeRect(blockLeft, pad, blockRight, _headsUpRect.top - 4);
}

void ColonyEngine::drawDashboardStep1() {
	if (_dashBoardRect.width() <= 0 || _dashBoardRect.height() <= 0)
		return;

	const int kFWALLType = 48;
	const uint32 panelBg = 24;
	const uint32 frame = 190;
	const uint32 accent = 220;
	const uint32 mark = 255;
	const uint32 miniMapObj = 235;
	const uint32 miniMapActor = 255;

	_gfx->fillRect(_dashBoardRect, panelBg);
	_gfx->drawRect(_dashBoardRect, frame);
	if (_screenR.left > 0)
		_gfx->drawLine(_screenR.left - 1, _screenR.top, _screenR.left - 1, _screenR.bottom - 1, mark);

	if (_compassRect.width() > 4 && _compassRect.height() > 4) {
		const int cx = (_compassRect.left + _compassRect.right) >> 1;
		const int cy = (_compassRect.top + _compassRect.bottom) >> 1;
		const int rx = MAX(2, (_compassRect.width() - 6) >> 1);
		const int ry = MAX(2, (_compassRect.height() - 6) >> 1);
		const int irx = MAX(1, rx - 2);
		const int iry = MAX(1, ry - 2);
		auto drawEllipse = [&](int erx, int ery, uint32 color) {
			const int segments = 48;
			int px = cx + erx;
			int py = cy;
			for (int i = 1; i <= segments; i++) {
				const double t = (6.28318530717958647692 * (double)i) / (double)segments;
				const int x = cx + (int)((double)erx * cos(t));
				const int y = cy + (int)((double)ery * sin(t));
				_gfx->drawLine(px, py, x, y, color);
				px = x;
				py = y;
			}
		};
		drawEllipse(rx, ry, frame);
		drawEllipse(irx, iry, accent);

		const int ex = cx + ((_cost[_me.look] * rx) >> 8);
		const int ey = cy - ((_sint[_me.look] * ry) >> 8);
		_gfx->drawLine(cx, cy, ex, ey, mark);
		_gfx->drawLine(cx - 2, cy, cx + 2, cy, accent);
		_gfx->drawLine(cx, cy - 2, cx, cy + 2, accent);
	}

	if (_headsUpRect.width() > 4 && _headsUpRect.height() > 4) {
		_gfx->drawRect(_headsUpRect, frame);
		const Common::Rect miniMapClip(_headsUpRect.left + 1, _headsUpRect.top + 1, _headsUpRect.right - 1, _headsUpRect.bottom - 1);
		auto drawMiniMapLine = [&](int x1, int y1, int x2, int y2, uint32 color) {
			if (clipLineToRect(x1, y1, x2, y2, miniMapClip))
				_gfx->drawLine(x1, y1, x2, y2, color);
		};

		const int lExtBase = _dashBoardRect.width() >> 1;
		int lExt = lExtBase + (lExtBase >> 1);
		if (lExt & 1)
			lExt--;
		const int sExt = lExt >> 1;
		const int xloc = (lExt * ((_me.xindex << 8) - _me.xloc)) >> 8;
		const int yloc = (lExt * ((_me.yindex << 8) - _me.yloc)) >> 8;
		const int ccenterx = (_headsUpRect.left + _headsUpRect.right) >> 1;
		const int ccentery = (_headsUpRect.top + _headsUpRect.bottom) >> 1;
		const int tsin = _sint[_me.look];
		const int tcos = _cost[_me.look];

		int xcorner[6];
		int ycorner[6];
		xcorner[0] = ccenterx + (((long)xloc * tsin - (long)yloc * tcos) >> 8);
		ycorner[0] = ccentery - (((long)yloc * tsin + (long)xloc * tcos) >> 8);
		xcorner[1] = ccenterx + (((long)(xloc + lExt) * tsin - (long)yloc * tcos) >> 8);
		ycorner[1] = ccentery - (((long)yloc * tsin + (long)(xloc + lExt) * tcos) >> 8);
		xcorner[2] = ccenterx + (((long)(xloc + lExt) * tsin - (long)(yloc + lExt) * tcos) >> 8);
		ycorner[2] = ccentery - (((long)(yloc + lExt) * tsin + (long)(xloc + lExt) * tcos) >> 8);
		xcorner[3] = ccenterx + (((long)xloc * tsin - (long)(yloc + lExt) * tcos) >> 8);
		ycorner[3] = ccentery - (((long)(yloc + lExt) * tsin + (long)xloc * tcos) >> 8);
		xcorner[4] = ccenterx + (((long)(xloc + sExt) * tsin - (long)(yloc + sExt) * tcos) >> 8);
		ycorner[4] = ccentery - (((long)(yloc + sExt) * tsin + (long)(xloc + sExt) * tcos) >> 8);
		xcorner[5] = ccenterx + (((long)(xloc + sExt) * tsin - (long)yloc * tcos) >> 8);
		ycorner[5] = ccentery - (((long)yloc * tsin + (long)(xloc + sExt) * tcos) >> 8);

		const int dx = xcorner[1] - xcorner[0];
		const int dy = ycorner[0] - ycorner[1];
		drawMiniMapLine(xcorner[0] - dx, ycorner[0] + dy, xcorner[1] + dx, ycorner[1] - dy, accent);
		drawMiniMapLine(xcorner[1] + dy, ycorner[1] + dx, xcorner[2] - dy, ycorner[2] - dx, accent);
		drawMiniMapLine(xcorner[2] + dx, ycorner[2] - dy, xcorner[3] - dx, ycorner[3] + dy, accent);
		drawMiniMapLine(xcorner[3] - dy, ycorner[3] - dx, xcorner[0] + dy, ycorner[0] + dx, accent);

		auto drawMarker = [&](int x, int y, int halfSize, uint32 color) {
			const int l = MAX<int>(_headsUpRect.left + 1, x - halfSize);
			const int t = MAX<int>(_headsUpRect.top + 1, y - halfSize);
			const int r = MIN<int>(_headsUpRect.right - 1, x + halfSize + 1);
			const int b = MIN<int>(_headsUpRect.bottom - 1, y + halfSize + 1);
			if (l >= r || t >= b)
				return;
			_gfx->drawRect(Common::Rect(l, t, r, b), color);
		};

		auto hasRobotAt = [&](int x, int y) -> bool {
			if (x < 0 || x >= 32 || y < 0 || y >= 32)
				return false;
			return _robotArray[x][y] != 0;
		};
		auto hasFoodAt = [&](int x, int y) -> bool {
			if (x < 0 || x >= 32 || y < 0 || y >= 32)
				return false;
			const uint8 num = _foodArray[x][y];
			if (num == 0)
				return false;
			if (num <= _objects.size())
				return _objects[num - 1].type < kFWALLType;
			return true;
		};

		if (hasFoodAt(_me.xindex, _me.yindex))
			drawMarker(xcorner[4], ycorner[4], 1, miniMapObj);

		if (_me.yindex > 0 && !(_wall[_me.xindex][_me.yindex] & 0x01)) {
			if (hasFoodAt(_me.xindex, _me.yindex - 1))
				drawMarker(xcorner[4] + dy, ycorner[4] + dx, 1, miniMapObj);
			if (hasRobotAt(_me.xindex, _me.yindex - 1))
				drawMarker(xcorner[4] + dy, ycorner[4] + dx, 2, miniMapActor);
		}
		if (_me.xindex > 0 && !(_wall[_me.xindex][_me.yindex] & 0x02)) {
			if (hasFoodAt(_me.xindex - 1, _me.yindex))
				drawMarker(xcorner[4] - dx, ycorner[4] + dy, 1, miniMapObj);
			if (hasRobotAt(_me.xindex - 1, _me.yindex))
				drawMarker(xcorner[4] - dx, ycorner[4] + dy, 2, miniMapActor);
		}
		if (_me.yindex < 30 && !(_wall[_me.xindex][_me.yindex + 1] & 0x01)) {
			if (hasFoodAt(_me.xindex, _me.yindex + 1))
				drawMarker(xcorner[4] - dy, ycorner[4] - dx, 1, miniMapObj);
			if (hasRobotAt(_me.xindex, _me.yindex + 1))
				drawMarker(xcorner[4] - dy, ycorner[4] - dx, 2, miniMapActor);
		}
		if (_me.xindex < 30 && !(_wall[_me.xindex + 1][_me.yindex] & 0x02)) {
			if (hasFoodAt(_me.xindex + 1, _me.yindex))
				drawMarker(xcorner[4] + dx, ycorner[4] - dy, 1, miniMapObj);
			if (hasRobotAt(_me.xindex + 1, _me.yindex))
				drawMarker(xcorner[4] + dx, ycorner[4] - dy, 2, miniMapActor);
		}

		drawMarker(ccenterx, ccentery, 2, mark);
		drawMarker(ccenterx, ccentery, 1, accent);
	}

	if (_powerRect.width() > 4 && _powerRect.height() > 4) {
		_gfx->drawRect(_powerRect, frame);
		const int barY = _powerRect.bottom - MAX(3, _powerRect.width() / 8);
		_gfx->drawLine(_powerRect.left + 1, barY, _powerRect.right - 2, barY, accent);
	}
}

void ColonyEngine::drawCrosshair() {
	if (!_crosshair || _screenR.width() <= 0 || _screenR.height() <= 0)
		return;

	const uint32 color = (_weapons > 0) ? 255 : 220;
	const int cx = _centerX;
	const int cy = _centerY;
	const int qx = MAX(2, _screenR.width() / 32);
	const int qy = MAX(2, _screenR.height() / 32);
	const int fx = (qx * 3) >> 1;
	const int fy = (qy * 3) >> 1;
	auto drawCrossLine = [&](int x1, int y1, int x2, int y2) {
		if (clipLineToRect(x1, y1, x2, y2, _screenR))
			_gfx->drawLine(x1, y1, x2, y2, color);
	};

	if (_weapons > 0) {
		const int yTop = _insight ? (cy - qy) : (cy - fy);
		const int yBottom = _insight ? (cy + qy) : (cy + fy);

		drawCrossLine(cx - qx, yTop, cx - fx, yTop);
		drawCrossLine(cx - fx, yTop, cx - fx, yBottom);
		drawCrossLine(cx - fx, yBottom, cx - qx, yBottom);
		drawCrossLine(cx + qx, yTop, cx + fx, yTop);
		drawCrossLine(cx + fx, yTop, cx + fx, yBottom);
		drawCrossLine(cx + fx, yBottom, cx + qx, yBottom);
		_insight = false;
	} else {
		drawCrossLine(cx - qx, cy, cx + qx, cy);
		drawCrossLine(cx, cy - qy, cx, cy + qy);
	}
}

int ColonyEngine::occupiedObjectAt(int x, int y, const Locate *pobject) {
	if (x < 0 || x >= 32 || y < 0 || y >= 32)
		return -1;
	const int rnum = _robotArray[x][y];
	if (rnum <= 0)
		return 0;
	if (pobject == &_me && rnum <= (int)_objects.size()) {
		Thing &obj = _objects[rnum - 1];
		if (obj.type <= BASEOBJECT)
			obj.where.look = obj.where.ang = _me.ang + 128;
	}
	return rnum;
}

int ColonyEngine::checkwall(int xnew, int ynew, Locate *pobject) {
	const int xind2 = xnew >> 8;
	const int yind2 = ynew >> 8;
	_change = true;

	auto occupied = [&]() -> int {
		return occupiedObjectAt(xind2, yind2, pobject);
	};
	auto moveTo = [&]() -> int {
		const int rnum = occupied();
		if (rnum)
			return rnum;
		pobject->yindex = yind2;
		pobject->xindex = xind2;
		pobject->dx = xnew - pobject->xloc;
		pobject->dy = ynew - pobject->yloc;
		pobject->xloc = xnew;
		pobject->yloc = ynew;
		return 0;
	};

	if (xind2 == pobject->xindex) {
		if (yind2 == pobject->yindex) {
			pobject->dx = xnew - pobject->xloc;
			pobject->dy = ynew - pobject->yloc;
			pobject->xloc = xnew;
			pobject->yloc = ynew;
			return 0;
		}

		if (yind2 > pobject->yindex) {
			if (!(_wall[pobject->xindex][yind2] & 1))
				return moveTo();
			if (tryPassThroughFeature(pobject->xindex, pobject->yindex, kDirNorth, pobject))
				return moveTo();
			debug("Collision South at x=%d y=%d", pobject->xindex, yind2);
			return -1;
		}

		if (!(_wall[pobject->xindex][pobject->yindex] & 1))
			return moveTo();
		if (tryPassThroughFeature(pobject->xindex, pobject->yindex, kDirSouth, pobject))
			return moveTo();
		debug("Collision North at x=%d y=%d", pobject->xindex, pobject->yindex);
		return -1;
	}

	if (yind2 == pobject->yindex) {
		if (xind2 > pobject->xindex) {
			if (!(_wall[xind2][pobject->yindex] & 2))
				return moveTo();
			if (tryPassThroughFeature(pobject->xindex, pobject->yindex, kDirEast, pobject))
				return moveTo();
			debug("Collision East at x=%d y=%d", xind2, pobject->yindex);
			return -1;
		}

		if (!(_wall[pobject->xindex][pobject->yindex] & 2))
			return moveTo();
		if (tryPassThroughFeature(pobject->xindex, pobject->yindex, kDirWest, pobject))
			return moveTo();
		debug("Collision West at x=%d y=%d", pobject->xindex, pobject->yindex);
		return -1;
	}

	// Diagonal
	if (xind2 > pobject->xindex) {
		if (yind2 > pobject->yindex) {
			if ((_wall[pobject->xindex][yind2] & 1) || (_wall[xind2][pobject->yindex] & 2) || (_wall[xind2][yind2] & 3)) {
				debug("Collision Diagonal SE");
				return -1;
			}
		} else {
			if ((_wall[pobject->xindex][pobject->yindex] & 1) || (_wall[xind2][yind2] & 2) || (_wall[xind2][pobject->yindex] & 3)) {
				debug("Collision Diagonal NE");
				return -1;
			}
		}
	} else {
		if (yind2 > pobject->yindex) {
			if ((_wall[xind2][yind2] & 1) || (_wall[pobject->xindex][pobject->yindex] & 2) || (_wall[pobject->xindex][yind2] & 3)) {
				debug("Collision Diagonal SW");
				return -1;
			}
		} else {
			if ((_wall[xind2][pobject->yindex] & 1) || (_wall[pobject->xindex][yind2] & 2) || (_wall[pobject->xindex][pobject->yindex] & 3)) {
				debug("Collision Diagonal NW");
				return -1;
			}
		}
	}

	return moveTo();
}

bool ColonyEngine::setDoorState(int x, int y, int direction, int state) {
	if (x < 0 || x >= 31 || y < 0 || y >= 31 || direction < 0 || direction > 3)
		return false;

	const uint8 wallType = _mapData[x][y][direction][0];
	if (wallType != kWallFeatureDoor && wallType != kWallFeatureAirlock)
		return false;

	_mapData[x][y][direction][1] = (uint8)state;

	int nx = x;
	int ny = y;
	int opposite = -1;
	switch (direction) {
	case kDirNorth:
		ny += 1;
		opposite = kDirSouth;
		break;
	case kDirEast:
		nx += 1;
		opposite = kDirWest;
		break;
	case kDirWest:
		nx -= 1;
		opposite = kDirEast;
		break;
	case kDirSouth:
		ny -= 1;
		opposite = kDirNorth;
		break;
	default:
		return true;
	}

	if (nx >= 0 && nx < 31 && ny >= 0 && ny < 31 && opposite >= 0) {
		if (_mapData[nx][ny][opposite][0] == wallType)
			_mapData[nx][ny][opposite][1] = (uint8)state;
	}

	return true;
}

int ColonyEngine::openAdjacentDoors(int x, int y) {
	int opened = 0;
	for (int dir = 0; dir < 4; dir++) {
		const uint8 *map = mapFeatureAt(x, y, dir);
		if (!map)
			continue;
		if (map[0] == kWallFeatureDoor && map[1] != 0) {
			if (setDoorState(x, y, dir, 0))
				opened++;
		}
	}
	return opened;
}

bool ColonyEngine::tryPassThroughFeature(int fromX, int fromY, int direction, Locate *pobject) {
	const uint8 *map = mapFeatureAt(fromX, fromY, direction);
	if (!map || map[0] == kWallFeatureNone)
		return false;

	switch (map[0]) {
	case kWallFeatureDoor:
		if (map[1] == 0)
			return true;
		if (pobject != &_me)
			return false;
		if (_hasKeycard || _unlocked) {
			setDoorState(fromX, fromY, direction, 0);
			return true;
		}
		return false;
	case kWallFeatureAirlock:
		if (map[1] == 0)
			return true;
		if (pobject == &_me && _unlocked) {
			setDoorState(fromX, fromY, direction, 0);
			return true;
		}
		return false;
	default:
		return false;
	}
}

void ColonyEngine::interactWithObject(int objNum) {
	if (objNum <= 0 || objNum > (int)_objects.size())
		return;

	const Thing &obj = _objects[objNum - 1];
	if (!obj.alive)
		return;

	const int x = CLIP<int>(obj.where.xindex, 0, 30);
	const int y = CLIP<int>(obj.where.yindex, 0, 30);
	const int action0 = _mapData[x][y][4][3];
	const int action1 = _mapData[x][y][4][4];

	switch (obj.type) {
	case kObjDesk:
		if (!_hasKeycard) {
			_hasKeycard = true;
			debug("CCommand: DESK granted keycard");
		} else {
			debug("CCommand: DESK action (%d, %d)", action0, action1);
		}
		break;
	case kObjConsole:
		switch (action0) {
		case 1:
			debug("CCommand: CONSOLE reactor (%d)", action1);
			break;
		case 2:
		{
			const int opened = openAdjacentDoors(_me.xindex, _me.yindex);
			debug("CCommand: CONSOLE controls opened %d nearby doors", opened);
			break;
		}
		case 3:
			_unlocked = true;
			debug("CCommand: CONSOLE security unlocked");
			break;
		default:
			debug("CCommand: CONSOLE action=%d", action0);
			break;
		}
		break;
	case kObjProjector:
		switch (action0) {
		case 1:
			debug("CCommand: PROJECTOR creatures");
			break;
		case 2:
			debug("CCommand: PROJECTOR teleporters");
			break;
		default:
			debug("CCommand: PROJECTOR action=%d", action0);
			break;
		}
		break;
	case kObjPowerSuit:
		_weapons = MAX(_weapons, 1);
		_crosshair = true;
		debug("CCommand: POWERSUIT");
		break;
	case kObjTeleport:
	{
		const int targetLevelRaw = _mapData[x][y][4][2];
		const int targetLevel = (targetLevelRaw == 0) ? _level : targetLevelRaw;
		const int targetX = _mapData[x][y][4][3];
		const int targetY = _mapData[x][y][4][4];
		if (targetLevel >= 100 || targetX <= 0 || targetX >= 31 || targetY <= 0 || targetY >= 31) {
			debug("CCommand: TELEPORT ignored invalid target L%d (%d,%d)", targetLevelRaw, targetX, targetY);
			break;
		}
		if (targetLevel != _level)
			loadMap(targetLevel);
		const int oldX = _me.xindex;
		const int oldY = _me.yindex;
		_me.xindex = targetX;
		_me.yindex = targetY;
		_me.xloc = (targetX << 8) + 128;
		_me.yloc = (targetY << 8) + 128;
		_me.ang = _me.look;
		if (oldX >= 0 && oldX < 32 && oldY >= 0 && oldY < 32)
			_robotArray[oldX][oldY] = 0;
		if (_me.xindex >= 0 && _me.xindex < 32 && _me.yindex >= 0 && _me.yindex < 32)
			_robotArray[_me.xindex][_me.yindex] = MENUM;
		_change = true;
		debug("CCommand: TELEPORT to L%d (%d,%d)", targetLevel, targetX, targetY);
		break;
	}
	case kObjDrawer:
		if (!_hasKeycard) {
			_hasKeycard = true;
			debug("CCommand: DRAWER vanity=%d (picked keycard)", action0);
		} else {
			debug("CCommand: DRAWER vanity=%d", action0);
		}
		break;
	case kObjScreen:
		debug("CCommand: SCREEN");
		break;
	case kObjToilet:
	case kObjPToilet:
		debug("CCommand: TOILET");
		break;
	case kObjTub:
		debug("CCommand: TUB");
		break;
	case kObjSink:
		debug("CCommand: SINK");
		break;
	case kObjCryo:
		debug("CCommand: CRYO text=%d", action0);
		break;
	case kObjTV:
		debug("CCommand: TV level=%d", _level);
		break;
	case kObjForkLift:
	case kObjReactor:
	case kObjBox1:
	case kObjBox2:
		debug("CCommand: object type %d requires forklift/reactor flow", obj.type);
		break;
	case kObjPlant:
	case kObjCChair:
	case kObjBed:
	case kObjTable:
	case kObjCouch:
	case kObjChair:
	case kObjBBed:
	case kObjFWall:
	case kObjCWall:
		// Matches DOS CCommand where these objects are non-interactive blockers.
		break;
	default:
		debug("CCommand: object type %d", obj.type);
		break;
	}
}

void ColonyEngine::cCommand(int xnew, int ynew, bool allowInteraction) {
	if (_me.xindex >= 0 && _me.xindex < 32 && _me.yindex >= 0 && _me.yindex < 32)
		_robotArray[_me.xindex][_me.yindex] = 0;

	const int robot = checkwall(xnew, ynew, &_me);
	if (robot > 0 && allowInteraction)
		interactWithObject(robot);

	if (_me.xindex >= 0 && _me.xindex < 32 && _me.yindex >= 0 && _me.yindex < 32)
		_robotArray[_me.xindex][_me.yindex] = MENUM;
}

} // End of namespace Colony
