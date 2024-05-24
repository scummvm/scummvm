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
#include "common/memstream.h"

#include "freescape/freescape.h"
#include "freescape/games/castle/castle.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

CastleEngine::CastleEngine(OSystem *syst, const ADGameDescription *gd) : FreescapeEngine(syst, gd) {
	_playerHeightNumber = 1;
	_playerHeights.push_back(16);
	_playerHeights.push_back(48);
	_playerHeight = _playerHeights[_playerHeightNumber];

	_playerSteps.clear();
	_playerSteps.push_back(1);
	_playerSteps.push_back(10);
	_playerSteps.push_back(25);
	_playerStepIndex = 2;

	_playerWidth = 8;
	_playerDepth = 8;
	_stepUpDistance = 32;
	_maxFallingDistance = 8192;
	_option = nullptr;
}

CastleEngine::~CastleEngine() {
	if (_option) {
		_option->free();
		delete _option;
	}
}

byte kFreescapeCastleFont[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x1c, 0x1c, 0x1c, 0x18, 0x18, 0x00, 0x18, 0x18,
	0x66, 0x66, 0x44, 0x22, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x00,
	0x10, 0x54, 0x38, 0xfe, 0x38, 0x54, 0x10, 0x00,
	0x3c, 0x42, 0x9d, 0xb1, 0xb1, 0x9d, 0x42, 0x3c,
	0x78, 0xcc, 0xcc, 0x78, 0xdb, 0xcf, 0xce, 0x7b,
	0x30, 0x30, 0x10, 0x20, 0x00, 0x00, 0x00, 0x00,
	0x10, 0x20, 0x40, 0x40, 0x40, 0x40, 0x20, 0x10,
	0x10, 0x08, 0x04, 0x04, 0x04, 0x04, 0x08, 0x10,
	0x10, 0x54, 0x38, 0xfe, 0x38, 0x54, 0x10, 0x00,
	0x00, 0x00, 0x10, 0x10, 0x7c, 0x10, 0x10, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x08, 0x10,
	0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18,
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
	0x18, 0x66, 0xc3, 0xc3, 0xc3, 0xc3, 0x66, 0x18,
	0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
	0x9e, 0x61, 0x01, 0x7e, 0xe0, 0xc6, 0xe3, 0xfe,
	0xee, 0x73, 0x03, 0x3e, 0x03, 0x01, 0x7f, 0xe6,
	0x0e, 0x1c, 0x38, 0x71, 0xfd, 0xe6, 0x0c, 0x0c,
	0xfd, 0x86, 0x80, 0x7e, 0x07, 0x63, 0xc7, 0x7c,
	0x3d, 0x66, 0xc0, 0xf0, 0xfc, 0xc6, 0x66, 0x3c,
	0xb3, 0x4e, 0x06, 0x0c, 0x0c, 0x18, 0x18, 0x3c,
	0x7c, 0xc6, 0xc6, 0x7c, 0xc6, 0xc2, 0xfe, 0x4c,
	0x3c, 0x4e, 0xc6, 0xc6, 0x4e, 0x36, 0x46, 0x3c,
	0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00,
	0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x08, 0x10,
	0x03, 0x0c, 0x30, 0xc0, 0x30, 0x0c, 0x03, 0x00,
	0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00,
	0xc0, 0x30, 0x0c, 0x03, 0x0c, 0x30, 0xc0, 0x00,
	0x7c, 0xc6, 0x06, 0x0c, 0x30, 0x30, 0x00, 0x30,
	0x00, 0x08, 0x0c, 0xfe, 0xff, 0xfe, 0x0c, 0x08,
	0x1e, 0x1c, 0x1e, 0x66, 0xbe, 0x26, 0x43, 0xe3,
	0xee, 0x73, 0x23, 0x3e, 0x23, 0x21, 0x7f, 0xe6,
	0x39, 0x6e, 0xc6, 0xc0, 0xc0, 0xc2, 0x63, 0x3e,
	0xec, 0x72, 0x23, 0x23, 0x23, 0x23, 0x72, 0xec,
	0xce, 0x7f, 0x61, 0x6c, 0x78, 0x61, 0x7f, 0xce,
	0xce, 0x7f, 0x61, 0x6c, 0x78, 0x60, 0x60, 0xf0,
	0x3d, 0x66, 0xc0, 0xc1, 0xce, 0xc6, 0x66, 0x3c,
	0xe7, 0x66, 0x66, 0x6e, 0x76, 0x66, 0x66, 0xe7,
	0x66, 0x3c, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x66,
	0x33, 0x1e, 0x0c, 0x8c, 0x4c, 0xcc, 0xdc, 0x78,
	0xf2, 0x67, 0x64, 0x68, 0x7e, 0x66, 0x66, 0xf3,
	0xd8, 0x70, 0x60, 0x60, 0x66, 0x61, 0xf3, 0x7e,
	0xc3, 0x66, 0x6e, 0x76, 0x56, 0x46, 0x46, 0xef,
	0x87, 0x62, 0x72, 0x7a, 0x5e, 0x4e, 0x46, 0xe1,
	0x18, 0x66, 0xc3, 0xc3, 0xc3, 0xc3, 0x66, 0x18,
	0xec, 0x72, 0x63, 0x63, 0x72, 0x6c, 0x60, 0xf0,
	0x3c, 0x66, 0xc3, 0xc3, 0x66, 0x3c, 0x31, 0x1e,
	0xec, 0x72, 0x63, 0x63, 0x76, 0x6c, 0x66, 0xf1,
	0x79, 0x86, 0x80, 0x7e, 0x07, 0x63, 0xc7, 0x7c,
	0x01, 0x7f, 0xfe, 0x98, 0x58, 0x18, 0x18, 0x3c,
	0xf7, 0x62, 0x62, 0x62, 0x62, 0x62, 0xf2, 0x3c,
	0xf3, 0x61, 0x72, 0x72, 0x32, 0x32, 0x1c, 0x3e,
	0xc3, 0x62, 0x62, 0x6a, 0x6e, 0x76, 0x66, 0xc3,
	0xf3, 0x72, 0x3c, 0x38, 0x1c, 0x3c, 0x4e, 0xcf,
	0xe3, 0x72, 0x34, 0x38, 0x18, 0x18, 0x18, 0x3c,
	0x7f, 0x87, 0x0e, 0x1c, 0x38, 0x71, 0xfd, 0xe6,
};

void CastleEngine::gotoArea(uint16 areaID, int entranceID) {
	debugC(1, kFreescapeDebugMove, "Jumping to area: %d, entrance: %d", areaID, entranceID);

	assert(_areaMap.contains(areaID));
	_currentArea = _areaMap[areaID];
	_currentArea->show();

	if (entranceID > 0)
		traverseEntrance(entranceID);

	_lastPosition = _position;

	if (_currentArea->_skyColor > 0 && _currentArea->_skyColor != 255) {
		_gfx->_keyColor = 0;
	} else
		_gfx->_keyColor = 255;

	_lastPosition = _position;
	_gameStateVars[0x1f] = 0;

	if (areaID == _startArea && entranceID == _startEntrance) {
		_yaw = 310;
		_pitch = 0;
		playSound(9, false);
	} else {
		playSound(5, false);
	}

	debugC(1, kFreescapeDebugMove, "starting player position: %f, %f, %f", _position.x(), _position.y(), _position.z());
	clearTemporalMessages();
	// Ignore sky/ground fields
	_gfx->_keyColor = 0;
	_gfx->clearColorPairArray();

	_gfx->_colorPair[_currentArea->_underFireBackgroundColor] = _currentArea->_extraColor[0];
	_gfx->_colorPair[_currentArea->_usualBackgroundColor] = _currentArea->_extraColor[1];
	_gfx->_colorPair[_currentArea->_paperColor] = _currentArea->_extraColor[2];
	_gfx->_colorPair[_currentArea->_inkColor] = _currentArea->_extraColor[3];

	swapPalette(areaID);
	resetInput();
}

void CastleEngine::initGameState() {
	FreescapeEngine::initGameState();
	_playerHeightNumber = 1;
	_playerHeight = _playerHeights[_playerHeightNumber];

	_gameStateVars[k8bitVariableShield] = 1;
	_gameStateVars[k8bitVariableEnergy] = 1;
	_countdown = INT_MAX;
}

void CastleEngine::endGame() {
	_shootingFrames = 0;
	_delayedShootObject = nullptr;
	_endGamePlayerEndArea = true;

	if (_endGameKeyPressed) {
		_gameStateControl = kFreescapeGameStateRestart;
		_endGameKeyPressed = false;
	}
}

void CastleEngine::pressedKey(const int keycode) {
	// This code is duplicated in the DrillerEngine::pressedKey (except for the J case)
	if (keycode == Common::KEYCODE_z) {
		rotate(-_angleRotations[_angleRotationIndex], 0);
	} else if (keycode == Common::KEYCODE_x) {
		rotate(_angleRotations[_angleRotationIndex], 0);
	} else if (keycode == Common::KEYCODE_s) {
		// TODO: show score
	} else if (keycode ==  Common::KEYCODE_r) {
		if (_playerHeightNumber == 0)
			rise();
		// TODO: raising can fail if there is no room, so the action should fail
		_playerStepIndex = 1;
		insertTemporaryMessage(_messagesList[15], _countdown - 2);
	} else if (keycode == Common::KEYCODE_w) {
		if (_playerHeightNumber == 0)
			rise();
		// TODO: raising can fail if there is no room, so the action should fail
		_playerStepIndex = 1;
		insertTemporaryMessage(_messagesList[14], _countdown - 2);
	} else if (keycode == Common::KEYCODE_c) {
		if (_playerHeightNumber == 1)
			lower();
		_playerStepIndex = 0;
		insertTemporaryMessage(_messagesList[13], _countdown - 2);
	} else if (keycode == Common::KEYCODE_f) {
		_pitch = 0;
		updateCamera();
	}
}

void CastleEngine::executePrint(FCLInstruction &instruction) {
	uint16 index = instruction._source;
	_currentAreaMessages.clear();
	if (index > 129) {
		index = index - 129;
		if (index < _riddleList.size())
			drawFullscreenRiddleAndWait(index);
		else
			debugC(1, kFreescapeDebugCode, "Riddle index %d out of bounds", index);
		return;
	}
	debugC(1, kFreescapeDebugCode, "Printing message %d: \"%s\"", index, _messagesList[index].c_str());
	insertTemporaryMessage(_messagesList[index], _countdown - 3);
}


void CastleEngine::loadRiddles(Common::SeekableReadStream *file, int offset, int number) {
	file->seek(offset);
	debugC(1, kFreescapeDebugParser, "Riddle table:");

	int numberAsteriskLines = 0;
	for (int i = 0; i < number; i++) {
		numberAsteriskLines = 0;
		debugC(1, kFreescapeDebugParser, "riddle %d extra byte each 6: %x", i, file->readByte());

		for (int j = 0; j < 6; j++) {
			int size = file->readByte();
			debugC(1, kFreescapeDebugParser, "size: %d (max 22?)", size);
			//if (size > 22)
			//	size = 22;
			int padSpaces = (22 - size) / 2;
			debugC(1, kFreescapeDebugParser, "extra byte: %x", file->readByte());
			Common::String message = "";
			int k = padSpaces;

			if (size > 0) {
				while (k-- > 0)
					message = message + " ";

				while (size-- > 0) {
					byte c = file->readByte();
					if (c != 0)
						message = message + c;
				}

				k = padSpaces;
				while (k-- > 0)
					message = message + " ";
			}


			if (isAmiga() || isAtariST())
				debug("extra byte: %x", file->readByte());
			debugC(1, kFreescapeDebugParser, "extra byte: %x", file->readByte());
			debugC(1, kFreescapeDebugParser, "extra byte: %x", file->readByte());
			debugC(1, kFreescapeDebugParser, "'%s'", message.c_str());

			_riddleList.push_back(message);
			if (message.size() > 0 && message[0] == '*')
				numberAsteriskLines++;

			if (numberAsteriskLines == 2 && j < 5) {
				assert(j == 4);
				_riddleList.push_back("");
				debugC(1, kFreescapeDebugParser, "Padded with ''");
				break;
			}
		}

	}
	debugC(1, kFreescapeDebugParser, "End of riddles at %lx", file->pos());
}

void CastleEngine::drawFullscreenRiddleAndWait(uint16 riddle) {
	_savedScreen = _gfx->getScreenshot();
	uint32 color = 0;
	switch (_renderMode) {
		case Common::kRenderCPC:
			color = 14;
			break;
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
	_gfx->readFromPalette(6, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);
	_gfx->readFromPalette(color, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(_screenW, _screenH, _gfx->_texturePixelFormat);

	Common::Event event;
	bool cont = true;
	while (!shouldQuit() && cont) {
		while (_eventManager->pollEvent(event)) {

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
			case Common::EVENT_RBUTTONDOWN:
				// fallthrough
			case Common::EVENT_LBUTTONDOWN:
				if (g_system->hasFeature(OSystem::kFeatureTouchscreen))
					cont = false;
				break;
			default:
				break;
			}
		}
		_gfx->clear(0, 0, 0, true);
		drawBorder();
		if (_currentArea)
			drawUI();
		drawRiddle(riddle, front, back, surface);
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(15); // try to target ~60 FPS
	}

	_savedScreen->free();
	delete _savedScreen;
	surface->free();
	delete surface;
}

void CastleEngine::drawRiddle(uint16 riddle, uint32 front, uint32 back, Graphics::Surface *surface) {

	Common::StringArray riddleMessages;
	for (int i = 6 * riddle; i < 6 * (riddle + 1); i++) {
		riddleMessages.push_back(_riddleList[i]);
	}

	uint32 noColor = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00);
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
	uint32 frame = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xA7, 0xA7, 0xA7);

	surface->fillRect(_fullscreenViewArea, noColor);
	surface->fillRect(_viewArea, black);

	surface->frameRect(Common::Rect(47, 47, 271, 147), frame);
	surface->frameRect(Common::Rect(53, 53, 266, 141), frame);

	surface->fillRect(Common::Rect(54, 54, 266, 139), back);
	int x = 0;
	int y = 0;
	int numberOfLines = 6;

	if (isDOS()) {
		x = 58;
		y = 66;
	} else if (isSpectrum() || isCPC()) {
		x = 60;
		y = 40;
	}

	for (int i = 0; i < numberOfLines; i++) {
		drawStringInSurface(riddleMessages[i], x, y, front, back, surface);
		y = y + 12;
	}
	drawFullscreenSurface(surface);
}

void CastleEngine::addGhosts() {
	for (auto &it : _areaMap) {
		for (auto &sensor : it._value->getSensors()) {
			if (sensor->getObjectID() == 125) {
				_areaMap[it._key]->addGroupFromArea(195, _areaMap[255]);
				_areaMap[it._key]->addGroupFromArea(212, _areaMap[255]);
			} else if (sensor->getObjectID() == 126)
				_areaMap[it._key]->addGroupFromArea(191, _areaMap[255]);
			else if (sensor->getObjectID() == 127)
				_areaMap[it._key]->addGroupFromArea(182, _areaMap[255]);
			else
				debugC(1, kFreescapeDebugParser, "Sensor %d in area %d", sensor->getObjectID(), it._key);
		}
	}
}

void CastleEngine::checkSensors() {
	if (_disableSensors)
		return;

	if (_lastTick == _ticks)
		return;

	_lastTick = _ticks;

	if (_sensors.empty())
		return;

	Sensor *sensor = (Sensor *)&_sensors[0];
	if (sensor->getObjectID() == 125) {
		Group *group = (Group *)_currentArea->objectWithID(195);
		if (!group->isDestroyed() && !group->isInvisible()) {
			group->_active = true;
		} else
			return;

		group = (Group *)_currentArea->objectWithID(212);
		if (!group->isDestroyed() && !group->isInvisible()) {
			group->_active = true;
		} else
			return;

	} else if (sensor->getObjectID() == 126) {
		Group *group = (Group *)_currentArea->objectWithID(191);
		if (!group->isDestroyed() && !group->isInvisible()) {
			group->_active = true;
		} else
			return;
	} else if (sensor->getObjectID() == 197) {
		Group *group = (Group *)_currentArea->objectWithID(182);
		if (!group->isDestroyed() && !group->isInvisible()) {
			group->_active = true;
		} else
			return;
	}

	/*int firingInterval = 10; // This is fixed for all the ghosts?
	if (_ticks % firingInterval == 0) {
		if (_underFireFrames <= 0)
			_underFireFrames = 4;
		takeDamageFromSensor();
	}*/
}


Common::Error CastleEngine::saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave) {
	return Common::kNoError;
}

Common::Error CastleEngine::loadGameStreamExtended(Common::SeekableReadStream *stream) {
	return Common::kNoError;
}

} // End of namespace Freescape
