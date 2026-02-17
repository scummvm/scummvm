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
#include "graphics/cursorman.h"
#include <math.h>

namespace Colony {



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
	_showDashBoard = true;
	_crosshair = true;
	_insight = false;
	_hasKeycard = false;
	_unlocked = false;
	_weapons = 0;
	_wireframe = false;
	_widescreen = ConfMan.getBool("widescreen_mod");

	// Render mode: EGA (DOS wireframe default) or Macintosh (filled polygons)
	if (!ConfMan.hasKey("render_mode") || ConfMan.get("render_mode").empty())
		_renderMode = Common::kRenderEGA;
	else
		_renderMode = Common::parseRenderMode(ConfMan.get("render_mode"));
	if (_renderMode == Common::kRenderDefault)
		_renderMode = Common::kRenderEGA;
	_speedShift = 2; // DOS default: speedshift=1, but 2 feels better with our frame rate
	
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
	_me.type = MENUM;

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
	for (int i = 0; i < 6; i++) _animDisplay[i] = 1;
	for (int i = 0; i < 2; i++) _coreState[i] = _coreHeight[i] = 0;
	for (int i = 0; i < 3; i++) _corePower[i] = 0;
	_coreIndex = 0;
	_orbit = 0;
	_armor = 0;
	_gametest = false;
	_blackoutColor = 15; // Set to white (vINTWHITE) for better visibility in darkness

	_sound = new Sound(this);
	initTrig();
}


ColonyEngine::~ColonyEngine() {
	deleteAnimation();
	delete _frameLimiter;
	delete _gfx;
	delete _sound;
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

	getWall();  // restore saved wall state changes (airlocks)
	doPatch();  // apply object relocations from patch table

	if (_me.xindex >= 0 && _me.xindex < 32 && _me.yindex >= 0 && _me.yindex < 32)
		_robotArray[_me.xindex][_me.yindex] = MENUM;
	debug("Successfully loaded map %d (static objects: %d)", mnum, (int)_objects.size());
}

// PATCH.C: Create a new object in _objects and register in _robotArray.
// Mirrors DOS CreateObject() — sets basic Thing fields for static objects.
void ColonyEngine::createObject(int type, int xloc, int yloc, uint8 ang) {
	Thing obj;
	memset(&obj, 0, sizeof(obj));
	while (ang > 255) ang -= 256;
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

	// Try to reuse a dead slot (starting after MENUM)
	int slot = -1;
	for (int j = MENUM; j < (int)_objects.size(); j++) {
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
	if (_level < 1 || _level > 8) return;
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
	if (_level < 1 || _level > 8) return;
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
	if (_patches.size() >= 100) return;
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

void ColonyEngine::initTrig() {
	// Compute standard sin/cos lookup tables (256 steps = full circle, scaled by 128)
	for (int i = 0; i < 256; i++) {
		float rad = (float)i * 2.0f * M_PI / 256.0f;
		_sint[i] = (int)roundf(128.0f * sinf(rad));
		_cost[i] = (int)roundf(128.0f * cosf(rad));
	}
}

Common::Error ColonyEngine::run() {
	_width = 640;
	_height = 350;

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
	static const byte ega_colors[16][3] = {
		{0, 0, 0}, {0, 0, 170}, {0, 170, 0}, {0, 170, 170},
		{170, 0, 0}, {170, 0, 170}, {170, 85, 0}, {170, 170, 170},
		{85, 85, 85}, {85, 85, 255}, {85, 255, 85}, {85, 255, 255},
		{255, 85, 85}, {255, 85, 255}, {255, 255, 85}, {255, 255, 255}
	};
	for (int i = 0; i < 16; i++) {
		pal[i * 3 + 0] = ega_colors[i][0];
		pal[i * 3 + 1] = ega_colors[i][1];
		pal[i * 3 + 2] = ega_colors[i][2];
	}
	for (int i = 16; i < 256; i++) {
		pal[i * 3 + 0] = i;
		pal[i * 3 + 1] = i;
		pal[i * 3 + 2] = i;
	}
	_gfx->setPalette(pal, 0, 256);

	// Frame limiter: target 60fps, like Freescape engine
	_frameLimiter = new Graphics::FrameLimiter(_system, 60);

	scrollInfo();

	loadMap(1); // Try to load the first map
	_system->lockMouse(true);
	_system->warpMouse(_centerX, _centerY);

	int mouseDX = 0, mouseDY = 0;
	bool mouseMoved = false;
	while (!shouldQuit()) {
		_frameLimiter->startFrame();
		Common::Event event;
		while (_system->getEventManager()->pollEvent(event)) {
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
						_system->lockMouse(true);
						_system->warpMouse(_centerX, _centerY);
						_system->getEventManager()->purgeMouseEvents();
						break;
					// Space: shoot or exit forklift (DOS: ScanCode 57)
					case Common::KEYCODE_SPACE:
						if (_fl == 2)
							dropCarriedObject();
						else if (_fl == 1)
							exitForklift();
						// else: shoot (TODO: implement shooting)
						break;
					// Escape: also opens ScummVM menu
					case Common::KEYCODE_ESCAPE:
						_system->lockMouse(false);
						openMainMenuDialog();
						_gfx->computeScreenViewport();
						_system->lockMouse(true);
						_system->warpMouse(_centerX, _centerY);
						_system->getEventManager()->purgeMouseEvents();
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

		if (mouseMoved) {
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
		
		_displayCount++; // Mac: count++ after Display()
		_frameLimiter->delayBeforeSwap();
		_gfx->copyToScreen();
	}

	return Common::kNoError;
}

void ColonyEngine::scrollInfo() {
	const char *story[] = {
		"Mankind has left the",
		"cradle of earth and",
		"is beginning to eye",
		"the galaxy. He has",
		"begun to colonize",
		"distant planets but has",
		"yet to meet any alien",
		"life forms.",
		"****",
		"Until now...",
		"****",
		"Press any key to begin",
		"the Adventure..."
	};
	const int storyLength = ARRAYSIZE(story);

	_gfx->clear(_gfx->black());
	Graphics::DosFont dosFont;
	const Graphics::Font *font = &dosFont;

	int centerY = _height / 2;
	centerY -= (storyLength * 10) / 2;
	centerY += 5;

	for (int i = 0; i < storyLength; i++) {
		_gfx->drawString(font, story[i], _width / 2, centerY + 10 * i, 9, Graphics::kTextAlignCenter);
	}
	_gfx->copyToScreen();

	bool waiting = true;
	while (waiting && !shouldQuit()) {
		Common::Event event;
		while (_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_LBUTTONDOWN)
				waiting = false;
		}
		_system->delayMillis(10);
	}

	_gfx->fillRect(_screenR, 0);
	_gfx->copyToScreen();
}

bool ColonyEngine::loadAnimation(const Common::String &name) {
	_animationName = name;
	for (int i = 0; i < 6; i++) _animDisplay[i] = 1;
	Common::String fileName = name + ".pic";
	Common::File file;
	if (!file.open(Common::Path(fileName))) {
		warning("Could not open animation file %s", fileName.c_str());
		return false;
	}

	deleteAnimation();

	// Read background data
	file.read(_topBG, 8);
	file.read(_bottomBG, 8);
	_divideBG = file.readSint16LE();
	_backgroundActive = file.readSint16LE() != 0;
	if (_backgroundActive) {
		_backgroundClip = readRect(file);
		_backgroundLocate = readRect(file);
		_backgroundMask = loadImage(file);
		_backgroundFG = loadImage(file);
	}

	// Read sprite data
	int16 maxsprite = file.readSint16LE();
	file.readSint16LE(); // locSprite
	for (int i = 0; i < maxsprite; i++) {
		Sprite *s = new Sprite();
		s->fg = loadImage(file);
		s->mask = loadImage(file);
		s->used = file.readSint16LE() != 0;
		s->clip = readRect(file);
		s->locate = readRect(file);
		_cSprites.push_back(s);
	}

	// Read complex sprite data
	int16 maxLSprite = file.readSint16LE();
	file.readSint16LE(); // anum
	for (int i = 0; i < maxLSprite; i++) {
		ComplexSprite *ls = new ComplexSprite();
		int16 size = file.readSint16LE();
		for (int j = 0; j < size; j++) {
			ComplexSprite::SubObject sub;
			sub.spritenum = file.readSint16LE();
			sub.xloc = file.readSint16LE();
			sub.yloc = file.readSint16LE();
			ls->objects.push_back(sub);
		}
		ls->bounds = readRect(file);
		ls->visible = file.readSint16LE() != 0;
		ls->current = file.readSint16LE();
		ls->xloc = file.readSint16LE();
		ls->yloc = file.readSint16LE();
		ls->acurrent = file.readSint16LE();
		ls->axloc = file.readSint16LE();
		ls->ayloc = file.readSint16LE();
		ls->type = file.readByte();
		ls->frozen = file.readByte();
		ls->locked = file.readByte();
		ls->link = file.readSint16LE();
		ls->key = file.readSint16LE();
		ls->lock = file.readSint16LE();
		ls->onoff = true;
		_lSprites.push_back(ls);
	}

	return true;
}

void ColonyEngine::deleteAnimation() {
	delete _backgroundMask; _backgroundMask = nullptr;
	delete _backgroundFG; _backgroundFG = nullptr;
	for (uint i = 0; i < _cSprites.size(); i++) delete _cSprites[i];
	_cSprites.clear();
	for (uint i = 0; i < _lSprites.size(); i++) delete _lSprites[i];
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
			SetObjectState(27 + i, _decode1[i]);
		}
	} else if (_animationName == "reactor") {
		for (int i = 0; i < 6; i++) {
			SetObjectOnOff(14 + i * 2, false);
			SetObjectState(13 + i * 2, 1);
		}
	} else if (_animationName == "controls") {
		switch (_corePower[_coreIndex]) {
		case 0: SetObjectState(2, 1); SetObjectState(5, 1); break;
		case 1: SetObjectState(2, 1); SetObjectState(5, 2); break;
		case 2: SetObjectState(2, 2); SetObjectState(5, 1); break;
		}
	} else if (_animationName == "desk") {
		if (!(_action0 == 11 || _action0 == 18)) {
			for (int i = 1; i <= 5; i++) SetObjectOnOff(i, false);
		} else {
			uint8 *decode = (_action0 == 11) ? _decode2 : _decode3;
			for (int i = 0; i < 4; i++) {
				if (decode[i])
					SetObjectState(i + 2, decode[i]);
				else
					SetObjectState(i + 2, 1);
			}
		}

		if (_action0 != 10) {
			SetObjectOnOff(23, false);
			SetObjectOnOff(24, false);
		}
		if (_action0 != 30) SetObjectOnOff(6, false); // Teeth
		if (_action0 != 33) { // Jack-in-the-box
			for (int i = 18; i <= 21; i++) SetObjectOnOff(i, false);
		}

		int ntype = _action1 / 10;
		switch (ntype) {
		case 0:
		case 1:
		case 2:
		case 3:
			SetObjectOnOff(7, false);
			SetObjectOnOff(8, false);
			SetObjectOnOff(9, false);
			SetObjectOnOff(22, false);
			SetObjectOnOff(25, false);
			break;
		case 4: // letters
			SetObjectOnOff(22, false);
			SetObjectOnOff(9, false);
			SetObjectOnOff(25, false);
			break;
		case 5: // book
			SetObjectOnOff(7, false);
			SetObjectOnOff(8, false);
			SetObjectOnOff(9, false);
			SetObjectOnOff(25, false);
			break;
		case 6: // clipboard
			SetObjectOnOff(22, false);
			SetObjectOnOff(7, false);
			SetObjectOnOff(8, false);
			SetObjectOnOff(25, false);
			break;
		case 7: // postit
			SetObjectOnOff(22, false);
			SetObjectOnOff(7, false);
			SetObjectOnOff(8, false);
			SetObjectOnOff(9, false);
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
			SetObjectState(1, 3);
		else if (_weapons)
			SetObjectState(1, 2);
		else if (_armor)
			SetObjectState(1, 1);
		else
			SetObjectState(1, 4);
		// Badge only visible on level 1
		if (_level != 1)
			SetObjectOnOff(14, false);
		// Hide items based on action0 (num parameter in DOS)
		if (_action0 < 90) { // coffee cup only
			SetObjectOnOff(4, false);
			SetObjectOnOff(7, false);
			SetObjectOnOff(13, false);
		} else if (_action0 < 100) { // paper
			SetObjectOnOff(12, false);
			SetObjectOnOff(4, false);
			SetObjectOnOff(7, false);
		} else if (_action0 < 110) { // diary
			SetObjectOnOff(12, false);
			SetObjectOnOff(13, false);
			SetObjectOnOff(7, false);
		} else if (_action0 < 120) { // book
			SetObjectOnOff(12, false);
			SetObjectOnOff(13, false);
			SetObjectOnOff(4, false);
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
	static uint32 lastUpdate = 0;
	uint32 now = _system->getMillis();
	if (now - lastUpdate < 50) // Reduced to 50ms (20 fps) to make it "move"
		return;
	lastUpdate = now;

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

void ColonyEngine::drawAnimation() {
	_gfx->clear(0);

	// Center 416x264 animation area on screen (from original InitDejaVu)
	int ox = _screenR.left + (_screenR.width() - 416) / 2;
	ox = (ox / 8) * 8;
	int oy = _screenR.top + (_screenR.height() - 264) / 2;

	// Fill background patterns (416x264 area)
	for (int y = 0; y < 264; y++) {
		byte *pat = (y < _divideBG) ? _topBG : _bottomBG;
		byte row = pat[y % 8];
		for (int x = 0; x < 416; x++) {
			bool set = (row & (0x80 >> (x % 8))) != 0;
			// Pattern bit: 1->Black(0), 0->White(15) based on original inversion
			// Actually Invert in readanim: ~data.
			// Let's assume set means "white" (15) and unset "black" (0) or vice versa.
			// In original: BackColor(Black). Pattern 1s draw ForeColor. 0s draw BackColor.
			// If we want "not black", we likely want some white pixels.
			// Let's try: set -> 15 (White), !set -> 0 (Black).
			_gfx->setPixel(ox + x, oy + y, set ? 15 : 0);
		}
	}

	// Draw background if active
	if (_backgroundActive && _backgroundFG) {
		drawAnimationImage(_backgroundFG, _backgroundMask, ox + _backgroundLocate.left, oy + _backgroundLocate.top);
	}

	// Draw complex sprites
	for (uint i = 0; i < _lSprites.size(); i++) {
		if (_lSprites[i]->onoff)
			drawComplexSprite(i, ox, oy);
	}
}

void ColonyEngine::drawComplexSprite(int index, int ox, int oy) {
	ComplexSprite *ls = _lSprites[index];
	if (!ls->onoff) return;

	int cnum = ls->current;
	if (cnum < 0 || cnum >= (int)ls->objects.size()) return;

	int spriteIdx = ls->objects[cnum].spritenum;
	if (spriteIdx < 0 || spriteIdx >= (int)_cSprites.size()) return;

	Sprite *s = _cSprites[spriteIdx];
	int x = ox + ls->xloc + ls->objects[cnum].xloc + s->clip.left;
	int y = oy + ls->yloc + ls->objects[cnum].yloc + s->clip.top;

	drawAnimationImage(s->fg, s->mask, x, y);
}

void ColonyEngine::drawAnimationImage(Image *img, Image *mask, int x, int y) {
	if (!img || !img->data) return;

	for (int iy = 0; iy < img->height; iy++) {
		for (int ix = 0; ix < img->width; ix++) {
			int byteIdx = iy * img->rowBytes + (ix / 8);
			int bitIdx = 7 - (ix % 8);
			
			bool maskSet = true;
			if (mask && mask->data) {
				maskSet = (mask->data[byteIdx] & (1 << bitIdx)) != 0;
			}

			if (!maskSet) continue;

			bool fgSet = (img->data[byteIdx] & (1 << bitIdx)) != 0;
			uint32 color = fgSet ? 15 : 0;

			_gfx->setPixel(x + ix, y + iy, color);
		}
	}
}

Image *ColonyEngine::loadImage(Common::SeekableReadStream &file) {
	Image *im = new Image();
	im->width = file.readSint16LE();
	im->height = file.readSint16LE();
	im->align = file.readSint16LE();
	im->rowBytes = file.readSint16LE();
	im->bits = file.readByte();
	im->planes = file.readByte();

	int16 tf = file.readSint16LE();
	uint32 size;
	if (tf) {
		/* uint32 bsize = */ file.readUint32LE();
		size = file.readUint32LE();
		im->data = (byte *)malloc(size);
		unpackBytes(file, im->data, size);
	} else {
		size = file.readUint32LE();
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
	int16 left = file.readSint16LE();
	int16 top = file.readSint16LE();
	int16 right = file.readSint16LE();
	int16 bottom = file.readSint16LE();
	return Common::Rect(left, top, right, bottom);
}

int ColonyEngine::whichSprite(const Common::Point &p) {
	int ox = _screenR.left + (_screenR.width() - 416) / 2;
	ox = (ox / 8) * 8;
	int oy = _screenR.top + (_screenR.height() - 264) / 2;
	Common::Point pt(p.x - ox, p.y - oy);

	debug(1, "Click at (%d, %d), relative (%d, %d)", p.x, p.y, pt.x, pt.y);

	for (int i = _lSprites.size() - 1; i >= 0; i--) {
		ComplexSprite *ls = _lSprites[i];
		if (!ls->onoff) continue;

		int cnum = ls->current;
		if (cnum < 0 || cnum >= (int)ls->objects.size()) continue;

		int spriteIdx = ls->objects[cnum].spritenum;
		if (spriteIdx < 0 || spriteIdx >= (int)_cSprites.size()) continue;

		Sprite *s = _cSprites[spriteIdx];
		int xloc = ls->xloc + ls->objects[cnum].xloc;
		int yloc = ls->yloc + ls->objects[cnum].yloc;

		Common::Rect r = s->clip;
		r.translate(xloc, yloc);

		if (!r.contains(pt)) continue;

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
				if (cnum < 0 || cnum >= (int)ls->objects.size()) continue;
				int spriteIdx = ls->objects[cnum].spritenum;
				if (spriteIdx < 0 || spriteIdx >= (int)_cSprites.size()) continue;
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
					SetObjectState(12, i);
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
			if (_creature == 0) _creature = 8;
			SetObjectState(1, _creature);
		} else if (item == 6) { // Next
			_creature++;
			if (_creature == 9) _creature = 1;
			SetObjectState(1, _creature);
		}
	} else if (_animationName == "teleshow") {
		if (item == 2) { // Speaker
			doText(269 + _creature, 0);
		} else if (item == 5) { // Prev
			_creature--;
			if (_creature == 0) _creature = 7;
			SetObjectState(1, _creature);
		} else if (item == 6) { // Next
			_creature++;
			if (_creature == 8) _creature = 1;
			SetObjectState(1, _creature);
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
				SetObjectState(i, 1);
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
						SetObjectState(1, i);
						SetObjectState(3, i / 2 + 1);
						drawAnimation(); _gfx->copyToScreen(); _system->delayMillis(30);
					}
					_armor = 0;
				} else {
					SetObjectState(1, (_armor * 2 + 1) + 1); // intermediate/pressed
					drawAnimation(); _gfx->copyToScreen(); _system->delayMillis(50);
					_armor++;
				}
				SetObjectState(1, _armor * 2 + 1); // target state
				SetObjectState(3, _armor + 1); // display state
				drawAnimation(); _gfx->copyToScreen();
				if (_armor == 3 && _weapons == 3) _corePower[_coreIndex] = 2;
			} else if (item == 2) { // Weapons
				if (_weapons == 3) {
					for (int i = 6; i >= 1; i--) {
						SetObjectState(2, i);
						SetObjectState(4, i / 2 + 1);
						drawAnimation(); _gfx->copyToScreen(); _system->delayMillis(30);
					}
					_weapons = 0;
				} else {
					SetObjectState(2, (_weapons * 2 + 1) + 1); // intermediate/pressed
					drawAnimation(); _gfx->copyToScreen(); _system->delayMillis(50);
					_weapons++;
				}
				SetObjectState(2, _weapons * 2 + 1);
				SetObjectState(4, _weapons + 1);
				drawAnimation(); _gfx->copyToScreen();
				if (_armor == 3 && _weapons == 3) _corePower[_coreIndex] = 2;
			}
		}
		if (_animationName == "reactor" || _animationName == "security") {
			if (item <= 12) {
				// SetObjectState(item, 1); // Reset to ensure animation runs Off -> On - handled by dolSprite
				if (item > 10) // Clear/Enter should return to Off
					SetObjectState(item, 1);
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
					SetObjectState(2, i);
					drawAnimation(); _gfx->copyToScreen(); _system->delayMillis(80);
				}
			} else {
				for (int i = 1; i < 4; i++) {
					_doorOpen = !_doorOpen;
					SetObjectState(2, i);
					drawAnimation(); _gfx->copyToScreen(); _system->delayMillis(80);
				}
			}
		}
		if (item == 1 || (item == 101 && ObjectState(2) == 3)) {
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
					SetObjectState(2, i);
					drawAnimation(); _gfx->copyToScreen(); _system->delayMillis(80);
				}
			} else {
				for (int i = 1; i < 4; i++) {
					SetObjectState(2, i);
					drawAnimation(); _gfx->copyToScreen(); _system->delayMillis(80);
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
						SetObjectState(3, i);
						SetObjectState(4, i);
						drawAnimation(); _gfx->copyToScreen(); _system->delayMillis(80);
					}
					_doorOpen = true;
				} else {
					for (int i = 3; i >= 1; i--) {
						SetObjectState(4, i);
						SetObjectState(3, i);
						drawAnimation(); _gfx->copyToScreen(); _system->delayMillis(80);
					}
					_doorOpen = false;
				}
			} else if (item == 2 || (item == 101 && _doorOpen)) {
				// Enter the elevator (transition to phase 2)
				_animationResult = 2;
				SetObjectOnOff(6, true);
				SetObjectOnOff(7, true);
				SetObjectOnOff(8, true);
				SetObjectOnOff(9, true);
				SetObjectOnOff(10, true);
				SetObjectOnOff(2, false);
				SetObjectOnOff(5, false);
				drawAnimation(); _gfx->copyToScreen();
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
					SetObjectState(item, 1); // already on this floor
				} else {
					_sound->play(Sound::kElevator);
					for (int i = 3; i >= 1; i--) {
						SetObjectState(4, i);
						SetObjectState(3, i);
						drawAnimation(); _gfx->copyToScreen(); _system->delayMillis(80);
					}
					_elevatorFloor = fl;
					for (int i = 1; i <= 3; i++) {
						SetObjectState(4, i);
						SetObjectState(3, i);
						drawAnimation(); _gfx->copyToScreen(); _system->delayMillis(80);
					}
					SetObjectState(item, 1);
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
					SetObjectState(4, i);
					drawAnimation(); _gfx->copyToScreen(); _system->delayMillis(30);
				}
				_animationRunning = false;
				return; // Exit animation immediately on success
			} else {
				debug(0, "Accelerator failed: power=%d, state=%d", _corePower[_coreIndex], _coreState[_coreIndex]);
				// Fail animation click
				SetObjectState(4, 1);
				// dolSprite(3); // Animate lever moving and returning - handled by top dolSprite
				for (int i = 6; i > 0; i--) {
					SetObjectState(4, i);
					drawAnimation(); _gfx->copyToScreen(); _system->delayMillis(20);
				}
			}
			break;
		case 5: // Emergency power
			// SetObjectState(5, 1); // Reset to ensure animation runs Off -> On - handled by dolSprite
			// dolSprite(4); // Animate the button press - handled by top dolSprite
			if (_coreState[_coreIndex] < 2) {
				if (_corePower[_coreIndex] == 0)
					_corePower[_coreIndex] = 1;
				else if (_corePower[_coreIndex] == 1)
					_corePower[_coreIndex] = 0;
			}
			// Finalize visual state according to power settings
			switch (_corePower[_coreIndex]) {
			case 0: SetObjectState(2, 1); SetObjectState(5, 1); break;
			case 1: SetObjectState(2, 1); SetObjectState(5, 2); break;
			case 2: SetObjectState(2, 2); SetObjectState(5, 1); break;
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
			
			SetObjectState(7, 1); // Reset button
			drawAnimation(); _gfx->copyToScreen();
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
	// NOTE: The original DOS hides dragged sprites during drag (SetObjectOnOff FALSE)
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

void ColonyEngine::SetObjectState(int num, int state) {
	num--;
	if (num >= 0 && num < (int)_lSprites.size())
		_lSprites[num]->current = state - 1;
}

int ColonyEngine::ObjectState(int num) const {
	num--;
	if (num >= 0 && num < (int)_lSprites.size())
		return _lSprites[num]->current + 1;
	return 0;
}

void ColonyEngine::SetObjectOnOff(int num, bool on) {
	num--;
	if (num >= 0 && num < (int)_lSprites.size())
		_lSprites[num]->onoff = on;
}

void ColonyEngine::refreshAnimationDisplay() {
	for (int i = 0; i < 6; i++) {
		if (_animDisplay[i] < 9) {
			SetObjectOnOff(13 + i * 2, true);
			SetObjectOnOff(14 + i * 2, false);
			SetObjectState(13 + i * 2, _animDisplay[i]);
		} else {
			SetObjectOnOff(14 + i * 2, true);
			SetObjectOnOff(13 + i * 2, false);
			SetObjectState(14 + i * 2, _animDisplay[i] - 8);
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
		if (res[m] < 0) res[m] = -res[m];
		sarray[m] = (uint8)(res[m] + 2);
	}
}

} // End of namespace Colony
