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

#include "common/system.h"
#include "common/debug-channels.h"
#include "common/debug.h"
#include "common/error.h"

#include "engines/util.h"

#include "macventure/macventure.h"

namespace MacVenture {

MacVentureEngine::MacVentureEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst) {
	_gameDescription = gameDesc;
	_rnd = new Common::RandomSource("macventure");

	_debugger= NULL;

	debug("MacVenture::MacVentureEngine()");
}

MacVentureEngine::~MacVentureEngine() {
	debug("MacVenture::~MacVentureEngine()");

	DebugMan.clearAllDebugChannels();
	delete _rnd;
	delete _debugger;
}

Common::Error MacVentureEngine::run() {
	debug("MacVenture::MacVentureEngine::init()");

	initGraphics(kScreenWidth, kScreenHeight, true);	

	_debugger = new Console(this);

	// Additional setup.
	debug("MacVentureEngine::init");

	_screen.create(kScreenWidth, kScreenHeight, Graphics::PixelFormat::createFormatCLUT8());

	_wm = new Graphics::MacWindowManager();
	_wm->setScreen(&_screen);

	_screen.fillRect(Common::Rect(0, 0, _screen.w, _screen.h), Graphics::kColorWhite);

	Graphics::MacWindow *w = _wm->addWindow(false, true, true);
	w->setDimensions(Common::Rect(100, 100));

	// Your main even loop should be (invoked from) here.
	debug("MacVentureEngine::go: Hello, World!");

	_shouldQuit = false;
	while (!_shouldQuit) {
		processEvents();

		_wm->draw();

		g_system->updateScreen();
		g_system->delayMillis(50);
	}

	return Common::kNoError;
}

void MacVentureEngine::processEvents() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
			case Common::EVENT_QUIT:
				_shouldQuit = true;
				break;
			default: 
				break;
		}
	}
}

} // End of namespace MacVenture
