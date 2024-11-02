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
#include "common/random.h"

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

	_soundIndexStart = 9;
	_soundIndexAreaChange = 5;
	k8bitVariableShield = 29;

	if (isDOS())
		initDOS();
	else if (isSpectrum())
		initZX();

	_playerHeightNumber = 1;
	_playerHeightMaxNumber = 1;
	_lastTenSeconds = -1;

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
	_spiritsMeterIndicatorSideFrame = nullptr;
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
	_gameOverBackgroundFrame = nullptr;

	_menuCrawlIndicator = nullptr;
	_menuWalkIndicator = nullptr;
	_menuRunIndicator = nullptr;
	_menuFxOnIndicator = nullptr;
	_menuFxOffIndicator = nullptr;

	_spiritsMeter = 32;
	_spiritsToKill = 26;
	_spiritsMeterPosition = 0;
	_spiritsMeterMax = 64;
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

	if (_spiritsMeterIndicatorSideFrame) {
		_spiritsMeterIndicatorSideFrame->free();
		delete _spiritsMeterIndicatorSideFrame;
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

	if (_gameOverBackgroundFrame) {
		_gameOverBackgroundFrame->free();
		delete _gameOverBackgroundFrame;
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
	if (_language == Common::ES_ESP)
		act->addDefaultInputMapping("c");
	else
		act->addDefaultInputMapping("l");
	infoScreenKeyMap->addAction(act);

	act = new Common::Action("QUIT", _("Quit Game"));
	act->setCustomEngineActionEvent(kActionEscape);
	if (isDOS() || isCPC())
		act->addDefaultInputMapping("ESCAPE");
	else if (isSpectrum())
		act->addDefaultInputMapping("q");

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
	act->setCustomEngineActionEvent(kActionWalkMode);
	act->addDefaultInputMapping("JOY_B");
	act->addDefaultInputMapping("w");
	engineKeyMap->addAction(act);

	act = new Common::Action("CRAWL", _("Crawl"));
	act->setCustomEngineActionEvent(kActionCrawlMode);
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

	if (!_areaMap.contains(areaID) && isDemo())
		return; // Abort area change if the destination does not exist (demo only)

	if (!_exploredAreas.contains(areaID)) {
		_gameStateVars[k8bitVariableScore] += 17500;
		_exploredAreas[areaID] = true;
	}

	assert(_areaMap.contains(areaID));
	_currentArea = _areaMap[areaID];
	_currentArea->show();

	if (entranceID > 0)
		traverseEntrance(entranceID);

	_position = _currentArea->separateFromWall(_position);
	_lastPosition = _position;

	if (_currentArea->_skyColor > 0 && _currentArea->_skyColor != 255) {
		_gfx->_keyColor = 0;
	} else
		_gfx->_keyColor = 255;

	_lastPosition = _position;
	_gameStateVars[0x1f] = 0;

	if (areaID == _startArea && entranceID == _startEntrance) {
		if (getGameBit(31))
			playSound(13, true);
		else
			playSound(_soundIndexStart, false);
	} else if (areaID == _endArea && entranceID == _endEntrance) {
		_pitch = -85;
	} else {
		// If escaped, play a different sound
		if (getGameBit(31))
			playSound(13, true);
		else
			playSound(_soundIndexAreaChange, true);
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

	/*if (entranceID > 0) {
		Entrance *entrance = (Entrance *)_currentArea->entranceWithID(entranceID);
		assert(entrance);
		executeEntranceConditions(entrance);
		executeMovementConditions();
	}*/
}

void CastleEngine::initGameState() {
	FreescapeEngine::initGameState();
	_playerHeightNumber = 1;

	_gameStateVars[k8bitVariableShield] = 16;
	_gameStateVars[k8bitVariableEnergy] = 1;
	_gameStateVars[8] = 128; // -1
	_countdown = INT_MAX - 8;
	_keysCollected.clear();
	_spiritsMeter = 32;

	_exploredAreas[_startArea] = true;
	if (_useRockTravel) // Enable cheat
		setGameBit(k8bitGameBitTravelRock);

	_gfx->_shakeOffset = Common::Point();

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	_lastMinute = minutes;
	_lastTenSeconds = seconds / 10;
}

bool CastleEngine::checkIfGameEnded() {
	if (_gameStateControl != kFreescapeGameStatePlaying)
		return false;

	if (getGameBit(31) || _currentArea->getAreaID() == 74) { // Escaped!
		_gameStateControl = kFreescapeGameStateEnd;
		return true;
	} else
		return FreescapeEngine::checkIfGameEnded();
}

void CastleEngine::endGame() {
	_shootingFrames = 0;
	_delayedShootObject = nullptr;
	_endGamePlayerEndArea = true;

	if (getGameBit(31) || _currentArea->getAreaID() == 74) {
		insertTemporaryMessage(_messagesList[5], INT_MIN);

		if (isDOS()) {
			drawFullscreenEndGameAndWait();
		}
	} else {
		drawFullscreenGameOverAndWait();
	}

	_gameStateControl = kFreescapeGameStateRestart;
	_endGameKeyPressed = false;
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
		if (_playerHeightNumber == 0) {
			if (_gameStateVars[k8bitVariableShield] <= 3) {
				insertTemporaryMessage(_messagesList[12], _countdown - 2);
				return;
			}

			if (!rise()) {
				_playerStepIndex = 0;
				insertTemporaryMessage(_messagesList[11], _countdown - 2);
				return;
			}
			_gameStateVars[k8bitVariableCrawling] = 0;
		}
		// TODO: raising can fail if there is no room, so the action should fail
		_playerStepIndex = 2;
		insertTemporaryMessage(_messagesList[15], _countdown - 2);
	} else if (keycode == kActionWalkMode) {
		if (_playerHeightNumber == 0) {
			if (_gameStateVars[k8bitVariableShield] <= 3) {
				insertTemporaryMessage(_messagesList[12], _countdown - 2);
				return;
			}

			if (!rise()) {
				_playerStepIndex = 0;
				insertTemporaryMessage(_messagesList[11], _countdown - 2);
				return;
			}
			_gameStateVars[k8bitVariableCrawling] = 0;
		}

		// TODO: raising can fail if there is no room, so the action should fail
		_playerStepIndex = 1;
		insertTemporaryMessage(_messagesList[14], _countdown - 2);
	} else if (keycode == kActionCrawlMode) {
		if (_playerHeightNumber == 1) {
			lower();
			_gameStateVars[k8bitVariableCrawling] = 128;
		}
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
	int shield = _gameStateVars[k8bitVariableShield];
	int spiritsDestroyed = _gameStateVars[k8bitVariableSpiritsDestroyed];
	Common::Array<Common::Rect> keyRects;

	if (isDOS()) {
		g_system->lockMouse(false);
		g_system->showMouse(true);
		surface->copyRectToSurface(*_menu, 47, 35, Common::Rect(0, 0, _menu->w, _menu->h));

		_gfx->readFromPalette(10, r, g, b);
		front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);
		drawStringInSurface(Common::String::format("%07d", score), 166, 71, front, black, surface);
		drawStringInSurface(centerAndPadString(Common::String::format("%s", _messagesList[135 + shield / 6].c_str()), 10), 151, 102,  front, black, surface);

		Common::String keysCollected = _messagesList[141];
		Common::replace(keysCollected, "X", Common::String::format("%d", _keysCollected.size()));
		drawStringInSurface(keysCollected, 103, 41,  front, black, surface);

		Common::String spiritsDestroyedString = _messagesList[133];
		Common::replace(spiritsDestroyedString, "X", Common::String::format("%d", spiritsDestroyed));
		drawStringInSurface(spiritsDestroyedString, 145 , 132,  front, black, surface);

		for (int  i = 0; i < int(_keysCollected.size()) ; i++) {
			int y = 58 + (i / 2) * 18;

			if (i % 2 == 0) {
				surface->copyRectToSurfaceWithKey(*_keysBorderFrames[i], 58, y, Common::Rect(0, 0, _keysBorderFrames[i]->w, _keysBorderFrames[i]->h), black);
				keyRects.push_back(Common::Rect(58, y, 58 + _keysBorderFrames[i]->w / 2, y + _keysBorderFrames[i]->h));
			} else {
				surface->copyRectToSurfaceWithKey(*_keysBorderFrames[i], 80, y, Common::Rect(0, 0, _keysBorderFrames[i]->w, _keysBorderFrames[i]->h), black);
				keyRects.push_back(Common::Rect(80, y, 80 + _keysBorderFrames[i]->w / 2, y + _keysBorderFrames[i]->h));
			}
		}
	} else if (isSpectrum()) {
		Common::Array<Common::String> lines;
		lines.push_back(centerAndPadString("********************", 21));

		if (_language == Common::EN_ANY) {
			lines.push_back(centerAndPadString("s-save l-load q-quit", 21));
			lines.push_back("");
			lines.push_back(centerAndPadString(Common::String::format("keys   %d collected", _keysCollected.size()), 21));
			lines.push_back(centerAndPadString(Common::String::format("spirits  %d destroyed", spiritsDestroyed), 21));
			lines.push_back(centerAndPadString(Common::String::format("strength  %s", _messagesList[62 + shield / 6].c_str()), 21));
			lines.push_back(centerAndPadString(Common::String::format("score   %07d", score), 21));
		} else if (_language == Common::ES_ESP) {
			lines.push_back(centerAndPadString("s-salv c-carg q-quit", 21));
			lines.push_back("");
			lines.push_back(centerAndPadString(Common::String::format("llaves %d recogidas", _keysCollected.size()), 21));
			lines.push_back(centerAndPadString(Common::String::format("espirit %d destruidos", spiritsDestroyed), 21));
			lines.push_back(centerAndPadString(Common::String::format("fuerza  %s", _messagesList[62 + shield / 6].c_str()), 21));
			lines.push_back(centerAndPadString(Common::String::format("puntos   %07d", score), 21));
		} else {
			error("Language not supported");
		}

		lines.push_back("");
		lines.push_back(centerAndPadString("********************", 21));
		surface = drawStringsInSurface(lines, surface);
	}

	Common::Event event;
	Common::Point mousePos;
	bool cont = true;

	Common::Rect loadGameRect(101, 67, 133, 79);
	Common::Rect saveGameRect(101, 82, 133, 95);
	Common::Rect toggleSoundRect(101, 101, 133, 114);
	Common::Rect cycleRect(101, 116, 133, 129);
	Common::Rect backRect(101, 131, 133, 144);

	Graphics::Surface *originalSurface = new Graphics::Surface();
	originalSurface->copyFrom(*surface);

	Texture *menuTexture = _gfx->createTexture(surface);
	while (!shouldQuit() && cont) {
		while (_eventManager->pollEvent(event)) {

			// Events
			switch (event.type) {
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				if (event.customType == kActionLoad) {
					_gfx->setViewport(_fullscreenViewArea);
					_eventManager->purgeKeyboardEvents();
					_eventManager->purgeMouseEvents();

					loadGameDialog();
					_eventManager->purgeMouseEvents();
					if (isDOS() || isAmiga() || isAtariST()) {
						g_system->lockMouse(false);
						g_system->showMouse(true);
					}

					_gfx->setViewport(_viewArea);
				} else if (event.customType == kActionSave) {
					_gfx->setViewport(_fullscreenViewArea);
					_eventManager->purgeKeyboardEvents();
					_eventManager->purgeMouseEvents();

					saveGameDialog();
					_eventManager->purgeMouseEvents();
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
				for (int i = 0; i < int(keyRects.size()); i++) {
					if (keyRects[i].contains(mousePos)) {
						surface->copyFrom(*originalSurface);
						surface->frameRect(keyRects[i], front);
						drawStringInSurface(_messagesList[ 145 + _keysCollected[i] ], 103, 41,  front, black, surface);
						menuTexture->update(surface);
						break;
					}
				}

				if (loadGameRect.contains(mousePos)) {
					_gfx->setViewport(_fullscreenViewArea);
					_eventManager->purgeKeyboardEvents();
					loadGameDialog();
					g_system->lockMouse(false);
					g_system->showMouse(true);

					_gfx->setViewport(_viewArea);
				} else if (saveGameRect.contains(mousePos)) {
					_gfx->setViewport(_fullscreenViewArea);
					_eventManager->purgeKeyboardEvents();
					saveGameDialog();
					g_system->lockMouse(false);
					g_system->showMouse(true);

					_gfx->setViewport(_viewArea);
				} else if (toggleSoundRect.contains(mousePos)) {
					// Toggle sounds
				} else if (cycleRect.contains(mousePos)) {
					// Cycle between crawl, walk or run
					// It can fail if there is no room
				} else if (backRect.contains(mousePos))
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

	originalSurface->free();
	delete originalSurface;
	surface->free();
	delete surface;

	delete menuTexture;
	pauseToken.clear();
	g_system->lockMouse(true);
	g_system->showMouse(false);
}

void CastleEngine::drawFullscreenEndGameAndWait() {
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(_screenW, _screenH, _gfx->_texturePixelFormat);
	surface->fillRect(_fullscreenViewArea, _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00));
	surface->fillRect(_viewArea, _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00));
	surface->copyRectToSurface(*_endGameBackgroundFrame, 46, 38, Common::Rect(0, 0, _endGameBackgroundFrame->w, _endGameBackgroundFrame->h));

	Common::Event event;
	bool cont = true;
	bool magisterAlive = true;
	while (!shouldQuit() && cont) {
		while (_eventManager->pollEvent(event)) {

			// Events
			switch (event.type) {
			case Common::EVENT_LBUTTONDOWN:
				if (magisterAlive) {
					surface->copyRectToSurface(*_endGameThroneFrame, 121, 52, Common::Rect(0, 0, _endGameThroneFrame->w - 1, _endGameThroneFrame->h));
					magisterAlive = false;
				} else
					cont = false;
				break;

			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				if (event.customType == kActionShoot) {
					if (magisterAlive) {
						surface->copyRectToSurface(*_endGameThroneFrame, 121, 52, Common::Rect(0, 0, _endGameThroneFrame->w - 1, _endGameThroneFrame->h));
						magisterAlive = false;
					} else
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
		_gfx->clear(0, 0, 0, true);
		drawBorder();
		if (_currentArea)
			drawUI();

		drawFullscreenSurface(surface);
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(15); // try to target ~60 FPS
	}

	surface->free();
	delete surface;
}

void CastleEngine::drawFullscreenGameOverAndWait() {
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(_screenW, _screenH, _gfx->_texturePixelFormat);
	surface->fillRect(_fullscreenViewArea, _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00));
	uint32 blue = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x24, 0xA5);
	surface->copyRectToSurfaceWithKey(*_gameOverBackgroundFrame, _viewArea.left, _viewArea.top, Common::Rect(0, 0, _gameOverBackgroundFrame->w, _gameOverBackgroundFrame->h), blue);

	Common::Event event;
	bool cont = true;

	int score = _gameStateVars[k8bitVariableScore];
	int spiritsDestroyed = _gameStateVars[k8bitVariableSpiritsDestroyed];

	Common::String keysCollectedString;
	if (isDOS())
		keysCollectedString = _messagesList[130];
	else if (isSpectrum()) {
		if (_language == Common::EN_ANY)
			keysCollectedString = "X COLLECTED";
		else if (_language == Common::ES_ESP)
			keysCollectedString = "X RECOGIDAS";
		else
			error("Language not supported");
	}

	if (isDOS() && _keysCollected.size() == 0)
		keysCollectedString = _messagesList[128];
	else
		Common::replace(keysCollectedString, "X", Common::String::format("%d", _keysCollected.size()));
	keysCollectedString = centerAndPadString(keysCollectedString, 15);

	Common::String scoreString;
	if (isDOS())
		scoreString = _messagesList[131];
	else if (isSpectrum()) {
		if (_language == Common::EN_ANY)
			scoreString = "SCORE XXXXXXX";
		else if (_language == Common::ES_ESP)
			scoreString = "PUNTAJE XXXXXXX";
		else
			error("Language not supported");
	}

	Common::replace(scoreString, "XXXXXXX", Common::String::format("%07d", score));
	scoreString = centerAndPadString(scoreString, 15);

	Common::String spiritsDestroyedString;
	if (isDOS())
		spiritsDestroyedString = _messagesList[133];
	else if (isSpectrum()) {
		if (_language == Common::EN_ANY)
			spiritsDestroyedString = "X DESTROYED";
		else if (_language == Common::ES_ESP)
			spiritsDestroyedString = "X DESTRUIDOS";
		else
			error("Language not supported");
	}

	Common::replace(spiritsDestroyedString, "X", Common::String::format("%d", spiritsDestroyed));
	spiritsDestroyedString = centerAndPadString(spiritsDestroyedString, 15);

	while (!shouldQuit() && cont) {
		if (_temporaryMessageDeadlines.empty()) {
			insertTemporaryMessage(scoreString, _countdown - 2);
			insertTemporaryMessage(spiritsDestroyedString, _countdown - 4);
			insertTemporaryMessage(keysCollectedString, _countdown - 6);
		}

		while (_eventManager->pollEvent(event)) {

			// Events
			switch (event.type) {
			case Common::EVENT_LBUTTONDOWN:
				cont = false;
				break;
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				if (event.customType == kActionShoot || event.customType == kActionChangeMode || event.customType == kActionSkip) {
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
		_gfx->clear(0, 0, 0, true);
		drawFrame();

		drawFullscreenSurface(surface);
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(15); // try to target ~60 FPS
	}

	surface->free();
	delete surface;
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
		_shootingFrames = 0;
		_gfx->_inkColor = _currentArea->_inkColor;
		_gfx->_shakeOffset = Common::Point();
	}

	if (obj->isDestroyed())
		debugC(1, kFreescapeDebugCode, "WARNING: Destroying obj %d in area %d already destroyed!", objectID, areaID);

	obj->destroy();
}

void CastleEngine::executePrint(FCLInstruction &instruction) {
	uint16 index = instruction._source;
	_currentAreaMessages.clear();
	if (index == 128 && isDemo()) {
		drawFullscreenRiddleAndWait(18);
		return;
	} else if (index >= 129) {
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

	addGhosts();
	_endArea = 1;
	_endEntrance = 42;

	_timeoutMessage = _messagesList[1];
	// Shield is unused in Castle Master
	_noEnergyMessage = _messagesList[2];
	_crushedMessage = _messagesList[3];
	_fallenMessage = _messagesList[4];
	_outOfReachMessage = _messagesList[7];
	_noEffectMessage = _messagesList[8];

	if (!isAmiga()) {
		Graphics::Surface *tmp;
		tmp = loadBundledImage("castle_gate", !isDOS());
		_gameOverBackgroundFrame = new Graphics::ManagedSurface;
		_gameOverBackgroundFrame->copyFrom(*tmp);
		_gameOverBackgroundFrame->convertToInPlace(_gfx->_texturePixelFormat);
		tmp->free();
		delete tmp;
	}

	if (isDOS()) {
		// Discard some global conditions
		// It is unclear why they hide/unhide objects that formed the spirits
		for (int i = 0; i < 3; i++) {
			debugC(kFreescapeDebugParser, "Discarding condition %s", _conditionSources[1].c_str());
			_conditions.remove_at(1);
			_conditionSources.remove_at(1);
		}

		for (auto &it : _areaMap) {
			it._value->addStructure(_areaMap[255]);
			it._value->addObjectFromArea(227, _areaMap[255]);
			it._value->addObjectFromArea(228, _areaMap[255]);
			it._value->addObjectFromArea(229, _areaMap[255]);
			it._value->addObjectFromArea(242, _areaMap[255]);
			it._value->addObjectFromArea(139, _areaMap[255]);
		}
	}
	_areaMap[1]->addFloor();
	_areaMap[2]->addFloor();

}

void CastleEngine::loadRiddles(Common::SeekableReadStream *file, int offset, int number) {
	file->seek(offset);

	Common::Array<Common::Point> origins;
	for (int i = 0; i < number; i++) {
		Common::Point origin;
		origin.x = file->readByte();
		origin.y = file->readByte();
		debugC(1, kFreescapeDebugParser, "riddle %d origin: %d, %d", i, origin.x, origin.y);
		origins.push_back(origin);
	}

	debugC(1, kFreescapeDebugParser, "Riddle table:");
	int maxLineSize = isSpectrum() ? 20 : 24;

	for (int i = 0; i < number; i++) {
		Riddle riddle;
		riddle._origin = origins[i];
		int numberLines = file->readByte();
		debugC(1, kFreescapeDebugParser, "riddle %d number of lines: %d", i, numberLines);

		int8 x, y;
		for (int j = 0; j < numberLines; j++) {

			x = file->readByte();
			y = file->readByte();
			int size = file->readByte();
			debugC(1, kFreescapeDebugParser, "size: %d (max %d?)", size, maxLineSize);

			Common::String message = "";
			if (size == 255) {
				size = 19;
				while (size-- > 0)
					message = message + "*";

				debugC(1, kFreescapeDebugParser, "'%s' with offset: %d, %d", message.c_str(), x, y);
				riddle._lines.push_back(RiddleText(x, y, message));
				continue;
			} else if (size > maxLineSize) {
				assert(0);
			} else if (size == 0) {
				assert(0);
			}

			debugC(1, kFreescapeDebugParser, "extra byte: %x", file->readByte());
			if (i == 20 && j == 1 && _language == Common::ES_ESP)
				size = size + 3;

			while (size-- > 0) {
				byte c = file->readByte();
				if (c > 0x7F) {
					file->seek(-1, SEEK_CUR);
					break;
				} else if (c != 0)
					message = message + c;
			}

			if (isAmiga() || isAtariST())
				debug("extra byte: %x", file->readByte());
			debugC(1, kFreescapeDebugParser, "'%s' with offset: %d, %d", message.c_str(), x, y);

			riddle._lines.push_back(RiddleText(x, y, message));
		}
		_riddleList.push_back(riddle);
	}
	debugC(1, kFreescapeDebugParser, "End of riddles at %" PRIx64, file->pos());
}

void CastleEngine::drawFullscreenRiddleAndWait(uint16 riddle) {
	debugC(1, kFreescapeDebugCode, "Printing fullscreen riddle %d", riddle);

	if (_savedScreen) {
		_savedScreen->free();
		delete _savedScreen;
	}

	_savedScreen = _gfx->getScreenshot();
	int frontColor = 6;
	switch (_renderMode) {
		case Common::kRenderZX:
			frontColor = 7;
			break;
		default:
			break;
	}
	uint8 r, g, b;
	_gfx->readFromPalette(frontColor, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);
	uint32 transparent = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00);

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
		drawRiddle(riddle, front, transparent, surface);
		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(15); // try to target ~60 FPS
	}

	_savedScreen->free();
	delete _savedScreen;
	_savedScreen = nullptr;
	surface->free();
	delete surface;
}

void CastleEngine::drawRiddle(uint16 riddle, uint32 front, uint32 back, Graphics::Surface *surface) {
	int x = 0;
	int y = 0;
	int maxWidth = 136;

	if (isDOS()) {
		x = 40;
		y = 34;
	} else if (isSpectrum()) {
		x = 64;
		y = 37;
	}
	surface->copyRectToSurface((const Graphics::Surface)*_riddleTopFrame, x, y, Common::Rect(0, 0, _riddleTopFrame->w, _riddleTopFrame->h));
	for (y += _riddleTopFrame->h; y < maxWidth;) {
		surface->copyRectToSurface((const Graphics::Surface)*_riddleBackgroundFrame, x, y, Common::Rect(0, 0, _riddleBackgroundFrame->w, _riddleBackgroundFrame->h));
		y += _riddleBackgroundFrame->h;
	}
	surface->copyRectToSurface((const Graphics::Surface)*_riddleBottomFrame, x, maxWidth, Common::Rect(0, 0, _riddleBottomFrame->w, _riddleBottomFrame->h - 1));

	Common::Array<RiddleText> riddleMessages = _riddleList[riddle]._lines;
	x = _riddleList[riddle]._origin.x;
	y = _riddleList[riddle]._origin.y;

	if (isDOS()) {
		x = 38;
		y = 33;
	} else if (isSpectrum()) {
		x = 64;
		y = 36;
	}

	for (int i = 0; i < int(riddleMessages.size()); i++) {
		x = x + riddleMessages[i]._dx;
		y = y + riddleMessages[i]._dy;
		drawRiddleStringInSurface(riddleMessages[i]._text, x, y, front, back, surface);
	}
	drawFullscreenSurface(surface);
}

void CastleEngine::drawRiddleStringInSurface(const Common::String &str, int x, int y, uint32 fontColor, uint32 backColor, Graphics::Surface *surface) {
	Common::String ustr = str;
	ustr.toUppercase();
	if (isDOS()) {
		_fontRiddle.setBackground(backColor);
		_fontRiddle.drawString(surface, ustr, x, y, _screenW, fontColor);
	} else {
		_font.setBackground(backColor);
		_font.drawString(surface, ustr, x, y, _screenW, fontColor);
	}
}

void CastleEngine::drawEnergyMeter(Graphics::Surface *surface, Common::Point origin) {
	if (!_strenghtBackgroundFrame)
		return;

	surface->copyRectToSurface((const Graphics::Surface)*_strenghtBackgroundFrame, origin.x, origin.y, Common::Rect(0, 0, _strenghtBackgroundFrame->w, _strenghtBackgroundFrame->h));
	if (!_strenghtBarFrame)
		return;

	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
	uint32 back = 0;

	if (isDOS())
		back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
	surface->copyRectToSurfaceWithKey((const Graphics::Surface)*_strenghtBarFrame, origin.x + 5, origin.y + 8, Common::Rect(0, 0, _strenghtBarFrame->w, _strenghtBarFrame->h), black);

	Common::Point weightPoint;
	int frameIdx = -1;

	weightPoint = Common::Point(origin.x + 10, origin.y);
	frameIdx = _gameStateVars[k8bitVariableShield] % 4;

	if (_strenghtWeightsFrames.empty())
		return;

	if (frameIdx != 0) {
		frameIdx = 4 - frameIdx;
		surface->copyRectToSurfaceWithKey((const Graphics::Surface)*_strenghtWeightsFrames[frameIdx], weightPoint.x, weightPoint.y, Common::Rect(0, 0, 3, _strenghtWeightsFrames[frameIdx]->h), back);
		weightPoint += Common::Point(3, 0);
	}

	for (int i = 0; i < _gameStateVars[k8bitVariableShield] / 4; i++) {
		surface->copyRectToSurfaceWithKey((const Graphics::Surface)*_strenghtWeightsFrames[0], weightPoint.x, weightPoint.y, Common::Rect(0, 0, 3, _strenghtWeightsFrames[0]->h), back);
		weightPoint += Common::Point(3, 0);
	}

	weightPoint = Common::Point(origin.x + 62, origin.y);
	frameIdx = _gameStateVars[k8bitVariableShield] % 4;

	if (frameIdx != 0) {
		frameIdx = 4 - frameIdx;
		surface->copyRectToSurfaceWithKey((const Graphics::Surface)*_strenghtWeightsFrames[frameIdx], weightPoint.x, weightPoint.y, Common::Rect(0, 0, 3, _strenghtWeightsFrames[frameIdx]->h), back);
		weightPoint += Common::Point(-3, 0);
	}

	for (int i = 0; i < _gameStateVars[k8bitVariableShield] / 4; i++) {
		surface->copyRectToSurfaceWithKey((const Graphics::Surface)*_strenghtWeightsFrames[0], weightPoint.x, weightPoint.y, Common::Rect(0, 0, 3, _strenghtWeightsFrames[0]->h), back);
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
	if (_lastTick == _ticks)
		return;

	_lastTick = _ticks;

	if (_sensors.empty()) {
		_gfx->_shakeOffset = Common::Point();
		return;
	}

	for (auto &it : _sensors) {
		Sensor *sensor = (Sensor *)it;
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
	}


	if (!ghostInArea()) {
		_gfx->_shakeOffset = Common::Point();
		return;
	}

	if (_disableSensors)
		return;

	// This is the frequency to shake the screen
	if (_ticks % 5 == 0) {
		if (_underFireFrames <= 0)
			_underFireFrames = 1;
	}

	// This is the frequency to take damage
	if (_ticks % 100 == 0) {
		takeDamageFromSensor();
	}
}

bool CastleEngine::ghostInArea() {
	for (auto &it : _sensors) {
		if (it->isDestroyed() || it->isInvisible())
			continue;
		return true;
		break;
	}
	return false;
}

void CastleEngine::drawSensorShoot(Sensor *sensor) {
	if (isSpectrum()) {
		_gfx->_inkColor = 1 + (_gfx->_inkColor + 1) % 7;
	} else if (isDOS()) {
		float shakeIntensity = 10;
		Common::Point shakeOffset;
		shakeOffset.x = (_rnd->getRandomNumber(10) / 10.0 - 0.5f) * shakeIntensity;
		shakeOffset.y = (_rnd->getRandomNumber(10) / 10.0 - 0.5f) * shakeIntensity;
		_gfx->_shakeOffset = shakeOffset;
	} else {
		/* TODO */
	}
}

void CastleEngine::tryToCollectKey() {
	if (_gameStateVars[32] > 0) { // Key collected!
		if (_keysCollected.size() < 10) {
			_gameStateVars[31]++;
			setGameBit(_gameStateVars[32]);
			_keysCollected.push_back(_gameStateVars[32]);
		}
		_gameStateVars[32] = 0;
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
		int spiritsDestroyed = _gameStateVars[k8bitVariableSpiritsDestroyed];
		_lastMinute = minutes / 2;
		_spiritsMeter++;
		_spiritsMeterPosition = _spiritsMeter * (_spiritsToKill - spiritsDestroyed) / _spiritsToKill;
		if (_spiritsMeterPosition >= _spiritsMeterMax)
			_countdown = -1;
	}

	if (_lastTenSeconds != seconds / 10) {
		//_gameStateVars[0x1e] += 1;
		//_gameStateVars[0x1f] += 1;
		_lastTenSeconds = seconds / 10;
		executeLocalGlobalConditions(false, false, true);
	}
}

void CastleEngine::borderScreen() {
	if (isAmiga() && isDemo())
		return; // Skip character selection

	if (isSpectrum())
		FreescapeEngine::borderScreen();
	else {
		uint32 color = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00);
		Graphics::Surface *surface = new Graphics::Surface();
		surface->create(_screenW, _screenH, _gfx->_texturePixelFormat);
		surface->fillRect(_fullscreenViewArea, color);

		int x = 40;
		int y = 34;

		Common::Array<RiddleText> selectMessage = _riddleList[19]._lines;
		for (int i = 0; i < int(selectMessage.size()); i++) {
			x = x + selectMessage[i]._dx;
			y = y + selectMessage[i]._dy;
			// Color is not important, as the font has already a palette
			drawStringInSurface(selectMessage[i]._text, x, y, 0, 0, surface);
		}
		drawFullscreenSurface(surface);
		drawBorderScreenAndWait(surface, 6 * 60);
		surface->free();
		delete surface;
	}
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

	if (isSpectrum()) {
		if (_language == Common::ES_ESP) {
			// No accent in "príncipe" since it is not supported by the font
			lines.push_back(centerAndPadString("*******************", 21));
			lines.push_back(centerAndPadString("Seleccion el ", 21));
			lines.push_back(centerAndPadString("personaje que quiera", 21));
			lines.push_back(centerAndPadString("ser y pulse enter", 21));
			lines.push_back("");
			lines.push_back(centerAndPadString("1. Principe", 21));
			lines.push_back(centerAndPadString("2. Princesa", 21));
			lines.push_back("");
			lines.push_back(centerAndPadString("*******************", 21));
		} else {
			lines.push_back(centerAndPadString("*******************", 21));
			lines.push_back(centerAndPadString("Select the character", 21));
			lines.push_back(centerAndPadString("you wish to play", 21));
			lines.push_back(centerAndPadString("and press enter", 21));
			lines.push_back("");
			lines.push_back(centerAndPadString("1. Prince  ", 21));
			lines.push_back(centerAndPadString("2. Princess", 21));
			lines.push_back("");
			lines.push_back(centerAndPadString("*******************", 21));
		}
		drawStringsInSurface(lines, surface);
	} else {
		int x = 0;
		int y = 0;

		Common::Array<RiddleText> selectMessage = _riddleList[21]._lines;
		for (int i = 0; i < int(selectMessage.size()); i++) {
			x = x + selectMessage[i]._dx;
			y = y + selectMessage[i]._dy;
			drawStringInSurface(selectMessage[i]._text, x, y, color, color, surface);
		}
		drawFullscreenSurface(surface);
	}

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
	stream->writeUint32LE(_keysCollected.size());
	for (auto &it : _keysCollected) {
		stream->writeUint32LE(it);
	}

	stream->writeUint32LE(_spiritsMeter);

	for (auto &it : _areaMap) {
		stream->writeUint16LE(it._key);
		stream->writeUint32LE(_exploredAreas[it._key]);
	}

	return Common::kNoError;
}

Common::Error CastleEngine::loadGameStreamExtended(Common::SeekableReadStream *stream) {
	_keysCollected.clear();
	int numberKeys = stream->readUint32LE();
	for (int i = 0; i < numberKeys; i++) {
		_keysCollected.push_back(stream->readUint32LE());
	}

	_spiritsMeter = stream->readUint32LE();

	for (uint i = 0; i < _areaMap.size(); i++) {
		uint16 key = stream->readUint16LE();
		_exploredAreas[key] = stream->readUint32LE();
	}

	if (_useRockTravel) // Enable cheat
		setGameBit(k8bitGameBitTravelRock);

	for (auto &it : _areaMap) {
		it._value->resetAreaGroups();
	}
	return Common::kNoError;
}

} // End of namespace Freescape
