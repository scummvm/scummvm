/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "engines/stark/stark.h"
#include "engines/stark/archive.h"
#include "engines/stark/debug.h"

#include "common/config-manager.h"
#include "common/events.h"
#include "sound/mixer.h"

namespace Stark {

StarkEngine::StarkEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc), _gfx(NULL), _scene(NULL) {
	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, 127);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	// Add the available debug channels
	Common::addDebugChannel(kDebugArchive, "Archive", "Debug the archive loading");
	Common::addDebugChannel(kDebugXMG, "XMG", "Debug the loading of XMG images");
	Common::addDebugChannel(kDebugUnknown, "Unknown", "Debug unknown values on the data");
}

StarkEngine::~StarkEngine() {
	delete _scene;
}

Common::Error StarkEngine::run() {
	_gfx = GfxDriver::create();

	// Get the screen prepared
	_gfx->setupScreen(640, 480, ConfMan.getBool("fullscreen"));

	// Start running
	mainLoop();

	return Common::kNoError;
}

void StarkEngine::mainLoop() {
	// Load the initial scene
	_scene = new Scene(_gfx);

	while (!shouldQuit()) {
		// Process events
		Common::Event e;
		while (g_system->getEventManager()->pollEvent(e)) {
			// Handle any buttons, keys and joystick operations
			if (e.type == Common::EVENT_KEYDOWN) {
				if (e.kbd.ascii == 'q') {
					quitGame();
					break;
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
	// Clear the screen
	_gfx->clearScreen();

	// Render the current scene
	_scene->render();

	// Swap buffers
	_gfx->flipBuffer();
}

} // End of namespace Stark
