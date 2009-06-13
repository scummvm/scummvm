/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/config-manager.h"
#include "common/events.h"
#include "common/system.h"
#include "common/file.h"

#include "asylum/asylum.h"

namespace Asylum {

AsylumEngine::AsylumEngine(OSystem *system, Common::Language language)
    : Engine(system) {

    Common::File::addDefaultDirectory(_gameDataDir.getChild("Data"));
    Common::File::addDefaultDirectory(_gameDataDir.getChild("Vids"));
	Common::File::addDefaultDirectory(_gameDataDir.getChild("Music"));

    _eventMan->registerRandomSource(_rnd, "asylum");
}

AsylumEngine::~AsylumEngine() {
    //Common::clearAllDebugChannels();
	delete _scene;
    delete _resMgr;
	_backBuffer.free();
}

Common::Error AsylumEngine::run() {
    Common::Error err;
    err = init();
    if (err != Common::kNoError)
            return err;
    return go();
}

// Will do the same as subroutine at address 0041A500
Common::Error AsylumEngine::init() {
	// initialize engine objects

	initGraphics(640, 480, true);
	_backBuffer.create(640, 480, 1);

	_resMgr = new ResourceManager(this);
    _scene = new Scene(this);

	// initializing game
	// TODO: save dialogue key codes into sntrm_k.txt (need to figure out why they use such thing)
	// TODO: get hand icon resource before starting main menu
	// TODO: load startup configurations (address 0041A970)
	// TODO: setup cinematics (address 0041A880) (probably we won't need it)
	// TODO: init unknown game stuffs (address 0040F430)

	// TODO: if savegame exists on folder, than start NewGame()

    return Common::kNoError;
}

Common::Error AsylumEngine::go() {
	// Play intro movie
	// Disabled for quick testing
	//_resMgr->loadVideo(0);

    // TODO: just some scene proof-of-concept
    _scene->load(5);

	// DEBUG
    // Control loop test. Basically just keep the
    // ScummVM window alive until ESC is pressed.
    // This will facilitate drawing tests ;)

	// DEBUG
	// Testing new game state abstraction class
	_state = new MenuState(this);

	while (!shouldQuit()) {

		checkForEvent();

		// Copy background image
		_system->copyRectToScreen((byte *)_backBuffer.pixels, _backBuffer.w, 0, 0, _backBuffer.w, _backBuffer.h);

		waitForTimer(60);
	}

    return Common::kNoError;
}

void AsylumEngine::waitForTimer(int msec_delay) {
	uint32 start_time = _system->getMillis();

	while (_system->getMillis() < start_time + msec_delay) {
		checkForEvent();
		_system->updateScreen();
		_system->delayMillis(10);
	}
}

void AsylumEngine::checkForEvent() {
	Common::Event ev;

	if (_system->getEventManager()->pollEvent(ev)) {
		if (ev.type == Common::EVENT_KEYDOWN) {
			if (ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
				// Push a quit event
				Common::Event event;
				event.type = Common::EVENT_QUIT;
				g_system->getEventManager()->pushEvent(event);
			}
		}
	}
	_state->handleEvent(&ev);
}

void AsylumEngine::copyToBackBuffer(byte *buffer, int x, int y, int width, int height) {
	int h = height;
	byte *dest = (byte *)_backBuffer.pixels;

	while (h--) {
		memcpy(dest, buffer, width);
		dest += 640;
		buffer += width;
	}
}

void AsylumEngine::copyRectToScreenWithTransparency(byte *buffer, int x, int y, int width, int height) {
	byte *screenBuffer = (byte *)_system->lockScreen()->pixels;

	for (int curY = 0; curY < height; curY++) {
		for (int curX = 0; curX < width; curX++) {
			if (buffer[curX + curY * width] != 0) {
				screenBuffer[x + curX + (y + curY) * 640] = buffer[curX + curY * width];
			}
		}
	}

	_system->unlockScreen();
}

} // namespace Asylum
