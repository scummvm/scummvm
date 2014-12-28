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
#include "engines/stark/archiveloader.h"
#include "engines/stark/console.h"
#include "engines/stark/debug.h"
#include "engines/stark/resourceprovider.h"
#include "engines/stark/resources/level.h"
#include "engines/stark/resources/location.h"
#include "engines/stark/scene.h"
#include "engines/stark/stateprovider.h"
#include "engines/stark/gfx/driver.h"

#include "common/config-manager.h"
#include "common/events.h"
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
		_archiveLoader(nullptr),
		_stateProvider(nullptr),
		_resourceProvider(nullptr) {
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
	delete _scene;
	delete _console;
	delete _gfx;
	delete _resourceProvider;
	delete _global;
	delete _stateProvider;
	delete _archiveLoader;
}

Common::Error StarkEngine::run() {
	_console = new Console(this);
	_gfx = GfxDriver::create();

	// Get the screen prepared
	_gfx->setupScreen(640, 480, ConfMan.getBool("fullscreen"));

	_archiveLoader = new ArchiveLoader();
	_stateProvider = new StateProvider();
	_global = new Global();
	_resourceProvider = new ResourceProvider(_archiveLoader, _stateProvider, _global);

	// Load global resources
	_resourceProvider->initGlobal();

	// Start us up at the house of all worlds
	_resourceProvider->requestLocationChange(0x45, 0x00);

	// Start running
	mainLoop();

	_resourceProvider->shutdown();

	return Common::kNoError;
}

void StarkEngine::mainLoop() {
	// Load the initial scene
	_scene = new Scene(_gfx);

	while (!shouldQuit()) {
		if (_resourceProvider->hasLocationChangeRequest()) {
			_resourceProvider->performLocationChange();
		}

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

		updateDisplayScene();
		g_system->delayMillis(50);
	}
}

void StarkEngine::updateDisplayScene() {
	// Get frame delay
	static uint32 lastFrame = g_system->getMillis();
	uint32 delta = g_system->getMillis() - lastFrame;
	lastFrame += delta;

	// Clear the screen
	_gfx->clearScreen();

	// Update the game resources
	_global->getLevel()->onGameLoop(delta);
	_global->getCurrent()->getLevel()->onGameLoop(delta);
	_global->getCurrent()->getLocation()->onGameLoop(delta);

	// Render the current scene
	_scene->render(delta);

	// Swap buffers
	_gfx->flipBuffer();
}

} // End of namespace Stark
