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
#include "common/debug.h"
#include <math.h>

namespace Colony {

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
			return 0; // teleported  position already updated by the feature
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
			{
				int r = tryFeature(kDirNorth);
				if (r != -2)
					return r;
			}
			debugC(1, kColonyDebugMove, "Collision South at x=%d y=%d", pobject->xindex, yind2);
			_sound->play(Sound::kBang);
			return -1;

		}

		if (!(_wall[pobject->xindex][pobject->yindex] & 1))
			return moveTo();
		{
			int r = tryFeature(kDirSouth);
			if (r != -2)
				return r;
		}
		debugC(1, kColonyDebugMove, "Collision North at x=%d y=%d", pobject->xindex, pobject->yindex);
		_sound->play(Sound::kBang);
		return -1;

	}

	if (yind2 == pobject->yindex) {
		if (xind2 > pobject->xindex) {
			if (!(_wall[xind2][pobject->yindex] & 2))
				return moveTo();
			{
				int r = tryFeature(kDirEast);
				if (r != -2)
					return r;
			}
			debugC(1, kColonyDebugMove, "Collision East at x=%d y=%d", xind2, pobject->yindex);
			_sound->play(Sound::kBang);
			return -1;

		}

		if (!(_wall[pobject->xindex][pobject->yindex] & 2))
			return moveTo();
		{
			int r = tryFeature(kDirWest);
			if (r != -2)
				return r;
		}
		debugC(1, kColonyDebugMove, "Collision West at x=%d y=%d", pobject->xindex, pobject->yindex);
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

	// Persist airlock state changes across level loads
	if (wallType == kWallFeatureAirlock) {
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
		if (map[1] == 0)
			return 1; // already open  pass through
		if (pobject != &_me)
			return 0;
		// DOS DoAirLock: play airlock animation
		{
			if (!loadAnimation("airlock"))
				return 0;
			_animationResult = 0;
			_doorOpen = false;
			if (getPlatform() == Common::kPlatformMacintosh) {
				setObjectState(3, 1); // original Mac DoAirLock: closed airlock frame
				setObjectState(2, 2); // control sprite starts in locked/closed state
			} else {
				setObjectState(2, 1); // original DOS DoAirLock: closed door frame
				setObjectState(1, 1); // control sprite starts in locked/closed state
			}
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
			return 1; // no destination data  just pass through

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
			_robotArray[pobject->xindex][pobject->yindex] = kMeNum;

		debugC(1, kColonyDebugMove, "Level change via %s: level=%d pos=(%d,%d)",
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

	// DOS tunnel(pt=TRUE): falling animation  nested rectangles shrinking toward
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

			// Draw nested rectangles  outer ring shrinks in, inner rings follow
			// The number of visible rings decreases as we fall deeper
			int visibleRings = maxRings - (int)(progress * (maxRings - 1));
			for (int ring = 0; ring < visibleRings; ring++) {
				// Each ring's depth combines the overall fall progress with per-ring spacing
				float depth = progress * 0.6f + (float)ring / (maxRings + 2.0f);
				if (depth >= 1.0f)
					break;
				float scale = 1.0f - depth;
				int rw = (int)(hw * scale);
				int rh = (int)(hh * scale);
				if (rw < 2 || rh < 2)
					break;
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

		_robotArray[targetX][targetY] = kMeNum;
	}

	// DOS: if(map) load_mapnum(map, TRUE)  always reload when map != 0
	if (targetMap > 0)
		loadMap(targetMap);

	debugC(1, kColonyDebugMove, "Fell through hole: level=%d pos=(%d,%d)", _level, _me.xindex, _me.yindex);
}

void ColonyEngine::checkCenter() {
	// DOS CCenter(): check if player is standing on a floor hole or hotfoot
	if (_me.xindex < 0 || _me.xindex >= 31 || _me.yindex < 0 || _me.yindex >= 31)
		return;

	const uint8 cellType = _mapData[_me.xindex][_me.yindex][4][0];
	if (cellType == 0)
		return;

	switch (cellType) {
	case 1: { // SMHOLEFLR  small floor hole, must be near center
		// DOS: xcheck=abs(xloc-(xindex<<8)); if(xcheck>64&&xcheck<192)
		int xcheck = ABS(_me.xloc - (_me.xindex << 8));
		int ycheck = ABS(_me.yloc - (_me.yindex << 8));
		if (xcheck > 64 && xcheck < 192 && ycheck > 64 && ycheck < 192)
			fallThroughHole();
		break;
	}
	case 2: // LGHOLEFLR  large floor hole, full cell
		fallThroughHole();
		break;
	case 5: // HOTFOOT  electric floor, damages power
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

	createObject(kObjForkLift, xloc, yloc, _me.ang);

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

	createObject(_carryType, xloc, yloc, ang);

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
