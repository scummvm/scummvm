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
#include "engines/stark/scene.h"
#include "engines/stark/services/userinterface.h"
#include "engines/stark/services/archiveloader.h"
#include "engines/stark/services/dialogplayer.h"
#include "engines/stark/services/fontprovider.h"
#include "engines/stark/services/gameinterface.h"
#include "engines/stark/services/global.h"
#include "engines/stark/services/resourceprovider.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/stateprovider.h"
#include "engines/stark/services/staticprovider.h"
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
		_userInterface(nullptr),
		_fontProvider(nullptr),
		_lastClickTime(0) {
	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, 127);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	// Add the available debug channels
	DebugMan.addDebugChannel(kDebugArchive, "Archive", "Debug the archive loading");
	DebugMan.addDebugChannel(kDebugXMG, "XMG", "Debug the loading of XMG images");
	DebugMan.addDebugChannel(kDebugXRC, "XRC", "Debug the loading of XRC resource trees");
	DebugMan.addDebugChannel(kDebugUnknown, "Unknown", "Debug unknown values on the data");
}

StarkEngine::~StarkEngine() {
	delete _gameInterface;
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

	StarkServices::destroy();
}

Common::Error StarkEngine::run() {
	_console = new Console();
	_gfx = Gfx::Driver::create();

	// Get the screen prepared
	_gfx->init();

	_frameLimiter = new Gfx::FrameLimiter(_system, 60);
	_archiveLoader = new ArchiveLoader();
	_stateProvider = new StateProvider();
	_global = new Global();
	_resourceProvider = new ResourceProvider(_archiveLoader, _stateProvider, _global);
	_staticProvider = new StaticProvider(_archiveLoader);
	_randomSource = new Common::RandomSource("stark");
	_fontProvider = new FontProvider();
	_scene = new Scene(_gfx);
	_dialogPlayer = new DialogPlayer();
	_gameInterface = new GameInterface();
	_userInterface = new UserInterface(_gfx);

	// Setup the public services
	StarkServices &services = StarkServices::instance();
	services.archiveLoader = _archiveLoader;
	services.dialogPlayer = _dialogPlayer;
	services.gfx = _gfx;
	services.global = _global;
	services.resourceProvider = _resourceProvider;
	services.randomSource = _randomSource;
	services.scene = _scene;
	services.staticProvider = _staticProvider;
	services.gameInterface = _gameInterface;
	services.userInterface = _userInterface;
	services.fontProvider = _fontProvider;

	// Load global resources
	_resourceProvider->initGlobal();
	_staticProvider->init();
	_fontProvider->initFonts();
	// Initialize the UI
	_userInterface->init();

	if (ConfMan.hasKey("save_slot")) {
		// Load game from specified slot, if any
		loadGameState(ConfMan.getInt("save_slot"));
	} else {
		// Otherwise, set the startup location, ie the House of All Worlds by default
		setStartupLocation();
	}

	// Start running
	mainLoop();

	_staticProvider->shutdown();
	_resourceProvider->shutdown();

	return Common::kNoError;
}

void StarkEngine::setStartupLocation() {
	if (ConfMan.hasKey("startup_chapter")) {
		_global->setCurrentChapter(ConfMan.getInt("startup_chapter"));
	} else {
		_global->setCurrentChapter(0);
	}

	if (ConfMan.hasKey("startup_level") && ConfMan.hasKey("startup_location")) {
		uint levelIndex = strtol(ConfMan.get("startup_level").c_str(), nullptr, 16);
		uint locationIndex = strtol(ConfMan.get("startup_location").c_str(), nullptr, 16);
		_resourceProvider->requestLocationChange(levelIndex, locationIndex);
	} else {
		if (isDemo()) {
			_resourceProvider->requestLocationChange(0x4f, 0x00);
		} else {
			// Start us up at the house of all worlds
			_resourceProvider->requestLocationChange(0x45, 0x00);
		}
	}
}

void StarkEngine::mainLoop() {
	while (!shouldQuit()) {
		_frameLimiter->startFrame();

		processEvents();

		if (_userInterface->shouldExit()) {
			quitGame();
			break;
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
		if (e.type == Common::EVENT_KEYDOWN) {
			if (e.kbd.keycode == Common::KEYCODE_d) {
				if (e.kbd.flags & Common::KBD_CTRL) {
					_console->attach();
					_console->onFrame();
				}
			} else if (e.kbd.keycode == Common::KEYCODE_ESCAPE) {
				// Quick-hack for now.
				bool skipped = _gameInterface->skipCurrentSpeeches();
				if (!skipped) {
					skipped = _userInterface->skipFMV();
				}
				if (!skipped) {
					_global->setFastForward();
				}
			}

		} else if (e.type == Common::EVENT_LBUTTONUP) {
			// Do nothing for now
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
		}
	}
}

void StarkEngine::updateDisplayScene() {
	_global->setMillisecondsPerGameloop(_frameLimiter->getLastFrameDuration());

	// Clear the screen
	_gfx->clearScreen();

	// Only update the world resources when on the game screen
	if (_userInterface->isInGameScreen()) {
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
	_userInterface->update();

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
	return true;
}

Common::Error StarkEngine::loadGameState(int slot) {
	// Open the save file
	Common::String filename = formatSaveName(_targetName.c_str(), slot);
	Common::InSaveFile *save = _saveFileMan->openForLoading(filename);
	if (!save) {
		return _saveFileMan->getError();
	}

	// Reset the UI
	_userInterface->skipFMV();
	_userInterface->clearLocationDependentState();
	_userInterface->setInteractive(true);

	// Clear the previous world resources
	_resourceProvider->shutdown();

	StateReadStream *stream = new StateReadStream(save);

	// Read the header
	Common::String desc = stream->readString();

	Common::String level = stream->readString();
	uint levelIndex = strtol(level.c_str(), nullptr, 16);

	Common::String location = stream->readString();
	uint locationIndex = strtol(location.c_str(), nullptr, 16);

	Common::String version = stream->readString();
	//TODO: Check the version

	// Read the resource trees state
	_stateProvider->readStateFromStream(stream);

	//TODO: Read the rest of the state

	delete stream;

	// Initialize the world resources with the loaded state
	_resourceProvider->initGlobal();
	_resourceProvider->setShouldRestoreCurrentState();
	_resourceProvider->requestLocationChange(levelIndex, locationIndex);

	return Common::kNoError;
}

bool StarkEngine::canSaveGameStateCurrently() {
	return _userInterface->isInteractive();
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
	// Save description
	save->writeUint32LE(desc.size());
	save->writeString(desc);

	// Level
	Common::String level = _global->getCurrent()->getLevel()->getIndexAsString();
	save->writeUint32LE(level.size());
	save->writeString(level);

	// Location
	Common::String location = _global->getCurrent()->getLocation()->getIndexAsString();
	save->writeUint32LE(location.size());
	save->writeString(location);

	// Version
	Common::String version = "Version:\t06";
	save->writeUint32LE(version.size());
	save->writeString(version);

	// 2. Write the resource trees state
	_stateProvider->writeStateToStream(save);

	//TODO: Write the rest of the state
	//TODO: Write a screenshot

	delete save;

	return Common::kNoError;
}

bool StarkEngine::isDemo() {
	return _gameDescription->flags & ADGF_DEMO;
}

Common::String StarkEngine::formatSaveName(const char *target, int slot) {
	return Common::String::format("%s-%03d.tlj", target, slot);
}

void StarkEngine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);

	if (!_global || !_frameLimiter) {
		// This function may be called when an error occurs before the engine is fully initialized
		return;
	}

	_frameLimiter->pause(pause);
}
} // End of namespace Stark
