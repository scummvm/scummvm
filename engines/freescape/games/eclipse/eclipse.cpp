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

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/standard-actions.h"
#include "common/translation.h"

#include "freescape/freescape.h"
#include "freescape/games/eclipse/eclipse.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

EclipseEngine::EclipseEngine(OSystem *syst, const ADGameDescription *gd) : FreescapeEngine(syst, gd) {
	// These sounds can be overriden by the class of each platform
	_soundIndexShoot = 8;
	_soundIndexCollide = 3;
	_soundIndexFall = 3;
	_soundIndexClimb = 4;
	_soundIndexMenu = -1;
	_soundIndexStart = 9;
	_soundIndexAreaChange = 5;

	_soundIndexStartFalling = -1;
	_soundIndexEndFalling = -1;

	_soundIndexNoShield = -1;
	_soundIndexNoEnergy = -1;
	_soundIndexFallen = -1;
	_soundIndexTimeout = -1;
	_soundIndexForceEndGame = -1;
	_soundIndexCrushed = -1;
	_soundIndexMissionComplete = -1;

	if (isDOS())
		initDOS();
	else if (isCPC())
		initCPC();
	else if (isSpectrum())
		initZX();
	else if (isAmiga() || isAtariST())
		initAmigaAtari();

	_playerHeightNumber = 1;
	_playerHeightMaxNumber = 1;

	_playerWidth = 8;
	_playerDepth = 8;
	_stepUpDistance = 32;

	_playerStepIndex = 2;
	_playerSteps.clear();
	_playerSteps.push_back(2);
	_playerSteps.push_back(30);
	_playerSteps.push_back(60);

	_angleRotationIndex = 1;
	_angleRotations.push_back(5);
	_angleRotations.push_back(10);
	_angleRotations.push_back(15);

	_maxEnergy = 27;
	_maxShield = 50;

	_initialEnergy = 16;
	_initialShield = 50;

	_endArea = 1;
	_endEntrance = 33;

	_lastThirtySeconds = 0;
	_lastSecond = -1;
	_resting = false;
}

void EclipseEngine::initGameState() {
	FreescapeEngine::initGameState();

	_playerHeightNumber = 1;

	_gameStateVars[k8bitVariableEnergy] = _initialEnergy;
	_gameStateVars[k8bitVariableShield] = _initialShield;

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	_lastThirtySeconds = seconds / 30;
	_resting = false;
}

void EclipseEngine::loadAssets() {
	FreescapeEngine::loadAssets();

	_timeoutMessage = _messagesList[1];
	_noShieldMessage = _messagesList[0];
	//_noEnergyMessage = _messagesList[16];
	_fallenMessage = _messagesList[3];
	_crushedMessage = _messagesList[2];

	_areaMap[1]->addFloor();
	if (isSpectrum())
		_areaMap[1]->_paperColor = 1;

	if (!isDemo() && !isEclipse2()) {
		_areaMap[51]->addFloor();
		_areaMap[51]->_paperColor = 1;

		// Workaround for fixing some planar objects from area 9 that have null size
		Object *obj = nullptr;
		obj = _areaMap[9]->objectWithID(7);
		assert(obj);
		obj->_size = 32 * Math::Vector3d(3, 0, 2);

		obj = _areaMap[9]->objectWithID(8);
		assert(obj);
		obj->_size = 32 * Math::Vector3d(3, 0, 2);

		obj = _areaMap[9]->objectWithID(9);
		assert(obj);
		obj->_size = 32 * Math::Vector3d(3, 0, 2);
	}
}

bool EclipseEngine::checkIfGameEnded() {
	if (_gameStateControl == kFreescapeGameStatePlaying) {
		if (_hasFallen && _avoidRenderingFrames == 0) {
			_hasFallen = false;
			if (isDOS())
				playSoundFx(4, false);
			else
				playSound(_soundIndexStartFalling, false);

			// If shield is less than 11 after a fall, the game ends
			if (_gameStateVars[k8bitVariableShield] > 15 + 11) {
				_gameStateVars[k8bitVariableShield] -= 15;
				return false; // Game can continue
			}
			if (!_fallenMessage.empty())
				insertTemporaryMessage(_fallenMessage, _countdown - 4);
			_gameStateControl = kFreescapeGameStateEnd;
		} else if (getGameBit(16)) {
			_gameStateControl = kFreescapeGameStateEnd;
			insertTemporaryMessage(_messagesList[4], INT_MIN);
		}

		FreescapeEngine::checkIfGameEnded();
	}
	return false;
}

void EclipseEngine::endGame() {
	FreescapeEngine::endGame();

	if (!_endGamePlayerEndArea)
		return;

	if (_gameStateControl == kFreescapeGameStateEnd) {
		removeTimers();
		if (getGameBit(16)) {
			if (_countdown > - 3600)
				_countdown -= 10;
			else
				_countdown = -3600;
		} else {
			if (_countdown > 0)
				_countdown -= 10;
			else
				_countdown = 0;
		}
	}

	if (_endGameKeyPressed && (_countdown == 0 || _countdown == -3600)) {
		if (isSpectrum())
			playSound(5, true);
		_gameStateControl = kFreescapeGameStateRestart;
	}
	_endGameKeyPressed = false;
}

void EclipseEngine::initKeymaps(Common::Keymap *engineKeyMap, Common::Keymap *infoScreenKeyMap, const char *target) {
	FreescapeEngine::initKeymaps(engineKeyMap, infoScreenKeyMap, target);
	Common::Action *act;

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
	if (isSpectrum())
		act->addDefaultInputMapping("1");
	else
		act->addDefaultInputMapping("ESCAPE");
	infoScreenKeyMap->addAction(act);

	act = new Common::Action("TOGGLESOUND", _("Toggle Sound"));
	act->setCustomEngineActionEvent(kActionToggleSound);
	act->addDefaultInputMapping("t");
	infoScreenKeyMap->addAction(act);

	act = new Common::Action("ROTL", _("Rotate Left"));
	act->setCustomEngineActionEvent(kActionRotateLeft);
	act->addDefaultInputMapping("q");
	engineKeyMap->addAction(act);

	act = new Common::Action("ROTR", _("Rotate Right"));
	act->setCustomEngineActionEvent(kActionRotateRight);
	act->addDefaultInputMapping("w");
	engineKeyMap->addAction(act);

	// I18N: Illustrates the angle at which you turn left or right.
	act = new Common::Action("CHNGANGLE", _("Change Angle"));
	act->setCustomEngineActionEvent(kActionChangeAngle);
	act->addDefaultInputMapping("a");
	engineKeyMap->addAction(act);

	// I18N: STEP SIZE: Measures the size of one movement in the direction you are facing (1-250 standard distance units (SDUs))
	act = new Common::Action("CHNGSTEPSIZE", _("Change Step Size"));
	act->setCustomEngineActionEvent(kActionChangeStepSize);
	act->addDefaultInputMapping("s");
	engineKeyMap->addAction(act);

	act = new Common::Action("TGGLHEIGHT", _("Toggle Height"));
	act->setCustomEngineActionEvent(kActionToggleRiseLower);
	act->addDefaultInputMapping("h");
	engineKeyMap->addAction(act);

	act = new Common::Action("REST", _("Rest"));
	act->setCustomEngineActionEvent(kActionRest);
	act->addDefaultInputMapping("r");
	engineKeyMap->addAction(act);

	act = new Common::Action("FACEFRWARD", _("Face Forward"));
	act->setCustomEngineActionEvent(kActionFaceForward);
	act->addDefaultInputMapping("f");
	engineKeyMap->addAction(act);
}

void EclipseEngine::gotoArea(uint16 areaID, int entranceID) {
	debugC(1, kFreescapeDebugMove, "Jumping to area: %d, entrance: %d", areaID, entranceID);

	assert(_areaMap.contains(areaID));
	_currentArea = _areaMap[areaID];
	_currentArea->show();

	_currentAreaMessages.clear();
	_currentAreaMessages.push_back(_currentArea->_name);

	if (entranceID > 0)
		traverseEntrance(entranceID);
	else if (entranceID == -1)
		debugC(1, kFreescapeDebugMove, "Loading game, no change in position");
	else
		error("Invalid area change!");

	_lastPosition = _position;

	if (areaID == _startArea && entranceID == _startEntrance) {
		playSound(_soundIndexStart, true);
		if (isEclipse2()) {
			_gameStateControl = kFreescapeGameStateStart;
		}

	} if (areaID == _endArea && entranceID == _endEntrance) {
		_flyMode = true;
		if (isDemo())
			_pitch = 20;
		else
			_pitch = 10;
	} else {
		playSound(_soundIndexAreaChange, false);
	}

	_gfx->_keyColor = 0;
	swapPalette(areaID);
	_currentArea->_usualBackgroundColor = isCPC() ? 1 : 0;
	if (isAmiga() || isAtariST())
		_currentArea->_skyColor = 15;

	resetInput();
}

void EclipseEngine::drawBackground() {
	clearBackground();
	_gfx->drawBackground(_currentArea->_skyColor);
	if (_currentArea && _currentArea->getAreaID() == 1) {
		if (ABS(_countdown) <= 15 * 60) // Last 15 minutes
			_gfx->drawBackground(5);
		if (ABS(_countdown) <= 10) // Last 10 seconds
			_gfx->drawBackground(1);

		float progress = 0;
		if (_countdown >= 0 || getGameBit(16))
			progress = float(_countdown) / _initialCountdown;

		uint8 color1 = 15;
		uint8 color2 = 10;

		if (isSpectrum() || isCPC()) {
			color1 = 2;
			color2 = 10;
		} else if (isAmiga() || isAtariST()) {
			color1 = 8;
			color2 = 14;
		}

		_gfx->drawEclipse(color1, color2, progress);
	}
}

void EclipseEngine::titleScreen() {
	if (isDOS())
		playSoundFx(2, true);
	FreescapeEngine::titleScreen();
}


void EclipseEngine::borderScreen() {
	if (_border) {
		drawBorder();
		if (isDemo() && isCPC()) {
			drawFullscreenMessageAndWait(_messagesList[23]);
			drawFullscreenMessageAndWait(_messagesList[24]);
			drawFullscreenMessageAndWait(_messagesList[25]);
		} else if (isDemo() && isSpectrum()) {
			if (_variant & GF_ZX_DEMO_MICROHOBBY) {
				drawFullscreenMessageAndWait(_messagesList[23]);
			} else if (_variant & GF_ZX_DEMO_CRASH) {
				drawFullscreenMessageAndWait(_messagesList[9]);
				drawFullscreenMessageAndWait(_messagesList[10]);
				drawFullscreenMessageAndWait(_messagesList[11]);
			}
		} else {
			FreescapeEngine::borderScreen();
		}
	}
}

void EclipseEngine::drawInfoMenu() {
	PauseToken pauseToken = pauseEngine();
	if (_savedScreen) {
		_savedScreen->free();
		delete _savedScreen;
	}
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
					_gfx->setViewport(_viewArea);
				} else if (event.customType == kActionSave) {
					_gfx->setViewport(_fullscreenViewArea);
					_eventManager->purgeKeyboardEvents();
					saveGameDialog();
					_gfx->setViewport(_viewArea);
				} else if (isDOS() && event.customType == kActionToggleSound) {
					playSound(_soundIndexMenu, true);
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
				break;

			default:
				break;
			}
		}
		drawFrame();
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
}

void EclipseEngine::pressedKey(const int keycode) {
	if (keycode == kActionRotateLeft) {
		rotate(-_angleRotations[_angleRotationIndex], 0);
	} else if (keycode == kActionRotateRight) {
		rotate(_angleRotations[_angleRotationIndex], 0);
	} else if (keycode == kActionChangeAngle) {
		changeAngle();
	} else if (keycode == kActionChangeStepSize) {
		changeStepSize();
	} else if (keycode == kActionToggleRiseLower) {
		if (_playerHeightNumber == 0)
			rise();
		else if (_playerHeightNumber == 1)
			lower();
		else
			error("Invalid player height index: %d", _playerHeightNumber);
	} else if (keycode == kActionRest) {
		if (_currentArea->getAreaID() == 1) {
			playSoundFx(3, false);
			if (_temporaryMessages.empty())
				insertTemporaryMessage(_messagesList[6], _countdown - 2);
		} else {
			_resting = true;
			if (_temporaryMessages.empty())
				insertTemporaryMessage(_messagesList[7], _countdown - 2);
			_countdown = _countdown - 5;
		}
	} else if (keycode == kActionFaceForward) {
		_pitch = 0;
		updateCamera();
	}
}

void EclipseEngine::releasedKey(const int keycode) {
	if (keycode == kActionRiseOrFlyUp)
		_resting = false;
}

void EclipseEngine::drawAnalogClock(Graphics::Surface *surface, int x, int y, uint32 colorHand1, uint32 colorHand2, uint32 colorBack) {
	// These calls will cover the pixels of the hardcoded clock image
	drawAnalogClockHand(surface, x, y, 6 * 6 - 90, 12, colorBack);
	drawAnalogClockHand(surface, x, y, 7 * 6 - 90, 12, colorBack);
	drawAnalogClockHand(surface, x, y, 41 * 6 - 90, 11, colorBack);
	drawAnalogClockHand(surface, x, y, 42 * 6 - 90, 11, colorBack);
	drawAnalogClockHand(surface, x, y, 0 * 6 - 90, 11, colorBack);

	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	hours = 7 + 2 - hours; // It's 7 o-clock when the game starts
	minutes = 59 - minutes;
	seconds = 59 - seconds;
	drawAnalogClockHand(surface, x, y, hours * 30 - 90, 11, colorHand1);
	drawAnalogClockHand(surface, x, y, minutes * 6 - 90, 11, colorHand1);
	drawAnalogClockHand(surface, x, y, seconds * 6 - 90, 11, colorHand2);
}

void EclipseEngine::drawAnalogClockHand(Graphics::Surface *surface, int x, int y, double degrees, double magnitude, uint32 color) {
	const double degtorad = (M_PI * 2) / 360;
	double w = magnitude * cos(degrees * degtorad);
	double h = magnitude * sin(degrees * degtorad);
	surface->drawLine(x, y, x+(int)w, y+(int)h, color);
}

void EclipseEngine::drawCompass(Graphics::Surface *surface, int x, int y, double degrees, double magnitude, uint32 color) {
	const double degtorad = (M_PI * 2) / 360;
	double w = magnitude * cos(-degrees * degtorad);
	double h = magnitude * sin(-degrees * degtorad);

	int dx = 0;
	int dy = 0;

	// Adjust dx and dy to make the compass look like a compass
	if (degrees == 0 || degrees == 360) {
		dx = 0;
		dy = 2;
	} else if (degrees > 0 && degrees < 90) {
		dx = 1;
		dy = 1;
	} else if (degrees == 90) {
		dx = 2;
		dy = 0;
	} else if (degrees > 90 && degrees < 180) {
		dx = 1;
		dy = -1;
	} else if (degrees == 180) {
		dx = 0;
		dy = 2;
	} else if (degrees > 180 && degrees < 270) {
		dx = -1;
		dy = -1;
	} else if (degrees == 270) {
		dx = 2;
		dy = 0;
	} else if (degrees > 270 && degrees < 360) {
		dx = -1;
		dy = 1;
	}

	surface->drawLine(x, y, x+(int)w, y+(int)h, color);
	surface->drawLine(x - dx, y - dy, x+(int)w, y+(int)h, color);
	surface->drawLine(x + dx, y + dy, x+(int)w, y+(int)h, color);

	surface->drawLine(x - dx, y - dy, x+(int)-w, y+(int)-h, color);
	surface->drawLine(x + dx, y + dy, x+(int)-w, y+(int)-h, color);
}

// Copied from BITMAP::circlefill in engines/ags/lib/allegro/surface.cpp
void fillCircle(Graphics::Surface *surface, int x, int y, int radius, int color) {
	int cx = 0;
	int cy = radius;
	int df = 1 - radius;
	int d_e = 3;
	int d_se = -2 * radius + 5;

	do {
		surface->hLine(x - cy, y - cx, x + cy, color);

		if (cx)
			surface->hLine(x - cy, y + cx, x + cy, color);

		if (df < 0) {
			df += d_e;
			d_e += 2;
			d_se += 2;
		} else {
			if (cx != cy) {
				surface->hLine(x - cx, y - cy, x + cx, color);

				if (cy)
					surface->hLine(x - cx, y + cy, x + cx, color);
			}

			df += d_se;
			d_e += 2;
			d_se += 4;
			cy--;
		}

		cx++;

	} while (cx <= cy);
}

void EclipseEngine::drawEclipseIndicator(Graphics::Surface *surface, int x, int y, uint32 color1, uint32 color2) {
	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);

	// These calls will cover the pixels of the hardcoded eclipse image
	surface->fillRect(Common::Rect(x, y, x + 50, y + 20), black);

	float progress = 0;
	if (_countdown >= 0)
		progress = float(_countdown) / _initialCountdown;

	int difference = 14 * progress;

	fillCircle(surface, x + 7, y + 10, 7, color1); // Sun
	fillCircle(surface, x + 7 + difference, y + 10, 7, color2); // Moon
}

void EclipseEngine::drawIndicator(Graphics::Surface *surface, int xPosition, int yPosition, int separation) {
	if (_indicators.size() == 0)
		return;

	for (int i = 0; i < 5; i++) {
		if (isSpectrum()) {
			if (_gameStateVars[kVariableEclipseAnkhs] <= i)
				continue;
		} else if (_gameStateVars[kVariableEclipseAnkhs] > i)
			continue;
		surface->copyRectToSurface(*_indicators[0], xPosition + separation * i, yPosition, Common::Rect(_indicators[0]->w, _indicators[0]->h));
	}
}

void EclipseEngine::drawSensorShoot(Sensor *sensor) {
	Math::Vector3d target;
	float distance = 5;
	int axisToSkip = -1;

	if (sensor->_axis == 0x1 || sensor->_axis == 0x2)
		axisToSkip = 0;

	if (sensor->_axis == 0x10 || sensor->_axis == 0x20)
		axisToSkip = 2;

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (i == j) {
				target = sensor->getOrigin();
				if (i != axisToSkip)
					target.setValue(i, target.getValue(i) + distance);

				_gfx->renderSensorShoot(1, sensor->getOrigin(), target, _viewArea);

				target = sensor->getOrigin();

				if (i != axisToSkip)
					target.setValue(i, target.getValue(i) - distance);

				_gfx->renderSensorShoot(1, sensor->getOrigin(), target, _viewArea);
			} else {
				target = sensor->getOrigin();
				if (i != axisToSkip)
					target.setValue(i, target.getValue(i) + distance);

				if (j != axisToSkip)
					target.setValue(j, target.getValue(j) + distance);

				_gfx->renderSensorShoot(1, sensor->getOrigin(), target, _viewArea);

				target = sensor->getOrigin();
				if (i != axisToSkip)
					target.setValue(i, target.getValue(i) + distance);

				if (j != axisToSkip)
					target.setValue(j, target.getValue(j) - distance);

				_gfx->renderSensorShoot(1, sensor->getOrigin(), target, _viewArea);

				target = sensor->getOrigin();

				if (i != axisToSkip)
					target.setValue(i, target.getValue(i) - distance);

				if (j != axisToSkip)
					target.setValue(j, target.getValue(j) - distance);

				_gfx->renderSensorShoot(1, sensor->getOrigin(), target, _viewArea);

				target = sensor->getOrigin();
				if (i != axisToSkip)
					target.setValue(i, target.getValue(i) - distance);

				if (j != axisToSkip)
					target.setValue(j, target.getValue(j) + distance);

				_gfx->renderSensorShoot(1, sensor->getOrigin(), target, _viewArea);
			}
		}
	}
}

void EclipseEngine::updateTimeVariables() {
	if (isEclipse2() && _gameStateControl == kFreescapeGameStateStart) {
		executeLocalGlobalConditions(false, true, false);
		_gameStateControl = kFreescapeGameStatePlaying;
	}

	if (_gameStateControl != kFreescapeGameStatePlaying)
		return;
	// This function only executes "on collision" room/global conditions
	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);
	if (_lastThirtySeconds != seconds / 30) {
		_lastThirtySeconds = seconds / 30;

		if (!_resting && _gameStateVars[k8bitVariableEnergy] > 0) {
			_gameStateVars[k8bitVariableEnergy] -= 1;
		}

		if (_gameStateVars[k8bitVariableShield] < _maxShield) {
			_gameStateVars[k8bitVariableShield] += 1;
		}

		executeLocalGlobalConditions(false, false, true);
	}

	if (isEclipse() && isSpectrum() && _currentArea->getAreaID() == 42) {
		if (_lastSecond != seconds) { // Swap ink and paper colors every second
			_lastSecond = seconds;
			int tmp = _gfx->_inkColor;
			_gfx->_inkColor = _gfx->_paperColor;
			_gfx->_paperColor = tmp;
		}
	}
}

void EclipseEngine::executePrint(FCLInstruction &instruction) {
	uint16 index = instruction._source - 1;
	debugC(1, kFreescapeDebugCode, "Printing message %d", index);
	if (index > 127) {
		index = _messagesList.size() - (index - 254) - 2;
		drawFullscreenMessageAndWait(_messagesList[index]);
		return;
	}
	insertTemporaryMessage(_messagesList[index], _countdown - 2);
}

Common::Error EclipseEngine::saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave) {
	return Common::kNoError;
}

Common::Error EclipseEngine::loadGameStreamExtended(Common::SeekableReadStream *stream) {
	return Common::kNoError;
}

} // End of namespace Freescape
