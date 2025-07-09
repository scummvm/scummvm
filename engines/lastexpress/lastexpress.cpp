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

#include "lastexpress/lastexpress.h"

#include "lastexpress/game/logic.h"
#include "lastexpress/data/gold_archive.h"

#include "lastexpress/menu/menu.h"
#include "lastexpress/menu/clock.h"

#include "lastexpress/sound/sound.h"
#include "lastexpress/sound/subtitle.h"

#include "lastexpress/graphics.h"
#include "lastexpress/helpers.h"

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/timer.h"
#include "common/compression/installshield_cab.h"

#include "engines/util.h"
#include "engines/advancedDetector.h"

namespace LastExpress {

LastExpressEngine::LastExpressEngine(OSystem *syst, const ADGameDescription *gd) :
	Engine(syst), _gameDescription(gd),
	_debugger(nullptr), _random("lastexpress") {
	// Setup mixer
	Engine::syncSoundSettings();

	// Adding the default directories
	const Common::FSNode gameDataDir(ConfMan.getPath("path"));

	if (isGoldEdition()) {
		SearchMan.addSubDirectoryMatching(gameDataDir, "roms");
	} else {
		SearchMan.addSubDirectoryMatching(gameDataDir, "data");
	}

	_soundMutex = new Common::Mutex();
}

LastExpressEngine::~LastExpressEngine() {
	SAFE_DELETE(_graphicsMan);
	SAFE_DELETE(_spriteMan);
	SAFE_DELETE(_otisMan);
	SAFE_DELETE(_subtitleMan);
	SAFE_DELETE(_archiveMan);
	SAFE_DELETE(_msgMan);
	SAFE_DELETE(_nisMan);
	SAFE_DELETE(_soundMan);
	SAFE_DELETE(_logicMan);
	SAFE_DELETE(_menu);
	SAFE_DELETE(_saveMan);
	SAFE_DELETE(_clock);
	SAFE_DELETE(_vcr);
	SAFE_DELETE(_soundMutex);
	SAFE_DELETE(_savegame);
	SAFE_DELETE(_memMan);

	//_debugger is deleted by Engine

	// Zero passed pointers
	_gameDescription = nullptr;
}

void LastExpressEngine::startUp() {
	getArchiveManager()->initHPFS();
	getMemoryManager()->initMem();

	getGraphicsManager()->clear(getGraphicsManager()->_screenSurface, 0, 0, 640, 480);
	getGraphicsManager()->clear(getGraphicsManager()->_backBuffer, 0, 0, 640, 480);
	getGraphicsManager()->clear(getGraphicsManager()->_frontBuffer, 0, 0, 640, 480);

	getVCR()->shuffleGames();
	getArchiveManager()->loadMice();
	getGraphicsManager()->setMouseDrawable(false);
	initGameData();
	getSubtitleManager()->initSubtitles();
	getLogicManager()->loadTrain(isDemo() ? 2 : 1);
}

void LastExpressEngine::shutDown() {
	getMemoryManager()->releaseMemory();
	getArchiveManager()->shutDownHPFS();
}

Common::String LastExpressEngine::getTargetName() const {
	return _targetName;
}

void LastExpressEngine::soundTimerHandler(void *refCon) {
	LastExpressEngine *engine = (LastExpressEngine *)refCon;

	if (!engine)
		return;

	Common::StackLock lock(*engine->_soundMutex);

	if ((engine->_soundFrameCounter & 1) != 0) {
		engine->getSoundManager()->soundDriverCopyBuffersToDevice();
	}

	if ((engine->_soundFrameCounter & 3) == 0) {
		if (engine->getEventTickInternal()) {
			engine->_soundFrameCounter++;

			return;
		}

		engine->getMessageManager()->addEvent(kEventChannelTimer, 0, 0, 0);
	}

	engine->_soundFrameCounter++;
}

Common::Error LastExpressEngine::run() {
	// Allow HD.HPF to be read directly from the InstallShield archive
	if (isCompressed()) {
		Common::Archive *cabinet = Common::makeInstallShieldArchive("data");
		if (cabinet) {
			SearchMan.add("data1.cab", cabinet);
		}
	}

	// Initialize the graphics
	const Graphics::PixelFormat dataPixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
	initGraphics(640, 480, &dataPixelFormat);

	// We do not support color conversion
	if (_system->getScreenFormat() != dataPixelFormat)
		return Common::kUnsupportedColorMode;

	// Create debugger. It requires GFX to be initialized
	_debugger = new Debugger(this);
	setDebugger(_debugger);

	if (gDebugLevel >= 3)
		DebugMan.enableDebugChannel(kDebugConsole);

	// Graphics manager
	_graphicsMan = new GraphicsManager(this);

	// Animation system
	_spriteMan = new SpriteManager(this);
	_otisMan = new OtisManager(this);

	// Archive manager
	if (isGoldEdition()) {
		_archiveMan = new GoldArchiveManager(this);
	} else {
		_archiveMan = new ArchiveManager(this);
	}

	// Memory manager
	_memMan = new MemoryManager(this);

	// Message manager
	_msgMan = new MessageManager(this);

	// NIS manager
	_nisMan = new NISManager(this);

	// Sound&subtitles manager
	_soundMan = new SoundManager(this);
	_subtitleMan = new SubtitleManager(this);

	// Logic manager
	_logicMan = new LogicManager(this);

	// Menu
	_menu = new Menu(this);

	// Save manager
	_saveMan = new SaveManager(this);

	// Clock
	_clock = new Clock(this);

	// VCR
	_vcr = new VCR(this);

	startUp();

	getGraphicsManager()->initLuminosityValues(dataPixelFormat.rMax() << 11, dataPixelFormat.gMax() << 5, dataPixelFormat.bMax() << 0);

#ifdef USE_IMGUI
	ImGuiCallbacks callbacks;
	callbacks.init = onImGuiInit;
	callbacks.render = onImGuiRender;
	callbacks.cleanup = onImGuiCleanup;
	_system->setImGuiCallbacks(callbacks);
#endif

	getMessageManager()->setEventHandle(kEventChannelEngine, &LastExpressEngine::engineEventHandlerWrapper);

	getTimerManager()->installTimerProc(soundTimerHandler, 17000, this, "LastExpressEngine");

	getGraphicsManager()->newMouseLoc();
	getArchiveManager()->loadMice();
	getVCR()->loadSettings();
	getMenu()->doEgg(false, kSavegameTypeIndex, 0);

	while (!shouldQuit()) {
		do {
			getSoundManager()->soundThread();
			getSubtitleManager()->subThread();
		} while (getMessageManager()->process());

		// Originally the game waited for at most four frames (17 * 4 ms).
		// Since the game relies on the sound timer for actual engine pacing,
		// we can reduce the time to 5 milliseconds of wait time, fetching
		// input in the meanwhile, making the cursor a lot smoother.
		waitForTimer(5);
	}

	bool haveEvent = true;
	while (_pendingExitEvent && haveEvent) {
		haveEvent = getMessageManager()->process();
	}

	getSoundManager()->destroyAllSound();

	getTimerManager()->removeTimerProc(soundTimerHandler);

	shutDown();

#ifdef USE_IMGUI
	_system->setImGuiCallbacks(ImGuiCallbacks());
#endif

	return Common::kNoError;
}

void LastExpressEngine::initGameData() {
	Sprite eraser;

	eraser.rect.top = 0;
	eraser.rect.left = 80;
	eraser.rect.right = 559;
	eraser.rect.bottom = 479;

	getSpriteManager()->queueErase(&eraser);

	for (int i = 0; i < 1000; i++) {
		getLogicManager()->_blockedViews[i] = 0;
	}

	for (int i = 0; i < 16; i++) {
		getLogicManager()->_blockedX[i] = 0;
		getLogicManager()->_softBlockedX[i] = 0;
	}

	for (int i = 0; i < 512; i++) {
		getLogicManager()->_doneNIS[i] = 0;
	}

	for (int i = 0; i < 128; i++) {
		getLogicManager()->_globals[i] = 0;
	}

	for (int i = 0; i < 128; i++) {
		getMessageManager()->_autoMessages[i].receiver = 0;
		getMessageManager()->_autoMessages[i].action = 0;
		getMessageManager()->_autoMessages[i].sender = 0;
		getMessageManager()->_autoMessages[i].param = 0;
	}

	for (int i = 0; i < 40; i++) {
		(_characters->characters[i]).reset();
		(_characters->characters[i]).walkStepSize = 30;	
	}

	for (int i = 0; i < 128; i++) {
		getLogicManager()->_doors[i].status = 0;
		getLogicManager()->_doors[i].who = 0;
		getLogicManager()->_doors[i].windowCursor = 10;
		getLogicManager()->_doors[i].handleCursor = 9;
		getLogicManager()->_doors[i].model = 0;
	}

	for (int i = 0; i < 32; i++) {
		getLogicManager()->_items[i].mnum = 0;
		getLogicManager()->_items[i].haveIt = 0;
		getLogicManager()->_items[i].floating = 0;
		getLogicManager()->_items[i].inPocket = 1;
	}

	getLogicManager()->_items[kItemMatchBox].mnum = 16;
	getLogicManager()->_items[kItemTelegram].mnum = 17;
	getLogicManager()->_items[kItemPassengerList].mnum = 18;
	getLogicManager()->_items[kItemArticle].mnum = 19;
	getLogicManager()->_items[kItemScarf].mnum = 20;
	getLogicManager()->_items[kItemPaper].mnum = 21;
	getLogicManager()->_items[kItemParchemin].mnum = 22;
	getLogicManager()->_items[kItemMatch].mnum = 23;
	getLogicManager()->_items[kItemWhistle].mnum = 24;
	getLogicManager()->_items[kItemKey].mnum = 25;
	getLogicManager()->_items[kItemBomb].mnum = 26;
	getLogicManager()->_items[kItemFirebird].mnum = 27;
	getLogicManager()->_items[kItemBriefcase].mnum = 28;
	getLogicManager()->_items[kItemCorpse].mnum = 29;
	getLogicManager()->_items[kItemMatchBox].useable = 1;
	getLogicManager()->_items[kItemMatch].useable = 1;
	getLogicManager()->_items[kItemTelegram].useable = 1;
	getLogicManager()->_items[kItemWhistle].useable = 1;
	getLogicManager()->_items[kItemKey].useable = 1;
	getLogicManager()->_items[kItemFirebird].useable = 1;
	getLogicManager()->_items[kItemBriefcase].useable = 1;
	getLogicManager()->_items[kItemCorpse].useable = 1;
	getLogicManager()->_items[kItemPassengerList].useable = 1;
	getLogicManager()->_items[kItem2].inPocket = 0;
	getLogicManager()->_items[kItem3].inPocket = 0;
	getLogicManager()->_items[kItem5].inPocket = 0;
	getLogicManager()->_items[kItem7].inPocket = 0;
	getLogicManager()->_items[kItem9].inPocket = 0;
	getLogicManager()->_items[kItemBeetle].inPocket = 0;
	getLogicManager()->_items[kItem11].inPocket = 0;
	getLogicManager()->_items[kItem17].inPocket = 0;
	getLogicManager()->_items[kItemFirebird].inPocket = 0;
	getLogicManager()->_items[kItemBriefcase].inPocket = 0;
	getLogicManager()->_items[kItemCorpse].inPocket = 0;
	getLogicManager()->_items[kItemGreenJacket].inPocket = 0;
	getLogicManager()->_items[kItem22].inPocket = 0;
	getLogicManager()->_items[kItemMatchBox].closeUp = 31;
	getLogicManager()->_items[kItemTelegram].closeUp = 32;
	getLogicManager()->_items[kItemPassengerList].closeUp = 33;
	getLogicManager()->_items[kItemScarf].closeUp = 34;
	getLogicManager()->_items[kItemParchemin].closeUp = 35;
	getLogicManager()->_items[kItemFirebird].closeUp = 38;
	getLogicManager()->_items[kItemBriefcase].closeUp = 39;
	getLogicManager()->_items[kItemPaper].closeUp = 37;
	getLogicManager()->_items[kItemArticle].closeUp = 36;
	getLogicManager()->_items[kItemTelegram].haveIt = 1;
	getLogicManager()->_items[kItemArticle].haveIt = 1;
	getLogicManager()->_globals[kGlobalCathIcon] = isDemo() ? 34: 32;
	getLogicManager()->_globals[kGlobalChapter] = isDemo() ? 3 : 1;
	getLogicManager()->_lastSavegameSessionTicks = 0;
	getLogicManager()->_realTime = 0;
	getLogicManager()->_closeUp = 0;
	getLogicManager()->_nodeReturn = 0;
	getLogicManager()->_nodeReturn2 = 0;
	getLogicManager()->_activeItem = 0;
	getLogicManager()->_gameTime = isDemo() ? 2241000 : 1037700;
	getLogicManager()->_timeSpeed = 3;
	getLogicManager()->_activeNode = 40;
}

void LastExpressEngine::startNewGame() {
	initGameData();
	getLogicManager()->CONS_All(true, kCharacterCath);
}

void LastExpressEngine::engineEventHandler(Event *event) {
	switch (event->flags) {
	case 1: // Quit signal request
		getNISManager()->abortNIS();
		abortFight();
		abortCredits();

		if (_pendingExitEvent) {
			if (getMenu()->isShowingMenu() || (getMessageManager()->getEventHandle(1) == &LastExpressEngine::nodeStepMouseWrapper && !getVCR()->isVirgin(_currentGameFileColorId))) {
				getMenu()->doEgg(true, 0, 0); // Save!
				_pendingExitEvent = false; // We're done, we can quit
			} else {
				getMessageManager()->addEvent(kEventChannelEngine, 0, 0, 2); // Give the engine the actual chance to abort NIS, fights and credits by running process()
				_pendingExitEvent = true;
			}
		}

		break;
	case 2: // Quit signal handler
		getNISManager()->abortNIS();
		abortFight();
		abortCredits();

		if (_pendingExitEvent && !getMenu()->isShowingMenu()) {
			if (getMessageManager()->getEventHandle(1) != &LastExpressEngine::nodeStepMouseWrapper || getVCR()->isVirgin(_currentGameFileColorId)) {
				_pendingExitEvent = true;
			} else {
				getMenu()->doEgg(true, 0, 0); // Save!
				_pendingExitEvent = false; // We're done, we can quit
			}
		}

		break;
	case 3:
		// Originally handled WM_PAINT events and triggered burstBox on the screen rectangle
		break;
	case 4:
	{
		bool oldCanDrawMouse = getGraphicsManager()->canDrawMouse();
		getGraphicsManager()->setMouseDrawable(false);
		getGraphicsManager()->burstMouseArea();
		getGraphicsManager()->setMouseDrawable(oldCanDrawMouse);
		break;
	}
	default:
		break;
	}
}

int32 LastExpressEngine::getSoundFrameCounter() {
	Common::StackLock lock(*_soundMutex);
	return _soundFrameCounter;
}

bool LastExpressEngine::getEventTickInternal() {
	Common::StackLock lock(*_soundMutex);
	return _eventTickInternal;
}

void LastExpressEngine::setEventTickInternal(bool flag) {
	Common::StackLock lock(*_soundMutex);
	_eventTickInternal = flag;
}

void LastExpressEngine::mouseSetLeftClicked(bool hasClicked) {
	Common::StackLock lock(*_soundMutex);
	_mouseHasLeftClicked = hasClicked;
}

void LastExpressEngine::mouseSetRightClicked(bool hasClicked) {
	Common::StackLock lock(*_soundMutex);
	_mouseHasRightClicked = hasClicked;
}

bool LastExpressEngine::mouseHasLeftClicked() {
	Common::StackLock lock(*_soundMutex);
	return _mouseHasLeftClicked;
}

bool LastExpressEngine::mouseHasRightClicked() {
	Common::StackLock lock(*_soundMutex);
	return _mouseHasRightClicked;
}

void LastExpressEngine::waitForTimer(int millis) {
	handleEvents();
	_system->delayMillis(millis);
}

bool LastExpressEngine::handleEvents() {
	// Handle input
	Common::Event ev;
	int32 curFlags = 0;
	bool eventWillUpdateScreen = false;

	while (_eventMan->pollEvent(ev)) {
		switch (ev.type) {

		case Common::EVENT_KEYDOWN:
			switch (ev.kbd.keycode) {
			case Common::KEYCODE_F4:
				if (_navigationEngineIsRunning && gDebugLevel >= 3)
					getLogicManager()->doF4();

				break;
			default:
				break;
			}

			break;
		case Common::EVENT_LBUTTONDOWN:
			_systemEventLeftMouseDown = true;
			curFlags |= kMouseFlagLeftButton;

			if (_systemEventRightMouseDown)
				curFlags |= kMouseFlagRightButton;

			getMessageManager()->addEvent(kEventChannelMouse, ev.mouse.x, ev.mouse.y, curFlags);
			eventWillUpdateScreen = true;
			break;
		case Common::EVENT_LBUTTONUP:
			_systemEventLeftMouseDown = false;

			if (_systemEventRightMouseDown)
				curFlags |= kMouseFlagRightButton;

			getMessageManager()->addEvent(kEventChannelMouse, ev.mouse.x, ev.mouse.y, curFlags);
			eventWillUpdateScreen = true;
			break;

		case Common::EVENT_RBUTTONDOWN:
			_systemEventRightMouseDown = true;
			curFlags |= kMouseFlagRightButton;

			if (_systemEventLeftMouseDown)
				curFlags |= kMouseFlagLeftButton;

			getMessageManager()->addEvent(kEventChannelMouse, ev.mouse.x, ev.mouse.y, curFlags);
			eventWillUpdateScreen = true;
			break;
		case Common::EVENT_RBUTTONUP:
			_systemEventRightMouseDown = false;

			if (_systemEventLeftMouseDown)
				curFlags |= kMouseFlagLeftButton;

			getMessageManager()->addEvent(kEventChannelMouse, ev.mouse.x, ev.mouse.y, curFlags);
			eventWillUpdateScreen = true;
			break;

		case Common::EVENT_MOUSEMOVE:
			// The reason I'm adding this check is because we don't want to
			// stall the "fast-movement" mode when dragging a mouse with
			// pressed buttons. The original doesn't do this, because the handler
			// for input events and the engine lived on separate threads, so it
			// works out nicely for them...
			if (!getLogicManager()->_doubleClickFlag) {
				if (_systemEventLeftMouseDown)
					curFlags |= kMouseFlagLeftButton;

				if (_systemEventRightMouseDown)
					curFlags |= kMouseFlagRightButton;
			}

			_systemEventLastMouseCoords.x = ev.mouse.x;
			_systemEventLastMouseCoords.y = ev.mouse.y;

			getMessageManager()->addEvent(kEventChannelMouse, ev.mouse.x, ev.mouse.y, curFlags);
			eventWillUpdateScreen = true;
			break;

		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			if (!_exitFromMenuButton) {
				getMessageManager()->addEvent(kEventChannelEngine, 0, 0, 1);
				_pendingExitEvent = true;
			}

			break;

		default:
			break;
		}
	}

	if (_exitFromMenuButton)
		_exitFromMenuButton = false;

#ifdef USE_IMGUI
	// Allow the debugger to pick up the changes...
	if (gDebugLevel >= 3) {
		_system->updateScreen();
	} else {
#endif

		// Force the update only if it hasn't been already triggered by an event...
		if (!eventWillUpdateScreen && (_system->getMillis() - _lastForcedScreenUpdateTicks >= 17)) {
			_lastForcedScreenUpdateTicks = _system->getMillis();
			_system->updateScreen();
		}

#ifdef USE_IMGUI
	}
#endif

	return true;
}

void LastExpressEngine::eggMouseWrapper(Event *event) {
	getMenu()->eggMouse(event);
}

void LastExpressEngine::eggTimerWrapper(Event *event) {
	getMenu()->eggTimer(event);
}

void LastExpressEngine::engineEventHandlerWrapper(Event *event) {
	engineEventHandler(event);
}

void LastExpressEngine::nodeStepMouseWrapper(Event *event) {
	getLogicManager()->nodeStepMouse(event);
}

void LastExpressEngine::nodeStepTimerWrapper(Event *event) {
	getLogicManager()->nodeStepTimer(event);
}

void LastExpressEngine::nisMouseWrapper(Event *event) {
	getNISManager()->nisMouse(event);
}

void LastExpressEngine::nisTimerWrapper(Event *event) {
	getNISManager()->nisTimer(event);
}

void LastExpressEngine::creditsMouseWrapper(Event *event) {
	creditsMouse(event);
}

void LastExpressEngine::creditsTimerWrapper(Event *event) {
	creditsTimer(event);
}

void LastExpressEngine::demoEndingMouseWrapper(Event *event) {
	demoEndingMouse(event);
}

void LastExpressEngine::demoEndingTimerWrapper(Event *event) {
	demoEndingTimer(event);
}

void LastExpressEngine::fightMouseWrapper(Event *event) {
	_fight->mouse(event);
}

void LastExpressEngine::fightTimerWrapper(Event *event) {
	_fight->timer(event, true);
}

void LastExpressEngine::emptyHandler(Event *event) {
	// No-op
}

///////////////////////////////////////////////////////////////////////////////////
/// Misc Engine
///////////////////////////////////////////////////////////////////////////////////
bool LastExpressEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsReturnToLauncher);
}

} // End of namespace LastExpress
