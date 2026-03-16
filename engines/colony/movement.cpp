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
#include "colony/renderer.h"
#include "common/system.h"
#include "common/debug.h"
#include <math.h>

namespace Colony {

static const int kTunnelXT[] = {
	4, 8, 8, 15, 16, 16, 16, 17, 20, 22,
	22, 22, 25, 25, 28, 25, 25, 23, 20, 18,
	18, 16, 14, 14, 13, 12, 10, 9, 7, 3,
	1, 0, 0, -2, -6, -8, -10, -12, -14, -16,
	-20, -20, -23, -20, -14, -8, -4, 0, 0, 0,
	0, 0, 0, 0, 0, 0
};

static const int kTunnelYT[] = {
	2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
	7, 8, 9, 10, 11, 12, 11, 9, 7, 6,
	5, 4, 3, 2, 1, 1, 0, 0, -1, -2,
	-3, -4, -5, -6, -6, -6, -7, -7, -8, -8,
	-9, -9, -10, -11, -12, -12, -13, -14, -12, -10,
	-7, -4, -2, 0, 0, 0, 0, 0, 0, 0
};

static const int kTunnelST[] = {
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2
};

static const int kTunnelStraight[60] = {0};

static uint32 packTunnelMacColor(const uint16 rgb[3]) {
	return 0xFF000000 | ((uint32)(rgb[0] >> 8) << 16) |
	       ((uint32)(rgb[1] >> 8) << 8) | (uint32)(rgb[2] >> 8);
}

static void fillTunnelPattern(Renderer *gfx, const Common::Rect &rect, uint32 fg, uint32 bg, int pattern) {
	if (rect.isEmpty())
		return;

	switch (pattern) {
	case 4:
		gfx->fillRect(rect, fg);
		return;
	case 0:
		gfx->fillRect(rect, bg);
		return;
	default:
		break;
	}

	if (pattern == 3)
		gfx->fillRect(rect, fg);
	else
		gfx->fillRect(rect, bg);

	for (int y = rect.top; y < rect.bottom; ++y) {
		const int py = y - rect.top;

		switch (pattern) {
		case 1: {
			const int start = rect.left + ((py & 1) ? 2 : 0);
			for (int x = start; x < rect.right; x += 4)
				gfx->setPixel(x, y, fg);
			break;
		}
		case 2: {
			const int start = rect.left + (py & 1);
			for (int x = start; x < rect.right; x += 2)
				gfx->setPixel(x, y, fg);
			break;
		}
		case 3: {
			const int start = rect.left + ((py & 1) ? 2 : 0);
			for (int x = start; x < rect.right; x += 4)
				gfx->setPixel(x, y, bg);
			break;
		}
		default:
			break;
		}
	}
}

static void projectTunnelPoint(const Common::Rect &rect, int pnt[2], int rox, int roy) {
	if (roy <= 0)
		roy = 1;

	const int centerX = (rect.left + rect.right) >> 1;
	const int centerY = (rect.top + rect.bottom) >> 1;
	long p = centerX + (((long)rox) * 512) / roy;

	if (p < -32000)
		p = -32000;
	else if (p > 32000)
		p = 32000;

	pnt[0] = (int)p;
	pnt[1] = centerY - (((160 * 128) / roy) << 1);
}

enum {
	kTunnelClipLeft = 1,
	kTunnelClipRight = 2,
	kTunnelClipTop = 4,
	kTunnelClipBottom = 8
};

static int tunnelClipCode(const Common::Rect &rect, int x, int y) {
	int code = 0;

	if (x < rect.left)
		code |= kTunnelClipLeft;
	else if (x >= rect.right)
		code |= kTunnelClipRight;

	if (y < rect.top)
		code |= kTunnelClipTop;
	else if (y >= rect.bottom)
		code |= kTunnelClipBottom;

	return code;
}

static void drawTunnelLine(Renderer *gfx, const Common::Rect &rect, int x1, int y1, int x2, int y2, uint32 color) {
	if (rect.isEmpty())
		return;

	int code1 = tunnelClipCode(rect, x1, y1);
	int code2 = tunnelClipCode(rect, x2, y2);

	while (true) {
		if (!(code1 | code2)) {
			gfx->drawLine(x1, y1, x2, y2, color);
			return;
		}

		if (code1 & code2)
			return;

		const int codeOut = code1 ? code1 : code2;
		double x = 0.0;
		double y = 0.0;

		if (codeOut & kTunnelClipBottom) {
			if (y2 == y1)
				return;
			y = rect.bottom - 1;
			x = x1 + (double)(x2 - x1) * (y - y1) / (double)(y2 - y1);
		} else if (codeOut & kTunnelClipTop) {
			if (y2 == y1)
				return;
			y = rect.top;
			x = x1 + (double)(x2 - x1) * (y - y1) / (double)(y2 - y1);
		} else if (codeOut & kTunnelClipRight) {
			if (x2 == x1)
				return;
			x = rect.right - 1;
			y = y1 + (double)(y2 - y1) * (x - x1) / (double)(x2 - x1);
		} else {
			if (x2 == x1)
				return;
			x = rect.left;
			y = y1 + (double)(y2 - y1) * (x - x1) / (double)(x2 - x1);
		}

		const int ix = (int)round(x);
		const int iy = (int)round(y);

		if (codeOut == code1) {
			x1 = ix;
			y1 = iy;
			code1 = tunnelClipCode(rect, x1, y1);
		} else {
			x2 = ix;
			y2 = iy;
			code2 = tunnelClipCode(rect, x2, y2);
		}
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
		if (obj.type <= kBaseObject)
			obj.where.look = obj.where.ang = _me.ang + 128;
	}
	return rnum;
}

bool ColonyEngine::hasInteractiveWallFeature(int cx, int cy, int dir) const {
	if (cx < 0 || cx >= 31 || cy < 0 || cy >= 31)
		return false;
	uint8 type = _mapData[cx][cy][dir][0];
	return type == kWallFeatureDoor || type == kWallFeatureAirlock ||
		type == kWallFeatureUpStairs || type == kWallFeatureDnStairs ||
		type == kWallFeatureTunnel || type == kWallFeatureElevator;
}

void ColonyEngine::clampToWalls(Locate *p) {
	static const int kWallPad = 40;
	int cx = p->xindex;
	int cy = p->yindex;
	int cellMinX = cx << 8;
	int cellMinY = cy << 8;
	int cellMaxX = cellMinX + 255;
	int cellMaxY = cellMinY + 255;

	// South wall of this cell (at cellMinY boundary)
	if ((wallAt(cx, cy) & 0x01) && !hasInteractiveWallFeature(cx, cy, kDirSouth))
		p->yloc = MAX(p->yloc, cellMinY + kWallPad);
	// North wall (at cellMaxY+1 boundary = south wall of cell above)
	if ((wallAt(cx, cy + 1) & 0x01) && !hasInteractiveWallFeature(cx, cy, kDirNorth))
		p->yloc = MIN(p->yloc, cellMaxY - kWallPad);
	// West wall of this cell (at cellMinX boundary)
	if ((wallAt(cx, cy) & 0x02) && !hasInteractiveWallFeature(cx, cy, kDirWest))
		p->xloc = MAX(p->xloc, cellMinX + kWallPad);
	// East wall (at cellMaxX+1 boundary = west wall of cell to right)
	if ((wallAt(cx + 1, cy) & 0x02) && !hasInteractiveWallFeature(cx, cy, kDirEast))
		p->xloc = MIN(p->xloc, cellMaxX - kWallPad);
}

int ColonyEngine::checkwallMoveTo(int xnew, int ynew, int xind2, int yind2, Locate *pobject, uint8 trailCode) {
	const int rnum = occupiedObjectAt(xind2, yind2, pobject);
	if (rnum)
		return rnum;
	if (trailCode != 0 && pobject->type == kMeNum &&
	    pobject->xindex >= 0 && pobject->xindex < 32 &&
	    pobject->yindex >= 0 && pobject->yindex < 32)
		_dirXY[pobject->xindex][pobject->yindex] = trailCode;
	pobject->yindex = yind2;
	pobject->xindex = xind2;
	pobject->dx = xnew - pobject->xloc;
	pobject->dy = ynew - pobject->yloc;
	pobject->xloc = xnew;
	pobject->yloc = ynew;
	clampToWalls(pobject);
	return 0;
}

int ColonyEngine::checkwallTryFeature(int xnew, int ynew, int xind2, int yind2, Locate *pobject, int dir) {
	const uint8 *feature = mapFeatureAt(pobject->xindex, pobject->yindex, dir);
	int r = tryPassThroughFeature(pobject->xindex, pobject->yindex, dir, pobject);
	if (r == 2)
		return 0; // teleported  position already updated by the feature
	if (r == 1) {
		const int rnum = occupiedObjectAt(xind2, yind2, pobject);
		if (rnum) {
			const bool showDoorText = (pobject == &_me && feature &&
				(feature[0] == kWallFeatureDoor || feature[0] == kWallFeatureAirlock));
			if (showDoorText)
				doText(75, 0);
			return rnum;
		}
		uint8 trailCode = 0;
		switch (dir) {
		case kDirNorth:
			trailCode = 1;
			break;
		case kDirSouth:
			trailCode = 5;
			break;
		case kDirEast:
			trailCode = 3;
			break;
		case kDirWest:
			trailCode = 7;
			break;
		default:
			break;
		}
		if (trailCode != 0 && pobject->type == kMeNum &&
		    pobject->xindex >= 0 && pobject->xindex < 32 &&
		    pobject->yindex >= 0 && pobject->yindex < 32)
			_dirXY[pobject->xindex][pobject->yindex] = trailCode;
		pobject->yindex = yind2;
		pobject->xindex = xind2;
		pobject->dx = xnew - pobject->xloc;
		pobject->dy = ynew - pobject->yloc;
		pobject->xloc = xnew;
		pobject->yloc = ynew;
		clampToWalls(pobject);
		return 0;
	}
	return -2; // blocked, caller handles
}

int ColonyEngine::checkwall(int xnew, int ynew, Locate *pobject) {
	const int xind2 = xnew >> 8;
	const int yind2 = ynew >> 8;

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
				return checkwallMoveTo(xnew, ynew, xind2, yind2, pobject, 1);
			if (pobject->type == 2)
				return -1;
			{
				int r = checkwallTryFeature(xnew, ynew, xind2, yind2, pobject, kDirNorth);
				if (r != -2)
					return r;
			}
			debugC(1, kColonyDebugMove, "Collision South at x=%d y=%d", pobject->xindex, yind2);
			if (!_suppressCollisionSound)
				_sound->play(Sound::kBang);
			return -1;

		}

		if (!(_wall[pobject->xindex][pobject->yindex] & 1))
			return checkwallMoveTo(xnew, ynew, xind2, yind2, pobject, 5);
		if (pobject->type == 2)
			return -1;
		{
			int r = checkwallTryFeature(xnew, ynew, xind2, yind2, pobject, kDirSouth);
			if (r != -2)
				return r;
		}
		debugC(1, kColonyDebugMove, "Collision North at x=%d y=%d", pobject->xindex, pobject->yindex);
		if (!_suppressCollisionSound)
			_sound->play(Sound::kBang);
		return -1;

	}

	if (yind2 == pobject->yindex) {
		if (xind2 > pobject->xindex) {
			if (!(_wall[xind2][pobject->yindex] & 2))
				return checkwallMoveTo(xnew, ynew, xind2, yind2, pobject, 3);
			if (pobject->type == 2)
				return -1;
			{
				int r = checkwallTryFeature(xnew, ynew, xind2, yind2, pobject, kDirEast);
				if (r != -2)
					return r;
			}
			debugC(1, kColonyDebugMove, "Collision East at x=%d y=%d", xind2, pobject->yindex);
			if (!_suppressCollisionSound)
				_sound->play(Sound::kBang);
			return -1;

		}

		if (!(_wall[pobject->xindex][pobject->yindex] & 2))
			return checkwallMoveTo(xnew, ynew, xind2, yind2, pobject, 7);
		if (pobject->type == 2)
			return -1;
		{
			int r = checkwallTryFeature(xnew, ynew, xind2, yind2, pobject, kDirWest);
			if (r != -2)
				return r;
		}
		debugC(1, kColonyDebugMove, "Collision West at x=%d y=%d", pobject->xindex, pobject->yindex);
		if (!_suppressCollisionSound)
			_sound->play(Sound::kBang);
		return -1;

	}

	// Diagonal
	if (xind2 > pobject->xindex) {
		if (yind2 > pobject->yindex) {
			if ((_wall[pobject->xindex][yind2] & 1) || (_wall[xind2][pobject->yindex] & 2) || (_wall[xind2][yind2] & 3)) {
				debugC(1, kColonyDebugMove, "Collision Diagonal SE");
				return -1;
			}
		} else {
			if ((_wall[pobject->xindex][pobject->yindex] & 1) || (_wall[xind2][yind2] & 2) || (_wall[xind2][pobject->yindex] & 3)) {
				debugC(1, kColonyDebugMove, "Collision Diagonal NE");
				return -1;
			}
		}
	} else {
		if (yind2 > pobject->yindex) {
			if ((_wall[xind2][yind2] & 1) || (_wall[pobject->xindex][pobject->yindex] & 2) || (_wall[pobject->xindex][yind2] & 3)) {
				debugC(1, kColonyDebugMove, "Collision Diagonal SW");
				return -1;
			}
		} else {
			if ((_wall[xind2][pobject->yindex] & 1) || (_wall[pobject->xindex][yind2] & 2) || (_wall[pobject->xindex][pobject->yindex] & 3)) {
				debugC(1, kColonyDebugMove, "Collision Diagonal NW");
				return -1;
			}
		}
	}

	return checkwallMoveTo(xnew, ynew, xind2, yind2, pobject, 0);
}

bool ColonyEngine::setDoorState(int x, int y, int direction, int state) {
	if (x < 0 || x >= 31 || y < 0 || y >= 31 || direction < 0 || direction > 3)
		return false;

	const uint8 wallType = _mapData[x][y][direction][0];
	if (wallType != kWallFeatureDoor && wallType != kWallFeatureAirlock)
		return false;

	const uint8 oldState = _mapData[x][y][direction][1];
	_mapData[x][y][direction][1] = (uint8)state;
	if (wallType == kWallFeatureDoor && state == 0)
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
		if (wallType == kWallFeatureAirlock || _mapData[nx][ny][opposite][0] == wallType)
			_mapData[nx][ny][opposite][1] = (uint8)state;
	}

	// Persist airlock state changes across level loads
	if (wallType == kWallFeatureAirlock) {
		if (oldState != state && _level >= 1 && _level <= 8) {
			LevelData &ld = _levelData[_level - 1];
			if (state == 0)
				ld.count++;
			else if (ld.count > 0)
				ld.count--;
		}
		saveWall(x, y, direction);
		if (nx >= 0 && nx < 31 && ny >= 0 && ny < 31 && opposite >= 0)
			saveWall(nx, ny, opposite);
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

int ColonyEngine::goToDestination(const uint8 *map, Locate *pobject) {
	if (!map)
		return 0;

	const int targetMap = map[2];
	const int targetX = map[3];
	const int targetY = map[4];

	if (targetMap == 0 && targetX == 0 && targetY == 0)
		return 1;

	if (targetMap == 127) {
		if (pobject != &_me)
			return 0;

		playTunnelAirlockEffect();

		if (_orbit || !(_armor || _fl)) {
			debugC(1, kColonyDebugMove,
				"Airlock death: no protection (orbit=%d armor=%d fl=%d) at (%d,%d) level=%d",
				_orbit, _armor, _fl, _me.xindex, _me.yindex, _level);
			terminateGame(false);
			return 0;
		}

		if (_me.xindex >= 0 && _me.xindex < 32 &&
		    _me.yindex >= 0 && _me.yindex < 32)
			_robotArray[_me.xindex][_me.yindex] = 0;

		_gameMode = kModeBattle;
		_projon = false;
		_pcount = 0;
		_me.xloc = targetX << 8;
		_me.yloc = targetY << 8;
		_me.xindex = targetX;
		_me.yindex = targetY;
		return 2;
	}

	if (targetMap == 100) {
		doText(78, 0);
		return 0;
	}

	if (targetX > 0 && targetX < 31 && targetY > 0 && targetY < 31) {
		if (targetMap == 0 && _robotArray[targetX][targetY] != 0) {
			if (pobject == &_me)
				doText(75, 0);
			return 0;
		}

		const int xmod = pobject->xloc - (pobject->xindex << 8);
		const int ymod = pobject->yloc - (pobject->yindex << 8);
		_robotArray[pobject->xindex][pobject->yindex] = 0;
		pobject->xloc = (targetX << 8) + xmod;
		pobject->xindex = targetX;
		pobject->yloc = (targetY << 8) + ymod;
		pobject->yindex = targetY;
	}

	if (targetMap > 0 && targetMap != _level) {
		loadMap(targetMap);
		_coreIndex = (targetMap == 1) ? 0 : 1;
	}

	if (pobject->xindex >= 0 && pobject->xindex < 32 &&
	    pobject->yindex >= 0 && pobject->yindex < 32)
		_robotArray[pobject->xindex][pobject->yindex] = kMeNum;

	return 2;
}

// Returns: 0 = blocked, 1 = can pass through normally, 2 = teleported (position already set)
int ColonyEngine::tryPassThroughFeature(int fromX, int fromY, int direction, Locate *pobject) {
	const uint8 *map = mapFeatureAt(fromX, fromY, direction);
	if (!map || map[0] == kWallFeatureNone)
		return 0;

	switch (map[0]) {
	case kWallFeatureDoor:
		if (map[1] == 0)
			return 1; // already open  pass through
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
			if (getPlatform() == Common::kPlatformMacintosh)
				setObjectState(3, 2); // original Mac DoDoor: closed handle state
			setObjectState(2, 1); // door starts closed
			playAnimation();
			if (_animationResult) {
				setDoorState(fromX, fromY, direction, 0);
				return 1; // pass through
			}
			return 0; // player didn't open the door
		}
	case kWallFeatureAirlock:
		if (pobject != &_me) {
			if (map[1] || map[2] || map[3] || map[4])
				return 0;
			return 1;
		}
		// DOS DoAirLock: play airlock animation
		{
			if (!loadAnimation("airlock"))
				return 0;
			_animationResult = 0;
			_airlockX = fromX;
			_airlockY = fromY;
			_airlockDirection = direction;
			_airlockTerminate = false;
			_doorOpen = (map[1] == 0);
			if (getPlatform() == Common::kPlatformMacintosh) {
				setObjectState(3, _doorOpen ? 5 : 1);
				setObjectState(2, _doorOpen ? 1 : 2);
			} else {
				setObjectState(2, _doorOpen ? 3 : 1);
				setObjectState(1, _doorOpen ? 2 : 1);
			}
			playAnimation();
			_airlockX = -1;
			_airlockY = -1;
			_airlockDirection = -1;
			if (_airlockTerminate) {
				terminateGame(true);
				return 0;
			}
			if (_animationResult)
				return goToDestination(map, pobject);
			return 0;
		}

	case kWallFeatureUpStairs:
	case kWallFeatureDnStairs:
	case kWallFeatureTunnel: {
		if (pobject != &_me)
			return 0; // robots don't use stairs/tunnels

		// Play appropriate sound
		if (map[0] == kWallFeatureDnStairs)
			_sound->play(Sound::kClatter);
		const int result = goToDestination(map, pobject);
		if (result == 2) {
			debugC(1, kColonyDebugMove, "Level change via %s: level=%d pos=(%d,%d)",
			      map[0] == kWallFeatureUpStairs ? "upstairs" :
			      map[0] == kWallFeatureDnStairs ? "downstairs" : "tunnel",
			      _level, pobject->xindex, pobject->yindex);
		}
		return result;
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
		setObjectOnOff(6, false);
		setObjectOnOff(7, false);
		setObjectOnOff(8, false);
		setObjectOnOff(9, false);
		setObjectOnOff(10, false);
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
				_robotArray[pobject->xindex][pobject->yindex] = kMeNum;

			debugC(1, kColonyDebugMove, "Elevator: level=%d pos=(%d,%d)", _level, pobject->xindex, pobject->yindex);
			return 2; // teleported
		}
		// Player entered but stayed on same floor  turn around
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

void ColonyEngine::playTunnelAirlockEffect() {
	const Common::Rect effectRect(0, _menuBarHeight, _width, _height);
	const bool macColor = (_renderMode == Common::kRenderMacintosh && _hasMacColors);
	const int tunnelColor = 24; // c_tunnel
	const uint32 fillFg = macColor ? packTunnelMacColor(_macColors[tunnelColor].fg) : 0;
	const uint32 fillBg = macColor ? packTunnelMacColor(_macColors[tunnelColor].bg) : 0;
	const uint32 lineColor = macColor ? 0xFF000000 : 15;
	int troy = 180;
	int counter = 4;
	int spd = 180 / counter;
	int remaining = 5;

	while (remaining > 0 && !shouldQuit()) {
		if (macColor) {
			fillTunnelPattern(_gfx, effectRect, fillFg, fillBg, _macColors[tunnelColor].pattern);
		} else {
			_gfx->fillRect(effectRect, 0);
			_gfx->drawRect(effectRect, 15);
		}

		Common::Rect clipRect = effectRect;
		int prevDL[3] = {0, 0, 0};
		int prevDR[3] = {0, 0, 0};
		int prevLT = 0;
		int prevRT = 0;
		int rox = -150;
		int roy = troy;

		for (int i = 0; i < remaining; ++i) {
			int dl[2];
			int dr[2];
			projectTunnelPoint(effectRect, dl, rox, roy);
			projectTunnelPoint(effectRect, dr, rox + 200, roy);

			int left[3] = {
				dl[0],
				dl[1],
				effectRect.bottom - 1 - (dl[1] - effectRect.top)
			};
			int right[3] = {
				dr[0],
				dr[1],
				effectRect.bottom - 1 - (dr[1] - effectRect.top)
			};

			drawTunnelLine(_gfx, clipRect, left[0], left[2], left[0], left[1], lineColor);
			drawTunnelLine(_gfx, clipRect, left[0], left[1], right[0], right[1], lineColor);
			drawTunnelLine(_gfx, clipRect, right[0], right[1], right[0], right[2], lineColor);

			const int center = (left[0] + right[0]) >> 1;
			const int lt = (left[0] + center) >> 1;
			const int rt = (right[0] + center) >> 1;

			if (i > 0) {
				drawTunnelLine(_gfx, clipRect, left[0], left[1], prevDL[0], prevDL[1], lineColor);
				drawTunnelLine(_gfx, clipRect, right[0], right[1], prevDR[0], prevDR[1], lineColor);
				drawTunnelLine(_gfx, clipRect, right[0], right[2], prevDR[0], prevDR[2], lineColor);
				drawTunnelLine(_gfx, clipRect, left[0], left[2], prevDL[0], prevDL[2], lineColor);
				drawTunnelLine(_gfx, clipRect, prevLT, prevDL[2], lt, left[2], lineColor);
				drawTunnelLine(_gfx, clipRect, lt, left[2], rt, right[2], lineColor);
				drawTunnelLine(_gfx, clipRect, rt, right[2], prevRT, prevDR[2], lineColor);
			}

			clipRect.left = MAX<int>(clipRect.left, left[0]);
			clipRect.right = MIN<int>(clipRect.right, right[0]);
			clipRect.top = MAX<int>(clipRect.top, left[1]);
			clipRect.bottom = MIN<int>(clipRect.bottom, right[2]);
			if (clipRect.bottom <= clipRect.top || clipRect.left >= clipRect.right)
				break;

			prevDL[0] = left[0];
			prevDL[1] = left[1];
			prevDL[2] = left[2];
			prevDR[0] = right[0];
			prevDR[1] = right[1];
			prevDR[2] = right[2];
			prevLT = lt;
			prevRT = rt;
			roy += 256;
			rox += kTunnelStraight[i] << 2;
		}

		_gfx->copyToScreen();
		_system->delayMillis(50);

		troy -= spd;
		counter--;
		if (counter == 0) {
			troy = 180;
			counter = 4;
			remaining--;
		}
	}
}

void ColonyEngine::playTunnelEffect(bool falling) {
	// Original TUNNEL.C: falling into the reactor reuses the tunnel renderer
	// with the falling flag set, which removes the tracks and shortens the run.
	const Common::Rect effectRect(0, _menuBarHeight, _width, _height);
	const bool macColor = (_renderMode == Common::kRenderMacintosh && _hasMacColors);
	const int tunnelColor = 24; // c_tunnel
	const int tunnelFrames = falling ? 10 : 49;
	const uint32 fillFg = macColor ? packTunnelMacColor(_macColors[tunnelColor].fg) : 0;
	const uint32 fillBg = macColor ? packTunnelMacColor(_macColors[tunnelColor].bg) : 0;
	const uint32 lineColor = macColor ? 0xFF000000 : 15;
	int troy = 180;
	int cnt = 0;
	int counter = falling ? 2 : kTunnelST[0];
	int spd = 180 / counter;

	_sound->play(Sound::kTunnel2);

	for (int remaining = tunnelFrames; remaining > 0 && !shouldQuit(); ) {
		if (!_sound->isPlaying())
			_sound->play(Sound::kTunnel2);

		if (macColor) {
			fillTunnelPattern(_gfx, effectRect, fillFg, fillBg, _macColors[tunnelColor].pattern);
		} else {
			_gfx->fillRect(effectRect, 0);
			_gfx->drawRect(effectRect, 15);
		}

		const int n = MIN<int>(remaining, 16);
		const double fcnt = falling ? (double)counter / 2.0 : (double)counter / kTunnelST[cnt];
		const int *pathX = &kTunnelXT[cnt];
		const int *pathY = &kTunnelYT[cnt];
		Common::Rect clipRect = effectRect;
		int prevDL[3] = {0, 0, 0};
		int prevDR[3] = {0, 0, 0};
		int prevLT = 0;
		int prevRT = 0;
		int rox = -100;
		int roy = troy;
		int ty = 0;

		for (int i = 0; i < n; ++i) {
			int tx;
			if (i == 0) {
				tx = (int)((pathX[i] << 2) * fcnt);
				ty = (int)(pathY[i] * fcnt);
			} else {
				tx = pathX[i] << 2;
				ty += pathY[i];
			}

			rox += tx;

			int dl[2];
			int dr[2];
			projectTunnelPoint(effectRect, dl, rox, roy);
			projectTunnelPoint(effectRect, dr, rox + 200, roy);

			int left[3] = {
				dl[0],
				dl[1] + ty,
				effectRect.bottom - 1 - (dl[1] - effectRect.top) + ty
			};
			int right[3] = {
				dr[0],
				dr[1] + ty,
				effectRect.bottom - 1 - (dr[1] - effectRect.top) + ty
			};

			drawTunnelLine(_gfx, clipRect, left[0], left[2], left[0], left[1], lineColor);
			drawTunnelLine(_gfx, clipRect, left[0], left[1], right[0], right[1], lineColor);
			drawTunnelLine(_gfx, clipRect, right[0], right[1], right[0], right[2], lineColor);

			int lt = 0;
			int rt = 0;
			if (!falling) {
				const int center = (left[0] + right[0]) >> 1;
				lt = (left[0] + center) >> 1;
				rt = (right[0] + center) >> 1;
			} else {
				drawTunnelLine(_gfx, clipRect, right[0], right[2], left[0], left[2], lineColor);
			}

			if (i > 0) {
				drawTunnelLine(_gfx, clipRect, left[0], left[1], prevDL[0], prevDL[1], lineColor);
				drawTunnelLine(_gfx, clipRect, right[0], right[1], prevDR[0], prevDR[1], lineColor);
				drawTunnelLine(_gfx, clipRect, right[0], right[2], prevDR[0], prevDR[2], lineColor);
				drawTunnelLine(_gfx, clipRect, left[0], left[2], prevDL[0], prevDL[2], lineColor);
				if (!falling) {
					drawTunnelLine(_gfx, clipRect, prevLT, prevDL[2], lt, left[2], lineColor);
					drawTunnelLine(_gfx, clipRect, lt, left[2], rt, right[2], lineColor);
					drawTunnelLine(_gfx, clipRect, rt, right[2], prevRT, prevDR[2], lineColor);
				}
			}

			if (clipRect.bottom < right[2])
				drawTunnelLine(_gfx, clipRect, clipRect.left, clipRect.bottom - 1, clipRect.right - 1, clipRect.bottom - 1, lineColor);

			clipRect.left = MAX<int>(clipRect.left, left[0]);
			clipRect.right = MIN<int>(clipRect.right, right[0]);
			clipRect.top = MAX<int>(clipRect.top, left[1]);
			clipRect.bottom = MIN<int>(clipRect.bottom, right[2]);
			if (clipRect.bottom <= clipRect.top || clipRect.left >= clipRect.right)
				break;

			prevDL[0] = left[0];
			prevDL[1] = left[1];
			prevDL[2] = left[2];
			prevDR[0] = right[0];
			prevDR[1] = right[1];
			prevDR[2] = right[2];
			prevLT = lt;
			prevRT = rt;
			roy += 256;
		}

		_gfx->copyToScreen();
		_system->delayMillis(50);

		counter--;
		troy -= spd;
		if (counter == 0) {
			troy = 180;
			cnt++;
			counter = falling ? 2 : kTunnelST[cnt];
			spd = 256 / counter;
			remaining--;
		}
	}

	_sound->stop();
}

void ColonyEngine::fallThroughHole() {
	// DOS/Mac tunnel(TRUE, mapdata) + GoTo(mapdata)
	// Called when player falls through a floor hole (SMHOLEFLR or LGHOLEFLR)
	const uint8 *mapdata = _mapData[_me.xindex][_me.yindex][4];
	int targetMap = mapdata[2];
	int targetX = mapdata[3];
	int targetY = mapdata[4];
	const bool deadFall = (targetMap == 0 && targetX == 0 && targetY == 0);

	// Original tunnel(TRUE): damage the player's three power bars.
	int damage = -(_level << 7);
	debugC(1, kColonyDebugCombat,
		"fallThroughHole: level=%d from=(%d,%d) deadFall=%d delta=[%d,%d,%d]",
		_level, _me.xindex, _me.yindex, deadFall ? 1 : 0, damage, damage, damage);
	setPower(damage, damage, damage);
	playTunnelEffect(true);

	if (deadFall) {
		terminateGame(false); // original tunnel(TRUE) still animates before death
		return;
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

		_robotArray[targetX][targetY] = kMeNum;
	}

	// DOS: if(map) load_mapnum(map, TRUE)  always reload when map != 0
	if (targetMap > 0)
		loadMap(targetMap);

	debugC(1, kColonyDebugMove, "Fell through hole: level=%d pos=(%d,%d)", _level, _me.xindex, _me.yindex);
}

void ColonyEngine::checkCenter() {
	// DOS CCenter(): check if player is standing on a floor hole or hotfoot.
	// Called every render frame at 60fps for responsive hole/egg detection.
	// HOTFOOT damage is time-gated via _hotfootAccum to match the original
	// Mac Display() cadence (~8fps). The original had no throttle — damage
	// fired once per Display() call, which was hardware-limited to ~5-8fps
	// on a Mac Plus.
	if (_me.xindex < 0 || _me.xindex >= 31 || _me.yindex < 0 || _me.yindex >= 31)
		return;

	const uint8 cellType = _mapData[_me.xindex][_me.yindex][4][0];
	if (cellType != 0) {
		debugC(2, kColonyDebugMove,
			"checkCenter: cellType=%d at (%d,%d) dest=[%d,%d,%d] level=%d",
			cellType, _me.xindex, _me.yindex,
			_mapData[_me.xindex][_me.yindex][4][2],
			_mapData[_me.xindex][_me.yindex][4][3],
			_mapData[_me.xindex][_me.yindex][4][4], _level);
		switch (cellType) {
		case 1: { // SMHOLEFLR  small floor hole, must be near center
			int xcheck = ABS(_me.xloc - (_me.xindex << 8));
			int ycheck = ABS(_me.yloc - (_me.yindex << 8));
			if (xcheck > 64 && xcheck < 192 && ycheck > 64 && ycheck < 192)
				fallThroughHole();
			break;
		}
		case 2: // LGHOLEFLR  large floor hole, full cell
			fallThroughHole();
			break;
		case 5: { // HOTFOOT  electric floor, damages power
			// Time-gate damage: accumulate ms, fire at ~8fps (125ms intervals)
			uint32 now = _system->getMillis();
			uint32 elapsed = now - _lastHotfootTime;
			if (elapsed >= 125) {
				_lastHotfootTime = now;
				_sound->play(Sound::kBzzz);
				debugC(1, kColonyDebugCombat,
					"hotfoot: level=%d cell=(%d,%d) delta=[%d,%d,%d]",
					_level, _me.xindex, _me.yindex,
					-(5 << _level), -(5 << _level), -(5 << _level));
				setPower(-(5 << _level), -(5 << _level), -(5 << _level));
			}
			break;
		}
		default:
			break;
		}
	}

	if (!_fl && _me.xindex >= 0 && _me.xindex < 32 && _me.yindex >= 0 && _me.yindex < 32) {
		const uint8 foodNum = _foodArray[_me.xindex][_me.yindex];
		if (foodNum > 0 && foodNum <= _objects.size() && _objects[foodNum - 1].type < kBaseObject) {
			const int xcheck = ABS(_me.xloc - (_me.xindex << 8));
			const int ycheck = ABS(_me.yloc - (_me.yindex << 8));
			if (xcheck > 64 && xcheck < 192 && ycheck > 64 && ycheck < 192)
				meEat();
		}
	}
}

void ColonyEngine::cCommand(int xnew, int ynew, bool allowInteraction) {
	if (_me.xindex >= 0 && _me.xindex < 32 && _me.yindex >= 0 && _me.yindex < 32)
		_robotArray[_me.xindex][_me.yindex] = 0;

	const int robot = checkwall(xnew, ynew, &_me);
	if (robot > 0 && allowInteraction)
		interactWithObject(robot);

	if (_me.xindex >= 0 && _me.xindex < 32 && _me.yindex >= 0 && _me.yindex < 32)
		_robotArray[_me.xindex][_me.yindex] = kMeNum;
}

// DOS ExitFL(): step back one cell and drop the forklift.
void ColonyEngine::exitForklift() {
	if (_fl != 1)
		return;

	int xloc = _me.xloc;
	int yloc = _me.yloc;
	int xindex = _me.xindex;
	int yindex = _me.yindex;

	// Walk backward until we move into a different cell
	while (_me.xindex == xindex && _me.yindex == yindex) {
		int xnew = _me.xloc - _cost[_me.ang];
		int ynew = _me.yloc - _sint[_me.ang];
		_me.type = 2; // temporary small collision type
		if (checkwall(xnew, ynew, &_me)) {
			_sound->play(Sound::kChime);
			_me.type = kMeNum;
			return;
		}
		_me.type = kMeNum;
	}

	// Snap to cell center for the dropped forklift
	xloc = (xloc >> 8);
	xloc = (xloc << 8) + 128;
	yloc = (yloc >> 8);
	yloc = (yloc << 8) + 128;

	if (!createObject(kObjForkLift, xloc, yloc, _me.ang)) {
		warning("exitForklift: failed to place forklift on level %d at (%d,%d)", _level, xindex, yindex);
		return;
	}

	PassPatch to;
	to.level = _level;
	to.xindex = xindex;
	to.yindex = yindex;
	to.xloc = xloc;
	to.yloc = yloc;
	to.ang = _me.ang;
	newPatch(kObjForkLift, _carryPatch[0], to, nullptr);

	_fl = 0;
}

// DOS DropFL(): step back one cell and drop the carried object, then return to fl=1.
void ColonyEngine::dropCarriedObject() {
	if (_fl != 2)
		return;

	// Special case: carrying reactor core
	if (_carryType == kObjReactor) {
		_sound->play(Sound::kChime); // glass break sound
		_carryType = 0;
		_fl = 1;
		return;
	}

	// Play the drop animation
	if (loadAnimation("lift")) {
		_animationResult = 0;
		playAnimation();
		if (!_animationResult) {
			// Animation was cancelled  don't drop
			return;
		}
	}

	int xloc = _me.xloc;
	int yloc = _me.yloc;
	int xindex = _me.xindex;
	int yindex = _me.yindex;

	// Walk backward until we move into a different cell
	while (_me.xindex == xindex && _me.yindex == yindex) {
		int xnew = _me.xloc - _cost[_me.ang];
		int ynew = _me.yloc - _sint[_me.ang];
		_me.type = 2;
		if (checkwall(xnew, ynew, &_me)) {
			_sound->play(Sound::kChime);
			_me.type = kMeNum;
			return;
		}
		_me.type = kMeNum;
	}

	// DOS: teleport always drops at ang=0; other objects use player's angle
	uint8 ang = (_carryType == kObjTeleport) ? 0 : _me.ang;

	xloc = (xloc >> 8);
	xloc = (xloc << 8) + 128;
	yloc = (yloc >> 8);
	yloc = (yloc << 8) + 128;

	if (!createObject(_carryType, xloc, yloc, ang)) {
		warning("dropCarriedObject: failed to place type %d on level %d at (%d,%d)",
			_carryType, _level, xindex, yindex);
		return;
	}

	PassPatch to;
	to.level = _level;
	to.xindex = xindex;
	to.yindex = yindex;
	to.xloc = xloc;
	to.yloc = yloc;
	to.ang = _me.ang;
	newPatch(_carryType, _carryPatch[1], to, nullptr);

	_fl = 1;
}

} // End of namespace Colony
