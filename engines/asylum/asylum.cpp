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
	int mouseX = 0, mouseY = 0;

	//_resMgr->loadVideo(0);

	showMainMenu();

	// DEBUG
    // Control loop test. Basically just keep the
    // ScummVM window alive until ESC is pressed.
    // This will facilitate drawing tests ;)

	// DEBUG
	uint32 lastUpdate = 0;

	Common::EventManager *em = _system->getEventManager();
	while (!shouldQuit()) {
		Common::Event ev;

		// Copy background image
		_system->copyRectToScreen((byte *)_backBuffer.pixels, _backBuffer.w, 0, 0, _backBuffer.w, _backBuffer.h);

		if (em->pollEvent(ev)) {
			if (ev.type == Common::EVENT_KEYDOWN) {
				if (ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
					// Push a quit event
					Common::Event event;
					event.type = Common::EVENT_QUIT;
					g_system->getEventManager()->pushEvent(event);
				}
				//if (ev.kbd.keycode == Common::KEYCODE_RETURN)
			} else if (ev.type == Common::EVENT_MOUSEMOVE) {
				mouseX = ev.mouse.x;
				mouseY = ev.mouse.y;
			}
		}

		// TODO: Just some proof-of concept to change icons here for now
		// Top row
		if (mouseY >= 20 && mouseY <= 20 + 48) {
			for (int i = 0; i <= 5; i++) {
				int curX = 40 + i * 100;
				if (mouseX >= curX && mouseX <= curX + 55) {
					GraphicResource *res = _resMgr->getGraphic(1, i + 4, 0);
					_system->copyRectToScreen(res->data, res->width, curX, 20, res->width, res->height);
				}
			}
		}

		// Bottom row
		if (mouseY >= 400 && mouseY <= 400 + 48) {
			for (int i = 0; i <= 5; i++) {
				int curX = 40 + i * 100;
				if (mouseX >= curX && mouseX <= curX + 55) {
					int graph = i + 10;

					// The last 2 icons are swapped
					if (graph == 14)
						graph = 15;
					else if (graph == 15)
						graph = 14;

					GraphicResource *res = _resMgr->getGraphic(1, graph, 0);
					_system->copyRectToScreen(res->data, res->width, curX, 400, res->width, res->height);
				}
			}
		}

		if (_system->getMillis() - lastUpdate > 50) {
			_system->updateScreen();
			lastUpdate = _system->getMillis();
		}

		_system->delayMillis(10);
	}

    return Common::kNoError;
}

void AsylumEngine::showMainMenu() {
	// main menu background
	_resMgr->loadGraphic(1, 0, 0);
	_resMgr->loadPalette(1, 17);
	_resMgr->loadCursor(1, 2, 0);
	_resMgr->loadMusic();
}

void AsylumEngine::copyToBackBuffer(int x, int y, int width, int height, byte *buffer) {
	int h = height;
	byte *dest = (byte *)_backBuffer.pixels;

	while (h--) {
		memcpy(dest, buffer, width);
		dest += 640;
		buffer += width;
	}
}
} // namespace Asylum
