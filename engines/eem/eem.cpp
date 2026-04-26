/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/events.h"
#include "common/system.h"

#include "engines/util.h"

#include "graphics/paletteman.h"

#include "eem/console.h"
#include "eem/detection.h"
#include "eem/eem.h"

namespace EEM {

EEMEngine::EEMEngine(OSystem *syst, const ADGameDescription *gameDesc)
	: Engine(syst), _gameDescription(gameDesc), _console(nullptr),
	  _rng("eem"), _lastScreen(kScreenInvalid), _nextScreen(kScreenTitle) {
}

EEMEngine::~EEMEngine() {
	// _console is owned by the Engine base class.
}

Common::Error EEMEngine::run() {
	// Original _main @ 1a35:0f59 enters mode 13h via _SetMode13X (320x200x256).
	initGraphics(320, 200);

	_console = new Console();
	setDebugger(_console);

	// _main's startup paints the screen black via _AllBlack @ 172b:0d4b before
	// the first screen handler runs; we do the same here.
	byte palette[3 * 256] = { 0 };
	g_system->getPaletteManager()->setPalette(palette, 0, 256);

	debugC(1, kDebugGeneral, "EEM engine starting; first screen = 0x%02X", _nextScreen);

	screenDriver();

	debugC(1, kDebugGeneral, "EEM engine exiting");
	return Common::kNoError;
}

void EEMEngine::screenDriver() {
	// Mirrors _ScreenDriver @ 1a35:0dc1. The original walks a 14-entry table at
	// 1a35:0e5e of (id, handler) pairs; we use a switch as we port handlers in.
	while (_nextScreen != kScreenInvalid && !shouldQuit()) {
		ScreenId next = static_cast<ScreenId>(_nextScreen);
		switch (next) {
		case kScreenTitle:
			// TODO(M3): port _ShowTitlePage @ 1a35:06b7
			warning("Screen 0x%02X (title) not implemented yet", next);
			_lastScreen = _nextScreen;
			_nextScreen = kScreenInvalid;
			break;
		default:
			warning("Unknown screen id 0x%02X; exiting", next);
			_nextScreen = kScreenInvalid;
			break;
		}

		// Until handlers run their own event loops, pump events here so the
		// engine remains responsive and the user can quit.
		if (!pollEvents())
			break;
	}
}

bool EEMEngine::pollEvents() {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			return false;
		default:
			break;
		}
	}
	g_system->updateScreen();
	g_system->delayMillis(10);
	return true;
}

} // End of namespace EEM
