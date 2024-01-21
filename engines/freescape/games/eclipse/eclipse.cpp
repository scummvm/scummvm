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

static const char *rawMessagesTable[] = {
	"HEART  FAILURE",
	"SUN ECLIPSED",
	"CRUSHED TO DEATH",
	"FATAL FALL",
	"CURSE OVERCOME",
	"TOTAL ECLIPSE",
	"TOO HOT TO REST!",
	"RESTING...",
	" ANKH FOUND ",
	"WAY  BLOCKED",
	"5 ANKHS REQUIRED",
	"$2M REWARD",
	"MAKE THE MATCH",
	"TOUCH TO COLLECT",
	"NO ENTRY",
	"REMOVE LID",
	"POISON AIR",
	"MATCH MADE",
	NULL};

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

	const char **messagePtr = rawMessagesTable;
	if (isDOS()) {
		debugC(1, kFreescapeDebugParser, "String table:");
		while (*messagePtr) {
			Common::String message(*messagePtr);
			_messagesList.push_back(message);
			debugC(1, kFreescapeDebugParser, "%s", message.c_str());
			messagePtr++;
		}
	}

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
	_maxShield = 10; // TODO

	_initialEnergy = 16;
	_initialShield = 10; // TODO
}

void EclipseEngine::initGameState() {
	_flyMode = false;
	_hasFallen = false;
	_noClipMode = false;
	_playerWasCrushed = false;
	_shootingFrames = 0;
	_underFireFrames = 0;
	_yaw = 0;
	_pitch = 0;

	for (int i = 0; i < k8bitMaxVariable; i++) // TODO: check maximum variable
		_gameStateVars[i] = 0;

	for (auto &it : _areaMap)
		it._value->resetArea();

	_gameStateBits = 0;

	_playerHeightNumber = 1;
	_playerHeight = _playerHeights[_playerHeightNumber];
	removeTimers();
	startCountdown(_initialCountdown);
	_lastMinute = 0;
	_demoIndex = 0;
	_demoEvents.clear();

	_gameStateVars[k8bitVariableEnergy] = _initialEnergy;
	_gameStateVars[k8bitVariableShield] = _initialShield;
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

	if (_currentArea->_skyColor > 0 && _currentArea->_skyColor != 255) {
		_gfx->_keyColor = 0;
	} else
		_gfx->_keyColor = 255;

	swapPalette(areaID);
	_currentArea->_usualBackgroundColor = isCPC() ? 1 : 0;

	resetInput();
}

void EclipseEngine::drawBackground() {
	clearBackground();
	_gfx->drawBackground(_currentArea->_skyColor);
	if (_currentArea && _currentArea->getAreaID() == 1) {
		_gfx->drawEclipse(15, 10);
	}
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
