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
 */

#include "common/config-manager.h"
#include "common/events.h"
#include "common/system.h"
#include "common/file.h"

#include "asylum/asylum.h"
#include "asylum/screen.h"

namespace Asylum {

AsylumEngine::AsylumEngine(OSystem *system, Common::Language language)
    : Engine(system) {

    Common::File::addDefaultDirectory(_gameDataDir.getChild("Data"));
    Common::File::addDefaultDirectory(_gameDataDir.getChild("Vids"));

    _eventMan->registerRandomSource(_rnd, "asylum");
}

AsylumEngine::~AsylumEngine() {
    //Common::clearAllDebugChannels();
    delete _screen;
	delete _resMgr;
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

	_screen = new Screen(_system);
	_resMgr = new ResourceManager;

	// initializing game
	// TODO: save dialogue key codes into sntrm_k.txt (need to figure out why they use such thing)
	// TODO: get hand icon resource before starting main menu
	// TODO: load startup configurations (address 0041A970)
	// TODO: setup cinematics (address 0041A880) (probably we won't need it)
	// TODO: init unknown game stuffs (address 0040F430)

	// TODO: load smaker intro movie (0)->(mov000.smk)
	// TODO: if savegame exists on folder, than start NewGame()

    return Common::kNoError;
}

Common::Error AsylumEngine::go() {

	showMainMenu();

	// DEBUG
    // Control loop test. Basically just keep the
    // ScummVM window alive until ESC is pressed.
    // This will facilitate drawing tests ;)
    bool end = false;

    bool forceRefresh = false;
    uint32 lastRefresh = 0;

	Common::EventManager *em = _system->getEventManager();
	while (!end) {
		Common::Event ev;
		if (em->pollEvent(ev)) {
			if (ev.type == Common::EVENT_KEYDOWN) {
				if (ev.kbd.keycode == Common::KEYCODE_ESCAPE)
					end = true;
				//if (ev.kbd.keycode == Common::KEYCODE_RETURN)
			}
			if (ev.type == Common::EVENT_MOUSEMOVE) {
				if (_system->getMillis() - lastRefresh > 50) {
					forceRefresh = true;
				}

			}

		}
		if (forceRefresh){
			_screen->updateScreen();
			forceRefresh = false;
			lastRefresh = _system->getMillis();
		}
		//_system->delayMillis(10);
	}

    return Common::kNoError;
}

void AsylumEngine::showMainMenu() {
	// eyes animation index table
	//const uint32 eyesTable[8] = {3, 5, 1, 7, 4, 8, 2, 6};
	PaletteBundle   *pal = _resMgr->getPalette(1, 17);
	GraphicResource *bg  = _resMgr->getGraphic(1, 0)->getEntry(0);
	GraphicResource *cur = _resMgr->getGraphic(1, 2)->getEntry(0);

	_system->setMouseCursor(cur->data, cur->width, cur->height, 1, 1, 0);
	_system->setCursorPalette(pal->getPalette(), 0, 1024);
	_system->showMouse(true);

	_screen->setFrontBuffer(
			0, 0,
			bg->width, bg->height,
			bg->data);
	_screen->setPalette(pal->getPalette());
	_screen->updateScreen();
}

} // namespace Asylum
