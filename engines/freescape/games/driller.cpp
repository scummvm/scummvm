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
	kDrillerCGAPalettePinkBlue = 0,
	kDrillerCGAPaletteRedGreen = 1,
};

static const struct CGAPalettteEntry {
	int areaId;
	int palette;
} rawCGAPaletteTable[] {
	{1, kDrillerCGAPaletteRedGreen},
	{2, kDrillerCGAPalettePinkBlue},
	{3, kDrillerCGAPaletteRedGreen},
	{8, kDrillerCGAPalettePinkBlue},
	{14, kDrillerCGAPalettePinkBlue},
	{0, 0}   // This marks the end
};

byte kDrillerCGAPalettePinkBlueData[4][3] = {
	{0x00, 0x00, 0x00},
	{0x00, 0xaa, 0xaa},
	{0xaa, 0x00, 0xaa},
	{0xaa, 0xaa, 0xaa},
};

byte kDrillerCGAPaletteRedGreenData[4][3] = {
	{0x00, 0x00, 0x00},
	{0x00, 0xaa, 0x00},
	{0xaa, 0x00, 0x00},
	{0xaa, 0x55, 0x00},
};

enum {
	kDrillerNoRig = 0,
	kDrillerRigInPlace = 1,
	kDrillerRigOutOfPlace = 2,
};

DrillerEngine::DrillerEngine(OSystem *syst, const ADGameDescription *gd) : FreescapeEngine(syst, gd) {

	if (!Common::parseBool(ConfMan.get("automatic_drilling"), _useAutomaticDrilling))
		error("Failed to parse bool from automatic_drilling option");

	if (isDOS()) {
		if (_renderMode == Common::kRenderEGA)
			_viewArea = Common::Rect(40, 16, 280, 117);
		else if (_renderMode == Common::kRenderCGA)
			_viewArea = Common::Rect(36, 16, 284, 117);
		else
			error("Invalid or unknown render mode");
	}
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
		if (isDOS() && isDemo()) // The DOS demo has a few missing objects
			if (i == 5)
				break;

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
	} else if (isDOS()) {
		_renderMode = Common::kRenderCGA; // DOS demos is CGA only
		_gfx->_renderMode = _renderMode;
		loadBundledImages();
		file.open("d2");
		if (!file.isOpen())
			error("Failed to open 'd2' file");

		loadFonts(&file, 0x4eb0);
		loadMessagesFixedSize(&file, 0x636, 14, 20);
		load8bitBinary(&file, 0x55b0, 4);
		loadGlobalObjects(&file, 0x8c);

		// Fixed for a corrupted area names in the demo data
		_areaMap[2]->_name = "LAPIS LAZULI";
		_areaMap[3]->_name = "EMERALD";
		_areaMap[8]->_name = "TOPAZ";
		file.close();
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
		file.open("DRILLE.EXE");

		if (!file.isOpen())
			error("Failed to open DRILLE.EXE");

		loadMessagesFixedSize(&file, 0x4135, 14, 20);
		loadFonts(&file, 0x99dd);
		loadGlobalObjects(&file, 0x3b42);
		load8bitBinary(&file, 0x9b40, 16);

	} else if (_renderMode == Common::kRenderCGA) {
		loadBundledImages();
		file.open("DRILLC.EXE");

		if (!file.isOpen())
			error("Failed to open DRILLC.EXE");

		loadFonts(&file, 0x07a4a);
		loadMessagesFixedSize(&file, 0x2585, 14, 20);
		load8bitBinary(&file, 0x7bb0, 4);
		loadGlobalObjects(&file, 0x1fa2);
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

void DrillerEngine::processBorder() {
	FreescapeEngine::processBorder();
	if (isDOS() && _renderMode == Common::kRenderCGA) { // Replace some colors for the CGA borders
		uint32 color1 = _border->format.ARGBToColor(0xFF, 0xAA, 0x00, 0xAA);
		uint32 color2 = _border->format.ARGBToColor(0xFF, 0xAA, 0x55, 0x00);

		uint32 colorA = _border->format.ARGBToColor(0xFF, 0x00, 0xAA, 0xAA);
		uint32 colorB = _border->format.ARGBToColor(0xFF, 0x00, 0xAA, 0x00);

		uint32 colorX = _border->format.ARGBToColor(0xFF, 0xAA, 0xAA, 0xAA);
		uint32 colorY = _border->format.ARGBToColor(0xFF, 0xAA, 0x00, 0x00);

		Graphics::Surface *borderRedGreen = new Graphics::Surface();
		borderRedGreen->create(1, 1, _border->format);
		borderRedGreen->copyFrom(*_border);

		for (int i = 0; i < _border->w; i++) {
			for (int j = 0; j < _border->h; j++) {
				if (borderRedGreen->getPixel(i, j) == color1)
					borderRedGreen->setPixel(i, j, color2);
				else if (borderRedGreen->getPixel(i, j) == colorA)
					borderRedGreen->setPixel(i, j, colorB);
				else if (borderRedGreen->getPixel(i, j) == colorX)
					borderRedGreen->setPixel(i, j, colorY);

			}
		}
		Texture *borderTextureRedGreen = _gfx->createTexture(borderRedGreen);

		const CGAPalettteEntry *entry = rawCGAPaletteTable;
		while (entry->areaId) {

			if (entry->palette == kDrillerCGAPaletteRedGreen) {
				_borderCGAByArea[entry->areaId] = borderTextureRedGreen; 
				_paletteCGAByArea[entry->areaId] = (byte *)kDrillerCGAPaletteRedGreenData;
			} else if (entry->palette == kDrillerCGAPalettePinkBlue) {
				_borderCGAByArea[entry->areaId] = _borderTexture; 
				_paletteCGAByArea[entry->areaId] = (byte *)kDrillerCGAPalettePinkBlueData;
			} else
				error("Invalid CGA palette to use");
			entry++;
		}
	}
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
	uint32 color = _renderMode == Common::kRenderCGA ? 1 : 14;
	uint8 r, g, b;

	_gfx->readFromPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = _currentArea->_usualBackgroundColor;
	if (_gfx->_colorRemaps && _gfx->_colorRemaps->contains(color)) {
		color = (*_gfx->_colorRemaps)[color];
	}

	_gfx->readFromPalette(color, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	int score = _gameStateVars[k8bitVariableScore];
	drawStringInSurface(_currentArea->_name, 196, 185, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", 2 * int(_position.x())), 150, 145, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", 2 * int(_position.z())), 150, 153, front, back, surface);
	drawStringInSurface(Common::String::format("%04d", 2 * int(_position.y())), 150, 161, front, back, surface);
	if (_playerHeightNumber >= 0)
		drawStringInSurface(Common::String::format("%d", _playerHeightNumber), 57, 161, front, back, surface);
	else
		drawStringInSurface(Common::String::format("%s", "J"), 57, 161, front, back, surface);

	drawStringInSurface(Common::String::format("%02d", int(_angleRotations[_angleRotationIndex])), 46, 145, front, back, surface);
	drawStringInSurface(Common::String::format("%3d", _playerSteps[_playerStepIndex]), _renderMode == Common::kRenderCGA ? 44 : 46, 153, front, back, surface);
	drawStringInSurface(Common::String::format("%07d", score), 238, 129, front, back, surface);

	int hours = _countdown <= 0 ? 0 : _countdown / 3600;
	drawStringInSurface(Common::String::format("%02d", hours), 208, 8, front, back, surface);
	int minutes = _countdown <= 0 ? 0 : (_countdown - hours * 3600) / 60;
	drawStringInSurface(Common::String::format("%02d", minutes), 230, 8, front, back, surface);
	int seconds = _countdown <= 0 ? 0 : _countdown - hours * 3600 - minutes * 60;
	drawStringInSurface(Common::String::format("%02d", seconds), 254, 8, front, back, surface);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 190, 177, back, front, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else {
		if (_currentArea->_gasPocketRadius == 0)
			message = _messagesList[2];
		else if (_drillStatusByArea[_currentArea->getAreaID()])
			message = _messagesList[0];
		else
			message = _messagesList[1];

		drawStringInSurface(message, 191, 177, front, back, surface);
	}

	int energy = _gameStateVars[k8bitVariableEnergy];
	int shield = _gameStateVars[k8bitVariableShield];

	if (energy >= 0) {
		Common::Rect backBar(20, 185, 88 - energy, 191);
		surface->fillRect(backBar, back);
		Common::Rect energyBar(87 - energy, 185, 88, 191);
		surface->fillRect(energyBar, front);
	}

	if (shield >= 0) {
		Common::Rect backBar(20, 177, 88 - shield, 183);
		surface->fillRect(backBar, back);

		Common::Rect shieldBar(87 - shield, 177, 88, 183);
		surface->fillRect(shieldBar, front);
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
		else if (_drillStatusByArea[_currentArea->getAreaID()])
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
		_drillScoreByArea[_currentArea->getAreaID()] = uint32(maxScore * success) / 100;
		_gameStateVars[k8bitVariableScore] += _drillScoreByArea[_currentArea->getAreaID()];

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

		_gameStateVars[k8bitVariableScore] -= _drillScoreByArea[_currentArea->getAreaID()];
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
		if (_drillStatusByArea[it._key] != kDrillerNoRig)
			removeDrill(it._value);
		_drillStatusByArea[it._key] = kDrillerNoRig;
		if (it._key != 255) {
			_drillMaxScoreByArea[it._key] = (10 + _rnd->getRandomNumber(89)) * 1000;
		}
		_drillScoreByArea[it._key] = 0;
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
		stream->writeUint32LE(_drillStatusByArea[it._key]);
		stream->writeUint32LE(_drillMaxScoreByArea[it._key]);
		stream->writeUint32LE(_drillScoreByArea[it._key]);
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
		_drillScoreByArea[key] = stream->readUint32LE();
	}

	return Common::kNoError;
}

} // End of namespace Freescape
