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
	_playerHeights.push_back(16);
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
	if (isDemo())
		_currentArea->_skyColor = 27;
	_currentArea->_usualBackgroundColor = isCPC() ? 1 : 0;

	resetInput();
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
