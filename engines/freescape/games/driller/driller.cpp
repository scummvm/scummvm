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
#include "common/random.h"

#include "freescape/freescape.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

enum {
	kDrillerNoRig = 0,
	kDrillerRigInPlace = 1,
	kDrillerRigOutOfPlace = 2,
	kDrillerRigNoGas = 3
};

DrillerEngine::DrillerEngine(OSystem *syst, const ADGameDescription *gd) : FreescapeEngine(syst, gd) {

	if (!Common::parseBool(ConfMan.get("automatic_drilling"), _useAutomaticDrilling))
		error("Failed to parse bool from automatic_drilling option");

	if (isDOS())
		initDOS();
	else if (isAmiga() || isAtariST())
		initAmigaAtari();
	else if (isSpectrum())
		initZX();
	else if (isCPC())
		initCPC();
	else if (isC64())
		initC64();

	_playerHeightNumber = 1;
	_playerHeights.push_back(16);
	_playerHeights.push_back(48);
	_playerHeights.push_back(80);
	_playerHeights.push_back(112);

	_angleRotations.push_back(5);
	_angleRotations.push_back(10);
	_angleRotations.push_back(15);
	_angleRotations.push_back(30);
	_angleRotations.push_back(45);
	_angleRotations.push_back(90);

	_playerHeight = _playerHeights[_playerHeightNumber];
	_playerWidth = 12;
	_playerDepth = 32;

	_initialTankEnergy = 48;
	_initialTankShield = 50;
	_initialJetEnergy = 29;
	_initialJetShield = 34;

	Math::Vector3d drillBaseOrigin = Math::Vector3d(0, 0, 0);
	Math::Vector3d drillBaseSize = Math::Vector3d(3, 2, 3);
	_drillBase = new GeometricObject(kCubeType, 0, 0, drillBaseOrigin, drillBaseSize, nullptr, nullptr, FCLInstructionVector(), "");
	assert(!_drillBase->isDestroyed() && !_drillBase->isInvisible());

	if (isDemo()) {
		_demoMode = !_disableDemoMode; // All the driller demos are non-interactive
		_angleRotationIndex = 0;
	}
}

DrillerEngine::~DrillerEngine() {
	delete _drillBase;
}

void DrillerEngine::titleScreen() {
	if (isAmiga() || isAtariST()) // TODO: implement these with their own animations
		return;

	if (_title) {
		drawTitle();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(3000);
	}
}
void DrillerEngine::borderScreen() {
	if (isAmiga() || isAtariST()) // TODO: implement these with their own animations
		return;

	if (_border) {
		drawBorder();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(3000);
	}
}

void DrillerEngine::gotoArea(uint16 areaID, int entranceID) {
	int prevAreaID = _currentArea ? _currentArea->getAreaID(): -1;
	debugC(1, kFreescapeDebugMove, "Jumping to area: %d, entrance: %d", areaID, entranceID);
	if (!_gameStateBits.contains(areaID))
		_gameStateBits[areaID] = 0;

	if (!_areaMap.contains(areaID)) {
		assert(isDOS() && isDemo());
		// Not included in the demo, abort area change
		return;
	}
	_currentArea = _areaMap[areaID];
	_currentArea->show();

	if (entranceID > 0 || areaID == 127) {
		traverseEntrance(entranceID);
	} else if (entranceID == 0) {
		int newPos = -1;
		// FIX: The next check will abort changing the current another
		// area if the player is too close to the corners
		if ((_position.z() < 100 && _position.x() > 3900) ||
			(_position.z() > 3900 && _position.x() < 100) ||
			(_position.z() < 100 && _position.x()  < 100) ||
			(_position.z() > 3900 && _position.x() > 3900)) {
				assert(prevAreaID > 0);
				_currentArea = _areaMap[prevAreaID];
				return;
		}
		if (_position.z() < 200 || _position.z() >= 3800) {
			if (_position.z() < 200)
				newPos = 4000;
			else
				newPos = 100;
			_position.setValue(2, newPos);
		} else if(_position.x() < 200 || _position.x() >= 3800)  {
			if (_position.x() < 200)
				newPos = 4000;
			else
				newPos = 100;
			_position.setValue(0, newPos);
		} else
			error("Invalid movement across areas");
		assert(newPos != -1);
		_sensors = _currentArea->getSensors();
	}
	_lastPosition = _position;
	_gameStateVars[0x1f] = 0;

	if (areaID == _startArea && entranceID == _startEntrance) {
		_yaw = 280;
		_pitch = 0;
	} else if (areaID == 127) {
		assert(entranceID == 0);
		_yaw = 90;
		_pitch = 335;
		_flyMode = true; // Avoid falling
		// Show the number of completed areas
		_areaMap[127]->_name.replace(0, 3, Common::String::format("%4d", _gameStateVars[32]));
	}

	debugC(1, kFreescapeDebugMove, "starting player position: %f, %f, %f", _position.x(), _position.y(), _position.z());
	clearTemporalMessages();
	playSound(5, false);
	// Ignore sky/ground fields
	_gfx->_keyColor = 0;
	_gfx->setColorRemaps(&_currentArea->_colorRemaps);

	swapPalette(areaID);
	_currentArea->_skyColor = 0;
	_currentArea->_usualBackgroundColor = 0;

	resetInput();
}

void DrillerEngine::loadAssetsFullGame() {
	FreescapeEngine::loadAssetsFullGame();
	/*
	We are going to inject a small script in the
	last area to force the game to end:
	IF COLLIDED? THEN
	IF VAR!=? (v32, 18) THEN END ENDIF
	GOTO (127, 0)
	*/

	FCLInstructionVector instructions;
	Common::Array<uint8> conditionArray;

	conditionArray.push_back(0xb);
	conditionArray.push_back(0x20);
	conditionArray.push_back(0x12);
	conditionArray.push_back(0x12);
	conditionArray.push_back(0x7f);
	conditionArray.push_back(0x0);

	Common::String conditionSource = detokenise8bitCondition(conditionArray, instructions, false);
	debugC(1, kFreescapeDebugParser, "%s", conditionSource.c_str());
	_areaMap[18]->_conditions.push_back(instructions);
	_areaMap[18]->_conditionSources.push_back(conditionSource);
}

void DrillerEngine::processBorder() {
	FreescapeEngine::processBorder();
}

void DrillerEngine::drawInfoMenu() {
	_savedScreen = _gfx->getScreenshot();

	uint32 color = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00);
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(_screenW, _screenH, _gfx->_texturePixelFormat);
	surface->fillRect(_fullscreenViewArea, color);

	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
	surface->fillRect(_viewArea, black);

	switch (_renderMode) {
		case Common::kRenderCGA:
			color = 1;
			break;
		case Common::kRenderZX:
			color = 6;
			break;
		case Common::kRenderCPC:
			color = _gfx->_underFireBackgroundColor;
			break;
		default:
			color = 14;
	}
	uint8 r, g, b;

	_gfx->readFromPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	drawStringInSurface(Common::String::format("%10s : %s", "sector", _currentArea->_name.c_str()), 69, 25, front, black, surface);
	Common::String rigStatus;
	Common::String gasFound;
	Common::String perTapped;
	Common::String gasTapped;

	switch (_drillStatusByArea[_currentArea->getAreaID()]) {
		case kDrillerNoRig:
			rigStatus = "Unpositioned";
			gasFound = "-";
			perTapped = "-";
			gasTapped = "-";
			break;
		case kDrillerRigInPlace:
		case kDrillerRigOutOfPlace:
			rigStatus = "Positioned";
			gasFound = Common::String::format("%d CFT", _drillMaxScoreByArea[_currentArea->getAreaID()]);
			perTapped = Common::String::format("%d %%", _drillSuccessByArea[_currentArea->getAreaID()]);
			gasTapped = Common::String::format("%d", uint32(_drillSuccessByArea[_currentArea->getAreaID()] * _drillMaxScoreByArea[_currentArea->getAreaID()]) / 100);
			break;
		case kDrillerRigNoGas:
			rigStatus = "Positioned";
			gasFound = "none";
			perTapped = "none";
			gasTapped = "zero";
			break;
		default:
			error("Invalid drill status");
			break;
	}

	drawStringInSurface(Common::String::format("%10s : %s", "rig status", rigStatus.c_str()), 69, 33, front, black, surface);
	drawStringInSurface(Common::String::format("%10s : %s", "gas found", gasFound.c_str()), 69, 41, front, black, surface);
	drawStringInSurface(Common::String::format("%10s : %s", "% tapped", perTapped.c_str()), 69, 49, front, black, surface);
	drawStringInSurface(Common::String::format("%10s : %s", "gas tapped", gasTapped.c_str()), 69, 57, front, black, surface);

	drawStringInSurface(Common::String::format("%13s : %d", "total sectors", 18), 84, 73, front, black, surface);
	drawStringInSurface(Common::String::format("%13s : %d", "safe sectors", _gameStateVars[32]), 84, 81, front, black, surface);

	if (isDOS() || isCPC()) {
		drawStringInSurface("l-load s-save esc-terminate", 53, 97, front, black, surface);
		drawStringInSurface("t-toggle sound on/off", 76, 105, front, black, surface);
	} else if (isSpectrum()) {
		drawStringInSurface("l-load s-save 1-abort", 76, 97, front, black, surface);
		drawStringInSurface("any other key-continue", 76, 105, front, black, surface);
	}

	_uiTexture->update(surface);
	_gfx->setViewport(_fullscreenViewArea);
	_gfx->drawTexturedRect2D(_fullscreenViewArea, _fullscreenViewArea, _uiTexture);
	_gfx->setViewport(_viewArea);

	_gfx->flipBuffer();
	g_system->updateScreen();

	Common::Event event;
	bool cont = true;
	while (!shouldQuit() && cont) {
		while (g_system->getEventManager()->pollEvent(event)) {

			// Events
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_l) {
					_gfx->setViewport(_fullscreenViewArea);
					loadGameDialog();
					_gfx->setViewport(_viewArea);
				} else if (event.kbd.keycode == Common::KEYCODE_s) {
					_gfx->setViewport(_fullscreenViewArea);
					saveGameDialog();
					_gfx->setViewport(_viewArea);
				} else if (isDOS() && event.kbd.keycode == Common::KEYCODE_t) {
					// TODO
				} else if ((isDOS() || isCPC()) && event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					_forceEndGame = true;
					cont = false;
				} else if (isSpectrum() && event.kbd.keycode == Common::KEYCODE_1) {
					_forceEndGame = true;
					cont = false;
				} else
					cont = false;
				break;
			case Common::EVENT_SCREEN_CHANGED:
				_gfx->computeScreenViewport();
				// TODO: properly refresh screen
				break;

			default:
				break;
			}
		}
		g_system->delayMillis(10);
	}

	_savedScreen->free();
	delete _savedScreen;
	surface->free();
	delete surface;
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
		if (isDOS() && isDemo()) // No support for drilling here yet
			return;
		clearTemporalMessages();
		Common::Point gasPocket = _currentArea->_gasPocketPosition;
		uint32 gasPocketRadius = _currentArea->_gasPocketRadius;
		if (gasPocketRadius == 0) {
			insertTemporaryMessage(_messagesList[2], _countdown - 2);
			return;
		}

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
		float distanceToPocket = (gasPocket3D - drill).length();
		float success = _useAutomaticDrilling ? 100.0 : 100.0 * (1.0 - distanceToPocket / _currentArea->_gasPocketRadius);
		insertTemporaryMessage(_messagesList[3], _countdown - 2);
		addDrill(drill, success > 0);
		if (success <= 0) {
			insertTemporaryMessage(_messagesList[9], _countdown - 4);
			_drillStatusByArea[_currentArea->getAreaID()] = kDrillerRigNoGas;
			return;
		}
		Common::String maxScoreMessage = _messagesList[5];
		int maxScore = _drillMaxScoreByArea[_currentArea->getAreaID()];
		maxScoreMessage.replace(2, 6, Common::String::format("%d", maxScore));
		insertTemporaryMessage(maxScoreMessage, _countdown - 4);
		Common::String successMessage = _messagesList[6];
		successMessage.replace(0, 4, Common::String::format("%d", int(success)));
		while (successMessage.size() < 14)
			successMessage += " ";
		insertTemporaryMessage(successMessage, _countdown - 6);
		_drillSuccessByArea[_currentArea->getAreaID()] = uint32(success);
		_gameStateVars[k8bitVariableScore] += uint32(maxScore * uint32(success)) / 100;

		if (success >= 50.0) {
			_drillStatusByArea[_currentArea->getAreaID()] = kDrillerRigInPlace;
			_gameStateVars[32]++;
		} else
			_drillStatusByArea[_currentArea->getAreaID()] = kDrillerRigOutOfPlace;
	} else if (keycode == Common::KEYCODE_c) {
		if (isDOS() && isDemo()) // No support for drilling here yet
			return;
		uint32 gasPocketRadius = _currentArea->_gasPocketRadius;
		clearTemporalMessages();
		if (gasPocketRadius == 0) {
			insertTemporaryMessage(_messagesList[2], _countdown - 2);
			return;
		}

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
		if (_drillStatusByArea[areaID] > 0) {
			if (_drillStatusByArea[areaID] == kDrillerRigInPlace)
				_gameStateVars[32]--;
			_drillStatusByArea[areaID] = kDrillerNoRig;
		}
		removeDrill(_currentArea);
		insertTemporaryMessage(_messagesList[10], _countdown - 2);
		int maxScore = _drillMaxScoreByArea[_currentArea->getAreaID()];
		uint32 success = _drillSuccessByArea[_currentArea->getAreaID()];
		uint32 scoreToRemove = uint32(maxScore * success) / 100;
		assert(scoreToRemove <= uint32(_gameStateVars[k8bitVariableScore]));
		_gameStateVars[k8bitVariableScore] -= scoreToRemove;
	}
}

Math::Vector3d DrillerEngine::drillPosition() {
	Math::Vector3d position = _position;
	position.setValue(1, position.y() - _playerHeight);
	position = position + 300 * getProjectionToPlane(_cameraFront, Math::Vector3d(0, 1, 0));

	Object *obj = (GeometricObject *)_areaMap[255]->objectWithID(255); // Drill base
	assert(obj);
	position.setValue(0, position.x() - 128);
	position.setValue(2, position.z() - 128);
	return position;
}

bool DrillerEngine::drillDeployed(Area *area) {
	return (area->objectWithID(255) != nullptr);
}

bool DrillerEngine::checkDrill(const Math::Vector3d position) {
	GeometricObject *obj = nullptr;
	Math::Vector3d origin = position;

	int16 id;
	int heightLastObject;

	origin.setValue(0, origin.x() + 128);
	origin.setValue(1, origin.y() - 5);
	origin.setValue(2, origin.z() + 128);

	_drillBase->setOrigin(origin);
	if (_currentArea->checkCollisions(_drillBase->_boundingBox).empty())
		return false;

	origin.setValue(0, origin.x() - 128);
	origin.setValue(2, origin.z() - 128);

	id = 255;
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	obj = (GeometricObject *)obj->duplicate();
	origin.setValue(1, origin.y() + 6);
	obj->setOrigin(origin);

	// This bounding box is too large and can result in the drill to float next to a wall
	if (!_currentArea->checkCollisions(obj->_boundingBox).empty())
		return false;

	origin.setValue(1, origin.y() + 15);
	obj->setOrigin(origin);

	if (!_currentArea->checkCollisions(obj->_boundingBox).empty())
		return false;

	origin.setValue(1, origin.y() - 10);
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
	return true;
}


void DrillerEngine::addSkanner(Area *area) {
	debug("area: %d", area->getAreaID());
	GeometricObject *obj = nullptr;
	int16 id;

	id = 248;
	// If first object is already added, do not re-add any
	if (area->objectWithID(id) != nullptr)
		return;

	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	obj = (GeometricObject *)obj->duplicate();
	obj->makeInvisible();
	area->addObject(obj);

	id = 249;
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	obj = (GeometricObject *)obj->duplicate();
	obj->makeInvisible();
	area->addObject(obj);

	id = 250;
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	obj = (GeometricObject *)obj->duplicate();
	obj->makeInvisible();
	area->addObject(obj);
}

void DrillerEngine::addDrill(const Math::Vector3d position, bool gasFound) {
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

	if (gasFound) {
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
}

void DrillerEngine::removeDrill(Area *area) {
	for (int16 id = 251; id < 256; id++) {
		if (id > 252)
			assert(area->objectWithID(id));

		if (area->objectWithID(id))
			area->removeObject(id);
	}
}

void DrillerEngine::initGameState() {
	_flyMode = false;
	_noClipMode = false;
	_shootingFrames = 0;
	_underFireFrames = 0;
	_yaw = 0;
	_pitch = 0;

	for (int i = 0; i < k8bitMaxVariable; i++) // TODO: check maximum variable
		_gameStateVars[i] = 0;

	for (auto &it : _areaMap) {
		it._value->resetArea();
		_gameStateBits[it._key] = 0;
		if (_drillStatusByArea[it._key] != kDrillerNoRig)
			removeDrill(it._value);
		_drillStatusByArea[it._key] = kDrillerNoRig;
		if (it._key != 255) {
			addSkanner(it._value);
			_drillMaxScoreByArea[it._key] = (10 + _rnd->getRandomNumber(89)) * 1000;
		}
		_drillSuccessByArea[it._key] = 0;
	}

	_gameStateVars[k8bitVariableEnergy] = _initialTankEnergy;
	_gameStateVars[k8bitVariableShield] = _initialTankShield;

	_gameStateVars[k8bitVariableEnergyDrillerTank] = _initialTankEnergy;
	_gameStateVars[k8bitVariableShieldDrillerTank] = _initialTankShield;

	_gameStateVars[k8bitVariableEnergyDrillerJet] = _initialJetEnergy;
	_gameStateVars[k8bitVariableShieldDrillerJet] = _initialJetShield;

	_playerHeightNumber = 1;
	_playerHeight = _playerHeights[_playerHeightNumber];
	removeTimers();
	startCountdown(_initialCountdown - 1);

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	_lastMinute = minutes;
	_demoIndex = 0;
	_demoEvents.clear();

	// Start playing music, if any, in any supported format
	playMusic("Matt Gray - The Best Of Reformation - 07 Driller Theme");
}

bool DrillerEngine::checkIfGameEnded() {
	if (isDemo() && _demoMode)
		return (_demoData[_demoIndex + 1] == 0x5f);

	if (_countdown <= 0) {
		insertTemporaryMessage(_messagesList[14], _countdown - 2);
		drawFrame();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(2000);
		gotoArea(127, 0);
	}

	if (_gameStateVars[k8bitVariableShield] == 0) {
		insertTemporaryMessage(_messagesList[15], _countdown - 2);
		drawFrame();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(2000);
		gotoArea(127, 0);
	}

	if (_gameStateVars[k8bitVariableEnergy] == 0) {
		insertTemporaryMessage(_messagesList[16], _countdown - 2);
		drawFrame();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(2000);
		gotoArea(127, 0);
	}

	if (_hasFallen) {
		_hasFallen = false;
		playSound(14, false);
		insertTemporaryMessage(_messagesList[17], _countdown - 4);
		drawBackground();
		drawBorder();
		drawUI();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(1000);
		gotoArea(127, 0);
	}

	if (_forceEndGame) {
		_forceEndGame = false;
		insertTemporaryMessage(_messagesList[18], _countdown - 2);
		drawFrame();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(2000);
		gotoArea(127, 0);
	}

	if (_currentArea->getAreaID() == 127) {
		if (_gameStateVars[32] == 18) { // All areas are complete
			insertTemporaryMessage(_messagesList[19], _countdown - 2);
			_gameStateVars[32] = 0;  // Avoid repeating the message
		}
		drawFrame();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(5000);
		return true;
	}
	return false;
}

bool DrillerEngine::onScreenControls(Common::Point mouse) {
	if (_moveFowardArea.contains(mouse)) {
		move(kForwardMovement, _scaleVector.x(), 20.0);
		return true;
	} else if (_moveLeftArea.contains(mouse)) {
		move(kLeftMovement, _scaleVector.y(), 20.0);
		return true;
	} else if (_moveRightArea.contains(mouse)) {
		move(kRightMovement, _scaleVector.y(), 20.0);
		return true;
	} else if (_moveBackArea.contains(mouse)) {
		move(kBackwardMovement, _scaleVector.x(), 20.0);
		return true;
	} else if (_moveUpArea.contains(mouse)) {
		rise();
		return true;
	} else if (_moveDownArea.contains(mouse)) {
		lower();
		return true;
	} else if (_deployDrillArea.contains(mouse)) {
		pressedKey(Common::KEYCODE_d);
		return true;
	} else if (_infoScreenArea.contains(mouse)) {
		drawInfoMenu();
		return true;
	} else if (_saveGameArea.contains(mouse)) {
		_gfx->setViewport(_fullscreenViewArea);
		saveGameDialog();
		_gfx->setViewport(_viewArea);
		return true;
	} else if (_loadGameArea.contains(mouse)) {
		_gfx->setViewport(_fullscreenViewArea);
		loadGameDialog();
		_gfx->setViewport(_viewArea);
		return true;
	}
	return false;
}


Common::Error DrillerEngine::saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave) {
	for (auto &it : _areaMap) { // All but skip area 255
		if (it._key == 255)
			continue;
		stream->writeUint16LE(it._key);
		stream->writeUint32LE(_drillStatusByArea[it._key]);
		stream->writeUint32LE(_drillMaxScoreByArea[it._key]);
		stream->writeUint32LE(_drillSuccessByArea[it._key]);
	}

	return Common::kNoError;
}

Common::Error DrillerEngine::loadGameStreamExtended(Common::SeekableReadStream *stream) {
	for (uint i = 0; i < _areaMap.size() - 1; i++) { // All except area 255
		uint16 key = stream->readUint16LE();
		assert(key != 255);
		assert(_areaMap.contains(key));
		_drillStatusByArea[key] = stream->readUint32LE();
		if (_drillStatusByArea[key] == kDrillerNoRig)
			if (drillDeployed(_areaMap[key]))
				removeDrill(_areaMap[key]);

		_drillMaxScoreByArea[key] = stream->readUint32LE();
		_drillSuccessByArea[key] = stream->readUint32LE();
	}

	return Common::kNoError;
}

} // End of namespace Freescape
