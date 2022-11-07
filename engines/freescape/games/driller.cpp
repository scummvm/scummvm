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

#include "common/events.h"
#include "common/file.h"

#include "freescape/freescape.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

DrillerEngine::DrillerEngine(OSystem *syst, const ADGameDescription *gd) : FreescapeEngine(syst, gd) {
	// if (isAmiga())
	//	_viewArea = Common::Rect(72, 66, 567, 269);
	// else
	_viewArea = Common::Rect(40, 16, 279, 116);
	_playerHeightNumber = 1;
	_playerHeights.push_back(16);
	_playerHeights.push_back(48);
	_playerHeights.push_back(80);
	_playerHeights.push_back(112);

	_playerHeight = _playerHeights[_playerHeightNumber];
	_playerWidth = 12;
	_playerDepth = 32;

	_initialProveEnergy = 48;
	_initialProveShield = 50;
	_initialJetEnergy = 29;
	_initialJetShield = 34;
}

void DrillerEngine::gotoArea(uint16 areaID, int entranceID) {
	debugC(1, kFreescapeDebugMove, "Jumping to area: %d, entrance: %d", areaID, entranceID);
	if (!_gameStateBits.contains(areaID))
		_gameStateBits[areaID] = 0;

	assert(_areaMap.contains(areaID));
	_currentArea = _areaMap[areaID];
	_currentArea->show();

	_currentAreaMessages.clear();
	if (_messagesList.size() > 2) {
		if (_currentArea->_gasPocketRadius > 0)
			_currentAreaMessages.push_back(_messagesList[1]);
		else
			_currentAreaMessages.push_back(_messagesList[2]);

		_currentAreaMessages.push_back(_currentArea->_name);
	}

	if (entranceID > 0 || areaID == 127) {
		traverseEntrance(entranceID);
	} else if (entranceID == 0) {
		Math::Vector3d diff = _lastPosition - _position;
		// debug("dif: %f %f %f", diff.x(), diff.y(), diff.z());
		//  diff should be used to determinate which entrance to use
		int newPos = -1;
		if (ABS(diff.x()) < ABS(diff.z())) {
			if (diff.z() > 0)
				newPos = 4000;
			else
				newPos = 100;
			_position.setValue(2, newPos);
		} else {
			if (diff.x() > 0)
				newPos = 4000;
			else
				newPos = 100;
			_position.setValue(0, newPos);
		}
		assert(newPos != -1);
	}
	_lastPosition = _position;

	debugC(1, kFreescapeDebugMove, "starting player position: %f, %f, %f", _position.x(), _position.y(), _position.z());
	playSound(5, false);
	// Ignore sky/ground fields
	if (_currentArea->getAreaFlags() == 1)
		_gfx->_keyColor = 0;
	else
		_gfx->_keyColor = 255;

	if (isAmiga() || isAtariST())
		swapPalette(areaID);
}

void DrillerEngine::loadGlobalObjects(Common::SeekableReadStream *file, int offset) {
	assert(!_areaMap.contains(255));
	ObjectMap *globalObjectsByID = new ObjectMap;
	file->seek(offset);
	for (int i = 0; i < 8; i++) {
		Object *gobj = load8bitObject(file);
		assert(gobj);
		assert(!globalObjectsByID->contains(gobj->getObjectID()));
		debugC(1, kFreescapeDebugParser, "Adding global object: %d", gobj->getObjectID());
		(*globalObjectsByID)[gobj->getObjectID()] = gobj;
	}

	_areaMap[255] = new Area(255, 0, globalObjectsByID, nullptr);
}

void DrillerEngine::loadAssets() {
	if (isDemo())
		loadAssetsDemo();
	else
		loadAssetsFullGame();

	_angleRotations.push_back(5.0);
	_angleRotations.push_back(9.5);

	// Start playing music, if any, in any supported format
	playMusic("Matt Gray - The Best Of Reformation - 07 Driller Theme");
}

void DrillerEngine::loadAssetsDemo() {
	Common::File file;
	if (isAmiga()) {
		file.open("lift.neo");
		if (!file.isOpen())
			error("Failed to open 'lift.neo' file");

		_title = loadAndConvertNeoImage(&file, 0);

		file.close();
		file.open("console.neo");
		if (!file.isOpen())
			error("Failed to open 'console.neo' file");

		_border = loadAndConvertNeoImage(&file, 0);

		file.close();
		file.open("demo.cmd");
		if (!file.isOpen())
			error("Failed to open 'demo.cmd' file");

		loadDemoData(&file, 0, 0x1000);

		file.close();
		file.open("data");
		if (!file.isOpen())
			error("Failed to open 'data' file");

		// loadGlobalObjects(file, 0xbd62);
		/*file->seek(0x29efe);
		load8bitArea(file, 16);
		file->seek(0x2a450);
		load8bitArea(file, 16);*/

		load8bitBinary(&file, 0x442, 16);
		loadPalettes(&file, 0x0);

		file.close();
		file.open("driller");
		if (!file.isOpen())
			error("Failed to open 'driller' file");
		loadMessagesFixedSize(&file, 0x3960, 14, 20);

		file.close();
		file.open("soundfx");
		if (!file.isOpen())
			error("Failed to open 'soundfx' executable for Amiga");

		loadSoundsFx(&file, 0, 25);
	} else if (isAtariST()) {
		file.open("lift.neo");
		if (!file.isOpen())
			error("Failed to open 'lift.neo' file");

		_title = loadAndConvertNeoImage(&file, 0);

		file.close();
		file.open("console.neo");
		if (!file.isOpen())
			error("Failed to open 'console.neo' file");

		_border = loadAndConvertNeoImage(&file, 0);

		file.close();
		file.open("demo.cmd");
		if (!file.isOpen())
			error("Failed to open 'demo.cmd' file");

		loadDemoData(&file, 0, 0x1000);

		file.close();
		file.open("data");

		if (!file.isOpen())
			error("Failed to open 'data' file");

		// loadGlobalObjects(file, 0xbd62);
		/*file->seek(0x29efe);
		load8bitArea(file, 16);
		file->seek(0x2a450);
		load8bitArea(file, 16);*/

		load8bitBinary(&file, 0x442, 16);
		loadPalettes(&file, 0x0);

		file.close();
		file.open("x.prg");
		if (!file.isOpen())
			error("Failed to open 'x.prg' file");
		loadMessagesFixedSize(&file, 0x3b90, 14, 20);

		file.close();
		file.open("soundfx");
		if (!file.isOpen())
			error("Failed to open 'soundfx' executable for AtariST demo");

		loadSoundsFx(&file, 0, 25);
	} else
		error("Unsupported demo for Driller");

	_demoMode = true;
	_angleRotationIndex = 1;
}

void DrillerEngine::loadAssetsFullGame() {
	Common::File file;
	if (isAmiga()) {
		if (_variant & ADGF_AMIGA_RETAIL) {
			file.open("driller");

			if (!file.isOpen())
				error("Failed to open 'driller' executable for Amiga");

			_border = loadAndConvertNeoImage(&file, 0x137f4);
			byte *palette = (byte *)malloc(16 * 3);
			for (int i = 0; i < 16; i++) { // gray scale palette
				palette[i * 3 + 0] = i * (255 / 16);
				palette[i * 3 + 1] = i * (255 / 16);
				palette[i * 3 + 2] = i * (255 / 16);
			}
			_title = loadAndConvertNeoImage(&file, 0x10, palette);

			loadMessagesFixedSize(&file, 0xc66e, 14, 20);
			loadGlobalObjects(&file, 0xbd62);
			load8bitBinary(&file, 0x29c16, 16);
			loadPalettes(&file, 0x297d4);
			loadSoundsFx(&file, 0x30e80, 25);
		} else if (_variant & ADGF_AMIGA_BUDGET) {
			file.open("lift.neo");
			if (!file.isOpen())
				error("Failed to open 'lift.neo' file");

			_title = loadAndConvertNeoImage(&file, 0);

			file.close();
			file.open("console.neo");
			if (!file.isOpen())
				error("Failed to open 'console.neo' file");

			_border = loadAndConvertNeoImage(&file, 0);

			file.close();
			file.open("driller");
			if (!file.isOpen())
				error("Failed to open 'driller' executable for Amiga");

			load8bitBinary(&file, 0x21a3e, 16);
			loadPalettes(&file, 0x215fc);

			file.close();
			file.open("soundfx");
			if (!file.isOpen())
				error("Failed to open 'soundfx' executable for Amiga");

			loadSoundsFx(&file, 0, 25);
		}
		else
			error("Invalid or unknown Amiga release");
	} else if (isAtariST()) {
		file.open("x.prg");

		if (!file.isOpen())
			error("Failed to open 'x.prg' executable for AtariST");

		_border = loadAndConvertNeoImage(&file, 0x1371a);
		byte *palette = (byte *)malloc(16 * 3);
		for (int i = 0; i < 16; i++) { // gray scale palette
			palette[i * 3 + 0] = i * (255 / 16);
			palette[i * 3 + 1] = i * (255 / 16);
			palette[i * 3 + 2] = i * (255 / 16);
		}
		_title = loadAndConvertNeoImage(&file, 0x10, palette);

		//loadMessagesFixedSize(&file, 0xc66e, 14, 20);
		//loadGlobalObjects(&file, 0xbd62);
		load8bitBinary(&file, 0x29b3c, 16);
		loadPalettes(&file, 0x296fa);
		loadSoundsFx(&file, 0x30da6, 25);
	} else if (_renderMode == Common::kRenderEGA) {
		loadBundledImages();
		_title = _border;
		file.open("DRILLE.EXE");

		if (!file.isOpen())
			error("Failed to open DRILLE.EXE");

		loadMessagesFixedSize(&file, 0x4135, 14, 20);
		loadFonts(&file, 0x99dd);
		loadGlobalObjects(&file, 0x3b42);
		load8bitBinary(&file, 0x9b40, 16);
	} else if (_renderMode == Common::kRenderEGA) {
		loadBundledImages();
		_title = _border;
		file.open("DRILLC.EXE");

		if (!file.isOpen())
			error("Failed to open DRILLC.EXE");
		load8bitBinary(&file, 0x7bb0, 4);
	} else
		error("Invalid or unsupported render mode %s for Driller", Common::getRenderModeDescription(_renderMode));
}

void DrillerEngine::drawUI() {
	_gfx->renderCrossair(0, _crossairPosition);
	uint32 yellow = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0xFF, 0x55);
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
	uint32 gray = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0xA0, 0xA0, 0xA0);

	Graphics::Surface *surface = nullptr;
	if (_border) { // This can be removed when all the borders are loaded
		surface = new Graphics::Surface();
		surface->create(_screenW, _screenH, _gfx->_texturePixelFormat);
		surface->fillRect(_fullscreenViewArea, gray);
	} else
		return;

	if (_currentAreaMessages.size() == 2) {
		int score = _gameStateVars[k8bitVariableScore];
		drawStringInSurface(_currentAreaMessages[0], 196, 177, yellow, black, surface);
		drawStringInSurface(_currentAreaMessages[1], 196, 185, yellow, black, surface);
		drawStringInSurface(Common::String::format("%04d", 2 * int(_position.x())), 150, 145, yellow, black, surface);
		drawStringInSurface(Common::String::format("%04d", 2 * int(_position.z())), 150, 153, yellow, black, surface);
		drawStringInSurface(Common::String::format("%04d", 2 * int(_position.y())), 150, 161, yellow, black, surface);
		drawStringInSurface(Common::String::format("%d", _playerHeightNumber), 57, 161, yellow, black, surface);
		drawStringInSurface(Common::String::format("%07d", score), 240, 129, yellow, black, surface);
	}

	int energy = _gameStateVars[k8bitVariableEnergy];
	int shield = _gameStateVars[k8bitVariableShield];
	if (_renderMode == Common::kRenderEGA) {
		if (energy >= 0) {
			Common::Rect back(20, 185, 88 - energy, 191);
			surface->fillRect(back, black);
			Common::Rect energyBar(87 - energy, 185, 88, 191);
			surface->fillRect(energyBar, yellow);
		}

		if (shield >= 0) {
			Common::Rect back(20, 177, 88 - shield, 183);
			surface->fillRect(back, black);

			Common::Rect shieldBar(87 - shield, 177, 88, 183);
			surface->fillRect(shieldBar, yellow);
		}
	}

	if (!_uiTexture)
		_uiTexture = _gfx->createTexture(surface);
	else
		_uiTexture->update(surface);

	_gfx->setViewport(_fullscreenViewArea);
	_gfx->drawTexturedRect2D(_fullscreenViewArea, _fullscreenViewArea, _uiTexture);
	_gfx->setViewport(_viewArea);

	surface->free();
	delete surface;
}

void DrillerEngine::pressedKey(const int keycode) {
	if (keycode == Common::KEYCODE_d) {
		Common::Point gasPocket = _currentArea->_gasPocketPosition;
		uint32 gasPocketRadius = _currentArea->_gasPocketRadius;
		if (gasPocketRadius == 0)
			return;

		if (_flyMode) {
			_currentAreaMessages[0] = _messagesList[8];
			return;
		}

		if (drillDeployed()) {
			_currentAreaMessages[0] = _messagesList[12];
			return;
		}

		if (_gameStateVars[k8bitVariableEnergy] < 5) {
			_currentAreaMessages[0] = _messagesList[7];
			return;
		}

		_gameStateVars[k8bitVariableEnergy] = _gameStateVars[k8bitVariableEnergy] - 5;
		_gameStateVars[32]++;
		// TODO: check if there is space for the drill
		Math::Vector3d drillPosition = _cameraFront;
		drillPosition = _position + 256 * drillPosition;

		debugC(1, kFreescapeDebugMove, "Current position at %f %f %f", _position.x(), _position.y(), _position.z());
		drillPosition.setValue(1, _position.y() - _playerHeight * _currentArea->getScale());
		debugC(1, kFreescapeDebugMove, "Trying to adding drill at %f %f %f", drillPosition.x(), drillPosition.y(), drillPosition.z());
		debugC(1, kFreescapeDebugMove, "with pitch: %f and yaw %f", _pitch, _yaw);

		const Math::Vector3d gasPocket3D(gasPocket.x, 1, gasPocket.y);
		addDrill(drillPosition);
		float distance = (gasPocket3D - drillPosition).length();
		debugC(1, kFreescapeDebugMove, "length to gas pocket: %f with radius %d", distance, _currentArea->_gasPocketRadius);
		// TODO: show the result of the drilling
		_currentAreaMessages[0] = _messagesList[3];

	} else if (keycode == Common::KEYCODE_c) {
		uint32 gasPocketRadius = _currentArea->_gasPocketRadius;
		if (gasPocketRadius == 0)
			return;

		if (_flyMode) {
			_currentAreaMessages[0] = _messagesList[8];
			return;
		}

		if (!drillDeployed()) {
			_currentAreaMessages[0] = _messagesList[13];
			return;
		}

		if (_gameStateVars[k8bitVariableEnergy] < 5) {
			_currentAreaMessages[0] = _messagesList[7];
			return;
		}

		_gameStateVars[k8bitVariableEnergy] = _gameStateVars[k8bitVariableEnergy] - 5;
		_gameStateVars[32]--;
		removeDrill();
	}
}

bool DrillerEngine::drillDeployed() {
	return (_currentArea->objectWithID(252) != nullptr);
}

void DrillerEngine::addDrill(const Math::Vector3d position) {
	// int drillObjectIDs[8] = {255, 254, 253, 252, 251, 250, 248, 247};
	GeometricObject *obj = nullptr;
	Math::Vector3d origin = position;

	int16 id;
	int heightLastObject;

	id = 255;
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	obj = obj->duplicate();
	obj->setOrigin(origin);
	// offset.setValue(1, offset.y() + obj->getSize().y());
	obj->makeVisible();
	_currentArea->addObject(obj);

	heightLastObject = obj->getSize().y();

	id = 254;
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	// Set position for object
	origin.setValue(0, origin.x() - obj->getSize().x() / 5);
	origin.setValue(1, origin.y() + heightLastObject);
	origin.setValue(2, origin.z() - obj->getSize().z() / 5);

	obj = obj->duplicate();
	obj->setOrigin(origin);
	obj->makeVisible();
	_currentArea->addObject(obj);

	// Undo offset
	origin.setValue(0, origin.x() + obj->getSize().x() / 5);
	heightLastObject = obj->getSize().y();
	origin.setValue(2, origin.z() + obj->getSize().z() / 5);

	id = 253;
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	obj = obj->duplicate();

	origin.setValue(0, origin.x() + obj->getSize().x() / 5);
	origin.setValue(1, origin.y() + heightLastObject);
	origin.setValue(2, origin.z() + obj->getSize().z() / 5);

	obj->setOrigin(origin);
	obj->makeVisible();
	_currentArea->addObject(obj);

	// Undo offset
	// origin.setValue(0, origin.x() - obj->getSize().x() / 5);
	heightLastObject = obj->getSize().y();
	// origin.setValue(2, origin.z() - obj->getSize().z() / 5);

	id = 252;
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	obj = obj->duplicate();
	origin.setValue(1, origin.y() + heightLastObject);
	obj->setOrigin(origin);
	assert(obj);
	obj->makeVisible();
	_currentArea->addObject(obj);
}

void DrillerEngine::removeDrill() {
	for (int16 id = 252; id < 256; id++) {
		_currentArea->removeObject(id);
	}
}

void DrillerEngine::initGameState() {
	for (int i = 0; i < k8bitMaxVariable; i++) // TODO: check maximum variable
		_gameStateVars[i] = 0;

	for (auto &it : _areaMap)
		_gameStateBits[it._key] = 0;

	_gameStateVars[k8bitVariableEnergy] = _initialProveEnergy;
	_gameStateVars[k8bitVariableShield] = _initialProveShield;
}

bool DrillerEngine::checkIfGameEnded() {
	if (_gameStateVars[k8bitVariableShield] == 0) {
		_flyMode = true;
		gotoArea(127, 0);
		drawFrame();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(5000);
		return true;
	}
	return false;
}

} // End of namespace Freescape
