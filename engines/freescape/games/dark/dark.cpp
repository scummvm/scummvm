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
#include "freescape/language/8bitDetokeniser.h"
#include "freescape/objects/global.h"
#include "freescape/objects/connections.h"

namespace Freescape {

DarkEngine::DarkEngine(OSystem *syst, const ADGameDescription *gd) : FreescapeEngine(syst, gd) {
	_viewArea = Common::Rect(40, 24, 279, 124);
	_playerHeightNumber = 1;
	_playerHeights.push_back(16);
	_playerHeights.push_back(48);

	_playerHeight = _playerHeights[_playerHeightNumber];
	_playerWidth = 12;
	_playerDepth = 32;
	_lastTenSeconds = -1;

	_angleRotations.push_back(5);
	_angleRotations.push_back(10);
	_angleRotations.push_back(15);
	_angleRotations.push_back(30);
	_angleRotations.push_back(45);
	_angleRotations.push_back(90);

	_initialFuel = 11;
	_initialShield = 15;
}

void DarkEngine::titleScreen() {
	if (isAmiga() || isAtariST()) // These releases has their own screens
		return;

	if (_title) {
		drawTitle();
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(3000);
	}
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
			assert((*(gobj->_condition[1]._thenInstructions))[0].getType() == Token::Type::GOTO);
			assert((*(gobj->_condition[1]._thenInstructions))[0]._destination == 0);
			(*(gobj->_condition[1]._thenInstructions))[0].setSource(target);
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

void DarkEngine::initGameState() {
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
	}

	_gameStateVars[k8bitVariableEnergy] = _initialFuel;
	_gameStateVars[k8bitVariableShield] = _initialShield;

	_playerHeightNumber = 1;
	_playerHeight = _playerHeights[_playerHeightNumber];
	removeTimers();
	startCountdown(_initialCountdown);
	_lastMinute = 0;
	_demoIndex = 0;
	_demoEvents.clear();
}

void DarkEngine::gotoArea(uint16 areaID, int entranceID) {
	debugC(1, kFreescapeDebugMove, "Jumping to area: %d, entrance: %d", areaID, entranceID);
	if (!_gameStateBits.contains(areaID))
		_gameStateBits[areaID] = 0;

	if (isDemo()) {
		if (!_areaMap.contains(areaID)) {
			drawFullscreenMessage(_messagesList[30]);
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
	}
}

void DarkEngine::updateTimeVariables() {
	// This function only executes "on collision" room/global conditions
	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	if (_lastTenSeconds != seconds / 10) {
		_lastTenSeconds = seconds / 10;
		executeLocalGlobalConditions(false, false, true);
	}

	if (_lastMinute != minutes) {
		_lastMinute = minutes;
		_gameStateVars[0x1e] += 1;
		_gameStateVars[0x1f] += 1;
		executeLocalGlobalConditions(false, true, false);
	}
}

void DarkEngine::borderScreen() {
	if (_border) {
		drawBorder();
		if (isDemo()) {
			drawFullscreenMessage(_messagesList[27]);
			drawFullscreenMessage(_messagesList[28]);
			drawFullscreenMessage(_messagesList[29]);
		} else {
			_gfx->flipBuffer();
			g_system->updateScreen();
			g_system->delayMillis(3000);
		}
	}
}

void DarkEngine::executePrint(FCLInstruction &instruction) {
	uint16 index = instruction._source - 1;
	debugC(1, kFreescapeDebugCode, "Printing message %d", index);
	_currentAreaMessages.clear();
	if (index > 127) {
		index = _messagesList.size() - (index - 254) - 2;
		drawFullscreenMessage(_messagesList[index]);
		return;
	}
	_currentAreaMessages.push_back(_messagesList[index]);
}

void DarkEngine::drawFullscreenMessage(Common::String message) {
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
		default:
			color = 14;
	}
	uint8 r, g, b;
	_gfx->readFromPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	int x, y;
	x = 42;
	y = 30;
	for (int i = 0; i < 8; i++) {
		Common::String line = message.substr(28 * i, 28);
		debug("'%s' %d", line.c_str(), line.size());
		drawStringInSurface(line, x, y, front, black, surface);
		y = y + 12;
	}

	if (!_uiTexture)
		_uiTexture = _gfx->createTexture(surface);
	else
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
				if (event.kbd.keycode == Common::KEYCODE_SPACE) {
					cont = false;
				}
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

Common::Error DarkEngine::saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave) {
	return Common::kNoError;
}

Common::Error DarkEngine::loadGameStreamExtended(Common::SeekableReadStream *stream) {
	return Common::kNoError;
}

} // End of namespace Freescape
