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

#define OFF(x) (_zstrOffset[kOffsetReveal] + (x))



// mask reveal puzzle
void FoolGame::revealRun() {
	// 134:0004
	fetchPuzzleData();
	var_i16_c00 = 1;
	arr_i16_1eb8[15] = puzzlesReadShort();
	arr_i16_1eb8[16] = puzzlesReadShort();
	arr_i16_1eb8[21] = puzzlesReadShort();
	debugC(5, kDebugLoading, "%d, %d, %04x", arr_i16_1eb8[15], arr_i16_1eb8[16], arr_i16_1eb8[21]);
	var_str_384 = puzzlesReadString();
	var_str_384 = _zbasic->str(OFF(0)) + var_str_384 + _zbasic->str(OFF(1));
	_zbasic->menu(8, 6, 1, var_str_384);
	for (int i = 1; i <= arr_i16_1eb8[15]; i++) {
		arr_i16_3738[i] = puzzlesReadByte();
	}
	// 134:00ce
	// rack up polygon information
	_toolbox->SetPort(var_i32_f24);
	arr_i16_1eb8[20] = 0;

	for (int i = 1; i <= arr_i16_1eb8[15]; i++) {
		var_i16_103a = puzzlesReadShort();
		_revealPoly[i] = _toolbox->OpenPoly();
		// 134:010e
		for (int j = 5; j <= var_i16_103a - 1; j += 2) {
			var_i16_484 = puzzlesReadShort();
			var_i16_7e4 = puzzlesReadShort();
			if (j == 5) {
				_toolbox->MoveTo(var_i16_7e4, var_i16_484 + 0xf);
			} else {
				_toolbox->LineTo(var_i16_7e4, var_i16_484 + 0xf);
			}
			// 134:015c
		}
		_toolbox->ClosePoly();
		if (i <= arr_i16_1eb8[16]) {
			arr_i16_1eb8[20] |= _bitLUT[i - 1];
		}
		// 134:01ce
	}
	// 134:01ea
	_toolbox->SetPort(var_i32_0);

	arr_i16_1eb8[17] = (0x1c2 / arr_i16_1eb8[15]);
	arr_i16_1eb8[18] = 0x100 - ((arr_i16_1eb8[15] * arr_i16_1eb8[17]) / 2);
	Common::U32String letters = puzzlesReadString().decode(Common::kMacRoman);
	zoomRect(0xb4, 0xff, 0xb6, 0x101, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH, 2, kPatCopy, 0x19);
	zoomRect(0x9a, 0xff, 0x9c, 0x101, 0x37, 0x13, 0xff, 0x1ee, 0, kPatCopy, 0x19);
	// 134:02f0
	arr_i16_1eb8[19] = _zbasic->decodeInt(_activePuzzleBuffer);
	var_i16_1aba = arr_i16_1eb8[18];
	_zbasic->text(kFontChicago, 0xc, Graphics::kMacFontRegular, kSrcOr);
	for (var_i16_7be = 1; var_i16_7be <= arr_i16_1eb8[15]; var_i16_7be++) {
		// play a random tone and draw each of the letter buttons in sequence
		playTone(
			_zbasic->rndInt(0x3e8) + 0x19,
			0x28,
			1
		);
		// 134:0352
		_screenGrid[var_i16_7be].top = 0x10f;
		_screenGrid[var_i16_7be].left = var_i16_1aba;
		_screenGrid[var_i16_7be].bottom = 0x137;
		_screenGrid[var_i16_7be].right = var_i16_1aba + arr_i16_1eb8[17];

		var_i16_1aba += arr_i16_1eb8[17];
		// 134:03fc
		_toolbox->FillRoundRect(_screenGrid[var_i16_7be], 0x19, 0x19, _patterns[0]);
		_toolbox->FrameRoundRect(_screenGrid[var_i16_7be], 0x19, 0x19);
		// 134:0448
		var_str_384 = _zbasic->midStr(letters, var_i16_7be, 1);
		var_i16_7ba = _toolbox->StringWidth(var_str_384);
		_toolbox->MoveTo(
			_screenGrid[var_i16_7be].left + (arr_i16_1eb8[17] / 2) - (var_i16_7ba / 2),
			0x127
		);
		_toolbox->DrawString(var_str_384);
		// 134:04d6
		if (arr_i16_1eb8[19] & _bitLUT[arr_i16_3738[var_i16_7be] - 1]) {
			revealSelectButton();
		}
	}
	// 134:0538
	_stateFlags = kStateNull;
	if (arr_i16_1eb8[20] == arr_i16_1eb8[19]) {
		_activePuzzleSolved = true;
	} else {
		// 134:0572
		_activePuzzleSolved = false;
	}
	_toolbox->PenNormal();
	// 134:057a
	// 134:057a: JMP - [0x61e]
	// main event loop
	while (((_stateFlags & kStateReturn) == 0) && (!_activePuzzleSolved)) {
		// 134:057e
		while ((_stateFlags == 0) && (!_activePuzzleSolved)) {
			// 134:0582
			getNextEvent(-1);
			if (_event.what == kMouseDown) {
				revealOnClick();
			}
			if (arr_i16_1eb8[20] == arr_i16_1eb8[19]) {
				_activePuzzleSolved = true;
			}
		}
		// 134:05e2
		if (_stateFlags == kStateUndo) {
			revealReset();
		}
		if (_stateFlags == kStateSaveGame) {
			_activePuzzleBuffer = _zbasic->encodeInt(arr_i16_1eb8[19]);
			saveGame();
		}
		// 134:061e
	}
	// 134:0648
	if (_activePuzzleSolved) {
		revealSuccess();
	}
	_activePuzzleBuffer = _zbasic->encodeInt(arr_i16_1eb8[19]);
	// 134:0678: JMP - [0x8ae]
	for (int i = 1; i <= arr_i16_1eb8[15]; i++) {
		_toolbox->KillPoly(_revealPoly[i]);
	}
}

void FoolGame::revealOnClick() {
	// 134:067c
	var_i16_7be = 0;
	for (int j = 1; j <= arr_i16_1eb8[15]; j++) {
		if (_toolbox->PtInRect(_event.where, _screenGrid[j])) {
			var_i16_7be = j;
			j = arr_i16_1eb8[15];
		}
	}
	// 134:06d0
	if (var_i16_7be == 0) {
		return;
	}
	// 134:06f4
	revealSelectButton();
	arr_i16_1eb8[19] ^= _bitLUT[arr_i16_3738[var_i16_7be]-1];
	debugC(5, kDebugLoading, "reveal: %d, %04x, %04x", var_i16_7be, arr_i16_1eb8[19], arr_i16_1eb8[20]);
	waitForMouseUp();
}

void FoolGame::revealSelectButton() {
	// 134:074a
	// invert the button
	_toolbox->InvertRoundRect(_screenGrid[var_i16_7be], 0x19, 0x19);
	_toolbox->PenPat(_patterns[arr_i16_1eb8[21]]);
	// fill the polygon
	_toolbox->PenMode(kPatXor);
	_toolbox->PaintPoly(_revealPoly[arr_i16_3738[var_i16_7be]]);
	_toolbox->PenNormal();
}

void FoolGame::revealReset() {
	// 134:07bc
	fillRect(0x37, 0x13, 0xff, 0x1ee, 0);
	for (int i = 1; i <= arr_i16_1eb8[15]; i++) {
		if (arr_i16_1eb8[19] & _bitLUT[arr_i16_3738[i]-1]) {
			_toolbox->InvertRoundRect(_screenGrid[i], 0x19, 0x19);
		}
	}
	// 134:085c
	arr_i16_1eb8[19] = 0;
	_stateFlags = kStateNull;
}

void FoolGame::revealSuccess() {
	// 134:0872
	if (_activePuzzleStatus < 0x64) {
		_activePuzzleStatus = 0x64;
	}
	menuClickMessage();
	waitForMouseUp();
	flashRect(0x37, 0x13, 0xff, 0x1ee, 0xc8);
}

}
