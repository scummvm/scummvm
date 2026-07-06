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

#include "graphics/managed_surface.h"

#include "fool/fool.h"
#include "fool/fool_game.h"
#include "fool/toolbox.h"
#include "fool/zbasic.h"

namespace Fool {



void FoolGame::straightPathRun() {
	// 143:0004
	fetchPuzzleData();
	for (int16 i = 0; i <= 0xe; i++) {
		arr_i16_1eb8[i] = puzzlesReadShort();
	}
	int16 gridIndex = 0;
	int16 gridX = arr_i16_1eb8[10];
	int16 gridY = arr_i16_1eb8[8];
	do {
		gridX = arr_i16_1eb8[10];
		do {
			gridIndex++;
			_toolbox->SetRect(
				_screenGrid[gridIndex],
				gridX,
				gridY,
				gridX + arr_i16_1eb8[13] - 1,
				gridY + arr_i16_1eb8[12] - 1
			);
		} while (_zbasic->incrAndCheck(gridX, arr_i16_1eb8[11], arr_i16_1eb8[6]));
	} while (_zbasic->incrAndCheck(gridY, arr_i16_1eb8[9], arr_i16_1eb8[7]));
	// 143:0128
	for (int16 j = 1; j <= arr_i16_1eb8[1]; j++) {
		for (int16 i = 1; i <= arr_i16_1eb8[0]; i++) {
			arr_i16_2f38[i*32 + j] = 0;
			arr_i16_3b38[i*32 + j] = 0;
		}
	}
	// 143:01a4
	_straightPathGoal.clear(); // was: str(366)
	arr_i16_1eb8[15] = puzzlesReadShort();
	for (int16 i = 1; i <= arr_i16_1eb8[15]; i++) {
		gridX = puzzlesReadByte();
		gridY = puzzlesReadByte();
		_zbasic->indexRawSet(puzzlesReadString(), 1, i);
		arr_i16_2f38[gridX*32 + gridX] = i;
		arr_i16_3b38[gridX*32 + gridY] = _zbasic->asc(_zbasic->leftStr(_zbasic->index(1, i), 1));
		_straightPathGoal += _zbasic->indexRaw(1, i);
		// 143:0276
	}
	_zbasic->text(kFontPuzzle, arr_i16_1eb8[14], 0, kSrcOr);
	_zbasic->unk_20();
	// 143:02b6
	for (int16 j = 0; j <= arr_i16_1eb8[1]; j++) {
		Common::Rect temp;
		temp.top = arr_i16_1eb8[8] + arr_i16_1eb8[7] * (j - 1);
		temp.left = 0;
		temp.bottom = temp.top + arr_i16_1eb8[7] + 2;
		temp.right = SCREEN_WIDTH;
		_toolbox->FillRect(temp, _patterns[0]);
		for (int16 i = 1; i <= arr_i16_1eb8[0]; i++) {
			gridIndex = (j - 1)*arr_i16_1eb8[0] + i;
			if (arr_i16_3b38[i*32 + j] == 0) {
				// 143:03c4
				arr_i16_3b38[i*32 + j] = _zbasic->rndInt(0x1a) + 0x40;
			}
			// 143:03f0
			_toolbox->MoveTo(
				_screenGrid[gridIndex].left + arr_i16_1eb8[2] - 1,
				_screenGrid[gridIndex].top + arr_i16_1eb8[3]
			);
			_toolbox->DrawString(_zbasic->chr(arr_i16_3b38[i*32 + j]));
		}
	}
	straightPathDrawText();
	// 143:04cc
	_straightPathSelected = 0;
	_stateFlags = kStateNull;
	_activePuzzleSolved = (_straightPathGoal == _activePuzzleBuffer);

	// JMP 0x582
	while (((_stateFlags & kStateReturn) == 0) && (!_activePuzzleSolved)) {
		// 143:0500
		while ((_stateFlags == kStateNull) && (!_activePuzzleSolved)) {
			getNextEvent(-1);
			if (_event.what == kMouseDown) {
				straightPathOnClick();
			}
			if ((_keyLastPressed == 3) || (_keyLastPressed == 0xd)) {
				straightPathReset();
			}
			if (_straightPathGoal == _activePuzzleBuffer) {
				_activePuzzleSolved = true;
			}
		}
		// 143:0574
		if (_stateFlags == kStateSaveGame) {
			saveGame();
		}
	}
	// 143:05ac
	if (_activePuzzleSolved != 0) {
		straightPathSuccess();
	}
}

void FoolGame::straightPathOnClick() {
	// 143:05c0
	int16 gridX = 0;
	int16 gridY = 0;
	getGridFromMouse(gridX, gridY);
	if (_straightPathSelected > 0) {
		_toolbox->InvertRect(_screenGrid[_straightPathSelected]);
		_straightPathSelected = 0;
	}
	// 143:05fc
	if ((gridX < 1) || (gridX > arr_i16_1eb8[0]) || (gridY < 1) || (gridY > arr_i16_1eb8[1]))
		return;
	// 143:066c
	_activePuzzleStatus++;
	if (arr_i16_2f38[gridX*32 + gridY] != _activePuzzleStatus) {
		_activePuzzleBuffer += _zbasic->chr(arr_i16_3b38[gridX*32 + gridY]);
	} else {
		// 143:06d8
		_activePuzzleBuffer += _zbasic->indexRaw(1, _activePuzzleStatus);
	}
	// 143:06fa
	straightPathDrawText();
	if (_activePuzzleStatus < arr_i16_1eb8[15]) {
		_straightPathSelected = (gridY - 1)*arr_i16_1eb8[0] + gridX;
		_toolbox->InvertRect(_screenGrid[_straightPathSelected]);
	} else {
		// 143:0766
		if (_activePuzzleBuffer != _straightPathGoal) {
			straightPathReset();
		}
	}
	// 143:077e
	waitForMouseUp();
}

void FoolGame::straightPathReset() {
	// 143:0784
	if (_straightPathSelected > 0) {
		_toolbox->InvertRect(_screenGrid[_straightPathSelected]);
	}
	// 143:07a4
	for (int16 i = 0; i <= 0x14; i++) {
		int16 gridIndex = _zbasic->rndInt(arr_i16_1eb8[0] * arr_i16_1eb8[1]);
		_toolbox->InvertRect(_screenGrid[gridIndex]);
		_toolbox->Delay(0);
		playTone(_zbasic->rndInt(0x2328) + 0xf, 0x28, true);
		// 143:0812
		_toolbox->InvertRect(_screenGrid[gridIndex]);
	}
	_activePuzzleStatus = 0;
	_activePuzzleBuffer.clear(); // was: str(367)
	_straightPathSelected = 0;
	_keyLastPressed = 0;
	straightPathClearText();
}

void FoolGame::straightPathDrawText() {
	// 143:0864
	straightPathClearText();
	_zbasic->text(kFontLarge, 0x18, 0x18, kSrcBic);
	drawTextCenter(_activePuzzleBuffer, 0x148);
}

void FoolGame::straightPathClearText() {
	// 143:0890
	fillRect(0x127, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2);
}

void FoolGame::straightPathSuccess() {
	// 143:08b0
	_toolbox->PenMode(kPatBic);
	_toolbox->PenPat(_patterns[1]);
	for (int16 j = 1; j <= arr_i16_1eb8[1]; j++) {
		for (int16 i = 1; i <= arr_i16_1eb8[0]; i++) {
			if (arr_i16_2f38[i*32 + j] == 0) {
				int16 gridIndex = (j - 1)*arr_i16_1eb8[0] + i;
				_toolbox->PaintRect(_screenGrid[gridIndex]);
			}
			// 143:0936
		}
	}
	_toolbox->PenNormal();
	menuClickMessage();
	waitForClick();
}


} // End of namespace Fool
