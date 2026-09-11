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

#include "igor/igor.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "graphics/cursorman.h"

namespace Igor {

static const uint8 kSentenceColorIndex[] = { 0xFD, 0xFB, 0xF1 };

void IgorEngine::waitForTimer(int ticks) {
	_system->copyRectToScreen(_screenVGA, 320, 0, _screenVGAVOffset, 320, 200 - _screenVGAVOffset);
	_system->updateScreen();

	uint32 endTicks = (ticks == -1) ? _nextTimer : _system->getMillis() + ticks * 1000 / kTickDelay;
	do {
		Common::Event ev;
		while (_eventMan->pollEvent(ev)) {
			switch (ev.type) {
			case Common::EVENT_QUIT:
				_inputVars[kInputEscape] = 1;
				_currentPart = kInvalidPart;
				_eventQuitGame = true;
				break;
						case Common::EVENT_KEYDOWN:
							if (ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
								_inputVars[kInputEscape] = 1;
							} else if (ev.kbd.keycode == Common::KEYCODE_SPACE) {
								_inputVars[kInputOptions] = 1;
							} else if (ev.kbd.keycode == Common::KEYCODE_p) {
								_inputVars[kInputPause] = 1;
							}
							// else if (ev.kbd.keycode == Common::KEYCODE_F11) {
							// 	sprintf(_saveStateDescriptions[kQuickSaveSlot], "Quicksave part %d", _currentPart);
							// 	saveGameState(kQuickSaveSlot);
							// } else if (ev.kbd.keycode == Common::KEYCODE_F12) {
							// 	loadGameState(kQuickSaveSlot);
							// }
							break;
						case Common::EVENT_MOUSEMOVE:
							_inputVars[kInputCursorXPos] = ev.mouse.x;
							_inputVars[kInputCursorYPos] = ev.mouse.y;
							break;
						case Common::EVENT_RBUTTONDOWN:
							_inputVars[kInputSkipDialogue] = 1;
							break;
						case Common::EVENT_LBUTTONDOWN:
							_inputVars[kInputClick] = 1;
							_inputVars[kInputCursorXPos] = ev.mouse.x;
							_inputVars[kInputCursorYPos] = ev.mouse.y;
							break;
			default:
				break;
			}
		}
		_system->delayMillis(10);
		if (_system->getMillis() >= endTicks) {
			break;
		}

	} while (true);
	_nextTimer = _system->getMillis() + kTimerTicksCount * 1000 / kTickDelay;
	if (ticks != -1) {
		return;
	}
	_gameTicks += kTimerTicksCount;
	if ((_gameTicks & 31) == 0) {
		setCursor(_currentCursor);
		_currentCursor = (_currentCursor + 1) & 3;
	}
	// if (_game.flags & kFlagFloppy) {
	// 	updateMusic();
	// }
	if (_gameTicks == 64) {
		_gameTicks = 0;
	}
}

void IgorEngine::setCursor(int num) {
	uint8 cursor[11 * 11];
	memset(cursor, 0, 11 * 11);
	const uint8 *mask = &_mouseCursorMask[num * 24];
	for (int i = 0; i < 24; ++i) {
		const int offset = ((int8)_mouseCursorData[i + 24] + 5) * 11 + ((int8)_mouseCursorData[i] + 5);
		if (mask[i]) {
			cursor[offset] = 255;
		}
	}
	CursorMan.replaceCursor(cursor, 11, 11, 5, 5, 0);
}

void IgorEngine::showCursor() {
	debugC(9, kDebugEngine, "showCursor()");
	_roomCursorOn = true;
	CursorMan.showMouse(_roomCursorOn);
}

void IgorEngine::hideCursor() {
	debugC(9, kDebugEngine, "hideCursor()");
	_roomCursorOn = false;
	CursorMan.showMouse(_roomCursorOn);

}

void IgorEngine::drawVerbsPanel() {
	memcpy(_screenVGA + 320 * 156, _verbsPanelBuffer, 320 * 12);
}

void IgorEngine::redrawVerb(uint8 verb, bool highlight) {
	uint8 verbBitmap[44 * 12];
	if (verb >= 2 && verb <= 8) {
		verb -= 2;
		for (int i = 0; i <= 11; ++i) {
			for (int j = 0; j <= 43; ++j) {
				uint8 color = _verbsPanelBuffer[i * 320 + verb * 46 + j];
				if (highlight && color != 0) {
					color += 8;
				}
				verbBitmap[i * 44 + j] = color;
			}
		}
		for (int i = 0; i <= 11; ++i) {
			memcpy(_screenVGA + 320 * 156 + verb * 46 + i * 320, verbBitmap + i * 44, 44);
		}
	}
}

void IgorEngine::handleRoomInput() {
	if (_inputVars[kInputPause]) {
		_inputVars[kInputPause] = 0;
		// handlePause();
	}
	if (_inputVars[kInputOptions]) {
		_inputVars[kInputOptions] = 0;
		// handleOptionsMenu();
	}
	if (_inputVars[kInputSkipDialogue] && _gameState.dialogueTextRunning) {
		// _talkDelayCounter = _talkDelay;
		// if (_gameState.talkMode != kTalkModeTextOnly && _talkSpeechCounter > 2) {
		// 	stopSound();
		// 	_talkSpeechCounter = -1;
		// }
		// _inputVars[kInputSkipDialogue] = 0;
	}

	if (!_roomCursorOn || _gameState.dialogueTextRunning /*|| _scrollInventory*/) {
		return;
	}
	if (_inputVars[kInputCursorYPos] >= 156 && _inputVars[kInputCursorYPos] <= 167) {
		if (_inputVars[kInputClick]) {
			int verb = getVerbUnderCursor(_inputVars[kInputCursorXPos]);
			if (verb != _currentAction.verb) {
				redrawVerb(_currentAction.verb, false);
				_currentAction.verb = verb;
				redrawVerb(_currentAction.verb, true);
			}
			// reset action command
			memset(&_currentAction, 0, sizeof(_currentAction));
			_currentAction.verb = verb;
			_currentAction.verbType = 0;
			formatActionSentence(0);
			_inputVars[kInputClick] = 0;
		}
		return;
	}
	// if (_inputVars[kInputCursorYPos] >= 172 && _inputVars[kInputCursorYPos] <= 183 && (_inputVars[kInputCursorXPos] < 15 || _inputVars[kInputCursorXPos] > 304)) {
	// 	if (_inputVars[kInputClick]) {
	// 		if (_inventoryInfo[72] > 1) {
	// 			_inventoryInfo[72] -= 7;
	// 			drawInventory(_inventoryInfo[72], 2);
	// 		}
	// 		_inputVars[kInputClick] = 0;
	// 	}
	// 	return;
	// }
	// if (_inputVars[kInputCursorYPos] >= 186 && _inputVars[kInputCursorYPos] <= 197 && (_inputVars[kInputCursorXPos] < 15 || _inputVars[kInputCursorXPos] > 304)) {
	// 	if (_inputVars[kInputClick]) {
	// 		if (_inventoryInfo[73] > _inventoryInfo[72] + 6) {
	// 			_inventoryInfo[72] += 7;
	// 			drawInventory(_inventoryInfo[72], 1);
	// 		}
	// 		_inputVars[kInputClick] = 0;
	// 	}
	// 	return;
	// }

	// if (_inputVars[kInputClick]) {
	// 	if (_gameState.igorMoving) {
	// 		_walkDataCurrentPosX = _walkData[_walkDataCurrentIndex - 1].x;
	// 		_walkDataCurrentPosY = _walkData[_walkDataCurrentIndex - 1].y;
	// 		if (_roomObjectAreasTable[_screenLayer2[_walkDataCurrentPosY * 320 + _walkDataCurrentPosX].area == 0) {
	// 			return;
	// 		}
	// 		_walkDataCurrentPosX = _walkData[_walkDataCurrentIndex + 1].x;
	// 		_walkDataCurrentPosY = _walkData[_walkDataCurrentIndex + 1].y;
	// 		if (_roomObjectAreasTable[_screenLayer2[_walkDataCurrentPosY * 320 + _walkDataCurrentPosX].area == 0) {
	// 			return;
	// 		}
	// 	}
	// 	_inputVars[kInputClick] = 0;
	// }
	bool actionHovering = !_inputVars[kInputClick];
	_inputVars[kInputClick] = 0;

	if (actionHovering && _actionCode != 0) {
		return;
	}

	if (_inputVars[kInputCursorYPos] >= 170 && _inputVars[kInputCursorYPos] <= 199) {

	} else if (_inputVars[kInputCursorYPos] < 144) {
		int area = _screenLayer2[_inputVars[kInputCursorYPos] * 320 + _inputVars[kInputCursorXPos]];
		int object = _roomObjectAreasTable[area].object;
		if (_currentAction.verbType == 0) {
			_currentAction.object1Num = object;
			_currentAction.object1Type = kObjectTypeRoom;
			// if (_currentAction.verb == kVerbUse && _roomActionsTable[_roomDataOffsets.action.useVerb + 48 + _currentAction.object1Num] != 0) {
			// 	formatActionSentence(0);
			// 	if (!actionHovering) {
			// 		_currentAction.verbType = 1;
			// 	}
			// 	return;
			// }
			// if (_currentAction.verb == kVerbGive && _roomActionsTable[_roomDataOffsets.action.giveVerb + 48 + _currentAction.object1Num] != 0) {
			// 	formatActionSentence(0);
			// 	if (!actionHovering) {
			// 		_currentAction.verbType = 2;
			// 	}
			// 	return;
			// }
		} else {
			_currentAction.object2Num = object;
			_currentAction.object2Type = kObjectTypeRoom;
		}
	}
	else {
		return;
	}

	// if (_currentAction.verbType == 0) {
	// 	if (_currentAction.object1Type == kObjectTypeInventory) {
	// 		_actionCode = _inventoryActionsTable[(_currentAction.verb - 1) * 2 + _currentAction.object1Num * 20];
	// 	} else {
	// 		_actionCode = _roomActionsTable[_roomDataOffsets.action.defaultVerb + _currentAction.verb * 2 + _currentAction.object1Num * 20];
	// 	}
	// }

	if (_currentAction.verbType == 1) {
		int offset = _roomActionsTable[_roomDataOffsets.action.object2 + _currentAction.object2Num + _currentAction.object2Type * 38] * 2;
		offset += _roomActionsTable[_roomDataOffsets.action.object1 + _currentAction.object1Num + _currentAction.object1Type * 38] * _roomDataOffsets.action.objectSize;
		_actionCode = _roomActionsTable[_roomDataOffsets.action.useVerb + offset];
	}
	if (_currentAction.verbType == 2) {
		int offset = _roomActionsTable[_roomDataOffsets.action.object2 + _currentAction.object2Num + _currentAction.object2Type * 38] * 2;
		offset += _roomActionsTable[_roomDataOffsets.action.object1 + _currentAction.object1Num + _currentAction.object1Type * 38] * _roomDataOffsets.action.objectSize;
		_actionCode = _roomActionsTable[_roomDataOffsets.action.giveVerb + offset];
	}

	if (actionHovering) {
		formatActionSentence(0);
		_currentAction.object2Num = 0;
		_actionCode = 0;
		return;
	}

}

void IgorEngine::formatActionSentence(uint8 color) {
	char actionSentence[512];
	if (_currentAction.verb == kVerbWalk && _inputVars[kInputCursorYPos] > 143) {
		_currentAction.object1Num = 0;
	}

	Common::strlcpy(actionSentence, _verbsName[_currentAction.verb], sizeof(actionSentence));
	if (_currentAction.object1Num != 0) {
		if (_currentAction.object1Type == kObjectTypeInventory) {
			Common::strlcat(actionSentence, _globalObjectNames[_currentAction.object1Num], sizeof(actionSentence));
		} else {
			Common::strlcat(actionSentence, _roomObjectNames[_currentAction.object1Num], sizeof(actionSentence));
		}
		if (_currentAction.verbType != 0) {
			Common::strlcat(actionSentence, _verbPrepositions[_currentAction.verbType], sizeof(actionSentence));
			if (_currentAction.object2Num != 0) {
				if (_currentAction.object2Type == kObjectTypeInventory) {
					Common::strlcat(actionSentence, _globalObjectNames[_currentAction.object2Num], sizeof(actionSentence));
				} else {
					Common::strlcat(actionSentence, _roomObjectNames[_currentAction.object2Num], sizeof(actionSentence));
				}
			}
		}
	}
	drawActionSentence(actionSentence, kSentenceColorIndex[color]);
}

void IgorEngine::drawActionSentence(const char *sentence, uint8 color) {
	memset(_screenVGA + 144 * 320, 0, 11 * 320);
	int w = _font.getStringWidth(sentence);
	int x = (320 - w) / 2;
	_font.drawString(_screenVGA, sentence, x, 144, color, 0, 0);
}

} // End of namespace Igor

