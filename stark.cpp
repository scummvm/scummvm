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
#include "engines/stark/xmg.h"

#include "common/config-manager.h"
#include "common/events.h"
#include "sound/mixer.h"

namespace Stark {

StarkEngine::StarkEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc), _gfx(NULL) {
	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, 127);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));
}

StarkEngine::~StarkEngine() {
	_xArchive.close();
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
	for (;;) {
		// Process events
		Common::Event e;
		while (g_system->getEventManager()->pollEvent(e)) {
			// Handle any buttons, keys and joystick operations
			if (e.type == Common::EVENT_KEYDOWN) {
				if (e.kbd.ascii == 'q') {
					/*handleExit();*/
					break;
				} else {
					//handleChars(event.type, event.kbd.keycode, event.kbd.flags, event.kbd.ascii);
				}
			}
			/*if (event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_KEYUP) {
				handleControls(event.type, event.kbd.keycode, event.kbd.flags, event.kbd.ascii);
			}*/
			// Check for "Hard" quit"
			if (e.type == Common::EVENT_QUIT)
				return;
			/*if (event.type == Common::EVENT_SCREEN_CHANGED)
				_refreshDrawNeeded = true;*/
		}

		updateDisplayScene();
		g_system->delayMillis(50);
	}
}

void StarkEngine::updateDisplayScene() {
	_gfx->clearScreen();

	// Draw bg

	// Draw other things
	XMGDecoder *xmg = new XMGDecoder();
	Common::File _f;
	_f.open("house_layercenter.xmg");
	Common::SeekableReadStream *dat = _f.readStream(_f.size());
	Graphics::Surface *surf;
	surf = xmg->decodeImage(dat);
	_gfx->drawSurface(surf);
	delete xmg;
	delete surf;
	delete dat;

	_f.close();

	// setup cam

	//_gfx->set3DMode();

	// setup lights

	// draw actors

	// draw overlay

	// Swap buffers
	_gfx->flipBuffer();
}

} // End of namespace Stark
