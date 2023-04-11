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

#include "lastexpress/sound/queue.h"
#include "lastexpress/sound/sound.h"

#include "lastexpress/graphics.h"
#include "lastexpress/helpers.h"
#include "lastexpress/resource.h"

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/fs.h"

#include "engines/util.h"

const char *g_actionNames[] = {"None", "Action1", "Action2", "ExitCompartment", "Action4", "ExcuseMeCath", "ExcuseMe", "INVALID", "Knock", "OpenDoor", "Action10", "Action11", "Default", "INVALID", "INVALID", "INVALID", "Action16", "DrawScene", "Callback"};
const char *g_directionNames[] = { "None", "Up", "Down", "Left", "Right", "Switch"};
const char *g_entityNames[] = { "Player", "Anna", "August", "Mertens", "Coudert", "Pascale", "Waiter1", "Waiter2", "Cooks", "Verges", "Tatiana", "Vassili", "Alexei", "Abbot", "Milos", "Vesna", "Ivo", "Salko", "Kronos", "Kahina", "Francois", "MmeBoutarel", "Boutarel", "Rebecca", "Sophie", "Mahmud", "Yasmin", "Hadija", "Alouan", "Gendarmes", "Max", "Chapters", "Train", "Tables0", "Tables1", "Tables2", "Tables3", "Tables4", "Tables5", "Entity39"};


namespace LastExpress {

LastExpressEngine::LastExpressEngine(OSystem *syst, const ADGameDescription *gd) :
	Engine(syst), _gameDescription(gd),
	_debugger(nullptr), _random("lastexpress"), _cursor(nullptr),
	_font(nullptr), _logic(nullptr), _menu(nullptr),
	_lastFrameCount(0),
	_graphicsMan(nullptr), _resMan(nullptr),
	_sceneMan(nullptr), _soundMan(nullptr),
	_eventMouse(nullptr), _eventTick(nullptr),
	_eventMouseBackup(nullptr), _eventTickBackup(nullptr)
	{
	// Setup mixer
	Engine::syncSoundSettings();

	// Adding the default directories
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "data");
}

LastExpressEngine::~LastExpressEngine() {
	// Delete the remaining objects
	SAFE_DELETE(_cursor);
	SAFE_DELETE(_font);
	SAFE_DELETE(_logic);
	SAFE_DELETE(_menu);
	SAFE_DELETE(_graphicsMan);
	SAFE_DELETE(_resMan);
	SAFE_DELETE(_sceneMan);
	SAFE_DELETE(_soundMan);
	//_debugger is deleted by Engine

	// Cleanup event handlers
	SAFE_DELETE(_eventMouse);
	SAFE_DELETE(_eventTick);
	SAFE_DELETE(_eventMouseBackup);
	SAFE_DELETE(_eventTickBackup);

	// Zero passed pointers
	_gameDescription = nullptr;
}

Common::String LastExpressEngine::getTargetName() const {
	return _targetName;
}

// TODO: which error should we return when some game files are missing/corrupted?
Common::Error LastExpressEngine::run() {
	// Initialize the graphics
	const Graphics::PixelFormat dataPixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0);
	initGraphics(640, 480, &dataPixelFormat);

	// We do not support color conversion
	if (_system->getScreenFormat() != dataPixelFormat)
		return Common::kUnsupportedColorMode;

	// Create debugger. It requires GFX to be initialized
	_debugger = new Debugger(this);
	setDebugger(_debugger);

	// Start the resource and graphics managers
	_resMan = new ResourceManager(isDemo());
	if (!_resMan->loadArchive(kArchiveCd1))
		return Common::kNoGameDataFoundError;

	_graphicsMan = new GraphicsManager();

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
	_soundMan = new SoundManager(this);

	// Menu
	_menu = new Menu(this);
	_menu->show(false, kSavegameTypeIndex, 0);

	while (!shouldQuit()) {
		_soundMan->getQueue()->updateQueue();
		_soundMan->getQueue()->updateSubtitles();

		if (handleEvents())
			continue;
	}

	return Common::kNoError;
}

uint32 LastExpressEngine::getFrameCounter() const {
	// the original game has a timer running at 60Hz incrementing a dedicated variable
	return (uint64)_system->getMillis() * 60 / 1000;
}

void LastExpressEngine::pollEvents() {
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

bool LastExpressEngine::handleEvents() {
	// Make sure all the subsystems have been initialized
	if (!_debugger || !_graphicsMan)
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
	_graphicsMan->update();
	_system->updateScreen();
	_system->delayMillis(50);

	// The event loop may have triggered the quit status. In this case,
	// stop the execution.
	if (shouldQuit()) {
		return true;
	}

	return false;
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
