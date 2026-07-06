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

#include "fool/fool.h"
#include "fool/fool_game.h"
#include "fool/toolbox.h"
#include "fool/zbasic.h"

namespace Fool {

#define OFF(x) (_zstrOffset[kOffsetHermit] + (x))



// hermit key trace
void FoolGame::hermitRun() {
	// 142:12ac
	if (_puzzleCompletionStatus[0x34] < 5) {
		_zbasic->menu(8, 3, 1, _zbasic->str(362)); // to find a hidden secret
		_zbasic->menu(8, 4, 1, _zbasic->str(363)); // follow each line, correct and true
	} else {
		// 142:12fc
		_zbasic->menu(8, 3, 1, _zbasic->str(364)); // to find the straight path
		_zbasic->menu(8, 4, 1, _zbasic->str(365)); // follow each line, correct and true
	}
	// 142:132c
	if (_activePuzzleStatus == 0) {
		_hermitPathStage = 1;
	}
	if ((_activePuzzleStatus > 0) && (_activePuzzleStatus < 0x63)) {
		_hermitPathStage = _activePuzzleStatus;
	}
	// 142:1360
	if (_activePuzzleStatus == 0x63) {
		_hermitPathStage = 6;
	}
	if (_activePuzzleStatus == 0x64) {
		_hermitPathStage = 1;
	}
	if (_activePuzzleStatus > 0x64) {
		_hermitPathStage = _activePuzzleStatus - 0x64;
	}
	thothKeyLast();
	if (_hermitPathStage == 6) {
		_activePuzzleStatus = 0x63;
	} else {
		if (_activePuzzleStatus < 0x63) {
			// 142:13be
			_activePuzzleStatus = _hermitPathStage;
			if (_activePuzzleSolved) {
				_activePuzzleStatus = 0x64;
			}
		} else {
			// 142:13da
			if (_activePuzzleStatus >= 0x64) {
				_activePuzzleStatus = _hermitPathStage + 0x64;
				if (_activePuzzleSolved) {
					_activePuzzleStatus = 0x65;
				}
			}
		}
	}
	// 142:1404
}

} // End of namespace Fool
