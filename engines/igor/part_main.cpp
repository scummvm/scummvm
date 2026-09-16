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

namespace Igor {
void IgorEngine::EXEC_MAIN_ACTION(int action) {
	switch (action) {
	case 0:
	case 1:
        break;
    default:
		warning("EXEC_MAIN_ACTION() Unhandled action %d", action);
		break;
    }
}

void IgorEngine::SET_EXEC_ACTION_FUNC(int i, ExecuteActionProc p) {
	switch (i) {
	case 0:
		_executeMainAction = p;
		break;
	case 1:
		_executeRoomAction = p;
		break;
	}
}

void IgorEngine::PART_MAIN() {
	SET_EXEC_ACTION_FUNC(0, &IgorEngine::EXEC_MAIN_ACTION);
	memset(_objectsState, 0, 112);
	_objectsState[21] = 1;
	_objectsState[49] = 1;

    // UPDATE_OBJECT_STATE(255);
    if (_currentPart != kStartupPart) { // boot param
		SET_PAL_208_96_1();
		SET_PAL_240_48_1();
		drawVerbsPanel();
		// drawInventory(1, 0);
	}
    do {
        debugC(9, kDebugGame, "PART_MAIN _currentPart %d", _currentPart);
		switch (_currentPart) {

        case 50:
        case 51:
        case 52:
            PART_05();// SpringRock
            break;
		case 60:
		case 61:
		case 62:
			PART_06();
			break;

		case 850: // Intro cutscene
            // Clear the entire screen buffer before starting the intro cutscene
            memset(_screenVGA, 0, 64000);
            _screenVGAVOffset = 24;
            // copy the initial portion of the screen to the display, with the vertical offset
			_system->copyRectToScreen(_screenVGA, 320, 0, 0, 320, _screenVGAVOffset);
			PART_85();
            // Clears the bottom portion of the screen
            memset(_screenVGA + 46080, 0, 17920);
            _nextTimer = _system->getMillis() + 1000 / 60;
			for (int y = _screenVGAVOffset; y >= 0; --y) {
				_system->copyRectToScreen(_screenVGA, 320, 0, y, 320, 145);
				_system->updateScreen();
				int diff = _nextTimer - _system->getMillis();
				if (diff > 0) {
					_system->delayMillis(diff);
				}
				_nextTimer = _system->getMillis() + 1000 / 60;
			}
            _screenVGAVOffset = 0;
			_inputVars[kInputCursorXPos] = 160;
			_inputVars[kInputCursorYPos] = 72;
			_system->warpMouse(_inputVars[kInputCursorXPos], _inputVars[kInputCursorYPos]);
			break;
		case 900: // Logo slideshow
		case 901:
		case 902:
		case 903:
		case 904:
			PART_90();
			break;

		default:
			error("PART_MAIN() Unhandled part %d", _currentPart);
			break;
		}
	} while (_currentPart != kInvalidPart && !_eventQuitGame);
}

} // End of namespace Igor
