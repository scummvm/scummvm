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

#include "common/config-manager.h"
#include "common/events.h"
#include "common/file.h"

#include "freescape/freescape.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

DrillerEngine::DrillerEngine(OSystem *syst) : FreescapeEngine(syst) {
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
	if (_currentArea->gasPocketRadius > 0)
		_currentAreaMessages.push_back(_messagesList[1]);
	else
		_currentAreaMessages.push_back(_messagesList[2]);

	_currentAreaMessages.push_back(_currentArea->name);

	if (entranceID > 0 || areaID == 127) {
		traverseEntrance(entranceID);
	} else if (entranceID == 0) {
		Math::Vector3d diff = _lastPosition - _position;
		//debug("dif: %f %f %f", diff.x(), diff.y(), diff.z());
		// diff should be used to determinate which entrance to use
		int newPos = -1;
		if (abs(diff.x()) < abs(diff.z())) {
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
	Common::SeekableReadStream *file = nullptr;
	Common::String path = ConfMan.get("path");
	Common::FSDirectory gameDir(path);

	Common::File exe;
	if (isAmiga()) {
		file = gameDir.createReadStreamForMember("driller");

		if (file == nullptr)
			error("Failed to open 'driller' executable for Amiga");

		loadGlobalObjects(file, 0xbd62);
		/*file->seek(0x29efe);
		load8bitArea(file, 16);
		file->seek(0x2a450);
		load8bitArea(file, 16);*/

		load8bitBinary(file, 0x29c16, 16);
	} else if (_renderMode == "ega") {
		file = gameDir.createReadStreamForMember("DRILLE.EXE");

		if (file == nullptr)
			error("Failed to open DRILLE.EXE");

		loadMessagesFixedSize(file, 0x4135, 14, 20);
		loadFonts(file, 0x99dd);
		loadGlobalObjects(file, 0x3b42);
		load8bitBinary(file, 0x9b40, 16);
	} else if (_renderMode == "cga") {
		file = gameDir.createReadStreamForMember("DRILLC.EXE");

		if (file == nullptr)
			error("Failed to open DRILLC.EXE");
		load8bitBinary(file, 0x7bb0, 4);
	} else
		error("Invalid render mode %s for Driller", _renderMode.c_str());
}

void DrillerEngine::drawUI() {
	_gfx->renderCrossair(0);

	if (_currentAreaMessages.size() == 2) {
		_gfx->setViewport(_fullscreenViewArea);

		Graphics::Surface *surface = new Graphics::Surface();
		surface->create(_screenW, _screenH, _gfx->_currentPixelFormat);
		surface->fillRect(_fullscreenViewArea, 0xA0A0A0FF);

		int score = _gameStateVars[k8bitVariableScore];

		uint32 yellow = 0xFFFF55FF;
		uint32 black = 0x000000FF;

		drawStringInSurface(_currentAreaMessages[0], 196, 177, yellow, black, surface);
		drawStringInSurface(_currentAreaMessages[1], 196, 185, yellow, black, surface);
		drawStringInSurface(Common::String::format("%04d", 2 * int(_position.x())), 150, 145, yellow, black, surface);
		drawStringInSurface(Common::String::format("%04d", 2 * int(_position.z())), 150, 153, yellow, black, surface);
		drawStringInSurface(Common::String::format("%04d", 2 * int(_position.y())), 150, 161, yellow, black, surface);
		drawStringInSurface(Common::String::format("%d", _playerHeightNumber), 57, 161, yellow, black, surface);


		drawStringInSurface(Common::String::format("%07d", score), 240, 129, yellow, black, surface);

		if (!_uiTexture)
			_uiTexture = _gfx->createTexture(surface);
		else
			_uiTexture->update(surface);

		_gfx->drawTexturedRect2D(_fullscreenViewArea, _fullscreenViewArea, _uiTexture);
		surface->free();
		delete surface;
	}

	int energy = _gameStateVars[k8bitVariableEnergy];
	int shield = _gameStateVars[k8bitVariableShield];
	if (_renderMode == "ega" && _border) {
		//Common::Rect black(20, 177, 87, 191);
		//_gfx->drawRect2D(black, 255, 0, 0, 0);

		if (energy >= 0) {
			Common::Rect black(20, 186, 87 - energy, 191);
			_gfx->drawRect2D(black, 255, 0, 0, 0);
			Common::Rect energyBar(87 - energy, 186, 87, 191);
			_gfx->drawRect2D(energyBar, 255, 0xfc, 0xfc, 0x54);
		}

		if (shield >= 0) {
			Common::Rect black(20, 178, 87 - shield, 183);
			_gfx->drawRect2D(black, 255, 0, 0, 0);

			Common::Rect shieldBar(87 - shield, 178, 87, 183);
			_gfx->drawRect2D(shieldBar, 255, 0xfc, 0xfc, 0x54);
		}
	}
	_gfx->setViewport(_viewArea);
}

void DrillerEngine::pressedKey(const int keycode) {
	if (keycode == Common::KEYCODE_d) {
		Common::Point gasPocket = _currentArea->gasPocketPosition;
		uint32 gasPocketRadius = _currentArea->gasPocketRadius;
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
		drillPosition =  _position + 256 * drillPosition;

		debugC(1, kFreescapeDebugMove, "Current position at %f %f %f", _position.x(), _position.y(), _position.z());
		drillPosition.setValue(1, _position.y() - _playerHeight * _currentArea->getScale());
		debugC(1, kFreescapeDebugMove, "Trying to adding drill at %f %f %f", drillPosition.x(), drillPosition.y(), drillPosition.z());
		debugC(1, kFreescapeDebugMove, "with pitch: %f and yaw %f", _pitch, _yaw);

		const Math::Vector3d gasPocket3D(gasPocket.x, 1, gasPocket.y);
		addDrill(drillPosition);
		float distance = (gasPocket3D - drillPosition).length();
		debugC(1, kFreescapeDebugMove, "length to gas pocket: %f with radius %d", distance, _currentArea->gasPocketRadius);
		// TODO: show the result of the drilling
		_currentAreaMessages[0] = _messagesList[3];

	} else if (keycode == Common::KEYCODE_c) {
		uint32 gasPocketRadius = _currentArea->gasPocketRadius;
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
	//int drillObjectIDs[8] = {255, 254, 253, 252, 251, 250, 248, 247};
	GeometricObject *obj = nullptr;
	Math::Vector3d origin = position;

	int16 id;
	int heightLastObject;

	id = 255;
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject*) _areaMap[255]->objectWithID(id);
	assert(obj);
	obj = obj->duplicate();
	obj->setOrigin(origin);
	//offset.setValue(1, offset.y() + obj->getSize().y());
	obj->makeVisible();
	_currentArea->addObject(obj);

	heightLastObject = obj->getSize().y();

	id = 254;
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject*) _areaMap[255]->objectWithID(id);
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
	obj = (GeometricObject*) _areaMap[255]->objectWithID(id);
	assert(obj);
	obj = obj->duplicate();

	origin.setValue(0, origin.x() + obj->getSize().x() / 5);
	origin.setValue(1, origin.y() + heightLastObject);
	origin.setValue(2, origin.z() + obj->getSize().z() / 5);

	obj->setOrigin(origin);
	obj->makeVisible();
	_currentArea->addObject(obj);

	// Undo offset
	//origin.setValue(0, origin.x() - obj->getSize().x() / 5);
	heightLastObject = obj->getSize().y();
	//origin.setValue(2, origin.z() - obj->getSize().z() / 5);

	id = 252;
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject*) _areaMap[255]->objectWithID(id);
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

	for (AreaMap::iterator it = _areaMap.begin(); it != _areaMap.end(); ++it)
		_gameStateBits[it->_key] = 0;

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