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

#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "graphics/cursorman.h"

#include "backends/keymapper/keymapper.h"

#include "igor/igor.h"

namespace Igor {

// Timer ticks elapsed since [startMillis], at the rate of the original timer (scaled by fast mode).
uint32 IgorEngine::ticksSince(uint32 startMillis) const {
	return (uint32)((uint64)(_system->getMillis() - startMillis) * kTickDelay * _fastMode / 1000);
}

void IgorEngine::waitForTimer(int ticks, bool presentScreen, uint32 deadline) {

	if (presentScreen && !_eventQuitGame) {
		flushScreenToBackend();

		_system->updateScreen();
	}

	// wait till next timer if no param is passed, [ticks] if not
	// _fastMode divides the wall-clock deadlines.
	uint32 endTicks = deadline ? deadline : ((ticks == -1) ? _nextTimer : _system->getMillis() + ticks * 1000 / kTickDelay / _fastMode);
	do {
		updateMusic();
		Common::Event ev;
		while (!_eventQuitGame && _eventMan->pollEvent(ev)) {
			switch (ev.type) {
			case Common::EVENT_RETURN_TO_LAUNCHER:
			case Common::EVENT_QUIT:
				_inputVars[kInputEscape] = 1;
				_currentPart = kInvalidPart;
				_eventQuitGame = true;
				cancelDialogue();
				break;
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				if (ev.customType == kActionFastMode) {
					_fastMode = (_fastMode == 1) ? kFastModeFactor : 1;
					debugC(1, kDebugEngine, "fast mode %d (%d ms per tick)",
					       _fastMode, kTimerTicksCount * 1000 / kTickDelay / _fastMode);
				} else if (ev.customType == kActionEscape) {
					_inputVars[kInputEscape] = 1;
				} else if (_game.version == kIdSpaFloppy && moveCursorWithKeyboard(ev.customType)) {
					// the cursor was moved with the arrow/keypad keys
				} else if (_game.version == kIdSpaFloppy && ev.customType == kActionKeyRightClick) {
					// Insert acts as the right button
					if (!_gameState.dialogueTextRunning && !ev.kbdRepeat) {
						_inputVars[kInputRightRelease] = 0;
						_inputVars[kInputRightClick] = 1;
					}
				} else if (ev.customType == kActionConfirm) {
					_inputVars[kInputConfirm] = 1;
					// Enter acts as the left button at the cursor position (first press only)
					if (_game.version == kIdSpaFloppy && !_gameState.dialogueTextRunning && !ev.kbdRepeat) {
						_inputVars[kInputClick] = 1;
					}
				} else if (ev.customType == kActionOptions) {
					_inputVars[kInputOptions] = 1;
				} else if (ev.customType == kActionPause) {
					_inputVars[kInputPause] = 1;
				}
				break;
			case Common::EVENT_CUSTOM_ENGINE_ACTION_END:
				if (_game.version == kIdSpaFloppy && isCursorKey(ev.customType)) {
					_keyboardCursorStep = 1;
				}
				if (ev.customType == kActionConfirm) {
					_inputVars[kInputConfirm] = 0;
				}
				break;
			case Common::EVENT_MOUSEMOVE:
				if (_rightButtonSelecting) {
					// righ button selects a verb in the panel
					if (ev.mouse.x < _rightButtonSelectCursorX) {
						_inputVars[kInputRightMoveLeft] = 1;
					} else if (ev.mouse.x > _rightButtonSelectCursorX) {
						_inputVars[kInputRightMoveRight] = 1;
					}
					_system->warpMouse(_rightButtonSelectCursorX, _rightButtonSelectCursorY);
				} else {
					_inputVars[kInputCursorXPos] = ev.mouse.x;
					_inputVars[kInputCursorYPos] = ev.mouse.y;
				}
				break;
			case Common::EVENT_RBUTTONDOWN:
				debugC(9, kDebugEngine, "waitForTimer() right button at %d,%d running %d",
				       ev.mouse.x, ev.mouse.y, _gameState.dialogueTextRunning);
				if (_gameState.dialogueTextRunning) {
					_inputVars[kInputSkipDialogue] = 1;
				} else {
					_inputVars[kInputCursorXPos] = ev.mouse.x;
					_inputVars[kInputCursorYPos] = ev.mouse.y;
					_inputVars[kInputRightRelease] = 0;
					_inputVars[kInputRightClick] = 1;
				}
				break;
			case Common::EVENT_RBUTTONUP:
				_inputVars[kInputRightRelease] = 1;
				break;
			case Common::EVENT_LBUTTONDOWN:
				debugC(9, kDebugEngine, "waitForTimer() left button at %d,%d running %d",
				       ev.mouse.x, ev.mouse.y, _gameState.dialogueTextRunning);
				_inputVars[kInputCursorXPos] = ev.mouse.x;
				_inputVars[kInputCursorYPos] = ev.mouse.y;
				if (_gameState.dialogueTextRunning) {
					// Dialogue dismissal and room clicks are separate DOS flags;
					// consuming this event here prevents it reaching the room loop.
					_inputVars[kInputClick] = 0;
					_inputVars[kInputSkipDialogue] = 1;
				} else {
					_inputVars[kInputClick] = 1;
				}
				break;
			default:
				break;
			}
		}
		// Keep advancing game ticks so scripted animation loops can finish,
		// but do not wait or present another frame during shutdown.
		if (_eventQuitGame)
			break;
		if (_rightButtonSelecting && !(_eventMan->getButtonState() & Common::EventManager::RBUTTON)) {
			_inputVars[kInputRightRelease] = 1;
		}
		const uint32 now = _system->getMillis();
		if (now >= endTicks) {
			break;
		}
		// Do not overshoot short explicit DOS-timer waits. One DOS timer unit
		// is 1000 / kTickDelay ms; if requested delay is smaller then respect that
		const uint32 pollDelay = MAX(1, 10 / _fastMode);
		_system->delayMillis(MIN<uint32>(pollDelay, endTicks - now));

	} while (true);
	_nextTimer = _system->getMillis() + kTimerTicksCount * 1000 / kTickDelay / _fastMode;
	if (ticks != -1) {
		return;
	}
	// Splash screens advance one tick at a time. Resume the aligned timer
	// steps used by the animation and room-loop checks.
	_gameTicks += kTimerTicksCount - _gameTicks % kTimerTicksCount;

	// Update cursor mask
	if ((_gameTicks & 31) == 0 && !_eventQuitGame) {
		setCursor(_currentCursor);
		_currentCursor = (_currentCursor + 1) & 3;
	}
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

// Default verb of the room object under a scene position.
int IgorEngine::getDefaultVerbAt(int x, int y) const {
	const int area = _screenLayer2[y * 320 + x];
	const int object = _roomObjectAreasTable[area].object;
	const int defaultVerbOffset = _roomDataOffsets.obj.walkPoints +
		_roomDataOffsets.obj.walkFacingPosition - _roomDataOffsets.action.defaultVerb;
	return _roomActionsTable[defaultVerbOffset + object];
}

/**
 * Action to select a verb with moving the right mouse button clicked
 */
void IgorEngine::beginRightButtonVerbPick() {
	_rightButtonSelecting = true;
	_rightButtonSelectCursorX = _inputVars[kInputCursorXPos];
	_rightButtonSelectCursorY = _inputVars[kInputCursorYPos];
	_inputVars[kInputRightMoveLeft] = 0;
	_inputVars[kInputRightMoveRight] = 0;

	const bool inventorySlot = _rightButtonSelectCursorY >= 170 &&
		((_game.version == kIdSpaFloppy) ? (_rightButtonSelectCursorY <= 197) :
		 (_rightButtonSelectCursorY <= 199 && _rightButtonSelectCursorX > 19 && _rightButtonSelectCursorX < 299));
	int defaultVerb = 0;
	// if in the scene, choose the hotspot's default verb as starting point
	if (_rightButtonSelectCursorY < 144) {
		defaultVerb = getDefaultVerbAt(_rightButtonSelectCursorX, _rightButtonSelectCursorY);
	} else if (inventorySlot) { // for items, look is default
		defaultVerb = kVerbLook;
	}
	if (defaultVerb < 2 && _currentAction.verb == kVerbWalk) {
		defaultVerb = kVerbUse;
	}
	// if in inventory area but no item is below the mouse, choose use
	if (inventorySlot && getObjectFromInventory(_rightButtonSelectCursorX) == 0 && defaultVerb == kVerbLook) {
		defaultVerb = (_currentAction.verb == kVerbWalk) ? (int)kVerbUse : (int)_currentAction.verb;
	}
	const int previousVerb = _currentAction.verb;
	if (defaultVerb > 0) {
		_currentAction.verb = defaultVerb;
	}
	if (previousVerb != _currentAction.verb) {
		// redraw the selected verb, and the previous one
		redrawVerb(previousVerb, false);
		redrawVerb(_currentAction.verb, true);
	}
	_currentAction.verbType = 0;
	_currentAction.object2Num = 0;
	_currentAction.object2Type = 0;
	// set the current action's object1 based on the cursor position
	if (inventorySlot) {
		_currentAction.object1Num = getObjectFromInventory(_rightButtonSelectCursorX);
		_currentAction.object1Type = kObjectTypeInventory;
	} else if (_rightButtonSelectCursorY < 144) {
		const int area = _screenLayer2[_rightButtonSelectCursorY * 320 + _rightButtonSelectCursorX];
		_currentAction.object1Num = _roomObjectAreasTable[area].object;
		_currentAction.object1Type = kObjectTypeRoom;
	}
	formatRightButtonSentence();
}

void IgorEngine::updateRightButtonVerbPick() {
	if (_inputVars[kInputRightMoveLeft]) {
		_inputVars[kInputRightMoveLeft] = 0;
		const int candidate = MAX<int>(_currentAction.verb - 1, (_game.version == kIdSpaFloppy) ? kVerbWalk : kVerbTalk);
		if (candidate != _currentAction.verb) {
			redrawVerb(_currentAction.verb, false);
			redrawVerb(candidate, true);
			_currentAction.verb = candidate;
			formatRightButtonSentence();
		}
	}
	if (_inputVars[kInputRightMoveRight]) {
		_inputVars[kInputRightMoveRight] = 0;
		const int candidate = MIN<int>(_currentAction.verb + 1, kVerbGive);
		if (candidate == _currentAction.verb) {
			return;
		}
		redrawVerb(_currentAction.verb, false);
		redrawVerb(candidate, true);
		_currentAction.verb = candidate;
		formatRightButtonSentence();
	}
}

void IgorEngine::endRightButtonVerbPick() {
	_rightButtonSelecting = false;
	_inputVars[kInputRightMoveLeft] = 0;
	_inputVars[kInputRightMoveRight] = 0;
	_inputVars[kInputClick] = 0;
	_inputVars[kInputCursorXPos] = _rightButtonSelectCursorX;
	_inputVars[kInputCursorYPos] = _rightButtonSelectCursorY;
	//return mouse to the position the right click happened
	_system->warpMouse(_rightButtonSelectCursorX, _rightButtonSelectCursorY);
}

/**
 * Formats the sentence displayed when the right mouse button is used to select a verb.
 */
void IgorEngine::formatRightButtonSentence() {
	const int object1Num = _currentAction.object1Num;
	uint8 actionCode = 0;
	if (object1Num != 0) {
		if (_currentAction.object1Type == kObjectTypeInventory) {
			actionCode = _inventoryActionsTable[(_currentAction.verb - 1) * 2 + object1Num * 20];
		} else {
			actionCode = _roomActionsTable[_roomDataOffsets.action.defaultVerb + _currentAction.verb * 2 + object1Num * 20];
		}
	}
	// Without an action byte the sentence names only the verb.
	if (actionCode == 0) {
		_currentAction.object1Num = 0;
		formatActionSentence(0);
		_currentAction.object1Num = object1Num;
	} else {
		formatActionSentence(0);
	}
}

bool IgorEngine::isCursorKey(int action) const {
	return action >= kActionUp && action <= kActionDownRight;
}

void IgorEngine::setKeymapperEnabled(bool enabled) {
	_eventMan->getKeymapper()->setEnabled(enabled);
	if (enabled) {
		// the release of a key pressed before the suspension was not seen as an action end
		_inputVars[kInputConfirm] = 0;
	}
}

// Arrow and keypad keys move the cursor by a step that grows with every key
// press (key repeat included) and returns to 1 when a key is released.
bool IgorEngine::moveCursorWithKeyboard(int action) {
	if (!isCursorKey(action))
		return false;
	int dx = 0;
	int dy = 0;
	switch (action) {
	case kActionUp: dy = -1; break;
	case kActionDown: dy = 1; break;
	case kActionLeft: dx = -1; break;
	case kActionRight: dx = 1; break;
	case kActionUpLeft: dx = -1; dy = -1; break;
	case kActionUpRight: dx = 1; dy = -1; break;
	case kActionDownLeft: dx = -1; dy = 1; break;
	default: dx = 1; dy = 1; break;
	}
	const int step = _keyboardCursorStep++;
	const int x = CLIP<int>(_inputVars[kInputCursorXPos] + dx * step, 0, 319);
	const int y = CLIP<int>(_inputVars[kInputCursorYPos] + dy * step, 0, 199);
	_inputVars[kInputCursorXPos] = x;
	_inputVars[kInputCursorYPos] = y;
	_system->warpMouse(x, y);
	return true;
}

/**
 * The Spanish floppy highlights, while the cursor is over a hotspot, the default verb of that hotspot
 * (Look for inventory objects) and shows the selected verb again when the cursor leaves.
 */
void IgorEngine::updateHoverVerbHighlight() {
	int wanted = kHoverVerbSelected;
	const bool idle = !_rightButtonSelecting && !_gameState.dialogueTextRunning && !_inputVars[kInputClick];
	const int x = _inputVars[kInputCursorXPos];
	const int y = _inputVars[kInputCursorYPos];
	if (idle) {
		if (y >= 0 && y < 144 && x >= 0 && x < 320) {
			wanted = getDefaultVerbAt(x, y);
		} else if (y >= 170 && y <= 197) {
			wanted = kVerbLook;
		}
	}
	if (wanted == _hoverVerbState)
		return;
	if (_hoverVerbState == kHoverVerbSelected)
		redrawVerb(_currentAction.verb, false);
	else
		redrawVerb(_hoverVerbState, false);
	if (wanted == kHoverVerbSelected)
		redrawVerb(_currentAction.verb, true);
	else
		redrawVerb(wanted, true);
	_hoverVerbState = wanted;
}

void IgorEngine::handleRoomInput() {
	// Escape is a transient abort input. Do not let an unused press from the
	// room loop leak into a later blocking walk or dialogue sequence.
	if (_inputVars[kInputEscape] && !_eventQuitGame) {
		_inputVars[kInputEscape] = 0;
	}

	if (_inputVars[kInputPause]) {
		_inputVars[kInputPause] = 0;
		handlePause();
	}
	if (_inputVars[kInputOptions]) {
		_inputVars[kInputOptions] = 0;
		handleOptionsMenu();
	}
	// dialogue skip
	if (_inputVars[kInputSkipDialogue] && _gameState.dialogueTextRunning) {
		_talkDelayCounter = _talkDelay;
		if (_talkDelay < 0 && _talkSpeechCounter > 2) {
			stopSound();
			_talkSpeechCounter = -1;
		}
		_inputVars[kInputSkipDialogue] = 0;
		// The click that dismissed the dialogue must not survive into the room handler
		_inputVars[kInputClick] = 0;
	}
	if (_rightButtonSelecting && _inputVars[kInputRightRelease]) {
		_inputVars[kInputRightRelease] = 0;
		endRightButtonVerbPick();
	}

	if (!_roomCursorOn || _gameState.dialogueTextRunning || _scrollInventory) {
		return;
	}
	if (_game.version == kIdSpaFloppy) {
		updateHoverVerbHighlight();
	}
	// Spanish floppy inventory paging works with left and right mouse buttons pressed on the 2 last rows of the screen
	if (_game.version == kIdSpaFloppy && !_rightButtonSelecting && _inputVars[kInputCursorYPos] >= 198) {
		if (_inputVars[kInputClick])
			pageFloppyInventory(false);
		if (_inputVars[kInputRightClick])
			pageFloppyInventory(true);
		_inputVars[kInputClick] = 0;
		_inputVars[kInputRightClick] = 0;
		_inputVars[kInputRightRelease] = 0;
		return;
	}
	if (_game.version == kIdSpaFloppy && _inputVars[kInputRightClick]) {
		// Right click runs the hovered object's default verb right away
		// (Look for inventory items) instead of opening a verb pick.
		_inputVars[kInputRightClick] = 0;
		const int x = _inputVars[kInputCursorXPos];
		const int y = _inputVars[kInputCursorYPos];
		int verb = 0;
		if (y < 144) {
			verb = getDefaultVerbAt(x, y);
		} else if (y >= 170 && y <= 197 && getObjectFromInventory(x) != 0) {
			verb = kVerbLook;
		}
		if (verb >= kVerbWalk && verb <= kVerbGive) {
			redrawVerb(_hoverVerbState == kHoverVerbSelected ? _currentAction.verb : _hoverVerbState, false);
			memset(&_currentAction, 0, sizeof(_currentAction));
			_currentAction.verb = verb;
			_hoverVerbState = verb;
			redrawVerb(verb, true);
			_inputVars[kInputClick] = 1;
		}
	} else if (_inputVars[kInputRightClick]) {
		_inputVars[kInputRightClick] = 0;
		if (!_rightButtonSelecting) {
			beginRightButtonVerbPick();
		}
	}
	if (_rightButtonSelecting) {
		if (_inputVars[kInputRightRelease]) {
			_inputVars[kInputRightRelease] = 0;
			endRightButtonVerbPick();
		} else {
			updateRightButtonVerbPick();
		}
		return;
	}
	_inputVars[kInputRightRelease] = 0;
	// verbs panel click
	if (_inputVars[kInputCursorYPos] >= 156 && _inputVars[kInputCursorYPos] <= ((_game.version == kIdSpaFloppy) ? 166 : 167)) {
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
	// Inventory arrows
	if (_game.version != kIdSpaFloppy && _inputVars[kInputCursorYPos] >= 172 && _inputVars[kInputCursorYPos] <= 183 && (_inputVars[kInputCursorXPos] < 15 || _inputVars[kInputCursorXPos] > 304)) {
		if (_inputVars[kInputClick]) {
			if (_inventoryInfo[kInventoryFirstVisible] > 1) {
				_inventoryInfo[kInventoryFirstVisible] -= 7;
				drawInventory(_inventoryInfo[kInventoryFirstVisible], 2);
			}
			_inputVars[kInputClick] = 0;
		}
		return;
	}
	// Inventory arrows
	if (_game.version != kIdSpaFloppy && _inputVars[kInputCursorYPos] >= 186 && _inputVars[kInputCursorYPos] <= 197 && (_inputVars[kInputCursorXPos] < 15 || _inputVars[kInputCursorXPos] > 304)) {
		if (_inputVars[kInputClick]) {
			if (_inventoryInfo[kInventoryCount] > _inventoryInfo[kInventoryFirstVisible] + 6) {
				_inventoryInfo[kInventoryFirstVisible] += 7;
				drawInventory(_inventoryInfo[kInventoryFirstVisible], 1);
			}
			_inputVars[kInputClick] = 0;
		}
		return;
	}

	// If current area results in zero just ignore click
	if (_inputVars[kInputClick] && _gameState.igorMoving) {
		_walkDataCurrentPosX = _walkData[_walkDataCurrentIndex - 1].x;
		_walkDataCurrentPosY = _walkData[_walkDataCurrentIndex - 1].y;
		if (_roomObjectAreasTable[_screenLayer2[_walkDataCurrentPosY * 320 + _walkDataCurrentPosX]].area == 0) {
			return;
		}
		_walkDataCurrentPosX = _walkData[_walkDataCurrentIndex + 1].x;
		_walkDataCurrentPosY = _walkData[_walkDataCurrentIndex + 1].y;
		if (_roomObjectAreasTable[_screenLayer2[_walkDataCurrentPosY * 320 + _walkDataCurrentPosX]].area == 0) {
			return;
		}
	}

	bool actionHovering = !_inputVars[kInputClick];
	_inputVars[kInputClick] = 0;

	if (actionHovering && _actionCode != 0) {
		return;
	}
	// checks whether the specified verb-object pair action is available
	// and thus doesnt move to the "use A with B" state.
	auto hasPairAction = [this](int verbOffset, int objectNum, int objectType) {
		const int offset = verbOffset +
			(_roomDataOffsets.action.object1 - _roomDataOffsets.action.useVerb) +
			objectType * 38 + objectNum;
		return _roomActionsTable[offset] != 0;
	};

	// Action previousAction = _currentAction;
	// Handle inventory and room object selection based on cursor position and clicks
	if (_inputVars[kInputCursorYPos] >= 170 && _inputVars[kInputCursorYPos] <= ((_game.version == kIdSpaFloppy) ? 197 : 199)) {
		int object = getObjectFromInventory(_inputVars[kInputCursorXPos]);
		if (_currentAction.verbType == 0) {
			_currentAction.object1Num = object;
			_currentAction.object1Type = kObjectTypeInventory;
			if (_currentAction.verb == kVerbUse && hasPairAction(_roomDataOffsets.action.useVerb, _currentAction.object1Num, kObjectTypeInventory)) {
				formatActionSentence(0);
				if (!actionHovering) {
					_currentAction.verbType = 1;
				}
				return;
			}
			if (_currentAction.verb == kVerbGive && hasPairAction(_roomDataOffsets.action.giveVerb, _currentAction.object1Num, kObjectTypeInventory)) {
				formatActionSentence(0);
				if (!actionHovering) {
					_currentAction.verbType = 2;
				}
				return;
			}
		} else {
			_currentAction.object2Num = object;
			_currentAction.object2Type = kObjectTypeInventory;
			// An object cannot be combined with itself.
			if (object == _currentAction.object1Num && _currentAction.object1Type == kObjectTypeInventory) {
				_currentAction.object2Num = 0;
			}
		}
	} else if (_inputVars[kInputCursorYPos] < 144) {
		// screen object under cursor
		int area = _screenLayer2[_inputVars[kInputCursorYPos] * 320 + _inputVars[kInputCursorXPos]];
		int object = _roomObjectAreasTable[area].object;
		if (_currentAction.verbType == 0) {
			_currentAction.object1Num = object;
			_currentAction.object1Type = kObjectTypeRoom;
			if (_currentAction.verb == kVerbUse && hasPairAction(_roomDataOffsets.action.useVerb, _currentAction.object1Num, kObjectTypeRoom)) {
				formatActionSentence(0);
				if (!actionHovering) {
					_currentAction.verbType = 1;
				}
				return;
			}
			if (_currentAction.verb == kVerbGive && hasPairAction(_roomDataOffsets.action.giveVerb, _currentAction.object1Num, kObjectTypeRoom)) {

				formatActionSentence(0);
				if (!actionHovering) {
					_currentAction.verbType = 2;
				}
				return;
			}
		} else {
			_currentAction.object2Num = object;
			_currentAction.object2Type = kObjectTypeRoom;
			if (object == _currentAction.object1Num && _currentAction.object1Type == kObjectTypeRoom) {
				_currentAction.object2Num = 0;
			}
		}
	} else {
		return;
	}

	if (_currentAction.verbType == 0) {
		if (_currentAction.object1Type == kObjectTypeInventory) {
			_actionCode = _inventoryActionsTable[(_currentAction.verb - 1) * 2 + _currentAction.object1Num * 20];
		} else {
			_actionCode = _roomActionsTable[_roomDataOffsets.action.defaultVerb + _currentAction.verb * 2 + _currentAction.object1Num * 20];
		}
		// Expose a room object when this action byte is
		// non-zero; otherwise hover text is verb-only.
		if (_actionCode == 0 && _currentAction.object1Type == kObjectTypeRoom) {
			_currentAction.object1Num = 0;
		}
	}

	int pairActionOffset = 0;
	int pairVerbOffset = 0;
	if (_currentAction.verbType != 0) {
		pairActionOffset = _roomActionsTable[_roomDataOffsets.action.object2 + _currentAction.object2Num + _currentAction.object2Type * 38] * 2;
		const int objectSize = (_currentAction.verbType == 2 && _roomGiveObjectSize != 0) ?
				_roomGiveObjectSize : _roomDataOffsets.action.objectSize;
		pairActionOffset += _roomActionsTable[_roomDataOffsets.action.object1 + _currentAction.object1Num + _currentAction.object1Type * 38] * objectSize;
		pairVerbOffset = _currentAction.verbType == 1 ? _roomDataOffsets.action.useVerb : _roomDataOffsets.action.giveVerb;
		_actionCode = _roomActionsTable[pairVerbOffset + pairActionOffset];
	}

	if (actionHovering) {
		formatActionSentence(0);
		_currentAction.object2Num = 0;
		_actionCode = 0;
		return;
	}
	debugC(9, kDebugEngine, "handleRoomInput() actionCode %d", _actionCode);
	if (_actionCode == 0) {
		debugC(9, kDebugEngine, "handleRoomInput() actionCode is 0, clearing action");
		// The Spanish floppy selects Look when the last clicked object is an inventory item, otherwise Walk.
		const int lastObjectType = _currentAction.verbType == 0 ? _currentAction.object1Type : _currentAction.object2Type;
		clearAction();
		if (_game.version == kIdSpaFloppy && lastObjectType == kObjectTypeInventory) {
			_currentAction.verb = kVerbLook;
			redrawVerb(kVerbLook, true);
		}
		return;
	}
	formatActionSentence(1);
	int walkObjectNum = 0;
	bool pairWalk = false;
	if (_currentAction.verbType == 0 && _currentAction.object1Type == kObjectTypeRoom) {
		walkObjectNum = _currentAction.object1Num;
		_actionWalkPoint = _roomActionsTable[_roomDataOffsets.action.defaultVerb + _currentAction.verb * 2 + walkObjectNum * 20 + 1];
	} else if (_currentAction.verbType != 0) {
		// Pair records are two bytes. Byte 0 is the action and byte 1 selects
		// the Action field containing the room object to approach. Values above
		// one walk; for example 5 selects object2Num.
		_actionWalkPoint = _roomActionsTable[pairVerbOffset + pairActionOffset + 1];
		if (_actionWalkPoint > 1 && _actionWalkPoint <= sizeof(Action)) {
			const uint8 *actionFields = reinterpret_cast<const uint8 *>(&_currentAction);
			walkObjectNum = actionFields[_actionWalkPoint - 1];
			pairWalk = true;
		}
	}
	if ((_currentAction.verbType == 0 && _actionWalkPoint > 0) || pairWalk) {
		if (!pairWalk && walkObjectNum == 0) {
			// no object selected, just walk
			_walkToObjectPosX = _inputVars[kInputCursorXPos];
			_walkToObjectPosY = _inputVars[kInputCursorYPos];
			if (_roomObjectAreasTable[_screenLayer2[_walkToObjectPosY * 320 + _walkToObjectPosX]].area == 0) {
				fixWalkPosition(&_walkToObjectPosX, &_walkToObjectPosY);
			}
		} else {
			// walk to object
			int offset = READ_LE_UINT16(_roomActionsTable + _roomDataOffsets.obj.walkPoints + walkObjectNum * 2);
			_walkToObjectPosX = offset % 320;
			_walkToObjectPosY = offset / 320;
			debugC(9, kDebugEngine, "handleRoomInput() walkToObject offset %d (0x%X)", offset, _roomDataOffsets.obj.walkPoints);
		}
		if (_gameState.igorMoving) {
			// stop igor at the current position
			_walkDataLastIndex = _walkDataCurrentIndex - 1;
			_walkDataCurrentPosX = _walkData[_walkDataLastIndex].x;
			_walkDataCurrentPosY = _walkData[_walkDataLastIndex].y;
			_walkCurrentFrame = _walkData[_walkDataLastIndex].frameNum;
			_walkCurrentPos = _walkData[_walkDataLastIndex].posNum;
			WalkData::setNextFrame(_walkCurrentPos, _walkCurrentFrame);
		} else {
			--_walkDataLastIndex;
			_walkDataCurrentPosX = _walkData[_walkDataLastIndex].x;
			_walkDataCurrentPosY = _walkData[_walkDataLastIndex].y;
			_walkCurrentPos = _walkData[_walkDataLastIndex].posNum;
			_walkCurrentFrame = 1;
		}
		if (_walkDataCurrentPosX != _walkToObjectPosX || _walkDataCurrentPosY != _walkToObjectPosY) {
			if (_roomDataOffsets.area.boxSize == 0) {
				buildWalkPathSimple(_walkDataCurrentPosX, _walkDataCurrentPosY, _walkToObjectPosX, _walkToObjectPosY);
			} else {
				buildWalkPath(_walkDataCurrentPosX, _walkDataCurrentPosY, _walkToObjectPosX, _walkToObjectPosY);
			}
			if (pairWalk || _actionWalkPoint != 3) {
				_walkCurrentFrame = 0;
				_walkData[_walkDataLastIndex].frameNum = 0;
			}
			if (pairWalk || _actionWalkPoint == 1) {
				_walkCurrentPos = _roomActionsTable[_roomDataOffsets.obj.walkFacingPosition + walkObjectNum];
				_walkData[_walkDataLastIndex].posNum = _walkCurrentPos;
			}
			_walkDataCurrentIndex = 1;
			_gameState.igorMoving = true;
		}
		return;
	}

	hideCursor();
	debugC(9, kDebugEngine, "handleRoomInput() executing action with actionCode %d", _actionCode);
	const int actionPart = _currentPart;
	executeAction(_actionCode);
	if (!_gameState.dialogueTextRunning) {
		showCursor();
	}
	clearAction(_currentPart == actionPart);
	return;
}

void IgorEngine::clearAction(bool redraw) {
	if (redraw) {
		if (_game.version == kIdSpaFloppy && _hoverVerbState != kHoverVerbSelected) {
			redrawVerb(_hoverVerbState, false);
			_hoverVerbState = kHoverVerbSelected;
		}
		redrawVerb(_currentAction.verb, false);
	}
	memset(&_currentAction, 0, sizeof(_currentAction));
	_currentAction.verb = kVerbWalk;
	_actionCode = 0;
	_actionWalkPoint = 0;
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

	drawActionSentence(actionSentence, (_game.version == kIdSpaFloppy) ? _actionSentenceColors[color] : _sentenceColorIndex[color]);
}

void IgorEngine::drawActionSentence(const char *sentence, uint8 color) {
	fillScreenRows(144, 11, 0);
	int w = _sentenceFont->getStringWidth(sentence);
	int x = (320 - w) / 2;
	// A sentence wider than the screen would start at a negative offset and
	// wrap into the previous scanline; keep it left-aligned and let drawString
	// stop at the right edge instead.
	if (x < 0) {
		x = 0;
	}
	_sentenceFont->drawString(_screenVGA, sentence, x, 144, color, 0, 0);
}

} // End of namespace Igor
