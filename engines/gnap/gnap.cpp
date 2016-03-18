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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "gnap/gnap.h"
#include "gnap/datarchive.h"
#include "gnap/gamesys.h"
#include "gnap/resource.h"
#include "gnap/sound.h"

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/timer.h"

#include "engines/util.h"

#include "graphics/cursorman.h"

namespace Gnap {

static const int kCursors[] = {
	LOOK_CURSOR,
	GRAB_CURSOR,
	TALK_CURSOR,
	PLAT_CURSOR
};

static const int kDisabledCursors[] = {
	NOLOOK_CURSOR,
	NOGRAB_CURSOR,
	NOTALK_CURSOR,
	NOPLAT_CURSOR
};

static const char *kCursorNames[] = {
	"LOOK_CURSOR",
	"GRAB_CURSOR",
	"TALK_CURSOR",
	"PLAT_CURSOR",
	"NOLOOK_CURSOR",
	"NOGRAB_CURSOR",
	"NOTALK_CURSOR",
	"NOPLAT_CURSOR",
	"EXIT_L_CURSOR",
	"EXIT_R_CURSOR",
	"EXIT_U_CURSOR",
	"EXIT_D_CURSOR",
	"EXIT_NE_CURSOR",
	"EXIT_NW_CURSOR",
	"EXIT_SE_CURSOR",
	"EXIT_SW_CURSOR",
	"WAIT_CURSOR"
};


static const int kCursorSpriteIds[30] = {
	0x005, 0x008, 0x00A, 0x004, 0x009, 0x003,
	0x006, 0x007, 0x00D, 0x00F, 0x00B, 0x00C,
	0x019, 0x01C, 0x015, 0x014, 0x010, 0x01A,
	0x018, 0x013, 0x011, 0x012, 0x01B, 0x016,
	0x017, 0x01D, 0x01E, 0x01F, 0x76A, 0x76B
};

static const char *kSceneNames[] = {
	"open", "pigpn", "truck", "creek", "mafrm", "frbrn", "inbrn", "crash",
	"porch", "barbk", "kitch", "bar", "juke", "wash", "john", "jkbox",
	"brawl", "stret", "frtoy", "intoy", "frgro", "park", "cash", "ingro",
	"frcir", "booth", "circ", "outcl", "incln", "monk", "elcir", "beer",
	"pig2", "trk2", "creek", "frbrn", "inbrn", "mafrm", "infrm", "efair",
	"fair", "souv", "chick", "ship", "kiss", "disco", "boot", "can",
	"can2", "drive", "tung", "puss", "space", "phone", "can3"
};

GnapEngine::GnapEngine(OSystem *syst, const ADGameDescription *gd) :
	Engine(syst), _gameDescription(gd) {
	
	_random = new Common::RandomSource("gnap");
	
	Engine::syncSoundSettings();

}

GnapEngine::~GnapEngine() {

	delete _random;

}

Common::Error GnapEngine::run() {
	// Initialize the graphics mode to ARGB8888
	Graphics::PixelFormat format = Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24);
	initGraphics(800, 600, true, &format);

	// We do not support color conversion yet
	if (_system->getScreenFormat() != format)
		return Common::kUnsupportedColorMode;
		
	_lastUpdateClock = 0;
		
	// >>>>> Variable initialization
	_cursorIndex = -1;
	_verbCursor = 1;
	invClear();
	clearFlags();
	_grabCursorSprite = 0;
	_newGrabCursorSpriteIndex = -1;
	_backgroundSurface = 0;
	_isStockDatLoaded = false;
	_gameDone = false;
	_isPaused = false;
	_pauseSprite = 0;

	////////////////////////////////////////////////////////////////////////////

	_exe = new Common::PEResources();
	if (!_exe->loadFromEXE("ufos.exe"))
		error("Could not load ufos.exe");

	_dat = new DatManager();
	_spriteCache = new SpriteCache(_dat);
	_soundCache = new SoundCache(_dat);
	_sequenceCache = new SequenceCache(_dat);
	_gameSys = new GameSys(this);
	_soundMan = new SoundMan(this);
	
	_menuBackgroundSurface = 0;
	
	initGlobalSceneVars();

#if 1

	mainLoop();

#else
	
	Graphics::Surface *testBack = new Graphics::Surface();
	testBack->create(800, 600, _system->getScreenFormat());
	//testBack->fillRect(Common::Rect(0, 0, 800, 600), 0xFFFFFFFF);
	testBack->fillRect(Common::Rect(0, 0, 800, 600), 0xFF000000);
	
	_currentSceneNum = 41;

    Common::String datFilename = Common::String::format("%s_n.dat", kSceneNames[_currentSceneNum]);
	_dat->open(0, datFilename.c_str());
	
	_gameSys->setBackgroundSurface(testBack, 0, 500, 1, 1000);

	_gameSys->insertSequence(0x11b, 100, -1, -1, kSeqNone, 0, 0, 0);

	CursorMan.showMouse(true);
	
	while (!shouldQuit()) {
		Common::Event event;

		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				break;
			case Common::EVENT_LBUTTONUP:
			case Common::EVENT_LBUTTONDOWN:
			case Common::EVENT_RBUTTONUP:
			case Common::EVENT_RBUTTONDOWN:
			case Common::EVENT_MOUSEMOVE:
				break;
			case Common::EVENT_QUIT:
				quitGame();
				break;
			default:
				break;
			}
		}
		
		_gameSys->fatUpdate();
		_gameSys->drawSprites();
		_gameSys->updateScreen();
		_gameSys->_gameSysClock++;
		updateTimers();

		_system->updateScreen();
		_system->delayMillis(100);

	}

	_dat->close(0);
	
	testBack->free();
	delete testBack;

	return Common::kNoError;

#endif

	delete _soundMan;
	delete _gameSys;
	delete _sequenceCache;
	delete _soundCache;
	delete _spriteCache;
	delete _dat;
	
	delete _exe;

	debug("run() done");
	
	return Common::kNoError;
}

bool GnapEngine::hasFeature(EngineFeature f) const {
	return
		false;
}

void GnapEngine::updateEvents() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			_keyPressState[event.kbd.keycode] = 1;
			_keyDownState[event.kbd.keycode] = 1;
			break;
		case Common::EVENT_KEYUP:
			_keyDownState[event.kbd.keycode] = 0;
  			break;
		case Common::EVENT_MOUSEMOVE:
  			_mouseX = event.mouse.x;
  			_mouseY = event.mouse.y;
  			break;
		case Common::EVENT_LBUTTONUP:
  			_mouseButtonState._left = false;
  			break;
		case Common::EVENT_LBUTTONDOWN:
			_leftClickMouseX = event.mouse.x;
			_leftClickMouseY = event.mouse.y;
			_mouseButtonState._left = true;
			_mouseClickState._left = true;
  			break;
		case Common::EVENT_RBUTTONUP:
			_mouseButtonState._right = false;
  			break;
		case Common::EVENT_RBUTTONDOWN:
  			_mouseButtonState._right = true;
  			_mouseClickState._right = true;
  			break;
		case Common::EVENT_QUIT:
			quitGame();
			break;
		default:
			break;
		}
	}
}

void GnapEngine::gameUpdateTick() {
	updateEvents();

	// TODO Check _gameDone in the various game loops
	if (shouldQuit()) {
		_gameDone = true;
		_sceneDone = true;
	}

	// TODO Improve this (variable frame delay to keep ~15fps)
	int currClock = _system->getMillis();
	if (currClock >= _lastUpdateClock + 66) {
		_gameSys->fatUpdate();
		_gameSys->drawSprites();
		_gameSys->updateScreen();
		_gameSys->updatePlaySounds();
		_gameSys->_gameSysClock++;
		updateTimers();
		_lastUpdateClock = currClock;
	}

	_soundMan->update();
	_system->updateScreen();
	_system->delayMillis(5);

}

void GnapEngine::saveTimers() {
	for (int i = 0; i < kMaxTimers; ++i )
		_savedTimers[i] = _timers[i];
}

void GnapEngine::restoreTimers() {
	for (int i = 0; i < kMaxTimers; ++i )
		_timers[i] = _savedTimers[i];
}

void GnapEngine::pauseGame() {
	if (!_isPaused) {
		saveTimers();
		hideCursor();
		setGrabCursorSprite(-1);
		_pauseSprite = _gameSys->createSurface(0x1076C);
		_gameSys->insertSpriteDrawItem(_pauseSprite, (800 - _pauseSprite->w) / 2, (600 - _pauseSprite->h) / 2, 356);
		_lastUpdateClock = 0;
		gameUpdateTick();
		// TODO playMidi("pause.mid");
		_isPaused = true;
	}
}

void GnapEngine::resumeGame() {
	if (_isPaused) {
		restoreTimers();
		_gameSys->removeSpriteDrawItem(_pauseSprite, 356);
		_lastUpdateClock = 0;
		gameUpdateTick();
		deleteSurface(&_pauseSprite);
		// TODO stopMidi();
		_isPaused = false;
		clearAllKeyStatus1();
		_mouseClickState._left = false;
		_mouseClickState._right = false;
		showCursor();
		_gameSys->_gameSysClock = 0;
		_gameSys->_lastUpdateClock = 0;
	}
}

void GnapEngine::updatePause() {
	while (_isPaused) {
		gameUpdateTick();
		if (isKeyStatus1(Common::KEYCODE_p)) {
			clearKeyStatus1(Common::KEYCODE_p);
			resumeGame();
		}
		//_system->delayMillis(100);
	}
}

int GnapEngine::getRandom(int max) {
	return _random->getRandomNumber(max - 1);
}

int GnapEngine::readSavegameDescription(int savegameNum, Common::String &description) {
	description = Common::String::format("Savegame %d", savegameNum);
	return 0;
}

int GnapEngine::loadSavegame(int savegameNum) {
	return 1;
}

void GnapEngine::delayTicks(int a1) {
	// TODO
	gameUpdateTick();
}

void GnapEngine::delayTicksCursor(int a1) {
	// TODO
	gameUpdateTick();
}

void GnapEngine::setHotspot(int index, int16 x1, int16 y1, int16 x2, int16 y2, uint16 flags,
	int16 walkX, int16 walkY) {
	_hotspots[index]._x1 = x1;
	_hotspots[index]._y1 = y1;
	_hotspots[index]._x2 = x2;
	_hotspots[index]._y2 = y2;
	_hotspots[index]._flags = flags;
	_hotspots[index]._id = index;
	_hotspotsWalkPos[index].x = walkX;
	_hotspotsWalkPos[index].y = walkY;
}

int GnapEngine::getHotspotIndexAtPos(int16 x, int16 y) {
	for (int i = 0; i < _hotspotsCount; ++i)
		if (!_hotspots[i].isFlag(SF_DISABLED) && _hotspots[i].isPointInside(x, y))
			return i;
	return -1;
}

void GnapEngine::updateCursorByHotspot() {
	if (!_isWaiting) {
		int hotspotIndex = getHotspotIndexAtPos(_mouseX, _mouseY);

#if 1
		// NOTE This causes some display glitches so don't worry
		char t[256];
		sprintf(t, "hotspot = %d", hotspotIndex);
		_gameSys->fillSurface(0, 10, 10, 80, 16, 0, 0, 0);
		_gameSys->drawTextToSurface(0, 10, 10, 255, 255, 255, t);
#endif		

		if (hotspotIndex < 0)
			setCursor(kDisabledCursors[_verbCursor]);
		else if (_hotspots[hotspotIndex]._flags & SF_EXIT_L_CURSOR)
			setCursor(EXIT_L_CURSOR);
		else if (_hotspots[hotspotIndex]._flags & SF_EXIT_R_CURSOR)
			setCursor(EXIT_R_CURSOR);
		else if (_hotspots[hotspotIndex]._flags & SF_EXIT_U_CURSOR)
			setCursor(EXIT_U_CURSOR);
		else if (_hotspots[hotspotIndex]._flags & SF_EXIT_D_CURSOR)
			setCursor(EXIT_D_CURSOR);
		else if (_hotspots[hotspotIndex]._flags & SF_EXIT_NE_CURSOR)
			setCursor(EXIT_NE_CURSOR);
		else if (_hotspots[hotspotIndex]._flags & SF_EXIT_NW_CURSOR)
			setCursor(EXIT_NW_CURSOR);
		else if (_hotspots[hotspotIndex]._flags & SF_EXIT_SE_CURSOR)
			setCursor(EXIT_SE_CURSOR);
		else if (_hotspots[hotspotIndex]._flags & SF_EXIT_SW_CURSOR)
			setCursor(EXIT_SW_CURSOR);
		else if (_hotspots[hotspotIndex]._flags & (1 << _verbCursor))
			setCursor(kCursors[_verbCursor]);
		else
			setCursor(kDisabledCursors[_verbCursor]);
	}
	// Update platypus hotspot
	_hotspots[0]._x1 = _gridMinX + 75 * _platX - 30;
	_hotspots[0]._y1 = _gridMinY + 48 * _platY - 100;
	_hotspots[0]._x2 = _gridMinX + 75 * _platX + 30;
	_hotspots[0]._y2 = _gridMinY + 48 * _platY;
}

int GnapEngine::getClickedHotspotId() {
	int result = -1;
	if (_isWaiting)
		_mouseClickState._left = false;
	else if (_mouseClickState._left) {
		int hotspotIndex = getHotspotIndexAtPos(_leftClickMouseX, _leftClickMouseY);
		if (hotspotIndex >= 0) {
			_mouseClickState._left = false;
			_timers[3] = 300;
			result = _hotspots[hotspotIndex]._id;
		}
	}
	return result;
}

int GnapEngine::getInventoryItemSpriteNum(int index) {
	return kCursorSpriteIds[index];
}

void GnapEngine::updateMouseCursor() {
	if (_mouseClickState._right) {
		// Switch through the verb cursors
		_mouseClickState._right = false;
		_timers[3] = 300;
		_verbCursor = (_verbCursor + 1) % 4;
		if (!isFlag(0) && _verbCursor == PLAT_CURSOR && _cursorValue == 1)
			_verbCursor = (_verbCursor + 1) % 4;
		if (!_isWaiting)
			setCursor(kDisabledCursors[_verbCursor]);
		setGrabCursorSprite(-1);
	}
	if (_isWaiting && ((_gnapActionStatus < 0 && _beaverActionStatus < 0) || _sceneWaiting)) {
		setCursor(kDisabledCursors[_verbCursor]);
		_isWaiting = false;
	} else if (!_isWaiting && (_gnapActionStatus >= 0 || _beaverActionStatus >= 0) && !_sceneWaiting) {
		setCursor(WAIT_CURSOR);
		_isWaiting = true;
	}
}

void GnapEngine::setVerbCursor(int verbCursor) {
	_verbCursor = verbCursor;
	if (!_isWaiting)
		setCursor(kDisabledCursors[_verbCursor]);
}

void GnapEngine::setCursor(int cursorIndex) {
	if (_cursorIndex != cursorIndex) {
		const char *cursorName = kCursorNames[cursorIndex];
		Graphics::WinCursorGroup *cursorGroup = Graphics::WinCursorGroup::createCursorGroup(*_exe, Common::WinResourceID(cursorName));
		if (cursorGroup) {
			Graphics::Cursor *cursor = cursorGroup->cursors[0].cursor;
			CursorMan.replaceCursor(cursor->getSurface(), cursor->getWidth(), cursor->getHeight(),
				cursor->getHotspotX(), cursor->getHotspotY(), cursor->getKeyColor());
			CursorMan.replaceCursorPalette(cursor->getPalette(), 0, 256);
			delete cursorGroup;
		}
		_cursorIndex = cursorIndex;
	}
}

void GnapEngine::showCursor() {
	CursorMan.showMouse(true);
}

void GnapEngine::hideCursor() {
	CursorMan.showMouse(false);
}

void GnapEngine::setGrabCursorSprite(int index) {
	freeGrabCursorSprite();
	if (index >= 0) {
		createGrabCursorSprite(makeRid(1, kCursorSpriteIds[index]));
		setVerbCursor(GRAB_CURSOR);
	}
	_grabCursorSpriteIndex = index;
}

void GnapEngine::createGrabCursorSprite(int spriteId) {
	_grabCursorSprite = _gameSys->createSurface(spriteId);
	_gameSys->insertSpriteDrawItem(_grabCursorSprite,
		_mouseX - (_grabCursorSprite->w / 2),
		_mouseY - (_grabCursorSprite->h / 2),
		300);
	delayTicks(5);
}

void GnapEngine::freeGrabCursorSprite() {
	if (_grabCursorSprite) {
		_gameSys->removeSpriteDrawItem(_grabCursorSprite, 300);
		_gameSys->removeSpriteDrawItem(_grabCursorSprite, 301);
		delayTicks(5);
		deleteSurface(&_grabCursorSprite);
	}
}

void GnapEngine::updateGrabCursorSprite(int x, int y) {
	if (_grabCursorSprite) {
		int newGrabCursorX = _mouseX - (_grabCursorSprite->w / 2) - x;
		int newGrabCursorY = _mouseY - (_grabCursorSprite->h / 2) - y;
		if (_currGrabCursorX != newGrabCursorX || _currGrabCursorY != newGrabCursorY) {
			_currGrabCursorX = newGrabCursorX;
			_currGrabCursorY = newGrabCursorY;
			Common::Rect rect(newGrabCursorX, newGrabCursorY,
				newGrabCursorX + _grabCursorSprite->w, newGrabCursorY + _grabCursorSprite->h);
			_gameSys->invalidateGrabCursorSprite(300, rect, _grabCursorSprite, _grabCursorSprite);
		}
	}
}

void GnapEngine::invClear() {
	_inventory = 0;
}

void GnapEngine::invAdd(int itemId) {
	_inventory |= (1 << itemId);
}

void GnapEngine::invRemove(int itemId) {
	_inventory &= ~(1 << itemId);
}

bool GnapEngine::invHas(int itemId) {
	return (_inventory & (1 << itemId)) != 0;
}

void GnapEngine::clearFlags() {
	_gameFlags = 0;
}

void GnapEngine::setFlag(int num) {
	_gameFlags |= (1 << num);
}

void GnapEngine::clearFlag(int num) {
	_gameFlags &= ~(1 << num);
}

bool GnapEngine::isFlag(int num) {
	return (_gameFlags & (1 << num)) != 0;
}

Graphics::Surface *GnapEngine::addFullScreenSprite(int resourceId, int id) {
	_fullScreenSpriteId = id;
	_fullScreenSprite = _gameSys->createSurface(resourceId);
	_gameSys->insertSpriteDrawItem(_fullScreenSprite, 0, 0, id);
	return _fullScreenSprite;
}

void GnapEngine::removeFullScreenSprite() {
	_gameSys->removeSpriteDrawItem(_fullScreenSprite, _fullScreenSpriteId);
	deleteSurface(&_fullScreenSprite);
}

void GnapEngine::showFullScreenSprite(int resourceId) {
	hideCursor();
	setGrabCursorSprite(-1);
	addFullScreenSprite(resourceId, 256);
	while (!_mouseClickState._left && !isKeyStatus1(Common::KEYCODE_ESCAPE) &&
		!isKeyStatus1(Common::KEYCODE_SPACE) && !isKeyStatus1(29)) {
		gameUpdateTick();
	}
	_mouseClickState._left = false;
	clearKeyStatus1(Common::KEYCODE_ESCAPE);
	clearKeyStatus1(29);
	clearKeyStatus1(Common::KEYCODE_SPACE);
	removeFullScreenSprite();
	showCursor();
}

void GnapEngine::queueInsertDeviceIcon() {
	_gameSys->insertSequence(0x10849, 20, 0, 0, kSeqNone, 0, _deviceX1, _deviceY1);
}

void GnapEngine::insertDeviceIconActive() {
	_gameSys->insertSequence(0x1084A, 21, 0, 0, kSeqNone, 0, _deviceX1, _deviceY1);
}

void GnapEngine::removeDeviceIconActive() {
	_gameSys->removeSequence(0x1084A, 21, true);
}

void GnapEngine::setDeviceHotspot(int hotspotIndex, int x1, int y1, int x2, int y2) {
	_deviceX1 = x1;
	_deviceX2 = x2;
	_deviceY1 = y1;
	_deviceY2 = y2;
	if (x1 == -1)
		_deviceX1 = 730;
	if (x2 == -1)
		_deviceX2 = 780;
	if (y1 == -1)
		_deviceY1 = 14;
	if (y2 == -1)
		_deviceY2 = 79;
	_hotspots[hotspotIndex]._x1 = _deviceX1;
	_hotspots[hotspotIndex]._y1 = _deviceY1;
	_hotspots[hotspotIndex]._x2 = _deviceX2;
	_hotspots[hotspotIndex]._y2 = _deviceY2;
	_hotspots[hotspotIndex]._flags = SF_TALK_CURSOR | SF_GRAB_CURSOR | SF_LOOK_CURSOR;
	_hotspots[hotspotIndex]._id = hotspotIndex;
}

int GnapEngine::getSequenceTotalDuration(int resourceId) {
	SequenceResource *sequenceResource = _sequenceCache->get(resourceId);	
	int maxValue = 0;
	for (int i = 0; i < sequenceResource->_animationsCount; ++i) {
		SequenceAnimation *animation = &sequenceResource->_animations[i];
		if (animation->_additionalDelay + animation->_maxTotalDuration > maxValue)
			maxValue = animation->_additionalDelay + animation->_maxTotalDuration;
	}
	int totalDuration = maxValue + sequenceResource->_totalDuration;
	_sequenceCache->release(resourceId);
	return totalDuration;
}

bool GnapEngine::isSoundPlaying(int resourceId) {
	return _soundMan->isSoundPlaying(resourceId);
}

void GnapEngine::playSound(int resourceId, bool looping) {
	debug(0, "playSound(%08X, %d)", resourceId, looping);
	_soundMan->playSound(resourceId, looping);
}

void GnapEngine::stopSound(int resourceId) {
	_soundMan->stopSound(resourceId);
}

void GnapEngine::setSoundVolume(int resourceId, int volume) {
	_soundMan->setSoundVolume(resourceId, volume);
}

void GnapEngine::updateTimers() {
	for (int i = 0; i < kMaxTimers; ++i)
		if (_timers[i] > 0)
			--_timers[i];
}

void GnapEngine::initGameFlags(int num) {
	invClear();
	invAdd(kItemMagazine);
	switch (num) {
	case 1:
		setFlag(26);
		break;
	case 2:
		clearFlags();
		break;
	case 3:
		invAdd(kItemDiceQuarterHole);
		clearFlags();
		break;
	case 4:
		invAdd(kItemDiceQuarterHole);
		invAdd(kItemHorn);
		invAdd(kItemLightbulb);
		clearFlags();
		setFlag(0);
		setFlag(1);
		setFlag(2);
		setFlag(3);
		setFlag(4);
		setFlag(5);
		setFlag(6);
		setFlag(7);
		break;
	}

	//DEBUG!
//	setFlag(0); // Enable platypus
	setFlag(25);
	invClear();
	invAdd(kItemMagazine);
#if 0
	invAdd(kItemDisguise);
#endif
#if 1
	//invAdd(kItemGas);
	invAdd(kItemJoint);
	//invAdd(kItemKeys);
	invAdd(kItemWrench);
	//invAdd(kItemTongs);
	invAdd(kItemDiceQuarterHole);
	//invAdd(kItemPill);
	//invAdd(kItemBucketWithBeer);
	invAdd(kItemChickenBucket);
	invAdd(kItemBanana);
	invAdd(kItemHorn);
#endif
}

void GnapEngine::loadStockDat() {
	if (!_isStockDatLoaded) {
		_isStockDatLoaded = true;
		_dat->open(1, "stock_n.dat");
		//createMenuSprite();
		// NOTE Skipped preloading of data
	}
}

void GnapEngine::mainLoop() {
	
	_newCursorValue = 1;
	_cursorValue = -1;
	_newSceneNum = 0;
	_currentSceneNum = 55;
	_prevSceneNum = 55;
	invClear();
	clearFlags();
	_grabCursorSpriteIndex = -1;
	_grabCursorSprite = 0;

	debug("MainLoop #1");

	// > DEBUG BEGIN
	_currentSceneNum = 53;
	_newSceneNum = 42;
	_newCursorValue = 3;
	// < DEBUG END

	loadStockDat();

	while (!_gameDone) {
	
		debug("New scene: %d", _newSceneNum);
	
		_prevSceneNum = _currentSceneNum;
		_currentSceneNum = _newSceneNum;
		
		debug("GnapEngine::mainLoop() _prevSceneNum: %d; _currentSceneNum: %d", _prevSceneNum, _currentSceneNum);

		if (_newCursorValue != _cursorValue) {
			debug("_newCursorValue: %d", _newCursorValue);
			_cursorValue = _newCursorValue;
			if (!_wasSavegameLoaded)
				initGameFlags(_cursorValue);
		}

		_sceneSavegameLoaded = _wasSavegameLoaded;
		_wasSavegameLoaded = false;

		initScene();
		
		runSceneLogic();
		afterScene();
		
		_soundMan->stopAll();

		// Force purge all resources
		_sequenceCache->purge(true);
		_soundCache->purge(true);
		_spriteCache->purge(true);

		if (isKeyStatus1(28)) {
			clearKeyStatus1(28);
			if (_debugLevel == 4)
				_gameDone = true;
		}

	}

	if (_backgroundSurface)
		deleteSurface(&_backgroundSurface);
	
	_dat->close(1);
	// TODO freeMenuSprite();
	// TODO freeFont();

	debug("MainLoop #XXX2");

}

void GnapEngine::initScene() {

	Common::String datFilename;
	
	_isLeavingScene = false;
	_sceneDone = false;
	_newSceneNum = 55;
	_gnapActionStatus = -1;
	_beaverActionStatus = -1;
	gnapInitBrainPulseRndValue();
	hideCursor();
	clearAllKeyStatus1();
	_mouseClickState._left = false;
	_mouseClickState._right = false;
	_sceneClickedHotspot = -1;

	datFilename = Common::String::format("%s_n.dat", kSceneNames[_currentSceneNum]);

	debug("GnapEngine::initScene() datFilename: %s", datFilename.c_str());

	_dat->open(0, datFilename.c_str());

	int backgroundId = initSceneLogic();

	if (!_backgroundSurface) {
		if (_currentSceneNum != 0)
			_backgroundSurface = _gameSys->loadBitmap(makeRid(1, 0x8AA));
		else
			_backgroundSurface = _gameSys->loadBitmap(makeRid(0, backgroundId));
		_gameSys->setBackgroundSurface(_backgroundSurface, 0, 500, 1, 1000);
	}

	if (_currentSceneNum != 0 && _currentSceneNum != 16 && _currentSceneNum != 47 &&
		_currentSceneNum != 48 && _currentSceneNum != 54) {
		_gameSys->drawBitmap(backgroundId);
	}

	if ((_cursorValue == 4 && isFlag(12)) || _currentSceneNum == 41)
		playSound(makeRid(1, 0x8F6), true);

}

void GnapEngine::endSceneInit() {
	showCursor();
	if (_newGrabCursorSpriteIndex >= 0)
		setGrabCursorSprite(_newGrabCursorSpriteIndex);
}

void GnapEngine::afterScene() {

	if (_gameDone)
		return;
	
	if (_newCursorValue == _cursorValue && _newSceneNum != 0 && _newSceneNum != 16 &&
		_newSceneNum != 47 && _newSceneNum != 48 && _newSceneNum != 54 && _newSceneNum != 49 &&
		_newSceneNum != 50 && _newSceneNum != 51 && _newSceneNum != 52)
		_newGrabCursorSpriteIndex = _grabCursorSpriteIndex;
	else
		_newGrabCursorSpriteIndex = -1;

	setGrabCursorSprite(-1);

	_gameSys->requestClear2(false);
	_gameSys->requestClear1();
	_gameSys->waitForUpdate();

	_gameSys->requestClear2(false);
	_gameSys->requestClear1();
	_gameSys->waitForUpdate();

	screenEffect(0, 0, 0, 0);

	_dat->close(0);

	for (int animationIndex = 0; animationIndex < 12; ++animationIndex)
		_gameSys->setAnimation(0, 0, animationIndex);

	clearKeyStatus1(Common::KEYCODE_p);

	_mouseClickState._left = false;
	_mouseClickState._right = false;

}

void GnapEngine::checkGameKeys() {
	if (isKeyStatus1(Common::KEYCODE_p)) {
		clearKeyStatus1(Common::KEYCODE_p);
		pauseGame();
		updatePause();
	}
	// TODO? Debug input
}

void GnapEngine::startSoundTimerA(int timerIndex) {
	_soundTimerIndexA = timerIndex;
	_timers[timerIndex] = getRandom(50) + 100;
}

int GnapEngine::playSoundA() {

	static const int kSoundIdsA[] = {
		0x93E, 0x93F, 0x941, 0x942, 0x943, 0x944,
		0x945, 0x946, 0x947, 0x948, 0x949
	};

	int soundId = -1;
	
	if (!_timers[_soundTimerIndexA]) {
		_timers[_soundTimerIndexA] = getRandom(50) + 100;
		soundId = kSoundIdsA[getRandom(11)];
		playSound(soundId | 0x10000, 0);
	}
	return soundId;
}

void GnapEngine::startSoundTimerB(int timerIndex) {
	_soundTimerIndexB = timerIndex;
	_timers[timerIndex] = getRandom(50) + 150;
}

int GnapEngine::playSoundB() {

	static const int kSoundIdsB[] = {
		0x93D, 0x929, 0x92A, 0x92B, 0x92C, 0x92D,
		0x92E, 0x92F, 0x930, 0x931, 0x932, 0x933,
		0x934, 0x935, 0x936, 0x937, 0x938, 0x939,
		0x93A
	};

	int soundId = -1;
	
	if (!_timers[_soundTimerIndexB]) {
		_timers[_soundTimerIndexB] = getRandom(50) + 150;
		soundId = kSoundIdsB[getRandom(19)];
		playSound(soundId | 0x10000, 0);
	}
	return soundId;
}

void GnapEngine::startSoundTimerC(int timerIndex) {
	_soundTimerIndexC = timerIndex;
	_timers[timerIndex] = getRandom(50) + 150;
}

int GnapEngine::playSoundC() {

	static const int kSoundIdsC[] = {
		0x918, 0x91F, 0x920, 0x922, 0x923, 0x924,
		0x926
	};

	int soundId = -1;

	if (!_timers[_soundTimerIndexC]) {
		_timers[_soundTimerIndexC] = getRandom(50) + 150;
		soundId = kSoundIdsC[getRandom(7)] ;
		playSound(soundId | 0x10000, 0);
	}
	return soundId;
}

void GnapEngine::startIdleTimer(int timerIndex) {
	_idleTimerIndex = timerIndex;
	_timers[timerIndex] = 3000;
}

void GnapEngine::updateIdleTimer() {
	if (!_timers[_idleTimerIndex]) {
		_timers[_idleTimerIndex] = 3000;
		_gameSys->insertSequence(0x1088B, 255, 0, 0, kSeqNone, 0, 0, 75);
	}
}

void GnapEngine::screenEffect(int dir, byte r, byte g, byte b) {
	if (dir == 1) {
		for (int y = 300; y < 600; y += 50) {
			_gameSys->fillSurface(0, 0, y, 800, 50, r, g, b);
			_gameSys->fillSurface(0, 0, 549 - y + 1, 800, 50, r, g, b);
			gameUpdateTick();
			_system->delayMillis(50);
		}
	} else {
		for (int y = 0; y < 300; y += 50) {
			_gameSys->fillSurface(0, 0, y, 800, 50, r, g, b);
			_gameSys->fillSurface(0, 0, 549 - y + 1, 800, 50, r, g, b);
			gameUpdateTick();
			_system->delayMillis(50);
		}
	}
}

bool GnapEngine::isKeyStatus1(int key) {
	return _keyPressState[key] != 0;;
}

bool GnapEngine::isKeyStatus2(int key) {
	return _keyDownState[key] != 0;;
}

void GnapEngine::clearKeyStatus1(int key) {
	_keyPressState[key] = 0;
	_keyDownState[key] = 0;
}

void GnapEngine::clearAllKeyStatus1() {
	_keyStatus1[0] = 0;
	_keyStatus1[1] = 0;
	memset(_keyPressState, 0, sizeof(_keyPressState));
	memset(_keyDownState, 0, sizeof(_keyDownState));
}

void GnapEngine::deleteSurface(Graphics::Surface **surface) {
	if (surface && *surface) {
		(*surface)->free();
		delete *surface;
		*surface = 0;
	}
}

int GnapEngine::getGnapSequenceId(int kind, int gridX, int gridY) {
	int sequenceId = 0;
	
	switch (kind) {

	case gskPullOutDevice:
		if (gridX > 0 && gridY > 0) {
			if (_gnapY > gridY) {
				if (_gnapX > gridX) {
					sequenceId = 0x83F;
					_gnapIdleFacing = 5;
				} else {
					sequenceId = 0x83D;
					_gnapIdleFacing = 7;
				}
			} else {
				if (_gnapX > gridX) {
					sequenceId = 0x83B;
					_gnapIdleFacing = 3;
				} else {
					sequenceId = 0x839;
					_gnapIdleFacing = 1;
				}
			}
		} else {
			switch (_gnapIdleFacing) {
			case 1:
				sequenceId = 0x839;
				break;
			case 3:
				sequenceId = 0x83B;
				break;
			case 7:
				sequenceId = 0x83D;
				break;
			default:
				sequenceId = 0x83F;
				break;
			}
		}
		break;

	case gskPullOutDeviceNonWorking:
		if (gridX > 0 && gridY > 0) {
			if (_gnapY > gridY) {
				if (_gnapX > gridX) {
					sequenceId = 0x829;
					_gnapIdleFacing = 5;
				} else {
					sequenceId = 0x828;
					_gnapIdleFacing = 7;
				}
			} else {
				if (_gnapX > gridX) {
					sequenceId = 0x827;
					_gnapIdleFacing = 3;
				} else {
					sequenceId = 0x826;
					_gnapIdleFacing = 1;
				}
			}
		} else {
			switch (_gnapIdleFacing) {
			case 1:
				sequenceId = 0x826;
				break;
			case 3:
				sequenceId = 0x827;
				break;
			case 7:
				sequenceId = 0x828;
				break;
			default:
				sequenceId = 0x829;
				break;
			}
		}
		break;

	case gskScratchingHead:
		if (gridX > 0 && gridY > 0) {
			if (_gnapY > gridY) {
				if (_gnapX > gridX) {
					sequenceId = 0x834;
					_gnapIdleFacing = 3;
				} else {
					sequenceId = 0x885;
					_gnapIdleFacing = 7;
				}
			} else {
				if (_gnapX > gridX) {
					sequenceId = 0x834;
					_gnapIdleFacing = 3;
				} else {
					sequenceId = 0x833;
					_gnapIdleFacing = 1;
				}
			}
		} else {
			switch (_gnapIdleFacing) {
			case 1:
				sequenceId = 0x833;
				_gnapIdleFacing = 1;
				break;
			case 3:
				sequenceId = 0x834;
				_gnapIdleFacing = 3;
				break;
			case 7:
				sequenceId = 0x885;
				_gnapIdleFacing = 7;
				break;
			default:
				sequenceId = 0x834;
				_gnapIdleFacing = 3;
				break;
			}
		}
		break;

	case gskIdle:
		if (gridX > 0 && gridY > 0) {
			if (_gnapY > gridY) {
				if (_gnapX > gridX) {
					sequenceId = 0x7BC;
					_gnapIdleFacing = 5;
				} else {
					sequenceId = 0x7BB;
					_gnapIdleFacing = 7;
				}
			} else {
				if (_gnapX > gridX) {
					sequenceId = 0x7BA;
					_gnapIdleFacing = 3;
				} else {
					sequenceId = 0x7B9;
					_gnapIdleFacing = 1;
				}
			}
		} else {
			switch (_gnapIdleFacing) {
			case 1:
				sequenceId = 0x7B9;
				break;
			case 3:
				sequenceId = 0x7BA;
				break;
			case 7:
				sequenceId = 0x7BB;
				break;
			default:
				sequenceId = 0x7BC;
				break;
			}
		}
		break;

	case gskBrainPulsating:
		_gnapBrainPulseNum = (_gnapBrainPulseNum + 1) & 1;
		if (gridX > 0 && gridY > 0) {
			if (_gnapY > gridY) {
				if (_gnapX > gridX) {
					sequenceId = _gnapBrainPulseRndValue + _gnapBrainPulseNum + 0x812;
					_gnapIdleFacing = 5;
				} else {
					sequenceId = _gnapBrainPulseRndValue + _gnapBrainPulseNum + 0x7FE;
					_gnapIdleFacing = 7;
				}
			} else {
				if (_gnapX > gridX) {
					sequenceId = _gnapBrainPulseRndValue + _gnapBrainPulseNum + 0x7D6;
					_gnapIdleFacing = 3;
				} else {
					sequenceId = _gnapBrainPulseRndValue + _gnapBrainPulseNum + 0x7EA;
					_gnapIdleFacing = 1;
				}
			}
		} else {
			switch (_gnapIdleFacing) {
			case 1:
				sequenceId = _gnapBrainPulseRndValue + _gnapBrainPulseNum + 0x7EA;
				break;
			case 3:
				sequenceId = _gnapBrainPulseRndValue + _gnapBrainPulseNum + 0x7D6;
				break;
			case 7:
				sequenceId = _gnapBrainPulseRndValue + _gnapBrainPulseNum + 0x7FE;
				break;
			default:
				sequenceId = _gnapBrainPulseRndValue + _gnapBrainPulseNum + 0x812;
				break;
			}
		}
		break;

	case gskImpossible:
		if (gridX > 0 && gridY > 0) {
			if (_gnapY > gridY) {
				if (_gnapX > gridX) {
					sequenceId = 0x831;
					_gnapIdleFacing = 3;
				} else {
					sequenceId = 0x7A8;
					_gnapIdleFacing = 1;
				}
			} else {
				if (_gnapX > gridX) {
					sequenceId = 0x831;
					_gnapIdleFacing = 3;
				} else {
					if (_gnapX % 2)
						sequenceId = 0x7A8;
					else
						sequenceId = 0x89A;
					_gnapIdleFacing = 1;
				}
			}
		} else if (_gnapIdleFacing != 1 && _gnapIdleFacing != 7) {
			sequenceId = 0x831;
			_gnapIdleFacing = 3;
		} else {
			if (_currentSceneNum % 2)
				sequenceId = 0x7A8;
			else
				sequenceId = 0x89A;
			_gnapIdleFacing = 1;
		}
		break;

	case gskDeflect:
		if (gridX > 0 && gridY > 0) {
			if (_gnapY > gridY) {
				if (_gnapX > gridX) {
					sequenceId = 0x830;
					_gnapIdleFacing = 5;
				} else {
					sequenceId = 0x82F;
					_gnapIdleFacing = 7;
				}
			} else {
				if (_gnapX > gridX) {
					sequenceId = 0x82E;
					_gnapIdleFacing = 3;
				} else {
					sequenceId = 0x7A7;
					_gnapIdleFacing = 1;
				}
			}
		} else {
			switch (_gnapIdleFacing) {
			case 1:
				sequenceId = 0x7A7;
				break;
			case 3:
				sequenceId = 0x82E;
				break;
			case 5:
				sequenceId = 0x830;
				break;
			case 7:
				sequenceId = 0x82F;
				break;
			}
		}
		break;

	case gskUseDevice:
		switch (_gnapIdleFacing) {
		case 1:
			sequenceId = 0x83A;
			break;
		case 3:
			sequenceId = 0x83C;
			break;
		case 5:
			sequenceId = 0x840;
			break;
		case 7:
			sequenceId = 0x83E;
			break;
		}
		break;

	case gskMoan1:
		if (gridX > 0 && gridY > 0) {
			if (_gnapY > gridY) {
				if (_gnapX > gridX) {
					sequenceId = 0x832;
					_gnapIdleFacing = 3;
				} else {
					sequenceId = 0x7AA;
					_gnapIdleFacing = 1;
				}
			} else {
				if (_gnapX > gridX) {
					sequenceId = 0x832;
					_gnapIdleFacing = 3;
				} else {
					sequenceId = 0x7AA;
					_gnapIdleFacing = 1;
				}
			}
		} else if (_gnapIdleFacing != 1 && _gnapIdleFacing != 7) {
			sequenceId = 0x832;
			_gnapIdleFacing = 3;
		} else {
			sequenceId = 0x7AA;
			_gnapIdleFacing = 1;
		}
		break;

	case gskMoan2:
		if (gridX > 0 && gridY > 0) {
			if (_gnapY > gridY) {
				if (_gnapX > gridX) {
					sequenceId = 0x832;
					_gnapIdleFacing = 3;
				} else {
					sequenceId = 0x7AA;
					_gnapIdleFacing = 1;
				}
			} else {
				if (_gnapX > gridX) {
					sequenceId = 0x832;
					_gnapIdleFacing = 3;
				} else {
					sequenceId = 0x7AA;
					_gnapIdleFacing = 1;
				}
			}
		} else if (_gnapIdleFacing != 1 && _gnapIdleFacing != 7) {
			sequenceId = 0x832;
			_gnapIdleFacing = 3;
		} else {
			sequenceId = 0x7AA;
			_gnapIdleFacing = 1;
		}
		break;

	}

	return sequenceId | 0x10000;
}

int GnapEngine::getGnapShowSequenceId(int index, int gridX, int gridY) {

	int sequenceId;
	int facing = _gnapIdleFacing;

	if (gridY > 0 && gridX > 0) {
		if (_gnapY > gridY) {
			if (_gnapX > gridX)
				_gnapIdleFacing = 5;
			else
				_gnapIdleFacing = 7;
		} else {
			if (_gnapX > gridX)
				_gnapIdleFacing = 5;
			else
				_gnapIdleFacing = 7;
		}
	} else if (_gnapIdleFacing != 1 && _gnapIdleFacing != 7) {
		_gnapIdleFacing = 5;
	} else {
		_gnapIdleFacing = 7;
	}

	switch (index) {
	case 0:
		if (_gnapIdleFacing == 7)
			sequenceId = 0x8A0;
		else
			sequenceId = 0x8A1;
		break;
	case 1:
		if (_gnapIdleFacing == 7)
			sequenceId = 0x880;
		else
			sequenceId = 0x895;
		break;
	case 2:
		if (_gnapIdleFacing == 7)
			sequenceId = 0x884;
		else
			sequenceId = 0x899;
		break;
	//Skip 3
	case 4:
		if (_gnapIdleFacing == 7)
			sequenceId = 0x881;
		else
			sequenceId = 0x896;
		break;
	case 5:
		if (_gnapIdleFacing == 7)
			sequenceId = 0x883;
		else
			sequenceId = 0x898;
		break;
	case 6:
		if (_gnapIdleFacing == 7)
			sequenceId = 0x87E;
		else
			sequenceId = 0x893;
		break;
	case 7:
		if (_gnapIdleFacing == 7)
			sequenceId = 0x848;
		else
			sequenceId = 0x890;
		break;
	case 8:
		if (_gnapIdleFacing == 7)
			sequenceId = 0x87D;
		else
			sequenceId = 0x892;
		break;
	case 9:
		if (_gnapIdleFacing == 7)
			sequenceId = 0x882;
		else
			sequenceId = 0x897;
		break;
	case 10:
		if (_gnapIdleFacing == 7)
			sequenceId = 0x87C;
		else
			sequenceId = 0x891;
		break;
	case 11:
		if (_gnapIdleFacing == 7)
			sequenceId = 0x87C;
		else
			sequenceId = 0x891;
		break;
	case 12:
		if (_gnapIdleFacing == 7)
			sequenceId = 0x87D;
		else
			sequenceId = 0x892;
		break;
	case 13:
		if (_gnapIdleFacing == 7)
			sequenceId = 0x888;
		else
			sequenceId = 0x89D;
		break;
	case 14:
		if (_gnapIdleFacing == 7)
			sequenceId = 0x87F;
		else
			sequenceId = 0x894;
		break;
	case 15:
		if (_gnapIdleFacing == 7)
			sequenceId = 0x87B;
		else
			sequenceId = 0x8A3;
		break;
	case 16:
		if (_gnapIdleFacing == 7)
			sequenceId = 0x877;
		else
			sequenceId = 0x88C;
		break;
	//Skip 17
	case 18:
		sequenceId = 0x887;
		break;
	case 19:
		if (_gnapIdleFacing == 7)
			sequenceId = 0x87A;
		else
			sequenceId = 0x88F;
		break;
	case 20:
		if (_gnapIdleFacing == 7)
			sequenceId = 0x878;
		else
			sequenceId = 0x88D;
		break;
	case 21:
		if (_gnapIdleFacing == 7)
			sequenceId = 0x879;
		else
			sequenceId = 0x88E;
		break;
	case 22:
		if (_gnapIdleFacing == 7)
			sequenceId = 0x88A;
		else
			sequenceId = 0x89F;
		break;
	case 23:
		if (_gnapIdleFacing == 7)
			sequenceId = 0x889;
		else
			sequenceId = 0x89E;
		break;
	case 24:
		if (_gnapIdleFacing == 7)
			sequenceId = 0x886;
		else
			sequenceId = 0x89B;
		break;
	case 25:
		if (_gnapIdleFacing == 7)
			sequenceId = 0x87A;
		else
			sequenceId = 0x88F;
		break;
	//Skip 26
	//Skip 27
	//Skip 28
	//Skip 29
	default:
		_gnapIdleFacing = facing;
		sequenceId = getGnapSequenceId(gskImpossible, 0, 0);
		break;
	}
	return sequenceId;
}

void GnapEngine::gnapIdle() {
	if (_gnapSequenceId != -1 && _gnapSequenceDatNum == 1 &&
		(_gnapSequenceId == 0x7A6 || _gnapSequenceId == 0x7AA ||
		_gnapSequenceId == 0x832 || _gnapSequenceId == 0x841 ||
		_gnapSequenceId == 0x842 || _gnapSequenceId == 0x8A2 ||
		_gnapSequenceId == 0x833 || _gnapSequenceId == 0x834 ||
		_gnapSequenceId == 0x885 || _gnapSequenceId == 0x7A8 ||
		_gnapSequenceId == 0x831 || _gnapSequenceId == 0x89A)) {
		_gameSys->insertSequence(getGnapSequenceId(gskIdle, 0, 0) | 0x10000, _gnapId,
			makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId,
			kSeqSyncExists, 0, 75 * _gnapX - _gnapGridX, 48 * _gnapY - _gnapGridY);
		_gnapSequenceId = getGnapSequenceId(gskIdle, 0, 0);
		_gnapSequenceDatNum = 1;
	}
}

void GnapEngine::gnapActionIdle(int sequenceId) {
	if (_gnapSequenceId != -1 && ridToDatIndex(sequenceId) == _gnapSequenceDatNum &&
		ridToEntryIndex(sequenceId) == _gnapSequenceId) {
		_gameSys->insertSequence(getGnapSequenceId(gskIdle, 0, 0) | 0x10000, _gnapId,
			makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId,
			kSeqSyncExists, 0, 75 * _gnapX - _gnapGridX, 48 * _gnapY - _gnapGridY);
		_gnapSequenceId = getGnapSequenceId(gskIdle, 0, 0);
		_gnapSequenceDatNum = 1;
	}
}

void GnapEngine::playGnapSequence(int sequenceId) {
	_timers[2] = getRandom(30) + 20;
	_timers[3] = 300;
	gnapIdle();
	// CHECKME: Check the value of the flag
	_gameSys->insertSequence(sequenceId, _gnapId,
		makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId,
		9, 0, 75 * _gnapX - _gnapGridX, 48 * _gnapY - _gnapGridY);
	_gnapSequenceId = ridToEntryIndex(sequenceId);
	_gnapSequenceDatNum = ridToDatIndex(sequenceId);
}

void GnapEngine::playGnapImpossible(int gridX, int gridY) {
	playGnapSequence(getGnapSequenceId(gskImpossible, gridX, gridY) | 0x10000);
}

void GnapEngine::playGnapScratchingHead(int gridX, int gridY) {
	playGnapSequence(getGnapSequenceId(gskScratchingHead, gridX, gridY) | 0x10000);
}

void GnapEngine::playGnapMoan1(int gridX, int gridY) {
	playGnapSequence(getGnapSequenceId(gskMoan1, gridX, gridY) | 0x10000);
}

void GnapEngine::playGnapMoan2(int gridX, int gridY) {
	playGnapSequence(getGnapSequenceId(gskMoan2, gridX, gridY) | 0x10000);
}

void GnapEngine::playGnapBrainPulsating(int gridX, int gridY) {
	playGnapSequence(getGnapSequenceId(gskBrainPulsating, gridX, gridY) | 0x10000);
}

void GnapEngine::playGnapPullOutDevice(int gridX, int gridY) {
	playGnapSequence(getGnapSequenceId(gskPullOutDevice, gridX, gridY) | 0x10000);
}

void GnapEngine::playGnapPullOutDeviceNonWorking(int gridX, int gridY) {
	playGnapSequence(getGnapSequenceId(gskPullOutDeviceNonWorking, gridX, gridY) | 0x10000);
}

void GnapEngine::playGnapUseDevice(int gridX, int gridY) {
	playGnapSequence(getGnapSequenceId(gskUseDevice, gridX, gridY) | 0x10000);
}

void GnapEngine::playGnapIdle(int gridX, int gridY) {
	playGnapSequence(getGnapSequenceId(gskIdle, gridX, gridY) | 0x10000);
}

void GnapEngine::playGnapShowItem(int itemIndex, int gridLookX, int gridLookY) {
	playGnapSequence(getGnapShowSequenceId(itemIndex, gridLookX, gridLookY) | 0x10000);
}

void GnapEngine::playGnapShowCurrItem(int gridX, int gridY, int gridLookX, int gridLookY) {
	if (_platX == gridX && _platY == gridY)
		beaverMakeRoom();
	gnapWalkTo(gridX, gridY, -1, -1, 1);
	playGnapShowItem(_grabCursorSpriteIndex, gridLookX, gridLookY);
}

void GnapEngine::updateGnapIdleSequence() {
	if (_gnapActionStatus < 0) {
		if (_timers[2] > 0) {
			if (_timers[3] == 0) {
				_timers[2] = 60;
				_timers[3] = 300;
				_gnapRandomValue = getRandom(5);
				if (_gnapIdleFacing == 1) {
					switch (_gnapRandomValue) {
					case 0:
						playGnapSequence(0x107A6);
						break;
					case 1:
						playGnapSequence(0x107AA);
						break;
					case 2:
						playGnapSequence(0x10841);
						break;
					default:
						playGnapSequence(0x108A2);
						break;
					}
				} else if (_gnapIdleFacing == 3) {
					if (_gnapRandomValue > 2)
						playGnapSequence(0x10832);
					else
						playGnapSequence(0x10842);
				}
			}
		} else {
			_timers[2] = getRandom(30) + 20;
			if (_gnapIdleFacing == 1) {
				_gameSys->insertSequence(0x107BD, _gnapId,
					makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId,
					kSeqSyncWait, 0, 75 * _gnapX - _gnapGridX, 48 * _gnapY - _gnapGridY);
				_gnapSequenceId = 0x7BD;
				_gnapSequenceDatNum = 1;
			} else if (_gnapIdleFacing == 3) {
				_gameSys->insertSequence(0x107BE, _gnapId,
					makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId,
					kSeqSyncWait, 0, 75 * _gnapX - _gnapGridX, 48 * _gnapY - _gnapGridY);
				_gnapSequenceId = 0x7BE;
				_gnapSequenceDatNum = 1;
			}
		}
	} else {
		_timers[2] = getRandom(30) + 20;
		_timers[3] = 300;
	}
}

void GnapEngine::updateGnapIdleSequence2() {
	if (_gnapActionStatus < 0) {
		if (_timers[2] > 0) {
			if (_timers[3] == 0) {
				_timers[2] = 60;
				_timers[3] = 300;
				if (_gnapIdleFacing == 1) {
					playGnapSequence(0x107AA);
				} else if (_gnapIdleFacing == 3) {
					playGnapSequence(0x10832);
				}
			}
		} else {
			_timers[2] = getRandom(30) + 20;
			if (_gnapIdleFacing == 1) {
				_gameSys->insertSequence(0x107BD, _gnapId,
					makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId,
					kSeqSyncWait, 0, 75 * _gnapX - _gnapGridX, 48 * _gnapY - _gnapGridY);
				_gnapSequenceId = 0x7BD;
				_gnapSequenceDatNum = 1;
			} else if (_gnapIdleFacing == 3) {
				_gameSys->insertSequence(0x107BE, _gnapId,
					makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId,
					kSeqSyncWait, 0, 75 * _gnapX - _gnapGridX, 48 * _gnapY - _gnapGridY);
				_gnapSequenceId = 0x7BE;
				_gnapSequenceDatNum = 1;
			}
		}
	} else {
		_timers[2] = getRandom(30) + 20;
		_timers[3] = 300;
	}
}

bool GnapEngine::testWalk(int animationIndex, int someStatus, int gridX1, int gridY1, int gridX2, int gridY2) {
	if (_mouseClickState._left && someStatus == _gnapActionStatus) {
		_isLeavingScene = false;
		_gameSys->setAnimation(0, 0, animationIndex);
		_gnapActionStatus = -1;
		_beaverActionStatus = -1;
		gnapWalkTo(gridX1, gridY1, -1, -1, 1);
		platypusWalkTo(gridX2, gridY2, -1, -1, 1);
		_mouseClickState._left = false;
		return true;
	}
	return false;
}

void GnapEngine::initGnapPos(int gridX, int gridY, int facing) {
	_timers[2] = 30;
	_timers[3] = 300;
	_gnapX = gridX;
	_gnapY = gridY;
	if (facing <= 0)
		_gnapIdleFacing = 1;
	else
		_gnapIdleFacing = facing;
	if (_gnapIdleFacing == 3) {
		_gnapSequenceId = 0x7B8;
  	} else {
		_gnapSequenceId = 0x7B5;
		_gnapIdleFacing = 1;
	}
	_gnapId = 20 * _gnapY;
	_gnapSequenceDatNum = 1;
	_gameSys->insertSequence(makeRid(1, _gnapSequenceId), 20 * _gnapY,
		0, 0,
		kSeqScale, 0, 75 * _gnapX - _gnapGridX, 48 * _gnapY - _gnapGridY);
}

void GnapEngine::gnapInitBrainPulseRndValue() {
	_gnapBrainPulseRndValue = 2 * getRandom(10);
}

void GnapEngine::gnapUseDeviceOnBeaver() {
	
	playGnapSequence(makeRid(1, getGnapSequenceId(gskPullOutDevice, _platX, _platY)));

	if (_beaverFacing != 0) {
		_gameSys->insertSequence(makeRid(1, 0x7D5), _beaverId,
			makeRid(_beaverSequenceDatNum, _beaverSequenceId), _beaverId,
			kSeqSyncWait, 0, 75 * _platX - _platGridX, 48 * _platY - _platGridY);
		_beaverSequenceId = 0x7D5;
		_beaverSequenceDatNum = 1;
	} else {
		_gameSys->insertSequence(makeRid(1, 0x7D4), _beaverId,
			makeRid(_beaverSequenceDatNum, _beaverSequenceId), _beaverId,
			kSeqSyncWait, 0, 75 * _platX - _platGridX, 48 * _platY - _platGridY);
		_beaverSequenceId = 0x7D4;
		_beaverSequenceDatNum = 1;
	}

	int newSequenceId = getGnapSequenceId(gskUseDevice, 0, 0);
	_gameSys->insertSequence(makeRid(1, newSequenceId), _gnapId,
		makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId,
		kSeqSyncWait, 0, 75 * _gnapX - _gnapGridX, 48 * _gnapY - _gnapGridY);
	_gnapSequenceId = newSequenceId;
	_gnapSequenceDatNum = 1;

}

void GnapEngine::doCallback(int callback) {
	switch (callback) {
	case 8:
		scene08_updateAnimationsCb();
		break;
	case 10:
		scene10_updateAnimationsCb();
		break;
	case 20:
		scene20_updateAnimationsCb();
		break;
	}
}

bool GnapEngine::gnapPlatypusAction(int gridX, int gridY, int platSequenceId, int callback) {
	bool result = false;
	
	if (_gnapActionStatus <= -1 && _beaverActionStatus <= -1) {
		_gnapActionStatus = 100;
		if (isPointBlocked(_platX + gridX, _platY + gridY) && (_platX + gridX != _gnapX || _platY + gridY != _gnapY))
			platypusWalkStep();
		if (!isPointBlocked(_platX + gridX, _platY + gridY) && (_platX + gridX != _gnapX || _platY + gridY != _gnapY)) {
			gnapWalkTo(_platX + gridX, _platY + gridY, 0, 0x107B9, 1);
			while (_gameSys->getAnimationStatus(0) != 2) {
				updateMouseCursor();
				doCallback(callback);
				gameUpdateTick();
			}
			_gameSys->setAnimation(0, 0, 0);
			if (_platX + gridX == _gnapX && _platY + gridY == _gnapY) {
				_gameSys->setAnimation(platSequenceId, _beaverId, 1);
				playBeaverSequence(platSequenceId);
				while (_gameSys->getAnimationStatus(1) != 2) {
					updateMouseCursor();
					doCallback(callback);
					gameUpdateTick();
				}
				result = true;
			}
		}
		_gnapActionStatus = -1;
	}
	return result;
}

void GnapEngine::gnapKissPlatypus(int callback) {
	if (gnapPlatypusAction(-1, 0, 0x107D1, callback)) {
		_gnapActionStatus = 100;
		_gameSys->setAnimation(0, 0, 1);
		_gameSys->setAnimation(0x10847, _gnapId, 0);
		_gameSys->insertSequence(0x10847, _gnapId,
			makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId,
			kSeqSyncWait, 0, 15 * (5 * _gnapX - 20) - (21 - _gridMinX), 48 * (_gnapY - 6) - (146 - _gridMinY));
		_gnapSequenceDatNum = 1;
		_gnapSequenceId = 0x847;
		_gameSys->insertSequence(0x107CB, _beaverId,
			makeRid(_beaverSequenceDatNum, _beaverSequenceId), _beaverId,
			kSeqSyncWait, getSequenceTotalDuration(0x10847), 75 * _platX - _platGridX, 48 * _platY - _platGridY);
		_beaverSequenceDatNum = 1;
		_beaverSequenceId = 0x7CB;
		_beaverFacing = 0;
		playGnapSequence(0x107B5);
		while (_gameSys->getAnimationStatus(0) != 2) {
			updateMouseCursor();
			doCallback(callback);
			gameUpdateTick();
		}
		_gameSys->setAnimation(0, 0, 0);
		_gnapActionStatus = -1;
	} else {
		playGnapSequence(getGnapSequenceId(gskScratchingHead, _platX, _platY) | 0x10000);
	}
}

void GnapEngine::gnapUseJointOnPlatypus() {
	setGrabCursorSprite(-1);
	if (gnapPlatypusAction(1, 0, 0x107C1, 0)) {
		_gnapActionStatus = 100;
		_gameSys->setAnimation(0, 0, 1);
		_gameSys->setAnimation(0x10876, _beaverId, 0);
		_gameSys->insertSequence(0x10875, _gnapId,
			makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId,
			kSeqSyncWait, 0, 15 * (5 * _gnapX - 30), 48 * (_gnapY - 7));
		_gnapSequenceDatNum = 1;
		_gnapSequenceId = 0x875;
		_gameSys->insertSequence(0x10876, _beaverId,
			_beaverSequenceId | (_beaverSequenceDatNum << 16), _beaverId,
			kSeqSyncWait, 0, 15 * (5 * _platX - 25), 48 * (_platY - 7));
		_beaverSequenceDatNum = 1;
		_beaverSequenceId = 0x876;
		_beaverFacing = 0;
		playGnapSequence(0x107B5);
		gnapWalkStep();
		while (_gameSys->getAnimationStatus(0) != 2) {
			updateMouseCursor();
			gameUpdateTick();
		}
		_gameSys->setAnimation(0, 0, 0);
		_gnapActionStatus = -1;
	} else {
		playGnapSequence(getGnapSequenceId(gskScratchingHead, _platX, _platY) | 0x10000);
	}
}

void GnapEngine::gnapUseDisguiseOnPlatypus() {
	_gameSys->setAnimation(0x10846, _gnapId, 0);
	playGnapSequence(0x10846);
	while (_gameSys->getAnimationStatus(0) != 2)
		gameUpdateTick();
	_newSceneNum = 47;
	_isLeavingScene = true;
	_sceneDone = true;
	setFlag(10);
}

int GnapEngine::getBeaverSequenceId(int kind, int gridX, int gridY) {
	int sequenceId;

	// TODO kind is always 0, remove that parameter
	if (kind != 0)
		return 0;

	if (gridX > 0 && gridY > 0) {
		if (gridX < _platX) {
			sequenceId = 0x7CC;
			_beaverFacing = 4;
		} else {
			sequenceId = 0x7CB;
			_beaverFacing = 0;
		}
	} else if (_beaverFacing != 0) {
		sequenceId = 0x7CC;
		_beaverFacing = 4;
	} else {
		sequenceId = 0x7CB;
		_beaverFacing = 0;
	}
	return sequenceId | 0x10000;
}

void GnapEngine::playBeaverSequence(int sequenceId) {
	// CHECKME: Check the value of the flag
	_gameSys->insertSequence(sequenceId, _beaverId,
		makeRid(_beaverSequenceDatNum, _beaverSequenceId), _beaverId,
		9, 0, 75 * _platX - _platGridX, 48 * _platY - _platGridY);
	_beaverSequenceId = ridToEntryIndex(sequenceId);
	_beaverSequenceDatNum = ridToDatIndex(sequenceId);
}

void GnapEngine::updateBeaverIdleSequence() {
	if (_beaverActionStatus < 0 && _gnapActionStatus < 0) {
		if (_timers[0] > 0) {
			if (_timers[1] == 0) {
				_timers[1] = getRandom(20) + 30;
				_gnapRandomValue = getRandom(10);
				if (_beaverFacing != 0) {
					if (_gnapRandomValue != 0 || _beaverSequenceId != 0x7CA) {
						if (_gnapRandomValue != 1 || _beaverSequenceId != 0x7CA)
							playBeaverSequence(0x107CA);
						else
							playBeaverSequence(0x10845);
					} else {
						playBeaverSequence(0x107CC);
					}
				} else if (_gnapRandomValue != 0 || _beaverSequenceId != 0x7C9) {
					if (_gnapRandomValue != 1 || _beaverSequenceId != 0x7C9) {
						if (_gnapRandomValue != 2 || _beaverSequenceId != 0x7C9)
							playBeaverSequence(0x107C9);
						else
							playBeaverSequence(0x108A4);
					} else {
						playBeaverSequence(0x10844);
					}
				} else {
					playBeaverSequence(0x107CB);
				}
			}
		} else {
			_timers[0] = getRandom(75) + 75;
			beaverMakeRoom();
		}
	} else {
		_timers[0] = 100;
		_timers[1] = 35;
	}
}

void GnapEngine::beaverSub426234() {
	if (_beaverActionStatus < 0 && _gnapActionStatus < 0) {
		if (_timers[0]) {
			if (!_timers[1]) {
				_timers[1] = getRandom(20) + 30;
				_gnapRandomValue = getRandom(10);
				if (_beaverFacing != 0) {
					if (_gnapRandomValue >= 2 || _beaverSequenceId != 0x7CA)
						playBeaverSequence(0x107CA);
					else
						playBeaverSequence(0x107CC);
				} else {
					if (_gnapRandomValue >= 2 || _beaverSequenceId != 0x7C9) {
						playBeaverSequence(0x107C9);
					} else {
						playBeaverSequence(0x107CB);
					}
				}
			}
		} else {
			_timers[0] = getRandom(75) + 75;
			beaverMakeRoom();
		}
	} else {
		_timers[0] = 100;
		_timers[1] = 35;
	}
}

void GnapEngine::initBeaverPos(int gridX, int gridY, int facing) {
	_timers[0] = 50;
	_timers[1] = 20;
	_platX = gridX;
	_platY = gridY;
	if (facing <= 0)
		_beaverFacing = 0;
	else
		_beaverFacing = facing;
	if (_beaverFacing == 4) {
		_beaverSequenceId = 0x7D1;
	} else {
		_beaverSequenceId = 0x7C1;
		_beaverFacing = 0;
	}
	_beaverId = 20 * _platY;
	_beaverSequenceDatNum = 1;
	_gameSys->insertSequence(makeRid(1, _beaverSequenceId), 20 * _platY,
		0, 0,
		kSeqScale, 0, 75 * _platX - _platGridX, 48 * _platY - _platGridY);
}

////////////////////////////////////////////////////////////////////////////////

void GnapEngine::initGlobalSceneVars() {

	// Scene 1
	_s01_pigsIdCtr = 0;
	_s01_smokeIdCtr = 0;
	_s01_spaceshipSurface = 0;
	
	// Scene 2
	_s02_truckGrillCtr = 0;
	
	// Scene 3
	_s03_nextPlatSequenceId = -1;
	_s03_platypusScared = false;
	_s03_platypusHypnotized = false;
	
	// Scene 4
	_s04_dogIdCtr = 0;
	//_s04_triedWindow = true;//??
	_s04_triedWindow = false;
	
	// Scene 5
	_s05_nextChickenSequenceId = -1;
	
	// Scene 6
	_s06_nextPlatSequenceId = -1;
	
	// Scene 11
	_s11_billardBallCtr = 0;
	
	// Scene 13
	_s13_backToiletCtr = -1;
	
	// Scene 17
	_s17_platTryGetWrenchCtr = 0;
	_s17_wrenchCtr = 2;
	_s17_nextCarWindowSequenceId = -1;
	_s17_nextWrenchSequenceId = -1;
	_s17_canTryGetWrench = true;
	_s17_platPhoneCtr = 0;
	_s17_nextPhoneSequenceId = -1;
	_s17_currPhoneSequenceId = -1;
	
	// Scene 18
	_s18_garbageCanPos = 8;
	_s18_platPhoneCtr = 0;
	_s18_platPhoneIter = 0;
	_s18_nextPhoneSequenceId = -1;
	_s18_currPhoneSequenceId = -1;
	
	// Scene 19
	_s19_toyGrabCtr = 0;
	_s19_pictureSurface = 0;
	_s19_shopAssistantCtr = 0;

	// Scene 20
	_s20_stonerGuyCtr = 3;
	_s20_stonerGuyShowingJoint = false;
	_s20_groceryStoreGuyCtr = 0;
	
	// Scene 22
	_s22_caughtBefore = false;
	_s22_cashierCtr = 3;
	
	// Scene 31
	_s31_beerGuyDistracted = false;
	_s31_clerkMeasureMaxCtr = 3;

	// Scene 50
	_s50_timesPlayed = 0;
	_s50_timesPlayedModifier = 0;
	_s50_attackCounter = 0;
	_s50_leftTongueEnergyBarPos = 10;
	_s50_leftTongueNextIdCtr = 0;
	_s50_rightTongueEnergyBarPos = 10;
	_s50_rightTongueNextIdCtr = 0;
	
	// Scene 52
	_s52_gameScore = 0;
	_s52_aliensInitialized = false;
	_s52_alienDirection = 0;
	_s52_soundToggle = false;
	
	// Scene 53
	_s53_callsMadeCtr = 0;
	_s53_callsRndUsed = 0;

	// Toy UFO
	_toyUfoId = 0;
	_toyUfoActionStatus = -1;
	_toyUfoX = 0;
	_toyUfoY = 50;

}

bool GnapEngine::sceneXX_sub_4466B1() {

	if (isKeyStatus1(Common::KEYCODE_ESCAPE)) {
		clearKeyStatus1(Common::KEYCODE_ESCAPE);
		clearKeyStatus1(Common::KEYCODE_UP);
		clearKeyStatus1(Common::KEYCODE_RIGHT);
		clearKeyStatus1(Common::KEYCODE_LEFT);
		clearKeyStatus1(Common::KEYCODE_p);
		return true;
	}

	if (isKeyStatus1(Common::KEYCODE_p)) {
		clearKeyStatus1(Common::KEYCODE_p);
		pauseGame();
		updatePause();
	}

	return false;

}

void GnapEngine::sceneXX_playRandomSound(int timerIndex) {
	if (!_timers[timerIndex]) {
		_timers[timerIndex] = getRandom(40) + 50;
		_gnapRandomValue = getRandom(4);
		switch (_gnapRandomValue) {
		case 0:
			playSound(0x1091B, 0);
			break;
		case 1:
			playSound(0x10921, 0);
			break;
		case 2:
			playSound(0x10927, 0);
			break;
		case 3:
			playSound(0x1091D, 0);
			break;
		}
	}
}

void GnapEngine::playSequences(int fullScreenSpriteId, int sequenceId1, int sequenceId2, int sequenceId3) {
	setGrabCursorSprite(-1);
	_gameSys->setAnimation(sequenceId2, _gnapId, 0);
	_gameSys->insertSequence(sequenceId2, _gnapId,
		makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId,
		kSeqSyncWait, 0, 15 * (5 * _gnapX - 25), 48 * (_gnapY - 8));
	_gnapSequenceId = sequenceId2;
	_gnapSequenceDatNum = 0;
	while (_gameSys->getAnimationStatus(0) != 2)
		gameUpdateTick();
	hideCursor();
	addFullScreenSprite(fullScreenSpriteId, 255);
	_gameSys->setAnimation(sequenceId1, 256, 0);
	_gameSys->insertSequence(sequenceId1, 256, 0, 0, kSeqNone, 0, 0, 0);
	while (_gameSys->getAnimationStatus(0) != 2)
		gameUpdateTick();
	_gameSys->setAnimation(sequenceId3, _gnapId, 0);
	_gameSys->insertSequence(sequenceId3, _gnapId,
		makeRid(_gnapSequenceDatNum, _gnapSequenceId), _gnapId,
		kSeqSyncWait, 0, 15 * (5 * _gnapX - 25), 48 * (_gnapY - 8));
	removeFullScreenSprite();
	showCursor();
	_gnapSequenceId = sequenceId3;
}

// Scene 0

int GnapEngine::scene00_init() {
	return 0x37C;
}

void GnapEngine::toyUfoSetStatus(int flagNum) {
	clearFlag(16);
	clearFlag(17);
	clearFlag(18);
	clearFlag(19);
	setFlag(flagNum);
}

int GnapEngine::toyUfoGetSequenceId() {
	if (isFlag(16))
		return 0x84E;
	if (isFlag(17))
		return 0x84B;
	if (isFlag(18))
		return 0x84D;
	if (isFlag(19))
		return 0x84C;
	return 0x84E;
}

bool GnapEngine::toyUfoCheckTimer() {
	if (!isFlag(12) || isFlag(18) || _timers[9] ||
		_toyUfoSequenceId == 0x870 || _toyUfoSequenceId == 0x871 || _toyUfoSequenceId == 0x872 || _toyUfoSequenceId == 0x873)
		return false;
	_sceneDone = true;
	_newSceneNum = 41;
	return true;
}

void GnapEngine::toyUfoFlyTo(int destX, int destY, int minX, int maxX, int minY, int maxY, int animationIndex) {
	GridStruct v16[34];
	
	if (destX == -1)
		destX = _leftClickMouseX;
	
	if (destY == -1)
		destY = _leftClickMouseY;

	//CHECKME
	
	int clippedDestX = CLIP(destX, minX, maxX);
	int clippedDestY = CLIP(destY, minY, maxY);
	int dirX, dirY; // 0, -1 or 1

	if (clippedDestX == _toyUfoX)
		dirX = 0;
	else
		dirX = (clippedDestX - _toyUfoX) / ABS(clippedDestX - _toyUfoX);
	
	if (clippedDestY == _toyUfoY)
		dirY = 0;
	else
		dirY = (clippedDestY - _toyUfoY) / ABS(clippedDestY - _toyUfoY);
	
	int deltaX = ABS(clippedDestX - _toyUfoX);
	int deltaY = ABS(clippedDestY - _toyUfoY);

	int i = 0;
	if (deltaY > deltaX) {
		int v15 = 32;
		int v22 = deltaY / v15;
		int v14 = 0;
		while (v14 < deltaY && i < 34) {
			if (v22 - 5 >= i) {
				v15 = MIN(36, 8 * i + 8);
			} else {
				v15 = MAX(6, v15 - 3);
			}
			v14 += v15;
			v16[i].gridX1 = _toyUfoX + dirX * deltaX * v14 / deltaY;
			v16[i].gridY1 = _toyUfoY + dirY * v14;
			++i;
		}
	} else {
		int v17 = 36;
		int v22 = deltaX / v17;
		int v14 = 0;
		while (v14 < deltaX && i < 34) {
			if (v22 - 5 >= i) {
				v17 = MIN(38, 8 * i + 8);
			} else {
				v17 = MAX(6, v17 - 3);
			}
			v14 += v17;
			v16[i].gridX1 = _toyUfoX + dirX * v14;
			v16[i].gridY1 = _toyUfoY + dirY * deltaY * v14 / deltaX;
			++i;
		}
	}

	int v21 = i - 1;
	
	_toyUfoX = clippedDestX;
	_toyUfoY = clippedDestY;
	
//	debug("v21: %d", v21);
	
	if (i - 1 > 0) {
		int seqId;
		if (isFlag(16))
			seqId = 0x867;
		else if (isFlag(17))
			seqId = 0x84F;
		else if (isFlag(18))
			seqId = 0x85F;
		else if (isFlag(19))
			seqId = 0x857;
		v16[0].sequenceId = seqId;
		v16[0].id = 0;
		_gameSys->insertSequence(seqId | 0x10000, 0,
			_toyUfoSequenceId | 0x10000, _toyUfoId,
			kSeqSyncWait, 0, v16[0].gridX1 - 365, v16[0].gridY1 - 128);
		for (i = 1; i < v21; ++i) {
			v16[i].sequenceId = seqId + (i % 8);
			v16[i].id = i;
			_gameSys->insertSequence(v16[i].sequenceId | 0x10000, v16[i].id,
				v16[i - 1].sequenceId | 0x10000, v16[i - 1].id,
				kSeqSyncWait, 0,
				v16[i].gridX1 - 365, v16[i].gridY1 - 128);
		}
	
		_toyUfoSequenceId = v16[v21 - 1].sequenceId;
		_toyUfoId = v16[v21 - 1].id;
	
		if (animationIndex >= 0)
			_gameSys->setAnimation(_toyUfoSequenceId | 0x10000, _toyUfoId, animationIndex);
	
	}

}

// Scene 99

int GnapEngine::cutscene_init() {
	return -1;
}

void GnapEngine::cutscene_run() {
	
	int itemIndex = 0;
	int soundId = -1;
	int volume = 100;
	int duration = 0;
	bool skip = false;

	if (_prevSceneNum == 2) {
		soundId = 0x36B;
		duration = MAX(1, 300 / getSequenceTotalDuration(_s99_dword_47F370[_s99_itemsCount - 1]));//CHECKME
		_timers[0] = 0;
	}

	if (soundId != -1)
		playSound(soundId, 0);

	hideCursor();

	_gameSys->drawSpriteToBackground(0, 0, _s99_dword_47F2F0[0]);

	for (int j = 0; j < _s99_dword_47F330[0]; ++j)
		_gameSys->insertSequence(_s99_dword_47F370[j], j + 2, 0, 0, kSeqNone, 0, 0, 0);
	_gameSys->setAnimation(_s99_dword_47F370[0], 2, 0);
		
	clearKeyStatus1(Common::KEYCODE_ESCAPE);
	clearKeyStatus1(Common::KEYCODE_SPACE);
	clearKeyStatus1(29);

	_mouseClickState._left = false;
	
	int v1 = 0;
	while (!_sceneDone) {

		gameUpdateTick();

		if (_gameSys->getAnimationStatus(0) == 2 || skip) {
			skip = false;
			_gameSys->requestClear2(false);
			_gameSys->requestClear1();
			_gameSys->setAnimation(0, 0, 0);
			v1 += _s99_dword_47F330[itemIndex++];
			if (itemIndex >= _s99_itemsCount) {
				_sceneDone = true;
			} else {
				for (int m = 0; m < _s99_dword_47F330[itemIndex]; ++m)
					_gameSys->insertSequence(_s99_dword_47F370[v1 + m], m + 2, 0, 0, kSeqNone, 0, 0, 0);
				_gameSys->drawSpriteToBackground(0, 0, _s99_dword_47F2F0[itemIndex]);
				_gameSys->setAnimation(_s99_dword_47F370[v1], 2, 0);
			}
		}

		if (isKeyStatus1(Common::KEYCODE_ESCAPE) || isKeyStatus1(Common::KEYCODE_SPACE) || isKeyStatus1(29)) {
			clearKeyStatus1(Common::KEYCODE_ESCAPE);
			clearKeyStatus1(Common::KEYCODE_SPACE);
			clearKeyStatus1(29);
			if (_s99_canSkip[itemIndex] & 1)
				skip = true;
			else
				_sceneDone = true;
		}

		if (!_timers[0] && itemIndex == _s99_itemsCount - 1) {
			_timers[0] = 2;
			volume = MAX(1, volume - duration);
			setSoundVolume(soundId, volume);
		}
		
	}

	if (soundId != -1)
		stopSound(soundId);
	
}

} // End of namespace Gnap
