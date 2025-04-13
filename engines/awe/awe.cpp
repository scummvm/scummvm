/* ScummVM - Graphic Adventure AweEngine
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

#include "audio/mixer.h"
#include "common/config-manager.h"
#include "engines/util.h"
#include "awe/util.h"
#include "awe/awe.h"

namespace Awe {

AweEngine::AweEngine(OSystem *syst, const ADGameDescription *gameDesc)
	: Engine(syst), _gameDescription(gameDesc),
	_stub(SystemStub_SDL_create()),
	_res(&_vid),
	_vid(&_res, _stub),
	_log(&_res, &_vid, _stub) {
}

AweEngine::~AweEngine() {
	delete _stub;
}

Common::Error AweEngine::run() {
	// Setup mixer
	if (!_mixer->isReady()) {
		::warning("Sound initialization failed.");
	}

	// Initialize backend
	initGraphics(320, 200);

	// Setup
	_stub->init("Out Of This World");
	setup();

	// Run the game
	_log.restartAt(0x3E80); // demo starts at 0x3E81
	while (!_stub->_pi.quit && !g_engine->shouldQuit()) {
		_log.setupScripts();
		_log.inp_updatePlayer();
		processInput();
		_log.runScripts();
	}

	finish();
	_stub->destroy();

	return Common::kNoError;
}

void AweEngine::setup() {
	_vid.init();
	_res.allocMemBlock();
	_res.readEntries();
	_log.init();
}

void AweEngine::finish() {
	// XXX
	_res.freeMemBlock();
}

void AweEngine::processInput() {
	if (_stub->_pi.load) {
		loadGameState(_stateSlot);
		_stub->_pi.load = false;
	}
	if (_stub->_pi.save) {
		saveGameState(_stateSlot, "Quicksave");
		_stub->_pi.save = false;
	}
	if (_stub->_pi.fastMode) {
		_log._fastMode = !_log._fastMode;
		_stub->_pi.fastMode = false;
	}
	if (_stub->_pi.stateSlot != 0) {
		int8 slot = _stateSlot + _stub->_pi.stateSlot;
		if (slot >= 0 && slot < 999) {
			_stateSlot = slot;
			debugC(kDebugInfo, "Current game state slot is %d", _stateSlot);
		}
		_stub->_pi.stateSlot = 0;
	}
}

} // namespace Awe
