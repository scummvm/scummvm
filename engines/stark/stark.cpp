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
#include "engines/stark/services/archiveloader.h"
#include "engines/stark/services/dialogplayer.h"
#include "engines/stark/services/global.h"
#include "engines/stark/services/resourceprovider.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/stateprovider.h"
#include "engines/stark/services/userinterface.h"
#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/renderentry.h"

#include "common/config-manager.h"
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
		_scene(nullptr),
		_console(nullptr),
		_global(nullptr),
		_userInterface(nullptr),
		_archiveLoader(nullptr),
		_stateProvider(nullptr),
		_resourceProvider(nullptr),
		_randomSource(nullptr),
		_dialogPlayer(nullptr) {
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
	delete _userInterface;
	delete _dialogPlayer;
	delete _randomSource;
	delete _scene;
	delete _console;
	delete _gfx;
	delete _resourceProvider;
	delete _global;
	delete _stateProvider;
	delete _archiveLoader;

	StarkServices::destroy();
}

Common::Error StarkEngine::run() {
	_console = new Console();
	_gfx = Gfx::GfxDriver::create();

	// Get the screen prepared
	_gfx->setupScreen(640, 480, ConfMan.getBool("fullscreen"));

	_archiveLoader = new ArchiveLoader();
	_stateProvider = new StateProvider();
	_global = new Global();
	_resourceProvider = new ResourceProvider(_archiveLoader, _stateProvider, _global);
	_randomSource = new Common::RandomSource("stark");
	_scene = new Scene(_gfx);
	_dialogPlayer = new DialogPlayer();
	_userInterface = new UserInterface();

	// Setup the public services
	StarkServices &services = StarkServices::instance();
	services.archiveLoader = _archiveLoader;
	services.dialogPlayer = _dialogPlayer;
	services.gfx = _gfx;
	services.global = _global;
	services.resourceProvider = _resourceProvider;
	services.randomSource = _randomSource;
	services.scene = _scene;

	// Load global resources
	_resourceProvider->initGlobal();

	// Start us up at the house of all worlds
	_global->setCurrentChapter(0);
	_resourceProvider->requestLocationChange(0x45, 0x00);

	// Start running
	mainLoop();

	_resourceProvider->shutdown();

	return Common::kNoError;
}

void StarkEngine::mainLoop() {
	while (!shouldQuit()) {
		// Process events
		Common::Event e;
		while (g_system->getEventManager()->pollEvent(e)) {
			// Handle any buttons, keys and joystick operations
			if (e.type == Common::EVENT_KEYDOWN) {
				if (e.kbd.ascii == 'q') {
					quitGame();
					break;
				} else if (e.kbd.keycode == Common::KEYCODE_d) {
					if (e.kbd.flags & Common::KBD_CTRL) {
						_console->attach();
						_console->onFrame();
					}
				} else {
					//handleChars(event.type, event.kbd.keycode, event.kbd.flags, event.kbd.ascii);
				}

			} else if (e.type == Common::EVENT_LBUTTONUP) {
				_userInterface->skipCurrentSpeeches();
			} else if (e.type == Common::EVENT_MOUSEMOVE) {
				_userInterface->scrollLocation(e.relMouse.x, e.relMouse.y);
			}
			/*if (event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_KEYUP) {
				handleControls(event.type, event.kbd.keycode, event.kbd.flags, event.kbd.ascii);
			}*/
			// Check for "Hard" quit"
			//if (e.type == Common::EVENT_QUIT)
			//	return;
			/*if (event.type == Common::EVENT_SCREEN_CHANGED)
				_refreshDrawNeeded = true;*/
		}

		_dialogPlayer->update();

		if (_resourceProvider->hasLocationChangeRequest()) {
			_resourceProvider->performLocationChange();
		}

		updateDisplayScene();
		g_system->delayMillis(50);
	}
}

void StarkEngine::updateDisplayScene() {
	// Get frame delay
	static uint32 lastFrame = g_system->getMillis();
	_global->setMillisecondsPerGameloop(g_system->getMillis() - lastFrame);
	lastFrame += _global->getMillisecondsPerGameloop();

	// Clear the screen
	_gfx->clearScreen();

	// Update the game resources
	_global->getLevel()->onGameLoop();
	_global->getCurrent()->getLevel()->onGameLoop();
	_global->getCurrent()->getLocation()->onGameLoop();

	// Render the current scene
	Gfx::RenderEntryArray renderEntries = _global->getCurrent()->getLocation()->listRenderEntries();
	_scene->render(renderEntries);

	// Swap buffers
	_gfx->flipBuffer();
}

bool StarkEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

bool StarkEngine::canLoadGameStateCurrently() {
	return true;
}

Common::Error StarkEngine::loadGameState(int slot) {
	// Open the save file
	Common::String filename = Common::String::format("Save%02d.tlj", slot);
	Common::InSaveFile *save = _saveFileMan->openForLoading(filename);
	if (!save) {
		return _saveFileMan->getError();
	}

	StateReadStream *stream = new StateReadStream(save);

	// 1. Read the header
	// Save description
	Common::String desc = stream->readString();

	// Level
	Common::String level = stream->readString();
	uint levelIndex = strtol(level.c_str(), nullptr, 16);

	// Location
	Common::String location = stream->readString();
	uint locationIndex = strtol(location.c_str(), nullptr, 16);

	// Version
	Common::String version = stream->readString();
	//TODO: Check the version

	// 2. Read the resource trees state
	_stateProvider->readStateFromStream(stream);

	//TODO: Read the rest of the state

	delete stream;

	_resourceProvider->shutdown();
	_resourceProvider->initGlobal();

	//TODO: Restore to the correct location
	_resourceProvider->requestLocationChange(levelIndex, locationIndex);

	return Common::kNoError;
}

bool StarkEngine::canSaveGameStateCurrently() {
	return true;
}

Common::Error StarkEngine::saveGameState(int slot, const Common::String &desc) {
	// Ensure the state store is up to date
	_resourceProvider->commitActiveLocationsState();

	// Open the save file
	Common::String filename = Common::String::format("Save%02d.tlj", slot);
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

} // End of namespace Stark
