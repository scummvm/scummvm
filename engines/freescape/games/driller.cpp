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

enum {
	kDrillerNoRig = 0,
	kDrillerRigInPlace = 1,
	kDrillerRigOutOfPlace = 2,
};

DrillerEngine::DrillerEngine(OSystem *syst, const ADGameDescription *gd) : FreescapeEngine(syst, gd) {

	if (!Common::parseBool(ConfMan.get("automatic_drilling"), _useAutomaticDrilling))
		error("Failed to parse bool from automatic_drilling option");

	if (isDOS())
		_viewArea = Common::Rect(40, 16, 280, 117);
	else if (isAmiga() || isAtariST())
		_viewArea = Common::Rect(36, 16, 284, 118);

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
}

void DrillerEngine::gotoArea(uint16 areaID, int entranceID) {
	int prevAreaID = _currentArea ? _currentArea->getAreaID(): -1;
	debugC(1, kFreescapeDebugMove, "Jumping to area: %d, entrance: %d", areaID, entranceID);
	if (!_gameStateBits.contains(areaID))
		_gameStateBits[areaID] = 0;

	assert(_areaMap.contains(areaID));
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

	if (areaID == _startArea && entranceID == _startEntrance) {
		_yaw = 280;
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

	if (isAmiga() || isAtariST())
		swapPalette(areaID);

	_currentArea->_skyColor = 0;
	_currentArea->_usualBackgroundColor = 0;

	if (areaID != _startArea || entranceID != _startEntrance) {
		g_system->warpMouse(_crossairPosition.x, _crossairPosition.y);
		_lastMousePos = _crossairPosition;
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

		load8bitBinary(&file, 0x442, 16);
		loadPalettes(&file, 0x0);

		file.close();
		file.open("driller");
		if (!file.isOpen())
			error("Failed to open 'driller' file");

		loadFonts(&file, 0xa30);
		loadMessagesFixedSize(&file, 0x3960, 14, 20);
		loadGlobalObjects(&file, 0x3716);

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

		load8bitBinary(&file, 0x442, 16);
		loadPalettes(&file, 0x0);

		file.close();
		file.open("x.prg");
		if (!file.isOpen())
			error("Failed to open 'x.prg' file");

		loadFonts(&file, 0x7bc);
		loadMessagesFixedSize(&file, 0x3b90, 14, 20);
		loadGlobalObjects(&file, 0x3946);

		file.close();
		file.open("soundfx");
		if (!file.isOpen())
			error("Failed to open 'soundfx' executable for AtariST demo");

		loadSoundsFx(&file, 0, 25);
	} else
		error("Unsupported demo for Driller");

	_demoMode = !_disableDemoMode;
	_angleRotationIndex = 0;
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

			loadFonts(&file, 0x8940);
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

			loadFonts(&file, 0xa62);
			loadMessagesFixedSize(&file, 0x499a, 14, 20);
			loadGlobalObjects(&file, 0x4098);
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

		loadFonts(&file, 0x8a32);
		loadMessagesFixedSize(&file, 0xc5d8, 14, 20);
		loadGlobalObjects(&file, 0xbccc);
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

	Common::String conditionSource = detokenise8bitCondition(conditionArray, instructions);
	debugC(1, kFreescapeDebugParser, "%s", conditionSource.c_str());
	_areaMap[18]->_conditions.push_back(instructions);
	_areaMap[18]->_conditionSources.push_back(conditionSource);
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
	drawStringInSurface(_currentArea->_name, 196, 185, yellow, black, surface);
	drawStringInSurface(Common::String::format("%04d", 2 * int(_position.x())), 150, 145, yellow, black, surface);
	drawStringInSurface(Common::String::format("%04d", 2 * int(_position.z())), 150, 153, yellow, black, surface);
	drawStringInSurface(Common::String::format("%04d", 2 * int(_position.y())), 150, 161, yellow, black, surface);
	if (_playerHeightNumber >= 0)
		drawStringInSurface(Common::String::format("%d", _playerHeightNumber), 57, 161, yellow, black, surface);
	else
		drawStringInSurface(Common::String::format("%s", "J"), 57, 161, yellow, black, surface);

	drawStringInSurface(Common::String::format("%02d", int(_angleRotations[_angleRotationIndex])), 46, 145, yellow, black, surface);
	drawStringInSurface(Common::String::format("%3d", _playerSteps[_playerStepIndex]), 46, 153, yellow, black, surface);
	drawStringInSurface(Common::String::format("%07d", score), 238, 129, yellow, black, surface);

	int hours = _countdown <= 0 ? 0 : _countdown / 3600;
	drawStringInSurface(Common::String::format("%02d", hours), 208, 8, yellow, black, surface);
	int minutes = _countdown <= 0 ? 0 : (_countdown - hours * 3600) / 60;
	drawStringInSurface(Common::String::format("%02d", minutes), 230, 8, yellow, black, surface);
	int seconds = _countdown <= 0 ? 0 : _countdown - hours * 3600 - minutes * 60;
	drawStringInSurface(Common::String::format("%02d", seconds), 254, 8, yellow, black, surface);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 190, 177, black, yellow, surface);
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
	uint32 white = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0xFF, 0xFF);
	uint32 yellow = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0xFF, 0x55);
	uint32 brownish = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x9E, 0x80, 0x20);
	uint32 brown = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x7E, 0x60, 0x19);
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
	uint32 transparent = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00);

	int score = _gameStateVars[k8bitVariableScore];
	Common::String coords;

	if (!isDemo()) { // It seems demos will not include the complete font?
		drawStringInSurface("x", 37, 18, white, transparent, surface, 82);
		coords = Common::String::format("%04d", 2 * int(_position.x()));
		for (int i = 0; i < 4; i++)
			drawStringInSurface(Common::String(coords[i]), 47 + 6*i, 18, white, transparent, surface, 112);

		drawStringInSurface("y", 37, 26, white, transparent, surface, 82);
		coords = Common::String::format("%04d", 2 * int(_position.z())); // Coords y and z are swapped!
		for (int i = 0; i < 4; i++)
			drawStringInSurface(Common::String(coords[i]), 47 + 6*i, 26, white, transparent, surface, 112);

		drawStringInSurface("z", 37, 34, white, transparent, surface, 82);
		coords = Common::String::format("%04d", 2 * int(_position.y())); // Coords y and z are swapped!
		for (int i = 0; i < 4; i++)
			drawStringInSurface(Common::String(coords[i]), 47 + 6*i, 34, white, transparent, surface, 112);
	}

	drawStringInSurface(_currentArea->_name, 188, 185, yellow, black, surface);
	drawStringInSurface(Common::String::format("%07d", score), 240, 129, yellow, black, surface);

	int hours = _countdown <= 0 ? 0 : _countdown / 3600;
	drawStringInSurface(Common::String::format("%02d:", hours), 208, 7, yellow, black, surface);
	int minutes = _countdown <= 0 ? 0 : (_countdown - hours * 3600) / 60;
	drawStringInSurface(Common::String::format("%02d:", minutes), 230, 7, yellow, black, surface);
	int seconds = _countdown <= 0 ? 0 : _countdown - hours * 3600 - minutes * 60;
	drawStringInSurface(Common::String::format("%02d", seconds), 254, 7, yellow, black, surface);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 188, 177, black, yellow, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else {
		if (_currentArea->_gasPocketRadius == 0)
			message = _messagesList[2];
		else if (_drilledAreas[_currentArea->getAreaID()])
			message = _messagesList[0];
		else
			message = _messagesList[1];

		drawStringInSurface(message, 188, 177, yellow, black, surface);
	}

	int energy = _gameStateVars[k8bitVariableEnergy];
	int shield = _gameStateVars[k8bitVariableShield];

	if (shield >= 0) {
		Common::Rect shieldBar;
		shieldBar = Common::Rect(11, 178, 76 - (k8bitMaxShield - shield), 184);
		surface->fillRect(shieldBar, brown);

		shieldBar = Common::Rect(11, 179, 76 - (k8bitMaxShield - shield), 183);
		surface->fillRect(shieldBar, brownish);

		shieldBar = Common::Rect(11, 180, 76 - (k8bitMaxShield - shield), 182);
		surface->fillRect(shieldBar, yellow);
	}

	if (energy >= 0) {
		Common::Rect energyBar;
		energyBar = Common::Rect(11, 186, 75 - (k8bitMaxEnergy - energy), 192);
		surface->fillRect(energyBar, brown);

		energyBar = Common::Rect(11, 187, 75 - (k8bitMaxEnergy - energy), 191);
		surface->fillRect(energyBar, brownish);

		energyBar = Common::Rect(11, 188, 75 - (k8bitMaxEnergy - energy), 190);
		surface->fillRect(energyBar, yellow);
	}
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
			return;
		}

		insertTemporaryMessage(_messagesList[5], _countdown - 4);
		Common::String successMessage = _messagesList[6];
		successMessage.replace(0, 4, Common::String::format("%d", int(success)));
		while (successMessage.size() < 14)
			successMessage += " ";
		insertTemporaryMessage(successMessage, _countdown - 6);
		if (success >= 50.0) {
			_drilledAreas[_currentArea->getAreaID()] = kDrillerRigInPlace;
			_gameStateVars[32]++;
		} else
			_drilledAreas[_currentArea->getAreaID()] = kDrillerRigOutOfPlace;
	} else if (keycode == Common::KEYCODE_c) {
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
	return (area->objectWithID(255) != nullptr);
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
	origin.setValue(1, origin.y() - 5);
	obj->setOrigin(origin);

	// This bounding box is too large and can result in the drill to float next to a wall
	if (_currentArea->checkCollisions(obj->_boundingBox).empty())
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
	_lastMousePos = Common::Point(0, 0);
	_yaw = 0;
	_pitch = 0;

	for (int i = 0; i < k8bitMaxVariable; i++) // TODO: check maximum variable
		_gameStateVars[i] = 0;

	for (auto &it : _areaMap) {
		it._value->resetArea();
		_gameStateBits[it._key] = 0;
		if (_drilledAreas[it._key] != kDrillerNoRig)
			removeDrill(it._value);
		_drilledAreas[it._key] = kDrillerNoRig;
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
	startCountdown(_initialCountdown);

	_demoIndex = 0;
	_demoEvents.clear();
}

bool DrillerEngine::checkIfGameEnded() {
	if (isDemo())
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

Common::Error DrillerEngine::saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave) {
	for (auto &it : _areaMap) { // All but skip area 255
		if (it._key == 255)
			continue;
		stream->writeUint16LE(it._key);
		stream->writeUint32LE(_drilledAreas[it._key]);
	}

	return Common::kNoError;
}

Common::Error DrillerEngine::loadGameStreamExtended(Common::SeekableReadStream *stream) {
	for (uint i = 0; i < _areaMap.size() - 1; i++) { // All except area 255
		uint16 key = stream->readUint16LE();
		assert(key != 255);
		assert(_areaMap.contains(key));
		_drilledAreas[key] = stream->readUint32LE();
		if (_drilledAreas[key] == kDrillerNoRig)
			if (drillDeployed(_areaMap[key]))
				removeDrill(_areaMap[key]);
	}

	return Common::kNoError;
}

} // End of namespace Freescape
