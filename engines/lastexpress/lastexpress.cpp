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

#include "lastexpress/data/cursor.h"
#include "lastexpress/data/font.h"

#include "lastexpress/game/logic.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/state.h"

#include "lastexpress/menu/menu.h"
#include "lastexpress/menu/clock.h"

#include "lastexpress/sound/queue.h"
#include "lastexpress/sound/sound.h"
#include "lastexpress/sound/subtitle.h"

#include "lastexpress/graphics.h"
#include "lastexpress/helpers.h"
#include "lastexpress/resource.h"
#include "lastexpress/strings.h"

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/timer.h"

#include "engines/util.h"

const char *g_actionNames[] = {"None", "Action1", "Action2", "ExitCompartment", "Action4", "ExcuseMeCath", "ExcuseMe", "INVALID", "Knock", "OpenDoor", "Action10", "Action11", "Default", "INVALID", "INVALID", "INVALID", "Action16", "DrawScene", "Callback"};
const char *g_directionNames[] = { "None", "Up", "Down", "Left", "Right", "Switch"};
const char *g_entityNames[] = { "Player", "Anna", "August", "Mertens", "Coudert", "Pascale", "Waiter1", "Waiter2", "Cooks", "Verges", "Tatiana", "Vassili", "Alexei", "Abbot", "Milos", "Vesna", "Ivo", "Salko", "Kronos", "Kahina", "Francois", "MmeBoutarel", "Boutarel", "Rebecca", "Sophie", "Mahmud", "Yasmin", "Hadija", "Alouan", "Gendarmes", "Max", "Chapters", "Train", "Tables0", "Tables1", "Tables2", "Tables3", "Tables4", "Tables5", "Entity39"};


namespace LastExpress {

LastExpressEngine::LastExpressEngine(OSystem *syst, const ADGameDescription *gd) :
	Engine(syst), _gameDescription(gd),
	_debugger(nullptr), _random("lastexpress"), _cursor(nullptr),
	_font(nullptr), _logic(nullptr), _menuOld(nullptr),
	_lastFrameCount(0),
	_graphicsManOld(nullptr), _resMan(nullptr),
	_sceneMan(nullptr), _soundManOld(nullptr),
	_eventMouse(nullptr), _eventTick(nullptr),
	_eventMouseBackup(nullptr), _eventTickBackup(nullptr) {
	// Setup mixer
	Engine::syncSoundSettings();

	// Adding the default directories
	const Common::FSNode gameDataDir(ConfMan.getPath("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "data");

	_soundMutex = new Common::Mutex();
}

LastExpressEngine::~LastExpressEngine() {
	// Delete the remaining objects
	SAFE_DELETE(_cursor);
	SAFE_DELETE(_font);
	SAFE_DELETE(_logic);
	SAFE_DELETE(_menuOld);
	SAFE_DELETE(_menu);
	SAFE_DELETE(_graphicsManOld);
	SAFE_DELETE(_resMan);
	SAFE_DELETE(_sceneMan);
	SAFE_DELETE(_soundManOld);
	SAFE_DELETE(_clock);
	SAFE_DELETE(_vcr);
	//_debugger is deleted by Engine

	// Cleanup event handlers
	SAFE_DELETE(_eventMouse);
	SAFE_DELETE(_eventTick);
	SAFE_DELETE(_eventMouseBackup);
	SAFE_DELETE(_eventTickBackup);

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
	getLogicManager()->loadTrain(1);
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

// TODO: which error should we return when some game files are missing/corrupted?
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

	// Start the resource and graphics managers
	_resMan = new ResourceManager(isDemo());
	if (!_resMan->loadArchive(kArchiveCd1))
		return Common::kNoGameDataFoundError;

	_graphicsManOld = new GraphicsManagerOld();
	_graphicsMan = new GraphicsManager(this);

	_spriteMan = new SpriteManager(this);
	_otisMan = new OtisManager(this);
	_subtitleMan = new SubtitleManager(this);

	// Archive manager
	_archiveMan = new ArchiveManager(this);

	// Memory manager
	_memMan = new MemoryManager(this);

	// Message manager
	_msgMan = new MessageManager(this);

	// NIS manager
	_nisMan = new NISManager(this);

	// Load the cursor data
	_cursor = _resMan->loadCursor();
	if (!_cursor)
		return Common::kNoGameDataFoundError;

	// Load the font data
	_font = _resMan->loadFont();
	if (!_font)
		return Common::kNoGameDataFoundError;

	// Start scene manager
	_sceneMan = new SceneManager(this);
	_sceneMan->loadSceneDataFile(kArchiveCd1);

	// Game logic
	_logic = new Logic(this);

	// Sound manager
	_soundManOld = new SoundManagerOld(this);
	_soundMan = new SoundManager(this);

	// Logic manager
	_logicMan = new LogicManager(this);

	// Menu
	_menuOld = new MenuOld(this);
	//_menuOld->show(false, kSavegameTypeIndex, 0);
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

		// Simulate a WM_TIMER event every 250ms
		if (elapsedMs >= 250) {
			elapsedMs -= 250;
			//if (_canDrawMouse)
			//	getMessageManager()->addEvent(4, 0, 0, 4);
		}	
	}

	getTimerManager()->removeTimerProc(soundTimerHandler);

	shutDown();

#ifdef USE_IMGUI
	_system->setImGuiCallbacks(ImGuiCallbacks());
#endif

	return Common::kNoError;
}

uint32 LastExpressEngine::getFrameCounter() const {
	// the original game has a timer running at 60Hz incrementing a dedicated variable
	return (uint64)_system->getMillis() * 60 / 1000;
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
	getLogicManager()->_gameProgress[kProgressPortrait] = 32;
	getLogicManager()->_gameProgress[kProgressChapter] = 1;
	getLogicManager()->_lastSavegameSessionTicks = 0;
	getLogicManager()->_currentGameSessionTicks = 0;
	getLogicManager()->_useLastSavedNodeIndex = 0;
	getLogicManager()->_lastNodeIndex = 0;
	getLogicManager()->_lastSavedNodeIndex = 0;
	getLogicManager()->_inventorySelectedItemIdx = 0;
	getLogicManager()->_gameTime = 1037700;
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
		//abortFight();
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
		//abortFight();
		warning("abortFight() missing from engineEventHandler");
		abortCredits();
		if (!shouldQuit() && !getMenu()->isShowingMenu()) {
			if (getMessageManager()->getEventHandle(1) != &LastExpressEngine::nodeStepMouseWrapper || getVCR()->isVirgin(_currentGameFileColorId))
				getMessageManager()->addEvent(4, 0, 0, 2);
			else
				getMenu()->doEgg(1, 0, 0);
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

void LastExpressEngine::pollEventsOld() {
	Common::Event ev;
	if (!_eventMan->pollEvent(ev))
		return;

	switch (ev.type) {
	case Common::EVENT_LBUTTONUP:
		getGameLogic()->getGameState()->getGameFlags()->mouseLeftClick = true;
		break;

	case Common::EVENT_RBUTTONUP:
		getGameLogic()->getGameState()->getGameFlags()->mouseRightClick = true;
		break;

	default:
		break;
	}
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

bool LastExpressEngine::handleEventsOld() {
	// Make sure all the subsystems have been initialized
	if (!_debugger || !_graphicsManOld)
		error("[LastExpressEngine::handleEvents] Called before the required subsystems have been initialized");

	// Execute stored commands
	if (_debugger->hasCommand()) {
		_debugger->callCommand();

		// re-attach the debugger
		_debugger->attach();
	}

	// Handle input
	Common::Event ev;
	while (_eventMan->pollEvent(ev)) {
		switch (ev.type) {

		case Common::EVENT_KEYDOWN:
			//// DEBUG: Quit game on escape
			//if (ev.kbd.keycode == Common::KEYCODE_ESCAPE)
			//	quitGame();

			break;

		case Common::EVENT_MAINMENU:
			// Closing the GMM

		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_LBUTTONDOWN:
			getGameLogic()->getGameState()->getGameFlags()->mouseLeftClick = true;
			getGameLogic()->getGameState()->getGameFlags()->mouseLeftPressed = (ev.type == Common::EVENT_LBUTTONDOWN) ? true : false;

			{
				// Adjust frameInterval flag
				uint32 frameCounter = getFrameCounter();
				if (frameCounter < _lastFrameCount + 30)
					getGameLogic()->getGameState()->getGameFlags()->frameInterval = true;
				_lastFrameCount = frameCounter;
			}

			if (_eventMouse && _eventMouse->isValid())
				(*_eventMouse)(ev);
			break;

		case Common::EVENT_RBUTTONUP:
		case Common::EVENT_RBUTTONDOWN:
			getGameLogic()->getGameState()->getGameFlags()->mouseRightClick = true;
			getGameLogic()->getGameState()->getGameFlags()->mouseRightPressed = (ev.type == Common::EVENT_RBUTTONDOWN) ? true : false;

			if (_eventMouse && _eventMouse->isValid())
				(*_eventMouse)(ev);
			break;

		case Common::EVENT_MOUSEMOVE:
			if (_eventMouse && _eventMouse->isValid())
				(*_eventMouse)(ev);
			break;

		case Common::EVENT_QUIT:
			quitGame();
			break;

		default:
			break;
		}
	}

	// Game tick event
	if (_eventTick && _eventTick->isValid())
		(*_eventTick)(ev);

	// Update the screen
	_graphicsManOld->update();
	_system->updateScreen();
	//_system->delayMillis(50);

	// The event loop may have triggered the quit status. In this case,
	// stop the execution.
	if (shouldQuit()) {
		return true;
	}

	return false;
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
	getNISManager()->NISMouse(event);
}

void LastExpressEngine::nisTimerWrapper(Event *event) {
	getNISManager()->NISTimer(event);
}

void LastExpressEngine::creditsMouseWrapper(Event *event) {
	creditsMouse(event);
}

void LastExpressEngine::creditsTimerWrapper(Event *event) {
	creditsTimer(event);
}

void LastExpressEngine::fightMouseWrapper(Event *event) {
	_fight->mouse(event);
}

void LastExpressEngine::fightTimerWrapper(Event *event) {
	_fight->timer(event, true);
}

///////////////////////////////////////////////////////////////////////////////////
/// Event Handling
///////////////////////////////////////////////////////////////////////////////////
void LastExpressEngine::backupEventHandlers() {
	if (_eventMouseBackup != nullptr || _eventTickBackup != nullptr)
		error("[LastExpressEngine::backupEventHandlers] backup event handlers are already set");

	_eventMouseBackup = _eventMouse;
	_eventTickBackup = _eventTick;
}

void LastExpressEngine::restoreEventHandlers() {
	if (_eventMouseBackup == nullptr || _eventTickBackup == nullptr)
		error("[LastExpressEngine::restoreEventHandlers] restore called before backing up the event handlers");

	// Cleanup previous event handlers
	SAFE_DELETE(_eventMouse);
	SAFE_DELETE(_eventTick);

	_eventMouse = _eventMouseBackup;
	_eventTick = _eventTickBackup;

	_eventMouseBackup = nullptr;
	_eventTickBackup = nullptr;
}

void LastExpressEngine::setEventHandlers(EventHandler::EventFunction *mouse, EventHandler::EventFunction *tick) {
	if (_eventMouse != _eventMouseBackup)
		SAFE_DELETE(_eventMouse);

	if (_eventTick != _eventTickBackup)
		SAFE_DELETE(_eventTick);

	_eventMouse = mouse;
	_eventTick = tick;
}

///////////////////////////////////////////////////////////////////////////////////
/// Misc Engine
///////////////////////////////////////////////////////////////////////////////////
bool LastExpressEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsReturnToLauncher);
}

} // End of namespace LastExpress
