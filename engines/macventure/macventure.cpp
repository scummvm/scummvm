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

// To move
#include "common/file.h"

namespace MacVenture {

enum {
	kMaxMenuTitleLength = 30
};

enum {
	kGlobalSettingsID = 0x80,
	kDiplomaGeometryID = 0x81,
	kTextHuffmanTableID = 0x83
};

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
	delete _gui;
}

Common::Error MacVentureEngine::run() {
	debug("MacVenture::MacVentureEngine::init()");

	initGraphics(kScreenWidth, kScreenHeight, true);

	_debugger = new Console(this);

	// Additional setup.
	debug("MacVentureEngine::init");

	_resourceManager = new Common::MacResManager();
	if (!_resourceManager->open(getGameFileName()))
		error("Could not open %s as a resource fork", getGameFileName());

	if (!loadGlobalSettings())
		error("Could not load the engine settings");

	_gui = new Gui(this, _resourceManager);

	_shouldQuit = false;
	while (!_shouldQuit) {
		processEvents();

		_gui->draw();

		g_system->updateScreen();
		g_system->delayMillis(50);
	}

	return Common::kNoError;
}

void MacVentureEngine::requestQuit() {
	_shouldQuit = true;
}

void MacVentureEngine::requestUnpause() {
	_paused = false;
}

const GlobalSettings& MacVentureEngine::getGlobalSettings() {
	return _globalSettings;
}

// Data retrieval

bool MacVentureEngine::isPaused() {
	return _paused;
}

Common::String MacVentureEngine::getCommandsPausedString() {
	return Common::String("Click to continue");
}

void MacVentureEngine::processEvents() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		if (_gui->processEvent(event))
			continue;

		switch (event.type) {
		case Common::EVENT_QUIT:
			_shouldQuit = true;
			break;
		default:
			break;
		}
	}
}

// Data loading

bool MacVentureEngine::loadGlobalSettings() {
	Common::MacResIDArray resArray;
	Common::SeekableReadStream *res;

	if ((resArray = _resourceManager->getResIDArray(MKTAG('G', 'N', 'R', 'L'))).size() == 0)
		return false;

	res = _resourceManager->getResource(MKTAG('G', 'N', 'R', 'L'), kGlobalSettingsID);
	if (res) {
		_globalSettings.numObjects = res->readUint16BE();
		_globalSettings.numGlobals = res->readUint16BE();
		_globalSettings.numCommands = res->readUint16BE();
		_globalSettings.numAttributes = res->readUint16BE();
		_globalSettings.numGroups = res->readUint16BE();
		res->readUint16BE(); // unknown
		_globalSettings.invTop = res->readUint16BE();
		_globalSettings.invLeft = res->readUint16BE();
		_globalSettings.invWidth = res->readUint16BE();
		_globalSettings.invHeight = res->readUint16BE();
		_globalSettings.invOffsetY = res->readUint16BE();
		_globalSettings.invOffsetX = res->readSint16BE();
		_globalSettings.defaultFont = res->readUint16BE();
		_globalSettings.defaultSize = res->readUint16BE();

		_globalSettings.attrIndices = new uint8[_globalSettings.numAttributes];
		res->read(_globalSettings.attrIndices, _globalSettings.numAttributes);

		_globalSettings.attrMasks = new uint16[_globalSettings.numAttributes];
		for (int i = 0; i < _globalSettings.numAttributes; i++)
			_globalSettings.attrMasks[i] = res->readUint16BE();

		_globalSettings.attrShifts = new uint8[_globalSettings.numAttributes];
		res->read(_globalSettings.attrShifts, _globalSettings.numAttributes);

		_globalSettings.cmdArgCnts = new uint8[_globalSettings.numCommands];
		res->read(_globalSettings.cmdArgCnts, _globalSettings.numCommands);

		_globalSettings.commands = new uint8[_globalSettings.numCommands];
		res->read(_globalSettings.commands, _globalSettings.numCommands);

		return true;
	}

	return false;
}


} // End of namespace MacVenture
