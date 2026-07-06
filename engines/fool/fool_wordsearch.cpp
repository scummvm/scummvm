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

#include "fool/detection.h"
#include "fool/fool.h"
#include "fool/fool_game.h"
#include "fool/toolbox.h"
#include "fool/zbasic.h"

namespace Fool {

#define OFF(x) (_zstrOffset[kOffsetWordSearch] + (x))



// word search game
void FoolGame::wordSearchRun() {
	// 131:0004
	fetchPuzzleData();
	var_str_188e = puzzlesReadString().decode(Common::kMacRoman);
	for (int i = 0; i <= 0xe; i++) {
		arr_i16_1eb8[i] = puzzlesReadShort();
	}
	// 131:0042
	var_i16_484 = 0;
	var_i16_68c = arr_i16_1eb8[8];
	do {
		var_i16_68a = arr_i16_1eb8[0xa];
		do {
			var_i16_484++;
			_toolbox->SetRect(
				_screenGrid[var_i16_484],
				var_i16_68a,
				var_i16_68c,
				var_i16_68a + arr_i16_1eb8[0xd] - 1,
				var_i16_68c + arr_i16_1eb8[0xc] - 1
			);
		// 131:00d4
		} while (_zbasic->incrAndCheck(var_i16_68a, arr_i16_1eb8[0xb], arr_i16_1eb8[6]));
	} while (_zbasic->incrAndCheck(var_i16_68c, arr_i16_1eb8[9], arr_i16_1eb8[7]));
	// 131:0134
	var_i16_484 = 0;

	for (int j = 1; j <= arr_i16_1eb8[1]; j++) {
		for (int i = 1; i <= arr_i16_1eb8[0]; i++) {
			arr_i16_2f38[i*32 + j] = 0;
			arr_i16_3b38[i*32 + j] = 0;
		}
	}

	// 131:01b6
	// total hidden word count
	var_i16_198e = puzzlesReadShort();
	if (_activePuzzleBuffer.empty()) { // was: str(OFF(0))
		_activePuzzleBuffer = _zbasic->space(var_i16_198e).encode(Common::kMacRoman);
	} else {
		// 131:01f2
		arr_i16_4758[0] = _zbasic->decodeInt(_zbasic->midStr(_activePuzzleBuffer, var_i16_198e + 1, 2));
		for (int i = 1; i <= arr_i16_4758[0]; i++) {
			arr_i16_4758[i] = _zbasic->decodeInt(_zbasic->midStr(
				_activePuzzleBuffer,
				var_i16_198e + 1 + i*2,
				2
			));
		}
		// 131:0282
		var_i16_484 = 0;
		var_i16_106a = 1;
		var_i16_68c = 1;

		for (int j = 1; j <= arr_i16_1eb8[1]; j++) {
			for (int i = 1; i <= arr_i16_1eb8[0]; i++) {
				// 131:029a
				if ((arr_i16_4758[var_i16_106a] & _bitLUT[var_i16_484]) != 0) {
					arr_i16_2f38[i*32 + j] = 1;
				}
				// 131:02e8
				var_i16_484++;
				if (var_i16_484 == 0x10) {
					var_i16_484 = 0;
					var_i16_106a++;
				}
				// 131:0300
			}
		}
		// 131:0336

	}
	// 131:0336
	var_i16_1990 = 0;
	var_i16_68a = 1;
	for (int i = 1; i <= var_i16_198e; i++) {
		arr_i16_3738[i] = _zbasic->castInt(_zbasic->midStr(_activePuzzleBuffer, i, 1));
		if (arr_i16_3738[i] != 0) {
			var_i16_1990++;
		}
		// 131:038e
	}
	// 131:039e
	var_i16_1992 = 1;
	do {
		_zbasic->indexSet(puzzlesReadString(), 1, var_i16_1992 + var_i16_198e);
		_zbasic->indexSet(Common::U32String(), 1, var_i16_1992); // was: str(OFF(1))
		var_i16_484 = 1;
		do {
			var_str_384 = _zbasic->midStr(
				_zbasic->index(1, var_i16_1992 + var_i16_198e),
				var_i16_484,
				1
			);
			// 131:0418
			if (var_str_384 != _zbasic->str(OFF(2))) { // ' '
				_zbasic->indexSet(_zbasic->index(1, var_i16_1992) + var_str_384, 1, var_i16_1992);
			}
			// 131:045c
			var_i16_484++;
		} while ((int16)(_zbasic->index(1, var_i16_1992 + var_i16_198e).size()) >= var_i16_484);

		// 131:0480
		var_i16_1992++;
	} while (var_i16_198e >= var_i16_1992);

	// 131:0492
	_toolbox->PenNormal();
	var_str_384 = var_str_188e;
	wordSearchDrawFooter();
	_zbasic->text(kFontPuzzle, arr_i16_1eb8[0xe], Graphics::kMacFontRegular, kSrcOr);
	_toolbox->PenPat(_patterns[1]);
	_toolbox->PenMode(kPatBic);
	_zbasic->unk_20();
	// 131:04e0
	for (int j = 1; j <= arr_i16_1eb8[1]; j++) {
		var_str_1994 = puzzlesReadString().decode(Common::kMacRoman);
		var_i16_1574 = 1 + (j - 1) * arr_i16_1eb8[0];
		// 131:051e
		Common::Rect temp; // arr_rect_5b7c
		temp.top = _screenGrid[var_i16_1574].top - 1;
		temp.left = 0;
		temp.bottom = _screenGrid[var_i16_1574].bottom + 1;
		temp.right = SCREEN_WIDTH;
		_toolbox->FillRect(temp, _patterns[0]);
		// 131:05b0
		for (int i = 1; i <= arr_i16_1eb8[0]; i++) {
			var_i16_1574 = i + (j - 1) * arr_i16_1eb8[0];
			var_str_384 = _zbasic->midStr(var_str_1994, i, 1);
			arr_i16_3b38[i*32 + j] = _zbasic->asc(var_str_384);

			_toolbox->MoveTo(
				_screenGrid[var_i16_1574].left + arr_i16_1eb8[2] - 1,
				_screenGrid[var_i16_1574].top + arr_i16_1eb8[3]
			);
			_toolbox->DrawString(var_str_384);
			// 131:069e
			if (arr_i16_2f38[i*32 + j] != 0) {
				_toolbox->PaintRect(_screenGrid[var_i16_1574]);
			}
			// 131:06dc
		}
		// 131:06f8
	}
	// 131:0714
	_toolbox->PenNormal();
	if ((var_i16_1990 > 0) && (var_i16_1990 < var_i16_198e)) {
		// only X to go
		var_str_384 = _zbasic->str(OFF(3)) + Common::U32String::format(" %d", var_i16_198e - var_i16_1990) + _zbasic->str(OFF(4));
		wordSearchDrawFooter();
	}
	// 131:077a
	var_str_9f4 = Common::U32String::format(" %d", var_i16_198e);
	if (_activePuzzle != 0x50) {
		// There are X words hidden below
		var_str_384 = _zbasic->str(OFF(5)) + var_str_9f4 + _zbasic->str(OFF(6));
		_zbasic->menu(8, 3, 1, var_str_384);
		_zbasic->menu(8, 4, 1, _zbasic->str(OFF(7)));
	} else {
	// 131:07f8
		var_str_384 = _zbasic->str(OFF(8)) + var_str_9f4 + _zbasic->str(OFF(9));
		_zbasic->menu(8, 3, 1, var_str_384);
		_zbasic->menu(8, 4, 1, _zbasic->str(OFF(10)));
	}
	// 131:084c
	_stateFlags = kStateNull;
	while ((_stateFlags & kStateReturn) == 0) {
		getNextEvent(-1);
		if (_event.what == 1) {
			wordSearchOnClick();
		}
		if (var_i16_1990 == var_i16_198e) {
			wordSearchSuccess();
		}
		if (_stateFlags == kStateSaveGame) {
			wordSearchStoreState();
			saveGame();
		}
	}
	wordSearchStoreState();
	// 131:08a6

	// JMP 0x1116
	// 131:1116
}

void FoolGame::wordSearchOnClick() {
	// 131:08aa
	var_i16_1a94 = 0; // length of word selected
	var_str_1070.clear(); // was: str(OFF(11))
	var_i16_1a96 = -1;
	var_i16_1a98 = -1;

	while ((_event.modifiers & 0x80) == 0) {
		// 131:08d4
		getNextEvent(0);
		getGridFromMouse(var_i16_68a, var_i16_68c);
		sub_128_342(var_i16_68a, var_i16_68c);
		if (!((var_i16_68a == var_i16_1a96) && (var_i16_68c == var_i16_1a98))) {
			// 131:0934
			var_i16_1574 = var_i16_68a + (var_i16_68c - 1)*arr_i16_1eb8[0];
			var_i16_1a96 = var_i16_68a;
			var_i16_1a98 = var_i16_68c;
			var_str_1070 += _zbasic->chr(arr_i16_3b38[var_i16_68a*32 + var_i16_68c]);
			_toolbox->InvertRect(_screenGrid[var_i16_1574]);
			var_i16_1a94++;
			// 131:09c4
			arr_i16_4338[var_i16_1a94] = var_i16_1574;
			if (var_i16_1a94 == 0x64) {
				wordSearchBadSelect();
				return;
			}
		}
		// 131:09ec
		_toolbox->Delay(0);
	}
	debugC(5, kDebugLoading, "wordSearchOnClick: pos (%d, %d)", var_i16_1a96, var_i16_1a98);
	// 131:09fc
	var_i16_1a9a = 0;
	var_i16_1a9c = arr_i16_4338[2] - arr_i16_4338[1];
	var_i16_68a = 1;
	for (int i = 1; i <= var_i16_1a94 - 1; i++) {
		if (arr_i16_4338[var_i16_68a + 1] - arr_i16_4338[var_i16_68a] != var_i16_1a9c) {
			var_i16_1a9a = 0x29a;
		}
	// 131:0a78
	}
	if (var_i16_1a9a == 0x29a) {
		wordSearchBadSelect();
		return;
	}
	var_i16_1a9a = 0;
	// 131:0aa0
	// check selected string against the remaining word list
	for (int i = 1; i <= var_i16_198e; i++) {
		if ((var_str_1070 == _zbasic->index(1, i)) && (arr_i16_3738[i] == 0)) {
			// 131:0ae0
			var_i16_1a9a = i;
			arr_i16_3738[i] = 1;
		}
		// 131:0af8
	}
	if (var_i16_1a9a == 0) {
		wordSearchBadSelect();
		return;
	}
	// found a word, update the screen
	_toolbox->PenNormal();
	_toolbox->PenPat(_patterns[1]);
	_toolbox->PenMode(kPatBic);
	// for every letter in the word
	for (int i = 1; i <= var_i16_1a94; i++) {
		// play a tone
		playTone(0xa + _zbasic->rndInt(0x3e8), 0x28, 1);
		// 131:0b54
		// remove the inverted square
		_toolbox->InvertRect(_screenGrid[arr_i16_4338[i]]);
		// paint the square with white checkerboard
		_toolbox->PaintRect(_screenGrid[arr_i16_4338[i]]);
		var_i16_484 = ((arr_i16_4338[i] - 1) % arr_i16_1eb8[0]) + 1;
		// 131:0bce
		var_i16_7e4 = ((arr_i16_4338[i] - 1) / arr_i16_1eb8[0]) + 1;
		// add the square to the selected list
		arr_i16_2f38[var_i16_484*32 + var_i16_7e4] = 1;
	}
	// 131:0c32
	_toolbox->PenNormal();
	var_i16_1990++;
	if (var_i16_1990 == var_i16_198e) {
		var_str_384 = _zbasic->str(OFF(12)); // ' '
		wordSearchDrawFooter();
		return;
	}
	// 131:0c64
	var_str_384 = _zbasic->index(1, var_i16_1a9a + var_i16_198e) + _zbasic->str(OFF(13)) + Common::U32String::format(" %d", var_i16_198e - var_i16_1990) + _zbasic->str(OFF(14));	// WORD... only X to go
	wordSearchDrawFooter();
	waitForMouseUp();
}

void FoolGame::wordSearchBadSelect() {
	// 131:0cbe
	// play failure noise
	playTone(0x14, 0x64, 0);
	// uninvert all highlighted boxes
	for (int i = 1; i <= var_i16_1a94; i++) {
		_toolbox->InvertRect(_screenGrid[arr_i16_4338[i]]);
	}
	// 131:0d0a
	var_i16_1a96 = -1;
	var_i16_1a98 = -1;
	var_i16_1a94 = 0;
	waitForMouseUp();
}

void FoolGame::wordSearchDrawFooter() {
	// 131:0d22
	fillRect(0x13b, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2);
	_zbasic->text(kFontFool, 0xc, 0, kSrcBic);
	drawTextCenter(var_str_384, 0x152);
}

void FoolGame::wordSearchStoreState() {
	// 131:0d66
	_activePuzzleBuffer.clear(); // was: str(OFF(15))
	for (int i = 1; i <= var_i16_198e; i++) {
		var_str_384 = Common::U32String::format("%d", arr_i16_3738[i]);
		_activePuzzleBuffer += _zbasic->rightStr(var_str_384, 1);
	}
	// 131:0dd6
	var_i16_484 = 0;
	var_i16_106a = 1;
	arr_i16_4758[0] = 1;
	arr_i16_4758[1] = 0;
	for (int j = 1; j <= arr_i16_1eb8[1]; j++) {
		for (int i = 1; i <= arr_i16_1eb8[0]; i++) {
			if (arr_i16_2f38[i*32 + j] != 0) {
				arr_i16_4758[var_i16_106a] |= _bitLUT[var_i16_484];
			}
			// 131:0e72
			var_i16_484++;
			if (var_i16_484 == 0x10) {
				var_i16_484 = 0;
				var_i16_106a++;
				arr_i16_4758[var_i16_106a] = 0;
				arr_i16_4758[0]++;
			}
			// 131:0ebe
		}
	}
	// 131:0ef6
	debugC(5, kDebugLoading, "wordSearchStoreState:");
	for (int i = 0; i <= arr_i16_4758[0]; i++) {
		debugCN(5, kDebugLoading, "%04x ", arr_i16_4758[i]);
		_activePuzzleBuffer += _zbasic->encodeInt(arr_i16_4758[i]);
	}
}

void FoolGame::wordSearchSuccess() {
	// 131:0f42
	if (_activePuzzleStatus < 0x64) {
		_activePuzzleStatus = 0x64;
	}
	// 131:0f52
	fillRect(1, kPatOr, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH);
	_zbasic->text(kFontPuzzle, arr_i16_1eb8[0xe], 0, kSrcOr);
	for (int j = 1; j <= arr_i16_1eb8[1]; j++) {
		for (int i = 1; i <= arr_i16_1eb8[0]; i++) {
			if (arr_i16_2f38[i*32 + j] != 0) {
				// 131:0fc8
				var_i16_1574 = i + (j - 1)* arr_i16_1eb8[0];
				_toolbox->EraseRect(_screenGrid[var_i16_1574]);
				_toolbox->MoveTo(
					_screenGrid[var_i16_1574].left + arr_i16_1eb8[2] - 1,
					_screenGrid[var_i16_1574].top + arr_i16_1eb8[3]
				);
				// 131:107a
				var_str_384 = _zbasic->chr(arr_i16_3b38[i*32 + j]);
				_toolbox->DrawString(var_str_384);
			}
			// 131:10b0
		}
	}
	// 131:10e8
	fillRect(0x1eb, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 1);
	menuClickMessage();
	waitForClick();
	_stateFlags = kStateReturn;
}


} // End of namespace Fool
