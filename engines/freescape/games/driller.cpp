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

enum {
	kDrillerNoRig = 0,
	kDrillerRigInPlace = 1,
	kDrillerRigOutOfPlace = 2,
};

DrillerEngine::DrillerEngine(OSystem *syst, const ADGameDescription *gd) : FreescapeEngine(syst, gd) {
	// if (isAmiga())
	//	_viewArea = Common::Rect(72, 66, 567, 269);
	// else
	if (isDOS())
		_viewArea = Common::Rect(40, 16, 280, 117);
	else if (isAmiga() || isAtariST())
		_viewArea = Common::Rect(36, 16, 284, 118);

	_playerHeightNumber = 1;
	_playerHeights.push_back(16);
	_playerHeights.push_back(48);
	_playerHeights.push_back(80);
	_playerHeights.push_back(112);

	_playerHeight = _playerHeights[_playerHeightNumber];
	_playerWidth = 12;
	_playerDepth = 32;

	_initialTankEnergy = 48;
	_initialTankShield = 50;
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
	_gfx->_keyColor = 0;

	if (isAmiga() || isAtariST())
		swapPalette(areaID);

	if (areaID != _startArea || entranceID != _startEntrance) {
		_lastMousePos = g_system->getEventManager()->getMousePos();
		rotate(0, 0);
	}
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
	} else if (_renderMode == Common::kRenderCGA) {
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
	Graphics::Surface *surface = nullptr;
	if (_border) { // This can be removed when all the borders are loaded
		uint32 gray = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0xA0, 0xA0, 0xA0);
		surface = new Graphics::Surface();
		surface->create(_screenW, _screenH, _gfx->_texturePixelFormat);
		surface->fillRect(_fullscreenViewArea, gray);
		drawCrossair(surface);
	} else
		return;

	if (isDOS())
		drawDOSUI(surface);
	else if (isAmiga() || isAtariST())
		drawAmigaAtariSTUI(surface);

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

void DrillerEngine::drawDOSUI(Graphics::Surface *surface) {
	uint32 yellow = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0xFF, 0x55);
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);

	int score = _gameStateVars[k8bitVariableScore];
	drawStringInSurface(_currentArea->_name, 195, 185, yellow, black, surface);
	drawStringInSurface(Common::String::format("%04d", 2 * int(_position.x())), 150, 145, yellow, black, surface);
	drawStringInSurface(Common::String::format("%04d", 2 * int(_position.z())), 150, 153, yellow, black, surface);
	drawStringInSurface(Common::String::format("%04d", 2 * int(_position.y())), 150, 161, yellow, black, surface);
	if (_playerHeightNumber >= 0)
		drawStringInSurface(Common::String::format("%d", _playerHeightNumber), 57, 161, yellow, black, surface);
	else
		drawStringInSurface(Common::String::format("%s", "J"), 57, 161, yellow, black, surface);

	drawStringInSurface(Common::String::format("%3d", _playerSteps[_playerStepIndex]), 46, 153, yellow, black, surface);
	drawStringInSurface(Common::String::format("%07d", score), 240, 129, yellow, black, surface);

	int hours = _countdown / 3600;
	drawStringInSurface(Common::String::format("%02d", hours), 208, 8, yellow, black, surface);
	int minutes = (_countdown - hours * 3600) / 60;
	drawStringInSurface(Common::String::format("%02d", minutes), 230, 8, yellow, black, surface);
	int seconds = _countdown - hours * 3600 - minutes * 60;
	drawStringInSurface(Common::String::format("%02d", seconds), 254, 8, yellow, black, surface);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 191, 177, black, yellow, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else {
		if (_currentArea->_gasPocketRadius == 0)
			message = _messagesList[2];
		else if (_drilledAreas[_currentArea->getAreaID()])
			message = _messagesList[0];
		else
			message = _messagesList[1];

		drawStringInSurface(message, 191, 177, yellow, black, surface);
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
}

void DrillerEngine::drawAmigaAtariSTUI(Graphics::Surface *surface) {
	// TODO: this needs to have fonts already parsed
}

Math::Vector3d getProjectionToPlane(const Math::Vector3d &vect, const Math::Vector3d normal) {
	assert (normal.length() == 1);
	// Formula: return p - n * (n . p)
	Math::Vector3d result = vect;
	result -= normal * normal.dotProduct(vect);
	return result;
}

void DrillerEngine::pressedKey(const int keycode) {
	if (keycode == Common::KEYCODE_d) {
		Common::Point gasPocket = _currentArea->_gasPocketPosition;
		uint32 gasPocketRadius = _currentArea->_gasPocketRadius;
		if (gasPocketRadius == 0)
			return;

		if (_flyMode) {
			insertTemporaryMessage(_messagesList[8], _countdown - 2);
			return;
		}

		if (drillDeployed(_currentArea)) {
			insertTemporaryMessage(_messagesList[12], _countdown - 2);
			return;
		}

		if (_gameStateVars[k8bitVariableEnergy] < 5) {
			insertTemporaryMessage(_messagesList[7], _countdown - 2);
			return;
		}

		Math::Vector3d drill = drillPosition();
		debugC(1, kFreescapeDebugMove, "Current position at %f %f %f", _position.x(), _position.y(), _position.z());
		debugC(1, kFreescapeDebugMove, "Trying to adding drill at %f %f %f", drill.x(), drill.y(), drill.z());
		debugC(1, kFreescapeDebugMove, "with pitch: %f and yaw %f", _pitch, _yaw);

		if (!checkDrill(drill)) {
			insertTemporaryMessage(_messagesList[4], _countdown - 2);
			return;
		}

		_gameStateVars[k8bitVariableEnergy] = _gameStateVars[k8bitVariableEnergy] - 5;
		const Math::Vector3d gasPocket3D(gasPocket.x, drill.y(), gasPocket.y);
		addDrill(drill);
		float distanceToPocket = (gasPocket3D - drill).length();
		float success = 100.0 * (1.0 - distanceToPocket / _currentArea->_gasPocketRadius);
		insertTemporaryMessage(_messagesList[3], _countdown - 2);

		if (success <= 0) {
			insertTemporaryMessage(_messagesList[9], _countdown - 4);
			return;
		}

		insertTemporaryMessage(_messagesList[5], _countdown - 4);
		Common::String successMessage = _messagesList[6];
		successMessage.replace(0, 4, Common::String::format("%d", int(success)));
		insertTemporaryMessage(successMessage, _countdown - 6);
		if (success >= 50.0) {
			_drilledAreas[_currentArea->getAreaID()] = kDrillerRigInPlace;
			_gameStateVars[32]++;
		} else
			_drilledAreas[_currentArea->getAreaID()] = kDrillerRigOutOfPlace;
	} else if (keycode == Common::KEYCODE_c) {
		uint32 gasPocketRadius = _currentArea->_gasPocketRadius;
		if (gasPocketRadius == 0)
			return;

		if (_flyMode) {
			insertTemporaryMessage(_messagesList[8], _countdown - 2);
			return;
		}

		if (!drillDeployed(_currentArea)) {
			insertTemporaryMessage(_messagesList[13], _countdown - 2);
			return;
		}

		if (_gameStateVars[k8bitVariableEnergy] < 5) {
			insertTemporaryMessage(_messagesList[7], _countdown - 2);
			return;
		}

		_gameStateVars[k8bitVariableEnergy] = _gameStateVars[k8bitVariableEnergy] - 5;

		uint16 areaID = _currentArea->getAreaID();
		if (_drilledAreas[areaID] > 0) {
			if (_drilledAreas[areaID] == kDrillerRigInPlace)
				_gameStateVars[32]--;
			_drilledAreas[areaID] = kDrillerNoRig;
		}
		removeDrill(_currentArea);
		insertTemporaryMessage(_messagesList[10], _countdown - 2);
	}
}

Math::Vector3d DrillerEngine::drillPosition() {
	Math::Vector3d position = _position;
	position.setValue(1, position.y() - _playerHeight);
	position = position + 300 * getProjectionToPlane(_cameraFront, Math::Vector3d(0, 1, 0));

	Object *obj = (GeometricObject *)_areaMap[255]->objectWithID(255); // Drill base
	assert(obj);
	position.setValue(0, position.x() - obj->getSize().x() / 2);
	position.setValue(2, position.z() - obj->getSize().z() / 2);
	return position;
}

bool DrillerEngine::drillDeployed(Area *area) {
	return (area->objectWithID(252) != nullptr);
}

bool DrillerEngine::checkDrill(const Math::Vector3d position) {
	GeometricObject *obj = nullptr;
	Math::Vector3d origin = position;

	int16 id;
	int heightLastObject;

	id = 255;
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	obj = (GeometricObject *)obj->duplicate();
	obj->setOrigin(origin);
	//if (!_currentArea->checkCollisions(obj->_boundingBox))
	//	return false;

	heightLastObject = obj->getSize().y();
	delete obj;

	id = 254;
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	// Set position for object
	origin.setValue(0, origin.x() - obj->getSize().x() / 5);
	origin.setValue(1, origin.y() + heightLastObject);
	origin.setValue(2, origin.z() - obj->getSize().z() / 5);

	obj = (GeometricObject *)obj->duplicate();
	obj->setOrigin(origin);
	if (!_currentArea->checkCollisions(obj->_boundingBox).empty())
		return false;

	// Undo offset
	origin.setValue(0, origin.x() + obj->getSize().x() / 5);
	heightLastObject = obj->getSize().y();
	origin.setValue(2, origin.z() + obj->getSize().z() / 5);
	delete obj;

	id = 253;
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	obj = (GeometricObject *)obj->duplicate();

	origin.setValue(0, origin.x() + obj->getSize().x() / 5);
	origin.setValue(1, origin.y() + heightLastObject);
	origin.setValue(2, origin.z() + obj->getSize().z() / 5);

	obj->setOrigin(origin);
	if (!_currentArea->checkCollisions(obj->_boundingBox).empty())
		return false;

	// Undo offset
	// origin.setValue(0, origin.x() - obj->getSize().x() / 5);
	heightLastObject = obj->getSize().y();
	// origin.setValue(2, origin.z() - obj->getSize().z() / 5);
	delete obj;

	id = 252;
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	obj = (GeometricObject *)obj->duplicate();
	origin.setValue(1, origin.y() + heightLastObject);
	obj->setOrigin(origin);
	assert(obj);

	if (!_currentArea->checkCollisions(obj->_boundingBox).empty())
		return false;

	delete obj;
	return true;
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
	obj = (GeometricObject *)obj->duplicate();
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

	obj = (GeometricObject *)obj->duplicate();
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
	obj = (GeometricObject *)obj->duplicate();

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
	obj = (GeometricObject *)obj->duplicate();
	origin.setValue(0, origin.x() + obj->getSize().x());
	origin.setValue(1, origin.y() + heightLastObject);
	origin.setValue(2, origin.z() + obj->getSize().z());
	obj->setOrigin(origin);
	assert(obj);
	obj->makeVisible();
	_currentArea->addObject(obj);

	heightLastObject = obj->getSize().y();

	id = 251;
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	obj = (GeometricObject *)obj->duplicate();
	origin.setValue(1, origin.y() + heightLastObject);
	obj->setOrigin(origin);
	assert(obj);
	obj->makeVisible();
	_currentArea->addObject(obj);
}

void DrillerEngine::removeDrill(Area *area) {
	for (int16 id = 251; id < 256; id++) {
		area->removeObject(id);
	}
}

void DrillerEngine::initGameState() {
	for (int i = 0; i < k8bitMaxVariable; i++) // TODO: check maximum variable
		_gameStateVars[i] = 0;

	for (auto &it : _areaMap) {
		_gameStateBits[it._key] = 0;
		_drilledAreas[it._key] = kDrillerNoRig;
	}

	_gameStateVars[k8bitVariableEnergy] = _initialTankEnergy;
	_gameStateVars[k8bitVariableShield] = _initialTankShield;

	_gameStateVars[k8bitVariableEnergyDrillerTank] = _initialTankEnergy;
	_gameStateVars[k8bitVariableShieldDrillerTank] = _initialTankShield;

	_gameStateVars[k8bitVariableEnergyDrillerJet] = _initialJetEnergy;
	_gameStateVars[k8bitVariableShieldDrillerJet] = _initialJetShield;

	if (_countdown > 0)
		startCountdown(_countdown);
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

Common::Error DrillerEngine::saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave) {
	for (auto &it : _areaMap) {
		stream->writeUint16LE(it._key);
		stream->writeUint32LE(_drilledAreas[it._key]);
	}

	return Common::kNoError;
}

Common::Error DrillerEngine::loadGameStreamExtended(Common::SeekableReadStream *stream) {
	for (uint i = 0; i < _areaMap.size(); i++) {
		uint16 key = stream->readUint16LE();
		assert(_areaMap.contains(key));
		_drilledAreas[key] = stream->readUint32LE();
		if (_drilledAreas[key] == kDrillerNoRig)
			if (drillDeployed(_areaMap[key]))
				removeDrill(_areaMap[key]);
	}

	return Common::kNoError;
}

} // End of namespace Freescape
