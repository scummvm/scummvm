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
#include "common/file.h"
#include "common/system.h"
#include "common/util.h"
#include "common/algorithm.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "engines/util.h"
#include "graphics/palette.h"
#include "graphics/paletteman.h"
#include <math.h>

namespace Colony {

static const int16 g_sintTable[256] = {
	90,  93,  95,  97,  99, 101, 103, 105,
	106, 108, 110, 111, 113, 114, 116, 117,
	118, 119, 121, 122, 122, 123, 124, 125,
	126, 126, 127, 127, 127, 128, 128, 128,
	128, 128, 128, 128, 127, 127, 127, 126,
	126, 125, 124, 123, 122, 122, 121, 119,
	118, 117, 116, 114, 113, 111, 110, 108,
	106, 105, 103, 101,  99,  97,  95,  93,
	90,  88,  86,  84,  81,  79,  76,  74,
	71,  68,  66,  63,  60,  56,  54,  52,
	49,  46,  43,  40,  37,  34,  31,  28,
	25,  23,  19,  16,  13,   9,   6,   3,
	0,  -3,  -6,  -9, -13, -16, -19, -23,
	-25, -28, -31, -34, -37, -40, -43, -46,
	-49, -52, -54, -56, -60, -63, -66, -68,
	-71, -74, -76, -79, -81, -84, -86, -88,
	-88, -90, -93, -95, -97, -99,-101,-103,
	-105,-106,-108,-110,-111,-113,-114,-116,
	-117,-118,-119,-121,-122,-122,-123,-124,
	-125,-126,-126,-127,-127,-127,-128,-128,
	-128,-128,-128,-128,-127,-127,-127,-126,
	-126,-125,-124,-123,-122,-122,-121,-119,
	-118,-117,-116,-114,-113,-111,-110,-108,
	-106,-105,-103,-101, -99, -97, -95, -93,
	-90, -88, -86, -84, -81, -79, -76, -74,
	-71, -68, -66, -63, -60, -56, -54, -52,
	-49, -46, -43, -40, -37, -34, -31, -28,
	-25, -23, -19, -16, -13,  -9,  -6,  -3,
	0,   3,   6,   9,  13,  16,  19,  23,
	25,  28,  31,  34,  37,  40,  43,  46,
	49,  52,  54,  56,  60,  63,  66,  68,
	71,  74,  76,  79,  81,  84,  86,  88
};

static const int g_indexTable[4][10] = {
	{0, 0,  0, 0,  0,  1,  1,  0,  1, 2},
	{1, 0,  0, 0, -1,  0,  0,  1,  2, 1},
	{0, 1,  1, 0,  0, -1, -1,  0,  1, 2},
	{0, 0,  0, 1,  1,  0,  0, -1,  2, 1}
};

enum WallFeatureType {
	kWallFeatureNone = 0,
	kWallFeatureDoor = 2,
	kWallFeatureWindow = 3,
	kWallFeatureShelves = 4,
	kWallFeatureUpStairs = 5,
	kWallFeatureDnStairs = 6,
	kWallFeatureChar = 7,
	kWallFeatureGlyph = 8,
	kWallFeatureElevator = 9,
	kWallFeatureTunnel = 10,
	kWallFeatureAirlock = 11,
	kWallFeatureColor = 12
};

enum MapDirection {
	kDirNorth = 0,
	kDirEast = 1,
	kDirWest = 2,
	kDirSouth = 3
};

static const int g_dirRight[4] = {1, 3, 0, 2};
static const int g_dirLeft[4] = {2, 0, 3, 1};

enum ObjectType {
	kObjDesk = 21,
	kObjPlant = 22,
	kObjCChair = 23,
	kObjBed = 24,
	kObjTable = 25,
	kObjCouch = 26,
	kObjChair = 27,
	kObjTV = 28,
	kObjScreen = 29,
	kObjConsole = 30,
	kObjPowerSuit = 31,
	kObjForkLift = 32,
	kObjCryo = 33,
	kObjBox1 = 34,
	kObjBox2 = 35,
	kObjTeleport = 36,
	kObjDrawer = 37,
	kObjTub = 38,
	kObjSink = 39,
	kObjToilet = 40,
	kObjPToilet = 43,
	kObjProjector = 45,
	kObjReactor = 46,
	kObjFWall = 48,
	kObjCWall = 49,
	kObjBBed = 42
};

struct ColonyEngine::PrismPartDef {
	int pointCount;
	const int (*points)[3];
	int surfaceCount;
	const int (*surfaces)[8];
};

ColonyEngine::ColonyEngine(OSystem *syst, const ADGameDescription *gd) : Engine(syst), _gameDescription(gd) {
	_level = 0;
	_robotNum = 0;
	_gfx = nullptr;
	_width = 640;
	_height = 480;
	_centerX = _width / 2;
	_centerY = _height / 2;
	_flip = false;
	_mouseSensitivity = 1;
	_change = true;
	_showDashBoard = true;
	_crosshair = true;
	_insight = false;
	_hasKeycard = false;
	_unlocked = false;
	_weapons = 0;
	
	memset(_wall, 0, sizeof(_wall));
	memset(_mapData, 0, sizeof(_mapData));
	memset(_robotArray, 0, sizeof(_robotArray));
	memset(_foodArray, 0, sizeof(_foodArray));

	_screenR = Common::Rect(0, 0, _width, _height);
	_clip = _screenR;
	_dashBoardRect = Common::Rect(0, 0, 0, 0);
	_compassRect = Common::Rect(0, 0, 0, 0);
	_headsUpRect = Common::Rect(0, 0, 0, 0);
	_powerRect = Common::Rect(0, 0, 0, 0);
	
	// DOS gameInit(): Me.ang=Me.look=32; Me.xloc=4400; Me.yloc=4400.
	memset(&_me, 0, sizeof(_me));
	_me.xloc = 4400;
	_me.yloc = 4400;
	_me.xindex = _me.xloc >> 8;
	_me.yindex = _me.yloc >> 8;
	_me.look = 32;
	_me.ang = 32;
	_me.type = MENUM;

	initTrig();
}

ColonyEngine::~ColonyEngine() {
}

void ColonyEngine::loadMap(int mnum) {
	Common::String mapName = Common::String::format("MAP.%d", mnum);
	Common::File file;
	if (!file.open(Common::Path(mapName))) {
		warning("Could not open map file %s", mapName.c_str());
		return;
	}

	file.readUint32BE(); // "DAVE" header
	int16 mapDefs[10];
	for (int i = 0; i < 10; i++) {
		mapDefs[i] = file.readSint16BE(); // Swapped in original code
	}

	uint16 bLength = file.readUint16BE(); // Swapped in original code
	uint8 *buffer = (uint8 *)malloc(bLength);
	if (!buffer) {
		error("Out of memory loading map");
	}
	file.read(buffer, bLength);
	file.close();

	memset(_mapData, 0, sizeof(_mapData));
	memset(_robotArray, 0, sizeof(_robotArray));
	memset(_foodArray, 0, sizeof(_foodArray));
	_objects.clear();

	// expand logic
	int c = 0;
	_robotNum = MENUM + 1;
	for (int i = 0; i < 32; i++) {
		for (int j = 0; j < 32; j++) {
			_wall[i][j] = buffer[c++];
			if (i < 31 && j < 31) {
				for (int k = 0; k < 5; k++) {
					if (_wall[i][j] & (1 << (k + 2))) {
						for (int l = 0; l < 5; l++) {
							_mapData[i][j][k][l] = buffer[c++];
						}
						// PACKIT.C: center feature type 6 marks static map objects.
						if (k == 4 && _mapData[i][j][4][0] == 6 && i < 31 && j < 31) {
							Thing obj;
							memset(&obj, 0, sizeof(obj));
							obj.alive = 1;
							obj.visible = 0;
							obj.type = _mapData[i][j][4][1] + BASEOBJECT;
							obj.where.xloc = (i << 8) + 128;
							obj.where.yloc = (j << 8) + 128;
							obj.where.xindex = i;
							obj.where.yindex = j;
							obj.where.ang = (uint8)(_mapData[i][j][4][2] + 32);
							obj.where.look = obj.where.ang;
							_objects.push_back(obj);
							const int objNum = (int)_objects.size(); // 1-based, DOS-style robot slots
							if (objNum > 0 && objNum < 256 && _robotArray[i][j] == 0)
								_robotArray[i][j] = (uint8)objNum;
						}
					} else {
						_mapData[i][j][k][0] = 0;
					}
				}
			}
		}
	}
	free(buffer);
	_level = mnum;
	_me.type = MENUM;
	if (_me.xindex >= 0 && _me.xindex < 32 && _me.yindex >= 0 && _me.yindex < 32)
		_robotArray[_me.xindex][_me.yindex] = MENUM;
	debug("Successfully loaded map %d (static objects: %d)", mnum, (int)_objects.size());
}

void ColonyEngine::initTrig() {
	for (int i = 0; i < 256; i++) {
		_sint[i] = g_sintTable[i];
		_cost[i] = g_sintTable[(i + 64) & 0xFF];
	}

	_rtable[0] = 32000;
	for (int i = 1; i < 11585; i++) {
		_rtable[i] = (160 * 128) / i;
	}
}

void ColonyEngine::rot_init(int x, int y) {
	_rox = ((long)x * _tsin - (long)y * _tcos) >> 8;
	_roy = ((long)y * _tsin + (long)x * _tcos) >> 8;
}

void ColonyEngine::perspective(int pnt[2], int rox, int roy) {
	long p;

	if (roy <= 0)
		roy = 1;
	p = _centerX + ((long)rox * 256) / roy;

	if (p < -32000)
		p = -32000;
	else if (p > 32000)
		p = 32000;
	pnt[0] = (int)p;

	if (_flip)
		pnt[1] = _centerY + _rtable[roy];
	else
		pnt[1] = _centerY - _rtable[roy];
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

void ColonyEngine::quadrant() {
	int remain;
	int quad;

	quad = _me.look >> 6;				/*divide by 64		*/
	remain = _me.look - (quad << 6);			/*multiply by 64	*/
	_tsin = _sint[remain];
	_tcos = _cost[remain];

	switch (quad) {
	case 0:
		rot_init((_me.xindex << 8) - _me.xloc, (_me.yindex << 8) - _me.yloc);
		_direction = 0; // NORTH
		break;
	case 1:
		rot_init((_me.yindex << 8) - _me.yloc, _me.xloc - ((_me.xindex + 1) << 8));
		_direction = 2; // WEST
		break;
	case 2:
		rot_init(_me.xloc - ((_me.xindex + 1) << 8), _me.yloc - ((_me.yindex + 1) << 8));
		_direction = 3; // SOUTH
		break;
	case 3:
		rot_init(_me.yloc - ((_me.yindex + 1) << 8), (_me.xindex << 8) - _me.xloc);
		_direction = 1; // EAST
		break;
	}

	_frntxWall = g_indexTable[quad][0];
	_frntyWall = g_indexTable[quad][1];
	_sidexWall = g_indexTable[quad][2];
	_sideyWall = g_indexTable[quad][3];
	_frntx = g_indexTable[quad][4];
	_frnty = g_indexTable[quad][5];
	_sidex = g_indexTable[quad][6];
	_sidey = g_indexTable[quad][7];
	_front = g_indexTable[quad][8];
	_side = g_indexTable[quad][9];
}

void ColonyEngine::corridor() {
	int length = 1;
	int xFrontLeft, yFrontLeft;
	int xFrontRight, yFrontRight;
	int xsstart, ysstart;
	int xfbehind, yfbehind;
	int roxsave, roysave;
	int left, right;
	int left2, right2;
	int cellx, celly;
	int cellxsave, cellysave;
	int dr[2];
	const int screenLeft = (int)_screenR.left;
	const int screenRight = (int)_screenR.right;

	quadrant();

	right = screenRight;
	left = screenLeft;
	right2 = right;
	left2 = left;

	xfbehind = _me.xindex + _frntxWall;
	yfbehind = _me.yindex + _frntyWall;
	xFrontLeft = xfbehind + _frntx;
	yFrontLeft = yfbehind + _frnty;
	xFrontRight = xFrontLeft + _sidex;
	yFrontRight = yFrontLeft + _sidey;
	xsstart = _me.xindex + _sidexWall;
	ysstart = _me.yindex + _sideyWall;
	cellxsave = cellx = _me.xindex;
	cellysave = celly = _me.yindex;

	int rox = _rox;
	int roy = _roy;

	if (_change) {
		perspective(dr, rox, roy);
		if (xfbehind >= 0 && xfbehind < 34 && yfbehind >= 0 && yfbehind < 34) {
			_drY[xfbehind][yfbehind] = dr[1];
			if (dr[0] > _screenR.left)
				_drX[xfbehind][yfbehind] = -32000;
			else
				_drX[xfbehind][yfbehind] = dr[0];
		}

		perspective(dr, rox + _tsin, roy + _tcos);
		if (xfbehind + _sidex >= 0 && xfbehind + _sidex < 34 && yfbehind + _sidey >= 0 && yfbehind + _sidey < 34) {
			_drY[xfbehind + _sidex][yfbehind + _sidey] = dr[1];
			if (dr[0] < _screenR.right)
				_drX[xfbehind + _sidex][yfbehind + _sidey] = 32000;
			else
				_drX[xfbehind + _sidex][yfbehind + _sidey] = dr[0];
		}
	}

	roxsave = rox;
	roysave = roy;

	// Move to the first wall in front of the observer.
	rox -= _tcos;
	roy += _tsin;

	if (_change) {
		perspective(dr, rox, roy);
		if (xFrontLeft >= 0 && xFrontLeft < 34 && yFrontLeft >= 0 && yFrontLeft < 34) {
			_drX[xFrontLeft][yFrontLeft] = dr[0];
			_drY[xFrontLeft][yFrontLeft] = dr[1];
		}
		perspective(dr, rox + _tsin, roy + _tcos);
		if (xFrontRight >= 0 && xFrontRight < 34 && yFrontRight >= 0 && yFrontRight < 34) {
			_drX[xFrontRight][yFrontRight] = dr[0];
			_drY[xFrontRight][yFrontRight] = dr[1];
		}
	}

	if (wallAt(cellx + _sidexWall, celly + _sideyWall) & _side)
		left2 = MAX(_drX[xFrontLeft][yFrontLeft], screenLeft);
	else
		left2 = MAX(left, left2);
	left2 = MAX(left2, screenLeft);

	if (wallAt(cellx + _sidexWall + _sidex, celly + _sideyWall + _sidey) & _side)
		right2 = _drX[xFrontRight][yFrontRight];
	else
		right2 = MIN(right, right2);

	uint32 white = _gfx->white();
	_gfx->drawLine(_drX[xFrontLeft][yFrontLeft], _drY[xFrontLeft][yFrontLeft],
	               _drX[xFrontRight][yFrontRight], _drY[xFrontRight][yFrontRight], white);
	if (wallAt(cellx, celly) & ~0x03)
		frontfeature(cellx, celly, xFrontLeft, yFrontLeft, left2, right2, rox, roy);

	while (!(wallAt(xFrontLeft, yFrontLeft) & _front)) {
		rox -= _tcos;
		roy += _tsin;
		xFrontLeft += _frntx;
		yFrontLeft += _frnty;
		xFrontRight += _frntx;
		yFrontRight += _frnty;
		if (_change) {
			perspective(dr, rox, roy);
			if (xFrontLeft >= 0 && xFrontLeft < 34 && yFrontLeft >= 0 && yFrontLeft < 34) {
				_drX[xFrontLeft][yFrontLeft] = dr[0];
				_drY[xFrontLeft][yFrontLeft] = dr[1];
			}
			perspective(dr, rox + _tsin, roy + _tcos);
			if (xFrontRight >= 0 && xFrontRight < 34 && yFrontRight >= 0 && yFrontRight < 34) {
				_drX[xFrontRight][yFrontRight] = dr[0];
				_drY[xFrontRight][yFrontRight] = dr[1];
			}
		}

		cellx += _frntx;
		celly += _frnty;
		if (wallAt(cellx + _sidexWall, celly + _sideyWall) & _side)
			left2 = MAX(screenLeft, _drX[xFrontLeft][yFrontLeft]);
		else
			left2 = MAX(left, left2);
		left2 = MAX(left2, screenLeft);
		if (wallAt(cellx + _sidexWall + _sidex, celly + _sideyWall + _sidey) & _side)
			right2 = _drX[xFrontRight][yFrontRight];
		else
			right2 = MIN(right, right2);
		if (cellx >= 0 && cellx < 32 && celly >= 0 && celly < 32) {
			if (_robotArray[cellx][celly])
				setRobot(left2, right2, _robotArray[cellx][celly]);
			if (_foodArray[cellx][celly])
				setRobot(left2, right2, _foodArray[cellx][celly]);
		}
		if (wallAt(cellx, celly) & ~0x03)
			features(cellx, celly, xFrontLeft, yFrontLeft, left2, right2, rox, roy);

		_gfx->drawLine(_drX[xFrontLeft][yFrontLeft], _drY[xFrontLeft][yFrontLeft],
		               _drX[xFrontRight][yFrontRight], _drY[xFrontRight][yFrontRight], white);

		length++;
		if (length > 30)
			break; // Safety break
	}
	drawend(xfbehind, yfbehind, xFrontLeft, yFrontLeft);

	left = screenLeft;
	right = MIN(right, _drX[xFrontLeft][yFrontLeft]);
	if (left < right)
		checkleft(xsstart, ysstart, xfbehind, yfbehind, left, right, roxsave, roysave, cellxsave, cellysave, length);

	left = MAX(left, _drX[xFrontRight][yFrontRight]);
	if (left < screenLeft)
		left = screenLeft;
	right = screenRight;
	xsstart += _sidex;
	ysstart += _sidey;
	xfbehind += _sidex;
	yfbehind += _sidey;
	if (left < right)
		checkright(xsstart, ysstart, xfbehind, yfbehind, left, right, roxsave + _tsin, roysave + _tcos, cellxsave, cellysave, length);

	_change = false;
}

void ColonyEngine::drawend(int xstart, int ystart, int xFrontLeft, int yFrontLeft) {
	int xFrontRight, yFrontRight;

	xFrontRight = xFrontLeft + _sidex;
	yFrontRight = yFrontLeft + _sidey;

	uint32 white = _gfx->white();

	if ((xstart != xFrontLeft) || (ystart != yFrontLeft)) {
		if (_drY[xstart + _frntx][ystart + _frnty] > 0) {
			_gfx->drawLine(_drX[xstart][ystart], _drY[xstart][ystart],
			               _drX[xstart + _frntx][ystart + _frnty], _drY[xstart + _frntx][ystart + _frnty], white);
		}
		_gfx->drawLine(_drX[xstart + _frntx][ystart + _frnty], _drY[xstart + _frntx][ystart + _frnty],
		               _drX[xFrontLeft][yFrontLeft], _drY[xFrontLeft][yFrontLeft], white);
		_gfx->drawLine(_drX[xFrontLeft][yFrontLeft], _drY[xFrontLeft][yFrontLeft],
		               _drX[xFrontLeft][yFrontLeft], _height - _drY[xFrontLeft][yFrontLeft], white);
		_gfx->drawLine(_drX[xFrontLeft][yFrontLeft], _height - _drY[xFrontLeft][yFrontLeft],
		               _drX[xFrontRight][yFrontRight], _height - _drY[xFrontRight][yFrontRight], white);
		_gfx->drawLine(_drX[xFrontRight][yFrontRight], _height - _drY[xFrontRight][yFrontRight],
		               _drX[xFrontRight][yFrontRight], _drY[xFrontRight][yFrontRight], white);
		_gfx->drawLine(_drX[xFrontRight][yFrontRight], _drY[xFrontRight][yFrontRight],
		               _drX[xstart + _sidex][ystart + _sidey], _drY[xstart + _sidex][ystart + _sidey], white);
		if (_drY[xstart + _sidex][ystart + _sidey] > 0) {
			_gfx->drawLine(_drX[xstart + _sidex][ystart + _sidey], _drY[xstart + _sidex][ystart + _sidey],
			               _drX[xstart + _frntx + _sidex][ystart + _frnty + _sidey], _drY[xstart + _frntx + _sidex][ystart + _frnty + _sidey], white);
		}
	} else {
		_gfx->drawLine(_drX[xFrontLeft][yFrontLeft], _drY[xFrontLeft][yFrontLeft],
		               _drX[xFrontLeft][yFrontLeft], _height - _drY[xFrontLeft][yFrontLeft], white);
		_gfx->drawLine(_drX[xFrontLeft][yFrontLeft], _height - _drY[xFrontLeft][yFrontLeft],
		               _drX[xFrontRight][yFrontRight], _height - _drY[xFrontRight][yFrontRight], white);
		_gfx->drawLine(_drX[xFrontRight][yFrontRight], _height - _drY[xFrontRight][yFrontRight],
		               _drX[xFrontRight][yFrontRight], _drY[xFrontRight][yFrontRight], white);
	}
}

uint8 ColonyEngine::wallAt(int x, int y) const {
	if (x < 0 || x >= 32 || y < 0 || y >= 32)
		return 3;
	return _wall[x][y];
}

void ColonyEngine::checkleft(int xs, int ys, int xf, int yf, int left, int right, int rx, int ry, int cellx, int celly, int len) {
	int i = 0, j;
	int xf2, yf2;
	int rox, roy;
	int xsstart, ysstart;
	int xfstart, yfstart;
	int xestart, yestart;
	int cellxsave, cellysave;
	int dr[2];
	uint32 white = _gfx->white();

	cellx -= _sidex;
	celly -= _sidey;
	rx = rx - _tsin;
	ry = ry - _tcos;

	while (i < len && left <= right) {
		if (wallAt(xs, ys) & _side) {
			if (_flip)
				_gfx->drawLine(_drX[xf][yf], 0, _drX[xf][yf], _height - _drY[xf][yf], white);

			while ((wallAt(xs, ys) & _side) && i < len && left <= right) {
				_gfx->drawLine(_drX[xf][yf], _drY[xf][yf], _drX[xf][yf], _height - _drY[xf][yf], white);
				_gfx->drawLine(_drX[xf][yf], _height - _drY[xf][yf],
				               _drX[xf + _frntx][yf + _frnty], _height - _drY[xf + _frntx][yf + _frnty], white);

				left = MAX(_drX[xf][yf], left);
				xf += _frntx;
				yf += _frnty;
				xs += _frntx;
				ys += _frnty;
				cellx += _frntx;
				celly += _frnty;
				rx -= _tcos;
				ry += _tsin;
				i++;
			}

			if (_flip)
				_gfx->drawLine(_drX[xf][yf], 0, _drX[xf][yf], _height - _drY[xf][yf], white);
			_gfx->drawLine(_drX[xf][yf], _height - _drY[xf][yf], _drX[xf][yf], _drY[xf][yf], white);
			left = MAX(_drX[xf][yf], left);
		}

		if (i < len && left <= right) {
			j = 0;
			xf2 = xf - _sidex;
			yf2 = yf - _sidey;
			xfstart = xf2;
			yfstart = yf2;
			xsstart = xs - _sidex;
			ysstart = ys - _sidey;
			cellxsave = cellx;
			cellysave = celly;

			rox = rx;
			roy = ry;
			if (_change) {
				perspective(dr, rx, ry);
				_drX[xf2][yf2] = dr[0];
				_drY[xf2][yf2] = dr[1];
			}

			while (!(wallAt(xs, ys) & _side) && i < len) {
				rx -= _tcos;
				ry += _tsin;
				if (_change) {
					perspective(dr, rx, ry);
					_drX[xf2 + _frntx][yf2 + _frnty] = dr[0];
					_drY[xf2 + _frntx][yf2 + _frnty] = dr[1];
				}

				if (_drX[xf + _frntx][yf + _frnty] > left) {
					_gfx->drawLine(_drX[xf2][yf2], _drY[xf2][yf2],
					               _drX[xf2 + _frntx][yf2 + _frnty], _drY[xf2 + _frntx][yf2 + _frnty], white);
					_gfx->drawLine(_drX[xf2 + _frntx][yf2 + _frnty], _drY[xf2 + _frntx][yf2 + _frnty],
					               _drX[xf + _frntx][yf + _frnty], _drY[xf + _frntx][yf + _frnty], white);
					if (wallAt(cellx, celly) & ~0x03)
						features(cellx, celly, xf2 + _frntx, yf2 + _frnty, left, right, rx, ry);
					if (cellx >= 0 && cellx < 32 && celly >= 0 && celly < 32) {
						if (_robotArray[cellx][celly])
							setRobot(left, right, _robotArray[cellx][celly]);
						if (_foodArray[cellx][celly])
							setRobot(left, right, _foodArray[cellx][celly]);
					}
				} else {
					j = 0;
					xfstart = xf2;
					yfstart = yf2;
					xsstart = xs - _sidex;
					ysstart = ys - _sidey;
					rox = rx + _tcos;
					roy = ry - _tsin;
					cellxsave = cellx;
					cellysave = celly;
				}

				xf2 += _frntx;
				yf2 += _frnty;
				xf += _frntx;
				yf += _frnty;
				xs += _frntx;
				ys += _frnty;
				cellx += _frntx;
				celly += _frnty;
				i++;
				j++;
			}

			if (wallAt(xf - _sidex, yf - _sidey) & _front) {
				_gfx->drawLine(_drX[xf2][yf2], _drY[xf2][yf2], _drX[xf2][yf2], _height - _drY[xf2][yf2], white);
				_gfx->drawLine(_drX[xf2][yf2], _height - _drY[xf2][yf2], _drX[xf][yf], _height - _drY[xf][yf], white);

				if (MIN(_drX[xf2][yf2], right) >= left) {
					checkleft(xsstart, ysstart, xfstart, yfstart, left, MIN(right, _drX[xf2][yf2]),
					          rox, roy, cellxsave, cellysave, j);
				}
			} else {
				if (_flip)
					_gfx->drawLine(_drX[xf][yf], 0, _drX[xf][yf], _height - _drY[xf][yf], white);
				xestart = xf2;
				yestart = yf2;

				while (!(wallAt(xf2, yf2) & _front)) {
					rx -= _tcos;
					ry += _tsin;
					cellx += _frntx;
					celly += _frnty;
					xf2 += _frntx;
					yf2 += _frnty;
					xf += _frntx;
					yf += _frnty;
					xs += _frntx;
					ys += _frnty;
					if (_change) {
						perspective(dr, rx, ry);
						_drX[xf2][yf2] = dr[0];
						_drY[xf2][yf2] = dr[1];
					}
					if (_change) {
						perspective(dr, rx + _tsin, ry + _tcos);
						_drX[xf][yf] = dr[0];
						_drY[xf][yf] = dr[1];
					}
					_gfx->drawLine(_drX[xf2][yf2], _drY[xf2][yf2], _drX[xf][yf], _drY[xf][yf], white);
					if (wallAt(cellx - _frntx, celly - _frnty) & ~0x03)
						features(cellx - _frntx, celly - _frnty, xf2, yf2, left, right, rx, ry);
					const int objxL = cellx - _frntx;
					const int objyL = celly - _frnty;
					if (objxL >= 0 && objxL < 32 && objyL >= 0 && objyL < 32) {
						if (_robotArray[objxL][objyL])
							setRobot(left, right, _robotArray[objxL][objyL]);
						if (_foodArray[objxL][objyL])
							setRobot(left, right, _foodArray[objxL][objyL]);
					}
					i++;
					j++;
				}

				_gfx->drawLine(_drX[xestart][yestart], _drY[xestart][yestart], _drX[xf2][yf2], _drY[xf2][yf2], white);
				_gfx->drawLine(_drX[xf2][yf2], _drY[xf2][yf2], _drX[xf2][yf2], _height - _drY[xf2][yf2], white);
				_gfx->drawLine(_drX[xf2][yf2], _height - _drY[xf2][yf2],
				               _drX[xf2 + _sidex][yf2 + _sidey], _height - _drY[xf2 + _sidex][yf2 + _sidey], white);

				if (MIN(_drX[xf2][yf2], right) >= left) {
					checkleft(xsstart, ysstart, xfstart, yfstart, left, MIN(_drX[xf2][yf2], right),
					          rox, roy, cellxsave, cellysave, j);
				}
			}
		}
	}
}

void ColonyEngine::checkright(int xs, int ys, int xf, int yf, int left, int right, int rx, int ry, int cellx, int celly, int len) {
	int i = 0, j;
	int xf2, yf2;
	int rox, roy;
	int xsstart, ysstart;
	int xfstart, yfstart;
	int xestart, yestart;
	int cellxsave, cellysave;
	int dr[2];
	uint32 white = _gfx->white();

	cellx += _sidex;
	celly += _sidey;
	rx = rx + _tsin;
	ry = ry + _tcos;

	while (i < len && left < right) {
		if (wallAt(xs, ys) & _side) {
			if (_flip)
				_gfx->drawLine(_drX[xf][yf], 0, _drX[xf][yf], _height - _drY[xf][yf], white);

			while ((wallAt(xs, ys) & _side) && i < len && left < right) {
				_gfx->drawLine(_drX[xf][yf], _drY[xf][yf], _drX[xf][yf], _height - _drY[xf][yf], white);
				_gfx->drawLine(_drX[xf][yf], _height - _drY[xf][yf],
				               _drX[xf + _frntx][yf + _frnty], _height - _drY[xf + _frntx][yf + _frnty], white);

				right = MIN(_drX[xf][yf], right);
				xf += _frntx;
				yf += _frnty;
				xs += _frntx;
				ys += _frnty;
				cellx += _frntx;
				celly += _frnty;
				rx -= _tcos;
				ry += _tsin;
				i++;
			}

			if (_flip)
				_gfx->drawLine(_drX[xf][yf], 0, _drX[xf][yf], _height - _drY[xf][yf], white);
			_gfx->drawLine(_drX[xf][yf], _height - _drY[xf][yf], _drX[xf][yf], _drY[xf][yf], white);
			right = MIN(_drX[xf][yf], right);
		}

		if (i < len && left < right) {
			j = 0;
			xf2 = xf + _sidex;
			yf2 = yf + _sidey;
			xfstart = xf2;
			yfstart = yf2;
			xsstart = xs + _sidex;
			ysstart = ys + _sidey;
			cellxsave = cellx;
			cellysave = celly;

			rox = rx;
			roy = ry;
			if (_change) {
				perspective(dr, rx, ry);
				_drX[xf2][yf2] = dr[0];
				_drY[xf2][yf2] = dr[1];
			}

			while (!(wallAt(xs, ys) & _side) && i < len) {
				rx -= _tcos;
				ry += _tsin;
				if (_change) {
					perspective(dr, rx, ry);
					_drX[xf2 + _frntx][yf2 + _frnty] = dr[0];
					_drY[xf2 + _frntx][yf2 + _frnty] = dr[1];
				}

				if (_drX[xf + _frntx][yf + _frnty] < right) {
					_gfx->drawLine(_drX[xf2][yf2], _drY[xf2][yf2],
					               _drX[xf2 + _frntx][yf2 + _frnty], _drY[xf2 + _frntx][yf2 + _frnty], white);
					_gfx->drawLine(_drX[xf2 + _frntx][yf2 + _frnty], _drY[xf2 + _frntx][yf2 + _frnty],
					               _drX[xf + _frntx][yf + _frnty], _drY[xf + _frntx][yf + _frnty], white);
					if (wallAt(cellx, celly) & ~0x03)
						features(cellx, celly, xf + _frntx, yf + _frnty, left, right, rx - _tsin, ry - _tcos);
					if (cellx >= 0 && cellx < 32 && celly >= 0 && celly < 32) {
						if (_robotArray[cellx][celly])
							setRobot(left, right, _robotArray[cellx][celly]);
						if (_foodArray[cellx][celly])
							setRobot(left, right, _foodArray[cellx][celly]);
					}
				} else {
					j = 0;
					xfstart = xf2;
					yfstart = yf2;
					xsstart = xs + _sidex;
					ysstart = ys + _sidey;
					rox = rx + _tcos;
					roy = ry - _tsin;
					cellxsave = cellx;
					cellysave = celly;
				}

				xf2 += _frntx;
				yf2 += _frnty;
				xf += _frntx;
				yf += _frnty;
				xs += _frntx;
				ys += _frnty;
				cellx += _frntx;
				celly += _frnty;
				i++;
				j++;
			}

			if (wallAt(xf, yf) & _front) {
				_gfx->drawLine(_drX[xf2][yf2], _drY[xf2][yf2], _drX[xf2][yf2], _height - _drY[xf2][yf2], white);
				_gfx->drawLine(_drX[xf2][yf2], _height - _drY[xf2][yf2], _drX[xf][yf], _height - _drY[xf][yf], white);

				if (MAX(_drX[xf2][yf2], left) < right) {
					checkright(xsstart, ysstart, xfstart, yfstart, MAX(left, _drX[xf2][yf2]), right,
					           rox, roy, cellxsave, cellysave, j);
				}
			} else {
				if (_flip)
					_gfx->drawLine(_drX[xf][yf], 0, _drX[xf][yf], _height - _drY[xf][yf], white);
				xestart = xf2;
				yestart = yf2;

				while (!(wallAt(xf, yf) & _front)) {
					rx -= _tcos;
					ry += _tsin;
					cellx += _frntx;
					celly += _frnty;
					xf2 += _frntx;
					yf2 += _frnty;
					xf += _frntx;
					yf += _frnty;
					xs += _frntx;
					ys += _frnty;
					if (_change) {
						perspective(dr, rx, ry);
						_drX[xf2][yf2] = dr[0];
						_drY[xf2][yf2] = dr[1];
					}
					if (_change) {
						perspective(dr, rx - _tsin, ry - _tcos);
						_drX[xf][yf] = dr[0];
						_drY[xf][yf] = dr[1];
					}
					_gfx->drawLine(_drX[xf2][yf2], _drY[xf2][yf2], _drX[xf][yf], _drY[xf][yf], white);
					if (wallAt(cellx - _frntx, celly - _frnty) & ~0x03)
						features(cellx - _frntx, celly - _frnty, xf, yf, left, right, rx - _tsin, ry - _tcos);
					const int objxR = cellx - _frntx;
					const int objyR = celly - _frnty;
					if (objxR >= 0 && objxR < 32 && objyR >= 0 && objyR < 32) {
						if (_robotArray[objxR][objyR])
							setRobot(left, right, _robotArray[objxR][objyR]);
						if (_foodArray[objxR][objyR])
							setRobot(left, right, _foodArray[objxR][objyR]);
					}
					i++;
					j++;
				}

				_gfx->drawLine(_drX[xestart][yestart], _drY[xestart][yestart], _drX[xf2][yf2], _drY[xf2][yf2], white);
				_gfx->drawLine(_drX[xf2][yf2], _drY[xf2][yf2], _drX[xf2][yf2], _height - _drY[xf2][yf2], white);
				_gfx->drawLine(_drX[xf2][yf2], _height - _drY[xf2][yf2],
				               _drX[xf2 - _sidex][yf2 - _sidey], _height - _drY[xf2 - _sidex][yf2 - _sidey], white);

				if (MAX(_drX[xf2][yf2], left) < right) {
					checkright(xsstart, ysstart, xfstart, yfstart, MAX(_drX[xf2][yf2], left), right,
					           rox, roy, cellxsave, cellysave, j);
				}
			}
		}
	}
}

const uint8 *ColonyEngine::mapFeatureAt(int x, int y, int direction) const {
	if (x < 0 || x >= 31 || y < 0 || y >= 31 || direction < 0 || direction >= 5)
		return nullptr;
	return _mapData[x][y][direction];
}

void ColonyEngine::frontfeature(int cellx, int celly, int xFront, int yFront, int left, int right, int rx, int ry) {
	int l[4], r[4];

	l[0] = _drX[xFront][yFront];
	l[2] = rx - _tcos;
	l[3] = ry + _tsin;
	r[0] = _drX[xFront + _sidex][yFront + _sidey];
	r[2] = rx + _tsin - _tcos;
	r[3] = ry + _tsin + _tcos;
	if (_flip) {
		l[1] = _height - _drY[xFront][yFront];
		r[1] = _height - _drY[xFront + _sidex][yFront + _sidey];
	} else {
		l[1] = _drY[xFront][yFront];
		r[1] = _drY[xFront + _sidex][yFront + _sidey];
	}

	if (MAX(left, l[0]) < MIN(right, r[0])) {
		const uint8 *map = mapFeatureAt(cellx, celly, _direction);
		if (map && map[0])
			dowall(cellx, celly, _direction, l, r);
	}
}

void ColonyEngine::features(int cellx, int celly, int xFront, int yFront, int left, int right, int rx, int ry) {
	int l[4], r[4], ll[4], rr[4];

	l[0] = _drX[xFront][yFront];
	l[2] = rx - _tcos;
	l[3] = ry + _tsin;
	r[0] = _drX[xFront + _sidex][yFront + _sidey];
	r[2] = rx + _tsin - _tcos;
	r[3] = ry + _tsin + _tcos;
	if (_flip) {
		l[1] = _height - _drY[xFront][yFront];
		r[1] = _height - _drY[xFront + _sidex][yFront + _sidey];
	} else {
		l[1] = _drY[xFront][yFront];
		r[1] = _drY[xFront + _sidex][yFront + _sidey];
	}

	if (MAX(left, l[0]) + 1 < MIN(right, r[0]) - 1) {
		const uint8 *map = mapFeatureAt(cellx, celly, _direction);
		if (map && map[0])
			dowall(cellx, celly, _direction, l, r);
	}

	ll[0] = r[0];
	ll[1] = r[1];
	ll[2] = rx + _tsin + _tsin;
	ll[3] = ry + _tcos + _tcos;
	rr[0] = _drX[xFront + _sidex - _frntx][yFront + _sidey - _frnty];
	if (_flip)
		rr[1] = _height - _drY[xFront + _sidex - _frntx][yFront + _sidey - _frnty];
	else
		rr[1] = _drY[xFront + _sidex - _frntx][yFront + _sidey - _frnty];
	rr[2] = rx + _tsin + _tsin + _tcos;
	rr[3] = ry + _tcos + _tcos - _tsin;
	if (MAX(left, ll[0]) + 1 < MIN(right, rr[0]) - 1) {
		const uint8 *map = mapFeatureAt(cellx, celly, g_dirRight[_direction]);
		if (map && map[0])
			dowall(cellx, celly, g_dirRight[_direction], ll, rr);
	}

	ll[0] = _drX[xFront - _frntx][yFront - _frnty];
	if (_flip)
		ll[1] = _height - _drY[xFront - _frntx][yFront - _frnty];
	else
		ll[1] = _drY[xFront - _frntx][yFront - _frnty];
	ll[2] = rx + _tcos - _tsin;
	ll[3] = (ry - _tcos) - _tsin;
	rr[0] = l[0];
	rr[1] = l[1];
	rr[2] = rx - _tsin;
	rr[3] = ry - _tcos;
	if (MAX(left, ll[0]) + 1 < MIN(right, rr[0]) - 1) {
		const uint8 *map = mapFeatureAt(cellx, celly, g_dirLeft[_direction]);
		if (map && map[0])
			dowall(cellx, celly, g_dirLeft[_direction], ll, rr);
	}
}

void ColonyEngine::dowall(int cellx, int celly, int direction, int left[4], int right[4]) {
	const uint8 *map = mapFeatureAt(cellx, celly, direction);
	int left2[2], right2[2];
	if (!map)
		return;

	switch (map[0]) {
	case kWallFeatureDoor:
		if (_level == 1 || _level == 5 || _level == 6) {
			if (map[1] == 0)
				drawOpenSSDoor(left, right);
			else
				drawClosedSSDoor(left, right);
		} else {
			if (map[1] == 0) {
				perspective(left2, left[2], left[3]);
				perspective(right2, right[2], right[3]);
				if (_flip) {
					left2[1] = _height - left2[1];
					right2[1] = _height - right2[1];
				}
				drawOpenDoor(left, right, left2, right2);
			} else {
				drawClosedDoor(left, right);
			}
		}
		break;
	case kWallFeatureWindow:
		drawWindow(left, right);
		break;
	case kWallFeatureShelves:
		perspective(left2, left[2], left[3]);
		perspective(right2, right[2], right[3]);
		if (_flip) {
			left2[1] = _height - left2[1];
			right2[1] = _height - right2[1];
		}
		drawBooks(left, right, left2, right2);
		break;
	case kWallFeatureUpStairs:
		perspective(left2, left[2], left[3]);
		perspective(right2, right[2], right[3]);
		if (_flip) {
			left2[1] = _height - left2[1];
			right2[1] = _height - right2[1];
		}
		drawUpStairs(left, right, left2, right2);
		break;
	case kWallFeatureDnStairs:
		perspective(left2, left[2], left[3]);
		perspective(right2, right[2], right[3]);
		if (_flip) {
			left2[1] = _height - left2[1];
			right2[1] = _height - right2[1];
		}
		drawDnStairs(left, right, left2, right2);
		break;
	case kWallFeatureGlyph:
		drawGlyphs(left, right);
		break;
	case kWallFeatureElevator:
		drawElevator(left, right);
		break;
	case kWallFeatureTunnel:
		perspective(left2, left[2], left[3]);
		perspective(right2, right[2], right[3]);
		if (_flip) {
			left2[1] = _height - left2[1];
			right2[1] = _height - right2[1];
		}
		drawTunnel(left, right, left2, right2);
		break;
	case kWallFeatureAirlock:
		if (map[1] == 0)
			drawALOpen(left, right);
		else
			drawALClosed(left, right);
		break;
	case kWallFeatureColor:
		drawColor(map, left, right);
		break;
	default:
		break;
	}
}

void ColonyEngine::drawWindow(int left[4], int right[4]) {
	const uint32 dark = 160;
	int x1 = left[0];
	int x2 = right[0];
	int y1 = left[1];
	int y2 = right[1];
	int y3 = _height - right[1];
	int y4 = _height - left[1];
	int xc = (x1 + x2) >> 1;
	int xx1 = (xc + x1) >> 1;
	int xx2 = (xc + x2) >> 1;
	if (xx2 < _screenR.left || xx1 > _screenR.right)
		return;
	int yl = (y1 + y4) >> 1;
	int yr = (y2 + y3) >> 1;
	int yy1 = (yl + y1) >> 1;
	int yy2 = (yr + y2) >> 1;
	int yy3 = (yl + y3) >> 1;
	int yy4 = (yr + y4) >> 1;
	int yy[4];
	yy[0] = _height - ((((yy1 + yy2) >> 1) + yy1) >> 1);
	yy[1] = _height - ((((yy1 + yy2) >> 1) + yy2) >> 1);
	yy[2] = _height - ((((yy3 + yy4) >> 1) + yy3) >> 1);
	yy[3] = _height - ((((yy3 + yy4) >> 1) + yy4) >> 1);
	_gfx->drawLine(xx1, yy[0], xx2, yy[1], dark);
	_gfx->drawLine(xx2, yy[1], xx2, yy[2], dark);
	_gfx->drawLine(xx2, yy[2], xx1, yy[3], dark);
	_gfx->drawLine(xx1, yy[3], xx1, yy[0], dark);
	_gfx->drawLine(xc, (yy[0] + yy[1]) >> 1, xc, (yy[2] + yy[3]) >> 1, dark);
	_gfx->drawLine(xx1, yl, xx2, yr, dark);
}

void ColonyEngine::drawClosedDoor(int left[4], int right[4]) {
	const uint32 dark = 160;
	int x1 = left[0];
	int x2 = right[0];
	int y1 = left[1];
	int y2 = right[1];
	int y3 = _height - right[1];
	int y4 = _height - left[1];
	int xc = (x1 + x2) >> 1;
	int xx1 = (xc + x1) >> 1;
	int xx2 = (xc + x2) >> 1;
	if (xx2 < _screenR.left || xx1 > _screenR.right)
		return;

	int yc = (y1 + y2) >> 1;
	int ytl = (yc + y1) >> 1;
	int ytr = (yc + y2) >> 1;
	yc = (y4 + y3) >> 1;
	int ybl = (yc + y4) >> 1;
	int ybr = (yc + y3) >> 1;
	ytl = (((((ybl + ytl) >> 1) + ytl) >> 1) + ytl) >> 1;
	ytr = (((((ybr + ytr) >> 1) + ytr) >> 1) + ytr) >> 1;

	_gfx->drawLine(xx1, ybl, xx1, ytl, dark);
	_gfx->drawLine(xx1, ytl, xx2, ytr, dark);
	_gfx->drawLine(xx2, ytr, xx2, ybr, dark);
	_gfx->drawLine(xx2, ybr, xx1, ybl, dark);

	ybl = (ybl + ytl) >> 1;
	ybr = (ybr + ytr) >> 1;
	yc = (ybl + ybr) >> 1;
	ybl = (((yc + ybl) >> 1) + ybl) >> 1;
	ybr = (((yc + ybr) >> 1) + ybr) >> 1;
	xx1 = (((xx1 + xc) >> 1) + xx1) >> 1;
	xx2 = (((xx2 + xc) >> 1) + xx2) >> 1;
	_gfx->drawLine(xx1, ybl, xx2, ybr, dark);
}

void ColonyEngine::drawOpenDoor(int left[4], int right[4], int left2[2], int right2[2]) {
	const uint32 dark = 160;
	const uint32 light = 210;
	int x1 = left[0];
	int x2 = right[0];
	int y1 = left[1];
	int y2 = right[1];
	int y3 = _height - right[1];
	int y4 = _height - left[1];
	int xc = (x1 + x2) >> 1;
	int xl = (xc + x1) >> 1;
	int xr = (xc + x2) >> 1;
	int yc = (y1 + y2) >> 1;
	int ytl = (yc + y1) >> 1;
	int ytr = (yc + y2) >> 1;
	yc = (y4 + y3) >> 1;
	int ybl = (yc + y4) >> 1;
	int ybr = (yc + y3) >> 1;
	ytl = (((((ybl + ytl) >> 1) + ytl) >> 1) + ytl) >> 1;
	ytr = (((((ybr + ytr) >> 1) + ytr) >> 1) + ytr) >> 1;
	if (xr < _screenR.left || xl > _screenR.right)
		return;

	_gfx->drawLine(xl, ybl, xl, ytl, dark);
	_gfx->drawLine(xl, ytl, xr, ytr, dark);
	_gfx->drawLine(xr, ytr, xr, ybr, dark);
	_gfx->drawLine(xr, ybr, xl, ybl, dark);

	x1 = left2[0];
	x2 = right2[0];
	y1 = _height - left2[1];
	y2 = _height - right2[1];
	xc = (x1 + x2) >> 1;
	int xfl = (xc + x1) >> 1;
	int xfr = (xc + x2) >> 1;
	yc = (y1 + y2) >> 1;
	int yfl = (yc + y1) >> 1;
	int yfr = (yc + y2) >> 1;

	_gfx->drawLine(xl, ybl, xfl, yfl, light);
	_gfx->drawLine(xfl, yfl, xfr, yfr, light);
	_gfx->drawLine(xfr, yfr, xr, ybr, light);
	_gfx->drawLine(xr, ybr, xl, ybl, light);
}

void ColonyEngine::drawTunnel(int left[4], int right[4], int left2[2], int right2[2]) {
	const uint32 dark = 120;
	int baseX[7], baseY[7], tunnelY[7][7];
	int xl = left[0];
	int xr = right[0];
	int ytl = left[1];
	int ytr = right[1];
	int ybr = _height - right[1];
	int ybl = _height - left[1];
	int hl = ybl - ytl;
	int hr = ybr - ytr;
	(void)left2;
	(void)right2;
	(void)MAX(hl, hr);
	split7(baseX, xl, xr);
	if (baseX[0] > _screenR.right || baseX[6] < _screenR.left)
		return;
	split7(baseY, ybl, ybr);
	for (int i = 0; i < 7; i++)
		split7(tunnelY[i], baseY[i], _height - baseY[i]);

	int x[6] = {baseX[0], baseX[0], baseX[1], baseX[5], baseX[6], baseX[6]};
	int y[6] = {baseY[0], tunnelY[0][5], tunnelY[1][6], tunnelY[5][6], tunnelY[6][5], baseY[6]};
	for (int i = 0; i < 6; i++) {
		int n = (i + 1) % 6;
		_gfx->drawLine(x[i], y[i], x[n], y[n], dark);
	}
}

void ColonyEngine::drawGlyphs(int left[4], int right[4]) {
	const uint32 dark = 170;
	int xl = left[0];
	int xr = right[0];
	int y1 = left[1];
	int y2 = right[1];
	int y3 = _height - right[1];
	int y4 = _height - left[1];
	int xc = (xl + xr) >> 1;
	xl = (((xc + xl) >> 1) + xl) >> 1;
	xr = (((xc + xr) >> 1) + xr) >> 1;
	int ytc = (y1 + y2) >> 1;
	int ybc = (y3 + y4) >> 1;
	int ytl = (((y1 + ytc) >> 1) + y1) >> 1;
	int ytr = (((y2 + ytc) >> 1) + y2) >> 1;
	int ybl = (((y4 + ybc) >> 1) + y4) >> 1;
	int ybr = (((y3 + ybc) >> 1) + y3) >> 1;
	int yl1 = (ytl + ybl) >> 1;
	int yr1 = (ytr + ybr) >> 1;
	int yl2 = (yl1 + ytl) >> 1;
	int yr2 = (yr1 + ytr) >> 1;
	int yl3 = (yl2 + yl1) >> 1;
	int yr3 = (yr2 + yr1) >> 1;
	int yl4 = (yl1 + ybl) >> 1;
	int yr4 = (yr1 + ybr) >> 1;
	int yr5 = (yr4 + yr1) >> 1;
	int yl5 = (yl4 + yl1) >> 1;

	_gfx->drawLine(xl, yl1, xr, yr1, dark);
	_gfx->drawLine(xl, yl2, xr, yr2, dark);
	_gfx->drawLine(xl, yl3, xr, yr3, dark);
	_gfx->drawLine(xl, yl4, xr, yr4, dark);
	_gfx->drawLine(xl, yl5, xr, yr5, dark);
	_gfx->drawLine(xl, (yl2 + yl3) >> 1, xr, (yr2 + yr3) >> 1, dark);
	_gfx->drawLine(xl, (yl3 + yl1) >> 1, xr, (yr3 + yr1) >> 1, dark);
	_gfx->drawLine(xl, (yl1 + yl5) >> 1, xr, (yr1 + yr5) >> 1, dark);
	_gfx->drawLine(xl, (yl4 + yl5) >> 1, xr, (yr4 + yr5) >> 1, dark);
}

void ColonyEngine::drawBooks(int left[4], int right[4], int left2[2], int right2[2]) {
	const uint32 dark = 170;
	int l2[2] = {left2[0], left2[1]};
	int r2[2] = {right2[0], right2[1]};
	for (int i = 0; i < 2; i++) {
		l2[0] = (l2[0] + left[0]) >> 1;
		l2[1] = (l2[1] + left[1]) >> 1;
		r2[0] = (r2[0] + right[0]) >> 1;
		r2[1] = (r2[1] + right[1]) >> 1;
	}
	_gfx->drawLine(l2[0], l2[1], l2[0], _height - l2[1], dark);
	_gfx->drawLine(l2[0], _height - l2[1], r2[0], _height - r2[1], dark);
	_gfx->drawLine(r2[0], _height - r2[1], r2[0], r2[1], dark);
	_gfx->drawLine(r2[0], r2[1], l2[0], l2[1], dark);
	_gfx->drawLine(left[0], left[1], l2[0], l2[1], dark);
	_gfx->drawLine(left[0], _height - left[1], l2[0], _height - l2[1], dark);
	_gfx->drawLine(right[0], right[1], r2[0], r2[1], dark);
	_gfx->drawLine(right[0], _height - right[1], r2[0], _height - r2[1], dark);

	int lf[7], rf[7], lb[7], rb[7];
	split7(lf, left[1], _height - left[1]);
	split7(rf, right[1], _height - right[1]);
	split7(lb, l2[1], _height - l2[1]);
	split7(rb, r2[1], _height - r2[1]);
	for (int i = 0; i < 7; i++) {
		_gfx->drawLine(left[0], lf[i], right[0], rf[i], dark);
		_gfx->drawLine(right[0], rf[i], r2[0], rb[i], dark);
		_gfx->drawLine(r2[0], rb[i], l2[0], lb[i], dark);
		_gfx->drawLine(l2[0], lb[i], left[0], lf[i], dark);
	}
}

void ColonyEngine::drawUpStairs(int left[4], int right[4], int left2[2], int right2[2]) {
	const uint32 dark = 170;
	int xl[7], xr[7], yl[7], yr[7];
	split7(xl, left[0], left2[0]);
	split7(xr, right[0], right2[0]);
	split7(yl, _height - left[1], left2[1]);
	split7(yr, _height - right[1], right2[1]);
	for (int i = 0; i < 6; i++) {
		_gfx->drawLine(xl[i], yl[i], xl[i + 1], yl[i + 1], dark);
		_gfx->drawLine(xr[i], yr[i], xr[i + 1], yr[i + 1], dark);
		_gfx->drawLine(xl[i], yl[i], xr[i], yr[i], dark);
	}
}

void ColonyEngine::drawDnStairs(int left[4], int right[4], int left2[2], int right2[2]) {
	const uint32 dark = 170;
	int xl[7], xr[7], yl[7], yr[7];
	split7(xl, left[0], left2[0]);
	split7(xr, right[0], right2[0]);
	split7(yl, left[1], left2[1]);
	split7(yr, right[1], right2[1]);
	for (int i = 0; i < 6; i++) {
		_gfx->drawLine(xl[i], yl[i], xl[i + 1], yl[i + 1], dark);
		_gfx->drawLine(xr[i], yr[i], xr[i + 1], yr[i + 1], dark);
		_gfx->drawLine(xl[i], _height - yl[i], xr[i], _height - yr[i], dark);
	}
}

void ColonyEngine::drawALOpen(int left[4], int right[4]) {
	const uint32 dark = 150;
	int lr[7], ud[7][7];
	split7x7(left, right, lr, ud);
	int x[8] = {lr[0], lr[1], lr[3], lr[5], lr[6], lr[5], lr[3], lr[1]};
	int y[8] = {ud[3][0], ud[5][1], ud[6][3], ud[5][5], ud[3][6], ud[1][5], ud[0][3], ud[1][1]};
	for (int i = 0; i < 8; i++) {
		int n = (i + 1) % 8;
		_gfx->drawLine(x[i], y[i], x[n], y[n], dark);
	}
}

void ColonyEngine::drawALClosed(int left[4], int right[4]) {
	const uint32 dark = 170;
	int lr[7], ud[7][7];
	split7x7(left, right, lr, ud);
	int x[8] = {lr[0], lr[1], lr[3], lr[5], lr[6], lr[5], lr[3], lr[1]};
	int y[8] = {ud[3][0], ud[5][1], ud[6][3], ud[5][5], ud[3][6], ud[1][5], ud[0][3], ud[1][1]};
	for (int i = 0; i < 8; i++) {
		int n = (i + 1) % 8;
		_gfx->drawLine(x[i], y[i], x[n], y[n], dark);
	}
	_gfx->drawLine(lr[0], ud[3][0], lr[3], ud[3][3], dark);
	_gfx->drawLine(lr[3], ud[6][3], lr[3], ud[3][3], dark);
	_gfx->drawLine(lr[6], ud[3][6], lr[3], ud[3][3], dark);
	_gfx->drawLine(lr[3], ud[0][3], lr[3], ud[3][3], dark);
}

void ColonyEngine::drawOpenSSDoor(int left[4], int right[4]) {
	const uint32 dark = 140;
	int lr[7], ud[7][7];
	split7x7(left, right, lr, ud);
	int x[8] = {lr[2], lr[1], lr[1], lr[2], lr[4], lr[5], lr[5], lr[4]};
	int y[8] = {ud[0][2], ud[1][1], ud[5][1], ud[6][2], ud[6][4], ud[5][5], ud[1][5], ud[0][4]};
	for (int i = 0; i < 8; i++) {
		int n = (i + 1) % 8;
		_gfx->drawLine(x[i], y[i], x[n], y[n], dark);
	}
}

void ColonyEngine::drawClosedSSDoor(int left[4], int right[4]) {
	const uint32 dark = 170;
	int lr[7], ud[7][7];
	split7x7(left, right, lr, ud);
	int x[8] = {lr[2], lr[1], lr[1], lr[2], lr[4], lr[5], lr[5], lr[4]};
	int y[8] = {ud[0][2], ud[1][1], ud[5][1], ud[6][2], ud[6][4], ud[5][5], ud[1][5], ud[0][4]};
	for (int i = 0; i < 8; i++) {
		int n = (i + 1) % 8;
		_gfx->drawLine(x[i], y[i], x[n], y[n], dark);
	}
	_gfx->drawLine(lr[2], ud[1][2], lr[2], ud[5][2], dark);
	_gfx->drawLine(lr[2], ud[5][2], lr[4], ud[5][4], dark);
	_gfx->drawLine(lr[4], ud[5][4], lr[4], ud[1][4], dark);
	_gfx->drawLine(lr[4], ud[1][4], lr[2], ud[1][2], dark);
}

void ColonyEngine::drawElevator(int left[4], int right[4]) {
	const uint32 dark = 170;
	int x1 = left[0];
	int x2 = right[0];
	int y1 = left[1];
	int y2 = right[1];
	int y3 = _height - right[1];
	int y4 = _height - left[1];
	int xc = (x1 + x2) >> 1;
	int xx1 = (xc + x1) >> 1;
	xx1 = (x1 + xx1) >> 1;
	int xx2 = (xc + x2) >> 1;
	xx2 = (x2 + xx2) >> 1;
	if (xx2 < _screenR.left || xx1 > _screenR.right)
		return;
	int ytc = (y1 + y2) >> 1;
	int ytl = (ytc + y1) >> 1;
	ytl = (ytl + y1) >> 1;
	int ytr = (ytc + y2) >> 1;
	ytr = (ytr + y2) >> 1;
	int ybc = (y4 + y3) >> 1;
	int ybl = (ybc + y4) >> 1;
	ybl = (ybl + y4) >> 1;
	int ybr = (ybc + y3) >> 1;
	ybr = (ybr + y3) >> 1;
	ytl = (((((ybl + ytl) >> 1) + ytl) >> 1) + ytl) >> 1;
	ytr = (((((ybr + ytr) >> 1) + ytr) >> 1) + ytr) >> 1;
	_gfx->drawLine(xx1, ybl, xx1, ytl, dark);
	_gfx->drawLine(xx1, ytl, xx2, ytr, dark);
	_gfx->drawLine(xx2, ytr, xx2, ybr, dark);
	_gfx->drawLine(xx2, ybr, xx1, ybl, dark);
	_gfx->drawLine(xc, ybc, xc, (ytl + ytr) >> 1, dark);
}

void ColonyEngine::drawColor(const uint8 *map, int left[4], int right[4]) {
	int xl = left[0];
	int xr = right[0];
	int yl[5], yr[5];
	yl[0] = left[1];
	yr[0] = right[1];
	yl[4] = _height - yl[0];
	yr[4] = _height - yr[0];
	yl[2] = (yl[0] + yl[4]) >> 1;
	yr[2] = (yr[0] + yr[4]) >> 1;
	yl[1] = (yl[0] + yl[2]) >> 1;
	yl[3] = (yl[2] + yl[4]) >> 1;
	yr[1] = (yr[0] + yr[2]) >> 1;
	yr[3] = (yr[2] + yr[4]) >> 1;

	if (map[1] || map[2] || map[3] || map[4]) {
		for (int i = 1; i <= 3; i++) {
			uint32 c = 120 + map[i] * 20;
			_gfx->drawLine(xl, yl[i], xr, yr[i], c);
		}
	} else {
		uint32 c = 100 + (_level * 15);
		_gfx->drawLine(xl, yl[1], xr, yr[1], c);
		_gfx->drawLine(xl, yl[2], xr, yr[2], c);
		_gfx->drawLine(xl, yl[3], xr, yr[3], c);
	}
}

void ColonyEngine::split7(int arr[7], int x1, int x2) const {
	arr[3] = (x1 + x2) >> 1;
	arr[1] = (x1 + arr[3]) >> 1;
	arr[0] = (x1 + arr[1]) >> 1;
	arr[2] = (arr[1] + arr[3]) >> 1;
	arr[5] = (arr[3] + x2) >> 1;
	arr[6] = (arr[5] + x2) >> 1;
	arr[4] = (arr[3] + arr[5]) >> 1;
}

void ColonyEngine::split7x7(int left[4], int right[4], int lr[7], int ud[7][7]) const {
	int leftX, rightX, leftY, rightY;
	int lud[7], rud[7];
	if (right[0] < left[0]) {
		rightX = left[0];
		leftX = right[0];
		rightY = left[1];
		leftY = right[1];
	} else {
		leftX = left[0];
		rightX = right[0];
		leftY = left[1];
		rightY = right[1];
	}
	split7(lr, leftX, rightX);
	if (_flip) {
		split7(lud, leftY, _height - leftY);
		split7(rud, rightY, _height - rightY);
	} else {
		split7(lud, _height - leftY, leftY);
		split7(rud, _height - rightY, rightY);
	}
	for (int i = 0; i < 7; i++)
		split7(ud[i], lud[i], rud[i]);
}

bool ColonyEngine::projectWorld(int worldX, int worldY, int &screenX, int &depth) const {
	long x = worldX - _me.xloc;
	long y = worldY - _me.yloc;
	long tsin = _cost[_me.look];
	long tcos = _sint[_me.look];
	long xx = (x * tcos - y * tsin) >> 7;
	long yy = (x * tsin + y * tcos) >> 7;

	if (yy <= 16)
		return false;
	if (yy >= 11585)
		yy = 11584;

	screenX = _centerX + (int)(((int64)xx * 256) / yy);
	depth = (int)yy;
	return true;
}

uint32 ColonyEngine::objectColor(int type) const {
	switch (type) {
	case 21: // DESK
		return 220;
	case 22: // PLANT
		return 100;
	case 24: // BED
	case 42: // BBED
		return 180;
	case 29: // SCREEN
	case 30: // CONSOLE
		return 240;
	case 31: // POWERSUIT
	case 46: // REACTOR
		return 255;
	case 36: // TELEPORT
		return 140;
	default:
		return 160 + ((uint32)(type * 7) & 0x3F);
	}
}

bool ColonyEngine::isSurfaceClockwise(const ProjectedPrismPart &part, const int surface[8]) const {
	const int n = surface[1];
	for (int i = 2; i < n; i++) {
		const int ia = surface[i];
		const int ib = surface[i + 1];
		const int ic = surface[i + 2];
		if (ia < 0 || ia >= part.pointCount || ib < 0 || ib >= part.pointCount || ic < 0 || ic >= part.pointCount)
			continue;
		const long dx = part.x[ia] - part.x[ib];
		const long dy = part.y[ia] - part.y[ib];
		const long dxp = part.x[ic] - part.x[ib];
		const long dyp = part.y[ic] - part.y[ib];
		if (dx < 0) {
			if (dy == 0) {
				if (dyp > 0)
					return false;
				if (dyp < 0)
					return true;
			} else {
				const long b = dy * dxp - dx * dyp;
				if (b > 0)
					return false;
				if (b < 0)
					return true;
			}
		} else if (dx > 0) {
			if (dy == 0) {
				if (dyp < 0)
					return false;
				if (dyp > 0)
					return true;
			} else {
				const long b = dx * dyp - dy * dxp;
				if (b < 0)
					return false;
				if (b > 0)
					return true;
			}
		} else {
			if (dy < 0) {
				if (dxp > 0)
					return true;
				if (dxp < 0)
					return false;
			}
			if (dy > 0) {
				if (dxp < 0)
					return true;
				if (dxp > 0)
					return false;
			}
		}
	}
	return false;
}

bool ColonyEngine::projectPrismPart(const Thing &obj, const PrismPartDef &part, bool useLook, ProjectedPrismPart &out) const {
	out.pointCount = CLIP<int>(part.pointCount, 0, ProjectedPrismPart::kMaxPoints);
	for (int i = 0; i < ProjectedPrismPart::kMaxSurfaces; i++)
		out.vsurface[i] = false;
	out.visible = false;
	if (out.pointCount <= 0 || !part.points || !part.surfaces)
		return false;

	const uint8 ang = useLook ? obj.where.look : obj.where.ang;
	const long rotCos = _cost[ang];
	const long rotSin = _sint[ang];
	const long viewSin = _cost[_me.look];
	const long viewCos = _sint[_me.look];

	int minX = 32000;
	int maxX = -32000;
	int minY = 32000;
	int maxY = -32000;
	// DOS InitObj() applies: Robot[i][j].pnt[k][2] -= Floor, with Floor == 160.
	// We keep source geometry unmodified and apply the same offset at projection time.
	static const int kFloorShift = 160;
	for (int i = 0; i < out.pointCount; i++) {
		const int px = part.points[i][0];
		const int py = part.points[i][1];
		const int pz = part.points[i][2];
		const long rx = ((long)px * rotCos - (long)py * rotSin) >> 7;
		const long ry = ((long)px * rotSin + (long)py * rotCos) >> 7;
		const long worldX = rx + obj.where.xloc;
		const long worldY = ry + obj.where.yloc;

		const long tx = worldX - _me.xloc;
		const long ty = worldY - _me.yloc;
		const long xx = (tx * viewCos - ty * viewSin) >> 7;
		long yy = (tx * viewSin + ty * viewCos) >> 7;
		if (yy <= 16)
			yy = 16;

		out.x[i] = _centerX + (int)(((int64)xx * 256) / yy);
		out.depth[i] = (int)yy;
		const long zrel = (long)pz - kFloorShift;
		out.y[i] = _centerY - (int)(((int64)zrel * 256) / yy);
		minX = MIN(minX, out.x[i]);
		maxX = MAX(maxX, out.x[i]);
		minY = MIN(minY, out.y[i]);
		maxY = MAX(maxY, out.y[i]);
	}

	out.visible = !(maxX < _screenR.left || minX >= _screenR.right || maxY < _screenR.top || minY >= _screenR.bottom);
	if (!out.visible)
		return false;

	const int surfCount = CLIP<int>(part.surfaceCount, 0, ProjectedPrismPart::kMaxSurfaces);
	for (int i = 0; i < surfCount; i++)
		out.vsurface[i] = isSurfaceClockwise(out, part.surfaces[i]);
	return true;
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
			if (y2 == y1)
				return false;
			x = x1 + (x2 - x1) * (b - y1) / (y2 - y1);
			y = b;
		} else if (cOut & 4) {
			if (y2 == y1)
				return false;
			x = x1 + (x2 - x1) * (t - y1) / (y2 - y1);
			y = t;
		} else if (cOut & 2) {
			if (x2 == x1)
				return false;
			y = y1 + (y2 - y1) * (r - x1) / (x2 - x1);
			x = r;
		} else {
			if (x2 == x1)
				return false;
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

void ColonyEngine::drawProjectedPrism(const ProjectedPrismPart &part, const PrismPartDef &def, int force, uint32 color, const Common::Rect &clip) {
	const int surfCount = CLIP<int>(def.surfaceCount, 0, ProjectedPrismPart::kMaxSurfaces);
	for (int i = 0; i < surfCount; i++) {
		if (!(part.vsurface[i] || force))
			continue;
		const int n = def.surfaces[i][1];
		if (n < 2)
			continue;
		int first = def.surfaces[i][2];
		if (first < 0 || first >= part.pointCount)
			continue;
		int prev = first;
		for (int j = 1; j < n; j++) {
			const int cur = def.surfaces[i][j + 2];
			if (cur < 0 || cur >= part.pointCount)
				continue;
			int x1 = part.x[prev];
			int y1 = part.y[prev];
			int x2 = part.x[cur];
			int y2 = part.y[cur];
			if (clipLineToRect(x1, y1, x2, y2, clip))
				_gfx->drawLine(x1, y1, x2, y2, color);
			prev = cur;
		}
		int x1 = part.x[prev];
		int y1 = part.y[prev];
		int x2 = part.x[first];
		int y2 = part.y[first];
		if (clipLineToRect(x1, y1, x2, y2, clip))
			_gfx->drawLine(x1, y1, x2, y2, color);
	}
}

bool ColonyEngine::drawStaticObjectPrisms(const Thing &obj, uint32 baseColor) {
	// DOS object geometry from SCREEN.H / TABLE.H / BED.H / DESK.H.
	static const int kScreenPts[8][3] = {
		{-16, 64, 0}, {16, 64, 0}, {16, -64, 0}, {-16, -64, 0},
		{-16, 64, 288}, {16, 64, 288}, {16, -64, 288}, {-16, -64, 288}
	};
	static const int kScreenSurf[4][8] = {
		{0, 4, 0, 3, 7, 4, 0, 0}, {0, 4, 3, 2, 6, 7, 0, 0},
		{0, 4, 1, 0, 4, 5, 0, 0}, {0, 4, 2, 1, 5, 6, 0, 0}
	};

	static const int kTableTopPts[4][3] = {
		{-128, 128, 100}, {128, 128, 100}, {128, -128, 100}, {-128, -128, 100}
	};
	static const int kTableTopSurf[1][8] = {{0, 4, 3, 2, 1, 0, 0, 0}};
	static const int kTableBasePts[8][3] = {
		{-5, 5, 0}, {5, 5, 0}, {5, -5, 0}, {-5, -5, 0},
		{-5, 5, 100}, {5, 5, 100}, {5, -5, 100}, {-5, -5, 100}
	};
	static const int kTableBaseSurf[4][8] = {
		{0, 4, 0, 3, 7, 4, 0, 0}, {0, 4, 3, 2, 6, 7, 0, 0},
		{0, 4, 1, 0, 4, 5, 0, 0}, {0, 4, 2, 1, 5, 6, 0, 0}
	};

	static const int kBedPostPts[4][3] = {
		{-82, 128, 100}, {82, 128, 100}, {82, 128, 0}, {-82, 128, 0}
	};
	static const int kBBedPostPts[4][3] = {
		{-130, 128, 100}, {130, 128, 100}, {130, 128, 0}, {-130, 128, 0}
	};
	static const int kBedPostSurf[1][8] = {{0, 4, 3, 2, 1, 0, 0, 0}};
	static const int kBlanketSurf[4][8] = {
		{0, 4, 0, 3, 7, 4, 0, 0}, {0, 4, 3, 2, 6, 7, 0, 0},
		{0, 4, 2, 1, 5, 6, 0, 0}, {0, 4, 7, 6, 5, 4, 0, 0}
	};
	static const int kSheetSurf[3][8] = {
		{0, 4, 0, 3, 7, 4, 0, 0}, {0, 4, 2, 1, 5, 6, 0, 0},
		{0, 4, 7, 6, 5, 4, 0, 0}
	};
	static const int kBedBlanketPts[8][3] = {
		{-80, 70, 0}, {80, 70, 0}, {80, -175, 0}, {-80, -175, 0},
		{-80, 70, 60}, {80, 70, 60}, {80, -175, 60}, {-80, -175, 60}
	};
	static const int kBedSheetPts[8][3] = {
		{-80, 128, 30}, {80, 128, 30}, {80, 70, 30}, {-80, 70, 30},
		{-80, 128, 60}, {80, 128, 60}, {80, 70, 60}, {-80, 70, 60}
	};
	static const int kBBedBlanketPts[8][3] = {
		{-128, 70, 0}, {128, 70, 0}, {128, -175, 0}, {-128, -175, 0},
		{-128, 70, 60}, {128, 70, 60}, {128, -175, 60}, {-128, -175, 60}
	};
	static const int kBBedSheetPts[8][3] = {
		{-128, 128, 30}, {128, 128, 30}, {128, 70, 30}, {-128, 70, 30},
		{-128, 128, 60}, {128, 128, 60}, {128, 70, 60}, {-128, 70, 60}
	};

	static const int kDeskTopPts[4][3] = {
		{-150, 110, 100}, {150, 110, 100}, {150, -110, 100}, {-150, -110, 100}
	};
	static const int kDeskTopSurf[1][8] = {{0, 4, 3, 2, 1, 0, 0, 0}};
	static const int kDeskLeftPts[8][3] = {
		{-135, 95, 0}, {-55, 95, 0}, {-55, -95, 0}, {-135, -95, 0},
		{-135, 95, 100}, {-55, 95, 100}, {-55, -95, 100}, {-135, -95, 100}
	};
	static const int kDeskRightPts[8][3] = {
		{55, 95, 0}, {135, 95, 0}, {135, -95, 0}, {55, -95, 0},
		{55, 95, 100}, {135, 95, 100}, {135, -95, 100}, {55, -95, 100}
	};
	static const int kDeskCabSurf[4][8] = {
		{0, 4, 0, 3, 7, 4, 0, 0}, {0, 4, 3, 2, 6, 7, 0, 0},
		{0, 4, 1, 0, 4, 5, 0, 0}, {0, 4, 2, 1, 5, 6, 0, 0}
	};
	static const int kSeatPts[4][3] = {
		{-40, 210, 60}, {40, 210, 60}, {40, 115, 60}, {-40, 115, 60}
	};
	static const int kSeatSurf[1][8] = {{0, 4, 3, 2, 1, 0, 0, 0}};
	static const int kArmLeftPts[4][3] = {
		{-40, 210, 90}, {-40, 210, 0}, {-40, 115, 0}, {-40, 115, 90}
	};
	static const int kArmRightPts[4][3] = {
		{40, 210, 90}, {40, 210, 0}, {40, 115, 0}, {40, 115, 90}
	};
	static const int kArmSurf[2][8] = {
		{0, 4, 3, 2, 1, 0, 0, 0}, {0, 4, 0, 1, 2, 3, 0, 0}
	};
	static const int kBackPts[4][3] = {
		{-40, 210, 130}, {40, 210, 130}, {40, 210, 70}, {-40, 210, 70}
	};
	static const int kBackSurf[2][8] = {
		{0, 4, 3, 2, 1, 0, 0, 0}, {0, 4, 0, 1, 2, 3, 0, 0}
	};
	static const int kComputerPts[8][3] = {
		{70, 25, 100}, {120, 25, 100}, {120, -25, 100}, {70, -25, 100},
		{70, 25, 120}, {120, 25, 120}, {120, -25, 120}, {70, -25, 120}
	};
	static const int kMonitorPts[8][3] = {
		{75, 20, 120}, {115, 20, 120}, {115, -20, 120}, {75, -20, 120},
		{75, 20, 155}, {115, 20, 155}, {115, -20, 145}, {75, -20, 145}
	};
	static const int kComputerSurf[5][8] = {
		{0, 4, 7, 6, 5, 4, 0, 0}, {0, 4, 0, 3, 7, 4, 0, 0},
		{0, 4, 3, 2, 6, 7, 0, 0}, {0, 4, 1, 0, 4, 5, 0, 0},
		{0, 4, 2, 1, 5, 6, 0, 0}
	};
	static const int kDeskScreenPts[4][3] = {
		{80, 20, 125}, {110, 20, 125}, {110, 20, 150}, {80, 20, 150}
	};
	static const int kDeskScreenSurf[1][8] = {{0, 4, 3, 2, 1, 0, 0, 0}};

	static const int kCSeatPts[4][3] = {
		{-40, 40, 60}, {40, 40, 60}, {40, -40, 60}, {-40, -40, 60}
	};
	static const int kCSeatSurf[1][8] = {{0, 4, 3, 2, 1, 0, 0, 0}};
	static const int kCArmLeftPts[4][3] = {
		{-50, 40, 90}, {-40, 40, 60}, {-40, -40, 60}, {-50, -40, 90}
	};
	static const int kCArmLeftSurf[1][8] = {{0, 4, 3, 2, 1, 0, 0, 0}};
	static const int kCArmRightPts[4][3] = {
		{50, 40, 90}, {40, 40, 60}, {40, -40, 60}, {50, -40, 90}
	};
	static const int kCArmRightSurf[1][8] = {{0, 4, 3, 2, 1, 0, 0, 0}};
	static const int kCBackPts[4][3] = {
		{-20, 60, 150}, {20, 60, 150}, {40, 40, 60}, {-40, 40, 60}
	};
	static const int kCBackSurf[2][8] = {
		{0, 4, 3, 2, 1, 0, 0, 0}, {0, 4, 0, 1, 2, 3, 0, 0}
	};
	static const int kCBasePts[8][3] = {
		{-5, 5, 0}, {5, 5, 0}, {5, -5, 0}, {-5, -5, 0},
		{-5, 5, 60}, {5, 5, 60}, {5, -5, 60}, {-5, -5, 60}
	};
	static const int kCBaseSurf[4][8] = {
		{0, 4, 0, 3, 7, 4, 0, 0}, {0, 4, 3, 2, 6, 7, 0, 0},
		{0, 4, 1, 0, 4, 5, 0, 0}, {0, 4, 2, 1, 5, 6, 0, 0}
	};

	static const int kConsolePts[8][3] = {
		{-5, 5, 0}, {5, 5, 0}, {5, -5, 0}, {-5, -5, 0},
		{-100, 70, 100}, {-35, 70, 140}, {-35, -70, 140}, {-100, -70, 100}
	};
	static const int kConsoleSurf[5][8] = {
		{0, 4, 4, 0, 3, 7, 0, 0}, {0, 4, 7, 3, 2, 6, 0, 0},
		{0, 4, 5, 1, 0, 4, 0, 0}, {0, 4, 6, 2, 1, 5, 0, 0},
		{0, 4, 7, 6, 5, 4, 0, 0}
	};

	static const int kCouchSurf[5][8] = {
		{0, 4, 0, 3, 7, 4, 0, 0}, {0, 4, 3, 2, 6, 7, 0, 0},
		{0, 4, 1, 0, 4, 5, 0, 0}, {0, 4, 2, 1, 5, 6, 0, 0},
		{0, 4, 7, 6, 5, 4, 0, 0}
	};
	static const int kACouchPts[8][3] = {
		{-50, 150, 0}, {50, 150, 0}, {50, -150, 0}, {-50, -150, 0},
		{-50, 150, 50}, {50, 150, 50}, {50, -150, 50}, {-50, -150, 50}
	};
	static const int kBCouchPts[8][3] = {
		{-80, 150, 0}, {-45, 150, 0}, {-45, -150, 0}, {-80, -150, 0},
		{-80, 150, 120}, {-55, 150, 120}, {-55, -150, 120}, {-80, -150, 120}
	};
	static const int kCCouchPts[8][3] = {
		{-70, 170, 0}, {50, 170, 0}, {50, 150, 0}, {-70, 150, 0},
		{-70, 170, 80}, {50, 170, 80}, {50, 150, 80}, {-70, 150, 80}
	};
	static const int kDCouchPts[8][3] = {
		{-70, -150, 0}, {50, -150, 0}, {50, -170, 0}, {-70, -170, 0},
		{-70, -150, 80}, {50, -150, 80}, {50, -170, 80}, {-70, -170, 80}
	};

	static const int kAChairPts[8][3] = {
		{-50, 50, 0}, {50, 50, 0}, {50, -50, 0}, {-50, -50, 0},
		{-50, 50, 50}, {50, 50, 50}, {50, -50, 50}, {-50, -50, 50}
	};
	static const int kBChairPts[8][3] = {
		{-80, 50, 0}, {-45, 50, 0}, {-45, -50, 0}, {-80, -50, 0},
		{-80, 50, 120}, {-55, 50, 120}, {-55, -50, 120}, {-80, -50, 120}
	};
	static const int kCChairPts2[8][3] = {
		{-70, 70, 0}, {50, 70, 0}, {50, 50, 0}, {-70, 50, 0},
		{-70, 70, 80}, {50, 70, 80}, {50, 50, 80}, {-70, 50, 80}
	};
	static const int kDChairPts[8][3] = {
		{-70, -50, 0}, {50, -50, 0}, {50, -70, 0}, {-70, -70, 0},
		{-70, -50, 80}, {50, -50, 80}, {50, -70, 80}, {-70, -70, 80}
	};

	static const int kTVBodyPts[8][3] = {
		{-30, 60, 0}, {30, 60, 0}, {30, -60, 0}, {-30, -60, 0},
		{-30, 60, 120}, {30, 60, 120}, {30, -60, 120}, {-30, -60, 120}
	};
	static const int kTVBodySurf[5][8] = {
		{0, 4, 0, 3, 7, 4, 0, 0}, {0, 4, 3, 2, 6, 7, 0, 0},
		{0, 4, 1, 0, 4, 5, 0, 0}, {0, 4, 2, 1, 5, 6, 0, 0},
		{0, 4, 7, 6, 5, 4, 0, 0}
	};
	static const int kTVScreenPts[4][3] = {
		{30, 50, 10}, {30, -50, 10}, {30, 50, 110}, {30, -50, 110}
	};
	static const int kTVScreenSurf[1][8] = {{0, 4, 1, 0, 2, 3, 0, 0}};

	static const int kDrawerPts[8][3] = {
		{-80, 70, 0}, {0, 70, 0}, {0, -70, 0}, {-80, -70, 0},
		{-80, 70, 100}, {0, 70, 100}, {0, -70, 100}, {-80, -70, 100}
	};
	static const int kDrawerSurf[5][8] = {
		{0, 4, 0, 3, 7, 4, 0, 0}, {0, 4, 3, 2, 6, 7, 0, 0},
		{0, 4, 1, 0, 4, 5, 0, 0}, {0, 4, 2, 1, 5, 6, 0, 0},
		{0, 4, 7, 6, 5, 4, 0, 0}
	};
	static const int kMirrorPts[4][3] = {
		{-80, 65, 100}, {-80, -65, 100}, {-80, 65, 210}, {-80, -65, 210}
	};
	static const int kMirrorSurf[1][8] = {{0, 4, 1, 0, 2, 3, 0, 0}};

	static const PrismPartDef kScreenPart = {8, kScreenPts, 4, kScreenSurf};
	static const PrismPartDef kTableParts[2] = {
		{4, kTableTopPts, 1, kTableTopSurf},
		{8, kTableBasePts, 4, kTableBaseSurf}
	};
	static const PrismPartDef kBedParts[3] = {
		{4, kBedPostPts, 1, kBedPostSurf},
		{8, kBedBlanketPts, 4, kBlanketSurf},
		{8, kBedSheetPts, 3, kSheetSurf}
	};
	static const PrismPartDef kBBedParts[3] = {
		{4, kBBedPostPts, 1, kBedPostSurf},
		{8, kBBedBlanketPts, 4, kBlanketSurf},
		{8, kBBedSheetPts, 3, kSheetSurf}
	};
	static const PrismPartDef kDeskParts[10] = {
		{4, kDeskTopPts, 1, kDeskTopSurf},
		{8, kDeskLeftPts, 4, kDeskCabSurf},
		{8, kDeskRightPts, 4, kDeskCabSurf},
		{4, kSeatPts, 1, kSeatSurf},
		{4, kArmLeftPts, 2, kArmSurf},
		{4, kArmRightPts, 2, kArmSurf},
		{4, kBackPts, 2, kBackSurf},
		{8, kComputerPts, 5, kComputerSurf},
		{8, kMonitorPts, 5, kComputerSurf},
		{4, kDeskScreenPts, 1, kDeskScreenSurf}
	};
	static const PrismPartDef kCChairParts[5] = {
		{4, kCSeatPts, 1, kCSeatSurf},
		{4, kCArmLeftPts, 1, kCArmLeftSurf},
		{4, kCArmRightPts, 1, kCArmRightSurf},
		{4, kCBackPts, 2, kCBackSurf},
		{8, kCBasePts, 4, kCBaseSurf}
	};
	static const PrismPartDef kConsolePart = {8, kConsolePts, 5, kConsoleSurf};
	static const PrismPartDef kCouchParts[4] = {
		{8, kACouchPts, 5, kCouchSurf},
		{8, kBCouchPts, 5, kCouchSurf},
		{8, kCCouchPts, 5, kCouchSurf},
		{8, kDCouchPts, 5, kCouchSurf}
	};
	static const PrismPartDef kChairParts[4] = {
		{8, kAChairPts, 5, kCouchSurf},
		{8, kBChairPts, 5, kCouchSurf},
		{8, kCChairPts2, 5, kCouchSurf},
		{8, kDChairPts, 5, kCouchSurf}
	};
	static const PrismPartDef kTVParts[2] = {
		{8, kTVBodyPts, 5, kTVBodySurf},
		{4, kTVScreenPts, 1, kTVScreenSurf}
	};
	static const PrismPartDef kDrawerParts[2] = {
		{8, kDrawerPts, 5, kDrawerSurf},
		{4, kMirrorPts, 1, kMirrorSurf}
	};

	/* CWALL (object 49) - ported from DOS INITOBJ.C: simple corner/quarter-wall prism */
	static const int kCWallPts[8][3] = {
		{-128, 128, -160}, {0, 112, -160}, {112, 0, -160}, {128, -128, -160},
		{-128, 128, 160},  {0, 112, 160},  {112, 0, 160},  {128, -128, 160}
	};
	static const int kCWallSurf[3][8] = {
		{0, 4, 1, 0, 4, 5, 0, 0}, {0, 4, 2, 1, 5, 6, 0, 0}, {0, 4, 3, 2, 6, 7, 0, 0}
	};
	static const PrismPartDef kCWallParts[1] = {{8, kCWallPts, 3, kCWallSurf}};

	/* PLANT (new) - simple pot + stem + two leaf plates (approximates DOS plant) */
	static const int kPlantPotPts[8][3] = {
		{-20, 20, 0}, {20, 20, 0}, {20, -20, 0}, {-20, -20, 0},
		{-20, 20, 40}, {20, 20, 40}, {20, -20, 40}, {-20, -20, 40}
	};
	static const int kPlantPotSurf[5][8] = {
		{0, 4, 0, 3, 7, 4, 0, 0}, {0, 4, 3, 2, 6, 7, 0, 0},
		{0, 4, 1, 0, 4, 5, 0, 0}, {0, 4, 2, 1, 5, 6, 0, 0}, {0, 4, 7, 6, 5, 4, 0, 0}
	};

	static const int kPlantStemPts[8][3] = {
		{-5, 5, 40}, {5, 5, 40}, {5, -5, 40}, {-5, -5, 40},
		{-5, 5, 120}, {5, 5, 120}, {5, -5, 120}, {-5, -5, 120}
	};
	static const int kPlantStemSurf[4][8] = {
		{0, 4, 0, 3, 7, 4, 0, 0}, {0, 4, 3, 2, 6, 7, 0, 0},
		{0, 4, 1, 0, 4, 5, 0, 0}, {0, 4, 2, 1, 5, 6, 0, 0}
	};

	static const int kPlantLeaf1Pts[4][3] = {
		{-40, 0, 120}, {40, 0, 120}, {40, 0, 140}, {-40, 0, 140}
	};
	static const int kPlantLeaf2Pts[4][3] = {
		{0, 40, 120}, {0, -40, 120}, {0, 40, 140}, {0, -40, 140}
	};
	static const int kPlantLeafSurf[1][8] = {{0, 4, 0, 1, 2, 3, 0, 0}};

	static const PrismPartDef kPlantParts[4] = {
		{8, kPlantPotPts, 5, kPlantPotSurf},
		{8, kPlantStemPts, 4, kPlantStemSurf},
		{4, kPlantLeaf1Pts, 1, kPlantLeafSurf},
		{4, kPlantLeaf2Pts, 1, kPlantLeafSurf}
	};

	const int clipLeft = MAX<int>((int)obj.clip.left, (int)_screenR.left);
	const int clipTop = MAX<int>((int)obj.clip.top, (int)_screenR.top);
	const int clipRight = MIN<int>((int)obj.clip.right, (int)_screenR.right);
	const int clipBottom = MIN<int>((int)obj.clip.bottom, (int)_screenR.bottom);
	if (clipLeft >= clipRight || clipTop >= clipBottom)
		return false;
	Common::Rect drawClip(clipLeft, clipTop, clipRight, clipBottom);

	auto tint = [](uint32 base, int delta) -> uint32 {
		return (uint32)CLIP<int>((int)base + delta, 0, 255);
	};

	ProjectedPrismPart p[10];
	switch (obj.type) {
	case kObjConsole:
		if (!projectPrismPart(obj, kConsolePart, false, p[0]) || !p[0].visible)
			return false;
		drawProjectedPrism(p[0], kConsolePart, 0, tint(baseColor, 0), drawClip);
		return true;
	case kObjCChair:
		for (int i = 0; i < 5; i++)
			projectPrismPart(obj, kCChairParts[i], false, p[i]);
		if (p[4].visible)
			drawProjectedPrism(p[4], kCChairParts[4], 0, tint(baseColor, -10), drawClip);
		if (!p[0].visible)
			return p[4].visible;
		drawProjectedPrism(p[0], kCChairParts[0], 0, tint(baseColor, 0), drawClip);
		if (p[3].vsurface[0]) {
			drawProjectedPrism(p[3], kCChairParts[3], 1, tint(baseColor, 0), drawClip);
			if (p[1].vsurface[0]) {
				drawProjectedPrism(p[1], kCChairParts[1], 1, tint(baseColor, 0), drawClip);
				drawProjectedPrism(p[2], kCChairParts[2], 1, tint(baseColor, 0), drawClip);
			} else {
				drawProjectedPrism(p[2], kCChairParts[2], 1, tint(baseColor, 0), drawClip);
				drawProjectedPrism(p[1], kCChairParts[1], 1, tint(baseColor, 0), drawClip);
			}
		} else {
			if (p[1].vsurface[0]) {
				drawProjectedPrism(p[1], kCChairParts[1], 1, tint(baseColor, 0), drawClip);
				drawProjectedPrism(p[2], kCChairParts[2], 1, tint(baseColor, 0), drawClip);
			} else {
				drawProjectedPrism(p[2], kCChairParts[2], 1, tint(baseColor, 0), drawClip);
				drawProjectedPrism(p[1], kCChairParts[1], 1, tint(baseColor, 0), drawClip);
			}
			drawProjectedPrism(p[3], kCChairParts[3], 1, tint(baseColor, 0), drawClip);
		}
		return true;
	case kObjPlant: {
		for (int i = 0; i < 4; i++)
			projectPrismPart(obj, kPlantParts[i], false, p[i]);
		// Pot must be visible for object to appear
		if (!p[0].visible)
			return false;
		// Draw pot, stem and leaves (pot darker, foliage lighter)
		drawProjectedPrism(p[0], kPlantParts[0], 0, tint(baseColor, -30), drawClip);
		if (p[1].visible)
			drawProjectedPrism(p[1], kPlantParts[1], 0, tint(baseColor, 10), drawClip);
		if (p[2].vsurface[0])
			drawProjectedPrism(p[2], kPlantParts[2], 0, tint(baseColor, 30), drawClip);
		if (p[3].vsurface[0])
			drawProjectedPrism(p[3], kPlantParts[3], 0, tint(baseColor, 30), drawClip);
		return true;
	}
	case kObjCouch:
	case kObjChair: {
		const PrismPartDef *parts = (obj.type == kObjCouch) ? kCouchParts : kChairParts;
		for (int i = 0; i < 4; i++)
			projectPrismPart(obj, parts[i], false, p[i]);
		if (!p[0].visible)
			return false;
		if (p[2].vsurface[1] && p[3].vsurface[2]) {
			drawProjectedPrism(p[2], parts[2], 0, tint(baseColor, -5), drawClip);
			drawProjectedPrism(p[3], parts[3], 0, tint(baseColor, -5), drawClip);
			if (p[0].vsurface[3]) {
				drawProjectedPrism(p[1], parts[1], 0, tint(baseColor, -5), drawClip);
				drawProjectedPrism(p[0], parts[0], 0, tint(baseColor, 5), drawClip);
			} else {
				drawProjectedPrism(p[0], parts[0], 0, tint(baseColor, 5), drawClip);
				drawProjectedPrism(p[1], parts[1], 0, tint(baseColor, -5), drawClip);
			}
		} else if (p[3].vsurface[1]) {
			drawProjectedPrism(p[2], parts[2], 0, tint(baseColor, -5), drawClip);
			if (p[0].vsurface[3]) {
				drawProjectedPrism(p[1], parts[1], 0, tint(baseColor, -5), drawClip);
				drawProjectedPrism(p[0], parts[0], 0, tint(baseColor, 5), drawClip);
			} else {
				drawProjectedPrism(p[0], parts[0], 0, tint(baseColor, 5), drawClip);
				drawProjectedPrism(p[1], parts[1], 0, tint(baseColor, -5), drawClip);
			}
			drawProjectedPrism(p[3], parts[3], 0, tint(baseColor, -5), drawClip);
		} else {
			drawProjectedPrism(p[3], parts[3], 0, tint(baseColor, -5), drawClip);
			if (p[0].vsurface[3]) {
				drawProjectedPrism(p[1], parts[1], 0, tint(baseColor, -5), drawClip);
				drawProjectedPrism(p[0], parts[0], 0, tint(baseColor, 5), drawClip);
			} else {
				drawProjectedPrism(p[0], parts[0], 0, tint(baseColor, 5), drawClip);
				drawProjectedPrism(p[1], parts[1], 0, tint(baseColor, -5), drawClip);
			}
			drawProjectedPrism(p[2], parts[2], 0, tint(baseColor, -5), drawClip);
		}
		return true;
	}
	case kObjTV:
		projectPrismPart(obj, kTVParts[0], false, p[0]);
		projectPrismPart(obj, kTVParts[1], false, p[1]);
		if (!p[0].visible)
			return false;
		drawProjectedPrism(p[0], kTVParts[0], 0, tint(baseColor, 0), drawClip);
		if (p[1].vsurface[0])
			drawProjectedPrism(p[1], kTVParts[1], 0, tint(baseColor, 35), drawClip);
		return true;
	case kObjDrawer:
		projectPrismPart(obj, kDrawerParts[0], false, p[0]);
		projectPrismPart(obj, kDrawerParts[1], false, p[1]);
		if (!p[0].visible)
			return false;
		drawProjectedPrism(p[0], kDrawerParts[0], 0, tint(baseColor, 0), drawClip);
		drawProjectedPrism(p[1], kDrawerParts[1], 1, tint(baseColor, 30), drawClip);
		return true;
case kObjFWall:
		// Simple flat wall part (fallback handled by prism below)
		if (!projectPrismPart(obj, kCWallParts[0], false, p[0]) || !p[0].visible)
			return false;
		drawProjectedPrism(p[0], kCWallParts[0], 0, tint(baseColor, -10), drawClip);
		return true;
case kObjCWall:
		// Corner wall (CWALL) — mirror of DOS CWallparts
		if (!projectPrismPart(obj, kCWallParts[0], false, p[0]) || !p[0].visible)
			return false;
		drawProjectedPrism(p[0], kCWallParts[0], 0, tint(baseColor, -5), drawClip);
		return true;
	case kObjScreen:
		if (!projectPrismPart(obj, kScreenPart, false, p[0]) || !p[0].visible)
			return false;
		drawProjectedPrism(p[0], kScreenPart, 0, tint(baseColor, 0), drawClip);
		return true;
	case kObjTable:
		projectPrismPart(obj, kTableParts[0], false, p[0]);
		projectPrismPart(obj, kTableParts[1], false, p[1]);
		if (!p[1].visible)
			return false;
		drawProjectedPrism(p[1], kTableParts[1], 0, tint(baseColor, -10), drawClip);
		drawProjectedPrism(p[0], kTableParts[0], 0, tint(baseColor, 20), drawClip);
		return true;
	case kObjBed:
	case kObjBBed: {
		const PrismPartDef *parts = (obj.type == kObjBBed) ? kBBedParts : kBedParts;
		projectPrismPart(obj, parts[0], false, p[0]);
		projectPrismPart(obj, parts[1], false, p[1]);
		projectPrismPart(obj, parts[2], false, p[2]);
		if (!p[1].visible)
			return false;
		if (p[0].vsurface[0]) {
			drawProjectedPrism(p[0], parts[0], 1, tint(baseColor, 15), drawClip);
			drawProjectedPrism(p[2], parts[2], 0, tint(baseColor, 5), drawClip);
			drawProjectedPrism(p[1], parts[1], 0, tint(baseColor, -10), drawClip);
		} else {
			drawProjectedPrism(p[1], parts[1], 0, tint(baseColor, -10), drawClip);
			drawProjectedPrism(p[2], parts[2], 0, tint(baseColor, 5), drawClip);
			drawProjectedPrism(p[0], parts[0], 1, tint(baseColor, 15), drawClip);
		}
		return true;
	}
	case kObjDesk:
		for (int i = 0; i < 10; i++)
			projectPrismPart(obj, kDeskParts[i], false, p[i]);
		if (!p[0].visible)
			return false;
		if (p[6].vsurface[1]) {
			if (p[1].vsurface[3] || p[2].vsurface[3]) {
				drawProjectedPrism(p[1], kDeskParts[1], 0, tint(baseColor, -15), drawClip);
				drawProjectedPrism(p[2], kDeskParts[2], 0, tint(baseColor, -15), drawClip);
			} else {
				drawProjectedPrism(p[2], kDeskParts[2], 0, tint(baseColor, -15), drawClip);
				drawProjectedPrism(p[1], kDeskParts[1], 0, tint(baseColor, -15), drawClip);
			}
			drawProjectedPrism(p[0], kDeskParts[0], 0, tint(baseColor, 15), drawClip);
			drawProjectedPrism(p[7], kDeskParts[7], 0, tint(baseColor, 25), drawClip);
			drawProjectedPrism(p[8], kDeskParts[8], 0, tint(baseColor, 25), drawClip);
			if (p[9].vsurface[0])
				drawProjectedPrism(p[9], kDeskParts[9], 0, tint(baseColor, 40), drawClip);
			if (p[4].vsurface[0]) {
				drawProjectedPrism(p[4], kDeskParts[4], 0, tint(baseColor, -5), drawClip);
				drawProjectedPrism(p[3], kDeskParts[3], 0, tint(baseColor, -5), drawClip);
				drawProjectedPrism(p[6], kDeskParts[6], 0, tint(baseColor, -5), drawClip);
				drawProjectedPrism(p[5], kDeskParts[5], 0, tint(baseColor, -5), drawClip);
			} else {
				drawProjectedPrism(p[5], kDeskParts[5], 0, tint(baseColor, -5), drawClip);
				drawProjectedPrism(p[3], kDeskParts[3], 0, tint(baseColor, -5), drawClip);
				drawProjectedPrism(p[6], kDeskParts[6], 0, tint(baseColor, -5), drawClip);
				drawProjectedPrism(p[4], kDeskParts[4], 0, tint(baseColor, -5), drawClip);
			}
		} else {
			if (p[4].vsurface[0]) {
				drawProjectedPrism(p[4], kDeskParts[4], 0, tint(baseColor, -5), drawClip);
				drawProjectedPrism(p[3], kDeskParts[3], 0, tint(baseColor, -5), drawClip);
				drawProjectedPrism(p[6], kDeskParts[6], 0, tint(baseColor, -5), drawClip);
				drawProjectedPrism(p[5], kDeskParts[5], 0, tint(baseColor, -5), drawClip);
			} else {
				drawProjectedPrism(p[5], kDeskParts[5], 0, tint(baseColor, -5), drawClip);
				drawProjectedPrism(p[3], kDeskParts[3], 0, tint(baseColor, -5), drawClip);
				drawProjectedPrism(p[6], kDeskParts[6], 0, tint(baseColor, -5), drawClip);
				drawProjectedPrism(p[4], kDeskParts[4], 0, tint(baseColor, -5), drawClip);
			}
			if (p[1].vsurface[3] || p[2].vsurface[3]) {
				drawProjectedPrism(p[1], kDeskParts[1], 0, tint(baseColor, -15), drawClip);
				drawProjectedPrism(p[2], kDeskParts[2], 0, tint(baseColor, -15), drawClip);
			} else {
				drawProjectedPrism(p[2], kDeskParts[2], 0, tint(baseColor, -15), drawClip);
				drawProjectedPrism(p[1], kDeskParts[1], 0, tint(baseColor, -15), drawClip);
			}
			drawProjectedPrism(p[0], kDeskParts[0], 0, tint(baseColor, 15), drawClip);
			drawProjectedPrism(p[7], kDeskParts[7], 0, tint(baseColor, 25), drawClip);
			drawProjectedPrism(p[8], kDeskParts[8], 0, tint(baseColor, 25), drawClip);
			if (p[9].vsurface[0])
				drawProjectedPrism(p[9], kDeskParts[9], 0, tint(baseColor, 40), drawClip);
		}
		return true;
	default:
		return false;
	}
}

void ColonyEngine::drawStaticObjectFallback(const Thing &obj, uint32 color, int depth, int sx) {
	int scale = _rtable[depth];
	int baseY = _height - (_centerY - scale);
	int h = CLIP<int>(scale, 4, 96);
	int w = CLIP<int>(h >> 1, 3, 64);
	Common::Rect body(sx - w, baseY - h, sx + w, baseY);
	const int bodyLeft = (int)body.left;
	const int bodyTop = (int)body.top;
	const int bodyRight = (int)body.right;
	const int bodyBottom = (int)body.bottom;
	const int clipLeft = MAX(bodyLeft, MAX((int)obj.clip.left, (int)_screenR.left));
	const int clipTop = MAX(bodyTop, MAX((int)obj.clip.top, (int)_screenR.top));
	const int clipRight = MIN(bodyRight, MIN((int)obj.clip.right, (int)_screenR.right));
	const int clipBottom = MIN(bodyBottom, MIN((int)obj.clip.bottom, (int)_screenR.bottom));
	if (clipLeft >= clipRight || clipTop >= clipBottom)
		return;
	Common::Rect clipped(clipLeft, clipTop, clipRight, clipBottom);
	_gfx->drawRect(clipped, color);
	_gfx->drawLine(clipped.left, clipped.bottom - 1, clipped.right - 1, clipped.bottom - 1, color);
}

void ColonyEngine::drawStaticObjects() {
	struct DrawCmd {
		int depth;
		int index;
		int screenX;
	};

	Common::Array<DrawCmd> drawList;
	drawList.reserve(_objects.size());

	for (uint i = 0; i < _objects.size(); i++) {
		const Thing &obj = _objects[i];
		if (!obj.alive || !obj.visible)
			continue;
		int sx, depth;
		if (!projectWorld(obj.where.xloc, obj.where.yloc, sx, depth))
			continue;
		if (depth > 11000)
			continue;
		DrawCmd cmd;
		cmd.depth = depth;
		cmd.index = (int)i;
		cmd.screenX = sx;
		drawList.push_back(cmd);
	}

	Common::sort(drawList.begin(), drawList.end(), [](const DrawCmd &a, const DrawCmd &b) {
		return a.depth > b.depth; // far to near
	});

	for (uint i = 0; i < drawList.size(); i++) {
		const DrawCmd &d = drawList[i];
		const Thing &obj = _objects[d.index];
		const uint32 color = objectColor(obj.type);
		if (!drawStaticObjectPrisms(obj, color))
			drawStaticObjectFallback(obj, color, d.depth, d.screenX);
	}
}

void ColonyEngine::setRobot(int l, int r, int num) {
	if (num <= 0 || num > (int)_objects.size())
		return;
	if (l < _screenR.left)
		l = _screenR.left;
	if (r > _screenR.right)
		r = _screenR.right;
	if (l >= r)
		return;
	const int clipLeft = l + 1;
	const int clipRight = r - 2;
	if (clipLeft >= clipRight)
		return;

	Thing &obj = _objects[num - 1];
	if (!obj.alive)
		return;
	obj.visible = 1;
	obj.clip.left = clipLeft;
	obj.clip.right = clipRight;
	obj.clip.top = _clip.top;
	obj.clip.bottom = _clip.bottom;
}

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

Common::Error ColonyEngine::run() {
	Graphics::PixelFormat format8bpp = Graphics::PixelFormat::createFormatCLUT8();
	initGraphics(_width, _height, &format8bpp);

	_width = _system->getWidth();
	_height = _system->getHeight();
	updateViewportLayout();
	const Graphics::PixelFormat format = _system->getScreenFormat();
	debug("Screen format: %d bytesPerPixel. Actual size: %dx%d", format.bytesPerPixel, _width, _height);

	// Setup a grayscale palette
	byte pal[256 * 3];
	for (int i = 0; i < 256; i++) {
		pal[i * 3 + 0] = i;
		pal[i * 3 + 1] = i;
		pal[i * 3 + 2] = i;
	}
	_system->getPaletteManager()->setPalette(pal, 0, 256);

	_gfx = new Gfx(_system, _width, _height);
	
	loadMap(1); // Try to load the first map
	_system->lockMouse(true);
	_system->warpMouse(_centerX, _centerY);

	while (!shouldQuit()) {
		Common::Event event;
		while (_system->getEventManager()->pollEvent(event)) {
				if (event.type == Common::EVENT_KEYDOWN) {
					debug("Key down: %d", event.kbd.keycode);
					const bool allowInteraction = (event.kbd.flags & Common::KBD_CTRL) == 0;
					switch (event.kbd.keycode) {
					case Common::KEYCODE_UP:
					{
						int xnew = _me.xloc + (_cost[_me.look] >> 2);
						int ynew = _me.yloc + (_sint[_me.look] >> 2);
						cCommand(xnew, ynew, allowInteraction);
						break;
					}
					case Common::KEYCODE_DOWN:
					{
						int xnew = _me.xloc - (_cost[_me.look] >> 2);
						int ynew = _me.yloc - (_sint[_me.look] >> 2);
						cCommand(xnew, ynew, allowInteraction);
						break;
					}
				case Common::KEYCODE_LEFT:
					_me.look = (uint8)((int)_me.look + 8);
					_change = true;
					break;
					case Common::KEYCODE_RIGHT:
						_me.look = (uint8)((int)_me.look - 8);
						_change = true;
						break;
					case Common::KEYCODE_F7:
						_showDashBoard = !_showDashBoard;
						updateViewportLayout();
						_system->warpMouse(_centerX, _centerY);
						_change = true;
						break;
					default:
						break;
					}
				debug("Me: x=%d y=%d look=%d", _me.xloc, _me.yloc, _me.look);
			} else if (event.type == Common::EVENT_MOUSEMOVE) {
				if (event.relMouse.x != 0) {
					// Subtract because increasing look turns left, but mouse right is positive rel X
					// Reduced sensitivity by half
					_me.look = (uint8)((int)_me.look - (event.relMouse.x / 2));
					_change = true;
				}
			}
		}
		_system->warpMouse(_centerX, _centerY);

		_gfx->clear(_gfx->black());
		for (uint i = 0; i < _objects.size(); i++)
			_objects[i].visible = 0;
		
		corridor();
		drawStaticObjects();
		drawDashboardStep1();
		drawCrosshair();
		
		_gfx->copyToScreen();
		_system->delayMillis(10);
	}

	delete _gfx;
	return Common::kNoError;
}

} // End of namespace Colony
