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



// The Sun's map
void FoolGame::sunMapRun() {
	// 137:0004
	var_i16_c00 = 1;
	fillRect(1, kPatOr, 0x14, 0, SCREEN_HEIGHT, SCREEN_WIDTH);
	arr_i16_1eb8[0] = 9;
	arr_i16_1eb8[1] = 9;
	arr_i16_1eb8[4] = 0x4b;
	arr_i16_1eb8[5] = -9;
	arr_i16_1eb8[6] = 0x23;
	arr_i16_1eb8[7] = 0x23;
	var_i16_484 = 0;
	for (int16 j = 0x1a; j <= 0x132; j += 0x23) {
		for (int16 i = 0x6e; i <= 0x186; i += 0x23) {
			var_i16_484++;
			_toolbox->SetRect(_screenGrid[var_i16_484], i, j, i+0x24, j+0x24);
		}
	}
	// 137:00fa
	var_i16_484 = 0;
	for (int16 j = 1; j <= 9; j++) {
		for (int16 i = 1; i <= 9; i++) {
			var_i16_484++;
			arr_i16_2f38[i*32 + j] = var_i16_484;
		}
	}
	// 137:0148
	for (int i = 1; i <= 0x51; i++) {
		var_i16_484 = arr_i16_4c7c[i];
		if ((_puzzleFlags[var_i16_484] & (kFlagMenuDiamond | kFlagMenuEnabled)) != 0) {
			arr_i32_192c0[i] = _sunMapTilePic[i];
		} else {
			// 137:01b2
			arr_i32_192c0[i] = _sunMapTilePic[0x52];
		}
		// 137:01d8
		if ((_puzzleFlags[var_i16_484] & 4) != 0) {
			_puzzleFlags[var_i16_484] ^= 4;
			_puzzleFlags[i] |= kFlagMapTile;
		}
		// 137:0252
		if ((_puzzleFlags[i] & kFlagMapTile) != 0) {
			arr_i32_192c0[i] = _sunMapTilePic[0x53];
		}
		// 137:0294
	}
	// 137:02a2
	fillRect(0x14, 0x67, 0x156, 0x1b1, 2);
	for (int i = 1; i <= 0x51; i++) {
		_zbasic->picture(_screenGrid[i].left, _screenGrid[i].top, arr_i32_192c0[_sunMapTileID[i]]);
	}
	// 137:0336
	_stateFlags = kStateNull;
	_zbasic->menu(8, 3, 0, Common::U32String());
	_toolbox->InitCursor();
	sunMapCheckIfSolved();
	while (((_stateFlags & kStateReturn) == 0) && (!_activePuzzleSolved)) {
		while ((_stateFlags == kStateNull) && (!_activePuzzleSolved)) {
			getNextEvent(-1);
			if (_event.what == kMouseDown) {
				sunMapOnClick();
			}
		}
		// 137:038a
		if (_stateFlags == kStateUndo) {
			sunMapUndoMove();
		}
		if (_stateFlags == kStateSaveGame) {
			saveGame();
		}
	}
	// 137:03ce
	if (!_activePuzzleSolved) {
		return;
	}
	// JMP 0x1618
	var_i16_68c = 0;
	for (int j = 0; j <= 5; j++) {
		for (int i = 1; i <= 0x51; i++) {
			_toolbox->InvertRect(_screenGrid[i]);
		}
	}
	// 137:1654
	_activePuzzle = 0;
	_activePuzzleStatus = 0x64;
	_sunMapRestored = 1;
	_zbasic->menu(2, 7, 1, Common::U32String());
	for (int i = 1; i <= 0x50; i++) {
		_puzzleFlags[i] = kFlagMenuEnabled;
	}
	for (int j = 3; j <= 7; j++) {
		for (int i = 1; i <= 0x10; i++) {
			_zbasic->menu(j, i, 1, Common::U32String());
		}
	}
}

void FoolGame::sunMapOnClick() {
	// 137:03e2
	getGridFromMouse(var_i16_68a, var_i16_68c);
	if ((var_i16_68a < 1) || (var_i16_68a > arr_i16_1eb8[0]) ||
		(var_i16_68c < 1) || (var_i16_68c > arr_i16_1eb8[1])) {
		return;
	}
	// 137:0466
	_menuDisabled = true;
	_zbasic->menu(8, 3, 0, Common::U32String());
	_toolbox->PenNormal();
	_toolbox->PenSize(3, 3);
	_toolbox->PenMode(kPatXor);
	arr_i16_4758[0] = var_i16_68a;
	arr_i16_4758[1] = var_i16_68c;
	if (_puzzleFlags[_sunMapTileID[arr_i16_2f38[var_i16_68a*32 + var_i16_68c]]] & kFlagMapTile) {
		sunMapRevealPiece();
		return;
	}
	sunMapDragSelect();
	// 137:0506
	_toolbox->FrameRect(Common::Rect(arr_i16_4758[12], arr_i16_4758[11], arr_i16_4758[14], arr_i16_4758[13]));
	arr_i16_4758[0] -= var_i16_68a;
	arr_i16_4758[1] -= var_i16_68c;
	sunMapMoveSelected();
	_toolbox->FrameRect(Common::Rect(arr_i16_4758[12], arr_i16_4758[11], arr_i16_4758[14], arr_i16_4758[13]));
	sunMapDropSelected();
	sunMapCheckIfSolved();
	waitForMouseUp();
	_zbasic->menu(8, 3, 1, Common::U32String());
	_menuDisabled = false;
}

void FoolGame::sunMapDragSelect() {
	// 137:0598
	do {
		getNextEvent(4);
		getGridFromMouse(var_i16_68a, var_i16_68c);
		sub_128_342(var_i16_68a, var_i16_68c);
		if (var_i16_68a >= arr_i16_4758[0]) {
			arr_i16_4758[2] = arr_i16_4758[0];
			arr_i16_4758[3] = var_i16_68a;
		} else {
		// 137:061a
			arr_i16_4758[2] = var_i16_68a;
			arr_i16_4758[3] = arr_i16_4758[0];
		}
		// 137:064a
		if (var_i16_68c >= arr_i16_4758[1]) {
			arr_i16_4758[6] = arr_i16_4758[1];
			arr_i16_4758[7] = var_i16_68c;
		} else {
			// 137:069e
			arr_i16_4758[6] = var_i16_68c;
			arr_i16_4758[7] = arr_i16_4758[1];
		}
		// 137:06ce
		arr_i16_4758[11] = _screenGrid[arr_i16_2f38[arr_i16_4758[2]*32 + arr_i16_4758[6]]].top;
		arr_i16_4758[12] = _screenGrid[arr_i16_2f38[arr_i16_4758[2]*32 + arr_i16_4758[6]]].left;
		arr_i16_4758[13] = _screenGrid[arr_i16_2f38[arr_i16_4758[3]*32 + arr_i16_4758[7]]].bottom;
		arr_i16_4758[14] = _screenGrid[arr_i16_2f38[arr_i16_4758[3]*32 + arr_i16_4758[7]]].right;
		// 137:084c
		_toolbox->FrameRect(Common::Rect(arr_i16_4758[12], arr_i16_4758[11], arr_i16_4758[14], arr_i16_4758[13]));
		delay(2);
		_toolbox->FrameRect(Common::Rect(arr_i16_4758[12], arr_i16_4758[11], arr_i16_4758[14], arr_i16_4758[13]));
		delay(1);
	} while (_event.what != kMouseUp);
	// 137:087e
}

void FoolGame::sunMapMoveSelected() {
	// 137:0880
	do {
		getNextEvent(2);
		getGridFromMouse(var_i16_68a, var_i16_68c);
		sub_128_342(var_i16_68a, var_i16_68c);
		if (arr_i16_4758[0] >= 0) {
			arr_i16_4758[4] = var_i16_68a;
			arr_i16_4758[5] = var_i16_68a + arr_i16_4758[0];
		} else {
			// 137:0900
			arr_i16_4758[4] = var_i16_68a + arr_i16_4758[0];
			arr_i16_4758[5] = var_i16_68a;
		}
		// 137:093c
		if (arr_i16_4758[1] >= 0) {
			arr_i16_4758[8] = var_i16_68c;
			arr_i16_4758[9] = var_i16_68c + arr_i16_4758[1];
		} else {
			// 137:098e
			arr_i16_4758[8] = var_i16_68c + arr_i16_4758[1];
			arr_i16_4758[9] = var_i16_68c;
		}
		// 137:09ca
		if (arr_i16_4758[4] < 1) {
			arr_i16_4758[4] = 1;
			arr_i16_4758[5] = 1 + ABS(arr_i16_4758[0]);
		}
		// 137:0a2e
		if (arr_i16_4758[8] < 1) {
			arr_i16_4758[8] = 1;
			arr_i16_4758[9] = 1 + ABS(arr_i16_4758[1]);
		}
		// 137:0a92
		if (arr_i16_4758[5] > arr_i16_1eb8[0]) {
			arr_i16_4758[4] = arr_i16_1eb8[0] - ABS(arr_i16_4758[0]);
			arr_i16_4758[5] = arr_i16_1eb8[0];
		}
		// 137:0b20
		if (arr_i16_4758[9] > arr_i16_1eb8[1]) {
			arr_i16_4758[8] = arr_i16_1eb8[1] - ABS(arr_i16_4758[1]);
			arr_i16_4758[9] = arr_i16_1eb8[1];
		}
		// 137:0bae
		arr_rect_4776.top = _screenGrid[arr_i16_2f38[arr_i16_4758[4]*32 + arr_i16_4758[8]]].top;
		arr_rect_4776.left = _screenGrid[arr_i16_2f38[arr_i16_4758[4]*32 + arr_i16_4758[8]]].left;
		arr_rect_4776.bottom = _screenGrid[arr_i16_2f38[arr_i16_4758[5]*32 + arr_i16_4758[9]]].bottom;
		arr_rect_4776.right = _screenGrid[arr_i16_2f38[arr_i16_4758[5]*32 + arr_i16_4758[9]]].right;
		_toolbox->FrameRect(arr_rect_4776);
		delay(2);
		_toolbox->FrameRect(arr_rect_4776);
		delay(1);
	} while (_event.what != kMouseDown);
}

void FoolGame::sunMapUndoMove() {
	// 137:0d60
	_stateFlags = kStateNull;
	_zbasic->swapInt(arr_i16_4758[2], arr_i16_4758[4]);
	_zbasic->swapInt(arr_i16_4758[3], arr_i16_4758[5]);
	_zbasic->swapInt(arr_i16_4758[6], arr_i16_4758[8]);
	_zbasic->swapInt(arr_i16_4758[7], arr_i16_4758[9]);
	sunMapDropSelected();
}

void FoolGame::sunMapDropSelected() {
	// 137:0dde
	var_i16_1aa4 = 0x51;
	for (int j = arr_i16_4758[6]; j <= arr_i16_4758[7]; j++) {
		for (int i = arr_i16_4758[2]; i <= arr_i16_4758[3]; i++) {
			var_i16_1aa4++;
			arr_i16_3738[var_i16_1aa4] = _sunMapTileID[arr_i16_2f38[i*32 + j]];
			// 137:0e5a
			_sunMapTileID[arr_i16_2f38[i*32 + j]] = 0;
		}
	}

	// 137:0ec2
	var_i16_1aa6 = var_i16_1aa4;
	var_i16_1aa4 = 0x51;
	for (int j = arr_i16_4758[8]; j <= arr_i16_4758[9]; j++) {
		for (int i = arr_i16_4758[4]; i <= arr_i16_4758[5]; i++) {
			var_i16_1aa4++;
			if (_sunMapTileID[arr_i16_2f38[i*32 + j]] > 0) {
				var_i16_1aa6++;
				arr_i16_3738[var_i16_1aa6] = _sunMapTileID[arr_i16_2f38[i*32 + j]];
			}
			// 137:0f7c
			_sunMapTileID[arr_i16_2f38[i*32 + j]] = arr_i16_3738[var_i16_1aa4];
			_zbasic->picture(
				_screenGrid[arr_i16_2f38[i*32 + j]].left,
				_screenGrid[arr_i16_2f38[i*32 + j]].top,
				arr_i32_192c0[_sunMapTileID[arr_i16_2f38[i*32 + j]]]
			);
		}
	}
	var_i16_1aa6 = var_i16_1aa4;
	for (int j = arr_i16_4758[6]; j <= arr_i16_4758[7]; j++) {
		for (int i = arr_i16_4758[2]; i <= arr_i16_4758[3]; i++) {
			if (_sunMapTileID[arr_i16_2f38[i*32 + j]] == 0) {
				// 137:1114
				var_i16_1aa6++;
				_sunMapTileID[arr_i16_2f38[i*32 + j]] = arr_i16_3738[var_i16_1aa6];
				_zbasic->picture(
					_screenGrid[arr_i16_2f38[i*32 + j]].left,
					_screenGrid[arr_i16_2f38[i*32 + j]].top,
					arr_i32_192c0[_sunMapTileID[arr_i16_2f38[i*32 + j]]]
				);
			}
			// 137:1214
		}
	}

	// 137:124c
}

void FoolGame::sunMapCheckIfSolved() {
	// 137:124e
	_activePuzzleSolved = true;
	for (int i = 1; i <= 0x51; i++) {
		if (_sunMapTileID[i] != i) {
			_activePuzzleSolved = false;
			i = 0x51;
		}
	}
	// 137:128e
	if (_activePuzzleSolved) {
		for (int i = 1; i <= 0x51; i++) {
			var_i16_484 = arr_i16_4c7c[i];
			if ((_puzzleFlags[var_i16_484] & kFlagMapTile) || (_puzzleFlags[var_i16_484] == kFlagMenuDisabled)) {
				_activePuzzleSolved = false;
				i = 0x51;
			}
			// 137:12fa
		}
	}
	// 137:1306
	if (_activePuzzleSolved) {
		_stateFlags = kStateReturn;
	}
}

void FoolGame::sunMapRevealPiece() {
	// 137:131a
	_puzzleFlags[_sunMapTileID[arr_i16_2f38[var_i16_68a*32 + var_i16_68c]]] ^= kFlagMapTile;
	arr_i32_192c0[_sunMapTileID[arr_i16_2f38[var_i16_68a*32 + var_i16_68c]]] = _sunMapTilePic[_sunMapTileID[arr_i16_2f38[var_i16_68a*32 + var_i16_68c]]];
	// 137:142c
	// unrolled loop
	arr_i16_4758[0] = _screenGrid[arr_i16_2f38[var_i16_68a*32 + var_i16_68c]].top;
	arr_i16_4758[1] = _screenGrid[arr_i16_2f38[var_i16_68a*32 + var_i16_68c]].left;
	arr_i16_4758[2] = _screenGrid[arr_i16_2f38[var_i16_68a*32 + var_i16_68c]].bottom;
	arr_i16_4758[3] = _screenGrid[arr_i16_2f38[var_i16_68a*32 + var_i16_68c]].right;
	// 137:1490
	for (int16 i = 0; i <= 0xf; i++) {
		arr_i16_4758[0] += i;
		arr_i16_4758[1] += i;
		arr_i16_4758[2] -= i;
		arr_i16_4758[3] -= i;
		Common::Rect temp;
		temp.top = arr_i16_4758[0];
		temp.left = arr_i16_4758[1];
		temp.bottom = arr_i16_4758[2];
		temp.right = arr_i16_4758[3];
		_toolbox->FrameRect(temp);
		delay(1);
	}
	// 137:1550
	_zbasic->picture(
		_screenGrid[arr_i16_2f38[var_i16_68a*32 + var_i16_68c]].left,
		_screenGrid[arr_i16_2f38[var_i16_68a*32 + var_i16_68c]].top,
		arr_i32_192c0[_sunMapTileID[arr_i16_2f38[var_i16_68a*32+var_i16_68c]]]
	);
	waitForMouseUp();
	sunMapCheckIfSolved();
	_menuDisabled = false;
}


} // End of namespace Fool
