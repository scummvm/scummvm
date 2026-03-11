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
#include "colony/debugger.h"
#include "colony/gfx.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/system.h"
#include "common/util.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/palette.h"
#include "graphics/paletteman.h"
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
	_mouseLocked = false;
	_mousePos = Common::Point(_centerX, _centerY);
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
	_moveForward = false;
	_moveBackward = false;
	_strafeLeft = false;
	_strafeRight = false;
	_rotateLeft = false;
	_rotateRight = false;
	_wm = nullptr;
	_macMenu = nullptr;
	_menuSurface = nullptr;
	_menuBarHeight = 0;

	memset(_wall, 0, sizeof(_wall));
	memset(_mapData, 0, sizeof(_mapData));
	memset(_robotArray, 0, sizeof(_robotArray));
	memset(_foodArray, 0, sizeof(_foodArray));
	memset(_dirXY, 0, sizeof(_dirXY));

	// PATCH.C init
	memset(_levelData, 0, sizeof(_levelData));
	_levelData[1].count = 1; // original uses 1-based leveldata[2] for map 2
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
	// intro.c: Me.power[0..2]=POWER(256); armor=SUIT(0); weapons=SUIT(0).
	memset(&_me, 0, sizeof(_me));
	_me.xloc = 4400;
	_me.yloc = 4400;
	_me.xindex = _me.xloc >> 8;
	_me.yindex = _me.yloc >> 8;
	_me.look = 32;
	_me.ang = 32;
	_me.type = kMeNum;
	_me.power[0] = 256; // weapons power
	_me.power[1] = 256; // life power
	_me.power[2] = 256; // armor power

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
	for (int i = 0; i < 2; i++) {
		_coreState[i] = 0;
		_coreHeight[i] = 256;
	}
	// intro.c: corepower[0]=0; corepower[1]=2; corepower[2]=0;
	_corePower[0] = 0;
	_corePower[1] = 2;
	_corePower[2] = 0;
	_coreIndex = 0;
	_hasMacColors = false;
	memset(_macColors, 0, sizeof(_macColors));
	_orbit = 0;
	_armor = 0;
	_gametest = false;
	_blackoutColor = 15; // Set to white (vINTWHITE) for better visibility in darkness

	setDebugger(new Debugger(this));

	_sound = new Sound(this);
	_resMan = new Common::MacResManager();
	_colorResMan = new Common::MacResManager();
	initTrig();
	battleInit();
	battleSet();
}


ColonyEngine::~ColonyEngine() {
	deleteAnimation();
	if (_savedScreen) {
		_savedScreen->free();
		delete _savedScreen;
	}
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

void ColonyEngine::pauseEngineIntern(bool pause) {
	if (pause && _gfx && _level >= 1 && _level <= 7) {
		if (_savedScreen) {
			_savedScreen->free();
			delete _savedScreen;
		}
		_savedScreen = _gfx->getScreenshot();
	}

	Engine::pauseEngineIntern(pause);

	if (_frameLimiter)
		_frameLimiter->pause(pause);
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
	debugC(1, kColonyDebugRender, "Loaded %d Mac colors", cnum);
}

void ColonyEngine::menuCommandsCallback(int action, Common::String &text, void *data) {
	ColonyEngine *engine = (ColonyEngine *)data;
	engine->handleMenuAction(action);
}

void ColonyEngine::syncMacMenuChecks() {
	if (!_macMenu)
		return;

	Graphics::MacMenuItem *optionsMenu = _macMenu->getMenuItem(3);
	if (!optionsMenu)
		return;

	if (Graphics::MacMenuItem *item = _macMenu->getSubMenuItem(optionsMenu, kMenuActionSound))
		_macMenu->setCheckMark(item, _soundOn);
	if (Graphics::MacMenuItem *item = _macMenu->getSubMenuItem(optionsMenu, kMenuActionCrosshair))
		_macMenu->setCheckMark(item, _crosshair);
	if (Graphics::MacMenuItem *item = _macMenu->getSubMenuItem(optionsMenu, kMenuActionPolyFill))
		_macMenu->setCheckMark(item, !_wireframe);
	if (Graphics::MacMenuItem *item = _macMenu->getSubMenuItem(optionsMenu, kMenuActionCursorShoot))
		_macMenu->setCheckMark(item, _cursorShoot);
}

void ColonyEngine::updateMouseCapture(bool recenter) {
	_system->lockMouse(_mouseLocked);
	_system->showMouse(!_mouseLocked);
	CursorMan.setDefaultArrowCursor();
	CursorMan.showMouse(!_mouseLocked);

	if (_mouseLocked && recenter) {
		_mousePos = Common::Point(_centerX, _centerY);
		_system->warpMouse(_centerX, _centerY);
		_system->getEventManager()->purgeMouseEvents();
	}
}

Common::Point ColonyEngine::getAimPoint() const {
	if (_cursorShoot && !_mouseLocked) {
		return Common::Point(CLIP<int>(_mousePos.x, _screenR.left, _screenR.right - 1),
		                     CLIP<int>(_mousePos.y, _screenR.top, _screenR.bottom - 1));
	}

	return Common::Point(_centerX, _centerY);
}

void ColonyEngine::handleMenuAction(int action) {
	switch (action) {
	case kMenuActionAbout:
		inform("The Colony\nCopyright 1988\nDavid A. Smith", true);
		break;
	case kMenuActionNew:
		startNewGame();
		break;
	case kMenuActionOpen:
		_system->lockMouse(false);
		_system->showMouse(true);
		CursorMan.setDefaultArrowCursor();
		CursorMan.showMouse(true);
		loadGameDialog();
		updateMouseCapture(true);
		break;
	case kMenuActionSave:
	case kMenuActionSaveAs:
		_system->lockMouse(false);
		_system->showMouse(true);
		CursorMan.setDefaultArrowCursor();
		CursorMan.showMouse(true);
		saveGameDialog();
		updateMouseCapture(true);
		break;
	case kMenuActionQuit:
		quitGame();
		break;
	case kMenuActionSound:
		_soundOn = !_soundOn;
		if (!_soundOn)
			_sound->stop();
		syncMacMenuChecks();
		break;
	case kMenuActionCrosshair:
		_crosshair = !_crosshair;
		syncMacMenuChecks();
		break;
	case kMenuActionPolyFill:
		_wireframe = !_wireframe;
		syncMacMenuChecks();
		break;
	case kMenuActionCursorShoot:
		_cursorShoot = !_cursorShoot;
		if (_cursorShoot && _mouseLocked) {
			_mouseLocked = false;
			updateMouseCapture(false);
		}
		syncMacMenuChecks();
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
		// Edit submenu (index 2, disabled  original Mac had these but non-functional)
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
	syncMacMenuChecks();

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

void ColonyEngine::startNewGame() {
	deleteAnimation();
	_sound->stop();

	memset(_wall, 0, sizeof(_wall));
	memset(_mapData, 0, sizeof(_mapData));
	memset(_robotArray, 0, sizeof(_robotArray));
	memset(_foodArray, 0, sizeof(_foodArray));
	memset(_dirXY, 0, sizeof(_dirXY));
	_objects.clear();
	_robotNum = 0;
	_dynamicObjectBase = 0;

	memset(_levelData, 0, sizeof(_levelData));
	_levelData[1].count = 1;
	memset(_carryPatch, 0, sizeof(_carryPatch));
	_patches.clear();
	_carryType = 0;
	_fl = 0;
	_level = 0;
	_gameMode = kModeColony;
	_hasKeycard = false;
	_unlocked = false;
	_weapons = 0;
	_armor = 0;
	_orbit = 0;
	_allGrow = false;
	_suppressCollisionSound = false;
	_action0 = 0;
	_action1 = 0;
	_creature = 0;

	_randomSource.setSeed(Common::RandomSource::generateNewSeed());

	memset(&_me, 0, sizeof(_me));
	_me.xloc = 4400;
	_me.yloc = 4400;
	_me.xindex = _me.xloc >> 8;
	_me.yindex = _me.yloc >> 8;
	_me.look = 32;
	_me.ang = 32;
	_me.type = kMeNum;
	_me.power[0] = 256;
	_me.power[1] = 256;
	_me.power[2] = 256;

	for (int i = 0; i < 4; i++)
		_decode1[i] = _decode2[i] = _decode3[i] = 0;
	for (int i = 0; i < 6; i++)
		_animDisplay[i] = 1;
	_coreState[0] = _coreState[1] = 0;
	_coreHeight[0] = _coreHeight[1] = 256;
	_corePower[0] = 0;
	_corePower[1] = 2;
	_corePower[2] = 0;
	_coreIndex = 0;
	_animationName.clear();
	_backgroundActive = false;
	_animationRunning = false;
	_animationResult = 0;
	_doorOpen = false;
	_elevatorFloor = 0;
	_airlockX = -1;
	_airlockY = -1;
	_airlockDirection = -1;
	_airlockTerminate = false;
	_lastClickTime = 0;
	_displayCount = 0;
	_lastAnimUpdate = 0;
	_lastWarningChimeTime = 0;
	_battleRound = 0;
	_battleMaxP = 0;
	_projon = false;
	_pcount = 0;

	battleInit();
	updateViewportLayout();
	loadMap(1);
	battleSet();
	_mousePos = Common::Point(_centerX, _centerY);
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
			debugC(1, kColonyDebugRender, "Color Colony resource fork not found (optional)");
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
	debugC(1, kColonyDebugRender, "Screen format: %d bytesPerPixel. Actual size: %dx%d", format.bytesPerPixel, _width, _height);

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

	const int startupLoadSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;
	if (startupLoadSlot >= 0) {
		Common::Error status = loadGameState(startupLoadSlot);
		if (status.getCode() != Common::kNoError)
			return status;
	} else {
		playIntro();
		loadMap(1); // Normal startup path begins on the first map after the intro
	}

	_mouseLocked = true;
	updateMouseCapture(true);

	int mouseDX = 0, mouseDY = 0;
	bool mouseMoved = false;
	uint32 lastMoveTick = _system->getMillis();
	uint32 lastColonyTick = lastMoveTick;
	uint32 lastBattleTick = lastMoveTick;
	while (!shouldQuit()) {
		_frameLimiter->startFrame();

		auto qlog = [](int32 x) -> int {
			int i = 0;
			while (x > 0) {
				x >>= 1;
				i++;
			}
			return i;
		};
		const int warningLevel = 4;
		const int lifePower = qlog(_me.power[1]);
		const uint32 now = _system->getMillis();
		if (lifePower <= warningLevel && !_sound->isPlaying() && now - _lastWarningChimeTime >= 250) {
			_sound->play(Sound::kChime);
			_lastWarningChimeTime = now;
		}

		if (_gameMode == kModeColony && now - lastColonyTick >= 66) {
			lastColonyTick = now;
			cThink();
		}

		// The original battle loop advanced AI on the game loop cadence, not
		// every rendered frame. Running this at 60 fps makes enemies and
		// projectiles several times more aggressive than DOS/Mac.
		if (_gameMode == kModeBattle && now - lastBattleTick >= 66) {
			lastBattleTick = now;
			battleThink();
		}

		Common::Event event;
		while (_system->getEventManager()->pollEvent(event)) {
			// Let MacWindowManager handle menu events first
			if (_wm) {
				bool wasMenuActive = _wm->isMenuActive();
				if (_wm->processEvent(event)) {
					// WM consumed the event (menu interaction)
					if (!wasMenuActive && _wm->isMenuActive()) {
						_system->lockMouse(false);
						_system->showMouse(true);
						CursorMan.setDefaultArrowCursor();
						CursorMan.showMouse(true);
					}
					continue;
				}
				if (wasMenuActive && !_wm->isMenuActive()) {
					updateMouseCapture(true);
					if (_mouseLocked) {
						mouseDX = mouseDY = 0;
						mouseMoved = false;
					}
				}
			}

			if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START) {
				switch (event.customType) {
				case kActionMoveForward:
					_moveForward = true;
					break;
				case kActionMoveBackward:
					_moveBackward = true;
					break;
				case kActionStrafeLeft:
					_strafeLeft = true;
					break;
				case kActionStrafeRight:
					_strafeRight = true;
					break;
				case kActionRotateLeft:
					_rotateLeft = true;
					break;
				case kActionRotateRight:
					_rotateRight = true;
					break;
				case kActionLookLeft:
					_me.look = _me.ang + 64;
					break;
				case kActionLookRight:
					_me.look = _me.ang - 64;
					break;
				case kActionLookBehind:
					_me.look = _me.ang + 128;
					break;
				case kActionToggleDashboard:
					_showDashBoard = !_showDashBoard;
					break;
				case kActionToggleWireframe:
					_wireframe = !_wireframe;
					debugC(1, kColonyDebugRender, "Polyfill: %s", _wireframe ? "off (wireframe)" : "on (filled)");
					break;
				case kActionToggleFullscreen:
					if (_macMenu) {
						_fullscreen = !_fullscreen;
						_menuBarHeight = _fullscreen ? 0 : 20;
						updateViewportLayout();
					}
					break;
				case kActionToggleMouselook:
					if (_fl == 2)
						dropCarriedObject();
					else if (_fl == 1)
						exitForklift();
					else {
						_mouseLocked = !_mouseLocked;
						updateMouseCapture(true);
						if (_mouseLocked) {
							mouseDX = mouseDY = 0;
							mouseMoved = false;
						}
					}
					break;
				case kActionFire:
					cShoot();
					break;
				case kActionEscape:
					_system->lockMouse(false);
					_system->showMouse(true);
					CursorMan.setDefaultArrowCursor();
					CursorMan.showMouse(true);
					openMainMenuDialog();
					_gfx->computeScreenViewport();
					updateMouseCapture(true);
					break;
				default:
					break;
				}
			} else if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_END) {
				switch (event.customType) {
				case kActionMoveForward:
					_moveForward = false;
					break;
				case kActionMoveBackward:
					_moveBackward = false;
					break;
				case kActionStrafeLeft:
					_strafeLeft = false;
					break;
				case kActionStrafeRight:
					_strafeRight = false;
					break;
				case kActionRotateLeft:
					_rotateLeft = false;
					break;
				case kActionRotateRight:
					_rotateRight = false;
					break;
				default:
					break;
				}
			} else if (event.type == Common::EVENT_KEYDOWN) {
				// Speed keys 1-5 remain as raw keyboard events
				switch (event.kbd.keycode) {
				case Common::KEYCODE_1:
				case Common::KEYCODE_2:
				case Common::KEYCODE_3:
				case Common::KEYCODE_4:
				case Common::KEYCODE_5:
					_speedShift = event.kbd.keycode - Common::KEYCODE_1 + 1;
					debugC(1, kColonyDebugUI, "Speed: %d", _speedShift);
					break;
				default:
					break;
				}
			} else if (event.type == Common::EVENT_LBUTTONDOWN && (_mouseLocked || _cursorShoot)) {
				cShoot();
			} else if (event.type == Common::EVENT_MOUSEMOVE) {
				_mousePos = event.mouse;
				if (_mouseLocked) {
					mouseDX += event.relMouse.x;
					mouseDY += event.relMouse.y;
					mouseMoved = true;
				}
			} else if (event.type == Common::EVENT_SCREEN_CHANGED) {
				_gfx->computeScreenViewport();
			}
		}

		if (mouseMoved && _mouseLocked) {
			if (mouseDX != 0) {
				_me.look = (uint8)((int)_me.look - (mouseDX * _mouseSensitivity));
				// In battle mode, body always faces look direction
				if (_gameMode == kModeBattle)
					_me.ang = _me.look;
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

		// Apply continuous movement/rotation from held keys,
		// throttled to ~15 ticks/sec to match original key-repeat feel
		if (now - lastMoveTick >= 66) {
			lastMoveTick = now;
			const int moveX = (_cost[_me.look] * (1 << _speedShift)) >> 4;
			const int moveY = (_sint[_me.look] * (1 << _speedShift)) >> 4;
			const int rotSpeed = 1 << (_speedShift - 1);

			if (_gameMode == kModeBattle) {
				if (_moveForward)
					battleCommand(_me.xloc + moveX, _me.yloc + moveY);
				if (_moveBackward)
					battleCommand(_me.xloc - moveX, _me.yloc - moveY);
				if (_strafeLeft) {
					uint8 strafeAngle = (uint8)((int)_me.look + 64);
					int sx = (_cost[strafeAngle] * (1 << _speedShift)) >> 4;
					int sy = (_sint[strafeAngle] * (1 << _speedShift)) >> 4;
					battleCommand(_me.xloc + sx, _me.yloc + sy);
				}
				if (_strafeRight) {
					uint8 strafeAngle = (uint8)((int)_me.look - 64);
					int sx = (_cost[strafeAngle] * (1 << _speedShift)) >> 4;
					int sy = (_sint[strafeAngle] * (1 << _speedShift)) >> 4;
					battleCommand(_me.xloc + sx, _me.yloc + sy);
				}
			} else {
				if (_moveForward)
					cCommand(_me.xloc + moveX, _me.yloc + moveY, true);
				if (_moveBackward)
					cCommand(_me.xloc - moveX, _me.yloc - moveY, true);
				if (_strafeLeft) {
					uint8 strafeAngle = (uint8)((int)_me.look + 64);
					int sx = (_cost[strafeAngle] * (1 << _speedShift)) >> 4;
					int sy = (_sint[strafeAngle] * (1 << _speedShift)) >> 4;
					cCommand(_me.xloc + sx, _me.yloc + sy, true);
				}
				if (_strafeRight) {
					uint8 strafeAngle = (uint8)((int)_me.look - 64);
					int sx = (_cost[strafeAngle] * (1 << _speedShift)) >> 4;
					int sy = (_sint[strafeAngle] * (1 << _speedShift)) >> 4;
					cCommand(_me.xloc + sx, _me.yloc + sy, true);
				}
			}
			if (_rotateLeft) {
				_me.ang += rotSpeed;
				_me.look += rotSpeed;
			}
			if (_rotateRight) {
				_me.ang -= rotSpeed;
				_me.look -= rotSpeed;
			}
		}

		if (_gameMode == kModeBattle) {
			renderBattle();
			drawDashboardStep1();
			drawCrosshair();
		} else {
			_gfx->clear((_corePower[_coreIndex] > 0) ? 15 : 0);
			corridor();
			drawDashboardStep1();
			drawCrosshair();
			checkCenter();
		}

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
	// Non-blocking check for skip during intro/animation sequences.
	// Drains pending events, handles screen refresh and quit events.
	// Returns true if skip action fired or shouldQuit() is true.
	Common::Event event;
	while (_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			return true;
		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			if (event.customType == kActionSkipIntro)
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
	// Returns true if skip action was triggered (for intro skip propagation).

	// Clear movement flags so held keys don't re-trigger on return
	_moveForward = _moveBackward = false;
	_strafeLeft = _strafeRight = false;
	_rotateLeft = _rotateRight = false;

	while (!shouldQuit()) {
		Common::Event event;
		while (_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				return false;
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				if (event.customType == kActionSkipIntro)
					return true;
				return false;
			case Common::EVENT_KEYDOWN:
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

} // End of namespace Colony
