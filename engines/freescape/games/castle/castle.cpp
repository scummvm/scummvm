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
#include "common/config-manager.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/standard-actions.h"
#include "common/translation.h"

#include "freescape/freescape.h"
#include "freescape/games/castle/castle.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

CastleEngine::CastleEngine(OSystem *syst, const ADGameDescription *gd) : FreescapeEngine(syst, gd) {

	if (!Common::parseBool(ConfMan.get("rock_travel"), _useRockTravel))
		error("Failed to parse bool from rock_travel option");

	if (isSpectrum())
		initZX();

	_playerHeightNumber = 1;
	_playerHeightMaxNumber = 1;

	_playerSteps.clear();
	_playerSteps.push_back(15);
	_playerSteps.push_back(30);
	_playerSteps.push_back(120);
	_playerStepIndex = 2;

	_playerWidth = 8;
	_playerDepth = 8;
	_stepUpDistance = 32;
	_maxFallingDistance = 8192;
	_maxShield = 24;

	_option = nullptr;
	_optionTexture = nullptr;
	_keysFrame = nullptr;
	_spiritsMeterIndicatorFrame = nullptr;
	_strenghtBackgroundFrame = nullptr;
	_strenghtBarFrame = nullptr;
	_menu = nullptr;

	_numberKeys = 0;
	_spiritsDestroyed = 0;
	_spiritsMeter = 32;
	_spiritsToKill = 26;

	_soundIndexStart = 9;
	_soundIndexAreaChange = 5;

}

CastleEngine::~CastleEngine() {
	if (_option) {
		_option->free();
		delete _option;
	}

	for (int i = 0; i < int(_strenghtWeightsFrames.size()); i++) {
		if (_strenghtWeightsFrames[i]) {
			_strenghtWeightsFrames[i]->free();
			delete _strenghtWeightsFrames[i];
		}
	}
}

void CastleEngine::initKeymaps(Common::Keymap *engineKeyMap, Common::Keymap *infoScreenKeyMap, const char *target) {
	FreescapeEngine::initKeymaps(engineKeyMap, infoScreenKeyMap, target);
	Common::Action *act;

	act = new Common::Action("SELECTPRINCE", _("Select Prince"));
	act->setCustomEngineActionEvent(kActionSelectPrince);
	act->addDefaultInputMapping("1");
	infoScreenKeyMap->addAction(act);

	act = new Common::Action("SELECTPRINCESS", _("Select Princess"));
	act->setCustomEngineActionEvent(kActionSelectPrincess);
	act->addDefaultInputMapping("2");
	infoScreenKeyMap->addAction(act);

	act = new Common::Action("SAVE", _("Save Game"));
	act->setCustomEngineActionEvent(kActionSave);
	act->addDefaultInputMapping("s");
	infoScreenKeyMap->addAction(act);

	act = new Common::Action("LOAD", _("Load Game"));
	act->setCustomEngineActionEvent(kActionLoad);
	act->addDefaultInputMapping("l");
	infoScreenKeyMap->addAction(act);

	act = new Common::Action("QUIT", _("Quit Game"));
	act->setCustomEngineActionEvent(kActionEscape);
	if (isDOS() || isCPC())
		act->addDefaultInputMapping("ESCAPE");
	else if (isSpectrum())
		act->addDefaultInputMapping("1");

	infoScreenKeyMap->addAction(act);

	act = new Common::Action("TOGGLESOUND", _("Toggle Sound"));
	act->setCustomEngineActionEvent(kActionToggleSound);
	act->addDefaultInputMapping("t");
	infoScreenKeyMap->addAction(act);

	act = new Common::Action("ROTL", _("Rotate Left"));
	act->setCustomEngineActionEvent(kActionRotateLeft);
	act->addDefaultInputMapping("z");
	engineKeyMap->addAction(act);

	act = new Common::Action("ROTR", _("Rotate Right"));
	act->setCustomEngineActionEvent(kActionRotateRight);
	act->addDefaultInputMapping("x");
	engineKeyMap->addAction(act);

	act = new Common::Action("RUNMODE", _("Run"));
	act->setCustomEngineActionEvent(kActionRunMode);
	act->addDefaultInputMapping("r");
	engineKeyMap->addAction(act);

	act = new Common::Action("WALK", _("Walk"));
	act->setCustomEngineActionEvent(kActionRiseOrFlyUp);
	act->addDefaultInputMapping("JOY_B");
	act->addDefaultInputMapping("w");
	engineKeyMap->addAction(act);

	act = new Common::Action("CRAWL", _("Crawl"));
	act->setCustomEngineActionEvent(kActionLowerOrFlyDown);
	act->addDefaultInputMapping("JOY_Y");
	act->addDefaultInputMapping("c");
	engineKeyMap->addAction(act);

	act = new Common::Action("FACEFRWARD", _("Face Forward"));
	act->setCustomEngineActionEvent(kActionFaceForward);
	act->addDefaultInputMapping("f");
	engineKeyMap->addAction(act);
}

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
		playSound(_soundIndexStart, false);
	} else {
		playSound(_soundIndexAreaChange, false);
	}

	debugC(1, kFreescapeDebugMove, "starting player position: %f, %f, %f", _position.x(), _position.y(), _position.z());
	clearTemporalMessages();
	// Ignore sky/ground fields
	_gfx->_keyColor = 0;
	_gfx->clearColorPairArray();

	if (isDOS()) {
		_gfx->_colorPair[_currentArea->_underFireBackgroundColor] = _currentArea->_extraColor[0];
		_gfx->_colorPair[_currentArea->_usualBackgroundColor] = _currentArea->_extraColor[1];
		_gfx->_colorPair[_currentArea->_paperColor] = _currentArea->_extraColor[2];
		_gfx->_colorPair[_currentArea->_inkColor] = _currentArea->_extraColor[3];
	}

	swapPalette(areaID);
	if (isSpectrum() || isCPC())
		_gfx->_paperColor = 0;
	resetInput();

	if (entranceID > 0) {
		Entrance *entrance = (Entrance *)_currentArea->entranceWithID(entranceID);
		assert(entrance);
		executeEntranceConditions(entrance);
		executeMovementConditions();
	}
}

void CastleEngine::initGameState() {
	FreescapeEngine::initGameState();
	_playerHeightNumber = 1;

	_gameStateVars[k8bitVariableShield] = 16;
	_gameStateVars[k8bitVariableEnergy] = 1;
	_countdown = INT_MAX;
	_numberKeys = 0;
	_spiritsDestroyed = 0;
	_spiritsMeter = 32;
	_spiritsMeterMax = 64;

	if (_useRockTravel) // Enable cheat
		setGameBit(k8bitGameBitTravelRock);
}

bool CastleEngine::checkIfGameEnded() {
	return FreescapeEngine::checkIfGameEnded();
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
	if (keycode == kActionRotateLeft) {
		rotate(-_angleRotations[_angleRotationIndex], 0);
	} else if (keycode == kActionRotateRight) {
		rotate(_angleRotations[_angleRotationIndex], 0);
	} else if (keycode == Common::KEYCODE_s) {
		// TODO: show score
	} else if (keycode == kActionRunMode) {
		if (_playerHeightNumber == 0)
			rise();
		// TODO: raising can fail if there is no room, so the action should fail
		_playerStepIndex = 2;
		insertTemporaryMessage(_messagesList[15], _countdown - 2);
	} else if (keycode == kActionRiseOrFlyUp) {
		if (_playerHeightNumber == 0)
			rise();
		// TODO: raising can fail if there is no room, so the action should fail
		_playerStepIndex = 1;
		insertTemporaryMessage(_messagesList[14], _countdown - 2);
	} else if (keycode == kActionLowerOrFlyDown) {
		if (_playerHeightNumber == 1)
			lower();
		_playerStepIndex = 0;
		insertTemporaryMessage(_messagesList[13], _countdown - 2);
	} else if (keycode == kActionFaceForward) {
		_pitch = 0;
		updateCamera();
	}
}

void CastleEngine::drawInfoMenu() {
	PauseToken pauseToken = pauseEngine();
	if (_savedScreen) {
		_savedScreen->free();
		delete _savedScreen;
	}
	_savedScreen = _gfx->getScreenshot();

	uint8 r, g, b;
	uint32 color = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00);
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(_screenW, _screenH, _gfx->_texturePixelFormat);
	surface->fillRect(_fullscreenViewArea, color);

	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
	uint32 front = 0;
	surface->fillRect(_viewArea, black);

	int score = _gameStateVars[k8bitVariableScore];
	if (isDOS()) {
		g_system->lockMouse(false);
		g_system->showMouse(true);
		surface->copyRectToSurface(*_menu, 40, 33, Common::Rect(0, 0, _menu->w, _menu->h));

		_gfx->readFromPalette(10, r, g, b);
		front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);
		drawStringInSurface(Common::String::format("%07d", score), 166, 71, front, black, surface);
	}

	Texture *menuTexture = _gfx->createTexture(surface);
	Common::Event event;
	bool cont = true;
	while (!shouldQuit() && cont) {
		while (_eventManager->pollEvent(event)) {

			// Events
			switch (event.type) {
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				if (event.customType == kActionLoad) {
					_gfx->setViewport(_fullscreenViewArea);
					_eventManager->purgeKeyboardEvents();
					loadGameDialog();
					if (isDOS()) {
						g_system->lockMouse(false);
						g_system->showMouse(true);
					}

					_gfx->setViewport(_viewArea);
				} else if (event.customType == kActionSave) {
					_gfx->setViewport(_fullscreenViewArea);
					_eventManager->purgeKeyboardEvents();
					saveGameDialog();
					if (isDOS()) {
						g_system->lockMouse(false);
						g_system->showMouse(true);
					}

					_gfx->setViewport(_viewArea);
				} else if (isDOS() && event.customType == kActionToggleSound) {
					// TODO
				} else if ((isDOS() || isCPC() || isSpectrum()) && event.customType == kActionEscape) {
					_forceEndGame = true;
					cont = false;
				} else
					cont = false;
				break;
			case Common::EVENT_KEYDOWN:
					cont = false;
				break;
			case Common::EVENT_SCREEN_CHANGED:
				_gfx->computeScreenViewport();
				// TODO: properly refresh screen
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
		drawFrame();
		if (surface)
			_gfx->drawTexturedRect2D(_fullscreenViewArea, _fullscreenViewArea, menuTexture);

		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(15); // try to target ~60 FPS
	}

	_savedScreen->free();
	delete _savedScreen;
	_savedScreen = nullptr;
	surface->free();
	delete surface;
	delete menuTexture;
	pauseToken.clear();
	g_system->lockMouse(true);
	g_system->showMouse(false);
}

void CastleEngine::executeMakeInvisible(FCLInstruction &instruction) {
	uint16 objectID = 0;
	uint16 areaID = _currentArea->getAreaID();

	if (instruction._destination > 0) {
		objectID = instruction._destination;
		areaID = instruction._source;
	} else {
		objectID = instruction._source;
	}

	debugC(1, kFreescapeDebugCode, "Making obj %d invisible in area %d!", objectID, areaID);
	if (_areaMap.contains(areaID)) {
		Object *obj = _areaMap[areaID]->objectWithID(objectID);
		if (!obj && isCastle())
			return; // No side effects
		assert(obj); // We assume the object was there

		if (!obj->isInvisible() && obj->getType() == kSensorType && isCastle()) {
			_spiritsDestroyed++;
		}

		obj->makeInvisible();
	} else {
		assert(isDOS() && isDemo()); // Should only happen in the DOS demo
	}

}

void CastleEngine::executePrint(FCLInstruction &instruction) {
	uint16 index = instruction._source;
	_currentAreaMessages.clear();
	if (index > 129) {
		index = index - 129;
		if (index < _riddleList.size() / 6)
			drawFullscreenRiddleAndWait(index);
		else
			debugC(1, kFreescapeDebugCode, "Riddle index %d out of bounds", index);
		return;
	}
	debugC(1, kFreescapeDebugCode, "Printing message %d: \"%s\"", index, _messagesList[index].c_str());
	insertTemporaryMessage(_messagesList[index], _countdown - 3);
}


void CastleEngine::loadAssets() {
	FreescapeEngine::loadAssets();
	if (isDOS()) {
		for (auto &it : _areaMap)
			it._value->addStructure(_areaMap[255]);

		_areaMap[1]->addFloor();
		_areaMap[2]->addFloor();

		_menu = loadBundledImage("castle_menu");
		assert(_menu);
		_menu->convertToInPlace(_gfx->_texturePixelFormat);

		_strenghtBackgroundFrame = loadBundledImage("castle_strength_background");
		_strenghtBackgroundFrame->convertToInPlace(_gfx->_texturePixelFormat);

		_strenghtBarFrame = loadBundledImage("castle_strength_bar");
		_strenghtBarFrame->convertToInPlace(_gfx->_texturePixelFormat);

		_strenghtWeightsFrames.push_back(loadBundledImage("castle_strength_weight_0"));
		_strenghtWeightsFrames[0]->convertToInPlace(_gfx->_texturePixelFormat);

		_strenghtWeightsFrames.push_back(loadBundledImage("castle_strength_weight_1"));
		_strenghtWeightsFrames[1]->convertToInPlace(_gfx->_texturePixelFormat);

		_strenghtWeightsFrames.push_back(loadBundledImage("castle_strength_weight_2"));
		_strenghtWeightsFrames[2]->convertToInPlace(_gfx->_texturePixelFormat);

		_strenghtWeightsFrames.push_back(loadBundledImage("castle_strength_weight_3"));
		_strenghtWeightsFrames[3]->convertToInPlace(_gfx->_texturePixelFormat);
	}
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

			Common::String message = "";
			if (size == 255) {
				size = 19;
				while (size-- > 0)
					message = message + "*";

				_riddleList.push_back(message);
				debugC(1, kFreescapeDebugParser, "extra byte: %x", file->readByte());
				debugC(1, kFreescapeDebugParser, "extra byte: %x", file->readByte());
				debugC(1, kFreescapeDebugParser, "'%s'", message.c_str());
				continue;
			}

			//if (size > 22)
			//	size = 22;
			int padSpaces = (22 - size) / 2;
			debugC(1, kFreescapeDebugParser, "extra byte: %x", file->readByte());

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
	debugC(1, kFreescapeDebugParser, "End of riddles at %" PRIx64, file->pos());
}

void CastleEngine::drawFullscreenRiddleAndWait(uint16 riddle) {
	_savedScreen = _gfx->getScreenshot();
	int frontColor = 6;
	int backColor = 0;
	switch (_renderMode) {
		case Common::kRenderCPC:
			backColor = 14;
			break;
		case Common::kRenderCGA:
			backColor = 1;
			break;
		case Common::kRenderZX:
			backColor = 0;
			frontColor = 7;
			break;
		default:
			backColor = 14;
	}
	uint8 r, g, b;
	_gfx->readFromPalette(frontColor, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);
	_gfx->readFromPalette(backColor, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(_screenW, _screenH, _gfx->_texturePixelFormat);

	Common::Event event;
	bool cont = true;
	while (!shouldQuit() && cont) {
		while (_eventManager->pollEvent(event)) {

			// Events
			switch (event.type) {
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				if (event.customType == kActionSkip) {
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
	uint32 grey = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x60, 0x60, 0x60);
	uint32 frame = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xA7, 0xA7, 0xA7);

	Common::Rect outerFrame(47, 47, 271, 147);
	Common::Rect innerFrame(53, 53, 266, 141);

	if (isDOS()) {
		black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
		grey = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x60, 0x60, 0x60);
		frame = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xA7, 0xA7, 0xA7);
	} else {
		outerFrame = Common::Rect(67, 47, 251, 143 - 5);
		innerFrame = Common::Rect(70, 49, 249, 141 - 5);
		grey = noColor;
		frame = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xD8, 0xD8, 0xD8);
	}

	surface->fillRect(_fullscreenViewArea, noColor);
	surface->fillRect(_viewArea, black);

	surface->fillRect(outerFrame, grey);
	surface->frameRect(outerFrame, frame);
	surface->frameRect(innerFrame, frame);

	surface->fillRect(Common::Rect(54, 54, 265, 140), back);
	int x = 0;
	int y = 0;
	int numberOfLines = 6;

	if (isDOS()) {
		x = 60;
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

void CastleEngine::drawStringInSurface(const Common::String &str, int x, int y, uint32 fontColor, uint32 backColor, Graphics::Surface *surface, int offset) {
	if (isSpectrum() || isCPC()) {
		FreescapeEngine::drawStringInSurface(str, x, y, fontColor, backColor, surface, offset);
		return;
	}

	uint32 transparent = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00);
	uint32 yellow = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xFF, 0xFF, 0x00);
	//uint32 green = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x80, 0x00);

	_font = _fontPlane1;
	FreescapeEngine::drawStringInSurface(str, x, y, fontColor, backColor, surface, offset);

	_font = _fontPlane2;
	FreescapeEngine::drawStringInSurface(str, x, y, yellow, transparent, surface, offset);

	//_font = _fontPlane3;
	//FreescapeEngine::drawStringInSurface(str, x, y, transparent, green, surface, offset);
}

void CastleEngine::drawEnergyMeter(Graphics::Surface *surface) {
	Common::Point origin;

	if (isDOS())
		origin = Common::Point(43, 157);
	if (isSpectrum())
		origin = Common::Point(63, 154);

	if (!_strenghtBackgroundFrame)
		return;

	surface->copyRectToSurface((const Graphics::Surface)*_strenghtBackgroundFrame, origin.x, origin.y, Common::Rect(0, 0, _strenghtBackgroundFrame->w, _strenghtBackgroundFrame->h));
	surface->copyRectToSurface((const Graphics::Surface)*_strenghtBarFrame, origin.x, origin.y + 8, Common::Rect(0, 0, _strenghtBarFrame->w, _strenghtBarFrame->h));

	Common::Point weightPoint;
	int frameIdx = -1;

	weightPoint = Common::Point(origin.x + 5, origin.y);
	frameIdx = 3 - _gameStateVars[k8bitVariableShield] % 4;
	frameIdx++;
	frameIdx = frameIdx % 4;

	surface->copyRectToSurface((const Graphics::Surface)*_strenghtWeightsFrames[frameIdx], weightPoint.x, weightPoint.y, Common::Rect(0, 0, 3, _strenghtWeightsFrames[frameIdx]->h));
	weightPoint += Common::Point(3, 0);

	for (int i = 0; i < _gameStateVars[k8bitVariableShield] / 4 - 1; i++) {
		surface->copyRectToSurface((const Graphics::Surface)*_strenghtWeightsFrames[0], weightPoint.x, weightPoint.y, Common::Rect(0, 0, 3, _strenghtWeightsFrames[0]->h));
		weightPoint += Common::Point(3, 0);
	}

	weightPoint = Common::Point(origin.x + 62, origin.y);
	frameIdx = 3 - _gameStateVars[k8bitVariableShield] % 4;
	frameIdx++;
	frameIdx = frameIdx % 4;

	surface->copyRectToSurface((const Graphics::Surface)*_strenghtWeightsFrames[frameIdx], weightPoint.x, weightPoint.y, Common::Rect(0, 0, 3, _strenghtWeightsFrames[frameIdx]->h));
	weightPoint += Common::Point(-3, 0);

	for (int i = 0; i < _gameStateVars[k8bitVariableShield] / 4 - 1; i++) {
		surface->copyRectToSurface((const Graphics::Surface)*_strenghtWeightsFrames[0], weightPoint.x, weightPoint.y, Common::Rect(0, 0, 3, _strenghtWeightsFrames[0]->h));
		weightPoint += Common::Point(-3, 0);
	}
}

void CastleEngine::addGhosts() {
	for (auto &it : _areaMap) {
		for (auto &sensor : it._value->getSensors()) {
			if (sensor->getObjectID() == 125) {
				if (isDOS()) {
					_areaMap[it._key]->addGroupFromArea(195, _areaMap[255]);
					_areaMap[it._key]->addGroupFromArea(212, _areaMap[255]);
				} else if (isSpectrum()) {
					_areaMap[it._key]->addObjectFromArea(170, _areaMap[255]);
					_areaMap[it._key]->addObjectFromArea(172, _areaMap[255]);
					_areaMap[it._key]->addObjectFromArea(173, _areaMap[255]);
				}
			} else if (sensor->getObjectID() == 126) {
				if (isDOS())
					_areaMap[it._key]->addGroupFromArea(191, _areaMap[255]);
				else if (isSpectrum()) {
					_areaMap[it._key]->addObjectFromArea(145, _areaMap[255]);
					_areaMap[it._key]->addObjectFromArea(165, _areaMap[255]);
					_areaMap[it._key]->addObjectFromArea(166, _areaMap[255]);
				}
			} else if (sensor->getObjectID() == 127) {
				if (isDOS())
					_areaMap[it._key]->addGroupFromArea(182, _areaMap[255]);
				else if (isSpectrum()) {
					_areaMap[it._key]->addObjectFromArea(142, _areaMap[255]);
					_areaMap[it._key]->addObjectFromArea(143, _areaMap[255]);
					_areaMap[it._key]->addObjectFromArea(144, _areaMap[255]);
				}
			} else
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
	if (isDOS()) { // Should be similar to Amiga/AtariST
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
	}

	/*int firingInterval = 10; // This is fixed for all the ghosts?
	if (_ticks % firingInterval == 0) {
		if (_underFireFrames <= 0)
			_underFireFrames = 4;
		takeDamageFromSensor();
	}*/
}

void CastleEngine::updateTimeVariables() {
	if (_gameStateControl != kFreescapeGameStatePlaying)
		return;
	// This function only executes "on collision" room/global conditions

	if (_gameStateVars[32] > 0) { // Key collected!
		setGameBit(_gameStateVars[32]);
		_gameStateVars[32] = 0;
		_numberKeys++;
	}

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	if (_lastMinute != minutes / 2) {
		_lastMinute = minutes / 2;
		_spiritsMeter++;
		_spiritsMeterPosition = _spiritsMeter * (_spiritsToKill - _spiritsDestroyed) / _spiritsToKill;
		if (_spiritsMeterPosition >= _spiritsMeterMax)
			_countdown = -1;
	}
}

void CastleEngine::borderScreen() {
	FreescapeEngine::borderScreen();
	selectCharacterScreen();
}

void CastleEngine::drawOption() {
	_gfx->setViewport(_fullscreenViewArea);
	if (_option) {
		if (!_optionTexture) {
			Graphics::Surface *title = _gfx->convertImageFormatIfNecessary(_option);
			_optionTexture = _gfx->createTexture(title);
			title->free();
			delete title;
		}
		_gfx->drawTexturedRect2D(_fullscreenViewArea, _fullscreenViewArea, _optionTexture);
	}
	_gfx->setViewport(_viewArea);
}

extern Common::String centerAndPadString(const Common::String &x, int y);

void CastleEngine::selectCharacterScreen() {
	Common::Array<Common::String> lines;
	switch (_language) {
		case Common::ES_ESP:
			// No accent in "príncipe" since it is not supported by the font
			if (isDOS()) {
				lines.push_back("Elija su personaje");
				lines.push_back("");
				lines.push_back("");
				lines.push_back("            1. Principe");
				lines.push_back("            2. Princesa");
			} else if (isSpectrum()) {
				lines.push_back(centerAndPadString("*******************", 21));
				lines.push_back(centerAndPadString("Seleccion el ", 21));
				lines.push_back(centerAndPadString("personaje que quiera", 21));
				lines.push_back(centerAndPadString("ser y precione enter", 21));
				lines.push_back("");
				lines.push_back(centerAndPadString("1. Principe", 21));
				lines.push_back(centerAndPadString("2. Princesa", 21));
				lines.push_back("");
				lines.push_back(centerAndPadString("*******************", 21));
			}
			break;
		default: //case Common::EN_ANY:
			if (isDOS()) {
				lines.push_back("Select your character");
				lines.push_back("");
				lines.push_back("");
				lines.push_back("            1. Prince");
				lines.push_back("            2. Princess");
			} else if (isSpectrum()) {
				lines.push_back(centerAndPadString("*******************", 21));
				lines.push_back(centerAndPadString("Select your character", 21));
				lines.push_back(centerAndPadString("you wish to play", 21));
				lines.push_back(centerAndPadString("and press enter", 21));
				lines.push_back("");
				lines.push_back(centerAndPadString("1. Prince  ", 21));
				lines.push_back(centerAndPadString("2. Princess", 21));
				lines.push_back("");
				lines.push_back(centerAndPadString("*******************", 21));
			}
			break;
	}

	Graphics::Surface *surface = drawStringsInSurface(lines);
	_system->lockMouse(false);
	_system->showMouse(true);
	Common::Rect princeSelector(82, 100, 163, 109);
	Common::Rect princessSelector(82, 110, 181, 120);

	bool selected = false;
	while (!selected) {
		Common::Event event;
		Common::Point mouse;
		while (_eventManager->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				quitGame();
				return;

			// Left mouse click
			case Common::EVENT_LBUTTONDOWN:
				// fallthrough
			case Common::EVENT_RBUTTONDOWN:
				mouse.x = _screenW * event.mouse.x / g_system->getWidth();
				mouse.y = _screenH * event.mouse.y / g_system->getHeight();

				if (princeSelector.contains(mouse)) {
					selected = true;
					// Nothing, since game bit should be already zero
				} else if (princessSelector.contains(mouse)) {
					selected = true;
					setGameBit(32);
				}
				break;
			case Common::EVENT_SCREEN_CHANGED:
				_gfx->computeScreenViewport();
				_gfx->clear(0, 0, 0, true);
				break;
			default:
				break;
			}
			switch (event.customType) {
				case kActionSelectPrince:
					selected = true;
					// Nothing, since game bit should be already zero
					break;
				case kActionSelectPrincess:
					selected = true;
					setGameBit(32);
					break;
				default:
					break;
			}
		}
		_gfx->clear(0, 0, 0, true);
		if (_option)
			drawOption();
		else
			drawBorder();
		drawFullscreenSurface(surface);
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(15); // try to target ~60 FPS
	}
	_system->lockMouse(true);
	_system->showMouse(false);
	_gfx->clear(0, 0, 0, true);

}

Common::Error CastleEngine::saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave) {
	stream->writeUint32LE(_numberKeys);
	stream->writeUint32LE(_spiritsMeter);
	stream->writeUint32LE(_spiritsDestroyed);
	return Common::kNoError;
}

Common::Error CastleEngine::loadGameStreamExtended(Common::SeekableReadStream *stream) {
	_numberKeys = stream->readUint32LE();
	_spiritsMeter = stream->readUint32LE();
	_spiritsDestroyed = stream->readUint32LE();

	if (_useRockTravel) // Enable cheat
		setGameBit(k8bitGameBitTravelRock);
	return Common::kNoError;
}

} // End of namespace Freescape
