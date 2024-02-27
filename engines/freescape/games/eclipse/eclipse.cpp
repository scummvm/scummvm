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
#include "freescape/games/eclipse/eclipse.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

EclipseEngine::EclipseEngine(OSystem *syst, const ADGameDescription *gd) : FreescapeEngine(syst, gd) {
	if (isDOS())
		initDOS();
	else if (isCPC())
		initCPC();
	else if (isSpectrum())
		initZX();

	_playerHeightNumber = 1;
	_playerHeights.push_back(32);
	_playerHeights.push_back(48);
	_playerHeight = _playerHeights[_playerHeightNumber];

	_playerWidth = 8;
	_playerDepth = 8;
	_stepUpDistance = 32;

	_playerStepIndex = 2;
	_playerSteps.clear();
	_playerSteps.push_back(1);
	_playerSteps.push_back(10);
	_playerSteps.push_back(25);

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
}

void EclipseEngine::initGameState() {
	FreescapeEngine::initGameState();

	_playerHeightNumber = 1;
	_playerHeight = _playerHeights[_playerHeightNumber];

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
}

bool EclipseEngine::checkIfGameEnded() {
	if (_hasFallen && _avoidRenderingFrames == 0) {
		_hasFallen = false;
		playSoundFx(4, false);

		// If shield is less than 11 after a fall, the game ends
		if (_gameStateVars[k8bitVariableShield] > 15 + 11) {
			_gameStateVars[k8bitVariableShield] -= 15;
			return false; // Game can continue
		}
		if (!_fallenMessage.empty())
			insertTemporaryMessage(_fallenMessage, _countdown - 4);
		_gameStateControl = kFreescapeGameStateEnd;
	}

	FreescapeEngine::checkIfGameEnded();
	return false;
}

void EclipseEngine::endGame() {
	if (_gameStateControl == kFreescapeGameStateEnd) {
		removeTimers();
		if (_countdown > 0)
			_countdown -= 10;
		else
			_countdown = 0;
	}

	if (_endGameKeyPressed && _countdown == 0) {
		_gameStateControl = kFreescapeGameStateRestart;
	}
	_endGameKeyPressed = false;
}

void EclipseEngine::gotoArea(uint16 areaID, int entranceID) {
	debugC(1, kFreescapeDebugMove, "Jumping to area: %d, entrance: %d", areaID, entranceID);

	assert(_areaMap.contains(areaID));
	_currentArea = _areaMap[areaID];
	_currentArea->show();

	_currentAreaMessages.clear();
	_currentAreaMessages.push_back(_currentArea->_name);

	if (entranceID == -1)
		return;

	assert(entranceID > 0);
	traverseEntrance(entranceID);

	_lastPosition = _position;

	if (areaID == _startArea && entranceID == _startEntrance)
		playSound(9, true);
	if (areaID == _endArea && entranceID == _endEntrance) {
		_flyMode = true;
		_pitch = 20;
	} else
		playSound(5, false);

	_gfx->_keyColor = 0;
	swapPalette(areaID);
	_currentArea->_usualBackgroundColor = isCPC() ? 1 : 0;

	resetInput();
}

void EclipseEngine::drawBackground() {
	clearBackground();
	_gfx->drawBackground(_currentArea->_skyColor);
	if (_currentArea && _currentArea->getAreaID() == 1) {
		if (_countdown <= 15 * 60) // Last 15 minutes
			_gfx->drawBackground(5);
		if (_countdown <= 10) // Last 10 seconds
			_gfx->drawBackground(1);

		float progress = 0;
		if (_countdown >= 0)
			progress = float(_countdown) / _initialCountdown;

		_gfx->drawEclipse(15, 10, progress);
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
		while (_eventManager->pollEvent(event)) {

			// Events
			switch (event.type) {
				case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_l) {
					_gfx->setViewport(_fullscreenViewArea);
					_eventManager->purgeKeyboardEvents();
					loadGameDialog();
					_gfx->setViewport(_viewArea);
				} else if (event.kbd.keycode == Common::KEYCODE_s) {
					_gfx->setViewport(_fullscreenViewArea);
					_eventManager->purgeKeyboardEvents();
					saveGameDialog();
					_gfx->setViewport(_viewArea);
				} else if (isDOS() && event.kbd.keycode == Common::KEYCODE_t) {
					playSound(6, true);
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
	pauseToken.clear();
}

void EclipseEngine::pressedKey(const int keycode) {
	if (keycode == Common::KEYCODE_r) {
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
	}
}

void EclipseEngine::releasedKey(const int keycode) {
	if (keycode == Common::KEYCODE_r)
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

void EclipseEngine::drawIndicator(Graphics::Surface *surface, int xPosition, int yPosition, int separation) {
	if (_indicators.size() == 0)
		return;

	for (int i = 0; i < 5; i++) {
		if (_gameStateVars[kVariableEclipseAnkhs] > i)
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
