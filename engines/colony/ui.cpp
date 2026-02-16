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

	// Clamp position to maintain minimum distance from walls within a cell.
	// This prevents the camera from clipping through thin walls.
	// Skip clamping for walls that have any interactive feature (door, airlock,
	// stairs, etc.) so the player can reach the boundary and trigger the interaction.
	// The wall itself still blocks via checkwall; clamping only prevents camera clipping
	// on plain walls with no features.
	static const int kWallPad = 40;
	auto hasFeature = [this](int cx, int cy, int dir) -> bool {
		if (cx < 0 || cx >= 31 || cy < 0 || cy >= 31)
			return false;
		uint8 type = _mapData[cx][cy][dir][0];
		return type == kWallFeatureDoor || type == kWallFeatureAirlock ||
			type == kWallFeatureUpStairs || type == kWallFeatureDnStairs ||
			type == kWallFeatureTunnel || type == kWallFeatureElevator;
	};
	auto clampToWalls = [this, &hasFeature](Locate *p) {
		int cx = p->xindex;
		int cy = p->yindex;
		int cellMinX = cx << 8;
		int cellMinY = cy << 8;
		int cellMaxX = cellMinX + 255;
		int cellMaxY = cellMinY + 255;

		// South wall of this cell (at cellMinY boundary)
		if ((wallAt(cx, cy) & 0x01) && !hasFeature(cx, cy, kDirSouth))
			p->yloc = MAX(p->yloc, cellMinY + kWallPad);
		// North wall (at cellMaxY+1 boundary = south wall of cell above)
		if ((wallAt(cx, cy + 1) & 0x01) && !hasFeature(cx, cy, kDirNorth))
			p->yloc = MIN(p->yloc, cellMaxY - kWallPad);
		// West wall of this cell (at cellMinX boundary)
		if ((wallAt(cx, cy) & 0x02) && !hasFeature(cx, cy, kDirWest))
			p->xloc = MAX(p->xloc, cellMinX + kWallPad);
		// East wall (at cellMaxX+1 boundary = west wall of cell to right)
		if ((wallAt(cx + 1, cy) & 0x02) && !hasFeature(cx, cy, kDirEast))
			p->xloc = MIN(p->xloc, cellMaxX - kWallPad);
	};

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
		clampToWalls(pobject);
		return 0;
	};

	// tryPassThroughFeature returns: 0=blocked, 1=pass through, 2=teleported (position already set)
	auto tryFeature = [&](int dir) -> int {
		int r = tryPassThroughFeature(pobject->xindex, pobject->yindex, dir, pobject);
		if (r == 2)
			return 0; // teleported — position already updated by the feature
		if (r == 1)
			return moveTo();
		return -2; // blocked, caller handles
	};

	if (xind2 == pobject->xindex) {
		if (yind2 == pobject->yindex) {
			pobject->dx = xnew - pobject->xloc;
			pobject->dy = ynew - pobject->yloc;
			pobject->xloc = xnew;
			pobject->yloc = ynew;
			clampToWalls(pobject);
			return 0;
		}

		if (yind2 > pobject->yindex) {
			if (!(_wall[pobject->xindex][yind2] & 1))
				return moveTo();
			{ int r = tryFeature(kDirNorth); if (r != -2) return r; }
			debug("Collision South at x=%d y=%d", pobject->xindex, yind2);
			_sound->play(Sound::kBang);
			return -1;

		}

		if (!(_wall[pobject->xindex][pobject->yindex] & 1))
			return moveTo();
		{ int r = tryFeature(kDirSouth); if (r != -2) return r; }
		debug("Collision North at x=%d y=%d", pobject->xindex, pobject->yindex);
		_sound->play(Sound::kBang);
		return -1;

	}

	if (yind2 == pobject->yindex) {
		if (xind2 > pobject->xindex) {
			if (!(_wall[xind2][pobject->yindex] & 2))
				return moveTo();
			{ int r = tryFeature(kDirEast); if (r != -2) return r; }
			debug("Collision East at x=%d y=%d", xind2, pobject->yindex);
			_sound->play(Sound::kBang);
			return -1;

		}

		if (!(_wall[pobject->xindex][pobject->yindex] & 2))
			return moveTo();
		{ int r = tryFeature(kDirWest); if (r != -2) return r; }
		debug("Collision West at x=%d y=%d", pobject->xindex, pobject->yindex);
		_sound->play(Sound::kBang);
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
	if (state == 0)
		_sound->play(Sound::kDoor);


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

// Returns: 0 = blocked, 1 = can pass through normally, 2 = teleported (position already set)
int ColonyEngine::tryPassThroughFeature(int fromX, int fromY, int direction, Locate *pobject) {
	const uint8 *map = mapFeatureAt(fromX, fromY, direction);
	if (!map || map[0] == kWallFeatureNone)
		return 0;

	switch (map[0]) {
	case kWallFeatureDoor:
		if (map[1] == 0)
			return 1; // already open — pass through
		if (pobject != &_me)
			return 0; // robots can't open doors
		// DOS DoDoor: play door animation, player clicks handle to open
		if (_corePower[_coreIndex] == 0)
			return 0; // no power
		{
			const char *animName = (_level == 1 || _level == 5 || _level == 6) ? "bulkhead" : "door";
			if (!loadAnimation(animName))
				return 0;
			_animationResult = 0;
			_doorOpen = false;
			SetObjectState(2, 1); // door starts closed
			playAnimation();
			if (_animationResult) {
				setDoorState(fromX, fromY, direction, 0);
				return 1; // pass through
			}
			return 0; // player didn't open the door
		}
	case kWallFeatureAirlock:
		if (map[1] == 0)
			return 1; // already open — pass through
		if (pobject != &_me)
			return 0;
		// DOS DoAirLock: play airlock animation
		{
			if (!loadAnimation("airlock"))
				return 0;
			_animationResult = 0;
			_doorOpen = false;
			SetObjectState(2, 1); // airlock starts closed
			SetObjectState(1, 1);
			playAnimation();
			if (_animationResult) {
				setDoorState(fromX, fromY, direction, 0);
				return 1;
			}
			return 0;
		}

	case kWallFeatureUpStairs:
	case kWallFeatureDnStairs:
	case kWallFeatureTunnel: {
		if (pobject != &_me)
			return 0; // robots don't use stairs/tunnels

		// DOS GoTo(): mapdata[2]=level, [3]=xcell, [4]=ycell
		// Must copy values BEFORE loadMap, which overwrites _mapData
		const int targetMap = map[2];
		const int targetX = map[3];
		const int targetY = map[4];

		if (targetMap == 0 && targetX == 0 && targetY == 0)
			return 1; // no destination data — just pass through

		// Special cases from DOS
		if (targetMap == 100) {
			doText(78, 0); // Dimension error
			return 0;
		}

		// Play appropriate sound
		if (map[0] == kWallFeatureDnStairs)
			_sound->play(Sound::kClatter);

		// Move player to target position (preserve sub-cell offset like DOS)
		if (targetX > 0 && targetX < 31 && targetY > 0 && targetY < 31) {
			const int xmod = pobject->xloc - (pobject->xindex << 8);
			const int ymod = pobject->yloc - (pobject->yindex << 8);
			_robotArray[pobject->xindex][pobject->yindex] = 0;
			pobject->xloc = (targetX << 8) + xmod;
			pobject->xindex = targetX;
			pobject->yloc = (targetY << 8) + ymod;
			pobject->yindex = targetY;
		}

		// Load new map if target level specified
		if (targetMap > 0 && targetMap != _level) {
			loadMap(targetMap);
			// DOS: coreindex depends on level
			_coreIndex = (targetMap == 1) ? 0 : 1;
		}

		if (pobject->xindex >= 0 && pobject->xindex < 32 &&
		    pobject->yindex >= 0 && pobject->yindex < 32)
			_robotArray[pobject->xindex][pobject->yindex] = MENUM;

		debug("Level change via %s: level=%d pos=(%d,%d)",
		      map[0] == kWallFeatureUpStairs ? "upstairs" :
		      map[0] == kWallFeatureDnStairs ? "downstairs" : "tunnel",
		      _level, pobject->xindex, pobject->yindex);
		return 2; // teleported
	}

	case kWallFeatureElevator: {
		if (pobject != &_me)
			return 0;
		if (_corePower[1] == 0) {
			inform("ELEVATOR HAS NO POWER.", true);
			return 0;
		}

		// DOS DoElevator: play elevator animation with floor selection
		if (!loadAnimation("elev"))
			return 0;
		_animationResult = 0;
		_doorOpen = false;
		_elevatorFloor = _level - 1; // DOS: fl = level-1
		SetObjectOnOff(6, false);
		SetObjectOnOff(7, false);
		SetObjectOnOff(8, false);
		SetObjectOnOff(9, false);
		SetObjectOnOff(10, false);
		playAnimation();

		bool entered = (_animationResult >= 2);
		if (entered && _elevatorFloor + 1 != _level) {
			// Player selected a different floor
			int targetMap = _elevatorFloor + 1;
			const int targetX = map[3];
			const int targetY = map[4];

			if (targetX > 0 && targetX < 31 && targetY > 0 && targetY < 31) {
				_robotArray[pobject->xindex][pobject->yindex] = 0;
				pobject->xloc = (targetX << 8) + 128;
				pobject->xindex = targetX;
				pobject->yloc = (targetY << 8) + 128;
				pobject->yindex = targetY;
				pobject->ang += 128;
				pobject->look = pobject->ang;
			}

			if (targetMap > 0 && targetMap != _level) {
				loadMap(targetMap);
				_coreIndex = (targetMap == 1) ? 0 : 1;
			}

			if (pobject->xindex >= 0 && pobject->xindex < 32 &&
			    pobject->yindex >= 0 && pobject->yindex < 32)
				_robotArray[pobject->xindex][pobject->yindex] = MENUM;

			debug("Elevator: level=%d pos=(%d,%d)", _level, pobject->xindex, pobject->yindex);
			return 2; // teleported
		}
		// Player entered but stayed on same floor — turn around
		if (entered) {
			pobject->ang += 128;
			pobject->look = pobject->ang;
		}
		return 0;
	}

	default:
		return 0;
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
		_sound->play(Sound::kTeleport);
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
		_sound->play(Sound::kStars1);
		inform("I CAN SEE THROUGH IT...", true);
		break;

	case kObjToilet:
	case kObjPToilet:
		_sound->play(Sound::kToilet);
		inform("IT'S A TOILET.", true); 
		break;
	case kObjTub:
		_sound->play(Sound::kBath);
		inform("A BATHTUB. NO TIME FOR A SOAK.", true);
		break;

	case kObjSink:
		_sound->play(Sound::kSink);
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
			_sound->play(Sound::kDit);
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

void ColonyEngine::fallThroughHole() {
	// DOS tunnel(TRUE, mapdata) + GoTo(mapdata)
	// Called when player falls through a floor hole (SMHOLEFLR or LGHOLEFLR)
	const uint8 *mapdata = _mapData[_me.xindex][_me.yindex][4];
	int targetMap = mapdata[2];
	int targetX = mapdata[3];
	int targetY = mapdata[4];

	if (targetMap == 0 && targetX == 0 && targetY == 0) {
		terminateGame(false); // you're dead
		return;
	}

	// DOS tunnel(TRUE,...): power damage from falling
	int damage = -(_level << 7);
	for (int i = 0; i < 3; i++)
		_corePower[i] += damage;

	_sound->play(Sound::kClatter);

	// DOS tunnel(pt=TRUE): falling animation — nested rectangles shrinking toward
	// center, simulating falling down a shaft. White outlines on black background.
	// DOS runs 10 steps × 2 frames = 20 display frames at ~15fps = ~1.3 seconds.
	// At 60fps we use 80 frames for the same duration, paced by the frame limiter.
	{
		const int cx = (_screenR.left + _screenR.right) / 2;
		const int cy = (_screenR.top + _screenR.bottom) / 2;
		const int hw = _screenR.width() / 2;
		const int hh = _screenR.height() / 2;
		const int totalFrames = 80;
		const int maxRings = 10;

		for (int frame = 0; frame < totalFrames && !shouldQuit(); frame++) {
			_frameLimiter->startFrame();

			_gfx->fillRect(_screenR, 0); // black background

			float progress = (float)frame / totalFrames;

			// Draw nested rectangles — outer ring shrinks in, inner rings follow
			// The number of visible rings decreases as we fall deeper
			int visibleRings = maxRings - (int)(progress * (maxRings - 1));
			for (int ring = 0; ring < visibleRings; ring++) {
				// Each ring's depth combines the overall fall progress with per-ring spacing
				float depth = progress * 0.6f + (float)ring / (maxRings + 2.0f);
				if (depth >= 1.0f) break;
				float scale = 1.0f - depth;
				int rw = (int)(hw * scale);
				int rh = (int)(hh * scale);
				if (rw < 2 || rh < 2) break;
				Common::Rect r(cx - rw, cy - rh, cx + rw, cy + rh);
				_gfx->drawRect(r, 15); // white outline
			}

			_frameLimiter->delayBeforeSwap();
			_gfx->copyToScreen();
		}
	}

	// DOS GoTo(): preserve sub-cell offset, move to destination
	if (targetX > 0 && targetY > 0) {
		// Don't go if destination is occupied on same level (DOS: (!map) && robotarray check)
		if (targetMap == 0 && _robotArray[targetX][targetY] != 0)
			return;

		_robotArray[_me.xindex][_me.yindex] = 0;

		// Preserve sub-cell offset (DOS: xmod = xloc - (xindex<<8))
		int xmod = _me.xloc - (_me.xindex << 8);
		int ymod = _me.yloc - (_me.yindex << 8);
		_me.xloc = (targetX << 8) + xmod;
		_me.xindex = targetX;
		_me.yloc = (targetY << 8) + ymod;
		_me.yindex = targetY;

		_robotArray[targetX][targetY] = MENUM;
	}

	// DOS: if(map) load_mapnum(map, TRUE) — always reload when map != 0
	if (targetMap > 0)
		loadMap(targetMap);

	debug("Fell through hole: level=%d pos=(%d,%d)", _level, _me.xindex, _me.yindex);
}

void ColonyEngine::checkCenter() {
	// DOS CCenter(): check if player is standing on a floor hole or hotfoot
	if (_me.xindex < 0 || _me.xindex >= 31 || _me.yindex < 0 || _me.yindex >= 31)
		return;

	const uint8 cellType = _mapData[_me.xindex][_me.yindex][4][0];
	if (cellType == 0)
		return;

	switch (cellType) {
	case 1: { // SMHOLEFLR — small floor hole, must be near center
		// DOS: xcheck=abs(xloc-(xindex<<8)); if(xcheck>64&&xcheck<192)
		int xcheck = ABS(_me.xloc - (_me.xindex << 8));
		int ycheck = ABS(_me.yloc - (_me.yindex << 8));
		if (xcheck > 64 && xcheck < 192 && ycheck > 64 && ycheck < 192)
			fallThroughHole();
		break;
	}
	case 2: // LGHOLEFLR — large floor hole, full cell
		fallThroughHole();
		break;
	case 5: // HOTFOOT — electric floor, damages power
		// DOS: SetPower(-(5<<level),-(5<<level),-(5<<level))
		for (int i = 0; i < 3; i++)
			_corePower[i] -= (5 << _level);
		break;
	default:
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
