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
	SearchMan.addSubDirectoryMatching(gameDataDir, "data");

	_soundMutex = new Common::Mutex();
}

LastExpressEngine::~LastExpressEngine() {
	SAFE_DELETE(_graphicsMan);
	SAFE_DELETE(_spriteMan);
	SAFE_DELETE(_otisMan);
	SAFE_DELETE(_subtitleMan);
	SAFE_DELETE(_archiveMan);
	SAFE_DELETE(_memMan);
	SAFE_DELETE(_msgMan);
	SAFE_DELETE(_nisMan);
	SAFE_DELETE(_soundMan);
	SAFE_DELETE(_logicMan);
	SAFE_DELETE(_menu);
	SAFE_DELETE(_saveMan);
	SAFE_DELETE(_clock);
	SAFE_DELETE(_vcr);

	//_debugger is deleted by Engine

	// Zero passed pointers
	_gameDescription = nullptr;
}

void LastExpressEngine::startUp() {
	getArchiveManager()->initHPFS();
	getMemoryManager()->initMem();

	getGraphicsManager()->clear(getGraphicsManager()->_screenSurface, 0, 0, 640, 480);
	getGraphicsManager()->clear(getGraphicsManager()->_screenBuffer, 0, 0, 640, 480);
	getGraphicsManager()->clear(getGraphicsManager()->_backgroundBuffer, 0, 0, 640, 480);

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

		engine->getMessageManager()->addEvent(3, 0, 0, 0);
	}

	engine->_soundFrameCounter++;
}

Common::Error LastExpressEngine::run() {
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
	_archiveMan = new ArchiveManager(this);

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

	getMessageManager()->setEventHandle(4, &LastExpressEngine::engineEventHandlerWrapper);

	getTimerManager()->installTimerProc(soundTimerHandler, 17000, this, "LastExpressEngine");

	getGraphicsManager()->newMouseLoc();
	getArchiveManager()->loadMice();
	getVCR()->loadSettings();
	getMenu()->doEgg(false, kSavegameTypeIndex, 0);

	int32 elapsedMs = 0;

	while (!shouldQuit()) {
		do {
			getSoundManager()->soundThread();
			getSubtitleManager()->subThread();
		} while (getMessageManager()->process());

		waitForTimer(4); // Wait 4 ticks (tick duration: 17 ms dictated by the sound timer)
		elapsedMs += 4 * 17;
	}

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
		getLogicManager()->_positions[i] = 0;
	}

	for (int i = 0; i < 16; i++) {
		getLogicManager()->_blockedEntitiesBits[i] = 0;
		getLogicManager()->_softBlockedEntitiesBits[i] = 0;
	}

	for (int i = 0; i < 512; i++) {
		getLogicManager()->_gameEvents[i] = 0;
	}

	for (int i = 0; i < 128; i++) {
		getLogicManager()->_gameProgress[i] = 0;
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
		getLogicManager()->_gameObjects[i].door = 0;
		getLogicManager()->_gameObjects[i].character = 0;
		getLogicManager()->_gameObjects[i].cursor = 10;
		getLogicManager()->_gameObjects[i].cursor2 = 9;
		getLogicManager()->_gameObjects[i].model = 0;
	}

	for (int i = 0; i < 32; i++) {
		getLogicManager()->_gameInventory[i].cursor = 0;
		getLogicManager()->_gameInventory[i].isPresent = 0;
		getLogicManager()->_gameInventory[i].location = 0;
		getLogicManager()->_gameInventory[i].manualSelect = 1;
	}

	getLogicManager()->_gameInventory[kItemMatchBox].cursor = 16;
	getLogicManager()->_gameInventory[kItemTelegram].cursor = 17;
	getLogicManager()->_gameInventory[kItemPassengerList].cursor = 18;
	getLogicManager()->_gameInventory[kItemArticle].cursor = 19;
	getLogicManager()->_gameInventory[kItemScarf].cursor = 20;
	getLogicManager()->_gameInventory[kItemPaper].cursor = 21;
	getLogicManager()->_gameInventory[kItemParchemin].cursor = 22;
	getLogicManager()->_gameInventory[kItemMatch].cursor = 23;
	getLogicManager()->_gameInventory[kItemWhistle].cursor = 24;
	getLogicManager()->_gameInventory[kItemKey].cursor = 25;
	getLogicManager()->_gameInventory[kItemBomb].cursor = 26;
	getLogicManager()->_gameInventory[kItemFirebird].cursor = 27;
	getLogicManager()->_gameInventory[kItemBriefcase].cursor = 28;
	getLogicManager()->_gameInventory[kItemCorpse].cursor = 29;
	getLogicManager()->_gameInventory[kItemMatchBox].isSelectable = 1;
	getLogicManager()->_gameInventory[kItemMatch].isSelectable = 1;
	getLogicManager()->_gameInventory[kItemTelegram].isSelectable = 1;
	getLogicManager()->_gameInventory[kItemWhistle].isSelectable = 1;
	getLogicManager()->_gameInventory[kItemKey].isSelectable = 1;
	getLogicManager()->_gameInventory[kItemFirebird].isSelectable = 1;
	getLogicManager()->_gameInventory[kItemBriefcase].isSelectable = 1;
	getLogicManager()->_gameInventory[kItemCorpse].isSelectable = 1;
	getLogicManager()->_gameInventory[kItemPassengerList].isSelectable = 1;
	getLogicManager()->_gameInventory[kItem2].manualSelect = 0;
	getLogicManager()->_gameInventory[kItem3].manualSelect = 0;
	getLogicManager()->_gameInventory[kItem5].manualSelect = 0;
	getLogicManager()->_gameInventory[kItem7].manualSelect = 0;
	getLogicManager()->_gameInventory[kItem9].manualSelect = 0;
	getLogicManager()->_gameInventory[kItemBeetle].manualSelect = 0;
	getLogicManager()->_gameInventory[kItem11].manualSelect = 0;
	getLogicManager()->_gameInventory[kItem17].manualSelect = 0;
	getLogicManager()->_gameInventory[kItemFirebird].manualSelect = 0;
	getLogicManager()->_gameInventory[kItemBriefcase].manualSelect = 0;
	getLogicManager()->_gameInventory[kItemCorpse].manualSelect = 0;
	getLogicManager()->_gameInventory[kItemGreenJacket].manualSelect = 0;
	getLogicManager()->_gameInventory[kItem22].manualSelect = 0;
	getLogicManager()->_gameInventory[kItemMatchBox].scene = 31;
	getLogicManager()->_gameInventory[kItemTelegram].scene = 32;
	getLogicManager()->_gameInventory[kItemPassengerList].scene = 33;
	getLogicManager()->_gameInventory[kItemScarf].scene = 34;
	getLogicManager()->_gameInventory[kItemParchemin].scene = 35;
	getLogicManager()->_gameInventory[kItemFirebird].scene = 38;
	getLogicManager()->_gameInventory[kItemBriefcase].scene = 39;
	getLogicManager()->_gameInventory[kItemPaper].scene = 37;
	getLogicManager()->_gameInventory[kItemArticle].scene = 36;
	getLogicManager()->_gameInventory[kItemTelegram].isPresent = 1;
	getLogicManager()->_gameInventory[kItemArticle].isPresent = 1;
	getLogicManager()->_gameProgress[kProgressPortrait] = isDemo() ? 34: 32;
	getLogicManager()->_gameProgress[kProgressChapter] = isDemo() ? 3 : 1;
	getLogicManager()->_lastSavegameSessionTicks = 0;
	getLogicManager()->_currentGameSessionTicks = 0;
	getLogicManager()->_useLastSavedNodeIndex = 0;
	getLogicManager()->_lastNodeIndex = 0;
	getLogicManager()->_lastSavedNodeIndex = 0;
	getLogicManager()->_inventorySelectedItemIdx = 0;
	getLogicManager()->_gameTime = isDemo() ? 2241000 : 1037700;
	getLogicManager()->_gameTimeTicksDelta = 3;
	getLogicManager()->_trainNodeIndex = 40;
}

void LastExpressEngine::startNewGame() {
	initGameData();
	getLogicManager()->CONS_All(true, kCharacterCath);
}

void LastExpressEngine::engineEventHandler(Event *event) {
	switch (event->flags) {
	case 1:
		//++g_numClicks;
		getNISManager()->abortNIS();
		warning("abortFight() missing from engineEventHandler");
		abortFight();
		abortCredits();
		if (shouldQuit()) {
			// g_flag_running = 0;
			// PostMessageA(g_engine_state->hwnd, WM_CLOSE, 0, 0);
			// g_flag_main_thread_running = 0;
			// AfxEndThread(0, 1);
		}

		if (getMenu()->isShowingMenu() || getMessageManager()->getEventHandle(1) == &LastExpressEngine::nodeStepMouseWrapper && !getVCR()->isVirgin(_currentGameFileColorId)) {
			getMenu()->doEgg(1, 0, 0);
			quitGame();
		} else {
			getMessageManager()->addEvent(4, 0, 0, 2);
		}

		if (shouldQuit() /* || g_numClicks > 100*/) {
			//g_flag_running = 0;
			//PostMessageA(g_engine_state->hwnd, WM_CLOSE, 0, 0);
			//g_flag_main_thread_running = 0;
			//AfxEndThread(0, 1);
		}

		break;
	case 2:
		getNISManager()->abortNIS();
		abortFight();
		warning("abortFight() missing from engineEventHandler");
		abortCredits();
		if (!shouldQuit() && !getMenu()->isShowingMenu()) {
			if (getMessageManager()->getEventHandle(1) != &LastExpressEngine::nodeStepMouseWrapper || getVCR()->isVirgin(_currentGameFileColorId))
				getMessageManager()->addEvent(4, 0, 0, 2);
			else
				getMenu()->doEgg(true, 0, 0);
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
		return;
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

void LastExpressEngine::waitForTimer(int frames) {
	uint32 startTime = _system->getMillis();
	uint32 waitTime = 17;

	for (int i = 0; i < frames; i++) {
		while (_system->getMillis() - startTime < waitTime) {
			handleEvents();
		}
	}
}

bool LastExpressEngine::handleEvents() {
	// Handle input
	Common::Event ev;
	int32 curFlags = 0;

	// Allow the debugger to pick up the changes...
	if (gDebugLevel >= 3) {
		_system->updateScreen();
	}

	while (_eventMan->pollEvent(ev)) {
		switch (ev.type) {

		case Common::EVENT_KEYDOWN:
			//// DEBUG: Quit game on escape
			// if (ev.kbd.keycode == Common::KEYCODE_ESCAPE)
			//	quitGame();

			break;

		//case Common::EVENT_MAINMENU:
			// Closing the GMM


		case Common::EVENT_LBUTTONDOWN:
			_systemEventLeftMouseDown = true;
			curFlags |= 1;

			if (_systemEventRightMouseDown)
				curFlags |= 2;

			getMessageManager()->addEvent(1, ev.mouse.x, ev.mouse.y, curFlags);
			break;
		case Common::EVENT_LBUTTONUP:
			_systemEventLeftMouseDown = false;

			if (_systemEventRightMouseDown)
				curFlags |= 2;

			getMessageManager()->addEvent(1, ev.mouse.x, ev.mouse.y, curFlags);
			break;

		case Common::EVENT_RBUTTONDOWN:
			_systemEventRightMouseDown = true;
			curFlags |= 2;

			if (_systemEventLeftMouseDown)
				curFlags |= 1;

			getMessageManager()->addEvent(1, ev.mouse.x, ev.mouse.y, curFlags);
			break;
		case Common::EVENT_RBUTTONUP:
			_systemEventRightMouseDown = false;

			if (_systemEventLeftMouseDown)
				curFlags |= 1;

			getMessageManager()->addEvent(1, ev.mouse.x, ev.mouse.y, curFlags);
			break;

		case Common::EVENT_MOUSEMOVE:
			// The reason I'm adding this check is because we don't want to
			// stall the "fast-movement" mode when dragging a mouse with
			// pressed buttons. The original doesn't do this, because the handler
			// for input events and the engine lived on separate threads, so it
			// works out nicely for them...
			if (!getLogicManager()->_doubleClickFlag) {
				if (_systemEventLeftMouseDown)
					curFlags |= 1;
			
				if (_systemEventRightMouseDown)
					curFlags |= 2;
			}

			_systemEventLastMouseCoords.x = ev.mouse.x;
			_systemEventLastMouseCoords.y = ev.mouse.y;

			getMessageManager()->addEvent(1, ev.mouse.x, ev.mouse.y, curFlags);
			break;

		case Common::EVENT_QUIT:
			getMessageManager()->addEvent(4, 0, 0, 1);
			quitGame();
			break;

		default:
			break;
		}
	}

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
