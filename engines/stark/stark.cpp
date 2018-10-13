/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/stark/stark.h"

#include "engines/stark/console.h"
#include "engines/stark/debug.h"
#include "engines/stark/resources/level.h"
#include "engines/stark/resources/location.h"
#include "engines/stark/savemetadata.h"
#include "engines/stark/scene.h"
#include "engines/stark/services/userinterface.h"
#include "engines/stark/services/archiveloader.h"
#include "engines/stark/services/dialogplayer.h"
#include "engines/stark/services/diary.h"
#include "engines/stark/services/fontprovider.h"
#include "engines/stark/services/gameinterface.h"
#include "engines/stark/services/global.h"
#include "engines/stark/services/resourceprovider.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/stateprovider.h"
#include "engines/stark/services/staticprovider.h"
#include "engines/stark/services/settings.h"
#include "engines/stark/services/gamechapter.h"
#include "engines/stark/services/gamemessage.h"
#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/framelimiter.h"

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/random.h"
#include "common/savefile.h"
#include "common/system.h"
#include "audio/mixer.h"

namespace Stark {

StarkEngine::StarkEngine(OSystem *syst, const ADGameDescription *gameDesc) :
		Engine(syst),
		_gameDescription(gameDesc),
		_gfx(nullptr),
		_frameLimiter(nullptr),
		_scene(nullptr),
		_console(nullptr),
		_global(nullptr),
		_gameInterface(nullptr),
		_archiveLoader(nullptr),
		_stateProvider(nullptr),
		_staticProvider(nullptr),
		_resourceProvider(nullptr),
		_randomSource(nullptr),
		_dialogPlayer(nullptr),
		_diary(nullptr),
		_userInterface(nullptr),
		_fontProvider(nullptr),
		_settings(nullptr),
		_gameChapter(nullptr),
		_gameMessage(nullptr),
		_lastClickTime(0) {
	// Add the available debug channels
	DebugMan.addDebugChannel(kDebugArchive, "Archive", "Debug the archive loading");
	DebugMan.addDebugChannel(kDebugXMG, "XMG", "Debug the loading of XMG images");
	DebugMan.addDebugChannel(kDebugXRC, "XRC", "Debug the loading of XRC resource trees");
	DebugMan.addDebugChannel(kDebugUnknown, "Unknown", "Debug unknown values on the data");
}

StarkEngine::~StarkEngine() {
	delete _gameInterface;
	delete _diary;
	delete _dialogPlayer;
	delete _randomSource;
	delete _scene;
	delete _console;
	delete _gfx;
	delete _frameLimiter;
	delete _staticProvider;
	delete _resourceProvider;
	delete _global;
	delete _stateProvider;
	delete _archiveLoader;
	delete _userInterface;
	delete _fontProvider;
	delete _settings;
	delete _gameChapter;
	delete _gameMessage;

	StarkServices::destroy();
}

Common::Error StarkEngine::run() {
	_console = new Console();
	_gfx = Gfx::Driver::create();

	// Get the screen prepared
	_gfx->init();

	_frameLimiter = new Gfx::FrameLimiter(_system, ConfMan.getInt("engine_speed"));
	_archiveLoader = new ArchiveLoader();
	_stateProvider = new StateProvider();
	_global = new Global();
	_resourceProvider = new ResourceProvider(_archiveLoader, _stateProvider, _global);
	_staticProvider = new StaticProvider(_archiveLoader);
	_randomSource = new Common::RandomSource("stark");
	_fontProvider = new FontProvider();
	_scene = new Scene(_gfx);
	_dialogPlayer = new DialogPlayer();
	_diary = new Diary();
	_gameInterface = new GameInterface();
	_userInterface = new UserInterface(_gfx);
	_settings = new Settings(_mixer, _gameDescription);
	_gameChapter = new GameChapter();
	_gameMessage = new GameMessage();

	// Setup the public services
	StarkServices &services = StarkServices::instance();
	services.archiveLoader = _archiveLoader;
	services.dialogPlayer = _dialogPlayer;
	services.diary = _diary;
	services.gfx = _gfx;
	services.global = _global;
	services.resourceProvider = _resourceProvider;
	services.randomSource = _randomSource;
	services.scene = _scene;
	services.staticProvider = _staticProvider;
	services.gameInterface = _gameInterface;
	services.userInterface = _userInterface;
	services.fontProvider = _fontProvider;
	services.settings = _settings;
	services.gameChapter = _gameChapter;
	services.gameMessage = _gameMessage;

	// Load global resources
	_staticProvider->init();
	_fontProvider->initFonts();

	// Apply the sound volume settings
	syncSoundSettings();

	// Initialize the UI
	_userInterface->init();

	// Load through ResidualVM launcher
	if (ConfMan.hasKey("save_slot")) {
		loadGameState(ConfMan.getInt("save_slot"));
	}

	// Start running
	mainLoop();

	_staticProvider->shutdown();
	_resourceProvider->shutdown();

	return Common::kNoError;
}

void StarkEngine::mainLoop() {
	while (!shouldQuit()) {
		_frameLimiter->startFrame();

		processEvents();

		if (_userInterface->shouldExit()) {
			quitGame();
			break;
		}

		if (_userInterface->hasQuitToMainMenuRequest()) {
			_userInterface->performQuitToMainMenu();
		}

		if (_resourceProvider->hasLocationChangeRequest()) {
			_global->setNormalSpeed();
			_resourceProvider->performLocationChange();
		}

		updateDisplayScene();

		// Swap buffers
		_frameLimiter->delayBeforeSwap();
		_gfx->flipBuffer();
	}
}

void StarkEngine::processEvents() {
	Common::Event e;
	while (g_system->getEventManager()->pollEvent(e)) {
		// Handle any buttons, keys and joystick operations

		if (isPaused()) {
			// Only pressing key P to resume the game is allowed when the game is paused
			if (e.type == Common::EVENT_KEYDOWN && e.kbd.keycode == Common::KEYCODE_p) {
				pauseEngine(false);
			}
			continue;
		} 

		if (e.type == Common::EVENT_KEYDOWN) {
			if (e.kbd.keycode == Common::KEYCODE_d) {
				if (e.kbd.flags & Common::KBD_CTRL) {
					_console->attach();
					_console->onFrame();
				}
			} else if (e.kbd.keycode == Common::KEYCODE_ESCAPE) {
				_userInterface->handleEscape();
			} else if ((e.kbd.keycode == Common::KEYCODE_RETURN
					|| e.kbd.keycode == Common::KEYCODE_KP_ENTER)) {
				if (e.kbd.hasFlags(Common::KBD_ALT)) {
					_gfx->toggleFullscreen();
				} else if (_userInterface->isInGameScreen()){
					_userInterface->selectFocusedDialogOption();
				}
			} else if (e.kbd.keycode == Common::KEYCODE_F1) {
				_userInterface->toggleScreen(Screen::kScreenDiaryIndex);
			} else if (e.kbd.keycode == Common::KEYCODE_F2) {
				if (_userInterface->isInSaveLoadMenuScreen() || canSaveGameStateCurrently()) {
					_userInterface->toggleScreen(Screen::kScreenSaveMenu);
				}
			} else if (e.kbd.keycode == Common::KEYCODE_F3) {
				_userInterface->toggleScreen(Screen::kScreenLoadMenu);
			} else if (e.kbd.keycode == Common::KEYCODE_F4) {
				_userInterface->toggleScreen(Screen::kScreenDialog);
			} else if (e.kbd.keycode == Common::KEYCODE_F5) {
				if (_diary->isEnabled()) {
					_userInterface->toggleScreen(Screen::kScreenDiaryPages);
				}
			} else if (e.kbd.keycode == Common::KEYCODE_F6) {
				_userInterface->toggleScreen(Screen::kScreenFMVMenu);
			} else if (e.kbd.keycode == Common::KEYCODE_F7) {
				_userInterface->toggleScreen(Screen::kScreenSettingsMenu);
			} else if (e.kbd.keycode == Common::KEYCODE_F8) {
				warning("TODO: Implement the screenshot saving to local game directory");
			} else if (e.kbd.keycode == Common::KEYCODE_F9) {
				if (_userInterface->isInGameScreen()) {
					_userInterface->requestToggleSubtitle();
				}
			} else if (e.kbd.keycode == Common::KEYCODE_F10) {
				if (_userInterface->isInGameScreen() || _userInterface->isInDiaryIndexScreen()) {
					if (_userInterface->confirm(GameMessage::kQuitGamePrompt)) {
						_userInterface->requestQuitToMainMenu();
					}
				}
			} else if (e.kbd.keycode == Common::KEYCODE_a) {
				if (_userInterface->isInGameScreen()) {
					_userInterface->cycleBackInventory();
				}
			} else if (e.kbd.keycode == Common::KEYCODE_s) {
				if (_userInterface->isInGameScreen()) {
					_userInterface->cycleForwardInventory();
				}
			} else if (e.kbd.keycode == Common::KEYCODE_i) {
				if (_userInterface->isInGameScreen()) {
					_userInterface->inventoryOpen(!_userInterface->isInventoryOpen());
				}
			} else if (e.kbd.keycode == Common::KEYCODE_x
					&& !e.kbd.hasFlags(Common::KBD_ALT)) {
				if (_userInterface->isInGameScreen()) {
					_userInterface->toggleExitDisplay();
				}
			} else if ((e.kbd.keycode == Common::KEYCODE_x
					|| e.kbd.keycode == Common::KEYCODE_q)
					&& e.kbd.hasFlags(Common::KBD_ALT)) {
				if (_userInterface->confirm(GameMessage::kQuitPrompt)) {
					_userInterface->notifyShouldExit();
				}
			} else if (e.kbd.keycode == Common::KEYCODE_p) {
				if (_userInterface->isInGameScreen()) {
					pauseEngine(true);
					debug("The game is paused");
				}
			} else if (e.kbd.keycode == Common::KEYCODE_PAGEUP) {
				if (_userInterface->isInGameScreen()) {
					if (_userInterface->isInventoryOpen()) {
						_userInterface->scrollInventoryUp();
					} else {
						_userInterface->scrollDialogUp();
					}
				}
			} else if (e.kbd.keycode == Common::KEYCODE_UP) {
				if (_userInterface->isInGameScreen()) {
					if (_userInterface->isInventoryOpen()) {
						_userInterface->scrollInventoryUp();
					} else {
						_userInterface->focusPrevDialogOption();
					}
				}
			} else if (e.kbd.keycode == Common::KEYCODE_PAGEDOWN) {
				if (_userInterface->isInGameScreen()) {
					if (_userInterface->isInventoryOpen()) {
						_userInterface->scrollInventoryDown();
					} else {
						_userInterface->scrollDialogDown();
					}
				}
			} else if (e.kbd.keycode == Common::KEYCODE_DOWN) {
				if (_userInterface->isInGameScreen()) {
					if (_userInterface->isInventoryOpen()) {
						_userInterface->scrollInventoryDown();
					} else {
						_userInterface->focusNextDialogOption();
					}
				}
			} else if (e.kbd.keycode >= Common::KEYCODE_1 && e.kbd.keycode <= Common::KEYCODE_9) {
				if (_userInterface->isInGameScreen()) {
					_userInterface->selectDialogOptionByIndex(e.kbd.keycode - Common::KEYCODE_1);
				}
			}

		} else if (e.type == Common::EVENT_LBUTTONUP) {
			_userInterface->handleMouseUp();
		} else if (e.type == Common::EVENT_MOUSEMOVE) {
			_userInterface->handleMouseMove(e.mouse);
		} else if (e.type == Common::EVENT_LBUTTONDOWN) {
			_userInterface->handleClick();
			if (_system->getMillis() - _lastClickTime < _doubleClickDelay) {
				_userInterface->handleDoubleClick();
			}
			_lastClickTime = _system->getMillis();
		} else if (e.type == Common::EVENT_RBUTTONDOWN) {
			_userInterface->handleRightClick();
		} else if (e.type == Common::EVENT_SCREEN_CHANGED) {
			onScreenChanged();
		}
	}
}

void StarkEngine::updateDisplayScene() {
	if (_global->isFastForward()) {
		// The original engine was frame limited to 30 fps.
		// Set the frame duration to 1000 / 30 ms so that fast forward
		// skips the same amount of simulated time as the original.
		_global->setMillisecondsPerGameloop(33);
	} else {
		_global->setMillisecondsPerGameloop(_frameLimiter->getLastFrameDuration());
	}

	// Clear the screen
	_gfx->clearScreen();

	// Only update the world resources when on the game screen
	if (_userInterface->isInGameScreen() && !isPaused()) {
		int frames = 0;
		do {
			// Update the game resources
			_global->getLevel()->onGameLoop();
			_global->getCurrent()->getLevel()->onGameLoop();
			_global->getCurrent()->getLocation()->onGameLoop();
			frames++;

			// When the game is in fast forward mode, update
			// the game resources for multiple frames,
			// but render only once.
		} while (_global->isFastForward() && frames < 100);
		_global->setNormalSpeed();
	}

	// Render the current scene
	// Update the UI state before displaying the scene
	_userInterface->onGameLoop();

	// Tell the UI to render, and update implicitly, if this leads to new mouse-over events.
	_userInterface->render();
}

bool StarkEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime) ||
		(f == kSupportsArbitraryResolutions) ||
		(f == kSupportsRTL);
}

bool StarkEngine::canLoadGameStateCurrently() {
	return !StarkUserInterface->isInSaveLoadMenuScreen();
}

Common::Error StarkEngine::loadGameState(int slot) {
	// Open the save file
	Common::String filename = formatSaveName(_targetName.c_str(), slot);
	Common::InSaveFile *save = _saveFileMan->openForLoading(filename);
	if (!save) {
		return _saveFileMan->getError();
	}

	StateReadStream stream(save);

	// Read the header
	SaveMetadata metadata;
	Common::ErrorCode metadataErrorCode = metadata.read(&stream, filename);
	if (metadataErrorCode != Common::kNoError) {
		return metadataErrorCode;
	}

	// Reset the UI
	_userInterface->skipFMV();
	_userInterface->clearLocationDependentState();
	_userInterface->setInteractive(true);
	_userInterface->changeScreen(Screen::kScreenGame);
	_userInterface->restoreScreenHistory();

	// Clear the previous world resources
	_resourceProvider->shutdown();

	if (metadata.version >= 9) {
		metadata.skipGameScreenThumbnail(&stream);
	}

	// Read the resource trees state
	_stateProvider->readStateFromStream(&stream, metadata.version);

	// Read the diary state
	_diary->readStateFromStream(&stream, metadata.version);

	if (stream.eos()) {
		warning("Unexpected end of file reached when reading '%s'", filename.c_str());
		return Common::kReadingFailed;
	}

	// Initialize the world resources with the loaded state
	_resourceProvider->initGlobal();
	_resourceProvider->setShouldRestoreCurrentState();
	_resourceProvider->requestLocationChange(metadata.levelIndex, metadata.locationIndex);

	if (metadata.version >= 9) {
		setTotalPlayTime(metadata.totalPlayTime);
	}

	return Common::kNoError;
}

bool StarkEngine::canSaveGameStateCurrently() {
	// Disallow saving when there is no level loaded or when a script is running
	// or when the save & load menu is currently displayed
	return _global->getLevel() && _userInterface->isInteractive() && !_userInterface->isInSaveLoadMenuScreen();
}

Common::Error StarkEngine::saveGameState(int slot, const Common::String &desc) {
	// Ensure the state store is up to date
	_resourceProvider->commitActiveLocationsState();

	// Open the save file
	Common::String filename = formatSaveName(_targetName.c_str(), slot);
	Common::OutSaveFile *save = _saveFileMan->openForSaving(filename);
	if (!save) {
		return _saveFileMan->getError();
	}

	// 1. Write the header
	SaveMetadata metadata;
	metadata.description = desc;
	metadata.version = StateProvider::kSaveVersion;
	metadata.levelIndex = _global->getCurrent()->getLevel()->getIndex();
	metadata.locationIndex = _global->getCurrent()->getLocation()->getIndex();
	metadata.totalPlayTime = getTotalPlayTime();
	metadata.gameWindowThumbnail = _userInterface->getGameWindowThumbnail();

	TimeDate timeDate;
	_system->getTimeAndDate(timeDate);
	metadata.setSaveTime(timeDate);

	metadata.write(save);
	metadata.writeGameScreenThumbnail(save);

	// 2. Write the resource trees state
	_stateProvider->writeStateToStream(save);

	// 3. Write the diary state
	_diary->writeStateToStream(save);

	delete save;

	return Common::kNoError;
}

Common::String StarkEngine::formatSaveName(const char *target, int slot) {
	return Common::String::format("%s-%03d.tlj", target, slot);
}

void StarkEngine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);

	// This function may be called when an error occurs before the engine is fully initialized
	if (_global && _global->getLevel() && _global->getCurrent()) {
		_global->getLevel()->onEnginePause(pause);
		_global->getCurrent()->getLevel()->onEnginePause(pause);
		_global->getCurrent()->getLocation()->onEnginePause(pause);
	}

	if (_frameLimiter) {
		_frameLimiter->pause(pause);
	}

	// Grab a game screen thumbnail in case we need one when writing a save file
	if (_userInterface && _userInterface->isInGameScreen()) {
		if (pause) {
			_userInterface->saveGameScreenThumbnail();
		} else {
			_userInterface->freeGameScreenThumbnail();
		}
	}

	// The user may have moved the mouse or resized the window while the engine was paused
	if (!pause && _userInterface) {
		onScreenChanged();
		_userInterface->handleMouseMove(_eventMan->getMousePos());
	}
}

void StarkEngine::onScreenChanged() const {
	bool changed = _gfx->computeScreenViewport();
	if (changed) {
		_fontProvider->initFonts();
		_userInterface->onScreenChanged();
	}
}

} // End of namespace Stark
