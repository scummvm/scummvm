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

extern Toolbox *g_toolbox;

void FoolGame::straightPathRun() {
	// 143:0004
	fetchPuzzleData();
	for (int16 i = 0; i <= 0xe; i++) {
		arr_i16_1eb8[i] = puzzlesReadShort();
	}
	var_i16_484 = 0;
	var_i16_68c = arr_i16_1eb8[8];
	do {
		var_i16_68a = arr_i16_1eb8[10];
		do {
			var_i16_484++;
			g_toolbox->SetRect(
				_screenGrid[var_i16_484],
				var_i16_68a,
				var_i16_68c,
				var_i16_68a + arr_i16_1eb8[13] - 1,
				var_i16_68c + arr_i16_1eb8[12] - 1
			);
		} while (_zbasic->incrAndCheck(var_i16_68a, arr_i16_1eb8[11], arr_i16_1eb8[6]));
	} while (_zbasic->incrAndCheck(var_i16_68c, arr_i16_1eb8[9], arr_i16_1eb8[7]));
	// 143:0128
	for (int16 j = 1; j <= arr_i16_1eb8[1]; j++) {
		for (int16 i = 1; i <= arr_i16_1eb8[0]; i++) {
			arr_i16_2f38[i*32 + j] = 0;
			arr_i16_3b38[i*32 + j] = 0;
		}
	}
	// 143:01a4
	var_str_1272.clear(); // was: str(366)
	arr_i16_1eb8[15] = puzzlesReadShort();
	for (int16 i = 1; i <= arr_i16_1eb8[15]; i++) {
		var_i16_1a96 = puzzlesReadByte();
		var_i16_1a98 = puzzlesReadByte();
		_zbasic->indexRawSet(puzzlesReadString(), 1, i);
		arr_i16_2f38[var_i16_1a96*32 + var_i16_1a98] = i;
		arr_i16_3b38[var_i16_1a96*32 + var_i16_1a98] = _zbasic->asc(_zbasic->leftStr(_zbasic->index(1, i), 1));
		var_str_1272 += _zbasic->index(1, i);
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
		g_toolbox->FillRect(temp, _patterns[0]);
		for (int16 i = 1; i <= arr_i16_1eb8[0]; i++) {
			var_i16_1574 = (j - 1)*arr_i16_1eb8[0] + i;
			if (arr_i16_3b38[i*32 + j] == 0) {
				// 143:03c4
				arr_i16_3b38[i*32 + j] = _zbasic->rndInt(0x1a) + 0x40;
			}
			// 143:03f0
			g_toolbox->MoveTo(
				_screenGrid[var_i16_1574].left + arr_i16_1eb8[2] - 1,
				_screenGrid[var_i16_1574].top + arr_i16_1eb8[3]
			);
			var_str_384 = _zbasic->chr(arr_i16_3b38[i*32 + j]);
			g_toolbox->DrawString(var_str_384);
		}
	}
	straightPathDrawText();
	// 143:04cc
	var_i16_1574 = 0;
	_stateFlags = kStateNull;
	if (var_str_1272 == _activePuzzleBuffer) {
		_activePuzzleSolved = true;
	} else {
		_activePuzzleSolved = false;
	}
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
			if (var_str_1272 == _activePuzzleBuffer) {
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
	sub_128_2be(var_i16_68a, var_i16_68c);
	if (var_i16_1574 > 0) {
		g_toolbox->InvertRect(_screenGrid[var_i16_1574]);
		var_i16_1574 = 0;
	}
	// 143:05fc
	if ((var_i16_68a < 1) || (var_i16_68a > arr_i16_1eb8[0]) || (var_i16_68c < 1) || (var_i16_68c > arr_i16_1eb8[1]))
		return;
	// 143:066c
	_activePuzzleStatus++;
	if (arr_i16_2f38[var_i16_68a*32 + var_i16_68c] != _activePuzzleStatus) {
		_activePuzzleBuffer += _zbasic->chr(arr_i16_3b38[var_i16_68a*32 + var_i16_68c]);
	} else {
		// 143:06d8
		_activePuzzleBuffer += _zbasic->indexRaw(1, _activePuzzleStatus);
	}
	// 143:06fa
	straightPathDrawText();
	if (_activePuzzleStatus < arr_i16_1eb8[15]) {
		var_i16_1574 = (var_i16_68c - 1)*arr_i16_1eb8[0] + var_i16_68a;
		g_toolbox->InvertRect(_screenGrid[var_i16_1574]);
	} else {
		// 143:0766
		if (_activePuzzleBuffer != var_str_1272.encode(Common::kMacRoman)) {
			straightPathReset();
		}
	}
	// 143:077e
	waitForMouseUp();
}

void FoolGame::straightPathReset() {
	// 143:0784
	if (var_i16_1574 > 0) {
		g_toolbox->InvertRect(_screenGrid[var_i16_1574]);
	}
	// 143:07a4
	for (int16 i = 0; i <= 0x14; i++) {
		var_i16_1574 = _zbasic->rndInt(arr_i16_1eb8[0] * arr_i16_1eb8[1]);
		g_toolbox->InvertRect(_screenGrid[var_i16_1574]);
		playTone(_zbasic->rndInt(0x2328) + 0xf, 0x28, 0x1);
		// 143:0812
		g_toolbox->InvertRect(_screenGrid[var_i16_1574]);
	}
	_activePuzzleStatus = 0;
	_activePuzzleBuffer.clear(); // was: str(367)
	var_i16_1574 = 0;
	_keyLastPressed = 0;
	straightPathClearText();
}

void FoolGame::straightPathDrawText() {
	// 143:0864
	straightPathClearText();
	_zbasic->text(kFontLarge, 0x18, 0x18, kSrcBic);
	var_i16_7a2 = 0x148;
	sub_128_918(_activePuzzleBuffer);
}

void FoolGame::straightPathClearText() {
	// 143:0890
	fillRect(0x127, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2);
}

void FoolGame::straightPathSuccess() {
	// 143:08b0
	g_toolbox->PenMode(kPatBic);
	g_toolbox->PenPat(_patterns[1]);
	for (int16 j = 1; j <= arr_i16_1eb8[1]; j++) {
		for (int16 i = 1; i <= arr_i16_1eb8[0]; i++) {
			if (arr_i16_2f38[i*32 + j] == 0) {
				var_i16_1574 = (j - 1)*arr_i16_1eb8[0] + i;
				g_toolbox->PaintRect(_screenGrid[var_i16_1574]);
			}
			// 143:0936
		}
	}
	g_toolbox->PenNormal();
	menuClickMessage();
	waitForClick();
}


}
