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

#include "voyeur/voyeur.h"
#include "voyeur/graphics.h"
#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"

namespace Voyeur {

VoyeurEngine *g_vm;

VoyeurEngine::VoyeurEngine(OSystem *syst, const VoyeurGameDescription *gameDesc) : Engine(syst),
		_gameDescription(gameDesc), _randomSource("Voyeur") {
	DebugMan.addDebugChannel(kDebugPath, "Path", "Pathfinding debug level");
	_bVoy = NULL;

	initialiseManagers();
}

VoyeurEngine::~VoyeurEngine() {
	delete _bVoy;
}

Common::String VoyeurEngine::generateSaveName(int slot) {
	return Common::String::format("%s.%03d", _targetName.c_str(), slot);
}

/**
 * Returns true if it is currently okay to restore a game
 */
bool VoyeurEngine::canLoadGameStateCurrently() {
	return true;
}

/**
 * Returns true if it is currently okay to save the game
 */
bool VoyeurEngine::canSaveGameStateCurrently() {
	return true;
}

/**
 * Load the savegame at the specified slot index
 */
Common::Error VoyeurEngine::loadGameState(int slot) {
	return Common::kNoError;
}

/**
 * Save the game to the given slot index, and with the given name
 */
Common::Error VoyeurEngine::saveGameState(int slot, const Common::String &desc) {
	//TODO
	return Common::kNoError;
}

Common::Error VoyeurEngine::run() {
	ESP_Init();
	globalInitBolt();

	_eventsManager.resetMouse();
	doHeadTitle();

	//doHeadTitle();

	return Common::kNoError;
}


int VoyeurEngine::getRandomNumber(int maxNumber) {
	return _randomSource.getRandomNumber(maxNumber);
}

void VoyeurEngine::initialiseManagers() {
	_debugger.setVm(this);
	_eventsManager.setVm(this);
	_filesManager.setVm(this);
	_graphicsManager.setVm(this);
}

void VoyeurEngine::ESP_Init() {
}

void VoyeurEngine::globalInitBolt() {
	initBolt();

	_filesManager.openBoltLib("bvoy.blt", _bVoy);
	_bVoy->getBoltGroup(0x10000);
	_bVoy->getBoltGroup(0x10100);
	_fontPtr = _bVoy->memberAddr(0x101);

	// Setup default flags
	Common::fill((byte *)&_voy, (byte *)&_voy + sizeof(SVoy), 0);
	_voy._eCursorOff[0x74 / 2] = 1;
	_voy._eCursorOff[0x68 / 2] = 0;
	_voy._eventTable[998]._data3 = 63;
	_voy._eventTable[998]._data4 = 63;
	_voy._evidence[19] = 0;
	_voy._evidence[17] = 0;
	_voy._evidence[18] = 9999;
	
	_voy._curICF0 = _graphicsManager._palFlag ? 0xFFFFA5E0 : 0x5F90; 
	_graphicsManager.addFadeInt();
}

void VoyeurEngine::initBolt() {
	vInitInterrupts();
	_graphicsManager.sInitGraphics();
	_graphicsManager.vInitColor();
	initInput();
}

void VoyeurEngine::vInitInterrupts() {
	_eventsManager._intPtr._palette = &_graphicsManager._VGAColors[0];
}

void VoyeurEngine::initInput() {
}

void VoyeurEngine::doHeadTitle() {
//	char dest[144];

	_eventsManager.startMainClockInt();

	// Show starting screen
	if (_bVoy->getBoltGroup(0x10500)) {
		_graphicsManager._backgroundPage = _bVoy->getBoltEntry(0x5020000)._picResource;
		(*_graphicsManager._vPort)->setupViewPort();
		(*_graphicsManager._vPort)->_flags |= 8;

		_graphicsManager.flipPage();
		_eventsManager.sWaitFlip();

		// Fade in the screen
		CMapResource *cMap = _bVoy->getCMapResource(0x5010000);
		assert(cMap);
		cMap->_steps = 60;
		cMap->startFade();

		_eventsManager.delay(150);
		if (shouldQuit())
			return;
		/* Commented out until fade in is working
		// Fade out the screen
		cMap->_steps = 30;
		cMap->startFade();
		if (shouldQuit())
			return;

		(*_graphicsManager._vPort)->_flags |= 8;
		_graphicsManager.flipPage();
		_eventsManager.sWaitFlip();

		while (!shouldQuit() && (_graphicsManager._fadeStatus & 1))
			_eventsManager.delay(1);

		_graphicsManager.screenReset();
		_bVoy->freeBoltGroup(0x10500);
		_graphicsManager.screenReset();

		if (shouldQuit())
			return;
		*/
	}
}

} // End of namespace Voyeur
