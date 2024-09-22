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

	_angleRotations.push_back(5);

	_playerWidth = 8;
	_playerDepth = 8;
	_stepUpDistance = 32;
	_maxFallingDistance = 8192;
	_maxShield = 24;

	_option = nullptr;
	_optionTexture = nullptr;
	_spiritsMeterIndicatorFrame = nullptr;
	_spiritsMeterIndicatorBackgroundFrame = nullptr;
	_strenghtBackgroundFrame = nullptr;
	_strenghtBarFrame = nullptr;
	_thunderFrame = nullptr;
	_menu = nullptr;
	_menuButtons = nullptr;

	_riddleTopFrame = nullptr;
	_riddleBottomFrame = nullptr;
	_riddleBackgroundFrame = nullptr;

	_endGameThroneFrame = nullptr;
	_endGameBackgroundFrame = nullptr;

	_menuCrawlIndicator = nullptr;
	_menuWalkIndicator = nullptr;
	_menuRunIndicator = nullptr;
	_menuFxOnIndicator = nullptr;
	_menuFxOffIndicator = nullptr;

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

	for (int i = 0; i < int(_keysBorderFrames.size()); i++) {
		if (_keysBorderFrames[i]) {
			_keysBorderFrames[i]->free();
			delete _keysBorderFrames[i];
		}
	}

	for (int i = 0; i < int(_keysMenuFrames.size()); i++) {
		if (_keysMenuFrames[i]) {
			_keysMenuFrames[i]->free();
			delete _keysMenuFrames[i];
		}
	}

	if (_spiritsMeterIndicatorBackgroundFrame) {
		_spiritsMeterIndicatorBackgroundFrame->free();
		delete _spiritsMeterIndicatorBackgroundFrame;
	}

	if (_spiritsMeterIndicatorFrame) {
		_spiritsMeterIndicatorFrame->free();
		delete _spiritsMeterIndicatorFrame;
	}

	if (_strenghtBackgroundFrame) {
		_strenghtBackgroundFrame->free();
		delete _strenghtBackgroundFrame;
	}

	if (_strenghtBarFrame) {
		_strenghtBarFrame->free();
		delete _strenghtBarFrame;
	}

	for (int i = 0; i < int(_strenghtWeightsFrames.size()); i++) {
		if (_strenghtWeightsFrames[i]) {
			_strenghtWeightsFrames[i]->free();
			delete _strenghtWeightsFrames[i];
		}
	}

	for (int i = 0; i < int(_flagFrames.size()); i++) {
		if (_flagFrames[i]) {
			_flagFrames[i]->free();
			delete _flagFrames[i];
		}
	}

	if (_thunderFrame) {
		_thunderFrame->free();
		delete _thunderFrame;
	}

	if (_riddleTopFrame) {
		_riddleTopFrame->free();
		delete _riddleTopFrame;
	}

	if (_riddleBackgroundFrame) {
		_riddleBackgroundFrame->free();
		delete _riddleBackgroundFrame;
	}

	if (_riddleBottomFrame) {
		_riddleBottomFrame->free();
		delete _riddleBottomFrame;
	}

	if (_endGameThroneFrame) {
		_endGameThroneFrame->free();
		delete _endGameThroneFrame;
	}

	if (_endGameBackgroundFrame) {
		_endGameBackgroundFrame->free();
		delete _endGameBackgroundFrame;
	}

	if (_menu) {
		_menu->free();
		delete _menu;
	}

	if (_menuButtons) {
		_menuButtons->free();
		delete _menuButtons;
	}

	if (_menuCrawlIndicator) {
		_menuCrawlIndicator->free();
		delete _menuCrawlIndicator;
	}

	if (_menuWalkIndicator) {
		_menuWalkIndicator->free();
		delete _menuWalkIndicator;
	}

	if (_menuRunIndicator) {
		_menuRunIndicator->free();
		delete _menuRunIndicator;
	}

	if (_menuFxOnIndicator) {
		_menuFxOnIndicator->free();
		delete _menuFxOnIndicator;
	}

	if (_menuFxOffIndicator) {
		_menuFxOffIndicator->free();
		delete _menuFxOffIndicator;
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

	if (!_exploredAreas.contains(areaID)) {
		_gameStateVars[k8bitVariableScore] += 17500;
		_exploredAreas[areaID] = true;
	}

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

	if (isDOS() || isAmiga()) {
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

	_exploredAreas[_startArea] = true;
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

extern Common::String centerAndPadString(const Common::String &x, int y);

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
		surface->copyRectToSurface(*_menu, 47, 35, Common::Rect(0, 0, _menu->w, _menu->h));

		_gfx->readFromPalette(10, r, g, b);
		front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);
		drawStringInSurface(Common::String::format("%07d", score), 166, 71, front, black, surface);
	} else if (isSpectrum()) {
		Common::Array<Common::String> lines;
		lines.push_back(centerAndPadString("********************", 21));
		lines.push_back(centerAndPadString("s-save l-load q-quit", 21));
		lines.push_back("");
		lines.push_back(centerAndPadString(Common::String::format("keys   %d collected", _numberKeys), 21));
		lines.push_back(centerAndPadString(Common::String::format("spirits  %d destroyed", _spiritsDestroyed), 21));
		lines.push_back(centerAndPadString("strength  strong", 21));
		lines.push_back(centerAndPadString(Common::String::format("score   %07d", score), 21));
		lines.push_back("");
		lines.push_back(centerAndPadString("********************", 21));
		surface = drawStringsInSurface(lines, surface);
	}

	Texture *menuTexture = _gfx->createTexture(surface);
	Common::Event event;
	Common::Point mousePos;
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
					if (isDOS() || isAmiga() || isAtariST()) {
						g_system->lockMouse(false);
						g_system->showMouse(true);
					}

					_gfx->setViewport(_viewArea);
				} else if (event.customType == kActionSave) {
					_gfx->setViewport(_fullscreenViewArea);
					_eventManager->purgeKeyboardEvents();

					saveGameDialog();
					if (isDOS() || isAmiga() || isAtariST()) {
						g_system->lockMouse(false);
						g_system->showMouse(true);
					}

					_gfx->setViewport(_viewArea);
				} else if (isDOS() && event.customType == kActionToggleSound) {
					// TODO
				} else if ((isCPC() || isSpectrum()) && event.customType == kActionEscape) {
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
				if (isSpectrum() || isCPC())
					break;

				mousePos = getNormalizedPosition(event.mouse);
				if (Common::Rect(101, 67, 133, 79).contains(mousePos)) {
					_gfx->setViewport(_fullscreenViewArea);
					_eventManager->purgeKeyboardEvents();
					loadGameDialog();
					g_system->lockMouse(false);
					g_system->showMouse(true);

					_gfx->setViewport(_viewArea);
				} else if (Common::Rect(101, 82, 133, 95).contains(mousePos)) {
					_gfx->setViewport(_fullscreenViewArea);
					_eventManager->purgeKeyboardEvents();
					saveGameDialog();
					g_system->lockMouse(false);
					g_system->showMouse(true);

					_gfx->setViewport(_viewArea);
				} else if (Common::Rect(101, 101, 133, 114).contains(mousePos)) {
					// Toggle sounds
				} else if (Common::Rect(101, 116, 133, 129).contains(mousePos)) {
					// Cycle between crawl, walk or run
					// It can fail if there is no room
				} else if (Common::Rect(101, 131, 133, 144).contains(mousePos))
					cont = false; // Back to game
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

// Same as FreescapeEngine::executeExecute but updates the spirits destroyed counter
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

// Same as FreescapeEngine::executeExecute but updates the spirits destroyed counter
void CastleEngine::executeDestroy(FCLInstruction &instruction) {
	uint16 objectID = 0;
	uint16 areaID = _currentArea->getAreaID();

	if (instruction._destination > 0) {
		objectID = instruction._destination;
		areaID = instruction._source;
	} else {
		objectID = instruction._source;
	}

	debugC(1, kFreescapeDebugCode, "Destroying obj %d in area %d!", objectID, areaID);
	assert(_areaMap.contains(areaID));
	Object *obj = _areaMap[areaID]->objectWithID(objectID);
	assert(obj); // We know that an object should be there

	if (!obj->isDestroyed() && obj->getType() == kSensorType && isCastle()) {
		_spiritsDestroyed++;
	}

	if (obj->isDestroyed())
		debugC(1, kFreescapeDebugCode, "WARNING: Destroying obj %d in area %d already destroyed!", objectID, areaID);

	obj->destroy();
}

void CastleEngine::executePrint(FCLInstruction &instruction) {
	uint16 index = instruction._source;
	_currentAreaMessages.clear();
	if (index >= 129) {
		index = index - 129;
		drawFullscreenRiddleAndWait(index);
		return;
	}
	debugC(1, kFreescapeDebugCode, "Printing message %d: \"%s\"", index, _messagesList[index].c_str());
	insertTemporaryMessage(_messagesList[index], _countdown - 3);
}

void CastleEngine::executeRedraw(FCLInstruction &instruction) {
	FreescapeEngine::executeRedraw(instruction);
	tryToCollectKey();
}

void CastleEngine::loadAssets() {
	FreescapeEngine::loadAssets();
	if (isDOS()) {
		for (auto &it : _areaMap) {
			it._value->addStructure(_areaMap[255]);
			it._value->addObjectFromArea(229, _areaMap[255]);
			it._value->addObjectFromArea(242, _areaMap[255]);
		}

		_areaMap[1]->addFloor();
		_areaMap[2]->addFloor();
	}
}

void CastleEngine::loadRiddles(Common::SeekableReadStream *file, int offset, int number) {
	file->seek(offset);
	debugC(1, kFreescapeDebugParser, "Riddle table:");
	int maxLineSize = isSpectrum() ? 20 : 23;

	for (int i = 0; i < number; i++) {
		int header = file->readByte();
		debugC(1, kFreescapeDebugParser, "riddle %d header: %x", i, header);
		int numberLines = 6;
		if (header == 0x18)
			numberLines = 8;
		else if (header == 0x15 || header == 0x1a || header == 0x1b || header == 0x1c || header == 0x1e)
			numberLines = 7;
		else if (header == 0x1d)
			numberLines = 6;
		else if (header == 0x27)
			numberLines = 5;

		if (isSpectrum())
			--numberLines;

		for (int j = 0; j < numberLines; j++) {
			int size = file->readByte();
			debugC(1, kFreescapeDebugParser, "size: %d (max %d?)", size, maxLineSize);

			Common::String message = "";
			if (size == 255) {
				size = 19;
				while (size-- > 0)
					message = message + "*";

				//debugC(1, kFreescapeDebugParser, "extra byte: %x", file->readByte());
				debugC(1, kFreescapeDebugParser, "extra byte: %x", file->readByte());
				debugC(1, kFreescapeDebugParser, "extra byte: %x", file->readByte());
				debugC(1, kFreescapeDebugParser, "'%s'", message.c_str());
				_riddleList.push_back(message);
				continue;
			} else if (size > maxLineSize) {
				for (int k = j; k < numberLines; k++)
					_riddleList.push_back(message);

				if (isSpectrum()) {
					debugC(1, kFreescapeDebugParser, "extra byte: %x", file->readByte());
					debugC(1, kFreescapeDebugParser, "extra byte: %x", file->readByte());
				}

				debugC(1, kFreescapeDebugParser, "'%s'", message.c_str());
				break;
			} else if (size == 0) {
				size = 20;
			}

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
		}

		if (numberLines < 7)
			for (int j = numberLines; j < 7; j++) {
				_riddleList.push_back("");
				debugC(1, kFreescapeDebugParser, "Padded with ''");
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
	for (int i = 7 * riddle; i < 7 * (riddle + 1); i++) {
		riddleMessages.push_back(_riddleList[i]);
	}
	uint32 frameColor = 0;
	if (isDOS()) {
		int w = 34;
		surface->copyRectToSurface((const Graphics::Surface)*_riddleTopFrame, 40, w, Common::Rect(0, 0, _riddleTopFrame->w, _riddleTopFrame->h));
		for (w += _riddleTopFrame->h; w < 136;) {
			surface->copyRectToSurface((const Graphics::Surface)*_riddleBackgroundFrame, 40, w, Common::Rect(0, 0, _riddleBackgroundFrame->w, _riddleBackgroundFrame->h));
			w += _riddleBackgroundFrame->h;
		}
		surface->copyRectToSurface((const Graphics::Surface)*_riddleBottomFrame, 40, 136, Common::Rect(0, 0, _riddleBottomFrame->w, _riddleBottomFrame->h - 1));
	} else {
		frameColor = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0xD8, 0xD8, 0xD8);
		surface->fillRect(_viewArea, frameColor);
	}

	int x = 0;
	int y = 0;
	int numberOfLines = 7;

	if (isDOS()) {
		x = 60;
		y = 62;
	} else if (isSpectrum() || isCPC()) {
		x = 60;
		y = 40;
	}

	for (int i = 0; i < numberOfLines; i++) {
		drawStringInSurface(riddleMessages[i], x, y, front, back, surface);
		y = y + 10;
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

	_font = Common::BitArray();
	//_font = _fontPlane3;
	//FreescapeEngine::drawStringInSurface(str, x, y, transparent, green, surface, offset);
}

void CastleEngine::drawEnergyMeter(Graphics::Surface *surface, Common::Point origin) {
	if (!_strenghtBackgroundFrame)
		return;


	surface->copyRectToSurface((const Graphics::Surface)*_strenghtBackgroundFrame, origin.x, origin.y, Common::Rect(0, 0, _strenghtBackgroundFrame->w, _strenghtBackgroundFrame->h));
	if (!_strenghtBarFrame)
		return;

	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
	surface->copyRectToSurfaceWithKey((const Graphics::Surface)*_strenghtBarFrame, origin.x + 5, origin.y + 8, Common::Rect(0, 0, _strenghtBarFrame->w, _strenghtBarFrame->h), black);

	Common::Point weightPoint;
	int frameIdx = -1;

	weightPoint = Common::Point(origin.x + 10, origin.y);
	frameIdx = 3 - _gameStateVars[k8bitVariableShield] % 4;
	frameIdx++;
	frameIdx = frameIdx % 4;

	if (_strenghtWeightsFrames.empty())
		return;

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

void CastleEngine::tryToCollectKey() {
	if (_gameStateVars[32] > 0) { // Key collected!
		setGameBit(_gameStateVars[32]);
		_gameStateVars[32] = 0;
		_numberKeys++;
	}
}

void CastleEngine::updateTimeVariables() {
	if (_gameStateControl != kFreescapeGameStatePlaying)
		return;
	// This function only executes "on collision" room/global conditions
	tryToCollectKey();

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
	if (isAmiga() && isDemo()) {
		// Skip character selection
	} else
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

void CastleEngine::selectCharacterScreen() {
	Common::Array<Common::String> lines;
	uint32 color = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00);
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(_screenW, _screenH, _gfx->_texturePixelFormat);
	surface->fillRect(_fullscreenViewArea, color);

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

	drawStringsInSurface(lines, surface);
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

	for (auto &it : _areaMap) {
		stream->writeUint16LE(it._key);
		stream->writeUint32LE(_exploredAreas[it._key]);
	}

	return Common::kNoError;
}

Common::Error CastleEngine::loadGameStreamExtended(Common::SeekableReadStream *stream) {
	_numberKeys = stream->readUint32LE();
	_spiritsMeter = stream->readUint32LE();
	_spiritsDestroyed = stream->readUint32LE();

	for (uint i = 0; i < _areaMap.size(); i++) {
		uint16 key = stream->readUint16LE();
		_exploredAreas[key] = stream->readUint32LE();
	}

	if (_useRockTravel) // Enable cheat
		setGameBit(k8bitGameBitTravelRock);
	return Common::kNoError;
}

} // End of namespace Freescape
