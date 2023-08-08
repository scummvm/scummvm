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

#include "common/file.h"

#include "freescape/freescape.h"
#include "freescape/games/dark/dark.h"
#include "freescape/language/8bitDetokeniser.h"
#include "freescape/objects/global.h"
#include "freescape/objects/connections.h"

namespace Freescape {

DarkEngine::DarkEngine(OSystem *syst, const ADGameDescription *gd) : FreescapeEngine(syst, gd) {
	if (isDOS())
		initDOS();
	else if (isSpectrum())
		initZX();
	else if (isAmiga() || isAtariST())
		initAmigaAtari();

	_playerHeightNumber = 1;
	_playerHeights.push_back(16);
	_playerHeights.push_back(48);

	_playerHeight = _playerHeights[_playerHeightNumber];
	_playerWidth = 12;
	_playerDepth = 32;
	_lastTenSeconds = -1;
	_lastSecond = -1;

	_angleRotations.push_back(5);
	_angleRotations.push_back(10);
	_angleRotations.push_back(15);
	_angleRotations.push_back(30);
	_angleRotations.push_back(45);
	_angleRotations.push_back(90);

	_initialEnergy = 11;
	_initialShield = 15;

	_jetFuelSeconds = _initialEnergy * 6;
}

void DarkEngine::addECDs(Area *area) {
	if (!area->entranceWithID(255))
		return;

	GlobalStructure *rs = (GlobalStructure *)area->entranceWithID(255);
	debugC(1, kFreescapeDebugParser, "ECD positions:");
	for (uint i = 0; i < rs->_structure.size(); i = i + 3) {
		int x = 32 * rs->_structure[i];
		int y = 32 * rs->_structure[i + 1];
		int z = 32 * rs->_structure[i + 2];

		debugC(1, kFreescapeDebugParser, "%d %d %d", x, y, z);
		if (x == 0 && y == 0 && z == 0) {
			debugC(1, kFreescapeDebugParser, "Skiping ECD zero position");
			continue;
		}
		addECD(area, Math::Vector3d(x, y, z), i / 3);
	}
}

void DarkEngine::addWalls(Area *area) {
	if (!area->entranceWithID(254))
		return;

	AreaConnections *cons = (AreaConnections *)area->entranceWithID(254);
	debugC(1, kFreescapeDebugParser, "Adding walls for area %d:", area->getAreaID());
	int id = 240;
	for (uint i = 1; i < cons->_connections.size(); i = i + 2) {
		int target = cons->_connections[i];
		debugC(1, kFreescapeDebugParser, "Connection to %d using id: %d", target, id);
		if (target > 0) {
			area->addObjectFromArea(id, _areaMap[255]);
			GeometricObject *gobj = (GeometricObject *)area->objectWithID(id);
			assert((*(gobj->_condition[0]._thenInstructions))[0].getType() == Token::Type::GOTO);
			assert((*(gobj->_condition[0]._thenInstructions))[0]._destination == 0);
			(*(gobj->_condition[0]._thenInstructions))[0].setSource(target);
		} else
			area->addObjectFromArea(id + 1, _areaMap[255]);

		id = id + 2;
	}
}

void DarkEngine::addECD(Area *area, const Math::Vector3d position, int index) {
	GeometricObject *obj = nullptr;
	Math::Vector3d origin = position;

	int16 id = 227 + index * 6;
	int heightLastObject = 0;
	for (int i = 0; i < 4; i++) {
		debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
		obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
		assert(obj);
		// Set position for object
		origin.setValue(0, origin.x());
		origin.setValue(1, origin.y() + heightLastObject);
		origin.setValue(2, origin.z());

		obj = (GeometricObject *)obj->duplicate();
		obj->setOrigin(origin);
		obj->makeVisible();
		area->addObject(obj);

		heightLastObject = obj->getSize().y();
		id--;
	}
}

void DarkEngine::restoreECD(Area *area, int index) {
	Object *obj = nullptr;
	int16 id = 227 + index * 6;
	for (int i = 0; i < 4; i++) {
		debugC(1, kFreescapeDebugParser, "Restoring object %d to from ECD %d", id, index);
		obj = (GeometricObject *)area->objectWithID(id);
		obj->restore();
		obj->makeVisible();
		id--;
	}
}

bool DarkEngine::checkECD(uint16 areaID, int index) {
	Area *area = _areaMap[areaID];
	assert(area != nullptr);
	int16 id = 227 + index * 6 - 2;
	debugC(1, kFreescapeDebugParser, "Checking object %d to from ECD %d", id, index);
	Object *obj = (GeometricObject *)area->objectWithID(id);
	assert(obj != nullptr);
	debugC(1, kFreescapeDebugParser, "Result: %d", !obj->isDestroyed());
	return !obj->isDestroyed();
}

void DarkEngine::initGameState() {
	_flyMode = false;
	_noClipMode = false;
	_playerWasCrushed = false;
	_shootingFrames = 0;
	_underFireFrames = 0;
	_yaw = 0;
	_pitch = 0;

	for (int i = 0; i < k8bitMaxVariable; i++) // TODO: check maximum variable
		_gameStateVars[i] = 0;

	for (auto &it : _areaMap) {
		it._value->resetArea();
		_gameStateBits[it._key] = 0;
	}

	_gameStateVars[k8bitVariableEnergy] = _initialEnergy;
	_gameStateVars[k8bitVariableShield] = _initialShield;
	_gameStateVars[kVariableActiveECDs] = 100;

	_playerHeightNumber = 1;
	_playerHeight = _playerHeights[_playerHeightNumber];
	removeTimers();
	startCountdown(_initialCountdown);
	_lastMinute = 0;
	_demoIndex = 0;
	_demoEvents.clear();
	_exploredAreas.clear();
	_exploredAreas[_startArea] = true;
}

bool DarkEngine::tryDestroyECDFullGame(int index) {
	switch (_currentArea->getAreaID()) {
		case 1:
			assert(index == 0);
			return true;

		case 4:
			assert(index == 0);
			return !(checkECD(1, 0) && checkECD(10, 0));

		case 5:
			assert(index == 0);
			return !(checkECD(12, 0) && checkECD(12, 1)); // Check both

		case 8:
			assert(index <= 1);
			if (index == 0)
				return true;
			else if (index == 1)
				return !(checkECD(18, 0) && checkECD(10, 0)); // Check both
			break;

		case 10:
			assert(index <= 2);
			if (index == 0)
				return !(checkECD(4, 0) && checkECD(18, 1));
			else if (index == 1) {
				int connections = 0;
				connections += checkECD(16, 0);
				connections += checkECD(8, 0);
				connections += checkECD(11, 0);
				return connections <= 1;
			} else if (index == 2)
				return true;
			break;

		case 11:
			assert(index <= 1);
			if (index == 0)
				return true;
			else if (index == 1)
				return !(checkECD(10, 0) && checkECD(12, 0)); // TODO: verify
			break;

		case 12:
			assert(index <= 2);
			if (index == 0)
				return !(checkECD(5, 0) && checkECD(11, 1)); // Check last one
			else if (index == 1)
				return !(checkECD(5, 0) && checkECD(13, 0)); // Check both
			else if (index == 2)
				return true;
			else
				assert(false);

			break;

		case 13:
			assert(index <= 1);
			if (index == 0)
				return !(checkECD(13, 1) && checkECD(12, 1));
			else if (index == 1)
				return true;
			else
				assert(false);
			break;

		case 14:
			if (index == 0)
				return true;
			else if (index == 1)
				return !(checkECD(14, 0) && checkECD(14, 2));
			else if (index == 2)
				return !(checkECD(14, 1) && checkECD(18, 0));
			else
				assert(false);
			break;
		case 16:
			assert(index <= 1);
			if (index == 0)
				return !(checkECD(10, 1) && checkECD(18, 0));
			else if (index == 1)
				return !(checkECD(10, 2) && checkECD(18, 1));
			else
				assert(false);
			break;

		case 17:
			assert(index <= 2);
			if (index == 0)
				return !(checkECD(12, 2) && checkECD(18, 1));
			else if (index == 1)
				return true;
			else if (index == 2)
				return !(checkECD(17, 1) && checkECD(18, 1));
			else
				assert(0);
			break;

		case 18:
			assert(index <= 1);
			if (index == 0) {
				int connections = 0;
				connections += checkECD(16, 0);
				connections += checkECD(8, 1);
				connections += checkECD(14, 2);
				return connections <= 1;
			} else if (index == 1) {
				int connections = 0;
				connections += checkECD(10, 0);
				connections += checkECD(16, 1);
				connections += checkECD(17, 0);
				connections += checkECD(17, 2);
				return connections <= 1;
			} else
				assert(false);
		default:
			break;

	}
	error("Not implemented");
}

bool DarkEngine::tryDestroyECD(int index) {
	if (isDemo()) {
		if (index == 1) {
			return false;
		}
		return true;
	} else {
		return tryDestroyECDFullGame(index);
	}
	return true; // Unreachable

}

void DarkEngine::addSkanner(Area *area) {
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

bool DarkEngine::checkIfGameEnded() {
	if (_gameStateVars[kVariableDarkECD] > 0) {
		int index = _gameStateVars[kVariableDarkECD] - 1;
		//insertTemporaryMessage(Common::String::format("%-14d", _gameStateVars[kVariableDarkECD] - 1), _countdown - 2);
		//restoreECD(_currentArea, _gameStateVars[kVariableDarkECD] - 1);
		bool destroyed = tryDestroyECD(index);
		if (destroyed) {
			_gameStateVars[kVariableActiveECDs] -= 4;
			_gameStateVars[k8bitVariableScore] += 52750;
			insertTemporaryMessage(_messagesList[2], _countdown - 2);
		} else {
			restoreECD(_currentArea, index);
			insertTemporaryMessage(_messagesList[1], _countdown - 2);
		}
		_gameStateVars[kVariableDarkECD] = 0;
	}

	if (_hasFallen) {
		_hasFallen = false;
		_gameStateVars[kVariableDarkEnding] = kDarkEndingEvathDestroyed;
		playSound(14, false);
		insertTemporaryMessage(_messagesList[17], _countdown - 4);
		drawBackground();
		drawBorder();
		drawUI();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(1000);
		gotoArea(1, 26);
	} else if (_playerWasCrushed) {
		insertTemporaryMessage(_messagesList[10], _countdown - 2);
		_gameStateVars[kVariableDarkEnding] = kDarkEndingEvathDestroyed;
		drawFrame();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(2000);
		_playerWasCrushed = false;
		gotoArea(1, 26);
	} else if (_gameStateVars[k8bitVariableShield] == 0) {
		insertTemporaryMessage(_messagesList[15], _countdown - 2);
		_gameStateVars[kVariableDarkEnding] = kDarkEndingEvathDestroyed;
		drawFrame();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(2000);
		gotoArea(1, 26);
	} else if (_forceEndGame) {
		_forceEndGame = false;
		insertTemporaryMessage(_messagesList[18], _countdown - 2);
		_gameStateVars[kVariableDarkEnding] = kDarkEndingEvathDestroyed;
		drawFrame();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(2000);
		gotoArea(1, 26);
	}

	if (_currentArea->getAreaID() == 1) {
		rotate(0, 10);
		drawFrame();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(20);
		executeLocalGlobalConditions(false, true, false);
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(200);
		return true;
	}
	return false;
}

void DarkEngine::gotoArea(uint16 areaID, int entranceID) {
	debugC(1, kFreescapeDebugMove, "Jumping to area: %d, entrance: %d", areaID, entranceID);
	if (!_gameStateBits.contains(areaID))
		_gameStateBits[areaID] = 0;

	if (!_exploredAreas.contains(areaID)) {
		_gameStateVars[k8bitVariableScore] += 17500;
		_exploredAreas[areaID] = true;
	}

	if (isDemo()) {
		if (!_areaMap.contains(areaID)) {
			drawFullscreenMessageAndWait(_messagesList[30]);
			return;
		}
	}

	assert(_areaMap.contains(areaID));
	_currentArea = _areaMap[areaID];
	_currentArea->show();

	_currentAreaMessages.clear();
	_currentAreaMessages.push_back(_currentArea->_name);

	int scale = _currentArea->getScale();
	assert(scale > 0);

	if (entranceID > 0 || areaID == 127) {
		traverseEntrance(entranceID);
	} else if (entranceID == 0) {
		int newPos = -1;
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
		}
		assert(newPos != -1);
		_sensors = _currentArea->getSensors();
	}

	_lastPosition = _position;
	_gameStateVars[0x1f] = 0;

	if (areaID == _startArea && entranceID == _startEntrance) {
		_yaw = 90;
		_pitch = 0;
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

void DarkEngine::pressedKey(const int keycode) {
	if (keycode == Common::KEYCODE_j) {
		_flyMode = !_flyMode;

		if (_flyMode && _gameStateVars[k8bitVariableEnergy] == 0) {
			_flyMode = false;
			insertTemporaryMessage(_messagesList[13], _countdown - 2);
		} else if (_flyMode)
			insertTemporaryMessage(_messagesList[11], _countdown - 2);
		else {
			resolveCollisions(_position);
			if (!_hasFallen)
				insertTemporaryMessage(_messagesList[12], _countdown - 2);
		}
	}
}

void DarkEngine::updateTimeVariables() {
	// This function only executes "on collision" room/global conditions
	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	if (_flyMode && seconds != _lastSecond) {
		_jetFuelSeconds--;
		_lastSecond = seconds;
		if (seconds % 6 == 0)
			if (_gameStateVars[k8bitVariableEnergy] > 0)
				_gameStateVars[k8bitVariableEnergy]--;

		if (_flyMode && _gameStateVars[k8bitVariableEnergy] == 0) {
			_flyMode = false;
			insertTemporaryMessage(_messagesList[13], _countdown - 2);
		}
	}
	if (_lastTenSeconds != seconds / 10) {
		_gameStateVars[0x1e] += 1;
		_gameStateVars[0x1f] += 1;
		_lastTenSeconds = seconds / 10;
		executeLocalGlobalConditions(false, false, true);
	}

	if (_lastMinute != minutes) {
		_lastMinute = minutes;
		executeLocalGlobalConditions(false, true, false);
	}
}

void DarkEngine::borderScreen() {
	if (_border) {
		drawBorder();
		if (isDemo()) {
			drawFullscreenMessageAndWait(_messagesList[27]);
			drawFullscreenMessageAndWait(_messagesList[28]);
			drawFullscreenMessageAndWait(_messagesList[29]);
		} else {
			FreescapeEngine::borderScreen();
		}
	}
}

void DarkEngine::executePrint(FCLInstruction &instruction) {
	uint16 index = instruction._source - 1;
	debugC(1, kFreescapeDebugCode, "Printing message %d", index);
	if (index > 127) {
		index = _messagesList.size() - (index - 254) - 2;
		drawFullscreenMessageAndWait(_messagesList[index]);
		return;
	}
	insertTemporaryMessage(_messagesList[index], _countdown - 2);
}

void DarkEngine::drawFullscreenMessage(Common::String message, uint32 front, Graphics::Surface *surface) {
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
	uint32 color = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00);

	surface->fillRect(_fullscreenViewArea, color);
	surface->fillRect(_viewArea, black);
	int x = 0;
	int y = 0;
	int letterPerLine = 0;
	int numberOfLines = 0;

	if (isDOS()) {
		x = 50;
		y = 32;
		letterPerLine = 28;
		numberOfLines = 10;
	} else if (isSpectrum()) {
		x = 60;
		y = 35;
		letterPerLine = 24;
		numberOfLines = 12;
	}

	for (int i = 0; i < numberOfLines; i++) {
		Common::String line = message.substr(letterPerLine * i, letterPerLine);
		//debug("'%s' %d", line.c_str(), line.size());
		drawStringInSurface(line, x, y, front, black, surface);
		y = y + 8;
	}

	drawFullscreenSurface(surface);
}

void DarkEngine::drawFullscreenMessageAndWait(Common::String message) {
	_savedScreen = _gfx->getScreenshot();
	uint32 color = 0;
	switch (_renderMode) {
		case Common::kRenderCGA:
			color = 1;
			break;
		case Common::kRenderZX:
			color = 6;
			break;
		default:
			color = 14;
	}
	uint8 r, g, b;
	_gfx->readFromPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(_screenW, _screenH, _gfx->_texturePixelFormat);

	Common::Event event;
	bool cont = true;
	while (!shouldQuit() && cont) {
		while (g_system->getEventManager()->pollEvent(event)) {

			// Events
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_SPACE) {
					cont = false;
				}
				break;
			case Common::EVENT_SCREEN_CHANGED:
				_gfx->computeScreenViewport();
				break;

			default:
				break;
			}
		}
		drawBorder();
		if (_currentArea)
			drawUI();
		drawFullscreenMessage(message, front, surface);
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(15); // try to target ~60 FPS
	}

	_savedScreen->free();
	delete _savedScreen;
	surface->free();
	delete surface;
}

void DarkEngine::drawSensorShoot(Sensor *sensor) {
	Math::Vector3d target;
	target = _position;
	target.y() = target.y() - _playerHeight;
	target.x() = target.x() - 5;
	_gfx->renderSensorShoot(1, sensor->getOrigin(), target, _viewArea);

	target = _position;
	target.y() = target.y() - _playerHeight;
	_gfx->renderSensorShoot(1, sensor->getOrigin(), target, _viewArea);

	target = _position;
	target.y() = target.y() - _playerHeight;
	target.x() = target.x() + 5;
	_gfx->renderSensorShoot(1, sensor->getOrigin(), target, _viewArea);
}

void DarkEngine::drawInfoMenu() {
	_savedScreen = _gfx->getScreenshot();
	uint32 color = 0;
	switch (_renderMode) {
		case Common::kRenderCGA:
			color = 1;
			break;
		case Common::kRenderZX:
			color = 6;
			break;
		default:
			color = 14;
	}
	uint8 r, g, b;
	_gfx->readFromPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);

	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(_screenW, _screenH, _gfx->_texturePixelFormat);

	surface->fillRect(Common::Rect(88, 48, 231, 103), black);
	surface->frameRect(Common::Rect(88, 48, 231, 103), front);

	surface->frameRect(Common::Rect(90, 50, 229, 101), front);

	drawStringInSurface("L-LOAD S-SAVE", 105, 56, front, black, surface);
	if (isSpectrum())
		drawStringInSurface("1-TERMINATE", 105, 64, front, black, surface);
	else
		drawStringInSurface("ESC-TERMINATE", 105, 64, front, black, surface);

	drawStringInSurface("T-TOGGLE", 128, 81, front, black, surface);
	drawStringInSurface("SOUND ON/OFF", 113, 88, front, black, surface);

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
				break;

			default:
				break;
			}
		}
		drawFrame();
		drawFullscreenSurface(surface);

		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(15); // try to target ~60 FPS
	}

	_savedScreen->free();
	delete _savedScreen;
	surface->free();
	delete surface;
}

Common::Error DarkEngine::saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave) {
	for (auto &it : _areaMap) {
		stream->writeUint16LE(it._key);
		stream->writeUint32LE(_exploredAreas[it._key]);
	}
	return Common::kNoError;
}

Common::Error DarkEngine::loadGameStreamExtended(Common::SeekableReadStream *stream) {
	for (uint i = 0; i < _areaMap.size(); i++) {
		uint16 key = stream->readUint16LE();
		_exploredAreas[key] = stream->readUint32LE();
	}
	return Common::kNoError;
}

} // End of namespace Freescape
