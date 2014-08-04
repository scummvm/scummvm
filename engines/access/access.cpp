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
 */

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "engines/util.h"
#include "access/access.h"

namespace Access {

AccessEngine::AccessEngine(OSystem *syst, const AccessGameDescription *gameDesc) :
		_gameDescription(gameDesc), Engine(syst), _randomSource("Access") {
	_debugger = nullptr;
	_events = nullptr;
	_files = nullptr;
	_graphics = nullptr;
	_screen = nullptr;
	_sound = nullptr;

	_destIn = nullptr;
	_objectsTable = nullptr;
}

AccessEngine::~AccessEngine() {
	delete _debugger;
	delete _events;
	delete _files;
	delete _graphics;
	delete _screen;
	delete _sound;

	_buffer1.free();
	_buffer2.free();
	delete[] _objectsTable;
}

void AccessEngine::setVGA() {
	initGraphics(320, 200, false);
}

void AccessEngine::initialize() {
	// Set up debug channels
	DebugMan.addDebugChannel(kDebugPath, "Path", "Pathfinding debug level");
	DebugMan.addDebugChannel(kDebugScripts, "scripts", "Game scripts");
	DebugMan.addDebugChannel(kDebugGraphics, "graphics", "Graphics handling");

	_debugger = new Debugger(this);
	_events = new EventsManager(this);
	_files = new FileManager(this);
	_graphics = new GraphicsManager(this);
	_screen = new Screen(this);
	_sound = new SoundManager(this, _mixer);

	_buffer1.create(g_system->getWidth(), g_system->getHeight(), Graphics::PixelFormat::createFormatCLUT8());
	_buffer2.create(g_system->getWidth(), g_system->getHeight(), Graphics::PixelFormat::createFormatCLUT8());
}

Common::Error AccessEngine::run() {
	setVGA();
	initialize();

	_screen->setInitialPalettte();
	_events->setCursor(CURSOR_0);
	_events->showCursor();
	_graphics->setPanel(0);
	doTitle();

	dummyLoop();
	return Common::kNoError;
}

void AccessEngine::dummyLoop() {
	// Dummy game loop
	while (!shouldQuit()) {
		_events->pollEvents();
		g_system->delayMillis(50);
		g_system->updateScreen();

		if (_events->_leftButton) {
			CursorType cursorId = _events->getCursor();
			_events->setCursor((cursorId == CURSOR_HELP) ? CURSOR_0 : (CursorType)(cursorId + 1));
		}
	}

}

int AccessEngine::getRandomNumber(int maxNumber) {
	return _randomSource.getRandomNumber(maxNumber);
}

} // End of namespace Access
