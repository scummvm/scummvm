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

#include "common/str.h"
#include "awe/awe.h"
#include "awe/engine.h"
#include "awe/file.h"
#include "awe/serializer.h"
#include "awe/systemstub.h"

namespace Awe {

Engine::Engine(SystemStub *stub, const char *dataDir, const char *saveDir)
	: _stub(stub), _log(&_res, &_vid, stub), _res(&_vid, dataDir), _vid(&_res, stub), 
	_dataDir(dataDir), _saveDir(saveDir), _stateSlot(0) {
}

void Engine::run() {
	_stub->init("Out Of This World");
	setup();
	// XXX
	_log.restartAt(0x3E80); // demo starts at 0x3E81
	while (!_stub->_pi.quit && !g_engine->shouldQuit()) {
		_log.setupScripts();
		_log.inp_updatePlayer();
		processInput();
		_log.runScripts();
	}
	finish();
	_stub->destroy();
}

void Engine::setup() {
	_vid.init();
	_res.allocMemBlock();
	_res.readEntries();
	_log.init();
}

void Engine::finish() {
	 // XXX
	 _res.freeMemBlock();
}

void Engine::processInput() {
	if (_stub->_pi.load) {
		loadGameState(_stateSlot);
		_stub->_pi.load = false;
	}
	if (_stub->_pi.save) {
		saveGameState(_stateSlot, "quicksave");
		_stub->_pi.save = false;
	}
	if (_stub->_pi.fastMode) {
		_log._fastMode = !_log._fastMode;
		_stub->_pi.fastMode = false;
	}
	if (_stub->_pi.stateSlot != 0) {
		int8 slot = _stateSlot + _stub->_pi.stateSlot;
		if (slot >= 0 && slot < MAX_SAVE_SLOTS) {
			_stateSlot = slot;
			debug(DBG_INFO, "Current game state slot is %d", _stateSlot);
		}
		_stub->_pi.stateSlot = 0;
	}
}

void Engine::makeGameStateName(uint8 slot, char *buf) {
	Common::sprintf_s(buf, 256, "raw.s%02d", slot);
}

void Engine::saveGameState(uint8 slot, const char *desc) {
#ifdef TODO
	char stateFile[20];
	makeGameStateName(slot, stateFile);
	File f(true);
	if (!f.open(stateFile, _saveDir, "wb")) {
		::warning("Unable to save state file '%s'", stateFile);
	} else {
		// header
		f.writeUint32BE('AWSV');
		f.writeUint16BE(Serializer::CUR_VER);
		f.writeUint16BE(0);
		char hdrdesc[32];
		strncpy(hdrdesc, desc, sizeof(hdrdesc) - 1);
		f.write(hdrdesc, sizeof(hdrdesc));
		// contents
		Serializer s(&f, Serializer::SM_SAVE, _res._memPtrStart);
		_log.saveOrLoad(s);
		_res.saveOrLoad(s);
		_vid.saveOrLoad(s);
		if (f.ioErr()) {
			::warning("I/O error when saving game state");
		} else {
			debug(DBG_INFO, "Saved state to slot %d", _stateSlot);
		}
	}
#endif
}

void Engine::loadGameState(uint8 slot) {
#ifdef TODO
	char stateFile[20];
	makeGameStateName(slot, stateFile);
	File f(true);
	if (!f.open(stateFile, _saveDir, "rb")) {
		::warning("Unable to open state file '%s'", stateFile);
	} else {
		uint32 id = f.readUint32BE();
		if (id != 'AWSV') {
			::warning("Bad savegame format");
		} else {
			uint16 ver = f.readUint16BE();
			f.readUint16BE();
			char hdrdesc[32];
			f.read(hdrdesc, sizeof(hdrdesc));
			// contents
			Serializer s(&f, Serializer::SM_LOAD, _res._memPtrStart, ver);
			_log.saveOrLoad(s);
			_res.saveOrLoad(s);
			_vid.saveOrLoad(s);
		}
		if (f.ioErr()) {
			::warning("I/O error when loading game state");
		} else {
			debug(DBG_INFO, "Loaded state from slot %d", _stateSlot);
		}
	}
#endif
}

} // namespace Awe
