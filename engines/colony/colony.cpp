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
#include "common/config-manager.h"
#include "common/hashmap.h"
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
#include "graphics/fontman.h"
#include "graphics/font.h"
#include "graphics/fonts/dosfont.h"
#include "graphics/fonts/macfont.h"
#include "graphics/cursorman.h"
#include "image/pict.h"
#include <math.h>

namespace Colony {

// Mac color indices from colordef.h enum (cColor[] table in Color256).
enum {
	mc_dwall = 6, mc_lwall = 7,
	mc_char0 = 8,    // char0..char6 = 8..14
	mc_bulkhead = 15, mc_door = 16,
	mc_desk = 58, mc_desktop = 59, mc_screen = 62,
	mc_proj = 72, mc_console = 79, mc_powerbase = 81,
	mc_box1 = 84, mc_forklift = 86, mc_flglass = 87,
	mc_cryo = 90, mc_ccore = 111,
	mc_teleport = 93, mc_teledoor = 94,
	mc_vanity = 96, mc_mirror = 103,
	mc_airlock = 25, mc_elevator = 23
};

// Mac Toolbox BackColor() constants.
enum {
	kMacWhite = 30, kMacBlack = 33,
	kMacYellow = 69, kMacMagenta = 137,
	kMacRed = 205, kMacCyan = 273,
	kMacGreen = 341, kMacBlue = 409
};

// BMColor arrays from ganimate.c — per-animation color maps.
// Index 0 = background top, 1 = background image, 2+ = per-sprite fill.
// Positive = cColor[] index, negative = -MacSystemColor, 0 = level-based.
static const int16 kBMC_Desk[] = {
	0, mc_desktop,
	-kMacRed, -kMacCyan, -kMacCyan, -kMacCyan, -kMacCyan,
	-kMacWhite, -kMacWhite, -kMacMagenta, -kMacYellow, mc_desk,
	mc_desk, mc_desk, mc_desk, mc_desk, mc_desk,
	-kMacWhite, mc_screen, -kMacMagenta, -kMacCyan, -kMacCyan,
	-kMacBlue, -kMacWhite, -kMacRed, -kMacWhite, -kMacYellow
};
static const int16 kBMC_Vanity[] = {
	0, mc_vanity,
	mc_mirror, -kMacRed, -kMacCyan, -kMacWhite, -kMacYellow,
	-kMacGreen, -kMacBlue, -kMacRed, -kMacMagenta, -kMacRed,
	mc_vanity, -kMacWhite, -kMacYellow, mc_mirror
};
static const int16 kBMC_Reactor[] = {
	0, mc_console,
	-kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow,
	-kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow,
	-kMacYellow, -kMacYellow, -kMacRed, -kMacRed, -kMacRed,
	-kMacRed, -kMacRed, -kMacRed, -kMacRed, -kMacRed,
	-kMacRed, -kMacRed, -kMacRed, -kMacRed, -kMacCyan,
	-kMacMagenta, -kMacWhite
};
static const int16 kBMC_Security[] = {
	0, mc_console,
	-kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow,
	-kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow,
	-kMacYellow, -kMacYellow, -kMacRed, -kMacRed, -kMacRed,
	-kMacRed, -kMacRed, -kMacRed, -kMacRed, -kMacRed,
	-kMacRed, -kMacRed, -kMacRed, -kMacRed, -kMacWhite,
	-kMacRed, -kMacCyan, -kMacCyan, -kMacCyan, -kMacCyan
};
static const int16 kBMC_Teleport[] = {
	0, mc_teleport, 0, mc_teledoor
};
static const int16 kBMC_Creatures[] = {
	-kMacWhite, 0, -kMacWhite, -kMacCyan, mc_proj,
	-kMacBlue, -kMacMagenta, -kMacMagenta
};
static const int16 kBMC_Controls[] = {
	0, mc_console,
	-kMacRed, -kMacYellow, -kMacYellow, -kMacBlue, -kMacYellow, -kMacGreen, mc_screen
};
static const int16 kBMC_Lift[] = {
	0, mc_flglass,
	mc_teleport, mc_box1, mc_cryo, mc_ccore, 0,
	-kMacRed, -kMacRed, -kMacCyan, -kMacCyan
};
static const int16 kBMC_Powersuit[] = {
	0, mc_powerbase,
	-kMacMagenta, -kMacMagenta, -kMacYellow, -kMacYellow, mc_powerbase, -kMacWhite
};
static const int16 kBMC_Forklift[] = {
	0, mc_forklift, mc_forklift, mc_forklift
};
static const int16 kBMC_Door[] = {
	0, mc_bulkhead, 0, mc_door, -kMacYellow
};
static const int16 kBMC_Bulkhead[] = {
	0, mc_bulkhead, 0, mc_bulkhead, -kMacYellow
};
static const int16 kBMC_Airlock[] = {
	0, mc_bulkhead, mc_bulkhead, -kMacRed, mc_airlock
};
static const int16 kBMC_Elevator[] = {
	0, mc_bulkhead, 0, mc_elevator, mc_elevator, -kMacYellow
};
static const int16 kBMC_Elevator2[] = {
	0, mc_bulkhead, 0, -kMacMagenta, mc_elevator, mc_elevator,
	-kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow, -kMacYellow
};

struct AnimColorEntry {
	const char *name;
	const int16 *colors;
	int count;
};

static const AnimColorEntry kAnimColors[] = {
	{ "desk",        kBMC_Desk,       ARRAYSIZE(kBMC_Desk) },
	{ "vanity",      kBMC_Vanity,     ARRAYSIZE(kBMC_Vanity) },
	{ "reactor",     kBMC_Reactor,    ARRAYSIZE(kBMC_Reactor) },
	{ "security",    kBMC_Security,   ARRAYSIZE(kBMC_Security) },
	{ "teleporter",  kBMC_Teleport,   ARRAYSIZE(kBMC_Teleport) },
	{ "teleporter2", kBMC_Teleport,   ARRAYSIZE(kBMC_Teleport) },
	{ "slides",      kBMC_Creatures,  ARRAYSIZE(kBMC_Creatures) },
	{ "slideshow",   kBMC_Creatures,  ARRAYSIZE(kBMC_Creatures) },
	{ "teleshow",    kBMC_Creatures,  ARRAYSIZE(kBMC_Creatures) },
	{ "controls",    kBMC_Controls,   ARRAYSIZE(kBMC_Controls) },
	{ "lift",        kBMC_Lift,       ARRAYSIZE(kBMC_Lift) },
	{ "lifter",      kBMC_Lift,       ARRAYSIZE(kBMC_Lift) },
	{ "suit",        kBMC_Powersuit,  ARRAYSIZE(kBMC_Powersuit) },
	{ "spacesuit",   kBMC_Powersuit,  ARRAYSIZE(kBMC_Powersuit) },
	{ "forklift",    kBMC_Forklift,   ARRAYSIZE(kBMC_Forklift) },
	{ "door",        kBMC_Door,       ARRAYSIZE(kBMC_Door) },
	{ "bulkhead",    kBMC_Bulkhead,   ARRAYSIZE(kBMC_Bulkhead) },
	{ "airlock",     kBMC_Airlock,    ARRAYSIZE(kBMC_Airlock) },
	{ "elev",        kBMC_Elevator,   ARRAYSIZE(kBMC_Elevator) },
	{ "elevator",    kBMC_Elevator,   ARRAYSIZE(kBMC_Elevator) },
	{ "elevator2",   kBMC_Elevator2,  ARRAYSIZE(kBMC_Elevator2) },
	{ nullptr, nullptr, 0 }
};

// Convert Mac Toolbox BackColor constant to ARGB.
static uint32 macSysColorToARGB(int sysColor) {
	switch (sysColor) {
	case kMacWhite:   return 0xFFFFFFFF;
	case kMacBlack:   return 0xFF000000;
	case kMacYellow:  return 0xFFFFFF00;
	case kMacMagenta: return 0xFFFF00FF;
	case kMacRed:     return 0xFFFF0000;
	case kMacCyan:    return 0xFF00FFFF;
	case kMacGreen:   return 0xFF00FF00;
	case kMacBlue:    return 0xFF0000FF;
	default:          return 0xFFFFFFFF;
	}
}

static uint32 packMacColorBG(const uint16 rgb[3]) {
	return 0xFF000000 | ((uint32)(rgb[0] >> 8) << 16) |
	       ((uint32)(rgb[1] >> 8) << 8) | (uint32)(rgb[2] >> 8);
}

ColonyEngine::ColonyEngine(OSystem *syst, const ADGameDescription *gd) : Engine(syst), _gameDescription(gd), _randomSource("colony") {
	_level = 0;
	_robotNum = 0;
	_gfx = nullptr;
	_frameLimiter = nullptr;
	_width = 640;
	_height = 350;
	_centerX = _width / 2;
	_centerY = _height / 2;
	_mouseSensitivity = 1;
	_mouseLocked = false;
	_showDashBoard = true;
	_crosshair = true;
	_insight = false;
	_hasKeycard = false;
	_unlocked = false;
	_weapons = 0;
	_widescreen = ConfMan.getBool("widescreen_mod");

	// Render mode: EGA (DOS wireframe default) or Macintosh (filled polygons)
	if (!ConfMan.hasKey("render_mode") || ConfMan.get("render_mode").empty())
		_renderMode = Common::kRenderDefault;
	else
		_renderMode = Common::parseRenderMode(ConfMan.get("render_mode"));

	if (_renderMode == Common::kRenderDefault) {
		if (getPlatform() == Common::kPlatformMacintosh)
			_renderMode = Common::kRenderMacintosh;
		else
			_renderMode = Common::kRenderEGA;
	}
	
	_wireframe = (_renderMode != Common::kRenderMacintosh);
	_fullscreen = false;
	_speedShift = 2; // DOS default: speedshift=1, but 2 feels better with our frame rate
	_wm = nullptr;
	_macMenu = nullptr;
	_menuSurface = nullptr;
	_menuBarHeight = 0;
	
	memset(_wall, 0, sizeof(_wall));
	memset(_mapData, 0, sizeof(_mapData));
	memset(_robotArray, 0, sizeof(_robotArray));
	memset(_foodArray, 0, sizeof(_foodArray));

	// PATCH.C init
	memset(_levelData, 0, sizeof(_levelData));
	memset(_carryPatch, 0, sizeof(_carryPatch));
	_carryType = 0;
	_fl = 0;
	_patches.clear();

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
	_me.type = kMeNum;

	// Animation system init
	_backgroundMask = nullptr;
	_backgroundFG = nullptr;
	_backgroundActive = false;
	_divideBG = 0;
	_animationRunning = false;
	_animationResult = 0;
	_doorOpen = false;
	_elevatorFloor = 0;

	for (int i = 0; i < 4; i++) {
		_decode1[i] = _decode2[i] = _decode3[i] = 0;
	}
	for (int i = 0; i < 6; i++)
		_animDisplay[i] = 1;
	for (int i = 0; i < 2; i++)
		_coreState[i] = _coreHeight[i] = 0;
	for (int i = 0; i < 3; i++)
		_corePower[i] = 0;
	_coreIndex = 0;
	_hasMacColors = false;
	memset(_macColors, 0, sizeof(_macColors));
	_orbit = 0;
	_armor = 0;
	_gametest = false;
	_blackoutColor = 15; // Set to white (vINTWHITE) for better visibility in darkness

	_sound = new Sound(this);
	_resMan = new Common::MacResManager();
	_colorResMan = new Common::MacResManager();
	initTrig();
}


ColonyEngine::~ColonyEngine() {
	deleteAnimation();
	if (_pictPower) {
		_pictPower->free();
		delete _pictPower;
	}
	if (_pictPowerNoArmor) {
		_pictPowerNoArmor->free();
		delete _pictPowerNoArmor;
	}
	if (_pictCompass) {
		_pictCompass->free();
		delete _pictCompass;
	}
	delete _frameLimiter;
	delete _gfx;
	delete _sound;
	delete _colorResMan;
	delete _resMan;
	delete _menuSurface;
	delete _wm;
}


void ColonyEngine::loadMap(int mnum) {
	Common::Path mapPath(Common::String::format("MAP.%d", mnum));
	Common::SeekableReadStream *file = Common::MacResManager::openFileOrDataFork(mapPath);
	if (!file) {
		// Try Mac-style path
		mapPath = Common::Path(Common::String::format("CData/map.%d", mnum));
		file = Common::MacResManager::openFileOrDataFork(mapPath);
		if (!file) {
			warning("Could not open map file %s", mapPath.toString().c_str());
			return;
		}
	}

	file->readUint32BE(); // "DAVE" header
	int16 mapDefs[10];
	for (int i = 0; i < 10; i++) {
		mapDefs[i] = file->readSint16BE();
	}

	uint16 bLength = file->readUint16BE();
	uint8 *buffer = (uint8 *)malloc(bLength);
	if (!buffer) {
		delete file;
		error("Out of memory loading map");
	}
	file->read(buffer, bLength);
	delete file;

	memset(_mapData, 0, sizeof(_mapData));
	memset(_robotArray, 0, sizeof(_robotArray));
	memset(_foodArray, 0, sizeof(_foodArray));
	_objects.clear();

	// expand logic
	int c = 0;
	_robotNum = kMeNum + 1;
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
							obj.type = _mapData[i][j][4][1] + kBaseObject;
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
	_me.type = kMeNum;

	getWall();  // restore saved wall state changes (airlocks)
	doPatch();  // apply object relocations from patch table
	initRobots();  // spawn robot objects for this level

	if (_me.xindex >= 0 && _me.xindex < 32 && _me.yindex >= 0 && _me.yindex < 32)
		_robotArray[_me.xindex][_me.yindex] = kMeNum;
	debug("Successfully loaded map %d (objects: %d)", mnum, (int)_objects.size());
}

// PATCH.C: Create a new object in _objects and register in _robotArray.
// Mirrors DOS CreateObject() — sets basic Thing fields for static objects.
void ColonyEngine::createObject(int type, int xloc, int yloc, uint8 ang) {
	Thing obj;
	memset(&obj, 0, sizeof(obj));
	while (ang > 255)
		ang -= 256;
	obj.alive = 1;
	obj.visible = 0;
	obj.type = type;
	obj.where.xloc = xloc;
	obj.where.yloc = yloc;
	obj.where.xindex = xloc >> 8;
	obj.where.yindex = yloc >> 8;
	obj.where.delta = 4;
	obj.where.ang = ang;
	obj.where.look = ang;

	// Try to reuse a dead slot (starting after kMeNum)
	int slot = -1;
	for (int j = kMeNum; j < (int)_objects.size(); j++) {
		if (!_objects[j].alive) {
			slot = j;
			break;
		}
	}
	if (slot >= 0) {
		_objects[slot] = obj;
	} else {
		_objects.push_back(obj);
		slot = (int)_objects.size() - 1;
	}
	int objNum = slot + 1; // 1-based for _robotArray
	if (obj.where.xindex >= 0 && obj.where.xindex < 32 &&
	    obj.where.yindex >= 0 && obj.where.yindex < 32)
		_robotArray[obj.where.xindex][obj.where.yindex] = (uint8)objNum;
	if (slot + 1 > _robotNum)
		_robotNum = slot + 1;
}

// PATCH.C: DoPatch() — remove originals and install relocated objects.
void ColonyEngine::doPatch() {
	// Pass 1: remove objects that were moved away from this level
	for (uint i = 0; i < _patches.size(); i++) {
		if (_level == _patches[i].from.level) {
			int robot = _robotArray[_patches[i].from.xindex][_patches[i].from.yindex];
			if (robot > 0 && robot <= (int)_objects.size()) {
				_robotArray[_objects[robot - 1].where.xindex][_objects[robot - 1].where.yindex] = 0;
				_objects[robot - 1].alive = 0;
			}
		}
	}
	// Pass 2: install objects that were moved to this level
	for (uint i = 0; i < _patches.size(); i++) {
		if (_level == _patches[i].to.level) {
			createObject(
				(int)_patches[i].type,
				(int)_patches[i].to.xloc,
				(int)_patches[i].to.yloc,
				_patches[i].to.ang);
		}
	}
}

// PATCH.C: savewall() — save 5 bytes of map feature data for persistence across level loads.
void ColonyEngine::saveWall(int x, int y, int direction) {
	if (_level < 1 || _level > 8)
		return;
	LevelData &ld = _levelData[_level - 1];

	// Search for existing entry at this location
	for (int i = 0; i < ld.size; i++) {
		if (ld.location[i][0] == x && ld.location[i][1] == y && ld.location[i][2] == direction) {
			for (int j = 0; j < 5; j++)
				ld.data[i][j] = _mapData[x][y][direction][j];
			return;
		}
	}
	// Add new entry (max 10)
	if (ld.size >= 10) {
		warning("saveWall: too many wall changes for level %d", _level);
		return;
	}
	int i = ld.size;
	for (int j = 0; j < 5; j++)
		ld.data[i][j] = _mapData[x][y][direction][j];
	ld.location[i][0] = x;
	ld.location[i][1] = y;
	ld.location[i][2] = direction;
	ld.size++;
}

// PATCH.C: getwall() — restore saved wall bytes into _mapData after level load.
void ColonyEngine::getWall() {
	if (_level < 1 || _level > 8)
		return;
	const LevelData &ld = _levelData[_level - 1];
	for (int i = 0; i < ld.size; i++) {
		int x = ld.location[i][0];
		int y = ld.location[i][1];
		int dir = ld.location[i][2];
		if (x < 31 && y < 31 && dir < 5) {
			for (int j = 0; j < 5; j++)
				_mapData[x][y][dir][j] = ld.data[i][j];
		}
	}
}

// PATCH.C: newpatch() — create or update a patch entry.
void ColonyEngine::newPatch(int type, const PassPatch &from, const PassPatch &to, const uint8 *mapdata) {
	// Search for existing patch where 'from' matches an existing 'to'
	for (uint i = 0; i < _patches.size(); i++) {
		if (from.level == _patches[i].to.level &&
		    from.xindex == _patches[i].to.xindex &&
		    from.yindex == _patches[i].to.yindex) {
			_patches[i].to.level = to.level;
			_patches[i].to.xindex = to.xindex;
			_patches[i].to.yindex = to.yindex;
			_patches[i].to.xloc = to.xloc;
			_patches[i].to.yloc = to.yloc;
			_patches[i].to.ang = to.ang;
			return;
		}
	}
	// Create new patch entry (max 100)
	if (_patches.size() >= 100)
		return;
	PatchEntry pe;
	pe.type = type;
	pe.from.level = from.level;
	pe.from.xindex = from.xindex;
	pe.from.yindex = from.yindex;
	pe.to.level = to.level;
	pe.to.xindex = to.xindex;
	pe.to.yindex = to.yindex;
	pe.to.xloc = to.xloc;
	pe.to.yloc = to.yloc;
	pe.to.ang = to.ang;
	if (mapdata) {
		for (int j = 0; j < 5; j++)
			pe.mapdata[j] = mapdata[j];
	} else {
		memset(pe.mapdata, 0, sizeof(pe.mapdata));
	}
	_patches.push_back(pe);
}

// PATCH.C: patchmapto() — find patch entry by destination, fill mapdata.
bool ColonyEngine::patchMapTo(const PassPatch &to, uint8 *mapdata) {
	for (uint i = 0; i < _patches.size(); i++) {
		if (to.level == _patches[i].to.level &&
		    to.xindex == _patches[i].to.xindex &&
		    to.yindex == _patches[i].to.yindex) {
			for (int j = 0; j < 5; j++)
				mapdata[j] = _patches[i].mapdata[j];
			return true;
		}
	}
	return false;
}

// PATCH.C: patchmapfrom() — find patch entry by source, fill destination into mapdata.
bool ColonyEngine::patchMapFrom(const PassPatch &from, uint8 *mapdata) {
	for (uint i = 0; i < _patches.size(); i++) {
		if (from.level == _patches[i].from.level &&
		    from.xindex == _patches[i].from.xindex &&
		    from.yindex == _patches[i].from.yindex) {
			mapdata[2] = _patches[i].to.level;
			mapdata[3] = _patches[i].to.xindex;
			mapdata[4] = _patches[i].to.yindex;
			return true;
		}
	}
	return false;
}

// DOS InitObject() — spawn robots for the current level.
// Level 1 = no robots; Level 2 = 25; Level 3-4 = 30; Level 5-7 = 35.
// Robot #1 = QUEEN, #2 = SNOOP, rest = random type weighted by level.
void ColonyEngine::initRobots() {
	if (_level == 1)
		return;  // Level 1 has no robots

	int maxrob;
	switch (_level) {
	case 2:  maxrob = 25; break;
	case 3:
	case 4:  maxrob = 30; break;
	default: maxrob = 35; break;
	}

	int lvl = _level - 1;
	if (lvl > 5)
		lvl = 5;

	for (int i = 1; i <= maxrob; i++) {
		uint8 ang = _randomSource.getRandomNumber(255);
		int xloc, yloc;

		// Find unoccupied cell (avoiding borders)
		do {
			if (_level == 7 && i == 1) {
				// Queen on level 7 has fixed position
				xloc = 27;
				yloc = 10;
			} else {
				xloc = 2 + _randomSource.getRandomNumber(26);  // 2..28
				yloc = 2 + _randomSource.getRandomNumber(26);  // 2..28
			}
		} while (_robotArray[xloc][yloc] != 0);

		// Convert grid coords to world coords (center of cell)
		int wxloc = (xloc << 8) + 128;
		int wyloc = (yloc << 8) + 128;

		int type;
		if (i == 1)
			type = kRobQueen;
		else if (i == 2)
			type = kRobSnoop;
		else {
			// Random type weighted by level difficulty
			int rnd = _randomSource.getRandomNumber(lvl);
			if (rnd > 5)
				rnd = 5;
			switch (rnd) {
			case 0: type = kRobCube; break;
			case 1: type = kRobPyramid; break;
			case 2: type = kRobUPyramid; break;
			case 3: type = kRobEye; break;
			case 4: type = kRobDrone; break;
			case 5: type = kRobSoldier; break;
			default: type = kRobCube; break;
			}
		}

		createObject(type, wxloc, wyloc, ang);
	}

	debug("initRobots: spawned %d robots on level %d", maxrob, _level);
}

void ColonyEngine::loadMacColors() {
	_hasMacColors = false;
	Common::SeekableReadStream *file = nullptr;

	// Try MacResManager first (for resource fork / AppleDouble files)
	Common::Path path("Color256");
	file = Common::MacResManager::openFileOrDataFork(path);
	if (!file) {
		path = Common::Path("CData/Color256");
		file = Common::MacResManager::openFileOrDataFork(path);
	}

	// Fallback to plain file open (for raw data files)
	if (!file) {
		Common::File *f = new Common::File();
		if (f->open(Common::Path("Color256"))) {
			file = f;
		} else if (f->open(Common::Path("CData/Color256"))) {
			file = f;
		} else {
			delete f;
		}
	}
	if (!file)
		return;

	uint32 vers = file->readUint32BE(); // Should be 'v1.0' = 0x76312E30
	(void)vers; // Ignore
	uint16 cnum = file->readUint16BE();
	if (cnum > 145) cnum = 145;

	for (int i = 0; i < cnum; i++) {
		_macColors[i].fg[0] = file->readUint16BE();
		_macColors[i].fg[1] = file->readUint16BE();
		_macColors[i].fg[2] = file->readUint16BE();
		_macColors[i].bg[0] = file->readUint16BE();
		_macColors[i].bg[1] = file->readUint16BE();
		_macColors[i].bg[2] = file->readUint16BE();
		_macColors[i].pattern = file->readUint16BE();
	}
	delete file;
	_hasMacColors = true;
	debug("Loaded %d Mac colors", cnum);
}

void ColonyEngine::menuCommandsCallback(int action, Common::String &text, void *data) {
	ColonyEngine *engine = (ColonyEngine *)data;
	engine->handleMenuAction(action);
}

void ColonyEngine::handleMenuAction(int action) {
	switch (action) {
	case kMenuActionAbout:
		inform("The Colony\nCopyright 1988\nDavid A. Smith", true);
		break;
	case kMenuActionNew:
		loadMap(1);
		break;
	case kMenuActionOpen:
		_system->lockMouse(false);
		loadGameDialog();
		_system->lockMouse(true);
		_system->warpMouse(_centerX, _centerY);
		_system->getEventManager()->purgeMouseEvents();
		break;
	case kMenuActionSave:
	case kMenuActionSaveAs:
		_system->lockMouse(false);
		saveGameDialog();
		_system->lockMouse(true);
		_system->warpMouse(_centerX, _centerY);
		_system->getEventManager()->purgeMouseEvents();
		break;
	case kMenuActionQuit:
		quitGame();
		break;
	case kMenuActionSound:
		// Sound toggle (TODO: implement sound on/off state)
		break;
	case kMenuActionCrosshair:
		_crosshair = !_crosshair;
		break;
	case kMenuActionPolyFill:
		_wireframe = !_wireframe;
		break;
	case kMenuActionCursorShoot:
		// Toggle cursor-based shooting (not yet implemented)
		break;
	default:
		break;
	}
}

void ColonyEngine::initMacMenus() {
	if (_renderMode != Common::kRenderMacintosh) {
		_menuBarHeight = 0;
		return;
	}

	// Create RGBA surface for the MacWindowManager to render into.
	Graphics::PixelFormat rgba(4, 8, 8, 8, 8, 24, 16, 8, 0);
	_menuSurface = new Graphics::ManagedSurface(_width, _height, rgba);

	_wm = new Graphics::MacWindowManager(Graphics::kWMModeNoDesktop | Graphics::kWMNoScummVMWallpaper | Graphics::kWMMode32bpp | Graphics::kWMModeNoSystemRedraw);

	// Override WM color values for 32bpp RGBA rendering.
	// The defaults are palette indices (0-6) which are meaningless in 32bpp mode.
	_wm->_colorBlack  = rgba.ARGBToColor(255,   0,   0,   0);
	_wm->_colorGray80 = rgba.ARGBToColor(255, 128, 128, 128);
	_wm->_colorGray88 = rgba.ARGBToColor(255, 136, 136, 136);
	_wm->_colorGrayEE = rgba.ARGBToColor(255, 238, 238, 238);
	_wm->_colorWhite  = rgba.ARGBToColor(255, 255, 255, 255);
	_wm->_colorGreen  = rgba.ARGBToColor(  0,   0, 255,   0); // transparent key
	_wm->_colorGreen2 = rgba.ARGBToColor(  0,   0, 207,   0); // transparent key 2

	_wm->setScreen(_menuSurface);

	_macMenu = _wm->addMenu();
	_macMenu->setCommandsCallback(menuCommandsCallback, this);

	// Build menus matching original Mac Colony (inits.c lines 43-53, gmain.c DoCommand).
	// addStaticMenus() auto-adds the Apple menu at index 0, so:
	//   index 0 = Apple, 1 = File, 2 = Edit, 3 = Options
	// NOTE: menunum=0 is the loop terminator, so Apple submenu items
	// must be added manually after addStaticMenus() (see WAGE pattern).
	static const Graphics::MacMenuData menuItems[] = {
		{-1, "File",            0, 0, true},
		{-1, "Edit",            0, 0, true},
		{-1, "Options",         0, 0, true},
		// File submenu (index 1)
		{1, "New Game",                     kMenuActionNew, 'N', true},
		{1, "Open Game...",                 kMenuActionOpen, 'O', true},
		{1, "Save Game",                    kMenuActionSave, 'S', true},
		{1, "Save As...",                   kMenuActionSaveAs, 0, true},
		{1, nullptr,                        0, 0, false},   // separator
		{1, "Quit",                         kMenuActionQuit, 'Q', true},
		// Edit submenu (index 2, disabled — original Mac had these but non-functional)
		{2, "Undo",                         0, 'Z', false},
		{2, nullptr,                        0, 0, false},
		{2, "Cut",                          0, 'X', false},
		{2, "Copy",                         0, 'C', false},
		{2, "Paste",                        0, 'V', false},
		// Options submenu (index 3)
		{3, "Sound",                        kMenuActionSound, 0, true},
		{3, "Crosshair",                    kMenuActionCrosshair, 0, true},
		{3, "Polygon Fill",                 kMenuActionPolyFill, 0, true},
		{3, "Cursor Shoot",                 kMenuActionCursorShoot, 0, true},
		// Terminator
		{0, nullptr,                        0, 0, false}
	};
	_macMenu->addStaticMenus(menuItems);

	// Add Apple submenu item manually (menunum=0 can't go through addStaticMenus)
	_macMenu->addSubMenu(nullptr, 0);
	_macMenu->addMenuItem(_macMenu->getSubmenu(nullptr, 0), "About The Colony", kMenuActionAbout);

	_macMenu->calcDimensions();

	_menuBarHeight = 20;
}

void ColonyEngine::initTrig() {
	// Compute standard sin/cos lookup tables (256 steps = full circle, scaled by 128)
	for (int i = 0; i < 256; i++) {
		float rad = (float)i * 2.0f * M_PI / 256.0f;
		_sint[i] = (int)roundf(128.0f * sinf(rad));
		_cost[i] = (int)roundf(128.0f * cosf(rad));
	}
}

Common::Error ColonyEngine::run() {
	// Open Colony resource fork (must happen in run(), not constructor,
	// because SearchMan doesn't have the game path until now)
	if (getPlatform() == Common::kPlatformMacintosh) {
		if (!_resMan->open("Colony")) {
			if (!_resMan->open("Colony.bin")) {
				warning("Failed to open Colony resource fork");
			}
		}
		// Try to open Color Colony for additional color PICT resources
		if (!_colorResMan->open("(Color) Colony")) {
			debug("Color Colony resource fork not found (optional)");
		}
		_sound->init();
	}

	// Original Mac Colony: rScreen capped at 640x480 (inits.c lines 111-112).
	// DOS EGA: 640x350 with non-square pixels displayed at 4:3.
	// Mac uses square pixels at native 640x480.
	if (_renderMode == Common::kRenderMacintosh) {
		_width = 640;
		_height = 480;
	} else {
		_width = 640;
		_height = 350;
	}

	if (_widescreen) {
		_width = _height * 16 / 9;
	}

	_gfx = createRenderer(_system, _width, _height);
	if (!_gfx)
		return Common::kUserCanceled;

	updateViewportLayout();
	const Graphics::PixelFormat format = _system->getScreenFormat();
	debug("Screen format: %d bytesPerPixel. Actual size: %dx%d", format.bytesPerPixel, _width, _height);

	// Setup a palette with standard 16 colors followed by grayscale
	byte pal[256 * 3];
	static const byte egaColors[16][3] = {
		{0, 0, 0}, {0, 0, 170}, {0, 170, 0}, {0, 170, 170},
		{170, 0, 0}, {170, 0, 170}, {170, 85, 0}, {170, 170, 170},
		{85, 85, 85}, {85, 85, 255}, {85, 255, 85}, {85, 255, 255},
		{255, 85, 85}, {255, 85, 255}, {255, 255, 85}, {255, 255, 255}
	};
	for (int i = 0; i < 16; i++) {
		pal[i * 3 + 0] = egaColors[i][0];
		pal[i * 3 + 1] = egaColors[i][1];
		pal[i * 3 + 2] = egaColors[i][2];
	}
	for (int i = 16; i < 256; i++) {
		pal[i * 3 + 0] = i;
		pal[i * 3 + 1] = i;
		pal[i * 3 + 2] = i;
	}

	loadMacColors();
	if (_hasMacColors) {
		for (int i = 0; i < 145; i++) {
			pal[(100 + i) * 3 + 0] = _macColors[i].fg[0] >> 8;
			pal[(100 + i) * 3 + 1] = _macColors[i].fg[1] >> 8;
			pal[(100 + i) * 3 + 2] = _macColors[i].fg[2] >> 8;
		}
	}

	_gfx->setPalette(pal, 0, 256);

	initMacMenus();
	updateViewportLayout(); // Recalculate for menu bar height

	// Frame limiter: target 60fps, like Freescape engine
	_frameLimiter = new Graphics::FrameLimiter(_system, 60);

	playIntro();

	loadMap(1); // Try to load the first map
	_mouseLocked = true;
	_system->lockMouse(true);
	_system->warpMouse(_centerX, _centerY);

	int mouseDX = 0, mouseDY = 0;
	bool mouseMoved = false;
	while (!shouldQuit()) {
		_frameLimiter->startFrame();
		Common::Event event;
		while (_system->getEventManager()->pollEvent(event)) {
			// Let MacWindowManager handle menu events first
			if (_wm) {
				bool wasMenuActive = _wm->isMenuActive();
				if (_wm->processEvent(event)) {
					// WM consumed the event (menu interaction)
					if (!wasMenuActive && _wm->isMenuActive()) {
						_system->lockMouse(false);
					}
					continue;
				}
				if (wasMenuActive && !_wm->isMenuActive()) {
					_system->lockMouse(_mouseLocked);
					if (_mouseLocked) {
						_system->warpMouse(_centerX, _centerY);
						_system->getEventManager()->purgeMouseEvents();
						mouseDX = mouseDY = 0;
						mouseMoved = false;
					}
				}
			}

				if (event.type == Common::EVENT_KEYDOWN) {
					debug("Key down: %d", event.kbd.keycode);
					const bool allowInteraction = (event.kbd.flags & Common::KBD_CTRL) == 0;
					// DOS movement: xai[ang] << speedshift, where xai[i] = cost[i] >> 4
					const int moveX = (_cost[_me.look] * (1 << _speedShift)) >> 4;
					const int moveY = (_sint[_me.look] * (1 << _speedShift)) >> 4;
					const int rotSpeed = 1 << (_speedShift - 1); // DOS: speed = 1 << (speedshift-1)
					switch (event.kbd.keycode) {
					// Move forward (DOS: w / 8 / up)
					case Common::KEYCODE_UP:
					case Common::KEYCODE_w:
						cCommand(_me.xloc + moveX, _me.yloc + moveY, allowInteraction);
						break;
					// Move backward (DOS: s / 5 / down)
					case Common::KEYCODE_DOWN:
					case Common::KEYCODE_s:
						cCommand(_me.xloc - moveX, _me.yloc - moveY, allowInteraction);
						break;
					// Strafe left (DOS: a / 4 / left)
					case Common::KEYCODE_LEFT:
					case Common::KEYCODE_a:
					{
						uint8 strafeAngle = (uint8)((int)_me.look + 64);
						int sx = (_cost[strafeAngle] * (1 << _speedShift)) >> 4;
						int sy = (_sint[strafeAngle] * (1 << _speedShift)) >> 4;
						cCommand(_me.xloc + sx, _me.yloc + sy, allowInteraction);
						break;
					}
					// Strafe right (DOS: d / 6 / right)
					case Common::KEYCODE_RIGHT:
					case Common::KEYCODE_d:
					{
						uint8 strafeAngle = (uint8)((int)_me.look - 64);
						int sx = (_cost[strafeAngle] * (1 << _speedShift)) >> 4;
						int sy = (_sint[strafeAngle] * (1 << _speedShift)) >> 4;
						cCommand(_me.xloc + sx, _me.yloc + sy, allowInteraction);
						break;
					}
					// Rotate left (DOS: q / 7)
					case Common::KEYCODE_q:
						_me.ang += rotSpeed;
						_me.look += rotSpeed;
						break;
					// Rotate right (DOS: e / 9)
					case Common::KEYCODE_e:
						_me.ang -= rotSpeed;
						_me.look -= rotSpeed;
						break;
					// Look left (DOS: z / 1)
					case Common::KEYCODE_z:
						_me.look = _me.ang + 64;
						break;
					// Look right (DOS: c / 3)
					case Common::KEYCODE_c:
						_me.look = _me.ang - 64;
						break;
					// Look behind (DOS: x / 2)
					case Common::KEYCODE_x:
						_me.look = _me.ang + 128;
						break;
					// Speed 1-5 (DOS: keys 1-5)
					case Common::KEYCODE_1:
					case Common::KEYCODE_2:
					case Common::KEYCODE_3:
					case Common::KEYCODE_4:
					case Common::KEYCODE_5:
						_speedShift = event.kbd.keycode - Common::KEYCODE_1 + 1;
						debug("Speed: %d", _speedShift);
						break;
					// F7: toggle dashboard (DOS: doFunctionKey case 7)
					case Common::KEYCODE_F7:
						_showDashBoard = !_showDashBoard;
						break;
					// F8: toggle polyfill (DOS: doFunctionKey case 8)
					case Common::KEYCODE_F8:
						_wireframe = !_wireframe;
						debug("Polyfill: %s", _wireframe ? "off (wireframe)" : "on (filled)");
						break;
					// F10: ScummVM menu (replaces DOS F10 pause)
					case Common::KEYCODE_F10:
						_system->lockMouse(false);
						openMainMenuDialog();
						_gfx->computeScreenViewport();
						_system->lockMouse(_mouseLocked);
						if (_mouseLocked) {
							_system->warpMouse(_centerX, _centerY);
							_system->getEventManager()->purgeMouseEvents();
						}
						break;
					// F11: toggle fullscreen (hide Mac menu bar)
					case Common::KEYCODE_F11:
						if (_macMenu) {
							_fullscreen = !_fullscreen;
							_menuBarHeight = _fullscreen ? 0 : 20;
							updateViewportLayout();
						}
						break;
					// Space: toggle mouselook / free cursor
					case Common::KEYCODE_SPACE:
						if (_fl == 2)
							dropCarriedObject();
						else if (_fl == 1)
							exitForklift();
						else {
							_mouseLocked = !_mouseLocked;
							_system->lockMouse(_mouseLocked);
							if (_mouseLocked) {
								_system->warpMouse(_centerX, _centerY);
								_system->getEventManager()->purgeMouseEvents();
								mouseDX = mouseDY = 0;
								mouseMoved = false;
							}
						}
						break;
					// Escape: also opens ScummVM menu
					case Common::KEYCODE_ESCAPE:
						_system->lockMouse(false);
						openMainMenuDialog();
						_gfx->computeScreenViewport();
						_system->lockMouse(_mouseLocked);
						if (_mouseLocked) {
							_system->warpMouse(_centerX, _centerY);
							_system->getEventManager()->purgeMouseEvents();
						}
						break;
					default:
						break;
					}
				debug("Me: x=%d y=%d", _me.xloc, _me.yloc);
			} else if (event.type == Common::EVENT_MOUSEMOVE) {
				mouseDX += event.relMouse.x;
				mouseDY += event.relMouse.y;
				mouseMoved = true;
			} else if (event.type == Common::EVENT_SCREEN_CHANGED) {
				_gfx->computeScreenViewport();
			}
		}

		if (mouseMoved && _mouseLocked) {
			if (mouseDX != 0) {
				_me.look = (uint8)((int)_me.look - (mouseDX * _mouseSensitivity));
			}
			if (mouseDY != 0) {
				_me.lookY = (int8)CLIP<int>((int)_me.lookY - (mouseDY * _mouseSensitivity), -64, 64);
			}
			// Warp back to center and purge remaining mouse events
			// to prevent the warp from generating phantom deltas (Freescape pattern)
			_system->warpMouse(_centerX, _centerY);
			_system->getEventManager()->purgeMouseEvents();
			mouseMoved = false;
			mouseDX = mouseDY = 0;
		}

		_gfx->clear((_corePower[_coreIndex] > 0) ? 15 : 0);
		
		corridor();
		drawDashboardStep1();
		drawCrosshair();
		checkCenter();
		
		// Draw Mac menu bar overlay (render directly to our surface, skip WM's
		// g_system->copyRectToScreen which conflicts with the OpenGL backend)
		if (_macMenu && _menuSurface && !_fullscreen) {
			_menuSurface->fillRect(Common::Rect(_width, _height), _menuSurface->format.ARGBToColor(0, 0, 0, 0));
			_macMenu->draw(_menuSurface, true);
			_gfx->drawSurface(&_menuSurface->rawSurface(), 0, 0);
		}

		_displayCount++; // Mac: count++ after Display()
		_frameLimiter->delayBeforeSwap();
		_gfx->copyToScreen();
	}

	return Common::kNoError;
}

bool ColonyEngine::checkSkipRequested() {
	// Non-blocking check for Shift+S skip during intro/animation sequences.
	// Drains pending events, handles screen refresh and quit events.
	// Returns true if Shift+S was pressed or shouldQuit() is true.
	Common::Event event;
	while (_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			return true;
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_s &&
			    (event.kbd.flags & Common::KBD_SHIFT))
				return true;
			break;
		case Common::EVENT_SCREEN_CHANGED:
			_gfx->computeScreenViewport();
			break;
		default:
			break;
		}
	}
	return shouldQuit();
}

bool ColonyEngine::waitForInput() {
	// Blocking wait for any key press or mouse click.
	// Handles screen refresh, quit events, and mouse movement while waiting.
	// Returns true if Shift+S was pressed (for intro skip propagation).
	while (!shouldQuit()) {
		Common::Event event;
		while (_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				return false;
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_s &&
				    (event.kbd.flags & Common::KBD_SHIFT))
					return true;
				return false;
			case Common::EVENT_LBUTTONDOWN:
				return false;
			case Common::EVENT_SCREEN_CHANGED:
				_gfx->computeScreenViewport();
				break;
			default:
				break;
			}
		}
		_system->updateScreen();
		_system->delayMillis(10);
	}
	return false;
}

void ColonyEngine::playIntro() {
	if (getPlatform() == Common::kPlatformMacintosh) {
		// Load the Mac "Commando" font (FOND 190, 12pt) from Colony resources.
		// Original intro.c: TextFont(190); TextSize(12);
		// FONT resource ID = FOND_ID * 128 + size = 190 * 128 + 12 = 24332
		// Some builds store it as NFNT instead of FONT.
		Graphics::MacFONTFont *macFont = nullptr;
		if (_resMan) {
			const uint16 fontResID = 24332;
			Common::SeekableReadStream *fontStream = _resMan->getResource(MKTAG('N', 'F', 'N', 'T'), fontResID);
			if (!fontStream)
				fontStream = _resMan->getResource(MKTAG('F', 'O', 'N', 'T'), fontResID);
			if (fontStream) {
				macFont = new Graphics::MacFONTFont();
				if (!macFont->loadFont(*fontStream)) {
					warning("playIntro: failed to load Commando 12pt font");
					delete macFont;
					macFont = nullptr;
				}
				delete fontStream;
			} else {
				// List available font resources for debugging
				Common::MacResIDArray nfntIDs = _resMan->getResIDArray(MKTAG('N', 'F', 'N', 'T'));
				Common::MacResIDArray fontIDs = _resMan->getResIDArray(MKTAG('F', 'O', 'N', 'T'));
				debug("playIntro: FONT/NFNT %d not found. Available NFNT IDs: %d, FONT IDs: %d",
				      fontResID, nfntIDs.size(), fontIDs.size());
				for (uint i = 0; i < nfntIDs.size(); i++)
					debug("  NFNT %d", nfntIDs[i]);
				for (uint i = 0; i < fontIDs.size(); i++)
					debug("  FONT %d", fontIDs[i]);
			}
		}

		// Original: intro() in intro.c, lines 40-119
		// qt flag propagates through sections — only modifier+click sets it
		bool qt = false;

		// 1. ScrollInfo() - scrolling story text with BeamMe sound
		qt = scrollInfo(macFont);

		// 2. Wait for BeamMe sound to finish
		// Original: if(!qt) while(!SoundDone());
		while (!qt && !shouldQuit() && _sound->isPlaying())
			_system->delayMillis(10);

		// Original: if(Button()) qt=OptionKey(); — check for skip
		if (!qt)
			qt = checkSkipRequested();

		if (!qt) {
			// 3. Logo 1 + PlayMars + makestars
			// Original: FillRect black; DoPicture; PlayMars(); makestars()
			_gfx->clear(_gfx->black());
			if (!drawPict(-32565))  // Color Colony
				drawPict(-32748);   // B&W Colony
			_sound->play(Sound::kMars);
			qt = makeStars(_screenR, 0);

			if (!qt) {
				// 4. Logo 2 + makestars (inside the same !qt block as original)
				// Original: FillRect black; DoPicture(-32564); makestars()
				_gfx->clear(_gfx->black());
				if (!drawPict(-32564))  // Color Colony
					drawPict(-32750);   // B&W Colony
				qt = makeStars(_screenR, 0);
			}

			if (!qt) {
				// 5. Empty starfield
				// Original: FillRect black; makestars()
				_gfx->clear(_gfx->black());
				_gfx->copyToScreen();
				qt = makeStars(_screenR, 0);
			}

			if (!qt) {
				// 6. TimeSquare("...BLACK HOLE COLLISION...")
				qt = timeSquare("...BLACK HOLE COLLISION...", macFont);
			}

			if (!qt) {
				// 7. Makeblackhole()
				_gfx->clear(_gfx->black());
				_gfx->copyToScreen();
				qt = makeBlackHole();
			}

			if (!qt) {
				// 8. TimeSquare("...FUEL HAS BEEN DEPLETED...")
				qt = timeSquare("...FUEL HAS BEEN DEPLETED...", macFont);
			}

			// Original: SetPort(&metaPort); before next TimeSquare (no !qt guard)
			if (!qt) {
				// 9. TimeSquare("...PREPARE FOR CRASH LANDING...")
				qt = timeSquare("...PREPARE FOR CRASH LANDING...", macFont);
			}

			if (!qt) {
				// 10. makeplanet() + EndCSound()
				// Simplified: starfield + delay (makeplanet draws a rotating planet)
				_gfx->clear(_gfx->black());
				_gfx->copyToScreen();
				qt = makeStars(_screenR, 0);
				_sound->stop(); // EndCSound()
			}
		}

		// 11. Final crash — always runs (even if qt)
		// Original: FillRect black; if(!qt) while(!SoundDone());
		_gfx->clear(_gfx->black());
		_gfx->copyToScreen();
		while (!qt && !shouldQuit() && _sound->isPlaying())
			_system->delayMillis(10);

		// Original: DoExplodeSound(); while(!SoundDone()) InvertRect(&rScreen); StopSound();
		_sound->play(Sound::kExplode);
		while (!shouldQuit() && _sound->isPlaying()) {
			_gfx->clear(_gfx->white());
			_gfx->copyToScreen();
			_system->delayMillis(50);
			_gfx->clear(_gfx->black());
			_gfx->copyToScreen();
			_system->delayMillis(50);
		}
		_sound->stop();
		_gfx->clear(_gfx->black());
		_gfx->copyToScreen();
		delete macFont;
		macFont = nullptr;

		// Restore palette entries modified during intro (128, 160-176, 200-213)
		// back to grayscale so normal gameplay rendering isn't affected
		byte restorePal[256 * 3];
		for (int i = 0; i < 256; i++) {
			restorePal[i * 3 + 0] = i;
			restorePal[i * 3 + 1] = i;
			restorePal[i * 3 + 2] = i;
		}
		_gfx->setPalette(restorePal + 128 * 3, 128, 128);
	} else {
		scrollInfo();
	}
}

bool ColonyEngine::scrollInfo(const Graphics::Font *macFont) {
	// Original: ScrollInfo() in intro.c, lines 138-221
	// Renders story text in blue gradient to offscreen half-width buffer,
	// scrolls it up from below screen with DoBeammeSound(),
	// waits for click, then scrolls it off the top.
	// Mac original: TextFont(190 = Commando); TextSize(12);
	// Text blue starts at 0xFFFF and fades by -4096 per visible line.
	const char *story[] = {
		"",
		"Mankind has left the",
		"cradle of earth and",
		"is beginning to eye",
		"the galaxy. He has",
		"begun to colonize",
		"distant planets but has",
		"yet to meet any alien",
		"life forms.",
		"",      // null separator in original
		"Until now...",
		"",      // null separator in original
		"Click to begin",
		"the Adventure..."
	};
	const int storyLength = ARRAYSIZE(story);

	if (getPlatform() == Common::kPlatformMacintosh)
		_sound->play(Sound::kBeamMe);

	_gfx->clear(_gfx->black());
	_gfx->copyToScreen();

	Graphics::DosFont dosFont;
	const Graphics::Font *font = macFont ? macFont : (const Graphics::Font *)&dosFont;

	// Original uses 19px line height, centers vertically within height
	int lineHeight = 19;
	int totalHeight = lineHeight * storyLength;
	int ht = (_height - totalHeight) / 2;

	// Set up blue gradient palette entries (200-213) for story text
	// Mac original: tColor.blue starts at 0xFFFF and decreases by 4096 per visible line
	byte pal[14 * 3]; // storyLength entries
	memset(pal, 0, sizeof(pal));
	for (int i = 0; i < storyLength; i++) {
		int blue = 255 - i * 16;
		if (blue < 0) blue = 0;
		pal[i * 3 + 0] = 0;     // R
		pal[i * 3 + 1] = 0;     // G
		pal[i * 3 + 2] = blue;  // B
	}
	_gfx->setPalette(pal, 200, storyLength);

	// Phase 1: Scroll text up from below screen
	// Original: scrollRect starts at bottom (stayRect.bottom..stayRect.bottom*2),
	// moves up by inc=4 each frame until text is visible at its correct position.
	// We simulate by drawing text with a y-offset that starts at _height and decreases to 0.
	int inc = 4;
	bool qt = false;

	for (int scrollOff = _height; scrollOff > 0 && !qt; scrollOff -= inc) {
		if (checkSkipRequested()) {
			qt = true;
			_sound->stop();
			break;
		}

		_gfx->clear(_gfx->black());
		for (int i = 0; i < storyLength; i++) {
			int drawY = ht + lineHeight * i + scrollOff;
			if (strlen(story[i]) > 0 && drawY >= 0 && drawY < _height)
				_gfx->drawString(font, story[i], _width / 2, drawY, 200 + i, Graphics::kTextAlignCenter);
		}
		_gfx->copyToScreen();
		_system->delayMillis(16);
	}

	// Draw final position (scrollOff = 0)
	if (!qt) {
		_gfx->clear(_gfx->black());
		for (int i = 0; i < storyLength; i++) {
			if (strlen(story[i]) > 0)
				_gfx->drawString(font, story[i], _width / 2, ht + lineHeight * i, 200 + i, Graphics::kTextAlignCenter);
		}
		_gfx->copyToScreen();
	}

	// Wait for click (original: while(!Button()); while(Button()&&!qt);)
	if (!qt)
		qt = waitForInput();

	// Phase 2: Scroll text off the top of the screen
	// Original: scrollRect continues moving up, text slides upward
	if (!qt) {
		for (int scrollOff = 0; scrollOff > -_height && !qt; scrollOff -= inc) {
			if (checkSkipRequested()) {
			qt = true;
			_sound->stop();
			break;
		}

			_gfx->clear(_gfx->black());
			for (int i = 0; i < storyLength; i++) {
				int drawY = ht + lineHeight * i + scrollOff;
				if (strlen(story[i]) > 0 && drawY >= -lineHeight && drawY < _height)
					_gfx->drawString(font, story[i], _width / 2, drawY, 200 + i, Graphics::kTextAlignCenter);
			}
			_gfx->copyToScreen();
			_system->delayMillis(16);
		}
	}

	// Original does NOT stop the sound here — BeamMe continues playing
	// and intro() waits for it with while(!SoundDone()) after ScrollInfo returns.
	// Only stop if skipping (qt already stops in the modifier+click handlers above).
	_gfx->clear(_gfx->black());
	_gfx->copyToScreen();
	return qt;
}

bool ColonyEngine::makeStars(const Common::Rect &r, int btn) {
	// Original: makestars() in stars.c
	// Uses 75 moving stars that streak outward from center using XOR lines.
	const int MAXSTAR = 0x1FF;
	const int NSTARS = 75;
	const int deltapd = 0x008;

	int centerX = r.width() / 2;
	int centerY = r.height() / 2;

	// Build perspective lookup table: rtable[i] = (128*128)/i
	int rtable[MAXSTAR + 1];
	rtable[0] = 32000;
	for (int i = 1; i <= MAXSTAR; i++)
		rtable[i] = (128 * 128) / i;

	// First draw static background stars (150 random dots)
	for (int i = 0; i < 150; i++) {
		int s = (int16)(_randomSource.getRandomNumber(0xFFFF)) >> 7;
		int c = (int16)(_randomSource.getRandomNumber(0xFFFF)) >> 7;
		int d = _randomSource.getRandomNumber(MAXSTAR);
		if (d < 1) d = 1;
		int rr = rtable[d];
		int xx = centerX + (int)(((long long)s * rr) >> 7);
		int yy = centerY + (int)(((long long)c * rr) >> 7);
		if (xx >= 0 && xx < _width && yy >= 0 && yy < _height)
			_gfx->setPixel(xx, yy, 15);
	}

	// Initialize moving stars — original uses PenMode(patXor) so stars
	// don't damage the logo underneath (XOR drawing the same line twice
	// restores the original pixels).
	int xang[NSTARS], yang[NSTARS], dist[NSTARS];
	int xsave1[NSTARS], ysave1[NSTARS], xsave2[NSTARS], ysave2[NSTARS];

	_gfx->setXorMode(true);

	for (int i = 0; i < NSTARS; i++) {
		int d = dist[i] = _randomSource.getRandomNumber(MAXSTAR);
		if (d <= 0x030) d = dist[i] = MAXSTAR;
		int s = xang[i] = (int16)(_randomSource.getRandomNumber(0xFFFF)) >> 7;
		int c = yang[i] = (int16)(_randomSource.getRandomNumber(0xFFFF)) >> 7;

		int rr = rtable[d];
		xsave1[i] = centerX + (int)(((long long)s * rr) >> 7);
		ysave1[i] = centerY + (int)(((long long)c * rr) >> 7);

		int d2 = d - deltapd;
		if (d2 < 1)
			d2 = 1;
		rr = rtable[d2];
		xsave2[i] = centerX + (int)(((long long)s * rr) >> 7);
		ysave2[i] = centerY + (int)(((long long)c * rr) >> 7);

		_gfx->drawLine(xsave1[i], ysave1[i], xsave2[i], ysave2[i], 15);
	}
	_gfx->copyToScreen();

	// Animate: original loops ~200 frames or until Mars sound repeats 2x
	for (int k = 0; k < 120; k++) {
		if (checkSkipRequested()) {
			_gfx->setXorMode(false);
			return true;
		}

		for (int i = 0; i < NSTARS; i++) {
			// Erase previous — XOR the same line again to restore underlying pixels
			_gfx->drawLine(xsave1[i], ysave1[i], xsave2[i], ysave2[i], 15);

			int s = xang[i];
			int c = yang[i];

			if (dist[i] <= 0x030) {
				dist[i] = MAXSTAR;
				int rr = rtable[MAXSTAR];
				xsave1[i] = centerX + (int)(((long long)s * rr) >> 7);
				ysave1[i] = centerY + (int)(((long long)c * rr) >> 7);
			} else {
				xsave1[i] = xsave2[i];
				ysave1[i] = ysave2[i];
			}

			int d = (dist[i] -= deltapd);
			if (d < 1) d = 1;
			int rr = rtable[d];
			xsave2[i] = centerX + (int)(((long long)s * rr) >> 7);
			ysave2[i] = centerY + (int)(((long long)c * rr) >> 7);

			// Draw new star position
			_gfx->drawLine(xsave1[i], ysave1[i], xsave2[i], ysave2[i], 15);
		}
		_gfx->copyToScreen();
		_system->delayMillis(16);
	}

	// Fade-out phase: stars fly off without resetting (trails accumulate via XOR)
	int nstars = 2 * ((MAXSTAR - 0x030) / deltapd);
	if (nstars > 200) nstars = 200;
	for (int k = 0; k < nstars; k++) {
		if (checkSkipRequested()) {
			_gfx->setXorMode(false);
			return true;
		}

		for (int i = 0; i < NSTARS; i++) {
			int d = dist[i];
			int s = xang[i];
			int c = yang[i];
			dist[i] -= deltapd;
			if (dist[i] <= 0x030) dist[i] = MAXSTAR;

			if (d >= 1 && d <= MAXSTAR) {
				int rr1 = rtable[d];
				int d2 = d - deltapd;
				if (d2 < 1)
			d2 = 1;
				int rr2 = rtable[d2];
				int x1 = centerX + (int)(((long long)s * rr1) >> 7);
				int y1 = centerY + (int)(((long long)c * rr1) >> 7);
				int x2 = centerX + (int)(((long long)s * rr2) >> 7);
				int y2 = centerY + (int)(((long long)c * rr2) >> 7);
				_gfx->drawLine(x1, y1, x2, y2, 15);
			}
		}
		_gfx->copyToScreen();
		_system->delayMillis(8);
	}

	_gfx->setXorMode(false);
	return false;
}

bool ColonyEngine::makeBlackHole() {
	// Original: Makeblackhole() in intro.c
	// Mac original draws spiral lines with fading colors:
	// bcolor starts at (0,0,0) and subtracts (rd=2048, gd=1024, bd=4096) per step,
	// which wraps around creating shifting color gradients.
	// We use palette entries 128-191 for the gradient colors.
	int centerX = _width / 2;
	int centerY = _height / 2;
	int dec = 16;
	int starcnt = 0;

	// Build a lookup table matching the original rtable: rtable[i] = (128*128)/i
	int rtable[1024];
	rtable[0] = 32000;
	for (int i = 1; i < 1024; i++)
		rtable[i] = (128 * 128) / i;

	for (int k = 0; k < 17; k += 4) {
		// Reset color per k-iteration (matches Mac: bcolor = {0,0,0} at start of each k)
		int colorR = 0, colorG = 0, colorB = 0;
		int rd = 2048, gd = 1024, bd = 4096;

		for (int i = 1000; i > 32; i -= dec) {
			// Mac original subtracts from color channels (wrapping as uint16);
			// We simulate this as a gradient from dark to bright
			// Since Mac uses unsigned wrap: 0 - 4096 = 0xF000 = bright.
			// After one full cycle (16 steps), the colors cycle.
			// Map to palette entry based on step
			colorB = (colorB - bd) & 0xFFFF;
			colorR = (colorR - rd) & 0xFFFF;
			colorG = (colorG - gd) & 0xFFFF;

			// Map Mac 16-bit color to 8-bit
			uint8 palR = (colorR >> 8) & 0xFF;
			uint8 palG = (colorG >> 8) & 0xFF;
			uint8 palB = (colorB >> 8) & 0xFF;

			// Use palette entry 128 for current step color
			byte pal[3] = { palR, palG, palB };
			_gfx->setPalette(pal, 128, 1);

			starcnt++;
			if (starcnt == 8) starcnt = 0;

			for (int j = 0; j < 256; j += 8) {
				int idx = (j + starcnt) & 0xFF;
				int rt1 = rtable[MIN(i + k, 1023)];
				int x1 = centerX + (int)(((long long)rt1 * _sint[idx]) >> 7);
				int y1 = centerY + (int)(((long long)rt1 * _cost[idx]) >> 7);

				int rt2 = rtable[MIN(i + k + 8, 1023)];
				int x2 = centerX + (int)(((long long)rt2 * _sint[idx]) >> 7);
				int y2 = centerY + (int)(((long long)rt2 * _cost[idx]) >> 7);

				_gfx->drawLine(x1, y1, x2, y2, 128);
			}

			// Update screen every step and add a small delay
			// to simulate the original 68k rendering speed
			_gfx->copyToScreen();
			_system->delayMillis(16);

			if (checkSkipRequested()) return true;
		}
	}
	_gfx->copyToScreen();
	return false;
}

bool ColonyEngine::timeSquare(const Common::String &str, const Graphics::Font *macFont) {
	// Original: TimeSquare() in intro.c
	// 1. Draw horizontal blue gradient lines above/below center
	// 2. Scroll red text from right to center
	// 3. Flash klaxon 6 times with inverted rect
	// 4. Play Mars again, scroll text off to the left
	//
	// Mac original: fcolor starts at (0,0,0xFFFF) and subtracts 4096 per pair of lines.
	// Text is drawn in red (0xFFFF,0,0) on black background.

	_gfx->clear(_gfx->black());

	Graphics::DosFont dosFont;
	const Graphics::Font *font = macFont ? macFont : (const Graphics::Font *)&dosFont;
	int swidth = font->getStringWidth(str);

	int centery = _height / 2 - 10;

	// Set up gradient palette entries (160-175) for the blue gradient lines
	// Mac original: blue starts at 0xFFFF and decreases by 4096 per line pair
	for (int i = 0; i < 16; i++) {
		int blue = 255 - i * 16; // 255, 239, 223, ... 15
		if (blue < 0) blue = 0;
		byte pal[3] = { 0, 0, (byte)blue };
		_gfx->setPalette(pal, 160 + i, 1);
	}
	// Set palette entry 176 for red text
	{
		byte pal[3] = { 255, 0, 0 };
		_gfx->setPalette(pal, 176, 1);
	}

	// Draw blue gradient lines above/below center band
	for (int i = 0; i < 16; i++) {
		_gfx->drawLine(0, centery - 2 - i * 2, _width, centery - 2 - i * 2, 160 + i);
		_gfx->drawLine(0, centery - 2 - (i * 2 + 1), _width, centery - 2 - (i * 2 + 1), 160 + i);
		_gfx->drawLine(0, centery + 16 + i * 2, _width, centery + 16 + i * 2, 160 + i);
		_gfx->drawLine(0, centery + 16 + i * 2 + 1, _width, centery + 16 + i * 2 + 1, 160 + i);
	}
	_gfx->copyToScreen();

	// Phase 1: Scroll text in from the right to center
	// Original: if(Button()) if(qt=OptionKey()) break;
	int targetX = (_width - swidth) / 2;
	for (int x = _width; x > targetX; x -= 2) {
		_gfx->fillRect(Common::Rect(0, centery + 1, _width, centery + 16), 0);
		_gfx->drawString(font, str, x, centery + 2, 176, Graphics::kTextAlignLeft);
		_gfx->copyToScreen();

		if (checkSkipRequested()) return true;
		_system->delayMillis(8);
	}

	// Phase 2: Klaxon flash — original: EndCSound(); then 6 iterations of:
	//   if(Button()) if(qt=OptionKey()) break;
	//   while(!SoundDone()); StopSound(); PlayKlaxon(); InvertRect(&invrt);
	_sound->stop(); // EndCSound()
	for (int i = 0; i < 6; i++) {
		if (checkSkipRequested()) return true;

		// Wait for previous klaxon to finish
		while (_sound->isPlaying() && !shouldQuit())
			_system->delayMillis(10);
		_sound->stop();

		_sound->play(Sound::kKlaxon);

		// InvertRect(&invrt) — toggle the text band
		_gfx->fillRect(Common::Rect(0, centery + 1, _width, centery + 16), i % 2 ? 0 : 15);
		_gfx->drawString(font, str, targetX, centery + 2, i % 2 ? 15 : 0, Graphics::kTextAlignLeft);
		_gfx->copyToScreen();
	}
	// Wait for last klaxon
	while (_sound->isPlaying() && !shouldQuit())
		_system->delayMillis(10);
	_sound->stop();

	// Phase 3: PlayMars(), scroll text off to the left
	_sound->play(Sound::kMars);
	for (int x = targetX; x > -swidth; x -= 2) {
		_gfx->fillRect(Common::Rect(0, centery + 1, _width, centery + 16), 0);
		_gfx->drawString(font, str, x, centery + 2, 176, Graphics::kTextAlignLeft);
		_gfx->copyToScreen();

		if (checkSkipRequested()) return true;
		_system->delayMillis(8);
	}

	return false;
}

bool ColonyEngine::drawPict(int resID) {
	// Original: DoPicture() in intro.c, lines 861-886
	// Loads a PICT resource, centers it in the screen rect, draws with srcCopy.
	// Original applies clip rect inset by 1 pixel on all sides.
	Common::SeekableReadStream *pictStream = nullptr;

	// Try Color Colony resource fork first
	if (_colorResMan && _colorResMan->hasResFork())
		pictStream = _colorResMan->getResource(MKTAG('P', 'I', 'C', 'T'), (int16)resID);

	// Fall back to B&W Colony resource fork
	if (!pictStream && _resMan && (_resMan->isMacFile() || _resMan->hasResFork()))
		pictStream = _resMan->getResource(MKTAG('P', 'I', 'C', 'T'), (int16)resID);

	if (!pictStream) {
		debug("drawPict: PICT %d not found", resID);
		return false;
	}

	::Image::PICTDecoder decoder;
	if (decoder.loadStream(*pictStream)) {
		const Graphics::Surface *surface = decoder.getSurface();
		if (surface) {
			// Center PICT on screen (original: locate = centered within rScreen)
			int x = (_width - surface->w) / 2;
			int y = (_height - surface->h) / 2;
			bool isCLUT8 = (surface->format == Graphics::PixelFormat::createFormatCLUT8());
			const Graphics::Palette &pictPal = decoder.getPalette();

			// Original DoPicture clips 1 pixel inset from locate rect
			// clip.top = locate.top+1, clip.left = locate.left+1, etc.
			int clipX1 = x + 1;
			int clipY1 = y + 1;
			int clipX2 = x + surface->w - 1;
			int clipY2 = y + surface->h - 1;

			debug("drawPict(%d): %dx%d at (%d,%d), format=%dbpp, palette=%d entries",
			      resID, surface->w, surface->h, x, y,
			      surface->format.bytesPerPixel * 8, pictPal.size());

			// Draw PICT pixels using direct RGB (packRGB) for full color support.
			for (int iy = 0; iy < surface->h; iy++) {
				int sy = y + iy;
				if (sy < clipY1 || sy >= clipY2)
					continue;
				for (int ix = 0; ix < surface->w; ix++) {
					int sx = x + ix;
					if (sx < clipX1 || sx >= clipX2)
						continue;
					byte r, g, b;
					if (isCLUT8) {
						byte idx = *((const byte *)surface->getBasePtr(ix, iy));
						if (pictPal.size() > 0 && idx < (int)pictPal.size()) {
							pictPal.get(idx, r, g, b);
						} else {
							// B&W PICT: 0=white, 1=black
							r = g = b = (idx == 0) ? 255 : 0;
						}
					} else {
						uint32 pixel = surface->getPixel(ix, iy);
						surface->format.colorToRGB(pixel, r, g, b);
					}
					_gfx->setPixel(sx, sy, 0xFF000000 | ((uint32)r << 16) | ((uint32)g << 8) | b);
				}
			}
			_gfx->copyToScreen();
			delete pictStream;
			return true;
		}
	} else {
		warning("drawPict: failed to decode PICT %d", resID);
	}
	delete pictStream;
	return false;
}

bool ColonyEngine::loadAnimation(const Common::String &name) {
	_animationName = name;
	for (int i = 0; i < 6; i++)
		_animDisplay[i] = 1;

	// Look up per-animation BMColor map (from ganimate.c).
	_animBMColors.clear();
	Common::String nameLower = name;
	nameLower.toLowercase();
	for (const AnimColorEntry *e = kAnimColors; e->name; e++) {
		if (nameLower == e->name) {
			_animBMColors.resize(e->count);
			for (int i = 0; i < e->count; i++)
				_animBMColors[i] = e->colors[i];
			break;
		}
	}

	Common::String fileName = name + ".pic";
	Common::SeekableReadStream *file = Common::MacResManager::openFileOrDataFork(Common::Path(fileName));
	if (!file) {
		// Try lowercase for Mac
		fileName = name;
		fileName.toLowercase();
		file = Common::MacResManager::openFileOrDataFork(Common::Path(fileName));
		if (!file) {
			// Try CData directory
			fileName = "CData/" + fileName;
			file = Common::MacResManager::openFileOrDataFork(Common::Path(fileName));
			if (!file) {
				warning("Could not open animation file %s", name.c_str());
				return false;
			}
		}
	}

	deleteAnimation();

	// Read background data
	file->read(_topBG, 8);
	file->read(_bottomBG, 8);
	_divideBG = readSint16(*file);
	_backgroundActive = readSint16(*file) != 0;
	if (_backgroundActive) {
		_backgroundClip = readRect(*file);
		_backgroundLocate = readRect(*file);
		_backgroundMask = loadImage(*file);
		_backgroundFG = loadImage(*file);
	}

	// Read sprite data
	int16 maxsprite = readSint16(*file);
	readSint16(*file); // locSprite
	for (int i = 0; i < maxsprite; i++) {
		Sprite *s = new Sprite();
		s->fg = loadImage(*file);
		s->mask = loadImage(*file);
		s->used = readSint16(*file) != 0;
		s->clip = readRect(*file);
		s->locate = readRect(*file);
		_cSprites.push_back(s);
	}

	// Read complex sprite data
	int16 maxLSprite = readSint16(*file);
	readSint16(*file); // anum
	for (int i = 0; i < maxLSprite; i++) {
		ComplexSprite *ls = new ComplexSprite();
		int16 size = readSint16(*file);
		for (int j = 0; j < size; j++) {
			ComplexSprite::SubObject sub;
			sub.spritenum = readSint16(*file);
			sub.xloc = readSint16(*file);
			sub.yloc = readSint16(*file);
			ls->objects.push_back(sub);
		}
		ls->bounds = readRect(*file);
		ls->visible = readSint16(*file) != 0;
		ls->current = readSint16(*file);
		ls->xloc = readSint16(*file);
		ls->yloc = readSint16(*file);
		ls->acurrent = readSint16(*file);
		ls->axloc = readSint16(*file);
		ls->ayloc = readSint16(*file);
		ls->type = file->readByte();
		ls->frozen = file->readByte();
		ls->locked = file->readByte();
		ls->link = readSint16(*file);
		ls->key = readSint16(*file);
		ls->lock = readSint16(*file);
		ls->onoff = true;
		_lSprites.push_back(ls);
	}

	delete file;
	return true;
}

void ColonyEngine::deleteAnimation() {
	delete _backgroundMask;
	_backgroundMask = nullptr;
	delete _backgroundFG;
	_backgroundFG = nullptr;
	for (uint i = 0; i < _cSprites.size(); i++)
		delete _cSprites[i];
	_cSprites.clear();
	for (uint i = 0; i < _lSprites.size(); i++)
		delete _lSprites[i];
	_lSprites.clear();
}

void ColonyEngine::playAnimation() {
	_animationRunning = true;
	_system->lockMouse(false);
	_system->showMouse(true);
	_system->warpMouse(_centerX, _centerY);
	CursorMan.setDefaultArrowCursor(true);
	CursorMan.showMouse(true);
	_system->updateScreen();

	if (_animationName == "security" && !_unlocked) {
		for (int i = 0; i < 4; i++) {
			_decode1[i] = (uint8)(2 + _randomSource.getRandomNumber(3));
			setObjectState(27 + i, _decode1[i]);
		}
	} else if (_animationName == "reactor") {
		for (int i = 0; i < 6; i++) {
			setObjectOnOff(14 + i * 2, false);
			setObjectState(13 + i * 2, 1);
		}
	} else if (_animationName == "controls") {
		switch (_corePower[_coreIndex]) {
		case 0: setObjectState(2, 1); setObjectState(5, 1); break;
		case 1: setObjectState(2, 1); setObjectState(5, 2); break;
		case 2: setObjectState(2, 2); setObjectState(5, 1); break;
		}
	} else if (_animationName == "desk") {
		if (!(_action0 == 11 || _action0 == 18)) {
			for (int i = 1; i <= 5; i++)
				setObjectOnOff(i, false);
		} else {
			uint8 *decode = (_action0 == 11) ? _decode2 : _decode3;
			for (int i = 0; i < 4; i++) {
				if (decode[i])
					setObjectState(i + 2, decode[i]);
				else
					setObjectState(i + 2, 1);
			}
		}

		if (_action0 != 10) {
			setObjectOnOff(23, false);
			setObjectOnOff(24, false);
		}
		if (_action0 != 30)
			setObjectOnOff(6, false); // Teeth
		if (_action0 != 33) { // Jack-in-the-box
			for (int i = 18; i <= 21; i++)
				setObjectOnOff(i, false);
		}

		int ntype = _action1 / 10;
		switch (ntype) {
		case 0:
		case 1:
		case 2:
		case 3:
			setObjectOnOff(7, false);
			setObjectOnOff(8, false);
			setObjectOnOff(9, false);
			setObjectOnOff(22, false);
			setObjectOnOff(25, false);
			break;
		case 4: // letters
			setObjectOnOff(22, false);
			setObjectOnOff(9, false);
			setObjectOnOff(25, false);
			break;
		case 5: // book
			setObjectOnOff(7, false);
			setObjectOnOff(8, false);
			setObjectOnOff(9, false);
			setObjectOnOff(25, false);
			break;
		case 6: // clipboard
			setObjectOnOff(22, false);
			setObjectOnOff(7, false);
			setObjectOnOff(8, false);
			setObjectOnOff(25, false);
			break;
		case 7: // postit
			setObjectOnOff(22, false);
			setObjectOnOff(7, false);
			setObjectOnOff(8, false);
			setObjectOnOff(9, false);
			break;
		}
	} else if (_animationName == "vanity") {
		debug(0, "Vanity init: action0=%d action1=%d level=%d weapons=%d armor=%d", _action0, _action1, _level, _weapons, _armor);
		for (int i = 0; i < (int)_lSprites.size(); i++) {
			ComplexSprite *ls = _lSprites[i];
			debug(0, "  Vanity sprite %d: type=%d frozen=%d locked=%d current=%d onoff=%d key=%d lock=%d frames=%d",
				i + 1, ls->type, ls->frozen, ls->locked, ls->current, (int)ls->onoff, ls->key, ls->lock, (int)ls->objects.size());
		}
		// DOS DoVanity: set suit state on mirror display (object 1)
		if (_weapons && _armor)
			setObjectState(1, 3);
		else if (_weapons)
			setObjectState(1, 2);
		else if (_armor)
			setObjectState(1, 1);
		else
			setObjectState(1, 4);
		// Badge only visible on level 1
		if (_level != 1)
			setObjectOnOff(14, false);
		// Hide items based on action0 (num parameter in DOS)
		if (_action0 < 90) { // coffee cup only
			setObjectOnOff(4, false);
			setObjectOnOff(7, false);
			setObjectOnOff(13, false);
		} else if (_action0 < 100) { // paper
			setObjectOnOff(12, false);
			setObjectOnOff(4, false);
			setObjectOnOff(7, false);
		} else if (_action0 < 110) { // diary
			setObjectOnOff(12, false);
			setObjectOnOff(13, false);
			setObjectOnOff(7, false);
		} else if (_action0 < 120) { // book
			setObjectOnOff(12, false);
			setObjectOnOff(13, false);
			setObjectOnOff(4, false);
		}
	}

	while (_animationRunning && !shouldQuit()) {
		updateAnimation();
		drawAnimation();
		_gfx->copyToScreen();

		Common::Event event;
		while (_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_LBUTTONDOWN) {
				int item = whichSprite(event.mouse);
				if (item > 0) {
					handleAnimationClick(item);
				}
			} else if (event.type == Common::EVENT_RBUTTONDOWN) {
				// DOS: right-click exits animation (AnimControl returns FALSE on button-up)
				_animationRunning = false;
			} else if (event.type == Common::EVENT_MOUSEMOVE) {
				debug(5, "Animation Mouse: %d, %d", event.mouse.x, event.mouse.y);
			} else if (event.type == Common::EVENT_KEYDOWN) {
				int item = 0;
				if (event.kbd.keycode >= Common::KEYCODE_0 && event.kbd.keycode <= Common::KEYCODE_9) {
					item = 1 + (event.kbd.keycode - Common::KEYCODE_0);
				} else if (event.kbd.keycode >= Common::KEYCODE_KP0 && event.kbd.keycode <= Common::KEYCODE_KP9) {
					item = 1 + (event.kbd.keycode - Common::KEYCODE_KP0);
				} else if (event.kbd.keycode == Common::KEYCODE_RETURN || event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
					item = 12; // Enter
				} else if (event.kbd.keycode == Common::KEYCODE_BACKSPACE || event.kbd.keycode == Common::KEYCODE_DELETE) {
					item = 11; // Clear
				} else if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					_animationRunning = false;
				}

				if (item > 0) {
					handleAnimationClick(item);
				}
			}
		}
		_system->delayMillis(20);
	}

	_system->lockMouse(true);
	_system->showMouse(false);
	CursorMan.showMouse(false);
	CursorMan.popAllCursors();
	deleteAnimation();
}

void ColonyEngine::updateAnimation() {
	uint32 now = _system->getMillis();
	if (now - _lastAnimUpdate < 50) // Reduced to 50ms (20 fps) to make it "move"
		return;
	_lastAnimUpdate = now;

	for (uint i = 0; i < _lSprites.size(); i++) {
		ComplexSprite *ls = _lSprites[i];
		// type 0 are displays that auto-animate
		// Original NoShowIt ONLY checked !ls->locked
		if (ls->onoff && ls->type == 0 && !ls->locked && ls->objects.size() > 1) {
			ls->current++;
			if (ls->current >= (int)ls->objects.size())
				ls->current = 0;
		}
	}
}

// Resolve a BMColor entry to an ARGB color.
// bmEntry > 0: cColor index → use _macColors[idx].bg
// bmEntry < 0: negated Mac system color constant
// bmEntry == 0: level-based character color (depends on corepower)
uint32 ColonyEngine::resolveAnimColor(int16 bmEntry) const {
	if (bmEntry < 0) {
		return macSysColorToARGB(-bmEntry);
	} else if (bmEntry > 0) {
		if (bmEntry < 145)
			return packMacColorBG(_macColors[bmEntry].bg);
		return 0xFFFFFFFF;
	} else {
		// Zero = level-based (original gamesprt.c DrawlSprite/DrawBackGround):
		//   if(corepower[coreindex]) RGBBackColor(&cColor[c_char0+level-1].f);
		//   else RGBBackColor(&cColor[c_dwall].b);
		if (_corePower[_coreIndex] > 0 && _level >= 1 && _level <= 7)
			return packMacColorBG(_macColors[mc_char0 + _level - 1].fg);
		return packMacColorBG(_macColors[mc_dwall].bg);
	}
}

void ColonyEngine::drawAnimation() {
	_gfx->clear(0);

	// Center 416x264 animation area on screen (from original InitDejaVu)
	int ox = _screenR.left + (_screenR.width() - 416) / 2;
	ox = (ox / 8) * 8;
	int oy = _screenR.top + (_screenR.height() - 264) / 2;

	const bool useColor = (_hasMacColors && _renderMode == Common::kRenderMacintosh
	                       && !_animBMColors.empty());

	// Fill background patterns (416x264 area).
	// Color mode: QuickDraw pattern bit 1 → ForeColor (black), bit 0 → BackColor.
	// Original DrawBackGround():
	//   Top: BMColor[0]<0 → system color; ==0 → powered:c_char0+level-1.f, else:c_dwall.b
	//   Bottom: powered → c_lwall.f; unpowered → inherits top BackColor
	// B&W/DOS: preserve existing palette-index behavior (bit 1 → 15, bit 0 → 0).
	if (useColor) {
		const bool powered = (_corePower[_coreIndex] > 0);
		uint32 topBG = resolveAnimColor(_animBMColors[0]);
		// Bottom: only uses c_lwall.f when powered; unpowered inherits top color
		uint32 botBG = powered ? packMacColorBG(_macColors[mc_lwall].fg) : topBG;
		for (int y = 0; y < 264; y++) {
			byte *pat = (y < _divideBG) ? _topBG : _bottomBG;
			byte row = pat[y % 8];
			uint32 bg = (y < _divideBG) ? topBG : botBG;
			for (int x = 0; x < 416; x++) {
				bool set = (row & (0x80 >> (x % 8))) != 0;
				_gfx->setPixel(ox + x, oy + y, set ? (uint32)0xFF000000 : bg);
			}
		}
	} else {
		for (int y = 0; y < 264; y++) {
			byte *pat = (y < _divideBG) ? _topBG : _bottomBG;
			byte row = pat[y % 8];
			for (int x = 0; x < 416; x++) {
				bool set = (row & (0x80 >> (x % 8))) != 0;
				_gfx->setPixel(ox + x, oy + y, set ? 15 : 0);
			}
		}
	}

	// Draw background image if active.
	// Original: BMColor[1] only applied when corepower[coreindex] > 0.
	if (_backgroundActive && _backgroundFG) {
		uint32 bgFill = 0xFFFFFFFF; // B&W default
		if (useColor && _animBMColors.size() > 1) {
			if (_corePower[_coreIndex] > 0)
				bgFill = resolveAnimColor(_animBMColors[1]);
			else
				bgFill = resolveAnimColor(_animBMColors[0]); // unpowered: inherits top
		}
		drawAnimationImage(_backgroundFG, _backgroundMask,
		                   ox + _backgroundLocate.left, oy + _backgroundLocate.top,
		                   bgFill);
	}

	// Draw complex sprites
	for (uint i = 0; i < _lSprites.size(); i++) {
		if (_lSprites[i]->onoff)
			drawComplexSprite(i, ox, oy);
	}
}

void ColonyEngine::drawComplexSprite(int index, int ox, int oy) {
	ComplexSprite *ls = _lSprites[index];
	if (!ls->onoff)
		return;

	int cnum = ls->current;
	if (cnum < 0 || cnum >= (int)ls->objects.size())
		return;

	int spriteIdx = ls->objects[cnum].spritenum;
	if (spriteIdx < 0 || spriteIdx >= (int)_cSprites.size())
		return;

	Sprite *s = _cSprites[spriteIdx];
	int x = ox + ls->xloc + ls->objects[cnum].xloc + s->clip.left;
	int y = oy + ls->yloc + ls->objects[cnum].yloc + s->clip.top;

	// Resolve fill color from BMColor[index+2] (ganimate.c DrawlSprite).
	uint32 fillColor = 0xFFFFFFFF; // B&W default: white
	const bool useColor = (_hasMacColors && _renderMode == Common::kRenderMacintosh
	                       && !_animBMColors.empty());
	if (useColor) {
		int bmIdx = index + 2;
		if (bmIdx < (int)_animBMColors.size())
			fillColor = resolveAnimColor(_animBMColors[bmIdx]);
		else
			fillColor = resolveAnimColor(0); // fallback to level-based
	}

	drawAnimationImage(s->fg, s->mask, x, y, fillColor);
}

void ColonyEngine::drawAnimationImage(Image *img, Image *mask, int x, int y, uint32 fillColor) {
	if (!img || !img->data)
		return;

	const bool useColor = (_hasMacColors && _renderMode == Common::kRenderMacintosh);
	// Mac QuickDraw srcBic+srcOr rendering:
	//   mask bit=1 → opaque (part of sprite)
	//   fg bit=1   → ForeColor (black outline)
	//   fg bit=0   → BackColor (fillColor from BMColor)
	// B&W/DOS fallback preserves existing palette-index behavior.
	const uint32 fgColor = useColor ? (uint32)0xFF000000 : 15;
	const uint32 bgColor = useColor ? fillColor : 0;

	for (int iy = 0; iy < img->height; iy++) {
		for (int ix = 0; ix < img->width; ix++) {
			int byteIdx = iy * img->rowBytes + (ix / 8);
			int bitIdx = 7 - (ix % 8);

			bool maskSet = true;
			if (mask && mask->data) {
				int mByteIdx = iy * mask->rowBytes + (ix / 8);
				int mBitIdx = 7 - (ix % 8);
				maskSet = (mask->data[mByteIdx] & (1 << mBitIdx)) != 0;
			}

			if (!maskSet)
				continue;

			bool fgSet = (img->data[byteIdx] & (1 << bitIdx)) != 0;
			uint32 color = fgSet ? fgColor : bgColor;

			_gfx->setPixel(x + ix, y + iy, color);
		}
	}
}

Image *ColonyEngine::loadImage(Common::SeekableReadStream &file) {
	Image *im = new Image();
	if (getPlatform() == Common::kPlatformMacintosh) {
		readUint32(file); // baseAddr placeholder
		im->rowBytes = readSint16(file);
		Common::Rect r = readRect(file);
		im->width = r.width();
		im->height = r.height();
		im->align = 0;
		im->bits = 1;
		im->planes = 1;
	} else {
		im->width = readSint16(file);
		im->height = readSint16(file);
		im->align = readSint16(file);
		im->rowBytes = readSint16(file);
		im->bits = file.readByte();
		im->planes = file.readByte();
	}

	int16 tf = readSint16(file);
	uint32 size;
	if (tf) {
		// Mac original loadbitmap: reads bsize bytes into a buffer, then
		// decompresses from that buffer. We must read exactly bsize bytes
		// from the stream to keep file position aligned.
		uint32 bsize = readUint32(file);
		size = readUint32(file);
		im->data = (byte *)malloc(size);
		byte *packed = (byte *)calloc(bsize + 8, 1); // +8 matches original NewPtr(bsize+8)
		file.read(packed, bsize);
		// Decompress: exact match of Mac UnPackBytes(src, dst, len).
		// Buffer is pairs of (count, value). Count is decremented in-place;
		// when it reaches 0, advance to next pair.
		byte *sp = packed;
		for (uint32 di = 0; di < size; di++) {
			if (*sp) {
				im->data[di] = *(sp + 1);
				(*sp)--;
			} else {
				sp += 2;
				im->data[di] = *(sp + 1);
				(*sp)--;
			}
		}
		free(packed);
	} else {
		size = readUint32(file);
		im->data = (byte *)malloc(size);
		file.read(im->data, size);
	}
	return im;
}

void ColonyEngine::unpackBytes(Common::SeekableReadStream &file, byte *dst, uint32 len) {
	uint32 i = 0;
	while (i < len) {
		byte count = file.readByte();
		byte value = file.readByte();
		for (int j = 0; j < count && i < len; j++) {
			dst[i++] = value;
		}
	}
}

Common::Rect ColonyEngine::readRect(Common::SeekableReadStream &file) {
	int16 top, left, bottom, right;
	if (getPlatform() == Common::kPlatformMacintosh) {
		top = readSint16(file);
		left = readSint16(file);
		bottom = readSint16(file);
		right = readSint16(file);
	} else {
		left = readSint16(file);
		top = readSint16(file);
		right = readSint16(file);
		bottom = readSint16(file);
	}
	// Guard against invalid rects from animation data
	if (left > right || top > bottom)
		return Common::Rect();
	return Common::Rect(left, top, right, bottom);
}

int16 ColonyEngine::readSint16(Common::SeekableReadStream &s) {
	if (getPlatform() == Common::kPlatformMacintosh)
		return s.readSint16BE();
	return s.readSint16LE();
}

uint16 ColonyEngine::readUint16(Common::SeekableReadStream &s) {
	if (getPlatform() == Common::kPlatformMacintosh)
		return s.readUint16BE();
	return s.readUint16LE();
}

uint32 ColonyEngine::readUint32(Common::SeekableReadStream &s) {
	if (getPlatform() == Common::kPlatformMacintosh)
		return s.readUint32BE();
	return s.readUint32LE();
}

int ColonyEngine::whichSprite(const Common::Point &p) {
	int ox = _screenR.left + (_screenR.width() - 416) / 2;
	ox = (ox / 8) * 8;
	int oy = _screenR.top + (_screenR.height() - 264) / 2;
	Common::Point pt(p.x - ox, p.y - oy);

	debug(1, "Click at (%d, %d), relative (%d, %d)", p.x, p.y, pt.x, pt.y);

	for (int i = _lSprites.size() - 1; i >= 0; i--) {
		ComplexSprite *ls = _lSprites[i];
		if (!ls->onoff)
			continue;

		int cnum = ls->current;
		if (cnum < 0 || cnum >= (int)ls->objects.size())
			continue;

		int spriteIdx = ls->objects[cnum].spritenum;
		if (spriteIdx < 0 || spriteIdx >= (int)_cSprites.size())
			continue;

		Sprite *s = _cSprites[spriteIdx];
		int xloc = ls->xloc + ls->objects[cnum].xloc;
		int yloc = ls->yloc + ls->objects[cnum].yloc;

		Common::Rect r = s->clip;
		r.translate(xloc, yloc);

		if (!r.contains(pt))
			continue;

		// Pixel-perfect mask test (matches DOS WhichlSprite)
		Image *mask = s->mask;
		if (mask && mask->data) {
			int row = pt.y - r.top;
			int col = pt.x - r.left;
			int bitCol = (col + mask->align) * mask->bits;
			int maskIndex = row * mask->rowBytes + (bitCol / 8);
			int shift = bitCol % 8;

			if (maskIndex >= 0 && maskIndex < mask->rowBytes * mask->height) {
				byte maskByte = mask->data[maskIndex];
				if (mask->planes == 2)
					maskByte |= mask->data[mask->rowBytes * mask->height + maskIndex];
				maskByte = maskByte >> shift;
				if (!(maskByte & ((1 << mask->bits) - 1))) {
					debug(0, "  Sprite %d (type=%d frz=%d): bbox hit but mask transparent at row=%d col=%d bits=%d align=%d",
						i + 1, ls->type, ls->frozen, row, col, mask->bits, mask->align);
					continue; // Transparent pixel, skip this sprite
				}
			} else {
				debug(0, "  Sprite %d: mask index %d out of bounds (max %d)", i + 1, maskIndex, mask->rowBytes * mask->height);
			}
		} else {
			debug(0, "  Sprite %d: no mask data, using bbox", i + 1);
		}

		debug(0, "Sprite %d HIT. type=%d frozen=%d Frame %d, Sprite %d. Box: (%d,%d,%d,%d)",
			i + 1, ls->type, ls->frozen, cnum, spriteIdx, r.left, r.top, r.right, r.bottom);
		return i + 1;
	}

	// Dump accurately calculated bounds if debug is high enough
	if (gDebugLevel >= 2) {
		for (int i = 0; i < (int)_lSprites.size(); i++) {
			ComplexSprite *ls = _lSprites[i];
			if (ls->onoff) {
				int cnum = ls->current;
				if (cnum < 0 || cnum >= (int)ls->objects.size())
					continue;
				int spriteIdx = ls->objects[cnum].spritenum;
				if (spriteIdx < 0 || spriteIdx >= (int)_cSprites.size())
					continue;
				Sprite *s = _cSprites[spriteIdx];

				int xloc = ls->xloc + ls->objects[cnum].xloc;
				int yloc = ls->yloc + ls->objects[cnum].yloc;
				Common::Rect r = s->clip;
				r.translate(xloc, yloc);

				debug(2, "  Sprite %d: Frame=%d Box=(%d,%d,%d,%d)", i + 1,
					cnum, r.left, r.top, r.right, r.bottom);
			}
		}
	}

	return 0;
}

void ColonyEngine::handleAnimationClick(int item) {
	uint32 now = _system->getMillis();
	if (now - _lastClickTime < 250) {
		debug("Ignoring rapid click on item %d", item);
		return;
	}
	_lastClickTime = now;
	debug(0, "Animation click on item %d in %s", item, _animationName.c_str());

	if (item > 0) {
		dolSprite(item - 1);
	}

	if (_animationName == "desk") {
		if (item >= 2 && item <= 5) {
			int idx = item - 2;
			uint8 *decode = (_level == 1) ? _decode2 : _decode3;
			if (decode[idx] == 0) {
				decode[idx] = (uint8)(2 + (_randomSource.getRandomNumber(3)));
				_lSprites[item - 1]->current = decode[idx] - 1;
				drawAnimation();
				_gfx->copyToScreen();
			}
		} else if (item == 7) { // Letter
			if (_lSprites[6]->current > 0)
				doText(_action1, 0);
		} else if (item == 9) { // Clipboard
			doText(_action1, 0);
		} else if (item == 17) { // Screen
			doText(_action0, 0);
		} else if (item == 22) { // Book
			doText(_action1, 0);
		} else if (item == 24) { // Cigarette
			doText(55, 0);
			terminateGame(false);
		} else if (item == 25) { // Post-it
			doText(_action1, 0);
		}
	} else if (_animationName == "vanity") {
		debug(0, "Vanity item %d clicked. Sprite type=%d frozen=%d locked=%d current=%d onoff=%d key=%d lock=%d size=%d",
			item,
			(item > 0 && item <= (int)_lSprites.size()) ? _lSprites[item-1]->type : -1,
			(item > 0 && item <= (int)_lSprites.size()) ? _lSprites[item-1]->frozen : -1,
			(item > 0 && item <= (int)_lSprites.size()) ? _lSprites[item-1]->locked : -1,
			(item > 0 && item <= (int)_lSprites.size()) ? _lSprites[item-1]->current : -1,
			(item > 0 && item <= (int)_lSprites.size()) ? (int)_lSprites[item-1]->onoff : -1,
			(item > 0 && item <= (int)_lSprites.size()) ? _lSprites[item-1]->key : -1,
			(item > 0 && item <= (int)_lSprites.size()) ? _lSprites[item-1]->lock : -1,
			(item > 0 && item <= (int)_lSprites.size()) ? (int)_lSprites[item-1]->objects.size() : -1);
		if (item == 12) { // Coffee cup - spill animation
			if (!_doorOpen) { // reuse _doorOpen as "spilled" flag
				for (int i = 1; i < 6; i++) {
					setObjectState(12, i);
					drawAnimation();
					_gfx->copyToScreen();
					_system->delayMillis(50);
				}
				_doorOpen = true;
			}
		} else if (item == 13) { // Paper
			doText(_action0, 0);
		} else if (item == 14) { // Badge
			doText(80, 0);
		} else if (item == 4) { // Diary
			doText(_action0, 0);
		} else if (item == 7) { // Book
			doText(_action0, 0);
		} else {
			debug(0, "Vanity: unhandled item %d", item);
		}
	} else if (_animationName == "slides") {
		if (item == 2) { // Speaker
			doText(261 + _creature, 0);
		} else if (item == 5) { // Prev
			_creature--;
			if (_creature == 0)
				_creature = 8;
			setObjectState(1, _creature);
		} else if (item == 6) { // Next
			_creature++;
			if (_creature == 9)
				_creature = 1;
			setObjectState(1, _creature);
		}
	} else if (_animationName == "teleshow") {
		if (item == 2) { // Speaker
			doText(269 + _creature, 0);
		} else if (item == 5) { // Prev
			_creature--;
			if (_creature == 0)
				_creature = 7;
			setObjectState(1, _creature);
		} else if (item == 6) { // Next
			_creature++;
			if (_creature == 8)
				_creature = 1;
			setObjectState(1, _creature);
		}
	} else if (_animationName == "reactor" || _animationName == "security" || _animationName == "suit") {
		if (item >= 1 && item <= 10 && _animationName != "suit") {
			for (int i = 5; i >= 1; i--)
				_animDisplay[i] = _animDisplay[i - 1];
			_animDisplay[0] = (uint8)(item + 1);
			refreshAnimationDisplay();
			drawAnimation();
			_gfx->copyToScreen();
			// Don't return, let dolSprite animate the button
		} else if (item == 11 && _animationName != "suit") { // Clear
			for (int i = 0; i < 6; i++)
				_animDisplay[i] = 1;
			// Reset keypad buttons to unpressed state
			for (int i = 1; i <= 10; i++)
				setObjectState(i, 1);
			refreshAnimationDisplay();
			drawAnimation();
			_gfx->copyToScreen();
		} else if (item == 12 && _animationName != "suit") { // Enter
			uint8 testarray[6];
			if (_animationName == "reactor") {
				if (_level == 1)
					crypt(testarray, _decode2[3] - 2, _decode2[2] - 2, _decode2[1] - 2, _decode2[0] - 2);
				else
					crypt(testarray, _decode3[3] - 2, _decode3[2] - 2, _decode3[1] - 2, _decode3[0] - 2);

				bool match = true;
				for (int i = 0; i < 6; i++) {
					if (testarray[i] != _animDisplay[5 - i])
						match = false;
				}
				if (match) {
					if (_coreState[_coreIndex] == 0)
						_coreState[_coreIndex] = 1;
					else if (_coreState[_coreIndex] == 1)
						_coreState[_coreIndex] = 0;
					_gametest = true;
				}
				_animationRunning = false;
			} else if (_animationName == "security") { // security
				crypt(testarray, _decode1[0] - 2, _decode1[1] - 2, _decode1[2] - 2, _decode1[3] - 2);
				bool match = true;
				for (int i = 0; i < 6; i++) {
					if (testarray[i] != _animDisplay[5 - i])
						match = false;
				}
				if (match) {
					_unlocked = true;
					_gametest = true;
				}
				_animationRunning = false;
			}
		} else if (_animationName == "suit") {
			if (item == 1) { // Armor
				if (_armor == 3) {
					for (int i = 6; i >= 1; i--) {
						setObjectState(1, i);
						setObjectState(3, i / 2 + 1);
						drawAnimation();
						_gfx->copyToScreen();
						_system->delayMillis(30);
					}
					_armor = 0;
				} else {
					setObjectState(1, (_armor * 2 + 1) + 1); // intermediate/pressed
					drawAnimation();
					_gfx->copyToScreen();
					_system->delayMillis(50);
					_armor++;
				}
				setObjectState(1, _armor * 2 + 1); // target state
				setObjectState(3, _armor + 1); // display state
				drawAnimation();
				_gfx->copyToScreen();
				if (_armor == 3 && _weapons == 3)
					_corePower[_coreIndex] = 2;
			} else if (item == 2) { // Weapons
				if (_weapons == 3) {
					for (int i = 6; i >= 1; i--) {
						setObjectState(2, i);
						setObjectState(4, i / 2 + 1);
						drawAnimation();
						_gfx->copyToScreen();
						_system->delayMillis(30);
					}
					_weapons = 0;
				} else {
					setObjectState(2, (_weapons * 2 + 1) + 1); // intermediate/pressed
					drawAnimation();
					_gfx->copyToScreen();
					_system->delayMillis(50);
					_weapons++;
				}
				setObjectState(2, _weapons * 2 + 1);
				setObjectState(4, _weapons + 1);
				drawAnimation();
				_gfx->copyToScreen();
				if (_armor == 3 && _weapons == 3)
					_corePower[_coreIndex] = 2;
			}
		}
		if (_animationName == "reactor" || _animationName == "security") {
			if (item <= 12) {
				// setObjectState(item, 1); // Reset to ensure animation runs Off -> On - handled by dolSprite
				if (item > 10) // Clear/Enter should return to Off
					setObjectState(item, 1);
				drawAnimation();
				_gfx->copyToScreen();
			}
		}
	} else if (_animationName == "door" || _animationName == "bulkhead") {
		// DOS DoDoor: item==3 toggles door open/close, item==1 or (item==101 && door open) exits
		if (item == 3) {
			_sound->play(Sound::kDoor);
			if (_doorOpen) {
				for (int i = 3; i >= 1; i--) {
					_doorOpen = !_doorOpen;
					setObjectState(2, i);
					drawAnimation();
					_gfx->copyToScreen();
					_system->delayMillis(80);
				}
			} else {
				for (int i = 1; i < 4; i++) {
					_doorOpen = !_doorOpen;
					setObjectState(2, i);
					drawAnimation();
					_gfx->copyToScreen();
					_system->delayMillis(80);
				}
			}
		}
		if (item == 1 || (item == 101 && objectState(2) == 3)) {
			_animationResult = 1;
			_animationRunning = false;
		}
	} else if (_animationName == "airlock") {
		// DOS DoAirLock: item==1 toggles airlock if power on && unlocked
		// item==2 or (item==101 && airlock open) exits with pass-through
		if ((item == 2 || item == 101) && _doorOpen) {
			_animationResult = 1;
			_animationRunning = false;
		} else if (item == 1 && _corePower[_coreIndex] && _unlocked) {
			_sound->play(Sound::kAirlock);
			if (_doorOpen) {
				for (int i = 3; i >= 1; i--) {
					setObjectState(2, i);
					drawAnimation();
					_gfx->copyToScreen();
					_system->delayMillis(80);
				}
			} else {
				for (int i = 1; i < 4; i++) {
					setObjectState(2, i);
					drawAnimation();
					_gfx->copyToScreen();
					_system->delayMillis(80);
				}
			}
			_doorOpen = !_doorOpen;
		} else if (item == 101 && !_doorOpen) {
			// Exit without opening
			_animationRunning = false;
		}
	} else if (_animationName == "elev") {
		// DOS DoElevator: two phases
		// _doorOpen=false: Phase 1 (outside) - item==5 toggles doors
		// _doorOpen=true: Phase 2 (inside) - items 6-10 select floor
		// _animationResult tracks: 0=outside, 1=doors open, 2=inside
		if (_animationResult < 2) {
			// Phase 1: outside the elevator
			if (item == 5) {
				_sound->play(Sound::kElevator);
				if (!_doorOpen) {
					for (int i = 1; i < 4; i++) {
						setObjectState(3, i);
						setObjectState(4, i);
						drawAnimation();
						_gfx->copyToScreen();
						_system->delayMillis(80);
					}
					_doorOpen = true;
				} else {
					for (int i = 3; i >= 1; i--) {
						setObjectState(4, i);
						setObjectState(3, i);
						drawAnimation();
						_gfx->copyToScreen();
						_system->delayMillis(80);
					}
					_doorOpen = false;
				}
			} else if (item == 2 || (item == 101 && _doorOpen)) {
				// Enter the elevator (transition to phase 2)
				_animationResult = 2;
				setObjectOnOff(6, true);
				setObjectOnOff(7, true);
				setObjectOnOff(8, true);
				setObjectOnOff(9, true);
				setObjectOnOff(10, true);
				setObjectOnOff(2, false);
				setObjectOnOff(5, false);
				drawAnimation();
				_gfx->copyToScreen();
			} else if (item == 101 && !_doorOpen) {
				// Exit without entering
				_animationResult = 0;
				_animationRunning = false;
			}
		} else {
			// Phase 2: inside — floor selection
			if (item >= 6 && item <= 10) {
				int fl = item - 5;
				if (fl == _elevatorFloor) {
					setObjectState(item, 1); // already on this floor
				} else {
					_sound->play(Sound::kElevator);
					for (int i = 3; i >= 1; i--) {
						setObjectState(4, i);
						setObjectState(3, i);
						drawAnimation();
						_gfx->copyToScreen();
						_system->delayMillis(80);
					}
					_elevatorFloor = fl;
					for (int i = 1; i <= 3; i++) {
						setObjectState(4, i);
						setObjectState(3, i);
						drawAnimation();
						_gfx->copyToScreen();
						_system->delayMillis(80);
					}
					setObjectState(item, 1);
				}
			} else if (item == 1 || item == 101) {
				// Exit elevator
				_animationRunning = false;
			}
		}
	} else if (_animationName == "controls") {
		switch (item) {
		case 4: // Accelerator
			if (_corePower[_coreIndex] >= 2 && _coreState[_coreIndex] == 0 && !_orbit) {
				_orbit = 1;
				debug(0, "Taking off!");
				// Animate the lever moving full range
				for (int i = 1; i <= 6; i++) {
					setObjectState(4, i);
					drawAnimation();
					_gfx->copyToScreen();
					_system->delayMillis(30);
				}
				_animationRunning = false;
				return; // Exit animation immediately on success
			} else {
				debug(0, "Accelerator failed: power=%d, state=%d", _corePower[_coreIndex], _coreState[_coreIndex]);
				// Fail animation click
				setObjectState(4, 1);
				// dolSprite(3); // Animate lever moving and returning - handled by top dolSprite
				for (int i = 6; i > 0; i--) {
					setObjectState(4, i);
					drawAnimation();
					_gfx->copyToScreen();
					_system->delayMillis(20);
				}
			}
			break;
		case 5: // Emergency power
			// setObjectState(5, 1); // Reset to ensure animation runs Off -> On - handled by dolSprite
			// dolSprite(4); // Animate the button press - handled by top dolSprite
			if (_coreState[_coreIndex] < 2) {
				if (_corePower[_coreIndex] == 0)
					_corePower[_coreIndex] = 1;
				else if (_corePower[_coreIndex] == 1)
					_corePower[_coreIndex] = 0;
			}
			// Finalize visual state according to power settings
			switch (_corePower[_coreIndex]) {
			case 0: setObjectState(2, 1); setObjectState(5, 1); break;
			case 1: setObjectState(2, 1); setObjectState(5, 2); break;
			case 2: setObjectState(2, 2); setObjectState(5, 1); break;
			}
			drawAnimation();
			_gfx->copyToScreen();
			break;
		case 7: // Damage report
		{
			// dolSprite(6); // Button animation - handled by top dolSprite
			if (_corePower[_coreIndex] < 2) {
				doText(15, 0); // Critical status
			} else if (!_orbit) {
				doText(49, 0); // Ready for liftoff
			} else {
				doText(66, 0); // Orbital stabilization
			}
			
			setObjectState(7, 1); // Reset button
			drawAnimation();
			_gfx->copyToScreen();
			break;
		}
			break;
		}
	}
}

void ColonyEngine::terminateGame(bool blowup) {
	debug(0, "YOU HAVE BEEN TERMINATED! (blowup=%d)", blowup);
	if (blowup)
		_sound->play(Sound::kExplode);
	
	const char *msg[] = {
		"   YOU HAVE BEEN TERMINATED!   ",
		" Type 'q' to quit the game.    ",
		nullptr
	};
	printMessage(msg, true);
	
	_system->quit();
}

void ColonyEngine::moveObject(int index) {
	if (index < 0 || index >= (int)_lSprites.size())
		return;

	ComplexSprite *ls = _lSprites[index];

	// Build link group
	Common::Array<int> linked;
	if (ls->link) {
		for (int i = 0; i < (int)_lSprites.size(); i++)
			if (_lSprites[i]->link == ls->link)
				linked.push_back(i);
	} else {
		linked.push_back(index);
	}

	// Get initial mouse position and animation origin
	Common::Point old = _system->getEventManager()->getMousePos();
	int ox = _screenR.left + (_screenR.width() - 416) / 2;
	ox = (ox / 8) * 8;
	int oy = _screenR.top + (_screenR.height() - 264) / 2;

	// Drag loop: track mouse while left button held.
	// NOTE: The original DOS hides dragged sprites during drag (setObjectOnOff FALSE)
	// and redraws them separately on top. We improve on this by keeping them visible
	// throughout, and drawing an extra copy on top so they render above drawers.
	while (!shouldQuit()) {
		Common::Event event;
		bool buttonDown = true;
		while (_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_LBUTTONUP) {
				buttonDown = false;
				break;
			}
		}
		if (!buttonDown)
			break;

		Common::Point cur = _system->getEventManager()->getMousePos();
		int dx = cur.x - old.x;
		int dy = cur.y - old.y;

		if (dx != 0 || dy != 0) {
			// Cycle frame for non-type-2 sprites
			if (ls->type != 2 && (int)ls->objects.size() > 1) {
				ls->current++;
				if (ls->current >= (int)ls->objects.size())
					ls->current = 0;
			}

			// Move all linked sprites
			for (uint i = 0; i < linked.size(); i++) {
				_lSprites[linked[i]]->xloc += dx;
				_lSprites[linked[i]]->yloc += dy;
			}

			old = cur;
		}

		// Draw all sprites normally, then draw dragged sprites again on top
		// so they appear above drawers and other overlapping sprites
		drawAnimation();
		for (uint i = 0; i < linked.size(); i++)
			drawComplexSprite(linked[i], ox, oy);
		_gfx->copyToScreen();

		_system->delayMillis(20);
	}

	// Reset frame for non-type-2
	if (ls->type != 2)
		ls->current = 0;

	drawAnimation();
	_gfx->copyToScreen();
}

void ColonyEngine::dolSprite(int index) {
	if (index < 0 || index >= (int)_lSprites.size())
		return;

	ComplexSprite *ls = _lSprites[index];
	int maxFrames = (int)ls->objects.size();

	switch (ls->type) {
	case 0: // Display
		if (!ls->frozen)
			moveObject(index);
		break;
	case 1: // Key and control
		if (ls->frozen) {
			// Container: can open or close if not locked, or if slightly open
			if (!ls->locked || ls->current) {
				if (ls->current > 1) {
					// Close: animate from current down to 0
					while (ls->current > 0) {
						ls->current--;
						// Sync linked sprites via key/lock
						if (ls->key) {
							for (int i = 0; i < (int)_lSprites.size(); i++) {
								if (i != index && _lSprites[i]->lock == ls->key) {
									_lSprites[i]->current = ls->current;
									if (_lSprites[i]->current >= (int)_lSprites[i]->objects.size())
										_lSprites[i]->current = (int)_lSprites[i]->objects.size() - 1;
								}
							}
						}
						drawAnimation();
						_gfx->copyToScreen();
						_system->delayMillis(50);
					}
				} else {
					// Open: animate from current up to max
					while (ls->current < maxFrames - 1) {
						ls->current++;
						// Sync linked sprites via key/lock
						if (ls->key) {
							for (int i = 0; i < (int)_lSprites.size(); i++) {
								if (i != index && _lSprites[i]->lock == ls->key) {
									_lSprites[i]->current = ls->current;
									if (_lSprites[i]->current >= (int)_lSprites[i]->objects.size())
										_lSprites[i]->current = (int)_lSprites[i]->objects.size() - 1;
								}
							}
						}
						drawAnimation();
						_gfx->copyToScreen();
						_system->delayMillis(50);
					}
				}
			}
		} else {
			moveObject(index);
		}
		break;
	case 2: // Container and object
		if (ls->frozen) {
			if (!ls->locked) {
				// Unlocked container: toggle open/close
				if (ls->current > 0) {
					while (ls->current > 0) {
						ls->current--;
						drawAnimation();
						_gfx->copyToScreen();
						_system->delayMillis(50);
					}
				} else {
					while (ls->current < maxFrames - 1) {
						ls->current++;
						drawAnimation();
						_gfx->copyToScreen();
						_system->delayMillis(50);
					}
				}
			} else {
				// Locked container: current>1 closes, current==1 opens further
				if (ls->current > 1) {
					while (ls->current > 0) {
						ls->current--;
						drawAnimation();
						_gfx->copyToScreen();
						_system->delayMillis(50);
					}
				} else if (ls->current == 1) {
					while (ls->current < maxFrames - 1) {
						ls->current++;
						drawAnimation();
						_gfx->copyToScreen();
						_system->delayMillis(50);
					}
				}
			}
		} else {
			moveObject(index);
		}
		break;
	default:
		break;
	}
}

void ColonyEngine::setObjectState(int num, int state) {
	num--;
	if (num >= 0 && num < (int)_lSprites.size())
		_lSprites[num]->current = state - 1;
}

int ColonyEngine::objectState(int num) const {
	num--;
	if (num >= 0 && num < (int)_lSprites.size())
		return _lSprites[num]->current + 1;
	return 0;
}

void ColonyEngine::setObjectOnOff(int num, bool on) {
	num--;
	if (num >= 0 && num < (int)_lSprites.size())
		_lSprites[num]->onoff = on;
}

void ColonyEngine::refreshAnimationDisplay() {
	for (int i = 0; i < 6; i++) {
		if (_animDisplay[i] < 9) {
			setObjectOnOff(13 + i * 2, true);
			setObjectOnOff(14 + i * 2, false);
			setObjectState(13 + i * 2, _animDisplay[i]);
		} else {
			setObjectOnOff(14 + i * 2, true);
			setObjectOnOff(13 + i * 2, false);
			setObjectState(14 + i * 2, _animDisplay[i] - 8);
		}
	}
}

void ColonyEngine::crypt(uint8 sarray[6], int i, int j, int k, int l) {
	int res[6];
	res[0] = ((3 * l) ^ i ^ j ^ k) % 10;
	res[1] = ((i * 3) ^ (j * 7) ^ (k * 11) ^ (l * 13)) % 10;
	res[2] = (3 + (l * 17) ^ (j * 19) ^ (k * 23) ^ (i * 29)) % 10;
	res[3] = ((l * 19) ^ (j * 23) ^ (k * 29) ^ (i * 31)) % 10;
	res[4] = ((l * 17) | (j * 19) | (k * 23) | (i * 29)) % 10;
	res[5] = (29 + (l * 17) - (j * 19) - (k * 23) - (i * 29)) % 10;
	for (int m = 0; m < 6; m++) {
		if (res[m] < 0)
			res[m] = -res[m];
		sarray[m] = (uint8)(res[m] + 2);
	}
}

} // End of namespace Colony
