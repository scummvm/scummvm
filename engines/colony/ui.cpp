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
#include "common/file.h"
#include "common/events.h"
#include "graphics/palette.h"
#include "graphics/fonts/dosfont.h"
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

	_gfx->fillRect(_dashBoardRect, 0); // Black background underlying
	_gfx->fillDitherRect(_dashBoardRect, 0, 15); // Dithered 50% gray background (Black/White)
	_gfx->drawRect(_dashBoardRect, 1); // Blue outer frame

	const int shiftX = MAX(1, _dashBoardRect.width() / 8);
	const int shiftY = MAX(1, _dashBoardRect.width() / 8);
	const Common::Rect r = _dashBoardRect;

	// Bevel lines
	_gfx->drawLine(r.left, r.top, r.left + shiftX, r.top + shiftY, 0);
	_gfx->drawLine(r.right - 1, r.top, r.right - 1 - shiftX, r.top + shiftY, 0);
	_gfx->drawLine(r.left, r.bottom - 1, r.left + shiftX, r.bottom - 1 - shiftY, 0);
	_gfx->drawLine(r.right - 1, r.bottom - 1, r.right - 1 - shiftX, r.bottom - 1 - shiftY, 0);

	const Common::Rect tr(r.left + shiftX, r.top + shiftY, r.right - shiftX, r.bottom - shiftY);
	_gfx->fillDitherRect(tr, 0, 15);
	_gfx->drawRect(tr, 0);

	if (_compassRect.width() > 4 && _compassRect.height() > 4) {
		const int cx = (_compassRect.left + _compassRect.right) >> 1;
		const int cy = (_compassRect.top + _compassRect.bottom) >> 1;
		const int rx = MAX(2, (_compassRect.width() - 6) >> 1);
		const int ry = MAX(2, (_compassRect.height() - 6) >> 1);

		_gfx->fillEllipse(cx, cy, rx, ry, 15); // White background
		_gfx->drawEllipse(cx, cy, rx, ry, 1);  // Blue frame

		const int ex = cx + ((_cost[_me.look] * rx) >> 8);
		const int ey = cy - ((_sint[_me.look] * ry) >> 8);
		_gfx->drawLine(cx, cy, ex, ey, 0); // Black mark
		_gfx->drawLine(cx - 2, cy, cx + 2, cy, 0);
		_gfx->drawLine(cx, cy - 2, cx, cy + 2, 0);
	}

	if (_screenR.left > 0)
		_gfx->drawLine(_screenR.left - 1, _screenR.top, _screenR.left - 1, _screenR.bottom - 1, 15);

	if (_headsUpRect.width() > 4 && _headsUpRect.height() > 4) {
		_gfx->fillRect(_headsUpRect, 15); // White background
		_gfx->drawRect(_headsUpRect, 1); // Blue frame

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
		drawMiniMapLine(xcorner[0] - dx, ycorner[0] + dy, xcorner[1] + dx, ycorner[1] - dy, 0); // Black for walls
		drawMiniMapLine(xcorner[1] + dy, ycorner[1] + dx, xcorner[2] - dy, ycorner[2] - dx, 0);
		drawMiniMapLine(xcorner[2] + dx, ycorner[2] - dy, xcorner[3] - dx, ycorner[3] + dy, 0);
		drawMiniMapLine(xcorner[3] - dy, ycorner[3] - dx, xcorner[0] + dy, ycorner[0] + dx, 0);

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
			drawMarker(xcorner[4], ycorner[4], 1, 0);

		if (_me.yindex > 0 && !(_wall[_me.xindex][_me.yindex] & 0x01)) {
			if (hasFoodAt(_me.xindex, _me.yindex - 1))
				drawMarker(xcorner[4] + dy, ycorner[4] + dx, 1, 0);
			if (hasRobotAt(_me.xindex, _me.yindex - 1))
				drawMarker(xcorner[4] + dy, ycorner[4] + dx, 2, 0);
		}
		if (_me.xindex > 0 && !(_wall[_me.xindex][_me.yindex] & 0x02)) {
			if (hasFoodAt(_me.xindex - 1, _me.yindex))
				drawMarker(xcorner[4] - dx, ycorner[4] + dy, 1, 0);
			if (hasRobotAt(_me.xindex - 1, _me.yindex))
				drawMarker(xcorner[4] - dx, ycorner[4] + dy, 2, 0);
		}
		if (_me.yindex < 30 && !(_wall[_me.xindex][_me.yindex + 1] & 0x01)) {
			if (hasFoodAt(_me.xindex, _me.yindex + 1))
				drawMarker(xcorner[4] - dy, ycorner[4] - dx, 1, 0);
			if (hasRobotAt(_me.xindex, _me.yindex + 1))
				drawMarker(xcorner[4] - dy, ycorner[4] - dx, 2, 0);
		}
		if (_me.xindex < 30 && !(_wall[_me.xindex + 1][_me.yindex] & 0x02)) {
			if (hasFoodAt(_me.xindex + 1, _me.yindex))
				drawMarker(xcorner[4] + dx, ycorner[4] - dy, 1, 0);
			if (hasRobotAt(_me.xindex + 1, _me.yindex))
				drawMarker(xcorner[4] + dx, ycorner[4] - dy, 2, 0);
		}

		// Eye icon in the center
		const int px = MAX(4, _dashBoardRect.width() / 10);
		const int py = MAX(2, _dashBoardRect.width() / 24);
		_gfx->drawEllipse(ccenterx, ccentery, px, py, 0); // Outer frame
		_gfx->fillEllipse(ccenterx, ccentery, px / 2, py, 0); // Inner pupil
	}

	if (_powerRect.width() > 4 && _powerRect.height() > 4) {
		_gfx->fillRect(_powerRect, 15); // White background
		_gfx->drawRect(_powerRect, 1); // Blue frame
		const int barY = _powerRect.bottom - MAX(3, _powerRect.width() / 8);
		_gfx->drawLine(_powerRect.left + 1, barY, _powerRect.right - 2, barY, 0); // Black divider
	}
}

void ColonyEngine::drawCrosshair() {
	if (!_crosshair || _screenR.width() <= 0 || _screenR.height() <= 0)
		return;

	uint32 color = (_weapons > 0) ? 15 : 7;
	if (_corePower[_coreIndex] > 0)
		color = 0;

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
		doText(75, 0); // "Door is locked" or similar from T.DAT
		return false;
	case kWallFeatureAirlock:
		if (map[1] == 0)
			return true;
		if (pobject == &_me) {
			if (_unlocked) {
				setDoorState(fromX, fromY, direction, 0);
				return true;
			}
			inform("AIRLOCK IS SEALED.", true);
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
	_action0 = _mapData[x][y][4][3];
	_action1 = _mapData[x][y][4][4];
	_creature = 1;

	switch (obj.type) {
	case kObjDesk:
		if (loadAnimation("desk"))
			playAnimation();
		break;
	case kObjConsole:
		switch (_action0) {
		case 1: // Reactor console
			if (loadAnimation("reactor"))
				playAnimation();
			break;
		case 2: // Main ship controls
			if (loadAnimation("controls"))
				playAnimation();
			break;
		case 3: // Security console
			if (loadAnimation("security"))
				playAnimation();
			break;
		default:
			inform("IT DOES NOT SEEM TO BE WORKING.", true);
			break;
		}
		break;
	case kObjProjector:
		switch (_action0) {
		case 1:
			if (loadAnimation("slides"))
				playAnimation();
			break;
		case 2:
			if (loadAnimation("teleshow")) // "teleshow" matches original interaction
				playAnimation();
			break;
		default:
			inform("PROJECTOR OFFLINE", true);
			break;
		}
		break;
	case kObjPowerSuit:
		if (loadAnimation("suit"))
			playAnimation();
		break;
	case kObjTeleport:
	{
		const int targetLevelRaw = _mapData[x][y][4][2];
		const int targetLevel = (targetLevelRaw == 0) ? _level : targetLevelRaw;
		const int targetX = _action0;
		const int targetY = _action1;
		if (targetLevel >= 100 || targetX <= 0 || targetX >= 31 || targetY <= 0 || targetY >= 31) {
			inform("TELEPORTER INITIALIZATION FAILED", true);
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
		break;
	}
	case kObjDrawer:
		if (loadAnimation("vanity"))
			playAnimation();
		break;
	case kObjScreen:
		// original game shows "Full of stars" effect/text
		inform("I CAN SEE THROUGH IT...", true);
		break;
	case kObjToilet:
	case kObjPToilet:
		inform("IT'S A TOILET.", true); 
		break;
	case kObjTub:
		inform("A BATHTUB. NO TIME FOR A SOAK.", true);
		break;
	case kObjSink:
		inform("A SINK. IT'S DRY.", true);
		break;
	case kObjCryo:
		doText(_action0, 0);
		break;
	case kObjTV:
		if (_level == 1) doText(56, 0);
		else doText(16, 0);
		break;
	case kObjForkLift:
	case kObjReactor:
	case kObjBox1:
	case kObjBox2:
		inform("NEEDS FORKLIFT INTERACTION", true);
		break;
	default:
		break;
	}
}

void ColonyEngine::inform(const char *text, bool hold) {
	const char *msg[3];
	msg[0] = text;
	msg[1] = hold ? "-Press Any Key to Continue-" : nullptr;
	msg[2] = nullptr;
	printMessage(msg, hold);
}

void ColonyEngine::printMessage(const char *text[], bool hold) {
	int numLines = 0;
	int width = 0;
	Graphics::DosFont font;
	
	while (text[numLines] != nullptr) {
		int w = font.getStringWidth(text[numLines]);
		if (w > width) width = w;
		numLines++;
	}

	int px_per_inch_x = 72;
	int px_per_inch_y = 72;

	Common::Rect rr;
	rr.top = _centerY - (numLines + 1) * (px_per_inch_y / 4);
	rr.bottom = _centerY + (numLines + 1) * (px_per_inch_y / 4);
	rr.left = _centerX - width / 2 - (px_per_inch_x / 2);
	rr.right = _centerX + width / 2 + (px_per_inch_x / 2);

	_gfx->fillDitherRect(_screenR, 0, 15);
	makeMessageRect(rr);

	int start;
	int step;
	if (numLines > 1) {
		start = rr.top + (px_per_inch_y / 4) * 2;
		step = (rr.height() - (px_per_inch_y / 4) * 4) / (numLines - 1);
	} else {
		start = (rr.top + rr.bottom) / 2;
		step = 0;
	}

	for (int i = 0; i < numLines; i++) {
		_gfx->drawString(&font, text[i], (rr.left + rr.right) / 2, start + i * step, 0, Graphics::kTextAlignCenter);
	}

	_gfx->copyToScreen();

	if (hold) {
		bool waiting = true;
		while (waiting && !shouldQuit()) {
			Common::Event event;
			while (_system->getEventManager()->pollEvent(event)) {
				if (event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_LBUTTONDOWN)
					waiting = false;
			}
			_system->delayMillis(10);
		}
	}
}

void ColonyEngine::makeMessageRect(Common::Rect &rr) {
	_gfx->fillRect(rr, 15);
	_gfx->drawRect(rr, 0);
	Common::Rect inner = rr;
	inner.grow(-2);
	_gfx->drawRect(inner, 0);
}

void ColonyEngine::doText(int entry, int center) {
	Common::File file;
	if (!file.open("T.DAT")) {
		warning("doText: Could not open T.DAT");
		return;
	}

	uint32 entries = file.readUint32BE();
	if (entry < 0 || (uint32)entry >= entries) {
		warning("doText: Entry %d out of range (max %d)", entry, entries);
		file.close();
		return;
	}

	file.seek(4 + entry * 8);
	uint32 offset = file.readUint32BE();
	uint16 ch = file.readUint16BE();
	file.readUint16BE(); // lines (unused)

	if (ch == 0) {
		file.close();
		return;
	}

	byte *page = (byte *)malloc(ch + 1);
	file.seek(offset);
	file.read(page, ch);
	file.close();
	page[ch] = 0;

	// Decode: Chain XOR starting from end with '\'
	page[ch - 1] ^= '\\';
	for (int n = ch - 2; n >= 0; n--)
		page[n] ^= page[n + 1];

	Common::Array<Common::String> lineArray;
	char *p = (char *)page;
	int start = 0;
	for (int i = 0; i < ch; i++) {
		if (p[i] == '\r' || p[i] == '\n') {
			p[i] = 0;
			if (p[start]) lineArray.push_back(&p[start]);
			start = i + 1;
		}
	}
	if (start < ch && p[start]) lineArray.push_back(&p[start]);

	Graphics::DosFont font;
	int width = 0;
	for (uint i = 0; i < lineArray.size(); i++) {
		int w = font.getStringWidth(lineArray[i]);
		if (w > width) width = w;
	}
	const char *kpress = "-Press Any Key to Continue-";
	int kw = font.getStringWidth(kpress);
	if (kw > width) width = kw;
	width += 12;

	int lineheight = 14;
	int maxlines = (_screenR.height() / lineheight) - 2;
	if (maxlines > (int)lineArray.size()) maxlines = lineArray.size();

	Common::Rect r;
	r.top = _centerY - (((maxlines + 1) * lineheight / 2) + 4);
	r.bottom = _centerY + (((maxlines + 1) * lineheight / 2) + 4);
	r.left = _centerX - (width / 2);
	r.right = _centerX + (width / 2);

	_gfx->fillDitherRect(_screenR, 0, 15);
	
	// Draw shadow/border (original draws 3 frames total)
	for (int i = 0; i < 2; i++) {
		_gfx->drawRect(r, 0);
		r.translate(-1, -1);
	}
	_gfx->fillRect(r, 15);
	_gfx->drawRect(r, 0);

	for (int i = 0; i < maxlines; i++) {
		_gfx->drawString(&font, lineArray[i], r.left + 3, r.top + 4 + i * lineheight, 0);
		if (center == 2) {
			// Teletype sound effect placeholder
			_system->delayMillis(20); 
		}
	}

	_gfx->drawString(&font, (int)lineArray.size() > maxlines ? "-More-" : kpress, (r.left + r.right) / 2, r.top + 6 + maxlines * lineheight, 0, Graphics::kTextAlignCenter);
	_gfx->copyToScreen();

	// Wait for key
	bool waiting = true;
	while (waiting && !shouldQuit()) {
		Common::Event event;
		while (_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_LBUTTONDOWN)
				waiting = false;
		}
		_system->delayMillis(10);
	}

	free(page);
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

bool ColonyEngine::clipLineToRect(int &x1, int &y1, int &x2, int &y2, const Common::Rect &clip) const {
	if (clip.left >= clip.right || clip.top >= clip.bottom)
		return false;
	const int l = clip.left;
	const int r = clip.right - 1;
	const int t = clip.top;
	const int b = clip.bottom - 1;
	auto outCode = [&](int x, int y) {
		int code = 0;
		if (x < l)
			code |= 1;
		else if (x > r)
			code |= 2;
		if (y < t)
			code |= 4;
		else if (y > b)
			code |= 8;
		return code;
	};

	int c1 = outCode(x1, y1);
	int c2 = outCode(x2, y2);
	while (true) {
		if ((c1 | c2) == 0)
			return true;
		if (c1 & c2)
			return false;

		const int cOut = c1 ? c1 : c2;
		int x = 0;
		int y = 0;
		if (cOut & 8) {
			if (y2 == y1) return false;
			x = x1 + (x2 - x1) * (b - y1) / (y2 - y1);
			y = b;
		} else if (cOut & 4) {
			if (y2 == y1) return false;
			x = x1 + (x2 - x1) * (t - y1) / (y2 - y1);
			y = t;
		} else if (cOut & 2) {
			if (x2 == x1) return false;
			y = y1 + (y2 - y1) * (r - x1) / (x2 - x1);
			x = r;
		} else {
			if (x2 == x1) return false;
			y = y1 + (y2 - y1) * (l - x1) / (x2 - x1);
			x = l;
		}

		if (cOut == c1) {
			x1 = x;
			y1 = y;
			c1 = outCode(x1, y1);
		} else {
			x2 = x;
			y2 = y;
			c2 = outCode(x2, y2);
		}
	}
}

} // End of namespace Colony
